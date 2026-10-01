#include "global.h"
#include "league_events.h"
#include "league_circuit.h"
#include "event_data.h"
#include "notable_trainers.h"
#include "pokemon_storage_system.h"
#include "load_save.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "malloc.h"
#include "random.h"
#include "randomizer.h"
#include "save.h"
#include "config/league_circuit.h"
#include "wayfarer_world.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/regions.h"

#if WAYFARER_LEAGUE_EVENTS

#define LEAGUE_EVENT_STATE_MAGIC 0x4C455631 // LEV1
#define LEAGUE_EVENT_TEAMS_MAGIC 0x4C455654 // LEVT

// Increment the relevant entry whenever accepted-lineup meaning changes.
static const u32 sLeagueContentVersions[LEAGUE_EVENT_CONTENT_VERSION_COUNT] =
{
    2, // league registry and hall conditions
    1, // trainer rosters
    1, // world progress and trainer scalers
    1, // growth archetypes
    1, // evolution-level table
    2, // move pools and learnsets
    1, // play styles and AI tiers
};

static bool32 IsLeagueId(enum LeagueId league)
{
    return league >= LEAGUE_ID_INDIGO && league <= LEAGUE_ID_HOENN;
}

static u32 HashBytes(const void *data, u32 size)
{
    const u8 *bytes = data;
    u32 hash = 2166136261u;
    u32 i;
    for (i = 0; i < size; i++)
        hash = (hash ^ bytes[i]) * 16777619u;
    return hash;
}

static u16 StateChecksum(const struct LeagueEventState *state)
{
    u32 hash = HashBytes(state, offsetof(struct LeagueEventState, stateChecksum));
    return hash ^ (hash >> 16);
}

static void SealState(void)
{
    gSaveBlock3Ptr->leagueEvent.stateChecksum = StateChecksum(&gSaveBlock3Ptr->leagueEvent);
}

static void ClearAcceptedTeams(void)
{
    memset(&gPokemonStoragePtr->leagueEventTeams, 0, sizeof(gPokemonStoragePtr->leagueEventTeams));
    gSaveBlock3Ptr->leagueEvent.lineupChecksum = 0;
    gSaveBlock3Ptr->leagueEvent.acceptedLeague = LEAGUE_ID_NONE;
    gSaveBlock3Ptr->leagueEvent.acceptedWorldProgress = 0;
}

void InitLeagueEventState(void)
{
    struct LeagueEventState *state = &gSaveBlock3Ptr->leagueEvent;
    memset(state, 0, sizeof(*state));
    if (gPokemonStoragePtr != NULL)
        memset(&gPokemonStoragePtr->leagueEventTeams, 0, sizeof(gPokemonStoragePtr->leagueEventTeams));
    state->magic = LEAGUE_EVENT_STATE_MAGIC;
    memcpy(state->contentVersions, sLeagueContentVersions, sizeof(sLeagueContentVersions));
    state->invitationState = LEAGUE_INVITATION_NOT_QUALIFIED;
    SealState();
}

static bool32 IsKnownCharacter(u32 characterId)
{
    const struct LeagueTrainer *entry = GetLeagueTrainer(characterId);
    return entry != NULL && entry->characterId == characterId;
}

static bool32 IsEligibleCharacter(u32 characterId)
{
    const struct LeagueTrainer *entry = GetLeagueTrainer(characterId);
    return entry != NULL && entry->enabled && entry->characterId == characterId;
}

static void DropRemovedCharacters(struct LeagueEventState *state)
{
    u32 i, league;
    for (i = 1; i <= LEAGUE_EVENT_NOTABLE_COUNT; i++)
        if (!IsKnownCharacter(i))
        {
            state->reignedIndigoMask &= ~(1ull << (i - 1));
            state->reignedHoennMask &= ~(1ull << (i - 1));
            state->galleryWins[i] = 0;
        }
    for (league = 0; league < 3; league++)
        if (state->champions[league] != 0
         && state->champions[league] != LEAGUE_EVENT_PLAYER_CHAMPION
         && !IsKnownCharacter(state->champions[league]))
            state->champions[league] = 0;
}

static bool32 IsSavedMonValid(const struct LeagueSavedMon *mon)
{
    u32 i, moveCount = 0;
    if (mon->species == SPECIES_NONE || mon->species >= NUM_SPECIES
     || mon->heldItem >= ITEMS_COUNT || mon->ability == ABILITY_NONE
     || mon->ability >= ABILITIES_COUNT
     || mon->abilityNum >= NUM_ABILITY_SLOTS
     || mon->level == 0 || mon->level > 100
     || mon->initialLevel == 0 || mon->initialLevel > 100
     || mon->nature >= NUM_NATURES || mon->teraType >= NUMBER_OF_MON_TYPES
     || mon->types[0] >= NUMBER_OF_MON_TYPES || mon->types[1] >= NUMBER_OF_MON_TYPES
     || mon->dynamaxLevel > 10 || (mon->iv & 0xC0000000) != 0
     || (mon->flags & ~(LEAGUE_SAVED_MON_GENDER_MASK | LEAGUE_SAVED_MON_SHINY
                      | LEAGUE_SAVED_MON_GIGANTAMAX | LEAGUE_SAVED_MON_CAN_DYNAMAX)) != 0)
        return FALSE;
    for (i = 0; i < 4; i++)
    {
        if (mon->moves[i] >= MOVES_COUNT_ALL)
            return FALSE;
        if (mon->moves[i] != MOVE_NONE)
            moveCount++;
    }
    return moveCount != 0;
}

static bool32 IsSavedTeamValid(const struct LeagueSavedTeam *team, bool32 randomizedParty)
{
    const struct LeagueTrainer *entry = GetLeagueTrainer(team->characterId);
    u8 seen = 0, sourceSeen = 0;
    u32 i;
    if (entry == NULL || !entry->enabled || entry->sourceTrainerId != team->sourceTrainerId
     || team->teamSize == 0 || team->teamSize > PARTY_SIZE
     || (!randomizedParty && team->aceCount == 0)
     || team->aceCount > 3 || team->aceCount > team->teamSize
     || team->aiFlags == 0)
        return FALSE;
    for (i = 0; i < team->teamSize; i++)
    {
        u8 slot = team->battleOrder[i];
        u8 sourceSlot = team->members[i].rosterSlot;
        if (slot >= team->teamSize || (seen & (1 << slot))
         || sourceSlot >= PARTY_SIZE || (sourceSeen & (1 << sourceSlot))
         || !IsSavedMonValid(&team->members[i]))
            return FALSE;
        seen |= 1 << slot;
        sourceSeen |= 1 << sourceSlot;
    }
    return seen == (1 << team->teamSize) - 1;
}

static bool32 IsRecentLineupValid(const struct LeagueEventState *state)
{
    u32 i, j, count = 0;
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
    {
        u16 id = state->recentIds[i];
        if (id == 0)
            continue;
        if (!IsKnownCharacter(id))
            return FALSE;
        count++;
        for (j = i + 1; j < LEAGUE_EVENT_LINEUP_SIZE; j++)
            if (id == state->recentIds[j])
                return FALSE;
    }
    return count == 0 || count == LEAGUE_EVENT_LINEUP_SIZE;
}

static bool32 HasLifetimeWin(enum LeagueId league)
{
    return HasClearedCircuitStage((enum CircuitStage)league);
}

static bool32 IsLeagueEligible(enum LeagueId league)
{
    if (league == LEAGUE_ID_INDIGO)
        return GetBadgeCountForRegion(REGION_KANTO) + GetBadgeCountForRegion(REGION_JOHTO) != 0;
    if (league == LEAGUE_ID_HOENN)
        return GetBadgeCountForRegion(REGION_HOENN) != 0;
    return HasLifetimeWin(LEAGUE_ID_INDIGO) && HasLifetimeWin(LEAGUE_ID_HOENN);
}

bool32 ValidateLeagueEventState(void)
{
    struct LeagueEventState *state = &gSaveBlock3Ptr->leagueEvent;
    struct LeagueSavedTeams *teams = &gPokemonStoragePtr->leagueEventTeams;
    bool32 contentChanged;
    u32 i, j, latestCall = 0, matchedLatest = 0;
    if (state->magic != LEAGUE_EVENT_STATE_MAGIC || state->stateChecksum != StateChecksum(state)
     || state->invitationState > LEAGUE_INVITATION_ACCEPTED)
        return FALSE;
    if (state->invitationState == LEAGUE_INVITATION_ACCEPTED)
    {
        if (!IsLeagueId(state->acceptedLeague) || state->eventId == 0
         || teams->magic != LEAGUE_EVENT_TEAMS_MAGIC || teams->eventId != state->eventId
         || (teams->acceptanceOptions & 0x80) != 0
         || state->lineupChecksum != HashBytes(teams, sizeof(*teams)))
            return FALSE;
        for (i = 0; i < ARRAY_COUNT(teams->reserved); i++)
            if (teams->reserved[i] != 0)
                return FALSE;
    }
    else if (state->acceptedLeague != LEAGUE_ID_NONE || state->lineupChecksum != 0)
        return FALSE;

    DropRemovedCharacters(state);
    contentChanged = memcmp(state->contentVersions, sLeagueContentVersions,
                            sizeof(sLeagueContentVersions)) != 0;
    if (contentChanged)
    {
        if (state->invitationState == LEAGUE_INVITATION_ACCEPTED)
        {
            state->invitationState = LEAGUE_INVITATION_INVITED;
            state->invitedLeague = state->acceptedLeague;
            ClearAcceptedTeams();
            memset(&gSaveBlock3Ptr->wayfarerHoenn.leagueRun, 0,
                   sizeof(gSaveBlock3Ptr->wayfarerHoenn.leagueRun));
        }
        memset(state->recentIds, 0, sizeof(state->recentIds));
        for (i = 0; i < 3; i++)
            if (state->champions[i] != 0 && state->champions[i] != LEAGUE_EVENT_PLAYER_CHAMPION
             && !IsEligibleCharacter(state->champions[i]))
                state->champions[i] = 0;
        memcpy(state->contentVersions, sLeagueContentVersions, sizeof(sLeagueContentVersions));
    }
    // Removed historical characters are pruned even without a version bump.
    SealState();

    for (i = 0; i < 3; i++)
    {
        u32 call = state->lastCallNumber[i];
        u16 champion = state->champions[i];
        if (call > state->callCounter)
            return FALSE;
        if (call == state->callCounter && call != 0)
            matchedLatest++;
        if (call > latestCall)
            latestCall = call;
        for (j = i + 1; j < 3; j++)
            if (call != 0 && call == state->lastCallNumber[j])
                return FALSE;
        if (champion != 0 && champion != LEAGUE_EVENT_PLAYER_CHAMPION && !IsKnownCharacter(champion))
            return FALSE;
        if (champion == LEAGUE_EVENT_PLAYER_CHAMPION && !HasLifetimeWin((enum LeagueId)(i + 1)))
            return FALSE;
    }
    if ((state->callCounter == 0 && latestCall != 0)
     || (state->callCounter != 0 && (latestCall != state->callCounter || matchedLatest != 1)))
        return FALSE;
    if (((state->reignedIndigoMask | state->reignedHoennMask) >> LEAGUE_EVENT_NOTABLE_COUNT) != 0)
        return FALSE;
    if ((state->lastCallNumber[LEAGUE_ID_MASTERS - 1] != 0
      || state->invitedLeague == LEAGUE_ID_MASTERS)
     && !IsLeagueEligible(LEAGUE_ID_MASTERS))
        return FALSE;
    if (!IsRecentLineupValid(state))
        return FALSE;
    if (state->champions[LEAGUE_ID_INDIGO - 1] != 0
     && state->champions[LEAGUE_ID_INDIGO - 1] != LEAGUE_EVENT_PLAYER_CHAMPION
     && !(state->reignedIndigoMask & (1ull << (state->champions[LEAGUE_ID_INDIGO - 1] - 1))))
        return FALSE;
    if (state->champions[LEAGUE_ID_HOENN - 1] != 0
     && state->champions[LEAGUE_ID_HOENN - 1] != LEAGUE_EVENT_PLAYER_CHAMPION
     && !(state->reignedHoennMask & (1ull << (state->champions[LEAGUE_ID_HOENN - 1] - 1))))
        return FALSE;
    if (state->champions[LEAGUE_ID_MASTERS - 1] != 0
     && state->galleryWins[state->champions[LEAGUE_ID_MASTERS - 1] == LEAGUE_EVENT_PLAYER_CHAMPION
                           ? 0 : state->champions[LEAGUE_ID_MASTERS - 1]] == 0)
        return FALSE;

    if (state->invitationState == LEAGUE_INVITATION_INVITED
     || state->invitationState == LEAGUE_INVITATION_ACCEPTED)
    {
        enum LeagueId invited = state->invitationState == LEAGUE_INVITATION_ACCEPTED
                              ? state->acceptedLeague : state->invitedLeague;
        if (!IsLeagueId(invited) || state->lastCallNumber[invited - 1] != state->callCounter)
            return FALSE;
    }
    if (state->invitationState == LEAGUE_INVITATION_COUNTING_DOWN && state->daysRemaining > 7)
        return FALSE;
    if ((state->invitationState == LEAGUE_INVITATION_NOT_QUALIFIED
      || state->invitationState == LEAGUE_INVITATION_COUNTING_DOWN)
     && state->invitedLeague != LEAGUE_ID_NONE)
        return FALSE;
    if (state->invitationState == LEAGUE_INVITATION_ACCEPTED
     && (state->invitedLeague != state->acceptedLeague || state->acceptedWorldProgress < 80))
        return FALSE;
    if (state->invitationState == LEAGUE_INVITATION_ACCEPTED)
    {
        for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
        {
            const struct LeagueSavedTeam *team = &teams->lineup[i];
            if (!IsSavedTeamValid(team,
                (teams->acceptanceOptions & LEAGUE_ACCEPT_OPTION_RANDOMIZED_PARTY) != 0))
                return FALSE;
            if (i > 0 && (team->trainerTR < teams->lineup[i - 1].trainerTR
             || (team->trainerTR == teams->lineup[i - 1].trainerTR
              && team->characterId <= teams->lineup[i - 1].characterId)))
                return FALSE;
            for (j = i + 1; j < LEAGUE_EVENT_LINEUP_SIZE; j++)
                if (team->characterId == teams->lineup[j].characterId)
                    return FALSE;
        }
    }
    else if (gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active)
        return FALSE;
    if (gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active > TRUE
     || gSaveBlock3Ptr->wayfarerHoenn.leagueRun.replay > TRUE)
        return FALSE;
    if (gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active
     && (gSaveBlock3Ptr->wayfarerHoenn.leagueRun.stage != state->acceptedLeague
      || gSaveBlock3Ptr->wayfarerHoenn.leagueRun.ratingAtEntry != state->acceptedWorldProgress
      || gSaveBlock3Ptr->wayfarerHoenn.leagueRun.replay != HasLifetimeWin(state->acceptedLeague)))
        return FALSE;
    return TRUE;
}

static bool32 PrepareAcceptedLineup(enum LeagueId league, u32 worldProgress,
                                    u32 eventId, struct LeagueSavedTeams *teams)
{
    struct LeagueEventState *state = &gSaveBlock3Ptr->leagueEvent;
    struct LeagueLineupSelection selection;
    u32 recent[LEAGUE_EVENT_LINEUP_SIZE];
    bool32 randomizedSpecies = RandomizerFeatureEnabled(RANDOMIZE_TRAINER_MON);
    u32 i;
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
        recent[i] = state->recentIds[i];
    if (!SelectLeagueLineup(league, worldProgress, recent,
                            state->reignedIndigoMask, state->reignedHoennMask,
                            &selection) || selection.count != LEAGUE_EVENT_LINEUP_SIZE)
        return FALSE;
    memset(teams, 0, sizeof(*teams));
    teams->magic = LEAGUE_EVENT_TEAMS_MAGIC;
    teams->eventId = eventId;
    teams->acceptanceOptions = (FlagGet(FLAG_LIMIT_TO_50) ? LEAGUE_ACCEPT_OPTION_LEVEL_CAP : 0)
        | ((gSaveBlock3Ptr->challengeSettings.tx_Challenges_BaseStatEqualizer << LEAGUE_ACCEPT_OPTION_EQUALIZER_SHIFT)
           & LEAGUE_ACCEPT_OPTION_EQUALIZER_MASK);
    if (randomizedSpecies)
        teams->acceptanceOptions |= LEAGUE_ACCEPT_OPTION_RANDOMIZED_PARTY;
    if (RandomizerFeatureEnabled(RANDOMIZE_BASE_STATS))
        teams->acceptanceOptions |= LEAGUE_ACCEPT_OPTION_RANDOM_BASE_STATS;
    if (RandomizerFeatureEnabled(RANDOMIZE_MON_TYPES))
        teams->acceptanceOptions |= LEAGUE_ACCEPT_OPTION_RANDOM_MON_TYPES;
    if (gSaveBlock3Ptr->challengeSettings.tx_Mode_Legendary_Abilities)
        teams->acceptanceOptions |= LEAGUE_ACCEPT_OPTION_LEGENDARY_ABILITIES;
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
    {
        const struct LeagueTrainer *entry = GetLeagueTrainer(selection.members[i].characterId);
        const struct NotableTrainer *trainer = GetNotableTrainerById(selection.members[i].characterId);
        struct NotableTrainerSnapshot snapshot;
        if (entry == NULL || !entry->enabled || trainer == NULL
         || !ResolveNotableTrainerSnapshot(trainer, worldProgress, randomizedSpecies, &snapshot)
         || snapshot.trainerTR != selection.members[i].trainerTR
         || !BuildLeagueEventSavedTeam(&snapshot, entry->sourceTrainerId, &teams->lineup[i]))
            return FALSE;
    }
    return TRUE;
}

enum LeagueAcceptResult AcceptLeagueEvent(enum LeagueId league)
{
    struct LeagueEventState *state = &gSaveBlock3Ptr->leagueEvent;
    struct LeagueEventState previous;
    struct LeagueSavedTeams *prepared;
    rng_value_t savedRng, savedRng2;
    u32 eventId, worldProgress;
    if (!ValidateLeagueEventState())
        return LEAGUE_ACCEPT_SELECTION_FAILED;
    if (!IsLeagueId(league))
        return LEAGUE_ACCEPT_INELIGIBLE;
    worldProgress = GetTrainerRating();
    if (worldProgress < 80)
        return LEAGUE_ACCEPT_NOT_QUALIFIED;
    if (!IsLeagueEligible(league))
        return LEAGUE_ACCEPT_INELIGIBLE;
    if (state->invitationState == LEAGUE_INVITATION_ACCEPTED
     || (state->invitationState == LEAGUE_INVITATION_INVITED && state->invitedLeague != league)
     || gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active)
        return LEAGUE_ACCEPT_BUSY;
    if (state->eventId == UINT_MAX
     || (state->invitationState != LEAGUE_INVITATION_INVITED && state->callCounter == UINT_MAX))
        return LEAGUE_ACCEPT_SELECTION_FAILED;
    eventId = state->eventId + 1;
    prepared = Alloc(sizeof(*prepared));
    if (prepared == NULL)
        return LEAGUE_ACCEPT_SELECTION_FAILED;
    savedRng = gRngValue;
    savedRng2 = gRng2Value;
    if (!PrepareAcceptedLineup(league, worldProgress, eventId, prepared))
    {
        gRngValue = savedRng;
        gRng2Value = savedRng2;
        Free(prepared);
        return LEAGUE_ACCEPT_SELECTION_FAILED;
    }
    previous = *state;
    if (state->invitationState != LEAGUE_INVITATION_INVITED)
    {
        state->callCounter++;
        state->lastCallNumber[league - 1] = state->callCounter;
    }
    state->eventId = eventId;
    state->acceptedWorldProgress = worldProgress;
    state->acceptedLeague = league;
    state->invitedLeague = league;
    state->invitationState = LEAGUE_INVITATION_ACCEPTED;
    state->daysRemaining = 0;
    memcpy(&gPokemonStoragePtr->leagueEventTeams, prepared, sizeof(*prepared));
    state->lineupChecksum = HashBytes(prepared, sizeof(*prepared));
    SealState();
    Free(prepared);
    gRngValue = savedRng;
    gRng2Value = savedRng2;
    if (TrySavingData(SAVE_NORMAL) != SAVE_STATUS_OK)
    {
        *state = previous;
        memset(&gPokemonStoragePtr->leagueEventTeams, 0, sizeof(gPokemonStoragePtr->leagueEventTeams));
        gRngValue = savedRng;
        gRng2Value = savedRng2;
        return LEAGUE_ACCEPT_SAVE_FAILED;
    }
    return LEAGUE_ACCEPT_OK;
}

u32 GetAcceptedLeagueEventId(void)
{
    return gSaveBlock3Ptr->leagueEvent.invitationState == LEAGUE_INVITATION_ACCEPTED
         ? gSaveBlock3Ptr->leagueEvent.eventId : 0;
}

enum LeagueId GetAcceptedLeagueEventLeagueId(void)
{
    return GetAcceptedLeagueEventId() != 0 ? gSaveBlock3Ptr->leagueEvent.acceptedLeague
                                          : LEAGUE_ID_NONE;
}

u32 GetAcceptedLeagueEventWorldProgress(void)
{
    return GetAcceptedLeagueEventId() != 0 ? gSaveBlock3Ptr->leagueEvent.acceptedWorldProgress : 0;
}

bool32 GetAcceptedLeagueEventMember(u8 match, const struct LeagueSavedTeam **team)
{
    if (team == NULL || match >= LEAGUE_EVENT_LINEUP_SIZE || GetAcceptedLeagueEventId() == 0)
        return FALSE;
    *team = &gPokemonStoragePtr->leagueEventTeams.lineup[match];
    return TRUE;
}

bool32 ResolveLeagueEvent(enum LeagueEventOutcome outcome, u32 expectedEventId)
{
    struct LeagueEventState *state = &gSaveBlock3Ptr->leagueEvent;
    struct LeagueSavedTeams *teams = &gPokemonStoragePtr->leagueEventTeams;
    enum LeagueId league;
    u16 champion;
    u32 i;
    if (!ValidateLeagueEventState())
        return FALSE;
    league = state->acceptedLeague;
    if (expectedEventId == 0 || expectedEventId != GetAcceptedLeagueEventId()
     || outcome > LEAGUE_EVENT_EXITED || !IsLeagueId(league)
     || !gSaveBlock3Ptr->wayfarerHoenn.leagueRun.active
     || gSaveBlock3Ptr->wayfarerHoenn.leagueRun.stage != league)
        return FALSE;
    champion = outcome == LEAGUE_EVENT_WON ? LEAGUE_EVENT_PLAYER_CHAMPION
               : teams->lineup[LEAGUE_EVENT_LINEUP_SIZE - 1].characterId;
    state->champions[league - 1] = champion;
    if (champion != LEAGUE_EVENT_PLAYER_CHAMPION)
    {
        if (league == LEAGUE_ID_INDIGO)
            state->reignedIndigoMask |= 1ull << (champion - 1);
        else if (league == LEAGUE_ID_HOENN)
            state->reignedHoennMask |= 1ull << (champion - 1);
    }
    if (league == LEAGUE_ID_MASTERS)
    {
        u16 *wins = &state->galleryWins[champion == LEAGUE_EVENT_PLAYER_CHAMPION ? 0 : champion];
        if (*wins != USHRT_MAX)
            ++*wins;
    }
    for (i = 0; i < LEAGUE_EVENT_LINEUP_SIZE; i++)
        state->recentIds[i] = teams->lineup[i].characterId;
    // Life events for the overworld simulation, in the same transaction.
    WayfarerWorld_OnLeagueResolved(state->recentIds, LEAGUE_EVENT_LINEUP_SIZE, outcome == LEAGUE_EVENT_WON,
                                   champion == LEAGUE_EVENT_PLAYER_CHAMPION ? 0 : champion);
    state->invitationState = LEAGUE_INVITATION_COUNTING_DOWN;
    state->invitedLeague = LEAGUE_ID_NONE;
    state->daysRemaining = 7;
    // Phone/day scheduling is deferred. Its first active tick must establish
    // lastCountedDay from that feature's in-game counter before subtracting.
    ClearAcceptedTeams();
    SealState();
    return TRUE;
}

u16 GetLeagueReigningChampion(enum LeagueId league)
{
    return IsLeagueId(league) ? gSaveBlock3Ptr->leagueEvent.champions[league - 1] : 0;
}

bool32 HasLeagueTrainerReigned(enum LeagueId league, u32 characterId)
{
    const struct LeagueEventState *state = &gSaveBlock3Ptr->leagueEvent;
    if (!IsKnownCharacter(characterId))
        return FALSE;
    if (league == LEAGUE_ID_INDIGO)
        return (state->reignedIndigoMask >> (characterId - 1)) & 1;
    if (league == LEAGUE_ID_HOENN)
        return (state->reignedHoennMask >> (characterId - 1)) & 1;
    return FALSE;
}

u16 GetLeagueGalleryWins(u32 characterId)
{
    return characterId <= LEAGUE_EVENT_NOTABLE_COUNT
         ? gSaveBlock3Ptr->leagueEvent.galleryWins[characterId] : 0;
}

void GetLeagueRecentLineup(u16 outIds[LEAGUE_EVENT_LINEUP_SIZE])
{
    if (outIds != NULL)
        memcpy(outIds, gSaveBlock3Ptr->leagueEvent.recentIds,
               sizeof(gSaveBlock3Ptr->leagueEvent.recentIds));
}

#if TESTING
void SealLeagueEventStateForTesting(void)
{
    SealState();
}
#endif

#else

void InitLeagueEventState(void) {}
bool32 ValidateLeagueEventState(void) { return TRUE; }
enum LeagueAcceptResult AcceptLeagueEvent(enum LeagueId league) { return LEAGUE_ACCEPT_INELIGIBLE; }
u32 GetAcceptedLeagueEventId(void) { return 0; }
enum LeagueId GetAcceptedLeagueEventLeagueId(void) { return LEAGUE_ID_NONE; }
u32 GetAcceptedLeagueEventWorldProgress(void) { return 0; }
bool32 GetAcceptedLeagueEventMember(u8 match, const struct LeagueSavedTeam **team) { return FALSE; }
bool32 ResolveLeagueEvent(enum LeagueEventOutcome outcome, u32 expectedEventId) { return FALSE; }
u16 GetLeagueReigningChampion(enum LeagueId league) { return 0; }
bool32 HasLeagueTrainerReigned(enum LeagueId league, u32 characterId) { return FALSE; }
u16 GetLeagueGalleryWins(u32 characterId) { return 0; }
void GetLeagueRecentLineup(u16 outIds[LEAGUE_EVENT_LINEUP_SIZE])
{
    if (outIds != NULL)
        memset(outIds, 0, sizeof(u16) * LEAGUE_EVENT_LINEUP_SIZE);
}
#if TESTING
void SealLeagueEventStateForTesting(void) {}
#endif

#endif // WAYFARER_LEAGUE_EVENTS
