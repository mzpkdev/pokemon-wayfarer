#ifndef GUARD_WILD_ENCOUNTER_H
#define GUARD_WILD_ENCOUNTER_H

#include "rtc.h"
#include "config/randomizer.h"
#include "constants/wild_encounter.h"

#define HEADER_NONE 0xFFFF

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

// The authored tables remain the source of encounter composition. These generated
// records are the small, shared authority used to project an authored level from
// a Trainer Rating without flattening a location's species or slot weights.
struct WildEncounterScalingConfig
{
    u8 projectionCap;
};

struct WildEncounterScalingAnchor
{
    u8 rating;
    u8 level;
};

struct WildEncounterScalingPoint
{
    u8 anchorLevel;
    u16 retentionNumerator;
    u16 retentionDenominator;
};

enum WildEncounterFishingRod
{
    WILD_ENCOUNTER_FISHING_ROD_OLD,
    WILD_ENCOUNTER_FISHING_ROD_GOOD,
    WILD_ENCOUNTER_FISHING_ROD_SUPER,
    WILD_ENCOUNTER_FISHING_ROD_NONE,
};

struct WildEncounterProfileOffset
{
    u16 headerId;
    u8 area;
    u8 timeOfDay;
    u8 fishingRod;
    s8 levelOffset;
};

struct WildEncounterSpeciesMetadata
{
    u16 species;
    u8 minimumLevel;
    u16 predecessorSpecies;
    u8 predecessorLevel;
    bool8 hasAlternateNonLevelRoute;
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


extern const struct WildPokemonHeader gWildMonHeaders[];
extern const struct WildEncounterScalingConfig gWildEncounterScalingConfig;
extern const struct WildEncounterScalingAnchor gWildEncounterScalingAnchors[];
extern const u16 gWildEncounterScalingAnchorCount;
extern const struct WildEncounterScalingPoint gWildEncounterScalingPoints[];
extern const u16 gWildEncounterScalingPointCount;
extern const struct WildEncounterProfileOffset gWildEncounterProfileOffsets[];
extern const u16 gWildEncounterProfileOffsetCount;
extern const struct WildEncounterSpeciesMetadata gWildEncounterSpeciesMetadata[];
extern const u16 gWildEncounterSpeciesMetadataCount;
extern const u8 gStandardRodFishingWeights[WILD_ENCOUNTER_FISHING_ROD_NONE][FISH_WILD_COUNT];
extern bool8 gIsFishingEncounter;
extern bool8 gIsSurfingEncounter;
extern u8 gChainFishingDexNavStreak;

void DisableWildEncounters(bool8 disabled);
bool8 StandardWildEncounter(u16 curMetatileBehavior, u16 prevMetatileBehavior);
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
u32 ChooseWildMonIndex_Land(void);
u32 ChooseWildMonIndex_Water(void);
u32 ChooseWildMonIndex_Rocks(void);
u32 ChooseHiddenMonIndex(void);
bool32 MapHasNoEncounterData(void);
enum TimeOfDay GetTimeOfDayForEncounters(u32 headerId, enum WildPokemonArea area);

// Pure ordinary-wild scaling helpers. A supplied slot is always the authored
// table index (rather than an index relative to entryStart). Selection never
// consumes RNG; callers provide both the randomizer policy and a roll within
// the eligible weight total. A true isWildRandomized policy preserves raw
// species/weights for RandomizeWildEncounter to map in its established order.
bool8 GetWildEncounterProfileView(const struct WildEncounterProfileContext *context, struct WildEncounterProfileView *view);
bool8 GetWildEncounterProfileEntry(const struct WildEncounterProfileView *view, u8 slot, const struct WildPokemon **entry);
u8 ProjectWildEncounterLevelWithOffset(u8 authoredLevel, u16 trainerRating, s8 levelOffset);
u8 ProjectWildEncounterLevel(const struct WildEncounterProfileView *view, u8 authoredLevel, u16 trainerRating);
bool8 GetWildEncounterSpeciesOutcome(const struct WildEncounterProfileView *view, u8 slot, u8 authoredLevel, u16 trainerRating, bool8 isWildRandomized, struct WildEncounterSpeciesOutcome *outcome);
bool8 GetCurrentWildEncounterSpeciesOutcome(const struct WildEncounterProfileView *view, u8 slot, u8 authoredLevel, struct WildEncounterSpeciesOutcome *outcome);
bool8 IsWildEncounterProfileSlotEligible(const struct WildEncounterProfileView *view, u8 slot, u16 trainerRating, bool8 isWildRandomized);
u16 GetWildEncounterProfileEligibleWeight(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized);
u16 GetWildEncounterProfileEffectiveWeight(const struct WildEncounterProfileView *view, u8 slot, u16 trainerRating, bool8 isWildRandomized);
bool8 SelectWildEncounterProfileSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u16 roll, u8 *slot);
// Type-attraction chooses uniformly among matching eligible slots, unlike the
// normal weighted selection. A false return also represents the legacy
// no-op case where every eligible slot already matches the requested type.
bool8 SelectWildEncounterProfileTypeSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u8 type, u8 roll, u8 *slot);
// Lures reverse the eligible slot sequence after weighted selection. Keeping
// this deterministic helper in the core ensures filtered profiles cannot map
// an otherwise valid selection back onto a locked authored slot.
bool8 GetWildEncounterProfileMirroredEligibleSlot(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized, u8 slot, u8 *mirroredSlot);
bool8 DoesWildEncounterProfileHaveAvailableEntries(const struct WildEncounterProfileView *view, u16 trainerRating, bool8 isWildRandomized);
bool8 IsCurrentWildEncounterProfileSlotEligible(const struct WildEncounterProfileView *view, u8 slot);
u16 GetCurrentWildEncounterProfileEligibleWeight(const struct WildEncounterProfileView *view);
u16 GetCurrentWildEncounterProfileEffectiveWeight(const struct WildEncounterProfileView *view, u8 slot);
bool8 SelectCurrentWildEncounterProfileSlot(const struct WildEncounterProfileView *view, u16 roll, u8 *slot);

#if TESTING
u16 GenerateFeebasFishingWildMonForTesting(u8 rod);
#if RANDOMIZER_AVAILABLE == TRUE
u16 RandomizeWildEncounterProfileEntryForTesting(const struct WildEncounterProfileView *view, u8 slot, u8 mapNum, u8 mapGroup, enum WildPokemonArea area);
#endif
#endif

// Live ordinary encounters apply their legacy lure, ability, and HNS Hoenn
// Sound semantics over these eligible populations. The pure helpers keep
// those selection policies deterministic and independently testable.

#endif // GUARD_WILD_ENCOUNTER_H
