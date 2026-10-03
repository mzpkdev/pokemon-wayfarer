#include "global.h"
#include "malloc.h"
#include "random.h"
#include "test/test.h"
#include "wayfarer_world_sim.h"
#include "constants/map_groups.h"
#include "constants/notable_trainers.h"

#if IS_WAYFARER

#define HEARTBEATS 200
#define DETOUR_HEARTBEAT 100

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

// The context for heartbeat n of the sliced-versus-whole comparison: the
// player and the frozen mask move around so both exclusions get exercised.
// Sets up a blocked trip with a detour: the mover travels from a node with
// two edges into different maps, both one hop from a common goal node, the
// first (the search's choice) into an interior (cap 1) map that a filler
// then holds. Blocking it leaves the detour. FALSE if no such place exists.
static bool8 SetUpDetour(struct WayfarerWorldState *state, void *workspace, u8 mover, u8 filler,
                         u16 *startNode, u16 *firstEdge)
{
    u16 node, e;

    for (node = 0; node < gWayfarerWorldNodeCount; node++)
    {
        const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[node];
        u16 a, b;
        for (a = data->firstEdge; a < data->firstEdge + data->edgeCount; a++)
        {
            u16 ta = gWayfarerWorldEdges[a].target;
            if (gWayfarerWorldEdges[a].kind == WORLD_EDGE_TRANSIT || !(gWayfarerWorldNodes[ta].flags & WORLD_NODE_FLAG_INTERIOR)
             || WorldSim_Occupancy(state, WorldSim_NodeMap(ta), mover) != 0)
                continue;
            for (b = data->firstEdge; b < data->firstEdge + data->edgeCount; b++)
            {
                u16 tb = gWayfarerWorldEdges[b].target, ea;
                if (b == a || gWayfarerWorldEdges[b].kind == WORLD_EDGE_TRANSIT
                 || WorldSim_NodeMap(tb) == WorldSim_NodeMap(ta) || WorldSim_NodeMap(tb) == WorldSim_NodeMap(node)
                 || WorldSim_Occupancy(state, WorldSim_NodeMap(tb), mover) >= WorldSim_MapCapacity(tb))
                    continue;
                for (ea = gWayfarerWorldNodes[ta].firstEdge; ea < gWayfarerWorldNodes[ta].firstEdge + gWayfarerWorldNodes[ta].edgeCount; ea++)
                {
                    u16 goal = gWayfarerWorldEdges[ea].target;
                    if (WorldSim_NodeMap(goal) == WorldSim_NodeMap(ta) || WorldSim_NodeMap(goal) == WorldSim_NodeMap(node)
                     || WorldSim_NodeMap(goal) == WorldSim_NodeMap(tb) || !IsEdgeTarget(tb, goal))
                        continue;
                    for (e = 0; e < gWayfarerWorldSpotCount; e++)
                    {
                        if (gWayfarerWorldSpots[e].node == goal)
                            break;
                    }
                    if (e == gWayfarerWorldSpotCount)
                        continue;
                    state->records[mover].node = node;
                    state->records[mover].state = WORLD_STATE_TRAVELLING;
                    state->records[mover].destKind = WORLD_DEST_SPOT;
                    state->records[mover].destId = e;
                    state->records[mover].arrival = WORLD_ARRIVAL_NONE;
                    state->records[mover].waited = FALSE;
                    WorldSim_ResetPathCache();
                    if (WorldSim_NextEdge(state, mover, workspace, NULL) != a)
                        continue;
                    state->records[filler].node = ta;
                    state->records[filler].state = WORLD_STATE_DWELLING;
                    state->records[filler].destKind = WORLD_DEST_NONE;
                    state->records[filler].destId = 0;
                    state->records[filler].arrival = WORLD_ARRIVAL_NONE;
                    state->records[filler].dwell = 60;
                    *startNode = node;
                    *firstEdge = a;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

static void SlicedContext(struct WayfarerWorldContext *ctx, u16 heartbeat)
{
    static const u16 sMaps[] = {MAP_VIRIDIAN_CITY_HNS, MAP_PALLET_TOWN_HNS, MAP_UNDEFINED, MAP_ROUTE2_HNS};
    Context(ctx, TRUE, sMaps[heartbeat % ARRAY_COUNT(sMaps)]);
    ctx->worldProgress = 40 + heartbeat / 16;
    ctx->frozenMask = (heartbeat % 5 == 0) ? (0x00421084u << (heartbeat % 3)) : 0;
    // A league lineup goes away and comes back every 24 heartbeats, so
    // returns (and their spot choices, in the sliced phase) are covered.
    if (heartbeat % 24 >= 16)
    {
        ctx->derived[WorldSim_SlotForCharacter(NOTABLE_TRAINER_WILL)] = WORLD_STATE_AWAY_LEAGUE;
        ctx->derived[WorldSim_SlotForCharacter(NOTABLE_TRAINER_KAREN)] = WORLD_STATE_AWAY_LEAGUE;
        ctx->derived[WorldSim_SlotForCharacter(NOTABLE_TRAINER_LANCE)] = WORLD_STATE_AWAY_LEAGUE;
    }
}

static u32 BeatChecksum(const struct WayfarerWorldState *state, const struct WayfarerWorldTrace *trace)
{
    const u8 *bytes = (const u8 *)state;
    u32 i, sum = 2166136261u;
    for (i = 0; i < sizeof(*state); i++)
        sum = (sum ^ bytes[i]) * 16777619u;
    sum = (sum ^ trace->searchNodes) * 16777619u;
    sum = (sum ^ trace->searchNodesMax) * 16777619u;
    sum = (sum ^ trace->searches) * 16777619u;
    sum = (sum ^ trace->hops) * 16777619u;
    sum = (sum ^ trace->waits) * 16777619u;
    sum = (sum ^ trace->reroutes) * 16777619u;
    sum = (sum ^ trace->advances) * 16777619u;
    sum = (sum ^ trace->skips) * 16777619u;
    return sum;
}

TEST("A heartbeat spread over tiny steps, with lost workspaces, matches one run at once")
{
    struct WayfarerWorldContext ctx;
    struct WayfarerWorldTrace whole = {0}, sliced = {0};
    struct WayfarerWorldHeartbeat hb;
    u32 size = WorldSim_WorkspaceSize();
    void *workspaces[2] = {Alloc(size), Alloc(size)};
    u32 *sums = Alloc(HEARTBEATS * sizeof(u32));
    u32 tick = 0, steps = 0, losses = 0;
    u16 heartbeat, detourNode, detourEdge;
    u8 current = 0;
    bool8 detoured = FALSE;

    // Whole heartbeats first (New Game resets the shared path cache, so the
    // second run starts from the same cache as the first).
    SlicedContext(&ctx, 0);
    WorldSim_NewGame(&sState, &ctx, workspaces[0]);
    for (heartbeat = 0; heartbeat < HEARTBEATS; heartbeat++)
    {
        struct WayfarerWorldTrace beat = {0};
        SlicedContext(&ctx, heartbeat);
        if (heartbeat == DETOUR_HEARTBEAT)
            detoured = SetUpDetour(&sState, workspaces[0], WorldSim_SlotForCharacter(NOTABLE_TRAINER_STEVEN), 0, &detourNode, &detourEdge);
        WorldSim_Heartbeat(&sState, &ctx, workspaces[0], &beat);
        sums[heartbeat] = BeatChecksum(&sState, &beat);
        whole.searchNodes += beat.searchNodes;
        whole.searches += beat.searches;
        whole.hops += beat.hops;
        whole.waits += beat.waits;
        whole.reroutes += beat.reroutes;
        whole.advances += beat.advances;
        whole.skips += beat.skips;
    }

    SlicedContext(&ctx, 0);
    WorldSim_NewGame(&sOther, &ctx, workspaces[0]);
    for (heartbeat = 0; heartbeat < HEARTBEATS; heartbeat++)
    {
        struct WayfarerWorldTrace beat = {0};
        SlicedContext(&ctx, heartbeat);
        bool8 lost = FALSE;
        if (heartbeat == DETOUR_HEARTBEAT)
            SetUpDetour(&sOther, workspaces[current], WorldSim_SlotForCharacter(NOTABLE_TRAINER_STEVEN), 0, &detourNode, &detourEdge);
        WorldSim_HeartbeatBegin(&hb, &sOther, &ctx, &beat);
        for (;;)
        {
            tick++;
            // Once per heartbeat the heap resets: a different block full of
            // garbage replaces the workspace, on even heartbeats in the
            // middle of a search, on odd ones between two searches.
            if (!lost && (heartbeat % 2 == 0 ? hb.searchLive : (hb.clean && !hb.searchLive)))
            {
                current ^= 1;
                memset(workspaces[current], 0xA5, size);
                WorldSim_HeartbeatLostWorkspace(&hb);
                lost = TRUE;
                losses++;
            }
            steps++;
            if (WorldSim_HeartbeatStep(&hb, &sOther, workspaces[current], 1 + tick % 3, &beat))
                break;
        }
        EXPECT_EQ(sums[heartbeat], BeatChecksum(&sOther, &beat));
        sliced.searchNodes += beat.searchNodes;
        sliced.searches += beat.searches;
        sliced.hops += beat.hops;
        sliced.waits += beat.waits;
        sliced.reroutes += beat.reroutes;
        sliced.advances += beat.advances;
        sliced.skips += beat.skips;
    }
    EXPECT(memcmp(&sState, &sOther, sizeof(sState)) == 0);
    EXPECT_EQ(whole.searchNodes, sliced.searchNodes);
    EXPECT_EQ(whole.searches, sliced.searches);
    EXPECT_EQ(whole.hops, sliced.hops);
    EXPECT_EQ(whole.waits, sliced.waits);
    EXPECT_EQ(whole.reroutes, sliced.reroutes);
    EXPECT_EQ(whole.advances, sliced.advances);
    EXPECT_EQ(whole.skips, sliced.skips);
    EXPECT(whole.hops > 0 && whole.searches > 0);
    // The blocked-traveller paths ran and still matched (a blocked trip
    // with a detour is set up at one heartbeat, the same in both runs).
    EXPECT(detoured);
    EXPECT(whole.waits > 0);
    EXPECT(whole.reroutes > 0);
    EXPECT(steps > 4 * HEARTBEATS);   // really spread out
    EXPECT(losses > HEARTBEATS / 4);
    Free(sums);
    Free(workspaces[1]);
    Free(workspaces[0]);
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
    {
        struct WayfarerWorldTrace first = {0}, second = {0};
        WorldSim_Heartbeat(&sState, &ctx, workspace, &first);
        EXPECT_EQ((u32)sState.records[mover].node, startNode);
        EXPECT_EQ((u32)sState.records[mover].waited, TRUE);
        EXPECT_EQ(first.waits, 1);
        EXPECT_EQ(first.hops, 0);
        WorldSim_Heartbeat(&sState, &ctx, workspace, &second);
        // Never into the full map: rerouted around it, or waiting again.
        EXPECT(sState.records[mover].node == startNode || WorldSim_NodeMap(sState.records[mover].node) != map);
        EXPECT_EQ(second.hops + second.waits, 1);
        EXPECT_EQ(second.reroutes, second.hops);
    }
    EXPECT(placed > 0);
    Free(workspace);
}

// A trainer blocked by a full map takes the detour when one exists.
TEST("A blocked traveller takes a detour around the full map")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u16 node = 0xFFFF, firstEdge = 0xFFFF;
    u8 mover = WorldSim_SlotForCharacter(NOTABLE_TRAINER_STEVEN), filler = 0, slot;
    bool8 found;
    struct WayfarerWorldTrace first = {0}, second = {0};

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    found = SetUpDetour(&sState, workspace, mover, filler, &node, &firstEdge);
    ASSUME(found);
    // Only the mover acts.
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (slot != mover)
            ctx.frozenMask |= 1u << slot;
    }
    EXPECT(!WorldSim_HopAllowed(&sState, mover, firstEdge));
    WorldSim_Heartbeat(&sState, &ctx, workspace, &first);
    EXPECT_EQ(first.waits, 1);
    EXPECT_EQ((u32)sState.records[mover].node, node);
    WorldSim_Heartbeat(&sState, &ctx, workspace, &second);
    EXPECT_EQ(second.reroutes, 1);
    EXPECT_EQ(second.hops, 1);
    EXPECT(WorldSim_NodeMap(sState.records[mover].node) != WorldSim_NodeMap(gWayfarerWorldEdges[firstEdge].target));
    EXPECT_NE((u32)sState.records[mover].node, node);
    Free(workspace);
}

TEST("Cached travel paths give the same next hop as a new search")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u16 heartbeat, cached, fresh, checked = 0;
    u8 slot;

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    for (heartbeat = 0; heartbeat < HEARTBEATS; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
        {
            if (sState.records[slot].state != WORLD_STATE_TRAVELLING)
                continue;
            cached = WorldSim_NextEdge(&sState, slot, workspace, NULL);
            WorldSim_ResetPathCache();
            fresh = WorldSim_NextEdge(&sState, slot, workspace, NULL);
            EXPECT_EQ(cached, fresh);
            checked++;
        }
    }
    EXPECT(checked > 0);
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
    // The same trainer twice can only be a corrupt save; an entry off the
    // saved map (the location was repaired to another map) or outside its
    // size is a stale hint to clear.
    EXPECT_EQ(WorldSim_CheckLocalActors(&sOther, WorldSim_NodeMap(sOther.records[slot].node), 40, 40),
              WORLD_LOCAL_ACTORS_CORRUPT);
    sOther.localActors[1 * WORLD_LOCAL_ACTOR_BYTES] = WORLD_LOCAL_ACTOR_NONE;
    sOther.localActors[1 * WORLD_LOCAL_ACTOR_BYTES + 1] = 0;
    sOther.localActors[1 * WORLD_LOCAL_ACTOR_BYTES + 2] = 0;
    EXPECT_EQ(WorldSim_CheckLocalActors(&sOther, WorldSim_NodeMap(sOther.records[slot].node), 40, 40),
              WORLD_LOCAL_ACTORS_OK);
    EXPECT_EQ(WorldSim_CheckLocalActors(&sOther, WorldSim_NodeMap(sOther.records[slot].node) ^ 1, 40, 40),
              WORLD_LOCAL_ACTORS_STALE);
    EXPECT_EQ(WorldSim_CheckLocalActors(&sOther, WorldSim_NodeMap(sOther.records[slot].node), 3, 40),
              WORLD_LOCAL_ACTORS_STALE);
    sOther.localActors[2 * WORLD_LOCAL_ACTOR_BYTES + 1] = 7;  // an empty entry with a stray byte
    EXPECT_EQ(WorldSim_CheckLocalActors(&sOther, WorldSim_NodeMap(sOther.records[slot].node), 40, 40),
              WORLD_LOCAL_ACTORS_CORRUPT);
    sOther.localActors[2 * WORLD_LOCAL_ACTOR_BYTES + 1] = 0;
    WorldSim_SetLocalActor(&sOther, 1, slot, 5, 5, DIR_NORTH);
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

// A local actor's exit commits a hop like an off-screen one (the critic's
// run 149: Will and Steven both walked out of Ecruteak into its cap-1 Route
// 38 gate): with one trainer in the gate, the next one may not follow, while
// a hop into the trainer's own destination map is always allowed.
TEST("A walker's exit into a full map is not allowed")
{
    struct WayfarerWorldContext ctx;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u8 will = WorldSim_SlotForCharacter(NOTABLE_TRAINER_WILL), steven = WorldSim_SlotForCharacter(NOTABLE_TRAINER_STEVEN);
    u16 node, src, e, gateEdge = 0xFFFF, spot;

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    // An outdoor node with a door into an interior (cap 1) map nobody is on.
    for (src = 0; src < gWayfarerWorldNodeCount && gateEdge == 0xFFFF; src++)
    {
        const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[src];
        if (data->flags & WORLD_NODE_FLAG_INTERIOR)
            continue;
        for (e = data->firstEdge; e < data->firstEdge + data->edgeCount && gateEdge == 0xFFFF; e++)
        {
            u16 target = gWayfarerWorldEdges[e].target;
            if ((gWayfarerWorldNodes[target].flags & WORLD_NODE_FLAG_INTERIOR) && gWayfarerWorldEdges[e].kind == WORLD_EDGE_WARP
             && WorldSim_Occupancy(&sState, WorldSim_NodeMap(target), 0xFF) == 0)
                gateEdge = e;
        }
    }
    ASSUME(gateEdge != 0xFFFF);
    node = gWayfarerWorldEdges[gateEdge].target;
    for (src = 0; src < gWayfarerWorldNodeCount; src++)
    {
        const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[src];
        if (gateEdge >= data->firstEdge && gateEdge < data->firstEdge + data->edgeCount)
            break;
    }
    // Both stand outside, travelling somewhere far (a spot on another map).
    for (spot = 0; spot < gWayfarerWorldSpotCount; spot++)
    {
        if (WorldSim_NodeMap(gWayfarerWorldSpots[spot].node) != WorldSim_NodeMap(node)
         && !(gWayfarerWorldNodes[gWayfarerWorldSpots[spot].node].flags & WORLD_NODE_FLAG_INTERIOR))
            break;
    }
    ASSUME(spot < gWayfarerWorldSpotCount);
    sState.records[will].state = sState.records[steven].state = WORLD_STATE_TRAVELLING;
    sState.records[will].destKind = sState.records[steven].destKind = WORLD_DEST_SPOT;
    sState.records[will].destId = sState.records[steven].destId = spot;
    sState.records[will].node = sState.records[steven].node = src;
    sState.records[will].arrival = sState.records[steven].arrival = WORLD_ARRIVAL_NONE;
    EXPECT(WorldSim_HopAllowed(&sState, will, gateEdge));
    WorldSim_TakeEdge(&sState, will, gateEdge, gWayfarerWorldEdges[gateEdge].c);
    EXPECT_EQ(WorldSim_Occupancy(&sState, WorldSim_NodeMap(node), 0xFF), 1);
    EXPECT(!WorldSim_HopAllowed(&sState, steven, gateEdge));
    // Into one's own destination's map room was reserved by spot choice.
    for (spot = 0; spot < gWayfarerWorldSpotCount; spot++)
    {
        if (WorldSim_NodeMap(gWayfarerWorldSpots[spot].node) == WorldSim_NodeMap(node))
            break;
    }
    if (spot < gWayfarerWorldSpotCount)
    {
        sState.records[steven].destId = spot;
        EXPECT(WorldSim_HopAllowed(&sState, steven, gateEdge));
    }
    EXPECT(!WorldSim_HopAllowed(&sState, steven, 0xFFFF));
    Free(workspace);
}

// The local actor plans a trip's first edge a slice per frame: the result is
// the whole search's, also when the workspace is lost (and scribbled over)
// halfway, and the cache answers without one.
TEST("A trip's first edge found in slices matches the whole search")
{
    struct WayfarerWorldContext ctx;
    u32 size = WorldSim_WorkspaceSize();
    void *workspaces[2] = {Alloc(size), Alloc(size)};
    u16 heartbeat, checked = 0, longest = 0;
    u8 slot;

    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspaces[0]);
    for (heartbeat = 0; heartbeat < 60; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspaces[0], NULL);
        for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
        {
            u16 whole, sliced = 0xFFFF, cached;
            u16 calls = 0;
            u8 current = 0;
            bool8 done;
            if (sState.records[slot].state != WORLD_STATE_TRAVELLING)
                continue;
            WorldSim_ResetPathCache();
            whole = WorldSim_NextEdge(&sState, slot, workspaces[0], NULL);
            WorldSim_ResetPathCache();
            done = WorldSim_NextEdgeBegin(&sState, slot, workspaces[current], FALSE, &sliced);
            while (!done)
            {
                calls++;
                if (calls == 3)
                {
                    // A heap reset: a new block of garbage, the search restarts.
                    current ^= 1;
                    memset(workspaces[current], 0x5A, size);
                    done = WorldSim_NextEdgeBegin(&sState, slot, workspaces[current], FALSE, &sliced);
                    continue;
                }
                done = WorldSim_NextEdgeRun(&sState, slot, workspaces[current], 1 + calls % 4, &sliced, NULL);
            }
            EXPECT_EQ(sliced, whole);
            // Cached now: answered without a workspace.
            EXPECT(WorldSim_NextEdgeBegin(&sState, slot, NULL, FALSE, &cached) || whole == 0xFFFF);
            if (whole != 0xFFFF)
                EXPECT_EQ(cached, whole);
            if (calls > longest)
                longest = calls;
            checked++;
        }
    }
    EXPECT(checked > 0);
    EXPECT(longest > 4);   // some searches really were spread out
    Free(workspaces[1]);
    Free(workspaces[0]);
}

// Back from the league onto a full home map, a trainer stays away until
// there is room; when they do come back, their next step is chosen in the
// heartbeat's sliced phase (first in its order), not inside Begin.
TEST("Trainers back from the league never overfill home and choose in slices")
{
    struct WayfarerWorldContext ctx;
    struct WayfarerWorldHeartbeat hb;
    void *workspace = Alloc(WorldSim_WorkspaceSize());
    u8 will = WorldSim_SlotForCharacter(NOTABLE_TRAINER_WILL), karen = WorldSim_SlotForCharacter(NOTABLE_TRAINER_KAREN);
    u16 home = gWayfarerWorldTrainers[will].homeNode, homeMap = WorldSim_NodeMap(home), heartbeat;
    u8 slot, parked = 0;

    ASSUME(gWayfarerWorldTrainers[karen].homeNode == home);
    Context(&ctx, TRUE, MAP_UNDEFINED);
    WorldSim_NewGame(&sState, &ctx, workspace);
    ctx.derived[will] = ctx.derived[karen] = WORLD_STATE_AWAY_LEAGUE;
    WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
    EXPECT_EQ((u32)sState.records[will].state, WORLD_STATE_AWAY_LEAGUE);
    // Park others at home, dwelling, up to the cap.
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT && WorldSim_Occupancy(&sState, homeMap, 0xFF) < WorldSim_MapCapacity(home); slot++)
    {
        struct WayfarerWorldRecord *record = &sState.records[slot];
        if (slot == will || slot == karen || !WorldSim_IsSimulated(record))
            continue;
        record->node = home;
        record->state = WORLD_STATE_DWELLING;
        record->destKind = WORLD_DEST_NONE;
        record->destId = 0;
        record->arrival = WORLD_ARRIVAL_NONE;
        record->dwell = 3;
        parked++;
    }
    ASSUME(WorldSim_Occupancy(&sState, homeMap, 0xFF) == WorldSim_MapCapacity(home));
    ctx.derived[will] = ctx.derived[karen] = WORLD_DERIVED_NONE;
    for (heartbeat = 0; heartbeat < 40; heartbeat++)
    {
        WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
        // Home is still full on the first one: both stay away.
        if (heartbeat == 0)
        {
            EXPECT_EQ((u32)sState.records[will].state, WORLD_STATE_AWAY_LEAGUE);
            EXPECT_EQ((u32)sState.records[karen].state, WORLD_STATE_AWAY_LEAGUE);
        }
        EXPECT_LE(WorldSim_Occupancy(&sState, homeMap, 0xFF), WorldSim_MapCapacity(home));
        EXPECT(CapsHold(&sState));
    }
    (void)parked;

    // A return with room: Begin seats them at home with no step chosen yet;
    // the first step picks it.
    WorldSim_NewGame(&sState, &ctx, workspace);
    ctx.derived[will] = WORLD_STATE_AWAY_LEAGUE;
    WorldSim_Heartbeat(&sState, &ctx, workspace, NULL);
    ctx.derived[will] = WORLD_DERIVED_NONE;
    ASSUME(WorldSim_Occupancy(&sState, homeMap, will) < WorldSim_MapCapacity(home));
    WorldSim_HeartbeatBegin(&hb, &sState, &ctx, NULL);
    EXPECT_EQ((u32)sState.records[will].state, WORLD_STATE_DWELLING);
    EXPECT_EQ((u32)sState.records[will].destKind, WORLD_DEST_NONE);
    EXPECT_EQ((u32)sState.records[will].node, home);
    EXPECT_EQ(hb.order[0], will);
    while (!WorldSim_HeartbeatStep(&hb, &sState, workspace, 1, NULL))
        ;
    EXPECT_NE((u32)sState.records[will].destKind, WORLD_DEST_NONE);
    Free(workspace);
}

#endif // IS_WAYFARER
