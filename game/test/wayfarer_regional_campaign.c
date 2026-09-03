#include "global.h"
#include "event_data.h"
#include "pokemon.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "constants/vars.h"

#if IS_WAYFARER

extern void UpdateTrainerFanClubGameClear(void);
extern bool8 IsStarterInParty(void);

TEST("Wayfarer Hoenn starter party checks use the Hoenn choice")
{
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_PETALBURG_CITY_POKEMON_CENTER_1F);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_PETALBURG_CITY_POKEMON_CENTER_1F);
    VarSet(VAR_STARTER_MON, HOENN_STARTER_CHOICE_TREECKO);
    VarSet(VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_TORCHIC);
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_TORCHIC, 5, 0, OTID_STRUCT_PLAYER_ID);

    EXPECT(IsStarterInParty());

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_NEW_BARK_TOWN_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_NEW_BARK_TOWN_HNS);
    EXPECT(!IsStarterInParty());
}

TEST("Wayfarer Hoenn badge count ignores HNS badges")
{
    u8 i;

    for (i = 0; i < WAYFARER_HOENN_BADGE_COUNT; i++)
    {
        SetBadgeStateForRegion(REGION_JOHTO, i, TRUE);
        SetBadgeStateForRegion(REGION_KANTO, i, TRUE);
        SetBadgeStateForRegion(REGION_HOENN, i, FALSE);
    }

    EXPECT_EQ(WayfarerGetHoennBadgeCountForScript(), 0);
    for (i = 0; i < WAYFARER_HOENN_BADGE_COUNT; i++)
    {
        SetBadgeStateForRegion(REGION_HOENN, i, TRUE);
        EXPECT_EQ(WayfarerGetHoennBadgeCountForScript(), i + 1);
    }
}

TEST("Wayfarer Hoenn game clear also records only the Hoenn Champion")
{
    FlagClear(FLAG_SYS_GAME_CLEAR);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_HOENN, FALSE);

    SetGameClearStateForRegion(REGION_HOENN, TRUE);

    EXPECT(GetGameClearStateForRegion(REGION_HOENN));
    EXPECT(GetChampionStateForRegion(REGION_HOENN));
    EXPECT(!GetGameClearStateForRegion(REGION_JOHTO));
    EXPECT(!GetGameClearStateForRegion(REGION_KANTO));
    EXPECT(!GetChampionStateForRegion(REGION_JOHTO));
    EXPECT(!GetChampionStateForRegion(REGION_KANTO));
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
}

TEST("Wayfarer Hoenn Hall of Fame fan club cleanup stays in Hoenn")
{
    static const u16 hoennSourceHideFlags[] =
    {
        0x315, // FLAG_HIDE_FANCLUB_OLD_LADY in Emerald.
        0x316, // FLAG_HIDE_FANCLUB_BOY in Emerald.
        0x317, // FLAG_HIDE_FANCLUB_LITTLE_BOY in Emerald.
        0x318, // FLAG_HIDE_FANCLUB_LADY in Emerald.
        0x2DA, // FLAG_HIDE_LILYCOVE_FAN_CLUB_INTERVIEWER in Emerald.
    };
    u8 i;

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_EVER_GRANDE_CITY_HALL_OF_FAME);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_EVER_GRANDE_CITY_HALL_OF_FAME);
    EXPECT(WayfarerIsCurrentMapHoennSource());

    VarSet(VAR_FANCLUB_FAN_COUNTER, 0x1234);
    VarSet(VAR_FANCLUB_LOSE_FAN_TIMER, 0x5678);
    VarSet(VAR_LILYCOVE_FAN_CLUB_STATE, 0);
    VarSet(HOENN_VAR_ID(VAR_FANCLUB_FAN_COUNTER), 0);
    VarSet(HOENN_VAR_ID(VAR_FANCLUB_LOSE_FAN_TIMER), 0);
    VarSet(HOENN_VAR_ID(VAR_LILYCOVE_FAN_CLUB_STATE), 0);
    for (i = 0; i < ARRAY_COUNT(hoennSourceHideFlags); i++)
    {
        FlagSet(hoennSourceHideFlags[i]);
        FlagSet(HOENN_FLAG_ID(hoennSourceHideFlags[i]));
    }

    UpdateTrainerFanClubGameClear();

    EXPECT_EQ(VarGet(VAR_FANCLUB_FAN_COUNTER), 0x1234);
    EXPECT_EQ(VarGet(VAR_FANCLUB_LOSE_FAN_TIMER), 0x5678);
    EXPECT_EQ(VarGet(VAR_LILYCOVE_FAN_CLUB_STATE), 0);
    EXPECT_NE(VarGet(HOENN_VAR_ID(VAR_FANCLUB_FAN_COUNTER)), 0);
    EXPECT_EQ(VarGet(HOENN_VAR_ID(VAR_LILYCOVE_FAN_CLUB_STATE)), 1);
    for (i = 0; i < ARRAY_COUNT(hoennSourceHideFlags); i++)
    {
        EXPECT(FlagGet(hoennSourceHideFlags[i]));
        EXPECT(!FlagGet(HOENN_FLAG_ID(hoennSourceHideFlags[i])));
    }
}

#endif // IS_WAYFARER
