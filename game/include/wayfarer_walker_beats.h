#ifndef GUARD_WAYFARER_WALKER_BEATS_H
#define GUARD_WAYFARER_WALKER_BEATS_H

// The beat runner of notable ambience (src/wayfarer_walker_beats.c): runs a
// chosen beat's primitives on a walker's object, one frame at a time.
// Spec: .product/specs/notable-ambience.md ("Primitives", "Interruptions").
//
// Internal to the walker layer: src/wayfarer_walkers.c owns the actors, the
// decision points and the interruptions, keeps one WalkerBeatRun per actor in
// its ambience heap block and calls these. The runner never sees an actor.

#include "global.h"

#if IS_WAYFARER

#define WALKER_BEAT_LOG_SIZE 32

enum
{
    WALKER_BEAT_EVENT_START,
    WALKER_BEAT_EVENT_END,
    WALKER_BEAT_EVENT_INTERRUPT,
};

// One beat event, for the SkyEmu verifier (tools/wayfarer_walkers/verify.py).
struct WalkerBeatLog
{
    u32 frame;      // gWayfarerWalkersDebug.frames
    u8 actor;
    u8 slot;
    u8 beat;
    u8 event;       // WALKER_BEAT_EVENT_*
};

// Why a companion was put away before its beat put it away.
enum
{
    WALKER_COMPANION_AWAY_SLOTS,        // fewer than SPAWN_FREE_SLOTS object slots free (a map object or a walker needs one)
    WALKER_COMPANION_AWAY_FOLLOWER,     // the follower rule would hide the player's following Pokemon
    WALKER_COMPANION_AWAY_VANISHED,     // the object is gone (culled out of view, a script, a reset)
    WALKER_COMPANION_AWAY_INTERRUPTED,  // its beat was interrupted (a push, a script, a map change...)
    WALKER_COMPANION_AWAY_COUNT,
};

// Why companion_room didn't hold (the first failing condition).
enum
{
    WALKER_COMPANION_DENY_PLACE,        // not dwelling, or at a store, the Game Corner or a Center
    WALKER_COMPANION_DENY_BUSY,         // another companion is out on the map
    WALKER_COMPANION_DENY_SLOTS,        // the walker rule's free object slots wouldn't be kept
    WALKER_COMPANION_DENY_FOLLOWER,     // the follower rule would hide the following Pokemon
    WALKER_COMPANION_DENY_PALETTE,      // no sprite palette for it and one more
    WALKER_COMPANION_DENY_TILE,         // no tile (or 2x2 area) beside the walker for any candidate
    WALKER_COMPANION_DENY_COUNT,
};

// Counters and the event ring, at the start of the walkers' ambience heap
// block; verify.py reads them through the sAmbience pointer. Keep the layout
// in sync with it. They start at 0 whenever the block is (re)allocated.
struct WalkerAmbienceDebug
{
    u16 started[3];     // beats started, by enum AmbienceClass
    u16 ended;          // ran to their end
    u16 interrupted;
    u16 icons;          // icons shown
    u16 effects;        // field effects started
    u16 paletteSkips;   // icons and effects skipped: no sprite palette free
    u16 iconSkips;      // FLDEFF icons skipped: one already showing
    u16 stepSkips;      // steps skipped: tile not free, object missing
    u8 running[4];      // the running beat per actor, AMBIENCE_BEAT_NONE when none
    u16 logCount;       // events written; the next goes to log[logCount % WALKER_BEAT_LOG_SIZE]
    u16 padding;
    struct WalkerBeatLog log[WALKER_BEAT_LOG_SIZE];
    // The ace companion (src/wayfarer_walkers.c, "Companion").
    u16 companionsOut;      // spawned by "companion out"
    u16 companionsIn;       // put away by their own beat ("companion in", or the beat's end)
    u16 companionAway[WALKER_COMPANION_AWAY_COUNT];     // put away early, by WALKER_COMPANION_AWAY_*
    u16 companionDenied[WALKER_COMPANION_DENY_COUNT];   // companion_room false at a decision, by its first failing WALKER_COMPANION_DENY_*
    u16 companionOutFailed; // "companion out" found no room any more (or the spawn failed): the beat ended
    u16 companionStrays;    // objects with the companion's local id that weren't the live companion, removed
};

// A running beat's state (the beats module's own; the walker layer only
// stores it). Zeroed when the actor spawns; WalkerBeats_Start sets it up.
struct WalkerBeatRun
{
    u8 step;            // the step under way
    u8 stage;           // the step's sub-state
    u16 frames;         // wait countdown
    u8 startFacing;     // the facing the beat started with (look_back, interruptions)
    u8 target;          // the spot's facing, captured at the start, or DIR_NONE
    s8 dx;              // where the walker lands, from its start tile
    s8 dy;
    u8 lastDir;         // the last unreturned step's direction, or DIR_NONE
    u8 turns;           // spin: turns left
    u8 grassSprite;     // a shaking-grass effect sprite, MAX_SPRITES when none
    u8 grassFrames;
    u8 npcObject;       // an NPC bowing for this beat, OBJECT_EVENTS_COUNT when none
    u8 flags;           // RUN_*
    u8 retry;           // frames a blocked return step has waited
    u8 otherObject;     // the partner's object (greetings), OBJECT_EVENTS_COUNT when none
    u8 otherLocalId;
    u8 parity;          // the beat counter's parity at the start (emote even/odd)
    u8 beat;
    u8 padding;
};

enum
{
    WALKER_BEAT_RUNNING,
    WALKER_BEAT_DONE,           // finished on its start tile
    WALKER_BEAT_DONE_DISPLACED, // finished off its start tile (a return was blocked)
};

// Starts a beat: captures the start facing, the spot's facing (target, or
// DIR_NONE) and the partner's object (or NULL).
void WalkerBeats_Start(struct WalkerBeatRun *run, struct ObjectEvent *obj, u8 beat, u8 target,
                       struct ObjectEvent *other, u8 parity);
// One frame of the beat. visible: the walker is in the camera's view
// (field effects and sounds only play then).
u8 WalkerBeats_Run(struct WalkerBeatRun *run, struct ObjectEvent *obj, bool8 visible,
                   struct WalkerAmbienceDebug *debug);
// Stops the beat at once (an interruption, or the cleanup after its end):
// the shaking grass stops, a bowing NPC is released, the facing is unlocked.
// obj may be NULL (the object is gone). Returns TRUE when a walk or a jump of
// the beat is still under way on obj: it can't be cancelled, so the caller
// waits for it and then calls WalkerBeats_RestoreFacing. Otherwise the facing
// is restored now (a held face action) unless restoreFacing is FALSE.
bool8 WalkerBeats_Stop(struct WalkerBeatRun *run, struct ObjectEvent *obj, bool8 restoreFacing);
// After a stopped beat's last movement: face its start facing again.
void WalkerBeats_RestoreFacing(struct WalkerBeatRun *run, struct ObjectEvent *obj);
// InitHeap is about to rewrite the heap and the field is being torn down: end
// the beat with plain field writes (facing, lock, a bowing NPC's held
// movement); no sprite calls.
void WalkerBeats_OnHeapReset(struct WalkerBeatRun *run, struct ObjectEvent *obj);
// Whether the beat walks the walker off its tile and where it lands.
void WalkerBeats_Displacement(const struct WalkerBeatRun *run, s8 *dx, s8 *dy);

// Provided by src/wayfarer_walkers.c: may a beat's step go this way (a free,
// standable tile, by the walker's own collision rules)?
bool8 WayfarerWalkers_BeatCanStep(struct ObjectEvent *obj, u8 dir);
// Provided by src/wayfarer_walkers.c, which owns the companion: for the
// walker whose object this is, bring its companion out (FALSE: no room any
// more, or the spawn failed; the beat then ends), put it away, and its live
// companion object (NULL when none is out).
bool8 WayfarerWalkers_CompanionOut(struct ObjectEvent *obj);
// The frame before "companion out": decompresses the planned companion's
// sprite sheet into VRAM (the costly part of a spawn), so it doesn't share a
// frame with the room check and the spawn.
void WayfarerWalkers_CompanionPrepare(struct ObjectEvent *obj);
void WayfarerWalkers_CompanionIn(struct ObjectEvent *obj);
struct ObjectEvent *WayfarerWalkers_Companion(const struct ObjectEvent *obj);

#endif // IS_WAYFARER
#endif // GUARD_WAYFARER_WALKER_BEATS_H
