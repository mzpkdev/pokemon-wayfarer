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
    u16 companionPrepares;  // companion sheets decompressed ("companion out"'s first frame: the one costly frame)
    u16 padding2;
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
    u8 roomWait;        // frames a costly start (icon, effect, companion) has waited for a frame with room
    u8 roomBest;        // the most lines a frame had left during that wait
    u8 heldAction;      // the held movement the runner last issued on the walker (RUN_HELD)
    u8 otherSlot;       // the partner's trainer slot (its object may be another walker's by now)
    u8 padding;
};

enum
{
    WALKER_BEAT_RUNNING,
    WALKER_BEAT_DONE,           // finished on its start tile
    WALKER_BEAT_DONE_DISPLACED, // finished off its start tile (a return was blocked)
};

// Starts a beat: captures the start facing (the facing the walker is turning
// to, when a turn is still queued), the spot's facing (target, or DIR_NONE)
// and the partner's object (or NULL) and trainer slot. A bow the run's last
// beat left to a script (WalkerBeats_ReleaseNpc) ends first.
void WalkerBeats_Start(struct WalkerBeatRun *run, u8 beat, u8 startFacing, u8 target,
                       struct ObjectEvent *other, u8 otherSlot, u8 parity);
// One frame of the beat. visible: the walker is in the camera's view
// (field effects and sounds only play then). quiet: no new step starts this
// frame (steps under way go on).
u8 WalkerBeats_Run(struct WalkerBeatRun *run, struct ObjectEvent *obj, bool8 visible, bool8 quiet,
                   struct WalkerAmbienceDebug *debug);
// Stops the beat at once (an interruption, or the cleanup after its end):
// the shaking grass stops, a bowing NPC is released, the facing is unlocked.
// obj may be NULL (the object is gone). Returns TRUE when a walk or a jump of
// the beat is still under way on obj: it can't be cancelled, so the caller
// waits for it and then calls WalkerBeats_RestoreFacing. Otherwise the facing
// is restored now unless restoreFacing is FALSE (a keep-walking beat never
// restores it: the walker's own walk turns it). locked: a script or a menu
// holds the field (objects may be frozen): no held movement is issued (it
// would unfreeze the walker; the facing is set with a plain turn) and a bowing
// NPC is left to the script until WalkerBeats_ReleaseNpc. Only a held
// movement the runner issued itself is ever cleared (never the walker's own
// walk under a keep-walking beat).
bool8 WalkerBeats_Stop(struct WalkerBeatRun *run, struct ObjectEvent *obj, bool8 restoreFacing, bool8 locked);
// After a stopped beat's last movement: face its start facing again.
void WalkerBeats_RestoreFacing(struct WalkerBeatRun *run, struct ObjectEvent *obj);
// With the field controls free again: releases an NPC a locked stop left
// bowing. The walker layer calls it for every run each unlocked frame (the
// run's actor may be gone) and before a run is reset.
void WalkerBeats_ReleaseNpc(struct WalkerBeatRun *run);
// InitHeap (the heap may already hold other data, so no beat state is read):
// any nurse still held in a bow ends it, with plain field writes.
void WalkerBeats_OnHeapReset(void);
// Whether the beat walks the walker off its tile and where it lands.
void WalkerBeats_Displacement(const struct WalkerBeatRun *run, s8 *dx, s8 *dy);

// Provided by src/wayfarer_walkers.c: may a beat's step go this way (a free,
// standable tile, by the walker's own collision rules)?
bool8 WayfarerWalkers_BeatCanStep(struct ObjectEvent *obj, u8 dir);
// Provided by src/wayfarer_walkers.c: the trainer slot of the walker whose
// object this is, or 0xFF (not a walker's object).
u8 WayfarerWalkers_ObjectSlot(const struct ObjectEvent *obj);
// Provided by src/wayfarer_walkers.c: claims this frame for one costly start
// (an icon, a field effect, the companion) of about this many scanlines.
// FALSE: another one or a full decision already took the frame, it is the
// walkers' busy frames (spawns, the follower rule), or (unless force) fewer lines are left before the
// frame's end margin; the caller tries again next frame. Lines left: the
// lines this frame still has (for the caller's own choice of frame).
bool8 WayfarerWalkers_ClaimFrame(u16 lines, bool8 force);
u16 WayfarerWalkers_FrameLinesLeft(void);
// The next frame is one of the busy ones (a start's new sprite costs there too).
bool8 WayfarerWalkers_NextFrameBusy(void);
// Provided by src/wayfarer_walkers.c, which owns the companion (reserved for
// the walker when its beat starts): for the walker whose object this is,
// bring its companion out (FALSE: no room any more, or the spawn failed; the
// beat then ends), put it away, and its live companion object (NULL when none
// is out). The runner picks the frames (WayfarerWalkers_ClaimFrame).
// The frame before "companion out": decompresses the planned companion's
// sprite sheet into VRAM (the costly part of a spawn), so it doesn't share a
// frame with the room check and the spawn.
void WayfarerWalkers_CompanionPrepare(struct ObjectEvent *obj);
enum
{
    WALKER_COMPANION_OUT_DONE,      // it spawned
    WALKER_COMPANION_OUT_FAILED,    // no room any more, or the spawn failed: the beat ends
    WALKER_COMPANION_OUT_PREPARE,   // the prepared species no longer fits but another does, and its
                                    // sheet needs decompressing: prepare again (in its own frame)
};
// mayPrepare: a PREPARE answer is allowed (else that case fails).
u8 WayfarerWalkers_CompanionOut(struct ObjectEvent *obj, bool8 mayPrepare);
void WayfarerWalkers_CompanionIn(struct ObjectEvent *obj);
struct ObjectEvent *WayfarerWalkers_Companion(const struct ObjectEvent *obj);

#endif // IS_WAYFARER
#endif // GUARD_WAYFARER_WALKER_BEATS_H
