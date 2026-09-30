#include "global.h"
#include "viridian_walker_world.h"

#if IS_WAYFARER && VIRIDIAN_WALKER_POC

#include "overworld.h"

#define WORLD_MAP_NONE 0xFFFF
#define WORLD_DOOR_DWELL 2

STATIC_ASSERT(sizeof(struct ViridianWalkerWorldState) == 12, ViridianWalkerWorldSaveSize);

const volatile u16 gViridianWalkerWorldSaveOffset = offsetof(struct SaveBlock3, viridianWalkerWorld);
const volatile u16 gViridianWalkerWorldSaveSize = sizeof(struct ViridianWalkerWorldState);
EWRAM_DATA struct ViridianWalkerWorldDebug gViridianWalkerWorldDebug = {0};
static EWRAM_DATA u16 sPreviousPlayerMap = WORLD_MAP_NONE;

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
    return world->state != WORLD_STATE_INSIDE && world->currentMap == GetPlayerMap();
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
        world->destinationMap = world->destinationMap == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS
            ? MAP_ROUTE1_HNS : MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS;
}

void ViridianWorld_InitNewGame(void)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    memset(world, 0, gViridianWalkerWorldSaveSize);
    memset(&gViridianWalkerWorldDebug, 0, sizeof(gViridianWalkerWorldDebug));
    sPreviousPlayerMap = WORLD_MAP_NONE;
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
    sPreviousPlayerMap = loadsWarp ? WORLD_MAP_NONE : GetPlayerMap();
}

void ViridianWorld_ActorMoved(u8 x, u8 y, u8 goal)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (!ViridianWorld_IsOnPlayerMap())
        return;
    world->x = x;
    world->y = y;
    world->goal = goal;
    world->state = WORLD_STATE_TRAVELLING;
    world->dwell = 0;
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
    world->dwell = WORLD_DOOR_DWELL;
    AdvanceDestination();
    gViridianWalkerWorldDebug.actorExits++;
}

void ViridianWorld_ActorReturnedFromDoor(u16 outdoorMap, u8 x, u8 y)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    if (world->state != WORLD_STATE_INSIDE)
        return;
    SetPosition(outdoorMap, WORLD_ARRIVAL_DOOR, world->crossing, x, y);
    world->state = WORLD_STATE_AT_SPOT;
    world->dwell = 1;
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
        else
            SetPosition(MAP_ROUTE2_HNS, WORLD_ARRIVAL_SOUTH, 24, 8, 79);
        world->state = WORLD_STATE_TRAVELLING;
        break;
    case MAP_ROUTE2_HNS:
        if (world->destinationMap == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS)
        {
            SetPosition(MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS, WORLD_ARRIVAL_DOOR, 6, 7, 9);
            world->state = WORLD_STATE_INSIDE;
            world->dwell = WORLD_DOOR_DWELL;
        }
        else
        {
            SetPosition(MAP_VIRIDIAN_CITY_HNS, WORLD_ARRIVAL_NORTH, 8, 24, 0);
            world->state = WORLD_STATE_TRAVELLING;
        }
        break;
    case MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS:
        SetPosition(MAP_ROUTE2_HNS, WORLD_ARRIVAL_DOOR, 7, 5, 52);
        world->state = WORLD_STATE_TRAVELLING;
        break;
    case MAP_ROUTE1_HNS:
        SetPosition(MAP_VIRIDIAN_CITY_HNS, WORLD_ARRIVAL_SOUTH, 23, 25, 49);
        world->state = WORLD_STATE_TRAVELLING;
        break;
    case MAP_VIRIDIAN_CITY_MART_HNS:
        SetPosition(MAP_VIRIDIAN_CITY_HNS, WORLD_ARRIVAL_DOOR, 40, 40, 28);
        world->state = WORLD_STATE_AT_SPOT;
        break;
    case MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS:
        SetPosition(MAP_VIRIDIAN_CITY_HNS, WORLD_ARRIVAL_DOOR, 30, 30, 37);
        world->state = WORLD_STATE_AT_SPOT;
        break;
    default:
        return;
    }
    AdvanceDestination();
    gViridianWalkerWorldDebug.hops++;
}

void ViridianWorld_OnMapLoad(void)
{
    struct ViridianWalkerWorldState *world = GetMutable();
    u16 playerMap = GetPlayerMap();

    // The first load after boot is a new game or Continue, never a travel tick.
    if (sPreviousPlayerMap == WORLD_MAP_NONE)
    {
        sPreviousPlayerMap = playerMap;
        return;
    }
    if (sPreviousPlayerMap == playerMap)
        return;
    sPreviousPlayerMap = playerMap;
    gViridianWalkerWorldDebug.heartbeats++;
    if (world->currentMap == playerMap)
        return;
    if (world->dwell > 0)
    {
        world->dwell--;
        if (world->dwell > 0)
            return;
    }
    if (world->state == WORLD_STATE_AT_SPOT || world->state == WORLD_STATE_INSIDE)
        world->state = WORLD_STATE_TRAVELLING;
    HopOneMap();
}

#endif // IS_WAYFARER && VIRIDIAN_WALKER_POC
