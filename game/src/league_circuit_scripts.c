#include "global.h"
#include "event_data.h"
#include "league_circuit.h"
#include "script.h"

u16 LeagueCircuit_CanChallengeGym(void)
{
    Script_RequestEffects(SCREFF_V1);
#if IS_WAYFARER
    return CanChallengeGymForBadge(gSpecialVar_0x8004, gSpecialVar_0x8005);
#else
    return TRUE;
#endif
}

u16 LeagueCircuit_GetRequiredRegion(void)
{
#if IS_WAYFARER
    return GetRequiredLeagueRegion();
#else
    return REGION_NONE;
#endif
}

u16 LeagueCircuit_IsEligible(void)
{
#if IS_WAYFARER
    return IsEligibleForLeague(gSpecialVar_0x8004);
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_RecordClear(void)
{
#if IS_WAYFARER
    return TryRecordLeagueClear(gSpecialVar_0x8004);
#else
    return FALSE;
#endif
}
