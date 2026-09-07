#include "global.h"
#include "event_data.h"
#include "heal_location.h"
#include "load_save.h"
#include "constants/heal_locations.h"
#include "overworld.h"
#include "config/league_circuit.h"
#include "constants/maps.h"
#include "constants/vars.h"
#include "league_circuit.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"

#if IS_WAYFARER
EWRAM_DATA static enum Region sRecordedLeagueClearRegion = REGION_NONE;
EWRAM_DATA static bool8 sLeagueRunLoadRecoveryPending = FALSE;

// Hoenn source ids are explicitly banked; HNS aliases for these flags are zero.
#define LEAGUE_HOENN_STATE HOENN_VAR_ID(0x409C)
#define LEAGUE_HOENN_FIRST_DEFEAT HOENN_FLAG_ID(0x4FB)

static const u16 sIndigoRooms[] = {
    MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS, MAP_POKEMON_LEAGUE_KOGAS_ROOM_HNS,
    MAP_POKEMON_LEAGUE_BRUNOS_ROOM_HNS, MAP_POKEMON_LEAGUE_KARENS_ROOM_HNS,
    MAP_POKEMON_LEAGUE_CHAMPIONS_ROOM_HNS, MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS,
};
static const u16 sHoennRooms[] = {
    MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM, MAP_EVER_GRANDE_CITY_PHOEBES_ROOM,
    MAP_EVER_GRANDE_CITY_GLACIAS_ROOM, MAP_EVER_GRANDE_CITY_DRAKES_ROOM,
    MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM, MAP_EVER_GRANDE_CITY_HALL_OF_FAME,
};
static const u16 sHoennHalls[] = {
    MAP_EVER_GRANDE_CITY_HALL1, MAP_EVER_GRANDE_CITY_HALL2,
    MAP_EVER_GRANDE_CITY_HALL3, MAP_EVER_GRANDE_CITY_HALL4,
};

static bool8 WarpIsMap(const struct WarpData *warp, u16 map)
{
    return warp->mapGroup == MAP_GROUP(map) && warp->mapNum == MAP_NUM(map);
}

static s8 FindRoom(const struct WarpData *warp, const u16 *rooms, u8 count)
{
    u8 i;
    for (i = 0; i < count; i++)
        if (WarpIsMap(warp, rooms[i]))
            return i;
    return -1;
}

static enum Region GetVenue(const struct WarpData *warp)
{
    if (FindRoom(warp, sIndigoRooms, ARRAY_COUNT(sIndigoRooms)) >= 0)
        return REGION_KANTO;
    if (FindRoom(warp, sHoennRooms, ARRAY_COUNT(sHoennRooms)) >= 0
     || FindRoom(warp, sHoennHalls, ARRAY_COUNT(sHoennHalls)) >= 0)
        return REGION_HOENN;
    return REGION_NONE;
}

static void ResetLeagueProgress(enum Region region)
{
    u8 i;
    if (region == REGION_KANTO || region == REGION_JOHTO)
        VarSet(VAR_LEAGUE_STATE, 1);
    else if (region == REGION_HOENN)
    {
        VarSet(LEAGUE_HOENN_STATE, 0);
        for (i = 0; i < 4; i++)
            FlagClear(LEAGUE_HOENN_FIRST_DEFEAT + i);
    }
}

void EndLeagueRun(void)
{
    ResetLeagueProgress(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.region);
    memset(&gSaveBlock3Ptr->wayfarerHoenn.leagueRun, 0,
           sizeof(gSaveBlock3Ptr->wayfarerHoenn.leagueRun));
}

enum Region GetActiveLeagueRunRegion(void)
{
    const struct LeagueRunState *run = &gSaveBlock3Ptr->wayfarerHoenn.leagueRun;
    if (run->active != TRUE || run->ratingAtEntry > 80
     || run->region < REGION_KANTO || run->region > REGION_HOENN
     || run->region != GetRequiredLeagueRegion())
        return REGION_NONE;
    return run->region;
}

// A prefix of defeated rooms is valid; a later flag without its predecessor is not.
static s8 GetHoennDefeatedCount(void)
{
    u8 i, count = 0;
    for (i = 0; i < 4; i++)
    {
        if (FlagGet(LEAGUE_HOENN_FIRST_DEFEAT + i))
        {
            if (i != count)
                return -1;
            count++;
        }
    }
    if (VarGet(LEAGUE_HOENN_STATE) == 5)
        return count == 4 ? 5 : -1;
    if (VarGet(LEAGUE_HOENN_STATE) > 5)
        return -1;
    return count;
}

static bool8 ValidateLeagueLocation(const struct WarpData *location)
{
    enum Region region = GetActiveLeagueRunRegion();
    s8 room, defeated, hall;
    u16 state;
    if (region == REGION_NONE)
        return FALSE;
    if (region != REGION_HOENN)
    {
        room = FindRoom(location, sIndigoRooms, ARRAY_COUNT(sIndigoRooms));
        state = VarGet(VAR_LEAGUE_STATE);
        if (room < 0 || state < 1 || state > 6)
            return FALSE;
        defeated = state - 1;
    }
    else
    {
        room = FindRoom(location, sHoennRooms, ARRAY_COUNT(sHoennRooms));
        defeated = GetHoennDefeatedCount();
        if (defeated < 0)
            return FALSE;
        state = VarGet(LEAGUE_HOENN_STATE);
        if (room < 0)
        {
            hall = FindRoom(location, sHoennHalls, ARRAY_COUNT(sHoennHalls));
            return hall >= 0 && defeated == hall + 1 && state == hall + 1;
        }
        if (room < 4 && (state != room && state != room + 1))
            return FALSE;
        if (room < 4 && defeated == room + 1 && state != room + 1)
            return FALSE;
        if (room >= 4 && state != 4 && state != 5)
            return FALSE;
    }
    // A player may still stand in a just-defeated room, but cannot skip a room.
    return room == 5 ? defeated == 5 : defeated == room || defeated == room + 1;
}

bool8 ValidateActiveLeagueRun(void)
{
    return ValidateLeagueLocation(&gSaveBlock1Ptr->location);
}

bool32 GetLeagueRunBattleRating(u32 region, u32 encounterIndex, u8 *rating)
{
    const u16 *rooms = region == REGION_HOENN ? sHoennRooms : sIndigoRooms;
    s8 defeated = region == REGION_HOENN ? GetHoennDefeatedCount() : VarGet(VAR_LEAGUE_STATE) - 1;
    if (encounterIndex >= 5 || GetActiveLeagueRunRegion() != region
     || !ValidateActiveLeagueRun() || defeated != encounterIndex
     || !WarpIsMap(&gSaveBlock1Ptr->location, rooms[encounterIndex]))
        return FALSE;
    *rating = gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry;
    return TRUE;
}

bool8 IsCurrentLeagueRoomDefeated(void)
{
    enum Region region = GetActiveLeagueRunRegion();
    s8 room = FindRoom(&gSaveBlock1Ptr->location,
                      region == REGION_HOENN ? sHoennRooms : sIndigoRooms, 5);
    s8 defeated = region == REGION_HOENN ? GetHoennDefeatedCount() : VarGet(VAR_LEAGUE_STATE) - 1;
    return ValidateActiveLeagueRun() && room >= 0 && defeated == room + 1;
}

bool8 MarkLeagueChampionDefeated(void)
{
    u8 rating;
    if (!GetLeagueRunBattleRating(REGION_HOENN, 4, &rating))
        return FALSE;
    VarSet(LEAGUE_HOENN_STATE, 5);
    return TRUE;
}

static bool8 IsLeagueRunComplete(void)
{
    enum Region region = GetActiveLeagueRunRegion();
    return region == REGION_HOENN
        ? WarpIsMap(&gSaveBlock1Ptr->location, sHoennRooms[5]) && GetHoennDefeatedCount() == 5
        : WarpIsMap(&gSaveBlock1Ptr->location, sIndigoRooms[5]) && VarGet(VAR_LEAGUE_STATE) == 6;
}

static void ReturnToLobby(struct WarpData *location, enum Region venue)
{
    const struct MapHeader *header;
    u16 map = venue == REGION_HOENN ? MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F
                                    : MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS;
    EndLeagueRun();
    ResetLeagueProgress(venue);
    location->mapGroup = MAP_GROUP(map);
    location->mapNum = MAP_NUM(map);
    location->warpId = 0;
    header = Overworld_GetMapHeaderByGroupAndId(location->mapGroup, location->mapNum);
    location->x = header->events->warps[0].x;
    location->y = header->events->warps[0].y;
}

void LeagueRunHandleWarp(const struct WarpData *source, struct WarpData *destination)
{
#if WAYFARER_LEAGUE_CIRCUIT_ENABLED
    enum Region venue = GetVenue(destination);
    enum Region region;
    bool8 admission = (WarpIsMap(source, MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS)
                      && WarpIsMap(destination, sIndigoRooms[0]))
                  || (WarpIsMap(source, MAP_EVER_GRANDE_CITY_HALL5)
                      && WarpIsMap(destination, sHoennRooms[0]));
    if (venue == REGION_NONE)
    {
        if (gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active)
            EndLeagueRun();
        return;
    }
    if (admission)
    {
        EndLeagueRun();
        region = venue == REGION_HOENN ? REGION_HOENN : GetRequiredLeagueRegion();
        if ((venue == REGION_HOENN || region == REGION_KANTO || region == REGION_JOHTO)
         && IsEligibleForLeague(region))
        {
            ResetLeagueProgress(region);
            gSaveBlock3Ptr->wayfarerHoenn.leagueRun.region = region;
            gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry = GetTrainerRating();
            gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = TRUE;
            return;
        }
    }
    if (!ValidateLeagueLocation(destination))
        ReturnToLobby(destination, venue);
#endif
}

bool8 ConsumeLeagueRunLoadRecovery(void)
{
    bool8 pending = sLeagueRunLoadRecoveryPending;
    sLeagueRunLoadRecoveryPending = FALSE;
    return pending;
}

static bool8 IsCompletedLeagueReturn(enum Region venue)
{
    const struct HealLocation *heal = GetHealLocation(venue == REGION_HOENN
        ? HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE : HEAL_LOCATION_INDIGO_PLATEAU_HNS);
    const struct WarpData *warp = &gSaveBlock1Ptr->continueGameWarp;
    const u16 *rooms = venue == REGION_HOENN ? sHoennRooms : sIndigoRooms;
    return gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active == FALSE
        && UseContinueGameWarp() && WarpIsMap(&gSaveBlock1Ptr->location, rooms[5])
        && GetGameClearStateForRegion(venue) && warp->warpId == WARP_ID_NONE
        && warp->mapGroup == heal->mapGroup && warp->mapNum == heal->mapNum
        && warp->x == heal->x && warp->y == heal->y;
}

void LeagueRunValidateSavedLocation(void)
{
#if WAYFARER_LEAGUE_CIRCUIT_ENABLED
    enum Region venue = GetVenue(&gSaveBlock1Ptr->location);
    sLeagueRunLoadRecoveryPending = FALSE;
    if (venue == REGION_NONE)
    {
        if (gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active)
            EndLeagueRun();
    }
    else if (!ValidateActiveLeagueRun() && !IsCompletedLeagueReturn(venue))
    {
        ReturnToLobby(&gSaveBlock1Ptr->location, venue);
        sLeagueRunLoadRecoveryPending = TRUE;
        gSaveBlock1Ptr->pos.x = gSaveBlock1Ptr->location.x;
        gSaveBlock1Ptr->pos.y = gSaveBlock1Ptr->location.y;
        gSaveBlock1Ptr->mapLayoutId = Overworld_GetMapHeaderByGroupAndId(
            gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum)->mapLayoutId;
    }
#endif
}

u8 GetGlobalBadgeCount(void)
{
    return GetBadgeCountForRegion(REGION_KANTO)
         + GetBadgeCountForRegion(REGION_JOHTO)
         + GetBadgeCountForRegion(REGION_HOENN);
}

enum Region GetRequiredLeagueRegion(void)
{
    if (!GetChampionStateForRegion(REGION_KANTO))
        return REGION_KANTO;
    if (!GetChampionStateForRegion(REGION_JOHTO))
        return REGION_JOHTO;
    if (!GetChampionStateForRegion(REGION_HOENN))
        return REGION_HOENN;
    return REGION_NONE;
}

enum LeagueAdmissionRequirement GetLeagueAdmissionRequirement(enum Region region)
{
    switch (region)
    {
    case REGION_KANTO:
        if (GetChampionStateForRegion(REGION_KANTO))
            return LEAGUE_ADMISSION_UNAVAILABLE;
        if (GetGlobalBadgeCount() < 8)
            return LEAGUE_ADMISSION_NEEDS_8_BADGES;
        return LEAGUE_ADMISSION_AVAILABLE;
    case REGION_JOHTO:
        if (GetChampionStateForRegion(REGION_JOHTO))
            return LEAGUE_ADMISSION_UNAVAILABLE;
        if (!GetChampionStateForRegion(REGION_KANTO))
            return LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR;
        if (GetGlobalBadgeCount() < 16)
            return LEAGUE_ADMISSION_NEEDS_16_BADGES;
        return LEAGUE_ADMISSION_AVAILABLE;
    case REGION_HOENN:
        if (GetChampionStateForRegion(REGION_HOENN))
            return LEAGUE_ADMISSION_UNAVAILABLE;
        if (!GetChampionStateForRegion(REGION_KANTO))
            return LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR;
        if (!GetChampionStateForRegion(REGION_JOHTO))
            return LEAGUE_ADMISSION_NEEDS_JOHTO_CLEAR;
        if (GetGlobalBadgeCount() < 24)
            return LEAGUE_ADMISSION_NEEDS_24_BADGES;
        return LEAGUE_ADMISSION_AVAILABLE;
    default:
        return LEAGUE_ADMISSION_UNAVAILABLE;
    }
}

bool8 IsEligibleForLeague(enum Region region)
{
    return GetLeagueAdmissionRequirement(region) == LEAGUE_ADMISSION_AVAILABLE;
}

bool8 TryRecordLeagueClear(enum Region region)
{
    if (!ValidateActiveLeagueRun()
     || GetActiveLeagueRunRegion() != region
     || !IsLeagueRunComplete()
     || !IsEligibleForLeague(region))
        return FALSE;
    SetGameClearStateForRegion(region, TRUE);
    GetTrainerRating();
    sRecordedLeagueClearRegion = region;
    EndLeagueRun();
    return TRUE;
}

enum Region ConsumeRecordedLeagueClearRegion(void)
{
    enum Region region = sRecordedLeagueClearRegion;
    sRecordedLeagueClearRegion = REGION_NONE;
    return region;
}

u8 CalculateLeagueCircuitTrainerRating(void)
{
    u8 badges = GetGlobalBadgeCount();
    u8 rating;

    if (badges <= 4)
        rating = 4 * badges;
    else if (badges <= 8)
        rating = 16 + 6 * (badges - 4);
    else
        rating = 40 + badges - 8;

    if (GetChampionStateForRegion(REGION_KANTO))
        rating += 15;
    if (GetChampionStateForRegion(REGION_JOHTO))
        rating += 5;
    if (GetChampionStateForRegion(REGION_HOENN))
        rating += 4;
    return ClampTrainerRating(rating);
}
#endif
