#ifndef GUARD_LEAGUE_CIRCUIT_H
#define GUARD_LEAGUE_CIRCUIT_H

#include "global.h"
#include "constants/regions.h"

#if IS_WAYFARER
u8 GetGlobalBadgeCount(void);
enum Region GetRequiredLeagueRegion(void);
u8 GetBadgeCertificationCap(void);
bool8 IsEligibleForLeague(enum Region region);
bool8 CanChallengeGymForBadge(enum Region region, u8 badgeIndex);
bool8 TryRecordLeagueClear(enum Region region);
enum Region ConsumeRecordedLeagueClearRegion(void);
u8 CalculateLeagueCircuitTrainerRating(void);
#endif

#endif
