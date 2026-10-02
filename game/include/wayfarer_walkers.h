#ifndef GUARD_WAYFARER_WALKERS_H
#define GUARD_WAYFARER_WALKERS_H

// Local actors for the notable world simulation: the on-screen walkers on
// the player's map, the handoffs between them and the saved world records,
// the shared grid search, and the spots' behaviour templates.
// Spec: .product/specs/notable-world-simulation.md ("Two layers", "Handoff",
// "Local actor budget") and notable-spots.md ("Behaviour templates").

#include "global.h"
#include "constants/wayfarer_world.h"

#if IS_WAYFARER

// Up to the outdoor capacity of local actors, plus one actor still walking
// out of view in a connection strip.
#define WALKER_ACTOR_COUNT      (WORLD_LOCAL_ACTOR_COUNT + 1)
// Runtime objects use dynamic local ids (looked up without a map), clear of
// map templates (1-64), the link players (0xF0-0xF4) and the followers.
#define WALKER_LOCALID_BASE     0xF5

enum WalkerMode
{
    WALKER_MODE_NONE,
    WALKER_MODE_LOCAL,  // on the player's map; the record follows it
    WALKER_MODE_STRIP,  // in a loaded connection strip, walking out of view
    WALKER_MODE_LEAVING,// handed off: walking to the nearest exit, then removed
};

enum WalkerPhase
{
    WALKER_PHASE_IDLE,      // standing still (template pause, unreachable goal)
    WALKER_PHASE_PLAN,      // needs a goal
    WALKER_PHASE_QUEUED,    // waiting for the shared grid search
    WALKER_PHASE_SEARCH,    // owns the grid search
    WALKER_PHASE_WALK,      // following a path
    WALKER_PHASE_TEMPLATE,  // at the spot, playing its template
    WALKER_PHASE_EXIT,      // stepping out through an edge or a warp
};

enum WalkerGoal
{
    WALKER_GOAL_NONE,
    WALKER_GOAL_TILE,   // a spot's anchor or a template tile
    WALKER_GOAL_EDGE,   // a lane tile on a map side, then a step out
    WALKER_GOAL_WARP,   // beside a warp (door, stairs, transit), then a step in
    WALKER_GOAL_AWAY,   // backing off: an open tile away from the player
};

// Exported for the SkyEmu verifier (tools/wayfarer_walkers/verify.py); keep
// the layout in sync with it. 16 bytes.
struct WayfarerWalkerActorDebug
{
    u8 slot;        // world slot, 0xFF when unused
    u8 mode;        // enum WalkerMode
    u8 phase;       // enum WalkerPhase
    u8 goalKind;    // enum WalkerGoal
    u8 objectId;    // gObjectEvents index
    u8 template;    // enum WorldTemplate
    u8 atSpot;
    u8 blocked;
    s16 x;          // map tile, no MAP_OFFSET
    s16 y;
    u8 goalX;
    u8 goalY;
    u16 goalEdge;
};

struct WayfarerWalkersDebug
{
    u32 frames;
    u16 spawns;
    u16 removals;
    u16 edgeExits;
    u16 warpExits;
    u16 arrivals;
    u16 rebases;            // actors kept across a seam (no respawn)
    u16 stripEntries;       // strip actors walking into the player's map
    u16 replans;
    u16 blockedSteps;
    u16 searches;
    u16 searchFails;
    u16 localDwellBeats;    // heartbeats of dwell run down while watched
    u16 localAdvances;      // routine advances while watched
    u16 emotes;
    u16 templateMoves;
    u16 floorChanges;
    u16 heapResets;
    u16 restores;           // actors spawned from the local actor block
    u16 lastSearchNodes;
    u16 maxSearchNodes;
    u16 lastSearchFrames;
    u16 maxSearchFrames;
    u16 maxSliceScanlines;  // worst search slice, VCOUNT-based
    u16 maxSliceNodes;
    u16 workspaceBytes;     // shared heap workspace
    u16 currentMap;
    u8 followerHidden;
    u8 activeActors;
    u16 storySuppressed;
    u32 worldState;         // address of the saved WayfarerWorldState (it moves on heap resets)
    struct WayfarerWalkerActorDebug actors[WALKER_ACTOR_COUNT];
    u32 lastHeartbeatScanlines; // the last heartbeat's work over all its frames, VCOUNT-based
    u32 maxHeartbeatScanlines;
    u32 maxSpawnScanlines;      // one TrySpawn
    u32 maxUpdateScanlines;     // one whole WayfarerWalkers_Update
    u16 backOffs;               // walkers that made room for the player
    u16 handoffs;               // walkers that left for the rest of a visit
    u16 visitorsVanished;       // Gym visitors that left without walking out
    u16 culls;                  // off-screen walkers that gave their slot back
    u16 lastContextScanlines;   // the heartbeat's WayfarerWorld_BuildContext
    u16 lastFailNodes;          // nodes a failed search expanded
    u16 maxFinishScanlines;     // the path rebuild at a search's end
    u16 walkOffs;               // handed-off walkers that walked out of view or to an exit
    u16 worldJobs;              // routine advances and trip searches spread over frames
    u16 maxJobScanlines;        // worst frame of that work
    u16 capacityWaits;          // exits put off because the next map was full
    u16 stripTimeouts;          // strip actors removed after standing blocked
};

extern struct WayfarerWalkersDebug gWayfarerWalkersDebug;

// Hooks.
void WayfarerWalkers_Update(void);                  // OverworldBasic, once per field frame
void WayfarerWalkers_OnWarp(void);                  // LoadMapFromWarp, before the heartbeat
void WayfarerWalkers_OnCameraTransition(void);      // LoadMapFromCameraTransition, before the heartbeat
void WayfarerWalkers_OnContinue(void);              // Continue: restore the local actor block once
void WayfarerWalkers_Reset(void);                   // New Game
void WayfarerWalkers_OnHeapReset(void);             // InitHeap(gHeap)
// Completes the walkers' sliced world writes now (a map load, a save, the
// league hook): a routine advance under way must not span them.
void WayfarerWalkers_FlushWorldJobs(void);
bool8 WayfarerWalkers_IsActorObject(const struct ObjectEvent *objectEvent);
bool8 WayfarerWalkers_HideTemplate(const struct ObjectEventTemplate *template);
bool8 WayfarerWalkers_HideFollower(void);
// Profiling: the heartbeat's cost, measured by wayfarer_world.c.
u32 WayfarerWalkers_ScanlineStamp(void);
u32 WayfarerWalkers_ScanlinesSince(u32 start);
void WayfarerWalkers_NoteHeartbeat(u32 scanlines, u32 contextScanlines);
// Scanlines the last WayfarerWalkers_Update took (the heartbeat's steps share the frame).
u32 WayfarerWalkers_LastUpdateScanlines(void);

#endif // IS_WAYFARER
#endif // GUARD_WAYFARER_WALKERS_H
