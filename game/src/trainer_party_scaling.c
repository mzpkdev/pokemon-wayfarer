#include "global.h"
#include "pokemon.h"
#include "data.h"
#include "config/battle.h"
#include "trainer_rating.h"
#include "trainer_party_scaling.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/trainers.h"
#include "constants/opponents.h"

struct TrainerScalingPredecessor
{
    u16 species;
    u16 predecessor;
    u8 level;
};

struct TrainerScalingMoveException
{
    u16 owner;
    u8 variant;
    u8 slot;
};

#if IS_WAYFARER
#include "data/trainer_scaling/move_exceptions.h"
#include "data/trainer_scaling/policies.h"
#include "data/trainer_scaling/predecessors.h"
#if B_GYM_LEADER_SCALING
#include "data/trainer_scaling/gym_leaders.h"
#endif
#endif

static EWRAM_DATA bool8 sHasRatingSnapshot = FALSE;
static EWRAM_DATA u8 sRatingSnapshot = 0;

u8 GetTrainerScalingLevel(u32 rating, u32 authoredLevel, u32 policy)
{
    static const u8 anchors[][2] = {{0, 7}, {4, 8}, {8, 10}, {16, 15}, {30, 22}, {40, 34}, {55, 52}, {65, 72}, {80, 92}};
    u32 i;
    s32 adjustment, level;
    rating = min(rating, 80);
    authoredLevel = min(max(authoredLevel, 1), 100);
    adjustment = (s32)authoredLevel - 5;
    adjustment = adjustment < 0 ? -((-adjustment + 2) / 5) : (adjustment + 2) / 5;
    adjustment = min(max(adjustment, -1), 8);
    level = 92;
    for (i = 1; i < ARRAY_COUNT(anchors); i++)
    {
        if (rating <= anchors[i][0])
        {
            u32 width = anchors[i][0] - anchors[i - 1][0];
            u32 rise = (rating - anchors[i - 1][0]) * (anchors[i][1] - anchors[i - 1][1]);
            level = anchors[i - 1][1] + (2 * rise + width) / (2 * width);
            break;
        }
    }
    return min(max(level + adjustment + (policy == TRAINER_SCALING_GYM_MEMBER ? 2 : 0), 1), 100);
}

u8 GetGymLeaderScalingPartySize(u32 rating)
{
    rating = min(rating, 80);
    if (rating < 8)
        return 2;
    if (rating < 22)
        return 3;
    if (rating < 34)
        return 4;
    if (rating < 40)
        return 5;
    return PARTY_SIZE;
}

u8 GetGymLeaderScalingLevel(u32 rating, s8 levelOffset)
{
    static const u8 anchors[][2] = {{0, 15}, {4, 16}, {8, 18}, {16, 23}, {30, 30}, {40, 42}, {55, 60}, {65, 80}, {80, 100}};
    u32 i;
    s32 level = 100;

    rating = min(rating, 80);
    for (i = 1; i < ARRAY_COUNT(anchors); i++)
    {
        if (rating <= anchors[i][0])
        {
            u32 width = anchors[i][0] - anchors[i - 1][0];
            u32 rise = (rating - anchors[i - 1][0]) * (anchors[i][1] - anchors[i - 1][1]);
            level = anchors[i - 1][1] + (2 * rise + width) / (2 * width);
            break;
        }
    }
    return min(max(level + levelOffset, 1), 100);
}

static bool32 IsGymLeaderScalingRosterValid(const struct GymLeaderScalingRoster *roster)
{
    u8 orderMask = 0;
    u32 i;

    if (roster == NULL || roster->party == NULL || roster->legacyParty == NULL
     || roster->legacyPartySize == 0 || roster->legacyPartySize > PARTY_SIZE)
        return FALSE;
    if (!roster->slots[0].isAce)
        return FALSE;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        const struct GymLeaderScalingSlot *slot = &roster->slots[i];
        if (slot->battleOrder >= PARTY_SIZE || (orderMask & (1 << slot->battleOrder)))
            return FALSE;
        if (slot->movePolicy != GYM_LEADER_MOVE_AUTHORED
         && slot->movePolicy != GYM_LEADER_MOVE_LEVEL_UP)
            return FALSE;
        if ((slot->isAce && (i > 1 || slot->levelOffset != 0))
         || (!slot->isAce && slot->levelOffset != -1 && slot->levelOffset != -2))
            return FALSE;
        orderMask |= 1 << slot->battleOrder;
    }
    if (roster->isDoubleBattle)
    {
        // Tate/Liza's two retained aces must also be the two constructed
        // opening members when the prefix has only two Pokémon.
        if (!roster->slots[1].isAce
         || roster->slots[0].battleOrder != 0
         || roster->slots[1].battleOrder != 1)
            return FALSE;
    }
    else
    {
        for (i = 0; i < PARTY_SIZE; i++)
            for (u32 j = 0; j < PARTY_SIZE; j++)
                if (roster->slots[i].isAce && !roster->slots[j].isAce
                 && roster->slots[i].battleOrder < roster->slots[j].battleOrder)
                    return FALSE;
    }
    return orderMask == (1 << PARTY_SIZE) - 1;
}

bool32 BuildGymLeaderScalingPlan(const struct GymLeaderScalingRoster *roster, u32 rating, struct GymLeaderScalingPlan *plan)
{
    u32 i, j;
    u8 count;

    if (plan == NULL || !IsGymLeaderScalingRosterValid(roster))
        return FALSE;

    count = GetGymLeaderScalingPartySize(rating);
    plan->count = count;
    for (i = 0; i < count; i++)
    {
        plan->sourceIndices[i] = i;
        plan->levels[i] = GetGymLeaderScalingLevel(rating, roster->slots[i].levelOffset);
    }

    // A stable insertion sort keeps authored source order for a malformed tie,
    // though valid generated metadata always provides a full permutation.
    for (i = 1; i < count; i++)
    {
        u8 sourceIndex = plan->sourceIndices[i];
        u8 level = plan->levels[i];
        for (j = i; j > 0
             && roster->slots[plan->sourceIndices[j - 1]].battleOrder > roster->slots[sourceIndex].battleOrder;
             j--)
        {
            plan->sourceIndices[j] = plan->sourceIndices[j - 1];
            plan->levels[j] = plan->levels[j - 1];
        }
        plan->sourceIndices[j] = sourceIndex;
        plan->levels[j] = level;
    }
    for (; i < PARTY_SIZE; i++)
    {
        plan->sourceIndices[i] = 0;
        plan->levels[i] = 0;
    }
    return TRUE;
}

#if IS_WAYFARER && B_GYM_LEADER_SCALING
const struct GymLeaderScalingRoster *GetGymLeaderScalingRoster(u16 trainerId, u16 ownerId, u8 difficulty)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sGymLeaderScalingRosters); i++)
    {
        const struct GymLeaderScalingRoster *roster = &sGymLeaderScalingRosters[i];
        if (roster->trainerId == trainerId && roster->ownerId == ownerId && roster->difficulty == difficulty)
            return roster;
    }
    return NULL;
}
#endif

u32 GetTrainerScalingPolicy(u32 trainerId)
{
#if IS_WAYFARER
    if (trainerId < ARRAY_COUNT(sTrainerScalingPolicies))
        return sTrainerScalingPolicies[trainerId];
#endif
    return TRAINER_SCALING_EXCLUDED;
}

u16 ResolveTrainerScalingSpecies(u16 species, u8 level)
{
#if IS_WAYFARER
    u32 depth;
    for (depth = 0; depth < ARRAY_COUNT(sTrainerScalingPredecessors); depth++)
    {
        u32 low = 0, high = ARRAY_COUNT(sTrainerScalingPredecessors);
        while (low < high)
        {
            u32 mid = low + (high - low) / 2;
            if (sTrainerScalingPredecessors[mid].species < species)
                low = mid + 1;
            else
                high = mid;
        }
        if (low == ARRAY_COUNT(sTrainerScalingPredecessors)
         || sTrainerScalingPredecessors[low].species != species
         || sTrainerScalingPredecessors[low].predecessor == SPECIES_NONE
         || level >= sTrainerScalingPredecessors[low].level)
            break;
        species = sTrainerScalingPredecessors[low].predecessor;
    }
#endif
    return species;
}

bool32 IsTrainerScalingBattleContext(u32 flags)
{
    return (flags & BATTLE_TYPE_TRAINER)
        && !(flags & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_EREADER_TRAINER
                    | BATTLE_TYPE_TRAINER_HILL | BATTLE_TYPE_SECRET_BASE | BATTLE_TYPE_RECORDED
                    | BATTLE_TYPE_RECORDED_LINK | BATTLE_TYPE_FIRST_BATTLE | BATTLE_TYPE_CATCH_TUTORIAL
                    | BATTLE_TYPE_POKEDUDE | BATTLE_TYPE_SAFARI | BATTLE_TYPE_RAID
                    | BATTLE_TYPE_INGAME_PARTNER));
}

void ResetTrainerScalingSnapshot(void)
{
    sHasRatingSnapshot = FALSE;
    sRatingSnapshot = 0;
}

u8 GetTrainerScalingSnapshot(void)
{
    if (!sHasRatingSnapshot)
    {
        sRatingSnapshot = GetTrainerRating();
        sHasRatingSnapshot = TRUE;
    }
    return sRatingSnapshot;
}

u32 GetTrainerScalingAbility(u16 species, u32 authoredAbility, u32 personalityHash)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];
    u32 i, count = 0;
    u32 legal[NUM_ABILITY_SLOTS];
    for (i = 0; i < ARRAY_COUNT(info->abilities); i++)
    {
        if (authoredAbility != ABILITY_NONE && info->abilities[i] == authoredAbility)
            return i;
        if (info->abilities[i] != ABILITY_NONE)
            legal[count++] = i;
    }
    if (authoredAbility == ABILITY_NONE && B_TRAINER_MON_RANDOM_ABILITY && count)
        return legal[personalityHash % count];
    for (i = 0; i < min(2, ARRAY_COUNT(info->abilities)); i++)
        if (info->abilities[i] != ABILITY_NONE)
            return i;
    return count ? legal[0] : 0;
}

bool32 HasTrainerScalingMoveException(u32 owner, u32 variant, u32 slot)
{
#if IS_WAYFARER
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sTrainerScalingMoveExceptions); i++)
        if (sTrainerScalingMoveExceptions[i].owner == owner
         && sTrainerScalingMoveExceptions[i].variant == variant
         && sTrainerScalingMoveExceptions[i].slot == slot)
            return TRUE;
#endif
    return FALSE;
}

bool32 CanRetainTrainerScalingMoves(const struct TrainerMon *entry, u16 species, u8 level)
{
    const struct LevelUpMove *learnset;
    u32 i, j;
    bool32 hasMove = FALSE;
    if (species != entry->species)
        return FALSE;
    learnset = GetSpeciesLevelUpLearnset(species);
    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        bool32 found = FALSE;
        if (entry->moves[i] == MOVE_NONE)
            continue;
        hasMove = TRUE;
        for (j = 0; learnset[j].move != LEVEL_UP_MOVE_END; j++)
            if (learnset[j].move == entry->moves[i] && learnset[j].level <= level)
            {
                found = TRUE;
                break;
            }
        if (!found)
            return FALSE;
    }
    return hasMove;
}
