#ifndef GUARD_E2E_TEST_H
#define GUARD_E2E_TEST_H

#ifdef E2E_TESTING

#include "constants/field_move.h"

#define E2E_TEST_MAX_VARS 8
#define E2E_TEST_MAX_FLAGS 8
#define E2E_TEST_MAX_PARTY PARTY_SIZE
#define E2E_TEST_MAX_BAG_ITEMS 8
#define E2E_TEST_MAX_PC_SLOTS 8
#define E2E_TEST_MAX_PARTY_MENU_ACTIONS 8
#define E2E_TEST_FIELD_MESSAGE_TEXT_LENGTH 32
#define E2E_TEST_KEEP_MAP 0xFFFF
#define E2E_TEST_KEEP_COORDINATE INT16_MIN
#define E2E_TEST_KEEP_TEXT_SPEED 0xFF

enum E2ETestStatus
{
    E2E_TEST_STATUS_IDLE,
    E2E_TEST_STATUS_PENDING,
    E2E_TEST_STATUS_RUNNING,
    E2E_TEST_STATUS_SUCCESS,
    E2E_TEST_STATUS_ERROR,
};

enum E2ETestCommand
{
    E2E_TEST_COMMAND_NONE,
    E2E_TEST_COMMAND_ARRANGE,
    E2E_TEST_COMMAND_START_WILD_BATTLE,
};

enum E2ETestCheckpoint
{
    E2E_TEST_CHECKPOINT_NONE,
    E2E_TEST_CHECKPOINT_BEDROOM_BEFORE_CLOCK,
    E2E_TEST_CHECKPOINT_NEW_BARK_AFTER_INTRO,
    E2E_TEST_CHECKPOINT_ELM_LAB_BEFORE_INTRO,
};

enum E2ETestArrangePhase
{
    E2E_TEST_ARRANGE_PHASE_NONE,
    E2E_TEST_ARRANGE_PHASE_VALIDATE,
    E2E_TEST_ARRANGE_PHASE_NEW_GAME,
    E2E_TEST_ARRANGE_PHASE_STATE,
    E2E_TEST_ARRANGE_PHASE_WARP,
    E2E_TEST_ARRANGE_PHASE_FIELD_READY,
};

enum E2ETestError
{
    E2E_TEST_ERROR_NONE,
    E2E_TEST_ERROR_COMMAND,
    E2E_TEST_ERROR_CHECKPOINT,
    E2E_TEST_ERROR_MAP,
    E2E_TEST_ERROR_COORDINATES,
    E2E_TEST_ERROR_FACING,
    E2E_TEST_ERROR_TEXT_SPEED,
    E2E_TEST_ERROR_VAR_COUNT,
    E2E_TEST_ERROR_VAR,
    E2E_TEST_ERROR_FLAG_COUNT,
    E2E_TEST_ERROR_FLAG,
    E2E_TEST_ERROR_PARTY_COUNT,
    E2E_TEST_ERROR_PARTY,
    E2E_TEST_ERROR_MOVE,
    E2E_TEST_ERROR_BAG_ITEM_COUNT,
    E2E_TEST_ERROR_BAG_ITEM,
    E2E_TEST_ERROR_SPECIES,
    E2E_TEST_ERROR_LEVEL,
    E2E_TEST_ERROR_ITEM_QUANTITY,
    E2E_TEST_ERROR_PC_SLOT_COUNT,
    E2E_TEST_ERROR_PC_BOX,
    E2E_TEST_ERROR_PC_SLOT,
    E2E_TEST_ERROR_BUSY,
};

enum E2ETestGamePhase
{
    E2E_TEST_GAME_PHASE_BOOT,
    E2E_TEST_GAME_PHASE_OVERWORLD,
    E2E_TEST_GAME_PHASE_DIALOGUE,
    E2E_TEST_GAME_PHASE_BATTLE,
};

enum E2ETestDialogueMessage
{
    E2E_TEST_DIALOGUE_NONE,
    E2E_TEST_DIALOGUE_UNKNOWN,
    E2E_TEST_DIALOGUE_FIELD_MOVE_USED,
    E2E_TEST_DIALOGUE_FIELD_MOVE_NEEDS_HM,
    E2E_TEST_DIALOGUE_FIELD_MOVE_NO_ELIGIBLE_MON,
    E2E_TEST_DIALOGUE_WANT_TO_USE_SURF,
    E2E_TEST_DIALOGUE_PLAYER_USED_SURF,
};

enum E2ETestFieldMoveResult
{
    E2E_TEST_FIELD_MOVE_SELECTED = FIELD_MOVE_USER_NO_ELIGIBLE_MON + 1,
};

enum E2ETestUiMode
{
    E2E_TEST_UI_OVERWORLD,
    E2E_TEST_UI_PAUSE_MENU,
    E2E_TEST_UI_DIALOGUE,
    E2E_TEST_UI_PARTY_MENU,
    E2E_TEST_UI_SUMMARY,
    E2E_TEST_UI_BATTLE,
    E2E_TEST_UI_CATCH_SWAP,
    E2E_TEST_UI_STORAGE,
};

enum E2ETestCatchSwapState
{
    E2E_TEST_CATCH_SWAP_NONE,
    E2E_TEST_CATCH_SWAP_PROMPT,
    E2E_TEST_CATCH_SWAP_CHOOSE_PARTY,
    E2E_TEST_CATCH_SWAP_RESOLVED,
};

enum E2ETestStorageUiState
{
    E2E_TEST_STORAGE_UI_NONE,
    E2E_TEST_STORAGE_UI_INITIALIZING,
    E2E_TEST_STORAGE_UI_PC_MENU,
    E2E_TEST_STORAGE_UI_READY,
    E2E_TEST_STORAGE_UI_MON_MENU,
    E2E_TEST_STORAGE_UI_DEPOSIT_BOX,
    E2E_TEST_STORAGE_UI_WITHDRAWING,
    E2E_TEST_STORAGE_UI_MOVING,
    E2E_TEST_STORAGE_UI_RELEASE_CONFIRM,
    E2E_TEST_STORAGE_UI_RELEASE_CHECK,
    E2E_TEST_STORAGE_UI_RELEASED,
    E2E_TEST_STORAGE_UI_RELEASE_BLOCKED,
    E2E_TEST_STORAGE_UI_EXIT_CONFIRM,
    E2E_TEST_STORAGE_UI_BUSY,
};

enum E2ETestStorageMode
{
    E2E_TEST_STORAGE_MODE_NONE,
    E2E_TEST_STORAGE_MODE_MOVE,
    E2E_TEST_STORAGE_MODE_DEPOSIT,
    E2E_TEST_STORAGE_MODE_WITHDRAW,
};

enum E2ETestBattleUiState
{
    E2E_TEST_BATTLE_UI_NONE,
    E2E_TEST_BATTLE_UI_STARTING,
    E2E_TEST_BATTLE_UI_ACTION_MENU,
    E2E_TEST_BATTLE_UI_BAG,
    E2E_TEST_BATTLE_UI_BAG_CONTEXT,
    E2E_TEST_BATTLE_UI_CAUGHT_DEX,
    E2E_TEST_BATTLE_UI_NICKNAME,
    E2E_TEST_BATTLE_UI_CATCH_SWAP_PROMPT,
    E2E_TEST_BATTLE_UI_CATCH_SWAP_PARTY,
    E2E_TEST_BATTLE_UI_OTHER,
    E2E_TEST_BATTLE_UI_TEXT,
};

struct E2ETestVarPatch
{
    u16 id;
    u16 value;
};

struct E2ETestFlagPatch
{
    u16 id;
    u8 value;
    u8 reserved;
};

struct E2ETestMonFixture
{
    u16 species;
    u16 moves[MAX_MON_MOVES];
    u8 level;
    u8 isEgg;
    u32 reserved;
};

struct E2ETestPartyMonFixture
{
    struct E2ETestMonFixture mon;
    u8 fainted;
    u8 reserved[3];
};

struct E2ETestBagItem
{
    u16 item;
    u16 quantity;
};

struct E2ETestPcSlot
{
    struct E2ETestMonFixture mon;
    u8 boxId;
    u8 boxPosition;
    u8 reserved[2];
};

struct E2ETestRequest
{
    u32 requestId;
    u16 mapGroup;
    u16 mapNum;
    s16 x;
    s16 y;
    u32 rngSeed;
    struct E2ETestVarPatch vars[E2E_TEST_MAX_VARS];
    struct E2ETestFlagPatch flags[E2E_TEST_MAX_FLAGS];
    u8 checkpoint;
    u8 facing;
    u8 varCount;
    u8 flagCount;
    u8 textSpeed;
    u8 useRngSeed;
    u8 command;
    u8 status;
    struct E2ETestPartyMonFixture party[E2E_TEST_MAX_PARTY];
    struct E2ETestBagItem bagItems[E2E_TEST_MAX_BAG_ITEMS];
    struct E2ETestPcSlot pcSlots[E2E_TEST_MAX_PC_SLOTS];
    struct E2ETestMonFixture wildMon;
    u8 partyCount;
    u8 bagItemCount;
    u8 pcSlotCount;
    u8 currentBox;
    u8 hmsOverwrite;
    u8 reserved[3];
};

struct E2ETestObservedPcSlot
{
    u16 species;
    u16 moves[MAX_MON_MOVES];
    u8 level;
    u8 isEgg;
    u8 boxId;
    u8 boxPosition;
    u16 reserved;
};

struct E2ETestResult
{
    u32 requestId;
    u16 mapGroup;
    u16 mapNum;
    s16 x;
    s16 y;
    u16 error;
    u8 status;
    u8 phase;
};

struct E2ETestState
{
    u32 frame;
    u16 mapGroup;
    u16 mapNum;
    s16 x;
    s16 y;
    u16 avatarFlags;
    u16 fieldMoveMove;
    u16 fieldMoveUserSpecies;
    u16 partySpecies[E2E_TEST_MAX_PARTY];
    u16 partyMoves[E2E_TEST_MAX_PARTY][MAX_MON_MOVES];
    u16 bagItemCounts[E2E_TEST_MAX_BAG_ITEMS];
    u8 phase;
    u8 ready;
    u8 controlsLocked;
    u8 scriptActive;
    u8 dialogueOpen;
    u8 facing;
    u8 avatarSurfing;
    u8 surfBlobCount;
    u8 surfEffectActive;
    u8 fieldMoveUser;
    u8 fieldMoveResult;
    u8 partyCount;
    u8 hmsOverwrite;
    u8 uiMode;
    u8 partyMenuActionCount;
    u8 dialogueMessage;
    u8 partyEggMask;
    u8 partyFaintedMask;
    u8 partyMenuActions[E2E_TEST_MAX_PARTY_MENU_ACTIONS];
    u32 dialogueSequence;
    u8 dialogueText[E2E_TEST_FIELD_MESSAGE_TEXT_LENGTH];
    u8 fieldMoveUnlocked;
    u16 bagItemIds[E2E_TEST_MAX_BAG_ITEMS];
    struct E2ETestObservedPcSlot pcSlots[E2E_TEST_MAX_PC_SLOTS];
    u16 battleEnemySpecies;
    u16 battleEnemyMoves[MAX_MON_MOVES];
    u16 caughtSpecies;
    u16 lastUsedItem;
    u8 battleEnemyLevel;
    u8 battleActive;
    u8 catchSwapState;
    u8 catchSwapCursor;
    u8 catchSwapSelectedParty;
    u8 catchSwapBox;
    u8 catchSwapSlot;
    u8 storageUiState;
    u8 storageOpen;
    u8 storageReady;
    u8 storageCursorArea;
    u8 storageCursorPosition;
    u8 storageMovingMon;
    u8 storageCurrentBox;
    u8 pcSlotCount;
    u8 battleUiState;
    u8 battleCursor;
    u8 battleBagPocket;
    u8 storageMode;
    u16 battleBagItem;
    u8 reserved2[4];
};

struct E2ETestAbi
{
    u16 version;
    u16 requestSize;
    u16 resultSize;
    u16 stateSize;
    u16 requestStatusOffset;
    u16 resultStatusOffset;
    u16 flagsOffset;
    u16 varsOffset;
};

extern volatile struct E2ETestRequest gE2ETestRequest;
extern volatile struct E2ETestResult gE2ETestResult;
extern volatile struct E2ETestState gE2ETestState;
extern const struct E2ETestAbi gE2ETestAbi;

void E2ETest_Update(void);
void E2ETest_RecordFieldMove(enum Move move, u8 partyIndex, u8 result);
void E2ETest_RecordFieldMessage(const u8 *str);
void E2ETest_RecordExpandedFieldMessage(const u8 *str);
void E2ETest_RecordCapture(u16 species);
void E2ETest_RecordCatchSwap(u8 state, u8 cursor, u8 selectedParty, u8 boxId, u8 boxPosition);
void E2ETest_RecordNicknamePrompt(bool32 active, u8 cursor);
bool32 E2ETest_IsPartyMenuOpen(void);
void E2ETest_GetPartyMenuActions(u8 *actions, u8 *count);
bool32 E2ETest_IsSummaryScreenOpen(void);
void E2ETest_SetStorageCurrentBox(u8 boxId);
void E2ETest_GetStorageUiState(u8 *uiState, u8 *mode, u8 *cursorArea, u8 *cursorPosition, bool8 *movingMon);
bool32 E2ETest_GetBattleActionMenuState(u8 *cursor);
bool32 E2ETest_IsBattleTextReady(void);
bool32 E2ETest_GetBattleBagState(u8 *state, u8 *pocket, u16 *item);
bool32 E2ETest_IsCaughtDexReady(void);
bool32 E2ETest_GetCatchSwapPartyState(u8 *cursor);

#endif // E2E_TESTING

#endif // GUARD_E2E_TEST_H
