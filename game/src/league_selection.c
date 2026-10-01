#include "global.h"
#include "league_selection.h"
#include "notable_trainers.h"
#include "constants/event_objects.h"
#include "constants/notable_trainers.h"
#include "constants/opponents.h"
#include "constants/opponents_hns.h"
#include "constants/wayfarer_indigo_trainers.h"
#include "constants/wayfarer_viridian_trainers.h"

#if IS_WAYFARER

#include "data/notable_trainers/league_registry.h"

struct LeagueCandidate
{
    struct LeagueLineupMember member;
    bool8 aloof;
    bool8 master;
    bool8 seated;
};

static bool32 IsHigherScore(const struct LeagueCandidate *left, const struct LeagueCandidate *right)
{
    return left->member.leagueScore > right->member.leagueScore
        || (left->member.leagueScore == right->member.leagueScore
         && left->member.characterId < right->member.characterId);
}

static bool32 IsEarlierBattle(const struct LeagueLineupMember *left, const struct LeagueLineupMember *right)
{
    return left->trainerTR < right->trainerTR
        || (left->trainerTR == right->trainerTR && left->characterId < right->characterId);
}

const struct LeagueTrainer *GetLeagueTrainer(u32 characterId)
{
    if (characterId < 1 || characterId > NOTABLE_TRAINER_COUNT)
        return NULL;
    return &gLeagueTrainers[characterId - 1];
}

const struct LeagueTrainer *GetLeagueTrainerByIndex(u32 index)
{
    if (index >= NOTABLE_TRAINER_COUNT)
        return NULL;
    return &gLeagueTrainers[index];
}

u32 GetLeagueTrainerCount(void)
{
    return NOTABLE_TRAINER_COUNT;
}

bool32 IsLeagueTrainerRegistryValid(void)
{
    u32 i;
    if (!IsNotableTrainerCatalogValid())
        return FALSE;
    for (i = 0; i < NOTABLE_TRAINER_COUNT; i++)
    {
        const struct LeagueTrainer *entry = &gLeagueTrainers[i];
        const struct NotableTrainer *trainer = GetNotableTrainerById(i + 1);
        if (entry->characterId != i + 1 || entry->sourceTrainerId == TRAINER_NONE
         || entry->sourceTrainerId >= TRAINERS_COUNT || entry->presentationId != i + 1
         || entry->objectGraphicsId >= NUM_OBJ_EVENT_GFX || entry->enabled > TRUE
         || entry->enabled == trainer->isDoubleBattle
         || GetNotableTrainerForEncounter(entry->sourceTrainerId) != trainer)
            return FALSE;
        if (entry->enabled && (GetTrainerPartySizeFromId(entry->sourceTrainerId) == 0
         || GetTrainerBattleType(entry->sourceTrainerId) != TRAINER_BATTLE_TYPE_SINGLES))
            return FALSE;
    }
    return TRUE;
}

static bool32 HasRecentTrainer(const u32 recentIds[LEAGUE_LINEUP_SIZE], u32 characterId)
{
    u32 i;
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
        if (recentIds[i] == characterId)
            return TRUE;
    return FALSE;
}

static bool32 AreRecentIdsValid(const u32 recentIds[LEAGUE_LINEUP_SIZE])
{
    u32 i, j, count = 0;
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
    {
        if (recentIds[i] == 0)
            continue;
        if (GetLeagueTrainer(recentIds[i]) == NULL || !GetLeagueTrainer(recentIds[i])->enabled)
            return FALSE;
        count++;
        for (j = i + 1; j < LEAGUE_LINEUP_SIZE; j++)
            if (recentIds[i] == recentIds[j])
                return FALSE;
    }
    return count == 0 || count == LEAGUE_LINEUP_SIZE;
}

bool32 SelectLeagueLineup(enum LeagueId leagueId, u32 worldProgress,
                          const u32 recentIds[LEAGUE_LINEUP_SIZE],
                          u64 reignedIndigoMask, u64 reignedHoennMask,
                          struct LeagueLineupSelection *out)
{
    struct LeagueCandidate candidates[NOTABLE_TRAINER_COUNT];
    u32 i, j, candidateCount = 0, baseCount = 0, baseLevel = 0;
    if (out == NULL)
        return FALSE;
    *out = (struct LeagueLineupSelection){0};
    if (leagueId < LEAGUE_ID_INDIGO || leagueId > LEAGUE_ID_HOENN
     || recentIds == NULL || !AreRecentIdsValid(recentIds)
     || ((reignedIndigoMask | reignedHoennMask) >> NOTABLE_TRAINER_COUNT) != 0
     || !IsLeagueTrainerRegistryValid())
        return FALSE;

    for (i = 0; i < GetLeagueTrainerCount(); i++)
    {
        const struct LeagueTrainer *entry = GetLeagueTrainerByIndex(i);
        const struct NotableTrainer *trainer;
        struct LeagueCandidate *candidate;
        u32 travelCost, fatigue, willingness;
        bool32 atHome;
        if (!entry->enabled)
            continue;
        trainer = GetNotableTrainerById(entry->characterId);
        candidate = &candidates[candidateCount++];
        candidate->member.characterId = entry->characterId;
        candidate->member.trainerTR = GetNotableTrainerRating(trainer, worldProgress);
        candidate->member.teamLevel = GetNotableTrainerTeamLevel(candidate->member.trainerTR);
        atHome = leagueId == LEAGUE_ID_MASTERS
              || (leagueId == LEAGUE_ID_INDIGO && trainer->homeRegion != NOTABLE_REGION_HOENN)
              || (leagueId == LEAGUE_ID_HOENN && trainer->homeRegion == NOTABLE_REGION_HOENN);
        travelCost = atHome ? 0 : (trainer->traveller ? 10 : 80);
        fatigue = HasRecentTrainer(recentIds, entry->characterId) ? 50 : 0;
        willingness = 100 - travelCost;
        willingness = willingness > fatigue + 5 ? willingness - fatigue : 5;
        candidate->member.willingness = willingness;
        candidate->member.leagueScore = candidate->member.trainerTR * willingness / 100;
        candidate->aloof = trainer->aloof;
        candidate->master = (reignedIndigoMask & (1ULL << i)) && (reignedHoennMask & (1ULL << i));
        candidate->seated = FALSE;
    }

    // Rank once by score; both the base lineup and the seat fills read this order.
    for (i = 1; i < candidateCount; i++)
    {
        struct LeagueCandidate next = candidates[i];
        for (j = i; j > 0 && IsHigherScore(&next, &candidates[j - 1]); j--)
            candidates[j] = candidates[j - 1];
        candidates[j] = next;
    }

    if (leagueId != LEAGUE_ID_MASTERS)
    {
        for (i = 0; i < candidateCount && baseCount < LEAGUE_LINEUP_SIZE; i++)
        {
            if (candidates[i].aloof)
                continue;
            if (candidates[i].member.teamLevel > baseLevel)
                baseLevel = candidates[i].member.teamLevel;
            baseCount++;
        }
    }
    else
    {
        for (i = 0; i < candidateCount && out->count < LEAGUE_LINEUP_SIZE; i++)
        {
            if (!candidates[i].master)
                continue;
            candidates[i].seated = TRUE;
            out->members[out->count++] = candidates[i].member;
        }
    }

    for (i = 0; i < candidateCount && out->count < LEAGUE_LINEUP_SIZE; i++)
    {
        if (candidates[i].seated)
            continue;
        if (leagueId != LEAGUE_ID_MASTERS && candidates[i].aloof
         && (baseCount == 0 || candidates[i].member.teamLevel > baseLevel + 10))
            continue;
        out->members[out->count++] = candidates[i].member;
    }
    if (out->count != LEAGUE_LINEUP_SIZE)
    {
        *out = (struct LeagueLineupSelection){0};
        return FALSE;
    }
    for (i = 1; i < LEAGUE_LINEUP_SIZE; i++)
    {
        struct LeagueLineupMember next = out->members[i];
        for (j = i; j > 0 && IsEarlierBattle(&next, &out->members[j - 1]); j--)
            out->members[j] = out->members[j - 1];
        out->members[j] = next;
    }
    return TRUE;
}

#else

const struct LeagueTrainer *GetLeagueTrainer(u32 characterId) { return NULL; }
const struct LeagueTrainer *GetLeagueTrainerByIndex(u32 index) { return NULL; }
u32 GetLeagueTrainerCount(void) { return 0; }
bool32 IsLeagueTrainerRegistryValid(void) { return FALSE; }
bool32 SelectLeagueLineup(enum LeagueId leagueId, u32 worldProgress,
                          const u32 recentIds[LEAGUE_LINEUP_SIZE],
                          u64 reignedIndigoMask, u64 reignedHoennMask,
                          struct LeagueLineupSelection *out)
{
    if (out != NULL)
        *out = (struct LeagueLineupSelection){0};
    return FALSE;
}

#endif // IS_WAYFARER
