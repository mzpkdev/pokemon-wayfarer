#include "global.h"
#include "viridian_walker_world.h"

#if IS_WAYFARER && VIRIDIAN_WALKER_POC

#include "overworld.h"
#include "viridian_walker_poc.h"

#define WORLD_INTERIOR_DWELL_TICKS 150 // Two rendered frames per tick, five seconds.
#define WORLD_HEARTBEAT_DWELL_TICKS 75 // Two off-map transitions approximate that stop.

STATIC_ASSERT(sizeof(struct ViridianWalkerWorldState) == 12, ViridianWalkerWorldSaveSize);

const volatile u16 gViridianWalkerWorldSaveOffset = offsetof(struct SaveBlock3, viridianWalkerWorld);
const volatile u16 gViridianWalkerWorldSaveSize = sizeof(struct ViridianWalkerWorldState);
EWRAM_DATA struct ViridianWalkerWorldDebug gViridianWalkerWorldDebug = {0};
// Zero means no previous load; real map IDs are stored with one added.
static EWRAM_DATA u16 sPreviousPlayerMapPlusOne = 0;

static struct ViridianWalkerWorldState *GetMutable(void)
{
    return (struct ViridianWalkerWorldState *)((u8 *)gSaveBlock3Ptr + gViridianWalkerWorldSaveOffset);
}

const struct ViridianWalkerWorldState *ViridianWorld_Get(void)
{
    return GetMutable();
}

static u16 GetPlayerMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | (u8)gSaveBlock1Ptr->location.mapNum;
}

bool8 ViridianWorld_IsOnPlayerMap(void)
{
    const struct ViridianWalkerWorldState *world = ViridianWorld_Get();
    return world->currentMap == GetPlayerMap();
}

static bool8 IsInteriorMap(u16 map)
{
    return map == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS
        || map == MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS
        || map == MAP_VIRIDIAN_CITY_MART_HNS;
}

static void SetPosition(u16 map, u8 arrival, u8 crossing, u8 x, u8 y)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    gViridianWalkerWorldDebug.fromMap = world->currentMap;
    gViridianWalkerWorldDebug.toMap = map;
    gViridianWalkerWorldDebug.lastArrival = arrival;
    gViridianWalkerWorldDebug.lastCrossing = crossing;
    gViridianWalkerWorldDebug.lastX = x;
    gViridianWalkerWorldDebug.lastY = y;
    world->currentMap = map;
    world->arrival = arrival;
    world->crossing = crossing;
    world->x = x;
    world->y = y;
}

static void AdvanceDestination(void)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (world->currentMap == world->destinationMap)
    {
        if (world->destinationMap == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS)
            world->destinationMap = MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS;
        else if (world->destinationMap == MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS)
            world->destinationMap = MAP_VIRIDIAN_CITY_MART_HNS;
        else if (world->destinationMap == MAP_VIRIDIAN_CITY_MART_HNS)
            world->destinationMap = MAP_ROUTE1_HNS;
        else
            world->destinationMap = MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS;
    }
}

void ViridianWorld_InitNewGame(void)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    memset(world, 0, gViridianWalkerWorldSaveSize);
    memset(&gViridianWalkerWorldDebug, 0, sizeof(gViridianWalkerWorldDebug));
    sPreviousPlayerMapPlusOne = 0;
    world->currentMap = MAP_VIRIDIAN_CITY_HNS;
    world->destinationMap = MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS;
    world->arrival = WORLD_ARRIVAL_NONE;
    world->state = WORLD_STATE_AT_SPOT;
    world->x = 32;
    world->y = 37;
    world->dwell = 0;
}

void ViridianWorld_OnContinue(bool8 loadsWarp)
{
    // Continue rebuilds the saved map without going through LoadMapFromWarp.
    // A continue-game warp does use that loader, but is still not travel.
    sPreviousPlayerMapPlusOne = loadsWarp ? 0 : GetPlayerMap() + 1;
}

void ViridianWorld_ActorMoved(u8 x, u8 y, u8 goal)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (!ViridianWorld_IsOnPlayerMap())
        return;
    world->x = x;
    world->y = y;
    world->goal = goal;
    if (IsInteriorMap(world->currentMap)
     && (goal == WALKER_GOAL_GATE_SPOT || goal == WALKER_GOAL_CENTER_SPOT
      || goal == WALKER_GOAL_MART_SPOT))
        return;
    world->state = WORLD_STATE_TRAVELLING;
    world->dwell = 0;
}

void ViridianWorld_ActorMovedAcrossSeam(u8 x, u8 y, u8 goal)
{
    // The actor remains visible in the loaded connection strip while its
    // authoritative map is already the adjoining map.
    struct ViridianWalkerWorldState *world = GetMutable();
    world->x = x;
    world->y = y;
    world->goal = goal;
    world->state = WORLD_STATE_TRAVELLING;
    world->dwell = 0;
}

void ViridianWorld_ActorEnteredPlayerMapFromProxy(u16 toMap, u8 arrival, u8 crossing, u8 x, u8 y)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (toMap != GetPlayerMap() || world->currentMap == toMap)
        return;
    SetPosition(toMap, arrival, crossing, x, y);
    world->state = WORLD_STATE_TRAVELLING;
    world->dwell = 0;
    AdvanceDestination();
    gViridianWalkerWorldDebug.actorExits++;
}

void ViridianWorld_ActorAtSpot(u8 x, u8 y, u8 goal)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (!ViridianWorld_IsOnPlayerMap())
        return;
    world->x = x;
    world->y = y;
    world->goal = goal;
    world->state = WORLD_STATE_AT_SPOT;
    world->dwell = 1;
}

void ViridianWorld_ActorAtInteriorSpot(u8 x, u8 y, u8 goal)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (!ViridianWorld_IsOnPlayerMap() || !IsInteriorMap(world->currentMap))
        return;
    world->x = x;
    world->y = y;
    world->goal = goal;
    world->state = WORLD_STATE_AT_SPOT;
    world->dwell = WORLD_INTERIOR_DWELL_TICKS;
}

void ViridianWorld_InteriorTick(void)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (ViridianWorld_IsOnPlayerMap() && IsInteriorMap(world->currentMap)
     && world->state == WORLD_STATE_AT_SPOT && world->dwell > 0)
        world->dwell--;
}

void ViridianWorld_ActorCrossedExit(u16 toMap, u8 arrival, u8 crossing, u8 x, u8 y)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (!ViridianWorld_IsOnPlayerMap())
        return;
    SetPosition(toMap, arrival, crossing, x, y);
    world->state = WORLD_STATE_TRAVELLING;
    world->dwell = 0;
    AdvanceDestination();
    gViridianWalkerWorldDebug.actorExits++;
}

void ViridianWorld_ActorInside(u16 interiorMap)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    u8 x, y;
    if (!ViridianWorld_IsOnPlayerMap())
        return;
    if (interiorMap == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS)
    {
        x = 7;
        y = 9;
    }
    else if (interiorMap == MAP_VIRIDIAN_CITY_MART_HNS)
    {
        x = 4;
        y = 7;
    }
    else if (interiorMap == MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS)
    {
        x = 7;
        y = 8;
    }
    else
        return;
    SetPosition(interiorMap, WORLD_ARRIVAL_DOOR, world->x, x, y);
    world->state = WORLD_STATE_INSIDE;
    world->dwell = WORLD_INTERIOR_DWELL_TICKS;
    world->goal = interiorMap == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS ? WALKER_GOAL_GATE_SPOT
        : interiorMap == MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS ? WALKER_GOAL_CENTER_SPOT
        : WALKER_GOAL_MART_SPOT;
    AdvanceDestination();
    gViridianWalkerWorldDebug.actorExits++;
}

void ViridianWorld_ActorReturnedFromDoor(u16 outdoorMap, u8 x, u8 y)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (!ViridianWorld_IsOnPlayerMap() || !IsInteriorMap(world->currentMap))
        return;
    SetPosition(outdoorMap, WORLD_ARRIVAL_DOOR, world->crossing, x, y);
    world->state = WORLD_STATE_TRAVELLING;
    world->dwell = 0;
    gViridianWalkerWorldDebug.actorExits++;
}

// These are the walkable south-side connections. Route 2's northern Pewter
// edge is disconnected by solid rows, so the Forest south gate is the third node.
static void HopOneMap(void)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    switch (world->currentMap)
    {
    case MAP_VIRIDIAN_CITY_HNS:
        if (world->destinationMap == MAP_ROUTE1_HNS)
            SetPosition(MAP_ROUTE1_HNS, WORLD_ARRIVAL_NORTH, 25, 23, 0);
        else if (world->destinationMap == MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS)
        {
            SetPosition(MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS, WORLD_ARRIVAL_DOOR, 30, 7, 8);
            world->goal = WALKER_GOAL_CENTER_SPOT;
            world->state = WORLD_STATE_INSIDE;
            world->dwell = WORLD_INTERIOR_DWELL_TICKS;
        }
        else if (world->destinationMap == MAP_VIRIDIAN_CITY_MART_HNS)
        {
            SetPosition(MAP_VIRIDIAN_CITY_MART_HNS, WORLD_ARRIVAL_DOOR, 40, 4, 7);
            world->goal = WALKER_GOAL_MART_SPOT;
            world->state = WORLD_STATE_INSIDE;
            world->dwell = WORLD_INTERIOR_DWELL_TICKS;
        }
        else
            SetPosition(MAP_ROUTE2_HNS, WORLD_ARRIVAL_SOUTH, 24, 8, 79);
        if (!IsInteriorMap(world->currentMap))
            world->state = WORLD_STATE_TRAVELLING;
        break;
    case MAP_ROUTE2_HNS:
        if (world->destinationMap == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS)
        {
            SetPosition(MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS, WORLD_ARRIVAL_DOOR, 6, 7, 9);
            world->goal = WALKER_GOAL_GATE_SPOT;
            world->state = WORLD_STATE_INSIDE;
            world->dwell = WORLD_INTERIOR_DWELL_TICKS;
        }
        else
        {
            SetPosition(MAP_VIRIDIAN_CITY_HNS, WORLD_ARRIVAL_NORTH, 8, 24, 0);
            world->state = WORLD_STATE_TRAVELLING;
        }
        break;
    case MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS:
        SetPosition(MAP_ROUTE2_HNS, WORLD_ARRIVAL_DOOR, 7, 6, 52);
        world->state = WORLD_STATE_TRAVELLING;
        break;
    case MAP_ROUTE1_HNS:
        SetPosition(MAP_VIRIDIAN_CITY_HNS, WORLD_ARRIVAL_SOUTH, 23, 25, 49);
        world->state = WORLD_STATE_TRAVELLING;
        break;
    case MAP_VIRIDIAN_CITY_MART_HNS:
        SetPosition(MAP_VIRIDIAN_CITY_HNS, WORLD_ARRIVAL_DOOR, 40, 41, 28);
        world->state = WORLD_STATE_AT_SPOT;
        break;
    case MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS:
        SetPosition(MAP_VIRIDIAN_CITY_HNS, WORLD_ARRIVAL_DOOR, 30, 31, 37);
        world->state = WORLD_STATE_AT_SPOT;
        break;
    default:
        return;
    }
    AdvanceDestination();
    gViridianWalkerWorldDebug.hops++;
}

static void OnMapLoad(bool8 cameraTransition)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    u16 playerMap = GetPlayerMap();
    u16 previousMap = sPreviousPlayerMapPlusOne - 1;

    // The first load after boot is a new game or Continue, never a travel tick.
    if (sPreviousPlayerMapPlusOne == 0)
    {
        sPreviousPlayerMapPlusOne = playerMap + 1;
        return;
    }
    if (sPreviousPlayerMapPlusOne == playerMap + 1)
        return;
    sPreviousPlayerMapPlusOne = playerMap + 1;
    gViridianWalkerWorldDebug.heartbeats++;
    if (world->currentMap == playerMap)
        return;
    if (cameraTransition && world->currentMap == previousMap
     && ViridianWalker_HasVisibleActorOnMap(previousMap))
        return;
    if (world->dwell > 0)
    {
        world->dwell = world->dwell > WORLD_HEARTBEAT_DWELL_TICKS
            ? world->dwell - WORLD_HEARTBEAT_DWELL_TICKS : 0;
        if (world->dwell > 0)
            return;
    }
    if (world->state == WORLD_STATE_AT_SPOT || world->state == WORLD_STATE_INSIDE)
        world->state = WORLD_STATE_TRAVELLING;
    HopOneMap();
}

void ViridianWorld_OnMapLoad(void)
{
    OnMapLoad(FALSE);
}

void ViridianWorld_OnCameraTransition(void)
{
    OnMapLoad(TRUE);
}

#endif // IS_WAYFARER && VIRIDIAN_WALKER_POC
