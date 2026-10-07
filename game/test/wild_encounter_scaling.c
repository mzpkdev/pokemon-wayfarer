#include "global.h"
#include "bug_contest.h"
#include "event_data.h"
#include "fake_rtc.h"
#include "fishing.h"
#include "item.h"
#include "malloc.h"
#include "notable_moves.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "task.h"
#include "randomizer.h"
#include "rtc.h"
#include "test/test.h"
#include "trainer_rating.h"
#include "wild_encounter.h"
#include "constants/item.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/metatile_behaviors.h"

// ---- Synthetic profiles for the generic selection helpers ----

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
    { 1, 1, SPECIES_NONE },
    { 10, 10, SPECIES_MAGIKARP },
};

static const struct WildPokemonInfo sFloorInfo =
{
    .encounterRate = 1,
    .wildPokemon = sFloorMons,
};

static const u8 sFloorWeights[] = { 70, 30 };

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

    EXPECT(FindFirstProfile(WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE, &view));
    EXPECT_EQ(view.entryStart, 0);
    EXPECT_EQ(view.entryCount, LAND_WILD_COUNT);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view), 100);
    EXPECT(GetWildEncounterProfileEntry(&view, 0, &entry));
    EXPECT_EQ(entry, &view.wildMonsInfo->wildPokemon[0]);

    EXPECT(FindFirstProfile(WILD_AREA_WATER, WILD_ENCOUNTER_FISHING_ROD_NONE, &view));
    EXPECT_EQ(view.entryStart, 0);
    EXPECT_EQ(view.entryCount, WATER_WILD_COUNT);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view), 100);

    EXPECT(FindFirstProfile(WILD_AREA_ROCKS, WILD_ENCOUNTER_FISHING_ROD_NONE, &view));
    EXPECT_EQ(view.entryStart, 0);
    EXPECT_EQ(view.entryCount, ROCK_WILD_COUNT);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view), 100);

    for (rod = WILD_ENCOUNTER_FISHING_ROD_OLD; rod <= WILD_ENCOUNTER_FISHING_ROD_SUPER; rod++)
    {
        EXPECT(FindFirstProfile(WILD_AREA_FISHING, rod, &view));
        EXPECT_EQ(view.entryStart, 0);
        EXPECT_EQ(view.entryCount, FISH_WILD_COUNT);
        for (slot = 0; slot < FISH_WILD_COUNT; slot++)
            EXPECT_EQ(view.weights[slot], expected[rod][slot]);
        EXPECT(GetWildEncounterProfileEntry(&view, 9, &entry));
        EXPECT(!GetWildEncounterProfileEntry(&view, 10, &entry));
        EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&view, 0, &mirroredSlot));
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
        EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view), 100);
        boundary = 0;
        for (candidate = 0; candidate < FISH_WILD_COUNT; candidate++)
        {
            EXPECT(SelectWildEncounterProfileSlot(&view, boundary, &slot));
            EXPECT_EQ(slot, candidate);
            boundary += view.weights[candidate];
            EXPECT(SelectWildEncounterProfileSlot(&view, boundary - 1, &slot));
            EXPECT_EQ(slot, candidate);
        }
        EXPECT_EQ(boundary, 100);
        EXPECT(!SelectWildEncounterProfileSlot(&view, boundary, &slot));
    }
}

#if IS_HNS
TEST("Hoenn Sound picks a Hoenn slot only when the radio is playing and falls through otherwise")
{
    struct WildEncounterProfileView hoennSoundView = MakeTestProfile(&sHoennSoundSelectionInfo, sHoennSoundSelectionWeights, ARRAY_COUNT(sHoennSoundSelectionMons));
    u8 slot;

    EXPECT(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&hoennSoundView, FALSE, 0, 0, 0, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&hoennSoundView, TRUE, 0, 0, 0, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&hoennSoundView, TRUE, 0, 1, 0, &slot));
    EXPECT_EQ(slot, 2);
    EXPECT(SelectWildEncounterProfileSlotWithHoennSoundForTesting(&hoennSoundView, TRUE, 1, 0, 0, &slot));
    EXPECT_EQ(slot, 0);
}
#endif

TEST("Wild encounter profiles reject unsupported or malformed contexts")
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

TEST("Wild encounter profiles select using only the slots that hold a species")
{
    struct WildEncounterProfileView view = MakeTestProfile(&sSelectionInfo, sSelectionWeights, ARRAY_COUNT(sSelectionMons));
    u8 slot;

    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view), 100);
    EXPECT_EQ(GetWildEncounterProfileEffectiveWeight(&view, 0), 70);
    EXPECT_EQ(GetWildEncounterProfileEffectiveWeight(&view, 1), 30);
    EXPECT(SelectWildEncounterProfileSlot(&view, 0, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(SelectWildEncounterProfileSlot(&view, 69, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(SelectWildEncounterProfileSlot(&view, 70, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(SelectWildEncounterProfileSlot(&view, 99, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(!SelectWildEncounterProfileSlot(&view, 100, &slot));

    view.entryCount = LAND_WILD_COUNT + 1;
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view), 0);
    EXPECT(!SelectWildEncounterProfileSlot(&view, 0, &slot));
}

TEST("Wild encounter profiles skip empty slots in selection and the Lure mirror")
{
    struct WildEncounterProfileView floorView = MakeTestProfile(&sFloorInfo, sFloorWeights, ARRAY_COUNT(sFloorMons));
    u8 slot;
    u8 mirroredSlot;

    EXPECT(!IsWildEncounterProfileSlotEligible(&floorView, 0));
    EXPECT_EQ(GetWildEncounterProfileEffectiveWeight(&floorView, 0), 0);
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&floorView), 30);
    EXPECT(SelectWildEncounterProfileSlot(&floorView, 0, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(SelectWildEncounterProfileSlot(&floorView, 29, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(!SelectWildEncounterProfileSlot(&floorView, 30, &slot));
    // The lure mirror works over the eligible sequence, never the raw indices.
    EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&floorView, 1, &mirroredSlot));
    EXPECT_EQ(mirroredSlot, 1);
    EXPECT(!GetWildEncounterProfileMirroredEligibleSlot(&floorView, 0, &mirroredSlot));
}

TEST("Standard Rod: all-empty profiles produce no encounter")
{
    struct WildEncounterProfileView view = MakeTestProfile(&sFloorInfo, sFloorWeights, 1);
    u8 slot;

    // The fishing caller uses this failed selection to return before it
    // advances fishing stats, sets the angler species, or starts a battle.
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view), 0);
    EXPECT(!SelectWildEncounterProfileSlot(&view, 0, &slot));
}

TEST("Standard Rod: fishing excludes empty slots")
{
    static const u8 weights[] = { 70, 30 };
    struct WildEncounterProfileView view = MakeTestProfile(&sEmptySelectionInfo, weights, ARRAY_COUNT(sEmptySelectionMons));
    u8 slot;

    EXPECT(!IsWildEncounterProfileSlotEligible(&view, 0));
    EXPECT_EQ(GetWildEncounterProfileEligibleWeight(&view), 30);
    EXPECT(SelectWildEncounterProfileSlot(&view, 0, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(!GetWildEncounterProfileMirroredEligibleSlot(&view, 0, &slot));
    EXPECT(GetWildEncounterProfileMirroredEligibleSlot(&view, 1, &slot));
    EXPECT_EQ(slot, 1);
}

TEST("Standard Rod: fishing availability rejects zero rates and zero eligible data")
{
    static const u8 weights[] = { 70, 30 };
    struct WildEncounterProfileView view = MakeTestProfile(&sEmptySelectionInfo, weights, ARRAY_COUNT(sEmptySelectionMons));

    EXPECT(DoesWildEncounterProfileHaveAvailableEntries(&view));

    view.wildMonsInfo = &sZeroRateSelectionInfo;
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(&view));

    view.wildMonsInfo = &sEmptySelectionInfo;
    view.entryCount = 1;
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(&view));

    view.wildMonsInfo = NULL;
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(&view));
    EXPECT(!DoesWildEncounterProfileHaveAvailableEntries(NULL));
}

TEST("Wild encounter profiles apply type attraction over the slots that hold a species")
{
    struct WildEncounterProfileView selectionView = MakeTestProfile(&sSelectionInfo, sSelectionWeights, ARRAY_COUNT(sSelectionMons));
    struct WildEncounterProfileView floorView = MakeTestProfile(&sFloorInfo, sFloorWeights, ARRAY_COUNT(sFloorMons));
    u8 slot;

    // The normal two-slot profile keeps the raw game's uniform matching-slot
    // selection, independently of the authored 70/30 encounter weights.
    EXPECT(SelectWildEncounterProfileTypeSlot(&selectionView, TYPE_WATER, 0, &slot));
    EXPECT_EQ(slot, 0);
    EXPECT(SelectWildEncounterProfileTypeSlot(&selectionView, TYPE_NORMAL, 0, &slot));
    EXPECT_EQ(slot, 1);
    EXPECT(!SelectWildEncounterProfileTypeSlot(&selectionView, TYPE_WATER, 1, &slot));

    // An empty slot cannot be chosen. The sole eligible Water slot is the
    // legacy no-op case (every eligible slot already matches).
    EXPECT(!SelectWildEncounterProfileTypeSlot(&floorView, TYPE_NORMAL, 0, &slot));
    EXPECT(!SelectWildEncounterProfileTypeSlot(&floorView, TYPE_WATER, 0, &slot));
}

#if IS_WAYFARER

void StartMarowakBattle(void);

// ======== Wild level scaling v2 (specs/wild-level-scaling.md), through production code ========

static u32 CountHeaders(void)
{
    u32 count;

    for (count = 0; gWildMonHeaders[count].mapGroup != MAP_GROUP(MAP_UNDEFINED); count++)
    {
    }
    return count;
}

static u32 FindPlace(u32 reach, u32 region)
{
    u32 header, count = CountHeaders();

    for (header = 0; header < count; header++)
        if (gWildEncounterPlaces[header].reach == reach && gWildEncounterPlaces[header].region == region)
            return header;
    return HEADER_NONE;
}

// The spec's scalers, with the scalers section's rounding (halves round up).
struct RefAnchor
{
    u16 tr;
    u16 value;
};

static const struct RefAnchor sRoadLevel[] = { { 0, 5 }, { 40, 20 }, { 80, 38 }, { 120, 56 }, { 160, 74 } };
static const struct RefAnchor sWildsBonus[] = { { 0, 4 }, { 80, 6 }, { 160, 11 } };
static const struct RefAnchor sOutlandsBonus[] = { { 0, 8 }, { 40, 8 }, { 80, 12 }, { 160, 24 } };

static u32 RefScaler(const struct RefAnchor *anchors, u32 count, u32 tr)
{
    u32 i;

    for (i = 1; i < count; i++)
    {
        if (tr < anchors[i].tr)
        {
            u32 d = anchors[i].tr - anchors[i - 1].tr;

            return anchors[i - 1].value + (2 * (tr - anchors[i - 1].tr) * (anchors[i].value - anchors[i - 1].value) + d) / (2 * d);
        }
    }
    return anchors[count - 1].value;
}

static u32 RefRoad(u32 tr)
{
    return RefScaler(sRoadLevel, ARRAY_COUNT(sRoadLevel), tr);
}

static u32 RefWilds(u32 tr)
{
    return RefRoad(tr) + RefScaler(sWildsBonus, ARRAY_COUNT(sWildsBonus), tr);
}

static u32 RefOutlands(u32 tr)
{
    return RefRoad(tr) + RefScaler(sOutlandsBonus, ARRAY_COUNT(sOutlandsBonus), tr);
}

// First and deepest floor level of an intent, and the intent's floor (0 none), before the cap at 100.
static void RefDungeonRange(u32 intent, u32 tr, u32 *first, u32 *deepest, u32 *floor)
{
    u32 road = RefRoad(tr), wilds = RefWilds(tr), outlands = RefOutlands(tr);

    *floor = 0;
    switch (intent)
    {
    case WILD_DUNGEON_MILD:
        *first = (road + wilds) / 2;
        *deepest = wilds;
        break;
    case WILD_DUNGEON_MILD_TO_MODERATE:
        *first = (road + wilds) / 2;
        *deepest = (wilds + outlands) / 2;
        break;
    case WILD_DUNGEON_MODERATE:
        *first = wilds;
        *deepest = (wilds + outlands) / 2;
        break;
    case WILD_DUNGEON_MODERATE_TO_HARD:
        *first = wilds;
        *deepest = outlands;
        break;
    case WILD_DUNGEON_HARD:
        *first = (wilds + outlands) / 2;
        *deepest = outlands + 3;
        break;
    default:
        *first = outlands;
        *deepest = outlands + 6;
        *floor = 50;
        break;
    }
}

// The spec's level of any place at a Trainer Rating.
static u32 RefPlaceLevel(u32 header, u32 tr)
{
    const struct WildEncounterPlace *place = &gWildEncounterPlaces[header];
    u32 first, deepest, floor, level;

    switch (place->reach)
    {
    case WILD_REACH_ROAD:
        return min(RefRoad(tr), MAX_LEVEL);
    case WILD_REACH_WILDS:
        return min(RefWilds(tr), MAX_LEVEL);
    case WILD_REACH_OUTLANDS:
        return min(RefOutlands(tr), MAX_LEVEL);
    }
    RefDungeonRange(place->intent, tr, &first, &deepest, &floor);
    if (place->flat || place->floorCount <= 1)
        level = (first + deepest) / 2;
    else
        level = first + (2 * (deepest - first) * place->floor + (place->floorCount - 1)) / (2 * (place->floorCount - 1));
    return min(max(level, floor), MAX_LEVEL);
}

TEST("Wild level scaling: every scaler hits its anchors, midpoints and adjacent ratings, and stays flat past 160")
{
    // Spec-derived values at anchors, midpoints and the ratings next to each anchor.
    static const struct RefAnchor roadChecks[] =
    {
        { 0, 5 }, { 1, 5 }, { 20, 13 }, { 39, 20 }, { 40, 20 }, { 41, 20 }, { 60, 29 }, { 79, 38 }, { 80, 38 }, { 81, 38 },
        { 100, 47 }, { 119, 56 }, { 120, 56 }, { 121, 56 }, { 140, 65 }, { 159, 74 }, { 160, 74 },
    };
    static const struct RefAnchor wildsBonusChecks[] =
    {
        { 0, 4 }, { 1, 4 }, { 40, 5 }, { 79, 6 }, { 80, 6 }, { 81, 6 }, { 120, 9 }, { 159, 11 }, { 160, 11 },
    };
    static const struct RefAnchor outlandsBonusChecks[] =
    {
        { 0, 8 }, { 1, 8 }, { 20, 8 }, { 39, 8 }, { 40, 8 }, { 41, 8 }, { 60, 10 }, { 79, 12 }, { 80, 12 }, { 81, 12 },
        { 120, 18 }, { 159, 24 }, { 160, 24 },
    };
    static const u32 sBeyond[] = { 161, 162, 200, 255, 256, 1000, 65535, 65536, 0xFFFFFFFF };
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 wilds = FindPlace(WILD_REACH_WILDS, WILD_PLACE_REGION_OTHER);
    u32 outlands = FindPlace(WILD_REACH_OUTLANDS, WILD_PLACE_REGION_OTHER);
    u32 i, tr;

    EXPECT_NE(road, HEADER_NONE);
    EXPECT_NE(wilds, HEADER_NONE);
    EXPECT_NE(outlands, HEADER_NONE);

    for (i = 0; i < ARRAY_COUNT(roadChecks); i++)
        EXPECT_EQ(GetWildEncounterPlaceLevel(road, roadChecks[i].tr), roadChecks[i].value);
    for (i = 0; i < ARRAY_COUNT(wildsBonusChecks); i++)
        EXPECT_EQ(GetWildEncounterPlaceLevel(wilds, wildsBonusChecks[i].tr) - GetWildEncounterPlaceLevel(road, wildsBonusChecks[i].tr), wildsBonusChecks[i].value);
    for (i = 0; i < ARRAY_COUNT(outlandsBonusChecks); i++)
        EXPECT_EQ(GetWildEncounterPlaceLevel(outlands, outlandsBonusChecks[i].tr) - GetWildEncounterPlaceLevel(road, outlandsBonusChecks[i].tr), outlandsBonusChecks[i].value);

    // Every rating from 0 past the last anchor follows the scalers, and only ever rises.
    for (tr = 0; tr <= 200; tr++)
    {
        EXPECT_EQ(GetWildEncounterPlaceLevel(road, tr), RefRoad(tr));
        EXPECT_EQ(GetWildEncounterPlaceLevel(wilds, tr), RefWilds(tr));
        EXPECT_EQ(GetWildEncounterPlaceLevel(outlands, tr), RefOutlands(tr));
        if (tr > 0)
        {
            EXPECT_GE(GetWildEncounterPlaceLevel(road, tr), GetWildEncounterPlaceLevel(road, tr - 1));
            EXPECT_GE(GetWildEncounterPlaceLevel(wilds, tr), GetWildEncounterPlaceLevel(wilds, tr - 1));
            EXPECT_GE(GetWildEncounterPlaceLevel(outlands, tr), GetWildEncounterPlaceLevel(outlands, tr - 1));
        }
    }

    // Flat past 160.
    for (i = 0; i < ARRAY_COUNT(sBeyond); i++)
    {
        EXPECT_EQ(GetWildEncounterPlaceLevel(road, sBeyond[i]), 74);
        EXPECT_EQ(GetWildEncounterPlaceLevel(wilds, sBeyond[i]), 85);
        EXPECT_EQ(GetWildEncounterPlaceLevel(outlands, sBeyond[i]), 98);
    }
    EXPECT_EQ(GetWildEncounterPlaceLevel(HEADER_NONE, 0), 0);
}

TEST("Wild level scaling: every reach reproduces the at-a-glance table at its badge ratings")
{
    // Badges 0, 2, 4, 6, 8, 12, 16, 20, 24 are Trainer Ratings 0, 20, ... 160.
    static const u8 sTable[][4] =
    {
        {   0,  5,  9, 13 }, {  20, 13, 18, 21 }, {  40, 20, 25, 28 }, {  60, 29, 35, 39 }, {  80, 38, 44, 50 },
        { 100, 47, 54, 62 }, { 120, 56, 65, 74 }, { 140, 65, 75, 86 }, { 160, 74, 85, 98 },
    };
    u32 header, count = CountHeaders(), i;
    u32 seen[3] = { 0, 0, 0 };

    EXPECT_GT(count, 0);
    for (header = 0; header < count; header++)
    {
        u32 reach = gWildEncounterPlaces[header].reach;

        // Every Road, Wilds and Outlands map follows its scaler; dungeons follow their intent below.
        if (reach == WILD_REACH_DUNGEON)
            continue;
        seen[reach]++;
        for (i = 0; i < ARRAY_COUNT(sTable); i++)
            EXPECT_EQ(GetWildEncounterPlaceLevel(header, sTable[i][0]), sTable[i][1 + reach]);
    }
    EXPECT_GT(seen[WILD_REACH_ROAD], 0);
    EXPECT_GT(seen[WILD_REACH_WILDS], 0);
    EXPECT_GT(seen[WILD_REACH_OUTLANDS], 0);
}

TEST("Wild level scaling: every dungeon intent reproduces the at-a-glance table at its badge ratings")
{
    // Ratings for badges 0, 4, 8, 16, 24; first floor then deepest floor per intent (before the cap at 100).
    static const u16 sRatings[] = { 0, 40, 80, 120, 160 };
    static const u8 sTable[][6][2] =
    {
        { {  7,  9 }, {  7, 11 }, {  9, 11 }, {  9, 13 }, { 11,  16 }, { 50,  50 } },
        { { 22, 25 }, { 22, 26 }, { 25, 26 }, { 25, 28 }, { 26,  31 }, { 50,  50 } },
        { { 41, 44 }, { 41, 47 }, { 44, 47 }, { 44, 50 }, { 47,  53 }, { 50,  56 } },
        { { 60, 65 }, { 60, 69 }, { 65, 69 }, { 65, 74 }, { 69,  77 }, { 74,  80 } },
        { { 79, 85 }, { 79, 91 }, { 85, 91 }, { 85, 98 }, { 91, 101 }, { 98, 104 } },
    };
    u32 header, count = CountHeaders(), i;
    u32 firstSeen[6] = { 0 }, deepestSeen[6] = { 0 }, flatSeen[6] = { 0 };

    for (header = 0; header < count; header++)
    {
        const struct WildEncounterPlace *place = &gWildEncounterPlaces[header];

        if (place->reach != WILD_REACH_DUNGEON)
            continue;
        for (i = 0; i < ARRAY_COUNT(sRatings); i++)
        {
            u32 first = sTable[i][place->intent][0], deepest = sTable[i][place->intent][1];
            u32 level = GetWildEncounterPlaceLevel(header, sRatings[i]);

            if (place->flat || place->floorCount <= 1)
            {
                // A single-floor or flat dungeon uses the middle of its range, rounded down.
                EXPECT_EQ(level, min((first + deepest) / 2, MAX_LEVEL));
                if (i == 0)
                    flatSeen[place->intent]++;
            }
            else if (place->floor == 0)
            {
                EXPECT_EQ(level, min(first, MAX_LEVEL));
                if (i == 0)
                    firstSeen[place->intent]++;
            }
            else if (place->floor == place->floorCount - 1)
            {
                EXPECT_EQ(level, min(deepest, MAX_LEVEL));
                if (i == 0)
                    deepestSeen[place->intent]++;
            }
        }
    }
    // Every intent is reproduced from both its first and its deepest floor.
    for (i = 0; i < 6; i++)
    {
        EXPECT_GT(firstSeen[i], 0);
        EXPECT_GT(deepestSeen[i], 0);
    }
}

TEST("Wild level scaling: every dungeon floor lies between its first and deepest floors and rises with depth")
{
    u32 header, other, count = CountHeaders(), tr;

    for (header = 0; header < count; header++)
    {
        const struct WildEncounterPlace *place = &gWildEncounterPlaces[header];

        if (place->reach != WILD_REACH_DUNGEON)
            continue;
        for (tr = 0; tr <= 200; tr++)
        {
            u32 first, deepest, floor;
            u32 level = GetWildEncounterPlaceLevel(header, tr);

            RefDungeonRange(place->intent, tr, &first, &deepest, &floor);
            first = max(first, floor);
            deepest = max(deepest, floor);
            EXPECT_GE(level, min(first, MAX_LEVEL));
            EXPECT_LE(level, min(deepest, MAX_LEVEL));
            EXPECT_EQ(level, RefPlaceLevel(header, tr));
            // A Brutal dungeon holds level 50 as a minimum on every map, whatever the rating.
            if (place->intent == WILD_DUNGEON_BRUTAL)
                EXPECT_GE(level, 50);
        }
    }

    // Floors rise with depth: the next floor of a dungeon of the same shape is never lower.
    for (header = 0; header < count; header++)
    {
        const struct WildEncounterPlace *place = &gWildEncounterPlaces[header];

        if (place->reach != WILD_REACH_DUNGEON || place->flat || place->floorCount <= 1 || place->floor + 1 >= place->floorCount)
            continue;
        for (other = 0; other < count; other++)
        {
            const struct WildEncounterPlace *deeper = &gWildEncounterPlaces[other];

            if (deeper->reach != WILD_REACH_DUNGEON || deeper->intent != place->intent || deeper->flat
             || deeper->floorCount != place->floorCount || deeper->floor != place->floor + 1)
                continue;
            for (tr = 0; tr <= 160; tr += 4)
                EXPECT_GE(GetWildEncounterPlaceLevel(other, tr), GetWildEncounterPlaceLevel(header, tr));
        }
    }
}

TEST("Wild level scaling: no place ever exceeds level 100")
{
    u32 header, count = CountHeaders(), tr;

    for (header = 0; header < count; header++)
        for (tr = 0; tr <= 200; tr += 5)
            EXPECT_LE(GetWildEncounterPlaceLevel(header, tr), MAX_LEVEL);
    // Hard and Brutal dungeons above 100 become 100 at the end of the scale.
    for (header = 0; header < count; header++)
        if (gWildEncounterPlaces[header].reach == WILD_REACH_DUNGEON && gWildEncounterPlaces[header].intent == WILD_DUNGEON_BRUTAL
         && gWildEncounterPlaces[header].floor + 1 == gWildEncounterPlaces[header].floorCount)
            EXPECT_EQ(GetWildEncounterPlaceLevel(header, 160), MAX_LEVEL);
}

TEST("Wild level scaling: Road maps roll 60 percent of the table encounter rate, rounded to the nearest value")
{
    static const u8 sRoadRates[] = { 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 7, 7, 8, 8, 9, 10, 10, 11, 11, 12 };
    u32 header, count = CountHeaders(), rate;
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 seen[4] = { 0 };

    EXPECT_NE(road, HEADER_NONE);
    for (rate = 0; rate < ARRAY_COUNT(sRoadRates); rate++)
        EXPECT_EQ(GetWildEncounterRateForHeader(road, rate), sRoadRates[rate]);
    EXPECT_EQ(GetWildEncounterRateForHeader(road, 255), 153);
    // Wilds, Outlands and dungeons use the table's rate as it is, and so does an unknown header.
    for (header = 0; header < count; header++)
    {
        u32 reach = gWildEncounterPlaces[header].reach;

        seen[reach]++;
        if (reach == WILD_REACH_ROAD)
            EXPECT_EQ(GetWildEncounterRateForHeader(header, 20), 12);
        else
            for (rate = 0; rate <= 40; rate++)
                EXPECT_EQ(GetWildEncounterRateForHeader(header, rate), rate);
    }
    EXPECT_EQ(GetWildEncounterRateForHeader(HEADER_NONE, 20), 20);
    EXPECT_GT(seen[WILD_REACH_ROAD], 0);
    EXPECT_GT(seen[WILD_REACH_WILDS], 0);
    EXPECT_GT(seen[WILD_REACH_OUTLANDS], 0);
    EXPECT_GT(seen[WILD_REACH_DUNGEON], 0);
}

// ---- Encounter level and stage mix ----

// The outcomes of a slot capped at a species, placed in a header, at a Trainer Rating.
static u32 SlotOutcomes(u16 cap, u32 header, u32 tr, struct WildEncounterSlotOutcome *outcomes)
{
    static const u8 weights[] = { 100 };
    struct WildPokemon mon = { 1, 1, cap };
    struct WildPokemonInfo info = { 1, &mon };
    struct WildEncounterProfileView view = MakeTestProfile(&info, weights, 1);

    view.headerId = header;
    return GetWildEncounterSlotOutcomes(&view, 0, tr, outcomes);
}

static u32 OutcomeWeight(const struct WildEncounterSlotOutcome *outcomes, u32 count, u16 species, u32 level)
{
    u32 i, weight = 0;

    for (i = 0; i < count; i++)
        if (outcomes[i].species == species && outcomes[i].level == level)
            weight += outcomes[i].weight;
    return weight;
}

static u32 OutcomeSpeciesWeight(const struct WildEncounterSlotOutcome *outcomes, u32 count, u16 species)
{
    u32 i, weight = 0;

    for (i = 0; i < count; i++)
        if (outcomes[i].species == species)
            weight += outcomes[i].weight;
    return weight;
}

TEST("Wild encounter level: every method rolls the place level from minus two to plus two, equally likely")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 count = SlotOutcomes(SPECIES_TAUROS, road, 80, outcomes); // Road level 38, no evolution, not a prowler
    u32 i;

    EXPECT_EQ(GetSpeciesLowestEvolutionLevel(SPECIES_TAUROS), 0);
    EXPECT(!IsBabySpecies(SPECIES_TAUROS));
    EXPECT_EQ(count, 5);
    for (i = 0; i < count; i++)
    {
        EXPECT_EQ(outcomes[i].species, SPECIES_TAUROS);
        EXPECT_EQ(outcomes[i].level, 36 + i);
        EXPECT_EQ(outcomes[i].weight, 10);
    }
    // The level is at least 1 and at most 100: the top of the scale clamps.
    count = SlotOutcomes(SPECIES_TAUROS, FindPlace(WILD_REACH_OUTLANDS, WILD_PLACE_REGION_OTHER), 160, outcomes); // 98
    EXPECT_EQ(count, 5);
    EXPECT_EQ(OutcomeWeight(outcomes, count, SPECIES_TAUROS, 96), 10);
    EXPECT_EQ(OutcomeWeight(outcomes, count, SPECIES_TAUROS, 100), 10);
    for (i = 0; i < count; i++)
        EXPECT_LE(outcomes[i].level, MAX_LEVEL);
}

TEST("Wild stage mix: a Pidgeotto slot at level 20 is Pidgeotto 30 percent of the time, and always Pidgeotto from level 27")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 count, level;
    u8 evolutionLevel;

    // Pidgey evolves at 18: the keep chance is 10 percent at 18 and rises 10 percent a level.
    EXPECT_EQ(GetSpeciesStepDownPredecessor(SPECIES_PIDGEOTTO, &evolutionLevel), SPECIES_PIDGEY);
    EXPECT_EQ(evolutionLevel, 18);

    count = SlotOutcomes(SPECIES_PIDGEOTTO, road, 40, outcomes); // Road level 20: encounter levels 18..22
    EXPECT_EQ(count, 10);
    for (level = 18; level <= 22; level++)
    {
        u32 keep = level - 17;

        EXPECT_EQ(OutcomeWeight(outcomes, count, SPECIES_PIDGEOTTO, level), keep);
        EXPECT_EQ(OutcomeWeight(outcomes, count, SPECIES_PIDGEY, level), 10 - keep);
    }
    // The level-20 encounter itself: 30 percent Pidgeotto.
    EXPECT_EQ(OutcomeWeight(outcomes, count, SPECIES_PIDGEOTTO, 20), 3);
    EXPECT_EQ(OutcomeWeight(outcomes, count, SPECIES_PIDGEY, 20), 7);

    // From level 27 it is always Pidgeotto: Road level 29 gives levels 27..31.
    count = SlotOutcomes(SPECIES_PIDGEOTTO, road, 60, outcomes);
    EXPECT_EQ(count, 5);
    EXPECT_EQ(OutcomeSpeciesWeight(outcomes, count, SPECIES_PIDGEOTTO), 50);

    // Below the evolution level the slot holds only the young stage: Road level 5 gives levels 3..7.
    count = SlotOutcomes(SPECIES_PIDGEOTTO, road, 0, outcomes);
    EXPECT_EQ(count, 5);
    EXPECT_EQ(OutcomeSpeciesWeight(outcomes, count, SPECIES_PIDGEY), 50);
}

TEST("Wild stage mix: the downward rule steps through every stage, and never into a baby")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 count, i;

    // Pikachu's baby, Pichu, is never a step down: low levels stay Pikachu.
    count = SlotOutcomes(SPECIES_PIKACHU, road, 0, outcomes);
    for (i = 0; i < count; i++)
        EXPECT_EQ(outcomes[i].species, SPECIES_PIKACHU);
    EXPECT_EQ(OutcomeSpeciesWeight(outcomes, count, SPECIES_PIKACHU), 50);
    // A Venusaur slot at Road level 5 is Bulbasaur; at 20 (levels 18..22) Ivysaur (evolves at 16) and Bulbasaur mix.
    count = SlotOutcomes(SPECIES_VENUSAUR, road, 0, outcomes);
    EXPECT_EQ(OutcomeSpeciesWeight(outcomes, count, SPECIES_BULBASAUR), 50);
    count = SlotOutcomes(SPECIES_VENUSAUR, road, 40, outcomes);
    EXPECT_EQ(OutcomeSpeciesWeight(outcomes, count, SPECIES_VENUSAUR), 0);
    EXPECT_GT(OutcomeSpeciesWeight(outcomes, count, SPECIES_IVYSAUR), 0);
    EXPECT_EQ(OutcomeSpeciesWeight(outcomes, count, SPECIES_IVYSAUR) + OutcomeSpeciesWeight(outcomes, count, SPECIES_BULBASAUR), 50);
    // A baby slot is the baby, whatever the level.
    count = SlotOutcomes(SPECIES_PICHU, road, 160, outcomes);
    for (i = 0; i < count; i++)
        EXPECT_EQ(outcomes[i].species, SPECIES_PICHU);
}

TEST("Wild young levels: a slot capped at an early stage stays below its next evolution level, babies at 10 or under")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 count, i;
    u8 evolutionLevel;

    // A Caterpie slot is at most level 6, even where Roads are level 74.
    count = SlotOutcomes(SPECIES_CATERPIE, road, 160, outcomes);
    for (i = 0; i < count; i++)
    {
        EXPECT_EQ(outcomes[i].species, SPECIES_CATERPIE);
        EXPECT_EQ(outcomes[i].level, 6);
    }
    EXPECT_EQ(OutcomeWeight(outcomes, count, SPECIES_CATERPIE, 6), 50);
    // A Pikachu slot stays under Raichu's level in the shared evolution-level table.
    EXPECT_EQ(GetSpeciesStepDownPredecessor(SPECIES_RAICHU, &evolutionLevel), SPECIES_PIKACHU);
    EXPECT_GT(evolutionLevel, 0);
    count = SlotOutcomes(SPECIES_PIKACHU, road, 160, outcomes);
    for (i = 0; i < count; i++)
        EXPECT_EQ(outcomes[i].level, evolutionLevel - 1);
    // Babies are at most level 10.
    count = SlotOutcomes(SPECIES_PICHU, road, 160, outcomes);
    for (i = 0; i < count; i++)
        EXPECT_EQ(outcomes[i].level, 10);
    // A line that never evolves has no limit (Tauros at Road level 74 reaches 76).
    count = SlotOutcomes(SPECIES_TAUROS, road, 160, outcomes);
    EXPECT_EQ(OutcomeWeight(outcomes, count, SPECIES_TAUROS, 76), 10);
}

TEST("Wild prowlers: a prowler takes the higher of its level and its minimum, which wins over a young limit")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 outlands = FindPlace(WILD_REACH_OUTLANDS, WILD_PLACE_REGION_OTHER);
    u32 count, i;

    // Dratini is a dangerous prowler (30), so a young Dratini slot can hold a level-30 Dratini
    // even though Dragonair evolves at 30 and the young limit is 29.
    count = SlotOutcomes(SPECIES_DRATINI, road, 0, outcomes);
    EXPECT_EQ(count, 5);
    for (i = 0; i < count; i++)
    {
        EXPECT_EQ(outcomes[i].species, SPECIES_DRATINI);
        EXPECT_EQ(outcomes[i].level, 30);
    }
    // Beyond the minimum, the place's level and the young limit apply: Outlands level 98 stays at 29... until the minimum wins again.
    count = SlotOutcomes(SPECIES_DRATINI, outlands, 160, outcomes);
    for (i = 0; i < count; i++)
        EXPECT_EQ(outcomes[i].level, 30);
}

TEST("Wild prowlers: Kalos rewards in the Safari Zones and Sinjoh's residents have no minimum")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    u32 other = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 safari = HEADER_NONE, sinjoh = HEADER_NONE;
    u32 header, count = CountHeaders(), row, i, n;

    for (header = 0; header < count; header++)
    {
        if (safari == HEADER_NONE && gWildEncounterPlaces[header].region == WILD_PLACE_REGION_SAFARI)
            safari = header;
        if (sinjoh == HEADER_NONE && gWildEncounterPlaces[header].region == WILD_PLACE_REGION_SINJOH)
            sinjoh = header;
    }
    EXPECT_NE(safari, HEADER_NONE);
    EXPECT_NE(sinjoh, HEADER_NONE);
    EXPECT_GT(gWildProwlerMinimumCount, 0);

    for (row = 0; row < gWildProwlerMinimumCount; row++)
    {
        const struct WildProwlerMinimum *prowler = &gWildProwlerMinimums[row];

        EXPECT(prowler->minimumLevel == 20 || prowler->minimumLevel == 25 || prowler->minimumLevel == 30);
        // The same species on an ordinary place at Rating 0 (levels 3..7) is held at its minimum...
        n = SlotOutcomes(prowler->species, other, 0, outcomes);
        for (i = 0; i < n; i++)
            EXPECT_GE(outcomes[i].level, prowler->minimumLevel);
        // ...and without one where the table exempts it. Safari and Sinjoh maps are Wilds or Outlands
        // places, so their levels at Rating 0 sit well under every minimum.
        if (prowler->exemptInSafari)
        {
            n = SlotOutcomes(prowler->species, safari, 0, outcomes);
            for (i = 0; i < n; i++)
                EXPECT_LT(outcomes[i].level, prowler->minimumLevel);
        }
        if (prowler->exemptInSinjoh)
        {
            n = SlotOutcomes(prowler->species, sinjoh, 0, outcomes);
            for (i = 0; i < n; i++)
                EXPECT_LT(outcomes[i].level, prowler->minimumLevel);
        }
    }
}

TEST("Wild encounters: the readers' likeliest species is the species with the most weight")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    struct WildPokemon mon = { 1, 1, SPECIES_PIDGEOTTO };
    struct WildPokemonInfo info = { 1, &mon };
    static const u8 weights[] = { 100 };
    struct WildEncounterProfileView view = MakeTestProfile(&info, weights, 1);
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);

    view.headerId = road;
    SetTrainerRating(40); // levels 18..22: Pidgeotto 15 of 50, Pidgey 35
    EXPECT_EQ(GetCurrentWildEncounterSlotOutcomes(&view, 0, outcomes), 10);
    EXPECT_EQ(GetCurrentWildEncounterSlotLikelySpecies(&view, 0), SPECIES_PIDGEY);
    SetTrainerRating(60); // levels 27..31: always Pidgeotto
    EXPECT_EQ(GetCurrentWildEncounterSlotLikelySpecies(&view, 0), SPECIES_PIDGEOTTO);
    mon.species = SPECIES_NONE;
    EXPECT_EQ(GetCurrentWildEncounterSlotLikelySpecies(&view, 0), SPECIES_NONE);
    EXPECT_EQ(GetCurrentWildEncounterSlotOutcomes(&view, 0, outcomes), 0);
}

#if RANDOMIZER_AVAILABLE == TRUE
TEST("Wild randomizer: randomized encounters keep the place level and spread, and skip stage mix, young levels and prowler minimums")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    struct WildEncounterSpeciesOutcome rolled;
    static const u8 weights[] = { 100 };
    struct WildPokemon mon = { 1, 1, SPECIES_CATERPIE };
    struct WildPokemonInfo info = { 1, &mon };
    struct WildEncounterProfileView view = MakeTestProfile(&info, weights, 1);
    u32 road = FindPlace(WILD_REACH_ROAD, WILD_PLACE_REGION_OTHER);
    u32 count, i;

    view.headerId = road;
    gSaveBlock3Ptr->challengeSettings.tx_Random_WildPokemon = TRUE;
    SetTrainerRating(160); // Road level 74
    // A Caterpie slot is level 6 at most when not randomized; randomized, it holds 72..76 as the raw slot species.
    count = GetCurrentWildEncounterSlotOutcomes(&view, 0, outcomes);
    EXPECT_EQ(count, 5);
    for (i = 0; i < count; i++)
    {
        EXPECT_EQ(outcomes[i].species, SPECIES_CATERPIE);
        EXPECT_EQ(outcomes[i].level, 72 + i);
        EXPECT_EQ(outcomes[i].weight, 10);
    }
    for (i = 0; i < 50; i++)
    {
        EXPECT(RollWildEncounterSlot(&view, 0, &rolled));
        EXPECT_EQ(rolled.species, SPECIES_CATERPIE);
        EXPECT_GE(rolled.level, 72);
        EXPECT_LE(rolled.level, 76);
    }
    // A prowler keeps no minimum: Dratini at Road level 5.
    mon.species = SPECIES_DRATINI;
    SetTrainerRating(0);
    count = GetCurrentWildEncounterSlotOutcomes(&view, 0, outcomes);
    for (i = 0; i < count; i++)
        EXPECT_EQ(outcomes[i].level, 3 + i);
    gSaveBlock3Ptr->challengeSettings.tx_Random_WildPokemon = FALSE;
    count = GetCurrentWildEncounterSlotOutcomes(&view, 0, outcomes);
    EXPECT_EQ(outcomes[0].level, 30);
}

TEST("Standard Rod: fishing randomizer receives the authored species and raw slot")
{
    struct WildPokemon mon = { 1, 1, SPECIES_GYARADOS };
    struct WildPokemonInfo info = { 1, &mon };
    static const u8 weights[] = { 100 };
    struct WildEncounterProfileView view = MakeTestProfile(&info, weights, 1);
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

// ---- Rolls: abilities, Lures, Repel ----

static void SetLeadMon(u16 species, u8 level)
{
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], species, level, 0, OTID_STRUCT_PRESET(0));
    gPlayerPartyCount = 1;
}

TEST("Wild encounter level: Pressure, Hustle and Vital Spirit favour the top of the spread, Lures take one level above it")
{
    static const u16 sLeads[] = { SPECIES_ZAPDOS, SPECIES_CORSOLA, SPECIES_MANKEY };
    static const enum Ability sAbilities[] = { ABILITY_PRESSURE, ABILITY_HUSTLE, ABILITY_VITAL_SPIRIT };
    struct WildEncounterSpeciesOutcome outcome;
    static const u8 weights[] = { 100 };
    struct WildPokemon mon = { 1, 1, SPECIES_TAUROS };
    struct WildPokemonInfo info = { 1, &mon };
    struct WildEncounterProfileView view = MakeTestProfile(&info, weights, 1);
    u32 wilds = FindPlace(WILD_REACH_WILDS, WILD_PLACE_REGION_OTHER);
    u32 lead, i, top, bottom;
    u16 steps = VarGet(VAR_REPEL_STEP_COUNT);

    view.headerId = wilds;
    SetTrainerRating(80); // Wilds level 44: encounter levels 42..46
    VarSet(VAR_REPEL_STEP_COUNT, 0);
    SeedRng(1);

    // Plain lead: uniform, so the top is about a fifth.
    SetLeadMon(SPECIES_PIDGEY, 5);
    top = 0;
    for (i = 0; i < 500; i++)
    {
        EXPECT(RollWildEncounterSlot(&view, 0, &outcome));
        EXPECT_EQ(outcome.species, SPECIES_TAUROS);
        EXPECT_GE(outcome.level, 42);
        EXPECT_LE(outcome.level, 46);
        top += outcome.level == 46;
    }
    EXPECT_LT(top, 160);
    EXPECT_GT(top, 40);

    // Each ability: half the rolls are the top (P+2); never above it.
    for (lead = 0; lead < ARRAY_COUNT(sLeads); lead++)
    {
        SetLeadMon(sLeads[lead], 5);
        EXPECT_EQ(GetMonAbility(&gPlayerParty[0]), sAbilities[lead]);
        top = bottom = 0;
        for (i = 0; i < 500; i++)
        {
            EXPECT(RollWildEncounterSlot(&view, 0, &outcome));
            EXPECT_GE(outcome.level, 42);
            EXPECT_LE(outcome.level, 46);
            top += outcome.level == 46;
            bottom += outcome.level == 42;
        }
        EXPECT_GT(top, 200);
        EXPECT_LT(bottom, 140);
    }

    // A Lure takes one level above the spread.
    SetLeadMon(SPECIES_PIDGEY, 5);
    VarSet(VAR_REPEL_STEP_COUNT, REPEL_LURE_MASK | 50);
    for (i = 0; i < 20; i++)
    {
        EXPECT(RollWildEncounterSlot(&view, 0, &outcome));
        EXPECT_EQ(outcome.level, 47);
    }
    VarSet(VAR_REPEL_STEP_COUNT, steps);
}

// A small harness that runs StandardWildEncounter on a real map without starting battles.
struct EncounterHarness
{
    struct MapHeader mapHeader;
    struct WarpData location;
    struct PlayerAvatar playerAvatar;
    struct Pokemon playerParty[PARTY_SIZE];
    struct Pokemon enemyParty[PARTY_SIZE];
    u16 repelSteps;
    u8 playerPartyCount;
    u8 enemyPartyCount;
};

static void EnterRoute(struct EncounterHarness *harness, u16 map)
{
    harness->mapHeader = gMapHeader;
    harness->location = gSaveBlock1Ptr->location;
    harness->playerAvatar = gPlayerAvatar;
    memcpy(harness->playerParty, gPlayerParty, sizeof(harness->playerParty));
    memcpy(harness->enemyParty, gEnemyParty, sizeof(harness->enemyParty));
    harness->repelSteps = VarGet(VAR_REPEL_STEP_COUNT);
    harness->playerPartyCount = gPlayerPartyCount;
    harness->enemyPartyCount = gEnemyPartyCount;

    DisableWildEncounters(FALSE);
    gPlayerAvatar.flags = 0;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(map);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(map);
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(map), MAP_NUM(map));
    gSaveBlock1Ptr->outbreakPokemonSpecies = SPECIES_NONE;
    gSaveBlock1Ptr->outbreakPokemonProbability = 0;
    SetWildStartInterceptionForTesting(TRUE);
}

static void LeaveRoute(const struct EncounterHarness *harness)
{
    SetWildStartInterceptionForTesting(FALSE);
    gMapHeader = harness->mapHeader;
    gSaveBlock1Ptr->location = harness->location;
    gPlayerAvatar = harness->playerAvatar;
    memcpy(gPlayerParty, harness->playerParty, sizeof(harness->playerParty));
    memcpy(gEnemyParty, harness->enemyParty, sizeof(harness->enemyParty));
    VarSet(VAR_REPEL_STEP_COUNT, harness->repelSteps);
    gPlayerPartyCount = harness->playerPartyCount;
    gEnemyPartyCount = harness->enemyPartyCount;
}

TEST("Wild encounters: Repel compares the lead's level with the encounter's final level, not the place's level")
{
    struct EncounterHarness *harness = Alloc(sizeof(*harness));
    u32 attempts, starts = 0, below = 0, atOrAbove = 0;
    bool32 repelOn;
    u32 repelFiltered = 0;

    EXPECT(harness != NULL);
    // Route 1 is a Road. At Rating 80 its level is 38, but its Igglybuff slot is a baby (level 10 at most)
    // and its Eevee slot stays young, so some encounters have a final level far under 38.
    EnterRoute(harness, MAP_ROUTE1_HNS);
    SetTrainerRating(80);
    EXPECT_EQ(GetWildEncounterPlaceLevel(GetCurrentMapWildMonHeaderId(), 80), 38);
    SetLeadMon(SPECIES_PIKACHU, 38);
    SeedRng(7);
    for (repelOn = FALSE; repelOn <= TRUE; repelOn++)
    {
        VarSet(VAR_REPEL_STEP_COUNT, repelOn ? 200 : 0);
        starts = below = atOrAbove = 0;
        for (attempts = 0; attempts < 6000; attempts++)
        {
            if (StandardWildEncounter(MB_TALL_GRASS, MB_TALL_GRASS))
            {
                u32 level = GetMonData(&gEnemyParty[0], MON_DATA_LEVEL);

                starts++;
                if (level < 38)
                    below++;
                else
                    atOrAbove++;
            }
        }
        if (!repelOn)
        {
            // Without Repel, young and baby slots do come up below the place's level.
            EXPECT_GT(starts, 50);
            EXPECT_GT(below, 0);
            EXPECT_GT(atOrAbove, 0);
            repelFiltered = below;
        }
        else
        {
            // With Repel on, only final levels of 38 and over start a battle.
            EXPECT_GT(starts, 10);
            EXPECT_EQ(below, 0);
            EXPECT_GT(atOrAbove, 0);
        }
    }
    EXPECT_GT(repelFiltered, 0);
    LeaveRoute(harness);
    Free(harness);
}

// ---- Time of day, the Bug-Catching Contest and the ghost Marowak ----

TEST("Wild encounters: morning and day use a map's day table, evening and night its night table")
{
    u32 header, count = CountHeaders(), method;
    u32 tablesDiffer = 0;

    for (header = 0; header < count; header++)
    {
        const struct WildEncounterTypes *morning = &gWildMonHeaders[header].encounterTypes[TIME_MORNING];
        const struct WildEncounterTypes *day = &gWildMonHeaders[header].encounterTypes[TIME_DAY];
        const struct WildEncounterTypes *evening = &gWildMonHeaders[header].encounterTypes[TIME_EVENING];
        const struct WildEncounterTypes *night = &gWildMonHeaders[header].encounterTypes[TIME_NIGHT];
        const struct WildPokemonInfo *const *morningInfos[] = { &morning->landMonsInfo, &morning->waterMonsInfo, &morning->rockSmashMonsInfo, &morning->fishingMonsInfo };
        const struct WildPokemonInfo *const *dayInfos[] = { &day->landMonsInfo, &day->waterMonsInfo, &day->rockSmashMonsInfo, &day->fishingMonsInfo };
        const struct WildPokemonInfo *const *eveningInfos[] = { &evening->landMonsInfo, &evening->waterMonsInfo, &evening->rockSmashMonsInfo, &evening->fishingMonsInfo };
        const struct WildPokemonInfo *const *nightInfos[] = { &night->landMonsInfo, &night->waterMonsInfo, &night->rockSmashMonsInfo, &night->fishingMonsInfo };

        for (method = 0; method < 4; method++)
        {
            EXPECT_EQ(*morningInfos[method], *dayInfos[method]);
            EXPECT_EQ(*eveningInfos[method], *nightInfos[method]);
            // Both tables exist together; a table with no night counterpart would leave the night empty.
            EXPECT_EQ(*dayInfos[method] == NULL, *nightInfos[method] == NULL);
            if (*dayInfos[method] != NULL && *dayInfos[method] != *nightInfos[method])
                tablesDiffer++;
        }
    }
    // Day and night really are distinct tables somewhere, so the aliasing above is not vacuous.
    EXPECT_GT(tablesDiffer, 0);
}

TEST("Wild encounters: the clock selects the day table in the morning and by day, the night table in the evening and at night")
{
    static const struct { u8 hour; enum TimeOfDay time; bool8 night; } sClocks[] =
    {
        { 7, TIME_MORNING, FALSE }, { 12, TIME_DAY, FALSE }, { 18, TIME_EVENING, TRUE }, { 22, TIME_NIGHT, TRUE },
    };
    u32 header = GetWildMonHeaderIdForMap(MAP_GROUP(MAP_ROUTE1_HNS), MAP_NUM(MAP_ROUTE1_HNS));
    bool8 savedRtc = gSaveBlock3Ptr->challengeSettings.tx_Features_RTCType;
    u32 i;

    EXPECT_NE(header, HEADER_NONE);
    gSaveBlock3Ptr->challengeSettings.tx_Features_RTCType = TRUE;
    for (i = 0; i < ARRAY_COUNT(sClocks); i++)
    {
        struct WildEncounterProfileContext context = { .headerId = header, .area = WILD_AREA_LAND, .fishingRod = WILD_ENCOUNTER_FISHING_ROD_NONE };
        struct WildEncounterProfileView view;

        FakeRtc_ManuallySetTime(0, sClocks[i].hour, 0, 0);
        EXPECT_EQ(GetTimeOfDay(), sClocks[i].time);
        context.timeOfDay = GetTimeOfDayForEncounters(header, WILD_AREA_LAND);
        EXPECT(GetWildEncounterProfileView(&context, &view));
        EXPECT_EQ(view.wildMonsInfo, sClocks[i].night ? gWildMonHeaders[header].encounterTypes[TIME_NIGHT].landMonsInfo
                                                       : gWildMonHeaders[header].encounterTypes[TIME_DAY].landMonsInfo);
    }
    gSaveBlock3Ptr->challengeSettings.tx_Features_RTCType = savedRtc;
}

TEST("Bug-Catching Contest: Tuesday, Thursday and Saturday each choose their own table, and other days hold no contest")
{
    static const enum Weekday sDays[] = { WEEKDAY_SUN, WEEKDAY_MON, WEEKDAY_TUE, WEEKDAY_WED, WEEKDAY_THU, WEEKDAY_FRI, WEEKDAY_SAT };
    u32 first = GetWildMonHeaderIdForMap(MAP_GROUP(MAP_NATIONAL_PARK_BUG_CONTEST_HNS), MAP_NUM(MAP_NATIONAL_PARK_BUG_CONTEST_HNS));
    bool8 savedRtc = gSaveBlock3Ptr->challengeSettings.tx_Features_RTCType;
    struct WarpData savedLocation = gSaveBlock1Ptr->location;
    u32 i, day;
    u32 held = 0;

    EXPECT_NE(first, HEADER_NONE);
    // Three consecutive headers for the one map, one per contest day.
    EXPECT_EQ(gWildMonHeaders[first + 1].mapGroup, MAP_GROUP(MAP_NATIONAL_PARK_BUG_CONTEST_HNS));
    EXPECT_EQ(gWildMonHeaders[first + 2].mapNum, MAP_NUM(MAP_NATIONAL_PARK_BUG_CONTEST_HNS));
    EXPECT_NE(gWildMonHeaders[first + 3].mapNum, MAP_NUM(MAP_NATIONAL_PARK_BUG_CONTEST_HNS));
    EXPECT(gWildMonHeaders[first].encounterTypes[TIME_DAY].landMonsInfo != gWildMonHeaders[first + 1].encounterTypes[TIME_DAY].landMonsInfo);
    EXPECT(gWildMonHeaders[first + 1].encounterTypes[TIME_DAY].landMonsInfo != gWildMonHeaders[first + 2].encounterTypes[TIME_DAY].landMonsInfo);

    gSaveBlock3Ptr->challengeSettings.tx_Features_RTCType = TRUE;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_NATIONAL_PARK_BUG_CONTEST_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_NATIONAL_PARK_BUG_CONTEST_HNS);
    for (i = 0; i < ARRAY_COUNT(sDays); i++)
    {
        // Find the fake date of each weekday.
        for (day = 0; day < 14; day++)
        {
            FakeRtc_ManuallySetTime(day, 12, 0, 0);
            if (GetDayOfWeek() == sDays[i])
                break;
        }
        EXPECT_LT(day, 14);
        EXPECT_EQ(GetDayOfWeek(), sDays[i]);
        switch (sDays[i])
        {
        case WEEKDAY_TUE:
            EXPECT(IsBugContestHeldToday());
            EXPECT_EQ(GetBugContestTableIndex(), 0);
            EXPECT_EQ(GetCurrentMapWildMonHeaderId(), first);
            held++;
            break;
        case WEEKDAY_THU:
            EXPECT(IsBugContestHeldToday());
            EXPECT_EQ(GetBugContestTableIndex(), 1);
            EXPECT_EQ(GetCurrentMapWildMonHeaderId(), first + 1);
            held++;
            break;
        case WEEKDAY_SAT:
            EXPECT(IsBugContestHeldToday());
            EXPECT_EQ(GetBugContestTableIndex(), 2);
            EXPECT_EQ(GetCurrentMapWildMonHeaderId(), first + 2);
            held++;
            break;
        default:
            // The gate attendant says it isn't on.
            EXPECT(!IsBugContestHeldToday());
            break;
        }
        gSpecialVar_Result = 0xFFFF;
        BugContest_CheckHeldToday(NULL);
        EXPECT_EQ(gSpecialVar_Result, IsBugContestHeldToday());
    }
    EXPECT_EQ(held, 3);
    gSaveBlock1Ptr->location = savedLocation;
    gSaveBlock3Ptr->challengeSettings.tx_Features_RTCType = savedRtc;
}

TEST("Pokemon Tower: the ghost Marowak takes its floor's place level, with no spread")
{
    static const u16 sRatings[] = { 0, 20, 40, 80, 120, 160 };
    u32 header = GetWildMonHeaderIdForMap(MAP_GROUP(MAP_POKEMON_TOWER_6F), MAP_NUM(MAP_POKEMON_TOWER_6F));
    u32 i, seed;

    EXPECT_NE(header, HEADER_NONE);
    EXPECT(AddBagItem(ITEM_SILPH_SCOPE, 1));
    for (i = 0; i < ARRAY_COUNT(sRatings); i++)
    {
        SetTrainerRating(sRatings[i]);
        for (seed = 0; seed < 3; seed++)
        {
            SeedRng(seed);
            StartMarowakBattle();
            ResetTasks(); // the battle start task is not run here
            EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), SPECIES_MAROWAK);
            EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), RefPlaceLevel(header, sRatings[i]));
        }
    }
}

TEST("Wild encounters: Route 119's Feebas takes its place's level like any table")
{
    struct WildEncounterSlotOutcome outcomes[WILD_ENCOUNTER_MAX_SLOT_OUTCOMES];
    u32 header = GetWildMonHeaderIdForMap(MAP_GROUP(MAP_ROUTE119), MAP_NUM(MAP_ROUTE119));
    u32 count, i, rod, found;

    EXPECT_NE(header, HEADER_NONE);
    SetTrainerRating(80);
    count = SlotOutcomes(SPECIES_FEEBAS, header, 80, outcomes);
    EXPECT_GT(count, 0);
    for (rod = WILD_ENCOUNTER_FISHING_ROD_OLD; rod <= WILD_ENCOUNTER_FISHING_ROD_SUPER; rod++)
    {
        EXPECT_EQ(GenerateFeebasFishingWildMonForTesting(rod), SPECIES_FEEBAS);
        found = 0;
        for (i = 0; i < count; i++)
            found |= outcomes[i].species == SPECIES_FEEBAS && outcomes[i].level == GetMonData(&gEnemyParty[0], MON_DATA_LEVEL);
        EXPECT(found);
    }
}

#endif // IS_WAYFARER
