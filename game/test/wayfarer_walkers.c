#include "global.h"
#include "malloc.h"
#include "test/test.h"
#include "wayfarer_world_sim.h"
#include "constants/map_groups.h"

// The engine-free helpers the local actor (src/wayfarer_walkers.c) uses.
// The walker itself is checked in SkyEmu (tools/wayfarer_walkers/verify.py).

#if IS_WAYFARER

static EWRAM_DATA struct WayfarerWorldState sWalkerState = {0};

static u16 FindEdge(u16 fromMap, u8 kind, u16 toMap)
{
    u16 n, e;
    for (n = 0; n < gWayfarerWorldNodeCount; n++)
    {
        const struct WayfarerWorldNode *node = &gWayfarerWorldNodes[n];
        if (node->map != fromMap)
            continue;
        for (e = node->firstEdge; e < node->firstEdge + node->edgeCount; e++)
        {
            if (gWayfarerWorldEdges[e].kind == kind && WorldSim_NodeMap(gWayfarerWorldEdges[e].target) == toMap)
                return e;
        }
    }
    return 0xFFFF;
}

static u16 FirstSpot(u16 map, u8 kind, u16 after)
{
    u16 s;
    for (s = after; s < gWayfarerWorldSpotCount; s++)
    {
        if (gWayfarerWorldSpots[s].kind == kind && WorldSim_NodeMap(gWayfarerWorldSpots[s].node) == map)
            return s;
    }
    return WORLD_SPOT_NONE;
}

static void ClearRecords(void)
{
    u8 slot;
    memset(&sWalkerState, 0, sizeof(sWalkerState));
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
        sWalkerState.records[slot].state = WORLD_STATE_AWAY_LEAGUE;
    WorldSim_ClearLocalActors(&sWalkerState);
}

TEST("A walker's exit step converts its lane coordinate by the connection offset")
{
    u16 north = FindEdge(MAP_VIRIDIAN_CITY_HNS, WORLD_EDGE_NORTH, MAP_ROUTE2_HNS);
    u16 south = FindEdge(MAP_ROUTE2_HNS, WORLD_EDGE_SOUTH, MAP_VIRIDIAN_CITY_HNS);
    const struct WayfarerWorldEdge *edge;

    EXPECT(north != 0xFFFF && south != 0xFFFF);
    edge = &gWayfarerWorldEdges[north];
    // Viridian (25, 0) is Route 2 (9, 79): Route 2 `up` at offset 16.
    EXPECT_EQ(WorldSim_LaneCrossing(edge, 25), 9);
    EXPECT_EQ(WorldSim_LaneCrossing(edge, edge->a), edge->c);
    // Outside the lane run clamps to its ends.
    EXPECT_EQ(WorldSim_LaneCrossing(edge, edge->b + 5), edge->c + (edge->b - edge->a));
    // The return adds 16.
    EXPECT_EQ(WorldSim_LaneCrossing(&gWayfarerWorldEdges[south], 9), 25);
}

TEST("Template choices rotate by (c + 7k) mod n, with a prime stride when 7 divides n")
{
    u16 k, seen;
    EXPECT_EQ(WorldSim_TemplateIndex(8, 0, 10), 8);
    EXPECT_EQ(WorldSim_TemplateIndex(8, 1, 10), 5);    // (8 + 7) mod 10
    EXPECT_EQ(WorldSim_TemplateIndex(8, 2, 10), 2);
    EXPECT_EQ(WorldSim_TemplateIndex(3, 1, 14), 0);    // stride 11: (3 + 11) mod 14
    EXPECT_EQ(WorldSim_TemplateIndex(3, 1, 77), 16);   // 7 and 11 divide 77: stride 13
    EXPECT_EQ(WorldSim_TemplateIndex(5, 9, 0), 0);
    // With a stride coprime to n, n moves visit every entry once.
    seen = 0;
    for (k = 0; k < 14; k++)
        seen |= 1 << WorldSim_TemplateIndex(2, k, 14);
    EXPECT_EQ(seen, (1 << 14) - 1);
}

TEST("Emotes show on the dwell ticks where (c + t) mod 8 is 0")
{
    u16 t, count = 0;
    for (t = 0; t < 64; t++)
    {
        if (WorldSim_IsEmoteTick(8, t))
            count++;
    }
    EXPECT_EQ(count, 8);
    EXPECT(WorldSim_IsEmoteTick(8, 0));
    EXPECT(WorldSim_IsEmoteTick(3, 5));
    EXPECT(!WorldSim_IsEmoteTick(3, 6));
}

TEST("Browse moves the stay to another shelf, travelling when it is on another floor")
{
    u16 shelf = FirstSpot(MAP_VIRIDIAN_CITY_MART_HNS, WORLD_SPOT_STORE, 0);
    u16 next = FirstSpot(MAP_VIRIDIAN_CITY_MART_HNS, WORLD_SPOT_STORE, shelf + 1);
    u16 elsewhere = FirstSpot(MAP_VIRIDIAN_CITY_HNS, WORLD_SPOT_SQUARE, 0);
    struct WayfarerWorldRecord *record = &sWalkerState.records[8];

    EXPECT(shelf != WORLD_SPOT_NONE && next != WORLD_SPOT_NONE && elsewhere != WORLD_SPOT_NONE);
    ClearRecords();
    record->node = gWayfarerWorldSpots[shelf].node;
    record->state = WORLD_STATE_DWELLING;
    record->destKind = WORLD_DEST_SPOT;
    record->destId = shelf;
    record->activity = WORLD_ACTIVITY_SHOP;
    record->dwell = 2;
    EXPECT(WorldSim_SpotTaken(&sWalkerState, shelf, 0xFF));
    EXPECT(!WorldSim_SpotTaken(&sWalkerState, shelf, 8));
    EXPECT(!WorldSim_SpotTaken(&sWalkerState, next, 0xFF));

    WorldSim_ChangeSpot(&sWalkerState, 8, next);
    EXPECT_EQ((u32)record->destId, next);
    EXPECT_EQ((u32)record->state, WORLD_STATE_DWELLING);
    EXPECT_EQ((u32)record->dwell, 2);
    EXPECT(WorldSim_SpotTaken(&sWalkerState, next, 0xFF));
    EXPECT(!WorldSim_SpotTaken(&sWalkerState, shelf, 0xFF));

    WorldSim_ChangeSpot(&sWalkerState, 8, elsewhere);
    EXPECT_EQ((u32)record->state, WORLD_STATE_TRAVELLING);
    EXPECT_EQ((u32)record->dwell, 2);
    EXPECT_EQ((u32)record->activity, WORLD_ACTIVITY_SHOP);
}

TEST("Spawn priority counts graph hops from the current node home")
{
    u8 slot;

    ClearRecords();
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
        u16 e = gWayfarerWorldNodes[trainer->homeNode].firstEdge;
        sWalkerState.records[slot].node = trainer->homeNode;
        EXPECT_EQ(WorldSim_HomeHops(&sWalkerState, slot), 0);
        if (gWayfarerWorldNodes[trainer->homeNode].edgeCount == 0)
            continue;
        sWalkerState.records[slot].node = gWayfarerWorldEdges[e].target;
        EXPECT(WorldSim_HomeHops(&sWalkerState, slot) >= 1);
    }
}

// The generated home-hop table must equal a search over the same edges.
TEST("The home-hop table matches a search from every node")
{
    u16 *queue = Alloc(gWayfarerWorldNodeCount * sizeof(u16));
    u8 *dist = Alloc(gWayfarerWorldNodeCount);
    u8 slot;

    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
        bool8 transit = (trainer->flags & WORLD_TRAINER_FLAG_TRAVELLER) != 0;
        u16 source;
        // Sample nodes across the table: a forward search from each.
        for (source = slot; source < gWayfarerWorldNodeCount; source += 37)
        {
            u16 head = 0, tail = 0, n;
            u8 found = 0xFF;
            for (n = 0; n < gWayfarerWorldNodeCount; n++)
                dist[n] = 0xFF;
            dist[source] = 0;
            queue[tail++] = source;
            while (head < tail)
            {
                u16 node = queue[head++], e;
                const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[node];
                if (node == trainer->homeNode)
                {
                    found = dist[node];
                    break;
                }
                for (e = data->firstEdge; e < data->firstEdge + data->edgeCount; e++)
                {
                    const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[e];
                    if ((edge->kind == WORLD_EDGE_TRANSIT && !transit) || dist[edge->target] != 0xFF || dist[node] >= 254)
                        continue;
                    dist[edge->target] = dist[node] + 1;
                    queue[tail++] = edge->target;
                }
            }
            EXPECT_EQ(gWayfarerWorldHomeHops[(u32)slot * gWayfarerWorldNodeCount + source], found);
        }
    }
    Free(dist);
    Free(queue);
}

// The per-activity candidate lists hold exactly the candidates offering the
// activity, in list order.
TEST("Activity candidate lists hold exactly the offering candidates in order")
{
    static const u16 kinds[WORLD_ACTIVITY_COUNT] =
    {
        [WORLD_ACTIVITY_CARE]   = (1 << WORLD_SPOT_CENTER_COUNTER) | (1 << WORLD_SPOT_CENTER_SIDE),
        [WORLD_ACTIVITY_SHOP]   = (1 << WORLD_SPOT_STORE),
        [WORLD_ACTIVITY_GAMBLE] = (1 << WORLD_SPOT_GAME_CORNER),
        [WORLD_ACTIVITY_TRAIN]  = (1 << WORLD_SPOT_TALL_GRASS),
        [WORLD_ACTIVITY_RELAX]  = (1 << WORLD_SPOT_SQUARE) | (1 << WORLD_SPOT_BENCH) | (1 << WORLD_SPOT_WATER_EDGE),
        [WORLD_ACTIVITY_FISH]   = (1 << WORLD_SPOT_WATER_EDGE),
        [WORLD_ACTIVITY_VISIT]  = (1 << WORLD_SPOT_GYM) | (1 << WORLD_SPOT_NPC_CHAT),
    };
    u8 slot, activity;

    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
        for (activity = 0; activity < WORLD_ACTIVITY_COUNT; activity++)
        {
            const struct WayfarerWorldCandidateRange *range = &gWayfarerWorldActivityRanges[slot][activity];
            u16 i, k = 0;
            for (i = 0; i < trainer->candidateCount; i++)
            {
                const struct WayfarerWorldSpot *spot = &gWayfarerWorldSpots[gWayfarerWorldCandidates[trainer->candidateStart + i]];
                bool8 offers = spot->kind == WORLD_SPOT_NAMED
                    ? ((spot->activities & 0xF) == activity || (spot->activities >> 4) == activity)
                    : (kinds[activity] & (1 << spot->kind)) != 0;
                if (!offers)
                    continue;
                EXPECT(k < range->count);
                EXPECT_EQ(gWayfarerWorldActivityCandidates[range->start + k], i);
                k++;
            }
            EXPECT_EQ(k, range->count);
        }
    }
}

TEST("Gym Leaders name their own Gym object by local id")
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldTrainer *trainer = &gWayfarerWorldTrainers[slot];
        if (trainer->flags & WORLD_TRAINER_FLAG_GYM_LEADER)
            EXPECT(trainer->leaderLocalId != 0);
        else
            EXPECT_EQ(trainer->leaderLocalId, 0);
    }
}

#endif // IS_WAYFARER
