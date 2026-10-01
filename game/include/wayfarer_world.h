#ifndef GUARD_WAYFARER_WORLD_H
#define GUARD_WAYFARER_WORLD_H

// Engine side of the notable world simulation: the heartbeat hook, New Game,
// load validation, the league resolution hook, and the context the core
// reads (wayfarer_world_sim.h).

#include "global.h"
#include "wayfarer_world_sim.h"

#if IS_WAYFARER

struct WayfarerWorldDebug
{
    u16 heartbeats;
    u16 skippedHeartbeats;   // Continue, same-map reloads, failed allocations
    u16 lastHeartbeatMap;
    u16 reseats;             // content-hash changes on load
    struct WayfarerWorldTrace lastTrace;
};

extern struct WayfarerWorldDebug gWayfarerWorldDebug;

struct WayfarerWorldState *WayfarerWorld_GetState(void);
void WayfarerWorld_BuildContext(struct WayfarerWorldContext *ctx);

void WayfarerWorld_InitNewGame(void);
// Continue: the map load that follows doesn't advance the world.
void WayfarerWorld_OnContinue(bool8 loadsWarp);
// Load validation; FALSE is an invalid save.
bool8 WayfarerWorld_OnLoad(void);
// The heartbeat: a map load from a warp or a camera transition.
void WayfarerWorld_OnMapLoad(void);
void WayfarerWorld_ForceHeartbeat(void);
// League event resolution (ResolveLeagueEvent); lineup in battle order.
void WayfarerWorld_OnLeagueResolved(const u16 *lineup, u8 count, bool8 playerWon, u16 championId);

// Set by the local actor layer: trainers with an actor or visible in a
// connection strip, whom the heartbeat leaves alone.
extern u32 gWayfarerWorldFrozenMask;

#endif // IS_WAYFARER
#endif // GUARD_WAYFARER_WORLD_H
