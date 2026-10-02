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

// A breadth-first search that can run in slices: SearchStart seeds it and
// each SearchRun expands up to a given number of nodes.
struct Search
{
    u16 *queue;
    u16 *via;     // the node a node was discovered from, VIA_SOURCE, or EDGE_NONE
    u8 *depth;
    const u16 *avoidMaps;
    u16 head;     // next node to expand
    u16 tail;     // nodes queued: exactly the ones with via set
    u16 expanded;
    u16 source;
    u16 targetA;
    u16 targetB;
    u8 bound;
    u8 avoidCount;
    bool8 transit;
    bool8 foundA;
    bool8 foundB;
    bool8 traced; // counted in the trace when it finishes (a valid source)
    u8 gathered;  // a heartbeat reroute: trainers checked for full maps so far
};

// The incremental heartbeat's phases for the trainer it is acting for.
enum
{
    HB_PHASE_START,         // dwell, or find the travel's first edge
    HB_PHASE_EDGE_SEARCH,   // searching for the first edge (no cached path)
    HB_PHASE_ACT,           // first edge known: travel, or advance the routine
    HB_PHASE_REROUTE,       // blocked twice: searching around the full maps
    HB_PHASE_ADVANCE,       // advancing the routine, one spot choice per unit
};

// Room for the maps a reroute avoids (two per other trainer).
#define FULL_MAPS_MAX (WORLD_SIM_TRAINER_COUNT * 2)
// Trainers a heartbeat reroute checks for full maps per work unit (each
// check is two occupancy lookups, about 1.5 scanlines on the GBA).
#define GATHER_PER_UNIT 5

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
    case WORLD_EDGE_WATER:   return WORLD_ARRIVAL_WATER;
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

// The caches below live in EWRAM: IWRAM is kept for the stack and the
// engine's hot data.
#ifdef __arm__
#define SIM_CACHE __attribute__((section(".sbss")))  // EWRAM_DATA
#else
#define SIM_CACHE
#endif

// Spot choice asks for map occupancy for every candidate; while one choice
// runs the records don't change. On the first question, CountsOnMap is
// summarised per trainer (the maps they count on); every answer is then a
// scan of that summary.
static SIM_CACHE u16 sCountMapA[WORLD_SIM_TRAINER_COUNT];   // 0xFFFF: counts nowhere
static SIM_CACHE u16 sCountMapB[WORLD_SIM_TRAINER_COUNT];   // 0xFFFF: none
static SIM_CACHE u16 sCountNotMap[WORLD_SIM_TRAINER_COUNT]; // a leader at home: never on their Gym's map
static SIM_CACHE u8 sOccupancySlot;       // the slot the summary excepts
static SIM_CACHE bool8 sOccupancyActive;  // a spot choice is running
static SIM_CACHE bool8 sOccupancyReady;   // the summary is built

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
static SIM_CACHE u16 sOccupancyMaps[OCCUPANCY_CACHE_SIZE];
static SIM_CACHE u8 sOccupancyCounts[OCCUPANCY_CACHE_SIZE];
static SIM_CACHE u8 sOccupancyCached;

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
static SIM_CACHE u16 sHeldSpots[WORLD_SIM_TRAINER_COUNT];
static SIM_CACHE u8 sHeldCount;
static SIM_CACHE bool8 sHeldReady;

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

// The workspace: the search arrays (queue, via, depth), then the state of
// a sliced heartbeat search and the maps its reroute avoids. The struct
// starts at its own alignment (4 on the GBA, 8 for the host's pointers).
#define SEARCH_ALIGN ((u32)__alignof__(struct Search))
static u32 SearchArraysSize(void)
{
    u32 nodes = gWayfarerWorldNodeCount;
    return ((nodes * 2 + nodes * 2 + nodes) + SEARCH_ALIGN - 1) & ~(SEARCH_ALIGN - 1);
}

u32 WorldSim_WorkspaceSize(void)
{
    return SearchArraysSize() + sizeof(struct Search) + FULL_MAPS_MAX * sizeof(u16);
}

static struct Search *WorkspaceSearch(void *workspace)
{
    return (struct Search *)((u8 *)workspace + SearchArraysSize());
}

static u16 *WorkspaceFullMaps(void *workspace)
{
    return (u16 *)(WorkspaceSearch(workspace) + 1);
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

// Seeds a search from source towards up to two targets within the bound.
// avoidMaps lists maps the path may not enter (rerouting around full maps).
// clean: the workspace's via[] is all EDGE_NONE already (after SearchRelease).
static void SearchStart(struct Search *search, void *workspace, bool8 clean, u16 source, u16 targetA, u16 targetB,
                        u8 bound, bool8 transit, const u16 *avoidMaps, u8 avoidCount)
{
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
    search->head = search->tail = search->expanded = 0;
    search->source = source;
    search->targetA = targetA;
    search->targetB = targetB;
    search->bound = bound;
    search->transit = transit;
    search->avoidMaps = avoidMaps;
    search->avoidCount = avoidCount;
    search->foundA = (targetA == WORLD_NODE_NONE);
    search->foundB = (targetB == WORLD_NODE_NONE);
    search->traced = FALSE;
    if (source >= gWayfarerWorldNodeCount)
        return;  // an empty search, never counted
    search->traced = TRUE;
    search->via[source] = VIA_SOURCE;
    search->depth[source] = 0;
    search->queue[search->tail++] = source;
    if (source == targetA)
        search->foundA = TRUE;
    if (source == targetB)
        search->foundB = TRUE;
}

static bool8 SearchDone(const struct Search *search)
{
    return search->head >= search->tail || (search->foundA && search->foundB);
}

// Expands up to maxNodes nodes (adding them to *spent). TRUE once the search
// has found both targets or run out of nodes; only then is it counted in the
// trace, so a search sliced over many calls counts exactly as one run at once.
static bool8 SearchRun(struct Search *search, u16 maxNodes, u16 *spent, struct WayfarerWorldTrace *trace)
{
    u16 ran = 0;

    while (ran < maxNodes && !SearchDone(search))
    {
        u16 node = search->queue[search->head++];
        const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[node];
        u16 e;

        ran++;
        search->expanded++;
        if (search->depth[node] >= search->bound)
            continue;
        for (e = data->firstEdge; e < data->firstEdge + data->edgeCount; e++)
        {
            const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[e];
            u16 next = edge->target;

            if (edge->kind == WORLD_EDGE_TRANSIT && !search->transit)
                continue;
            if (next >= gWayfarerWorldNodeCount || search->via[next] != EDGE_NONE)
                continue;
            if (search->avoidCount != 0 && next != search->targetA
             && MapListed(search->avoidMaps, search->avoidCount, gWayfarerWorldNodes[next].map))
                continue;
            search->via[next] = node;
            search->depth[next] = search->depth[node] + 1;
            search->queue[search->tail++] = next;
            if (next == search->targetA)
                search->foundA = TRUE;
            if (next == search->targetB)
                search->foundB = TRUE;
        }
    }
    if (spent != NULL)
        *spent += ran;
    if (!SearchDone(search))
        return FALSE;

    if (trace != NULL && search->traced)
    {
        trace->searches++;
        trace->searchNodes += search->expanded;
        if (search->expanded > trace->searchNodesMax)
            trace->searchNodesMax = search->expanded;
    }
    search->traced = FALSE;
    return TRUE;
}

// Searches from source until both targets are found or the bound is reached.
static void SearchFrom(struct Search *search, void *workspace, bool8 clean, u16 source, u16 targetA, u16 targetB,
                       u8 bound, bool8 transit, const u16 *avoidMaps, u8 avoidCount, struct WayfarerWorldTrace *trace)
{
    SearchStart(search, workspace, clean, source, targetA, targetB, bound, transit, avoidMaps, avoidCount);
    while (!SearchRun(search, 0xFFFF, NULL, trace))
        ;
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

// The first edge in table order from one node to another that the search
// may take: the edge that discovered `to` (a later duplicate never does).
static u16 EdgeBetween(u16 from, u16 to, bool8 transit)
{
    const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[from];
    u16 e;
    for (e = data->firstEdge; e < data->firstEdge + data->edgeCount; e++)
    {
        if (gWayfarerWorldEdges[e].target == to && !(gWayfarerWorldEdges[e].kind == WORLD_EDGE_TRANSIT && !transit))
            return e;
    }
    return EDGE_NONE;
}

// The first edge of the path to a found node: the edge from the source to
// the path's first node (the discovery order makes it the lowest such edge).
static u16 SearchFirstEdge(const struct Search *search, u16 node)
{
    if (node >= gWayfarerWorldNodeCount || search->via[node] == EDGE_NONE || search->via[node] == VIA_SOURCE)
        return EDGE_NONE;
    while (search->via[node] != search->source)
        node = search->via[node];
    return EdgeBetween(search->source, node, search->transit);
}

// Travel path cache (EWRAM, rebuilt on demand): the first nodes of each
// trainer's last searched path to their destination. Every suffix of a
// breadth-first path is the path a new search from that node would find
// (ties go the same way), so following it gives the same hops as searching
// at every heartbeat; long trips over Surf-linked seas would otherwise
// search most of the graph each heartbeat. Reroutes (avoiding full maps)
// always search.
#define PATH_CACHE_NODES 12  // re-search every 11 hops; EWRAM is tight (the test build)

struct PathCache
{
    u16 dest;
    u8 count;     // nodes held; 0 = empty
    u8 padding;
    u16 nodes[PATH_CACHE_NODES];
};

static SIM_CACHE struct PathCache sPaths[WORLD_SIM_TRAINER_COUNT];

void WorldSim_ResetPathCache(void)
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
        sPaths[slot].count = 0;
}

static u16 CachedFirstEdge(u8 slot, u16 node, u16 dest)
{
    const struct PathCache *path = &sPaths[slot];
    u8 i;
    if (path->dest != dest || path->count > PATH_CACHE_NODES)
        return EDGE_NONE;
    for (i = 0; i + 1 < path->count; i++)
    {
        if (path->nodes[i] == node)
            return EdgeBetween(node, path->nodes[i + 1], IsTraveller(slot));
    }
    return EDGE_NONE;
}

static void CachePath(u8 slot, const struct Search *search, u16 dest)
{
    struct PathCache *path = &sPaths[slot];
    u16 node = dest;
    u8 depth;

    path->count = 0;
    if (dest >= gWayfarerWorldNodeCount || search->via[dest] == EDGE_NONE)
        return;
    depth = search->depth[dest];
    path->dest = dest;
    for (;;)
    {
        if (depth < PATH_CACHE_NODES)
            path->nodes[depth] = node;
        if (depth == 0)
            break;
        node = search->via[node];
        depth--;
    }
    path->count = search->depth[dest] + 1 < PATH_CACHE_NODES ? search->depth[dest] + 1 : PATH_CACHE_NODES;
}

u16 WorldSim_NextEdge(const struct WayfarerWorldState *state, u8 slot, void *workspace, struct WayfarerWorldTrace *trace)
{
    u16 edge;

    if (WorldSim_NextEdgeBegin(state, slot, workspace, FALSE, &edge))
        return edge;
    while (!WorldSim_NextEdgeRun(state, slot, workspace, 0xFFFF, &edge, trace))
        ;
    return edge;
}

// The same question in slices (the local actor plans a trip over frames):
// the cached path answers at once; otherwise a search starts in the
// workspace and each run expands up to `budget` nodes. The search reads only
// the trainer's node and destination, so the caller restarts it (Begin) if
// either changes or the workspace is lost.
bool8 WorldSim_NextEdgeBegin(const struct WayfarerWorldState *state, u8 slot, void *workspace, bool8 clean, u16 *edge)
{
    const struct WayfarerWorldRecord *record = &state->records[slot];
    u16 dest = WorldSim_DestNode(state, slot);

    *edge = EDGE_NONE;
    if (dest == WORLD_NODE_NONE || dest == record->node)
        return TRUE;
    *edge = CachedFirstEdge(slot, record->node, dest);
    if (*edge != EDGE_NONE)
        return TRUE;
    if (workspace == NULL)
        return FALSE;  // only asking the cache
    SearchStart(WorkspaceSearch(workspace), workspace, clean, record->node, dest, WORLD_NODE_NONE,
                Trainer(slot)->searchBound, IsTraveller(slot), NULL, 0);
    return FALSE;
}

bool8 WorldSim_NextEdgeRun(const struct WayfarerWorldState *state, u8 slot, void *workspace, u16 budget, u16 *edge,
                           struct WayfarerWorldTrace *trace)
{
    struct Search *search = WorkspaceSearch(workspace);
    u16 dest = WorldSim_DestNode(state, slot);

    if (!SearchRun(search, budget == 0 ? 1 : budget, NULL, trace))
        return FALSE;
    *edge = SearchFirstEdge(search, dest);
    CachePath(slot, search, dest);
    SearchRelease(search);
    return TRUE;
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

// The routine's next step, one spot choice per call (the heartbeat spreads
// a long search for a fitting step over frames). *attempt starts at 0;
// TRUE once the step is settled. Between calls nothing else may change the
// records, so the result is the same as making every choice at once.
static bool8 AdvanceRoutineStep(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx,
                                struct WayfarerWorldTrace *trace, u8 *attempt, u8 *attemptMax)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);

    if (*attempt == 0)
    {
        if (trace != NULL)
            trace->advances++;
        // A leader at home takes no room in their Gym. Leaving it would make them
        // count there while a visitor is inside, so they stay home until it's free.
        if (IsLeaderHome(state, slot) && record->node == trainer->gymNode && IsMapFull(state, record->node, slot))
        {
            record->state = WORLD_STATE_DWELLING;
            record->dwell = 1;
            return TRUE;
        }
        // Fixed as the advance starts: the life steps count down below.
        *attemptMax = trainer->cycleLength + record->lifeSteps;
    }
    if (*attempt < *attemptMax)
    {
        u8 activity, lifeEvent = WORLD_LIFE_NONE;
        (*attempt)++;
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
        return ChooseSpot(state, slot, ctx, activity, lifeEvent, trace);
    }
    FallBack(state, slot, ctx, trace);
    return TRUE;
}

void WorldSim_AdvanceRoutine(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx, struct WayfarerWorldTrace *trace)
{
    u8 attempt = 0, attemptMax = 0;
    while (!AdvanceRoutineStep(state, slot, ctx, trace, &attempt, &attemptMax))
        ;
}

bool8 WorldSim_AdvanceRoutineStep(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx,
                                  struct WayfarerWorldTrace *trace, u8 *attempt, u8 *attemptMax)
{
    return AdvanceRoutineStep(state, slot, ctx, trace, attempt, attemptMax);
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

// returning: NULL to advance a trainer back from Away at once; otherwise
// the trainers who came back, to advance in the heartbeat's sliced phase
// (set dwelling at home with no dwell left, which starts the advance).
static u32 ApplyDerived(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx,
                        struct WayfarerWorldTrace *trace, bool8 enterOnly, u32 *returning)
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
        // Back from Away onto a full home map: stay away until there is
        // room, so the return never overfills it (capacity on the move).
        if (want == WORLD_DERIVED_NONE && (current == WORLD_STATE_AWAY_LEAGUE || current == WORLD_STATE_AWAY_PARTNER)
         && IsMapFull(state, Trainer(slot)->homeNode, slot))
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
            if (returning != NULL)
            {
                record->dwell = 0;
                *returning |= 1u << slot;
            }
            else
            {
                WorldSim_AdvanceRoutine(state, slot, ctx, trace);
            }
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
    return ApplyDerived(state, ctx, trace, FALSE, NULL);
}

// Load: a trainer who has gone away (or is home-locked) leaves the world at
// once, so the restored map never shows them. Coming back is a routine step
// and waits for the next heartbeat, exactly as without the reload.
void WorldSim_ApplyDerivedOnLoad(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx)
{
    ApplyDerived(state, ctx, NULL, TRUE, NULL);
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

bool8 WorldSim_HopAllowed(const struct WayfarerWorldState *state, u8 slot, u16 edgeIndex)
{
    if (edgeIndex >= gWayfarerWorldEdgeCount)
        return FALSE;
    return HopAllowed(state, slot, edgeIndex, WorldSim_DestNode(state, slot));
}

// A travelling trainer's move with its path's first edge: arrive, hop, or
// wait one heartbeat when the next map is full. TRUE when that settles it;
// FALSE when the trainer waited before and must route around every full map
// (TravelGatherFullMaps, a search, then TravelRerouted).
static bool8 TravelStart(struct WayfarerWorldState *state, u8 slot, u16 firstEdge, struct WayfarerWorldTrace *trace)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    u16 destNode = WorldSim_DestNode(state, slot);

    if (destNode == WORLD_NODE_NONE || destNode == record->node)
    {
        WorldSim_Arrive(state, slot);
        return TRUE;
    }
    if (firstEdge == EDGE_NONE)
    {
        // Unreachable within the bound: give the step up.
        return TRUE;
    }
    if (!record->waited)
    {
        if (HopAllowed(state, slot, firstEdge, destNode))
        {
            Hop(state, slot, firstEdge, trace);
            return TRUE;
        }
        // Blocked: wait one heartbeat.
        record->waited = TRUE;
        if (trace != NULL)
            trace->waits++;
        return TRUE;
    }
    return FALSE;
}

// Blocked before: the maps to route around, from the other trainers in
// [from, to), appended to fullMaps[*fullCount]. The waited bit stays set
// across rerouted hops, so the next heartbeat keeps avoiding the full map
// instead of heading straight back to it.
static void TravelGatherFullMaps(const struct WayfarerWorldState *state, u8 slot, u16 *fullMaps, u8 *fullCount,
                                 u8 from, u8 to)
{
    u16 destNode = WorldSim_DestNode(state, slot);
    u8 other;

    // Occupancy questions while no record changes: answer them from the
    // per-choice summary, as spot choice does, not 25 records each.
    sOccupancySlot = slot;
    sOccupancyActive = TRUE;
    sOccupancyReady = FALSE;
    sOccupancyCached = 0;
    for (other = from; other < to; other++)
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
            if (nodes[i] == WORLD_NODE_NONE || map == WorldSim_NodeMap(destNode) || MapListed(fullMaps, *fullCount, map))
                continue;
            if (IsMapFull(state, nodes[i], slot))
                fullMaps[(*fullCount)++] = map;
        }
    }
    sOccupancyActive = FALSE;
}

// The reroute search (around the full maps) has finished: hop if it found a
// way, else wait again.
static void TravelRerouted(struct WayfarerWorldState *state, u8 slot, u16 firstEdge, const struct Search *search,
                           struct WayfarerWorldTrace *trace)
{
    struct WayfarerWorldRecord *record = &state->records[slot];
    u16 destNode = WorldSim_DestNode(state, slot);
    u16 rerouted = SearchFirstEdge(search, destNode);

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

// The heartbeat runs in two parts so the engine can spread it over frames:
// Begin applies the derived states and fixes the priority order; each Step
// then acts for the trainers in that order. A trainer's first edge is found
// just before it acts. That gives the same result as finding every first
// edge up front: it depends only on the trainer's own record, its
// destination and its own path cache entry, and acting for one trainer
// changes only that trainer's record and cache entry.
void WorldSim_HeartbeatBegin(struct WayfarerWorldHeartbeat *hb, struct WayfarerWorldState *state,
                             const struct WayfarerWorldContext *ctx, struct WayfarerWorldTrace *trace)
{
    u8 homeHops[WORLD_SIM_TRAINER_COUNT];
    u8 slot, count = 0, i, first;
    u32 returning = 0;
    // A trainer whose derived state changed has acted this heartbeat; one
    // back from Away still picks their next step, first, in the sliced
    // phase (one spot choice per unit, as every routine advance).
    u32 changed = ApplyDerived(state, ctx, trace, FALSE, &returning);

    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (returning & (1u << slot))
        {
            homeHops[slot] = 0;
            hb->order[count++] = slot;
        }
    }
    first = count;
    // Priority order: fewer hops home first (the generated table), then
    // catalog order. Trainers on the player's map or frozen by a local
    // actor stay put; both are fixed here, as the heartbeat starts.
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldRecord *record = &state->records[slot];

        if (!WorldSim_IsSimulated(record) || IsOnPlayerMap(state, slot, ctx) || ((ctx->frozenMask | changed) & (1u << slot)))
            continue;
        homeHops[slot] = HomeHopsFrom(slot, record->node);
        for (i = count; i > first && homeHops[hb->order[i - 1]] > homeHops[slot]; i--)
            hb->order[i] = hb->order[i - 1];
        hb->order[i] = slot;
        count++;
    }
    hb->worldProgress = ctx->worldProgress;
    hb->provisionalMask = ctx->provisionalMask;
    hb->risingMask = ctx->risingMask;
    hb->count = count;
    hb->next = 0;
    hb->phase = HB_PHASE_START;
    hb->firstEdge = EDGE_NONE;
    hb->active = TRUE;
    hb->clean = FALSE;
    hb->searchLive = FALSE;
}

// The workspace was lost (the engine's heap reset): a search under way
// starts again from scratch on the next one. Searches are pure, so the
// trainer's result is unchanged; trainers that already acted stay done.
void WorldSim_HeartbeatLostWorkspace(struct WayfarerWorldHeartbeat *hb)
{
    hb->clean = FALSE;
    hb->searchLive = FALSE;
}

bool8 WorldSim_HeartbeatIsPending(const struct WayfarerWorldHeartbeat *hb, u8 slot)
{
    u8 i;
    if (!hb->active)
        return FALSE;
    for (i = hb->next; i < hb->count; i++)
    {
        if (hb->order[i] == slot)
            return TRUE;
    }
    return FALSE;
}

// The unit that can take a large part of a frame on the GBA: one spot
// choice of a routine advance (about 20 scanlines on average, over 100 when
// it scans a long candidate list). Everything else takes a few.
static bool8 IsHeavyUnit(const struct WayfarerWorldHeartbeat *hb)
{
    return hb->active && hb->next < hb->count && hb->phase == HB_PHASE_ADVANCE;
}

bool8 WorldSim_HeartbeatNextIsHeavy(const struct WayfarerWorldHeartbeat *hb)
{
    return IsHeavyUnit(hb);
}

static void StartAdvance(struct WayfarerWorldHeartbeat *hb)
{
    hb->phase = HB_PHASE_ADVANCE;
    hb->attempt = 0;
    hb->attemptMax = 0;
}

static void NextTrainer(struct WayfarerWorldHeartbeat *hb)
{
    hb->next++;
    hb->phase = HB_PHASE_START;
    hb->firstEdge = EDGE_NONE;
}

// A finished heartbeat search: put its marks back so the next one needn't
// clear the whole workspace.
static void EndSearch(struct WayfarerWorldHeartbeat *hb, struct Search *search)
{
    SearchRelease(search);
    hb->clean = TRUE;
    hb->searchLive = FALSE;
}

bool8 WorldSim_HeartbeatStep(struct WayfarerWorldHeartbeat *hb, struct WayfarerWorldState *state, void *workspace,
                             u16 budget, struct WayfarerWorldTrace *trace)
{
    struct WayfarerWorldContext ctx = {0};
    u16 spent = 0;

    if (!hb->active)
        return TRUE;
    // Acting needs only these; the rest of the context did its work at Begin.
    ctx.worldProgress = hb->worldProgress;
    ctx.provisionalMask = hb->provisionalMask;
    ctx.risingMask = hb->risingMask;
    if (budget == 0)
        budget = 1;

    while (hb->next < hb->count)
    {
        u8 slot = hb->order[hb->next];
        struct WayfarerWorldRecord *record = &state->records[slot];
        struct Search *search;
        u16 destNode;

        // A heavy unit only ever starts a call, so a caller that stops at
        // one (WorldSim_HeartbeatNextIsHeavy) begins its next slice with it.
        if (spent >= budget || (spent != 0 && IsHeavyUnit(hb)))
            return FALSE;
        switch (hb->phase)
        {
        case HB_PHASE_START:
        default:
            spent++;
            if (record->state == WORLD_STATE_DWELLING)
            {
                if (record->dwell > 0)
                    record->dwell--;
                if (record->dwell == 0)
                    StartAdvance(hb);
                else
                    NextTrainer(hb);
                break;
            }
            if (record->state != WORLD_STATE_TRAVELLING)
            {
                NextTrainer(hb);
                break;
            }
            destNode = WorldSim_DestNode(state, slot);
            hb->firstEdge = EDGE_NONE;
            hb->phase = HB_PHASE_ACT;
            if (destNode != WORLD_NODE_NONE && destNode != record->node)
            {
                // The cached path, or a new search (cached for the next hops).
                hb->firstEdge = CachedFirstEdge(slot, record->node, destNode);
                if (hb->firstEdge == EDGE_NONE)
                {
                    hb->phase = HB_PHASE_EDGE_SEARCH;
                    hb->searchLive = FALSE;
                }
            }
            break;
        case HB_PHASE_EDGE_SEARCH:
            search = WorkspaceSearch(workspace);
            destNode = WorldSim_DestNode(state, slot);
            if (!hb->searchLive)
            {
                SearchStart(search, workspace, hb->clean, record->node, destNode, WORLD_NODE_NONE,
                            Trainer(slot)->searchBound, IsTraveller(slot), NULL, 0);
                hb->clean = FALSE;
                hb->searchLive = TRUE;
                spent++;
                break;
            }
            if (!SearchRun(search, budget - spent, &spent, trace))
                return FALSE;
            hb->firstEdge = SearchFirstEdge(search, destNode);
            CachePath(slot, search, destNode);
            EndSearch(hb, search);
            hb->phase = HB_PHASE_ACT;
            break;
        case HB_PHASE_ACT:
            spent++;
            if (hb->firstEdge == EDGE_NONE && WorldSim_DestNode(state, slot) != record->node)
            {
                // No path within the bound (should not happen: the build
                // checks reachability). Pick the next step instead.
                StartAdvance(hb);
                break;
            }
            if (TravelStart(state, slot, hb->firstEdge, trace))
            {
                NextTrainer(hb);
                break;
            }
            hb->phase = HB_PHASE_REROUTE;
            hb->searchLive = FALSE;
            break;
        case HB_PHASE_REROUTE:
            search = WorkspaceSearch(workspace);
            if (!hb->searchLive)
            {
                // Gathering the full maps, a few trainers per unit. Nothing
                // changes the records meanwhile, and after a lost workspace
                // it starts over and finds the same maps.
                search->gathered = 0;
                search->avoidCount = 0;
                hb->searchLive = TRUE;
            }
            if (search->gathered < WORLD_SIM_TRAINER_COUNT)
            {
                u8 to = search->gathered + GATHER_PER_UNIT;
                if (to > WORLD_SIM_TRAINER_COUNT)
                    to = WORLD_SIM_TRAINER_COUNT;
                TravelGatherFullMaps(state, slot, WorkspaceFullMaps(workspace), &search->avoidCount, search->gathered, to);
                search->gathered = to;
                if (to == WORLD_SIM_TRAINER_COUNT)
                {
                    SearchStart(search, workspace, hb->clean, record->node, WorldSim_DestNode(state, slot), WORLD_NODE_NONE,
                                Trainer(slot)->searchBound, IsTraveller(slot), WorkspaceFullMaps(workspace), search->avoidCount);
                    hb->clean = FALSE;
                }
                spent++;
                break;
            }
            if (!SearchRun(search, budget - spent, &spent, trace))
                return FALSE;
            TravelRerouted(state, slot, hb->firstEdge, search, trace);
            EndSearch(hb, search);
            NextTrainer(hb);
            break;
        case HB_PHASE_ADVANCE:
            spent++;
            if (AdvanceRoutineStep(state, slot, &ctx, trace, &hb->attempt, &hb->attemptMax))
                NextTrainer(hb);
            break;
        }
    }
    hb->active = FALSE;
    return TRUE;
}

void WorldSim_Heartbeat(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace, struct WayfarerWorldTrace *trace)
{
    struct WayfarerWorldHeartbeat hb;

    WorldSim_HeartbeatBegin(&hb, state, ctx, trace);
    while (!WorldSim_HeartbeatStep(&hb, state, workspace, 0xFFFF, trace))
        ;
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
    WorldSim_ResetPathCache();
    state->schemaVersion = WORLD_SCHEMA_VERSION;
    state->reserved = 0;
    state->contentHash = gWayfarerWorldContentHash;
    WorldSim_ClearLocalActors(state);
    state->walkerFlags = 0;
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

// Malformed bytes (a bad empty entry, a slot out of range or listed twice)
// can only come from a corrupt save. An entry that no longer fits the saved
// location (its record isn't simulated or isn't on that map, or the tile is
// outside it) is stale: a load-time repair moved the location (the league
// run's recovery to the lobby), and the block is only a restore hint.
u8 WorldSim_CheckLocalActors(const struct WayfarerWorldState *state, u16 map, u16 width, u16 height)
{
    u32 seen = 0;
    u8 i, slot, x, y, facing, result = WORLD_LOCAL_ACTORS_OK;

    for (i = 0; i < WORLD_LOCAL_ACTOR_COUNT; i++)
    {
        if (!WorldSim_GetLocalActor(state, i, &slot, &x, &y, &facing))
        {
            const u8 *entry = &state->localActors[i * WORLD_LOCAL_ACTOR_BYTES];
            if (entry[0] != WORLD_LOCAL_ACTOR_NONE || entry[1] != 0 || entry[2] != 0)
                return WORLD_LOCAL_ACTORS_CORRUPT;
            continue;
        }
        if (slot >= WORLD_SIM_TRAINER_COUNT || (seen & (1u << slot)))
            return WORLD_LOCAL_ACTORS_CORRUPT;
        seen |= 1u << slot;
        if (!WorldSim_IsSimulated(&state->records[slot]) || WorldSim_NodeMap(state->records[slot].node) != map
         || x >= width || y >= height)
            result = WORLD_LOCAL_ACTORS_STALE;
    }
    return result;
}

bool8 WorldSim_LocalActorsValid(const struct WayfarerWorldState *state, u16 map, u16 width, u16 height)
{
    return WorldSim_CheckLocalActors(state, map, width, height) == WORLD_LOCAL_ACTORS_OK;
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

u8 WorldSim_HomeHops(const struct WayfarerWorldState *state, u8 slot)
{
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
    state->padding[0] = state->padding[1] = 0;
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
