#ifndef GUARD_WAYFARER_STORY_ENCOUNTER_H
#define GUARD_WAYFARER_STORY_ENCOUNTER_H

#include "global.h"
#include "constants/wayfarer_story_encounters.h"

// Authored story entries deliberately use the address of a trainerbattle command's
// argument block.  Trainer id, trainer class, and an object's graphics are not a
// sufficient description of an encounter's story consequences.
enum WayfarerStoryEncounterPolicy
{
    WAYFARER_STORY_POLICY_NONE,
    WAYFARER_STORY_POLICY_ORDINARY,
    WAYFARER_STORY_POLICY_DEFERRED_RIVAL,
    WAYFARER_STORY_POLICY_OBJECTIVE_GUARD,
    WAYFARER_STORY_POLICY_EXCEPTION,
};

enum WayfarerStoryDialogue
{
    WAYFARER_STORY_DIALOGUE_ORDINARY,
    WAYFARER_STORY_DIALOGUE_RIVAL,
    WAYFARER_STORY_DIALOGUE_ROCKET_GUARD,
    WAYFARER_STORY_DIALOGUE_DISGUISED_DIRECTOR,
    WAYFARER_STORY_DIALOGUE_AQUA_GUARD,
    WAYFARER_STORY_DIALOGUE_MAGMA_GUARD,
    WAYFARER_STORY_DIALOGUE_BIKER_GUARD,
    WAYFARER_STORY_DIALOGUE_TRIAL,
    WAYFARER_STORY_DIALOGUE_GYM,
    WAYFARER_STORY_DIALOGUE_STERN,
    WAYFARER_STORY_DIALOGUE_PARTNER,
    WAYFARER_STORY_DIALOGUE_FOSSIL,
    WAYFARER_STORY_DIALOGUE_RECRUITER,
    WAYFARER_STORY_DIALOGUE_GIDEON,
    WAYFARER_STORY_DIALOGUE_SELPHY,
    WAYFARER_STORY_DIALOGUE_LEAGUE,
    WAYFARER_STORY_DIALOGUE_INVESTIGATE,
    WAYFARER_STORY_DIALOGUE_RETREAT,
};

enum WayfarerStoryEncounterFlags
{
    // A supported non-victory returns to lossRedirect before an authored success script.
    WAYFARER_STORY_FLAG_LOSS_RETURN = (1 << 0),
    // This map object is a battle-only rival and can be removed without its flag.
    WAYFARER_STORY_FLAG_TRANSIENT_OBJECT = (1 << 1),
    // Suppress a registered MAP_SCRIPT_ON_FRAME script while the scene is deferred.
    WAYFARER_STORY_FLAG_SUPPRESS_ON_FRAME = (1 << 2),
    // After a refusal, recovery must wait for the player to leave this trigger tile.
    WAYFARER_STORY_FLAG_REARM_ON_LEAVE = (1 << 3),
    // This exact base trainer caller preserves its existing after-battle talk
    // once its trainer flag is set. Rematch callers intentionally omit it.
    WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT = (1 << 4),
    // A script-provisioned rival object is removed without a canonical flag
    // while unprotected, but its own trigger (rather than map reconciliation)
    // is solely responsible for the next spawn. Do not combine with TRANSIENT.
    WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE = (1 << 5),
};

// Script-side result constants from constants/wayfarer_story_encounters.h:
// DEFER differs from a refusal because battle-only rivals and recovered trigger
// tiles must release silently until the player leaves the activation area.

#define WAYFARER_STORY_NO_COORD (-1)
#define WAYFARER_STORY_ANY_ELEVATION 0xFF

struct WayfarerStoryEncounterDescriptor
{
    // The descriptor deliberately excludes a caller. Regional rival and player
    // variants share every remaining field, while callers retain source order.
    const u8 *lossRedirect;
    const u8 *triggerScript;
    u16 stableKey;
    u16 sceneId;
    u8 policy;
    u8 dialogue;
    u8 flags;
    u8 mapGroup;
    u8 mapNum;
    u8 localId;
    u8 elevation;
    u8 activationWidth;
    u8 activationHeight;
    s16 x;
    s16 y;
    bool8 (*isNarrativelyEligible)(void);
};

struct WayfarerStoryEncounter
{
    // Exact pointer passed to TrainerBattleLoadArgs: the assembler call label + 1.
    const u8 *caller;
    // A per-scene retry/retreat script. Valid only with LOSS_RETURN.
    const u8 *lossRedirect;
    // Optional coordinate or ON_FRAME script associated with the scene.
    const u8 *triggerScript;
    // Stable dialogue assignment. Rematches must reuse their base caller's key.
    u16 stableKey;
    // Script-side staging key (read from VAR_0x8004 by CanStartScene).
    u16 sceneId;
    u8 policy;
    u8 dialogue;
    u8 flags;
    u8 mapGroup;
    u8 mapNum;
    u8 localId;
    // Map-event coordinates (without MAP_OFFSET). Width/height of zero means a
    // one-tile activation area. ANY_ELEVATION is permitted for script-only gates.
    u8 elevation;
    u8 activationWidth;
    u8 activationHeight;
    s16 x;
    s16 y;
    // A rival/object may only be hidden while its chapter itself remains pending.
    bool8 (*isNarrativelyEligible)(void);
};

// Registry construction is intentionally split by scope.  The ordinary manifest
// is owned by the root integration; each region owns only its own data include.
extern const u8 *const gWayfarerStoryOrdinaryCallers[];
extern const u8 gWayfarerStoryOrdinaryMetadata[];
extern const u32 gWayfarerStoryOrdinaryEncounterCount;
extern const struct WayfarerStoryEncounterDescriptor gWayfarerStoryJohtoDescriptors[];
extern const u32 gWayfarerStoryJohtoDescriptorCount;
extern const u8 *const gWayfarerStoryJohtoCallers[];
extern const u8 gWayfarerStoryJohtoDescriptorIndices[];
extern const u32 gWayfarerStoryJohtoEncounterCount;
extern const struct WayfarerStoryEncounterDescriptor gWayfarerStoryHoennDescriptors[];
extern const u32 gWayfarerStoryHoennDescriptorCount;
extern const u8 *const gWayfarerStoryHoennCallers[];
extern const u8 gWayfarerStoryHoennDescriptorIndices[];
extern const u32 gWayfarerStoryHoennEncounterCount;

bool8 WayfarerStoryFindCaller(const u8 *caller, struct WayfarerStoryEncounter *out);
bool8 WayfarerStoryFindScene(u16 sceneId, struct WayfarerStoryEncounter *out);

// Called by the trainerbattle command and TrainerBattleLoadArgs respectively.
// They are deliberately separate: entry refusal must happen before script staging,
// while loss redirection is only meaningful after trainer parameters are loaded.
bool8 WayfarerStoryTryStartTrainerBattle(const u8 *caller);
void WayfarerStoryConfigureTrainerBattleCaller(const u8 *caller);

// Script specials. CanStartScene reads VAR_0x8004 and returns a gate result.
u16 WayfarerStoryCanStartScene(void);
u16 WayfarerStoryShowSelectedNoPartyDialogue(void);
u16 WayfarerStoryShowRetreatDialogue(void);

// Object/trigger lifecycle hooks.  These only affect explicit registry entries.
bool8 WayfarerStoryCanSpawnObject(u8 localId, u8 mapNum, u8 mapGroup);
void WayfarerStoryReconcileCurrentMap(void);
bool8 WayfarerStoryShouldSuppressOnFrameScript(const u8 *script);
void WayfarerStoryOnPlayerPositionChanged(void);

// Test and script-facing helpers; the latter never set an implicit loss redirect.
bool8 WayfarerStoryCanUseEncounter(const struct WayfarerStoryEncounter *entry);
const u8 *WayfarerStoryGetDialogue(const struct WayfarerStoryEncounter *entry);
bool8 WayfarerStoryShouldSilentlyDeferActiveEncounter(void);

#if TESTING
bool8 Test_WayfarerStoryRegistryIsValid(void);
bool8 Test_WayfarerStoryPreservesCompletedTrainerText(const struct WayfarerStoryEncounter *entry, const u8 *caller);
bool8 Test_WayfarerStoryElevationsAreCompatibleForObjectRestore(u8 a, u8 b);
#endif

#endif
