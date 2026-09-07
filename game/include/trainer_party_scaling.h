#ifndef GUARD_TRAINER_PARTY_SCALING_H
#define GUARD_TRAINER_PARTY_SCALING_H

#include "config/trainer_party_scaling.h"

#define TRAINER_SCALING_EXCLUDED 0
#define TRAINER_SCALING_ORDINARY 1
#define TRAINER_SCALING_GYM_MEMBER 2
#define TRAINER_SCALING_GYM_LEADER 3
#define TRAINER_SCALING_LEAGUE 4

enum GymLeaderMovePolicy
{
    GYM_LEADER_MOVE_AUTHORED,
    GYM_LEADER_MOVE_LEVEL_UP,
};

struct Pokemon;
struct TrainerMon;

// These records are generated from the Gym Leader roster source.  Source
// indices are retention indices; they never become party indices just because
// battleOrder rearranges the constructed party.
struct GymLeaderScalingSlot
{
    u8 battleOrder;
    bool8 isAce;
    s8 levelOffset;
    u8 movePolicy;
};

struct GymLeaderScalingRoster
{
    u16 trainerId;
    u16 ownerId;
    u8 difficulty;
    const struct TrainerMon *party;
    const struct TrainerMon *legacyParty;
    u8 legacyPartySize;
    struct GymLeaderScalingSlot slots[PARTY_SIZE];
    bool8 isDoubleBattle;
};

struct GymLeaderScalingPlan
{
    u8 count;
    u8 sourceIndices[PARTY_SIZE];
    u8 levels[PARTY_SIZE];
};

struct LeagueScalingRoster
{
    u16 trainerId;
    u16 ownerId;
    u8 region;
    u8 difficulty;
    u8 encounterIndex;
    s8 encounterOffset;
    u8 count;
    u8 aceSlot;
    s8 offsets[PARTY_SIZE];
    u16 species[PARTY_SIZE];
    u16 items[PARTY_SIZE];
    u16 moves[PARTY_SIZE][MAX_MON_MOVES];
};

u8 GetLeagueScalingBaseline(u32 rating);
u8 GetLeagueScalingLevel(u32 rating, s8 encounterOffset, s8 slotOffset);
const struct LeagueScalingRoster *GetLeagueScalingRoster(u16 trainerId, u16 ownerId, u8 difficulty);
bool32 IsLeagueScalingRosterValid(const struct LeagueScalingRoster *roster, const struct TrainerMon *party, u32 count);

u8 GetTrainerScalingLevel(u32 rating, u32 authoredLevel, u32 policy);
u32 GetTrainerScalingPolicy(u32 trainerId);
u16 ResolveTrainerScalingSpecies(u16 species, u8 level);
bool32 IsTrainerScalingBattleContext(u32 battleTypeFlags);
void ResetTrainerScalingSnapshot(void);
u8 GetTrainerScalingSnapshot(void);
bool32 HasTrainerScalingMoveException(u32 owner, u32 variant, u32 slot);
bool32 CanRetainTrainerScalingMoves(const struct TrainerMon *entry, u16 species, u8 level);
u32 GetTrainerScalingAbility(u16 species, u32 authoredAbility, u32 personalityHash);
const struct GymLeaderScalingRoster *GetGymLeaderScalingRoster(u16 trainerId, u16 ownerId, u8 difficulty);
u8 GetGymLeaderScalingPartySize(u32 rating);
u8 GetGymLeaderScalingLevel(u32 rating, s8 levelOffset);
bool32 BuildGymLeaderScalingPlan(const struct GymLeaderScalingRoster *roster, u32 rating, struct GymLeaderScalingPlan *plan);
u8 CreateNPCTrainerPartyForOpponent(struct Pokemon *party, u16 trainerId, bool32 firstTrainer, u32 battleTypeFlags);

#endif
