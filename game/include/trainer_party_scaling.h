#ifndef GUARD_TRAINER_PARTY_SCALING_H
#define GUARD_TRAINER_PARTY_SCALING_H

#include "config/trainer_party_scaling.h"

#define TRAINER_SCALING_EXCLUDED 0
#define TRAINER_SCALING_ORDINARY 1
#define TRAINER_SCALING_GYM_MEMBER 2

struct Pokemon;
struct TrainerMon;

u8 GetTrainerScalingLevel(u32 rating, u32 authoredLevel, u32 policy);
u32 GetTrainerScalingPolicy(u32 trainerId);
u16 ResolveTrainerScalingSpecies(u16 species, u8 level);
bool32 IsTrainerScalingBattleContext(u32 battleTypeFlags);
void ResetTrainerScalingSnapshot(void);
u8 GetTrainerScalingSnapshot(void);
bool32 HasTrainerScalingMoveException(u32 owner, u32 variant, u32 slot);
bool32 CanRetainTrainerScalingMoves(const struct TrainerMon *entry, u16 species, u8 level);
u32 GetTrainerScalingAbility(u16 species, u32 authoredAbility, u32 personalityHash);
u8 CreateNPCTrainerPartyForOpponent(struct Pokemon *party, u16 trainerId, bool32 firstTrainer, u32 battleTypeFlags);

#endif
