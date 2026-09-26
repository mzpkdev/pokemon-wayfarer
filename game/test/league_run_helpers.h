#ifndef GUARD_TEST_LEAGUE_RUN_HELPERS_H
#define GUARD_TEST_LEAGUE_RUN_HELPERS_H

#include "event_data.h"
#include "league_circuit.h"
#include "constants/maps.h"
#include "constants/vars.h"

#if IS_WAYFARER
static inline void Test_SetLeagueMap(struct WarpData *warp, u16 map)
{
    memset(warp, 0, sizeof(*warp));
    warp->mapGroup = MAP_GROUP(map);
    warp->mapNum = MAP_NUM(map);
}

static inline enum CircuitStage Test_StageForRegion(enum Region region)
{
    return region == REGION_KANTO ? CIRCUIT_STAGE_INDIGO
         : region == REGION_JOHTO ? CIRCUIT_STAGE_MASTERS
         : region == REGION_HOENN ? CIRCUIT_STAGE_HOENN : CIRCUIT_STAGE_NONE;
}

static inline bool8 Test_AdmitCircuitRun(enum CircuitStage stage)
{
    struct WarpData source, destination;
    if (stage == CIRCUIT_STAGE_MASTERS)
    {
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_SEVEN_ISLAND_HOUSE_ROOM1);
        if (!BeginCircuitRun(stage))
            return FALSE;
    }
    Test_SetLeagueMap(&source, stage == CIRCUIT_STAGE_HOENN
        ? MAP_EVER_GRANDE_CITY_HALL5 : stage == CIRCUIT_STAGE_MASTERS
        ? MAP_SEVEN_ISLAND_HOUSE_ROOM2 : MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS);
    Test_SetLeagueMap(&destination, stage == CIRCUIT_STAGE_HOENN
        ? MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM : stage == CIRCUIT_STAGE_MASTERS
        ? MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS : MAP_POKEMON_LEAGUE_LORELEIS_ROOM);
    LeagueRunHandleWarp(&source, &destination);
    gSaveBlock1Ptr->location = destination;
    return GetActiveLeagueRunStage() == stage;
}

static inline bool8 Test_AdmitLeagueRun(enum Region region)
{
    return Test_AdmitCircuitRun(Test_StageForRegion(region));
}

static inline void Test_CompleteCircuitRooms(enum CircuitStage stage)
{
    u8 i;
    if (stage == CIRCUIT_STAGE_HOENN)
    {
        for (i = 0; i < 4; i++)
            FlagSet(HOENN_FLAG_ID(0x4FB) + i);
        VarSet(HOENN_VAR_ID(0x409C), 5);
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_EVER_GRANDE_CITY_HALL_OF_FAME);
    }
    else if (stage == CIRCUIT_STAGE_MASTERS)
    {
        VarSet(VAR_LEAGUE_STATE, 6);
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
    }
    else
    {
        gSaveBlock3Ptr->wayfarerHoenn.indigoRoomDefeats = 0x1F;
        VarSet(VAR_LEAGUE_STATE, 5);
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_POKEMON_LEAGUE_HALL_OF_FAME);
    }
}

static inline void Test_CompleteLeagueRooms(enum Region region)
{
    Test_CompleteCircuitRooms(Test_StageForRegion(region));
}

static inline enum CircuitCommitResult Test_CompleteAndCommitCircuit(enum CircuitStage stage)
{
    if (!Test_AdmitCircuitRun(stage))
        return CIRCUIT_COMMIT_INVALID;
    Test_CompleteCircuitRooms(stage);
    return CommitCircuitRun(stage);
}

static inline bool8 Test_CompleteAndRecordLeague(enum Region region)
{
    return Test_CompleteAndCommitCircuit(Test_StageForRegion(region)) == CIRCUIT_COMMIT_FIRST_CLEAR;
}
#endif
#endif
