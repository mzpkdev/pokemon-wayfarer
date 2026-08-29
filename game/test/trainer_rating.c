#include "global.h"
#include "event_data.h"
#include "test/test.h"
#include "trainer_rating.h"

TEST("Trainer Rating derives from current progression facts")
{
    EXPECT_EQ(CalculateTrainerRatingFromCurrentFacts(), 0);

    FlagSet(FLAG_BADGE01_GET);
    FlagSet(FLAG_BADGE02_GET);
    FlagSet(FLAG_BADGE03_GET);
    FlagSet(FLAG_BADGE04_GET);
    EXPECT_EQ(CalculateTrainerRatingFromCurrentFacts(), 16);

    FlagSet(FLAG_BADGE05_GET);
    FlagSet(FLAG_BADGE06_GET);
    FlagSet(FLAG_BADGE07_GET);
    FlagSet(FLAG_BADGE08_GET);
    EXPECT_EQ(CalculateTrainerRatingFromCurrentFacts(), 40);

#if IS_FRLG
    FlagSet(FLAG_SYS_GAME_CLEAR);
#else
    FlagSet(FLAG_IS_CHAMPION);
#endif
    EXPECT_EQ(CalculateTrainerRatingFromCurrentFacts(), 55);

#if IS_HNS
    FlagSet(FLAG_BADGE09_GET);
    FlagSet(FLAG_BADGE10_GET);
    FlagSet(FLAG_BADGE11_GET);
    FlagSet(FLAG_BADGE12_GET);
    FlagSet(FLAG_BADGE13_GET);
    FlagSet(FLAG_BADGE14_GET);
    FlagSet(FLAG_BADGE15_GET);
    FlagSet(FLAG_BADGE16_GET);
    EXPECT_EQ(CalculateTrainerRatingFromCurrentFacts(), 63);

    FlagSet(FLAG_IS_KANTO_CHAMPION);
    EXPECT_EQ(CalculateTrainerRatingFromCurrentFacts(), 65);
#elif IS_FRLG
    FlagSet(FLAG_RECOVERED_SAPPHIRE);
    EXPECT_EQ(CalculateTrainerRatingFromCurrentFacts(), 65);
#else
    FlagSet(FLAG_SYS_NATIONAL_DEX);
    EXPECT_EQ(CalculateTrainerRatingFromCurrentFacts(), 65);
#endif
}

TEST("Trainer Rating clamps to its supported range")
{
    EXPECT_EQ(ClampTrainerRating(0), 0);
    EXPECT_EQ(ClampTrainerRating(TRAINER_RATING_MAX), TRAINER_RATING_MAX);
    EXPECT_EQ(ClampTrainerRating(TRAINER_RATING_MAX + 1), TRAINER_RATING_MAX);
    EXPECT_EQ(ClampTrainerRating(65535), TRAINER_RATING_MAX);
}

TEST("Trainer Rating persists the greatest valid progression")
{
    VarSet(VAR_TRAINER_RATING, 3);
    FlagSet(FLAG_BADGE01_GET);

    EXPECT_EQ(GetTrainerRating(), 4);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 4);

    FlagClear(FLAG_BADGE01_GET);
    EXPECT_EQ(GetTrainerRating(), 4);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 4);

    VarSet(VAR_TRAINER_RATING, 65535);
    EXPECT_EQ(GetTrainerRating(), TRAINER_RATING_MAX);
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), TRAINER_RATING_MAX);
}

TEST("Trainer Rating migration ignores legacy variable contents")
{
    VarSet(VAR_TRAINER_RATING, 65535);
    FlagSet(FLAG_BADGE01_GET);
#if IS_FRLG
    FlagSet(FLAG_SYS_GAME_CLEAR);
    FlagSet(FLAG_RECOVERED_SAPPHIRE);
#elif !IS_HNS
    FlagSet(FLAG_IS_CHAMPION);
    FlagSet(FLAG_SYS_NATIONAL_DEX);
#else
    FlagSet(FLAG_IS_CHAMPION);
#endif

    InitializeTrainerRatingForSaveMigration();
#if !IS_HNS
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 29);
#else
    EXPECT_EQ(VarGet(VAR_TRAINER_RATING), 19);
#endif
}
