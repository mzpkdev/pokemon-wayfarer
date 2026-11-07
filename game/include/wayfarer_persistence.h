#ifndef GUARD_WAYFARER_PERSISTENCE_H
#define GUARD_WAYFARER_PERSISTENCE_H

#include "global.h"
#include "constants/regions.h"

void WayfarerInitPersistentState(void);
bool8 WayfarerPersistentStateIsValid(void);
void WayfarerValidatePersistentState(void);
void WayfarerSeviiInitPersistentState(void);
bool8 WayfarerSeviiPersistentStateIsValid(void);
struct WayfarerSeviiTrainerTowerRecords *WayfarerSevii_GetTrainerTowerRecords(void);
bool32 WayfarerSeviiTrainerDefeatGet(u16 slot);
void WayfarerSeviiTrainerDefeatSet(u16 slot);
void WayfarerSeviiTrainerDefeatClear(u16 slot);

bool8 WayfarerHoennStateIsInitialized(void);
void WayfarerSetHoennStateInitialized(bool8 initialized);

enum Region WayfarerGetSavedCurrentRegion(void);
void WayfarerSetSavedCurrentRegion(enum Region region);
#if IS_WAYFARER
// Emerald FLAG_RECEIVED_HM_DIVE (source id 0x07B) is the dedicated
// authorization set by Steven's Hoenn reward script.
#define WAYFARER_HOENN_DIVE_AUTHORIZATION_FLAG HOENN_FLAG_ID(0x07B)
// Emerald's own ids for what Hoenn's scripts write: the engine build has no such names.
#define WAYFARER_HOENN_MT_CHIMNEY_DONE_FLAG    HOENN_FLAG_ID(0x08B) // FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY
#define WAYFARER_HOENN_MT_PYRE_ORB_FLAG        HOENN_FLAG_ID(0x0D4) // FLAG_RECEIVED_RED_OR_BLUE_ORB
#define WAYFARER_HOENN_SKY_PILLAR_STATE_VAR    HOENN_VAR_ID(0x40CA) // VAR_SKY_PILLAR_STATE

// Misc Hoenn state (secret bases, daycare, department store, cycling road, Sootopolis, ribbons, landmarks).
// Emerald ids (SYSTEM_FLAGS is 0x860 there); the Hoenn map scripts and common scripts on Hoenn maps use these copies.
#define WAYFARER_HOENN_SYS_FLAG(offset)        HOENN_FLAG_ID(0x860 + (offset))
#define WAYFARER_HOENN_CURRENT_SECRET_BASE_VAR HOENN_VAR_ID(0x4054)       // VAR_CURRENT_SECRET_BASE
#define WAYFARER_HOENN_INIT_SECRET_BASE_VAR    HOENN_VAR_ID(0x4097)       // VAR_INIT_SECRET_BASE
#define WAYFARER_HOENN_SECRET_BASE_INITIALIZED_VAR HOENN_VAR_ID(0x4089)   // VAR_SECRET_BASE_INITIALIZED
#define WAYFARER_HOENN_SECRET_BASE_REGISTRY_FLAG HOENN_FLAG_ID(0x10C)     // FLAG_SECRET_BASE_REGISTRY_ENABLED
#define WAYFARER_HOENN_RECEIVED_SECRET_POWER_FLAG HOENN_FLAG_ID(0x60)     // FLAG_RECEIVED_SECRET_POWER
#define WAYFARER_HOENN_PENDING_DAYCARE_EGG_FLAG HOENN_FLAG_ID(0x86)       // FLAG_PENDING_DAYCARE_EGG
#define WAYFARER_HOENN_DEPT_STORE_FLOOR_VAR    HOENN_VAR_ID(0x4043)       // VAR_DEPT_STORE_FLOOR
#define WAYFARER_HOENN_CYCLING_CHALLENGE_VAR   HOENN_VAR_ID(0x40A9)       // VAR_CYCLING_CHALLENGE_STATE
#define WAYFARER_HOENN_CYCLING_ROAD_FLAG       WAYFARER_HOENN_SYS_FLAG(0x2B) // FLAG_SYS_CYCLING_ROAD
#define WAYFARER_HOENN_LEGENDARIES_IN_SOOTOPOLIS_FLAG HOENN_FLAG_ID(0x53) // FLAG_LEGENDARIES_IN_SOOTOPOLIS
#define WAYFARER_HOENN_RIBBON_GET_FLAG         WAYFARER_HOENN_SYS_FLAG(0x3B) // FLAG_SYS_RIBBON_GET
// FLAG_DECORATION_1..14: the object events of the secret base maps carry these Hoenn copies as their flags.
#define WAYFARER_HOENN_DECORATION_1_FLAG       HOENN_FLAG_ID(0xAE)
#define WAYFARER_HOENN_DECORATION_14_FLAG      HOENN_FLAG_ID(0xBB)
// The decoration object flags of the current map: HOENN's player room and secret bases carry the Hoenn
// copies, while the Johto player's room in New Bark carries the engine FLAG_DECORATION_* ids.
u16 WayfarerDecorationFlag(u8 index);
// FLAG_LANDMARK_*: the Hoenn scripts set these on entering the place.
#define WAYFARER_HOENN_LANDMARK_FLOWER_SHOP_FLAG        WAYFARER_HOENN_SYS_FLAG(0x3C)
#define WAYFARER_HOENN_LANDMARK_MR_BRINEY_HOUSE_FLAG    WAYFARER_HOENN_SYS_FLAG(0x3D)
#define WAYFARER_HOENN_LANDMARK_ABANDONED_SHIP_FLAG     WAYFARER_HOENN_SYS_FLAG(0x3E)
#define WAYFARER_HOENN_LANDMARK_SEASHORE_HOUSE_FLAG     WAYFARER_HOENN_SYS_FLAG(0x3F)
#define WAYFARER_HOENN_LANDMARK_NEW_MAUVILLE_FLAG       WAYFARER_HOENN_SYS_FLAG(0x40)
#define WAYFARER_HOENN_LANDMARK_OLD_LADY_REST_SHOP_FLAG WAYFARER_HOENN_SYS_FLAG(0x41)
#define WAYFARER_HOENN_LANDMARK_TRICK_HOUSE_FLAG        WAYFARER_HOENN_SYS_FLAG(0x42)
#define WAYFARER_HOENN_LANDMARK_WINSTRATE_FAMILY_FLAG   WAYFARER_HOENN_SYS_FLAG(0x43)
#define WAYFARER_HOENN_LANDMARK_GLASS_WORKSHOP_FLAG     WAYFARER_HOENN_SYS_FLAG(0x44)
#define WAYFARER_HOENN_LANDMARK_LANETTES_HOUSE_FLAG     WAYFARER_HOENN_SYS_FLAG(0x45)
#define WAYFARER_HOENN_LANDMARK_POKEMON_DAYCARE_FLAG    WAYFARER_HOENN_SYS_FLAG(0x46)
#define WAYFARER_HOENN_LANDMARK_SEAFLOOR_CAVERN_FLAG    WAYFARER_HOENN_SYS_FLAG(0x47)
#define WAYFARER_HOENN_LANDMARK_FIERY_PATH_FLAG         WAYFARER_HOENN_SYS_FLAG(0x4A)
#define WAYFARER_HOENN_LANDMARK_ISLAND_CAVE_FLAG        WAYFARER_HOENN_SYS_FLAG(0x55)
#define WAYFARER_HOENN_LANDMARK_DESERT_RUINS_FLAG       WAYFARER_HOENN_SYS_FLAG(0x56)
#define WAYFARER_HOENN_LANDMARK_FOSSIL_MANIACS_HOUSE_FLAG WAYFARER_HOENN_SYS_FLAG(0x57)
#define WAYFARER_HOENN_LANDMARK_SCORCHED_SLAB_FLAG      WAYFARER_HOENN_SYS_FLAG(0x58)
#define WAYFARER_HOENN_LANDMARK_ANCIENT_TOMB_FLAG       WAYFARER_HOENN_SYS_FLAG(0x59)
#define WAYFARER_HOENN_LANDMARK_TUNNELERS_REST_HOUSE_FLAG WAYFARER_HOENN_SYS_FLAG(0x5A)
#define WAYFARER_HOENN_LANDMARK_HUNTERS_HOUSE_FLAG      WAYFARER_HOENN_SYS_FLAG(0x5B)
#define WAYFARER_HOENN_LANDMARK_SEALED_CHAMBER_FLAG     WAYFARER_HOENN_SYS_FLAG(0x5C)
#define WAYFARER_HOENN_LANDMARK_SKY_PILLAR_FLAG         WAYFARER_HOENN_SYS_FLAG(0x5E)
#define WAYFARER_HOENN_LANDMARK_BERRY_MASTERS_HOUSE_FLAG WAYFARER_HOENN_SYS_FLAG(0x63)
#define WAYFARER_HOENN_LANDMARK_MIRAGE_TOWER_FLAG       HOENN_FLAG_ID(0x78)
#define WAYFARER_HOENN_LANDMARK_ALTERING_CAVE_FLAG      WAYFARER_HOENN_SYS_FLAG(0x7D)
#define WAYFARER_HOENN_LANDMARK_DESERT_UNDERPASS_FLAG   WAYFARER_HOENN_SYS_FLAG(0x7E)
#define WAYFARER_HOENN_LANDMARK_ARTISAN_CAVE_FLAG       WAYFARER_HOENN_SYS_FLAG(0x7F)
#define WAYFARER_HOENN_LANDMARK_TRAINER_HILL_FLAG       WAYFARER_HOENN_SYS_FLAG(0x82)

enum WayfarerDiveMapContext
{
    WAYFARER_DIVE_MAP_UNKNOWN,
    WAYFARER_DIVE_MAP_HNS,
    WAYFARER_DIVE_MAP_HOENN,
};

enum Region WayfarerGetRegionForMap(s16 mapGroup, s16 mapNum);
enum Region WayfarerGetCurrentMapRegion(void);
void WayfarerUpdateHnsRegionContextForMap(s16 mapGroup, s16 mapNum);
bool8 WayfarerIsMapHoennSource(s16 mapGroup, s16 mapNum);
bool8 WayfarerIsCurrentMapHoennSource(void);
enum WayfarerDiveMapContext WayfarerGetDiveMapContext(s16 mapGroup, s16 mapNum);
bool8 WayfarerIsDiveAuthorizedForMap(s16 mapGroup, s16 mapNum);
bool8 WayfarerIsDiveAuthorizedForCurrentMap(void);
u16 WayfarerGetCurrentRegionForScript(void);
u16 WayfarerGetKantoBadgeCountForScript(void);
u16 WayfarerGetHoennBadgeCountForScript(void);
u16 WayfarerIsCurrentRegionChampionForScript(void);
u16 WayfarerShouldWhiteOutToLavaridge(void);
bool8 WayfarerInitializeHoennState(s16 mapGroup, s16 mapNum, s16 x, s16 y);
bool8 WayfarerPrepareHoennEntry(s16 mapGroup, s16 mapNum, s16 x, s16 y, u8 healLocationId);
u16 WayfarerCoast_IsArticunoEligible(void);
u16 WayfarerPowerPlant_IsZapdosEligible(void);
u16 WayfarerHoenn_IsLateLegendaryReady(void);
u16 WayfarerHoenn_IsMtPyreLead(void);
void WayfarerGrantKantoBadge15(void);
#endif

bool8 WayfarerFieldMoveFlagGet(u16 flagId);
void WayfarerFieldMoveFlagSet(u16 flagId);
void WayfarerFieldMoveFlagClear(u16 flagId);

u8 GetBadgeCountForRegion(enum Region region);
bool8 GetBadgeStateForRegion(enum Region region, u8 badgeIndex);
void SetBadgeStateForRegion(enum Region region, u8 badgeIndex, bool8 value);

bool8 GetChampionStateForRegion(enum Region region);
void SetChampionStateForRegion(enum Region region, bool8 value);
bool8 GetGameClearStateForRegion(enum Region region);
void SetGameClearStateForRegion(enum Region region, bool8 value);

bool8 GetRegionVisitedState(enum Region region);
void SetRegionVisitedState(enum Region region, bool8 value);
bool8 GetLocationVisitedStateForRegion(enum Region region, u16 flagId);
void SetLocationVisitedStateForRegion(enum Region region, u16 flagId, bool8 value);

bool8 WayfarerHoennTrainerFlagGet(u16 mappedTrainerId);
void WayfarerHoennTrainerFlagSet(u16 mappedTrainerId);
void WayfarerHoennTrainerFlagClear(u16 mappedTrainerId);

#endif // GUARD_WAYFARER_PERSISTENCE_H
