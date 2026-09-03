#include "global.h"
#include "event_data.h"
#include "overworld.h"
#include "regions.h"
#include "wayfarer_persistence.h"
#include "constants/maps.h"
#include "constants/opponents.h"
#if IS_WAYFARER
#include "data/map_group_count.h"
#include "data/wayfarer_map_sources.h"
#endif

#define HOENN_LEAGUE_GAME_CLEAR (1 << 0)
#define HOENN_LEAGUE_CHAMPION   (1 << 1)

#if IS_WAYFARER
static bool8 IsWayfarerCoreRegion(enum Region region)
{
    return region == REGION_JOHTO || region == REGION_KANTO || region == REGION_HOENN;
}
#endif

#if !IS_WAYFARER
static bool8 IsStandaloneProductRegion(enum Region region)
{
#if IS_HNS
    return region == REGION_JOHTO || region == REGION_KANTO;
#elif IS_FRLG
    return region == REGION_KANTO;
#else
    return region == REGION_HOENN;
#endif
}
#endif

#if IS_WAYFARER
static bool8 IsWayfarerMapHoennSource(s16 mapGroup, s16 mapNum)
{
    u16 sourceIndex;

    if (mapGroup < 0 || mapGroup >= MAP_GROUPS_COUNT
     || mapNum < 0 || mapNum >= MAP_GROUP_COUNT[mapGroup])
        return FALSE;

    sourceIndex = sWayfarerMapSourceOffsets[mapGroup] + mapNum;
    return (sWayfarerHoennMapSourceBits[sourceIndex / 8] >> (sourceIndex & 7)) & 1;
}

static enum Region GetWayfarerExplicitMapRegion(s16 mapGroup, s16 mapNum)
{
    u16 sourceIndex;

    if (mapGroup < 0 || mapGroup >= MAP_GROUPS_COUNT
     || mapNum < 0 || mapNum >= MAP_GROUP_COUNT[mapGroup])
        return REGION_NONE;

    sourceIndex = sWayfarerMapSourceOffsets[mapGroup] + mapNum;
    return (sWayfarerMapRegionNibbles[sourceIndex / 2] >> ((sourceIndex & 1) * 4)) & 0xF;
}

static enum Region GetHnsMapRegionOrFallback(s16 mapGroup, s16 mapNum, enum Region fallback)
{
    const struct MapHeader *mapHeader;
    enum Region explicitRegion;
    enum Region sectionRegion;

    if (fallback != REGION_KANTO)
        fallback = REGION_JOHTO;

    if (mapGroup < 0 || mapGroup >= MAP_GROUPS_COUNT
     || mapNum < 0 || mapNum >= MAP_GROUP_COUNT[mapGroup])
        return fallback;

    if (IsWayfarerMapHoennSource(mapGroup, mapNum))
        return REGION_HOENN;

    explicitRegion = GetWayfarerExplicitMapRegion(mapGroup, mapNum);
    if (explicitRegion > REGION_NONE && explicitRegion < REGIONS_COUNT)
        return explicitRegion;

    mapHeader = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    if (mapHeader == NULL)
        return fallback;

    sectionRegion = GetRegionForSectionId(mapHeader->regionMapSectionId);
    if (sectionRegion == REGION_JOHTO || sectionRegion == REGION_KANTO)
        return sectionRegion;

    // HNS has special areas whose visual map sections originated in Emerald,
    // including its Battle Frontier. Preserve the last HNS core region for
    // those maps, but never infer Hoenn from their presentation metadata.
    return fallback;
}

static enum Region GetRegionFromSavedMap(void)
{
    enum Region region = GetHnsMapRegionOrFallback(gSaveBlock1Ptr->location.mapGroup,
                                                   gSaveBlock1Ptr->location.mapNum,
                                                   REGION_JOHTO);

    return IsWayfarerCoreRegion(region) ? region : REGION_JOHTO;
}

static enum Region GetSavedHnsRegionContext(void)
{
    return gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext == REGION_KANTO
        ? REGION_KANTO
        : REGION_JOHTO;
}

static void SetHoennLeagueFlag(u8 mask, bool8 value)
{
    if (value)
        gSaveBlock3Ptr->wayfarerHoenn.leagueFlags |= mask;
    else
        gSaveBlock3Ptr->wayfarerHoenn.leagueFlags &= ~mask;
}
#endif

void WayfarerInitPersistentState(void)
{
#if IS_WAYFARER
    memset(&gSaveBlock3Ptr->wayfarerHoenn, 0, sizeof(gSaveBlock3Ptr->wayfarerHoenn));
    gSaveBlock3Ptr->wayfarerHoenn.magic = WAYFARER_HOENN_STATE_MAGIC;
    gSaveBlock3Ptr->wayfarerHoenn.currentRegion = REGION_JOHTO;
    gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext = REGION_JOHTO;
    gSaveBlock3Ptr->wayfarerHoenn.visitedRegions = 1 << REGION_JOHTO;
#endif
}

void WayfarerInitPersistentStateFromSavedMap(void)
{
#if IS_WAYFARER
    enum Region savedMapRegion = GetRegionFromSavedMap();
    WayfarerInitPersistentState();
    gSaveBlock3Ptr->wayfarerHoenn.currentRegion = savedMapRegion;
    if (savedMapRegion == REGION_KANTO || savedMapRegion == REGION_JOHTO)
        gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext = savedMapRegion;
    gSaveBlock3Ptr->wayfarerHoenn.visitedRegions |= 1 << savedMapRegion;
#endif
}

void WayfarerValidatePersistentState(void)
{
#if IS_WAYFARER
    enum Region savedMapRegion = GetRegionFromSavedMap();

    if (gSaveBlock3Ptr->wayfarerHoenn.magic != WAYFARER_HOENN_STATE_MAGIC)
    {
        WayfarerInitPersistentStateFromSavedMap();
        return;
    }

    if (gSaveBlock3Ptr->wayfarerHoenn.initialized > TRUE)
        gSaveBlock3Ptr->wayfarerHoenn.initialized = FALSE;

    if (!IsWayfarerCoreRegion(gSaveBlock3Ptr->wayfarerHoenn.currentRegion))
        gSaveBlock3Ptr->wayfarerHoenn.currentRegion = savedMapRegion;

    if (gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext != REGION_KANTO
     && gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext != REGION_JOHTO)
    {
        if (savedMapRegion == REGION_KANTO || savedMapRegion == REGION_JOHTO)
            gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext = savedMapRegion;
        else if (gSaveBlock3Ptr->wayfarerHoenn.currentRegion == REGION_KANTO)
            gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext = REGION_KANTO;
        else
            gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext = REGION_JOHTO;
    }

    gSaveBlock3Ptr->wayfarerHoenn.visitedRegions &=
        (1 << REGION_JOHTO) | (1 << REGION_KANTO) | (1 << REGION_HOENN);
    gSaveBlock3Ptr->wayfarerHoenn.visitedRegions |=
        1 << gSaveBlock3Ptr->wayfarerHoenn.currentRegion;
#endif
}

bool8 WayfarerHoennStateIsInitialized(void)
{
#if IS_WAYFARER
    return gSaveBlock3Ptr->wayfarerHoenn.initialized == TRUE;
#else
    return TRUE;
#endif
}

void WayfarerSetHoennStateInitialized(bool8 initialized)
{
#if IS_WAYFARER
    gSaveBlock3Ptr->wayfarerHoenn.initialized = initialized ? TRUE : FALSE;
#endif
}

enum Region WayfarerGetSavedCurrentRegion(void)
{
#if IS_WAYFARER
    enum Region region = gSaveBlock3Ptr->wayfarerHoenn.currentRegion;
    return IsWayfarerCoreRegion(region) ? region : REGION_JOHTO;
#else
    return GetCurrentRegion();
#endif
}

void WayfarerSetSavedCurrentRegion(enum Region region)
{
#if IS_WAYFARER
    if (!IsWayfarerCoreRegion(region))
        return;
    gSaveBlock3Ptr->wayfarerHoenn.currentRegion = region;
    if (region == REGION_KANTO || region == REGION_JOHTO)
        gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext = region;
    gSaveBlock3Ptr->wayfarerHoenn.visitedRegions |= 1 << region;
#endif
}

#if IS_WAYFARER
enum Region WayfarerGetRegionForMap(s16 mapGroup, s16 mapNum)
{
    return GetHnsMapRegionOrFallback(mapGroup, mapNum, GetSavedHnsRegionContext());
}

enum Region WayfarerGetCurrentMapRegion(void)
{
    return WayfarerGetRegionForMap(gSaveBlock1Ptr->location.mapGroup,
                                  gSaveBlock1Ptr->location.mapNum);
}

void WayfarerUpdateHnsRegionContextForMap(s16 mapGroup, s16 mapNum)
{
    enum Region explicitRegion;

    if (IsWayfarerMapHoennSource(mapGroup, mapNum))
    {
        WayfarerSetSavedCurrentRegion(REGION_HOENN);
        return;
    }

    explicitRegion = GetWayfarerExplicitMapRegion(mapGroup, mapNum);
    if (explicitRegion == REGION_JOHTO || explicitRegion == REGION_KANTO)
        WayfarerSetSavedCurrentRegion(explicitRegion);
}

bool8 WayfarerIsCurrentMapHoennSource(void)
{
    return IsWayfarerMapHoennSource(gSaveBlock1Ptr->location.mapGroup,
                                   gSaveBlock1Ptr->location.mapNum);
}

u16 WayfarerGetCurrentRegionForScript(void)
{
    return WayfarerGetCurrentMapRegion();
}

u16 WayfarerShouldWhiteOutToLavaridge(void)
{
    return WayfarerGetCurrentMapRegion() == REGION_HOENN
        && FlagGet(HOENN_FLAG_ID(WAYFARER_HOENN_WHITEOUT_TO_LAVARIDGE_FLAG));
}
#endif

static u16 GetFieldMoveFlagForCurrentSource(u16 flagId)
{
#if IS_WAYFARER
    if (WayfarerIsCurrentMapHoennSource())
    {
        if (flagId == FLAG_SYS_USE_FLASH)
            return HOENN_FLAG_ID(WAYFARER_HOENN_USE_FLASH_FLAG);
        if (flagId == FLAG_SYS_USE_STRENGTH)
            return HOENN_FLAG_ID(WAYFARER_HOENN_USE_STRENGTH_FLAG);
    }
#endif
    return flagId;
}

bool8 WayfarerFieldMoveFlagGet(u16 flagId)
{
    return FlagGet(GetFieldMoveFlagForCurrentSource(flagId));
}

void WayfarerFieldMoveFlagSet(u16 flagId)
{
    FlagSet(GetFieldMoveFlagForCurrentSource(flagId));
}

void WayfarerFieldMoveFlagClear(u16 flagId)
{
    FlagClear(flagId);
#if IS_WAYFARER
    if (flagId == FLAG_SYS_USE_FLASH)
        FlagClear(HOENN_FLAG_ID(WAYFARER_HOENN_USE_FLASH_FLAG));
    else if (flagId == FLAG_SYS_USE_STRENGTH)
        FlagClear(HOENN_FLAG_ID(WAYFARER_HOENN_USE_STRENGTH_FLAG));
#endif
}

bool8 GetBadgeStateForRegion(enum Region region, u8 badgeIndex)
{
    if (badgeIndex >= WAYFARER_HOENN_BADGE_COUNT)
        return FALSE;

#if IS_WAYFARER
    if (region == REGION_HOENN)
        return (gSaveBlock3Ptr->wayfarerHoenn.badges >> badgeIndex) & 1;
#endif

#if IS_HNS
    if (region == REGION_JOHTO)
        return FlagGet(FLAG_BADGE01_GET + badgeIndex);
    if (region == REGION_KANTO)
        return FlagGet(FLAG_BADGE09_GET + badgeIndex);
#elif IS_FRLG
    if (region == REGION_KANTO)
        return FlagGet(FLAG_BADGE01_GET + badgeIndex);
#else
    if (region == REGION_HOENN)
        return FlagGet(FLAG_BADGE01_GET + badgeIndex);
#endif
    return FALSE;
}

void SetBadgeStateForRegion(enum Region region, u8 badgeIndex, bool8 value)
{
    u16 flagId;

    if (badgeIndex >= WAYFARER_HOENN_BADGE_COUNT)
        return;

#if IS_WAYFARER
    if (region == REGION_HOENN)
    {
        if (value)
            gSaveBlock3Ptr->wayfarerHoenn.badges |= 1 << badgeIndex;
        else
            gSaveBlock3Ptr->wayfarerHoenn.badges &= ~(1 << badgeIndex);
        return;
    }
#endif

#if IS_HNS
    if (region == REGION_JOHTO)
        flagId = FLAG_BADGE01_GET + badgeIndex;
    else if (region == REGION_KANTO)
        flagId = FLAG_BADGE09_GET + badgeIndex;
    else
        return;
#elif IS_FRLG
    if (region != REGION_KANTO)
        return;
    flagId = FLAG_BADGE01_GET + badgeIndex;
#else
    if (region != REGION_HOENN)
        return;
    flagId = FLAG_BADGE01_GET + badgeIndex;
#endif

    if (value)
        FlagSet(flagId);
    else
        FlagClear(flagId);
}

u8 GetBadgeCountForRegion(enum Region region)
{
    u8 count = 0;
    u8 i;

    for (i = 0; i < WAYFARER_HOENN_BADGE_COUNT; i++)
    {
        if (GetBadgeStateForRegion(region, i))
            count++;
    }
    return count;
}

bool8 GetChampionStateForRegion(enum Region region)
{
#if IS_WAYFARER
    if (region == REGION_HOENN)
        return (gSaveBlock3Ptr->wayfarerHoenn.leagueFlags & HOENN_LEAGUE_CHAMPION) != 0;
#endif

#if IS_HNS
    if (region == REGION_JOHTO)
        return FlagGet(FLAG_IS_CHAMPION);
    if (region == REGION_KANTO)
        return FlagGet(FLAG_IS_KANTO_CHAMPION);
#elif IS_FRLG
    if (region == REGION_KANTO)
        return FlagGet(FLAG_IS_CHAMPION);
#else
    if (region == REGION_HOENN)
        return FlagGet(FLAG_IS_CHAMPION);
#endif
    return FALSE;
}

void SetChampionStateForRegion(enum Region region, bool8 value)
{
    u16 flagId;

#if IS_WAYFARER
    if (region == REGION_HOENN)
    {
        SetHoennLeagueFlag(HOENN_LEAGUE_CHAMPION, value);
        return;
    }
#endif

#if IS_HNS
    if (region == REGION_JOHTO)
        flagId = FLAG_IS_CHAMPION;
    else if (region == REGION_KANTO)
        flagId = FLAG_IS_KANTO_CHAMPION;
    else
        return;
#elif IS_FRLG
    if (region != REGION_KANTO)
        return;
    flagId = FLAG_IS_CHAMPION;
#else
    if (region != REGION_HOENN)
        return;
    flagId = FLAG_IS_CHAMPION;
#endif

    if (value)
        FlagSet(flagId);
    else
        FlagClear(flagId);
}

bool8 GetGameClearStateForRegion(enum Region region)
{
#if IS_WAYFARER
    if (region == REGION_HOENN)
        return (gSaveBlock3Ptr->wayfarerHoenn.leagueFlags & HOENN_LEAGUE_GAME_CLEAR) != 0;
    if (region == REGION_JOHTO)
        return FlagGet(FLAG_IS_CHAMPION);
    if (region == REGION_KANTO)
        return FlagGet(FLAG_IS_KANTO_CHAMPION);
    return FALSE;
#else
    if (IsStandaloneProductRegion(region))
        return FlagGet(FLAG_SYS_GAME_CLEAR);
    return FALSE;
#endif
}

void SetGameClearStateForRegion(enum Region region, bool8 value)
{
#if IS_WAYFARER
    if (region == REGION_HOENN)
    {
        SetHoennLeagueFlag(HOENN_LEAGUE_GAME_CLEAR, value);
        return;
    }
    if (region == REGION_JOHTO)
    {
        SetChampionStateForRegion(region, value);
        if (value)
            FlagSet(FLAG_SYS_GAME_CLEAR);
        return;
    }
    if (region == REGION_KANTO)
    {
        SetChampionStateForRegion(region, value);
        if (value)
            FlagSet(FLAG_SYS_GAME_CLEAR);
        return;
    }
#else
    if (IsStandaloneProductRegion(region))
    {
        if (value)
            FlagSet(FLAG_SYS_GAME_CLEAR);
        else
            FlagClear(FLAG_SYS_GAME_CLEAR);
    }
#endif
}

bool8 GetRegionVisitedState(enum Region region)
{
#if IS_WAYFARER
    if (region == REGION_JOHTO)
        return TRUE;
    if (region == REGION_KANTO)
        return FlagGet(FLAG_VISITED_KANTO);
    if (region == REGION_HOENN)
        return (gSaveBlock3Ptr->wayfarerHoenn.visitedRegions >> REGION_HOENN) & 1;
    return FALSE;
#elif IS_HNS
    if (region == REGION_JOHTO)
        return TRUE;
    if (region == REGION_KANTO)
        return FlagGet(FLAG_VISITED_KANTO);
    return FALSE;
#elif IS_FRLG
    return region == REGION_KANTO;
#else
    return region == REGION_HOENN;
#endif
}

void SetRegionVisitedState(enum Region region, bool8 value)
{
#if IS_WAYFARER
    if (region == REGION_KANTO)
    {
        if (value)
            FlagSet(FLAG_VISITED_KANTO);
        else
            FlagClear(FLAG_VISITED_KANTO);
    }
    else if (region == REGION_HOENN)
    {
        if (value)
            gSaveBlock3Ptr->wayfarerHoenn.visitedRegions |= 1 << REGION_HOENN;
        else
            gSaveBlock3Ptr->wayfarerHoenn.visitedRegions &= ~(1 << REGION_HOENN);
    }
#elif IS_HNS
    if (region == REGION_KANTO)
    {
        if (value)
            FlagSet(FLAG_VISITED_KANTO);
        else
            FlagClear(FLAG_VISITED_KANTO);
    }
#endif
}

bool8 GetLocationVisitedStateForRegion(enum Region region, u16 flagId)
{
#if IS_WAYFARER
    if (IS_HOENN_FLAG_ID(flagId) && region != REGION_HOENN)
        return FALSE;
    if (region == REGION_HOENN)
    {
        u16 sourceId;
        if (!IS_HOENN_FLAG_ID(flagId))
            return FALSE;
        sourceId = HOENN_FLAG_SOURCE_ID(flagId);
        if (sourceId < WAYFARER_HOENN_VISITED_FLAG_START
         || sourceId >= WAYFARER_HOENN_VISITED_FLAG_START + WAYFARER_HOENN_VISITED_COUNT)
            return FALSE;
        return FlagGet(flagId);
    }
#endif
    return flagId != 0 && FlagGet(flagId);
}

void SetLocationVisitedStateForRegion(enum Region region, u16 flagId, bool8 value)
{
#if IS_WAYFARER
    if (IS_HOENN_FLAG_ID(flagId) && region != REGION_HOENN)
        return;
    if (region == REGION_HOENN)
    {
        u16 sourceId;
        if (!IS_HOENN_FLAG_ID(flagId))
            return;
        sourceId = HOENN_FLAG_SOURCE_ID(flagId);
        if (sourceId < WAYFARER_HOENN_VISITED_FLAG_START
         || sourceId >= WAYFARER_HOENN_VISITED_FLAG_START + WAYFARER_HOENN_VISITED_COUNT)
            return;
        if (value)
            SetRegionVisitedState(region, TRUE);
    }
#endif

    if (flagId == 0)
        return;
    if (value)
        FlagSet(flagId);
    else
        FlagClear(flagId);
}

bool8 WayfarerHoennTrainerFlagGet(u16 mappedTrainerId)
{
#if IS_WAYFARER
    u16 sourceTrainerId;
    if (mappedTrainerId <= WAYFARER_HOENN_TRAINER_OFFSET
     || mappedTrainerId >= WAYFARER_HOENN_TRAINER_OFFSET + WAYFARER_HOENN_TRAINERS_COUNT)
        return FALSE;
    sourceTrainerId = mappedTrainerId - WAYFARER_HOENN_TRAINER_OFFSET;
    return (gSaveBlock3Ptr->wayfarerHoenn.trainerFlags[sourceTrainerId / 8] >> (sourceTrainerId & 7)) & 1;
#else
    (void)mappedTrainerId;
    return FALSE;
#endif
}

void WayfarerHoennTrainerFlagSet(u16 mappedTrainerId)
{
#if IS_WAYFARER
    u16 sourceTrainerId;
    if (mappedTrainerId <= WAYFARER_HOENN_TRAINER_OFFSET
     || mappedTrainerId >= WAYFARER_HOENN_TRAINER_OFFSET + WAYFARER_HOENN_TRAINERS_COUNT)
        return;
    sourceTrainerId = mappedTrainerId - WAYFARER_HOENN_TRAINER_OFFSET;
    gSaveBlock3Ptr->wayfarerHoenn.trainerFlags[sourceTrainerId / 8] |= 1 << (sourceTrainerId & 7);
#else
    (void)mappedTrainerId;
#endif
}

void WayfarerHoennTrainerFlagClear(u16 mappedTrainerId)
{
#if IS_WAYFARER
    u16 sourceTrainerId;
    if (mappedTrainerId <= WAYFARER_HOENN_TRAINER_OFFSET
     || mappedTrainerId >= WAYFARER_HOENN_TRAINER_OFFSET + WAYFARER_HOENN_TRAINERS_COUNT)
        return;
    sourceTrainerId = mappedTrainerId - WAYFARER_HOENN_TRAINER_OFFSET;
    gSaveBlock3Ptr->wayfarerHoenn.trainerFlags[sourceTrainerId / 8] &= ~(1 << (sourceTrainerId & 7));
#else
    (void)mappedTrainerId;
#endif
}
