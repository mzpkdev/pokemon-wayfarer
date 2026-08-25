#ifndef GUARD_E2E_TEST_H
#define GUARD_E2E_TEST_H

#ifdef E2E_TESTING

#define E2E_TEST_MAX_VARS 8
#define E2E_TEST_MAX_FLAGS 8
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
};

enum E2ETestGamePhase
{
    E2E_TEST_GAME_PHASE_BOOT,
    E2E_TEST_GAME_PHASE_OVERWORLD,
    E2E_TEST_GAME_PHASE_DIALOGUE,
    E2E_TEST_GAME_PHASE_BATTLE,
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
    u8 phase;
    u8 ready;
    u8 controlsLocked;
    u8 scriptActive;
    u8 dialogueOpen;
    u8 facing;
    u8 reserved[2];
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

#endif // E2E_TESTING

#endif // GUARD_E2E_TEST_H
