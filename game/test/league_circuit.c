#include "global.h"
#include "credits.h"
#include "event_data.h"
#include "heal_location.h"
#include "league_circuit.h"
#include "league_run_helpers.h"
#include "main.h"
#include "load_save.h"
#include "save.h"
#include "gba/flash_internal.h"
#include "wayfarer_persistence.h"
#include "trainer_rating.h"
#include "test/test.h"
#include "config/league_circuit.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"

#if IS_WAYFARER
static void SetRegionalBadges(enum Region region, u8 count)
{
    u8 i;

    for (i = 0; i < 8; i++)
        SetBadgeStateForRegion(region, i, i < count);
}

static void SetTotalBadges(u8 count)
{
    u8 i;

    for (i = 0; i < 24; i++)
        SetBadgeStateForRegion(REGION_KANTO + i / 8, i % 8, i < count);
}

TEST("League circuit aggregates all 729 regional badge distributions without duplicates")
{
    u8 kanto, johto, hoenn;

    for (kanto = 0; kanto <= 8; kanto++)
    {
        SetRegionalBadges(REGION_KANTO, kanto);
        for (johto = 0; johto <= 8; johto++)
        {
            SetRegionalBadges(REGION_JOHTO, johto);
            for (hoenn = 0; hoenn <= 8; hoenn++)
            {
                SetRegionalBadges(REGION_HOENN, hoenn);
                EXPECT_EQ(GetGlobalBadgeCount(), kanto + johto + hoenn);
                SetRegionalBadges(REGION_HOENN, hoenn);
                EXPECT_EQ(GetGlobalBadgeCount(), kanto + johto + hoenn);
            }
        }
    }
}

TEST("League circuit permits all 24 badges without any League clear")
{
    enum Region region;

    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        SetRegionalBadges(region, 8);
        EXPECT_EQ(GetGlobalBadgeCount(), 8 * region);
        EXPECT(!GetChampionStateForRegion(REGION_KANTO));
        EXPECT(!GetChampionStateForRegion(REGION_JOHTO));
        EXPECT(!GetChampionStateForRegion(REGION_HOENN));
    }

    EXPECT_EQ(GetGlobalBadgeCount(), 24);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_KANTO);
    EXPECT(IsEligibleForLeague(REGION_KANTO));
    EXPECT(!IsEligibleForLeague(REGION_JOHTO));
    EXPECT(!IsEligibleForLeague(REGION_HOENN));
}

TEST("League circuit checks exact badge thresholds and preceding clears")
{
    SetTotalBadges(7);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_KANTO), LEAGUE_ADMISSION_NEEDS_8_BADGES);
    EXPECT(!IsEligibleForLeague(REGION_KANTO));
    SetTotalBadges(8);
    EXPECT(IsEligibleForLeague(REGION_KANTO));

    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_JOHTO), LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR);
    SetGameClearStateForRegion(REGION_KANTO, TRUE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_JOHTO), LEAGUE_ADMISSION_NEEDS_16_BADGES);
    SetTotalBadges(15);
    EXPECT(!IsEligibleForLeague(REGION_JOHTO));
    SetTotalBadges(16);
    EXPECT(IsEligibleForLeague(REGION_JOHTO));

    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_HOENN), LEAGUE_ADMISSION_NEEDS_JOHTO_CLEAR);
    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_HOENN), LEAGUE_ADMISSION_NEEDS_24_BADGES);
    SetTotalBadges(23);
    EXPECT(!IsEligibleForLeague(REGION_HOENN));
    SetTotalBadges(24);
    EXPECT(IsEligibleForLeague(REGION_HOENN));
}

TEST("League circuit accepts mixed badges and exact thresholds without host badges")
{
    SetRegionalBadges(REGION_JOHTO, 4);
    SetRegionalBadges(REGION_HOENN, 4);
    EXPECT_EQ(GetGlobalBadgeCount(), 8);
    EXPECT(IsEligibleForLeague(REGION_KANTO));
    EXPECT(Test_CompleteAndRecordLeague(REGION_KANTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);

    SetRegionalBadges(REGION_JOHTO, 0);
    SetRegionalBadges(REGION_KANTO, 8);
    SetRegionalBadges(REGION_HOENN, 8);
    EXPECT_EQ(GetGlobalBadgeCount(), 16);
    EXPECT(IsEligibleForLeague(REGION_JOHTO));
    EXPECT(Test_CompleteAndRecordLeague(REGION_JOHTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_JOHTO);

    SetRegionalBadges(REGION_JOHTO, 8);
    EXPECT_EQ(GetGlobalBadgeCount(), 24);
    EXPECT(IsEligibleForLeague(REGION_HOENN));
    EXPECT(Test_CompleteAndRecordLeague(REGION_HOENN));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_HOENN);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_NONE);
}

TEST("League circuit clear order stays Kanto then Johto then Hoenn with all badges earned")
{
    enum Region region;

    SetTotalBadges(24);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT_EQ(GetRequiredLeagueRegion(), region);
        EXPECT(IsEligibleForLeague(region));
        if (region != REGION_KANTO)
            EXPECT(!IsEligibleForLeague(region - 1));
        if (region != REGION_HOENN)
            EXPECT(!IsEligibleForLeague(region + 1));
        EXPECT(Test_CompleteAndRecordLeague(region));
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), region);
    }
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_NONE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_NONE), LEAGUE_ADMISSION_UNAVAILABLE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_HOENN), LEAGUE_ADMISSION_UNAVAILABLE);
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));
}

TEST("League circuit rejects mixed out-of-order clear states")
{
    SetTotalBadges(24);
    EXPECT(!TryRecordLeagueClear(REGION_JOHTO));
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));

    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_KANTO);
    EXPECT(IsEligibleForLeague(REGION_KANTO));
    EXPECT(!IsEligibleForLeague(REGION_JOHTO));
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_JOHTO), LEAGUE_ADMISSION_UNAVAILABLE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_HOENN), LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR);

    EXPECT(Test_CompleteAndRecordLeague(REGION_KANTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_HOENN);
    EXPECT(IsEligibleForLeague(REGION_HOENN));
}

TEST("League circuit clear preserves every badge and unrelated regional state")
{
    enum Region region, other;
    u8 badge;

    SetTotalBadges(24);
    FlagSet(HOENN_FLAG_ID(0x52));
    FlagSet(0x52);
    VarSet(HOENN_VAR_ID(0x4001), 1234);
    VarSet(0x4001, 5678);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT(Test_CompleteAndRecordLeague(region));
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), region);
        EXPECT(GetGameClearStateForRegion(region));
        EXPECT(!TryRecordLeagueClear(region));
        for (other = REGION_KANTO; other <= REGION_HOENN; other++)
        {
            EXPECT_EQ(GetChampionStateForRegion(other), other <= region);
            EXPECT_EQ(GetGameClearStateForRegion(other), other <= region);
            for (badge = 0; badge < 8; badge++)
                EXPECT(GetBadgeStateForRegion(other, badge));
        }
        EXPECT(FlagGet(HOENN_FLAG_ID(0x52)));
        EXPECT(FlagGet(0x52));
        EXPECT_EQ(VarGet(HOENN_VAR_ID(0x4001)), 1234);
        EXPECT_EQ(VarGet(0x4001), 5678);
    }
}

TEST("League circuit clear handoff is one-shot and ignores rejected clears")
{
    ConsumeRecordedLeagueClearRegion();
    SetTotalBadges(24);
    EXPECT(Test_CompleteAndRecordLeague(REGION_KANTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
    EXPECT(Test_CompleteAndRecordLeague(REGION_JOHTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_JOHTO);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
}

#if WAYFARER_LEAGUE_CIRCUIT_ENABLED

TEST("League run admission captures current producer once and rejects absent out of context state")
{
    struct WarpData source, destination;
    u8 rating = 255;

    WayfarerInitPersistentState();
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, FALSE);
    SetTotalBadges(7);
    EXPECT(!Test_AdmitLeagueRun(REGION_KANTO));
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, FALSE);
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS));
    SetTotalBadges(8);
    EXPECT(Test_AdmitLeagueRun(REGION_KANTO));
    EXPECT(GetLeagueRunBattleRating(REGION_KANTO, 0, &rating));
    EXPECT_EQ(rating, GetTrainerRating());
    EXPECT_EQ(rating, 40);
    EXPECT(!TryRecordLeagueClear(REGION_KANTO));
    EXPECT(!GetChampionStateForRegion(REGION_KANTO));
    SetTotalBadges(24);
    EXPECT_EQ(GetTrainerRating(), 56);
    EXPECT(GetLeagueRunBattleRating(REGION_KANTO, 0, &rating));
    EXPECT_EQ(rating, 40);
    source = destination = gSaveBlock1Ptr->location;
    LeagueRunHandleWarp(&source, &destination);
    gSaveBlock1Ptr->location = destination;
    EXPECT(GetLeagueRunBattleRating(REGION_KANTO, 0, &rating));
    EXPECT_EQ(rating, 40);
    EXPECT(!GetLeagueRunBattleRating(REGION_JOHTO, 0, &rating));
    EXPECT(!GetLeagueRunBattleRating(REGION_KANTO, 1, &rating));
    EndLeagueRun();
    EXPECT(!GetLeagueRunBattleRating(REGION_KANTO, 0, &rating));
}

TEST("League saved run validation preserves rating and defeated rooms across both venues")
{
    enum Region region;
    struct LeagueRunState saved;
    u8 rating;

    WayfarerInitPersistentState();
    SetTotalBadges(24);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT(Test_AdmitLeagueRun(region));
        saved = gSaveBlock3Ptr->wayfarerHoenn.leagueRun;
        if (region == REGION_HOENN)
        {
            FlagSet(HOENN_FLAG_ID(0x4FB));
            VarSet(HOENN_VAR_ID(0x409C), 1);
        }
        else
            VarSet(VAR_LEAGUE_STATE, 2);
        EXPECT(IsCurrentLeagueRoomDefeated());
        WayfarerValidatePersistentState();
        EXPECT(IsCurrentLeagueRoomDefeated());
        EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, saved.active);
        EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.region, saved.region);
        EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry, saved.ratingAtEntry);
        EXPECT(!GetLeagueRunBattleRating(region, 0, &rating));
        Test_CompleteLeagueRooms(region);
        EXPECT(TryRecordLeagueClear(region));
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), region);
        EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, FALSE);
        EXPECT(!TryRecordLeagueClear(region));
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
    }
}

TEST("League run production save and load preserve entry rating and room progression")
{
    enum Region region;
    u8 ratingAtEntry, ratingAfterLoad;
    u8 loadStatus;
    u16 stateVar;

    CheckForFlashMemory();
    if (gFlashMemoryPresent != TRUE)
    {
        gFlashMemoryPresent = TRUE;
        InitFlashTimer();
    }
    ASSUME(gPokemonStoragePtr != NULL);
    ClearSaveData();
    Save_ResetSaveCounters();
    gSaveBlock1Ptr->saveVersionMagic = SAVE_VERSION_MAGIC;
    gSaveBlock1Ptr->saveVersion = SAVE_VERSION;
    WayfarerInitPersistentState();
    SetTotalBadges(24);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT(Test_AdmitLeagueRun(region));
        ratingAtEntry = gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry;
        stateVar = region == REGION_HOENN ? HOENN_VAR_ID(0x409C) : VAR_LEAGUE_STATE;
        if (region == REGION_HOENN)
        {
            FlagSet(HOENN_FLAG_ID(0x4FB));
            VarSet(stateVar, 1);
            Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_EVER_GRANDE_CITY_PHOEBES_ROOM);
        }
        else
        {
            VarSet(stateVar, 2);
            Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_POKEMON_LEAGUE_KOGAS_ROOM_HNS);
        }
        HandleSavingData(SAVE_NORMAL);
        ClearSav1();
        ClearSav2();
        ClearSav3();
        loadStatus = LoadGameSave(SAVE_NORMAL);
        EXPECT_EQ(loadStatus, SAVE_STATUS_OK);
        EXPECT_EQ(GetActiveLeagueRunRegion(), region);
        EXPECT(GetLeagueRunBattleRating(region, 1, &ratingAfterLoad));
        EXPECT_EQ(ratingAfterLoad, ratingAtEntry);
        EXPECT_EQ(VarGet(stateVar), region == REGION_HOENN ? 1 : 2);
        EXPECT(!ConsumeLeagueRunLoadRecovery());
        Test_CompleteLeagueRooms(region);
        EXPECT(TryRecordLeagueClear(region));
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), region);
    }
    ClearSaveData();
    Save_ResetSaveCounters();
}

TEST("League loss and departure discard entry TR and reset only their venue")
{
    struct WarpData source, destination;
    u8 rating;

    SetTotalBadges(8);
    EXPECT(Test_AdmitLeagueRun(REGION_KANTO));
    VarSet(VAR_LEAGUE_STATE, 3);
    VarSet(HOENN_VAR_ID(0x409C), 123);
    EndLeagueRun();
    EXPECT_EQ(VarGet(VAR_LEAGUE_STATE), 1);
    EXPECT_EQ(VarGet(HOENN_VAR_ID(0x409C)), 123);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, FALSE);
    SetTotalBadges(24);
    EXPECT(Test_AdmitLeagueRun(REGION_KANTO));
    EXPECT(GetLeagueRunBattleRating(REGION_KANTO, 0, &rating));
    EXPECT_EQ(rating, 56);
    source = gSaveBlock1Ptr->location;
    Test_SetLeagueMap(&destination, MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS);
    LeagueRunHandleWarp(&source, &destination);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, FALSE);
    EXPECT_EQ(VarGet(VAR_LEAGUE_STATE), 1);
    EXPECT(!GetChampionStateForRegion(REGION_KANTO));
}

TEST("League load rejects invalid or missing record in every room without an award")
{
    static const u16 rooms[] = {
        MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS, MAP_POKEMON_LEAGUE_KOGAS_ROOM_HNS,
        MAP_POKEMON_LEAGUE_BRUNOS_ROOM_HNS, MAP_POKEMON_LEAGUE_KARENS_ROOM_HNS,
        MAP_POKEMON_LEAGUE_CHAMPIONS_ROOM_HNS, MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS,
        MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM, MAP_EVER_GRANDE_CITY_PHOEBES_ROOM,
        MAP_EVER_GRANDE_CITY_GLACIAS_ROOM, MAP_EVER_GRANDE_CITY_DRAKES_ROOM,
        MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM, MAP_EVER_GRANDE_CITY_HALL_OF_FAME,
        MAP_EVER_GRANDE_CITY_HALL1, MAP_EVER_GRANDE_CITY_HALL2,
        MAP_EVER_GRANDE_CITY_HALL3, MAP_EVER_GRANDE_CITY_HALL4,
    };
    u8 i, corruption;
    u16 lobby;
    for (i = 0; i < ARRAY_COUNT(rooms); i++)
    {
        lobby = i < 6 ? MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS : MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F;
        for (corruption = 0; corruption < 4; corruption++)
        {
            WayfarerInitPersistentState();
            Test_SetLeagueMap(&gSaveBlock1Ptr->location, rooms[i]);
            gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = corruption == 0 ? FALSE : TRUE;
            gSaveBlock3Ptr->wayfarerHoenn.leagueRun.region = corruption == 1 ? 255 : REGION_KANTO;
            gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry = corruption == 2 ? 81 : 40;
            VarSet(VAR_LEAGUE_STATE, 99);
            VarSet(HOENN_VAR_ID(0x409C), 99);
            WayfarerValidatePersistentState();
            EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, FALSE);
            EXPECT(ConsumeLeagueRunLoadRecovery());
            EXPECT(!ConsumeLeagueRunLoadRecovery());
            EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(lobby));
            EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(lobby));
            EXPECT_EQ(VarGet(i < 6 ? VAR_LEAGUE_STATE : HOENN_VAR_ID(0x409C)), i < 6 ? 1 : 0);
            EXPECT(!GetChampionStateForRegion(REGION_KANTO));
            EXPECT(!GetChampionStateForRegion(REGION_JOHTO));
            EXPECT(!GetChampionStateForRegion(REGION_HOENN));
        }
    }
}

TEST("League run warp validation rejects skipped rooms and preserves ordinary Hoenn halls")
{
    struct WarpData source, destination;
    u8 rating;

    SetTotalBadges(24);
    EXPECT(Test_AdmitLeagueRun(REGION_KANTO));
    source = gSaveBlock1Ptr->location;
    Test_SetLeagueMap(&destination, MAP_POKEMON_LEAGUE_KOGAS_ROOM_HNS);
    LeagueRunHandleWarp(&source, &destination);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, FALSE);
    EXPECT_EQ(destination.mapNum, MAP_NUM(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS));
    SetGameClearStateForRegion(REGION_KANTO, TRUE);
    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    EXPECT(Test_AdmitLeagueRun(REGION_HOENN));
    VarSet(HOENN_VAR_ID(0x409C), 1);
    FlagSet(HOENN_FLAG_ID(0x4FB));
    source = gSaveBlock1Ptr->location;
    Test_SetLeagueMap(&destination, MAP_EVER_GRANDE_CITY_HALL1);
    LeagueRunHandleWarp(&source, &destination);
    gSaveBlock1Ptr->location = destination;
    EXPECT(ValidateActiveLeagueRun());
    EXPECT(!GetLeagueRunBattleRating(REGION_HOENN, 1, &rating));
    source = destination;
    Test_SetLeagueMap(&destination, MAP_EVER_GRANDE_CITY_PHOEBES_ROOM);
    LeagueRunHandleWarp(&source, &destination);
    gSaveBlock1Ptr->location = destination;
    EXPECT(GetLeagueRunBattleRating(REGION_HOENN, 1, &rating));
    EXPECT_EQ(rating, gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry);
    FlagSet(HOENN_FLAG_ID(0x4FE));
    EXPECT(!ValidateActiveLeagueRun());
    WayfarerValidatePersistentState();
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active, FALSE);
    EXPECT_EQ(VarGet(HOENN_VAR_ID(0x409C)), 0);
    EXPECT(!FlagGet(HOENN_FLAG_ID(0x4FB)));
    EXPECT(!FlagGet(HOENN_FLAG_ID(0x4FE)));
}

TEST("League Hoenn Champion completion requires all rooms and survives cleanup")
{
    u8 i;
    SetTotalBadges(24);
    SetGameClearStateForRegion(REGION_KANTO, TRUE);
    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    EXPECT(Test_AdmitLeagueRun(REGION_HOENN));
    EXPECT(!MarkLeagueChampionDefeated());
    Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_EVER_GRANDE_CITY_CHAMPIONS_ROOM);
    VarSet(HOENN_VAR_ID(0x409C), 4);
    EXPECT(!MarkLeagueChampionDefeated());
    for (i = 0; i < 4; i++)
        FlagSet(HOENN_FLAG_ID(0x4FB) + i);
    EXPECT(MarkLeagueChampionDefeated());
    EXPECT(!MarkLeagueChampionDefeated());
    EXPECT(IsCurrentLeagueRoomDefeated());
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));
    Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_EVER_GRANDE_CITY_HALL_OF_FAME);
    EXPECT(TryRecordLeagueClear(REGION_HOENN));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_HOENN);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_NONE);
    EXPECT_EQ(VarGet(HOENN_VAR_ID(0x409C)), 0);
    for (i = 0; i < 4; i++)
        EXPECT(!FlagGet(HOENN_FLAG_ID(0x4FB) + i));
}

extern int GameClear(void);

TEST("League circuit Hall of Fame consumes each recorded tier and sets its return warp")
{
    MainCallback testCallback = gMain.callback2;
    enum Region region;
    const struct HealLocation *heal;

    SetTotalBadges(24);
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT(Test_CompleteAndRecordLeague(region));
        GameClear();
        SetMainCallback2(testCallback);
        EXPECT(GetGameClearStateForRegion(region));
        EXPECT_EQ(GetGlobalBadgeCount(), 24);
        heal = GetHealLocation(region == REGION_HOENN
            ? HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE : HEAL_LOCATION_INDIGO_PLATEAU_HNS);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.mapGroup, heal->mapGroup);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.mapNum, heal->mapNum);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.x, heal->x);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.y, heal->y);
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
        gSaveBlock3Ptr->wayfarerHoenn.magic = WAYFARER_HOENN_STATE_MAGIC;
        WayfarerValidatePersistentState();
        EXPECT(!ConsumeLeagueRunLoadRecovery());
        EXPECT(UseContinueGameWarp());
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.x, heal->x);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.y, heal->y);
    }
}

TEST("League circuit Hall of Fame rejects a missing clear handoff without state changes")
{
    MainCallback testCallback = gMain.callback2;

    ConsumeRecordedLeagueClearRegion();
    GameClear();
    EXPECT(gMain.callback2 == testCallback);
    EXPECT(!GetChampionStateForRegion(REGION_KANTO));
    EXPECT(!GetChampionStateForRegion(REGION_JOHTO));
    EXPECT(!GetChampionStateForRegion(REGION_HOENN));
}
#endif
#endif
