#ifndef GUARD_WAYFARER_AMBIENCE_H
#define GUARD_WAYFARER_AMBIENCE_H

// Notable ambience: deterministic beat selection for the on-screen walkers.
// Spec: .product/specs/notable-ambience.md ("Context", "Selection").
//
// Engine-free, like wayfarer_world_sim.c: the walker layer
// (src/wayfarer_walkers.c) gathers the context from the engine, calls these
// at its decision points and runs the chosen beat's steps. Nothing here reads
// the random number generator; the same calls give the same beats.

#include "global.h"
#include "wayfarer_ambience_data.h"

#if IS_WAYFARER

#define AMBIENCE_BEAT_NONE      0xFF
#define AMBIENCE_DISTANCE_NONE  0xFF
#define AMBIENCE_ACTOR_NONE     0xFF

// Quiet gap (spec, "Selection" step 1): 16 + 4 * (c mod 3) ticks, three
// times that for a stoic trainer. Idle odds (step 4).
#define AMBIENCE_GAP_BASE           16
#define AMBIENCE_GAP_STEP           4
#define AMBIENCE_STOIC_GAP_FACTOR   3
#define AMBIENCE_IDLE_DECISIONS     4   // (c + t) mod 4 == 0
#define AMBIENCE_IDLE_STEPS         10  // (c + s) mod 10 == 0 while walking
#define AMBIENCE_IDLE_MIN_STEPS     6   // and at least this many steps since the last beat
#define AMBIENCE_PICK_STRIDE        7   // entry (c + 7k) mod n

// What kind of decision point the walker is at.
enum AmbienceDecision
{
    AMBIENCE_DECIDE_STEP,   // a step boundary while walking (counts toward s)
    AMBIENCE_DECIDE_ARRIVE, // just reached the spot, before the template starts
    AMBIENCE_DECIDE_LEAVE,  // the stay ended, before the next goal is planned
    AMBIENCE_DECIDE_DWELL,  // a dwell tick (60 frames) at a spot
    AMBIENCE_DECIDE_REACT,  // any frame: react beats only, no counters advance
};

// Latches (the walker keeps them in its actor, so they survive a heap reset).
#define AMBIENCE_LATCH_APPROACH     (1 << 0) // a once-per-approach beat fired; cleared once out of range
#define AMBIENCE_LATCH_EPISODE      (1 << 1) // a once-per-episode beat fired; cleared once the player moves off
#define AMBIENCE_LATCH_GREETED_SHIFT 4       // bits 4..7: actor indices greeted this visit (once per pair)

// The context at a decision point, gathered by the walker layer.
struct AmbienceContext
{
    u32 facts;              // the parameterless AMBIENCE_FACT_* that hold now:
                            // WALKING .. FACING_COUNTER (bits 0..10)
    u8 spotKind;            // WORLD_SPOT_* while dwelling, 0xFF otherwise
    u8 spotActivities;      // a named spot's activities (low/high nibble), 0xFF otherwise
    u8 activity;            // the record's WORLD_ACTIVITY_*
    u8 playerDistance;      // Manhattan tiles while the walker is in view, else AMBIENCE_DISTANCE_NONE
    u8 adjacentTicks;       // ticks the player has stood adjacent and facing, not pushing
    // The nearest other walker of each relation (index AMBIENCE_RELATION_NONE
    // = any walker) and its actor index, or AMBIENCE_DISTANCE_NONE.
    u8 notableDistance[AMBIENCE_RELATION_COLLEAGUE + 1];
    u8 notableActor[AMBIENCE_RELATION_COLLEAGUE + 1];
};

// Per-walker selection state. The walker layer keeps one per actor in its
// heap block, set up by Ambience_InitWalker when the actor spawns (and after
// a heap reset): counters at 0, the quiet gap and the steps since the last
// beat already passed (saturated).
struct AmbienceWalker
{
    u8 beat;                // the running beat, or AMBIENCE_BEAT_NONE
    u8 other;               // the actor a running beat reacts to (greetings), or AMBIENCE_ACTOR_NONE
    u8 beatCounter;         // k
    u8 decisionCounter;     // t
    u8 stepCount;           // s
    u8 stepsSinceBeat;      // saturating
    u8 gapTicks;            // ticks since the last beat ended, saturating
    u8 padding;
    u8 cooldown[AMBIENCE_MAX_BEATS]; // ticks left per beat
};

void Ambience_InitWalker(struct AmbienceWalker *walker);

// Does beat hold for this trainer (slot) in this context (when, who and
// reaction limits; not cooldowns, gap or odds)?
bool8 Ambience_BeatHolds(u8 beat, u8 slot, const struct AmbienceContext *ctx, u8 latches);

// A decision point with no beat running: the spec's gate (idle beats only:
// react and transition beats ignore the quiet gap), filter, class, idle odds
// and pick. Returns the beat to start (and marks it running in
// walker, sets walker->other for a greeting, sets the latches its limit
// needs, and advances k), or AMBIENCE_BEAT_NONE. Advances t at ARRIVE,
// LEAVE and DWELL decisions and s at STEP decisions. A once-limited beat
// that holds but is on cooldown sets its latches all the same (its moment
// passed); one that loses the pick does not.
u8 Ambience_Select(struct AmbienceWalker *walker, u8 *latches, u8 slot,
                   const struct AmbienceContext *ctx, u8 decision);

// Whether an idle beat could win at this decision point (the gate and the
// idle odds, as Ambience_Select is about to apply them; nothing advances).
// The walker layer gathers costly idle-only facts (companion_room) only then.
bool8 Ambience_IdleCanWin(const struct AmbienceWalker *walker, u8 slot, u8 decision);

// The running beat ended (finished, skipped or interrupted): records its
// cooldown and restarts the quiet gap.
void Ambience_EndBeat(struct AmbienceWalker *walker);

// Once every AMBIENCE_TICK_FRAMES frames: cooldowns run down, the gap grows.
void Ambience_Tick(struct AmbienceWalker *walker);

// Clears the approach latch once no once-per-approach beat's range holds,
// and the episode latch once the player no longer stands adjacent.
void Ambience_UpdateLatches(u8 *latches, u8 slot, const struct AmbienceContext *ctx);

// The quiet gap for a trainer, in ticks.
u8 Ambience_QuietGap(u8 slot);

#endif // IS_WAYFARER
#endif // GUARD_WAYFARER_AMBIENCE_H
