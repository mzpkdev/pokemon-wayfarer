#ifndef GUARD_WAYFARER_WORLD_SIM_H
#define GUARD_WAYFARER_WORLD_SIM_H

// Off-screen world simulation core: routines, spot choice, travel and
// capacity over the generated tables. It has no engine dependencies; the
// engine layer (wayfarer_world.c) and the offline report tool supply the
// inputs through struct WayfarerWorldContext.

#include "wayfarer_world_data.h"

#define WORLD_DERIVED_NONE 0xFF  // context.derived[]: the trainer is Simulated

// The seam to league state, the Masters partner, haunts, momentum and the
// local actor. Recomputed by the caller before every call that takes it.
struct WayfarerWorldContext
{
    u16 playerMap;                          // MAP_* the player is on (or MAP_UNDEFINED)
    u32 worldProgress;                      // GetTrainerRating()
    u8 derived[WORLD_SIM_TRAINER_COUNT];    // WORLD_STATE_AWAY_*/PINNED/HOME_LOCKED or WORLD_DERIVED_NONE
    u32 provisionalMask;                    // in the provisional lineup of a waiting invitation
    u32 risingMask;                         // momentum rising
    u32 frozenMask;                         // has a local actor or is visible in a connection strip
};

enum WorldSkipReason
{
    WORLD_SKIP_RADIUS,  // no spot of the step's kinds within the radius
    WORLD_SKIP_ALOOF,   // only public spots, closed to an aloof trainer
    WORLD_SKIP_FULL,    // every fitting spot is taken or on a full map
};

// Optional instrumentation, used by the offline report and the tests.
struct WayfarerWorldTrace
{
    u32 searchNodes;        // nodes expanded by every search
    u16 searchNodesMax;     // largest single search
    u16 searches;
    u16 hops;
    u16 waits;
    u16 reroutes;
    u16 advances;
    u16 skips;
    void (*onSkip)(void *user, u8 slot, u8 activity, u8 reason);
    void *user;
};

// Bytes of scratch memory a search needs; the caller allocates it.
u32 WorldSim_WorkspaceSize(void);

void WorldSim_NewGame(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace);
// Content hash changed: re-seat as New Game does, keeping cycle steps and life events.
void WorldSim_Reseat(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace);
// Load checks other than the local actor tiles (which need map dimensions).
// Without a workspace, the destination reachability check is skipped.
bool8 WorldSim_IsValid(const struct WayfarerWorldState *state, void *workspace);
// Local actor block checks against the saved map and its size in tiles:
// WORLD_LOCAL_ACTORS_OK, _STALE (entries that no longer fit the location:
// clear the block) or _CORRUPT (malformed bytes).
u8 WorldSim_CheckLocalActors(const struct WayfarerWorldState *state, u16 map, u16 width, u16 height);
bool8 WorldSim_LocalActorsValid(const struct WayfarerWorldState *state, u16 map, u16 width, u16 height);  // == OK
// Step 1 of a heartbeat. Returns the slots it changed.
u32 WorldSim_ApplyDerived(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace, struct WayfarerWorldTrace *trace);
// On load: only entering a derived state applies; leaving one waits for the
// next heartbeat, so a reload never changes how the world evolves.
void WorldSim_ApplyDerivedOnLoad(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx);
// The map-change heartbeat, all at once: WorldSim_HeartbeatBegin, then
// WorldSim_HeartbeatStep until it returns TRUE.
void WorldSim_Heartbeat(struct WayfarerWorldState *state, const struct WayfarerWorldContext *ctx, void *workspace, struct WayfarerWorldTrace *trace);

// The same heartbeat spread over calls (the engine runs a few steps per
// frame). The caller keeps this between calls; it holds no pointers, so it
// survives a heap reset that loses the workspace.
struct WayfarerWorldHeartbeat
{
    u32 worldProgress;                      // the context's, as at Begin
    u32 provisionalMask;
    u32 risingMask;
    u16 firstEdge;                          // the current trainer's, once known
    u8 order[WORLD_SIM_TRAINER_COUNT];      // trainers left to act, in priority order
    u8 count;
    u8 next;                                // index in order of the trainer acting now
    u8 phase;
    u8 attempt;                             // a routine advance's spot choices so far
    u8 attemptMax;
    bool8 active;                           // Begin ran and some trainer hasn't acted yet
    bool8 clean;                            // the workspace's search marks are clear
    bool8 searchLive;                       // a search for the current trainer is under way in the workspace
};

// Applies the derived states and fixes who acts, in which order. The frozen
// mask and the player's map count as they are now.
void WorldSim_HeartbeatBegin(struct WayfarerWorldHeartbeat *hb, struct WayfarerWorldState *state,
                             const struct WayfarerWorldContext *ctx, struct WayfarerWorldTrace *trace);
// Acts for trainers in order, spending about `budget` work units (always at
// least one). A unit is one node a search expands, a search's start, a few
// trainers of a reroute's full-map check, a dwell tick, a spot choice or a
// hop. A heavy unit (WorldSim_HeartbeatNextIsHeavy) is only ever the first
// of a call. TRUE once every trainer has acted. The workspace must be
// WorldSim_WorkspaceSize() bytes; it may change between calls only after
// WorldSim_HeartbeatLostWorkspace.
bool8 WorldSim_HeartbeatStep(struct WayfarerWorldHeartbeat *hb, struct WayfarerWorldState *state, void *workspace,
                             u16 budget, struct WayfarerWorldTrace *trace);
// The next unit may take a large part of a frame (one spot choice of a
// routine advance): a caller slicing by time starts its next slice with it.
bool8 WorldSim_HeartbeatNextIsHeavy(const struct WayfarerWorldHeartbeat *hb);
// The workspace is gone (heap reset): the search under way restarts on the next step.
void WorldSim_HeartbeatLostWorkspace(struct WayfarerWorldHeartbeat *hb);
// TRUE while the trainer in this slot still has to act in this heartbeat.
bool8 WorldSim_HeartbeatIsPending(const struct WayfarerWorldHeartbeat *hb, u8 slot);
// The life-event hook at league event resolution. lineup is in battle order;
// championId is a NOTABLE_TRAINER_* or 0 when the player won.
void WorldSim_OnLeagueResolved(struct WayfarerWorldState *state, const u16 *lineup, u8 count, bool8 playerWon, u16 championId);

// A dwell ran out (locally while watched): advance the routine now.
void WorldSim_AdvanceRoutine(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx, struct WayfarerWorldTrace *trace);
// The same advance, one spot choice per call (*attempt and *attemptMax start
// at 0); TRUE once the step is settled. Only the trainer's own record may
// change between calls.
bool8 WorldSim_AdvanceRoutineStep(struct WayfarerWorldState *state, u8 slot, const struct WayfarerWorldContext *ctx,
                                  struct WayfarerWorldTrace *trace, u8 *attempt, u8 *attemptMax);
// The first edge of the shortest path from the trainer's node to their
// destination, or 0xFFFF. Used by the local actor to walk to the next hop.
u16 WorldSim_NextEdge(const struct WayfarerWorldState *state, u8 slot, void *workspace, struct WayfarerWorldTrace *trace);
// The same in slices: Begin answers at once (TRUE, *edge set) from the cached
// path or when there is no trip; otherwise it starts a search in the
// workspace (clean: its marks are clear; NULL only asks the cache and
// returns FALSE) and Run expands up to `budget` nodes
// per call, TRUE once *edge is known. The result is NextEdge's. Restart with
// Begin if the trainer's node or destination changes or the workspace is lost.
bool8 WorldSim_NextEdgeBegin(const struct WayfarerWorldState *state, u8 slot, void *workspace, bool8 clean, u16 *edge);
bool8 WorldSim_NextEdgeRun(const struct WayfarerWorldState *state, u8 slot, void *workspace, u16 budget, u16 *edge,
                           struct WayfarerWorldTrace *trace);
// The off-screen hop rule for one edge: into the destination's map always,
// elsewhere only while that map is under its cap. A local actor checks it
// before it commits an exit (TakeEdge).
bool8 WorldSim_HopAllowed(const struct WayfarerWorldState *state, u8 slot, u16 edgeIndex);
// Forget every cached travel path (New Game and the engine's load check do
// this; the cache only saves searches, it never changes a result).
void WorldSim_ResetPathCache(void);
// Move a record along an edge (a local actor left the map, or a heartbeat hop).
void WorldSim_TakeEdge(struct WayfarerWorldState *state, u8 slot, u16 edgeIndex, u8 crossing);
// A local actor reached its spot.
void WorldSim_Arrive(struct WayfarerWorldState *state, u8 slot);

// Queries.
u16 WorldSim_DestNode(const struct WayfarerWorldState *state, u8 slot);
u16 WorldSim_NodeMap(u16 node);
bool8 WorldSim_IsSimulated(const struct WayfarerWorldRecord *record);
u8 WorldSim_DwellFor(u8 activity);
u8 WorldSim_MapCapacity(u16 node);
u8 WorldSim_Occupancy(const struct WayfarerWorldState *state, u16 map, u8 exceptSlot);
u8 WorldSim_SlotForCharacter(u16 characterId);  // 0xFF when not simulated
u8 WorldSim_ArrivalForEdge(u8 edgeKind);
u8 WorldSim_DefaultTemplate(const struct WayfarerWorldSpot *spot, u8 activity);
u8 WorldSim_TemplateEmote(const struct WayfarerWorldSpot *spot, u8 activity);

// Local actor helpers (wayfarer_walkers.c). All engine-free and deterministic.
// Graph hops from the trainer's node to their home node (priority order), or 0xFF.
u8 WorldSim_HomeHops(const struct WayfarerWorldState *state, u8 slot);
// TRUE when the spot already holds as many trainers as it can (except exceptSlot).
bool8 WorldSim_SpotTaken(const struct WayfarerWorldState *state, u16 spot, u8 exceptSlot);
// A template moves the stay to another spot of the same kind (Browse): keeps
// the activity and dwell; travelling when the spot is on another node.
void WorldSim_ChangeSpot(struct WayfarerWorldState *state, u8 slot, u16 spot);
// A map-side edge's crossing on the target map for a step out at coord (a..b).
u8 WorldSim_LaneCrossing(const struct WayfarerWorldEdge *edge, u8 coord);
// Behaviour template choice: entry (c + 7k) mod n, with the stride moved to
// the next prime that doesn't divide n when 7 does.
u16 WorldSim_TemplateIndex(u8 c, u16 k, u16 n);
// Emote cadence: the dwell ticks t where (c + t) mod 8 == 0.
bool8 WorldSim_IsEmoteTick(u8 c, u16 t);

// Local actor block helpers. Clearing the block keeps walkerFlags.
void WorldSim_ClearLocalActors(struct WayfarerWorldState *state);
bool8 WorldSim_GetLocalActor(const struct WayfarerWorldState *state, u8 index, u8 *slot, u8 *x, u8 *y, u8 *facing);
void WorldSim_SetLocalActor(struct WayfarerWorldState *state, u8 index, u8 slot, u8 x, u8 y, u8 facing);

#endif // GUARD_WAYFARER_WORLD_SIM_H
