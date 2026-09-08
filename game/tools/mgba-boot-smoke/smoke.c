/* flags.h must precede other mGBA headers: its feature flags affect the ABI. */
#include <mgba/flags.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba-util/configuration.h>
#include <mgba/gba/core.h>
#include <mgba-util/vfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* GBA layouts from include/main.h and include/task.h (32-bit pointers). */
enum { CALLBACK2_OFFSET = 4, TASK_SIZE = 40, TASK_ACTIVE_OFFSET = 4,
       TASK_DATA_OFFSET = 8, TASK_COUNT = 16, WIDTH = 240, HEIGHT = 160 };

static uint32_t address(const char *text)
{
    return (uint32_t)strtoul(text, NULL, 0);
}

static int same_function(uint32_t a, uint32_t b)
{
    return (a & ~1u) == (b & ~1u);
}

static void log_message(struct mLogger *logger, int category, enum mLogLevel level,
                        const char *format, va_list args)
{
    (void)logger;
    if (!(level & (mLOG_FATAL | mLOG_ERROR)))
        return;
    fprintf(stderr, "mGBA %s: ", mLogCategoryName(category));
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
}

int main(int argc, char **argv)
{
    if (argc != 8)
    {
        fprintf(stderr, "usage: smoke ROM FRAMES gMain gTasks CB2_MainMenu Task_HandleMainMenuInput gFlashMemoryPresent\n");
        return 2;
    }
    unsigned limit = address(argv[2]);
    uint32_t main_address = address(argv[3]);
    uint32_t tasks = address(argv[4]);
    uint32_t menu = address(argv[5]);
    uint32_t input = address(argv[6]);
    uint32_t flash = address(argv[7]);
    struct mLogger logger = { .log = log_message };
    mLogSetDefaultLogger(&logger);
    struct mCore *core = GBACoreCreate();
    color_t *pixels = calloc(WIDTH * HEIGHT, sizeof(*pixels));
    if (!core || !pixels || !core->init(core))
    {
        fprintf(stderr, "FAIL: could not initialize mGBA\n");
        return 1;
    }
    mCoreInitConfig(core, "wayfarer-boot-smoke");
    /* Do not load user config, a save file, or any save-type override. */
    /* BWFE is not in mGBA's cartridge database. Enable its RTC peripheral
     * (hardware mask 1) without specifying or touching save memory. */
    ConfigurationSetValue(mCoreConfigGetOverrides(&core->config),
                          "override.BWFE", "hardware", "1");
    mCoreLoadForeignConfig(core, &core->config);
    if (!mCoreLoadFile(core, argv[1]))
    {
        fprintf(stderr, "FAIL: could not load ROM %s\n", argv[1]);
        return 1;
    }
    core->setVideoBuffer(core, pixels, WIDTH);
    core->reset(core);
    unsigned stable = 0;
    int passed = 0;
    for (unsigned frame = 0; frame < limit; ++frame)
    {
        /* Start skips the intro/title but cannot select New Game or dismiss
         * the main menu's save/battery error windows, which require A. */
        core->setKeys(core, frame % 60 < 2 ? 1u << 3 : 0);
        core->runFrame(core);
        int ready = 0;
        if (same_function(core->busRead32(core, main_address + CALLBACK2_OFFSET), menu))
            for (unsigned i = 0; i < TASK_COUNT; ++i)
            {
                uint32_t task = tasks + i * TASK_SIZE;
                if (core->busRead8(core, task + TASK_ACTIVE_OFFSET)
                    && same_function(core->busRead32(core, task), input)
                    && core->busRead16(core, task + TASK_DATA_OFFSET) == 0)
                    ready = 1;
            }
        stable = ready ? stable + 1 : 0;
        if (frame % 600 == 0 || stable == 30)
        {
            printf("frame=%u callback2=0x%08x flash_present=%u menu_ready=%d\n",
                   frame + 1, core->busRead32(core, main_address + CALLBACK2_OFFSET),
                   core->busRead32(core, flash), ready);
            fflush(stdout);
        }
        if (stable == 30)
        {
            passed = 1;
            break;
        }
    }
    if (!passed)
    {
        fprintf(stderr, "FAIL: fresh-save main menu did not become ready within %u frames\n", limit);
        for (unsigned i = 0; i < TASK_COUNT; ++i)
        {
            uint32_t task = tasks + i * TASK_SIZE;
            if (core->busRead8(core, task + TASK_ACTIVE_OFFSET))
                fprintf(stderr, "active_task[%u]=0x%08x data[0]=%u\n", i,
                        core->busRead32(core, task), core->busRead16(core, task + TASK_DATA_OFFSET));
        }
    }
    struct VFile *png = VFileOpen("screen.png", O_CREAT | O_TRUNC | O_WRONLY);
    if (png)
    {
        mCoreTakeScreenshotVF(core, png);
        png->close(png);
    }
    printf("%s: main menu smoke\n", passed ? "PASS" : "FAIL");
    mCoreConfigDeinit(&core->config);
    core->deinit(core);
    free(pixels);
    return passed ? 0 : 1;
}
