#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "string_util.h"
#include "wayfarer_loss_policy.h"
#include "wayfarer_story_encounter.h"
#include "constants/battle_setup.h"
#include "constants/trainers.h"
#include "constants/wayfarer_persistence.h"
#include "constants/wayfarer_story_encounters.h"
#include "test/test.h"

#if IS_WAYFARER
TEST("Wayfarer story encounter manifests keep caller routing explicit")
{
    static const u8 sUnauditedCaller[] = {0};
    struct WayfarerStoryEncounter resolved;

    EXPECT(Test_WayfarerStoryRegistryIsValid());
    EXPECT(!WayfarerStoryFindCaller(NULL, &resolved));
    EXPECT(!WayfarerStoryFindCaller(sUnauditedCaller, &resolved));
    WayfarerResetLossContext();
    WayfarerStoryConfigureTrainerBattleCaller(sUnauditedCaller);
    EXPECT(WayfarerGetTrainerLossRedirect() == NULL);
}

TEST("Wayfarer story compact ordinary callers retain exact routing and independent lookup values")
{
    extern const u8 EventScript_WayfarerStoryLossRetreat[];
    struct WayfarerStoryEncounter first, resolved;

    EXPECT_EQ(sizeof(struct WayfarerStoryEncounterDescriptor), 32);
    EXPECT_EQ(gWayfarerStoryOrdinaryEncounterCount, 851);
    EXPECT(WayfarerStoryFindCaller(gWayfarerStoryOrdinaryCallers[0], &first));
    for (u32 i = 0; i < gWayfarerStoryOrdinaryEncounterCount; i++)
    {
        u8 metadata = gWayfarerStoryOrdinaryMetadata[i];
        u8 expectedDialogue = metadata & 3;
        u8 expectedFlags = ((metadata & 0x10) ? WAYFARER_STORY_FLAG_LOSS_RETURN : 0)
            | ((metadata & 0x20) ? WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT : 0);

        EXPECT(WayfarerStoryFindCaller(gWayfarerStoryOrdinaryCallers[i], &resolved));
        EXPECT(resolved.caller == gWayfarerStoryOrdinaryCallers[i]);
        EXPECT_EQ(resolved.stableKey, (metadata >> 2) & 3);
        if (expectedDialogue == 1)
            expectedDialogue = WAYFARER_STORY_DIALOGUE_AQUA_GUARD;
        else if (expectedDialogue == 2)
            expectedDialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD;
        else if (expectedDialogue == 3)
            expectedDialogue = WAYFARER_STORY_DIALOGUE_MAGMA_GUARD;
        else
            expectedDialogue = WAYFARER_STORY_DIALOGUE_ORDINARY;
        EXPECT_EQ(resolved.dialogue, expectedDialogue);
        EXPECT_EQ(resolved.flags, expectedFlags);
        EXPECT_EQ(resolved.policy, WAYFARER_STORY_POLICY_ORDINARY);
        EXPECT_EQ(resolved.x, WAYFARER_STORY_NO_COORD);
        EXPECT_EQ(resolved.y, WAYFARER_STORY_NO_COORD);
        EXPECT_EQ(resolved.sceneId, 0);
        EXPECT_EQ(resolved.localId, 0);
        EXPECT(resolved.triggerScript == NULL);
        EXPECT(resolved.lossRedirect == ((expectedFlags & WAYFARER_STORY_FLAG_LOSS_RETURN)
            ? EventScript_WayfarerStoryLossRetreat : NULL));
        EXPECT(first.caller == gWayfarerStoryOrdinaryCallers[0]);
        EXPECT_EQ(first.stableKey, (gWayfarerStoryOrdinaryMetadata[0] >> 2) & 3);
    }
}

TEST("Wayfarer story compact regional scenes preserve first-match order and independent outputs")
{
    extern const u8 CherryGroveCity_Silver_Battle_Cyndaquil[];
    extern const u8 AzaleaTown_Silver_Battle_Cyndaquil[];
    struct WayfarerStoryEncounter cherrygrove;
    struct WayfarerStoryEncounter azalea;

    // Starter variants share each scene. The logical fixture's first caller
    // remains authoritative, and a second lookup must not overwrite the first.
    EXPECT(WayfarerStoryFindScene(WAYFARER_STORY_SCENE_SILVER_CHERRYGROVE, &cherrygrove));
    EXPECT(WayfarerStoryFindScene(WAYFARER_STORY_SCENE_SILVER_AZALEA, &azalea));
    EXPECT(cherrygrove.caller == CherryGroveCity_Silver_Battle_Cyndaquil + 1);
    EXPECT(azalea.caller == AzaleaTown_Silver_Battle_Cyndaquil + 1);
    EXPECT_EQ(cherrygrove.sceneId, WAYFARER_STORY_SCENE_SILVER_CHERRYGROVE);
    EXPECT_EQ(azalea.sceneId, WAYFARER_STORY_SCENE_SILVER_AZALEA);
    EXPECT(!WayfarerStoryFindScene(0, &azalea));
}

TEST("Wayfarer story ordinary refusal dialogue is stable and tutorial-neutral")
{
    struct WayfarerStoryEncounter entry =
    {
        .policy = WAYFARER_STORY_POLICY_ORDINARY,
        .dialogue = WAYFARER_STORY_DIALOGUE_ORDINARY,
        .stableKey = 2,
    };

    EXPECT_EQ(StringCompare(WayfarerStoryGetDialogue(&entry), COMPOUND_STRING("I was hoping for a battle!\nMaybe next time.")), 0);
    entry.stableKey += 4;
    EXPECT_EQ(StringCompare(WayfarerStoryGetDialogue(&entry), COMPOUND_STRING("I was hoping for a battle!\nMaybe next time.")), 0);
    entry.stableKey = 0;
    EXPECT_EQ(StringCompare(WayfarerStoryGetDialogue(&entry), COMPOUND_STRING("Come back when you have a\nPOKéMON that can battle.")), 0);
}

TEST("Wayfarer story entry reuses the canonical usable-party predicate")
{
    struct WayfarerStoryEncounter entry =
    {
        .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD,
        .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD,
    };
    u16 hp = 0;
    bool8 egg = TRUE;

    ZeroPlayerPartyMons();
    EXPECT(!WayfarerStoryCanUseEncounter(&entry));
    CreateMon(&gPlayerParty[0], SPECIES_RATTATA, 5, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[0]);
    gPlayerPartyCount = 1;
    EXPECT(WayfarerStoryCanUseEncounter(&entry));
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &hp);
    EXPECT(!WayfarerStoryCanUseEncounter(&entry));
    HealPlayerParty();
    SetMonData(&gPlayerParty[0], MON_DATA_IS_EGG, &egg);
    EXPECT(!WayfarerStoryCanUseEncounter(&entry));
}

TEST("Wayfarer story ordinary base callers preserve completed trainer text, including doubles")
{
    struct WayfarerStoryEncounter entry =
    {
        .flags = WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT,
    };
    TrainerBattleParameter battle = {0};

    battle.params.opponentA = TRAINER_CHARLIE;
    battle.params.mode = TRAINER_BATTLE_SINGLE;
    ClearTrainerFlag(battle.params.opponentA);
    EXPECT(!Test_WayfarerStoryPreservesCompletedTrainerText(&entry, battle.data));
    SetTrainerFlag(battle.params.opponentA);
    EXPECT(Test_WayfarerStoryPreservesCompletedTrainerText(&entry, battle.data));

    battle.params.mode = TRAINER_BATTLE_DOUBLE;
    EXPECT(Test_WayfarerStoryPreservesCompletedTrainerText(&entry, battle.data));
    entry.flags = 0; // A rematch/unauthorized caller must still be refused.
    EXPECT(!Test_WayfarerStoryPreservesCompletedTrainerText(&entry, battle.data));
    ClearTrainerFlag(battle.params.opponentA);
}

TEST("Wayfarer story transient restore keeps transition elevations collision-safe")
{
    EXPECT(Test_WayfarerStoryElevationsAreCompatibleForObjectRestore(ELEVATION_TRANSITION, ELEVATION_DEFAULT));
    EXPECT(Test_WayfarerStoryElevationsAreCompatibleForObjectRestore(ELEVATION_DEFAULT, ELEVATION_TRANSITION));
    EXPECT(Test_WayfarerStoryElevationsAreCompatibleForObjectRestore(ELEVATION_DEFAULT, ELEVATION_DEFAULT));
    EXPECT(!Test_WayfarerStoryElevationsAreCompatibleForObjectRestore(ELEVATION_DEFAULT, ELEVATION_SURF));
}

TEST("Wayfarer story Hoenn predicates use banked source state and retire completed chapters")
{
    enum
    {
        HNS_GOLDENROD_CITY_STATE = 0x4069,
        HNS_UNDERGROUND_SILVER_HIDE_FLAG = 0x6F,
        HOENN_ROUTE110_STATE = HOENN_VAR_ID(0x4069),
        HOENN_MAGMA_HIDEOUT_COMPLETE = HOENN_FLAG_ID(0x6F),
    };
    struct WayfarerStoryEncounter route110;
    struct WayfarerStoryEncounter magmaHideout;
    bool8 foundRoute110;
    bool8 foundMagmaHideout;
    u16 oldHoennRoute110State = VarGet(HOENN_ROUTE110_STATE);
    u16 oldHnsGoldenrodState = VarGet(HNS_GOLDENROD_CITY_STATE);
    bool8 oldHoennMagmaHideoutComplete = FlagGet(HOENN_MAGMA_HIDEOUT_COMPLETE);
    bool8 oldHnsSilverHideFlag = FlagGet(HNS_UNDERGROUND_SILVER_HIDE_FLAG);

    foundRoute110 = WayfarerStoryFindScene(WAYFARER_STORY_SCENE_ROUTE110_RIVAL, &route110);
    foundMagmaHideout = WayfarerStoryFindScene(WAYFARER_STORY_SCENE_MAGMA_HIDEOUT_MAXIE, &magmaHideout);
    EXPECT(foundRoute110);
    EXPECT(foundMagmaHideout);
    if (foundRoute110 && foundMagmaHideout)
    {
        // 0x4069 is Goldenrod's HNS state. It must not decide Route 110's
        // Emerald chapter, which lives at the banked 0x7069 source ID.
        VarSet(HNS_GOLDENROD_CITY_STATE, 10);
        VarSet(HOENN_ROUTE110_STATE, 0);
        EXPECT(route110.isNarrativelyEligible());
        VarSet(HOENN_ROUTE110_STATE, 1);
        EXPECT(!route110.isNarrativelyEligible());
        EXPECT_EQ(VarGet(HNS_GOLDENROD_CITY_STATE), 10);

        // The HNS flag at source ID 0x6F hides Underground Silver. It must
        // remain independent of Magma Hideout completion at banked 0x606F.
        FlagSet(HNS_UNDERGROUND_SILVER_HIDE_FLAG);
        FlagClear(HOENN_MAGMA_HIDEOUT_COMPLETE);
        EXPECT(magmaHideout.isNarrativelyEligible());
        FlagSet(HOENN_MAGMA_HIDEOUT_COMPLETE);
        EXPECT(!magmaHideout.isNarrativelyEligible());
        EXPECT(FlagGet(HNS_UNDERGROUND_SILVER_HIDE_FLAG));
    }

    VarSet(HOENN_ROUTE110_STATE, oldHoennRoute110State);
    VarSet(HNS_GOLDENROD_CITY_STATE, oldHnsGoldenrodState);
    if (oldHoennMagmaHideoutComplete)
        FlagSet(HOENN_MAGMA_HIDEOUT_COMPLETE);
    else
        FlagClear(HOENN_MAGMA_HIDEOUT_COMPLETE);
    if (oldHnsSilverHideFlag)
        FlagSet(HNS_UNDERGROUND_SILVER_HIDE_FLAG);
    else
        FlagClear(HNS_UNDERGROUND_SILVER_HIDE_FLAG);
}
#endif
