#include "global.h"
#include "config/league_circuit.h"
#include "event_data.h"
#include "league_circuit.h"
#include "league_run_helpers.h"
#include "load_save.h"
#include "save.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "gba/flash_internal.h"

#if IS_WAYFARER

TEST("Trainer Rating initializes new Wayfarer saves at zero")
{
    SetTrainerRating(TRAINER_RATING_MAX);
    InitializeTrainerRatingForNewGame();
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 0);
    EXPECT_EQ(GetTrainerRating(), 0);
}

TEST("Trainer Rating clamps to its supported range")
{
    EXPECT_EQ(ClampTrainerRating(0), 0);
    EXPECT_EQ(ClampTrainerRating(TRAINER_RATING_MIN), 0);
    EXPECT_EQ(ClampTrainerRating(TRAINER_RATING_MAX), TRAINER_RATING_MAX);
    EXPECT_EQ(ClampTrainerRating(TRAINER_RATING_MAX + 1), TRAINER_RATING_MAX);
    EXPECT_EQ(ClampTrainerRating(65535), TRAINER_RATING_MAX);
}

TEST("Trainer Rating preserves seeded high-water values and clamps corrupt saved values")
{
    SetTrainerRating(17);
    EXPECT_EQ(GetTrainerRating(), 17);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 17);

    VarSet(VAR_TRAINER_RATING, 65535);
    EXPECT_EQ(GetTrainerRating(), TRAINER_RATING_MAX);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), TRAINER_RATING_MAX);
}

#if WAYFARER_LEAGUE_CIRCUIT_ENABLED
TEST("Trainer Rating derives every badge total and every regional clear combination")
{
    static const u8 sBadgeRatings[] =
    {
        0, 4, 8, 12, 16, 22, 28, 34, 40,
        41, 42, 43, 44, 45, 46, 47, 48,
        49, 50, 51, 52, 53, 54, 55, 56,
    };
    u8 badges, clears, index;
    for (badges = 0; badges <= 24; badges++)
    {
        for (index = 0; index < 24; index++)
            SetBadgeStateForRegion(REGION_KANTO + index / 8, index % 8, index < badges);
        for (clears = 0; clears < 8; clears++)
        {
            u8 expected = sBadgeRatings[badges];
            SetChampionStateForRegion(REGION_KANTO, (clears & 1) != 0);
            SetChampionStateForRegion(REGION_JOHTO, (clears & 2) != 0);
            SetChampionStateForRegion(REGION_HOENN, (clears & 4) != 0);
            if (clears & 1)
                expected += 15;
            if (clears & 2)
                expected += 5;
            if (clears & 4)
                expected += 4;
            SetTrainerRating(0);
            EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), expected);
            EXPECT_EQ(GetTrainerRating(), expected);
            EXPECT_EQ(VarGet(VAR_TRAINER_RATING), expected);
        }
    }
}

TEST("Trainer Rating circuit milestones preserve the independent party cap curve")
{
    static const u8 sMilestones[][4] =
    {
        { 0, 0, 0, 15 }, { 4, 0, 16, 23 }, { 8, 0, 40, 42 },
        { 8, 1, 55, 60 }, { 16, 1, 63, 76 }, { 16, 3, 68, 84 },
        { 24, 3, 76, 95 }, { 24, 7, 80, 100 },
    };
    u8 row, index;
    for (row = 0; row < ARRAY_COUNT(sMilestones); row++)
    {
        for (index = 0; index < 24; index++)
            SetBadgeStateForRegion(REGION_KANTO + index / 8, index % 8, index < sMilestones[row][0]);
        SetChampionStateForRegion(REGION_KANTO, (sMilestones[row][1] & 1) != 0);
        SetChampionStateForRegion(REGION_JOHTO, (sMilestones[row][1] & 2) != 0);
        SetChampionStateForRegion(REGION_HOENN, (sMilestones[row][1] & 4) != 0);
        EXPECT_EQ(GetTrainerRating(), sMilestones[row][2]);
        EXPECT_EQ(GetTrainerRatingSoftLevelCap(), sMilestones[row][3]);
    }
}

TEST("Trainer Rating stays at its high-water mark after cleanup and failed League attempts")
{
    u8 index;
    u16 savedRating;
    for (index = 0; index < 8; index++)
        SetBadgeStateForRegion(REGION_HOENN, index, TRUE);
    EXPECT(Test_CompleteAndRecordLeague(REGION_KANTO));
    EXPECT_EQ(GetTrainerRating(), 55);
    savedRating = VarGet(VAR_TRAINER_RATING);
    for (index = 0; index < 8; index++)
        SetBadgeStateForRegion(REGION_HOENN, index, FALSE);
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), 0);
    EXPECT_EQ(GetTrainerRating(), 55);
    EXPECT(!TryRecordLeagueClear(REGION_JOHTO));
    EXPECT_EQ(GetTrainerRating(), 55);
    VarSet(VAR_TRAINER_RATING, savedRating);
    EXPECT_EQ(GetTrainerRating(), 55);
    EXPECT_EQ(GetTrainerRating(), 55);
}

TEST("Trainer Rating high-water value survives production save and load after regional cleanup")
{
    u8 badge;
    u8 loadStatus;

    CheckForFlashMemory();
    if (gFlashMemoryPresent != TRUE)
    {
        gFlashMemoryPresent = TRUE;
        InitFlashTimer();
    }
    ASSUME(gPokemonStoragePtr != NULL);
    ClearSaveData();
    Save_ResetSaveCounters();
    gSaveBlock1Ptr->saveVersionMagic = SAVE_VERSION_MAGIC;
    gSaveBlock1Ptr->saveVersion = SAVE_VERSION;
    WayfarerInitPersistentState();
    for (badge = 0; badge < 8; badge++)
        SetBadgeStateForRegion(REGION_HOENN, badge, TRUE);
    EXPECT(Test_CompleteAndRecordLeague(REGION_KANTO));
    EXPECT_EQ(GetTrainerRating(), 55);
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    HandleSavingData(SAVE_NORMAL);
    ClearSav1();
    ClearSav2();
    ClearSav3();
    loadStatus = LoadGameSave(SAVE_NORMAL);
    ClearSaveData();
    Save_ResetSaveCounters();
    EXPECT_EQ(loadStatus, SAVE_STATUS_OK);
    EXPECT_EQ(GetGlobalBadgeCount(), 8);
    EXPECT(!GetChampionStateForRegion(REGION_KANTO));
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), 40);
    EXPECT_EQ(GetTrainerRating(), 55);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 55);
}
#else
TEST("Trainer Rating rollback preserves saved Rating without circuit badge derivation")
{
    u8 index;

    for (index = 0; index < 8; index++)
    {
        SetBadgeStateForRegion(REGION_HOENN, index, TRUE);
        SetBadgeStateForRegion(REGION_JOHTO, index, TRUE);
    }
    SetChampionStateForRegion(REGION_KANTO, TRUE);
    SetChampionStateForRegion(REGION_JOHTO, TRUE);
    SetChampionStateForRegion(REGION_HOENN, TRUE);

    SetTrainerRating(0);
    EXPECT_EQ(GetTrainerRating(), 0);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 0);

    SetTrainerRating(17);
    EXPECT_EQ(GetTrainerRating(), 17);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 17);
}
#endif

TEST("Trainer Rating soft cap interpolates every seeded rating monotonically")
{
    static const u8 sExpectedAnchors[][2] =
    {
        { 0, 15 }, { 4, 16 }, { 8, 18 }, { 16, 23 }, { 30, 30 },
        { 40, 42 }, { 55, 60 }, { 65, 80 }, { 80, 100 },
    };
    u8 rating;
    u8 previousCap = 0;
    u8 i;

    for (rating = TRAINER_RATING_MIN; rating <= TRAINER_RATING_MAX; rating++)
    {
        SetTrainerRating(rating);
        EXPECT_GE(GetTrainerRatingSoftLevelCap(), previousCap);
        previousCap = GetTrainerRatingSoftLevelCap();
    }
    for (i = 0; i < ARRAY_COUNT(sExpectedAnchors); i++)
    {
        SetTrainerRating(sExpectedAnchors[i][0]);
        EXPECT_EQ(GetTrainerRatingSoftLevelCap(), sExpectedAnchors[i][1]);
    }
    SetTrainerRating(1);
    EXPECT_EQ(GetTrainerRatingSoftLevelCap(), 15);
    SetTrainerRating(2);
    EXPECT_EQ(GetTrainerRatingSoftLevelCap(), 16);
}

TEST("Trainer Rating experience reduction grants full experience through the cap")
{
    u16 species = SPECIES_BULBASAUR;
    u32 capExp;

    SetTrainerRating(0);
    capExp = gExperienceTables[gSpeciesInfo[species].growthRate][GetTrainerRatingSoftLevelCap()];
    EXPECT_EQ(ApplyTrainerRatingExperienceReduction(species, capExp - 5, 5), 5);
    EXPECT_EQ(ApplyTrainerRatingExperienceReduction(species, capExp - 5, 6), 5 + 1);
    EXPECT_EQ(ApplyTrainerRatingExperienceReduction(species, capExp, 8), 4);
    EXPECT_EQ(ApplyTrainerRatingExperienceReduction(species, capExp, 1), 1);
    EXPECT_EQ(ApplyTrainerRatingExperienceReduction(species, capExp, 0), 0);
}

#endif // IS_WAYFARER
