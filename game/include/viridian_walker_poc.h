#ifndef GUARD_VIRIDIAN_WALKER_POC_H
#define GUARD_VIRIDIAN_WALKER_POC_H

#include "config/viridian_walker_poc.h"

#define VIRIDIAN_WALKER_LOCAL_ID LOCALID_VIRIDIAN_WALKER_POC

enum ViridianWalkerGoal {
    WALKER_GOAL_GRASS,
    WALKER_GOAL_MART,
    WALKER_GOAL_CENTER,
    WALKER_GOAL_ROUTE1,
    WALKER_GOAL_ROUTE2,
    WALKER_GOAL_ROUTE22,
    WALKER_GOAL_ROUTE1_SPOT = 8,
    WALKER_GOAL_ROUTE1_RETURN,
    WALKER_GOAL_ROUTE2_RETURN,
    WALKER_GOAL_ROUTE2_GATE,
};

enum ViridianWalkerPhase {
    WALKER_PHASE_INACTIVE,
    WALKER_PHASE_SEARCH,
    WALKER_PHASE_WALK,
    WALKER_PHASE_WAIT,
    WALKER_PHASE_HIDDEN,
};

// Exported for the POC's symbol based emulator checks. Coordinates include MAP_OFFSET.
struct ViridianWalkerDebug {
    u16 completed;
    u16 replans;
    u16 searches;
    u16 searchNodes;
    u16 searchFrames;
    u16 maxNodesPerFrame;
    u16 maxSliceVblanks;
    u16 grassTilesFound;
    u16 blockedSteps;
    s16 x;
    s16 y;
    s16 goalX;
    s16 goalY;
    s16 nextX;
    s16 nextY;
    u8 phase;
    u8 goal;
    u8 lastCompletedGoal;
    u8 usedGrassFallback;
    u16 maxSliceScanlines;
    u16 currentMap;
    u8 localId;
};

#if IS_WAYFARER && VIRIDIAN_WALKER_POC
extern struct ViridianWalkerDebug gViridianWalkerDebug;
void ViridianWalker_Update(void);
void ViridianWalker_OnHeapReset(void);
bool8 ViridianWalker_IsObject(const struct ObjectEvent *objectEvent);
bool8 ViridianWalker_HasVisibleActorOnMap(u16 map);
#endif

#endif
