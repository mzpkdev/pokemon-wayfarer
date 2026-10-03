// Host tests of the engine seam (src/wayfarer_world.c), compiled with
// prelude.h standing in for the engine (critic loop cycle 2, C9): the seam
// heartbeat queue gives the same records as heartbeats run whole at each map
// load, a full queue finishes one heartbeat at a seam, a forced finish with
// no heap drops the queue, a pending heartbeat stops waiting for heap, and
// OnLoad clears a stale local actor block but rejects a corrupt one.
// Prints "ok <name>" per passing check; exits non-zero on the first failure.
#include <stdio.h>
#include <stdlib.h>
#include "wayfarer_world.h"
#include "constants/map_groups.h"

static struct SaveBlock1 sSaveBlock1;
static struct SaveBlock3 sSaveBlock3;
static struct PokemonStorage sStorage;
struct SaveBlock1 *gSaveBlock1Ptr = &sSaveBlock1;
struct SaveBlock3 *gSaveBlock3Ptr = &sSaveBlock3;
struct PokemonStorage *gPokemonStoragePtr = &sStorage;
struct PaletteFadeControl gPaletteFade;
struct WayfarerWalkersDebug gWayfarerWalkersDebug;
u16 gHostVcount;

static int sAllocFails, sCompleted;
static u32 sClock;

bool32 GetAcceptedLeagueEventMember(u8 match, const struct LeagueSavedTeam **team) { (void)match; (void)team; return FALSE; }
bool32 SelectLeagueLineup(u8 l, u32 w, const u32 r[LEAGUE_LINEUP_SIZE], u64 a, u64 b, struct LeagueLineupSelection *o)
{ (void)l; (void)w; (void)r; (void)a; (void)b; (void)o; return FALSE; }
bool8 GetBadgeStateForRegion(u8 region, u8 badgeIndex) { (void)region; (void)badgeIndex; return TRUE; }
u32 GetTrainerRating(void) { return 40; }
void *Alloc(u32 size) { return sAllocFails ? NULL : malloc(size); }
void Free(void *pointer) { free(pointer); }
static const struct MapLayout sLayout = {40, 40};
static const struct MapHeader sHeader = {&sLayout};
const struct MapHeader *Overworld_GetMapHeaderByGroupAndId(u16 g, u16 n) { (void)g; (void)n; return &sHeader; }
void WayfarerWalkers_Reset(void) {}
void WayfarerWalkers_OnContinue(void) {}
void WayfarerWalkers_FlushWorldJobs(void) {}
u32 WayfarerWalkers_ScanlineStamp(void) { return sClock; }
u32 WayfarerWalkers_ScanlinesSince(u32 start) { sClock += 2; return sClock - start; }
u32 WayfarerWalkers_LastUpdateScanlines(void) { return 10; }
void WayfarerWalkers_NoteHeartbeat(u32 a, u32 b) { (void)a; (void)b; sCompleted++; }

#define CHECK(cond, name) do { if (!(cond)) { printf("FAIL %s (line %d)\n", name, __LINE__); exit(1); } } while (0)

static const u16 sMaps[] = {MAP_VIRIDIAN_CITY_HNS, MAP_ROUTE2_HNS, MAP_PALLET_TOWN_HNS, MAP_ROUTE1_HNS,
                            MAP_PEWTER_CITY_HNS, MAP_ROUTE22_HNS};

static void SetMap(u16 map)
{
    gSaveBlock1Ptr->location.mapGroup = map >> 8;
    gSaveBlock1Ptr->location.mapNum = map & 0xFF;
}

static u32 sRng = 12345;
static u32 Next(void) { sRng = sRng * 1103515245u + 12345u; return (sRng >> 16) & 0x7FFF; }

// Map loads (seams and warps, with 0-6 field frames between them) through
// the engine, against a whole heartbeat at each load from the same records.
static void TestQueueMatchesWhole(void)
{
    static struct WayfarerWorldState reference;
    void *workspace = malloc(WorldSim_WorkspaceSize());
    u16 loads[400];
    u8 seams[400], frames[400];
    int i, n = 400, deferred;

    for (i = 0; i < n; i++)
    {
        loads[i] = sMaps[Next() % ARRAY_COUNT(sMaps)];
        seams[i] = Next() % 4 != 0;
        frames[i] = Next() % 7;
    }
    SetMap(sMaps[0]);
    WayfarerWorld_InitNewGame();
    reference = *WayfarerWorld_GetState();
    SetMap(sMaps[0]);
    WayfarerWorld_OnMapLoad(FALSE);     // New Game's first warp: skipped
    for (i = 0; i < n; i++)
    {
        int f;
        SetMap(loads[i]);
        WayfarerWorld_OnMapLoad(seams[i]);
        for (f = 0; f < frames[i]; f++)
            WayfarerWorld_Update();
    }
    WayfarerWorld_FinishHeartbeat();
    deferred = gWayfarerWorldDebug.deferredBegins;

    // The reference: the same loads, each heartbeat whole at its load (a load
    // of the map just loaded is no heartbeat).
    {
        u16 last = sMaps[0];
        WorldSim_ResetPathCache();
        for (i = 0; i < n; i++)
        {
            struct WayfarerWorldContext ctx;
            if (loads[i] == last)
                continue;
            last = loads[i];
            SetMap(loads[i]);
            WayfarerWorld_BuildContext(&ctx);
            ctx.frozenMask = 0;
            WorldSim_Heartbeat(&reference, &ctx, workspace, NULL);
        }
    }
    CHECK(deferred > 10, "the run queued seam heartbeats");
    CHECK(memcmp(&reference, WayfarerWorld_GetState(), sizeof(reference)) == 0, "queued == whole");
    printf("ok queue matches whole (%d loads, %d queued)\n", n, deferred);
    free(workspace);
}

// A seam with the queue full: one heartbeat finishes in that load, the next
// begins, and the new one joins the queue.
static void TestFullQueueFinishesOne(void)
{
    int before;
    SetMap(sMaps[0]);
    WayfarerWorld_InitNewGame();
    WayfarerWorld_OnMapLoad(FALSE);
    SetMap(sMaps[1]); WayfarerWorld_OnMapLoad(TRUE);    // active
    SetMap(sMaps[0]); WayfarerWorld_OnMapLoad(TRUE);    // queued 1
    SetMap(sMaps[1]); WayfarerWorld_OnMapLoad(TRUE);    // queued 2 (full)
    before = sCompleted;
    SetMap(sMaps[0]); WayfarerWorld_OnMapLoad(TRUE);
    CHECK(sCompleted - before == 1, "one heartbeat finished at the full seam");
    CHECK(WayfarerWorld_IsHeartbeatPending(), "the rest stay pending");
    WayfarerWorld_FinishHeartbeat();
    CHECK(sCompleted - before == 4, "all four ran in the end");
    printf("ok full queue finishes one\n");
}

// A forced finish without heap skips the active heartbeat and the queue.
static void TestNoHeapForcedFinish(void)
{
    u16 skipped;
    SetMap(sMaps[0]);
    WayfarerWorld_InitNewGame();
    WayfarerWorld_OnMapLoad(FALSE);
    SetMap(sMaps[1]); WayfarerWorld_OnMapLoad(TRUE);    // active, no workspace yet
    SetMap(sMaps[0]); WayfarerWorld_OnMapLoad(TRUE);    // queued
    skipped = gWayfarerWorldDebug.skippedHeartbeats;
    sAllocFails = 1;
    WayfarerWorld_FinishHeartbeat();
    sAllocFails = 0;
    CHECK(!WayfarerWorld_IsHeartbeatPending(), "nothing left pending");
    CHECK(gWayfarerWorldDebug.skippedHeartbeats - skipped == 2, "both counted skipped");
    printf("ok no-heap forced finish drops the queue\n");
}

// A pending heartbeat without heap stops waiting after a while.
static void TestWorkspaceWaitCap(void)
{
    int f;
    SetMap(sMaps[0]);
    WayfarerWorld_InitNewGame();
    WayfarerWorld_OnMapLoad(FALSE);
    SetMap(sMaps[1]); WayfarerWorld_OnMapLoad(TRUE);
    sAllocFails = 1;
    for (f = 0; f < 59; f++)
        WayfarerWorld_Update();
    CHECK(WayfarerWorld_IsHeartbeatPending(), "still waiting at 59 frames");
    WayfarerWorld_Update();
    sAllocFails = 0;
    CHECK(!WayfarerWorld_IsHeartbeatPending(), "gave up at 60 frames");
    printf("ok workspace wait is capped\n");
}

// OnLoad: a local actor entry off the saved map is cleared; a slot listed
// twice is a corrupt save.
static void TestOnLoadLocalActors(void)
{
    struct WayfarerWorldState *state = WayfarerWorld_GetState();
    u8 slot = 8, other;
    SetMap(sMaps[0]);
    WayfarerWorld_InitNewGame();
    for (other = 0; other < WORLD_SIM_TRAINER_COUNT; other++)
        if (WorldSim_IsSimulated(&state->records[other]) && WorldSim_NodeMap(state->records[other].node) != sMaps[0])
            break;
    slot = other;
    CHECK(slot < WORLD_SIM_TRAINER_COUNT, "a trainer off the saved map");
    WorldSim_SetLocalActor(state, 0, slot, 3, 4, 2);
    CHECK(WayfarerWorld_OnLoad(), "a stale block keeps the save");
    CHECK((state->localActors[0] & 0x3F) == WORLD_LOCAL_ACTOR_NONE, "and is cleared");
    WorldSim_SetLocalActor(state, 0, slot, 3, 4, 2);
    WorldSim_SetLocalActor(state, 1, slot, 5, 5, 2);
    CHECK(!WayfarerWorld_OnLoad(), "a slot listed twice is corrupt");
    printf("ok OnLoad clears a stale block, rejects a corrupt one\n");
}

int main(void)
{
    gHostVcount = 0;     // early in the frame: a full budget
    TestQueueMatchesWhole();
    TestFullQueueFinishesOne();
    TestNoHeapForcedFinish();
    TestWorkspaceWaitCap();
    TestOnLoadLocalActors();
    printf("all ok\n");
    return 0;
}
