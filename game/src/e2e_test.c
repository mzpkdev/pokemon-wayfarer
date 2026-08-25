#ifdef E2E_TESTING

#include "global.h"
#include "e2e_test.h"
#include "event_data.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "load_save.h"
#include "main.h"
#include "new_game.h"
#include "overworld.h"
#include "random.h"
#include "script.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/global.h"
#include "constants/maps.h"
#include "constants/vars.h"
#include "constants/vars_hns.h"
#include "data/map_group_count.h"

volatile struct E2ETestRequest gE2ETestRequest;
volatile struct E2ETestResult gE2ETestResult;
volatile struct E2ETestState gE2ETestState;

const struct E2ETestAbi gE2ETestAbi =
{
    .version = 1,
    .requestSize = sizeof(struct E2ETestRequest),
    .resultSize = sizeof(struct E2ETestResult),
    .stateSize = sizeof(struct E2ETestState),
    .requestStatusOffset = offsetof(struct E2ETestRequest, status),
    .resultStatusOffset = offsetof(struct E2ETestResult, status),
    .flagsOffset = offsetof(struct SaveBlock1, flags),
    .varsOffset = offsetof(struct SaveBlock1, vars),
};

STATIC_ASSERT(sizeof(struct E2ETestRequest) == 88, E2ETestRequestSize);
STATIC_ASSERT(offsetof(struct E2ETestRequest, status) == 87, E2ETestRequestStatusOffset);
STATIC_ASSERT(sizeof(struct E2ETestResult) == 16, E2ETestResultSize);
STATIC_ASSERT(offsetof(struct E2ETestResult, status) == 14, E2ETestResultStatusOffset);
STATIC_ASSERT(sizeof(struct E2ETestState) == 20, E2ETestStateSize);
STATIC_ASSERT(sizeof(struct E2ETestAbi) == 16, E2ETestAbiSize);

enum E2ETestInternalStage
{
    E2E_TEST_STAGE_IDLE,
    E2E_TEST_STAGE_WAIT_NEW_GAME,
    E2E_TEST_STAGE_WAIT_FIELD,
    E2E_TEST_STAGE_WAIT_FACING,
};

static const u8 sDefaultPlayerName[] = COMPOUND_STRING("ETHAN");

static struct E2ETestRequest sRequest;
static enum E2ETestInternalStage sStage;
static u16 sMapGroup;
static u16 sMapNum;
static s16 sX;
static s16 sY;

static bool32 IsSettledOverworld(void)
{
    return gSaveBlock1Ptr != NULL
        && gSaveBlock2Ptr != NULL
        && gMain.callback1 == CB1_Overworld
        && gMain.callback2 == CB2_Overworld
        && gMain.state == 0
        && !gMain.inBattle
        && !gLinkTransferringData
        && !ArePlayerFieldControlsLocked()
        && IsFieldMessageBoxHidden();
}

static void CopyRequest(void)
{
    u32 i;

    sRequest.requestId = gE2ETestRequest.requestId;
    sRequest.mapGroup = gE2ETestRequest.mapGroup;
    sRequest.mapNum = gE2ETestRequest.mapNum;
    sRequest.x = gE2ETestRequest.x;
    sRequest.y = gE2ETestRequest.y;
    sRequest.rngSeed = gE2ETestRequest.rngSeed;
    for (i = 0; i < E2E_TEST_MAX_VARS; i++)
    {
        sRequest.vars[i].id = gE2ETestRequest.vars[i].id;
        sRequest.vars[i].value = gE2ETestRequest.vars[i].value;
    }
    for (i = 0; i < E2E_TEST_MAX_FLAGS; i++)
    {
        sRequest.flags[i].id = gE2ETestRequest.flags[i].id;
        sRequest.flags[i].value = gE2ETestRequest.flags[i].value;
        sRequest.flags[i].reserved = gE2ETestRequest.flags[i].reserved;
    }
    sRequest.checkpoint = gE2ETestRequest.checkpoint;
    sRequest.facing = gE2ETestRequest.facing;
    sRequest.varCount = gE2ETestRequest.varCount;
    sRequest.flagCount = gE2ETestRequest.flagCount;
    sRequest.textSpeed = gE2ETestRequest.textSpeed;
    sRequest.useRngSeed = gE2ETestRequest.useRngSeed;
    sRequest.command = gE2ETestRequest.command;
    sRequest.status = gE2ETestRequest.status;
}

static void PublishResult(u8 status, u8 phase, u16 error)
{
    gE2ETestResult.requestId = sRequest.requestId;
    gE2ETestResult.mapGroup = sMapGroup;
    gE2ETestResult.mapNum = sMapNum;
    gE2ETestResult.x = sX;
    gE2ETestResult.y = sY;
    gE2ETestResult.error = error;
    gE2ETestResult.phase = phase;
    gE2ETestResult.status = status;
}

static void FailRequest(enum E2ETestError error)
{
    sStage = E2E_TEST_STAGE_IDLE;
    gE2ETestRequest.status = E2E_TEST_STATUS_ERROR;
    PublishResult(E2E_TEST_STATUS_ERROR, E2E_TEST_ARRANGE_PHASE_VALIDATE, error);
}

static bool32 ResolveCheckpoint(void)
{
    switch (sRequest.checkpoint)
    {
    case E2E_TEST_CHECKPOINT_BEDROOM_BEFORE_CLOCK:
        sMapGroup = MAP_GROUP(MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_2F_HNS);
        sMapNum = MAP_NUM(MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_2F_HNS);
        sX = 10;
        sY = 6;
        break;
    case E2E_TEST_CHECKPOINT_NEW_BARK_AFTER_INTRO:
        sMapGroup = MAP_GROUP(MAP_NEW_BARK_TOWN_HNS);
        sMapNum = MAP_NUM(MAP_NEW_BARK_TOWN_HNS);
        sX = 9;
        sY = 11;
        break;
    case E2E_TEST_CHECKPOINT_ELM_LAB_BEFORE_INTRO:
        sMapGroup = MAP_GROUP(MAP_NEW_BARK_TOWN_LAB_HNS);
        sMapNum = MAP_NUM(MAP_NEW_BARK_TOWN_LAB_HNS);
        sX = 6;
        sY = 8;
        break;
    default:
        return FALSE;
    }

    if (sRequest.mapGroup != E2E_TEST_KEEP_MAP || sRequest.mapNum != E2E_TEST_KEEP_MAP)
    {
        if (sRequest.mapGroup == E2E_TEST_KEEP_MAP || sRequest.mapNum == E2E_TEST_KEEP_MAP)
            return FALSE;
        sMapGroup = sRequest.mapGroup;
        sMapNum = sRequest.mapNum;
    }
    if (sRequest.x != E2E_TEST_KEEP_COORDINATE || sRequest.y != E2E_TEST_KEEP_COORDINATE)
    {
        if (sRequest.x == E2E_TEST_KEEP_COORDINATE || sRequest.y == E2E_TEST_KEEP_COORDINATE)
            return FALSE;
        sX = sRequest.x;
        sY = sRequest.y;
    }

    return TRUE;
}

static enum E2ETestError ValidateRequest(void)
{
    const struct MapHeader *mapHeader;
    u32 i;

    if (sRequest.command != E2E_TEST_COMMAND_ARRANGE)
        return E2E_TEST_ERROR_COMMAND;
    if (!ResolveCheckpoint())
        return E2E_TEST_ERROR_CHECKPOINT;
    if (sMapGroup >= MAP_GROUPS_COUNT || sMapNum >= MAP_GROUP_COUNT[sMapGroup])
        return E2E_TEST_ERROR_MAP;
    mapHeader = Overworld_GetMapHeaderByGroupAndId(sMapGroup, sMapNum);
    if (mapHeader == NULL || mapHeader->mapLayout == NULL)
        return E2E_TEST_ERROR_MAP;
    if (sX < 0 || sY < 0 || sX >= mapHeader->mapLayout->width || sY >= mapHeader->mapLayout->height)
        return E2E_TEST_ERROR_COORDINATES;
    if (sRequest.facing < DIR_SOUTH || sRequest.facing > DIR_EAST)
        return E2E_TEST_ERROR_FACING;
    if (sRequest.textSpeed != E2E_TEST_KEEP_TEXT_SPEED
     && sRequest.textSpeed > OPTIONS_TEXT_SPEED_INSTANT)
        return E2E_TEST_ERROR_TEXT_SPEED;
    if (sRequest.varCount > E2E_TEST_MAX_VARS)
        return E2E_TEST_ERROR_VAR_COUNT;
    for (i = 0; i < sRequest.varCount; i++)
    {
        if (sRequest.vars[i].id < VARS_START || sRequest.vars[i].id > VARS_END)
            return E2E_TEST_ERROR_VAR;
    }
    if (sRequest.flagCount > E2E_TEST_MAX_FLAGS)
        return E2E_TEST_ERROR_FLAG_COUNT;
    for (i = 0; i < sRequest.flagCount; i++)
    {
        if (sRequest.flags[i].id == 0
         || sRequest.flags[i].id >= FLAGS_COUNT
         || sRequest.flags[i].value > TRUE
         || sRequest.flags[i].reserved != 0)
            return E2E_TEST_ERROR_FLAG;
    }

    return E2E_TEST_ERROR_NONE;
}

static void ApplyCheckpointDefaults(void)
{
    StringCopy(gSaveBlock2Ptr->playerName, sDefaultPlayerName);
    gSaveBlock2Ptr->playerGender = MALE;

    switch (sRequest.checkpoint)
    {
    case E2E_TEST_CHECKPOINT_BEDROOM_BEFORE_CLOCK:
        VarSet(VAR_NEWBARK_TOWN_STATE, 0);
        VarSet(VAR_NEWBARKTOWN_LABSTATE, 0);
        break;
    case E2E_TEST_CHECKPOINT_NEW_BARK_AFTER_INTRO:
    case E2E_TEST_CHECKPOINT_ELM_LAB_BEFORE_INTRO:
        VarSet(VAR_NEWBARK_TOWN_STATE, 2);
        VarSet(VAR_NEWBARKTOWN_LABSTATE, 0);
        break;
    }
}

static void ApplyOverrides(void)
{
    u32 i;

    for (i = 0; i < sRequest.varCount; i++)
        VarSet(sRequest.vars[i].id, sRequest.vars[i].value);
    for (i = 0; i < sRequest.flagCount; i++)
    {
        if (sRequest.flags[i].value)
            FlagSet(sRequest.flags[i].id);
        else
            FlagClear(sRequest.flags[i].id);
    }
    if (sRequest.textSpeed != E2E_TEST_KEEP_TEXT_SPEED)
        gSaveBlock2Ptr->optionsTextSpeed = sRequest.textSpeed;
    if (sRequest.useRngSeed)
    {
        SeedRng(sRequest.rngSeed);
        SeedRng2(sRequest.rngSeed);
    }
}

static void StartWarp(void)
{
    SetWarpDestination(sMapGroup, sMapNum, WARP_ID_NONE, sX, sY);
    WarpIntoMap();
    gFieldCallback = FieldCB_WarpExitFadeFromBlack;
    gFieldCallback2 = NULL;
    PublishResult(E2E_TEST_STATUS_RUNNING, E2E_TEST_ARRANGE_PHASE_WARP, E2E_TEST_ERROR_NONE);
    SetMainCallback2(CB2_LoadMap);
    sStage = E2E_TEST_STAGE_WAIT_FIELD;
}

static void BeginRequest(void)
{
    enum E2ETestError error;

    CopyRequest();
    error = ValidateRequest();
    if (error != E2E_TEST_ERROR_NONE)
    {
        FailRequest(error);
        return;
    }

    if (gSaveBlock1Ptr == NULL || gSaveBlock2Ptr == NULL)
        SetSaveBlocksPointers(0);

    gE2ETestRequest.status = E2E_TEST_STATUS_RUNNING;
    PublishResult(E2E_TEST_STATUS_RUNNING, E2E_TEST_ARRANGE_PHASE_NEW_GAME, E2E_TEST_ERROR_NONE);
    SetMainCallback2(CB2_NewGame);
    sStage = E2E_TEST_STAGE_WAIT_NEW_GAME;
}

static void UpdateRequest(void)
{
    switch (sStage)
    {
    case E2E_TEST_STAGE_IDLE:
        if (gE2ETestRequest.status == E2E_TEST_STATUS_PENDING)
            BeginRequest();
        break;
    case E2E_TEST_STAGE_WAIT_NEW_GAME:
        if (!IsSettledOverworld())
            break;
        PublishResult(E2E_TEST_STATUS_RUNNING, E2E_TEST_ARRANGE_PHASE_STATE, E2E_TEST_ERROR_NONE);
        ApplyCheckpointDefaults();
        ApplyOverrides();
        StartWarp();
        break;
    case E2E_TEST_STAGE_WAIT_FIELD:
        if (!IsSettledOverworld()
         || gSaveBlock1Ptr->location.mapGroup != sMapGroup
         || gSaveBlock1Ptr->location.mapNum != sMapNum)
            break;
        PlayerFaceDirection(sRequest.facing);
        sStage = E2E_TEST_STAGE_WAIT_FACING;
        break;
    case E2E_TEST_STAGE_WAIT_FACING:
        if (!IsSettledOverworld() || !IsPlayerStandingStill())
            break;
        sStage = E2E_TEST_STAGE_IDLE;
        gE2ETestRequest.status = E2E_TEST_STATUS_SUCCESS;
        PublishResult(E2E_TEST_STATUS_SUCCESS, E2E_TEST_ARRANGE_PHASE_FIELD_READY, E2E_TEST_ERROR_NONE);
        break;
    }
}

static void UpdateState(void)
{
    bool32 overworld = gMain.callback1 == CB1_Overworld && gMain.callback2 == CB2_Overworld;

    gE2ETestState.frame++;
    gE2ETestState.phase = E2E_TEST_GAME_PHASE_BOOT;
    gE2ETestState.ready = FALSE;
    gE2ETestState.controlsLocked = TRUE;
    gE2ETestState.scriptActive = FALSE;
    gE2ETestState.dialogueOpen = FALSE;
    gE2ETestState.facing = 0;
    gE2ETestState.mapGroup = E2E_TEST_KEEP_MAP;
    gE2ETestState.mapNum = E2E_TEST_KEEP_MAP;
    gE2ETestState.x = E2E_TEST_KEEP_COORDINATE;
    gE2ETestState.y = E2E_TEST_KEEP_COORDINATE;

    if (gMain.inBattle)
        gE2ETestState.phase = E2E_TEST_GAME_PHASE_BATTLE;
    else if (overworld)
        gE2ETestState.phase = IsFieldMessageBoxHidden()
            ? E2E_TEST_GAME_PHASE_OVERWORLD
            : E2E_TEST_GAME_PHASE_DIALOGUE;

    if (gSaveBlock1Ptr == NULL)
        return;

    gE2ETestState.mapGroup = gSaveBlock1Ptr->location.mapGroup;
    gE2ETestState.mapNum = gSaveBlock1Ptr->location.mapNum;
    gE2ETestState.x = gSaveBlock1Ptr->pos.x;
    gE2ETestState.y = gSaveBlock1Ptr->pos.y;
    if (!overworld)
        return;

    gE2ETestState.controlsLocked = ArePlayerFieldControlsLocked();
    gE2ETestState.scriptActive = ScriptContext_IsEnabled();
    gE2ETestState.dialogueOpen = !IsFieldMessageBoxHidden();
    gE2ETestState.facing = GetPlayerFacingDirection();
    gE2ETestState.ready = IsSettledOverworld();
}

void E2ETest_Update(void)
{
    UpdateState();
    UpdateRequest();
}

#endif // E2E_TESTING
