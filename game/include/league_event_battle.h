#ifndef GUARD_LEAGUE_EVENT_BATTLE_H
#define GUARD_LEAGUE_EVENT_BATTLE_H

#include "global.h"
#include "league_event_types.h"

bool32 IsLeagueEventBattleInProgress(void);
bool32 GetPreparedLeagueEventBattle(u16 trainerId, const struct LeagueSavedTeam **team, u8 *acceptanceOptions);
bool32 GetLeagueEventFrozenAbilityForMon(const struct Pokemon *mon, u16 *ability);
bool32 GetLeagueEventFrozenAbilityForPartyIndex(u8 index, u16 *ability);
bool32 GetLeagueEventFrozenTypesForMon(const struct Pokemon *mon, u8 types[2]);
bool32 GetLeagueEventFrozenTypesForPartyIndex(u8 index, u8 types[2]);
bool32 ConsumeLeagueEventBattleVictory(u32 eventId, u8 match);
void ResetLeagueEventBattleProof(void);
void ResetLeagueEventMonOverrides(void);

#if TESTING
void SetLeagueEventBattleVictoryForTesting(u32 eventId, u8 match);
#endif

#endif // GUARD_LEAGUE_EVENT_BATTLE_H
