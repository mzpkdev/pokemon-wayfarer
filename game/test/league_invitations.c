// League invitations by phone: qualification, the countdown, which league calls, accepting and declining, the
// owed result call, the lobby's refusal, and load validation. Spec: .product/specs/leagues.md#invitations.
#include "global.h"
#include "config/league_circuit.h"
#include "event_data.h"
#include "league_circuit.h"
#include "league_events.h"
#include "league_phone.h"

extern u16 LeaguePhone_Decline(void);
extern u16 LeaguePhone_BufferResult(void);
extern void LeaguePhone_FinishResult(void);
#include "league_run_helpers.h"
#include "league_selection.h"
#include "load_save.h"
#include "random.h"
#include "safari_zone.h"
#include "script.h"
#include "save.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "wayfarer_world.h"
#include "test/test.h"
#include "gba/flash_internal.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/regions.h"
#include "constants/vars.h"
#include "constants/wayfarer_origin.h"

#if WAYFARER_LEAGUE_EVENTS

static struct LeagueEventState *State(void)
{
    return &gSaveBlock3Ptr->leagueEvent;
}

static void SetBadges(u8 kanto, u8 johto, u8 hoenn)
{
    u8 badge;
    for (badge = 0; badge < 8; badge++)
    {
        SetBadgeStateForRegion(REGION_KANTO, badge, badge < kanto);
        SetBadgeStateForRegion(REGION_JOHTO, badge, badge < johto);
        SetBadgeStateForRegion(REGION_HOENN, badge, badge < hoenn);
    }
}

static void PrepareInvitationTest(u8 kanto, u8 johto, u8 hoenn, u32 trainerTR, u16 today)
{
    WayfarerInitPersistentState();
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    gSaveBlock3Ptr->wayfarerHoenn.fallbackHealLocation = HEAL_LOCATION_NEW_BARK_TOWN_HNS;
    CheckForFlashMemory();
    if (gFlashMemoryPresent != TRUE)
    {
        gFlashMemoryPresent = TRUE;
        InitFlashTimer();
    }
    ClearSaveData();
    Save_ResetSaveCounters();
    gSaveBlock1Ptr->saveVersionMagic = SAVE_VERSION_MAGIC;
    gSaveBlock1Ptr->saveVersion = SAVE_VERSION;
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    SetGameClearStateForRegion(REGION_HOENN, FALSE);
    SetBadges(kanto, johto, hoenn);
    SetTrainerRating(trainerTR);
    VarSet(VAR_DAYS, today);
    WayfarerWorld_InitNewGame();  // a saved game always has a seeded world state
}

static void FinishInvitationTest(void)
{
    ClearSaveData();
    Save_ResetSaveCounters();
}

// Advances the day counter and runs the check the field runs every frame.
static void PassDays(u16 days)
{
    VarSet(VAR_DAYS, VarGet(VAR_DAYS) + days);
    UpdateLeagueInvitations();
}

// Lets the countdown run out and returns the league that called.
static enum LeagueId WaitForCall(void)
{
    u32 day;
    for (day = 0; day < 8 && GetLeagueInvitationLeague() == LEAGUE_ID_NONE; day++)
        PassDays(1);
    return GetLeagueInvitationLeague();
}

TEST("No call below TR 80; at 80 the countdown starts that day and the first call comes 7 days later")
{
    // Seven badges rate 70; the player's TR is the higher of that and the saved high-water value.
    PrepareInvitationTest(7, 0, 0, 79, 10);
    EXPECT(!UpdateLeagueInvitations());
    PassDays(30);
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_NOT_QUALIFIED);

    SetTrainerRating(80);
    EXPECT(UpdateLeagueInvitations());
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_COUNTING_DOWN);
    EXPECT_EQ(State()->daysRemaining, 7);
    EXPECT_EQ(State()->lastCountedDay, 40);
    EXPECT_EQ(GetLeagueInvitationDaysRemaining(), 7);
    EXPECT(!UpdateLeagueInvitations()); // the same day changes nothing
    PassDays(6);
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_NONE);
    EXPECT_EQ(GetLeagueInvitationDaysRemaining(), 1);
    PassDays(1);
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(State()->callCounter, 1);
    EXPECT_EQ(State()->lastCallNumber[LEAGUE_ID_INDIGO - 1], 1);
    EXPECT(ValidateLeagueEventState());
    FinishInvitationTest();
}

TEST("A day counter turned back neither advances nor resets the countdown, and counts on from the lower day")
{
    PrepareInvitationTest(8, 0, 0, 80, 100);
    UpdateLeagueInvitations();
    PassDays(3);
    EXPECT_EQ(State()->daysRemaining, 4);
    VarSet(VAR_DAYS, 50);
    UpdateLeagueInvitations();
    EXPECT_EQ(State()->daysRemaining, 4);
    EXPECT_EQ(State()->lastCountedDay, 50);
    PassDays(3);
    EXPECT_EQ(State()->daysRemaining, 1);
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_NONE);
    PassDays(1);
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_INDIGO);
    FinishInvitationTest();
}

TEST("A jump of many days brings one call and no backlog; no call comes while one waits or an event is accepted")
{
    PrepareInvitationTest(8, 0, 1, 80, 1);
    UpdateLeagueInvitations();
    PassDays(500);
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(State()->callCounter, 1);
    PassDays(500);
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(State()->callCounter, 1);

    EXPECT_EQ(AcceptLeagueInvitation(), LEAGUE_ACCEPT_OK);
    PassDays(500);
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_ACCEPTED);
    EXPECT_EQ(State()->callCounter, 1);
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_NONE);
    FinishInvitationTest();
}

TEST("A league calls only where the player is known; one eligible league calls every time")
{
    PrepareInvitationTest(0, 0, 3, 80, 1);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_HOENN);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_HOENN);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_HOENN);
    EXPECT_EQ(State()->callCounter, 3);
    EXPECT_EQ(State()->lastCallNumber[LEAGUE_ID_INDIGO - 1], 0);
    EXPECT_EQ(State()->lastCallNumber[LEAGUE_ID_HOENN - 1], 3);

    // A league that becomes eligible later has never called, so it calls next.
    EXPECT(DeclineLeagueInvitation());
    SetBadges(0, 1, 3);
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_HOENN);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    FinishInvitationTest();
}

TEST("The first call comes from the league with the most badges, ties go to Indigo, then the leagues alternate")
{
    PrepareInvitationTest(2, 1, 4, 80, 1);
    EXPECT_EQ(GetNextCallingLeague(), LEAGUE_ID_HOENN);
    SetBadges(2, 2, 4);
    EXPECT_EQ(GetNextCallingLeague(), LEAGUE_ID_INDIGO); // Kanto and Johto badges both count for Indigo
    SetBadges(3, 1, 4);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT(DeclineLeagueInvitation());
    // Badges no longer matter once both have called: the least recently called calls.
    SetBadges(8, 8, 1);
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_HOENN);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(State()->callCounter, 3);
    // A day counter turned back never changes the order.
    EXPECT(DeclineLeagueInvitation());
    VarSet(VAR_DAYS, 1);
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_HOENN);
    FinishInvitationTest();
}

TEST("The Sevii Masters never calls, even for a Master; it keeps its lobby acceptance")
{
    PrepareInvitationTest(8, 0, 8, 160, 1);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, TRUE);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_HOENN);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(State()->lastCallNumber[LEAGUE_ID_MASTERS - 1], 0);
    // While a call waits, the Masters' lobby is busy; once it is answered, the lobby accepts.
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_MASTERS), LEAGUE_ACCEPT_BUSY);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_MASTERS), LEAGUE_ACCEPT_OK);
    EXPECT_EQ(State()->lastCallNumber[LEAGUE_ID_MASTERS - 1], State()->callCounter);
    EXPECT(ValidateLeagueEventState());
    FinishInvitationTest();
}

TEST("With no league that knows the player, a due call waits and is checked again each day")
{
    PrepareInvitationTest(0, 0, 0, 80, 1);
    UpdateLeagueInvitations();
    PassDays(7);
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_COUNTING_DOWN);
    EXPECT_EQ(State()->daysRemaining, 0);
    PassDays(3);
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_NONE);
    EXPECT_EQ(State()->callCounter, 0);
    SetBadges(0, 0, 1);
    UpdateLeagueInvitations();
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_HOENN);
    FinishInvitationTest();
}

TEST("Indigo and Hoenn are accepted only by answering their call")
{
    PrepareInvitationTest(8, 0, 1, 80, 1);
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_INDIGO), LEAGUE_ACCEPT_NOT_INVITED);
    EXPECT_EQ(AcceptLeagueInvitation(), LEAGUE_ACCEPT_NOT_INVITED);
    UpdateLeagueInvitations();
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_HOENN), LEAGUE_ACCEPT_NOT_INVITED);
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_HOENN), LEAGUE_ACCEPT_BUSY);
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_INDIGO), LEAGUE_ADMISSION_CALL_PENDING);
    EXPECT_EQ(AcceptLeagueInvitation(), LEAGUE_ACCEPT_OK);
    EXPECT_EQ(GetAcceptedLeagueEventLeagueId(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(State()->callCounter, 1); // answering takes no new call number
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_INDIGO), LEAGUE_ADMISSION_AVAILABLE);
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_HOENN), LEAGUE_ADMISSION_EVENT_ELSEWHERE);
    EXPECT(!DeclineLeagueInvitation());
    FinishInvitationTest();
}

TEST("Declining crowns the strongest of the lineup selected then, records the reign, and restarts the countdown")
{
    struct LeagueLineupSelection expected;
    u32 noRecent[LEAGUE_EVENT_LINEUP_SIZE] = {0};
    u16 recent[LEAGUE_EVENT_LINEUP_SIZE];
    rng_value_t rng, rng2;
    u16 champion;
    u32 i;

    PrepareInvitationTest(8, 0, 0, 120, 5);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT(SelectLeagueLineup(LEAGUE_ID_INDIGO, 120, noRecent, 0, 0, &expected));
    champion = expected.members[LEAGUE_EVENT_LINEUP_SIZE - 1].characterId;
    rng = gRngValue;
    rng2 = gRng2Value;
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(memcmp(&gRngValue, &rng, sizeof(rng)), 0);
    EXPECT_EQ(memcmp(&gRng2Value, &rng2, sizeof(rng2)), 0);
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), champion);
    EXPECT(HasLeagueTrainerReigned(LEAGUE_ID_INDIGO, champion));
    EXPECT(!HasLeagueTrainerReigned(LEAGUE_ID_HOENN, champion));
    GetLeagueRecentLineup(recent);
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
        EXPECT_EQ(recent[i], expected.members[i].characterId);
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_COUNTING_DOWN);
    EXPECT_EQ(State()->daysRemaining, 7);
    EXPECT_EQ(State()->lastCountedDay, VarGet(VAR_DAYS));
    EXPECT_EQ(GetAcceptedLeagueEventId(), 0);
    EXPECT_EQ(GetLeagueResultCall(), 0); // the call itself named the champion
    EXPECT(!DeclineLeagueInvitation());

    // The decline was saved before it was revealed.
    ClearSav1();
    ClearSav2();
    ClearSav3();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), champion);
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_COUNTING_DOWN);
    EXPECT_EQ(State()->daysRemaining, 7);
    FinishInvitationTest();
}

TEST("A declined lineup fatigues the next event at any league")
{
    struct LeagueLineupSelection fresh, fatigued;
    u32 noRecent[LEAGUE_EVENT_LINEUP_SIZE] = {0}, recentInput[LEAGUE_EVENT_LINEUP_SIZE];
    u16 recent[LEAGUE_EVENT_LINEUP_SIZE];
    u32 i;

    PrepareInvitationTest(8, 0, 1, 160, 1);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT(DeclineLeagueInvitation());
    GetLeagueRecentLineup(recent);
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
        recentInput[i] = recent[i];
    EXPECT(SelectLeagueLineup(LEAGUE_ID_HOENN, 160, noRecent, 0, 0, &fresh));
    EXPECT(SelectLeagueLineup(LEAGUE_ID_HOENN, 160, recentInput,
                              State()->reignedIndigoMask, State()->reignedHoennMask, &fatigued));
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_HOENN);
    EXPECT_EQ(AcceptLeagueInvitation(), LEAGUE_ACCEPT_OK);
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
    {
        const struct LeagueSavedTeam *team;
        EXPECT(GetAcceptedLeagueEventMember(i, &team));
        EXPECT_EQ(team->characterId, fatigued.members[i].characterId);
    }
    FinishInvitationTest();
}

TEST("Each resolution of an accepted event restarts the countdown and owes the league's result call")
{
    u32 eventId;

    PrepareInvitationTest(8, 0, 0, 80, 3);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    VarSet(VAR_DAYS, 20);
    eventId = GetAcceptedLeagueEventId();
    EXPECT(ResolveLeagueEvent(LEAGUE_EVENT_LOST, eventId));
    EndLeagueRun();
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_COUNTING_DOWN);
    EXPECT_EQ(State()->daysRemaining, 7);
    EXPECT_EQ(State()->lastCountedDay, 20);
    EXPECT_EQ(GetLeagueResultCall(), LEAGUE_ID_INDIGO);
    EXPECT(ValidateLeagueEventState());
    ClearLeagueResultCall();
    EXPECT_EQ(GetLeagueResultCall(), 0);
    EXPECT(ValidateLeagueEventState());

    // A win owes the winner's call.
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(CommitCircuitRun(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    EXPECT_EQ(GetLeagueResultCall(), LEAGUE_ID_INDIGO | LEAGUE_RESULT_CALL_WON);
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_COUNTING_DOWN);
    EXPECT_EQ(State()->daysRemaining, 7);
    EXPECT(ValidateLeagueEventState());
    ConsumeRecordedCircuitClearStage();
    FinishInvitationTest();
}

TEST("A decline drops an owed result call for its league, which would no longer match its champion")
{
    PrepareInvitationTest(8, 0, 0, 80, 3);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    Test_CompleteCircuitRooms(CIRCUIT_STAGE_INDIGO);
    EXPECT_EQ(CommitCircuitRun(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    ConsumeRecordedCircuitClearStage();
    EXPECT_EQ(GetLeagueResultCall(), LEAGUE_ID_INDIGO | LEAGUE_RESULT_CALL_WON);
    // The player never took the result call (no phone); the next invitation comes and is declined.
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT(DeclineLeagueInvitation());
    EXPECT_EQ(GetLeagueResultCall(), 0);
    EXPECT_NE(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), LEAGUE_EVENT_PLAYER_CHAMPION);
    EXPECT(ValidateLeagueEventState());
    FinishInvitationTest();
}

TEST("A Masters event owes no result call, and leaves an owed Indigo or Hoenn call alone")
{
    PrepareInvitationTest(8, 0, 8, 160, 1);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, TRUE);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_MASTERS));
    EXPECT(ResolveLeagueEvent(LEAGUE_EVENT_LOST, GetAcceptedLeagueEventId()));
    EndLeagueRun();
    EXPECT_EQ(GetLeagueResultCall(), 0);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    EXPECT(ResolveLeagueEvent(LEAGUE_EVENT_LOST, GetAcceptedLeagueEventId()));
    EndLeagueRun();
    EXPECT_EQ(GetLeagueResultCall(), LEAGUE_ID_INDIGO);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_MASTERS));
    EXPECT(ResolveLeagueEvent(LEAGUE_EVENT_LOST, GetAcceptedLeagueEventId()));
    EndLeagueRun();
    EXPECT_EQ(GetLeagueResultCall(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_COUNTING_DOWN);
    FinishInvitationTest();
}

TEST("A last counted day ahead of the day counter loads clamped and is never a reason to reject")
{
    PrepareInvitationTest(8, 0, 0, 80, 10);
    UpdateLeagueInvitations();
    PassDays(2);
    State()->lastCountedDay = 900;
    SealLeagueEventStateForTesting();
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(State()->lastCountedDay, 12);
    EXPECT_EQ(State()->daysRemaining, 5);
    FinishInvitationTest();
}

TEST("Load validation rejects a Masters invitation and a corrupt owed result call")
{
    PrepareInvitationTest(8, 0, 8, 160, 1);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, TRUE);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    // The Masters never phones, so a saved Masters invitation is corrupt even for a Master.
    State()->invitedLeague = LEAGUE_ID_MASTERS;
    State()->lastCallNumber[LEAGUE_ID_INDIGO - 1] = 0;
    State()->lastCallNumber[LEAGUE_ID_MASTERS - 1] = State()->callCounter;
    SealLeagueEventStateForTesting();
    EXPECT(!ValidateLeagueEventState());
    State()->invitedLeague = LEAGUE_ID_INDIGO;
    State()->lastCallNumber[LEAGUE_ID_INDIGO - 1] = State()->callCounter;
    State()->lastCallNumber[LEAGUE_ID_MASTERS - 1] = 0;
    SealLeagueEventStateForTesting();
    EXPECT(ValidateLeagueEventState());

    // A loss's result call needs a trainer reigning there; a win's, the player.
    State()->resultCall = LEAGUE_ID_HOENN;
    SealLeagueEventStateForTesting();
    EXPECT(!ValidateLeagueEventState());
    State()->resultCall = LEAGUE_ID_MASTERS;
    SealLeagueEventStateForTesting();
    EXPECT(!ValidateLeagueEventState());
    State()->resultCall = 0x08 | LEAGUE_ID_INDIGO;
    SealLeagueEventStateForTesting();
    EXPECT(!ValidateLeagueEventState());
    State()->resultCall = 0;
    State()->reserved = 1;
    SealLeagueEventStateForTesting();
    EXPECT(!ValidateLeagueEventState());
    State()->reserved = 0;
    SealLeagueEventStateForTesting();
    EXPECT(ValidateLeagueEventState());
    FinishInvitationTest();
}

TEST("A content change returns an accepted Masters event to a due countdown, since the Masters never calls")
{
    PrepareInvitationTest(8, 0, 8, 160, 4);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, TRUE);
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_MASTERS), LEAGUE_ACCEPT_OK);
    State()->contentVersions[0]++;
    SealLeagueEventStateForTesting();
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(State()->invitationState, LEAGUE_INVITATION_COUNTING_DOWN);
    EXPECT_EQ(State()->daysRemaining, 0);
    EXPECT_EQ(GetAcceptedLeagueEventId(), 0);
    // The next check places the call from the rotation.
    UpdateLeagueInvitations();
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_INDIGO);
    EXPECT(ValidateLeagueEventState());
    FinishInvitationTest();
}

// Starts a ring check and, when a call starts, stops its script again: the test only asks whether it rang.
static bool8 Rings(void)
{
    bool8 rang = LeaguePhone_TryStartCall();
    if (rang)
        ScriptContext_Init();
    return rang;
}

TEST("The phone rings only for a player who has it, and not while a league run is active")
{
    PrepareInvitationTest(8, 0, 0, 80, 1);
    FlagClear(FLAG_HAS_MATCH_CALL);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT(!Rings());
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_INDIGO);
    FlagSet(FLAG_HAS_MATCH_CALL);
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = TRUE;
    EXPECT(!Rings());
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = FALSE;
    EXPECT(Rings());
    EXPECT(Rings()); // unanswered, it rings again
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_INDIGO);
    FlagClear(FLAG_HAS_MATCH_CALL);
    FinishInvitationTest();
}

TEST("No call rings where the player couldn't save, such as the Safari Zone")
{
    PrepareInvitationTest(8, 0, 0, 80, 1);
    FlagSet(FLAG_HAS_MATCH_CALL);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    SetSafariZoneFlag();
    EXPECT(!Rings());
    ResetSafariZoneFlag();
    EXPECT(Rings());
    FlagClear(FLAG_HAS_MATCH_CALL);
    FinishInvitationTest();
}

TEST("A failed answer holds the ring until the player is on another map")
{
    PrepareInvitationTest(8, 0, 0, 80, 1);
    FlagSet(FLAG_HAS_MATCH_CALL);
    gSaveBlock1Ptr->location.mapGroup = 1;
    gSaveBlock1Ptr->location.mapNum = 2;
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    // An answer that can't be saved fails; a stale state (here, a run) makes the decline fail the same way.
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = TRUE;
    EXPECT_EQ(LeaguePhone_Decline(), FALSE);
    gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active = FALSE;
    EXPECT_EQ(GetLeagueInvitationLeague(), LEAGUE_ID_INDIGO);
    EXPECT(!Rings());
    EXPECT(!Rings());
    gSaveBlock1Ptr->location.mapNum = 3;
    EXPECT(Rings());
    FlagClear(FLAG_HAS_MATCH_CALL);
    FinishInvitationTest();
}

TEST("An owed result call rings first and is cleared once it has played")
{
    PrepareInvitationTest(8, 0, 0, 80, 3);
    FlagSet(FLAG_HAS_MATCH_CALL);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    EXPECT(ResolveLeagueEvent(LEAGUE_EVENT_LOST, GetAcceptedLeagueEventId()));
    EndLeagueRun();
    AcknowledgeLeagueProgressSave();
    EXPECT_EQ(GetLeagueResultCall(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(LeaguePhone_BufferResult(), FALSE); // a loss
    EXPECT(Rings());
    LeaguePhone_FinishResult();
    EXPECT_EQ(GetLeagueResultCall(), 0);
    EXPECT(!Rings()); // nothing else is due yet
    FlagClear(FLAG_HAS_MATCH_CALL);
    FinishInvitationTest();
}

TEST("A first Masters win holds no call, while an Indigo first win waits for its Hall of Fame")
{
    PrepareInvitationTest(8, 0, 8, 160, 1);
    FlagSet(FLAG_HAS_MATCH_CALL);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, TRUE);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_MASTERS), CIRCUIT_COMMIT_FIRST_CLEAR);
    AcknowledgeLeagueProgressSave();
    EXPECT(!IsLeagueCeremonyPending());
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT(Rings());
    ConsumeRecordedCircuitClearStage();
    FlagClear(FLAG_HAS_MATCH_CALL);
    FinishInvitationTest();

    PrepareInvitationTest(8, 0, 0, 80, 1);
    FlagSet(FLAG_HAS_MATCH_CALL);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    AcknowledgeLeagueProgressSave();
    EXPECT(IsLeagueCeremonyPending());
    EXPECT(!Rings());
    EXPECT_EQ(ConsumeRecordedCircuitClearStage(), CIRCUIT_STAGE_INDIGO);
    EXPECT(!IsLeagueCeremonyPending());
    EXPECT(Rings()); // the winner's result call
    FlagClear(FLAG_HAS_MATCH_CALL);
    FinishInvitationTest();
}

TEST("A player who isn't a Master hears that first at the Masters, even with a call waiting")
{
    PrepareInvitationTest(8, 0, 0, 80, 1);
    UpdateLeagueInvitations();
    EXPECT_EQ(WaitForCall(), LEAGUE_ID_INDIGO);
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_MASTERS), LEAGUE_ADMISSION_NEEDS_MASTER);
    EXPECT_EQ(GetCircuitAdmissionRequirement(CIRCUIT_STAGE_HOENN), LEAGUE_ADMISSION_CALL_PENDING);
    FinishInvitationTest();
}

#endif // WAYFARER_LEAGUE_EVENTS
