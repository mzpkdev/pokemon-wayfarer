#include "global.h"
#include "malloc.h"
#include "random.h"
#include "test/test.h"
#include "wayfarer_world_sim.h"
#include "constants/map_groups.h"
#include "constants/notable_trainers.h"

#if IS_WAYFARER

#define HEARTBEATS 200

static const u16 sSimulated[WORLD_SIM_TRAINER_COUNT] =
{
    NOTABLE_TRAINER_BROCK, NOTABLE_TRAINER_MISTY, NOTABLE_TRAINER_LT_SURGE, NOTABLE_TRAINER_ERIKA,
    NOTABLE_TRAINER_JANINE, NOTABLE_TRAINER_SABRINA, NOTABLE_TRAINER_BLAINE, NOTABLE_TRAINER_GIOVANNI,
    NOTABLE_TRAINER_BLUE, NOTABLE_TRAINER_LORELEI, NOTABLE_TRAINER_LANCE, NOTABLE_TRAINER_FALKNER, NOTABLE_TRAINER_BUGSY, NOTABLE_TRAINER_WHITNEY,
    NOTABLE_TRAINER_MORTY, NOTABLE_TRAINER_CHUCK, NOTABLE_TRAINER_JASMINE, NOTABLE_TRAINER_PRYCE,
    NOTABLE_TRAINER_CLAIR, NOTABLE_TRAINER_WILL, NOTABLE_TRAINER_KAREN, NOTABLE_TRAINER_NORMAN,
    NOTABLE_TRAINER_JUAN, NOTABLE_TRAINER_WALLACE, NOTABLE_TRAINER_STEVEN,
};

static EWRAM_DATA struct WayfarerWorldState sState = {0};
static EWRAM_DATA struct WayfarerWorldState sOther = {0};

static void Context(struct WayfarerWorldContext *ctx, bool8 allBadges, u16 playerMap)
{
    u8 slot;
    *ctx = (struct WayfarerWorldContext){0};
    ctx->playerMap = playerMap;
    ctx->worldProgress = 40;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        ctx->derived[slot] = WORLD_DERIVED_NONE;
        if (!allBadges && (gWayfarerWorldTrainers[slot].flags & WORLD_TRAINER_FLAG_GYM_LEADER))
            ctx->derived[slot] = WORLD_STATE_HOME_LOCKED;
    }
}

static bool8 IsEdgeTarget(u16 from, u16 to)
{
    const struct WayfarerWorldNode *node = &gWayfarerWorldNodes[from];
    u16 e;
    for (e = node->firstEdge; e < node->firstEdge + node->edgeCount; e++)
    {
        if (gWayfarerWorldEdges[e].target == to)
            return TRUE;
    }
    return FALSE;
}

// No map holds more notables than its cap.
static bool8 CapsHold(const struct WayfarerWorldState *state)
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        u16 nodes[2], i;
        nodes[0] = state->records[slot].node;
        nodes[1] = WorldSim_DestNode(state, slot);
        for (i = 0; i < 2; i++)
        {
            if (nodes[i] == WORLD_NODE_NONE)
                continue;
            if (WorldSim_Occupancy(state, WorldSim_NodeMap(nodes[i]), 0xFF) > WorldSim_MapCapacity(nodes[i]))
                return FALSE;
        }
    }
    return TRUE;
}

TEST("World tables hold the walking notables in catalog order")
{
    u8 slot;
    EXPECT(gWayfarerWorldNodeCount > 0 && gWayfarerWorldNodeCount <= WORLD_MAX_NODES);
    EXPECT(gWayfarerWorldSpotCount > 0 && gWayfarerWorldSpotCount <= WORLD_MAX_SPOTS);
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
        EXPECT_EQ(trainer->characterId, sSimulated[slot]);
        EXPECT(trainer->homeNode < gWayfarerWorldNodeCount);
        EXPECT_EQ(WorldSim_NodeMap(trainer->homeNode), trainer->homeMap);
        EXPECT(trainer->cycleLength >= 3 && trainer->cycleLength <= WORLD_CYCLE_MAX_STEPS);
        EXPECT(trainer->searchBound >= 2 * trainer->radius);
        if (trainer->flags & WORLD_TRAINER_FLAG_GYM_LEADER)
            EXPECT(trainer->gymNode < gWayfarerWorldNodeCount);
    }
    EXPECT_EQ(WorldSim_SlotForCharacter(NOTABLE_TRAINER_AGATHA), 0xFF);
    EXPECT_EQ(WorldSim_SlotForCharacter(NOTABLE_TRAINER_TATE_LIZA), 0xFF);
    EXPECT_EQ(WorldSim_SlotForCharacter(NOTABLE_TRAINER_BRUNO), 0xFF);  // face-only sprite
    EXPECT_EQ(WorldSim_SlotForCharacter(NOTABLE_TRAINER_KOGA), 0xFF);
    EXPECT_EQ(WorldSim_SlotForCharacter(NOTABLE_TRAINER_STEVEN), WORLD_SIM_TRAINER_COUNT - 1);
}

TEST("World record and state keep the saved layout")
{
    EXPECT_EQ(sizeof(struct WayfarerWorldRecord), 8);
    EXPECT_EQ(sizeof(struct WayfarerWorldState), WORLD_STATE_SIZE);
}

TEST("Viridian's north lane converts to Route 2 by the connection offset")
{
    u16 n, e;
    bool8 found = FALSE;
    for (n = 0; n < gWayfarerWorldNodeCount; n++)
    {
        const struct WayfarerWorldNode *node = &gWayfarerWorldNodes[n];
        if (node->map != MAP_VIRIDIAN_CITY_HNS)
            continue;
        for (e = node->firstEdge; e < node->firstEdge + node->edgeCount; e++)
        {
            const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[e];
            if (edge->kind != WORLD_EDGE_NORTH || WorldSim_NodeMap(edge->target) != MAP_ROUTE2_HNS)
                continue;
            if (edge->a <= 25 && edge->b >= 25)
            {
                EXPECT_EQ(edge->c + (25 - edge->a), 9);
                found = TRUE;
            }
        }
    }
    EXPECT(found);
}

TEST("New Game seats leaders in their Gyms and everyone else at their first step's spot")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    rng_value_t rng = gRngValue;
    u8 slot;

    Context(&ctx, FALSE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    EXPECT_EQ(sState.schemaVersion, WORLD_SCHEMA_VERSION);
    EXPECT_EQ(sState.contentHash, gWayfarerWorldContentHash);
    EXPECT(WorldSim_IsValid(&sState, workspace));
    EXPECT(CapsHold(&sState));
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldRecord *record = &sState.records[slot];
        const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
        if (trainer->flags & WORLD_TRAINER_FLAG_GYM_LEADER)
        {
            EXPECT_EQ((u32)record->state, WORLD_STATE_HOME_LOCKED);
            EXPECT_EQ((u32)record->node, trainer->gymNode);
        }
        else
        {
            EXPECT_EQ((u32)record->state, WORLD_STATE_DWELLING);
            if (record->destKind == WORLD_DEST_NONE)
            {
                // Nothing fit (a shared home place taken): stays home for one heartbeat.
                EXPECT_EQ((u32)record->node, trainer->homeNode);
                EXPECT_EQ((u32)record->dwell, 1);
            }
            else
            {
                EXPECT_EQ((u32)record->node, WorldSim_DestNode(&sState, slot));
                EXPECT_EQ((u32)record->dwell, WorldSim_DwellFor(record->activity));
            }
        }
    }
    for (slot = 0; slot < WORLD_LOCAL_ACTOR_COUNT; slot++)
        EXPECT_EQ(sState.localActors[slot * WORLD_LOCAL_ACTOR_BYTES], WORLD_LOCAL_ACTOR_NONE);
    EXPECT(memcmp(&gRngValue, &rng, sizeof(rng)) == 0);
    Free(workspace);
}

TEST("Heartbeats move trainers one hop at most, never on the player's map, within caps")
{
    struct WayfarerWorldContext ctx;
    struct WayfarerWorldTrace trace = {0};
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    rng_value_t rng = gRngValue;
    u16 heartbeat, playerMap;
    u8 slot;

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    // Sit the player on Misty's map: she must never move while they stay.
    playerMap = WorldSim_NodeMap(sState.records[WorldSim_SlotForCharacter(NOTABLE_TRAINER_MISTY)].node);
    for (heartbeat = 0; heartbeat < HEARTBEATS; heartbeat++)
    {
        u16 before[WORLD_SIM_TRAINER_COUNT];
        ctx.playerMap = heartbeat < 20 ? playerMap : MAP_UNDEFINED;
        for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
            before[slot] = sState.records[slot].node;
        WorldSim_Heartbeat(&sState, &ctx, workspace, &trace);
        for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
        {
            u16 node = sState.records[slot].node;
            if (WorldSim_NodeMap(before[slot]) == ctx.playerMap)
                EXPECT_EQ(node, before[slot]);
            else if (node != before[slot])
                EXPECT(IsEdgeTarget(before[slot], node));
        }
        EXPECT(CapsHold(&sState));
        EXPECT(WorldSim_IsValid(&sState, workspace));
    }
    EXPECT(trace.hops > 0);
    EXPECT(trace.advances > 0);
    EXPECT(memcmp(&gRngValue, &rng, sizeof(rng)) == 0);
    Free(workspace);
}

TEST("The same map loads give identical world records")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u16 heartbeat;

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    WorldSim_NewGame(&sOther, &ctx, workspace);
    for (heartbeat = 0; heartbeat < HEARTBEATS; heartbeat++)
    {
        ctx.playerMap = (heartbeat % 3) ? MAP_VIRIDIAN_CITY_HNS : MAP_PALLET_TOWN_HNS;
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        WorldSim_Heartbeat(&sOther, &ctx, workspace, NULL);
    }
    EXPECT(memcmp(&sState, &sOther, sizeof(sState)) == 0);
    Free(workspace);
}

TEST("Aloof trainers never take a public spot")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u16 heartbeat;
    u8 slot;

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    for (heartbeat = 0; heartbeat < HEARTBEATS; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
        {
            const struct WayfarerWorldRecord *record = &sState.records[slot];
            if (!(gWayfarerWorldTrainers[slot].flags & WORLD_TRAINER_FLAG_ALOOF) || record->destKind != WORLD_DEST_SPOT)
                continue;
            EXPECT_EQ(gWayfarerWorldSpots[record->destId].flags & WORLD_SPOT_FLAG_PUBLIC, 0);
        }
    }
    Free(workspace);
}

TEST("Gym Leaders stay home-locked in their Gym until the player holds their badge")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u8 slot = WorldSim_SlotForCharacter(NOTABLE_TRAINER_BROCK);
    u16 heartbeat;

    Context(&ctx, FALSE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    for (heartbeat = 0; heartbeat < 50; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        EXPECT_EQ((u32)sState.records[slot].state, WORLD_STATE_HOME_LOCKED);
        EXPECT_EQ((u32)sState.records[slot].node, gWayfarerWorldTrainers[slot].gymNode);
    }
    // Winning the badge: dwelling at home in the Gym, then the routine runs.
    ctx.derived[slot] = WORLD_DERIVED_NONE;
    WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
    EXPECT_EQ((u32)sState.records[slot].destKind, WORLD_DEST_HOME);
    EXPECT_EQ((u32)sState.records[slot].node, gWayfarerWorldTrainers[slot].gymNode);
    for (heartbeat = 0; heartbeat < 10 && sState.records[slot].destKind == WORLD_DEST_HOME; heartbeat++)
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
    EXPECT_EQ((u32)sState.records[slot].destKind, WORLD_DEST_SPOT);
    Free(workspace);
}

TEST("League resolution sets celebrating, brooding and recovering for two steps")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    static const u16 lineup[] = {NOTABLE_TRAINER_WILL, NOTABLE_TRAINER_LANCE, NOTABLE_TRAINER_KAREN,
                                 NOTABLE_TRAINER_WALLACE, NOTABLE_TRAINER_STEVEN};
    u8 i, steven = WorldSim_SlotForCharacter(NOTABLE_TRAINER_STEVEN);
    u8 will = WorldSim_SlotForCharacter(NOTABLE_TRAINER_WILL);

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    // Accepted: the lineup is away at the league.
    for (i = 0; i < ARRAY_COUNT(lineup); i++)
        ctx.derived[WorldSim_SlotForCharacter(lineup[i])] = WORLD_STATE_AWAY_LEAGUE;
    WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
    EXPECT_EQ((u32)sState.records[steven].state, WORLD_STATE_AWAY_LEAGUE);

    // The player won: the final's loser broods, the others recover.
    WorldSim_OnLeagueResolved(&sState, lineup, ARRAY_COUNT(lineup), TRUE, 0);
    EXPECT_EQ((u32)sState.records[steven].lifeEvent, WORLD_LIFE_BROODING);
    EXPECT_EQ((u32)sState.records[steven].lifeSteps, WORLD_LIFE_EVENT_STEPS);
    EXPECT_EQ((u32)sState.records[will].lifeEvent, WORLD_LIFE_RECOVERING);
    for (i = 0; i < ARRAY_COUNT(lineup); i++)
        ctx.derived[WorldSim_SlotForCharacter(lineup[i])] = WORLD_DERIVED_NONE;
    WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
    // Back at home, the first override step is taken (or skipped) at once.
    EXPECT(WorldSim_IsSimulated(&sState.records[will]));
    EXPECT_EQ((u32)sState.records[will].lifeSteps, 1);
    EXPECT_EQ((u32)sState.records[will].activity, WORLD_ACTIVITY_HOME);

    // The player lost: the champion celebrates.
    WorldSim_OnLeagueResolved(&sState, lineup, ARRAY_COUNT(lineup), FALSE, NOTABLE_TRAINER_STEVEN);
    EXPECT_EQ((u32)sState.records[steven].lifeEvent, WORLD_LIFE_CELEBRATING);
    EXPECT(WorldSim_IsValid(&sState, workspace));
    Free(workspace);
}

TEST("A waiting invitation's provisional lineup trains on every non-home step")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u8 slot = WorldSim_SlotForCharacter(NOTABLE_TRAINER_BLUE);
    u16 heartbeat;

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    ctx.provisionalMask = 1u << slot;
    for (heartbeat = 0; heartbeat < 60; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        if (sState.records[slot].destKind == WORLD_DEST_SPOT)
            EXPECT_EQ((u32)sState.records[slot].activity, WORLD_ACTIVITY_TRAIN);
    }
    Free(workspace);
}

TEST("A blocked traveller waits one heartbeat, then reroutes or waits again")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u16 heartbeat, edge = 0xFFFF, target, map, startNode;
    u8 slot, mover = 0xFF, filler, placed = 0;

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    // Run until someone is travelling with a next hop onto a map other than
    // their destination's.
    for (heartbeat = 0; heartbeat < HEARTBEATS && mover == 0xFF; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT && mover == 0xFF; slot++)
        {
            if (sState.records[slot].state != WORLD_STATE_TRAVELLING)
                continue;
            edge = WorldSim_NextEdge(&sState, slot, workspace, NULL);
            if (edge != 0xFFFF && WorldSim_NodeMap(gWayfarerWorldEdges[edge].target)
                               != WorldSim_NodeMap(WorldSim_DestNode(&sState, slot)))
                mover = slot;
        }
    }
    ASSUME(mover != 0xFF);
    target = gWayfarerWorldEdges[edge].target;
    map = WorldSim_NodeMap(target);
    // Fill the next map to its cap with other trainers parked there.
    for (filler = 0; filler < WORLD_SIM_TRAINER_COUNT && WorldSim_Occupancy(&sState, map, mover) < WorldSim_MapCapacity(target); filler++)
    {
        struct WayfarerWorldRecord *record = &sState.records[filler];
        if (filler == mover)
            continue;
        record->node = target;
        record->state = WORLD_STATE_DWELLING;
        record->destKind = WORLD_DEST_NONE;
        record->destId = 0;
        record->arrival = WORLD_ARRIVAL_NONE;
        record->dwell = 60;
        ctx.derived[filler] = WORLD_DERIVED_NONE;
        placed++;
    }
    // Keep everyone else still so only the mover acts.
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (slot != mover && sState.records[slot].node != target)
            ctx.frozenMask |= 1u << slot;
    }
    ctx.playerMap = map;  // and the fillers stay put on it
    startNode = sState.records[mover].node;
    WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
    EXPECT_EQ((u32)sState.records[mover].node, startNode);
    EXPECT_EQ((u32)sState.records[mover].waited, TRUE);
    WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
    // Rerouted around the full map, or still waiting in place.
    EXPECT(sState.records[mover].node == startNode || WorldSim_NodeMap(sState.records[mover].node) != map);
    EXPECT(placed > 0);
    Free(workspace);
}

TEST("Load checks reject bad records and a content change re-seats them")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u8 slot = WorldSim_SlotForCharacter(NOTABLE_TRAINER_BLUE);

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    EXPECT(WorldSim_IsValid(&sState, workspace));

    sOther = sState;
    sOther.records[slot].destKind = WORLD_DEST_RESERVED_HAUNT;
    EXPECT(!WorldSim_IsValid(&sOther, workspace));
    sOther = sState;
    sOther.records[slot].reserved = 1;
    EXPECT(!WorldSim_IsValid(&sOther, workspace));
    sOther = sState;
    sOther.records[slot].lifeEvent = WORLD_LIFE_NONE;
    sOther.records[slot].lifeSteps = 1;
    EXPECT(!WorldSim_IsValid(&sOther, workspace));
    sOther = sState;
    sOther.records[slot].arrival = WORLD_ARRIVAL_DOOR;  // never while dwelling
    EXPECT(!WorldSim_IsValid(&sOther, workspace));
    sOther = sState;
    sOther.records[slot].node = WORLD_NODE_NONE;
    EXPECT(!WorldSim_IsValid(&sOther, workspace));

    // Local actor block: one entry per trainer, on the saved map.
    sOther = sState;
    WorldSim_SetLocalActor(&sOther, 0, slot, 3, 4, DIR_EAST);
    EXPECT(WorldSim_LocalActorsValid(&sOther, WorldSim_NodeMap(sOther.records[slot].node), 40, 40));
    EXPECT(!WorldSim_LocalActorsValid(&sOther, WorldSim_NodeMap(sOther.records[slot].node), 3, 40));
    WorldSim_SetLocalActor(&sOther, 1, slot, 5, 5, DIR_NORTH);
    EXPECT(!WorldSim_LocalActorsValid(&sOther, WorldSim_NodeMap(sOther.records[slot].node), 40, 40));
    {
        u8 s, x, y, facing;
        EXPECT(WorldSim_GetLocalActor(&sOther, 0, &s, &x, &y, &facing));
        EXPECT_EQ(s, slot);
        EXPECT_EQ(x, 3);
        EXPECT_EQ(y, 4);
        EXPECT_EQ(facing, DIR_EAST);
    }

    // A content change keeps each trainer's cycle step and life event.
    sOther = sState;
    sOther.contentHash ^= 0xFFFF;
    sOther.records[slot].step = 1;
    sOther.records[slot].lifeEvent = WORLD_LIFE_RECOVERING;
    sOther.records[slot].lifeSteps = 2;
    WorldSim_Reseat(&sOther, &ctx, workspace);
    EXPECT_EQ(sOther.contentHash, gWayfarerWorldContentHash);
    EXPECT_EQ((u32)sOther.records[slot].lifeEvent, WORLD_LIFE_RECOVERING);
    EXPECT_EQ((u32)sOther.records[slot].lifeSteps, 2);
    EXPECT(sOther.records[slot].step >= 1);
    EXPECT(WorldSim_IsValid(&sOther, workspace));
    Free(workspace);
}

TEST("A Gym Leader stays home while a visitor is in their Gym")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u8 leader = WorldSim_SlotForCharacter(NOTABLE_TRAINER_GIOVANNI);
    u8 visitor = WorldSim_SlotForCharacter(NOTABLE_TRAINER_BLUE);
    const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[leader];
    u16 gymMap = WorldSim_NodeMap(trainer->gymNode), heartbeat;

    ASSUME(trainer->ownGymSpot != WORLD_SPOT_NONE);
    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    // The leader dwells at home with his dwell about to run out, and a
    // visitor stands at the Gym's visitor spot.
    sState.records[leader].node = trainer->gymNode;
    sState.records[leader].state = WORLD_STATE_DWELLING;
    sState.records[leader].destKind = WORLD_DEST_HOME;
    sState.records[leader].destId = WORLD_SPOT_NONE;
    sState.records[leader].activity = WORLD_ACTIVITY_HOME;
    sState.records[leader].dwell = 1;
    sState.records[visitor].node = gWayfarerWorldSpots[trainer->ownGymSpot].node;
    sState.records[visitor].state = WORLD_STATE_DWELLING;
    sState.records[visitor].destKind = WORLD_DEST_SPOT;
    sState.records[visitor].destId = trainer->ownGymSpot;
    sState.records[visitor].arrival = WORLD_ARRIVAL_NONE;
    sState.records[visitor].activity = WORLD_ACTIVITY_VISIT;
    sState.records[visitor].dwell = 20;
    // Make him want to go out: a provisional lineup turns his steps into train.
    ctx.provisionalMask = 1u << leader;
    for (heartbeat = 0; heartbeat < 6; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        EXPECT_LE(WorldSim_Occupancy(&sState, gymMap, 0xFF), 1);
        EXPECT_EQ((u32)sState.records[leader].destKind, WORLD_DEST_HOME);
        EXPECT_EQ((u32)sState.records[leader].node, trainer->gymNode);
    }
    // Once the visitor moves on, caps still hold whatever the leader does.
    for (heartbeat = 0; heartbeat < 40; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        EXPECT_LE(WorldSim_Occupancy(&sState, gymMap, 0xFF), 1);
        EXPECT(CapsHold(&sState));
    }
    Free(workspace);
}

#endif // IS_WAYFARER
