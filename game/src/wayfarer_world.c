// Engine side of the notable world simulation: builds the context the core
// reads from saved league state and badges, runs the map-change heartbeat,
// seeds New Game, and validates the world state on load.
// Spec: .product/specs/notable-world-simulation.md.

#include "global.h"
#include "wayfarer_world.h"

#if IS_WAYFARER
#include "league_events.h"
#include "league_selection.h"
#include "malloc.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "trainer_rating.h"
#include "wayfarer_walkers.h"
#include "wayfarer_persistence.h"
#include "constants/regions.h"

EWRAM_DATA struct WayfarerWorldDebug gWayfarerWorldDebug = {0};
EWRAM_DATA u32 gWayfarerWorldFrozenMask = 0;
static EWRAM_DATA bool8 sSkipNextHeartbeat = FALSE;
static EWRAM_DATA u16 sLastMap = 0;

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

void WayfarerWorld_InitNewGame(void)
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());

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
    valid = WorldSim_IsValid(state, workspace)
         && header != NULL && header->mapLayout != NULL
         && WorldSim_LocalActorsValid(state, CurrentMap(), header->mapLayout->width, header->mapLayout->height);
    if (valid)
        WorldSim_ApplyDerivedOnLoad(state, &ctx);
    if (workspace != NULL)
        Free(workspace);
    return valid;
}

static void RunHeartbeat(void)
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());

    if (workspace == NULL)
    {
        gWayfarerWorldDebug.skippedHeartbeats++;
        return;
    }
    WayfarerWorld_BuildContext(&ctx);
    gWayfarerWorldDebug.lastTrace = (struct WayfarerWorldTrace){0};
    WorldSim_Heartbeat(WayfarerWorld_GetState(), &ctx, workspace, &gWayfarerWorldDebug.lastTrace);
    Free(workspace);
    gWayfarerWorldDebug.heartbeats++;
    gWayfarerWorldDebug.lastHeartbeatMap = ctx.playerMap;
}

void WayfarerWorld_OnMapLoad(void)
{
    u16 map = CurrentMap();

    if (sSkipNextHeartbeat || map == sLastMap)
    {
        // Continue, New Game's first warp, or a script reloading the same map.
        sSkipNextHeartbeat = FALSE;
        sLastMap = map;
        gWayfarerWorldDebug.skippedHeartbeats++;
        return;
    }
    sLastMap = map;
    RunHeartbeat();
}

void WayfarerWorld_ForceHeartbeat(void)
{
    sLastMap = CurrentMap();
    RunHeartbeat();
}

void WayfarerWorld_OnLeagueResolved(const u16 *lineup, u8 count, bool8 playerWon, u16 championId)
{
    WorldSim_OnLeagueResolved(WayfarerWorld_GetState(), lineup, count, playerWon, championId);
}

#endif // IS_WAYFARER
