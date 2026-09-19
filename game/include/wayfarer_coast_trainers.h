#ifndef GUARD_WAYFARER_COAST_TRAINERS_H
#define GUARD_WAYFARER_COAST_TRAINERS_H

#include "global.h"
#include "constants/flags.h"
#include "constants/vars.h"

#if IS_WAYFARER

#define WAYFARER_COAST_TRAINER_DEFEAT_SLOT_FIRST (FLAG_WAYFARER_COAST_TRAINER_DEFEAT_FIRST & WAYFARER_PERSISTENCE_VALUE_MASK)
#define WAYFARER_COAST_TRAINER_DEFEAT_COUNT       45
#define WAYFARER_COAST_REMATCH_PENDING_SLOT_FIRST (FLAG_WAYFARER_COAST_REMATCH_PENDING_FIRST & WAYFARER_PERSISTENCE_VALUE_MASK)
#define WAYFARER_COAST_REMATCH_FAMILY_COUNT       9
#define WAYFARER_COAST_REMATCH_STAGE_VAR_LOW      (VAR_WAYFARER_COAST_REMATCH_STAGES_LOW & WAYFARER_PERSISTENCE_VALUE_MASK)
#define WAYFARER_COAST_REMATCH_STAGE_VAR_HIGH     (VAR_WAYFARER_COAST_REMATCH_STAGES_HIGH & WAYFARER_PERSISTENCE_VALUE_MASK)

bool32 WayfarerCoastTrainerDefeatGet(u16 slot);
void WayfarerCoastTrainerDefeatSet(u16 slot);
void WayfarerCoastTrainerDefeatClear(u16 slot);

bool8 WayfarerCoastRematchHasFamily(u16 trainerId);
bool8 WayfarerCoastRematchIsReady(u16 trainerId);
void WayfarerCoastRematchSetReady(u16 trainerId);
void WayfarerCoastRematchClearReady(u16 trainerId);
void WayfarerCoastRematchClearAllReady(void);
bool8 WayfarerCoastRematchCompleteIfReady(u16 trainerId);
u16 WayfarerCoastRematchGetOpponent(u16 trainerId);
void WayfarerCoastRematchAdvance(u16 trainerId);
s32 WayfarerCoastRematchFamilyIndex(u16 trainerId);

#endif  // IS_WAYFARER

#endif  // GUARD_WAYFARER_COAST_TRAINERS_H
