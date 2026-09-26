#include "global.h"
#include "event_data.h"
#include "league_circuit.h"
#include "script.h"
#include "string_util.h"

#if IS_WAYFARER
extern const u8 LeagueCircuit_Text_NeedEightBadges[];
extern const u8 LeagueCircuit_Text_NeedIndigoClear[];
extern const u8 LeagueCircuit_Text_NeedSixteenBadges[];
extern const u8 LeagueCircuit_Text_NeedMastersClear[];
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

u16 LeagueCircuit_GetRequiredStage(void)
{
#if IS_WAYFARER
    return GetRequiredCircuitStage();
#else
    return 0;
#endif
}

u16 LeagueCircuit_IsEligible(void)
{
#if IS_WAYFARER
    return IsEligibleForCircuitStage(gSpecialVar_0x8004);
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_RecordClear(void)
{
#if IS_WAYFARER
    return TryRecordCircuitClear(gSpecialVar_0x8004);
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_BufferAdmissionDenial(void)
{
#if IS_WAYFARER
    enum LeagueAdmissionRequirement requirement = GetCircuitAdmissionRequirement(gSpecialVar_0x8004);
    const u8 *text;

    switch (requirement)
    {
    case LEAGUE_ADMISSION_NEEDS_8_BADGES:
        text = LeagueCircuit_Text_NeedEightBadges;
        break;
    case LEAGUE_ADMISSION_NEEDS_INDIGO_CLEAR:
        text = LeagueCircuit_Text_NeedIndigoClear;
        break;
    case LEAGUE_ADMISSION_NEEDS_16_BADGES:
        text = LeagueCircuit_Text_NeedSixteenBadges;
        break;
    case LEAGUE_ADMISSION_NEEDS_MASTERS_CLEAR:
        text = LeagueCircuit_Text_NeedMastersClear;
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

u16 LeagueCircuit_GetRunStage(void)
{
#if IS_WAYFARER
    return GetActiveLeagueRunStage();
#else
    return 0;
#endif
}

u16 LeagueCircuit_BeginRun(void)
{
#if IS_WAYFARER
    return BeginCircuitRun(gSpecialVar_0x8004);
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_ValidateRoomBattle(void)
{
#if IS_WAYFARER
    return ValidateCircuitRoomBattle(gSpecialVar_0x8004, gSpecialVar_0x8005);
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_RecordRoomVictory(void)
{
#if IS_WAYFARER
    return RecordCircuitRoomVictory(gSpecialVar_0x8004, gSpecialVar_0x8005);
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_CanCompleteRun(void)
{
#if IS_WAYFARER
    return CanCompleteCircuitRun(gSpecialVar_0x8004);
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_CommitRunClear(void)
{
#if IS_WAYFARER
    return CommitCircuitRun(gSpecialVar_0x8004);
#else
    return 0;
#endif
}

u16 LeagueCircuit_AbandonRun(void)
{
#if IS_WAYFARER
    EndLeagueRun();
#endif
    return TRUE;
}

u16 LeagueCircuit_GetAdmissionRequirement(void)
{
#if IS_WAYFARER
    return GetCircuitAdmissionRequirement(gSpecialVar_0x8004);
#else
    return 0;
#endif
}

u16 LeagueCircuit_HasClearedStage(void)
{
#if IS_WAYFARER
    return HasClearedCircuitStage(gSpecialVar_0x8004);
#else
    return FALSE;
#endif
}

u16 LeagueCircuit_IsComplete(void)
{
#if IS_WAYFARER
    return HasClearedCircuitStage(CIRCUIT_STAGE_INDIGO)
        && HasClearedCircuitStage(CIRCUIT_STAGE_MASTERS)
        && HasClearedCircuitStage(CIRCUIT_STAGE_HOENN);
#else
    return FALSE;
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
