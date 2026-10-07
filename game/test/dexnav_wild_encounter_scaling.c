#include "global.h"
#include "dexnav.h"
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

TEST("DexNav leaves hidden data raw")
{
    EXPECT_EQ(DexNavGetHiddenProfileSpeciesForTesting(&sDexNavHiddenInfo, 0), SPECIES_VENUSAUR);
    EXPECT_EQ(DexNavGetHiddenProfileSpeciesForTesting(&sDexNavHiddenInfo, HIDDEN_WILD_COUNT), SPECIES_NONE);
}

TEST("DexNav ordinary detector fallback mirrors the eligible profile only for lure rolls below 20%")
{
    struct WildEncounterProfileView profile = MakeDexNavNormalProfile();
    u8 slot;

    EXPECT(DexNavSelectProfileFallbackSlotWithRollsForTesting(&profile, 0, TRUE, 1, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(DexNavSelectProfileFallbackSlotWithRollsForTesting(&profile, 0, TRUE, 2, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(DexNavSelectProfileFallbackSlotWithRollsForTesting(&profile, 70, TRUE, 0, &slot));
    EXPECT_EQ(slot, 0);
}

#if IS_WAYFARER
// DexNav's species search draws from the same exact distribution the encounter rolls: every
// (level) of a species has the mass slot weight times outcome weight, as the slot outcomes give.
TEST("DexNav species search follows the slot outcome distribution at the current Trainer Rating")
{
    static const u16 sRatings[] = { 0, 40, 60, 160 };
    static const u8 weights[] = { 70, 30 };
    struct WildPokemon mons[2] = { { 1, 1, SPECIES_PIDGEOTTO }, { 1, 1, SPECIES_PIDGEY } };
    struct WildPokemonInfo info = { 1, mons };
    struct WildEncounterProfileView view = MakeDexNavNormalProfile();
    u32 road = HEADER_NONE, header, r;
    static const u16 sSpecies[] = { SPECIES_PIDGEY, SPECIES_PIDGEOTTO };

    for (header = 0; gWildMonHeaders[header].mapGroup != MAP_GROUP(MAP_UNDEFINED); header++)
        if (gWildEncounterPlaces[header].reach == WILD_REACH_ROAD) { road = header; break; }
    EXPECT_NE(road, HEADER_NONE);
    view.wildMonsInfo = &info;
    view.weights = weights;
    view.headerId = road;
    for (r = 0; r < ARRAY_COUNT(sRatings); r++)
    {
        u32 s;

        SetTrainerRating(sRatings[r]);
        for (s = 0; s < ARRAY_COUNT(sSpecies); s++)
        {
            u32 mass = 0, slot, i, roll, total;

            for (slot = 0; slot < 2; slot++)
            {
                struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
                u32 count = GetCurrentWildEncounterSlotOutcomes(&view, slot, outcomes);

                for (i = 0; i < count; i++)
                    if (outcomes[i].species == sSpecies[s])
                    {
                        mass += weights[slot] * outcomes[i].weight;
                    }
            }
            total = DexNavSelectProfileOutcomeWithRollForTesting(&view, sSpecies[s], 0, NULL);
            EXPECT_EQ(total, mass);
            if (mass == 0)
                continue;
            // Walk the slots and outcomes in order: each entry owns a run of rolls; its first and last
            // roll must select exactly that species and level.
            roll = 0;
            for (slot = 0; slot < 2; slot++)
            {
                struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
                u32 count = GetCurrentWildEncounterSlotOutcomes(&view, slot, outcomes);

                for (i = 0; i < count; i++)
                {
                    struct WildEncounterSpeciesOutcome outcome;
                    u32 entryMass = weights[slot] * outcomes[i].weight;

                    if (outcomes[i].species != sSpecies[s] || entryMass == 0)
                        continue;
                    EXPECT_EQ(DexNavSelectProfileOutcomeWithRollForTesting(&view, sSpecies[s], roll, &outcome), mass);
                    EXPECT_EQ(outcome.species, sSpecies[s]);
                    EXPECT_EQ(outcome.level, outcomes[i].level);
                    EXPECT_EQ(DexNavSelectProfileOutcomeWithRollForTesting(&view, sSpecies[s], roll + entryMass - 1, &outcome), mass);
                    EXPECT_EQ(outcome.level, outcomes[i].level);
                    roll += entryMass;
                }
            }
            EXPECT_EQ(roll, mass);
            EXPECT_EQ(DexNavSelectProfileOutcomeWithRollForTesting(&view, sSpecies[s], mass, &(struct WildEncounterSpeciesOutcome){0}), 0);
        }
    }
}
#endif
