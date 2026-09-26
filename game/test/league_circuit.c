#include "global.h"
#include "event_data.h"
#include "league_circuit.h"
#include "league_run_helpers.h"
#include "load_save.h"
#include "main.h"
#include "save.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "gba/flash_internal.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"
#include "constants/wayfarer_origin.h"

#if IS_WAYFARER
static void ResetCircuitFacts(void)
{
    u8 region, badge;
    WayfarerInitPersistentState();
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    gSaveBlock3Ptr->wayfarerHoenn.fallbackHealLocation = HEAL_LOCATION_NEW_BARK_TOWN_HNS;
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
        for (badge = 0; badge < 8; badge++)
            SetBadgeStateForRegion(region, badge, FALSE);
    SetTrainerRating(0);
    ConsumeRecordedCircuitClearStage();
}

static void SetTotalBadges(u8 count)
{
    u8 i;
    for (i = 0; i < 24; i++)
        SetBadgeStateForRegion(REGION_KANTO + i / 8, i % 8, i < count);
}

static void SetDistribution(u8 kanto, u8 johto, u8 hoenn)
{
    u8 i;
    for (i = 0; i < 8; i++)
    {
        SetBadgeStateForRegion(REGION_KANTO, i, i < kanto);
        SetBadgeStateForRegion(REGION_JOHTO, i, i < johto);
        SetBadgeStateForRegion(REGION_HOENN, i, i < hoenn);
    }
}

static u16 GetSecondRoom(enum CircuitStage stage)
{
    if (stage == CIRCUIT_STAGE_INDIGO)
        return MAP_POKEMON_LEAGUE_BRUNOS_ROOM;
    if (stage == CIRCUIT_STAGE_MASTERS)
        return MAP_POKEMON_LEAGUE_KOGAS_ROOM_HNS;
    return MAP_EVER_GRANDE_CITY_PHOEBES_ROOM;
}

TEST("Circuit badges aggregate every regional distribution and duplicate awards are idempotent")
{
    u8 kanto, johto, hoenn;
    ResetCircuitFacts();
    for (kanto = 0; kanto <= 8; kanto++)
        for (johto = 0; johto <= 8; johto++)
            for (hoenn = 0; hoenn <= 8; hoenn++)
            {
                SetDistribution(kanto, johto, hoenn);
                EXPECT_EQ(GetGlobalBadgeCount(), kanto + johto + hoenn);
                SetDistribution(kanto, johto, hoenn);
                EXPECT_EQ(GetGlobalBadgeCount(), kanto + johto + hoenn);
            }
}

TEST("Circuit first-clear admission obeys 8 16 24 boundaries and canonical prerequisites")
{
    ResetCircuitFacts();
    SetTotalBadges(7);
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_INDIGO), LEAGUE_ADMISSION_NEEDS_8_BADGES);
    SetTotalBadges(8);
    EXPECT(IsEligibleForCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_MASTERS), LEAGUE_ADMISSION_NEEDS_INDIGO_CLEAR);

    // Regional title projection cannot substitute for the canonical Indigo clear.
    SetGameClearStateForRegion(REGION_KANTO, TRUE);
    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    EXPECT(!HasCommittedFirstIndigoVictory());
    EXPECT_EQ(GetRequiredCircuitStage(), CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_MASTERS), LEAGUE_ADMISSION_NEEDS_INDIGO_CLEAR);
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);

    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT(HasCommittedFirstIndigoVictory());
    EXPECT(GetChampionStateForRegion(REGION_KANTO));
    EXPECT(GetChampionStateForRegion(REGION_JOHTO));
    EXPECT_EQ(GetRequiredCircuitStage(), CIRCUIT_STAGE_MASTERS);
    SetTotalBadges(15);
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_MASTERS), LEAGUE_ADMISSION_NEEDS_16_BADGES);
    SetTotalBadges(16);
    EXPECT(IsEligibleForCircuitStage(CIRCUIT_STAGE_MASTERS));
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_MASTERS), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(GetRequiredCircuitStage(), CIRCUIT_STAGE_HOENN);
    EXPECT(!GetChampionStateForRegion(REGION_HOENN));
    SetTotalBadges(23);
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_HOENN), LEAGUE_ADMISSION_NEEDS_24_BADGES);
    SetTotalBadges(24);
    EXPECT(IsEligibleForCircuitStage(CIRCUIT_STAGE_HOENN));
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_HOENN), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(GetRequiredCircuitStage(), CIRCUIT_STAGE_NONE);
    EXPECT(GetChampionStateForRegion(REGION_HOENN));
}

TEST("All 24 badges may precede every circuit clear and Rating rises eight per first clear")
{
    ResetCircuitFacts();
    SetTotalBadges(24);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), 56);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), 64);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_MASTERS), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), 72);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_HOENN), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), 80);
    EXPECT_EQ(ConsumeRecordedCircuitClearStage(), CIRCUIT_STAGE_HOENN);
    EXPECT_EQ(ConsumeRecordedCircuitClearStage(), CIRCUIT_STAGE_NONE);
}

TEST("Cleared venue replay captures fresh Rating and commits without another reward")
{
    u8 rating;
    ResetCircuitFacts();
    SetTotalBadges(8);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), 48);
    SetTotalBadges(16);
    EXPECT(IsEligibleForCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT_EQ(GetRequiredCircuitStage(), CIRCUIT_STAGE_MASTERS);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    EXPECT(IsActiveLeagueRunReplay());
    EXPECT(GetCircuitRunBattleRating(CIRCUIT_STAGE_INDIGO, 0, &rating));
    EXPECT_EQ(rating, 56);
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(CommitCircuitRun(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_REPLAY);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), 56);
    EXPECT_EQ(GetRequiredCircuitStage(), CIRCUIT_STAGE_MASTERS);
    EXPECT_EQ(ConsumeRecordedCircuitClearStage(), CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(ConsumeRecordedCircuitClearStage(), CIRCUIT_STAGE_NONE);
}

TEST("Room victory validation uses stage, room and saved Rating snapshot")
{
    static const enum CircuitStage stages[] = {
        CIRCUIT_STAGE_INDIGO, CIRCUIT_STAGE_MASTERS, CIRCUIT_STAGE_HOENN,
    };
    u8 i, rating, firstRating;
    ResetCircuitFacts();
    SetTotalBadges(24);
    for (i = 0; i < ARRAY_COUNT(stages); i++)
    {
        enum CircuitStage stage = stages[i];
        EXPECT(Test_AdmitCircuitRun(stage));
        firstRating = gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry;
        EXPECT(!ValidateCircuitRoomBattle(stage, 1));
        EXPECT(!ValidateCircuitRoomBattle(stage == CIRCUIT_STAGE_INDIGO
            ? CIRCUIT_STAGE_MASTERS : CIRCUIT_STAGE_INDIGO, 0));
        EXPECT(ValidateCircuitRoomBattle(stage, 0));
        EXPECT(RecordCircuitRoomVictory(stage, 0));
        if (stage == CIRCUIT_STAGE_INDIGO)
            EXPECT_EQ(VarGet(VAR_LEAGUE_STATE), 1);
        EXPECT(!RecordCircuitRoomVictory(stage, 0));
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, GetSecondRoom(stage));
        EXPECT(GetCircuitRunBattleRating(stage, 1, &rating));
        EXPECT_EQ(rating, firstRating);
        Test_CompleteCircuitRooms(stage);
        EXPECT(CanCompleteCircuitRun(stage));
        EXPECT_EQ(CommitCircuitRun(stage), CIRCUIT_COMMIT_FIRST_CLEAR);
        EXPECT(!CanCompleteCircuitRun(stage));
        EXPECT_EQ(CommitCircuitRun(stage), CIRCUIT_COMMIT_INVALID);
    }
}

TEST("Production save and load preserve stage, mode, Rating and room progress")
{
    u8 loadStatus, rating;
    struct LeagueRunState saved;
    ResetCircuitFacts();
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
    SetTotalBadges(8);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    saved = gSaveBlock3Ptr->wayfarerHoenn.leagueRun;
    EXPECT(RecordCircuitRoomVictory(CIRCUIT_STAGE_INDIGO, 0));
    Test_SetLeagueMap(&gSaveBlock1Ptr->location, MAP_POKEMON_LEAGUE_BRUNOS_ROOM);
    HandleSavingData(SAVE_NORMAL);
    ClearSav1();
    ClearSav2();
    ClearSav3();
    loadStatus = LoadGameSave(SAVE_NORMAL);
    EXPECT_EQ(loadStatus, SAVE_STATUS_OK);
    EXPECT_EQ(GetActiveLeagueRunStage(), CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.replay, saved.replay);
    EXPECT(GetCircuitRunBattleRating(CIRCUIT_STAGE_INDIGO, 1, &rating));
    EXPECT_EQ(rating, saved.ratingAtEntry);
    EXPECT(!ConsumeLeagueRunLoadRecovery());
    ClearSaveData();
    Save_ResetSaveCounters();
}

TEST("Invalid saved run in each venue recovers to its own lobby")
{
    static const u16 roomMaps[] = {
        MAP_POKEMON_LEAGUE_LORELEIS_ROOM,
        MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS,
        MAP_EVER_GRANDE_CITY_SIDNEYS_ROOM,
    };
    static const u16 lobbyMaps[] = {
        MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS,
        MAP_SEVEN_ISLAND_HOUSE_ROOM1,
        MAP_EVER_GRANDE_CITY_POKEMON_LEAGUE_1F,
    };
    u8 i;
    ResetCircuitFacts();
    for (i = 0; i < ARRAY_COUNT(roomMaps); i++)
    {
        Test_SetLeagueMap(&gSaveBlock1Ptr->location, roomMaps[i]);
        LeagueRunValidateSavedLocation();
        EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(lobbyMaps[i]));
        EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(lobbyMaps[i]));
        EXPECT(ConsumeLeagueRunLoadRecovery());
        EXPECT_EQ(GetActiveLeagueRunStage(), CIRCUIT_STAGE_NONE);
    }
}

TEST("Masters rooms retain Kanto Seven Island context without an active run")
{
    struct WarpData recovery;

    ResetCircuitFacts();
    EXPECT(IsWayfarerMastersCircuitMap(MAP_GROUP(MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS),
                                       MAP_NUM(MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS)));
    EXPECT_EQ(WayfarerGetRegionForMap(MAP_GROUP(MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS),
                                      MAP_NUM(MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS)),
              REGION_KANTO);
    EXPECT(SetWayfarerMastersHouseWarpForMap(MAP_GROUP(MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS),
                                             MAP_NUM(MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS),
                                             &recovery));
    EXPECT_EQ(recovery.mapGroup, MAP_GROUP(MAP_SEVEN_ISLAND_HOUSE_ROOM1));
    EXPECT_EQ(recovery.mapNum, MAP_NUM(MAP_SEVEN_ISLAND_HOUSE_ROOM1));
    EXPECT_EQ(recovery.warpId, 0);
    EXPECT(!SetWayfarerMastersHouseWarpForMap(MAP_GROUP(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS),
                                              MAP_NUM(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS),
                                              &recovery));
}

TEST("Circuit room field exits return through their owning lobby")
{
    struct WarpData recovery;

    ResetCircuitFacts();
    EXPECT(SetWayfarerCircuitLobbyWarpForMap(MAP_GROUP(MAP_POKEMON_LEAGUE_LORELEIS_ROOM),
                                             MAP_NUM(MAP_POKEMON_LEAGUE_LORELEIS_ROOM),
                                             &recovery));
    EXPECT_EQ(recovery.mapGroup, MAP_GROUP(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS));
    EXPECT_EQ(recovery.mapNum, MAP_NUM(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS));
    EXPECT(SetWayfarerCircuitLobbyWarpForMap(MAP_GROUP(MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS),
                                             MAP_NUM(MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS),
                                             &recovery));
    EXPECT_EQ(recovery.mapGroup, MAP_GROUP(MAP_SEVEN_ISLAND_HOUSE_ROOM1));
    EXPECT_EQ(recovery.mapNum, MAP_NUM(MAP_SEVEN_ISLAND_HOUSE_ROOM1));
}

TEST("Masters antechamber preserves admission but Will backtracking abandons to Room 1")
{
    struct WarpData source;
    struct WarpData destination;

    ResetCircuitFacts();
    SetTotalBadges(16);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_MASTERS));

    Test_SetLeagueMap(&source, MAP_SEVEN_ISLAND_HOUSE_ROOM1);
    Test_SetLeagueMap(&destination, MAP_SEVEN_ISLAND_HOUSE_ROOM2);
    LeagueRunHandleWarp(&source, &destination);
    EXPECT_EQ(GetActiveLeagueRunStage(), CIRCUIT_STAGE_MASTERS);
    EXPECT_EQ(destination.mapNum, MAP_NUM(MAP_SEVEN_ISLAND_HOUSE_ROOM2));

    Test_SetLeagueMap(&source, MAP_POKEMON_LEAGUE_WILLS_ROOM_HNS);
    Test_SetLeagueMap(&destination, MAP_SEVEN_ISLAND_HOUSE_ROOM2);
    LeagueRunHandleWarp(&source, &destination);
    EXPECT_EQ(GetActiveLeagueRunStage(), CIRCUIT_STAGE_NONE);
    EXPECT_EQ(destination.mapGroup, MAP_GROUP(MAP_SEVEN_ISLAND_HOUSE_ROOM1));
    EXPECT_EQ(destination.mapNum, MAP_NUM(MAP_SEVEN_ISLAND_HOUSE_ROOM1));
}
#endif
