#ifdef E2E_TESTING

#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
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
#include "pokemon_storage_system.h"
#include "random.h"
#include "region_map.h"
#include "save.h"
#include "script.h"
#include "sprite.h"
#include "string_util.h"
#include "wild_encounter.h"
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
    .version = 8,
    .requestSize = sizeof(struct E2ETestRequest),
    .resultSize = sizeof(struct E2ETestResult),
    .stateSize = sizeof(struct E2ETestState),
    .requestStatusOffset = offsetof(struct E2ETestRequest, status),
    .resultStatusOffset = offsetof(struct E2ETestResult, status),
    .flagsOffset = offsetof(struct SaveBlock1, flags),
    .varsOffset = offsetof(struct SaveBlock1, vars),
};

STATIC_ASSERT(sizeof(struct E2ETestRequest) == 424, E2ETestRequestSize);
STATIC_ASSERT(offsetof(struct E2ETestRequest, status) == 87, E2ETestRequestStatusOffset);
STATIC_ASSERT(sizeof(struct E2ETestResult) == 16, E2ETestResultSize);
STATIC_ASSERT(offsetof(struct E2ETestResult, status) == 14, E2ETestResultStatusOffset);
STATIC_ASSERT(sizeof(struct E2ETestState) == 344, E2ETestStateSize);
STATIC_ASSERT(sizeof(struct E2ETestAbi) == 16, E2ETestAbiSize);

enum E2ETestInternalStage
{
    E2E_TEST_STAGE_IDLE,
    E2E_TEST_STAGE_WAIT_NEW_GAME,
    E2E_TEST_STAGE_WAIT_FIELD,
    E2E_TEST_STAGE_WAIT_FACING,
    E2E_TEST_STAGE_WAIT_BATTLE,
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
static struct E2ETestBagItem sObservedBagItems[E2E_TEST_MAX_BAG_ITEMS];
static struct E2ETestPcSlot sObservedPcSlots[E2E_TEST_MAX_PC_SLOTS];
static u8 sObservedBagItemCount;
static u8 sObservedPcSlotCount;
static u16 sCaughtSpecies;
static u8 sCatchSwapState;
static u8 sCatchSwapCursor;
static u8 sCatchSwapSelectedParty;
static u8 sCatchSwapBox;
static u8 sCatchSwapSlot;
static bool32 sNicknamePrompt;
static u8 sNicknameCursor;
static u8 sRejectedResultFrames;
static struct RegionMap sObservedPokedexRegionMap;

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

void E2ETest_RecordCapture(u16 species)
{
    sCaughtSpecies = species;
}

void E2ETest_RecordCatchSwap(u8 state, u8 cursor, u8 selectedParty, u8 boxId, u8 boxPosition)
{
    sCatchSwapState = state;
    sCatchSwapCursor = cursor;
    sCatchSwapSelectedParty = selectedParty;
    sCatchSwapBox = boxId;
    sCatchSwapSlot = boxPosition;
}

void E2ETest_RecordNicknamePrompt(bool32 active, u8 cursor)
{
    sNicknamePrompt = active;
    sNicknameCursor = cursor;
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
    sCaughtSpecies = SPECIES_NONE;
    sCatchSwapState = E2E_TEST_CATCH_SWAP_NONE;
    sCatchSwapCursor = 0;
    sCatchSwapSelectedParty = PARTY_SIZE;
    sCatchSwapBox = TOTAL_BOXES_COUNT;
    sCatchSwapSlot = IN_BOX_COUNT;
    sNicknamePrompt = FALSE;
    sNicknameCursor = 0;
}

static void CreateFixtureMon(struct Pokemon *mon, const struct E2ETestMonFixture *fixture, u32 personality, bool32 fainted)
{
    u32 move;

    CreateMonWithIVs(mon, fixture->species, fixture->level, personality, OTID_STRUCT_PRESET(0), 0);
    for (move = 0; move < MAX_MON_MOVES; move++)
        SetMonMoveSlot(mon, fixture->moves[move], move);
    if (fixture->isEgg)
    {
        bool8 isEgg = TRUE;

        SetMonData(mon, MON_DATA_IS_EGG, &isEgg);
    }
    if (fainted)
    {
        u16 hp = 0;

        SetMonData(mon, MON_DATA_HP, &hp);
    }
}

static void ApplyPartyFixtures(void)
{
    u32 i;

    memset(gPlayerParty, 0, sizeof(gPlayerParty));
    gPlayerPartyCount = 0;
    for (i = 0; i < sRequest.partyCount; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];

        CreateFixtureMon(mon, &sRequest.party[i].mon, i, sRequest.party[i].fainted);
        gPlayerPartyCount++;
    }

    if (gPlayerPartyCount != 0)
        FlagSet(FLAG_SYS_POKEMON_GET);
}

static void ApplyPcFixtures(void)
{
    u32 i;

    ResetPokemonStorageSystem();
    E2ETest_SetStorageCurrentBox(sRequest.currentBox);
    VarSet(VAR_PC_BOX_TO_SEND_MON, sRequest.currentBox);
    sObservedPcSlotCount = sRequest.pcSlotCount;
    for (i = 0; i < sRequest.pcSlotCount; i++)
    {
        struct Pokemon mon;

        sObservedPcSlots[i] = sRequest.pcSlots[i];
        if (sRequest.pcSlots[i].mon.species == SPECIES_NONE)
            continue;
        CreateFixtureMon(&mon, &sRequest.pcSlots[i].mon, PARTY_SIZE + i, FALSE);
        SetBoxMonAt(sRequest.pcSlots[i].boxId, sRequest.pcSlots[i].boxPosition, &mon.box);
    }
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

    if (sRequest.fullPocketMask & E2E_TEST_FULL_POCKET_ITEMS)
    {
        struct BagPocket *pocket = &gBagPockets[POCKET_ITEMS];

        for (i = 0; i < pocket->capacity; i++)
        {
            if (BagPocket_GetSlotData(pocket, i).itemId == ITEM_NONE)
                BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_LEFTOVERS, 1);
        }
    }
    if (sRequest.fullPocketMask & E2E_TEST_FULL_POCKET_KEY_ITEMS)
    {
        struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];

        for (i = 0; i < pocket->capacity; i++)
        {
            if (BagPocket_GetSlotData(pocket, i).itemId == ITEM_NONE)
                BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_OLD_ROD, 1);
        }
    }
    if (sRequest.fullPocketMask & E2E_TEST_FULL_POCKET_TM_HM)
    {
        struct BagPocket *pocket = &gBagPockets[POCKET_TM_HM];

        for (i = 0; i < pocket->capacity; i++)
        {
            if (BagPocket_GetSlotData(pocket, i).itemId == ITEM_NONE)
                BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_TM_FOCUS_PUNCH, 1);
        }
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

        sRequest.party[i].mon.species = gE2ETestRequest.party[i].mon.species;
        for (move = 0; move < MAX_MON_MOVES; move++)
            sRequest.party[i].mon.moves[move] = gE2ETestRequest.party[i].mon.moves[move];
        sRequest.party[i].mon.level = gE2ETestRequest.party[i].mon.level;
        sRequest.party[i].mon.isEgg = gE2ETestRequest.party[i].mon.isEgg;
        sRequest.party[i].mon.reserved = gE2ETestRequest.party[i].mon.reserved;
        sRequest.party[i].fainted = gE2ETestRequest.party[i].fainted;
        memcpy(sRequest.party[i].reserved, (const void *)gE2ETestRequest.party[i].reserved, sizeof(sRequest.party[i].reserved));
    }
    for (i = 0; i < E2E_TEST_MAX_BAG_ITEMS; i++)
    {
        sRequest.bagItems[i].item = gE2ETestRequest.bagItems[i].item;
        sRequest.bagItems[i].quantity = gE2ETestRequest.bagItems[i].quantity;
    }
    for (i = 0; i < E2E_TEST_MAX_PC_SLOTS; i++)
    {
        u32 move;

        sRequest.pcSlots[i].mon.species = gE2ETestRequest.pcSlots[i].mon.species;
        for (move = 0; move < MAX_MON_MOVES; move++)
            sRequest.pcSlots[i].mon.moves[move] = gE2ETestRequest.pcSlots[i].mon.moves[move];
        sRequest.pcSlots[i].mon.level = gE2ETestRequest.pcSlots[i].mon.level;
        sRequest.pcSlots[i].mon.isEgg = gE2ETestRequest.pcSlots[i].mon.isEgg;
        sRequest.pcSlots[i].mon.reserved = gE2ETestRequest.pcSlots[i].mon.reserved;
        sRequest.pcSlots[i].boxId = gE2ETestRequest.pcSlots[i].boxId;
        sRequest.pcSlots[i].boxPosition = gE2ETestRequest.pcSlots[i].boxPosition;
        memcpy(sRequest.pcSlots[i].reserved, (const void *)gE2ETestRequest.pcSlots[i].reserved, sizeof(sRequest.pcSlots[i].reserved));
    }
    sRequest.wildMon.species = gE2ETestRequest.wildMon.species;
    for (i = 0; i < MAX_MON_MOVES; i++)
        sRequest.wildMon.moves[i] = gE2ETestRequest.wildMon.moves[i];
    sRequest.wildMon.level = gE2ETestRequest.wildMon.level;
    sRequest.wildMon.isEgg = gE2ETestRequest.wildMon.isEgg;
    sRequest.wildMon.reserved = gE2ETestRequest.wildMon.reserved;
    sRequest.partyCount = gE2ETestRequest.partyCount;
    sRequest.bagItemCount = gE2ETestRequest.bagItemCount;
    sRequest.pcSlotCount = gE2ETestRequest.pcSlotCount;
    sRequest.currentBox = gE2ETestRequest.currentBox;
    sRequest.hmsOverwrite = gE2ETestRequest.hmsOverwrite;
    sRequest.fullPocketMask = gE2ETestRequest.fullPocketMask;
    memcpy(sRequest.reserved, (const void *)gE2ETestRequest.reserved, sizeof(sRequest.reserved));
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

static void RejectPendingRequestWhileBusy(void)
{
    gE2ETestResult.requestId = gE2ETestRequest.requestId;
    gE2ETestResult.mapGroup = sMapGroup;
    gE2ETestResult.mapNum = sMapNum;
    gE2ETestResult.x = sX;
    gE2ETestResult.y = sY;
    gE2ETestResult.error = E2E_TEST_ERROR_BUSY;
    gE2ETestResult.phase = E2E_TEST_ARRANGE_PHASE_VALIDATE;
    gE2ETestResult.status = E2E_TEST_STATUS_ERROR;
    gE2ETestRequest.status = E2E_TEST_STATUS_ERROR;
    // The TypeScript mailbox polls every two frames. Keep this result visible
    // without abandoning the command that still owns sStage.
    sRejectedResultFrames = 2;
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

static enum E2ETestError ValidateMonFixture(const struct E2ETestMonFixture *mon, bool32 allowEmpty)
{
    u32 move;

    if (mon->species == SPECIES_NONE)
    {
        if (allowEmpty
         && mon->level == 0
         && mon->isEgg == FALSE
         && mon->reserved == 0)
        {
            for (move = 0; move < MAX_MON_MOVES; move++)
            {
                if (mon->moves[move] != MOVE_NONE)
                    return E2E_TEST_ERROR_MOVE;
            }
            return E2E_TEST_ERROR_NONE;
        }
        return E2E_TEST_ERROR_SPECIES;
    }
    if (mon->species >= NUM_SPECIES || !IsSpeciesEnabled(mon->species))
        return E2E_TEST_ERROR_SPECIES;
    if (mon->level == 0 || mon->level > MAX_LEVEL)
        return E2E_TEST_ERROR_LEVEL;
    if (mon->isEgg > TRUE || mon->reserved != 0)
        return E2E_TEST_ERROR_PARTY;
    for (move = 0; move < MAX_MON_MOVES; move++)
    {
        if (mon->moves[move] >= MOVES_COUNT)
            return E2E_TEST_ERROR_MOVE;
    }
    return E2E_TEST_ERROR_NONE;
}

static enum E2ETestError ValidateArrangeRequest(void)
{
    const struct MapHeader *mapHeader;
    u32 i;

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
        enum E2ETestError error = ValidateMonFixture(&sRequest.party[i].mon, FALSE);

        if (error != E2E_TEST_ERROR_NONE)
            return error;
        if (sRequest.party[i].fainted > TRUE
         || sRequest.party[i].reserved[0] != 0
         || sRequest.party[i].reserved[1] != 0
         || sRequest.party[i].reserved[2] != 0)
            return E2E_TEST_ERROR_PARTY;
    }
    if (sRequest.bagItemCount > E2E_TEST_MAX_BAG_ITEMS)
        return E2E_TEST_ERROR_BAG_ITEM_COUNT;
    for (i = 0; i < sRequest.bagItemCount; i++)
    {
        if (sRequest.bagItems[i].item == ITEM_NONE
         || sRequest.bagItems[i].item >= ITEMS_COUNT
         || GetItemPocket(sRequest.bagItems[i].item) >= POCKETS_COUNT)
            return E2E_TEST_ERROR_BAG_ITEM;
        if (sRequest.bagItems[i].quantity == 0
         || sRequest.bagItems[i].quantity > MAX_BAG_ITEM_CAPACITY)
            return E2E_TEST_ERROR_ITEM_QUANTITY;
    }
    if (sRequest.pcSlotCount > E2E_TEST_MAX_PC_SLOTS)
        return E2E_TEST_ERROR_PC_SLOT_COUNT;
    if (sRequest.currentBox >= TOTAL_BOXES_COUNT)
        return E2E_TEST_ERROR_PC_BOX;
    for (i = 0; i < sRequest.pcSlotCount; i++)
    {
        enum E2ETestError error;
        u32 j;

        if (sRequest.pcSlots[i].boxId >= TOTAL_BOXES_COUNT)
            return E2E_TEST_ERROR_PC_BOX;
        if (sRequest.pcSlots[i].boxPosition >= IN_BOX_COUNT)
            return E2E_TEST_ERROR_PC_SLOT;
        if (sRequest.pcSlots[i].reserved[0] != 0 || sRequest.pcSlots[i].reserved[1] != 0)
            return E2E_TEST_ERROR_PC_SLOT;
        for (j = 0; j < i; j++)
        {
            if (sRequest.pcSlots[i].boxId == sRequest.pcSlots[j].boxId
             && sRequest.pcSlots[i].boxPosition == sRequest.pcSlots[j].boxPosition)
                return E2E_TEST_ERROR_PC_SLOT;
        }
        error = ValidateMonFixture(&sRequest.pcSlots[i].mon, TRUE);
        if (error != E2E_TEST_ERROR_NONE)
            return error;
    }
    if (sRequest.hmsOverwrite > TRUE)
        return E2E_TEST_ERROR_PARTY;
    if (sRequest.fullPocketMask & ~E2E_TEST_FULL_POCKET_MASK)
        return E2E_TEST_ERROR_FULL_POCKET_MASK;
    if (sRequest.reserved[0] != 0
     || sRequest.reserved[1] != 0)
        return E2E_TEST_ERROR_PARTY;

    return E2E_TEST_ERROR_NONE;
}

static enum E2ETestError ValidateRequest(void)
{
    switch (sRequest.command)
    {
    case E2E_TEST_COMMAND_ARRANGE:
        return ValidateArrangeRequest();
    case E2E_TEST_COMMAND_START_WILD_BATTLE:
        if (ValidateMonFixture(&sRequest.wildMon, FALSE) != E2E_TEST_ERROR_NONE)
            return ValidateMonFixture(&sRequest.wildMon, FALSE);
        if (sRequest.wildMon.isEgg)
            return E2E_TEST_ERROR_PARTY;
        return E2E_TEST_ERROR_NONE;
    case E2E_TEST_COMMAND_SAVE:
    case E2E_TEST_COMMAND_OBSERVE_REGION_MAP:
    case E2E_TEST_COMMAND_OBSERVE_REGION_MAP_SECTION:
    case E2E_TEST_COMMAND_WIN_BATTLE:
        return E2E_TEST_ERROR_NONE;
    default:
        return E2E_TEST_ERROR_COMMAND;
    }
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
    sObservedBagItemCount = sRequest.bagItemCount;
    memcpy(sObservedBagItems, sRequest.bagItems, sizeof(sObservedBagItems));
    ApplyPcFixtures();
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

static bool32 IsStorageStateMachineActive(void)
{
    u8 uiState;
    u8 mode;
    u8 cursorArea;
    u8 cursorPosition;
    bool8 movingMon;

    E2ETest_GetStorageUiState(&uiState, &mode, &cursorArea, &cursorPosition, &movingMon);
    return uiState != E2E_TEST_STORAGE_UI_NONE;
}

static void StartWildBattle(void)
{
    u32 move;

    CreateWildMon(sRequest.wildMon.species, sRequest.wildMon.level);
    for (move = 0; move < MAX_MON_MOVES; move++)
        SetMonMoveSlot(&gEnemyParty[0], sRequest.wildMon.moves[move], move);
    ResetObservations();
    gE2ETestRequest.status = E2E_TEST_STATUS_RUNNING;
    PublishResult(E2E_TEST_STATUS_RUNNING, E2E_TEST_ARRANGE_PHASE_FIELD_READY, E2E_TEST_ERROR_NONE);
    BattleSetup_StartWildBattle();
    sStage = E2E_TEST_STAGE_WAIT_BATTLE;
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

    if (sRequest.command == E2E_TEST_COMMAND_WIN_BATTLE)
    {
        if (!gMain.inBattle)
        {
            FailRequest(E2E_TEST_ERROR_BUSY);
            return;
        }

        BattleDebug_WonBattle();
        gE2ETestRequest.status = E2E_TEST_STATUS_SUCCESS;
        PublishResult(E2E_TEST_STATUS_SUCCESS, E2E_TEST_ARRANGE_PHASE_STATE, E2E_TEST_ERROR_NONE);
        return;
    }

    if (gMain.inBattle || IsStorageStateMachineActive())
    {
        FailRequest(E2E_TEST_ERROR_BUSY);
        return;
    }

    if (sRequest.command == E2E_TEST_COMMAND_START_WILD_BATTLE)
    {
        if (!IsSettledOverworld())
        {
            FailRequest(E2E_TEST_ERROR_BUSY);
            return;
        }
        sMapGroup = gSaveBlock1Ptr->location.mapGroup;
        sMapNum = gSaveBlock1Ptr->location.mapNum;
        sX = gSaveBlock1Ptr->pos.x;
        sY = gSaveBlock1Ptr->pos.y;
        StartWildBattle();
        return;
    }

    if (sRequest.command == E2E_TEST_COMMAND_SAVE)
    {
        u8 saveStatus;

        if (!IsSettledOverworld())
        {
            FailRequest(E2E_TEST_ERROR_BUSY);
            return;
        }
        gE2ETestRequest.status = E2E_TEST_STATUS_RUNNING;
        PublishResult(E2E_TEST_STATUS_RUNNING, E2E_TEST_ARRANGE_PHASE_STATE, E2E_TEST_ERROR_NONE);
        if (gDifferentSaveFile == TRUE)
        {
            saveStatus = TrySavingData(SAVE_OVERWRITE_DIFFERENT_FILE);
            gDifferentSaveFile = FALSE;
        }
        else
        {
            saveStatus = TrySavingData(SAVE_NORMAL);
        }
        if (saveStatus != SAVE_STATUS_OK)
        {
            FailRequest(E2E_TEST_ERROR_SAVE);
            return;
        }
        gE2ETestRequest.status = E2E_TEST_STATUS_SUCCESS;
        PublishResult(E2E_TEST_STATUS_SUCCESS, E2E_TEST_ARRANGE_PHASE_STATE, E2E_TEST_ERROR_NONE);
        return;
    }

    if (sRequest.command == E2E_TEST_COMMAND_OBSERVE_REGION_MAP)
    {
        if (!IsSettledOverworld())
        {
            FailRequest(E2E_TEST_ERROR_BUSY);
            return;
        }

        // Exercise the same initialization entry point used by the Pokedex
        // area screen. The retained test-only storage keeps region_map.c's
        // internal pointer valid until the next real region-map UI replaces it.
        memset(&sObservedPokedexRegionMap, 0, sizeof(sObservedPokedexRegionMap));
        ShowRegionMapForPokedexAreaScreen(&sObservedPokedexRegionMap);
        sMapGroup = GetRegionMapType(gMapHeader.regionMapSectionId);
        sMapNum = sObservedPokedexRegionMap.mapSecId;
        sX = sObservedPokedexRegionMap.cursorPosX;
        sY = sObservedPokedexRegionMap.cursorPosY;
        gE2ETestRequest.status = E2E_TEST_STATUS_SUCCESS;
        PublishResult(E2E_TEST_STATUS_SUCCESS, E2E_TEST_ARRANGE_PHASE_STATE, E2E_TEST_ERROR_NONE);
        return;
    }

    if (sRequest.command == E2E_TEST_COMMAND_OBSERVE_REGION_MAP_SECTION)
    {
        if (!IsSettledOverworld())
        {
            FailRequest(E2E_TEST_ERROR_BUSY);
            return;
        }

        sMapGroup = GetRegionMapType(gMapHeader.regionMapSectionId);
        sMapNum = GetRegionMapSecIdAt(sRequest.x, sRequest.y);
        sX = sRequest.x;
        sY = sRequest.y;
        gE2ETestRequest.status = E2E_TEST_STATUS_SUCCESS;
        PublishResult(E2E_TEST_STATUS_SUCCESS, E2E_TEST_ARRANGE_PHASE_STATE, E2E_TEST_ERROR_NONE);
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
    if (sStage != E2E_TEST_STAGE_IDLE && gE2ETestRequest.status == E2E_TEST_STATUS_PENDING)
    {
        RejectPendingRequestWhileBusy();
        return;
    }
    if (sRejectedResultFrames != 0)
    {
        sRejectedResultFrames--;
        return;
    }

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
    case E2E_TEST_STAGE_WAIT_BATTLE:
    {
        u8 cursor;

        if (!gMain.inBattle
         || (!E2ETest_IsBattleTextReady() && !E2ETest_GetBattleActionMenuState(&cursor)))
            break;
        sStage = E2E_TEST_STAGE_IDLE;
        gE2ETestRequest.status = E2E_TEST_STATUS_SUCCESS;
        PublishResult(E2E_TEST_STATUS_SUCCESS, E2E_TEST_ARRANGE_PHASE_FIELD_READY, E2E_TEST_ERROR_NONE);
        break;
    }
    }
}

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
    u8 storageCursorArea;
    u8 storageCursorPosition;
    u8 storageUiState;
    u8 storageMode;
    bool8 storageMovingMon;

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
    gE2ETestState.battleEnemySpecies = SPECIES_NONE;
    gE2ETestState.caughtSpecies = sCaughtSpecies;
    gE2ETestState.lastUsedItem = gLastUsedItem;
    gE2ETestState.battleEnemyLevel = 0;
    gE2ETestState.battleActive = gMain.inBattle;
    gE2ETestState.catchSwapState = sCatchSwapState;
    gE2ETestState.catchSwapCursor = sCatchSwapCursor;
    gE2ETestState.catchSwapSelectedParty = sCatchSwapSelectedParty;
    gE2ETestState.catchSwapBox = sCatchSwapBox;
    gE2ETestState.catchSwapSlot = sCatchSwapSlot;
    gE2ETestState.storageUiState = E2E_TEST_STORAGE_UI_NONE;
    gE2ETestState.storageOpen = FALSE;
    gE2ETestState.storageReady = FALSE;
    gE2ETestState.storageCursorArea = 0xFF;
    gE2ETestState.storageCursorPosition = 0xFF;
    gE2ETestState.storageMovingMon = FALSE;
    gE2ETestState.storageCurrentBox = TOTAL_BOXES_COUNT;
    gE2ETestState.pcSlotCount = sObservedPcSlotCount;
    gE2ETestState.battleUiState = gMain.inBattle ? E2E_TEST_BATTLE_UI_OTHER : E2E_TEST_BATTLE_UI_NONE;
    gE2ETestState.battleCursor = 0xFF;
    gE2ETestState.battleBagPocket = 0xFF;
    gE2ETestState.battleBagItem = ITEM_NONE;
    gE2ETestState.storageMode = E2E_TEST_STORAGE_MODE_NONE;
    memset((void *)gE2ETestState.reserved2, 0, sizeof(gE2ETestState.reserved2));
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
        gE2ETestState.bagItemIds[i] = ITEM_NONE;
        gE2ETestState.partyMenuActions[i] = 0xFF;
    }
    for (i = 0; i < E2E_TEST_MAX_PC_SLOTS; i++)
    {
        u32 move;

        gE2ETestState.pcSlots[i].species = SPECIES_NONE;
        for (move = 0; move < MAX_MON_MOVES; move++)
            gE2ETestState.pcSlots[i].moves[move] = MOVE_NONE;
        gE2ETestState.pcSlots[i].level = 0;
        gE2ETestState.pcSlots[i].isEgg = FALSE;
        gE2ETestState.pcSlots[i].boxId = TOTAL_BOXES_COUNT;
        gE2ETestState.pcSlots[i].boxPosition = IN_BOX_COUNT;
        gE2ETestState.pcSlots[i].reserved = 0;
    }
    for (i = 0; i < MAX_MON_MOVES; i++)
        gE2ETestState.battleEnemyMoves[i] = MOVE_NONE;

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
    for (i = 0; i < sObservedBagItemCount; i++)
    {
        gE2ETestState.bagItemIds[i] = sObservedBagItems[i].item;
        gE2ETestState.bagItemCounts[i] = CountTotalItemQuantityInBag(sObservedBagItems[i].item);
    }
    for (i = 0; i < sObservedPcSlotCount; i++)
    {
        u32 move;
        struct BoxPokemon *mon = GetBoxedMonPtr(sObservedPcSlots[i].boxId, sObservedPcSlots[i].boxPosition);

        gE2ETestState.pcSlots[i].species = GetBoxMonData(mon, MON_DATA_SPECIES);
        for (move = 0; move < MAX_MON_MOVES; move++)
            gE2ETestState.pcSlots[i].moves[move] = GetBoxMonData(mon, MON_DATA_MOVE1 + move);
        gE2ETestState.pcSlots[i].level = GetLevelFromBoxMonExp(mon);
        gE2ETestState.pcSlots[i].isEgg = GetBoxMonData(mon, MON_DATA_IS_EGG);
        gE2ETestState.pcSlots[i].boxId = sObservedPcSlots[i].boxId;
        gE2ETestState.pcSlots[i].boxPosition = sObservedPcSlots[i].boxPosition;
        gE2ETestState.pcSlots[i].reserved = 0;
    }
    gE2ETestState.storageCurrentBox = StorageGetCurrentBox();
    E2ETest_GetStorageUiState(&storageUiState, &storageMode, &storageCursorArea, &storageCursorPosition, &storageMovingMon);
    gE2ETestState.storageUiState = storageUiState;
    gE2ETestState.storageMode = storageMode;
    gE2ETestState.storageOpen = storageUiState != E2E_TEST_STORAGE_UI_NONE;
    gE2ETestState.storageReady = storageUiState == E2E_TEST_STORAGE_UI_READY
                              || storageUiState == E2E_TEST_STORAGE_UI_PC_MENU;
    gE2ETestState.storageCursorArea = storageCursorArea;
    gE2ETestState.storageCursorPosition = storageCursorPosition;
    gE2ETestState.storageMovingMon = storageMovingMon;
    if (gMain.inBattle)
    {
        u8 cursor;
        u8 bagUiState;
        u8 pocket;
        u16 item;

        gE2ETestState.battleEnemySpecies = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES);
        gE2ETestState.battleEnemyLevel = GetMonData(&gEnemyParty[0], MON_DATA_LEVEL);
        for (i = 0; i < MAX_MON_MOVES; i++)
            gE2ETestState.battleEnemyMoves[i] = GetMonData(&gEnemyParty[0], MON_DATA_MOVE1 + i);
        if (E2ETest_GetBattleActionMenuState(&cursor))
        {
            gE2ETestState.battleUiState = E2E_TEST_BATTLE_UI_ACTION_MENU;
            gE2ETestState.battleCursor = cursor;
        }
        else if (E2ETest_GetBattleBagState(&bagUiState, &pocket, &item))
        {
            gE2ETestState.battleUiState = bagUiState;
            gE2ETestState.battleBagPocket = pocket;
            gE2ETestState.battleBagItem = item;
        }
        else if (E2ETest_IsCaughtDexReady())
            gE2ETestState.battleUiState = E2E_TEST_BATTLE_UI_CAUGHT_DEX;
        else if (sNicknamePrompt)
        {
            gE2ETestState.battleUiState = E2E_TEST_BATTLE_UI_NICKNAME;
            gE2ETestState.battleCursor = sNicknameCursor;
        }
        else if (sCatchSwapState == E2E_TEST_CATCH_SWAP_PROMPT)
        {
            gE2ETestState.battleUiState = E2E_TEST_BATTLE_UI_CATCH_SWAP_PROMPT;
            gE2ETestState.battleCursor = sCatchSwapCursor;
        }
        else if (E2ETest_GetCatchSwapPartyState(&cursor))
        {
            gE2ETestState.battleUiState = E2E_TEST_BATTLE_UI_CATCH_SWAP_PARTY;
            gE2ETestState.battleCursor = cursor;
        }
        else if (E2ETest_IsBattleTextReady())
            gE2ETestState.battleUiState = E2E_TEST_BATTLE_UI_TEXT;
    }
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
    if (gE2ETestState.storageOpen)
        gE2ETestState.uiMode = E2E_TEST_UI_STORAGE;
    else if (gE2ETestState.battleUiState == E2E_TEST_BATTLE_UI_CATCH_SWAP_PROMPT
          || gE2ETestState.battleUiState == E2E_TEST_BATTLE_UI_CATCH_SWAP_PARTY)
        gE2ETestState.uiMode = E2E_TEST_UI_CATCH_SWAP;
    else if (gMain.inBattle)
        gE2ETestState.uiMode = E2E_TEST_UI_BATTLE;

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
