#ifndef GUARD_CONSTANTS_WAYFARER_AMBIENCE_H
#define GUARD_CONSTANTS_WAYFARER_AMBIENCE_H

// Notable ambience (.product/specs/notable-ambience.md): the beat pool's
// constants. tools/wayfarer_world/ambience.py emits the tables with these
// names and keeps its own lists in the same order; the C compiler checks the
// names, and the order is mirrored by hand, so edit both together.

#define AMBIENCE_MAX_STEPS        8
#define AMBIENCE_MAX_BEATS        40    // the runtime keeps a cooldown byte per beat
#define AMBIENCE_TICK_FRAMES      15    // a beat's wait and cooldown tick
#define AMBIENCE_COMPANION_MAX    4     // companion candidates per trainer
#define AMBIENCE_PREFERRED_NONE   0xFF
#define AMBIENCE_DEFAULT_COOLDOWN 80

// Companion candidate: species | AMBIENCE_COMPANION_BIG for a 64x64 follower sprite.
#define AMBIENCE_COMPANION_BIG          (1 << 15)
#define AMBIENCE_COMPANION_SPECIES_MASK 0x7FFF

// Beat classes, in priority order: only the highest class present is picked.
enum AmbienceClass
{
    AMBIENCE_CLASS_IDLE,
    AMBIENCE_CLASS_TRANSITION,
    AMBIENCE_CLASS_REACT,
};

// Step ops. A step is {op, arg}; the arg column says what arg holds.
enum AmbienceOp
{
    AMBIENCE_OP_FACE,          // arg: AMBIENCE_TARGET_*
    AMBIENCE_OP_LOOK_BACK,     // back to the facing the beat started with
    AMBIENCE_OP_WAIT,          // arg: ticks (AMBIENCE_TICK_FRAMES each)
    AMBIENCE_OP_STEP,          // arg: AMBIENCE_STEP_*
    AMBIENCE_OP_JUMP,          // jump in place
    AMBIENCE_OP_SPIN,          // spin once
    AMBIENCE_OP_BOW,           // arg: AMBIENCE_OBJECT_NURSE (the adjacent object bows)
    AMBIENCE_OP_TURN,          // arg: AMBIENCE_OBJECT_NPC (the faced object turns to the walker)
    AMBIENCE_OP_EMOTE,         // arg: AMBIENCE_ICON_*
    AMBIENCE_OP_EMOTE_PARITY,  // arg: icon on an even beat counter | icon on an odd one << 4
    AMBIENCE_OP_EFFECT,        // arg: AMBIENCE_EFFECT_* | AMBIENCE_WHERE_* << 4
    AMBIENCE_OP_COMPANION_OUT,
    AMBIENCE_OP_COMPANION_IN,
    AMBIENCE_OP_COMPANION_DO,  // arg: AMBIENCE_COMPANION_ACT_*
    AMBIENCE_OP_COUNT,
};

// Face targets: the four directions equal DIR_SOUTH/NORTH/WEST/EAST.
// Targets are captured at beat start ("target" is the spot's facing,
// "away" its opposite, "other" the walker the beat reacts to).
enum AmbienceTarget
{
    AMBIENCE_TARGET_DOWN = 1,
    AMBIENCE_TARGET_UP,
    AMBIENCE_TARGET_LEFT,
    AMBIENCE_TARGET_RIGHT,
    AMBIENCE_TARGET_PLAYER,
    AMBIENCE_TARGET_OTHER,
    AMBIENCE_TARGET_TARGET,
    AMBIENCE_TARGET_AWAY,
    AMBIENCE_TARGET_COMPANION,
};

// One slow step. AHEAD walks the current facing; LEFT and RIGHT walk west and
// east (turning that way); BACK undoes the last unreturned step, or with none
// steps backwards. BACK keeps the facing. The generator checks every beat
// stays within 1 tile of its start and ends on it.
enum AmbienceStep
{
    AMBIENCE_STEP_AHEAD,
    AMBIENCE_STEP_BACK,
    AMBIENCE_STEP_LEFT,
    AMBIENCE_STEP_RIGHT,
};

enum AmbienceObject
{
    AMBIENCE_OBJECT_NURSE,
    AMBIENCE_OBJECT_NPC,
};

// Icons: the first five are movement-action emotes, the rest FLDEFF_EMOTE
// frames of the follower emote sheet. Packed 4 bits in EMOTE_PARITY.
enum AmbienceIcon
{
    AMBIENCE_ICON_EXCLAMATION,   // !
    AMBIENCE_ICON_DOUBLE_EXCL,   // !!
    AMBIENCE_ICON_QUESTION,      // ?
    AMBIENCE_ICON_X,             // X
    AMBIENCE_ICON_HEART,         // heart
    AMBIENCE_ICON_ELLIPSIS,      // ...
    AMBIENCE_ICON_HAPPY,
    AMBIENCE_ICON_MUSIC,
    AMBIENCE_ICON_LOVE,
    AMBIENCE_ICON_CURIOUS,
    AMBIENCE_ICON_PENSIVE,
    AMBIENCE_ICON_SAD,
    AMBIENCE_ICON_ANGRY,
    AMBIENCE_ICON_SURPRISE,
    AMBIENCE_ICON_COUNT,
};

#define AMBIENCE_ICON_FIRST_FLDEFF AMBIENCE_ICON_ELLIPSIS

enum AmbienceEffect
{
    AMBIENCE_EFFECT_RIPPLE,       // FLDEFF_RIPPLE
    AMBIENCE_EFFECT_SPLASH,       // FLDEFF_SPLASH
    AMBIENCE_EFFECT_GRASS_SHAKE,  // FLDEFF_SHAKING_GRASS
    AMBIENCE_EFFECT_DUST,         // FLDEFF_DUST
    AMBIENCE_EFFECT_SPARKLE,      // FLDEFF_SPARKLE
};

enum AmbienceWhere
{
    AMBIENCE_WHERE_AHEAD,  // the faced tile
    AMBIENCE_WHERE_OWN,    // the walker's own tile
};

#define AMBIENCE_EFFECT_MASK   0x0F
#define AMBIENCE_WHERE_SHIFT   4
#define AMBIENCE_PARITY_SHIFT  4

enum AmbienceCompanionAct
{
    AMBIENCE_COMPANION_ACT_FACE,  // the companion faces the walker
    AMBIENCE_COMPANION_ACT_JUMP,
};

// gWayfarerAmbienceRelations values. In a beat, AMBIENCE_RELATION_NONE means
// "any other walker" for AMBIENCE_FACT_NOTABLE.
enum AmbienceRelation
{
    AMBIENCE_RELATION_NONE,
    AMBIENCE_RELATION_FAMILY,
    AMBIENCE_RELATION_FRIEND,
    AMBIENCE_RELATION_RIVAL,
    AMBIENCE_RELATION_COLLEAGUE,
};

// Trainer tags (bits of tags, tagsAny, tagsNone).
#define AMBIENCE_TAG_ATHLETIC  (1 << 0)
#define AMBIENCE_TAG_CHEERFUL  (1 << 1)
#define AMBIENCE_TAG_CURIOUS   (1 << 2)
#define AMBIENCE_TAG_DREAMY    (1 << 3)
#define AMBIENCE_TAG_MYSTIC    (1 << 4)
#define AMBIENCE_TAG_ELEGANT   (1 << 5)
#define AMBIENCE_TAG_WATER     (1 << 6)
#define AMBIENCE_TAG_ELDER     (1 << 7)
#define AMBIENCE_TAG_NIMBLE    (1 << 8)
#define AMBIENCE_TAG_COCKY     (1 << 9)
#define AMBIENCE_TAG_STOIC     (1 << 10)

// Context facts (bits of a beat's all and any masks). A beat holds when every
// fact in `all` holds and, if `any` is not 0, at least one fact in `any`.
// SPOT and NOT_SPOT hold only while dwelling. Their parameters live in the
// beat: spotKinds/spotNamed, notSpotKinds, activities, playerWithin,
// adjacentTicks, notableWithin and relation.
#define AMBIENCE_FACT_WALKING          (1 << 0)
#define AMBIENCE_FACT_DWELLING         (1 << 1)
#define AMBIENCE_FACT_ARRIVING         (1 << 2)
#define AMBIENCE_FACT_LEAVING          (1 << 3)
#define AMBIENCE_FACT_BLOCKED          (1 << 4)
#define AMBIENCE_FACT_OUTDOORS         (1 << 5)
#define AMBIENCE_FACT_COMPANION_ROOM   (1 << 6)
#define AMBIENCE_FACT_FACING_WATER     (1 << 7)
#define AMBIENCE_FACT_FACING_GRASS     (1 << 8)
#define AMBIENCE_FACT_FACING_NPC       (1 << 9)
#define AMBIENCE_FACT_FACING_COUNTER   (1 << 10)
#define AMBIENCE_FACT_SPOT             (1 << 11) // dwelling at a spot of spotKinds, or a named spot offering one of spotNamed
#define AMBIENCE_FACT_NOT_SPOT         (1 << 12) // dwelling at a spot not of notSpotKinds
#define AMBIENCE_FACT_ACTIVITY         (1 << 13) // the current activity is one of activities
#define AMBIENCE_FACT_PLAYER_WITHIN    (1 << 14) // the player within playerWithin tiles (Manhattan) and in view
#define AMBIENCE_FACT_PLAYER_ADJACENT  (1 << 15) // the player adjacent, facing the walker, for adjacentTicks ticks
#define AMBIENCE_FACT_NOTABLE          (1 << 16) // another walker within notableWithin tiles, of relation (NONE = any)

// Beat flags.
#define AMBIENCE_FLAG_KEEP_WALKING      (1 << 0) // runs without stopping the walk (hum)
#define AMBIENCE_FLAG_ONCE_PER_APPROACH (1 << 1) // once each time the player comes within range
#define AMBIENCE_FLAG_ONCE_PER_PAIR     (1 << 2) // once per pair per map visit (greetings)
#define AMBIENCE_FLAG_ONCE_PER_EPISODE  (1 << 3) // once per standing-still episode
#define AMBIENCE_FLAG_NEEDS_COMPANION   (1 << 4) // brings the companion out (derived), or who.companion

#endif // GUARD_CONSTANTS_WAYFARER_AMBIENCE_H
