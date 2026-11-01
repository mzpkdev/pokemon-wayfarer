// League invitations and results by phone (Wayfarer). Spec: .product/specs/leagues.md (Invitations, Lifecycle,
// Presentation).
//
// The saved state and its rules live in league_events.c. What lives here is the ring: when the field may start a
// league's call, and the specials its scripts (data/scripts/league_phone.inc) use to word the call. Each calling
// league's front desk calls: Indigo Plateau's for the Indigo League, Ever Grande City's for the Hoenn League.

#include "global.h"
#include "league_phone.h"

#if IS_WAYFARER
#include "config/league_circuit.h"
#endif

#if IS_WAYFARER && WAYFARER_LEAGUE_EVENTS
#include "data.h"
#include "event_data.h"
#include "field_name_box.h"
#include "league_circuit.h"
#include "league_events.h"
#include "league_selection.h"
#include "script.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/flags.h"

extern const u8 LeaguePhone_EventScript_Invitation[];
extern const u8 LeaguePhone_EventScript_Result[];

static const u8 sIndigoCaller[] = _("INDIGO LEAGUE");
static const u8 sHoennCaller[] = _("HOENN LEAGUE");
static const u8 sIndigoPlace[] = _("INDIGO PLATEAU");
static const u8 sHoennPlace[] = _("EVER GRANDE CITY");
static const u8 sMastersName[] = _("SEVII MASTERS");
static const u8 sUnknownTrainer[] = _("a TRAINER");
static const u8 sLineupAnd[] = _("and last, ");

// A failed answer leaves the invitation, so the call would ring again on the next frame; it waits instead until
// the player is on another map (or reloads), so a failing save can't lock the field in a ringing loop.
EWRAM_DATA static u16 sRingHeldMap = 0;
EWRAM_DATA static bool8 sRingHeld = FALSE;

#if TESTING || defined(E2E_TESTING)
// Debug and E2E builds can make the next check place a call now (a league id, or LEAGUE_PHONE_DEBUG_ROTATION
// for the usual rotation; zero is none), so a journey does not wait out seven in-game days.
EWRAM_DATA u8 gLeaguePhoneDebugInvitation = 0;
#endif

static u16 CurrentMapKey(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

static void HoldRing(void)
{
    sRingHeld = TRUE;
    sRingHeldMap = CurrentMapKey();
}

static bool32 IsRingHeld(void)
{
    if (sRingHeld && sRingHeldMap != CurrentMapKey())
        sRingHeld = FALSE;
    return sRingHeld;
}

// gStringVar1 is the league, gStringVar2 its place.
static void BufferLeagueNames(enum LeagueId league)
{
    if (league == LEAGUE_ID_MASTERS)
    {
        StringCopy(gStringVar1, sMastersName);
        gStringVar2[0] = EOS;
        return;
    }
    StringCopy(gStringVar1, league == LEAGUE_ID_HOENN ? sHoennCaller : sIndigoCaller);
    StringCopy(gStringVar2, league == LEAGUE_ID_HOENN ? sHoennPlace : sIndigoPlace);
}

// The call's name box shows the calling league.
static void SetLeagueCaller(enum LeagueId league)
{
    gSpeakerName = league == LEAGUE_ID_HOENN ? sHoennCaller : sIndigoCaller;
    BufferLeagueNames(league);
}

static void BufferCharacterName(u8 *dest, u32 characterId)
{
    const struct LeagueTrainer *trainer = GetLeagueTrainer(characterId);

    // The league registry names each character's own encounter, whatever alias fought the player.
    if (trainer == NULL || trainer->characterId != characterId || trainer->sourceTrainerId >= TRAINERS_COUNT)
        StringCopy(dest, sUnknownTrainer);
    else
        StringCopy(dest, GetTrainerNameFromId(trainer->sourceTrainerId));
}

// The field polls this every frame it has control (no script, battle or menu running). It counts the countdown
// and places a due call first, then starts the owed result call or the waiting invitation's call.
bool8 LeaguePhone_TryStartCall(void)
{
    enum LeagueId league;

#if TESTING || defined(E2E_TESTING)
    if (gLeaguePhoneDebugInvitation != 0)
    {
        ForceLeagueInvitationForTesting(gLeaguePhoneDebugInvitation == LEAGUE_PHONE_DEBUG_ROTATION
                                        ? LEAGUE_ID_NONE : gLeaguePhoneDebugInvitation);
        gLeaguePhoneDebugInvitation = 0;
    }
#endif
    UpdateLeagueInvitations();
    // The call needs the phone, and waits for any league ceremony or save to finish.
    if (!FlagGet(FLAG_HAS_MATCH_CALL) || IsLeagueCeremonyPending() || IsRingHeld())
        return FALSE;
    if (GetLeagueResultCall() != 0)
    {
        ScriptContext_SetupScript(LeaguePhone_EventScript_Result);
        return TRUE;
    }
    league = GetLeagueInvitationLeague();
    if (league == LEAGUE_ID_NONE)
        return FALSE;
    gSpecialVar_0x8004 = league;
    ScriptContext_SetupScript(LeaguePhone_EventScript_Invitation);
    return TRUE;
}

// Special: words the invitation's call. VAR_RESULT is the inviting league.
u16 LeaguePhone_BufferInvitation(void)
{
    enum LeagueId league = GetLeagueInvitationLeague();

    SetLeagueCaller(league);
    return league;
}

// Special: the league's caller speaks again (the yes/no prompt is an ordinary message box between calls).
void LeaguePhone_SetCaller(void)
{
    SetLeagueCaller(gSpecialVar_0x8004);
}

// Special: accepts the waiting invitation; on success gStringVar3 holds the lineup in battle order.
u16 LeaguePhone_Accept(void)
{
    enum LeagueAcceptResult result = AcceptLeagueInvitation();
    const struct LeagueSavedTeam *team;
    u8 *ptr = gStringVar3;
    u8 match;

    if (result != LEAGUE_ACCEPT_OK)
    {
        HoldRing();
        return result;
    }
    // Two names a line: "A, B,\nC, D,\pand last, E." The call box holds two lines.
    for (match = 0; match < LEAGUE_EVENT_LINEUP_SIZE; match++)
    {
        if (!GetAcceptedLeagueEventMember(match, &team))
            break;
        if (match == LEAGUE_EVENT_LINEUP_SIZE - 1)
            ptr = StringCopy(ptr, sLineupAnd);
        BufferCharacterName(ptr, team->characterId);
        ptr += StringLength(ptr);
        if (match == LEAGUE_EVENT_LINEUP_SIZE - 1)
            *ptr++ = CHAR_PERIOD;
        else
        {
            *ptr++ = CHAR_COMMA;
            *ptr++ = match % 2 == 0 ? CHAR_SPACE : (match == 1 ? CHAR_NEWLINE : CHAR_PROMPT_CLEAR);
        }
    }
    *ptr = EOS;
    return result;
}

// Special: declines the waiting invitation, which runs the event without the player; on success gStringVar3
// names its champion.
u16 LeaguePhone_Decline(void)
{
    enum LeagueId league = GetLeagueInvitationLeague();

    if (!DeclineLeagueInvitation())
    {
        HoldRing();
        return FALSE;
    }
    BufferCharacterName(gStringVar3, GetLeagueReigningChampion(league));
    return TRUE;
}

// Special: words the owed result call. VAR_RESULT is TRUE for a win; gStringVar3 names the reigning champion.
u16 LeaguePhone_BufferResult(void)
{
    u8 resultCall = GetLeagueResultCall();
    enum LeagueId league = resultCall & LEAGUE_RESULT_CALL_LEAGUE_MASK;

    SetLeagueCaller(league);
    if (resultCall & LEAGUE_RESULT_CALL_WON)
    {
        gStringVar3[0] = EOS;
        return TRUE;
    }
    BufferCharacterName(gStringVar3, GetLeagueReigningChampion(league));
    return FALSE;
}

// Special: the result call has played.
void LeaguePhone_FinishResult(void)
{
    ClearLeagueResultCall();
}

// Special: the lobby's word on the reigning champion of the league in VAR_0x8004 (a circuit stage, which is
// its league id). gStringVar1 is the league, gStringVar3 the champion; VAR_RESULT says which text applies.
u16 LeaguePhone_BufferReigningChampion(void)
{
    enum LeagueId league = gSpecialVar_0x8004;
    u16 champion = GetLeagueReigningChampion(league);

    BufferLeagueNames(league);
    if (champion == 0)
        return LEAGUE_PHONE_CHAMPION_NONE;
    if (champion == LEAGUE_EVENT_PLAYER_CHAMPION)
        return LEAGUE_PHONE_CHAMPION_PLAYER;
    BufferCharacterName(gStringVar3, champion);
    return LEAGUE_PHONE_CHAMPION_TRAINER;
}

#else

bool8 LeaguePhone_TryStartCall(void)
{
    return FALSE;
}

#endif
