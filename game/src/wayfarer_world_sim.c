// Off-screen world simulation for notable trainers: routines, spot choice,
// shortest-path travel and capacity over the generated walker graph.
// Spec: .product/specs/notable-world-simulation.md and notable-spots.md.
//
// This file is engine-free: the ROM, the mechanics tests and the offline
// report tool (tools/wayfarer_world/report) all compile it. Nothing here
// reads the random number generator; every choice comes from the saved
// records, the context and the tables.

#include "constants/global.h"
#include "wayfarer_world_sim.h"

#if IS_WAYFARER

#define EDGE_NONE   0xFFFF
#define VIA_SOURCE  0xFFFE
#define HOPS_NONE   0xFF

#define SLOT_NONE   0xFF

struct Search
{
    u16 *queue;
    u16 *via;     // first edge of the path from the source, or EDGE_NONE
    u8 *depth;
    u16 tail;     // nodes queued: exactly the ones with via set
};

static const u16 sActivityKinds[WORLD_ACTIVITY_COUNT] =
{
    [WORLD_ACTIVITY_CARE]     = (1 << WORLD_SPOT_CENTER_COUNTER) | (1 << WORLD_SPOT_CENTER_SIDE),
    [WORLD_ACTIVITY_SHOP]     = (1 << WORLD_SPOT_STORE),
    [WORLD_ACTIVITY_GAMBLE]   = (1 << WORLD_SPOT_GAME_CORNER),
    [WORLD_ACTIVITY_TRAIN]    = (1 << WORLD_SPOT_TALL_GRASS),
    [WORLD_ACTIVITY_RELAX]    = (1 << WORLD_SPOT_SQUARE) | (1 << WORLD_SPOT_BENCH) | (1 << WORLD_SPOT_WATER_EDGE),
    [WORLD_ACTIVITY_FISH]     = (1 << WORLD_SPOT_WATER_EDGE),
    [WORLD_ACTIVITY_VISIT]    = (1 << WORLD_SPOT_GYM) | (1 << WORLD_SPOT_NPC_CHAT),
    // Study, home, sightsee and lie low only have named spots.
};

static const u8 sDwell[WORLD_ACTIVITY_COUNT] =
{
    [WORLD_ACTIVITY_VISIT]    = 1,
    [WORLD_ACTIVITY_CARE]     = 2,
    [WORLD_ACTIVITY_SHOP]     = 2,
    [WORLD_ACTIVITY_RELAX]    = 2,
    [WORLD_ACTIVITY_SIGHTSEE] = 2,
    [WORLD_ACTIVITY_GAMBLE]   = 3,
    [WORLD_ACTIVITY_TRAIN]    = 3,
    [WORLD_ACTIVITY_FISH]     = 3,
    [WORLD_ACTIVITY_STUDY]    = 3,
    [WORLD_ACTIVITY_HOME]     = 4,
    [WORLD_ACTIVITY_LIE_LOW]  = 4,
};

static const u8 sLifeActivities[][WORLD_LIFE_EVENT_STEPS] =
{
    [WORLD_LIFE_RECOVERING]  = {WORLD_ACTIVITY_HOME, WORLD_ACTIVITY_RELAX},
    [WORLD_LIFE_CELEBRATING] = {WORLD_ACTIVITY_RELAX, WORLD_ACTIVITY_VISIT},
    [WORLD_LIFE_BROODING]    = {WORLD_ACTIVITY_LIE_LOW, WORLD_ACTIVITY_TRAIN},
};

static const u8 sKindTemplate[WORLD_SPOT_KIND_COUNT] =
{
    [WORLD_SPOT_CENTER_COUNTER] = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_SPOT_CENTER_SIDE]    = WORLD_TEMPLATE_SIT_OR_IDLE,
    [WORLD_SPOT_STORE]          = WORLD_TEMPLATE_BROWSE,
    [WORLD_SPOT_GAME_CORNER]    = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_SPOT_GYM]            = WORLD_TEMPLATE_JUST_LEAVING,
    [WORLD_SPOT_TALL_GRASS]     = WORLD_TEMPLATE_WANDER,
    [WORLD_SPOT_WATER_EDGE]     = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_SPOT_SQUARE]         = WORLD_TEMPLATE_WANDER,
    [WORLD_SPOT_BENCH]          = WORLD_TEMPLATE_SIT_OR_IDLE,
    [WORLD_SPOT_NPC_CHAT]       = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_SPOT_NAMED]          = WORLD_TEMPLATE_DEFAULT,
};

static const u8 sKindEmote[WORLD_SPOT_KIND_COUNT] =
{
    [WORLD_SPOT_TALL_GRASS] = WORLD_EMOTE_EXCLAMATION,
    [WORLD_SPOT_WATER_EDGE] = WORLD_EMOTE_EXCLAMATION,
    [WORLD_SPOT_NPC_CHAT]   = WORLD_EMOTE_ELLIPSIS,
};

static const u8 sNamedTemplate[WORLD_ACTIVITY_COUNT] =
{
    [WORLD_ACTIVITY_STUDY]    = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_ACTIVITY_CARE]     = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_ACTIVITY_SHOP]     = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_ACTIVITY_GAMBLE]   = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_ACTIVITY_TRAIN]    = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_ACTIVITY_FISH]     = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_ACTIVITY_VISIT]    = WORLD_TEMPLATE_STAND_AND_FACE,
    [WORLD_ACTIVITY_RELAX]    = WORLD_TEMPLATE_SIT_OR_IDLE,
    [WORLD_ACTIVITY_SIGHTSEE] = WORLD_TEMPLATE_SIT_OR_IDLE,
    [WORLD_ACTIVITY_HOME]     = WORLD_TEMPLATE_SIT_OR_IDLE,
    [WORLD_ACTIVITY_LIE_LOW]  = WORLD_TEMPLATE_SIT_OR_IDLE,
};

static const u8 sNamedEmote[WORLD_ACTIVITY_COUNT] =
{
    [WORLD_ACTIVITY_TRAIN] = WORLD_EMOTE_EXCLAMATION,
    [WORLD_ACTIVITY_FISH]  = WORLD_EMOTE_EXCLAMATION,
    [WORLD_ACTIVITY_VISIT] = WORLD_EMOTE_ELLIPSIS,
};

// ---------------------------------------------------------------------------
// Small queries

static const struct WayfarerWorldTrainer *Trainer(u8 slot)
{
    return &gWayfarerWorldTrainers[slot];
}

static bool8 IsLeader(u8 slot)
{
    return (Trainer(slot)->flags & WORLD_TRAINER_FLAG_GYM_LEADER) != 0;
}

static bool8 IsTraveller(u8 slot)
{
    return (Trainer(slot)->flags & WORLD_TRAINER_FLAG_TRAVELLER) != 0;
}

u16 WorldSim_NodeMap(u16 node)
{
    if (node >= gWayfarerWorldNodeCount)
        return 0xFFFF;
    return gWayfarerWorldNodes[node].map;
}

static u16 SpotNode(u16 spot)
{
    if (spot >= gWayfarerWorldSpotCount)
        return WORLD_NODE_NONE;
    return gWayfarerWorldSpots[spot].node;
}

bool8 WorldSim_IsSimulated(const struct WayfarerWorldRecord *record)
{
    return record->state == WORLD_STATE_TRAVELLING || record->state == WORLD_STATE_DWELLING;
}

u8 WorldSim_DwellFor(u8 activity)
{
    if (activity >= WORLD_ACTIVITY_COUNT)
        return 1;
    return sDwell[activity];
}

u8 WorldSim_MapCapacity(u16 node)
{
    if (node < gWayfarerWorldNodeCount && (gWayfarerWorldNodes[node].flags & WORLD_NODE_FLAG_INTERIOR))
        return WORLD_CAPACITY_INTERIOR;
    return WORLD_CAPACITY_OUTDOOR;
}

u8 WorldSim_SlotForCharacter(u16 characterId)
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (gWayfarerWorldTrainers[slot].characterId == characterId)
            return slot;
    }
    return SLOT_NONE;
}

u8 WorldSim_ArrivalForEdge(u8 edgeKind)
{
    switch (edgeKind)
    {
    case WORLD_EDGE_NORTH:   return WORLD_ARRIVAL_SOUTH;
    case WORLD_EDGE_SOUTH:   return WORLD_ARRIVAL_NORTH;
    case WORLD_EDGE_EAST:    return WORLD_ARRIVAL_WEST;
    case WORLD_EDGE_WEST:    return WORLD_ARRIVAL_EAST;
    case WORLD_EDGE_WARP:    return WORLD_ARRIVAL_DOOR;
    case WORLD_EDGE_TRANSIT: return WORLD_ARRIVAL_TRANSIT;
    }
    return WORLD_ARRIVAL_NONE;
}

u8 WorldSim_DefaultTemplate(const struct WayfarerWorldSpot *spot, u8 activity)
{
    u8 template;
    if (spot->kind != WORLD_SPOT_NAMED)
        return sKindTemplate[spot->kind];
    template = (spot->flags & WORLD_SPOT_TEMPLATE_MASK) >> WORLD_SPOT_TEMPLATE_SHIFT;
    if (template != WORLD_TEMPLATE_DEFAULT)
        return template;
    if (activity < WORLD_ACTIVITY_COUNT)
        return sNamedTemplate[activity];
    return WORLD_TEMPLATE_SIT_OR_IDLE;
}

u8 WorldSim_TemplateEmote(const struct WayfarerWorldSpot *spot, u8 activity)
{
    if (spot->kind != WORLD_SPOT_NAMED)
        return sKindEmote[spot->kind];
    if (WorldSim_DefaultTemplate(spot, activity) != WORLD_TEMPLATE_STAND_AND_FACE)
        return WORLD_EMOTE_NONE;
    if (activity < WORLD_ACTIVITY_COUNT)
        return sNamedEmote[activity];
    return WORLD_EMOTE_NONE;
}

// The node a record is heading for: its spot's node, or a Gym Leader's Gym.
u16 WorldSim_DestNode(const struct WayfarerWorldState *state, u8 slot)
{
    const struct WayfarerWorldRecord *record = &state->records[slot];
    switch (record->destKind)
    {
    case WORLD_DEST_SPOT:
        return SpotNode(record->destId);
    case WORLD_DEST_HOME:
        if (IsLeader(slot))
            return Trainer(slot)->gymNode;
        return SpotNode(record->destId);
    }
    return WORLD_NODE_NONE;
}

// A Gym Leader at home in their own Gym takes no room there.
static bool8 IsLeaderHome(const struct WayfarerWorldState *state, u8 slot)
{
    return IsLeader(slot) && state->records[slot].destKind == WORLD_DEST_HOME;
}

static bool8 CountsOnMap(const struct WayfarerWorldState *state, u8 slot, u16 map)
{
    const struct WayfarerWorldRecord *record = &state->records[slot];
    u16 destNode;

    if (!WorldSim_IsSimulated(record) && record->state != WORLD_STATE_PINNED)
        return FALSE;
    if (IsLeaderHome(state, slot))
    {
        u16 gymMap = WorldSim_NodeMap(Trainer(slot)->gymNode);
        return map != gymMap && WorldSim_NodeMap(record->node) == map;
    }
    if (WorldSim_NodeMap(record->node) == map)
        return TRUE;
    destNode = WorldSim_DestNode(state, slot);
    return destNode != WORLD_NODE_NONE && WorldSim_NodeMap(destNode) == map;
}

u8 WorldSim_Occupancy(const struct WayfarerWorldState *state, u16 map, u8 exceptSlot)
{
    u8 slot, count = 0;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (slot != exceptSlot && CountsOnMap(state, slot, map))
            count++;
    }
    return count;
}

// Spot choice asks for map occupancy for every candidate; while one choice
// runs the records don't change. On the first question, CountsOnMap is
// summarised per trainer (the maps they count on); every answer is then a
// scan of that summary.
static u16 sCountMapA[WORLD_SIM_TRAINER_COUNT];   // 0xFFFF: counts nowhere
static u16 sCountMapB[WORLD_SIM_TRAINER_COUNT];   // 0xFFFF: none
static u16 sCountNotMap[WORLD_SIM_TRAINER_COUNT]; // a leader at home: never on their Gym's map
static u8 sOccupancySlot;       // the slot the summary excepts
static bool8 sOccupancyActive;  // a spot choice is running
static bool8 sOccupancyReady;   // the summary is built

static void SummariseOccupancy(const struct WayfarerWorldState *state, u8 exceptSlot)
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldRecord *record = &state->records[slot];
        u16 destNode;

        sCountMapA[slot] = sCountMapB[slot] = sCountNotMap[slot] = 0xFFFF;
        if (slot == exceptSlot || (!WorldSim_IsSimulated(record) && record->state != WORLD_STATE_PINNED))
            continue;
        sCountMapA[slot] = WorldSim_NodeMap(record->node);
        if (IsLeaderHome(state, slot))
        {
            sCountNotMap[slot] = WorldSim_NodeMap(Trainer(slot)->gymNode);
            continue;
        }
        destNode = WorldSim_DestNode(state, slot);
        if (destNode != WORLD_NODE_NONE)
            sCountMapB[slot] = WorldSim_NodeMap(destNode);
    }
    sOccupancyReady = TRUE;
}

#define OCCUPANCY_CACHE_SIZE 16
static u16 sOccupancyMaps[OCCUPANCY_CACHE_SIZE];
static u8 sOccupancyCounts[OCCUPANCY_CACHE_SIZE];
static u8 sOccupancyCached;

static u8 CachedOccupancy(const struct WayfarerWorldState *state, u16 map, u8 exceptSlot)
{
    u8 i, count = 0;
    if (!sOccupancyActive || exceptSlot != sOccupancySlot || map == 0xFFFF)
        return WorldSim_Occupancy(state, map, exceptSlot);
    for (i = 0; i < sOccupancyCached; i++)
    {
        if (sOccupancyMaps[i] == map)
            return sOccupancyCounts[i];
    }
    if (!sOccupancyReady)
        SummariseOccupancy(state, exceptSlot);
    for (i = 0; i < WORLD_SIM_TRAINER_COUNT; i++)
    {
        if (sCountMapA[i] == map ? sCountNotMap[i] != map : sCountMapB[i] == map)
            count++;
    }
    if (sOccupancyCached < OCCUPANCY_CACHE_SIZE)
    {
        sOccupancyMaps[sOccupancyCached] = map;
        sOccupancyCounts[sOccupancyCached++] = count;
    }
    return count;
}

static bool8 IsMapFull(const struct WayfarerWorldState *state, u16 node, u8 exceptSlot)
{
    return CachedOccupancy(state, WorldSim_NodeMap(node), exceptSlot) >= WorldSim_MapCapacity(node);
}

static u8 SpotHolders(const struct WayfarerWorldState *state, u16 spot, u8 exceptSlot)
{
    u8 slot, count = 0;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldRecord *record = &state->records[slot];
        if (slot == exceptSlot || !WorldSim_IsSimulated(record))
            continue;
        if ((record->destKind == WORLD_DEST_SPOT || record->destKind == WORLD_DEST_HOME)
         && record->destId == spot && !IsLeaderHome(state, slot))
            count++;
    }
    return count;
}

// While a spot choice runs: the spots the other trainers hold (SpotHolders'
// rule), gathered once instead of per candidate.
static u16 sHeldSpots[WORLD_SIM_TRAINER_COUNT];
static u8 sHeldCount;
static bool8 sHeldReady;

static void GatherHeldSpots(const struct WayfarerWorldState *state, u8 exceptSlot)
{
    u8 slot;
    sHeldCount = 0;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldRecord *record = &state->records[slot];
        if (slot == exceptSlot || !WorldSim_IsSimulated(record))
            continue;
        if ((record->destKind == WORLD_DEST_SPOT || record->destKind == WORLD_DEST_HOME) && !IsLeaderHome(state, slot))
            sHeldSpots[sHeldCount++] = record->destId;
    }
}

static bool8 IsSpotTaken(const struct WayfarerWorldState *state, u16 spot, u8 exceptSlot)
{
    const struct WayfarerWorldSpot *data = &gWayfarerWorldSpots[spot];
    u8 capacity = (data->flags & WORLD_SPOT_FLAG_CAPACITY_2) ? 2 : 1;
    if (sOccupancyActive && exceptSlot == sOccupancySlot)
    {
        u8 i, holders = 0;
        if (!sHeldReady)
        {
            GatherHeldSpots(state, exceptSlot);
            sHeldReady = TRUE;
        }
        for (i = 0; i < sHeldCount; i++)
        {
            if (sHeldSpots[i] == spot)
                holders++;
        }
        return holders >= capacity;
    }
    return SpotHolders(state, spot, exceptSlot) >= capacity;
}

static bool8 IsChattingOnMap(const struct WayfarerWorldState *state, u16 map, u8 exceptSlot)
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldRecord *record = &state->records[slot];
        if (slot == exceptSlot || !WorldSim_IsSimulated(record) || record->destKind != WORLD_DEST_SPOT)
            continue;
        if (gWayfarerWorldSpots[record->destId].kind == WORLD_SPOT_NPC_CHAT
         && WorldSim_NodeMap(gWayfarerWorldSpots[record->destId].node) == map)
            return TRUE;
    }
    return FALSE;
}

static bool8 SpotOffers(const struct WayfarerWorldSpot *spot, u8 activity)
{
    if (spot->kind == WORLD_SPOT_NAMED)
        return (spot->activities & 0xF) == activity || (spot->activities >> 4) == activity;
    return (sActivityKinds[activity] & (1 << spot->kind)) != 0;
}

// ---------------------------------------------------------------------------
// Breadth-first search over the walker graph. Edges are expanded in table
// order and the first discovery wins, so ties between paths go to the lower
// edge order at each node and the path is fixed.

u32 WorldSim_WorkspaceSize(void)
{
    u32 nodes = gWayfarerWorldNodeCount;
    return ((nodes * 2 + nodes * 2 + nodes) + 3) & ~3u;
}

static void SearchInit(struct Search *search, void *workspace)
{
    u32 i, nodes = gWayfarerWorldNodeCount;
    search->queue = (u16 *)workspace;
    search->via = search->queue + nodes;
    search->depth = (u8 *)(search->via + nodes);
    for (i = 0; i < nodes; i++)
        search->via[i] = EDGE_NONE;
}

static bool8 MapListed(const u16 *maps, u8 count, u16 map)
{
    u8 i;
    for (i = 0; i < count; i++)
    {
        if (maps[i] == map)
            return TRUE;
    }
    return FALSE;
}

// Puts back EDGE_NONE on the nodes the last search set, so the next search
// on the same workspace needn't clear every node.
static void SearchRelease(struct Search *search)
{
    u16 i;
    for (i = 0; i < search->tail; i++)
        search->via[search->queue[i]] = EDGE_NONE;
    search->tail = 0;
}

// Searches from source until both targets are found or the bound is reached.
// avoidMaps lists maps the path may not enter (rerouting around full maps).
// clean: the workspace's via[] is all EDGE_NONE already (after SearchRelease).
static void SearchFrom(struct Search *search, void *workspace, bool8 clean, u16 source, u16 targetA, u16 targetB,
                       u8 bound, bool8 transit, const u16 *avoidMaps, u8 avoidCount, struct WayfarerWorldTrace *trace)
{
    u16 head = 0, tail = 0, expanded = 0;
    bool8 foundA = (targetA == WORLD_NODE_NONE), foundB = (targetB == WORLD_NODE_NONE);

    if (clean)
    {
        search->queue = (u16 *)workspace;
        search->via = search->queue + gWayfarerWorldNodeCount;
        search->depth = (u8 *)(search->via + gWayfarerWorldNodeCount);
    }
    else
    {
        SearchInit(search, workspace);
    }
    search->tail = 0;
    if (source >= gWayfarerWorldNodeCount)
        return;
    search->via[source] = VIA_SOURCE;
    search->depth[source] = 0;
    search->queue[tail++] = source;
    if (source == targetA)
        foundA = TRUE;
    if (source == targetB)
        foundB = TRUE;

    while (head < tail && !(foundA && foundB))
    {
        u16 node = search->queue[head++];
        const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[node];
        u16 e;

        expanded++;
        if (search->depth[node] >= bound)
            continue;
        for (e = data->firstEdge; e < data->firstEdge + data->edgeCount; e++)
        {
            const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[e];
            u16 next = edge->target;

            if (edge->kind == WORLD_EDGE_TRANSIT && !transit)
                continue;
            if (next >= gWayfarerWorldNodeCount || search->via[next] != EDGE_NONE)
                continue;
            if (avoidCount != 0 && next != targetA && MapListed(avoidMaps, avoidCount, gWayfarerWorldNodes[next].map))
                continue;
            search->via[next] = (node == source) ? e : search->via[node];
            search->depth[next] = search->depth[node] + 1;
            search->queue[tail++] = next;
            if (next == targetA)
                foundA = TRUE;
            if (next == targetB)
                foundB = TRUE;
        }
    }
    search->tail = tail;

    if (trace != NULL)
    {
        trace->searches++;
        trace->searchNodes += expanded;
        if (expanded > trace->searchNodesMax)
            trace->searchNodesMax = expanded;
    }
}

static void Search(struct Search *search, void *workspace, u16 source, u16 targetA, u16 targetB, u8 bound,
                   bool8 transit, const u16 *avoidMaps, u8 avoidCount, struct WayfarerWorldTrace *trace)
{
    SearchFrom(search, workspace, FALSE, source, targetA, targetB, bound, transit, avoidMaps, avoidCount, trace);
}

static u8 SearchDepth(const struct Search *search, u16 node)
{
    if (node >= gWayfarerWorldNodeCount || search->via[node] == EDGE_NONE)
        return HOPS_NONE;
    return search->depth[node];
}

static u16 SearchFirstEdge(const struct Search *search, u16 node)
{
    if (node >= gWayfarerWorldNodeCount || search->via[node] == EDGE_NONE || search->via[node] == VIA_SOURCE)
        return EDGE_NONE;
    return search->via[node];
}

u16 WorldSim_NextEdge(const struct WayfarerWorldState *state, u8 slot, void *workspace, struct WayfarerWorldTrace *trace)
{
    struct Search search;
    const struct WayfarerWorldRecord *record = &state->records[slot];
    u16 dest = WorldSim_DestNode(state, slot);

    if (dest == WORLD_NODE_NONE || dest == record->node)
        return EDGE_NONE;
    Search(&search, workspace, record->node, dest, WORLD_NODE_NONE, Trainer(slot)->searchBound,
           IsTraveller(slot), NULL, 0, trace);
    return SearchFirstEdge(&search, dest);
}

// ---------------------------------------------------------------------------
// Spot choice (notable-spots.md, "Choosing a spot", plus the routine's
// filters in notable-world-simulation.md, "Choosing the step's spot").

struct Pick
{
    u8 reasons;   // bits of enum WorldSkipReason seen while filtering
};

#define REASON(r) (1 << (r))

static bool8 SpotFits(const struct WayfarerWorldState *state, u8 slot, u16 spot, u8 activity, u8 lifeEvent,
                      bool8 home, struct Pick *pick)
{
    const struct WayfarerWorldSpot *data = &gWayfarerWorldSpots[spot];
    bool8 public = (data->flags & WORLD_SPOT_FLAG_PUBLIC) != 0;

    if (!home)
    {
        bool8 aloof = (Trainer(slot)->flags & WORLD_TRAINER_FLAG_ALOOF) != 0;
        if (aloof && public)
        {
            pick->reasons |= REASON(WORLD_SKIP_ALOOF);
            return FALSE;
        }
        // Celebrating keeps to public spots, except that an aloof trainer
        // celebrates at remote spots near home; brooding keeps to remote ones.
        if ((lifeEvent == WORLD_LIFE_CELEBRATING && !public && !aloof) || (lifeEvent == WORLD_LIFE_BROODING && public))
        {
            pick->reasons |= REASON(WORLD_SKIP_ALOOF);
            return FALSE;
        }
    }
    if (activity == WORLD_ACTIVITY_VISIT)
    {
        if (spot == Trainer(slot)->ownGymSpot)
            return FALSE;
        if (data->kind == WORLD_SPOT_NPC_CHAT && IsChattingOnMap(state, WorldSim_NodeMap(data->node), slot))
        {
            pick->reasons |= REASON(WORLD_SKIP_FULL);
            return FALSE;
        }
    }
    if (IsSpotTaken(state, spot, slot) || IsMapFull(state, data->node, slot))
    {
        pick->reasons |= REASON(WORLD_SKIP_FULL);
        return FALSE;
    }
    return TRUE;
}

static u32 Rotation(u8 slot, const struct WayfarerWorldContext *ctx)
{
    return (u32)(Trainer(slot)->characterId - 1) + ctx->worldProgress;
}

// A favourite's run of spots: the (c + wp) mod n-th spot that fits.
static u16 PickFromRun(const struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx,
                       u16 first, u16 count, u8 activity, struct Pick *pick)
{
    u16 i, fitting = 0, chosen;
    for (i = 0; i < count; i++)
    {
        if (SpotFits(state, slot, first + i, activity, WORLD_LIFE_NONE, FALSE, pick))
            fitting++;
    }
    if (fitting == 0)
        return WORLD_SPOT_NONE;
    chosen = Rotation(slot, ctx) % fitting;
    for (i = 0; i < count; i++)
    {
        if (SpotFits(state, slot, first + i, activity, WORLD_LIFE_NONE, FALSE, pick) && chosen-- == 0)
            return first + i;
    }
    return WORLD_SPOT_NONE;
}

// Scans the trainer's candidate list (sorted by hops, then spot order) for
// the nearest (or farthest) fitting spot within [0, maxHops], optionally
// only on one map, and applies the rotating tie-break.
static u16 PickCandidate(const struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx,
                         u8 activity, u8 lifeEvent, u8 maxHops, u16 onlyMap, bool8 farthest, struct Pick *pick)
{
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);
    const u16 *spots = &gWayfarerWorldCandidates[trainer->candidateStart];
    const u8 *hops = &gWayfarerWorldCandidateHops[trainer->candidateStart];
    // Only the candidates offering the activity, in list order (sorted by
    // hops, then spot order): the same ones SpotOffers would let through.
    const struct WayfarerWorldCandidateRange *range;
    const u16 *positions;
    u16 j, i, best = HOPS_NONE, ties = 0, chosen;
    bool8 offered = FALSE;

    if (activity >= WORLD_ACTIVITY_COUNT)
    {
        pick->reasons |= REASON(WORLD_SKIP_RADIUS);
        return WORLD_SPOT_NONE;
    }
    range = &gWayfarerWorldActivityRanges[slot][activity];
    positions = &gWayfarerWorldActivityCandidates[range->start];

    for (j = 0; j < range->count; j++)
    {
        const struct WayfarerWorldSpot *data;
        i = positions[j];
        data = &gWayfarerWorldSpots[spots[i]];
        // Past the nearest fitting candidate there is nothing nearer.
        if ((!farthest && best != HOPS_NONE && hops[i] > best) || hops[i] > maxHops)
            break;
        if (!SpotOffers(data, activity))
            continue;
        if (onlyMap != 0xFFFF && WorldSim_NodeMap(data->node) != onlyMap)
            continue;
        offered = TRUE;
        if (!SpotFits(state, slot, spots[i], activity, lifeEvent, FALSE, pick))
            continue;
        if (best == HOPS_NONE || (farthest ? hops[i] > best : hops[i] < best))
        {
            best = hops[i];
            ties = 0;
        }
        if (hops[i] == best)
            ties++;
    }
    if (!offered)
        pick->reasons |= REASON(WORLD_SKIP_RADIUS);
    if (ties == 0)
        return WORLD_SPOT_NONE;

    chosen = Rotation(slot, ctx) % ties;
    for (j = 0; j < range->count; j++)
    {
        const struct WayfarerWorldSpot *data;
        i = positions[j];
        data = &gWayfarerWorldSpots[spots[i]];
        if (hops[i] > best)
            break;
        if (hops[i] != best || !SpotOffers(data, activity))
            continue;
        if (onlyMap != 0xFFFF && WorldSim_NodeMap(data->node) != onlyMap)
            continue;
        if (SpotFits(state, slot, spots[i], activity, lifeEvent, FALSE, pick) && chosen-- == 0)
            return spots[i];
    }
    return WORLD_SPOT_NONE;
}

static void SetDestination(struct WayfarerWorldState *state, u8 slot, u8 destKind, u16 destId, u8 activity)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    u16 destNode;

    record->destKind = destKind;
    record->destId = destId;
    record->activity = activity;
    record->dwell = WorldSim_DwellFor(activity);
    record->waited = FALSE;
    destNode = WorldSim_DestNode(state, slot);
    if (destNode == record->node)
        WorldSim_Arrive(state, slot);
    else
        record->state = WORLD_STATE_TRAVELLING;
}

// Picks a spot for one activity and sets it as the destination.
static bool8 ChooseSpot(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx,
                        u8 activity, u8 lifeEvent, struct WayfarerWorldTrace *trace)
{
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);
    struct Pick pick = {0};
    u16 spot = WORLD_SPOT_NONE;
    u8 i;

    // Nothing changes the records until SetDestination below.
    sOccupancySlot = slot;
    sOccupancyActive = TRUE;
    sOccupancyReady = FALSE;
    sOccupancyCached = 0;
    sHeldReady = FALSE;
    if (activity == WORLD_ACTIVITY_HOME)
    {
        if (IsLeader(slot))
        {
            sOccupancyActive = FALSE;
            SetDestination(state, slot, WORLD_DEST_HOME, WORLD_SPOT_NONE, activity);
            return TRUE;
        }
        if (trainer->homePlace != WORLD_SPOT_NONE
         && SpotFits(state, slot, trainer->homePlace, activity, WORLD_LIFE_NONE, TRUE, &pick))
        {
            sOccupancyActive = FALSE;
            SetDestination(state, slot, WORLD_DEST_HOME, trainer->homePlace, activity);
            return TRUE;
        }
        if (trainer->homePlace == WORLD_SPOT_NONE)
            pick.reasons |= REASON(WORLD_SKIP_RADIUS);
    }
    else if (lifeEvent == WORLD_LIFE_CELEBRATING)
    {
        // Public kinds, the home map first, then the narrow radius.
        spot = PickCandidate(state, slot, ctx, activity, lifeEvent, HOPS_NONE - 1, trainer->homeMap, FALSE, &pick);
        if (spot == WORLD_SPOT_NONE)
            spot = PickCandidate(state, slot, ctx, activity, lifeEvent, WORLD_RADIUS_NARROW, 0xFFFF, FALSE, &pick);
    }
    else if (lifeEvent == WORLD_LIFE_BROODING)
    {
        // Remote spots, the farthest candidate in radius.
        spot = PickCandidate(state, slot, ctx, activity, lifeEvent, trainer->radius, 0xFFFF, TRUE, &pick);
    }
    else
    {
        for (i = 0; i < trainer->favouriteCount && spot == WORLD_SPOT_NONE; i++)
        {
            const struct WayfarerWorldFavourite *favourite = &trainer->favourites[i];
            if (favourite->activity == activity)
                spot = PickFromRun(state, slot, ctx, favourite->firstSpot, favourite->spotCount, activity, &pick);
        }
        if (spot == WORLD_SPOT_NONE)
            spot = PickCandidate(state, slot, ctx, activity, lifeEvent, trainer->radius, 0xFFFF, FALSE, &pick);
    }

    sOccupancyActive = FALSE;
    if (spot != WORLD_SPOT_NONE)
    {
        SetDestination(state, slot, WORLD_DEST_SPOT, spot, activity);
        return TRUE;
    }
    if (trace != NULL)
    {
        u8 reason = (pick.reasons & REASON(WORLD_SKIP_FULL)) ? WORLD_SKIP_FULL
                  : (pick.reasons & REASON(WORLD_SKIP_ALOOF)) ? WORLD_SKIP_ALOOF
                  : WORLD_SKIP_RADIUS;
        trace->skips++;
        if (trace->onSkip != NULL)
            trace->onSkip(trace->user, slot, activity, reason);
    }
    return FALSE;
}

// Preparing: a waiting invitation's provisional lineup trains on every
// non-home step; rising momentum turns relax and sightsee into train.
static u8 AdjustForPreparing(u8 slot, u8 activity, const struct WayfarerWorldContext *ctx)
{
    if ((ctx->provisionalMask & (1u << slot)) && activity != WORLD_ACTIVITY_HOME)
        return WORLD_ACTIVITY_TRAIN;
    if ((ctx->risingMask & (1u << slot)) && (activity == WORLD_ACTIVITY_RELAX || activity == WORLD_ACTIVITY_SIGHTSEE))
        return WORLD_ACTIVITY_TRAIN;
    return activity;
}

// No step fits: go home, or stay put for one heartbeat.
static void FallBack(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx, struct WayfarerWorldTrace *trace)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    if (ChooseSpot(state, slot, ctx, WORLD_ACTIVITY_HOME, WORLD_LIFE_NONE, trace))
        return;
    // Staying put keeps the spot they stand on, so nobody else takes it.
    if (WorldSim_DestNode(state, slot) != record->node
     || (record->destKind != WORLD_DEST_NONE && !(IsLeader(slot) && record->destKind == WORLD_DEST_HOME)
         && IsSpotTaken(state, record->destId, slot)))
    {
        record->destKind = WORLD_DEST_NONE;
        record->destId = 0;
    }
    record->state = WORLD_STATE_DWELLING;
    record->arrival = WORLD_ARRIVAL_NONE;
    record->crossing = 0;
    record->dwell = 1;
    record->waited = FALSE;
}

void WorldSim_AdvanceRoutine(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx, struct WayfarerWorldTrace *trace)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);
    u8 attempts = 0, maxAttempts = trainer->cycleLength + record->lifeSteps;

    if (trace != NULL)
        trace->advances++;
    // A leader at home takes no room in their Gym. Leaving it would make them
    // count there while a visitor is inside, so they stay home until it's free.
    if (IsLeaderHome(state, slot) && record->node == trainer->gymNode && IsMapFull(state, record->node, slot))
    {
        record->state = WORLD_STATE_DWELLING;
        record->dwell = 1;
        return;
    }
    while (attempts++ < maxAttempts)
    {
        u8 activity, lifeEvent = WORLD_LIFE_NONE;
        if (record->lifeSteps != 0 && record->lifeEvent != WORLD_LIFE_NONE)
        {
            u8 index = record->lifeSteps >= WORLD_LIFE_EVENT_STEPS ? 0 : WORLD_LIFE_EVENT_STEPS - record->lifeSteps;
            lifeEvent = record->lifeEvent;
            activity = sLifeActivities[lifeEvent][index];
            if (--record->lifeSteps == 0)
                record->lifeEvent = WORLD_LIFE_NONE;
        }
        else
        {
            record->lifeEvent = WORLD_LIFE_NONE;
            record->lifeSteps = 0;
            record->step = (record->step + 1) % trainer->cycleLength;
            activity = AdjustForPreparing(slot, trainer->cycle[record->step], ctx);
        }
        if (ChooseSpot(state, slot, ctx, activity, lifeEvent, trace))
            return;
    }
    FallBack(state, slot, ctx, trace);
}

// Seats a record directly at the spot of its current cycle step (New Game,
// or after a content change), skipping steps without a candidate.
static void Seat(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx, struct WayfarerWorldTrace *trace)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);
    u8 tries;
    bool8 chosen = FALSE;

    record->node = trainer->homeNode;
    record->arrival = WORLD_ARRIVAL_NONE;
    record->crossing = 0;
    record->waited = FALSE;
    record->stayBits = 0;
    for (tries = 0; tries < trainer->cycleLength && !chosen; tries++)
    {
        u8 activity = AdjustForPreparing(slot, trainer->cycle[record->step], ctx);
        chosen = ChooseSpot(state, slot, ctx, activity, WORLD_LIFE_NONE, trace);
        if (!chosen)
            record->step = (record->step + 1) % trainer->cycleLength;
    }
    if (!chosen)
        FallBack(state, slot, ctx, trace);
    if (record->state == WORLD_STATE_TRAVELLING)
    {
        record->node = WorldSim_DestNode(state, slot);
        WorldSim_Arrive(state, slot);
    }
}

// ---------------------------------------------------------------------------
// Derived states (notable-world-simulation.md, "Who is simulated").

static void EnterHomeLocked(struct WayfarerWorldRecord *record, u8 slot)
{
    record->state = WORLD_STATE_HOME_LOCKED;
    record->node = Trainer(slot)->gymNode;
    record->destKind = WORLD_DEST_HOME;
    record->destId = WORLD_SPOT_NONE;
    record->activity = WORLD_ACTIVITY_HOME;
    record->arrival = WORLD_ARRIVAL_NONE;
    record->crossing = 0;
    record->dwell = 0;
    record->waited = FALSE;
}

// The routine starts at its first step: a home first step is the one being
// done now; otherwise the next advance takes step 0.
static void RestartCycle(struct WayfarerWorldRecord *record, u8 slot)
{
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);
    record->step = trainer->cycle[0] == WORLD_ACTIVITY_HOME ? 0 : trainer->cycleLength - 1;
}

static u32 ApplyDerived(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx,
                        struct WayfarerWorldTrace *trace, bool8 enterOnly)
{
    u32 changed = 0;
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        struct WayfarerWorldRecord *record = &state->records[slot];
        u8 want = ctx->derived[slot];
        u8 current = WorldSim_IsSimulated(record) ? WORLD_DERIVED_NONE : record->state;

        if (want == current || (enterOnly && want == WORLD_DERIVED_NONE))
            continue;
        changed |= 1u << slot;
        if (want != WORLD_DERIVED_NONE)
        {
            if (want == WORLD_STATE_HOME_LOCKED)
            {
                EnterHomeLocked(record, slot);
            }
            else
            {
                record->state = want;
                record->arrival = WORLD_ARRIVAL_NONE;
                record->crossing = 0;
                record->waited = FALSE;
            }
            continue;
        }

        record->arrival = WORLD_ARRIVAL_NONE;
        record->crossing = 0;
        record->waited = FALSE;
        switch (current)
        {
        case WORLD_STATE_AWAY_PARTNER:
            // The next advance takes the routine's first step.
            record->step = Trainer(slot)->cycleLength - 1;
            // fallthrough
        case WORLD_STATE_AWAY_LEAGUE:
            // Back at the home base, picking the next step at once. Whatever
            // spot they held before going away is no longer theirs.
            record->node = Trainer(slot)->homeNode;
            record->state = WORLD_STATE_DWELLING;
            record->destKind = WORLD_DEST_NONE;
            record->destId = 0;
            WorldSim_AdvanceRoutine(state, slot, ctx, trace);
            break;
        case WORLD_STATE_PINNED:
            // Moves on at the next heartbeat.
            record->state = WORLD_STATE_DWELLING;
            record->destKind = WORLD_DEST_NONE;
            record->destId = 0;
            record->dwell = 0;
            break;
        case WORLD_STATE_HOME_LOCKED:
        default:
            // Dwelling at home in their Gym; the leader object stays put.
            RestartCycle(record, slot);
            record->node = Trainer(slot)->gymNode;
            record->destKind = WORLD_DEST_HOME;
            record->destId = WORLD_SPOT_NONE;
            record->activity = WORLD_ACTIVITY_HOME;
            record->dwell = WorldSim_DwellFor(WORLD_ACTIVITY_HOME);
            record->state = WORLD_STATE_DWELLING;
            break;
        }
    }
    return changed;
}

u32 WorldSim_ApplyDerived(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace, struct WayfarerWorldTrace *trace)
{
    (void)workspace;
    return ApplyDerived(state, ctx, trace, FALSE);
}

// Load: a trainer who has gone away (or is home-locked) leaves the world at
// once, so the restored map never shows them. Coming back is a routine step
// and waits for the next heartbeat, exactly as without the reload.
void WorldSim_ApplyDerivedOnLoad(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx)
{
    ApplyDerived(state, ctx, NULL, TRUE);
}

// ---------------------------------------------------------------------------
// Travel

void WorldSim_Arrive(struct WayfarerWorldState *state, u8 slot)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    record->state = WORLD_STATE_DWELLING;
    record->arrival = WORLD_ARRIVAL_NONE;
    record->crossing = 0;
    record->stayBits = 0;
    record->waited = FALSE;
}

void WorldSim_TakeEdge(struct WayfarerWorldState *state, u8 slot, u16 edgeIndex, u8 crossing)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[edgeIndex];

    // Leaving the spot ends the stay: a dwelling record becomes travelling,
    // since a dwelling record never carries an arrival.
    if (record->state == WORLD_STATE_DWELLING)
        record->state = WORLD_STATE_TRAVELLING;
    record->node = edge->target;
    record->arrival = WorldSim_ArrivalForEdge(edge->kind);
    record->crossing = crossing;
    record->waited = FALSE;
    if (record->state == WORLD_STATE_TRAVELLING && record->node == WorldSim_DestNode(state, slot))
        WorldSim_Arrive(state, slot);
}

// The crossing an off-screen hop records: the middle of a lane, converted to
// the target map, or the landing warp.
static u8 EdgeCrossing(const struct WayfarerWorldEdge *edge)
{
    switch (edge->kind)
    {
    case WORLD_EDGE_NORTH:
    case WORLD_EDGE_SOUTH:
    case WORLD_EDGE_EAST:
    case WORLD_EDGE_WEST:
        return edge->c + (edge->b - edge->a) / 2;
    }
    return edge->c;
}

static void Hop(struct WayfarerWorldState *state, u8 slot, u16 edgeIndex, struct WayfarerWorldTrace *trace)
{
    WorldSim_TakeEdge(state, slot, edgeIndex, EdgeCrossing(&gWayfarerWorldEdges[edgeIndex]));
    if (trace != NULL)
        trace->hops++;
}

static bool8 HopAllowed(const struct WayfarerWorldState *state, u8 slot, u16 edgeIndex, u16 destNode)
{
    u16 target = gWayfarerWorldEdges[edgeIndex].target;
    if (WorldSim_NodeMap(target) == WorldSim_NodeMap(destNode))
        return TRUE;  // spot choice already reserved room there
    return !IsMapFull(state, target, slot);
}

static void Travel(struct WayfarerWorldState *state, u8 slot, u16 firstEdge, void *workspace, struct WayfarerWorldTrace *trace)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    u16 destNode = WorldSim_DestNode(state, slot);
    u16 fullMaps[WORLD_SIM_TRAINER_COUNT * 2];
    u16 rerouted;
    u8 fullCount = 0, other;
    struct Search search;

    if (destNode == WORLD_NODE_NONE || destNode == record->node)
    {
        WorldSim_Arrive(state, slot);
        return;
    }
    if (firstEdge == EDGE_NONE)
    {
        // Unreachable within the bound: give the step up.
        return;
    }
    if (!record->waited)
    {
        if (HopAllowed(state, slot, firstEdge, destNode))
        {
            Hop(state, slot, firstEdge, trace);
            return;
        }
        // Blocked: wait one heartbeat.
        record->waited = TRUE;
        if (trace != NULL)
            trace->waits++;
        return;
    }

    // Blocked before: route around every full map, if a path exists. The
    // waited bit stays set across rerouted hops, so the next heartbeat keeps
    // avoiding the full map instead of heading straight back to it.
    for (other = 0; other < WORLD_SIM_TRAINER_COUNT; other++)
    {
        const struct WayfarerWorldRecord *otherRecord = &state->records[other];
        u16 nodes[2], i;
        nodes[0] = otherRecord->node;
        nodes[1] = WorldSim_DestNode(state, other);
        if (other == slot)
            continue;
        for (i = 0; i < 2; i++)
        {
            u16 map = WorldSim_NodeMap(nodes[i]);
            if (nodes[i] == WORLD_NODE_NONE || map == WorldSim_NodeMap(destNode) || MapListed(fullMaps, fullCount, map))
                continue;
            if (IsMapFull(state, nodes[i], slot))
                fullMaps[fullCount++] = map;
        }
    }
    Search(&search, workspace, record->node, destNode, WORLD_NODE_NONE, Trainer(slot)->searchBound,
           IsTraveller(slot), fullMaps, fullCount, trace);
    rerouted = SearchFirstEdge(&search, destNode);
    if (rerouted != EDGE_NONE && HopAllowed(state, slot, rerouted, destNode))
    {
        Hop(state, slot, rerouted, trace);
        if (rerouted != firstEdge)
        {
            if (trace != NULL)
                trace->reroutes++;
            if (record->state == WORLD_STATE_TRAVELLING)
                record->waited = TRUE;
        }
    }
    else if (trace != NULL)
    {
        trace->waits++;
    }
}

// ---------------------------------------------------------------------------
// Heartbeat

// Hops from a node to the trainer's home node within their search bound:
// the generator's reverse search over the same edges, so the same as a
// search from the node would find.
static u8 HomeHopsFrom(u8 slot, u16 node)
{
    u8 hops;
    if (node >= gWayfarerWorldNodeCount)
        return HOPS_NONE;
    hops = gWayfarerWorldHomeHops[(u32)slot * gWayfarerWorldNodeCount + node];
    return hops <= Trainer(slot)->searchBound ? hops : HOPS_NONE;
}

static bool8 IsOnPlayerMap(const struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx)
{
    return WorldSim_NodeMap(state->records[slot].node) == ctx->playerMap;
}

void WorldSim_Heartbeat(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace, struct WayfarerWorldTrace *trace)
{
    u8 order[WORLD_SIM_TRAINER_COUNT];
    u8 homeHops[WORLD_SIM_TRAINER_COUNT];
    u16 firstEdge[WORLD_SIM_TRAINER_COUNT];
    u8 slot, count = 0, i, j;
    struct Search search;
    bool8 clean = FALSE;
    // A trainer whose derived state changed has already acted this heartbeat.
    u32 changed = WorldSim_ApplyDerived(state, ctx, workspace, trace);

    // The priority (hops from the current node home) comes from the
    // generated table; a search per travelling trainer gives the path's
    // first edge (stopping at the destination finds the same edge).
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        struct WayfarerWorldRecord *record = &state->records[slot];
        u16 destNode;

        firstEdge[slot] = EDGE_NONE;
        if (!WorldSim_IsSimulated(record) || IsOnPlayerMap(state, slot, ctx) || ((ctx->frozenMask | changed) & (1u << slot)))
            continue;
        destNode = record->state == WORLD_STATE_TRAVELLING ? WorldSim_DestNode(state, slot) : WORLD_NODE_NONE;
        homeHops[slot] = HomeHopsFrom(slot, record->node);
        if (destNode != WORLD_NODE_NONE)
        {
            SearchFrom(&search, workspace, clean, record->node, destNode, WORLD_NODE_NONE,
                       Trainer(slot)->searchBound, IsTraveller(slot), NULL, 0, trace);
            firstEdge[slot] = SearchFirstEdge(&search, destNode);
            SearchRelease(&search);
            clean = TRUE;
        }

        // Priority order: fewer hops home first, then catalog order.
        for (i = count; i > 0 && homeHops[order[i - 1]] > homeHops[slot]; i--)
            order[i] = order[i - 1];
        order[i] = slot;
        count++;
    }

    for (j = 0; j < count; j++)
    {
        struct WayfarerWorldRecord *record;
        slot = order[j];
        record = &state->records[slot];
        if (record->state == WORLD_STATE_DWELLING)
        {
            if (record->dwell > 0)
                record->dwell--;
            if (record->dwell == 0)
                WorldSim_AdvanceRoutine(state, slot, ctx, trace);
        }
        else if (record->state == WORLD_STATE_TRAVELLING)
        {
            if (firstEdge[slot] == EDGE_NONE && WorldSim_DestNode(state, slot) != record->node)
            {
                // No path within the bound (should not happen: the build
                // checks reachability). Pick the next step instead.
                WorldSim_AdvanceRoutine(state, slot, ctx, trace);
                continue;
            }
            Travel(state, slot, firstEdge[slot], workspace, trace);
        }
    }
}

// ---------------------------------------------------------------------------
// New Game, content changes, load checks and the league hook

static void SeatAll(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, bool8 keepRoutine)
{
    u8 slot;

    // Everyone starts at home, so priority order is catalog order.
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        struct WayfarerWorldRecord *record = &state->records[slot];
        u8 step = record->step, lifeEvent = record->lifeEvent, lifeSteps = record->lifeSteps;

        *record = (struct WayfarerWorldRecord){0};
        record->node = Trainer(slot)->homeNode;
        record->destKind = WORLD_DEST_NONE;
        // Not placed yet: an away state keeps unseated trainers out of the
        // occupancy counts until Seat gives them a spot.
        record->state = WORLD_STATE_AWAY_LEAGUE;
        if (keepRoutine)
        {
            record->step = step < Trainer(slot)->cycleLength ? step : 0;
            record->lifeEvent = lifeEvent;
            record->lifeSteps = lifeSteps;
        }
        if (ctx->derived[slot] != WORLD_DERIVED_NONE)
        {
            if (ctx->derived[slot] == WORLD_STATE_HOME_LOCKED)
            {
                if (!keepRoutine)
                    RestartCycle(record, slot);
                EnterHomeLocked(record, slot);
            }
            else
            {
                record->state = ctx->derived[slot];
            }
        }
    }
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (ctx->derived[slot] == WORLD_DERIVED_NONE)
            Seat(state, slot, ctx, NULL);
    }
}

void WorldSim_NewGame(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace)
{
    (void)workspace;
    state->schemaVersion = WORLD_SCHEMA_VERSION;
    state->reserved = 0;
    state->contentHash = gWayfarerWorldContentHash;
    WorldSim_ClearLocalActors(state);
    SeatAll(state, ctx, FALSE);
}

void WorldSim_Reseat(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace)
{
    (void)workspace;
    state->schemaVersion = WORLD_SCHEMA_VERSION;
    state->reserved = 0;
    state->contentHash = gWayfarerWorldContentHash;
    WorldSim_ClearLocalActors(state);
    SeatAll(state, ctx, TRUE);
}

bool8 WorldSim_IsValid(const struct WayfarerWorldState *state, void *workspace)
{
    u8 slot;
    struct Search search;

    if (state->schemaVersion != WORLD_SCHEMA_VERSION || state->reserved != 0)
        return FALSE;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldRecord *record = &state->records[slot];
        u16 destNode;

        if (record->node >= gWayfarerWorldNodeCount
         || record->destKind == WORLD_DEST_RESERVED_HAUNT
         || record->state >= WORLD_STATE_COUNT
         || record->arrival >= WORLD_ARRIVAL_COUNT
         || (record->state == WORLD_STATE_DWELLING && record->arrival != WORLD_ARRIVAL_NONE)
         || record->activity >= WORLD_ACTIVITY_COUNT
         || (record->lifeEvent == WORLD_LIFE_NONE && record->lifeSteps != 0)
         || record->step >= Trainer(slot)->cycleLength
         || record->reserved != 0)
            return FALSE;
        if (record->destKind == WORLD_DEST_SPOT && record->destId >= gWayfarerWorldSpotCount)
            return FALSE;
        if (record->destKind == WORLD_DEST_HOME
         && (IsLeader(slot) ? record->destId != WORLD_SPOT_NONE : record->destId >= gWayfarerWorldSpotCount))
            return FALSE;
        if (record->state == WORLD_STATE_HOME_LOCKED && !IsLeader(slot))
            return FALSE;
        if (!WorldSim_IsSimulated(record))
            continue;
        destNode = WorldSim_DestNode(state, slot);
        if (destNode == WORLD_NODE_NONE || destNode == record->node || workspace == NULL)
            continue;
        Search(&search, workspace, record->node, destNode, WORLD_NODE_NONE, HOPS_NONE - 1, IsTraveller(slot), NULL, 0, NULL);
        if (SearchDepth(&search, destNode) == HOPS_NONE)
            return FALSE;
    }
    return TRUE;
}

bool8 WorldSim_LocalActorsValid(const struct WayfarerWorldState *state, u16 map, u16 width, u16 height)
{
    u32 seen = 0;
    u8 i, slot, x, y, facing;

    for (i = 0; i < WORLD_LOCAL_ACTOR_COUNT; i++)
    {
        if (!WorldSim_GetLocalActor(state, i, &slot, &x, &y, &facing))
        {
            const u8 *entry = &state->localActors[i * WORLD_LOCAL_ACTOR_BYTES];
            if (entry[0] != WORLD_LOCAL_ACTOR_NONE || entry[1] != 0 || entry[2] != 0)
                return FALSE;
            continue;
        }
        if (slot >= WORLD_SIM_TRAINER_COUNT || (seen & (1u << slot)))
            return FALSE;
        seen |= 1u << slot;
        if (!WorldSim_IsSimulated(&state->records[slot]) || WorldSim_NodeMap(state->records[slot].node) != map)
            return FALSE;
        if (x >= width || y >= height)
            return FALSE;
    }
    return TRUE;
}

void WorldSim_OnLeagueResolved(struct WayfarerWorldState *state, const u16 *lineup, u8 count, bool8 playerWon, u16 championId)
{
    u8 i;
    for (i = 0; i < count; i++)
    {
        u8 slot = WorldSim_SlotForCharacter(lineup[i]);
        struct WayfarerWorldRecord *record;
        if (slot == SLOT_NONE)
            continue;
        record = &state->records[slot];
        if (!playerWon && lineup[i] == championId)
            record->lifeEvent = WORLD_LIFE_CELEBRATING;
        else if (playerWon && i == count - 1)
            record->lifeEvent = WORLD_LIFE_BROODING;
        else
            record->lifeEvent = WORLD_LIFE_RECOVERING;
        record->lifeSteps = WORLD_LIFE_EVENT_STEPS;
    }
}

// ---------------------------------------------------------------------------
// Local actor helpers

u8 WorldSim_HomeHops(const struct WayfarerWorldState *state, u8 slot, void *workspace)
{
    (void)workspace;
    return HomeHopsFrom(slot, state->records[slot].node);
}

bool8 WorldSim_SpotTaken(const struct WayfarerWorldState *state, u16 spot, u8 exceptSlot)
{
    if (spot >= gWayfarerWorldSpotCount)
        return TRUE;
    return IsSpotTaken(state, spot, exceptSlot);
}

void WorldSim_ChangeSpot(struct WayfarerWorldState *state, u8 slot, u16 spot)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    if (spot >= gWayfarerWorldSpotCount || !WorldSim_IsSimulated(record))
        return;
    record->destKind = WORLD_DEST_SPOT;
    record->destId = spot;
    record->waited = FALSE;
    if (SpotNode(spot) != record->node)
        record->state = WORLD_STATE_TRAVELLING;
}

u8 WorldSim_LaneCrossing(const struct WayfarerWorldEdge *edge, u8 coord)
{
    if (coord < edge->a)
        coord = edge->a;
    if (coord > edge->b)
        coord = edge->b;
    return edge->c + (coord - edge->a);
}

u16 WorldSim_TemplateIndex(u8 c, u16 k, u16 n)
{
    static const u8 sPrimes[] = {7, 11, 13, 17, 19, 23, 29, 31};
    u32 stride = 7;
    u8 i;

    if (n == 0)
        return 0;
    for (i = 0; i < sizeof(sPrimes); i++)
    {
        stride = sPrimes[i];
        if (n % stride != 0)
            break;
    }
    return ((u32)c + stride * k) % n;
}

bool8 WorldSim_IsEmoteTick(u8 c, u16 t)
{
    return ((u32)c + t) % 8 == 0;
}

// ---------------------------------------------------------------------------
// Local actor block

void WorldSim_ClearLocalActors(struct WayfarerWorldState *state)
{
    u8 i;
    for (i = 0; i < WORLD_LOCAL_ACTOR_COUNT; i++)
    {
        state->localActors[i * WORLD_LOCAL_ACTOR_BYTES + 0] = WORLD_LOCAL_ACTOR_NONE;
        state->localActors[i * WORLD_LOCAL_ACTOR_BYTES + 1] = 0;
        state->localActors[i * WORLD_LOCAL_ACTOR_BYTES + 2] = 0;
    }
    state->padding[0] = state->padding[1] = state->padding[2] = 0;
}

bool8 WorldSim_GetLocalActor(const struct WayfarerWorldState *state, u8 index, u8 *slot, u8 *x, u8 *y, u8 *facing)
{
    const u8 *entry = &state->localActors[index * WORLD_LOCAL_ACTOR_BYTES];
    if ((entry[0] & 0x3F) == WORLD_LOCAL_ACTOR_NONE)
        return FALSE;
    *slot = entry[0] & 0x3F;
    *facing = (entry[0] >> 6) + 1;  // DIR_SOUTH..DIR_EAST
    *x = entry[1];
    *y = entry[2];
    return TRUE;
}

void WorldSim_SetLocalActor(struct WayfarerWorldState *state, u8 index, u8 slot, u8 x, u8 y, u8 facing)
{
    u8 *entry = &state->localActors[index * WORLD_LOCAL_ACTOR_BYTES];
    u8 dir = (facing >= 1 && facing <= 4) ? facing - 1 : 0;
    entry[0] = (slot & 0x3F) | (dir << 6);
    entry[1] = x;
    entry[2] = y;
}

#endif // IS_WAYFARER
