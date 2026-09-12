#ifndef GUARD_TRAINER_ONLY_ENCOUNTER_H
#define GUARD_TRAINER_ONLY_ENCOUNTER_H

#include "capture_context.h"

enum TrainerOnlyAction { TRAINER_ONLY_ROCK, TRAINER_ONLY_BALL, TRAINER_ONLY_NEAR, TRAINER_ONLY_RUN, TRAINER_ONLY_FOOD, TRAINER_ONLY_QUICK_BALL };
#define TRAINER_ONLY_ITEM_BALL TRAINER_ONLY_BALL
#define TRAINER_ONLY_ITEM_FOOD TRAINER_ONLY_FOOD
struct TrainerOnlyState
{
    bool8 active;
    u8 cap, level, approach, initialCatchFactor, catchFactor, escapeFactor;
    u8 rocks, anger, foodTurns, escapeAttempts, completedTurns;
    bool8 warned;
};
enum TrainerOnlyResolution { TRAINER_ONLY_ONGOING, TRAINER_ONLY_FLED, TRAINER_ONLY_RETALIATION };
void TrainerOnlyApplyFood(struct TrainerOnlyState *state);
u32 TrainerOnlyResolveSurvivingTurn(struct TrainerOnlyState *state, u32 fleeRoll);
bool32 IsTrainerOnlyEncounter(void);
void TrainerOnlyPrepareEncounter(void);
void TrainerOnlyResetEncounter(void);
bool8 TrainerOnlyCanEnterWildEncounter(void);
void TrainerOnlyGetCaptureContext(struct CaptureContext *context);
void TrainerOnlyStartController(void);
void TrainerOnlyFinishBattle(void);
void TrainerOnlyCommitItem(enum TrainerOnlyAction action);
void SetControllerToTrainerOnly(enum BattlerId battler);
void TrainerOnlyBufferExecCompleted(enum BattlerId battler);
u32 TrainerOnlyRockDamage(u32 cap, u32 level, u32 maxHp, u32 hp);
u32 TrainerOnlyPassiveAnger(u32 cap, u32 level);
u32 TrainerOnlyFleeChance(const struct TrainerOnlyState *state);
void TrainerOnlyApplyApproach(struct TrainerOnlyState *state);
u32 TrainerOnlyEscapeChance(u32 cap, u32 level, u32 failures);
bool32 TrainerOnlyRunSucceeds(u32 chance, bool32 lessEscapes, u32 roll);
u32 TrainerOnlyWarningThreshold(u32 passiveAnger);
#if E2E_TESTING
void TrainerOnlyReadState(struct TrainerOnlyState *out);
bool32 TrainerOnlyIsActionMenu(void);
#endif
#endif
