#include "global.h"
#include "event_data.h"
#include "hall_of_fame.h"
#include "league_circuit.h"
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

TEST("Circuit script specials expose immediate requirement and ordered clears")
{
    WayfarerInitPersistentState();
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
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
    u8 rating;

    WayfarerInitPersistentState();
    FlagClear(FLAG_SYS_GAME_CLEAR);
    SetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME, 0);
    gSaveBlock2Ptr->playTimeHours = 1;
    SetTotalBadges(8);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    rating = CalculateLeagueCircuitTrainerRating();
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    EXPECT(LeagueCircuit_CommitAndRegisterIndigo());
    EXPECT(!HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT_NE(GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME), 0);
    EXPECT(CommitPendingIndigoHallOfFame());
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));

    FinishIndigoHallOfFameSaveTransaction(FALSE);

    EXPECT(!HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT_EQ(GetGameStat(GAME_STAT_FIRST_HOF_PLAY_TIME), 0);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), rating);
    EXPECT_EQ(GetTrainerRating(), rating);
    EXPECT_EQ(GetActiveLeagueRunStage(), CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry, rating);
    EXPECT(CanCompleteCircuitRun(CIRCUIT_STAGE_INDIGO));
    SetMainCallback2(testCallback);
}

TEST("Recoverable Indigo Hall of Fame save failure retains the commit for retry")
{
    MainCallback testCallback = gMain.callback2;
    u32 hallOfFameCount;
    u8 rating;

    WayfarerInitPersistentState();
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    FlagClear(FLAG_SYS_GAME_CLEAR);
    SetTotalBadges(8);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    rating = CalculateLeagueCircuitTrainerRating();
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    EXPECT(LeagueCircuit_CommitAndRegisterIndigo());
    EXPECT(CommitPendingIndigoHallOfFame());
    EXPECT(IsIndigoHallOfFameSaveTransactionActive());
    hallOfFameCount = GetGameStat(GAME_STAT_ENTERED_HOF);
    EXPECT(TryIncrementIndigoHallOfFameSaveCount());
    EXPECT(TryIncrementIndigoHallOfFameSaveCount());
    EXPECT_EQ(GetGameStat(GAME_STAT_ENTERED_HOF), hallOfFameCount + 1);

    ResolveIndigoHallOfFameSaveAttempt(FALSE, TRUE);

    EXPECT(IsIndigoHallOfFameSaveTransactionActive());
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), rating + 8);
    EXPECT_EQ(GetGameStat(GAME_STAT_ENTERED_HOF), hallOfFameCount + 1);

    ResolveIndigoHallOfFameSaveAttempt(TRUE, FALSE);

    EXPECT(!IsIndigoHallOfFameSaveTransactionActive());
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), rating + 8);
    SetMainCallback2(testCallback);
}

TEST("Red completion starts authored credits without changing circuit rewards")
{
    MainCallback testCallback = gMain.callback2;
    u8 rating;

    WayfarerInitPersistentState();
    SetTotalBadges(24);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_MASTERS), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_HOENN), CIRCUIT_COMMIT_FIRST_CLEAR);
    rating = CalculateLeagueCircuitTrainerRating();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_MT_SILVER_SUMMIT_DAY_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_MT_SILVER_SUMMIT_DAY_HNS);

    WayfarerRedGameClear();

    EXPECT_EQ(gMain.callback2, CB2_DoHallOfFameScreen);
    EXPECT_EQ(CalculateLeagueCircuitTrainerRating(), rating);
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO));
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_MASTERS));
    EXPECT(HasClearedCircuitStage(CIRCUIT_STAGE_HOENN));
    EXPECT_EQ(ConsumeRecordedCircuitClearStage(), CIRCUIT_STAGE_HOENN);
    SetMainCallback2(testCallback);
}
#endif
