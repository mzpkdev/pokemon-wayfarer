#ifndef GUARD_WAYFARER_PERSISTENCE_H
#define GUARD_WAYFARER_PERSISTENCE_H

#include "global.h"
#include "constants/regions.h"

void WayfarerInitPersistentState(void);
void WayfarerInitPersistentStateFromSavedMap(void);
void WayfarerValidatePersistentState(void);

bool8 WayfarerHoennStateIsInitialized(void);
void WayfarerSetHoennStateInitialized(bool8 initialized);

enum Region WayfarerGetSavedCurrentRegion(void);
void WayfarerSetSavedCurrentRegion(enum Region region);
#if IS_WAYFARER
// Emerald FLAG_RECEIVED_HM_DIVE (source id 0x07B) is the dedicated
// authorization set by Steven's Hoenn reward script.
#define WAYFARER_HOENN_DIVE_AUTHORIZATION_FLAG HOENN_FLAG_ID(0x07B)

enum WayfarerDiveMapContext
{
    WAYFARER_DIVE_MAP_UNKNOWN,
    WAYFARER_DIVE_MAP_HNS,
    WAYFARER_DIVE_MAP_HOENN,
};

enum Region WayfarerGetRegionForMap(s16 mapGroup, s16 mapNum);
enum Region WayfarerGetCurrentMapRegion(void);
void WayfarerUpdateHnsRegionContextForMap(s16 mapGroup, s16 mapNum);
bool8 WayfarerIsCurrentMapHoennSource(void);
enum WayfarerDiveMapContext WayfarerGetDiveMapContext(s16 mapGroup, s16 mapNum);
bool8 WayfarerIsDiveAuthorizedForMap(s16 mapGroup, s16 mapNum);
bool8 WayfarerIsDiveAuthorizedForCurrentMap(void);
u16 WayfarerGetCurrentRegionForScript(void);
u16 WayfarerGetHoennBadgeCountForScript(void);
u16 WayfarerShouldWhiteOutToLavaridge(void);
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
