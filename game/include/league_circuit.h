#ifndef GUARD_LEAGUE_CIRCUIT_H
#define GUARD_LEAGUE_CIRCUIT_H

#include "global.h"
#include "constants/regions.h"

#if IS_WAYFARER
enum LeagueAdmissionRequirement
{
    LEAGUE_ADMISSION_AVAILABLE,
    LEAGUE_ADMISSION_NEEDS_8_BADGES,
    LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR,
    LEAGUE_ADMISSION_NEEDS_16_BADGES,
    LEAGUE_ADMISSION_NEEDS_JOHTO_CLEAR,
    LEAGUE_ADMISSION_NEEDS_24_BADGES,
    LEAGUE_ADMISSION_UNAVAILABLE,
};

enum Region GetActiveLeagueRunRegion(void);
bool8 ValidateActiveLeagueRun(void);
bool32 GetLeagueRunBattleRating(u32 region, u32 encounterIndex, u8 *rating);
void EndLeagueRun(void);
void LeagueRunHandleWarp(const struct WarpData *source, struct WarpData *destination);
void LeagueRunValidateSavedLocation(void);
bool8 ConsumeLeagueRunLoadRecovery(void);
bool8 MarkLeagueChampionDefeated(void);
bool8 IsCurrentLeagueRoomDefeated(void);

u8 GetGlobalBadgeCount(void);
enum Region GetRequiredLeagueRegion(void);
enum LeagueAdmissionRequirement GetLeagueAdmissionRequirement(enum Region region);
bool8 IsEligibleForLeague(enum Region region);
bool8 TryRecordLeagueClear(enum Region region);
enum Region ConsumeRecordedLeagueClearRegion(void);
u8 CalculateLeagueCircuitTrainerRating(void);
#endif

#endif
