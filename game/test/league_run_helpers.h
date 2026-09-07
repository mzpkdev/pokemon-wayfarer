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

static inline bool8 Test_AdmitLeagueRun(enum Region region)
{
    struct WarpData source, destination;
    Test_SetLeagueMap(&source, region == REGION_HOENN
        ? MAP_EVER_GRANDE_CITY_HALL5 : MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS);
    Test_SetLeagueMap(&destination, region == REGION_HOENN
        ? MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM : MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS);
    LeagueRunHandleWarp(&source, &destination);
    gSaveBlock1Ptr->location = destination;
    return GetActiveLeagueRunRegion() == region;
}

static inline void Test_CompleteLeagueRooms(enum Region region)
{
    u8 i;
    if (region == REGION_HOENN)
    {
        for (i = 0; i < 4; i++)
            FlagSet(HOENN_FLAG_ID(0x4FB) + i);
        VarSet(HOENN_VAR_ID(0x409C), 5);
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_EVER_GRANDE_CITY_HALL_OF_FAME);
    }
    else
    {
        VarSet(VAR_LEAGUE_STATE, 6);
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
    }
}

static inline bool8 Test_CompleteAndRecordLeague(enum Region region)
{
    if (!Test_AdmitLeagueRun(region))
        return FALSE;
    Test_CompleteLeagueRooms(region);
    return TryRecordLeagueClear(region);
}
#endif
#endif
