#include "global.h"
#include "battle_setup.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "fieldmap.h"
#include "fishing.h"
#include "follower_npc.h"
#include "random.h"
#include "field_player_avatar.h"
#include "link.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "ow_synchronize.h"
#include "pokeblock.h"
#include "pokemon.h"
#include "random.h"
#include "roamer.h"
#include "safari_zone.h"
#include "script.h"
#include "tv.h"
#include "wild_encounter.h"
#include "randomizer.h"
#include "battle_debug.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "constants/abilities.h"
#include "constants/game_stat.h"
#include "constants/item.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/songs.h"
#include "constants/weather.h"
#include "pokenav.h"
#include "sound.h"
#include "trainer_rating.h"

extern const u8 EventScript_SprayWoreOff[];

#define MAX_ENCOUNTER_RATE 2880

#define NUM_FEEBAS_SPOTS 6

// Number of accessible fishing spots in each section of Route 119
// Each section is an area of the route between the y coordinates in sRoute119WaterTileData
#define NUM_FISHING_SPOTS_1 131
#define NUM_FISHING_SPOTS_2 167
#define NUM_FISHING_SPOTS_3 149
#define NUM_FISHING_SPOTS (NUM_FISHING_SPOTS_1 + NUM_FISHING_SPOTS_2 + NUM_FISHING_SPOTS_3)

#define WILD_CHECK_REPEL    (1 << 0)
#define WILD_CHECK_KEEN_EYE (1 << 1)

static u16 FeebasRandom(void);
static void FeebasSeedRng(u16 seed);
static bool8 IsWildLevelAllowedByRepel(u8 level);
static void ApplyFluteEncounterRateMod(u32 *encRate);
static void ApplyCleanseTagEncounterRateMod(u32 *encRate);
static u8 GetMaxLevelOfSpeciesInWildTable(const struct WildPokemon *wildMon, u16 species, enum WildPokemonArea area);
#ifdef BUGFIX
static bool8 TryGetAbilityInfluencedWildMonIndex(const struct WildPokemon *wildMon, enum Type type, enum Ability ability, u8 *monIndex, u32 size);
#else
static bool8 TryGetAbilityInfluencedWildMonIndex(const struct WildPokemon *wildMon, enum Type type, enum Ability ability, u8 *monIndex);
#endif
static bool8 IsAbilityAllowingEncounter(u8 level);
static bool8 TryGenerateWildMonFromProfile(u32 headerId, enum TimeOfDay timeOfDay, enum WildPokemonArea area, enum WildEncounterFishingRod fishingRod, u8 flags);
static u16 GenerateFishingWildMon(u32 headerId, enum TimeOfDay timeOfDay, u8 rod, bool8 useFeebasOverride);
static u16 GenerateFishingWildMonFromProfile(u32 headerId, enum TimeOfDay timeOfDay, u8 rod);
static u16 GetLocalWildEncounterProfileSpecies(const struct WildEncounterProfileView *view);
#if IS_HNS
static bool8 TryGetHoennSoundWildMonIndex(const struct WildPokemon *wildMon, u8 numMon, u8 *monIndex);
#endif

EWRAM_DATA static u8 sWildEncountersDisabled = 0;
EWRAM_DATA static u32 sFeebasRngValue = 0;
EWRAM_DATA bool8 gIsFishingEncounter = 0;
EWRAM_DATA bool8 gIsSurfingEncounter = 0;
EWRAM_DATA u8 gChainFishingDexNavStreak = 0;

#include "data/wild_encounters.h"

static const u8 sWildEncounterLandWeights[LAND_WILD_COUNT] =
{
    20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1,
};

static const u8 sWildEncounterWaterWeights[WATER_WILD_COUNT] =
{
    60, 30, 5, 4, 1,
};

static const u8 sWildEncounterRockWeights[ROCK_WILD_COUNT] =
{
    60, 30, 5, 4, 1,
};

static u16 GetWildMonHeaderCount(void)
{
    u16 count;

    for (count = 0; gWildMonHeaders[count].mapGroup != MAP_GROUP(MAP_UNDEFINED); count++)
    {
    }
    return count;
}

static bool8 IsWildEncounterProfileSlotInRange(const struct WildEncounterProfileView *view, u8 slot)
{
    if (view == NULL || view->wildMonsInfo == NULL || view->weights == NULL)
        return FALSE;

    // LAND_WILD_COUNT is the largest ordinary authored table. Keeping the
    // public view bounded makes malformed synthetic views fail safely too.
    if (view->entryCount == 0
     || view->entryStart >= LAND_WILD_COUNT
     || view->entryCount > LAND_WILD_COUNT - view->entryStart)
        return FALSE;

    return slot >= view->entryStart && slot < view->entryStart + view->entryCount;
}

static s32 RoundWildEncounterDivision(s32 numerator, u16 denominator)
{
    if (denominator == 0)
        return 0;
    if (numerator >= 0)
        return (numerator + denominator / 2) / denominator;
    return -((-numerator + denominator / 2) / denominator);
}

static s8 GetWildEncounterProfileOffset(const struct WildEncounterProfileView *view)
{
    u16 i;

    if (view == NULL)
        return 0;

    for (i = 0; i < gWildEncounterProfileOffsetCount; i++)
    {
        const struct WildEncounterProfileOffset *offset = &gWildEncounterProfileOffsets[i];

        if (offset->headerId == view->headerId
         && offset->timeOfDay == view->timeOfDay
         && offset->area == view->area
         && offset->fishingRod == view->fishingRod)
            return offset->levelOffset;
    }
    return 0;
}

static u8 ClampWildEncounterLevel(s32 level)
{
    if (level < 1)
        return 1;
    if (level > MAX_LEVEL)
        return MAX_LEVEL;
    return level;
}

static const struct WildEncounterSpeciesMetadata *GetWildEncounterSpeciesMetadata(u16 species)
{
    u16 i;

    for (i = 0; i < gWildEncounterSpeciesMetadataCount; i++)
    {
        if (gWildEncounterSpeciesMetadata[i].species == species)
            return &gWildEncounterSpeciesMetadata[i];
    }
    return NULL;
}

static bool8 IsCurrentWildEncounterRandomized(void)
{
#if RANDOMIZER_AVAILABLE
    return RandomizerFeatureEnabled(RANDOMIZE_WILD_MON);
#else
    return FALSE;
#endif
}

#if RANDOMIZER_AVAILABLE == TRUE
static u16 RandomizeAuthoredWildEncounter(const struct WildPokemon *entry, u8 mapNum, u8 mapGroup, enum WildPokemonArea area, u8 slot)
{
    return RandomizeWildEncounter(entry->species, mapNum, mapGroup, area, slot);
}
#endif

static u16 ResolveWildEncounterSpecies(u16 species, u8 projectedLevel, bool8 isWildRandomized)
{
    // Randomized species are selected after this core resolves a slot. Applying
    // authored evolution metadata here would change that mapping's order.
    if (isWildRandomized)
        return species;

    const struct WildEncounterSpeciesMetadata *metadata;

    while ((metadata = GetWildEncounterSpeciesMetadata(species)) != NULL)
    {
        if (metadata->predecessorSpecies == SPECIES_NONE
         || projectedLevel >= metadata->predecessorLevel)
            break;
        species = metadata->predecessorSpecies;
    }
    return species;
}

bool8 GetWildEncounterProfileView(const struct WildEncounterProfileContext *context, struct WildEncounterProfileView *view)
{
    const struct WildPokemonInfo *wildMonsInfo;

    if (context == NULL || view == NULL
     || context->headerId == HEADER_NONE
     || context->headerId >= GetWildMonHeaderCount()
     || (s32)context->timeOfDay < 0
     || context->timeOfDay >= TIMES_OF_DAY_COUNT)
        return FALSE;

    switch (context->area)
    {
    case WILD_AREA_LAND:
        if (context->fishingRod != WILD_ENCOUNTER_FISHING_ROD_NONE)
            return FALSE;
        wildMonsInfo = gWildMonHeaders[context->headerId].encounterTypes[context->timeOfDay].landMonsInfo;
        view->weights = sWildEncounterLandWeights;
        view->entryStart = 0;
        view->entryCount = LAND_WILD_COUNT;
        break;
    case WILD_AREA_WATER:
        if (context->fishingRod != WILD_ENCOUNTER_FISHING_ROD_NONE)
            return FALSE;
        wildMonsInfo = gWildMonHeaders[context->headerId].encounterTypes[context->timeOfDay].waterMonsInfo;
        view->weights = sWildEncounterWaterWeights;
        view->entryStart = 0;
        view->entryCount = WATER_WILD_COUNT;
        break;
    case WILD_AREA_ROCKS:
        if (context->fishingRod != WILD_ENCOUNTER_FISHING_ROD_NONE)
            return FALSE;
        wildMonsInfo = gWildMonHeaders[context->headerId].encounterTypes[context->timeOfDay].rockSmashMonsInfo;
        view->weights = sWildEncounterRockWeights;
        view->entryStart = 0;
        view->entryCount = ROCK_WILD_COUNT;
        break;
    case WILD_AREA_FISHING:
        wildMonsInfo = gWildMonHeaders[context->headerId].encounterTypes[context->timeOfDay].fishingMonsInfo;
        switch (context->fishingRod)
        {
        case WILD_ENCOUNTER_FISHING_ROD_OLD:
        case WILD_ENCOUNTER_FISHING_ROD_GOOD:
        case WILD_ENCOUNTER_FISHING_ROD_SUPER:
            view->weights = gStandardRodFishingWeights[context->fishingRod];
            view->entryStart = 0;
            view->entryCount = FISH_WILD_COUNT;
            break;
        default:
            return FALSE;
        }
        break;
    default:
        // Hidden, scripted, Frontier, and other special sources retain their
        // authored behavior until they receive an explicit policy.
        return FALSE;
    }

    if (wildMonsInfo == NULL)
        return FALSE;

    view->wildMonsInfo = wildMonsInfo;
    view->headerId = context->headerId;
    view->timeOfDay = context->timeOfDay;
    view->area = context->area;
    view->fishingRod = context->fishingRod;
    return TRUE;
}

bool8 GetWildEncounterProfileEntry(const struct WildEncounterProfileView *view, u8 slot, const struct WildPokemon **entry)
{
    if (entry == NULL || !IsWildEncounterProfileSlotInRange(view, slot))
        return FALSE;

    *entry = &view->wildMonsInfo->wildPokemon[slot];
    return TRUE;
}

u8 ProjectWildEncounterLevelWithOffset(u8 authoredLevel, u16 trainerRating, s8 levelOffset)
{
    s32 highWater = -0x7FFFFFFF;
    u16 rating;
    u16 maximumRating;
    u8 baseLevel;

    if (gWildEncounterScalingPointCount == 0)
        return ClampWildEncounterLevel(authoredLevel + levelOffset);

    maximumRating = gWildEncounterScalingConfig.projectionCap;
    if (maximumRating >= gWildEncounterScalingPointCount)
        maximumRating = gWildEncounterScalingPointCount - 1;
    if (trainerRating > maximumRating)
        trainerRating = maximumRating;

    baseLevel = gWildEncounterScalingPoints[0].anchorLevel;
    for (rating = 0; rating <= trainerRating; rating++)
    {
        const struct WildEncounterScalingPoint *point = &gWildEncounterScalingPoints[rating];
        s32 rawLevel = point->anchorLevel + RoundWildEncounterDivision((authoredLevel - baseLevel) * point->retentionNumerator, point->retentionDenominator);

        if (rawLevel > highWater)
            highWater = rawLevel;
    }

    return ClampWildEncounterLevel(highWater + levelOffset);
}

u8 ProjectWildEncounterLevel(const struct WildEncounterProfileView *view, u8 authoredLevel, u16 trainerRating)
{
    return ProjectWildEncounterLevelWithOffset(authoredLevel, trainerRating, GetWildEncounterProfileOffset(view));
}

bool8 GetWildEncounterSpeciesOutcome(const struct WildEncounterProfileView *view, u8 slot, u8 authoredLevel, u16 trainerRating, bool8 isWildRandomized, struct WildEncounterSpeciesOutcome *outcome)
{
    const struct WildPokemon *entry;

    if (outcome == NULL || authoredLevel == 0 || !GetWildEncounterProfileEntry(view, slot, &entry))
        return FALSE;

    outcome->level = ProjectWildEncounterLevel(view, authoredLevel, trainerRating);
    outcome->species = ResolveWildEncounterSpecies(entry->species, outcome->level, isWildRandomized);
    return TRUE;
}

bool8 GetCurrentWildEncounterSpeciesOutcome(const struct WildEncounterProfileView *view, u8 slot, u8 authoredLevel, struct WildEncounterSpeciesOutcome *outcome)
{
    return GetWildEncounterSpeciesOutcome(view, slot, authoredLevel, GetTrainerRating(), IsCurrentWildEncounterRandomized(), outcome);
}

bool8 IsWildEncounterProfileSlotEligible(const struct WildEncounterProfileView *view, u8 slot, u16 trainerRating, bool8 isWildRandomized)
{
    const struct WildPokemon *entry;

    if (!GetWildEncounterProfileEntry(view, slot, &entry))
        return FALSE;

    if (entry->species == SPECIES_NONE)
        return FALSE;

    if (isWildRandomized)
        return TRUE;

    u16 level;
    u8 minimumLevel = min(entry->minLevel, entry->maxLevel);
    u8 maximumLevel = max(entry->minLevel, entry->maxLevel);

    for (level = minimumLevel; level <= maximumLevel; level++)
    {
        const struct WildEncounterSpeciesMetadata *metadata;
        struct WildEncounterSpeciesOutcome outcome;

        if (!GetWildEncounterSpeciesOutcome(view, slot, (u8)level, trainerRating, FALSE, &outcome))
            return FALSE;
        metadata = GetWildEncounterSpeciesMetadata(outcome.species);
        if (metadata != NULL && outcome.level < metadata->minimumLevel)
            return FALSE;
    }
    return TRUE;
}

u16 GetWildEncounterProfileEligibleWeight(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized)
{
    u8 slot;
    u16 total = 0;

    if (view == NULL || !IsWildEncounterProfileSlotInRange(view, view->entryStart))
        return 0;

    for (slot = view->entryStart; slot < view->entryStart + view->entryCount; slot++)
        total += GetWildEncounterProfileEffectiveWeight(view, slot, trainerRating, isWildRandomized);
    return total;
}

u16 GetWildEncounterProfileEffectiveWeight(const struct WildEncounterProfileView *view, u8 slot, u16 trainerRating, bool8 isWildRandomized)
{
    if (!IsWildEncounterProfileSlotInRange(view, slot) || !IsWildEncounterProfileSlotEligible(view, slot, trainerRating, isWildRandomized))
        return 0;
    return view->weights[slot];
}

bool8 SelectWildEncounterProfileSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u16 roll, u8 *slot)
{
    u8 candidate;
    u16 total = GetWildEncounterProfileEligibleWeight(view, trainerRating, isWildRandomized);

    if (slot == NULL || total == 0 || roll >= total)
        return FALSE;

    for (candidate = view->entryStart; candidate < view->entryStart + view->entryCount; candidate++)
    {
        u16 weight = GetWildEncounterProfileEffectiveWeight(view, candidate, trainerRating, isWildRandomized);

        if (roll < weight)
        {
            *slot = candidate;
            return TRUE;
        }
        roll -= weight;
    }

    return FALSE;
}

bool8 IsCurrentWildEncounterProfileSlotEligible(const struct WildEncounterProfileView *view, u8 slot)
{
    return IsWildEncounterProfileSlotEligible(view, slot, GetTrainerRating(), IsCurrentWildEncounterRandomized());
}

u16 GetCurrentWildEncounterProfileEligibleWeight(const struct WildEncounterProfileView *view)
{
    return GetWildEncounterProfileEligibleWeight(view, GetTrainerRating(), IsCurrentWildEncounterRandomized());
}

u16 GetCurrentWildEncounterProfileEffectiveWeight(const struct WildEncounterProfileView *view, u8 slot)
{
    return GetWildEncounterProfileEffectiveWeight(view, slot, GetTrainerRating(), IsCurrentWildEncounterRandomized());
}

bool8 SelectCurrentWildEncounterProfileSlot(const struct WildEncounterProfileView *view, u16 roll, u8 *slot)
{
    return SelectWildEncounterProfileSlot(view, GetTrainerRating(), IsCurrentWildEncounterRandomized(), roll, slot);
}

static const struct WildPokemon sWildFeebas = {20, 25, SPECIES_FEEBAS};

static const u16 sRoute119WaterTileData[] =
{
//yMin, yMax, numSpots in previous sections
     0,  45,  0,
    46,  91,  NUM_FISHING_SPOTS_1,
    92, 139,  NUM_FISHING_SPOTS_1 + NUM_FISHING_SPOTS_2,
};

void DisableWildEncounters(bool8 disabled)
{
    sWildEncountersDisabled = disabled;
}

// Each fishing spot on Route 119 is given a number between 1 and NUM_FISHING_SPOTS inclusive.
// The number is determined by counting the valid fishing spots left to right top to bottom.
// The map is divided into three sections, with each section having a pre-counted number of
// fishing spots to start from to avoid counting a large number of spots at the bottom of the map.
// Note that a spot is considered valid if it is surfable and not a waterfall. To exclude all
// of the inaccessible water metatiles (so that they can't be selected as a Feebas spot) they
// use a different metatile that isn't actually surfable because it has MB_NORMAL instead.
// This function is given the coordinates and section of a fishing spot and returns which number it is.
static u16 GetFeebasFishingSpotId(s16 targetX, s16 targetY, u8 section)
{
    u16 x, y;
    u16 yMin = sRoute119WaterTileData[section * 3 + 0];
    u16 yMax = sRoute119WaterTileData[section * 3 + 1];
    u16 spotId = sRoute119WaterTileData[section * 3 + 2];

    for (y = yMin; y <= yMax; y++)
    {
        for (x = 0; x < gMapHeader.mapLayout->width; x++)
        {
            u8 behavior = MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET);
            if (MetatileBehavior_IsSurfableAndNotWaterfall(behavior) == TRUE)
            {
                spotId++;
                if (targetX == x && targetY == y)
                    return spotId;
            }
        }
    }
    return spotId + 1;
}

static bool8 CheckFeebas(void)
{
    u8 i;
    u16 feebasSpots[NUM_FEEBAS_SPOTS];
    s16 x, y;
    u8 route119Section = 0;
    u16 spotId;

    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ROUTE119)
     && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ROUTE119))
    {
        GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
        x -= MAP_OFFSET;
        y -= MAP_OFFSET;

        // Get which third of the map the player is in
        if (y >= sRoute119WaterTileData[3 * 0 + 0] && y <= sRoute119WaterTileData[3 * 0 + 1])
            route119Section = 0;
        if (y >= sRoute119WaterTileData[3 * 1 + 0] && y <= sRoute119WaterTileData[3 * 1 + 1])
            route119Section = 1;
        if (y >= sRoute119WaterTileData[3 * 2 + 0] && y <= sRoute119WaterTileData[3 * 2 + 1])
            route119Section = 2;

        // 50% chance of encountering Feebas (assuming this is a Feebas spot)
        if (Random() % 100 > 49)
            return FALSE;

        FeebasSeedRng(gSaveBlock1Ptr->dewfordTrends[0].rand);

        // Assign each Feebas spot to a random fishing spot.
        // Randomness is fixed depending on the seed above.
        for (i = 0; i != NUM_FEEBAS_SPOTS;)
        {
            feebasSpots[i] = FeebasRandom() % NUM_FISHING_SPOTS;
            if (feebasSpots[i] == 0)
                feebasSpots[i] = NUM_FISHING_SPOTS;

            // < 1 below is a pointless check, it will never be TRUE.
            // >= 4 to skip fishing spots 1-3, because these are inaccessible
            // spots at the top of the map, at (9,7), (7,13), and (15,16).
            // The first accessible fishing spot is spot 4 at (18,18).
            if (feebasSpots[i] < 1 || feebasSpots[i] >= 4)
                i++;
        }

        // Check which fishing spot the player is at, and see if
        // it matches any of the Feebas spots.
        spotId = GetFeebasFishingSpotId(x, y, route119Section);
        for (i = 0; i < NUM_FEEBAS_SPOTS; i++)
        {
            if (spotId == feebasSpots[i])
                return TRUE;
        }
    }
    return FALSE;
}

static u16 FeebasRandom(void)
{
    sFeebasRngValue = ISO_RANDOMIZE2(sFeebasRngValue);
    return sFeebasRngValue >> 16;
}

static void FeebasSeedRng(u16 seed)
{
    sFeebasRngValue = seed;
}

// LAND_WILD_COUNT
u32 ChooseWildMonIndex_Land(void)
{
    u8 wildMonIndex = 0;
    bool8 swap = FALSE;
    u8 rand = Random() % ENCOUNTER_CHANCE_LAND_MONS_TOTAL;

    if (rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_0)
        wildMonIndex = 0;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_0 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_1)
        wildMonIndex = 1;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_1 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_2)
        wildMonIndex = 2;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_2 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_3)
        wildMonIndex = 3;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_3 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_4)
        wildMonIndex = 4;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_4 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_5)
        wildMonIndex = 5;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_5 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_6)
        wildMonIndex = 6;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_6 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_7)
        wildMonIndex = 7;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_7 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_8)
        wildMonIndex = 8;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_8 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_9)
        wildMonIndex = 9;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_9 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_10)
        wildMonIndex = 10;
    else
        wildMonIndex = 11;

    if (LURE_STEP_COUNT != 0 && (Random() % 10 < 2))
        swap = TRUE;

    if (swap)
        wildMonIndex = 11 - wildMonIndex;

    return wildMonIndex;
}

// WATER_WILD_COUNT
u32 ChooseWildMonIndex_Water(void)
{
    u32 wildMonIndex = 0;
    bool8 swap = FALSE;
    u8 rand = Random() % ENCOUNTER_CHANCE_WATER_MONS_TOTAL;

    if (rand < ENCOUNTER_CHANCE_WATER_MONS_SLOT_0)
        wildMonIndex = 0;
    else if (rand >= ENCOUNTER_CHANCE_WATER_MONS_SLOT_0 && rand < ENCOUNTER_CHANCE_WATER_MONS_SLOT_1)
        wildMonIndex = 1;
    else if (rand >= ENCOUNTER_CHANCE_WATER_MONS_SLOT_1 && rand < ENCOUNTER_CHANCE_WATER_MONS_SLOT_2)
        wildMonIndex = 2;
    else if (rand >= ENCOUNTER_CHANCE_WATER_MONS_SLOT_2 && rand < ENCOUNTER_CHANCE_WATER_MONS_SLOT_3)
        wildMonIndex = 3;
    else
        wildMonIndex = 4;

    if (LURE_STEP_COUNT != 0 && (Random() % 10 < 2))
        swap = TRUE;

    if (swap)
        wildMonIndex = 4 - wildMonIndex;

    return wildMonIndex;
}

// ROCK_WILD_COUNT
u32 ChooseWildMonIndex_Rocks(void)
{
    u32 wildMonIndex = 0;
    bool8 swap = FALSE;
    u8 rand = Random() % ENCOUNTER_CHANCE_ROCK_SMASH_MONS_TOTAL;

    if (rand < ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_0)
        wildMonIndex = 0;
    else if (rand >= ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_0 && rand < ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_1)
        wildMonIndex = 1;
    else if (rand >= ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_1 && rand < ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_2)
        wildMonIndex = 2;
    else if (rand >= ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_2 && rand < ENCOUNTER_CHANCE_ROCK_SMASH_MONS_SLOT_3)
        wildMonIndex = 3;
    else
        wildMonIndex = 4;

    if (LURE_STEP_COUNT != 0 && (Random() % 10 < 2))
        swap = TRUE;

    if (swap)
        wildMonIndex = 4 - wildMonIndex;

    return wildMonIndex;
}

static u8 ChooseWildMonLevel(const struct WildPokemon *wildPokemon, u8 wildMonIndex, enum WildPokemonArea area)
{
    u8 min;
    u8 max;
    u8 range;
    u8 rand;

    if (LURE_STEP_COUNT == 0)
    {
        // Make sure minimum level is less than maximum level
        if (wildPokemon[wildMonIndex].maxLevel >= wildPokemon[wildMonIndex].minLevel)
        {
            min = wildPokemon[wildMonIndex].minLevel;
            max = wildPokemon[wildMonIndex].maxLevel;
        }
        else
        {
            min = wildPokemon[wildMonIndex].maxLevel;
            max = wildPokemon[wildMonIndex].minLevel;
        }
        range = max - min + 1;
        rand = Random() % range;

        // check ability for max level mon
        if (!GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
        {
            enum Ability ability = GetMonAbility(&gPlayerParty[0]);
            if (ability == ABILITY_HUSTLE || ability == ABILITY_VITAL_SPIRIT || ability == ABILITY_PRESSURE)
            {
                if (Random() % 2 == 0)
                    return max;

                if (rand != 0)
                    rand--;
            }
        }
        return min + rand;
    }
    else
    {
        // Looks for the max level of all slots that share the same species as the selected slot.
        max = GetMaxLevelOfSpeciesInWildTable(wildPokemon, wildPokemon[wildMonIndex].species, area);
        if (max > 0)
            return max + 1;
        else // Failsafe
            return wildPokemon[wildMonIndex].maxLevel + 1;
    }
}

u16 GetCurrentMapWildMonHeaderId(void)
{
    u16 i;

    for (i = 0; ; i++)
    {
        const struct WildPokemonHeader *wildHeader = &gWildMonHeaders[i];
        if (wildHeader->mapGroup == MAP_GROUP(MAP_UNDEFINED))
            break;

        if (gWildMonHeaders[i].mapGroup == gSaveBlock1Ptr->location.mapGroup &&
            gWildMonHeaders[i].mapNum == gSaveBlock1Ptr->location.mapNum)
        {
            if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_ALTERING_CAVE) &&
                gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_ALTERING_CAVE))
            {
                u16 alteringCaveId = VarGet(VAR_ALTERING_CAVE_WILD_SET);
                if (alteringCaveId >= NUM_ALTERING_CAVE_TABLES)
                    alteringCaveId = 0;

                i += alteringCaveId;
            }

            return i;
        }
    }

    return HEADER_NONE;
}

enum TimeOfDay GetTimeOfDayForEncounters(u32 headerId, enum WildPokemonArea area)
{
    const struct WildPokemonInfo *wildMonInfo;
    enum TimeOfDay timeOfDay = GetTimeOfDay();

    if (!OW_TIME_OF_DAY_ENCOUNTERS)
        return TIME_OF_DAY_DEFAULT;

    if (InBattlePike() || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
        return OW_TIME_OF_DAY_FALLBACK;

    switch (area)
    {
    default:
    case WILD_AREA_LAND:
        wildMonInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo;
        break;
    case WILD_AREA_WATER:
        wildMonInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo;
        break;
    case WILD_AREA_ROCKS:
        wildMonInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].rockSmashMonsInfo;
        break;
    case WILD_AREA_FISHING:
        wildMonInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].fishingMonsInfo;
        break;
    case WILD_AREA_HIDDEN:
        wildMonInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].hiddenMonsInfo;
        break;
    }

    if (wildMonInfo == NULL && !OW_TIME_OF_DAY_DISABLE_FALLBACK)
        return OW_TIME_OF_DAY_FALLBACK;
    else
        return GenConfigTimeOfDay(timeOfDay);
}

static u8 PickWildMonNature(u32 species)
{
    u8 i;
    struct Pokeblock *safariPokeblock;
    u8 natures[NUM_NATURES];

    if (GetSafariZoneFlag() == TRUE && Random() % 100 < 80)
    {
        safariPokeblock = SafariZoneGetActivePokeblock();
        if (safariPokeblock != NULL)
        {
            for (i = 0; i < NUM_NATURES; i++)
                natures[i] = i;
            Shuffle(natures, NUM_NATURES, sizeof(natures[0]));
            for (i = 0; i < NUM_NATURES; i++)
            {
                if (PokeblockGetGain(natures[i], safariPokeblock) > 0)
                    return natures[i];
            }
        }
    }

    return GetSynchronizedNature(WILDMON_ORIGIN, species);
}

void CreateWildMon(u16 species, u8 level)
{
    ZeroEnemyPartyMons();
    u32 personality = GetMonPersonality(species, GetSynchronizedGender(WILDMON_ORIGIN, species), PickWildMonNature(species), RANDOM_UNOWN_LETTER);
    CreateMonWithIVs(&gEnemyParty[0], species, level, personality, OTID_STRUCT_PLAYER_ID, USE_RANDOM_IVS);
    GiveMonInitialMoveset(&gEnemyParty[0]);
}

#ifdef BUGFIX
#define TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildPokemon, type, ability, ptr, count) TryGetAbilityInfluencedWildMonIndex(wildPokemon, type, ability, ptr, count)
#else
#define TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildPokemon, type, ability, ptr, count) TryGetAbilityInfluencedWildMonIndex(wildPokemon, type, ability, ptr)
#endif

static bool8 TryGenerateWildMon(const struct WildPokemonInfo *wildMonInfo, enum WildPokemonArea area, u8 flags)
{
    u8 wildMonIndex = 0;
    u8 level;

    switch (area)
    {
    case WILD_AREA_LAND:
        if (TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_STEEL, ABILITY_MAGNET_PULL, &wildMonIndex, LAND_WILD_COUNT))
            break;
        if (TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_ELECTRIC, ABILITY_STATIC, &wildMonIndex, LAND_WILD_COUNT))
            break;
        if (OW_LIGHTNING_ROD >= GEN_8 && TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_ELECTRIC, ABILITY_LIGHTNING_ROD, &wildMonIndex, LAND_WILD_COUNT))
            break;
        if (OW_FLASH_FIRE >= GEN_8 && TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_FIRE, ABILITY_FLASH_FIRE, &wildMonIndex, LAND_WILD_COUNT))
            break;
        if (OW_HARVEST >= GEN_8 && TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_GRASS, ABILITY_HARVEST, &wildMonIndex, LAND_WILD_COUNT))
            break;
        if (OW_STORM_DRAIN >= GEN_8 && TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_WATER, ABILITY_STORM_DRAIN, &wildMonIndex, LAND_WILD_COUNT))
            break;
    #if IS_HNS
        if (TryGetHoennSoundWildMonIndex(wildMonInfo->wildPokemon, LAND_WILD_COUNT, &wildMonIndex))
            break;
    #endif

        wildMonIndex = ChooseWildMonIndex_Land();
        break;
    case WILD_AREA_WATER:
        if (TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_STEEL, ABILITY_MAGNET_PULL, &wildMonIndex, WATER_WILD_COUNT))
            break;
        if (TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_ELECTRIC, ABILITY_STATIC, &wildMonIndex, WATER_WILD_COUNT))
            break;
        if (OW_LIGHTNING_ROD >= GEN_8 && TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_ELECTRIC, ABILITY_LIGHTNING_ROD, &wildMonIndex, WATER_WILD_COUNT))
            break;
        if (OW_FLASH_FIRE >= GEN_8 && TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_FIRE, ABILITY_FLASH_FIRE, &wildMonIndex, WATER_WILD_COUNT))
            break;
        if (OW_HARVEST >= GEN_8 && TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_GRASS, ABILITY_HARVEST, &wildMonIndex, WATER_WILD_COUNT))
            break;
        if (OW_STORM_DRAIN >= GEN_8 && TRY_GET_ABILITY_INFLUENCED_WILD_MON_INDEX(wildMonInfo->wildPokemon, TYPE_WATER, ABILITY_STORM_DRAIN, &wildMonIndex, WATER_WILD_COUNT))
            break;
    #if IS_HNS
        if (TryGetHoennSoundWildMonIndex(wildMonInfo->wildPokemon, WATER_WILD_COUNT, &wildMonIndex))
            break;
    #endif

        wildMonIndex = ChooseWildMonIndex_Water();
        break;
    case WILD_AREA_ROCKS:
        wildMonIndex = ChooseWildMonIndex_Rocks();
        break;
    default:
    case WILD_AREA_FISHING:
    case WILD_AREA_HIDDEN:
        break;
    }

    level = ChooseWildMonLevel(wildMonInfo->wildPokemon, wildMonIndex, area);
    if (flags & WILD_CHECK_REPEL && !IsWildLevelAllowedByRepel(level))
        return FALSE;
    if ((gMapHeader.mapLayoutId != LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS && gMapHeader.mapLayoutId != LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS_HNS) && flags & WILD_CHECK_KEEN_EYE && !IsAbilityAllowingEncounter(level))
        return FALSE;

    {
        u16 species = wildMonInfo->wildPokemon[wildMonIndex].species;
        #if RANDOMIZER_AVAILABLE == TRUE
        species = RandomizeWildEncounter(species, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, area, wildMonIndex);
        #endif
        CreateWildMon(species, level);
    }
    return TRUE;
}

static bool8 SetUpMassOutbreakEncounter(u8 flags)
{
    u16 i;

    if (flags & WILD_CHECK_REPEL && !IsWildLevelAllowedByRepel(gSaveBlock1Ptr->outbreakPokemonLevel))
        return FALSE;

    CreateWildMon(gSaveBlock1Ptr->outbreakPokemonSpecies, gSaveBlock1Ptr->outbreakPokemonLevel);
    for (i = 0; i < MAX_MON_MOVES; i++)
        SetMonMoveSlot(&gEnemyParty[0], gSaveBlock1Ptr->outbreakPokemonMoves[i], i);

    return TRUE;
}

static bool8 DoMassOutbreakEncounterTest(void)
{
    if (gSaveBlock1Ptr->outbreakPokemonSpecies != SPECIES_NONE
     && gSaveBlock1Ptr->location.mapNum == gSaveBlock1Ptr->outbreakLocationMapNum
     && gSaveBlock1Ptr->location.mapGroup == gSaveBlock1Ptr->outbreakLocationMapGroup)
    {
        if (Random() % 100 < gSaveBlock1Ptr->outbreakPokemonProbability)
            return TRUE;
    }
    return FALSE;
}

static bool8 EncounterOddsCheck(u16 encounterRate)
{
    if (Random() % MAX_ENCOUNTER_RATE < encounterRate)
        return TRUE;
    else
        return FALSE;
}

// Returns true if it will try to create a wild encounter.
static bool8 WildEncounterCheck(u32 encounterRate, bool8 ignoreAbility)
{
    encounterRate *= 16;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE))
        encounterRate = encounterRate * 80 / 100;
    ApplyFluteEncounterRateMod(&encounterRate);
    ApplyCleanseTagEncounterRateMod(&encounterRate);
    if (LURE_STEP_COUNT != 0)
        encounterRate *= 2;
    if (!ignoreAbility && !GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
    {
        enum Ability ability = GetMonAbility(&gPlayerParty[0]);

        if (ability == ABILITY_STENCH && (gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR || gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR_HNS))
            encounterRate = encounterRate * 3 / 4;
        else if (ability == ABILITY_STENCH)
            encounterRate /= 2;
        else if (ability == ABILITY_ILLUMINATE)
            encounterRate *= 2;
        else if (ability == ABILITY_WHITE_SMOKE)
            encounterRate /= 2;
        else if (ability == ABILITY_ARENA_TRAP)
            encounterRate *= 2;
        else if (ability == ABILITY_SAND_VEIL && gSaveBlock1Ptr->weather == WEATHER_SANDSTORM)
            encounterRate /= 2;
        else if (ability == ABILITY_SNOW_CLOAK && gSaveBlock1Ptr->weather == WEATHER_SNOW)
            encounterRate /= 2;
        else if (ability == ABILITY_QUICK_FEET)
            encounterRate /= 2;
        else if (ability == ABILITY_INFILTRATOR && OW_INFILTRATOR >= GEN_8)
            encounterRate /= 2;
        else if (ability == ABILITY_NO_GUARD)
            encounterRate *= 2;
    }
    if (encounterRate > MAX_ENCOUNTER_RATE)
        encounterRate = MAX_ENCOUNTER_RATE;
    return EncounterOddsCheck(encounterRate);
}

// When you first step on a different type of metatile, there's a 40% chance it
// skips the wild encounter check entirely.
static bool8 AllowWildCheckOnNewMetatile(void)
{
    if (Random() % 100 >= 60)
        return FALSE;
    else
        return TRUE;
}

static bool8 AreLegendariesInSootopolisPreventingEncounters(void)
{
    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(MAP_SOOTOPOLIS_CITY)
     || gSaveBlock1Ptr->location.mapNum != MAP_NUM(MAP_SOOTOPOLIS_CITY))
    {
        return FALSE;
    }

    return FlagGet(FLAG_LEGENDARIES_IN_SOOTOPOLIS);
}

bool8 StandardWildEncounter(u16 curMetatileBehavior, u16 prevMetatileBehavior)
{
    u32 headerId;
    enum TimeOfDay timeOfDay;
    struct Roamer *roamer;

    if (sWildEncountersDisabled == TRUE)
        return FALSE;

    headerId = GetCurrentMapWildMonHeaderId();
    if (headerId == HEADER_NONE)
    {
        if ((gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS || gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS_HNS))
        {
            headerId = GetBattlePikeWildMonHeaderId();
            timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);

            if (prevMetatileBehavior != curMetatileBehavior && !AllowWildCheckOnNewMetatile())
                return FALSE;
            else if (WildEncounterCheck(gBattlePikeWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo->encounterRate, FALSE) != TRUE)
                return FALSE;
            else if (TryGenerateWildMon(gBattlePikeWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo, WILD_AREA_LAND, WILD_CHECK_KEEN_EYE) != TRUE)
                return FALSE;
            else if (!TryGenerateBattlePikeWildMon(TRUE))
                return FALSE;

            BattleSetup_StartBattlePikeWildBattle();
            return TRUE;
        }
        if ((gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR || gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR_HNS))
        {
            headerId = gSaveBlock2Ptr->frontier.curChallengeBattleNum;
            timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);

            if (prevMetatileBehavior != curMetatileBehavior && !AllowWildCheckOnNewMetatile())
                return FALSE;
            else if (WildEncounterCheck(gBattlePyramidWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo->encounterRate, FALSE) != TRUE)
                return FALSE;
            else if (TryGenerateWildMon(gBattlePyramidWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo, WILD_AREA_LAND, WILD_CHECK_KEEN_EYE) != TRUE)
                return FALSE;

            GenerateBattlePyramidWildMon();
            BattleSetup_StartWildBattle();
            return TRUE;
        }
    }
    else
    {
        if (MetatileBehavior_IsLandWildEncounter(curMetatileBehavior) == TRUE)
        {
            timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);

            if (gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo == NULL)
                return FALSE;
            else if (prevMetatileBehavior != curMetatileBehavior && !AllowWildCheckOnNewMetatile())
                return FALSE;
            else if (WildEncounterCheck(gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo->encounterRate, FALSE) != TRUE)
                return FALSE;

            if (TryStartRoamerEncounter())
            {
                roamer = &gSaveBlock1Ptr->roamer[gEncounteredRoamerIndex];
                if (!IsWildLevelAllowedByRepel(roamer->level))
                    return FALSE;

                BattleSetup_StartRoamerBattle();
                return TRUE;
            }
            else
            {
                if (DoMassOutbreakEncounterTest() == TRUE && SetUpMassOutbreakEncounter(WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE) == TRUE)
                {
                    BattleSetup_StartWildBattle();
                    return TRUE;
                }

                // try a regular wild land encounter
                if (TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE, WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE) == TRUE)
                {
                    if (TryDoDoubleWildBattle())
                    {
                        struct Pokemon mon1 = gEnemyParty[0];
                        TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE, WILD_CHECK_KEEN_EYE);
                        gEnemyParty[1] = mon1;
                        BattleSetup_StartDoubleWildBattle();
                    }
                    else
                    {
                        BattleSetup_StartWildBattle();
                    }
                    return TRUE;
                }

                return FALSE;
            }
        }
        else if (MetatileBehavior_IsWaterWildEncounter(curMetatileBehavior) == TRUE
                 || (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) && MetatileBehavior_IsBridgeOverWater(curMetatileBehavior) == TRUE))
        {
            timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_WATER);

            if (AreLegendariesInSootopolisPreventingEncounters() == TRUE)
                return FALSE;
            else if (gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo == NULL)
                return FALSE;
            else if (prevMetatileBehavior != curMetatileBehavior && !AllowWildCheckOnNewMetatile())
                return FALSE;
            else if (WildEncounterCheck(gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo->encounterRate, FALSE) != TRUE)
                return FALSE;

            if (TryStartRoamerEncounter())
            {
                roamer = &gSaveBlock1Ptr->roamer[gEncounteredRoamerIndex];
                if (!IsWildLevelAllowedByRepel(roamer->level))
                    return FALSE;

                BattleSetup_StartRoamerBattle();
                return TRUE;
            }
            else // try a regular surfing encounter
            {
                if (TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_WATER, WILD_ENCOUNTER_FISHING_ROD_NONE, WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE) == TRUE)
                {
                    gIsSurfingEncounter = TRUE;
                    if (TryDoDoubleWildBattle())
                    {
                        struct Pokemon mon1 = gEnemyParty[0];
                        TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_WATER, WILD_ENCOUNTER_FISHING_ROD_NONE, WILD_CHECK_KEEN_EYE);
                        gEnemyParty[1] = mon1;
                        BattleSetup_StartDoubleWildBattle();
                    }
                    else
                    {
                        BattleSetup_StartWildBattle();
                    }
                    return TRUE;
                }

                return FALSE;
            }
        }
    }

    return FALSE;
}

void RockSmashWildEncounter(void)
{
    u32 headerId = GetCurrentMapWildMonHeaderId();
    enum TimeOfDay timeOfDay;

    if (headerId != HEADER_NONE)
    {
        timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_ROCKS);

        const struct WildPokemonInfo *wildPokemonInfo = gWildMonHeaders[headerId].encounterTypes[timeOfDay].rockSmashMonsInfo;

        if (wildPokemonInfo == NULL)
        {
            gSpecialVar_Result = FALSE;
        }
        else if (WildEncounterCheck(wildPokemonInfo->encounterRate, TRUE) == TRUE
         && TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_ROCKS, WILD_ENCOUNTER_FISHING_ROD_NONE, WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE) == TRUE)
        {
            if (TryDoDoubleWildBattle())
            {
                struct Pokemon mon1 = gEnemyParty[0];
                TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_ROCKS, WILD_ENCOUNTER_FISHING_ROD_NONE, WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE);
                gEnemyParty[1] = mon1;
                BattleSetup_StartDoubleWildBattle();
                gSpecialVar_Result = TRUE;
            }
            else {
                BattleSetup_StartWildBattle();
                gSpecialVar_Result = TRUE;
            }
        }
        else
        {
            gSpecialVar_Result = FALSE;
        }
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

bool8 SweetScentWildEncounter(void)
{
    s16 x, y;
    u32 headerId;
    enum TimeOfDay timeOfDay;

    PlayerGetDestCoords(&x, &y);
    headerId = GetCurrentMapWildMonHeaderId();
    if (headerId == HEADER_NONE)
    {
        if ((gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS || gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS_HNS))
        {
            headerId = GetBattlePikeWildMonHeaderId();
            timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);

            if (TryGenerateWildMon(gBattlePikeWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo, WILD_AREA_LAND, 0) != TRUE)
                return FALSE;

            TryGenerateBattlePikeWildMon(FALSE);
            BattleSetup_StartBattlePikeWildBattle();
            return TRUE;
        }
        if ((gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR || gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR_HNS))
        {
            headerId = gSaveBlock2Ptr->frontier.curChallengeBattleNum;
            timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);

            if (TryGenerateWildMon(gBattlePyramidWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo, WILD_AREA_LAND, 0) != TRUE)
                return FALSE;

            GenerateBattlePyramidWildMon();
            BattleSetup_StartWildBattle();
            return TRUE;
        }
    }
    else
    {
        if (MetatileBehavior_IsLandWildEncounter(MapGridGetMetatileBehaviorAt(x, y)) == TRUE)
        {
            timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);

            if (gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo == NULL)
                return FALSE;

            if (TryStartRoamerEncounter())
            {
                BattleSetup_StartRoamerBattle();
                return TRUE;
            }

            if (DoMassOutbreakEncounterTest() == TRUE)
                SetUpMassOutbreakEncounter(0);
            else if (!TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE, 0))
                return FALSE;

            BattleSetup_StartWildBattle();
            return TRUE;
        }
        else if (MetatileBehavior_IsWaterWildEncounter(MapGridGetMetatileBehaviorAt(x, y)) == TRUE)
        {
            timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_WATER);

            if (AreLegendariesInSootopolisPreventingEncounters() == TRUE)
                return FALSE;
            if (gWildMonHeaders[headerId].encounterTypes[timeOfDay].waterMonsInfo == NULL)
                return FALSE;

            if (TryStartRoamerEncounter())
            {
                BattleSetup_StartRoamerBattle();
                return TRUE;
            }

            if (!TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_WATER, WILD_ENCOUNTER_FISHING_ROD_NONE, 0))
                return FALSE;
            BattleSetup_StartWildBattle();
            return TRUE;
        }
    }

    return FALSE;
}

bool8 DoesCurrentMapHaveFishingMons(u8 rod)
{
    u32 headerId = GetCurrentMapWildMonHeaderId();
    enum TimeOfDay timeOfDay;
    struct WildEncounterProfileContext context;
    struct WildEncounterProfileView view;

    if (headerId == HEADER_NONE)
        return FALSE;

    timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_FISHING);
    context = (struct WildEncounterProfileContext)
    {
        .headerId = headerId,
        .timeOfDay = timeOfDay,
        .area = WILD_AREA_FISHING,
        .fishingRod = rod,
    };
    if (!GetWildEncounterProfileView(&context, &view))
        return FALSE;
    return DoesWildEncounterProfileHaveAvailableEntries(&view, GetTrainerRating(), IsCurrentWildEncounterRandomized());
}

void FishingWildEncounter(u8 rod)
{
    u16 species;
    u32 headerId = HEADER_NONE;
    enum TimeOfDay timeOfDay = TIME_OF_DAY_DEFAULT;
    bool8 useFeebasOverride;

    gIsFishingEncounter = FALSE;
    useFeebasOverride = CheckFeebas();
    if (!useFeebasOverride)
    {
        headerId = GetCurrentMapWildMonHeaderId();
        if (headerId == HEADER_NONE)
            return;

        timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_FISHING);
    }
    species = GenerateFishingWildMon(headerId, timeOfDay, rod, useFeebasOverride);

    // A malformed or future all-locked profile does not create an enemy. Do
    // not advance fishing state or start an invalid SPECIES_NONE battle.
    if (species == SPECIES_NONE)
        return;

    gIsFishingEncounter = TRUE;
    IncrementGameStat(GAME_STAT_FISHING_ENCOUNTERS);
    SetPokemonAnglerSpecies(species);
    BattleSetup_StartWildBattle();
}

static u16 GenerateFishingWildMon(u32 headerId, enum TimeOfDay timeOfDay, u8 rod, bool8 useFeebasOverride)
{
    if (useFeebasOverride)
    {
        u8 level = ChooseWildMonLevel(&sWildFeebas, 0, WILD_AREA_FISHING);

        CreateWildMon(sWildFeebas.species, level);
        return sWildFeebas.species;
    }
    return GenerateFishingWildMonFromProfile(headerId, timeOfDay, rod);
}

u16 GetLocalWildMon(bool8 *isWaterMon)
{
    u32 headerId;
    enum TimeOfDay landTimeOfDay;
    enum TimeOfDay waterTimeOfDay;
    struct WildEncounterProfileContext context;
    struct WildEncounterProfileView landView;
    struct WildEncounterProfileView waterView;
    bool8 hasLand;
    bool8 hasWater;

    *isWaterMon = FALSE;
    headerId = GetCurrentMapWildMonHeaderId();
    if (headerId == HEADER_NONE)
        return SPECIES_NONE;

    landTimeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);
    context = (struct WildEncounterProfileContext)
    {
        .headerId = headerId,
        .timeOfDay = landTimeOfDay,
        .area = WILD_AREA_LAND,
        .fishingRod = WILD_ENCOUNTER_FISHING_ROD_NONE,
    };
    hasLand = GetWildEncounterProfileView(&context, &landView);

    waterTimeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_WATER);
    context.timeOfDay = waterTimeOfDay;
    context.area = WILD_AREA_WATER;
    hasWater = GetWildEncounterProfileView(&context, &waterView);

    // Neither
    if (!hasLand && !hasWater)
        return SPECIES_NONE;
    // Land Pokémon
    else if (hasLand && !hasWater)
        return GetLocalWildEncounterProfileSpecies(&landView);
    // Water Pokémon
    else if (!hasLand && hasWater)
    {
        *isWaterMon = TRUE;
        return GetLocalWildEncounterProfileSpecies(&waterView);
    }
    // Either land or water Pokémon
    if ((Random() % 100) < 80)
    {
        return GetLocalWildEncounterProfileSpecies(&landView);
    }
    else
    {
        *isWaterMon = TRUE;
        return GetLocalWildEncounterProfileSpecies(&waterView);
    }
}

u16 GetLocalWaterMon(void)
{
    u32 headerId = GetCurrentMapWildMonHeaderId();
    enum TimeOfDay timeOfDay;

    if (headerId != HEADER_NONE)
    {
        timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_WATER);
        struct WildEncounterProfileContext context =
        {
            .headerId = headerId,
            .timeOfDay = timeOfDay,
            .area = WILD_AREA_WATER,
            .fishingRod = WILD_ENCOUNTER_FISHING_ROD_NONE,
        };
        struct WildEncounterProfileView view;

        if (GetWildEncounterProfileView(&context, &view))
            return GetLocalWildEncounterProfileSpecies(&view);
    }
    return SPECIES_NONE;
}

bool8 UpdateRepelCounter(void)
{
    u16 repelLureVar = VarGet(VAR_REPEL_STEP_COUNT);
    u16 steps = REPEL_LURE_STEPS(repelLureVar);
    bool32 isLure = IS_LAST_USED_LURE(repelLureVar);

    if (InBattlePike() || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
        return FALSE;
    if (InUnionRoom() == TRUE)
        return FALSE;

    if (steps != 0)
    {
        steps--;
        if (!isLure)
        {
            VarSet(VAR_REPEL_STEP_COUNT, steps);
            if (steps == 0)
            {
                ScriptContext_SetupScript(EventScript_SprayWoreOff);
                return TRUE;
            }
        }
        else
        {
            VarSet(VAR_REPEL_STEP_COUNT, steps | REPEL_LURE_MASK);
            if (steps == 0)
            {
                ScriptContext_SetupScript(EventScript_SprayWoreOff);
                return TRUE;
            }
        }

    }
    return FALSE;
}

static bool8 IsWildLevelAllowedByRepel(u8 wildLevel)
{
    u8 i;

    if (!REPEL_STEP_COUNT)
        return TRUE;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (I_REPEL_INCLUDE_FAINTED == GEN_1 || I_REPEL_INCLUDE_FAINTED >= GEN_6 || GetMonData(&gPlayerParty[i], MON_DATA_HP))
        {
            if (!GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
                return wildLevel >= GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);
        }
    }

    return FALSE;
}

static bool8 IsAbilityAllowingEncounter(u8 level)
{
    enum Ability ability;

    if (GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
        return TRUE;

    ability = GetMonAbility(&gPlayerParty[0]);
    if (ability == ABILITY_KEEN_EYE || ability == ABILITY_INTIMIDATE)
    {
        u8 playerMonLevel = GetMonData(&gPlayerParty[0], MON_DATA_LEVEL);
        if (playerMonLevel > 5 && level <= playerMonLevel - 5 && !(Random() % 2))
            return FALSE;
    }

    return TRUE;
}

static bool8 TryGetRandomWildMonIndexByType(const struct WildPokemon *wildMon, enum Type type, u8 numMon, u8 *monIndex)
{
    u8 validIndexes[numMon]; // variable length array, an interesting feature
    u8 i, validMonCount;

    for (i = 0; i < numMon; i++)
        validIndexes[i] = 0;

    for (validMonCount = 0, i = 0; i < numMon; i++)
    {
        if (GetSpeciesType(wildMon[i].species, 0) == type || GetSpeciesType(wildMon[i].species, 1) == type)
            validIndexes[validMonCount++] = i;
    }

    if (validMonCount == 0 || validMonCount == numMon)
        return FALSE;

    *monIndex = validIndexes[Random() % validMonCount];
    return TRUE;
}

#if IS_HNS
#define HOENN_DEX_START SPECIES_TREECKO
#define HOENN_DEX_END   386

static bool8 TryGetHoennSoundWildMonIndex(const struct WildPokemon *wildMon, u8 numMon, u8 *monIndex)
{
    u8 validIndexes[12];
    u8 i, validMonCount;

    if (!IsHoennSoundPlaying())
        return FALSE;
    if (Random() % 10 != 0)
        return FALSE;

    for (validMonCount = 0, i = 0; i < numMon; i++)
    {
        u16 species = wildMon[i].species;
        if (species >= HOENN_DEX_START && species <= HOENN_DEX_END)
            validIndexes[validMonCount++] = i;
    }

    if (validMonCount == 0 || validMonCount == numMon)
        return FALSE;

    *monIndex = validIndexes[Random() % validMonCount];
    return TRUE;
}
#endif

#include "data.h"

static u8 GetMaxLevelOfSpeciesInWildTable(const struct WildPokemon *wildMon, u16 species, enum WildPokemonArea area)
{
    u8 i, maxLevel = 0, numMon = 0;

    switch (area)
    {
    case WILD_AREA_LAND:
        numMon = LAND_WILD_COUNT;
        break;
    case WILD_AREA_WATER:
        numMon = WATER_WILD_COUNT;
        break;
    case WILD_AREA_ROCKS:
        numMon = ROCK_WILD_COUNT;
        break;
    default:
    case WILD_AREA_FISHING:
    case WILD_AREA_HIDDEN:
        break;
    }

    for (i = 0; i < numMon; i++)
    {
        if (wildMon[i].species == species && wildMon[i].maxLevel > maxLevel)
            maxLevel = wildMon[i].maxLevel;
    }

    return maxLevel;
}

#ifdef BUGFIX
static bool8 TryGetAbilityInfluencedWildMonIndex(const struct WildPokemon *wildMon, enum Type type, enum Ability ability, u8 *monIndex, u32 size)
#else
static bool8 TryGetAbilityInfluencedWildMonIndex(const struct WildPokemon *wildMon, enum Type type, enum Ability ability, u8 *monIndex)
#endif
{
    if (GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
        return FALSE;
    else if (GetMonAbility(&gPlayerParty[0]) != ability)
        return FALSE;
    else if (Random() % 2 != 0)
        return FALSE;

#ifdef BUGFIX
    return TryGetRandomWildMonIndexByType(wildMon, type, size, monIndex);
#else
    return TryGetRandomWildMonIndexByType(wildMon, type, LAND_WILD_COUNT, monIndex);
#endif
}

// The ordinary path selects from the profile's eligible slots, but retains the
// raw game's modifier order. In particular, type-attraction and Hoenn Sound
// use a uniform matching-slot choice rather than weighted encounter odds.
static bool8 GetWildEncounterProfileTypeSlots(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u8 type, u8 *matchingSlots, u8 *eligibleCount, u8 *matchingCount)
{
    u8 candidate;

    if (view == NULL || matchingSlots == NULL || eligibleCount == NULL || matchingCount == NULL
     || !IsWildEncounterProfileSlotInRange(view, view->entryStart))
        return FALSE;

    *eligibleCount = 0;
    *matchingCount = 0;

    for (candidate = view->entryStart; candidate < view->entryStart + view->entryCount; candidate++)
    {
        const struct WildPokemon *entry;

        if (!IsWildEncounterProfileSlotEligible(view, candidate, trainerRating, isWildRandomized))
            continue;

        (*eligibleCount)++;
        if (!GetWildEncounterProfileEntry(view, candidate, &entry))
            return FALSE;
        if (GetSpeciesType(entry->species, 0) == type || GetSpeciesType(entry->species, 1) == type)
            matchingSlots[(*matchingCount)++] = candidate;
    }

    return TRUE;
}

bool8 SelectWildEncounterProfileTypeSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u8 type, u8 roll, u8 *slot)
{
    u8 matchingSlots[LAND_WILD_COUNT];
    u8 eligibleCount;
    u8 matchingCount;

    if (slot == NULL
     || !GetWildEncounterProfileTypeSlots(view, trainerRating, isWildRandomized, type, matchingSlots, &eligibleCount, &matchingCount)
     || matchingCount == 0
     || matchingCount == eligibleCount
     || roll >= matchingCount)
        return FALSE;

    *slot = matchingSlots[roll];
    return TRUE;
}

static bool8 TryGetRandomWildEncounterProfileSlotByType(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, enum Type type, u8 *slot)
{
    u8 matchingSlots[LAND_WILD_COUNT];
    u8 eligibleCount;
    u8 matchingCount;

    if (!GetWildEncounterProfileTypeSlots(view, trainerRating, isWildRandomized, type, matchingSlots, &eligibleCount, &matchingCount)
     || matchingCount == 0
     || matchingCount == eligibleCount)
        return FALSE;

    *slot = matchingSlots[Random() % matchingCount];
    return TRUE;
}

static bool8 TryGetAbilityInfluencedWildEncounterProfileSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, enum Type type, enum Ability ability, u8 *slot)
{
    if (GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
        return FALSE;
    else if (GetMonAbility(&gPlayerParty[0]) != ability)
        return FALSE;
    else if (Random() % 2 != 0)
        return FALSE;

    return TryGetRandomWildEncounterProfileSlotByType(view, trainerRating, isWildRandomized, type, slot);
}

#if IS_HNS
static bool8 TrySelectHoennSoundWildEncounterProfileSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, bool8 useSuppliedRoll, u8 suppliedRoll, u8 *slot)
{
    u8 matchingSlots[LAND_WILD_COUNT];
    u8 candidate;
    u8 eligibleCount = 0;
    u8 matchingCount = 0;

    for (candidate = view->entryStart; candidate < view->entryStart + view->entryCount; candidate++)
    {
        const struct WildPokemon *entry;

        if (!IsWildEncounterProfileSlotEligible(view, candidate, trainerRating, isWildRandomized))
            continue;

        eligibleCount++;
        if (!GetWildEncounterProfileEntry(view, candidate, &entry))
            return FALSE;
        if (entry->species >= HOENN_DEX_START && entry->species <= HOENN_DEX_END)
            matchingSlots[matchingCount++] = candidate;
    }

    if (matchingCount == 0 || matchingCount == eligibleCount)
        return FALSE;

    *slot = matchingSlots[(useSuppliedRoll ? suppliedRoll : Random()) % matchingCount];
    return TRUE;
}

static bool8 TryGetHoennSoundWildEncounterProfileSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u8 *slot)
{
    if (!IsHoennSoundPlaying())
        return FALSE;
    if (Random() % 10 != 0)
        return FALSE;

    return TrySelectHoennSoundWildEncounterProfileSlot(view, trainerRating, isWildRandomized, FALSE, 0, slot);
}
#endif

bool8 GetWildEncounterProfileMirroredEligibleSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u8 slot, u8 *mirroredSlot)
{
    u8 eligibleSlots[LAND_WILD_COUNT];
    u8 candidate;
    u8 eligibleCount = 0;
    u8 position;

    if (mirroredSlot == NULL || view == NULL || !IsWildEncounterProfileSlotInRange(view, view->entryStart))
        return FALSE;

    for (candidate = view->entryStart; candidate < view->entryStart + view->entryCount; candidate++)
    {
        if (IsWildEncounterProfileSlotEligible(view, candidate, trainerRating, isWildRandomized))
            eligibleSlots[eligibleCount++] = candidate;
    }

    for (position = 0; position < eligibleCount; position++)
    {
        if (eligibleSlots[position] == slot)
        {
            *mirroredSlot = eligibleSlots[eligibleCount - position - 1];
            return TRUE;
        }
    }

    return FALSE;
}

bool8 DoesWildEncounterProfileHaveAvailableEntries(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized)
{
    return view != NULL
        && view->wildMonsInfo != NULL
        && view->wildMonsInfo->encounterRate != 0
        && GetWildEncounterProfileEligibleWeight(view, trainerRating, isWildRandomized) != 0;
}

static bool8 TrySelectWildEncounterProfileBaseSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u8 *slot)
{
    u16 eligibleWeight = GetWildEncounterProfileEligibleWeight(view, trainerRating, isWildRandomized);

    if (eligibleWeight == 0 || !SelectWildEncounterProfileSlot(view, trainerRating, isWildRandomized, Random() % eligibleWeight, slot))
        return FALSE;

    // In an unfiltered profile this is exactly the legacy index reversal. If
    // scaling locks a slot, reverse only across the eligible sequence so a
    // lure cannot revive a locked encounter.
    if (LURE_STEP_COUNT != 0 && Random() % 10 < 2)
        return GetWildEncounterProfileMirroredEligibleSlot(view, trainerRating, isWildRandomized, *slot, slot);

    return TRUE;
}

static bool8 TrySelectWildEncounterProfileSlotWithModifiers(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u8 *slot)
{
    switch (view->area)
    {
    case WILD_AREA_LAND:
        if (TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_STEEL, ABILITY_MAGNET_PULL, slot))
            return TRUE;
        if (TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_ELECTRIC, ABILITY_STATIC, slot))
            return TRUE;
        if (OW_LIGHTNING_ROD >= GEN_8 && TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_ELECTRIC, ABILITY_LIGHTNING_ROD, slot))
            return TRUE;
        if (OW_FLASH_FIRE >= GEN_8 && TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_FIRE, ABILITY_FLASH_FIRE, slot))
            return TRUE;
        if (OW_HARVEST >= GEN_8 && TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_GRASS, ABILITY_HARVEST, slot))
            return TRUE;
        if (OW_STORM_DRAIN >= GEN_8 && TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_WATER, ABILITY_STORM_DRAIN, slot))
            return TRUE;
    #if IS_HNS
        if (TryGetHoennSoundWildEncounterProfileSlot(view, trainerRating, isWildRandomized, slot))
            return TRUE;
    #endif
        break;
    case WILD_AREA_WATER:
        if (TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_STEEL, ABILITY_MAGNET_PULL, slot))
            return TRUE;
        if (TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_ELECTRIC, ABILITY_STATIC, slot))
            return TRUE;
        if (OW_LIGHTNING_ROD >= GEN_8 && TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_ELECTRIC, ABILITY_LIGHTNING_ROD, slot))
            return TRUE;
        if (OW_FLASH_FIRE >= GEN_8 && TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_FIRE, ABILITY_FLASH_FIRE, slot))
            return TRUE;
        if (OW_HARVEST >= GEN_8 && TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_GRASS, ABILITY_HARVEST, slot))
            return TRUE;
        if (OW_STORM_DRAIN >= GEN_8 && TryGetAbilityInfluencedWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TYPE_WATER, ABILITY_STORM_DRAIN, slot))
            return TRUE;
    #if IS_HNS
        if (TryGetHoennSoundWildEncounterProfileSlot(view, trainerRating, isWildRandomized, slot))
            return TRUE;
    #endif
        break;
    default:
        break;
    }

    return TrySelectWildEncounterProfileBaseSlot(view, trainerRating, isWildRandomized, slot);
}

static bool8 TryGenerateWildMonFromProfile(u32 headerId, enum TimeOfDay timeOfDay, enum WildPokemonArea area, enum WildEncounterFishingRod fishingRod, u8 flags)
{
    struct WildEncounterProfileContext context =
    {
        .headerId = headerId,
        .timeOfDay = timeOfDay,
        .area = area,
        .fishingRod = fishingRod,
    };
    struct WildEncounterProfileView view;
    const struct WildPokemon *entry;
    struct WildEncounterSpeciesOutcome outcome;
    u16 trainerRating = GetTrainerRating();
    bool8 isWildRandomized = IsCurrentWildEncounterRandomized();
    u8 slot;
    u8 authoredLevel;
    u16 species;

    if (!GetWildEncounterProfileView(&context, &view)
     || !TrySelectWildEncounterProfileSlotWithModifiers(&view, trainerRating, isWildRandomized, &slot)
     || !GetWildEncounterProfileEntry(&view, slot, &entry))
        return FALSE;

    // Roll the authored range first. This retains Pressure / Vital Spirit and
    // lure-level behavior before the rating projection changes the result.
    authoredLevel = ChooseWildMonLevel(view.wildMonsInfo->wildPokemon, slot, area);
    if (!GetWildEncounterSpeciesOutcome(&view, slot, authoredLevel, trainerRating, isWildRandomized, &outcome))
        return FALSE;

    if (flags & WILD_CHECK_REPEL && !IsWildLevelAllowedByRepel(outcome.level))
        return FALSE;
    if ((gMapHeader.mapLayoutId != LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS && gMapHeader.mapLayoutId != LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS_HNS) && flags & WILD_CHECK_KEEN_EYE && !IsAbilityAllowingEncounter(outcome.level))
        return FALSE;

    species = outcome.species;
    #if RANDOMIZER_AVAILABLE == TRUE
    // The randomizer still receives the authored species and raw slot index in
    // its original position after all selection and level checks.
    species = RandomizeAuthoredWildEncounter(entry, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, area, slot);
    #endif
    CreateWildMon(species, outcome.level);
    return TRUE;
}

static u16 GenerateFishingWildMonFromProfile(u32 headerId, enum TimeOfDay timeOfDay, u8 rod)
{
    struct WildEncounterProfileContext context =
    {
        .headerId = headerId,
        .timeOfDay = timeOfDay,
        .area = WILD_AREA_FISHING,
        .fishingRod = rod,
    };
    struct WildEncounterProfileView view;
    const struct WildPokemon *entry;
    struct WildEncounterSpeciesOutcome outcome;
    u16 trainerRating = GetTrainerRating();
    bool8 isWildRandomized = IsCurrentWildEncounterRandomized();
    u8 slot;
    u8 authoredLevel;
    u16 species;

    if (!GetWildEncounterProfileView(&context, &view)
     || !TrySelectWildEncounterProfileSlotWithModifiers(&view, trainerRating, isWildRandomized, &slot)
     || !GetWildEncounterProfileEntry(&view, slot, &entry))
        return SPECIES_NONE;

    authoredLevel = ChooseWildMonLevel(view.wildMonsInfo->wildPokemon, slot, WILD_AREA_FISHING);
    if (!GetWildEncounterSpeciesOutcome(&view, slot, authoredLevel, trainerRating, isWildRandomized, &outcome))
        return SPECIES_NONE;

    species = outcome.species;
    #if RANDOMIZER_AVAILABLE == TRUE
    species = RandomizeAuthoredWildEncounter(entry, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, WILD_AREA_FISHING, slot);
    #endif

    UpdateChainFishingStreak();
    CreateWildMon(species, outcome.level);
    return species;
}

#if TESTING
u16 GenerateFeebasFishingWildMonForTesting(u8 rod)
{
    return GenerateFishingWildMon(HEADER_NONE, TIME_OF_DAY_DEFAULT, rod, TRUE);
}

#if IS_HNS
bool8 SelectWildEncounterProfileSlotWithHoennSoundForTesting(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, bool8 isHoennSoundPlaying, u8 activationRoll, u8 selectionRoll, u16 baseRoll, u8 *slot)
{
    if (isHoennSoundPlaying
     && activationRoll % 10 == 0
     && TrySelectHoennSoundWildEncounterProfileSlot(view, trainerRating, isWildRandomized, TRUE, selectionRoll, slot))
        return TRUE;

    return SelectWildEncounterProfileSlot(view, trainerRating, isWildRandomized, baseRoll, slot);
}
#endif

#if RANDOMIZER_AVAILABLE == TRUE
u16 RandomizeWildEncounterProfileEntryForTesting(const struct WildEncounterProfileView *view, u8 slot, u8 mapNum, u8 mapGroup, enum WildPokemonArea area)
{
    const struct WildPokemon *entry;

    if (!GetWildEncounterProfileEntry(view, slot, &entry))
        return SPECIES_NONE;
    return RandomizeAuthoredWildEncounter(entry, mapNum, mapGroup, area, slot);
}
#endif
#endif

static u16 GetLocalWildEncounterProfileSpecies(const struct WildEncounterProfileView *view)
{
    const struct WildPokemon *entry;
    struct WildEncounterSpeciesOutcome outcome;
    u16 trainerRating = GetTrainerRating();
    bool8 isWildRandomized = IsCurrentWildEncounterRandomized();
    u8 slot;

    // These readers have historically owned only their slot-selection RNG.
    // Use the low end of that slot's authored range to resolve an effective
    // species without adding a level-roll side effect.
    if (!TrySelectWildEncounterProfileBaseSlot(view, trainerRating, isWildRandomized, &slot)
     || !GetWildEncounterProfileEntry(view, slot, &entry)
     || !GetWildEncounterSpeciesOutcome(view, slot, min(entry->minLevel, entry->maxLevel), trainerRating, isWildRandomized, &outcome))
        return SPECIES_NONE;

    return outcome.species;
}

static void ApplyFluteEncounterRateMod(u32 *encRate)
{
    if (FlagGet(FLAG_SYS_ENC_UP_ITEM) == TRUE)
        *encRate += *encRate / 2;
    else if (FlagGet(FLAG_SYS_ENC_DOWN_ITEM) == TRUE)
        *encRate = *encRate / 2;
}

static void ApplyCleanseTagEncounterRateMod(u32 *encRate)
{
    if (GetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM) == ITEM_CLEANSE_TAG)
        *encRate = *encRate * 2 / 3;
}

bool8 TryDoDoubleWildBattle(void)
{
    if (GetSafariZoneFlag()
      || (B_DOUBLE_WILD_REQUIRE_2_MONS == TRUE && GetMonsStateToDoubles() != PLAYER_HAS_TWO_USABLE_MONS))
        return FALSE;
    if (FollowerNPCIsBattlePartner() && FNPC_FLAG_PARTNER_WILD_BATTLES != 0
     && (FNPC_FLAG_PARTNER_WILD_BATTLES == FNPC_ALWAYS || FlagGet(FNPC_FLAG_PARTNER_WILD_BATTLES)) && FNPC_NPC_FOLLOWER_WILD_BATTLE_VS_2 == TRUE)
        return TRUE;
    else if (B_FLAG_FORCE_DOUBLE_WILD != 0 && FlagGet(B_FLAG_FORCE_DOUBLE_WILD))
        return TRUE;
    else if (B_DOUBLE_WILD_CHANCE != 0 && ((Random() % 100) + 1 <= B_DOUBLE_WILD_CHANCE))
        return TRUE;
    return FALSE;
}

bool8 StandardWildEncounter_Debug(void)
{
    u32 headerId = GetCurrentMapWildMonHeaderId();
    enum TimeOfDay timeOfDay;

    if (headerId == HEADER_NONE)
        return FALSE;

    timeOfDay = GetTimeOfDayForEncounters(headerId, WILD_AREA_LAND);

    if (TryGenerateWildMonFromProfile(headerId, timeOfDay, WILD_AREA_LAND, WILD_ENCOUNTER_FISHING_ROD_NONE, 0) != TRUE)
        return FALSE;

    DoStandardWildBattle_Debug();
    return TRUE;
}

u32 ChooseHiddenMonIndex(void)
{
    #ifdef ENCOUNTER_CHANCE_HIDDEN_MONS_TOTAL
        u8 rand = Random() % ENCOUNTER_CHANCE_HIDDEN_MONS_TOTAL;

        if (rand < ENCOUNTER_CHANCE_HIDDEN_MONS_SLOT_0)
            return 0;
        else if (rand >= ENCOUNTER_CHANCE_HIDDEN_MONS_SLOT_0 && rand < ENCOUNTER_CHANCE_HIDDEN_MONS_SLOT_1)
            return 1;
        else
            return 2;
    #else
        return 0xFF;
    #endif
}

bool32 MapHasNoEncounterData(void)
{
    return (GetCurrentMapWildMonHeaderId() == HEADER_NONE);
}
