#ifndef GUARD_LEAGUE_CIRCUIT_H
#define GUARD_LEAGUE_CIRCUIT_H

#include "global.h"
#include "constants/regions.h"

#if IS_WAYFARER
enum CircuitStage
{
    CIRCUIT_STAGE_NONE,
    CIRCUIT_STAGE_INDIGO,
    CIRCUIT_STAGE_MASTERS,
    CIRCUIT_STAGE_HOENN,
};

enum LeagueAdmissionRequirement
{
    LEAGUE_ADMISSION_AVAILABLE,
    LEAGUE_ADMISSION_NEEDS_8_BADGES,
    LEAGUE_ADMISSION_NEEDS_INDIGO_CLEAR,
    LEAGUE_ADMISSION_NEEDS_16_BADGES,
    LEAGUE_ADMISSION_NEEDS_MASTERS_CLEAR,
    LEAGUE_ADMISSION_NEEDS_24_BADGES,
    LEAGUE_ADMISSION_UNAVAILABLE,
    LEAGUE_ADMISSION_NEEDS_QUALIFICATION,
    LEAGUE_ADMISSION_NEEDS_REGIONAL_BADGE,
    LEAGUE_ADMISSION_NEEDS_MASTER,
    LEAGUE_ADMISSION_EVENT_ELSEWHERE,
    LEAGUE_ADMISSION_NO_ACCEPTED_EVENT,
};

enum CircuitCommitResult
{
    CIRCUIT_COMMIT_INVALID,
    CIRCUIT_COMMIT_REPLAY,
    CIRCUIT_COMMIT_FIRST_CLEAR,
};

enum CircuitStage GetActiveLeagueRunStage(void);
u16 GetCircuitStageRoom(enum CircuitStage stage, u8 match);
bool8 IsActiveLeagueRunReplay(void);
bool32 GetCircuitRunBattleRating(enum CircuitStage stage, u32 encounterIndex, u32 *rating);
bool8 ValidateCircuitRoomBattle(enum CircuitStage stage, u8 encounterIndex);
s8 GetCurrentLeagueEventMatch(void);
bool8 RecordCircuitRoomVictory(enum CircuitStage stage, u8 encounterIndex);
bool8 CanCompleteCircuitRun(enum CircuitStage stage);
bool8 BeginCircuitRun(enum CircuitStage stage);
enum CircuitCommitResult CommitCircuitRun(enum CircuitStage stage);
bool8 CommitPendingIndigoHallOfFame(void);
void RollbackIndigoHallOfFameCommit(u32 ratingAtEntry, u32 storedRatingBefore);
bool8 IsIndigoHallOfFameSaveTransactionActive(void);
void FinishIndigoHallOfFameSaveTransaction(bool8 success);
void ResolveIndigoHallOfFameSaveAttempt(bool8 success, bool8 retryPending);
bool8 TryIncrementIndigoHallOfFameSaveCount(void);
bool8 TryRecordCircuitClear(enum CircuitStage stage);
bool8 HasClearedCircuitStage(enum CircuitStage stage);
bool8 HasCommittedFirstIndigoVictory(void);
enum CircuitStage GetRequiredCircuitStage(void);
enum LeagueAdmissionRequirement GetCircuitAdmissionRequirement(enum CircuitStage stage);
bool8 IsEligibleForCircuitStage(enum CircuitStage stage);

// Region adapters are kept for existing script and test consumers.
enum Region GetActiveLeagueRunRegion(void);
bool8 ValidateActiveLeagueRun(void);
bool32 GetLeagueRunBattleRating(u32 region, u32 encounterIndex, u32 *rating);
void EndLeagueRun(void);
void ResetLeagueCircuitTransientState(void);
bool8 SavePendingLeagueProgress(void);
void AcknowledgeLeagueProgressSave(void);
void LeagueRunHandleWarp(const struct WarpData *source, struct WarpData *destination);
void LeagueRunValidateSavedLocation(void);
bool8 ConsumeLeagueRunLoadRecovery(void);
bool8 MarkLeagueChampionDefeated(void);
bool8 IsCurrentLeagueRoomDefeated(void);
bool8 IsWayfarerMastersCircuitMap(s16 mapGroup, s16 mapNum);
bool8 SetWayfarerMastersHouseWarpForMap(s16 mapGroup, s16 mapNum, struct WarpData *warp);
bool8 SetWayfarerCircuitLobbyWarpForMap(s16 mapGroup, s16 mapNum, struct WarpData *warp);

u8 GetGlobalBadgeCount(void);
enum Region GetRequiredLeagueRegion(void);
enum LeagueAdmissionRequirement GetLeagueAdmissionRequirement(enum Region region);
bool8 IsEligibleForLeague(enum Region region);
bool8 TryRecordLeagueClear(enum Region region);
enum Region ConsumeRecordedLeagueClearRegion(void);
enum CircuitStage ConsumeRecordedCircuitClearStage(void);
u32 CalculateLeagueCircuitTrainerRating(void);
#if TESTING || defined(E2E_TESTING)
void SetCircuitClearForTesting(enum CircuitStage stage, bool8 value);
#endif
#endif

#endif
