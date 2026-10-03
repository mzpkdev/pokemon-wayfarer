// The beat runner of notable ambience: runs a chosen beat's primitives on a
// walker's object, a frame at a time. Spec: .product/specs/notable-ambience.md
// ("Primitives", "Icons", "Interruptions"); the decision points, the context
// and the interruptions live in wayfarer_walkers.c, which owns the actors.
//
// Engine rules every primitive follows:
// - A held movement is only issued once the object's last one is cleared
//   (ObjectEventSetHeldMovement refuses while one is active, finished or not,
//   and returns TRUE then), and the next step waits for it to finish.
// - A walk or a jump is never cancelled half-way (the object's coordinates
//   are already on the destination, and a jump's arc is mid-air): a stopped
//   beat lets it finish (the walker's actionPending wait) and only then turns
//   the walker back.
// - "step back" keeps the facing with obj->facingDirectionLocked; while it is
//   set no face action can turn the walker, so it is cleared as soon as the
//   step is over and on every stop.
// - MOVEMENT_ACTION_SPIN_* slides a tile at fast speed: spin is four face
//   turns in rotation instead.
// - Field effects and icons load a sprite palette: with no slot free the
//   palette load would index out of bounds, so they are skipped then.
// - FLDEFF_EMOTE shares its id with FLDEFF_QUESTION_MARK_ICON ("?"): only one
//   shows at a time; a busy icon waits 1 tick, then is skipped.

#include "global.h"
#include "wayfarer_walker_beats.h"

#if IS_WAYFARER
#include "event_object_movement.h"
#include "field_effect.h"
#include "field_effect_helpers.h"
#include "fieldmap.h"
#include "metatile_behavior.h"
#include "sound.h"
#include "sprite.h"
#include "wayfarer_ambience.h"
#include "wayfarer_walkers.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"
#include "constants/songs.h"
#include "constants/trainer_types.h"

#define RUN_LOCKED       (1 << 0)   // facing locked for a step back
#define RUN_MOVING       (1 << 1)   // a walk or a jump under way
#define RUN_SKIP_RETURN  (1 << 2)   // a step was skipped: skip the step that returns it
#define RUN_RESTORE      (1 << 3)   // stopped mid-movement: turn back once it is over

#define SPIN_TURN_FRAMES    4
#define GRASS_SHAKE_FRAMES  (2 * AMBIENCE_TICK_FRAMES)
#define RETURN_WAIT_FRAMES  60      // a blocked return step retries this long
#define BOW_MAX_FRAMES      120
#define NURSE_RANGE         3       // the nurse stands behind the counter

enum
{
    STAGE_START,
    STAGE_HELD,     // waiting for the walker's held movement
    STAGE_COUNT,    // a frame countdown
    STAGE_BUSY,     // an icon waiting for FLDEFF_EMOTE to free up
    STAGE_NPC,      // waiting for the NPC's bow
};

// verify.py reads the debug block at fixed offsets.
STATIC_ASSERT(sizeof(struct WalkerBeatLog) == 8, WalkerBeatLogSize);
STATIC_ASSERT(offsetof(struct WalkerAmbienceDebug, running) == 20, WalkerAmbienceDebugRunning);
STATIC_ASSERT(offsetof(struct WalkerAmbienceDebug, log) == 28, WalkerAmbienceDebugLog);

static const s8 sDx[5] = {0, 0, 0, -1, 1};
static const s8 sDy[5] = {0, 1, -1, 0, 0};
static const u8 sOpposite[5] = {DIR_NONE, DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST};
// Spin: south, west, north, east, round to south again.
static const u8 sClockwise[5] = {DIR_SOUTH, DIR_WEST, DIR_EAST, DIR_NORTH, DIR_SOUTH};

static const u8 sIconActions[AMBIENCE_ICON_FIRST_FLDEFF] = {
    [AMBIENCE_ICON_EXCLAMATION] = MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK,
    [AMBIENCE_ICON_DOUBLE_EXCL] = MOVEMENT_ACTION_EMOTE_DOUBLE_EXCL_MARK,
    [AMBIENCE_ICON_QUESTION]    = MOVEMENT_ACTION_EMOTE_QUESTION_MARK,
    [AMBIENCE_ICON_X]           = MOVEMENT_ACTION_EMOTE_X,
    [AMBIENCE_ICON_HEART]       = MOVEMENT_ACTION_EMOTE_HEART,
};

// The follower emote sheet's frames (FldEff_QuestionMarkIcon, args[7]).
static const u8 sIconFrames[AMBIENCE_ICON_COUNT - AMBIENCE_ICON_FIRST_FLDEFF] = {
    [AMBIENCE_ICON_ELLIPSIS - AMBIENCE_ICON_FIRST_FLDEFF] = 5,  // FOLLOWER_EMOTION_PENSIVE: the "..." bubble
    [AMBIENCE_ICON_HAPPY - AMBIENCE_ICON_FIRST_FLDEFF]    = 0,
    [AMBIENCE_ICON_MUSIC - AMBIENCE_ICON_FIRST_FLDEFF]    = 9,
    [AMBIENCE_ICON_LOVE - AMBIENCE_ICON_FIRST_FLDEFF]     = 6,
    [AMBIENCE_ICON_CURIOUS - AMBIENCE_ICON_FIRST_FLDEFF]  = 8,
    [AMBIENCE_ICON_PENSIVE - AMBIENCE_ICON_FIRST_FLDEFF]  = 5,
    [AMBIENCE_ICON_SAD - AMBIENCE_ICON_FIRST_FLDEFF]      = 2,
    [AMBIENCE_ICON_ANGRY - AMBIENCE_ICON_FIRST_FLDEFF]    = 4,
    [AMBIENCE_ICON_SURPRISE - AMBIENCE_ICON_FIRST_FLDEFF] = 7,
};

// The palettes the icons' sprites use (trainer_see.c's templates).
static u16 IconPaletteTag(u8 icon)
{
    if (icon >= AMBIENCE_ICON_FIRST_FLDEFF)
        return OBJ_EVENT_PAL_TAG_EMOTES;
    if (icon == AMBIENCE_ICON_HEART)
        return OBJ_EVENT_PAL_TAG_NPC_1;
    return OBJ_EVENT_PAL_TAG_MAY;
}

static const u16 sEffectPaletteTags[] = {
    [AMBIENCE_EFFECT_RIPPLE]      = FLDEFF_PAL_TAG_GENERAL_1,
    [AMBIENCE_EFFECT_SPLASH]      = FLDEFF_PAL_TAG_GENERAL_1,   // shown as a ripple
    [AMBIENCE_EFFECT_GRASS_SHAKE] = FLDEFF_PAL_TAG_GENERAL_1,
    [AMBIENCE_EFFECT_DUST]        = FLDEFF_PAL_TAG_GENERAL_0,
    [AMBIENCE_EFFECT_SPARKLE]     = FLDEFF_PAL_TAG_SMALL_SPARKLE,
};

// A sprite palette is loaded already or a slot is free for it.
static bool8 HasPalette(u16 tag)
{
    return IndexOfSpritePaletteTag(tag) != 0xFF || IndexOfSpritePaletteTag(TAG_NONE) != 0xFF;
}

static bool8 IsCardinal(u8 dir)
{
    return dir >= DIR_SOUTH && dir <= DIR_EAST;
}

static u8 DirectionTowards(s16 x, s16 y, s16 tx, s16 ty)
{
    s16 dx = tx - x, dy = ty - y;
    s16 ax = dx < 0 ? -dx : dx, ay = dy < 0 ? -dy : dy;
    if (ax == 0 && ay == 0)
        return DIR_NONE;
    // The longer axis; vertical on a tie (the walkers' own rule).
    if (ay >= ax)
        return dy > 0 ? DIR_SOUTH : DIR_NORTH;
    return dx > 0 ? DIR_EAST : DIR_WEST;
}

// Issue a held movement on an object whose last one may be finished but not
// yet cleared. FALSE: the object is busy; try again next frame.
static bool8 Issue(struct ObjectEvent *obj, u8 action)
{
    ObjectEventClearHeldMovementIfFinished(obj);
    return !ObjectEventSetHeldMovement(obj, action);
}

// The held movement is over (or there is none).
static bool8 HeldDone(struct ObjectEvent *obj)
{
    return ObjectEventClearHeldMovementIfFinished(obj) != 0;
}

static struct ObjectEvent *Partner(const struct WalkerBeatRun *run)
{
    struct ObjectEvent *other;
    if (run->otherObject >= OBJECT_EVENTS_COUNT)
        return NULL;
    other = &gObjectEvents[run->otherObject];
    return (other->active && other->localId == run->otherLocalId) ? other : NULL;
}

static bool8 IsGrassSprite(u8 spriteId)
{
    return spriteId < MAX_SPRITES && gSprites[spriteId].inUse
        && gSprites[spriteId].callback == WaitFieldEffectSpriteAnim
        && gSprites[spriteId].data[0] == FLDEFF_SHAKING_GRASS;
}

static void StopGrass(struct WalkerBeatRun *run)
{
    if (IsGrassSprite(run->grassSprite))
        FieldEffectStop(&gSprites[run->grassSprite], FLDEFF_SHAKING_GRASS);
    run->grassSprite = MAX_SPRITES;
}

// Releases an NPC still bowing for this beat (its movement type restarts).
static void ReleaseNpc(struct WalkerBeatRun *run)
{
    if (run->npcObject < OBJECT_EVENTS_COUNT)
    {
        struct ObjectEvent *npc = &gObjectEvents[run->npcObject];
        if (npc->active && npc->heldMovementActive && npc->movementActionId == MOVEMENT_ACTION_NURSE_JOY_BOW_DOWN)
            ObjectEventClearHeldMovement(npc);
    }
    run->npcObject = OBJECT_EVENTS_COUNT;
}

void WalkerBeats_Start(struct WalkerBeatRun *run, struct ObjectEvent *obj, u8 beat, u8 target,
                       struct ObjectEvent *other, u8 parity)
{
    memset(run, 0, sizeof(*run));
    run->beat = beat;
    run->startFacing = obj->facingDirection;
    run->target = IsCardinal(target) ? target : DIR_NONE;
    run->lastDir = DIR_NONE;
    run->grassSprite = MAX_SPRITES;
    run->npcObject = OBJECT_EVENTS_COUNT;
    run->otherObject = OBJECT_EVENTS_COUNT;
    if (other != NULL)
    {
        run->otherObject = other - gObjectEvents;
        run->otherLocalId = other->localId;
    }
    run->parity = parity & 1;
}

// ---------------------------------------------------------------------------
// Primitives. Each returns TRUE once its step is over.

static u8 FaceTarget(const struct WalkerBeatRun *run, struct ObjectEvent *obj, u8 target)
{
    struct ObjectEvent *other;
    switch (target)
    {
    case AMBIENCE_TARGET_DOWN:
    case AMBIENCE_TARGET_UP:
    case AMBIENCE_TARGET_LEFT:
    case AMBIENCE_TARGET_RIGHT:
        return target;  // DIR_SOUTH..DIR_EAST
    case AMBIENCE_TARGET_PLAYER:
        other = &gObjectEvents[gPlayerAvatar.objectEventId];
        return DirectionTowards(obj->currentCoords.x, obj->currentCoords.y, other->currentCoords.x, other->currentCoords.y);
    case AMBIENCE_TARGET_OTHER:
        other = Partner(run);
        if (other == NULL)
            return DIR_NONE;
        return DirectionTowards(obj->currentCoords.x, obj->currentCoords.y, other->currentCoords.x, other->currentCoords.y);
    case AMBIENCE_TARGET_TARGET:
        return run->target;
    case AMBIENCE_TARGET_AWAY:
        return sOpposite[run->target];
    }
    return DIR_NONE;    // the companion: stage 4
}

static bool8 DoFace(struct WalkerBeatRun *run, struct ObjectEvent *obj, u8 dir, struct WalkerAmbienceDebug *debug)
{
    if (run->stage == STAGE_HELD)
        return HeldDone(obj);
    if (!IsCardinal(dir))
    {
        debug->stepSkips++;
        return TRUE;
    }
    if (obj->facingDirection == dir)
        return TRUE;
    if (!Issue(obj, GetFaceDirectionMovementAction(dir)))
        return FALSE;
    run->stage = STAGE_HELD;
    return FALSE;
}

static bool8 DoWait(struct WalkerBeatRun *run, u8 ticks)
{
    if (run->stage == STAGE_START)
    {
        run->frames = ticks * AMBIENCE_TICK_FRAMES;
        run->stage = STAGE_COUNT;
    }
    if (run->frames != 0)
        run->frames--;
    return run->frames == 0;
}

static bool8 DoStep(struct WalkerBeatRun *run, struct ObjectEvent *obj, u8 kind, struct WalkerAmbienceDebug *debug)
{
    u8 dir;
    bool8 isReturn = FALSE, back = FALSE;

    if (run->stage == STAGE_HELD)
    {
        if (!HeldDone(obj))
            return FALSE;
        run->flags &= ~RUN_MOVING;
        if (run->flags & RUN_LOCKED)
        {
            obj->facingDirectionLocked = FALSE;
            run->flags &= ~RUN_LOCKED;
        }
        return TRUE;
    }
    if (run->flags & RUN_SKIP_RETURN)
    {
        // The step this one would return was skipped: so is this one.
        run->flags &= ~RUN_SKIP_RETURN;
        return TRUE;
    }
    switch (kind)
    {
    case AMBIENCE_STEP_BACK:
        back = TRUE;
        if (run->lastDir != DIR_NONE)
        {
            dir = sOpposite[run->lastDir];
            isReturn = TRUE;
        }
        else
        {
            dir = sOpposite[obj->facingDirection];
        }
        break;
    case AMBIENCE_STEP_LEFT:
        dir = DIR_WEST;
        break;
    case AMBIENCE_STEP_RIGHT:
        dir = DIR_EAST;
        break;
    default:
        dir = obj->facingDirection;
        break;
    }
    if (!IsCardinal(dir))
    {
        debug->stepSkips++;
        return TRUE;
    }
    if (!isReturn && run->lastDir != DIR_NONE)
    {
        if (dir == sOpposite[run->lastDir])
            isReturn = TRUE;
        else
        {
            // Never more than a tile off the start (the generator rules
            // this out; a skipped return could leave the walker there).
            debug->stepSkips++;
            return TRUE;
        }
    }
    if (!WayfarerWalkers_BeatCanStep(obj, dir))
    {
        if (isReturn)
        {
            // Back to the start tile: wait a little for it to free up, then
            // give up the beat (the walker layer walks it back).
            if (++run->retry < RETURN_WAIT_FRAMES)
                return FALSE;
            run->step = AMBIENCE_MAX_STEPS;
            return TRUE;
        }
        debug->stepSkips++;
        run->flags |= RUN_SKIP_RETURN;
        return TRUE;
    }
    if (back)
        obj->facingDirectionLocked = TRUE;
    if (!Issue(obj, GetWalkSlowMovementAction(dir)))
    {
        if (back)
            obj->facingDirectionLocked = FALSE;
        return FALSE;
    }
    run->retry = 0;
    if (back)
        run->flags |= RUN_LOCKED;
    run->flags |= RUN_MOVING;
    run->dx += sDx[dir];
    run->dy += sDy[dir];
    run->lastDir = isReturn ? DIR_NONE : dir;
    run->stage = STAGE_HELD;
    return FALSE;
}

static bool8 DoJump(struct WalkerBeatRun *run, struct ObjectEvent *obj)
{
    if (run->stage == STAGE_HELD)
    {
        if (!HeldDone(obj))
            return FALSE;
        run->flags &= ~RUN_MOVING;
        return TRUE;
    }
    if (!Issue(obj, GetJumpInPlaceMovementAction(obj->facingDirection)))
        return FALSE;
    run->flags |= RUN_MOVING;
    run->stage = STAGE_HELD;
    return FALSE;
}

static bool8 DoSpin(struct WalkerBeatRun *run, struct ObjectEvent *obj)
{
    switch (run->stage)
    {
    case STAGE_START:
        if (run->turns == 0)
            run->turns = 4;
        if (!Issue(obj, GetFaceDirectionMovementAction(sClockwise[obj->facingDirection <= DIR_EAST ? obj->facingDirection : 0])))
            return FALSE;
        run->stage = STAGE_HELD;
        return FALSE;
    case STAGE_HELD:
        if (!HeldDone(obj))
            return FALSE;
        if (--run->turns == 0)
            return TRUE;
        run->frames = SPIN_TURN_FRAMES;
        run->stage = STAGE_COUNT;
        return FALSE;
    default:
        if (run->frames != 0 && --run->frames != 0)
            return FALSE;
        run->stage = STAGE_START;
        return FALSE;
    }
}

static bool8 DoEmote(struct WalkerBeatRun *run, struct ObjectEvent *obj, u8 icon, struct WalkerAmbienceDebug *debug)
{
    // "?" is FLDEFF_QUESTION_MARK_ICON, the same effect id as FLDEFF_EMOTE.
    bool8 shared = icon >= AMBIENCE_ICON_FIRST_FLDEFF || icon == AMBIENCE_ICON_QUESTION;

    if (icon >= AMBIENCE_ICON_COUNT)
        return TRUE;
    switch (run->stage)
    {
    case STAGE_HELD:
        return HeldDone(obj);
    case STAGE_START:
        if (shared && FieldEffectActiveListContains(FLDEFF_EMOTE))
        {
            run->frames = AMBIENCE_TICK_FRAMES;
            run->stage = STAGE_BUSY;
            return FALSE;
        }
        break;
    case STAGE_BUSY:
        if (run->frames != 0 && --run->frames != 0)
            return FALSE;
        if (FieldEffectActiveListContains(FLDEFF_EMOTE))
        {
            debug->iconSkips++;
            return TRUE;
        }
        break;
    }
    if (!HasPalette(IconPaletteTag(icon)))
    {
        debug->paletteSkips++;
        return TRUE;
    }
    if (icon < AMBIENCE_ICON_FIRST_FLDEFF)
    {
        if (!Issue(obj, sIconActions[icon]))
            return FALSE;
        run->stage = STAGE_HELD;
    }
    else
    {
        gFieldEffectArguments[0] = obj->localId;
        gFieldEffectArguments[1] = obj->mapNum;
        gFieldEffectArguments[2] = obj->mapGroup;
        gFieldEffectArguments[7] = sIconFrames[icon - AMBIENCE_ICON_FIRST_FLDEFF];
        FieldEffectStart(FLDEFF_EMOTE);
    }
    debug->icons++;
    gWayfarerWalkersDebug.emotes++;
    return run->stage != STAGE_HELD;
}

static void DoEffect(struct WalkerBeatRun *run, struct ObjectEvent *obj, u8 arg, bool8 visible,
                     struct WalkerAmbienceDebug *debug)
{
    u8 kind = arg & AMBIENCE_EFFECT_MASK, where = arg >> AMBIENCE_WHERE_SHIFT;
    s16 x = obj->currentCoords.x, y = obj->currentCoords.y, px, py;
    u8 dir = obj->facingDirection, priority = gSprites[obj->spriteId].oam.priority;

    if (kind >= ARRAY_COUNT(sEffectPaletteTags) || !visible)
        return;
    if (where == AMBIENCE_WHERE_AHEAD && IsCardinal(dir))
    {
        x += sDx[dir];
        y += sDy[dir];
    }
    if (!HasPalette(sEffectPaletteTags[kind]))
    {
        debug->paletteSkips++;
        return;
    }
    switch (kind)
    {
    case AMBIENCE_EFFECT_RIPPLE:
    case AMBIENCE_EFFECT_SPLASH:
        // FLDEFF_SPLASH is drawn at an object's feet (and reads past the
        // object table if the object is missing): on a water tile ahead the
        // splash is a ripple there with the splash's sound.
        SetSpritePosToMapCoords(x, y, &px, &py);
        gFieldEffectArguments[0] = px + 8;
        gFieldEffectArguments[1] = py + 14;
        gFieldEffectArguments[2] = 151;
        gFieldEffectArguments[3] = 3;
        FieldEffectStart(FLDEFF_RIPPLE);
        if (kind == AMBIENCE_EFFECT_SPLASH)
            PlaySE(SE_PUDDLE);
        break;
    case AMBIENCE_EFFECT_GRASS_SHAKE:
    {
        u8 behavior = MapGridGetMetatileBehaviorAt(x, y);
        u32 spriteId;
        // Only grass shakes; the effect loops until it is stopped.
        if (!MetatileBehavior_IsTallGrass(behavior) && !MetatileBehavior_IsLongGrass(behavior))
            return;
        StopGrass(run);
        gFieldEffectArguments[0] = x;
        gFieldEffectArguments[1] = y;
        gFieldEffectArguments[2] = 0xFF;    // as DexNav starts it
        gFieldEffectArguments[3] = 2;
        spriteId = FieldEffectStart(FLDEFF_SHAKING_GRASS);
        if (spriteId >= MAX_SPRITES)
            return;
        run->grassSprite = spriteId;
        run->grassFrames = 0;
        break;
    }
    case AMBIENCE_EFFECT_DUST:
        gFieldEffectArguments[0] = x;
        gFieldEffectArguments[1] = y;
        gFieldEffectArguments[2] = obj->currentElevation;
        gFieldEffectArguments[3] = priority;
        FieldEffectStart(FLDEFF_DUST);
        break;
    case AMBIENCE_EFFECT_SPARKLE:
        gFieldEffectArguments[0] = x - MAP_OFFSET;  // FldEff_Sparkle adds MAP_OFFSET
        gFieldEffectArguments[1] = y - MAP_OFFSET;
        gFieldEffectArguments[2] = priority;
        FieldEffectStart(FLDEFF_SPARKLE);
        break;
    }
    debug->effects++;
}

static bool8 IsNurse(u16 gfx)
{
    // Only these have the bow animation (ANIM_NURSE_BOW); never the Chansey.
    return gfx == OBJ_EVENT_GFX_NURSE || gfx == OBJ_EVENT_GFX_NURSE_FRLG || gfx == OBJ_EVENT_GFX_NURSE_HNS;
}

static bool8 IsIdleNpc(const struct ObjectEvent *npc)
{
    return npc->active && !npc->isPlayer && !npc->frozen && !npc->singleMovementActive && !npc->heldMovementActive
        && !WayfarerWalkers_IsActorObject(npc) && npc->localId != OBJ_EVENT_ID_FOLLOWER;
}

static bool8 DoBow(struct WalkerBeatRun *run, struct ObjectEvent *obj, struct WalkerAmbienceDebug *debug)
{
    u8 i;
    if (run->stage == STAGE_NPC)
    {
        struct ObjectEvent *npc = &gObjectEvents[run->npcObject];
        if (run->npcObject < OBJECT_EVENTS_COUNT && npc->active && npc->heldMovementActive
         && !npc->heldMovementFinished && ++run->frames < BOW_MAX_FRAMES)
            return FALSE;
        ReleaseNpc(run);
        return TRUE;
    }
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *npc = &gObjectEvents[i];
        s16 dx = npc->currentCoords.x - obj->currentCoords.x, dy = npc->currentCoords.y - obj->currentCoords.y;
        if (!IsNurse(npc->graphicsId) || !IsIdleNpc(npc)
         || (dx < 0 ? -dx : dx) + (dy < 0 ? -dy : dy) > NURSE_RANGE)
            continue;
        if (ObjectEventSetHeldMovement(npc, MOVEMENT_ACTION_NURSE_JOY_BOW_DOWN))
            break;
        run->npcObject = i;
        run->frames = 0;
        run->stage = STAGE_NPC;
        return FALSE;
    }
    debug->stepSkips++;
    return TRUE;
}

// "turn npc": the NPC on the faced tile turns to the walker. ObjectEventTurn
// (no held movement, so nothing to clear or fight the NPC's movement type),
// only while it stands still, and never a trainer (its sight follows its facing).
static void DoTurnNpc(struct ObjectEvent *obj, struct WalkerAmbienceDebug *debug)
{
    u8 dir = obj->facingDirection, id;
    if (IsCardinal(dir))
    {
        id = GetObjectEventIdByXY(obj->currentCoords.x + sDx[dir], obj->currentCoords.y + sDy[dir]);
        if (id < OBJECT_EVENTS_COUNT && IsIdleNpc(&gObjectEvents[id]) && gObjectEvents[id].trainerType == TRAINER_TYPE_NONE
         && !gObjectEvents[id].facingDirectionLocked)
        {
            ObjectEventTurn(&gObjectEvents[id], sOpposite[dir]);
            return;
        }
    }
    debug->stepSkips++;
}

static bool8 RunStep(struct WalkerBeatRun *run, struct ObjectEvent *obj, u8 op, u8 arg, bool8 visible,
                     struct WalkerAmbienceDebug *debug)
{
    switch (op)
    {
    case AMBIENCE_OP_FACE:
        return DoFace(run, obj, run->stage == STAGE_START ? FaceTarget(run, obj, arg) : DIR_NONE, debug);
    case AMBIENCE_OP_LOOK_BACK:
        return DoFace(run, obj, run->startFacing, debug);
    case AMBIENCE_OP_WAIT:
        return DoWait(run, arg);
    case AMBIENCE_OP_STEP:
        return DoStep(run, obj, arg, debug);
    case AMBIENCE_OP_JUMP:
        return DoJump(run, obj);
    case AMBIENCE_OP_SPIN:
        return DoSpin(run, obj);
    case AMBIENCE_OP_BOW:
        return DoBow(run, obj, debug);
    case AMBIENCE_OP_TURN:
        DoTurnNpc(obj, debug);
        return TRUE;
    case AMBIENCE_OP_EMOTE:
        return DoEmote(run, obj, arg, debug);
    case AMBIENCE_OP_EMOTE_PARITY:
        return DoEmote(run, obj, run->parity ? (arg >> AMBIENCE_PARITY_SHIFT) : (arg & 0xF), debug);
    case AMBIENCE_OP_EFFECT:
        DoEffect(run, obj, arg, visible, debug);
        return TRUE;
    default:
        // The companion comes in stage 4: a beat that needs it ends here
        // (companion_room never holds yet, so none is picked).
        run->step = AMBIENCE_MAX_STEPS;
        return TRUE;
    }
}

u8 WalkerBeats_Run(struct WalkerBeatRun *run, struct ObjectEvent *obj, bool8 visible,
                   struct WalkerAmbienceDebug *debug)
{
    const struct WayfarerAmbienceBeat *beat = &gWayfarerAmbienceBeats[run->beat];
    u8 guard;

    if (run->grassSprite != MAX_SPRITES && ++run->grassFrames >= GRASS_SHAKE_FRAMES)
        StopGrass(run);
    // Steps that finish at once (effects, an FLDEFF icon, a face already
    // right) chain within the frame.
    for (guard = 0; guard <= AMBIENCE_MAX_STEPS && run->step < beat->stepCount; guard++)
    {
        if (!RunStep(run, obj, beat->steps[run->step][0], beat->steps[run->step][1], visible, debug))
            return WALKER_BEAT_RUNNING;
        run->step++;
        run->stage = STAGE_START;
        run->frames = 0;
    }
    if (run->step < beat->stepCount)
        return WALKER_BEAT_RUNNING;
    return (run->dx != 0 || run->dy != 0) ? WALKER_BEAT_DONE_DISPLACED : WALKER_BEAT_DONE;
}

bool8 WalkerBeats_Stop(struct WalkerBeatRun *run, struct ObjectEvent *obj, bool8 restoreFacing)
{
    StopGrass(run);
    ReleaseNpc(run);
    if (obj == NULL)
        return FALSE;
    obj->facingDirectionLocked = FALSE;
    run->flags &= ~RUN_LOCKED;
    if ((run->flags & RUN_MOVING) && ObjectEventCheckHeldMovementStatus(obj) == 0)
    {
        // A walk or a jump can't be cancelled: turn back once it is over.
        run->flags &= ~RUN_MOVING;
        if (restoreFacing)
            run->flags |= RUN_RESTORE;
        return TRUE;
    }
    run->flags &= ~RUN_MOVING;
    if (restoreFacing)
    {
        run->flags |= RUN_RESTORE;
        WalkerBeats_RestoreFacing(run, obj);
        return (run->flags & RUN_RESTORE) != 0;
    }
    return FALSE;
}

void WalkerBeats_RestoreFacing(struct WalkerBeatRun *run, struct ObjectEvent *obj)
{
    if (!(run->flags & RUN_RESTORE))
        return;
    obj->facingDirectionLocked = FALSE;
    if (!IsCardinal(run->startFacing) || obj->facingDirection == run->startFacing
     || Issue(obj, GetFaceDirectionMovementAction(run->startFacing)))
        run->flags &= ~RUN_RESTORE;
}

void WalkerBeats_OnHeapReset(struct WalkerBeatRun *run, struct ObjectEvent *obj)
{
    // The field is being torn down: plain field writes only. The sprites go
    // with it (the shaking grass too); the objects keep their fields.
    run->grassSprite = MAX_SPRITES;
    if (run->npcObject < OBJECT_EVENTS_COUNT)
    {
        struct ObjectEvent *npc = &gObjectEvents[run->npcObject];
        if (npc->active && npc->movementActionId == MOVEMENT_ACTION_NURSE_JOY_BOW_DOWN)
        {
            npc->movementActionId = MOVEMENT_ACTION_NONE;
            npc->heldMovementActive = FALSE;
            npc->heldMovementFinished = FALSE;
        }
        run->npcObject = OBJECT_EVENTS_COUNT;
    }
    run->flags = 0;
    if (obj == NULL)
        return;
    obj->facingDirectionLocked = FALSE;
    if (IsCardinal(run->startFacing))
    {
        obj->facingDirection = run->startFacing;
        obj->movementDirection = run->startFacing;
    }
}

void WalkerBeats_Displacement(const struct WalkerBeatRun *run, s8 *dx, s8 *dy)
{
    *dx = run->dx;
    *dy = run->dy;
}

#endif // IS_WAYFARER
