#ifndef GUARD_LEAGUE_EVENTS_H
#define GUARD_LEAGUE_EVENTS_H

#include "global.h"
#include "league_selection.h"
#include "league_event_types.h"

enum LeagueInvitationState
{
    LEAGUE_INVITATION_NOT_QUALIFIED,
    LEAGUE_INVITATION_COUNTING_DOWN,
    LEAGUE_INVITATION_INVITED,
    LEAGUE_INVITATION_ACCEPTED,
};

enum LeagueAcceptResult
{
    LEAGUE_ACCEPT_OK,
    LEAGUE_ACCEPT_NOT_QUALIFIED,
    LEAGUE_ACCEPT_INELIGIBLE,
    LEAGUE_ACCEPT_BUSY,
    LEAGUE_ACCEPT_SELECTION_FAILED,
    LEAGUE_ACCEPT_SAVE_FAILED,
};

enum LeagueEventOutcome
{
    LEAGUE_EVENT_WON,
    LEAGUE_EVENT_LOST,
    LEAGUE_EVENT_EXITED,
};

void InitLeagueEventState(void);
bool32 ValidateLeagueEventState(void);
enum LeagueAcceptResult AcceptLeagueEvent(enum LeagueId leagueId);
u32 GetAcceptedLeagueEventId(void);
enum LeagueId GetAcceptedLeagueEventLeagueId(void);
u32 GetAcceptedLeagueEventWorldProgress(void);
bool32 GetAcceptedLeagueEventMember(u8 match, const struct LeagueSavedTeam **team);
bool32 ResolveLeagueEvent(enum LeagueEventOutcome outcome, u32 expectedEventId);
u16 GetLeagueReigningChampion(enum LeagueId leagueId);
bool32 HasLeagueTrainerReigned(enum LeagueId leagueId, u32 characterId);
u16 GetLeagueGalleryWins(u32 characterId); // characterId 0 is the player
void GetLeagueRecentLineup(u16 outIds[LEAGUE_EVENT_LINEUP_SIZE]);
#if TESTING
void SealLeagueEventStateForTesting(void);
#endif

// Battle construction is deliberately separate from saved-state mutation.
bool32 BuildLeagueEventSavedTeam(const struct NotableTrainerSnapshot *snapshot,
                                u16 sourceTrainerId, struct LeagueSavedTeam *team);

#endif // GUARD_LEAGUE_EVENTS_H
