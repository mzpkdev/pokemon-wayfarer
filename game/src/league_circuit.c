#include "global.h"
#include "event_data.h"
#include "heal_location.h"
#include "load_save.h"
#include "overworld.h"
#include "config/league_circuit.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"
#include "constants/vars.h"
#include "league_circuit.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"

#if IS_WAYFARER
EWRAM_DATA static enum CircuitStage sRecordedCircuitClearStage = CIRCUIT_STAGE_NONE;
EWRAM_DATA static bool8 sLeagueRunLoadRecoveryPending = FALSE;

#define LEAGUE_HOENN_STATE HOENN_VAR_ID(0x409C)
#define LEAGUE_HOENN_FIRST_DEFEAT HOENN_FLAG_ID(0x4FB)
#define CIRCUIT_CLEAR_INDIGO (1 << 2)
#define CIRCUIT_CLEAR_MASTERS (1 << 3)
#define CIRCUIT_CLEAR_HOENN (1 << 4)

static const u16 sIndigoRooms[] = {
    MAP_POKEMON_LEAGUE_LORELEIS_ROOM, MAP_POKEMON_LEAGUE_BRUNOS_ROOM,
    MAP_POKEMON_LEAGUE_AGATHAS_ROOM, MAP_POKEMON_LEAGUE_LANCES_ROOM,
    MAP_POKEMON_LEAGUE_CHAMPIONS_ROOM, MAP_POKEMON_LEAGUE_HALL_OF_FAME,
};
static const u16 sMastersRooms[] = {
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

static const u16 *GetStageRooms(enum CircuitStage stage)
{
    if (stage == CIRCUIT_STAGE_INDIGO)
        return sIndigoRooms;
    if (stage == CIRCUIT_STAGE_MASTERS)
        return sMastersRooms;
    return sHoennRooms;
}

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

bool8 IsWayfarerMastersCircuitMap(s16 mapGroup, s16 mapNum)
{
    struct WarpData location = {
        .mapGroup = mapGroup,
        .mapNum = mapNum,
    };

    return FindRoom(&location, sMastersRooms, ARRAY_COUNT(sMastersRooms)) >= 0;
}

bool8 SetWayfarerMastersHouseWarpForMap(s16 mapGroup, s16 mapNum, struct WarpData *warp)
{
    const struct MapHeader *header;

    if (!IsWayfarerMastersCircuitMap(mapGroup, mapNum) || warp == NULL)
        return FALSE;

    warp->mapGroup = MAP_GROUP(MAP_SEVEN_ISLAND_HOUSE_ROOM1);
    warp->mapNum = MAP_NUM(MAP_SEVEN_ISLAND_HOUSE_ROOM1);
    warp->warpId = 0;
    header = Overworld_GetMapHeaderByGroupAndId(warp->mapGroup, warp->mapNum);
    warp->x = header->events->warps[0].x;
    warp->y = header->events->warps[0].y;
    return TRUE;
}

bool8 SetWayfarerCircuitLobbyWarpForMap(s16 mapGroup, s16 mapNum, struct WarpData *warp)
{
    struct WarpData location = {
        .mapGroup = mapGroup,
        .mapNum = mapNum,
    };
    const struct MapHeader *header;

    if (warp == NULL)
        return FALSE;
    if (FindRoom(&location, sIndigoRooms, ARRAY_COUNT(sIndigoRooms)) >= 0)
    {
        warp->mapGroup = MAP_GROUP(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS);
        warp->mapNum = MAP_NUM(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS);
        warp->warpId = 0;
        header = Overworld_GetMapHeaderByGroupAndId(warp->mapGroup, warp->mapNum);
        warp->x = header->events->warps[0].x;
        warp->y = header->events->warps[0].y;
        return TRUE;
    }
    return SetWayfarerMastersHouseWarpForMap(mapGroup, mapNum, warp);
}

static enum CircuitStage GetVenue(const struct WarpData *warp)
{
    if (FindRoom(warp, sIndigoRooms, ARRAY_COUNT(sIndigoRooms)) >= 0)
        return CIRCUIT_STAGE_INDIGO;
    if (FindRoom(warp, sMastersRooms, ARRAY_COUNT(sMastersRooms)) >= 0)
        return CIRCUIT_STAGE_MASTERS;
    if (FindRoom(warp, sHoennRooms, ARRAY_COUNT(sHoennRooms)) >= 0
     || FindRoom(warp, sHoennHalls, ARRAY_COUNT(sHoennHalls)) >= 0)
        return CIRCUIT_STAGE_HOENN;
    return CIRCUIT_STAGE_NONE;
}

static void ResetLeagueProgress(enum CircuitStage stage)
{
    u8 i;
    if (stage == CIRCUIT_STAGE_INDIGO)
    {
        gSaveBlock3Ptr->wayfarerHoenn.indigoRoomDefeats = 0;
        VarSet(VAR_LEAGUE_STATE, 0);
    }
    else if (stage == CIRCUIT_STAGE_MASTERS)
        VarSet(VAR_LEAGUE_STATE, 1);
    else if (stage == CIRCUIT_STAGE_HOENN)
    {
        VarSet(LEAGUE_HOENN_STATE, 0);
        for (i = 0; i < 4; i++)
            FlagClear(LEAGUE_HOENN_FIRST_DEFEAT + i);
    }
}

void EndLeagueRun(void)
{
    struct LeagueRunState *run = &gSaveBlock3Ptr->wayfarerHoenn.leagueRun;
    if (run->stage >= CIRCUIT_STAGE_INDIGO && run->stage <= CIRCUIT_STAGE_HOENN)
        ResetLeagueProgress(run->stage);
    memset(run, 0, sizeof(*run));
}

bool8 HasClearedCircuitStage(enum CircuitStage stage)
{
    u8 flags = gSaveBlock3Ptr->wayfarerHoenn.leagueFlags;
    switch (stage)
    {
    case CIRCUIT_STAGE_INDIGO: return (flags & CIRCUIT_CLEAR_INDIGO) != 0;
    case CIRCUIT_STAGE_MASTERS: return (flags & CIRCUIT_CLEAR_MASTERS) != 0;
    case CIRCUIT_STAGE_HOENN: return (flags & CIRCUIT_CLEAR_HOENN) != 0;
    default: return FALSE;
    }
}

bool8 HasCommittedFirstIndigoVictory(void)
{
    return HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO);
}

enum CircuitStage GetActiveLeagueRunStage(void)
{
    const struct LeagueRunState *run = &gSaveBlock3Ptr->wayfarerHoenn.leagueRun;
    if (run->active != TRUE || run->ratingAtEntry > 80
     || run->stage < CIRCUIT_STAGE_INDIGO || run->stage > CIRCUIT_STAGE_HOENN
     || run->replay > TRUE || run->replay != HasClearedCircuitStage(run->stage))
        return CIRCUIT_STAGE_NONE;
    return run->stage;
}

bool8 IsActiveLeagueRunReplay(void)
{
    return GetActiveLeagueRunStage() != CIRCUIT_STAGE_NONE
        && gSaveBlock3Ptr->wayfarerHoenn.leagueRun.replay;
}

// Kept for region-based callers; Masters remains Kanto for ordinary systems.
enum Region GetActiveLeagueRunRegion(void)
{
    enum CircuitStage stage = GetActiveLeagueRunStage();
    if (stage == CIRCUIT_STAGE_HOENN)
        return REGION_HOENN;
    if (stage == CIRCUIT_STAGE_INDIGO || stage == CIRCUIT_STAGE_MASTERS)
        return REGION_KANTO;
    return REGION_NONE;
}

// A prefix of defeated members is valid; a later defeat without its predecessor is not.
static s8 GetDefeatedCount(enum CircuitStage stage)
{
    u8 i, count = 0;
    u16 state;
    if (stage == CIRCUIT_STAGE_INDIGO)
    {
        u8 bits = gSaveBlock3Ptr->wayfarerHoenn.indigoRoomDefeats;
        for (i = 0; i < 5; i++)
            if (bits & (1 << i))
            {
                if (i != count)
                    return -1;
                count++;
            }
        return (bits & ~0x1F) ? -1 : count;
    }
    if (stage == CIRCUIT_STAGE_MASTERS)
    {
        state = VarGet(VAR_LEAGUE_STATE);
        return state >= 1 && state <= 6 ? state - 1 : -1;
    }
    for (i = 0; i < 4; i++)
        if (FlagGet(LEAGUE_HOENN_FIRST_DEFEAT + i))
        {
            if (i != count)
                return -1;
            count++;
        }
    state = VarGet(LEAGUE_HOENN_STATE);
    if (state == 5)
        return count == 4 ? 5 : -1;
    return state <= 4 ? count : -1;
}

static bool8 ValidateLeagueLocation(const struct WarpData *location)
{
    enum CircuitStage stage = GetActiveLeagueRunStage();
    s8 room, defeated, hall;
    u16 state;
    if (stage == CIRCUIT_STAGE_NONE || GetVenue(location) != stage)
        return FALSE;
    room = FindRoom(location, GetStageRooms(stage), 6);
    defeated = GetDefeatedCount(stage);
    if (defeated < 0)
        return FALSE;
    if (stage == CIRCUIT_STAGE_INDIGO)
    {
        state = VarGet(VAR_LEAGUE_STATE);
        if (room == 5 ? state != 5 : state != room && state != room + 1)
            return FALSE;
    }
    if (stage == CIRCUIT_STAGE_HOENN)
    {
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
    return room >= 0 && (room == 5 ? defeated == 5
                         : defeated == room || defeated == room + 1);
}

bool8 ValidateActiveLeagueRun(void)
{
    return ValidateLeagueLocation(&gSaveBlock1Ptr->location);
}

bool8 ValidateCircuitRoomBattle(enum CircuitStage stage, u8 encounterIndex)
{
    return encounterIndex < 5 && GetActiveLeagueRunStage() == stage
        && ValidateActiveLeagueRun() && GetDefeatedCount(stage) == encounterIndex
        && WarpIsMap(&gSaveBlock1Ptr->location, GetStageRooms(stage)[encounterIndex]);
}

bool32 GetCircuitRunBattleRating(enum CircuitStage stage, u32 encounterIndex, u8 *rating)
{
    if (!ValidateCircuitRoomBattle(stage, encounterIndex))
        return FALSE;
    *rating = gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry;
    return TRUE;
}

bool32 GetLeagueRunBattleRating(u32 region, u32 encounterIndex, u8 *rating)
{
    enum CircuitStage stage = region == REGION_HOENN ? CIRCUIT_STAGE_HOENN
        : region == REGION_KANTO ? CIRCUIT_STAGE_INDIGO
        : region == REGION_JOHTO ? CIRCUIT_STAGE_MASTERS : CIRCUIT_STAGE_NONE;
    return GetCircuitRunBattleRating(stage, encounterIndex, rating);
}

bool8 RecordCircuitRoomVictory(enum CircuitStage stage, u8 encounterIndex)
{
    if (!ValidateCircuitRoomBattle(stage, encounterIndex))
        return FALSE;
    if (stage == CIRCUIT_STAGE_INDIGO)
    {
        gSaveBlock3Ptr->wayfarerHoenn.indigoRoomDefeats |= 1 << encounterIndex;
        // The FRLG room scripts use this as their scene counter. In particular,
        // the Hall of Fame destination must observe state 5 after Blue's win.
        VarSet(VAR_LEAGUE_STATE, encounterIndex + 1);
    }
    else if (stage == CIRCUIT_STAGE_MASTERS)
        VarSet(VAR_LEAGUE_STATE, encounterIndex + 2);
    else if (encounterIndex < 4)
    {
        FlagSet(LEAGUE_HOENN_FIRST_DEFEAT + encounterIndex);
        VarSet(LEAGUE_HOENN_STATE, encounterIndex + 1);
    }
    else
        VarSet(LEAGUE_HOENN_STATE, 5);
    return TRUE;
}

bool8 IsCurrentLeagueRoomDefeated(void)
{
    enum CircuitStage stage = GetActiveLeagueRunStage();
    s8 room;
    if (stage == CIRCUIT_STAGE_NONE || !ValidateActiveLeagueRun())
        return FALSE;
    room = FindRoom(&gSaveBlock1Ptr->location, GetStageRooms(stage), 5);
    return room >= 0 && GetDefeatedCount(stage) == room + 1;
}

bool8 MarkLeagueChampionDefeated(void)
{
    return RecordCircuitRoomVictory(CIRCUIT_STAGE_HOENN, 4);
}

bool8 CanCompleteCircuitRun(enum CircuitStage stage)
{
    return stage != CIRCUIT_STAGE_NONE && GetActiveLeagueRunStage() == stage
        && ValidateActiveLeagueRun() && GetDefeatedCount(stage) == 5
        && WarpIsMap(&gSaveBlock1Ptr->location, GetStageRooms(stage)[5]);
}

static void ReturnToLobby(struct WarpData *location, enum CircuitStage venue)
{
    const struct MapHeader *header;
    u16 map = venue == CIRCUIT_STAGE_HOENN ? MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F
        : venue == CIRCUIT_STAGE_MASTERS ? MAP_SEVEN_ISLAND_HOUSE_ROOM1
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
    enum CircuitStage venue = GetVenue(destination);
    bool8 admission = (WarpIsMap(source, MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS)
                       && WarpIsMap(destination, sIndigoRooms[0]))
                   || (WarpIsMap(source, MAP_SEVEN_ISLAND_HOUSE_ROOM2)
                       && WarpIsMap(destination, sMastersRooms[0]))
                   || (WarpIsMap(source, MAP_EVER_GRANDE_CITY_HALL5)
                       && WarpIsMap(destination, sHoennRooms[0]));
    if (venue == CIRCUIT_STAGE_NONE)
    {
        // Masters admission is accepted in Room 1; Room 2 is its antechamber.
        if (WarpIsMap(destination, MAP_SEVEN_ISLAND_HOUSE_ROOM2)
         && GetActiveLeagueRunStage() == CIRCUIT_STAGE_MASTERS)
        {
            if (WarpIsMap(source, MAP_SEVEN_ISLAND_HOUSE_ROOM1))
                return;
            ReturnToLobby(destination, CIRCUIT_STAGE_MASTERS);
            return;
        }
        if (gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active)
            EndLeagueRun();
        return;
    }
    if (admission)
    {
        if (venue == CIRCUIT_STAGE_MASTERS)
        {
            if (GetActiveLeagueRunStage() == venue)
                return;
            ReturnToLobby(destination, venue);
            return;
        }
        EndLeagueRun();
        if (IsEligibleForCircuitStage(venue))
        {
            struct LeagueRunState *run = &gSaveBlock3Ptr->wayfarerHoenn.leagueRun;
            ResetLeagueProgress(venue);
            run->stage = venue;
            run->replay = HasClearedCircuitStage(venue);
            run->ratingAtEntry = GetTrainerRating();
            run->active = TRUE;
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

static bool8 IsCompletedLeagueReturn(enum CircuitStage venue)
{
    const struct HealLocation *heal;
    const struct WarpData *warp = &gSaveBlock1Ptr->continueGameWarp;
    if (venue != CIRCUIT_STAGE_HOENN)
        return FALSE;
    heal = GetHealLocation(HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE);
    return gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active == FALSE
        && UseContinueGameWarp() && WarpIsMap(&gSaveBlock1Ptr->location, sHoennRooms[5])
        && GetGameClearStateForRegion(REGION_HOENN) && warp->warpId == WARP_ID_NONE
        && warp->mapGroup == heal->mapGroup && warp->mapNum == heal->mapNum
        && warp->x == heal->x && warp->y == heal->y;
}

void LeagueRunValidateSavedLocation(void)
{
#if WAYFARER_LEAGUE_CIRCUIT_ENABLED
    enum CircuitStage venue = GetVenue(&gSaveBlock1Ptr->location);
    sLeagueRunLoadRecoveryPending = FALSE;
    if (WarpIsMap(&gSaveBlock1Ptr->location, MAP_SEVEN_ISLAND_HOUSE_ROOM2)
     && GetActiveLeagueRunStage() == CIRCUIT_STAGE_MASTERS)
        return;
    if (venue == CIRCUIT_STAGE_NONE)
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

enum CircuitStage GetRequiredCircuitStage(void)
{
    if (!HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO))
        return CIRCUIT_STAGE_INDIGO;
    if (!HasClearedCircuitStage(CIRCUIT_STAGE_MASTERS))
        return CIRCUIT_STAGE_MASTERS;
    if (!HasClearedCircuitStage(CIRCUIT_STAGE_HOENN))
        return CIRCUIT_STAGE_HOENN;
    return CIRCUIT_STAGE_NONE;
}

enum LeagueAdmissionRequirement GetCircuitAdmissionRequirement(enum CircuitStage stage)
{
    if (HasClearedCircuitStage(stage))
        return LEAGUE_ADMISSION_AVAILABLE;
    switch (stage)
    {
    case CIRCUIT_STAGE_INDIGO:
        return GetGlobalBadgeCount() >= 8 ? LEAGUE_ADMISSION_AVAILABLE
            : LEAGUE_ADMISSION_NEEDS_8_BADGES;
    case CIRCUIT_STAGE_MASTERS:
        if (!HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO))
            return LEAGUE_ADMISSION_NEEDS_INDIGO_CLEAR;
        return GetGlobalBadgeCount() >= 16 ? LEAGUE_ADMISSION_AVAILABLE
            : LEAGUE_ADMISSION_NEEDS_16_BADGES;
    case CIRCUIT_STAGE_HOENN:
        if (!HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO))
            return LEAGUE_ADMISSION_NEEDS_INDIGO_CLEAR;
        if (!HasClearedCircuitStage(CIRCUIT_STAGE_MASTERS))
            return LEAGUE_ADMISSION_NEEDS_MASTERS_CLEAR;
        return GetGlobalBadgeCount() == 24 ? LEAGUE_ADMISSION_AVAILABLE
            : LEAGUE_ADMISSION_NEEDS_24_BADGES;
    default:
        return LEAGUE_ADMISSION_UNAVAILABLE;
    }
}

bool8 IsEligibleForCircuitStage(enum CircuitStage stage)
{
    return GetCircuitAdmissionRequirement(stage) == LEAGUE_ADMISSION_AVAILABLE;
}

bool8 BeginCircuitRun(enum CircuitStage stage)
{
    struct LeagueRunState *run = &gSaveBlock3Ptr->wayfarerHoenn.leagueRun;
    if (GetActiveLeagueRunStage() == stage)
        return TRUE;
    if (run->active || !IsEligibleForCircuitStage(stage))
        return FALSE;
    if (stage == CIRCUIT_STAGE_MASTERS
     && !WarpIsMap(&gSaveBlock1Ptr->location, MAP_SEVEN_ISLAND_HOUSE_ROOM1))
        return FALSE;
    ResetLeagueProgress(stage);
    run->stage = stage;
    run->replay = HasClearedCircuitStage(stage);
    run->ratingAtEntry = GetTrainerRating();
    run->active = TRUE;
    return TRUE;
}

enum CircuitCommitResult CommitCircuitRun(enum CircuitStage stage)
{
    struct LeagueRunState *run = &gSaveBlock3Ptr->wayfarerHoenn.leagueRun;
    bool8 firstClear;
    if (!CanCompleteCircuitRun(stage))
        return CIRCUIT_COMMIT_INVALID;
    firstClear = !run->replay;
    if (firstClear)
    {
        if (stage == CIRCUIT_STAGE_INDIGO)
        {
            gSaveBlock3Ptr->wayfarerHoenn.leagueFlags |= CIRCUIT_CLEAR_INDIGO;
            SetGameClearStateForRegion(REGION_KANTO, TRUE);
            SetGameClearStateForRegion(REGION_JOHTO, TRUE);
        }
        else if (stage == CIRCUIT_STAGE_MASTERS)
            gSaveBlock3Ptr->wayfarerHoenn.leagueFlags |= CIRCUIT_CLEAR_MASTERS;
        else
        {
            gSaveBlock3Ptr->wayfarerHoenn.leagueFlags |= CIRCUIT_CLEAR_HOENN;
            SetGameClearStateForRegion(REGION_HOENN, TRUE);
        }
        GetTrainerRating();
        sRecordedCircuitClearStage = stage;
    }
    EndLeagueRun();
    return firstClear ? CIRCUIT_COMMIT_FIRST_CLEAR : CIRCUIT_COMMIT_REPLAY;
}

void RollbackIndigoHallOfFameCommit(u8 ratingAtEntry, u8 storedRatingBefore)
{
    struct LeagueRunState *run = &gSaveBlock3Ptr->wayfarerHoenn.leagueRun;

    gSaveBlock3Ptr->wayfarerHoenn.leagueFlags &= ~CIRCUIT_CLEAR_INDIGO;
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    sRecordedCircuitClearStage = CIRCUIT_STAGE_NONE;
    memset(run, 0, sizeof(*run));
    run->stage = CIRCUIT_STAGE_INDIGO;
    run->ratingAtEntry = ratingAtEntry;
    run->active = TRUE;
    gSaveBlock3Ptr->wayfarerHoenn.indigoRoomDefeats = 0x1F;
    VarSet(VAR_LEAGUE_STATE, 5);
    SetTrainerRating(storedRatingBefore);
}

bool8 TryRecordCircuitClear(enum CircuitStage stage)
{
    return CommitCircuitRun(stage) == CIRCUIT_COMMIT_FIRST_CLEAR;
}

enum CircuitStage ConsumeRecordedCircuitClearStage(void)
{
    enum CircuitStage stage = sRecordedCircuitClearStage;
    sRecordedCircuitClearStage = CIRCUIT_STAGE_NONE;
    return stage;
}

static enum CircuitStage StageFromRegion(enum Region region)
{
    return region == REGION_KANTO ? CIRCUIT_STAGE_INDIGO
        : region == REGION_JOHTO ? CIRCUIT_STAGE_MASTERS
        : region == REGION_HOENN ? CIRCUIT_STAGE_HOENN : CIRCUIT_STAGE_NONE;
}

enum Region GetRequiredLeagueRegion(void)
{
    enum CircuitStage stage = GetRequiredCircuitStage();
    return stage == CIRCUIT_STAGE_INDIGO ? REGION_KANTO
        : stage == CIRCUIT_STAGE_MASTERS ? REGION_JOHTO
        : stage == CIRCUIT_STAGE_HOENN ? REGION_HOENN : REGION_NONE;
}

enum LeagueAdmissionRequirement GetLeagueAdmissionRequirement(enum Region region)
{
    return GetCircuitAdmissionRequirement(StageFromRegion(region));
}

bool8 IsEligibleForLeague(enum Region region)
{
    return IsEligibleForCircuitStage(StageFromRegion(region));
}

bool8 TryRecordLeagueClear(enum Region region)
{
    return TryRecordCircuitClear(StageFromRegion(region));
}

enum Region ConsumeRecordedLeagueClearRegion(void)
{
    enum CircuitStage stage = ConsumeRecordedCircuitClearStage();
    return stage == CIRCUIT_STAGE_INDIGO ? REGION_KANTO
        : stage == CIRCUIT_STAGE_MASTERS ? REGION_JOHTO
        : stage == CIRCUIT_STAGE_HOENN ? REGION_HOENN : REGION_NONE;
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
    if (HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO))
        rating += 8;
    if (HasClearedCircuitStage(CIRCUIT_STAGE_MASTERS))
        rating += 8;
    if (HasClearedCircuitStage(CIRCUIT_STAGE_HOENN))
        rating += 8;
    return ClampTrainerRating(rating);
}

#if TESTING || defined(E2E_TESTING)
void SetCircuitClearForTesting(enum CircuitStage stage, bool8 value)
{
    u8 mask;
    if (stage == CIRCUIT_STAGE_INDIGO)
        mask = CIRCUIT_CLEAR_INDIGO;
    else if (stage == CIRCUIT_STAGE_MASTERS)
        mask = CIRCUIT_CLEAR_MASTERS;
    else if (stage == CIRCUIT_STAGE_HOENN)
        mask = CIRCUIT_CLEAR_HOENN;
    else
        return;
    if (value)
        gSaveBlock3Ptr->wayfarerHoenn.leagueFlags |= mask;
    else
        gSaveBlock3Ptr->wayfarerHoenn.leagueFlags &= ~mask;
    if (stage == CIRCUIT_STAGE_INDIGO)
    {
        SetGameClearStateForRegion(REGION_KANTO, value);
        SetGameClearStateForRegion(REGION_JOHTO, value);
    }
    else if (stage == CIRCUIT_STAGE_HOENN)
        SetGameClearStateForRegion(REGION_HOENN, value);
    GetTrainerRating();
}
#endif
#endif
