#include "global.h"
#include "notable_trainers.h"
#include "notable_moves.h"
#include "notable_ai.h"
#include "trainer_scaler.h"
#include "constants/notable_trainers.h"
#include "constants/opponents.h"
#include "constants/opponents_frlg.h"
#include "constants/opponents_hns.h"
#include "constants/wayfarer_celadon_hideout_trainers.h"
#include "constants/wayfarer_coast_trainers.h"
#include "constants/wayfarer_indigo_trainers.h"
#include "constants/wayfarer_kanto_trainers.h"
#include "constants/wayfarer_local_trainers.h"
#include "constants/wayfarer_viridian_trainers.h"
#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/abilities.h"

#if IS_WAYFARER

#include "data/notable_trainers/catalog.h"

static const struct TrainerScalerAnchor sSteady[] = {{0, 0}, {40, 25}, {80, 50}, {120, 75}, {160, 100}};
static const struct TrainerScalerAnchor sProdigy[] = {{0, 0}, {40, 50}, {80, 80}, {120, 95}, {160, 100}};
static const struct TrainerScalerAnchor sSleeper[] = {{0, 0}, {40, 10}, {80, 25}, {120, 55}, {160, 100}};
static const struct TrainerScalerAnchor sVeteran[] = {{0, 0}, {40, 60}, {80, 100}, {120, 100}, {160, 100}};
static const struct TrainerScalerAnchor sRival[] = {{0, 0}, {20, 15}, {40, 29}, {80, 53}, {120, 76}, {160, 100}};
static const struct TrainerScalerAnchor sLegend[] = {{0, 0}, {160, 0}};
static const struct TrainerScalerAnchor sStar[] = {{0, 0}, {40, 10}, {80, 50}, {120, 90}, {160, 100}};
static const struct TrainerScalerAnchor sComeback[] = {{0, 0}, {40, 45}, {80, 50}, {120, 55}, {160, 100}};
static const struct TrainerScalerAnchor sBurst[] = {{0, 0}, {40, 25}, {80, 50}, {120, 75}, {160, 100}};

static const struct TrainerScalerAnchor sTeamLevel[] = {{0, 5}, {20, 14}, {40, 28}, {80, 50}, {120, 75}, {160, 100}};
static const struct TrainerScalerAnchor sTeamSize[] = {{0, 1}, {11, 2}, {29, 3}, {44, 4}, {57, 5}, {71, 6}};

static u16 GetGrowthPercent(u8 archetype, u32 worldProgress)
{
    switch (archetype)
    {
    case NOTABLE_ARCHETYPE_STEADY: return EvaluateTrainerScaler(sSteady, ARRAY_COUNT(sSteady), worldProgress, FALSE);
    case NOTABLE_ARCHETYPE_PRODIGY: return EvaluateTrainerScaler(sProdigy, ARRAY_COUNT(sProdigy), worldProgress, FALSE);
    case NOTABLE_ARCHETYPE_SLEEPER: return EvaluateTrainerScaler(sSleeper, ARRAY_COUNT(sSleeper), worldProgress, FALSE);
    case NOTABLE_ARCHETYPE_VETERAN: return EvaluateTrainerScaler(sVeteran, ARRAY_COUNT(sVeteran), worldProgress, FALSE);
    case NOTABLE_ARCHETYPE_RIVAL: return EvaluateTrainerScaler(sRival, ARRAY_COUNT(sRival), worldProgress, FALSE);
    case NOTABLE_ARCHETYPE_LEGEND: return EvaluateTrainerScaler(sLegend, ARRAY_COUNT(sLegend), worldProgress, FALSE);
    case NOTABLE_ARCHETYPE_STAR: return EvaluateTrainerScaler(sStar, ARRAY_COUNT(sStar), worldProgress, FALSE);
    case NOTABLE_ARCHETYPE_COMEBACK: return EvaluateTrainerScaler(sComeback, ARRAY_COUNT(sComeback), worldProgress, FALSE);
    case NOTABLE_ARCHETYPE_BURST: return EvaluateTrainerScaler(sBurst, ARRAY_COUNT(sBurst), worldProgress, TRUE);
    default: return 0;
    }
}

const struct NotableTrainer *GetNotableTrainerById(u32 characterId)
{
    if (characterId < 1 || characterId > NOTABLE_TRAINER_COUNT)
        return NULL;
    return &gNotableTrainers[characterId - 1];
}

const struct NotableTrainer *GetNotableTrainerForEncounter(u16 trainerId)
{
    return GetNotableTrainerById(GetNotableCharacterIdForEncounter(trainerId));
}

bool32 IsNotableTrainerCatalogValid(void)
{
    u32 i, j, aceCount;
    for (i = 0; i < NOTABLE_TRAINER_COUNT; i++)
    {
        const struct NotableTrainer *trainer = &gNotableTrainers[i];
        if (trainer->characterId != i + 1 || trainer->peakTR < trainer->startTR
         || trainer->archetype > NOTABLE_ARCHETYPE_BURST || trainer->homeRegion > NOTABLE_REGION_HOENN
         || trainer->playStyle > NOTABLE_STYLE_BOMBER || trainer->movePool == NULL || trainer->movePoolCount == 0
         || !(trainer->aceMask & 1) || trainer->levelOffsets[0] != 0)
            return FALSE;
        aceCount = 0;
        for (j = 0; j < PARTY_SIZE; j++)
        {
            if (trainer->aceMask & (1 << j))
                aceCount++;
            if (trainer->roster[j].species == SPECIES_NONE || trainer->levelOffsets[j] < -6 || trainer->levelOffsets[j] > 0)
                return FALSE;
        }
        if (aceCount == 0 || aceCount > 3 || (trainer->archetype == NOTABLE_ARCHETYPE_LEGEND && trainer->startTR != trainer->peakTR))
            return FALSE;
        for (j = 0; j < trainer->movePoolCount; j++)
            if (trainer->movePool[j].move == MOVE_NONE || trainer->movePool[j].fromLevel > 100)
                return FALSE;
    }
    for (i = 0; i < ARRAY_COUNT(sNotableEncounterAliases); i++)
    {
        const struct NotableEncounterAlias *alias = &sNotableEncounterAliases[i];
        if (alias->characterId < 1 || alias->characterId > NOTABLE_TRAINER_COUNT
         || GetNotableCharacterIdForEncounter(alias->trainerId) != alias->characterId)
            return FALSE;
        for (j = i + 1; j < ARRAY_COUNT(sNotableEncounterAliases); j++)
            if (alias->trainerId == sNotableEncounterAliases[j].trainerId)
                return FALSE;
    }
    return TRUE;
}

u32 GetNotableTrainerRating(const struct NotableTrainer *trainer, u32 worldProgress)
{
    u32 fraction;
    if (trainer == NULL || trainer->peakTR < trainer->startTR || trainer->archetype > NOTABLE_ARCHETYPE_BURST)
        return 0;
    fraction = GetGrowthPercent(trainer->archetype, worldProgress);
    return trainer->startTR + ((trainer->peakTR - trainer->startTR) * fraction + 50) / 100;
}

bool32 ResolveNotableTrainerSnapshot(const struct NotableTrainer *trainer, u32 worldProgress, bool32 skipMovePool, struct NotableTrainerSnapshot *snapshot)
{
    u32 slot;
    u8 level, count, output = 0;
    if (snapshot == NULL)
        return FALSE;
    *snapshot = (struct NotableTrainerSnapshot){0};
    if (trainer == NULL || trainer->characterId < 1 || trainer->characterId > NOTABLE_TRAINER_COUNT
     || trainer->peakTR < trainer->startTR || trainer->archetype > NOTABLE_ARCHETYPE_BURST
     || !(trainer->aceMask & 1) || trainer->movePool == NULL || trainer->movePoolCount == 0)
        return FALSE;

    snapshot->trainer = trainer;
    snapshot->worldProgress = worldProgress;
    snapshot->trainerTR = GetNotableTrainerRating(trainer, worldProgress);
    count = EvaluateTrainerScaler(sTeamSize, ARRAY_COUNT(sTeamSize), snapshot->trainerTR, TRUE);
    level = EvaluateTrainerScaler(sTeamLevel, ARRAY_COUNT(sTeamLevel), snapshot->trainerTR, FALSE);
    if (count == 0 || count > PARTY_SIZE || level == 0 || level > 100)
        return FALSE;
    snapshot->teamSize = count;
    for (slot = 0; slot < count; slot++)
    {
        s32 memberLevel = level + trainer->levelOffsets[slot];
        if (trainer->roster[slot].species == SPECIES_NONE || trainer->levelOffsets[slot] < -6 || trainer->levelOffsets[slot] > 0)
            return FALSE;
        if (memberLevel < 1)
            memberLevel = 1;
        if (memberLevel > 100)
            memberLevel = 100;
        snapshot->members[slot] = trainer->roster[slot];
        snapshot->members[slot].lvl = memberLevel;
        snapshot->members[slot].species = StepDownSpeciesToLevel(trainer->roster[slot].species, memberLevel);
        if (snapshot->members[slot].species == SPECIES_NONE)
            return FALSE;
    }
    // Output fillers in reverse roster order, then aces in reverse roster order.
    for (slot = count; slot > 0; slot--)
        if (!(trainer->aceMask & (1 << (slot - 1))))
            snapshot->battleOrder[output++] = slot - 1;
    for (slot = count; slot > 0; slot--)
        if (trainer->aceMask & (1 << (slot - 1)))
        {
            snapshot->battleOrder[output++] = slot - 1;
            snapshot->aceCount++;
        }
    if (!ResolveNotableTrainerMoves(snapshot->members, snapshot->teamSize, trainer->aceMask & ((1 << snapshot->teamSize) - 1), trainer->movePool, trainer->movePoolCount, skipMovePool))
        return FALSE;
    snapshot->aiFlags = ResolveNotableTrainerAi(snapshot, FALSE, trainer->isDoubleBattle);
    if (snapshot->aiFlags == 0)
        return FALSE;
    return TRUE;
}

#else

const struct NotableTrainer *GetNotableTrainerById(u32 characterId)
{
    return NULL;
}

const struct NotableTrainer *GetNotableTrainerForEncounter(u16 trainerId)
{
    return NULL;
}

u32 GetNotableTrainerRating(const struct NotableTrainer *trainer, u32 worldProgress)
{
    return 0;
}

bool32 ResolveNotableTrainerSnapshot(const struct NotableTrainer *trainer, u32 worldProgress, bool32 skipMovePool, struct NotableTrainerSnapshot *snapshot)
{
    return FALSE;
}

bool32 IsNotableTrainerCatalogValid(void)
{
    return FALSE;
}

#endif // IS_WAYFARER
