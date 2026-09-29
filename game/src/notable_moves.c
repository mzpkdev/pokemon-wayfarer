#include "global.h"
#include "data.h"
#include "pokemon.h"
#include "notable_moves.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"

#define MAX_NOTABLE_ANCESTRY 8

struct NotablePredecessor
{
    u16 species;
    u16 predecessor;
    u8 authoredLevel; // Only non-level methods use this; EVO_LEVEL reads game data.
};

#include "data/notable_moves/predecessors.h"

static const struct NotablePredecessor *FindPredecessor(u16 species)
{
    u32 low = 0, high = ARRAY_COUNT(sNotablePredecessors);

    while (low < high)
    {
        u32 mid = low + (high - low) / 2;
        if (sNotablePredecessors[mid].species < species)
            low = mid + 1;
        else
            high = mid;
    }
    if (low < ARRAY_COUNT(sNotablePredecessors) && sNotablePredecessors[low].species == species)
        return &sNotablePredecessors[low];
    return NULL;
}

// Babies are excluded from party step-down, but own their lines' egg moves.
static const struct NotablePredecessor *FindEggMovePredecessor(u16 species)
{
    const struct NotablePredecessor *edge = FindPredecessor(species);
    u32 i;
    if (edge != NULL)
        return edge;
    for (i = 0; i < ARRAY_COUNT(sNotableBabyPredecessors); i++)
        if (sNotableBabyPredecessors[i].species == species)
            return &sNotableBabyPredecessors[i];
    return NULL;
}

static u8 GetEvolutionLevel(const struct NotablePredecessor *edge)
{
    const struct Evolution *evolutions = GetSpeciesEvolutions(edge->predecessor);
    u32 i;

    for (i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
        if (evolutions[i].targetSpecies == edge->species
         && (evolutions[i].method == EVO_LEVEL || evolutions[i].method == EVO_LEVEL_BATTLE_ONLY)
         && evolutions[i].param != 0)
            return evolutions[i].param;
    return edge->authoredLevel;
}

u16 StepDownSpeciesToLevel(u16 species, u8 level)
{
    u32 depth;

    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return species;
    for (depth = 0; depth < MAX_NOTABLE_ANCESTRY; depth++)
    {
        const struct NotablePredecessor *edge = FindPredecessor(species);
        if (edge == NULL || level >= GetEvolutionLevel(edge))
            break;
        species = edge->predecessor;
    }
    return species;
}

static u16 GetLineBase(u16 species)
{
    u32 depth;
    for (depth = 0; depth < MAX_NOTABLE_ANCESTRY; depth++)
    {
        const struct NotablePredecessor *edge = FindEggMovePredecessor(species);
        if (edge == NULL)
            break;
        species = edge->predecessor;
    }
    return species;
}

static bool32 HasMove(const u16 *moves, u16 move)
{
    u32 i;
    for (i = 0; moves[i] != MOVE_UNAVAILABLE; i++)
        if (moves[i] == move)
            return TRUE;
    return FALSE;
}

static bool32 NaturalLearnLevel(u16 species, u16 move, u16 *level)
{
    u32 depth;
    bool32 found = FALSE;
    *level = MAX_LEVEL + 1;

    for (depth = 0; depth < MAX_NOTABLE_ANCESTRY; depth++)
    {
        const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
        const struct NotablePredecessor *edge;
        u32 i;
        for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
            if (learnset[i].move == move && learnset[i].level < *level)
            {
                *level = learnset[i].level;
                found = TRUE;
            }
        edge = FindPredecessor(species);
        if (edge == NULL)
            break;
        species = edge->predecessor;
    }
    return found;
}

static bool32 IsPoolEntryEligible(const struct TrainerMon *member, const struct NotableMovePoolEntry *entry)
{
    u16 naturalLevel;
    bool32 natural = NaturalLearnLevel(member->species, entry->move, &naturalLevel);

    if (entry->fromLevel == 0)
        return natural && naturalLevel <= member->lvl;
    if (entry->fromLevel > member->lvl)
        return FALSE;
    return natural
        || HasMove(GetSpeciesTeachableLearnset(member->species), entry->move)
        || HasMove(GetSpeciesEggMoves(GetLineBase(member->species)), entry->move);
}

static void FillDefaultMoves(struct TrainerMon *member)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(member->species);
    u32 i, j, count = 0;

    for (i = 0; i < MAX_MON_MOVES; i++)
        member->moves[i] = MOVE_NONE;
    for (i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        bool32 alreadyKnown = FALSE;
        if (learnset[i].level > member->lvl)
            break;
        if (learnset[i].level == 0)
            continue;
        for (j = 0; j < count; j++)
            if (member->moves[j] == learnset[i].move)
                alreadyKnown = TRUE;
        if (alreadyKnown)
            continue;
        if (count == MAX_MON_MOVES)
        {
            for (j = 1; j < MAX_MON_MOVES; j++)
                member->moves[j - 1] = member->moves[j];
            count--;
        }
        member->moves[count++] = learnset[i].move;
    }
}

static void ResolveMemberPool(struct TrainerMon *member, const struct NotableMovePoolEntry *pool,
                              u32 poolCount, bool8 *taken)
{
    bool8 protected[MAX_MON_MOVES] = {FALSE};
    u16 added[MAX_MON_MOVES];
    u32 i, j, selected = 0, addCount = 0;

    for (i = 0; i < poolCount && selected < MAX_MON_MOVES; i++)
    {
        bool32 duplicate = FALSE;
        if (taken[i] || !IsPoolEntryEligible(member, &pool[i]))
            continue;
        for (j = 0; j < MAX_MON_MOVES; j++)
            if (member->moves[j] == pool[i].move && protected[j])
                duplicate = TRUE;
        for (j = 0; j < addCount; j++)
            if (added[j] == pool[i].move)
                duplicate = TRUE;
        if (duplicate)
            continue;
        for (j = 0; j < MAX_MON_MOVES; j++)
            if (member->moves[j] == pool[i].move)
            {
                protected[j] = TRUE;
                break;
            }
        if (j == MAX_MON_MOVES)
            added[addCount++] = pool[i].move;
        taken[i] = TRUE;
        selected++;
    }
    for (i = 0; i < addCount; i++)
    {
        for (j = 0; j < MAX_MON_MOVES; j++)
            if (member->moves[j] == MOVE_NONE)
                break;
        if (j == MAX_MON_MOVES)
            for (j = 0; j < MAX_MON_MOVES; j++)
                if (!protected[j])
                    break;
        if (j == MAX_MON_MOVES)
            break;
        member->moves[j] = added[i];
        protected[j] = TRUE;
    }
}

bool32 ResolveNotableTrainerMoves(struct TrainerMon *members, u8 count, u8 aceMask,
                                  const struct NotableMovePoolEntry *pool, u32 poolCount,
                                  bool32 skipPool)
{
    bool8 taken[MAX_NOTABLE_MOVE_POOL] = {FALSE};
    u32 i, pass;

    if (members == NULL || count == 0 || count > PARTY_SIZE || (aceMask >> PARTY_SIZE) != 0
     || poolCount > MAX_NOTABLE_MOVE_POOL || (poolCount != 0 && pool == NULL))
        return FALSE;
    for (i = 0; i < count; i++)
        if (members[i].species == SPECIES_NONE || members[i].species >= NUM_SPECIES
         || members[i].lvl == 0 || members[i].lvl > MAX_LEVEL)
            return FALSE;
    for (i = 0; i < poolCount; i++)
        if (pool[i].move == MOVE_NONE || pool[i].move >= MOVES_COUNT_ALL
         || pool[i].fromLevel > MAX_LEVEL)
            return FALSE;

    for (i = 0; i < count; i++)
        FillDefaultMoves(&members[i]);
    if (skipPool)
        return TRUE;
    for (pass = 0; pass < 2; pass++)
        for (i = 0; i < count; i++)
            if (((aceMask >> i) & 1) == (pass == 0))
                ResolveMemberPool(&members[i], pool, poolCount, taken);
    return TRUE;
}
