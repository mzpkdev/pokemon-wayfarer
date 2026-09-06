#include "global.h"
#include "event_data.h"
#include "test/test.h"
#include "trainer_rating.h"

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

TEST("Trainer Rating persists deterministic seeded values without local progression")
{
    SetTrainerRating(17);
    EXPECT_EQ(GetTrainerRating(), 17);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 17);

    VarSet(VAR_TRAINER_RATING, 65535);
    EXPECT_EQ(GetTrainerRating(), TRAINER_RATING_MAX);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), TRAINER_RATING_MAX);
}

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
