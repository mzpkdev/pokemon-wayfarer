#include "global.h"
#include "wayfarer_story_encounter.h"

#include "event_data.h"
#include "event_object_movement.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "battle_setup.h"
#include "wayfarer_loss_policy.h"
#include "wayfarer_origin.h"

#include "data/wayfarer_story_encounter_ordinary.h"
#include "data/wayfarer_story_encounter_johto.h"
#include "data/wayfarer_story_encounter_hoenn.h"

struct WayfarerStoryRegistry
{
    const struct WayfarerStoryEncounter *entries;
    u32 count;
};

static const struct WayfarerStoryRegistry sRegistries[] =
{
    {gWayfarerStoryOrdinaryEncounters, gWayfarerStoryOrdinaryEncounterCount},
    {gWayfarerStoryJohtoEncounters, gWayfarerStoryJohtoEncounterCount},
    {gWayfarerStoryHoennEncounters, gWayfarerStoryHoennEncounterCount},
};

// Ordinary callers have no map-object or frame-script lifecycle. Keeping them
// out of these scans prevents the 854-entry ordinary allowlist from becoming an
// overworld per-frame cost.
static const struct WayfarerStoryRegistry sFieldRegistries[] =
{
    {gWayfarerStoryJohtoEncounters, gWayfarerStoryJohtoEncounterCount},
    {gWayfarerStoryHoennEncounters, gWayfarerStoryHoennEncounterCount},
};

static EWRAM_DATA const struct WayfarerStoryEncounter *sActiveEncounter = NULL;
static EWRAM_DATA u16 sRearmSceneId = 0;
static EWRAM_DATA bool8 sUsablePartyStateKnown = FALSE;
static EWRAM_DATA bool8 sHadUsableParty = FALSE;
static EWRAM_DATA bool8 sPlayerPositionKnown = FALSE;
static EWRAM_DATA s16 sLastPlayerX;
static EWRAM_DATA s16 sLastPlayerY;
static EWRAM_DATA s16 sLastPlayerDestX;
static EWRAM_DATA s16 sLastPlayerDestY;
static EWRAM_DATA u8 sLastPlayerElevation;
static EWRAM_DATA u8 sLastMapGroup;
static EWRAM_DATA u8 sLastMapNum;

static const u8 sText_OrdinaryNeutral[] = _("Come back when you have a\nPOKéMON that can battle.");
static const u8 sText_OrdinaryRelaxed[] = _("No POKéMON ready? We can\nbattle another time.");
static const u8 sText_OrdinaryEager[] = _("I was hoping for a battle!\nMaybe next time.");
static const u8 sText_OrdinaryKind[] = _("Don't worry about a battle.\nTake care out there!");
static const u8 sText_Rival[] = _("We'll battle when you've got\na POKéMON ready.");
static const u8 sText_RocketGuard[] = _("You've got no POKéMON to\nstop us. Get lost!");
static const u8 sText_DisguisedDirector[] = _("This is no place for you\nwithout a POKéMON. Leave.");
static const u8 sText_AquaGuard[] = _("You're in over your head.\nBeat it!");
static const u8 sText_MagmaGuard[] = _("Stay out of our way.");
static const u8 sText_BikerGuard[] = _("Keep walking, unless you\nwant trouble!");
static const u8 sText_Trial[] = _("Return with a POKéMON that\ncan battle. The trial will wait.");
static const u8 sText_Gym[] = _("Come back with a POKéMON\nready to challenge this GYM.");
static const u8 sText_Stern[] = _("Please bring a POKéMON that\ncan battle before we handle\nthese parts.");
static const u8 sText_Partner[] = _("We need a POKéMON that can\nbattle before we take them on.");
static const u8 sText_Fossil[] = _("Want a fossil? Bring a\nPOKéMON that can battle first!");
static const u8 sText_Recruiter[] = _("Come back with a POKéMON\nready to battle. Then we'll talk.");
static const u8 sText_Gideon[] = _("The SAPPHIRE stays with me.\nNow get out!");
static const u8 sText_Selphy[] = _("A battle? Come back with a\nPOKéMON that's ready!");
static const u8 sText_League[] = _("You need a POKéMON that can\nbattle before you enter this\nchallenge.");
static const u8 sText_Investigate[] = _("You need a POKéMON that can\nbattle before you investigate.");
static const u8 sText_Retreat[] = _("You can't keep battling.\nYou step back.");

static bool8 IsCurrentMap(const struct WayfarerStoryEncounter *entry)
{
    return entry->mapGroup == gSaveBlock1Ptr->location.mapGroup
        && entry->mapNum == gSaveBlock1Ptr->location.mapNum;
}

static bool8 IsNarrativelyEligible(const struct WayfarerStoryEncounter *entry)
{
    return entry->isNarrativelyEligible == NULL || entry->isNarrativelyEligible();
}

static bool8 IsDeferredHideableObject(const struct WayfarerStoryEncounter *entry)
{
    return entry->policy == WAYFARER_STORY_POLICY_DEFERRED_RIVAL
        && (entry->flags & (WAYFARER_STORY_FLAG_TRANSIENT_OBJECT | WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE));
}

static bool8 IsRestorableTransientObject(const struct WayfarerStoryEncounter *entry)
{
    return entry->policy == WAYFARER_STORY_POLICY_DEFERRED_RIVAL
        && (entry->flags & WAYFARER_STORY_FLAG_TRANSIENT_OBJECT);
}

static bool8 AreElevationsCompatibleForObjectRestore(u8 a, u8 b)
{
    return a == ELEVATION_TRANSITION || b == ELEVATION_TRANSITION || a == b;
}

static bool8 IsPlayerStandingAtObjectTemplate(const struct WayfarerStoryEncounter *entry)
{
    s16 destX;
    s16 destY;
    const struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    const struct ObjectEventTemplate *objectTemplate = GetObjectEventTemplateByLocalIdAndMap(entry->localId, entry->mapNum, entry->mapGroup);

    // Keep the same transition-elevation compatibility as object collision. A
    // transition template must not be restored onto a player on its matching
    // tile merely because the player has a concrete current elevation.
    if (objectTemplate == NULL
     || !AreElevationsCompatibleForObjectRestore(objectTemplate->elevation, player->currentElevation))
        return FALSE;
    PlayerGetDestCoords(&destX, &destY);
    return (destX - MAP_OFFSET == objectTemplate->x && destY - MAP_OFFSET == objectTemplate->y)
        || (player->currentCoords.x - MAP_OFFSET == objectTemplate->x
         && player->currentCoords.y - MAP_OFFSET == objectTemplate->y);
}

static bool8 PlayerPositionChanged(void)
{
    const struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 destX;
    s16 destY;
    bool8 changed;

    PlayerGetDestCoords(&destX, &destY);
    changed = !sPlayerPositionKnown
        || sLastPlayerX != player->currentCoords.x
        || sLastPlayerY != player->currentCoords.y
        || sLastPlayerDestX != destX
        || sLastPlayerDestY != destY
        || sLastPlayerElevation != player->currentElevation
        || sLastMapGroup != gSaveBlock1Ptr->location.mapGroup
        || sLastMapNum != gSaveBlock1Ptr->location.mapNum;
    sPlayerPositionKnown = TRUE;
    sLastPlayerX = player->currentCoords.x;
    sLastPlayerY = player->currentCoords.y;
    sLastPlayerDestX = destX;
    sLastPlayerDestY = destY;
    sLastPlayerElevation = player->currentElevation;
    sLastMapGroup = gSaveBlock1Ptr->location.mapGroup;
    sLastMapNum = gSaveBlock1Ptr->location.mapNum;
    return changed;
}

static bool8 IsPlayerInActivationArea(const struct WayfarerStoryEncounter *entry)
{
    const struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    s16 destX;
    s16 destY;
    s16 width = max(1, entry->activationWidth);
    s16 height = max(1, entry->activationHeight);

    if (entry->x == WAYFARER_STORY_NO_COORD || entry->y == WAYFARER_STORY_NO_COORD)
        return FALSE;
    PlayerGetDestCoords(&destX, &destY);
    if (entry->elevation != WAYFARER_STORY_ANY_ELEVATION && entry->elevation != player->currentElevation)
        return FALSE;
    return ((destX - MAP_OFFSET >= entry->x && destX - MAP_OFFSET < entry->x + width
          && destY - MAP_OFFSET >= entry->y && destY - MAP_OFFSET < entry->y + height)
         || (player->currentCoords.x - MAP_OFFSET >= entry->x && player->currentCoords.x - MAP_OFFSET < entry->x + width
          && player->currentCoords.y - MAP_OFFSET >= entry->y && player->currentCoords.y - MAP_OFFSET < entry->y + height));
}

static void NoteNoPartyRefusal(const struct WayfarerStoryEncounter *entry)
{
    sActiveEncounter = entry;
    if (entry != NULL && (entry->flags & WAYFARER_STORY_FLAG_REARM_ON_LEAVE))
        sRearmSceneId = entry->sceneId;
}

static bool8 PreservesCompletedTrainerText(const struct WayfarerStoryEncounter *entry, const u8 *caller)
{
    TrainerBattleParameter trainerBattle;

    if (!(entry->flags & WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT))
        return FALSE;
    memcpy(trainerBattle.data, caller, sizeof(trainerBattle));
    return HasTrainerBeenFought(trainerBattle.params.opponentA);
}

const struct WayfarerStoryEncounter *WayfarerStoryFindCaller(const u8 *caller)
{
    u32 registry;

    // Coordinate-only entries deliberately have a NULL caller. A missing caller
    // must remain unauthorized rather than accidentally selecting the first one.
    if (caller == NULL)
        return NULL;
    for (registry = 0; registry < ARRAY_COUNT(sRegistries); registry++)
    {
        u32 i;
        for (i = 0; i < sRegistries[registry].count; i++)
        {
            const struct WayfarerStoryEncounter *entry = &sRegistries[registry].entries[i];
            if (entry->caller == caller)
                return entry;
        }
    }
    return NULL;
}

const struct WayfarerStoryEncounter *WayfarerStoryFindScene(u16 sceneId)
{
    u32 registry;

    if (sceneId == 0)
        return NULL;
    for (registry = 0; registry < ARRAY_COUNT(sRegistries); registry++)
    {
        u32 i;
        for (i = 0; i < sRegistries[registry].count; i++)
        {
            const struct WayfarerStoryEncounter *entry = &sRegistries[registry].entries[i];
            if (entry->sceneId == sceneId)
                return entry;
        }
    }
    return NULL;
}

bool8 WayfarerStoryCanUseEncounter(const struct WayfarerStoryEncounter *entry)
{
#if IS_WAYFARER
    return entry != NULL && IsNarrativelyEligible(entry) && WayfarerCanStartOrdinaryBattle();
#else
    return TRUE;
#endif
}

bool8 WayfarerStoryTryStartTrainerBattle(const u8 *caller)
{
#if IS_WAYFARER
    const struct WayfarerStoryEncounter *entry = WayfarerStoryFindCaller(caller);

    // An omitted caller is deliberately not reclassified. Its existing script owns
    // its exceptional/tutorial/challenge behavior until it is explicitly audited.
    if (entry == NULL)
        return TRUE;

    // A retired deferred rival must never revive a chapter simply because a
    // stale map script reached its former trainerbattle command.
    if (!IsNarrativelyEligible(entry)
     && entry->policy == WAYFARER_STORY_POLICY_DEFERRED_RIVAL)
    {
        sActiveEncounter = entry;
        return FALSE;
    }

    if (WayfarerCanStartOrdinaryBattle())
        return TRUE;

    // A completed ordinary trainer's base caller needs to reach the existing
    // trainer-flag branch so it can show its authored after-battle text. This is
    // opt-in per exact caller; a ready rematch has no such exemption.
    if (PreservesCompletedTrainerText(entry, caller))
        return TRUE;

    // Objective and challenge scripts retain their own explicit state routing.
    if (!IsNarrativelyEligible(entry))
        return TRUE;

    NoteNoPartyRefusal(entry);
    return FALSE;
#else
    return TRUE;
#endif
}

void WayfarerStoryConfigureTrainerBattleCaller(const u8 *caller)
{
#if IS_WAYFARER
    const struct WayfarerStoryEncounter *entry = WayfarerStoryFindCaller(caller);

    if (entry == NULL)
        return;
    sActiveEncounter = entry;
    if ((entry->flags & WAYFARER_STORY_FLAG_LOSS_RETURN) && entry->lossRedirect != NULL)
        WayfarerSetTrainerLossRedirect(entry->lossRedirect);
#endif
}

u16 WayfarerStoryCanStartScene(void)
{
#if IS_WAYFARER
    const struct WayfarerStoryEncounter *entry = WayfarerStoryFindScene(gSpecialVar_0x8004);

    // Script authors only call this for an explicit scene id. Treating an unknown
    // id as eligible preserves the caller's current behavior instead of hiding an
    // unaudited sequence because of a typo or an unported map.
    if (entry == NULL)
        return WAYFARER_STORY_GATE_ALLOW;
    if (!IsNarrativelyEligible(entry))
        return entry->policy == WAYFARER_STORY_POLICY_DEFERRED_RIVAL
            ? WAYFARER_STORY_GATE_DEFER
            : WAYFARER_STORY_GATE_ALLOW;
    if (sRearmSceneId == entry->sceneId && IsCurrentMap(entry) && IsPlayerInActivationArea(entry))
        return WAYFARER_STORY_GATE_DEFER;
    if (WayfarerCanStartOrdinaryBattle())
        return WAYFARER_STORY_GATE_ALLOW;

    NoteNoPartyRefusal(entry);
    return entry->policy == WAYFARER_STORY_POLICY_DEFERRED_RIVAL
        ? WAYFARER_STORY_GATE_DEFER
        : WAYFARER_STORY_GATE_REFUSE;
#else
    return WAYFARER_STORY_GATE_ALLOW;
#endif
}

const u8 *WayfarerStoryGetDialogue(const struct WayfarerStoryEncounter *entry)
{
    static const u8 *const sOrdinaryDialogue[] =
    {
        sText_OrdinaryNeutral,
        sText_OrdinaryRelaxed,
        sText_OrdinaryEager,
        sText_OrdinaryKind,
    };
    u8 dialogue = entry == NULL ? WAYFARER_STORY_DIALOGUE_ORDINARY : entry->dialogue;

    switch (dialogue)
    {
    case WAYFARER_STORY_DIALOGUE_ORDINARY:
        return sOrdinaryDialogue[(entry == NULL ? 0 : entry->stableKey) % ARRAY_COUNT(sOrdinaryDialogue)];
    case WAYFARER_STORY_DIALOGUE_RIVAL:
        return sText_Rival;
    case WAYFARER_STORY_DIALOGUE_ROCKET_GUARD:
        return sText_RocketGuard;
    case WAYFARER_STORY_DIALOGUE_DISGUISED_DIRECTOR:
        return sText_DisguisedDirector;
    case WAYFARER_STORY_DIALOGUE_AQUA_GUARD:
        return sText_AquaGuard;
    case WAYFARER_STORY_DIALOGUE_MAGMA_GUARD:
        return sText_MagmaGuard;
    case WAYFARER_STORY_DIALOGUE_BIKER_GUARD:
        return sText_BikerGuard;
    case WAYFARER_STORY_DIALOGUE_TRIAL:
        return sText_Trial;
    case WAYFARER_STORY_DIALOGUE_GYM:
        return sText_Gym;
    case WAYFARER_STORY_DIALOGUE_STERN:
        return sText_Stern;
    case WAYFARER_STORY_DIALOGUE_PARTNER:
        return sText_Partner;
    case WAYFARER_STORY_DIALOGUE_FOSSIL:
        return sText_Fossil;
    case WAYFARER_STORY_DIALOGUE_RECRUITER:
        return sText_Recruiter;
    case WAYFARER_STORY_DIALOGUE_GIDEON:
        return sText_Gideon;
    case WAYFARER_STORY_DIALOGUE_SELPHY:
        return sText_Selphy;
    case WAYFARER_STORY_DIALOGUE_LEAGUE:
        return sText_League;
    case WAYFARER_STORY_DIALOGUE_INVESTIGATE:
        return sText_Investigate;
    case WAYFARER_STORY_DIALOGUE_RETREAT:
        return sText_Retreat;
    default:
        return sText_OrdinaryNeutral;
    }
}

u16 WayfarerStoryShowSelectedNoPartyDialogue(void)
{
#if IS_WAYFARER
    ShowFieldMessage(WayfarerStoryGetDialogue(sActiveEncounter));
#endif
    return 0;
}

u16 WayfarerStoryShowRetreatDialogue(void)
{
#if IS_WAYFARER
    ShowFieldMessage(sText_Retreat);
#endif
    return 0;
}

bool8 WayfarerStoryShouldSilentlyDeferActiveEncounter(void)
{
#if IS_WAYFARER
    return sActiveEncounter != NULL
        && sActiveEncounter->policy == WAYFARER_STORY_POLICY_DEFERRED_RIVAL;
#else
    return FALSE;
#endif
}

bool8 WayfarerStoryCanSpawnObject(u8 localId, u8 mapNum, u8 mapGroup)
{
#if IS_WAYFARER
    u32 registry;
    bool8 hasUsableParty = WayfarerCanStartOrdinaryBattle();

    for (registry = 0; registry < ARRAY_COUNT(sFieldRegistries); registry++)
    {
        u32 i;
        for (i = 0; i < sFieldRegistries[registry].count; i++)
        {
            const struct WayfarerStoryEncounter *entry = &sFieldRegistries[registry].entries[i];
            if (IsDeferredHideableObject(entry)
             && entry->localId == localId
             && entry->mapNum == mapNum
             && entry->mapGroup == mapGroup
             && IsNarrativelyEligible(entry))
            {
                if (!hasUsableParty)
                    return FALSE;
                if (IsRestorableTransientObject(entry)
                 && IsCurrentMap(entry)
                 && IsPlayerStandingAtObjectTemplate(entry))
                    return FALSE;
            }
        }
    }
#endif
    return TRUE;
}

void WayfarerStoryReconcileCurrentMap(void)
{
#if IS_WAYFARER
    u32 registry;
    bool8 hasUsableParty = WayfarerCanStartOrdinaryBattle();

    sUsablePartyStateKnown = TRUE;
    sHadUsableParty = hasUsableParty;
    for (registry = 0; registry < ARRAY_COUNT(sFieldRegistries); registry++)
    {
        u32 i;
        for (i = 0; i < sFieldRegistries[registry].count; i++)
        {
            const struct WayfarerStoryEncounter *entry = &sFieldRegistries[registry].entries[i];
            u8 objectEventId;

            if (!IsDeferredHideableObject(entry) || !IsCurrentMap(entry) || !IsNarrativelyEligible(entry))
                continue;
            if (!hasUsableParty)
            {
                if (!TryGetObjectEventIdByLocalIdAndMap(entry->localId, entry->mapNum, entry->mapGroup, &objectEventId))
                    RemoveObjectEvent(&gObjectEvents[objectEventId]);
            }
            else if (IsRestorableTransientObject(entry)
             && !IsPlayerStandingAtObjectTemplate(entry)
             && TryGetObjectEventIdByLocalIdAndMap(entry->localId, entry->mapNum, entry->mapGroup, &objectEventId))
            {
                // This is the raw spawn path, so it neither changes the object's
                // canonical flag nor recreates it over a saved player position.
                TrySpawnObjectEvent(entry->localId, entry->mapNum, entry->mapGroup);
            }
        }
    }
#endif
}

bool8 WayfarerStoryShouldSuppressOnFrameScript(const u8 *script)
{
#if IS_WAYFARER
    u32 registry;

    for (registry = 0; registry < ARRAY_COUNT(sFieldRegistries); registry++)
    {
        u32 i;
        for (i = 0; i < sFieldRegistries[registry].count; i++)
        {
            const struct WayfarerStoryEncounter *entry = &sFieldRegistries[registry].entries[i];
            if (entry->triggerScript != script || !IsCurrentMap(entry))
                continue;
            // A visible objective/tutorial must run once to select and show its
            // refusal; REARM then prevents its map frame table from reopening it
            // until the player leaves the full authored area. Silent off-screen
            // rival scripts opt into suppression even before their first call.
            if (sRearmSceneId == entry->sceneId && IsPlayerInActivationArea(entry))
                return TRUE;
            if ((entry->flags & WAYFARER_STORY_FLAG_SUPPRESS_ON_FRAME)
             && !WayfarerCanStartOrdinaryBattle()
             && IsNarrativelyEligible(entry))
                return TRUE;
        }
    }
#endif
    return FALSE;
}

void WayfarerStoryOnPlayerPositionChanged(void)
{
#if IS_WAYFARER
    bool8 hasUsableParty = WayfarerCanStartOrdinaryBattle();
    bool8 positionChanged = PlayerPositionChanged();

    if (sRearmSceneId != 0 && positionChanged)
    {
        const struct WayfarerStoryEncounter *entry = WayfarerStoryFindScene(sRearmSceneId);
        if (entry == NULL || !IsCurrentMap(entry) || !IsPlayerInActivationArea(entry))
            sRearmSceneId = 0;
    }
    if (!sUsablePartyStateKnown || sHadUsableParty != hasUsableParty)
        WayfarerStoryReconcileCurrentMap();
    else if (hasUsableParty && positionChanged)
        // A recovered rival is deliberately not spawned on the player's current
        // or destination tile. Recheck the bounded explicit registry as movement
        // carries the player away from that tile.
        WayfarerStoryReconcileCurrentMap();
#endif
}

#if TESTING
bool8 Test_WayfarerStoryPreservesCompletedTrainerText(const struct WayfarerStoryEncounter *entry, const u8 *caller)
{
    return PreservesCompletedTrainerText(entry, caller);
}

bool8 Test_WayfarerStoryElevationsAreCompatibleForObjectRestore(u8 a, u8 b)
{
    return AreElevationsCompatibleForObjectRestore(a, b);
}

bool8 Test_WayfarerStoryRegistryIsValid(void)
{
    u32 registry;

    for (registry = 0; registry < ARRAY_COUNT(sRegistries); registry++)
    {
        u32 i;
        for (i = 0; i < sRegistries[registry].count; i++)
        {
            const struct WayfarerStoryEncounter *entry = &sRegistries[registry].entries[i];
            u32 otherRegistry;

            if (entry->policy == WAYFARER_STORY_POLICY_NONE
             || ((entry->flags & WAYFARER_STORY_FLAG_LOSS_RETURN) && entry->lossRedirect == NULL)
             || (!(entry->flags & WAYFARER_STORY_FLAG_LOSS_RETURN) && entry->lossRedirect != NULL)
             || ((entry->flags & WAYFARER_STORY_FLAG_TRANSIENT_OBJECT)
              && (entry->flags & WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE))
             || ((entry->flags & (WAYFARER_STORY_FLAG_TRANSIENT_OBJECT | WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE))
              && (entry->localId == 0 || entry->policy != WAYFARER_STORY_POLICY_DEFERRED_RIVAL)))
                return FALSE;
            if (entry->caller == NULL)
                continue; // Scripted-wild/coordinate-only entries have no trainer caller.
            for (otherRegistry = registry; otherRegistry < ARRAY_COUNT(sRegistries); otherRegistry++)
            {
                u32 j = otherRegistry == registry ? i + 1 : 0;
                for (; j < sRegistries[otherRegistry].count; j++)
                    if (entry->caller == sRegistries[otherRegistry].entries[j].caller)
                        return FALSE;
            }
        }
    }
    return TRUE;
}
#endif
