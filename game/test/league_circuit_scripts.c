#include "global.h"
#include "config/notable_trainers.h"
#include "event_data.h"
#include "hall_of_fame.h"
#include "league_circuit.h"
#include "league_events.h"
#include "league_run_helpers.h"
#include "main.h"
#include "overworld.h"
#include "string_util.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "config/league_circuit.h"

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
extern const u8 LeagueCircuit_Text_NeedEightBadges[];
extern const u8 LeagueCircuit_Text_NeedIndigoClear[];
extern const u8 LeagueCircuit_Text_NeedSixteenBadges[];
extern const u8 LeagueCircuit_Text_NeedMastersClear[];
extern const u8 LeagueCircuit_Text_NeedTwentyFourBadges[];
extern const u8 LeagueCircuit_Text_NeedQualification[];
extern const u8 LeagueCircuit_Text_NeedRegionalBadge[];
extern const u8 LeagueCircuit_Text_NeedMaster[];
extern const u8 LeagueCircuit_Text_EventElsewhere[];
extern u16 LeagueCircuit_GetRequiredStage(void);
extern u16 LeagueCircuit_IsEligible(void);
extern u16 LeagueCircuit_GetAdmissionRequirement(void);
extern u16 LeagueCircuit_BufferAdmissionDenial(void);
extern u16 LeagueCircuit_CommitRunClear(void);
extern u16 LeagueCircuit_IsComplete(void);
extern u16 LeagueCircuit_CommitAndRegisterIndigo(void);
extern int WayfarerRedGameClear(void);

static void SetTotalBadges(u8 count)
{
    u8 i;
    for (i = 0; i < 24; i++)
        SetBadgeStateForRegion(REGION_KANTO + i / 8, i % 8, i < count);
}

static void ExpectDenial(enum CircuitStage stage, enum LeagueAdmissionRequirement requirement, const u8 *text)
{
    gSpecialVar_0x8004 = stage;
    EXPECT_EQ(LeagueCircuit_GetAdmissionRequirement(), requirement);
    EXPECT_EQ(LeagueCircuit_BufferAdmissionDenial(), requirement);
    EXPECT_EQ(StringCompare(gStringVar4, text), 0);
}

TEST("Circuit script specials expose current admission and independent league clears")
{
    WayfarerInitPersistentState();
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
#if WAYFARER_LEAGUE_EVENTS
    SetTrainerRating(79);
    SetTotalBadges(7);
    EXPECT_EQ(LeagueCircuit_GetRequiredStage(), CIRCUIT_STAGE_INDIGO);
    ExpectDenial(CIRCUIT_STAGE_INDIGO, LEAGUE_ADMISSION_NEEDS_QUALIFICATION, LeagueCircuit_Text_NeedQualification);
    SetTrainerRating(80);
    ExpectDenial(CIRCUIT_STAGE_HOENN, LEAGUE_ADMISSION_NEEDS_REGIONAL_BADGE, LeagueCircuit_Text_NeedRegionalBadge);
    ExpectDenial(CIRCUIT_STAGE_MASTERS, LEAGUE_ADMISSION_NEEDS_MASTER, LeagueCircuit_Text_NeedMaster);
    gSpecialVar_0x8004 = CIRCUIT_STAGE_INDIGO;
    EXPECT(LeagueCircuit_IsEligible());
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    ExpectDenial(CIRCUIT_STAGE_HOENN, LEAGUE_ADMISSION_EVENT_ELSEWHERE, LeagueCircuit_Text_EventElsewhere);
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    gSpecialVar_0x8004 = CIRCUIT_STAGE_INDIGO;
    EXPECT_EQ(LeagueCircuit_CommitRunClear(), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(LeagueCircuit_GetRequiredStage(), CIRCUIT_STAGE_HOENN);
    SetTotalBadges(24);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_HOENN));
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_HOENN);
    gSpecialVar_0x8004 = CIRCUIT_STAGE_HOENN;
    EXPECT_EQ(LeagueCircuit_CommitRunClear(), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(LeagueCircuit_GetRequiredStage(), CIRCUIT_STAGE_MASTERS);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_MASTERS));
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_MASTERS);
    gSpecialVar_0x8004 = CIRCUIT_STAGE_MASTERS;
    EXPECT_EQ(LeagueCircuit_CommitRunClear(), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT(LeagueCircuit_IsComplete());
#else
    SetTotalBadges(7);
    EXPECT_EQ(LeagueCircuit_GetRequiredStage(), CIRCUIT_STAGE_INDIGO);
    ExpectDenial(CIRCUIT_STAGE_INDIGO, LEAGUE_ADMISSION_NEEDS_8_BADGES, LeagueCircuit_Text_NeedEightBadges);
    ExpectDenial(CIRCUIT_STAGE_MASTERS, LEAGUE_ADMISSION_NEEDS_INDIGO_CLEAR, LeagueCircuit_Text_NeedIndigoClear);
    ExpectDenial(CIRCUIT_STAGE_HOENN, LEAGUE_ADMISSION_NEEDS_INDIGO_CLEAR, LeagueCircuit_Text_NeedIndigoClear);

    SetTotalBadges(8);
    gSpecialVar_0x8004 = CIRCUIT_STAGE_INDIGO;
    EXPECT(LeagueCircuit_IsEligible());
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(LeagueCircuit_CommitRunClear(), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(LeagueCircuit_GetRequiredStage(), CIRCUIT_STAGE_MASTERS);
    SetTotalBadges(15);
    ExpectDenial(CIRCUIT_STAGE_MASTERS, LEAGUE_ADMISSION_NEEDS_16_BADGES, LeagueCircuit_Text_NeedSixteenBadges);
    SetTotalBadges(16);
    ExpectDenial(CIRCUIT_STAGE_HOENN, LEAGUE_ADMISSION_NEEDS_MASTERS_CLEAR, LeagueCircuit_Text_NeedMastersClear);
    gSpecialVar_0x8004 = CIRCUIT_STAGE_MASTERS;
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_MASTERS));
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_MASTERS);
    EXPECT_EQ(LeagueCircuit_CommitRunClear(), CIRCUIT_COMMIT_FIRST_CLEAR);
    SetTotalBadges(23);
    ExpectDenial(CIRCUIT_STAGE_HOENN, LEAGUE_ADMISSION_NEEDS_24_BADGES, LeagueCircuit_Text_NeedTwentyFourBadges);
    SetTotalBadges(24);
    gSpecialVar_0x8004 = CIRCUIT_STAGE_HOENN;
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_HOENN));
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_HOENN);
    EXPECT_EQ(LeagueCircuit_CommitRunClear(), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT(LeagueCircuit_IsComplete());
#endif
}

TEST("Circuit script replay result differs from invalid completion")
{
    WayfarerInitPersistentState();
    SetTotalBadges(8);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    gSpecialVar_0x8004 = CIRCUIT_STAGE_INDIGO;
    EXPECT_EQ(LeagueCircuit_CommitRunClear(), CIRCUIT_COMMIT_REPLAY);
    EXPECT_EQ(LeagueCircuit_CommitRunClear(), CIRCUIT_COMMIT_INVALID);
}

TEST("Failed Indigo Hall of Fame save rolls back the clear and reward")
{
    MainCallback testCallback = gMain.callback2;
    u32 rating;
#if WAYFARER_LEAGUE_EVENTS
    const struct LeagueSavedTeam *team;
    u16 firstSpecies, firstCharacter;
    u32 eventId;
    u16 recentBefore[LEAGUE_EVENT_LINEUP_SIZE];
    u16 recentAfter[LEAGUE_EVENT_LINEUP_SIZE];
#endif

    WayfarerInitPersistentState();
    FlagClear(FLAG_SYS_GAME_CLEAR);
    SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME, 0);
    gSaveBlock2Ptr->playTimeHours = 1;
    SetTotalBadges(8);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
#if WAYFARER_LEAGUE_EVENTS
    eventId = GetAcceptedLeagueEventId();
    EXPECT(GetAcceptedLeagueEventMember(0, &team));
    firstCharacter = team->characterId;
    firstSpecies = team->members[team->battleOrder[0]].species;
    GetLeagueRecentLineup(recentBefore);
#endif
    rating = CalculateLeagueCircuitTrainerRating();
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    EXPECT(LeagueCircuit_CommitAndRegisterIndigo());
    EXPECT(!HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT_NE(GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME), 0);
    EXPECT(CommitPendingIndigoHallOfFame());
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
#if WAYFARER_LEAGUE_EVENTS
    EXPECT_EQ(GetAcceptedLeagueEventId(), 0);
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), LEAGUE_EVENT_PLAYER_CHAMPION);
#endif

    FinishIndigoHallOfFameSaveTransaction(FALSE);

    EXPECT(!HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT_EQ(GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME), 0);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), rating);
    EXPECT_EQ(GetTrainerRating(), rating);
    EXPECT_EQ(GetActiveLeagueRunStage(), CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry, rating);
    EXPECT(CanCompleteCircuitRun(CIRCUIT_STAGE_INDIGO));
#if WAYFARER_LEAGUE_EVENTS
    EXPECT_EQ(GetAcceptedLeagueEventId(), eventId);
    EXPECT(GetAcceptedLeagueEventMember(0, &team));
    EXPECT_EQ(team->characterId, firstCharacter);
    EXPECT_EQ(team->members[team->battleOrder[0]].species, firstSpecies);
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), 0);
    GetLeagueRecentLineup(recentAfter);
    EXPECT_EQ(memcmp(recentBefore, recentAfter, sizeof(recentBefore)), 0);
    EXPECT(ValidateLeagueEventState());
#endif
    SetMainCallback2(testCallback);
}

TEST("Recoverable Indigo Hall of Fame save failure retains the commit for retry")
{
    MainCallback testCallback = gMain.callback2;
    u32 hallOfFameCount;
    u32 rating;
#if WAYFARER_LEAGUE_EVENTS
    u16 recent[LEAGUE_EVENT_LINEUP_SIZE];
    u16 lineup[LEAGUE_EVENT_LINEUP_SIZE];
    const struct LeagueSavedTeam *team;
    u8 i;
#endif

    WayfarerInitPersistentState();
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    FlagClear(FLAG_SYS_GAME_CLEAR);
    SetTotalBadges(8);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
#if WAYFARER_LEAGUE_EVENTS
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
    {
        EXPECT(GetAcceptedLeagueEventMember(i, &team));
        lineup[i] = team->characterId;
    }
#endif
    rating = CalculateLeagueCircuitTrainerRating();
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    EXPECT(LeagueCircuit_CommitAndRegisterIndigo());
    EXPECT(CommitPendingIndigoHallOfFame());
    EXPECT(IsIndigoHallOfFameSaveTransactionActive());
    hallOfFameCount = GetGameStat(GAME_STAT_ENTERED_HOF);
    EXPECT(TryIncrementIndigoHallOfFameSaveCount());
    EXPECT(TryIncrementIndigoHallOfFameSaveCount());
    EXPECT_EQ(GetGameStat(GAME_STAT_ENTERED_HOF), hallOfFameCount + 1);
#if WAYFARER_LEAGUE_EVENTS
    EXPECT_EQ(GetAcceptedLeagueEventId(), 0);
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), LEAGUE_EVENT_PLAYER_CHAMPION);
    GetLeagueRecentLineup(recent);
    EXPECT_EQ(memcmp(recent, lineup, sizeof(recent)), 0);
#endif

    ResolveIndigoHallOfFameSaveAttempt(FALSE, TRUE);

    EXPECT(IsIndigoHallOfFameSaveTransactionActive());
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), rating +
#if WAYFARER_V0_TRAINERS
              0);
#else
              8);
#endif
    EXPECT_EQ(GetGameStat(GAME_STAT_ENTERED_HOF), hallOfFameCount + 1);
#if WAYFARER_LEAGUE_EVENTS
    EXPECT_EQ(GetAcceptedLeagueEventId(), 0);
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), LEAGUE_EVENT_PLAYER_CHAMPION);
    GetLeagueRecentLineup(recent);
    EXPECT_EQ(memcmp(recent, lineup, sizeof(recent)), 0);
    EXPECT(ValidateLeagueEventState());
#endif

    ResolveIndigoHallOfFameSaveAttempt(TRUE, FALSE);

    EXPECT(!IsIndigoHallOfFameSaveTransactionActive());
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), rating +
#if WAYFARER_V0_TRAINERS
              0);
#else
              8);
#endif
#if WAYFARER_LEAGUE_EVENTS
    EXPECT_EQ(GetAcceptedLeagueEventId(), 0);
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), LEAGUE_EVENT_PLAYER_CHAMPION);
    GetLeagueRecentLineup(recent);
    EXPECT_EQ(memcmp(recent, lineup, sizeof(recent)), 0);
#endif
    SetMainCallback2(testCallback);
}

TEST("Red completion starts authored credits without changing circuit rewards")
{
    MainCallback testCallback = gMain.callback2;
    u32 rating;

    WayfarerInitPersistentState();
    SetTotalBadges(24);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
#if WAYFARER_LEAGUE_EVENTS
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_HOENN), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_MASTERS), CIRCUIT_COMMIT_FIRST_CLEAR);
#else
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_MASTERS), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_HOENN), CIRCUIT_COMMIT_FIRST_CLEAR);
#endif
    rating = CalculateLeagueCircuitTrainerRating();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_MT_SILVER_SUMMIT_DAY_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_MT_SILVER_SUMMIT_DAY_HNS);

    WayfarerRedGameClear();

    EXPECT_EQ(gMain.callback2, CB2_DoHallOfFameScreen);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), rating);
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_MASTERS));
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_HOENN));
    EXPECT_EQ(ConsumeRecordedCircuitClearStage(),
#if WAYFARER_LEAGUE_EVENTS
              CIRCUIT_STAGE_MASTERS);
#else
              CIRCUIT_STAGE_HOENN);
#endif
    SetMainCallback2(testCallback);
}
#endif
