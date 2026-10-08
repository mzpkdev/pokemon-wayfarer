#ifndef GUARD_WILD_ENCOUNTER_H
#define GUARD_WILD_ENCOUNTER_H

#include "rtc.h"
#include "config/randomizer.h"
#include "constants/wild_encounter.h"

#define HEADER_NONE 0xFFFF

#if TESTING
void SetWildStartInterceptionForTesting(bool8 enabled);
u32 GetWildStartsForTesting(void);
#endif

enum WildPokemonArea {
    WILD_AREA_LAND,
    WILD_AREA_WATER,
    WILD_AREA_ROCKS,
    WILD_AREA_FISHING,
    WILD_AREA_HIDDEN
};

struct WildPokemon
{
    u8 minLevel;
    u8 maxLevel;
    u16 species;
};

struct WildPokemonInfo
{
    u8 encounterRate;
    const struct WildPokemon *wildPokemon;
};

struct WildEncounterTypes
{
    const struct WildPokemonInfo *landMonsInfo;
    const struct WildPokemonInfo *waterMonsInfo;
    const struct WildPokemonInfo *rockSmashMonsInfo;
    const struct WildPokemonInfo *fishingMonsInfo;
    const struct WildPokemonInfo *hiddenMonsInfo;
};

struct WildPokemonHeader
{
    u8 mapGroup;
    u8 mapNum;
    const struct WildEncounterTypes encounterTypes[TIMES_OF_DAY_COUNT];
};

// Wild level scaling v2. A place's level comes from the Trainer Rating and the
// place's reach (see specs/wild-level-scaling.md); these generated records say
// which reach, dungeon intent and floor each header has, and which species are
// prowlers with a minimum level.
enum WildEncounterReach
{
    WILD_REACH_ROAD,
    WILD_REACH_WILDS,
    WILD_REACH_OUTLANDS,
    WILD_REACH_DUNGEON,
};

enum WildDungeonIntent
{
    WILD_DUNGEON_MILD,
    WILD_DUNGEON_MILD_TO_MODERATE,
    WILD_DUNGEON_MODERATE,
    WILD_DUNGEON_MODERATE_TO_HARD,
    WILD_DUNGEON_HARD,
    WILD_DUNGEON_BRUTAL,
};

enum WildEncounterPlaceRegion
{
    WILD_PLACE_REGION_OTHER,
    WILD_PLACE_REGION_SAFARI,
    WILD_PLACE_REGION_SINJOH,
};

struct WildEncounterPlace
{
    u8 reach:2;        // enum WildEncounterReach
    u8 intent:3;       // enum WildDungeonIntent (dungeons only, else 0)
    u8 flat:1;         // dungeon is single-floor or flat -> middle of range on every map
    u8 region:2;       // enum WildEncounterPlaceRegion (prowler exemptions)
    u8 floor;          // 0-based step in the dungeon floor order (0 when not a dungeon)
    u8 floorCount;     // number of steps (0/1 when single-floor/flat or not a dungeon)
};

struct WildProwlerMinimum
{
    u16 species;            // every species of a prowler line (each stage, regional form as its own line)
    u8 minimumLevel:6;      // 20, 25 or 30
    u8 exemptInSafari:1;    // Kalos reward: no minimum in Safari-region places
    u8 exemptInSinjoh:1;    // Sinjoh resident: no minimum in Sinjoh-region places
};

enum WildEncounterFishingRod
{
    WILD_ENCOUNTER_FISHING_ROD_OLD,
    WILD_ENCOUNTER_FISHING_ROD_GOOD,
    WILD_ENCOUNTER_FISHING_ROD_SUPER,
    WILD_ENCOUNTER_FISHING_ROD_NONE,
};

// A context identifies one authored, ordinary wild encounter table. Hidden and
// special encounter sources deliberately do not resolve to a profile view.
struct WildEncounterProfileContext
{
    u16 headerId;
    enum TimeOfDay timeOfDay;
    enum WildPokemonArea area;
    enum WildEncounterFishingRod fishingRod;
};

// The active slice of an authored table. Every fishing quality views the same
// authored ten-entry prefix with a different generated weight profile.
struct WildEncounterProfileView
{
    const struct WildPokemonInfo *wildMonsInfo;
    const u8 *weights;
    u16 headerId;
    u8 timeOfDay;
    u8 area;
    u8 fishingRod;
    u8 entryStart;
    u8 entryCount;
};

struct WildEncounterSpeciesOutcome
{
    u16 species;
    u8 level;
};

// One entry of a slot's exact outcome distribution. The weights of a slot's
// entries add up to WILD_ENCOUNTER_OUTCOME_DENOMINATOR.
#define WILD_ENCOUNTER_OUTCOME_DENOMINATOR 50
#define WILD_ENCOUNTER_MAX_SLOT_OUTCOMES 10

struct WildEncounterSlotOutcome
{
    u16 species;
    u8 level;
    u8 weight;
};


extern const struct WildPokemonHeader gWildMonHeaders[];
extern const struct WildEncounterPlace gWildEncounterPlaces[]; // parallel to gWildMonHeaders
extern const struct WildProwlerMinimum gWildProwlerMinimums[];   // sorted by species
extern const u16 gWildProwlerMinimumCount;
extern const u16 gWildMonHeaderCount;
extern const u8 gStandardRodFishingWeights[WILD_ENCOUNTER_FISHING_ROD_NONE][FISH_WILD_COUNT];
extern bool8 gIsFishingEncounter;
extern bool8 gIsSurfingEncounter;
extern u8 gChainFishingDexNavStreak;

void DisableWildEncounters(bool8 disabled);
bool8 StandardWildEncounter(u16 curMetatileBehavior, u16 prevMetatileBehavior);
void RockSmashWildEncounter(void);
bool8 SweetScentWildEncounter(void);
bool8 DoesCurrentMapHaveFishingMons(u8 rod);
void FishingWildEncounter(u8 rod);
u16 GetLocalWildMon(bool8 *isWaterMon);
u16 GetLocalWaterMon(void);
bool8 UpdateRepelCounter(void);
bool8 TryDoDoubleWildBattle(void);
bool8 StandardWildEncounter_Debug(void);
u32 CalculateChainFishingShinyRolls(void);
void CreateWildMon(u16 species, u8 level);
u16 GetCurrentMapWildMonHeaderId(void);
// The first header of a map, or HEADER_NONE. Maps with one table per day or set have consecutive headers.
u32 GetWildMonHeaderIdForMap(u8 mapGroup, u8 mapNum);
u32 ChooseWildMonIndex_Land(void);
u32 ChooseWildMonIndex_Water(void);
u32 ChooseWildMonIndex_Rocks(void);
u32 ChooseHiddenMonIndex(void);
bool32 MapHasNoEncounterData(void);
enum TimeOfDay GetTimeOfDayForEncounters(u32 headerId, enum WildPokemonArea area);

// Pure ordinary-wild helpers. A supplied slot is always the authored table
// index (rather than an index relative to entryStart). Selection never consumes
// RNG; callers provide the roll within the eligible weight total. A slot is
// eligible when it holds a species.
bool8 GetWildEncounterProfileView(const struct WildEncounterProfileContext *context, struct WildEncounterProfileView *view);
bool8 GetWildEncounterProfileEntry(const struct WildEncounterProfileView *view, u8 slot, const struct WildPokemon **entry);
bool8 IsWildEncounterProfileSlotEligible(const struct WildEncounterProfileView *view, u8 slot);
u16 GetWildEncounterProfileEligibleWeight(const struct WildEncounterProfileView *view);
u16 GetWildEncounterProfileEffectiveWeight(const struct WildEncounterProfileView *view, u8 slot);
bool8 SelectWildEncounterProfileSlot(const struct WildEncounterProfileView *view, u16 roll, u8 *slot);
// Type-attraction chooses uniformly among matching eligible slots, unlike the
// normal weighted selection. A false return also represents the legacy
// no-op case where every eligible slot already matches the requested type.
bool8 SelectWildEncounterProfileTypeSlot(const struct WildEncounterProfileView *view, u8 type, u8 roll, u8 *slot);
// Lures reverse the eligible slot sequence after weighted selection. Keeping
// this deterministic helper in the core ensures an empty slot is never picked.
bool8 GetWildEncounterProfileMirroredEligibleSlot(const struct WildEncounterProfileView *view, u8 slot, u8 *mirroredSlot);
bool8 DoesWildEncounterProfileHaveAvailableEntries(const struct WildEncounterProfileView *view);

// The level of a place (a header) at a Trainer Rating: the reach scalers, or the
// dungeon's intent and floor. Never above MAX_LEVEL. Returns 0 for an unknown header.
u32 GetWildEncounterPlaceLevel(u32 headerId, u32 trainerRating);
// The rate a method rolls: Road maps roll 60% of the table's encounter rate for walking and
// surfing (WILD_AREA_LAND, WILD_AREA_WATER), rounded to the nearest value. Rock Smash,
// Headbutt and fishing keep the full rate.
u32 GetWildEncounterRateForHeader(u32 headerId, enum WildPokemonArea area, u32 encounterRate);
// A level for `species` in a place: the species' prowler minimum (none in an
// exempt region), then its young limit (below its lowest evolution level, babies
// 10; the prowler minimum wins again), then 1..MAX_LEVEL. Equal to the clamp every
// wild roll applies. An unknown header counts as an ordinary region.
u32 GetWildEncounterClampedLevel(u16 species, u32 headerId, s32 level);
// The level of a TV mass outbreak Pokémon here: the current place's level with the
// ordinary spread (Lure, Pressure, Hustle, Vital Spirit included), clamped like
// any wild level. The authored level on a map without a header. Consumes RNG.
u8 GetMassOutbreakLevel(void);
// The exact outcome distribution of one slot at a Trainer Rating: the place
// level and its -2..+2 spread, the prowler minimum, the young limit, the
// downward rule and the stage mix. RNG-free; fills up to
// WILD_ENCOUNTER_MAX_SLOT_OUTCOMES entries in spread order (an entry per spread
// step and stage, so equal species and levels may repeat) and returns the count,
// whose weights add up to WILD_ENCOUNTER_OUTCOME_DENOMINATOR. 0 for an empty slot.
u32 GetWildEncounterSlotOutcomes(const struct WildEncounterProfileView *view, u8 slot, u32 trainerRating, struct WildEncounterSlotOutcome *outcomes);
// The same distribution at the current Trainer Rating. When the wild
// randomizer is on it holds the raw slot species at the place level spread,
// since the randomizer picks the species afterwards.
u32 GetCurrentWildEncounterSlotOutcomes(const struct WildEncounterProfileView *view, u8 slot, struct WildEncounterSlotOutcome *outcomes);
// The species a slot most often produces at the current Trainer Rating (a
// species' weight is summed over its levels; the first of equals wins), for
// readers that name a local wild Pokémon without rolling. RNG-free.
u16 GetCurrentWildEncounterSlotLikelySpecies(const struct WildEncounterProfileView *view, u8 slot);
// Rolls one encounter of a slot from that distribution (the spread, Pressure,
// Hustle, Vital Spirit and Lures included). Consumes RNG.
bool8 RollWildEncounterSlot(const struct WildEncounterProfileView *view, u8 slot, struct WildEncounterSpeciesOutcome *outcome);

#if TESTING
u16 GenerateFeebasFishingWildMonForTesting(u8 rod);
#if IS_HNS
// Deterministic Hoenn Sound selection with ability attraction and Lures off.
// A failed radio override falls through to the supplied base-weight roll.
bool8 SelectWildEncounterProfileSlotWithHoennSoundForTesting(const struct WildEncounterProfileView *view, bool8 isHoennSoundPlaying, u8 activationRoll, u8 selectionRoll, u16 baseRoll, u8 *slot);
#endif
#if RANDOMIZER_AVAILABLE == TRUE
u16 RandomizeWildEncounterProfileEntryForTesting(const struct WildEncounterProfileView *view, u8 slot, u8 mapNum, u8 mapGroup, enum WildPokemonArea area);
#endif
#endif

#endif // GUARD_WILD_ENCOUNTER_H
