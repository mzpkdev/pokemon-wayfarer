#include "global.h"
#include "test/test.h"
#include "wild_encounter.h"

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

TEST("Wild encounter scaling keeps profile slices and fishing rod partitions exact")
{
    struct WildEncounterProfileView view;
    const struct WildPokemon *entry;
    u8 mirroredSlot;

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

    ASSUME(FindFirstProfile(WILD_AREA_FISHING, WILD_ENCOUNTER_FISHING_ROD_OLD, &view));
    EXPECT_EQ(view.entryStart, 0);
    EXPECT_EQ(view.entryCount, 2);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 100);
    EXPECT(!GetWildEncounterProfileEntry(&view, 2, &entry));
    EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&view, 0, FALSE, 0, &mirroredSlot));
    EXPECT_EQ(mirroredSlot, 1);

    ASSUME(FindFirstProfile(WILD_AREA_FISHING, WILD_ENCOUNTER_FISHING_ROD_GOOD, &view));
    EXPECT_EQ(view.entryStart, 2);
    EXPECT_EQ(view.entryCount, 3);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 100);
    EXPECT(!GetWildEncounterProfileEntry(&view, 1, &entry));
    EXPECT(!GetWildEncounterProfileEntry(&view, 5, &entry));
    EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&view, 0, FALSE, 2, &mirroredSlot));
    EXPECT_EQ(mirroredSlot, 4);

    ASSUME(FindFirstProfile(WILD_AREA_FISHING, WILD_ENCOUNTER_FISHING_ROD_SUPER, &view));
    EXPECT_EQ(view.entryStart, 5);
    EXPECT_EQ(view.entryCount, 5);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 100);
    EXPECT(!GetWildEncounterProfileEntry(&view, 4, &entry));
    EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&view, 0, FALSE, 5, &mirroredSlot));
    EXPECT_EQ(mirroredSlot, 9);
}

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
    struct WildEncounterSpeciesOutcome outcome;
    u8 slot;
    u8 mirroredSlot;

    EXPECT(GetWildEncounterSpeciesOutcome(&evolutionView, 0, 32, 0, FALSE, &outcome));
    EXPECT_EQ(outcome.species, SPECIES_BULBASAUR);
    // Golem has both a numeric Graveler predecessor and a trade evolution
    // route. Its unambiguous numeric chain still reverses at a low rating.
    EXPECT(GetWildEncounterSpeciesOutcome(&alternateEvolutionView, 0, 51, 0, FALSE, &outcome));
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

TEST("Wild encounter scaling treats all-locked profiles as no encounter")
{
    struct WildEncounterProfileView view = MakeTestProfile(&sFloorInfo, sFloorWeights, 1);
    u8 slot;

    // The fishing caller uses this failed selection to return before it
    // advances fishing stats, sets the angler species, or starts a battle.
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view, 0, FALSE), 0);
    EXPECT(!SelectWildEncounterProfileSlot(&view, 0, FALSE, 0, &slot));
}

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
