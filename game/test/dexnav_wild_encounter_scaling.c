#include "global.h"
#include "dexnav.h"
#include "event_data.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wild_encounter.h"

static const struct WildPokemon sDexNavNormalMons[] =
{
    { 10, 10, SPECIES_GYARADOS },
    { 10, 11, SPECIES_GYARADOS },
};

static const struct WildPokemonInfo sDexNavNormalInfo =
{
    .encounterRate = 1,
    .wildPokemon = sDexNavNormalMons,
};

static const u8 sDexNavNormalWeights[] = { 70, 30 };

static const struct WildPokemon sDexNavHiddenMons[] =
{
    { 32, 32, SPECIES_VENUSAUR },
};

static const struct WildPokemonInfo sDexNavHiddenInfo =
{
    .encounterRate = 0,
    .wildPokemon = sDexNavHiddenMons,
};

static struct WildEncounterProfileView MakeDexNavNormalProfile(void)
{
    return (struct WildEncounterProfileView)
    {
        .wildMonsInfo = &sDexNavNormalInfo,
        .weights = sDexNavNormalWeights,
        .headerId = HEADER_NONE,
        .timeOfDay = TIME_OF_DAY_DEFAULT,
        .area = WILD_AREA_LAND,
        .fishingRod = WILD_ENCOUNTER_FISHING_ROD_NONE,
        .entryStart = 0,
        .entryCount = ARRAY_COUNT(sDexNavNormalMons),
    };
}

static void ResetDexNavTrainerRating(void)
{
    FlagClear(FLAG_BADGE01_GET);
    FlagClear(FLAG_BADGE02_GET);
    FlagClear(FLAG_BADGE03_GET);
    FlagClear(FLAG_BADGE04_GET);
    FlagClear(FLAG_BADGE05_GET);
    FlagClear(FLAG_BADGE06_GET);
    FlagClear(FLAG_BADGE07_GET);
    FlagClear(FLAG_BADGE08_GET);

#if IS_HNS
    FlagClear(FLAG_BADGE09_GET);
    FlagClear(FLAG_BADGE10_GET);
    FlagClear(FLAG_BADGE11_GET);
    FlagClear(FLAG_BADGE12_GET);
    FlagClear(FLAG_BADGE13_GET);
    FlagClear(FLAG_BADGE14_GET);
    FlagClear(FLAG_BADGE15_GET);
    FlagClear(FLAG_BADGE16_GET);
    FlagClear(FLAG_IS_KANTO_CHAMPION);
#endif

#if IS_FRLG
    FlagClear(FLAG_SYS_GAME_CLEAR);
    FlagClear(FLAG_RECOVERED_SAPPHIRE);
#else
    FlagClear(FLAG_IS_CHAMPION);
#endif
    VarSet(VAR_TRAINER_RATING, TRAINER_RATING_MIN);
}

TEST("DexNav leaves hidden data raw while ordinary profiles use effective species")
{
    struct WildEncounterProfileView profile = MakeDexNavNormalProfile();
    struct WildEncounterSpeciesOutcome outcome;

    ResetDexNavTrainerRating();
    EXPECT_EQ(GetTrainerRating(), TRAINER_RATING_MIN);

    // The normal UI list and ordinary detector fallback both resolve entries
    // through this effective profile boundary. The authored-under-threshold
    // Gyarados reverses to Magikarp at the minimum Trainer Rating.
    EXPECT(DexNavGetEffectiveProfileOutcomeForTesting(&profile, 0, 10, &outcome));
    EXPECT_EQ(outcome.species, SPECIES_MAGIKARP);

    // Hidden DexNav has no ordinary-profile projection and continues to read
    // its authored source directly.
    EXPECT_EQ(DexNavGetHiddenProfileSpeciesForTesting(&sDexNavHiddenInfo, 0), SPECIES_VENUSAUR);
    EXPECT_EQ(DexNavGetHiddenProfileSpeciesForTesting(&sDexNavHiddenInfo, HIDDEN_WILD_COUNT), SPECIES_NONE);
}

TEST("DexNav ordinary detector fallback mirrors the eligible profile only for lure rolls below 20%")
{
    struct WildEncounterProfileView profile = MakeDexNavNormalProfile();
    u8 slot;

    ResetDexNavTrainerRating();
    EXPECT_EQ(GetTrainerRating(), TRAINER_RATING_MIN);

    // The ordinary fallback first does its normal weighted pick (slot 0 for
    // roll 0), then a 0 or 1 lure roll reverses its eligible slot sequence.
    EXPECT(DexNavSelectProfileFallbackSlotWithRollsForTesting(&profile, 0, TRUE, 1, &slot));
    EXPECT_EQ(slot, 1);

    // A non-triggering lure roll preserves that weighted selection, while a
    // different weighted result mirrors in the opposite direction.
    EXPECT(DexNavSelectProfileFallbackSlotWithRollsForTesting(&profile, 0, TRUE, 2, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(DexNavSelectProfileFallbackSlotWithRollsForTesting(&profile, 70, TRUE, 0, &slot));
    EXPECT_EQ(slot, 0);
}

TEST("DexNav selected species preserve conditional raw source and level weights")
{
    struct WildEncounterProfileView profile = MakeDexNavNormalProfile();
    struct WildEncounterSpeciesOutcome outcome;
    bool8 accepted;

    ResetDexNavTrainerRating();
    EXPECT_EQ(GetTrainerRating(), TRAINER_RATING_MIN);

    // Both raw sources yield Magikarp. Their proposal mass is 70 * 1 for the
    // one-level source and 30 * 2 for the two-level source. The correction
    // accepts the latter with probability 1 / 2, giving each accepted raw
    // level its ordinary source weight divided by its full authored range.
    EXPECT(DexNavSelectProfileOutcomeWithRollsForTesting(&profile, SPECIES_MAGIKARP, 0, 0, &accepted, &outcome));
    EXPECT(accepted);
    EXPECT_EQ(outcome.level, ProjectWildEncounterLevel(&profile, 10, TRAINER_RATING_MIN));

    EXPECT(DexNavSelectProfileOutcomeWithRollsForTesting(&profile, SPECIES_MAGIKARP, 70, 1, &accepted, &outcome));
    EXPECT(!accepted);

    EXPECT(DexNavSelectProfileOutcomeWithRollsForTesting(&profile, SPECIES_MAGIKARP, 100, 0, &accepted, &outcome));
    EXPECT(accepted);
    EXPECT_EQ(outcome.level, ProjectWildEncounterLevel(&profile, 11, TRAINER_RATING_MIN));
}
