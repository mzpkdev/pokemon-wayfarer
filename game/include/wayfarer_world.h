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
    // The heartbeat spread over field frames (read by the SkyEmu verifier,
    // tools/wayfarer_walkers/verify.py; keep the layout in sync with it).
    u16 lastHeartbeatFrames; // field frames the last heartbeat spanned (waiting ones too)
    u16 maxHeartbeatFrames;
    u16 maxStepScanlines;    // worst frame of heartbeat work: Begin, or one frame's steps
    u16 maxFrameScanlines;   // worst frame of steps plus the walkers' update
    u16 maxFinishScanlines;  // worst rest of a heartbeat finished at once (below)
    u16 pendingAtLoad;       // map loads that finished the previous heartbeat first
    u16 forcedFinishes;      // saves, league results, Gym leader objects and Gym visitors that did
    u16 workspaceLosses;     // heap resets while a heartbeat was pending
    u16 pending;             // a heartbeat is still running
    u16 workspaceWaits;      // frames a pending heartbeat waited for heap for its workspace
    u16 deferredBegins;      // seam loads that queued their heartbeat behind a pending one
};

extern struct WayfarerWorldDebug gWayfarerWorldDebug;

struct WayfarerWorldState *WayfarerWorld_GetState(void);
void WayfarerWorld_BuildContext(struct WayfarerWorldContext *ctx);

void WayfarerWorld_InitNewGame(void);
// Continue: the map load that follows doesn't advance the world.
void WayfarerWorld_OnContinue(bool8 loadsWarp);
// Load validation; FALSE is an invalid save.
bool8 WayfarerWorld_OnLoad(void);
// The heartbeat: a map load from a warp or a camera transition starts it;
// WayfarerWorld_Update runs it a few steps per field frame.
void WayfarerWorld_OnMapLoad(bool8 seam);      // seam: a camera transition
void WayfarerWorld_Update(void);                // OverworldBasic, after WayfarerWalkers_Update
void WayfarerWorld_OnHeapReset(void);           // InitHeap(gHeap)
void WayfarerWorld_ForceHeartbeat(void);
// Anything that writes or saves the world state, or shows what a pending
// trainer's record decides, finishes the pending heartbeat first.
bool8 WayfarerWorld_IsHeartbeatPending(void);
bool8 WayfarerWorld_IsSlotPending(u8 slot);     // that trainer hasn't acted yet
void WayfarerWorld_FinishHeartbeat(void);
void WayfarerWorld_FinishHeartbeatEarly(void);  // the same, counted in forcedFinishes
// A Gym Leader whose badge the player doesn't hold yet.
bool8 WayfarerWorld_IsLeaderUnbeaten(u8 slot);
// League event resolution (ResolveLeagueEvent); lineup in battle order.
void WayfarerWorld_OnLeagueResolved(const u16 *lineup, u8 count, bool8 playerWon, u16 championId);

// Set by the local actor layer: trainers with an actor or visible in a
// connection strip, whom the heartbeat leaves alone.
extern u32 gWayfarerWorldFrozenMask;

#endif // IS_WAYFARER
#endif // GUARD_WAYFARER_WORLD_H
