#include "global.h"
#include "notable_ai.h"
#include "notable_trainers.h"
#include "constants/battle_ai.h"

u64 ResolveNotableTrainerAi(const struct NotableTrainerSnapshot *snapshot, bool32 speciesRandomized, bool32 doubleBattle)
{
    static const u64 styleFlags[] =
    {
        [NOTABLE_STYLE_FIELD_MARSHAL] = AI_FLAG_POWERFUL_STATUS,
        [NOTABLE_STYLE_GAMBLER] = AI_FLAG_RISKY,
        [NOTABLE_STYLE_HEXER] = AI_FLAG_PREFER_STATUS_MOVES | AI_FLAG_HP_AWARE,
        [NOTABLE_STYLE_BRAWLER] = AI_FLAG_TRY_TO_2HKO | AI_FLAG_PREFER_HIGHEST_DAMAGE_MOVE,
        [NOTABLE_STYLE_TACTICIAN] = AI_FLAG_HP_AWARE,
        [NOTABLE_STYLE_SWEEPER] = AI_FLAG_FORCE_SETUP_FIRST_TURN,
        [NOTABLE_STYLE_TURTLE] = AI_FLAG_CONSERVATIVE | AI_FLAG_HP_AWARE,
        [NOTABLE_STYLE_BOMBER] = AI_FLAG_RISKY | AI_FLAG_WILL_SUICIDE,
    };
    u64 flags;

    if (snapshot == NULL || snapshot->trainer == NULL
     || snapshot->trainer->playStyle >= ARRAY_COUNT(styleFlags)
     || snapshot->aceCount == 0 || snapshot->aceCount > 3)
        return 0;

    flags = AI_FLAG_BASIC_TRAINER | styleFlags[snapshot->trainer->playStyle];
    if (snapshot->trainerTR >= 30)
        flags |= AI_FLAG_SMART_MON_CHOICES | AI_FLAG_ASSUME_STAB;
    if (snapshot->trainerTR >= 70)
        flags |= AI_FLAG_SMART_SWITCHING | AI_FLAG_ASSUME_STATUS_MOVES | AI_FLAG_WEIGH_ABILITY_PREDICTION;
    if (snapshot->trainerTR >= 110)
        flags |= AI_FLAG_PREDICT_SWITCH | AI_FLAG_PREDICT_INCOMING_MON | AI_FLAG_PREDICT_MOVE;
    if (!speciesRandomized)
        flags |= snapshot->aceCount == 1 ? AI_FLAG_ACE_POKEMON : AI_FLAG_DOUBLE_ACE_POKEMON;
    if (snapshot->trainer->bossOmniscient)
        flags |= AI_FLAG_OMNISCIENT;
    if (doubleBattle)
        flags |= AI_FLAG_DOUBLE_BATTLE;
    // Preserve the normal setup's implied flags when replacing its result.
    if (flags & AI_FLAG_SMART_SWITCHING)
        flags |= AI_FLAG_SMART_MON_CHOICES;
    if (flags & AI_FLAG_PREDICT_INCOMING_MON)
        flags |= AI_FLAG_PREDICT_SWITCH;
    return flags;
}
