#include "global.h"
#include "event_data.h"
#include "league_circuit.h"
#include "script.h"
#include "string_util.h"
#include "config/league_circuit.h"

#if WAYFARER_LEAGUE_EVENTS
#include "data.h"
#include "league_events.h"
#include "constants/characters.h"
#include "constants/event_objects.h"
#include "constants/vars.h"
#endif

#if IS_WAYFARER
extern const u8 LeagueCircuit_Text_NeedEightBadges[];
extern const u8 LeagueCircuit_Text_NeedIndigoClear[];
extern const u8 LeagueCircuit_Text_NeedSixteenBadges[];
extern const u8 LeagueCircuit_Text_NeedMastersClear[];
extern const u8 LeagueCircuit_Text_NeedTwentyFourBadges[];
extern const u8 LeagueCircuit_Text_Unavailable[];
extern const u8 LeagueCircuit_Text_NeedQualification[];
extern const u8 LeagueCircuit_Text_NeedRegionalBadge[];
extern const u8 LeagueCircuit_Text_NeedMaster[];
extern const u8 LeagueCircuit_Text_EventElsewhere[];
extern const u8 LeagueCircuit_Text_NoAcceptedEvent[];
#endif

#if WAYFARER_LEAGUE_EVENTS
static const u8 sLeagueEventUnknownTrainer[] = _("TRAINER");
static const u8 sLeagueEventLineupLabel[] = _("Your opponents:");

static const struct LeagueSavedTeam *GetCurrentLeagueRoomTeam(void)
{
    const struct LeagueSavedTeam *team;
    s8 match = GetCurrentLeagueEventMatch();

    if (match < 0 || !GetAcceptedLeagueEventMember(match, &team))
        return NULL;
    return team;
}

static const u8 *GetLeagueEventMemberName(const struct LeagueSavedTeam *team)
{
    if (team == NULL || team->sourceTrainerId >= TRAINERS_COUNT)
        return sLeagueEventUnknownTrainer;
    return gTrainers[team->sourceTrainerId].trainerName;
}

u16 LeagueEvent_Accept(void)
{
    return AcceptLeagueEvent((enum LeagueId)gSpecialVar_0x8004);
}

u16 LeagueEvent_GetAcceptedLeague(void)
{
    return GetAcceptedLeagueEventLeagueId();
}

u16 LeagueEvent_BufferRoomName(void)
{
    const struct LeagueSavedTeam *team = GetCurrentLeagueRoomTeam();

    StringCopy(gStringVar1, GetLeagueEventMemberName(team));
    return team != NULL;
}

u16 LeagueEvent_SetRoomGraphics(void)
{
    const struct LeagueSavedTeam *team = GetCurrentLeagueRoomTeam();
    const struct LeagueTrainer *trainer = team != NULL ? GetLeagueTrainer(team->characterId) : NULL;

    VarSet(VAR_OBJ_GFX_ID_1, trainer != NULL && trainer->enabled
           ? trainer->objectGraphicsId : OBJ_EVENT_GFX_YOUNGSTER);
    return trainer != NULL;
}

u16 LeagueEvent_BufferChampion(void)
{
    const struct LeagueSavedTeam *team;

    if (!GetAcceptedLeagueEventMember(LEAGUE_LINEUP_SIZE - 1, &team))
        team = NULL;
    StringCopy(gStringVar1, GetLeagueEventMemberName(team));
    return team != NULL;
}

u16 LeagueEvent_BufferLineup(void)
{
    const struct LeagueSavedTeam *team;
    u8 *ptr = StringCopy(gStringVar4, sLeagueEventLineupLabel);
    u8 match;

    for (match = 0; match < LEAGUE_LINEUP_SIZE; match++)
    {
        if (!GetAcceptedLeagueEventMember(match, &team))
            return FALSE;
        if (match == 0)
            *ptr++ = CHAR_NEWLINE;
        else if (match % 2 != 0)
        {
            *ptr++ = CHAR_COMMA;
            *ptr++ = CHAR_SPACE;
        }
        ptr = StringCopy(ptr, GetLeagueEventMemberName(team));
        if (match == 1 || match == 3)
            *ptr++ = CHAR_PROMPT_CLEAR;
    }
    *ptr = EOS;
    return TRUE;
}
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
    case LEAGUE_ADMISSION_NEEDS_QUALIFICATION:
        text = LeagueCircuit_Text_NeedQualification;
        break;
    case LEAGUE_ADMISSION_NEEDS_REGIONAL_BADGE:
        text = LeagueCircuit_Text_NeedRegionalBadge;
        break;
    case LEAGUE_ADMISSION_NEEDS_MASTER:
        text = LeagueCircuit_Text_NeedMaster;
        break;
    case LEAGUE_ADMISSION_EVENT_ELSEWHERE:
        text = LeagueCircuit_Text_EventElsewhere;
        break;
    case LEAGUE_ADMISSION_NO_ACCEPTED_EVENT:
        text = LeagueCircuit_Text_NoAcceptedEvent;
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
