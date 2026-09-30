#ifndef GUARD_VIRIDIAN_WALKER_WORLD_H
#define GUARD_VIRIDIAN_WALKER_WORLD_H

#include "config/viridian_walker_poc.h"

#if IS_WAYFARER && VIRIDIAN_WALKER_POC

// Coordinates are map-local. arrival names the edge on currentMap; crossing is
// the coordinate along the source edge before a map connection was crossed.
enum ViridianWorldArrival
{
    WORLD_ARRIVAL_NONE,
    WORLD_ARRIVAL_NORTH,
    WORLD_ARRIVAL_SOUTH,
    WORLD_ARRIVAL_EAST,
    WORLD_ARRIVAL_WEST,
    WORLD_ARRIVAL_DOOR,
};

enum ViridianWorldState
{
    WORLD_STATE_TRAVELLING,
    WORLD_STATE_AT_SPOT,
    WORLD_STATE_INSIDE,
};

struct ViridianWalkerWorldState
{
    u16 currentMap;
    u16 destinationMap;
    u8 arrival;
    u8 state;
    u8 crossing;
    u8 x;
    u8 y;
    u8 goal;
    u8 dwell;
};

// The offset and size address the actual serialized bytes in gSaveBlock3Ptr.
extern const volatile u16 gViridianWalkerWorldSaveOffset;
extern const volatile u16 gViridianWalkerWorldSaveSize;

struct ViridianWalkerWorldDebug
{
    u16 heartbeats;
    u16 hops;
    u16 actorExits;
    u16 fromMap;
    u16 toMap;
    u8 lastArrival;
    u8 lastCrossing;
    u8 lastX;
    u8 lastY;
};

extern struct ViridianWalkerWorldDebug gViridianWalkerWorldDebug;

const struct ViridianWalkerWorldState *ViridianWorld_Get(void);
bool8 ViridianWorld_IsOnPlayerMap(void);
void ViridianWorld_InitNewGame(void);
void ViridianWorld_OnContinue(bool8 loadsWarp);
void ViridianWorld_OnMapLoad(void);
void ViridianWorld_OnCameraTransition(void);
void ViridianWorld_ActorMoved(u8 x, u8 y, u8 goal);
void ViridianWorld_ActorMovedAcrossSeam(u8 x, u8 y, u8 goal);
void ViridianWorld_ActorEnteredPlayerMapFromProxy(u16 toMap, u8 arrival, u8 crossing, u8 x, u8 y);
void ViridianWorld_ActorAtSpot(u8 x, u8 y, u8 goal);
void ViridianWorld_ActorCrossedExit(u16 toMap, u8 arrival, u8 crossing, u8 x, u8 y);
void ViridianWorld_ActorInside(u16 interiorMap);
void ViridianWorld_ActorReturnedFromDoor(u16 outdoorMap, u8 x, u8 y);

#endif // IS_WAYFARER && VIRIDIAN_WALKER_POC
#endif // GUARD_VIRIDIAN_WALKER_WORLD_H
