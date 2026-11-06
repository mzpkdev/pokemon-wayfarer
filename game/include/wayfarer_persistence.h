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
