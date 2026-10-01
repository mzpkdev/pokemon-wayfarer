#include "global.h"
#include "config/league_circuit.h"
#include "event_data.h"
#include "league_circuit.h"
#include "league_events.h"
#include "league_run_helpers.h"
#include "load_save.h"
#include "random.h"
#include "save.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "wayfarer_world.h"
#include "gba/flash_internal.h"
#include "constants/notable_trainers.h"
#include "constants/heal_locations.h"
#include "constants/regions.h"
#include "constants/wayfarer_origin.h"

#if WAYFARER_LEAGUE_EVENTS

static void PrepareLeagueEventTest(u8 kantoBadges, u8 hoennBadges, u32 trainerTR)
{
    u8 badge;
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
    SetGameClearStateForRegion(REGION_HOENN, FALSE);
    for (badge = 0; badge < 8; badge++)
    {
        SetBadgeStateForRegion(REGION_KANTO, badge, badge < kantoBadges);
        SetBadgeStateForRegion(REGION_JOHTO, badge, FALSE);
        SetBadgeStateForRegion(REGION_HOENN, badge, badge < hoennBadges);
    }
    SetTrainerRating(trainerTR);
    WayfarerWorld_InitNewGame();  // a saved game always has a seeded world state
}

TEST("League acceptance preserves RNG and rejects a second event without changing its snapshot")
{
    struct LeagueSavedTeam frozen;
    const struct LeagueSavedTeam *team;
    rng_value_t firstRng, secondRng;
    u32 eventId;
    PrepareLeagueEventTest(8, 0, 80);
    firstRng = gRngValue;
    secondRng = gRng2Value;
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_INDIGO), LEAGUE_ACCEPT_OK);
    EXPECT_EQ(memcmp(&gRngValue, &firstRng, sizeof(firstRng)), 0);
    EXPECT_EQ(memcmp(&gRng2Value, &secondRng, sizeof(secondRng)), 0);
    eventId = GetAcceptedLeagueEventId();
    EXPECT_NE(eventId, 0);
    EXPECT_EQ(GetAcceptedLeagueEventWorldProgress(), 80);
    EXPECT(GetAcceptedLeagueEventMember(0, &team));
    frozen = *team;
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_INDIGO), LEAGUE_ACCEPT_BUSY);
    EXPECT_EQ(GetAcceptedLeagueEventId(), eventId);
    EXPECT(GetAcceptedLeagueEventMember(0, &team));
    EXPECT_EQ(memcmp(team, &frozen, sizeof(frozen)), 0);
    EXPECT_EQ(memcmp(&gRngValue, &firstRng, sizeof(firstRng)), 0);
    EXPECT(ValidateLeagueEventState());
    ClearSaveData();
    Save_ResetSaveCounters();
}

TEST("Accepted League teams remain frozen through player growth and full save reload")
{
    struct LeagueSavedTeam first, final;
    const struct LeagueSavedTeam *team;
    u32 eventId;
    PrepareLeagueEventTest(8, 0, 80);
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_INDIGO), LEAGUE_ACCEPT_OK);
    eventId = GetAcceptedLeagueEventId();
    EXPECT(GetAcceptedLeagueEventMember(0, &team));
    first = *team;
    EXPECT(GetAcceptedLeagueEventMember(LEAGUE_EVENT_LINEUP_SIZE - 1, &team));
    final = *team;
    SetTrainerRating(160);
    EXPECT_EQ(GetAcceptedLeagueEventWorldProgress(), 80);
    EXPECT(GetAcceptedLeagueEventMember(0, &team));
    EXPECT_EQ(memcmp(team, &first, sizeof(first)), 0);
    ClearSav1();
    ClearSav2();
    ClearSav3();
    EXPECT_EQ(LoadGameSave(SAVE_NORMAL), SAVE_STATUS_OK);
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(GetAcceptedLeagueEventId(), eventId);
    EXPECT_EQ(GetAcceptedLeagueEventWorldProgress(), 80);
    EXPECT(GetAcceptedLeagueEventMember(0, &team));
    EXPECT_EQ(memcmp(team, &first, sizeof(first)), 0);
    EXPECT(GetAcceptedLeagueEventMember(LEAGUE_EVENT_LINEUP_SIZE - 1, &team));
    EXPECT_EQ(memcmp(team, &final, sizeof(final)), 0);
    ClearSaveData();
    Save_ResetSaveCounters();
}

TEST("Stale resolution changes no history; loss crowns strongest and fatigues the next League")
{
    struct LeagueEventState before, resolved;
    struct LeagueLineupSelection freshHoenn, fatiguedHoenn;
    const struct LeagueSavedTeam *last;
    u16 recent[LEAGUE_EVENT_LINEUP_SIZE];
    u32 recentInput[LEAGUE_EVENT_LINEUP_SIZE], empty[LEAGUE_EVENT_LINEUP_SIZE] = {0};
    u32 eventId, i;
    u16 strongest;
    PrepareLeagueEventTest(8, 1, 160);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_INDIGO));
    eventId = GetAcceptedLeagueEventId();
    EXPECT(GetAcceptedLeagueEventMember(LEAGUE_EVENT_LINEUP_SIZE - 1, &last));
    strongest = last->characterId;
    before = gSaveBlock3Ptr->leagueEvent;
    EXPECT(!ResolveLeagueEvent(LEAGUE_EVENT_LOST, eventId + 1));
    EXPECT_EQ(memcmp(&gSaveBlock3Ptr->leagueEvent, &before, sizeof(before)), 0);
    EndLeagueRun();
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(GetAcceptedLeagueEventId(), 0);
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_INDIGO), strongest);
    EXPECT(HasLeagueTrainerReigned(LEAGUE_ID_INDIGO, strongest));
    GetLeagueRecentLineup(recent);
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
    {
        EXPECT_NE(recent[i], 0);
        recentInput[i] = recent[i];
    }
    EXPECT(SelectLeagueLineup(LEAGUE_ID_HOENN, 160, empty, 0, 0, &freshHoenn));
    EXPECT(SelectLeagueLineup(LEAGUE_ID_HOENN, 160, recentInput, 0, 0, &fatiguedHoenn));
    EXPECT_NE(memcmp(&freshHoenn, &fatiguedHoenn, sizeof(freshHoenn)), 0);
    resolved = gSaveBlock3Ptr->leagueEvent;
    EXPECT(!ResolveLeagueEvent(LEAGUE_EVENT_LOST, eventId));
    EXPECT_EQ(memcmp(&gSaveBlock3Ptr->leagueEvent, &resolved, sizeof(resolved)), 0);
    ClearSaveData();
    Save_ResetSaveCounters();
}

TEST("Leaving a Hoenn event crowns its strongest member and records a Hoenn reign")
{
    const struct LeagueSavedTeam *last;
    u32 eventId;
    u16 strongest;
    PrepareLeagueEventTest(8, 1, 160);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_HOENN));
    eventId = GetAcceptedLeagueEventId();
    EXPECT(GetAcceptedLeagueEventMember(LEAGUE_EVENT_LINEUP_SIZE - 1, &last));
    strongest = last->characterId;
    EXPECT(ResolveLeagueEvent(LEAGUE_EVENT_EXITED, eventId));
    EndLeagueRun(); // The circuit owns the active-run cleanup after resolution.
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_HOENN), strongest);
    EXPECT(HasLeagueTrainerReigned(LEAGUE_ID_HOENN, strongest));
    EXPECT(!HasLeagueTrainerReigned(LEAGUE_ID_INDIGO, strongest));
    ClearSaveData();
    Save_ResetSaveCounters();
}

TEST("Masters Gallery counts saturate for a notable champion and the player")
{
    const struct LeagueSavedTeam *last;
    u32 eventId;
    u16 strongest;
    PrepareLeagueEventTest(8, 1, 160);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, TRUE);
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_MASTERS));
    eventId = GetAcceptedLeagueEventId();
    EXPECT(GetAcceptedLeagueEventMember(LEAGUE_EVENT_LINEUP_SIZE - 1, &last));
    strongest = last->characterId;
    gSaveBlock3Ptr->leagueEvent.galleryWins[strongest] = USHRT_MAX;
    SealLeagueEventStateForTesting();
    EXPECT(ValidateLeagueEventState());
    EndLeagueRun();
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_MASTERS), strongest);
    EXPECT_EQ(GetLeagueGalleryWins(strongest), USHRT_MAX);
    EXPECT(!HasLeagueTrainerReigned(LEAGUE_ID_INDIGO, strongest));
    EXPECT(!HasLeagueTrainerReigned(LEAGUE_ID_HOENN, strongest));
    EXPECT(!ResolveLeagueEvent(LEAGUE_EVENT_LOST, eventId));

    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_MASTERS));
    eventId = GetAcceptedLeagueEventId();
    gSaveBlock3Ptr->leagueEvent.galleryWins[0] = USHRT_MAX;
    SealLeagueEventStateForTesting();
    EXPECT(ResolveLeagueEvent(LEAGUE_EVENT_WON, eventId));
    EndLeagueRun();
    SetCircuitClearForTesting(CIRCUIT_STAGE_MASTERS, TRUE);
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(GetLeagueReigningChampion(LEAGUE_ID_MASTERS), LEAGUE_EVENT_PLAYER_CHAMPION);
    EXPECT_EQ(GetLeagueGalleryWins(0), USHRT_MAX);
    ClearSaveData();
    Save_ResetSaveCounters();
}

TEST("Corrupt League state and team fail closed; content changes return acceptance to invitation")
{
    struct LeagueEventState *state;
    struct LeagueSavedTeams *teams;
    u32 eventId, oldVersion;
    u8 oldLevel;
    PrepareLeagueEventTest(8, 0, 80);
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_INDIGO), LEAGUE_ACCEPT_OK);
    state = &gSaveBlock3Ptr->leagueEvent;
    teams = &gPokemonStoragePtr->leagueEventTeams;
    eventId = GetAcceptedLeagueEventId();
    state->daysRemaining ^= 1;
    EXPECT(!ValidateLeagueEventState());
    EXPECT_EQ(AcceptLeagueEvent(LEAGUE_ID_INDIGO), LEAGUE_ACCEPT_SELECTION_FAILED);
    state->daysRemaining ^= 1;
    EXPECT(ValidateLeagueEventState());
    oldLevel = teams->lineup[0].members[0].level;
    teams->lineup[0].members[0].level = 0;
    EXPECT(!ValidateLeagueEventState());
    teams->lineup[0].members[0].level = oldLevel;
    EXPECT(ValidateLeagueEventState());
    oldVersion = state->contentVersions[0];
    state->contentVersions[0] = oldVersion + 1;
    SealLeagueEventStateForTesting();
    EXPECT(ValidateLeagueEventState());
    EXPECT_EQ(state->invitationState, LEAGUE_INVITATION_INVITED);
    EXPECT_EQ(state->invitedLeague, LEAGUE_ID_INDIGO);
    EXPECT_EQ(state->eventId, eventId);
    EXPECT_EQ(GetAcceptedLeagueEventId(), 0);
    EXPECT_EQ(teams->magic, 0);
    EXPECT_EQ(state->contentVersions[0], oldVersion);
    ClearSaveData();
    Save_ResetSaveCounters();
}

#endif // WAYFARER_LEAGUE_EVENTS
