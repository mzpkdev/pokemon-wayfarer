#ifdef E2E_TESTING

#include "global.h"
#include "e2e_test.h"
#include "challenge_menu.h"
#include "event_data.h"
#include "field_move.h"
#include "field_effect.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "item.h"
#include "load_save.h"
#include "main.h"
#include "menu.h"
#include "new_game.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "script.h"
#include "sprite.h"
#include "string_util.h"
#include "constants/field_effects.h"
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
    .version = 6,
    .requestSize = sizeof(struct E2ETestRequest),
    .resultSize = sizeof(struct E2ETestResult),
    .stateSize = sizeof(struct E2ETestState),
    .requestStatusOffset = offsetof(struct E2ETestRequest, status),
    .resultStatusOffset = offsetof(struct E2ETestResult, status),
    .flagsOffset = offsetof(struct SaveBlock1, flags),
    .varsOffset = offsetof(struct SaveBlock1, vars),
};

STATIC_ASSERT(sizeof(struct E2ETestRequest) == 196, E2ETestRequestSize);
STATIC_ASSERT(offsetof(struct E2ETestRequest, status) == 87, E2ETestRequestStatusOffset);
STATIC_ASSERT(sizeof(struct E2ETestResult) == 16, E2ETestResultSize);
STATIC_ASSERT(offsetof(struct E2ETestResult, status) == 14, E2ETestResultStatusOffset);
STATIC_ASSERT(sizeof(struct E2ETestState) == 160, E2ETestStateSize);
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
static u16 sLastFieldMove = MOVE_NONE;
static u8 sLastFieldMoveUser;
static u8 sLastFieldMoveResult;
static bool32 sLastFieldMoveUnlocked;
static u8 sLastDialogueMessage;
static u32 sDialogueSequence;
static u8 sLastDialogueText[E2E_TEST_FIELD_MESSAGE_TEXT_LENGTH];

extern void UpdateSurfBlobFieldEffect(struct Sprite *sprite);

static const u8 sTextFieldMoveUsed[] = _("{STR_VAR_1} used {STR_VAR_2}!");
static const u8 sTextFieldMoveNeedsHm[] = _("You need the matching HM to use\nthat move.");
static const u8 sTextFieldMoveNoEligibleMon[] = _("None of your party POKéMON can use\nthat move.");
static const u8 sTextWantToUseSurf[] = _("The water is dyed a deep blue…\nWould you like to SURF?");
static const u8 sTextPlayerUsedSurf[] = _("{STR_VAR_1} used SURF!");

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

void E2ETest_RecordFieldMove(enum Move move, u8 partyIndex, u8 result)
{
    enum FieldMove fieldMove;

    sLastFieldMove = move;
    sLastFieldMoveUser = partyIndex;
    sLastFieldMoveResult = result;
    sLastFieldMoveUnlocked = FALSE;
    for (fieldMove = 0; fieldMove < FIELD_MOVES_COUNT; fieldMove++)
    {
        if (FieldMove_GetMoveId(fieldMove) == move)
        {
            sLastFieldMoveUnlocked = IsFieldMoveUnlocked(fieldMove);
            break;
        }
    }
}

void E2ETest_RecordFieldMessage(const u8 *str)
{
    if (StringCompare(str, sTextFieldMoveUsed) == 0)
        sLastDialogueMessage = E2E_TEST_DIALOGUE_FIELD_MOVE_USED;
    else if (StringCompare(str, sTextFieldMoveNeedsHm) == 0)
        sLastDialogueMessage = E2E_TEST_DIALOGUE_FIELD_MOVE_NEEDS_HM;
    else if (StringCompare(str, sTextFieldMoveNoEligibleMon) == 0)
        sLastDialogueMessage = E2E_TEST_DIALOGUE_FIELD_MOVE_NO_ELIGIBLE_MON;
    else if (StringCompare(str, sTextWantToUseSurf) == 0)
        sLastDialogueMessage = E2E_TEST_DIALOGUE_WANT_TO_USE_SURF;
    else if (StringCompare(str, sTextPlayerUsedSurf) == 0)
        sLastDialogueMessage = E2E_TEST_DIALOGUE_PLAYER_USED_SURF;
    else
        sLastDialogueMessage = E2E_TEST_DIALOGUE_UNKNOWN;

    sDialogueSequence++;
}

void E2ETest_RecordExpandedFieldMessage(const u8 *str)
{
    u32 i;

    for (i = 0; i < E2E_TEST_FIELD_MESSAGE_TEXT_LENGTH - 1 && str[i] != EOS; i++)
        sLastDialogueText[i] = str[i];
    sLastDialogueText[i] = EOS;
    for (i++; i < E2E_TEST_FIELD_MESSAGE_TEXT_LENGTH; i++)
        sLastDialogueText[i] = EOS;
}

static void ResetObservations(void)
{
    sLastFieldMove = MOVE_NONE;
    sLastFieldMoveUser = PARTY_SIZE;
    sLastFieldMoveResult = 0xFF;
    sLastFieldMoveUnlocked = FALSE;
    sLastDialogueMessage = E2E_TEST_DIALOGUE_NONE;
    sDialogueSequence = 0;
    memset(sLastDialogueText, EOS, sizeof(sLastDialogueText));
}

static void ApplyPartyFixtures(void)
{
    u32 i;

    memset(gPlayerParty, 0, sizeof(gPlayerParty));
    gPlayerPartyCount = 0;
    for (i = 0; i < sRequest.partyCount; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u32 move;

        CreateMonWithIVs(mon, sRequest.party[i].species, 20, i, OTID_STRUCT_PRESET(0), 0);
        for (move = 0; move < MAX_MON_MOVES; move++)
            SetMonMoveSlot(mon, sRequest.party[i].moves[move], move);
        if (sRequest.party[i].isEgg)
        {
            bool8 isEgg = TRUE;

            SetMonData(mon, MON_DATA_IS_EGG, &isEgg);
        }
        if (sRequest.party[i].fainted)
        {
            u16 hp = 0;

            SetMonData(mon, MON_DATA_HP, &hp);
        }
        gPlayerPartyCount++;
    }

    if (gPlayerPartyCount != 0)
        FlagSet(FLAG_SYS_POKEMON_GET);
}

static bool32 ApplyBagFixtures(void)
{
    u32 i;

    ClearBag();
    for (i = 0; i < sRequest.bagItemCount; i++)
    {
        if (!AddBagItem(sRequest.bagItems[i].item, sRequest.bagItems[i].quantity))
            return FALSE;
    }

    return TRUE;
}

static void ApplyHMsOverwriteFixture(void)
{
    memset(&gSaveBlock3Ptr->challengeSettings, 0, sizeof(gSaveBlock3Ptr->challengeSettings));
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_OneTypeChallenge = 31; // ONE_TYPE_OFF
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_Nuzlocke = sRequest.hmsOverwrite;
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
    for (i = 0; i < E2E_TEST_MAX_PARTY; i++)
    {
        u32 move;

        sRequest.party[i].species = gE2ETestRequest.party[i].species;
        for (move = 0; move < MAX_MON_MOVES; move++)
            sRequest.party[i].moves[move] = gE2ETestRequest.party[i].moves[move];
        sRequest.party[i].isEgg = gE2ETestRequest.party[i].isEgg;
        sRequest.party[i].fainted = gE2ETestRequest.party[i].fainted;
    }
    for (i = 0; i < E2E_TEST_MAX_BAG_ITEMS; i++)
    {
        sRequest.bagItems[i].item = gE2ETestRequest.bagItems[i].item;
        sRequest.bagItems[i].quantity = gE2ETestRequest.bagItems[i].quantity;
    }
    sRequest.partyCount = gE2ETestRequest.partyCount;
    sRequest.bagItemCount = gE2ETestRequest.bagItemCount;
    sRequest.hmsOverwrite = gE2ETestRequest.hmsOverwrite;
    sRequest.reserved = gE2ETestRequest.reserved;
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
    if (sRequest.partyCount > E2E_TEST_MAX_PARTY)
        return E2E_TEST_ERROR_PARTY_COUNT;
    for (i = 0; i < sRequest.partyCount; i++)
    {
        u32 move;

        if (sRequest.party[i].species == SPECIES_NONE
         || sRequest.party[i].species > NUM_SPECIES
         || !IsSpeciesEnabled(sRequest.party[i].species)
         || sRequest.party[i].isEgg > TRUE
         || sRequest.party[i].fainted > TRUE)
            return E2E_TEST_ERROR_PARTY;
        for (move = 0; move < MAX_MON_MOVES; move++)
        {
            if (sRequest.party[i].moves[move] >= MOVES_COUNT)
                return E2E_TEST_ERROR_MOVE;
        }
    }
    if (sRequest.bagItemCount > E2E_TEST_MAX_BAG_ITEMS)
        return E2E_TEST_ERROR_BAG_ITEM_COUNT;
    for (i = 0; i < sRequest.bagItemCount; i++)
    {
        enum Move move = GetItemTMHMMoveId(sRequest.bagItems[i].item);

        if (!IsMoveHM(move)
         || sRequest.bagItems[i].quantity == 0
         || sRequest.bagItems[i].quantity > MAX_BAG_ITEM_CAPACITY)
            return E2E_TEST_ERROR_BAG_ITEM;
    }
    if (sRequest.hmsOverwrite > TRUE || sRequest.reserved != 0)
        return E2E_TEST_ERROR_PARTY;

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

static bool32 ApplyOverrides(void)
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
    ApplyPartyFixtures();
    if (!ApplyBagFixtures())
        return FALSE;
    ApplyHMsOverwriteFixture();
    ResetObservations();

    return TRUE;
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
        if (!ApplyOverrides())
        {
            FailRequest(E2E_TEST_ERROR_BAG_ITEM);
            break;
        }
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

static const enum Item sHmItems[E2E_TEST_MAX_BAG_ITEMS] =
{
    ITEM_HM_CUT,
    ITEM_HM_FLY,
    ITEM_HM_SURF,
    ITEM_HM_STRENGTH,
    ITEM_HM_FLASH,
    ITEM_HM_ROCK_SMASH,
    ITEM_HM_WATERFALL,
    ITEM_HM_WHIRLPOOL,
};

static u8 CountSurfBlobs(void)
{
    u32 i;
    u8 count = 0;

    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].callback == UpdateSurfBlobFieldEffect)
            count++;
    }
    return count;
}

static void UpdateState(void)
{
    bool32 overworld = gMain.callback1 == CB1_Overworld && gMain.callback2 == CB2_Overworld;
    u32 i;
    u8 partyMenuActionCount;
    u8 partyMenuActions[E2E_TEST_MAX_PARTY_MENU_ACTIONS];

    gE2ETestState.frame++;
    gE2ETestState.phase = E2E_TEST_GAME_PHASE_BOOT;
    gE2ETestState.ready = FALSE;
    gE2ETestState.controlsLocked = TRUE;
    gE2ETestState.scriptActive = FALSE;
    gE2ETestState.dialogueOpen = FALSE;
    gE2ETestState.facing = 0;
    gE2ETestState.avatarFlags = 0;
    gE2ETestState.avatarSurfing = FALSE;
    gE2ETestState.surfBlobCount = 0;
    gE2ETestState.surfEffectActive = FALSE;
    gE2ETestState.fieldMoveMove = sLastFieldMove;
    gE2ETestState.fieldMoveUser = sLastFieldMoveUser;
    gE2ETestState.fieldMoveResult = sLastFieldMoveResult;
    gE2ETestState.fieldMoveUnlocked = sLastFieldMoveUnlocked;
    gE2ETestState.fieldMoveUserSpecies = SPECIES_NONE;
    gE2ETestState.partyCount = 0;
    gE2ETestState.hmsOverwrite = FALSE;
    gE2ETestState.uiMode = E2E_TEST_UI_OVERWORLD;
    gE2ETestState.partyMenuActionCount = 0;
    gE2ETestState.dialogueMessage = sLastDialogueMessage;
    gE2ETestState.dialogueSequence = sDialogueSequence;
    memcpy((void *)gE2ETestState.dialogueText, sLastDialogueText, sizeof(sLastDialogueText));
    gE2ETestState.partyEggMask = 0;
    gE2ETestState.partyFaintedMask = 0;
    gE2ETestState.mapGroup = E2E_TEST_KEEP_MAP;
    gE2ETestState.mapNum = E2E_TEST_KEEP_MAP;
    gE2ETestState.x = E2E_TEST_KEEP_COORDINATE;
    gE2ETestState.y = E2E_TEST_KEEP_COORDINATE;
    for (i = 0; i < E2E_TEST_MAX_PARTY; i++)
    {
        u32 move;

        gE2ETestState.partySpecies[i] = SPECIES_NONE;
        for (move = 0; move < MAX_MON_MOVES; move++)
            gE2ETestState.partyMoves[i][move] = MOVE_NONE;
    }
    for (i = 0; i < E2E_TEST_MAX_BAG_ITEMS; i++)
    {
        gE2ETestState.bagItemCounts[i] = 0;
        gE2ETestState.partyMenuActions[i] = 0xFF;
    }

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
    gE2ETestState.partyCount = gPlayerPartyCount;
    gE2ETestState.hmsOverwrite = HMsOverwriteOptionActive();
    for (i = 0; i < E2E_TEST_MAX_PARTY; i++)
    {
        u32 move;
        struct Pokemon *mon = &gPlayerParty[i];

        gE2ETestState.partySpecies[i] = GetMonData(mon, MON_DATA_SPECIES);
        for (move = 0; move < MAX_MON_MOVES; move++)
            gE2ETestState.partyMoves[i][move] = GetMonData(mon, MON_DATA_MOVE1 + move);
        if (GetMonData(mon, MON_DATA_IS_EGG))
            gE2ETestState.partyEggMask |= 1 << i;
        if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE && GetMonData(mon, MON_DATA_HP) == 0)
            gE2ETestState.partyFaintedMask |= 1 << i;
    }
    for (i = 0; i < E2E_TEST_MAX_BAG_ITEMS; i++)
        gE2ETestState.bagItemCounts[i] = CountTotalItemQuantityInBag(sHmItems[i]);
    if (sLastFieldMoveUser < PARTY_SIZE)
        gE2ETestState.fieldMoveUserSpecies = GetMonData(&gPlayerParty[sLastFieldMoveUser], MON_DATA_SPECIES);
    if (E2ETest_IsSummaryScreenOpen())
        gE2ETestState.uiMode = E2E_TEST_UI_SUMMARY;
    else if (E2ETest_IsPartyMenuOpen())
        gE2ETestState.uiMode = E2E_TEST_UI_PARTY_MENU;
    else if (GetStartMenuWindowId() != WINDOW_NONE)
        gE2ETestState.uiMode = E2E_TEST_UI_PAUSE_MENU;
    else if (!IsFieldMessageBoxHidden())
        gE2ETestState.uiMode = E2E_TEST_UI_DIALOGUE;

    E2ETest_GetPartyMenuActions(partyMenuActions, &partyMenuActionCount);
    gE2ETestState.partyMenuActionCount = partyMenuActionCount;
    for (i = 0; i < partyMenuActionCount; i++)
        gE2ETestState.partyMenuActions[i] = partyMenuActions[i];

    if (!overworld)
        return;

    gE2ETestState.controlsLocked = ArePlayerFieldControlsLocked();
    gE2ETestState.scriptActive = ScriptContext_IsEnabled();
    gE2ETestState.dialogueOpen = !IsFieldMessageBoxHidden();
    gE2ETestState.facing = GetPlayerFacingDirection();
    gE2ETestState.avatarFlags = GetPlayerAvatarFlags();
    gE2ETestState.avatarSurfing = TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING);
    gE2ETestState.surfBlobCount = CountSurfBlobs();
    gE2ETestState.surfEffectActive = FieldEffectActiveListContains(FLDEFF_USE_SURF);
    gE2ETestState.ready = IsSettledOverworld();
}

void E2ETest_Update(void)
{
    UpdateState();
    UpdateRequest();
}

#endif // E2E_TESTING
