/* Two headless mGBA cores connected by mGBA's native GBA SIO lockstep driver.
 * The scheduling callbacks adapt cycle accounting from mGBA 0.10.5's
 * src/platform/qt/MultiplayerController.cpp, Copyright (c) 2013-2016
 * Jeffrey Pfau, under the Mozilla Public License 2.0:
 * https://www.mozilla.org/en-US/MPL/2.0/
 * All emulation runs on this one host thread. */
#include <mgba/flags.h>
#include <mgba/core/core.h>
#include <mgba/core/lockstep.h>
#include <mgba/core/log.h>
#include <mgba/gba/core.h>
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gba/sio/lockstep.h>
#include <mgba-util/configuration.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { PLAYERS = 2, WIDTH = 240, HEIGHT = 160, MAX_EVENTS = 256, MAX_WATCH = 16,
       MAX_CAPTURE = 16 };

struct InputEvent { unsigned frame, player, keys; };
struct Watch { char name[32]; uint32_t address; unsigned width; };
struct Runner {
    struct GBASIOLockstep link;
    struct GBASIOLockstepNode node[PLAYERS];
    struct mCore *core[PLAYERS];
    color_t *pixels[PLAYERS];
    int awake[PLAYERS];
    int32_t cyclesPosted[PLAYERS];
    unsigned waitMask;
    struct InputEvent input[MAX_EVENTS];
    unsigned inputCount;
    unsigned applied[PLAYERS];
    uint32_t e2eAbi, e2eRequest, e2eResult;
    unsigned e2eStatusOffset, e2eResultStatusOffset;
    int staged[PLAYERS];
    int ready;
    unsigned originFrame;
    unsigned stageX, stageY;
    uint32_t diagAddress;
    struct Watch watch[MAX_WATCH];
    unsigned watchCount;
    unsigned lastWatchFrame[PLAYERS];
    unsigned capture[MAX_CAPTURE];
    unsigned captureCount;
    int captured[PLAYERS][MAX_CAPTURE];
};

static struct Runner *runner(struct mLockstep *lockstep)
{
    return lockstep->context;
}

static bool signal_node(struct mLockstep *lockstep, unsigned mask)
{
    struct Runner *r = runner(lockstep);
    r->waitMask &= ~mask;
    if (!r->waitMask && !r->awake[0]) {
        r->awake[0] = 1;
        return true;
    }
    return false;
}

static bool wait_node(struct mLockstep *lockstep, unsigned mask)
{
    struct Runner *r = runner(lockstep);
    r->waitMask |= mask;
    if (r->awake[0]) {
        r->awake[0] = 0;
        return true;
    }
    return false;
}

static void add_cycles(struct mLockstep *lockstep, int id, int32_t cycles)
{
    struct Runner *r = runner(lockstep);
    if (cycles < 0) abort();
    if (id == 0) {
        for (int i = 1; i < PLAYERS; ++i) {
            if (r->node[i].d.p->mode > SIO_MULTI) continue;
            r->cyclesPosted[i] += cycles;
            if (!r->awake[i]) r->node[i].nextEvent += r->cyclesPosted[i];
            r->awake[i] = 1;
        }
    } else {
        r->cyclesPosted[id] += cycles;
    }
}

static int32_t use_cycles(struct mLockstep *lockstep, int id, int32_t cycles)
{
    struct Runner *r = runner(lockstep);
    r->cyclesPosted[id] -= cycles;
    if (r->cyclesPosted[id] <= 0) r->awake[id] = 0;
    return r->cyclesPosted[id];
}

static int32_t unused_cycles(struct mLockstep *lockstep, int id)
{
    return runner(lockstep)->cyclesPosted[id];
}

static void unload_node(struct mLockstep *lockstep, int id)
{
    struct Runner *r = runner(lockstep);
    if (id) {
        r->cyclesPosted[id] = 0;
        signal_node(lockstep, 1u << id);
    } else {
        for (int i = 1; i < PLAYERS; ++i) {
            r->cyclesPosted[i] += r->node[0].eventDiff;
            if (!r->awake[i]) r->node[i].nextEvent += r->cyclesPosted[i];
            r->awake[i] = 1;
        }
    }
}

static void log_message(struct mLogger *logger, int category, enum mLogLevel level,
                        const char *format, va_list args)
{
    (void)logger;
    if (!(level & (mLOG_FATAL | mLOG_ERROR | mLOG_WARN))) return;
    fprintf(stderr, "mGBA %s: ", mLogCategoryName(category));
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
}

static unsigned number(const char *text)
{
    char *end;
    errno = 0;
    unsigned long value = strtoul(text, &end, 0);
    if (errno || !text[0] || *end || value > UINT32_MAX) {
        fprintf(stderr, "invalid number: %s\n", text);
        exit(2);
    }
    return (unsigned)value;
}

static void load_input(struct Runner *r, const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file) { perror(path); exit(2); }
    char line[160];
    unsigned previous[PLAYERS] = { 0, 0 };
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        unsigned frame, player, keys;
        if (sscanf(line, "%u %u %x", &frame, &player, &keys) != 3
            || player >= PLAYERS || frame < previous[player]
            || r->inputCount == MAX_EVENTS) {
            fprintf(stderr, "invalid input line: %s", line);
            exit(2);
        }
        previous[player] = frame;
        r->input[r->inputCount++] = (struct InputEvent){ frame, player, keys };
    }
    fclose(file);
}

static void load_watch(struct Runner *r, const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file) { perror(path); exit(2); }
    char line[160];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n') continue;
        if (r->watchCount == MAX_WATCH) {
            fprintf(stderr, "too many watch entries\n");
            exit(2);
        }
        struct Watch *watch = &r->watch[r->watchCount];
        if (sscanf(line, "%31s %" SCNx32 " %u", watch->name, &watch->address,
                      &watch->width) != 3
            || (watch->width != 1 && watch->width != 2 && watch->width != 4)) {
            fprintf(stderr, "invalid watch line: %s", line);
            exit(2);
        }
        ++r->watchCount;
    }
    fclose(file);
}

static void load_capture(struct Runner *r, const char *path)
{
    FILE *file = fopen(path, "r");
    if (!file) { perror(path); exit(2); }
    unsigned frame;
    while (fscanf(file, "%u", &frame) == 1) {
        if (r->captureCount == MAX_CAPTURE) {
            fprintf(stderr, "too many screenshot frames\n");
            exit(2);
        }
        r->capture[r->captureCount++] = frame;
    }
    fclose(file);
}

static void snapshot(struct Runner *r, int player)
{
    struct mCore *core = r->core[player];
    unsigned frame = core->frameCounter(core);
    if (frame % 60 || frame == r->lastWatchFrame[player]) return;
    r->lastWatchFrame[player] = frame;
    printf("watch player=%d frame=%u mode=%d attachedMulti=%d phase=%d siocnt=%04x rcnt=%04x",
           player, frame, r->node[player].mode, r->link.attachedMulti,
           r->link.d.transferActive, core->busRead16(core, 0x04000128),
           core->busRead16(core, 0x04000134));
    for (unsigned i = 0; i < r->watchCount; ++i) {
        const struct Watch *w = &r->watch[i];
        unsigned value = w->width == 1 ? core->busRead8(core, w->address)
            : w->width == 2 ? core->busRead16(core, w->address)
            : core->busRead32(core, w->address);
        printf(" %s=%08x", w->name, value);
    }
    if (r->diagAddress) {
        uint32_t base = r->diagAddress;
        printf(" pocState=%u localMap=%u peerMap=%u localXY=%u,%u peerXY=%u,%u tx=%u rx=%u",
               core->busRead32(core, base + 12), core->busRead32(core, base + 28),
               core->busRead32(core, base + 32), core->busRead32(core, base + 36),
               core->busRead32(core, base + 40), core->busRead32(core, base + 44),
               core->busRead32(core, base + 48), core->busRead32(core, base + 60),
               core->busRead32(core, base + 64));
    }
    putchar('\n');
}

static void apply_input(struct Runner *r, int player)
{
    unsigned frame = r->core[player]->frameCounter(r->core[player]);
    if (!r->ready || frame < r->originFrame) return;
    frame -= r->originFrame;
    unsigned keys = 0;
    for (unsigned i = 0; i < r->inputCount; ++i)
        if (r->input[i].player == (unsigned)player && r->input[i].frame <= frame)
            keys = r->input[i].keys;
    if (keys != r->applied[player]) {
        r->core[player]->setKeys(r->core[player], keys);
        r->applied[player] = keys;
        printf("input player=%d frame=%u keys=0x%03x\n", player, frame, keys);
    }
}

static void stage_game(struct Runner *r, int player)
{
    struct mCore *core = r->core[player];
    unsigned frame = core->frameCounter(core);
    if (!r->e2eRequest || frame < 30) return;
    if (!r->staged[player]) {
        /* Existing test-only ROM mailbox. This arranges a fresh New Bark game;
         * packet transfer and player controls still use native GBA SIO/input. */
        if (core->busRead16(core, r->e2eAbi) != 23
            || core->busRead16(core, r->e2eAbi + 2) != 372
            || core->busRead16(core, r->e2eAbi + 8) != 87) {
            fprintf(stderr, "unsupported E2E request ABI for player %d\n", player);
            exit(1);
        }
        r->e2eStatusOffset = core->busRead16(core, r->e2eAbi + 8);
        r->e2eResultStatusOffset = core->busRead16(core, r->e2eAbi + 10);
        for (unsigned i = 0; i < 372; ++i) core->busWrite8(core, r->e2eRequest + i, 0);
        core->busWrite32(core, r->e2eRequest, player + 1); /* request ID */
        core->busWrite16(core, r->e2eRequest + 4, 0xffff); /* keep checkpoint map */
        core->busWrite16(core, r->e2eRequest + 6, 0xffff);
        core->busWrite16(core, r->e2eRequest + 8, r->stageX);
        core->busWrite16(core, r->e2eRequest + 10, r->stageY);
        core->busWrite32(core, r->e2eRequest + 12, player + 1); /* independent RNG */
        core->busWrite8(core, r->e2eRequest + 80, 2); /* new-bark-after-intro */
        core->busWrite8(core, r->e2eRequest + 81, 2); /* face up */
        core->busWrite8(core, r->e2eRequest + 84, 3); /* instant text */
        core->busWrite8(core, r->e2eRequest + 85, 1); /* seeded */
        core->busWrite8(core, r->e2eRequest + 86, 1); /* arrange */
        core->busWrite8(core, r->e2eRequest + r->e2eStatusOffset, 1); /* pending last */
        r->staged[player] = 1;
        printf("e2e_arrange_requested player=%d frame=%u\n", player, frame);
    }
    if (r->staged[player] == 1) {
        unsigned status = core->busRead8(core, r->e2eRequest + r->e2eStatusOffset);
        if (status == 4) {
            fprintf(stderr, "e2e arrange failed player=%d error=%u\n", player,
                    core->busRead16(core, r->e2eResult + 12));
            exit(1);
        }
        if (status == 3) {
            r->staged[player] = 2;
            printf("e2e_arrange_ready player=%d frame=%u\n", player, frame);
        }
    }
    if (!r->ready && r->staged[0] == 2 && r->staged[1] == 2) {
        unsigned f0 = r->core[0]->frameCounter(r->core[0]);
        unsigned f1 = r->core[1]->frameCounter(r->core[1]);
        r->originFrame = f0 > f1 ? f0 : f1;
        r->ready = 1;
        printf("input_origin_frame=%u\n", r->originFrame);
    }
}

static void screenshot(const color_t *pixels, const char *path)
{
    FILE *file = fopen(path, "wb");
    if (!file) { perror(path); return; }
    fprintf(file, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
    for (unsigned i = 0; i < WIDTH * HEIGHT; ++i) {
#ifdef COLOR_16_BIT
        fputc(M_R8(pixels[i]), file);
        fputc(M_G8(pixels[i]), file);
        fputc(M_B8(pixels[i]), file);
#else
        fputc(pixels[i] & 0xff, file);
        fputc((pixels[i] >> 8) & 0xff, file);
        fputc((pixels[i] >> 16) & 0xff, file);
#endif
    }
    fclose(file);
}

static void dump_diag(struct mCore *core, const char *path, uint32_t address, unsigned size)
{
    FILE *file = fopen(path, "wb");
    if (!file) { perror(path); return; }
    for (unsigned i = 0; i < size; ++i) {
        unsigned char value = core->busRead8(core, address + i);
        fwrite(&value, 1, 1, file);
    }
    fclose(file);
}

int main(int argc, char **argv)
{
    if (argc != 15) {
        fprintf(stderr, "usage: link ROM0 ROM1 FRAMES INPUT_SCRIPT WATCH_SCRIPT CAPTURE_SCRIPT DIAG_ADDR DIAG_SIZE DISCONNECT_FRAME E2E_ABI E2E_REQUEST E2E_RESULT STAGE_X STAGE_Y\n");
        return 2;
    }
    unsigned frames = number(argv[3]);
    uint32_t diag = number(argv[7]);
    unsigned diagSize = number(argv[8]);
    unsigned disconnectFrame = number(argv[9]);
    if (!frames || diagSize > 1024 || (disconnectFrame && disconnectFrame >= frames)) return 2;

    struct Runner r = { 0 };
    r.diagAddress = diag;
    r.e2eAbi = number(argv[10]);
    r.e2eRequest = number(argv[11]);
    r.e2eResult = number(argv[12]);
    r.stageX = number(argv[13]);
    r.stageY = number(argv[14]);
    r.ready = !r.e2eRequest;
    struct mLogger logger = { .log = log_message };
    mLogSetDefaultLogger(&logger);
    load_input(&r, argv[4]);
    load_watch(&r, argv[5]);
    load_capture(&r, argv[6]);
    mLockstepInit(&r.link.d);
    GBASIOLockstepInit(&r.link);
    r.link.d.context = &r;
    r.link.d.signal = signal_node;
    r.link.d.wait = wait_node;
    r.link.d.addCycles = add_cycles;
    r.link.d.useCycles = use_cycles;
    r.link.d.unusedCycles = unused_cycles;
    r.link.d.unload = unload_node;

    for (int i = 0; i < PLAYERS; ++i) {
        r.core[i] = GBACoreCreate();
        r.pixels[i] = calloc(WIDTH * HEIGHT, sizeof(color_t));
        if (!r.core[i] || !r.pixels[i] || !r.core[i]->init(r.core[i])) {
            fprintf(stderr, "could not initialize player %d\n", i);
            return 1;
        }
        mCoreInitConfig(r.core[i], "wayfarer-multiplayer-poc");
        ConfigurationSetValue(mCoreConfigGetOverrides(&r.core[i]->config),
                              "override.BWFE", "hardware", "1");
        mCoreLoadForeignConfig(r.core[i], &r.core[i]->config);
        if (!mCoreLoadFile(r.core[i], argv[1 + i])) {
            fprintf(stderr, "could not load ROM for player %d\n", i);
            return 1;
        }
        r.core[i]->setVideoBuffer(r.core[i], r.pixels[i], WIDTH);
        r.core[i]->reset(r.core[i]);
        GBASIOLockstepNodeCreate(&r.node[i]);
        if (!GBASIOLockstepAttachNode(&r.link, &r.node[i])) return 1;
        struct GBA *gba = r.core[i]->board;
        GBASIOSetDriver(&gba->sio, &r.node[i].d, SIO_MULTI);
        GBASIOSetDriver(&gba->sio, &r.node[i].d, SIO_NORMAL_32);
        r.awake[i] = 1;
    }
    printf("attached=%d native_sio_multi=1 native_sio_normal32=1\n", r.link.d.attached);
    fflush(stdout);

    uint64_t loops = 0;
    int disconnected = 0;
    int result = 0;
    while (!r.ready || r.core[0]->frameCounter(r.core[0]) < r.originFrame + frames
        || r.core[1]->frameCounter(r.core[1]) < r.originFrame + frames) {
        unsigned active = 0;
        for (int i = 0; i < PLAYERS; ++i) {
            if (!r.awake[i]) continue;
            stage_game(&r, i);
            apply_input(&r, i);
            snapshot(&r, i);
            if (r.ready) {
                unsigned frame = r.core[i]->frameCounter(r.core[i]);
                if (frame >= r.originFrame)
                    for (unsigned j = 0; j < r.captureCount; ++j)
                        if (!r.captured[i][j] && frame >= r.originFrame + r.capture[j]) {
                            char path[48];
                            snprintf(path, sizeof(path), "player%d-frame%04u.ppm", i, r.capture[j]);
                            screenshot(r.pixels[i], path);
                            r.captured[i][j] = 1;
                        }
            }
            r.core[i]->runLoop(r.core[i]);
            ++active;
        }
        if (++loops % 100000 == 0) {
            unsigned f0 = r.core[0]->frameCounter(r.core[0]);
            unsigned f1 = r.core[1]->frameCounter(r.core[1]);
            printf("loop=%" PRIu64 " frames=%u,%u awake=%d,%d phase=%d posted=%d,%d\n",
                   loops, f0, f1, r.awake[0], r.awake[1], r.link.d.transferActive,
                   r.cyclesPosted[0], r.cyclesPosted[1]);
            fflush(stdout);
        }
        if (!active || loops > (uint64_t)(frames + 3600) * 10000) {
            fprintf(stderr, "scheduler stalled: loops=%" PRIu64 " frames=%u,%u awake=%d,%d phase=%d\n",
                    loops, r.core[0]->frameCounter(r.core[0]), r.core[1]->frameCounter(r.core[1]),
                    r.awake[0], r.awake[1], r.link.d.transferActive);
            result = 1;
            break;
        }
        if (disconnectFrame && !disconnected
            && r.ready && r.core[0]->frameCounter(r.core[0]) >= r.originFrame + disconnectFrame
            && r.core[1]->frameCounter(r.core[1]) >= r.originFrame + disconnectFrame) {
            struct GBA *gba = r.core[1]->board;
            GBASIOSetDriver(&gba->sio, NULL, SIO_MULTI);
            GBASIOSetDriver(&gba->sio, NULL, SIO_NORMAL_32);
            GBASIOLockstepDetachNode(&r.link, &r.node[1]);
            r.awake[0] = r.awake[1] = 1;
            disconnected = 1;
            printf("cable_removed frame=%u\n", disconnectFrame);
        }
    }
    for (int i = 0; i < PLAYERS; ++i) {
        char path[32];
        snprintf(path, sizeof(path), "player%d.ppm", i);
        screenshot(r.pixels[i], path);
        if (diagSize) {
            snprintf(path, sizeof(path), "player%d.diag", i);
            dump_diag(r.core[i], path, diag, diagSize);
        }
        printf("player=%d frames=%u keys=0x%x sio_mode=%d\n", i,
               r.core[i]->frameCounter(r.core[i]), r.applied[i], r.node[i].mode);
    }
    fflush(stdout);
    /* Deliberately let process teardown release cores and drivers. The lockstep
     * driver has pointers into each core and deinit ordering is subtle. */
    return result;
}
