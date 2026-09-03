#include "global.h"
#include "fishing.h"
#include "randomizer.h"
#include "test/test.h"
#include "wild_encounter.h"
#include "constants/items.h"

static const struct WildPokemon sSelectionMons[] =
{
    { 10, 10, SPECIES_MAGIKARP },
    { 10, 10, SPECIES_ZIGZAGOON },
};

static const struct WildPokemonInfo sSelectionInfo =
{
    .encounterRate = 1,
    .wildPokemon = sSelectionMons,
};

static const u8 sSelectionWeights[] = { 70, 30 };

#if IS_HNS
static const struct WildPokemon sHoennSoundSelectionMons[] =
{
    { 10, 10, SPECIES_MAGIKARP },
    { 10, 10, SPECIES_TREECKO },
    { 10, 10, SPECIES_TAILLOW },
};

static const struct WildPokemonInfo sHoennSoundSelectionInfo =
{
    .encounterRate = 1,
    .wildPokemon = sHoennSoundSelectionMons,
};

static const u8 sHoennSoundSelectionWeights[] = { 70, 20, 10 };
#endif

static const struct WildPokemon sFishingSelectionMons[FISH_WILD_COUNT] =
{
    { 10, 10, SPECIES_MAGIKARP }, { 10, 10, SPECIES_MAGIKARP },
    { 10, 10, SPECIES_MAGIKARP }, { 10, 10, SPECIES_MAGIKARP },
    { 10, 10, SPECIES_MAGIKARP }, { 10, 10, SPECIES_MAGIKARP },
    { 10, 10, SPECIES_MAGIKARP }, { 10, 10, SPECIES_MAGIKARP },
    { 10, 10, SPECIES_MAGIKARP }, { 10, 10, SPECIES_MAGIKARP },
};

static const struct WildPokemonInfo sFishingSelectionInfo =
{
    .encounterRate = 1,
    .wildPokemon = sFishingSelectionMons,
};

static const struct WildPokemon sEmptySelectionMons[] =
{
    { 10, 10, SPECIES_NONE },
    { 10, 10, SPECIES_MAGIKARP },
};

static const struct WildPokemonInfo sEmptySelectionInfo =
{
    .encounterRate = 1,
    .wildPokemon = sEmptySelectionMons,
};

static const struct WildPokemonInfo sZeroRateSelectionInfo =
{
    .encounterRate = 0,
    .wildPokemon = sEmptySelectionMons,
};

static const struct WildPokemon sFloorMons[] =
{
    { 1, 1, SPECIES_KECLEON },
    { 10, 10, SPECIES_MAGIKARP },
};

static const struct WildPokemonInfo sFloorInfo =
{
    .encounterRate = 1,
    .wildPokemon = sFloorMons,
};

static const u8 sFloorWeights[] = { 70, 30 };

static const struct WildPokemon sEvolutionMons[] =
{
    { 32, 32, SPECIES_VENUSAUR },
};

static const struct WildPokemonInfo sEvolutionInfo =
{
    .encounterRate = 1,
    .wildPokemon = sEvolutionMons,
};

static const u8 sEvolutionWeights[] = { 100 };

static const struct WildPokemon sAlternateEvolutionMons[] =
{
    { 51, 51, SPECIES_GOLEM },
};

static const struct WildPokemonInfo sAlternateEvolutionInfo =
{
    .encounterRate = 1,
    .wildPokemon = sAlternateEvolutionMons,
};

static const u8 sAlternateEvolutionWeights[] = { 100 };

static const struct WildPokemon sUnderThresholdEvolutionMons[] =
{
    { 10, 10, SPECIES_GYARADOS },
};

static const struct WildPokemonInfo sUnderThresholdEvolutionInfo =
{
    .encounterRate = 1,
    .wildPokemon = sUnderThresholdEvolutionMons,
};

static const u8 sUnderThresholdEvolutionWeights[] = { 100 };

static struct WildEncounterProfileView MakeTestProfile(const struct WildPokemonInfo *info, const u8 *weights, u8 entryCount)
{
    return (struct WildEncounterProfileView)
    {
        .wildMonsInfo = info,
        .weights = weights,
        .headerId = HEADER_NONE,
        .timeOfDay = TIME_OF_DAY_DEFAULT,
        .area = WILD_AREA_HIDDEN,
        .fishingRod = WILD_ENCOUNTER_FISHING_ROD_NONE,
        .entryStart = 0,
        .entryCount = entryCount,
    };
}

static bool8 FindFirstProfile(enum WildPokemonArea area, enum WildEncounterFishingRod rod, struct WildEncounterProfileView *view)
{
    struct WildEncounterProfileContext context;
    u16 headerId;
    u8 timeOfDay;

    for (headerId = 0; gWildMonHeaders[headerId].mapGroup != MAP_GROUP(MAP_UNDEFINED); headerId++)
    {
        for (timeOfDay = 0; timeOfDay < TIMES_OF_DAY_COUNT; timeOfDay++)
        {
            context.headerId = headerId;
            context.timeOfDay = timeOfDay;
            context.area = area;
            context.fishingRod = rod;
            if (GetWildEncounterProfileView(&context, view))
                return TRUE;
        }
    }
    return FALSE;
}

#if IS_HNS
static bool8 FindProfileForMap(u16 map, enum TimeOfDay timeOfDay, enum WildPokemonArea area, enum WildEncounterFishingRod rod, struct WildEncounterProfileView *view)
{
    struct WildEncounterProfileContext context;
    u16 headerId;

    for (headerId = 0; gWildMonHeaders[headerId].mapGroup != MAP_GROUP(MAP_UNDEFINED); headerId++)
    {
        u16 headerMap = (gWildMonHeaders[headerId].mapGroup << 8) | gWildMonHeaders[headerId].mapNum;

        if (headerMap != map)
            continue;
        context.headerId = headerId;
        context.timeOfDay = timeOfDay;
        context.area = area;
        context.fishingRod = rod;
        if (GetWildEncounterProfileView(&context, view))
            return TRUE;
    }
    return FALSE;
}
#endif

TEST("Wild encounter scaling follows anchors, high-water marks, and bounds")
{
    static const u8 sRatings[] = { 0, 4, 8, 16, 30, 40, 55, 65, 80 };
    static const u8 sExpectedLevels[] = { 5, 6, 8, 13, 20, 32, 50, 70, 90 };
    static const u8 sAuthoredLevels[] = { 1, MAX_LEVEL };
    u8 i;
    u8 authoredLevel;

    for (i = 0; i < ARRAY_COUNT(sRatings); i++)
        EXPECT_EQ(ProjectWildEncounterLevelWithOffset(5, sRatings[i], 0), sExpectedLevels[i]);

    for (authoredLevel = 0; authoredLevel < ARRAY_COUNT(sAuthoredLevels); authoredLevel++)
    {
        u8 previous = 0;

        for (i = 0; i <= 100; i++)
        {
            u8 level = ProjectWildEncounterLevelWithOffset(sAuthoredLevels[authoredLevel], i, 0);

            EXPECT_GE(level, previous);
            previous = level;
        }
    }

    EXPECT_EQ(ProjectWildEncounterLevelWithOffset(5, 65535, 0), 90);
    EXPECT_EQ(ProjectWildEncounterLevelWithOffset(5, 0, -8), 1);
    EXPECT_EQ(ProjectWildEncounterLevelWithOffset(90, 80, 20), MAX_LEVEL);
}

TEST("Standard Rod: every fishing quality uses the generated ten-slot profile")
{
    static const u8 expected[][FISH_WILD_COUNT] =
    {
        [WILD_ENCOUNTER_FISHING_ROD_OLD] = { 38, 22, 10, 8, 8, 4, 3, 3, 2, 2 },
        [WILD_ENCOUNTER_FISHING_ROD_GOOD] = { 25, 18, 12, 10, 9, 7, 6, 5, 4, 4 },
        [WILD_ENCOUNTER_FISHING_ROD_SUPER] = { 12, 10, 11, 10, 10, 10, 10, 9, 9, 9 },
    };
    struct WildEncounterProfileView view;
    const struct WildPokemon *entry;
    u8 mirroredSlot;
    u8 rod;
    u8 slot;

    ASSUME(FindFirstProfile(WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE, &view));
    EXPECT_EQ(view.entryStart, 0);
    EXPECT_EQ(view.entryCount, LAND_WILD_COUNT);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 100);
    EXPECT(GetWildEncounterProfileEntry(&view, 0, &entry));
    EXPECT_EQ(entry, &view.wildMonsInfo->wildPokemon[0]);

    ASSUME(FindFirstProfile(WILD_AREA_WATER, WILD_ENCOUNTER_FISHING_ROD_NONE, &view));
    EXPECT_EQ(view.entryStart, 0);
    EXPECT_EQ(view.entryCount, WATER_WILD_COUNT);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 100);

    ASSUME(FindFirstProfile(WILD_AREA_ROCKS, WILD_ENCOUNTER_FISHING_ROD_NONE, &view));
    EXPECT_EQ(view.entryStart, 0);
    EXPECT_EQ(view.entryCount, ROCK_WILD_COUNT);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 100);

    for (rod = WILD_ENCOUNTER_FISHING_ROD_OLD; rod <= WILD_ENCOUNTER_FISHING_ROD_SUPER; rod++)
    {
        ASSUME(FindFirstProfile(WILD_AREA_FISHING, rod, &view));
        EXPECT_EQ(view.entryStart, 0);
        EXPECT_EQ(view.entryCount, FISH_WILD_COUNT);
        for (slot = 0; slot < FISH_WILD_COUNT; slot++)
            EXPECT_EQ(view.weights[slot], expected[rod][slot]);
        EXPECT(GetWildEncounterProfileEntry(&view, 9, &entry));
        EXPECT(!GetWildEncounterProfileEntry(&view, 10, &entry));
        EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&view, 80, FALSE, 0, &mirroredSlot));
        EXPECT_EQ(mirroredSlot, 9);
    }
}

TEST("Standard Rod weighted selection covers every exact boundary")
{
    struct WildEncounterProfileView view = MakeTestProfile(&sFishingSelectionInfo, NULL, FISH_WILD_COUNT);
    u8 rod;
    u8 slot;
    u8 candidate;
    u16 boundary;

    for (rod = WILD_ENCOUNTER_FISHING_ROD_OLD; rod <= WILD_ENCOUNTER_FISHING_ROD_SUPER; rod++)
    {
        view.weights = gStandardRodFishingWeights[rod];
        EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 80, FALSE), 100);
        boundary = 0;
        for (candidate = 0; candidate < FISH_WILD_COUNT; candidate++)
        {
            EXPECT(SelectWildEncounterProfileSlot(&view, 80, FALSE, boundary, &slot));
            EXPECT_EQ(slot, candidate);
            boundary += view.weights[candidate];
            EXPECT(SelectWildEncounterProfileSlot(&view, 80, FALSE, boundary - 1, &slot));
            EXPECT_EQ(slot, candidate);
        }
        EXPECT_EQ(boundary, 100);
        EXPECT(!SelectWildEncounterProfileSlot(&view, 80, FALSE, boundary, &slot));
    }
}

#if IS_HNS
TEST("Kanto land and Surf profiles are unchanged by Hoenn Sound")
{
    static const u8 sRatings[] = { 10, 16, 40, 55, 65, 80 };
    static const enum TimeOfDay sTimesOfDay[] = { TIME_DAY, TIME_NIGHT };
    static const struct
    {
        u16 map;
        enum WildPokemonArea area;
    } sProfiles[] =
    {
        { MAP_ROUTE1_HNS, WILD_AREA_LAND },
        { MAP_ROUTE10_HNS, WILD_AREA_LAND },
        { MAP_ROUTE10_HNS, WILD_AREA_WATER },
        { MAP_CINNABAR_ISLAND_HNS, WILD_AREA_WATER },
        { MAP_MT_MOON_CAVE_HNS, WILD_AREA_LAND },
    };
    struct WildEncounterProfileView hoennSoundView = MakeTestProfile(&sHoennSoundSelectionInfo, sHoennSoundSelectionWeights, ARRAY_COUNT(sHoennSoundSelectionMons));
    u8 hoennSoundSlot;
    u8 profileId;

    // Prove the radio path is independently active before checking that it
    // falls through for Kanto-only profiles.
    ASSUME(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&hoennSoundView, 80, FALSE, FALSE, 0, 0, 0, &hoennSoundSlot));
    EXPECT_EQ(hoennSoundSlot, 0);
    ASSUME(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&hoennSoundView, 80, FALSE, TRUE, 0, 0, 0, &hoennSoundSlot));
    EXPECT_EQ(hoennSoundSlot, 1);
    ASSUME(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&hoennSoundView, 80, FALSE, TRUE, 0, 1, 0, &hoennSoundSlot));
    EXPECT_EQ(hoennSoundSlot, 2);
    ASSUME(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&hoennSoundView, 80, FALSE, TRUE, 1, 0, 0, &hoennSoundSlot));
    EXPECT_EQ(hoennSoundSlot, 0);

    for (profileId = 0; profileId < ARRAY_COUNT(sProfiles); profileId++)
    {
        u8 timeOfDayId;

        for (timeOfDayId = 0; timeOfDayId < ARRAY_COUNT(sTimesOfDay); timeOfDayId++)
        {
            struct WildEncounterProfileView view;
            u8 slot;
            u16 rating = sRatings[(profileId * ARRAY_COUNT(sTimesOfDay) + timeOfDayId) % ARRAY_COUNT(sRatings)];

            ASSUME(FindProfileForMap(sProfiles[profileId].map, sTimesOfDay[timeOfDayId], sProfiles[profileId].area, WILD_ENCOUNTER_FISHING_ROD_NONE, &view));
            for (slot = view.entryStart; slot < view.entryStart + view.entryCount; slot++)
            {
                const struct WildPokemon *entry;

                ASSUME(GetWildEncounterProfileEntry(&view, slot, &entry));
                EXPECT(entry->species < SPECIES_TREECKO || entry->species > SPECIES_DEOXYS_NORMAL);
            }

            {
                u16 eligibleWeight = GetWildEncounterProfileEligibleWeight(&view, rating, FALSE);
                u16 boundary = 0;

                ASSUME(eligibleWeight > 0);
                for (slot = view.entryStart; slot < view.entryStart + view.entryCount; slot++)
                {
                    u16 weight = GetWildEncounterProfileEffectiveWeight(&view, slot, rating, FALSE);
                    u16 boundaryRolls[2];
                    u8 boundaryId;

                    if (weight == 0)
                        continue;
                    boundaryRolls[0] = boundary;
                    boundaryRolls[1] = boundary + weight - 1;
                    for (boundaryId = 0; boundaryId < ARRAY_COUNT(boundaryRolls); boundaryId++)
                    {
                        u8 soundOffSlot;
                        u8 soundOnSlot;

                        ASSUME(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&view, rating, FALSE, FALSE, 0, 0, boundaryRolls[boundaryId], &soundOffSlot));
                        ASSUME(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&view, rating, FALSE, TRUE, 0, 0, boundaryRolls[boundaryId], &soundOnSlot));
                        EXPECT_EQ(soundOffSlot, slot);
                        EXPECT_EQ(soundOnSlot, soundOffSlot);
                    }
                    boundary += weight;
                }
                EXPECT_EQ(boundary, eligibleWeight);
            }
        }
    }
}

TEST("Kanto Chinchou records retain exact Old Rod accessibility")
{
    static const struct
    {
        u16 map;
        enum TimeOfDay timeOfDay;
    } sRecords[] =
    {
        { MAP_VERMILION_CITY_HNS, TIME_DAY },
        { MAP_VERMILION_CITY_HNS, TIME_NIGHT },
        { MAP_VERMILION_CITY_PORT_OUTSIDE_HNS, TIME_DAY },
        { MAP_VERMILION_CITY_PORT_OUTSIDE_HNS, TIME_NIGHT },
        { MAP_CINNABAR_ISLAND_HNS, TIME_DAY },
        { MAP_CINNABAR_ISLAND_HNS, TIME_NIGHT },
    };
    u8 recordId;
    u32 oldRodBitePercent = CalculateFishingBiteOddsWithBonuses(OLD_ROD, FALSE, 0, 0, 0);

    EXPECT_EQ(oldRodBitePercent, 25);
    EXPECT_EQ(oldRodBitePercent * 11, 275);

    for (recordId = 0; recordId < ARRAY_COUNT(sRecords); recordId++)
    {
        struct WildEncounterProfileView view;
        u8 lowestEffectiveLevel = MAX_LEVEL;
        u16 rating;

        ASSUME(FindProfileForMap(sRecords[recordId].map, sRecords[recordId].timeOfDay, WILD_AREA_FISHING, WILD_ENCOUNTER_FISHING_ROD_OLD, &view));
        EXPECT_EQ(view.wildMonsInfo->encounterRate, 30);
        EXPECT_EQ(view.entryCount, FISH_WILD_COUNT);

        for (rating = 10; rating <= 80; rating++)
        {
            u16 chinchouWeight = 0;
            u8 slot;

            EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, rating, FALSE), 100);
            for (slot = view.entryStart; slot < view.entryStart + view.entryCount; slot++)
            {
                const struct WildPokemon *entry;
                u8 authoredLevel;

                ASSUME(GetWildEncounterProfileEntry(&view, slot, &entry));
                if (entry->species != SPECIES_CHINCHOU)
                    continue;
                chinchouWeight += GetWildEncounterProfileEffectiveWeight(&view, slot, rating, FALSE);
                for (authoredLevel = entry->minLevel; authoredLevel <= entry->maxLevel; authoredLevel++)
                {
                    struct WildEncounterSpeciesOutcome outcome;

                    ASSUME(GetWildEncounterSpeciesOutcome(&view, slot, authoredLevel, rating, FALSE, &outcome));
                    EXPECT_EQ(outcome.species, SPECIES_CHINCHOU);
                    if (outcome.level < lowestEffectiveLevel)
                        lowestEffectiveLevel = outcome.level;
                }
            }
            EXPECT_EQ(chinchouWeight, 11);
        }
        EXPECT_EQ(lowestEffectiveLevel, 9);
    }
}
#endif

TEST("Wild encounter scaling rejects unsupported or malformed profile contexts")
{
    struct WildEncounterProfileContext context =
    {
        .headerId = HEADER_NONE,
        .timeOfDay = TIME_OF_DAY_DEFAULT,
        .area = WILD_AREA_LAND,
        .fishingRod = WILD_ENCOUNTER_FISHING_ROD_NONE,
    };
    struct WildEncounterProfileView view;

    EXPECT(!GetWildEncounterProfileView(&context, &view));
    context.headerId = 65534;
    EXPECT(!GetWildEncounterProfileView(&context, &view));
    context.headerId = 0;
    context.timeOfDay = (enum TimeOfDay)-1;
    EXPECT(!GetWildEncounterProfileView(&context, &view));
    context.timeOfDay = TIMES_OF_DAY_COUNT;
    EXPECT(!GetWildEncounterProfileView(&context, &view));
    context.timeOfDay = TIME_OF_DAY_DEFAULT;
    context.area = WILD_AREA_HIDDEN;
    EXPECT(!GetWildEncounterProfileView(&context, &view));
    context.area = WILD_AREA_LAND;
    context.fishingRod = WILD_ENCOUNTER_FISHING_ROD_OLD;
    EXPECT(!GetWildEncounterProfileView(&context, &view));
    context.area = WILD_AREA_FISHING;
    context.fishingRod = WILD_ENCOUNTER_FISHING_ROD_NONE;
    EXPECT(!GetWildEncounterProfileView(&context, &view));
}

TEST("Wild encounter scaling selects using only eligible authored weights")
{
    struct WildEncounterProfileView view = MakeTestProfile(&sSelectionInfo, sSelectionWeights, ARRAY_COUNT(sSelectionMons));
    u8 slot;

    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 100);
    EXPECT_EQ(GetWildEncounterProfileEffectiveWeight(&view, 0, 0, FALSE), 70);
    EXPECT_EQ(GetWildEncounterProfileEffectiveWeight(&view, 1, 0, FALSE), 30);
    EXPECT(SelectWildEncounterProfileSlot(&view, 0, FALSE, 0, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(SelectWildEncounterProfileSlot(&view, 0, FALSE, 69, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(SelectWildEncounterProfileSlot(&view, 0, FALSE, 70, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(SelectWildEncounterProfileSlot(&view, 0, FALSE, 99, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(!SelectWildEncounterProfileSlot(&view, 0, FALSE, 100, &slot));

    view.entryCount = LAND_WILD_COUNT + 1;
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 0);
    EXPECT(!SelectWildEncounterProfileSlot(&view, 0, FALSE, 0, &slot));
}

TEST("Wild encounter scaling applies floor and evolution policy before selection")
{
    struct WildEncounterProfileView floorView = MakeTestProfile(&sFloorInfo, sFloorWeights, ARRAY_COUNT(sFloorMons));
    struct WildEncounterProfileView evolutionView = MakeTestProfile(&sEvolutionInfo, sEvolutionWeights, ARRAY_COUNT(sEvolutionMons));
    struct WildEncounterProfileView alternateEvolutionView = MakeTestProfile(&sAlternateEvolutionInfo, sAlternateEvolutionWeights, ARRAY_COUNT(sAlternateEvolutionMons));
    struct WildEncounterProfileView underThresholdEvolutionView = MakeTestProfile(&sUnderThresholdEvolutionInfo, sUnderThresholdEvolutionWeights, ARRAY_COUNT(sUnderThresholdEvolutionMons));
    struct WildEncounterSpeciesOutcome outcome;
    u8 slot;
    u8 mirroredSlot;

    EXPECT(GetWildEncounterSpeciesOutcome(&evolutionView, 0, 32, 0, FALSE, &outcome));
    EXPECT_EQ(outcome.species, SPECIES_BULBASAUR);
    // The projection, rather than authored table validity, governs ordinary
    // profile reversal. An authored level-10 Gyarados becomes Magikarp.
    EXPECT(GetWildEncounterSpeciesOutcome(&underThresholdEvolutionView, 0, 10, 0, FALSE, &outcome));
    EXPECT_EQ(outcome.species, SPECIES_MAGIKARP);
    // Golem has both a numeric Graveler predecessor and a trade evolution
    // route. Its authored-below-threshold numeric chain still reverses.
    EXPECT(GetWildEncounterSpeciesOutcome(&alternateEvolutionView, 0, 32, 0, FALSE, &outcome));
    EXPECT_EQ(outcome.species, SPECIES_GEODUDE);
    EXPECT(!IsWildEncounterProfileSlotEligible(&floorView, 0, 0, FALSE));
    EXPECT_EQ(GetWildEncounterProfileEffectiveWeight(&floorView, 0, 0, FALSE), 0);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&floorView, 0, FALSE), 30);
    EXPECT(SelectWildEncounterProfileSlot(&floorView, 0, FALSE, 0, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(SelectWildEncounterProfileSlot(&floorView, 0, FALSE, 29, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(!SelectWildEncounterProfileSlot(&floorView, 0, FALSE, 30, &slot));
    // The lure mirror works over the selected profile's eligible sequence,
    // never its raw indices. The first slot is locked here, so the surviving
    // slot maps to itself rather than back onto that locked entry.
    EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&floorView, 0, FALSE, 1, &mirroredSlot));
    EXPECT_EQ(mirroredSlot, 1);
    EXPECT(!GetWildEncounterProfileMirroredEligibleSlot(&floorView, 0, FALSE, 0, &mirroredSlot));

    // An actively enabled wild randomizer keeps authored slots intact so its
    // existing deterministic mapping receives its original index and species.
    EXPECT(GetWildEncounterSpeciesOutcome(&evolutionView, 0, 32, 0, TRUE, &outcome));
    EXPECT_EQ(outcome.species, SPECIES_VENUSAUR);
    EXPECT(IsWildEncounterProfileSlotEligible(&floorView, 0, 0, TRUE));
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&floorView, 0, TRUE), 100);
    EXPECT(SelectWildEncounterProfileSlot(&floorView, 0, TRUE, 0, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&floorView, 0, TRUE, 0, &mirroredSlot));
    EXPECT_EQ(mirroredSlot, 1);
}

TEST("Standard Rod: all-locked profiles produce no encounter")
{
    struct WildEncounterProfileView view = MakeTestProfile(&sFloorInfo, sFloorWeights, 1);
    u8 slot;

    // The fishing caller uses this failed selection to return before it
    // advances fishing stats, sets the angler species, or starts a battle.
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 0);
    EXPECT(!SelectWildEncounterProfileSlot(&view, 0, FALSE, 0, &slot));
}

TEST("Standard Rod: fishing excludes empty slots even when randomized")
{
    static const u8 weights[] = { 70, 30 };
    struct WildEncounterProfileView view = MakeTestProfile(&sEmptySelectionInfo, weights, ARRAY_COUNT(sEmptySelectionMons));
    u8 slot;

    EXPECT(!IsWildEncounterProfileSlotEligible(&view, 0, 80, FALSE));
    EXPECT(!IsWildEncounterProfileSlotEligible(&view, 0, 80, TRUE));
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 80, TRUE), 30);
    EXPECT(SelectWildEncounterProfileSlot(&view, 80, TRUE, 0, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(!GetWildEncounterProfileMirroredEligibleSlot(&view, 80, TRUE, 0, &slot));
    EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&view, 80, TRUE, 1, &slot));
    EXPECT_EQ(slot, 1);
}

TEST("Standard Rod: fishing availability rejects zero rates and zero eligible data")
{
    static const u8 weights[] = { 70, 30 };
    struct WildEncounterProfileView view = MakeTestProfile(&sEmptySelectionInfo, weights, ARRAY_COUNT(sEmptySelectionMons));

    EXPECT(DoesWildEncounterProfileHaveAvailableEntries(&view, 80, FALSE));
    EXPECT(DoesWildEncounterProfileHaveAvailableEntries(&view, 80, TRUE));

    view.wildMonsInfo = &sZeroRateSelectionInfo;
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(&view, 80, FALSE));

    view.wildMonsInfo = &sEmptySelectionInfo;
    view.entryCount = 1;
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(&view, 80, FALSE));
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(&view, 80, TRUE));

    view.wildMonsInfo = NULL;
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(&view, 80, FALSE));
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(NULL, 80, FALSE));
}

TEST("Standard Rod: Route 119 Feebas override stays ahead of every rod profile")
{
    u8 rod;

    for (rod = WILD_ENCOUNTER_FISHING_ROD_OLD; rod <= WILD_ENCOUNTER_FISHING_ROD_SUPER; rod++)
        EXPECT_EQ(GenerateFeebasFishingWildMonForTesting(rod), SPECIES_FEEBAS);
}

#if RANDOMIZER_AVAILABLE == TRUE
TEST("Standard Rod: fishing randomizer receives the authored species and raw slot")
{
    struct WildEncounterProfileView view = MakeTestProfile(&sUnderThresholdEvolutionInfo, sUnderThresholdEvolutionWeights, ARRAY_COUNT(sUnderThresholdEvolutionMons));
    u16 randomizedAuthored;
    u16 randomizedEffective;
    u16 actual;
    u16 mapNum;
    bool8 foundDistinctMapping = FALSE;

    gSaveBlock3Ptr->challengeSettings.tx_Random_WildPokemon = TRUE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_MapBased = TRUE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Chaos = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Similar = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_IncludeLegendaries = FALSE;

    for (mapNum = 0; mapNum <= 255; mapNum++)
    {
        randomizedAuthored = RandomizeWildEncounter(SPECIES_GYARADOS, mapNum, 42, WILD_AREA_FISHING, 0);
        randomizedEffective = RandomizeWildEncounter(SPECIES_MAGIKARP, mapNum, 42, WILD_AREA_FISHING, 0);
        if (randomizedAuthored == randomizedEffective)
            continue;
        actual = RandomizeWildEncounterProfileEntryForTesting(&view, 0, mapNum, 42, WILD_AREA_FISHING);
        EXPECT_EQ(actual, randomizedAuthored);
        EXPECT_NE(actual, randomizedEffective);
        foundDistinctMapping = TRUE;
        break;
    }
    EXPECT(foundDistinctMapping);
}
#endif

TEST("Wild encounter scaling applies type attraction over eligible slots")
{
    struct WildEncounterProfileView selectionView = MakeTestProfile(&sSelectionInfo, sSelectionWeights, ARRAY_COUNT(sSelectionMons));
    struct WildEncounterProfileView floorView = MakeTestProfile(&sFloorInfo, sFloorWeights, ARRAY_COUNT(sFloorMons));
    u8 slot;

    // The normal two-slot profile retains the raw game's uniform matching-slot
    // selection, independently of the authored 70/30 encounter weights.
    EXPECT(SelectWildEncounterProfileTypeSlot(&selectionView, 0, FALSE, TYPE_WATER, 0, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(SelectWildEncounterProfileTypeSlot(&selectionView, 0, FALSE, TYPE_NORMAL, 0, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(!SelectWildEncounterProfileTypeSlot(&selectionView, 0, FALSE, TYPE_WATER, 1, &slot));

    // A locked Normal slot cannot be chosen. The sole eligible Water slot is
    // the legacy no-op case (all eligible slots already match), whereas an
    // active randomizer restores both raw slots to the matching population.
    EXPECT(!SelectWildEncounterProfileTypeSlot(&floorView, 0, FALSE, TYPE_NORMAL, 0, &slot));
    EXPECT(!SelectWildEncounterProfileTypeSlot(&floorView, 0, FALSE, TYPE_WATER, 0, &slot));
    EXPECT(SelectWildEncounterProfileTypeSlot(&floorView, 0, TRUE, TYPE_NORMAL, 0, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(SelectWildEncounterProfileTypeSlot(&floorView, 0, TRUE, TYPE_WATER, 0, &slot));
    EXPECT_EQ(slot, 1);
}
