#ifndef GUARD_LEAGUE_PHONE_H
#define GUARD_LEAGUE_PHONE_H

#include "constants/league_phone.h"

// League invitations and results by phone (.product/specs/leagues.md#presentation).
bool8 LeaguePhone_TryStartCall(void);
#if TESTING || defined(E2E_TESTING)
extern u8 gLeaguePhoneDebugInvitation;
#endif

#endif // GUARD_LEAGUE_PHONE_H
