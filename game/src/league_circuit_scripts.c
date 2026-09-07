#include "global.h"
#include "event_data.h"
#include "league_circuit.h"
#include "script.h"
#include "string_util.h"

#if IS_WAYFARER
extern const u8 LeagueCircuit_Text_NeedEightBadges[];
extern const u8 LeagueCircuit_Text_NeedKantoClear[];
extern const u8 LeagueCircuit_Text_NeedSixteenBadges[];
extern const u8 LeagueCircuit_Text_NeedJohtoClear[];
extern const u8 LeagueCircuit_Text_NeedTwentyFourBadges[];
extern const u8 LeagueCircuit_Text_Unavailable[];
#endif

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

u16 LeagueCircuit_BufferAdmissionDenial(void)
{
#if IS_WAYFARER
    enum LeagueAdmissionRequirement requirement = GetLeagueAdmissionRequirement(gSpecialVar_0x8004);
    const u8 *text;

    switch (requirement)
    {
    case LEAGUE_ADMISSION_NEEDS_8_BADGES:
        text = LeagueCircuit_Text_NeedEightBadges;
        break;
    case LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR:
        text = LeagueCircuit_Text_NeedKantoClear;
        break;
    case LEAGUE_ADMISSION_NEEDS_16_BADGES:
        text = LeagueCircuit_Text_NeedSixteenBadges;
        break;
    case LEAGUE_ADMISSION_NEEDS_JOHTO_CLEAR:
        text = LeagueCircuit_Text_NeedJohtoClear;
        break;
    case LEAGUE_ADMISSION_NEEDS_24_BADGES:
        text = LeagueCircuit_Text_NeedTwentyFourBadges;
        break;
    default:
        text = LeagueCircuit_Text_Unavailable;
        break;
    }
    StringCopy(gStringVar4, text);
    return requirement;
#else
    return 0;
#endif
}

u16 LeagueCircuit_GetRunRegion(void)
{
#if IS_WAYFARER
    return GetActiveLeagueRunRegion();
#else
    return REGION_NONE;
#endif
}

u16 LeagueCircuit_ValidateRun(void)
{
#if IS_WAYFARER
    return ValidateActiveLeagueRun();
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_MarkChampionDefeated(void)
{
#if IS_WAYFARER
    return MarkLeagueChampionDefeated();
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_IsCurrentRoomDefeated(void)
{
#if IS_WAYFARER
    return IsCurrentLeagueRoomDefeated();
#else
    return FALSE;
#endif
}
