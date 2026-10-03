// Engine side of the notable world simulation: builds the context the core
// reads from saved league state and badges, runs the map-change heartbeat
// (spread over the following field frames), seeds New Game, and validates
// the world state on load.
// Spec: .product/specs/notable-world-simulation.md.

#include "global.h"
#include "wayfarer_world.h"

#if IS_WAYFARER
#include "league_events.h"
#include "league_selection.h"
#include "malloc.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon_storage_system.h"
#include "trainer_rating.h"
#include "wayfarer_walkers.h"
#include "wayfarer_persistence.h"
#include "constants/regions.h"

EWRAM_DATA struct WayfarerWorldDebug gWayfarerWorldDebug = {0};
EWRAM_DATA u32 gWayfarerWorldFrozenMask = 0;
static EWRAM_DATA bool8 sSkipNextHeartbeat = FALSE;
static EWRAM_DATA u16 sLastMap = 0;

// The pending heartbeat. A map load starts it (Begin, cheap); every field
// frame then runs steps until the frame's share of the budget is spent.
// The workspace is allocated on the first step: a warp resets the heap
// right after LoadMapFromWarp. A heap reset loses it (never Free it then).
static EWRAM_DATA struct WayfarerWorldHeartbeat sHeartbeat = {0};
static EWRAM_DATA void *sHeartbeatWorkspace = NULL;
static EWRAM_DATA u32 sHeartbeatScanlines = 0;  // the pending heartbeat's work so far
static EWRAM_DATA u8 sStarvedFrames = 0;
// A seam crossed again before the last heartbeat ended: the new heartbeat's
// Begin waits for it, with the context as it was at that map load. The
// result is the same as finishing the pending one inside the load (nothing
// else writes the records meanwhile: walkers wait while a heartbeat is
// pending), without the multi-frame hitch in the seam's frame.
// Up to this many queue: running back and forth over a seam can cross again
// before even the queued heartbeat has begun. Kept packed (EWRAM is tight in
// the mechanics-test build): derived states at 4 bits each.
#define DEFERRED_MAX 2
#define DERIVED_PACKED_NONE 0xF
struct DeferredContext
{
    u32 worldProgress;
    u32 provisionalMask;
    u32 risingMask;
    u32 frozenMask;
    u16 playerMap;
    u8 derived[(WORLD_SIM_TRAINER_COUNT + 1) / 2];
};
STATIC_ASSERT(WORLD_STATE_COUNT <= DERIVED_PACKED_NONE, DerivedStatesFitANibble);
static EWRAM_DATA struct DeferredContext sDeferredContexts[DEFERRED_MAX] = {0};
static EWRAM_DATA u8 sDeferredCount = 0;
// Frames a pending heartbeat may wait for its workspace before the rest of
// it skips (the walkers pause while it is pending).
#define WORKSPACE_WAIT_MAX 60
static EWRAM_DATA u8 sWorkspaceWaitFrames = 0;
#define sDeferredBegin (sDeferredCount != 0)

static void PackContext(struct DeferredContext *packed, const struct WayfarerWorldContext *ctx)
{
    u8 slot;
    packed->worldProgress = ctx->worldProgress;
    packed->provisionalMask = ctx->provisionalMask;
    packed->risingMask = ctx->risingMask;
    packed->frozenMask = ctx->frozenMask;
    packed->playerMap = ctx->playerMap;
    memset(packed->derived, 0, sizeof(packed->derived));
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        u8 value = ctx->derived[slot] == WORLD_DERIVED_NONE ? DERIVED_PACKED_NONE : ctx->derived[slot];
        packed->derived[slot / 2] |= value << ((slot & 1) * 4);
    }
}

static void UnpackContext(struct WayfarerWorldContext *ctx, const struct DeferredContext *packed)
{
    u8 slot;
    *ctx = (struct WayfarerWorldContext){0};
    ctx->worldProgress = packed->worldProgress;
    ctx->provisionalMask = packed->provisionalMask;
    ctx->risingMask = packed->risingMask;
    ctx->frozenMask = packed->frozenMask;
    ctx->playerMap = packed->playerMap;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        u8 value = (packed->derived[slot / 2] >> ((slot & 1) * 4)) & 0xF;
        ctx->derived[slot] = value == DERIVED_PACKED_NONE ? WORLD_DERIVED_NONE : value;
    }
}

// Per field frame, the walkers' update and the simulation's steps together
// stay under this many scanlines (a frame is 228); the steps take what the
// walkers left. A heavy unit (one spot choice: about 20 lines on average,
// up to about 120) only ever starts a frame's slice, so it never lands on
// top of a spent budget.
#define FRAME_BUDGET_SCANLINES  88
// And they end this many lines before the next VBlank: the rest of the
// field frame after this hook (sprites, the camera, the palette fade) takes
// 16 to 32 lines, rarely 56, and an overrun is a lag frame.
#define FRAME_END_MARGIN        40
// A frame with no time left runs no steps, but never more than this many in
// a row: then one work unit runs anyway, so the heartbeat always moves on.
#define STARVED_FRAMES_MAX      8
// The same during a palette fade, which lasts well under a second.
#define FADE_FRAMES_MAX         60
// Work units per step call between scanline checks: one (a node expansion
// is one or two scanlines, a few trainers of a full-map check up to ~20).
#define STEP_UNITS              1

static const u8 sBadgeRegions[] =
{
    [WORLD_REGION_KANTO] = REGION_KANTO,
    [WORLD_REGION_JOHTO] = REGION_JOHTO,
    [WORLD_REGION_HOENN] = REGION_HOENN,
};

struct WayfarerWorldState *WayfarerWorld_GetState(void)
{
    return &gPokemonStoragePtr->wayfarerWorld;
}

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

static void MarkSlot(struct WayfarerWorldContext *ctx, u16 characterId, u8 state)
{
    u8 slot = WorldSim_SlotForCharacter(characterId);
    if (slot < WORLD_SIM_TRAINER_COUNT && ctx->derived[slot] == WORLD_DERIVED_NONE)
        ctx->derived[slot] = state;
}

// The provisional lineup: what selection would pick if the player accepted
// the waiting invitation now. Recomputed every time, never saved.
static u32 ProvisionalMask(u32 worldProgress)
{
    const struct LeagueEventState *league = &gSaveBlock3Ptr->leagueEvent;
    struct LeagueLineupSelection selection;
    u32 recent[LEAGUE_LINEUP_SIZE];
    u32 mask = 0;
    u8 i;

    if (league->invitationState != LEAGUE_INVITATION_INVITED)
        return 0;
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
        recent[i] = league->recentIds[i];
    if (!SelectLeagueLineup(league->invitedLeague, worldProgress, recent,
                            league->reignedIndigoMask, league->reignedHoennMask, &selection))
        return 0;
    for (i = 0; i < selection.count; i++)
    {
        u8 slot = WorldSim_SlotForCharacter(selection.members[i].characterId);
        if (slot < WORLD_SIM_TRAINER_COUNT)
            mask |= 1u << slot;
    }
    return mask;
}

void WayfarerWorld_BuildContext(struct WayfarerWorldContext *ctx)
{
    u8 slot, match;
    const struct LeagueSavedTeam *team;

    ctx->playerMap = CurrentMap();
    ctx->worldProgress = GetTrainerRating();
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
        ctx->derived[slot] = WORLD_DERIVED_NONE;

    // First row that applies wins: Away: league, Away: partner, Pinned,
    // Home-locked. No Masters partner or haunt placement exists yet.
    for (match = 0; match < LEAGUE_EVENT_LINEUP_SIZE; match++)
    {
        if (GetAcceptedLeagueEventMember(match, &team))
            MarkSlot(ctx, team->characterId, WORLD_STATE_AWAY_LEAGUE);
    }
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
        if (!(trainer->flags & WORLD_TRAINER_FLAG_GYM_LEADER) || trainer->badgeRegion >= ARRAY_COUNT(sBadgeRegions))
            continue;
        if (!GetBadgeStateForRegion(sBadgeRegions[trainer->badgeRegion], trainer->badgeIndex))
            MarkSlot(ctx, trainer->characterId, WORLD_STATE_HOME_LOCKED);
    }

    ctx->provisionalMask = ProvisionalMask(ctx->worldProgress);
    ctx->risingMask = 0;  // Momentum isn't implemented yet.
    ctx->frozenMask = gWayfarerWorldFrozenMask;
}

bool8 WayfarerWorld_IsLeaderUnbeaten(u8 slot)
{
    const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
    if (!(trainer->flags & WORLD_TRAINER_FLAG_GYM_LEADER) || trainer->badgeRegion >= ARRAY_COUNT(sBadgeRegions))
        return FALSE;
    return !GetBadgeStateForRegion(sBadgeRegions[trainer->badgeRegion], trainer->badgeIndex);
}

static void SetPending(bool8 pending)
{
    sHeartbeat.active = pending;
    gWayfarerWorldDebug.pending = pending || sDeferredBegin;
}

static u32 BeginHeartbeatWith(const struct WayfarerWorldContext *ctx, u32 contextScanlines);

static void FreeHeartbeatWorkspace(void)
{
    if (sHeartbeatWorkspace != NULL)
        Free(sHeartbeatWorkspace);
    sHeartbeatWorkspace = NULL;
}

// New Game, Continue and a save load replace the world state: whatever
// heartbeat was pending belonged to the old one.
static void DropHeartbeat(void)
{
    sDeferredCount = 0;
    sWorkspaceWaitFrames = 0;
    SetPending(FALSE);
    FreeHeartbeatWorkspace();
}

void WayfarerWorld_InitNewGame(void)
{
    struct WayfarerWorldContext ctx;
    void *workspace;

    DropHeartbeat();
    workspace = Alloc(WorldSim_WorkspaceSize());

    WayfarerWorld_BuildContext(&ctx);
    WorldSim_NewGame(WayfarerWorld_GetState(), &ctx, workspace);
    Free(workspace);
    gWayfarerWorldFrozenMask = 0;
    WayfarerWalkers_Reset();
    // The warp into the starting map is not a heartbeat.
    sSkipNextHeartbeat = TRUE;
    sLastMap = 0xFFFF;
}

void WayfarerWorld_OnContinue(bool8 loadsWarp)
{
    DropHeartbeat();
    gWayfarerWorldFrozenMask = 0;
    WayfarerWalkers_OnContinue();
    sSkipNextHeartbeat = loadsWarp;
    sLastMap = CurrentMap();
}

bool8 WayfarerWorld_OnLoad(void)
{
    struct WayfarerWorldState *state = WayfarerWorld_GetState();
    const struct MapHeader *header;
    struct WayfarerWorldContext ctx;
    void *workspace;
    bool8 valid;

    DropHeartbeat();
    WorldSim_ResetPathCache();  // EWRAM isn't cleared at boot
    if (state->schemaVersion != WORLD_SCHEMA_VERSION)
        return FALSE;
    // Without a workspace only the reachability check is skipped: a full
    // heap must never make a good save look invalid.
    workspace = Alloc(WorldSim_WorkspaceSize());
    WayfarerWorld_BuildContext(&ctx);
    if (state->contentHash != gWayfarerWorldContentHash)
    {
        // The graph or spot table changed: re-seat, never an invalid save.
        WorldSim_Reseat(state, &ctx, workspace);
        gWayfarerWorldDebug.reseats++;
    }
    header = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    valid = WorldSim_IsValid(state, workspace) && header != NULL && header->mapLayout != NULL;
    if (valid)
    {
        // The local actor block is a restore hint for the saved location.
        // A load-time repair that moved the location (the league run's
        // recovery to the lobby) leaves it stale: drop it, keep the save.
        // Malformed bytes still mean a corrupt save.
        switch (WorldSim_CheckLocalActors(state, CurrentMap(), header->mapLayout->width, header->mapLayout->height))
        {
        case WORLD_LOCAL_ACTORS_STALE:
            WorldSim_ClearLocalActors(state);
            break;
        case WORLD_LOCAL_ACTORS_CORRUPT:
            valid = FALSE;
            break;
        }
    }
    if (valid)
        WorldSim_ApplyDerivedOnLoad(state, &ctx);
    if (workspace != NULL)
        Free(workspace);
    return valid;
}

// One frame's heartbeat work (Begin, or a frame's steps), and the frame's
// total with the walkers' update that ran before it in the same frame.
static void NoteFrameCost(u32 scanlines)
{
    u32 frame = scanlines + WayfarerWalkers_LastUpdateScanlines();
    sHeartbeatScanlines += scanlines;
    if (scanlines > gWayfarerWorldDebug.maxStepScanlines)
        gWayfarerWorldDebug.maxStepScanlines = scanlines > 0xFFFF ? 0xFFFF : scanlines;
    if (frame > gWayfarerWorldDebug.maxFrameScanlines)
        gWayfarerWorldDebug.maxFrameScanlines = frame > 0xFFFF ? 0xFFFF : frame;
}

static bool8 EnsureHeartbeatWorkspace(void)
{
    if (sHeartbeatWorkspace == NULL)
    {
        sHeartbeatWorkspace = Alloc(WorldSim_WorkspaceSize());
        if (sHeartbeatWorkspace == NULL)
            return FALSE;
        // A new block: its contents are garbage, so no search marks can be
        // trusted (the first one after Begin clears them anyway).
        WorldSim_HeartbeatLostWorkspace(&sHeartbeat);
    }
    return TRUE;
}

static void CompleteHeartbeat(void)
{
    SetPending(FALSE);
    WayfarerWalkers_NoteHeartbeat(sHeartbeatScanlines, gWayfarerWalkersDebug.lastContextScanlines);
    gWayfarerWorldDebug.heartbeats++;
    if (gWayfarerWorldDebug.lastHeartbeatFrames > gWayfarerWorldDebug.maxHeartbeatFrames)
        gWayfarerWorldDebug.maxHeartbeatFrames = gWayfarerWorldDebug.lastHeartbeatFrames;
    // A queued heartbeat (a seam crossed again) begins on its own turn,
    // never on top of this one's frame; it reuses the workspace.
    if (!sDeferredBegin)
        FreeHeartbeatWorkspace();
}

// Begins the oldest queued heartbeat, as it was at its map load. Returns
// its cost; the caller notes it with the rest of its frame.
static u32 BeginDeferred(void)
{
    struct WayfarerWorldContext ctx;
    UnpackContext(&ctx, &sDeferredContexts[0]);
    sDeferredCount--;
    memmove(&sDeferredContexts[0], &sDeferredContexts[1], sizeof(sDeferredContexts[0]) * sDeferredCount);
    return BeginHeartbeatWith(&ctx, 0);
}

// Runs the active heartbeat to its end now (only that one).
static void FinishActive(void)
{
    u32 start, cost;

    if (!sHeartbeat.active)
        return;
    start = WayfarerWalkers_ScanlineStamp();
    if (!EnsureHeartbeatWorkspace())
    {
        // No heap for a search: the trainers still to act skip this
        // heartbeat, as a whole heartbeat used to when its allocation
        // failed, and so do the queued ones (they would fail alike, and must
        // not run later, out of order).
        gWayfarerWorldDebug.skippedHeartbeats += 1 + sDeferredCount;
        sDeferredCount = 0;
        SetPending(FALSE);
        return;
    }
    while (!WorldSim_HeartbeatStep(&sHeartbeat, WayfarerWorld_GetState(), sHeartbeatWorkspace, 0xFFFF,
                                   &gWayfarerWorldDebug.lastTrace))
        ;
    cost = WayfarerWalkers_ScanlinesSince(start);
    sHeartbeatScanlines += cost;
    if (cost > gWayfarerWorldDebug.maxFinishScanlines)
        gWayfarerWorldDebug.maxFinishScanlines = cost > 0xFFFF ? 0xFFFF : cost;
    CompleteHeartbeat();
}

// Runs whatever is left of the pending heartbeat, and of any queued behind
// it, now. Every world write outside the heartbeat (a save, the league
// hook, a warp's map load, a Gym leader's object) comes after this, so it
// sees the heartbeats whole, as when they ran inside their map loads.
void WayfarerWorld_FinishHeartbeat(void)
{
    while (sHeartbeat.active || sDeferredBegin)
    {
        if (!sHeartbeat.active)
            BeginDeferred();
        FinishActive();
    }
}

bool8 WayfarerWorld_IsHeartbeatPending(void)
{
    return sHeartbeat.active || sDeferredBegin;
}

bool8 WayfarerWorld_IsSlotPending(u8 slot)
{
    // A deferred heartbeat will act for every trainer it didn't find frozen.
    u8 i;
    for (i = 0; i < sDeferredCount; i++)
    {
        if (slot < WORLD_SIM_TRAINER_COUNT && !(sDeferredContexts[i].frozenMask & (1u << slot)))
            return TRUE;
    }
    return WorldSim_HeartbeatIsPending(&sHeartbeat, slot);
}

void WayfarerWorld_FinishHeartbeatEarly(void)
{
    // A save or the league hook. The pending heartbeat finishes first, then
    // the walkers' own sliced world writes (a watched trainer's routine
    // advance, which a seam can leave queued behind a heartbeat): the same
    // order as when they run out on their own (walkers wait while a
    // heartbeat is pending).
    if (WayfarerWorld_IsHeartbeatPending())
    {
        gWayfarerWorldDebug.forcedFinishes++;
        WayfarerWorld_FinishHeartbeat();
    }
    WayfarerWalkers_FlushWorldJobs();
}

static void BeginHeartbeat(void)
{
    struct WayfarerWorldContext ctx;
    u32 start = WayfarerWalkers_ScanlineStamp(), contextDone;

    WayfarerWorld_BuildContext(&ctx);
    contextDone = WayfarerWalkers_ScanlineStamp();
    NoteFrameCost(BeginHeartbeatWith(&ctx, (s32)(contextDone - start) < 0 ? 0 : contextDone - start));
}

static u32 BeginHeartbeatWith(const struct WayfarerWorldContext *contextIn, u32 contextScanlines)
{
    struct WayfarerWorldContext ctx = *contextIn;
    u32 start = WayfarerWalkers_ScanlineStamp();

    gWayfarerWalkersDebug.lastContextScanlines = contextScanlines;
    gWayfarerWorldDebug.lastTrace = (struct WayfarerWorldTrace){0};
    gWayfarerWorldDebug.lastHeartbeatFrames = 0;
    gWayfarerWorldDebug.lastHeartbeatMap = ctx.playerMap;
    sHeartbeatScanlines = 0;
    // Fixes who acts and in what order, with the frozen mask and the
    // player's map as they are now; the walkers' later changes to the
    // frozen mask don't reach this heartbeat.
    WorldSim_HeartbeatBegin(&sHeartbeat, WayfarerWorld_GetState(), &ctx, &gWayfarerWorldDebug.lastTrace);
    gWayfarerWorldDebug.pending = TRUE;
    return WayfarerWalkers_ScanlinesSince(start) + contextScanlines;
}

// Once per field frame (OverworldBasic, after the walkers' update). Scripts,
// locked controls and menus don't stop it: it only moves off-screen records,
// and the walkers keep their hands off the world while it runs.
void WayfarerWorld_Update(void)
{
    u32 start, walkers, budget, line, cost = 0;
    bool8 done;

    if (!sHeartbeat.active && !sDeferredBegin)
        return;
    gWayfarerWorldDebug.lastHeartbeatFrames++;
    walkers = WayfarerWalkers_LastUpdateScanlines();
    budget = walkers < FRAME_BUDGET_SCANLINES ? FRAME_BUDGET_SCANLINES - walkers : 0;
    // Lines since this frame's VBlank began; the frame ends at 228.
    line = (REG_VCOUNT + 68) % 228;
    if (line + FRAME_END_MARGIN >= 228)
        budget = 0;
    else if (budget > 228 - FRAME_END_MARGIN - line)
        budget = 228 - FRAME_END_MARGIN - line;
    // A palette fade (a warp's fade-in) already fills the frame: wait for it.
    if (gPaletteFade.active || budget == 0)
    {
        if (++sStarvedFrames < (gPaletteFade.active ? FADE_FRAMES_MAX : STARVED_FRAMES_MAX))
            return;
        budget = 0;  // just the one unit
    }
    sStarvedFrames = 0;
    start = WayfarerWalkers_ScanlineStamp();
    // A queued heartbeat begins as this frame's first unit: its cost counts
    // with the frame's steps, inside the frame's budget and end margin.
    if (!sHeartbeat.active)
        BeginDeferred();
    if (!EnsureHeartbeatWorkspace())
    {
        // No heap for a search right now: try again next frame, so the
        // heartbeat is never split between trainers who acted and trainers
        // who skipped. The walkers wait meanwhile, so not for ever: after
        // WORKSPACE_WAIT_MAX frames the rest of it skips, as a heartbeat
        // whose allocation failed always did.
        gWayfarerWorldDebug.workspaceWaits++;
        NoteFrameCost(WayfarerWalkers_ScanlinesSince(start));
        if (++sWorkspaceWaitFrames >= WORKSPACE_WAIT_MAX)
        {
            sWorkspaceWaitFrames = 0;
            gWayfarerWorldDebug.skippedHeartbeats++;
            SetPending(FALSE);
        }
        return;
    }
    sWorkspaceWaitFrames = 0;
    do
    {
        done = WorldSim_HeartbeatStep(&sHeartbeat, WayfarerWorld_GetState(), sHeartbeatWorkspace, STEP_UNITS,
                                      &gWayfarerWorldDebug.lastTrace);
    } while (!done && WayfarerWalkers_ScanlinesSince(start) < budget && !WorldSim_HeartbeatNextIsHeavy(&sHeartbeat));
    cost = WayfarerWalkers_ScanlinesSince(start);
    NoteFrameCost(cost);
    if (done)
        CompleteHeartbeat();
}

void WayfarerWorld_OnHeapReset(void)
{
#if TESTING
    // The test runner resets the heap between tests: nothing carries over.
    SetPending(FALSE);
#endif
    // InitHeap overwrote the allocator: the block is gone, never Free it.
    // The trainer whose search was under way searches again on the next
    // frame with a new workspace; trainers that already acted stay done.
    if (sHeartbeatWorkspace == NULL)
        return;
    sHeartbeatWorkspace = NULL;
    if (sHeartbeat.active)
    {
        WorldSim_HeartbeatLostWorkspace(&sHeartbeat);
        gWayfarerWorldDebug.workspaceLosses++;
    }
}

void WayfarerWorld_OnMapLoad(bool8 seam)
{
    u16 map = CurrentMap();
    bool8 defer = FALSE;

    // A map load while the last heartbeat is still running: heartbeats never
    // overlap and keep their order. Behind a warp's fade the rest runs now;
    // on a seam the new heartbeat queues behind it instead.
    if (sHeartbeat.active || sDeferredBegin)
    {
        gWayfarerWorldDebug.pendingAtLoad++;
        if (!seam)
        {
            WayfarerWorld_FinishHeartbeat();
        }
        else
        {
            // With the queue full, only the oldest heartbeat runs to its
            // end here; the next queued one begins (sliced) to make room.
            if (sDeferredCount >= DEFERRED_MAX)
            {
                if (!sHeartbeat.active)
                    NoteFrameCost(BeginDeferred());
                FinishActive();
                if (sDeferredBegin)
                    NoteFrameCost(BeginDeferred());
            }
            defer = sHeartbeat.active || sDeferredBegin;
        }
    }
    if (sSkipNextHeartbeat || map == sLastMap)
    {
        // Continue, New Game's first warp, or a script reloading the same map.
        sSkipNextHeartbeat = FALSE;
        sLastMap = map;
        gWayfarerWorldDebug.skippedHeartbeats++;
        return;
    }
    sLastMap = map;
    if (defer)
    {
        struct WayfarerWorldContext ctx;
        WayfarerWorld_BuildContext(&ctx);
        PackContext(&sDeferredContexts[sDeferredCount++], &ctx);
        gWayfarerWorldDebug.deferredBegins++;
        return;
    }
    BeginHeartbeat();
}

// All at once (a debugging hook).
void WayfarerWorld_ForceHeartbeat(void)
{
    WayfarerWorld_FinishHeartbeat();
    sLastMap = CurrentMap();
    BeginHeartbeat();
    WayfarerWorld_FinishHeartbeat();
}

void WayfarerWorld_OnLeagueResolved(const u16 *lineup, u8 count, bool8 playerWon, u16 championId)
{
    // Life events change how the routine advances: never mid-heartbeat.
    WayfarerWorld_FinishHeartbeatEarly();
    WorldSim_OnLeagueResolved(WayfarerWorld_GetState(), lineup, count, playerWon, championId);
}

#endif // IS_WAYFARER
