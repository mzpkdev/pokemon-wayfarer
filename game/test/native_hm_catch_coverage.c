#include "global.h"
#include "pokemon.h"
#include "test/test.h"
#include "wild_encounter.h"
#include "constants/maps.h"

#if IS_WAYFARER

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
    bool8 modern;
    u16 profiles[81];
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

static void SetCoverageMode(bool8 modern)
{
    gSaveBlock3Ptr->challengeSettings.tx_Mode_Modern_Moves = modern;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_WildPokemon = FALSE;
    memset(sMoveCache, 0, sizeof(sMoveCache));
}

static bool8 CaughtMonKnowsMove(u16 species, u8 level, u16 move)
{
    u32 key = species * 128 + level;
    struct MoveCacheEntry *cache = &sMoveCache[key % ARRAY_COUNT(sMoveCache)];

    // Each test case asks about one move in one learnset mode. Only actual
    // production-created movesets enter this cache.
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
    for (header = 0; gWildMonHeaders[header].mapGroup != MAP_GROUP(MAP_UNDEFINED); header++)
    {
        if (gWildMonHeaders[header].mapGroup == MAP_GROUP(profile->map)
         && gWildMonHeaders[header].mapNum == MAP_NUM(profile->map))
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

TEST("Wayfarer native HM windows cover all 3888 approved regional cells")
{
    u16 row = 0;
    u16 parameterRow;
    u16 rating;
    const struct RegionalCoverage *cell;

    for (parameterRow = 0; parameterRow < ARRAY_COUNT(sRegionalCoverage); parameterRow++)
        PARAMETRIZE_LABEL("%s", sRegionalCoverage[parameterRow].name) { row = parameterRow; }

    EXPECT_EQ(ARRAY_COUNT(sRegionalCoverage) * 81, 3888);
    cell = &sRegionalCoverage[row];
    SetCoverageMode(cell->modern);
    for (rating = 0; rating <= 80; rating++)
    {
        const struct CoverageProfile *profile = &sRegionalProfiles[cell->profiles[rating]];
        struct WildEncounterProfileView view;
        struct CoverageChance chance;

        EXPECT(ResolveCoverageProfile(profile, &view));
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

TEST("Wayfarer native HM acquisition has a source in all 10692 directional cases")
{
    u8 scenarioId = 0, modern = FALSE, clock = TIME_DAY, rod = WILD_ENCOUNTER_FISHING_ROD_OLD;
    u8 parameterScenario, parameterMode, parameterClock, parameterRod, parameterBlock;
    u8 ratingStart = 0;
    u16 rating;
    const struct AcquisitionScenario *scenario;

    for (parameterScenario = 0; parameterScenario < ARRAY_COUNT(sAcquisitionScenarios); parameterScenario++)
    for (parameterMode = 0; parameterMode < 2; parameterMode++)
    for (parameterClock = 0; parameterClock < 2; parameterClock++)
    for (parameterRod = WILD_ENCOUNTER_FISHING_ROD_OLD; parameterRod <= WILD_ENCOUNTER_FISHING_ROD_SUPER; parameterRod++)
    for (parameterBlock = 0; parameterBlock < 9; parameterBlock++)
        PARAMETRIZE_LABEL("%s mode %d clock %d rod %d TR %d-%d", sAcquisitionScenarios[parameterScenario].name, parameterMode, parameterClock, parameterRod, parameterBlock * 9, parameterBlock * 9 + 8)
        {
            scenarioId = parameterScenario;
            modern = parameterMode;
            clock = parameterClock == 0 ? TIME_DAY : TIME_NIGHT;
            rod = parameterRod;
            ratingStart = parameterBlock * 9;
        }

    EXPECT_EQ(ARRAY_COUNT(sAcquisitionScenarios) * 2 * 2 * 3 * 81, 10692);
    scenario = &sAcquisitionScenarios[scenarioId];
    SetCoverageMode(modern);
    // Bound each parameter case below the mechanics runner's GBA timeout.
    // Nine disjoint blocks still enumerate every supported rating exactly once.
    for (rating = ratingStart; rating < ratingStart + 9; rating++)
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
                ":L%s:%d: scenario %s mode %d time %d rod %d TR %d has no qualifying single source",
                gTestRunnerState.test->filename, __LINE__, scenario->name, modern, clock, rod, rating);
    }
}
#endif
