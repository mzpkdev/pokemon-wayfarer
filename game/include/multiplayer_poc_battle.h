#ifndef GUARD_MULTIPLAYER_POC_BATTLE_H
#define GUARD_MULTIPLAYER_POC_BATTLE_H

#if WAYFARER_MULTIPLAYER_POC

#define MULTIPLAYER_POC_BATTLE_DIAG_MAGIC 0x42415431 // "BAT1"
#define MULTIPLAYER_POC_BATTLE_ENCOUNTER_ID 1

enum MultiplayerPocBattleState
{
    POC_BATTLE_IDLE,
    POC_BATTLE_OFFER,
    POC_BATTLE_READY,
    POC_BATTLE_START,
    POC_BATTLE_ACK,
    POC_BATTLE_GO,
    POC_BATTLE_SETUP,
    POC_BATTLE_IN_PROGRESS,
    POC_BATTLE_RESULT,
    POC_BATTLE_RETURNING,
    POC_BATTLE_COMPLETE,
    POC_BATTLE_ABORTED,
};

enum MultiplayerPocBattleError
{
    POC_BATTLE_ERROR_NONE,
    POC_BATTLE_ERROR_CONSENT_TIMEOUT,
    POC_BATTLE_ERROR_LINK_LOST,
    POC_BATTLE_ERROR_RESULT_TIMEOUT,
    POC_BATTLE_ERROR_RESULT_MISMATCH,
};

// All fields are 32-bit for stable emulator memory inspection. EWRAM only.
struct MultiplayerPocBattleDiag
{
    u32 magic;
    u32 version;
    u32 size;
    u32 state;
    u32 error;
    u32 encounterId;
    u32 localPlayerId;
    u32 localBattlerId;
    u32 peerWord;
    u32 localMoveChoices;
    u32 localLastChosenMove;
    u32 localLastChosenTurn;
    u32 localExecutedMoves;
    u32 localLastExecutedMove;
    u32 localLastExecutedTurn;
    u32 beforePartyHash;
    u32 afterPartyHash;
    u32 beforePartyCount;
    u32 afterPartyCount;
    u32 localOutcome;
    u32 peerOutcome;
    u32 resultAgreed;
    u32 rewardResult;
    u32 frame;
};

extern volatile struct MultiplayerPocBattleDiag gMultiplayerPocBattleDiag;

bool8 MultiplayerPocBattle_TryRequest(void);
u16 MultiplayerPocBattle_GetTxWord(void);
void MultiplayerPocBattle_OnPeerWord(u16 word);
void MultiplayerPocBattle_Update(void);
bool8 MultiplayerPocBattle_OwnsTransport(void);
bool8 MultiplayerPocBattle_IsResultExchange(void);
bool8 MultiplayerPocBattle_IsExperimentActive(void);
void MultiplayerPocBattle_OnPresenceLost(void);
void MultiplayerPocBattle_AbortFromLinkError(void);
void MultiplayerPocBattle_RecordMoveChoice(u8 battler, u16 move, u16 turn);
void MultiplayerPocBattle_RecordMoveAnimation(u8 battler, u16 move, u16 turn);

#endif // WAYFARER_MULTIPLAYER_POC

#endif // GUARD_MULTIPLAYER_POC_BATTLE_H
