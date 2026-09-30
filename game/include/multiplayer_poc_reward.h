#ifndef GUARD_MULTIPLAYER_POC_REWARD_H
#define GUARD_MULTIPLAYER_POC_REWARD_H

#if WAYFARER_MULTIPLAYER_POC

#define MULTIPLAYER_POC_REWARD_DIAG_MAGIC 0x52574431 // "RWD1"
#define MULTIPLAYER_POC_REWARD_DIAG_VERSION 1
#define MULTIPLAYER_POC_REWARD_ENCOUNTER_ID 1

enum MultiplayerPocRewardState
{
    MULTIPLAYER_POC_REWARD_NONE,
    MULTIPLAYER_POC_REWARD_PENDING,
    MULTIPLAYER_POC_REWARD_CLAIMED,
    MULTIPLAYER_POC_REWARD_INVALID,
};

enum MultiplayerPocRewardSavePhase
{
    MULTIPLAYER_POC_REWARD_SAVE_IDLE,
    MULTIPLAYER_POC_REWARD_SAVE_PREPARING,
    MULTIPLAYER_POC_REWARD_SAVE_WRITING,
    MULTIPLAYER_POC_REWARD_SAVE_SUCCEEDED,
    MULTIPLAYER_POC_REWARD_SAVE_FAILED,
};

enum MultiplayerPocRewardResult
{
    MULTIPLAYER_POC_REWARD_RESULT_NONE,
    MULTIPLAYER_POC_REWARD_RESULT_CLAIMED,
    MULTIPLAYER_POC_REWARD_RESULT_PENDING,
    MULTIPLAYER_POC_REWARD_RESULT_ALREADY_CLAIMED,
    MULTIPLAYER_POC_REWARD_RESULT_NO_PENDING,
    MULTIPLAYER_POC_REWARD_RESULT_WRONG_ENCOUNTER,
    MULTIPLAYER_POC_REWARD_RESULT_NOT_IN_FIELD,
    MULTIPLAYER_POC_REWARD_RESULT_NO_BASELINE,
    MULTIPLAYER_POC_REWARD_RESULT_INVALID_LEDGER,
    MULTIPLAYER_POC_REWARD_RESULT_SAVE_FAILED,
    MULTIPLAYER_POC_REWARD_RESULT_BAG_ADD_FAILED,
    MULTIPLAYER_POC_REWARD_RESULT_WRONG_BAG,
};

// Fixed-width, volatile observations for the emulator harness. Only the two
// ledger bytes in SaveBlock1 are persisted; this diagnostic is never saved.
struct MultiplayerPocRewardDiag
{
    u32 magic;
    u32 version;
    u32 size;
    u32 encounterId;
    u32 ledgerState;
    u32 bagPotionCount;
    u32 savePhase;
    u32 saveResult;
    u32 operationResult;
    u32 grantAttempts;
    u32 grantsCommitted;
    u32 duplicateSuppressions;
    u32 retryCount;
    u32 saveAttempts;
};

extern volatile struct MultiplayerPocRewardDiag gMultiplayerPocRewardDiag;

void MultiplayerPocReward_UpdateDiag(void);
enum MultiplayerPocRewardState MultiplayerPocReward_GetState(void);
enum MultiplayerPocRewardResult MultiplayerPocReward_OnAgreedWin(u16 encounterId);
enum MultiplayerPocRewardResult MultiplayerPocReward_TryClaimPending(void);

#endif // WAYFARER_MULTIPLAYER_POC

#endif // GUARD_MULTIPLAYER_POC_REWARD_H
