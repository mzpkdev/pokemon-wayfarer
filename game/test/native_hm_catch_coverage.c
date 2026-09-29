#include "global.h"
#include "pokemon.h"
#include "test/test.h"
#include "wild_encounter.h"
#include "constants/maps.h"
#include "config/notable_trainers.h"

#if IS_WAYFARER

#if WAYFARER_V0_TRAINERS
#define NATIVE_HM_MAX_RATING 160
#else
#define NATIVE_HM_MAX_RATING 80
#endif
#define NATIVE_HM_RATING_COUNT (NATIVE_HM_MAX_RATING + 1)
#define NATIVE_HM_REGIONAL_BLOCK_SIZE 4
#define NATIVE_HM_DIRECTIONAL_BLOCK_SIZE 3

struct CoverageProfile
{
    u16 map;
    u8 time;
    u8 area;
    u8 rod;
};

struct RegionalCoverage
{
    const char *name;
    u16 move;
    u16 profiles[NATIVE_HM_RATING_COUNT];
};

struct AcquisitionSource
{
    u16 map;
    u8 area;
    bool8 staticDay;
};

struct AcquisitionScenario
{
    const char *name;
    u16 move;
    const struct AcquisitionSource *sources;
    u8 sourceCount;
};

#include "data/native_hm_coverage.h"
#if WAYFARER_V0_TRAINERS
#include "data/native_hm_v0_coverage.h"
#define sActiveRegionalCoverage sV0RegionalCoverage
#define sActiveRegionalProfiles sV0RegionalProfiles
#else
#define sActiveRegionalCoverage sRegionalCoverage
#define sActiveRegionalProfiles sRegionalProfiles
#endif

struct CoverageChance
{
    u64 numerator;
    u64 denominator;
};

struct MoveCacheEntry
{
    u32 key;
    bool8 knowsMove;
};

static struct MoveCacheEntry sMoveCache[256];

// The approved coverage fixture names HNS source maps. Resolve retired coast
// maps to the selected full Wayfarer maps before measuring their real tables.
static u16 SelectedWayfarerCoverageMap(u16 map)
{
    switch (map)
    {
    case MAP_CINNABAR_ISLAND_HNS:
        return MAP_CINNABAR_ISLAND;
    case MAP_ROUTE19_HNS:
        return MAP_ROUTE19;
    case MAP_SEAFOAM_ISLANDS_1F_HNS:
        return MAP_SEAFOAM_ISLANDS_1F;
    case MAP_SEAFOAM_ISLANDS_B1F_HNS:
        return MAP_SEAFOAM_ISLANDS_B1F;
    default:
        return map;
    }
}

static void SetCoverageMode(void)
{
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_WildPokemon = FALSE;
    memset(sMoveCache, 0, sizeof(sMoveCache));
}

static bool8 CaughtMonKnowsMove(u16 species, u8 level, u16 move)
{
    u32 key = species * 128 + level;
    struct MoveCacheEntry *cache = &sMoveCache[key % ARRAY_COUNT(sMoveCache)];

    // Only actual production-created movesets enter this cache.
    if (cache->key != key)
    {
        struct Pokemon mon;
        u8 slot;

        CreateMon(&mon, species, level, 0, OTID_STRUCT_PRESET(0));
        GiveMonInitialMoveset(&mon);
        cache->key = key;
        cache->knowsMove = FALSE;
        for (slot = 0; slot < MAX_MON_MOVES; slot++)
        {
            if (GetMonData(&mon, MON_DATA_MOVE1 + slot) == move)
                cache->knowsMove = TRUE;
        }
    }
    return cache->knowsMove;
}

static bool8 ResolveCoverageProfile(const struct CoverageProfile *profile, struct WildEncounterProfileView *view)
{
    u16 header;
    u16 map = SelectedWayfarerCoverageMap(profile->map);
    for (header = 0; gWildMonHeaders[header].mapGroup != MAP_GROUP(MAP_UNDEFINED); header++)
    {
        if (gWildMonHeaders[header].mapGroup == MAP_GROUP(map)
         && gWildMonHeaders[header].mapNum == MAP_NUM(map))
        {
            struct WildEncounterProfileContext context =
            {
                .headerId = header,
                .timeOfDay = profile->time,
                .area = profile->area,
                .fishingRod = profile->rod,
            };
            return GetWildEncounterProfileView(&context, view);
        }
    }
    return FALSE;
}

static u32 GreatestCommonDivisor(u32 a, u32 b)
{
    while (b)
    {
        u32 remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

static struct CoverageChance ProfileMoveChance(const struct WildEncounterProfileView *view, u16 rating, u16 move)
{
    u16 eligibleWeight = GetWildEncounterProfileEligibleWeight(view, rating, FALSE);
    u16 successes[LAND_WILD_COUNT] = {0};
    u16 spans[LAND_WILD_COUNT] = {0};
    u16 weights[LAND_WILD_COUNT] = {0};
    u32 commonSpan = 1;
    u8 slot;
    struct CoverageChance chance = {0, 1};

    if (eligibleWeight == 0)
        return chance;

    for (slot = view->entryStart; slot < view->entryStart + view->entryCount; slot++)
    {
        const struct WildPokemon *entry;
        u16 authoredLevel;
        u16 low, high;
        u32 multiplier;

        weights[slot] = GetWildEncounterProfileEffectiveWeight(view, slot, rating, FALSE);
        if (weights[slot] == 0)
            continue;
        EXPECT(GetWildEncounterProfileEntry(view, slot, &entry));
        low = min(entry->minLevel, entry->maxLevel);
        high = max(entry->minLevel, entry->maxLevel);
        spans[slot] = high - low + 1;
        multiplier = spans[slot] / GreatestCommonDivisor(commonSpan, spans[slot]);
        EXPECT_LE(commonSpan, 0xFFFFFFFFu / multiplier);
        commonSpan *= multiplier;
        for (authoredLevel = low; authoredLevel <= high; authoredLevel++)
        {
            struct WildEncounterSpeciesOutcome outcome;
            EXPECT(GetWildEncounterSpeciesOutcome(view, slot, authoredLevel, rating, FALSE, &outcome));
            EXPECT_EQ(outcome.level, ProjectWildEncounterLevel(view, authoredLevel, rating));
            if (CaughtMonKnowsMove(outcome.species, outcome.level, move))
                successes[slot]++;
        }
    }
    // Aggregate disjoint slots and uniform authored-level rolls within one
    // source. The exact fraction avoids rounding an 8% boundary up or down.
    chance.denominator = (u64)eligibleWeight * commonSpan;
    for (slot = view->entryStart; slot < view->entryStart + view->entryCount; slot++)
    {
        if (weights[slot])
            chance.numerator += (u64)weights[slot] * successes[slot] * (commonSpan / spans[slot]);
    }
    return chance;
}

static void CheckRegionalCoverage(u8 row)
{
    u16 ratingStart = 0;
    u16 parameterBlock;
    u16 rating;
    const struct RegionalCoverage *cell;

    for (parameterBlock = 0; parameterBlock * NATIVE_HM_REGIONAL_BLOCK_SIZE <= NATIVE_HM_MAX_RATING; parameterBlock++)
        PARAMETRIZE_LABEL("%s TR %d-%d", sActiveRegionalCoverage[row].name,
                          parameterBlock * NATIVE_HM_REGIONAL_BLOCK_SIZE,
                          min((parameterBlock + 1) * NATIVE_HM_REGIONAL_BLOCK_SIZE - 1, NATIVE_HM_MAX_RATING))
        {
            ratingStart = parameterBlock * NATIVE_HM_REGIONAL_BLOCK_SIZE;
        }

    EXPECT_EQ(ARRAY_COUNT(sActiveRegionalCoverage), 24);
    cell = &sActiveRegionalCoverage[row];
    SetCoverageMode();
    for (rating = ratingStart; rating < ratingStart + NATIVE_HM_REGIONAL_BLOCK_SIZE && rating <= NATIVE_HM_MAX_RATING; rating++)
    {
        const struct CoverageProfile *profile = &sActiveRegionalProfiles[cell->profiles[rating]];
        struct WildEncounterProfileView view;
        struct CoverageChance chance;

        if (!ResolveCoverageProfile(profile, &view))
            Test_ExitWithResult(TEST_RESULT_FAIL, __LINE__,
                ":L%s:%d: regional %s TR %d is missing map %d area %d time %d rod %d",
                gTestRunnerState.test->filename, __LINE__, cell->name, rating,
                profile->map, profile->area, profile->time, profile->rod);
        if (cell->move == MOVE_SURF)
            EXPECT(profile->area == WILD_AREA_LAND || profile->area == WILD_AREA_FISHING);
        chance = ProfileMoveChance(&view, rating, cell->move);
        if (chance.numerator == 0)
            Test_ExitWithResult(TEST_RESULT_FAIL, __LINE__,
                ":L%s:%d: regional %s TR %d map %d area %d time %d rod %d has no carrier",
                gTestRunnerState.test->filename, __LINE__, cell->name, rating,
                profile->map, profile->area, profile->time, profile->rod);
    }
}

#define REGIONAL_COVERAGE_TEST(row, label) \
    TEST("Wayfarer native HM regional " label " retains a carrier at every rating") \
    { \
        CheckRegionalCoverage(row); \
    }

REGIONAL_COVERAGE_TEST(0, "Johto Cut")
REGIONAL_COVERAGE_TEST(1, "Johto Flash")
REGIONAL_COVERAGE_TEST(2, "Johto Surf")
REGIONAL_COVERAGE_TEST(3, "Johto Strength")
REGIONAL_COVERAGE_TEST(4, "Johto Rock Smash")
REGIONAL_COVERAGE_TEST(5, "Johto Waterfall")
REGIONAL_COVERAGE_TEST(6, "Johto Whirlpool")
REGIONAL_COVERAGE_TEST(7, "Johto Dive")
REGIONAL_COVERAGE_TEST(8, "Kanto Cut")
REGIONAL_COVERAGE_TEST(9, "Kanto Flash")
REGIONAL_COVERAGE_TEST(10, "Kanto Surf")
REGIONAL_COVERAGE_TEST(11, "Kanto Strength")
REGIONAL_COVERAGE_TEST(12, "Kanto Rock Smash")
REGIONAL_COVERAGE_TEST(13, "Kanto Waterfall")
REGIONAL_COVERAGE_TEST(14, "Kanto Whirlpool")
REGIONAL_COVERAGE_TEST(15, "Kanto Dive")
REGIONAL_COVERAGE_TEST(16, "Hoenn Cut")
REGIONAL_COVERAGE_TEST(17, "Hoenn Flash")
REGIONAL_COVERAGE_TEST(18, "Hoenn Surf")
REGIONAL_COVERAGE_TEST(19, "Hoenn Strength")
REGIONAL_COVERAGE_TEST(20, "Hoenn Rock Smash")
REGIONAL_COVERAGE_TEST(21, "Hoenn Waterfall")
REGIONAL_COVERAGE_TEST(22, "Hoenn Whirlpool")
REGIONAL_COVERAGE_TEST(23, "Hoenn Dive")

static void CheckDirectionalAcquisition(u8 scenarioId)
{
    u8 clock = TIME_DAY, rod = WILD_ENCOUNTER_FISHING_ROD_OLD;
    u8 parameterClock, parameterRod;
    u16 parameterBlock, ratingStart = 0;
    u16 rating;
    const struct AcquisitionScenario *scenario;

    for (parameterClock = 0; parameterClock < 2; parameterClock++)
    for (parameterRod = WILD_ENCOUNTER_FISHING_ROD_OLD; parameterRod <= WILD_ENCOUNTER_FISHING_ROD_SUPER; parameterRod++)
    for (parameterBlock = 0; parameterBlock * NATIVE_HM_DIRECTIONAL_BLOCK_SIZE <= NATIVE_HM_MAX_RATING; parameterBlock++)
        PARAMETRIZE_LABEL("%s clock %d rod %d TR %d-%d", sAcquisitionScenarios[scenarioId].name, parameterClock, parameterRod,
                          parameterBlock * NATIVE_HM_DIRECTIONAL_BLOCK_SIZE,
                          min((parameterBlock + 1) * NATIVE_HM_DIRECTIONAL_BLOCK_SIZE - 1, NATIVE_HM_MAX_RATING))
        {
            clock = parameterClock == 0 ? TIME_DAY : TIME_NIGHT;
            rod = parameterRod;
            ratingStart = parameterBlock * NATIVE_HM_DIRECTIONAL_BLOCK_SIZE;
        }

    EXPECT_EQ(ARRAY_COUNT(sAcquisitionScenarios), 11);
    scenario = &sAcquisitionScenarios[scenarioId];
    SetCoverageMode();
    // Bound each parameter case below the mechanics runner's GBA timeout.
    // Disjoint blocks enumerate every supported rating exactly once.
    for (rating = ratingStart; rating < ratingStart + NATIVE_HM_DIRECTIONAL_BLOCK_SIZE && rating <= NATIVE_HM_MAX_RATING; rating++)
    {
        bool8 found = FALSE;
        u8 sourceId;
        for (sourceId = 0; sourceId < scenario->sourceCount; sourceId++)
        {
            const struct AcquisitionSource *source = &scenario->sources[sourceId];
            const struct CoverageProfile profile =
            {
                .map = source->map,
                .time = source->staticDay ? TIME_DAY : clock,
                .area = source->area,
                .rod = source->area == WILD_AREA_FISHING ? rod : WILD_ENCOUNTER_FISHING_ROD_NONE,
            };
            struct WildEncounterProfileView view;
            struct CoverageChance chance;

            if (!ResolveCoverageProfile(&profile, &view))
                continue;
            chance = ProfileMoveChance(&view, rating, scenario->move);
            if (source->area == WILD_AREA_LAND || rod == WILD_ENCOUNTER_FISHING_ROD_OLD)
                found |= chance.numerator * 100 >= chance.denominator * 8;
            else
                found |= chance.numerator != 0;
        }
        if (!found)
            Test_ExitWithResult(TEST_RESULT_FAIL, __LINE__,
                ":L%s:%d: scenario %s time %d rod %d TR %d has no qualifying single source",
                gTestRunnerState.test->filename, __LINE__, scenario->name, clock, rod, rating);
    }
}

#define DIRECTIONAL_ACQUISITION_TEST(scenario, label) \
    TEST("Wayfarer native HM acquisition " label " has a source at every rating") \
    { \
        CheckDirectionalAcquisition(scenario); \
    }

DIRECTIONAL_ACQUISITION_TEST(0, "Cianwood Surf")
DIRECTIONAL_ACQUISITION_TEST(1, "Olivine Surf")
DIRECTIONAL_ACQUISITION_TEST(2, "Vermilion Surf")
DIRECTIONAL_ACQUISITION_TEST(3, "Cinnabar Surf")
DIRECTIONAL_ACQUISITION_TEST(4, "Lilycove Surf")
DIRECTIONAL_ACQUISITION_TEST(5, "Mossdeep Surf")
DIRECTIONAL_ACQUISITION_TEST(6, "Pacifidlog Surf")
DIRECTIONAL_ACQUISITION_TEST(7, "Route 118 West Surf")
DIRECTIONAL_ACQUISITION_TEST(8, "Route 118 East Surf")
DIRECTIONAL_ACQUISITION_TEST(9, "Blackthorn Surf")
DIRECTIONAL_ACQUISITION_TEST(10, "Dragon's Den Whirlpool")
#endif
