#include "global.h"

#if WAYFARER_MULTIPLAYER_POC

#include "battle_pyramid.h"
#include "event_data.h"
#include "fieldmap.h"
#include "item.h"
#include "load_save.h"
#include "main.h"
#include "multiplayer_poc_reward.h"
#include "new_game.h"
#include "overworld.h"
#include "save.h"
#include "constants/battle_pyramid.h"
#include "constants/flags.h"
#include "constants/items.h"

#define REWARD_LEDGER_SCHEMA 0xA1

EWRAM_DATA volatile struct MultiplayerPocRewardDiag gMultiplayerPocRewardDiag;

// AddBagItem mutates encrypted slots in SaveBlock1. Keep their exact bytes so
// a failed full save can roll back both the local bag and the local claim bit.
static EWRAM_DATA struct Bag sBagBeforeClaim;

STATIC_ASSERT(sizeof(struct MultiplayerPocRewardDiag) == 56, MultiplayerPocRewardDiagSize);
STATIC_ASSERT(sizeof(((struct SaveBlock1 *)0)->unused_9C6) == 2, MultiplayerPocRewardLedgerSize);

static enum MultiplayerPocRewardState ReadLedger(void)
{
    const u8 *ledger;

    if (gSaveBlock1Ptr == NULL)
        return MULTIPLAYER_POC_REWARD_INVALID;

    ledger = gSaveBlock1Ptr->unused_9C6;
    if ((ledger[0] == 0 && ledger[1] == 0)
     || (ledger[0] == 0xFF && ledger[1] == 0xFF))
        return MULTIPLAYER_POC_REWARD_NONE;
    if (ledger[0] != REWARD_LEDGER_SCHEMA || ledger[1] > MULTIPLAYER_POC_REWARD_CLAIMED)
        return MULTIPLAYER_POC_REWARD_INVALID;
    return ledger[1];
}

static void WriteLedger(enum MultiplayerPocRewardState state)
{
    gSaveBlock1Ptr->unused_9C6[0] = REWARD_LEDGER_SCHEMA;
    gSaveBlock1Ptr->unused_9C6[1] = state;
}

void MultiplayerPocReward_UpdateDiag(void)
{
    volatile struct MultiplayerPocRewardDiag *diag = &gMultiplayerPocRewardDiag;

    if (diag->magic != MULTIPLAYER_POC_REWARD_DIAG_MAGIC)
    {
        diag->magic = MULTIPLAYER_POC_REWARD_DIAG_MAGIC;
        diag->version = MULTIPLAYER_POC_REWARD_DIAG_VERSION;
        diag->size = sizeof(*diag);
        diag->encounterId = MULTIPLAYER_POC_REWARD_ENCOUNTER_ID;
        diag->savePhase = MULTIPLAYER_POC_REWARD_SAVE_IDLE;
    }

    diag->ledgerState = ReadLedger();
    if (gSaveBlock2Ptr != NULL && gBagPockets[POCKET_MEDICINE].itemSlots != NULL)
        diag->bagPotionCount = CountTotalItemQuantityInBag(ITEM_POTION);
    else
        diag->bagPotionCount = 0;
}

enum MultiplayerPocRewardState MultiplayerPocReward_GetState(void)
{
    MultiplayerPocReward_UpdateDiag();
    return gMultiplayerPocRewardDiag.ledgerState;
}

static enum MultiplayerPocRewardResult SetResult(enum MultiplayerPocRewardResult result)
{
    gMultiplayerPocRewardDiag.operationResult = result;
    MultiplayerPocReward_UpdateDiag();
    return result;
}

static enum MultiplayerPocRewardResult CheckSavePreconditions(void)
{
    if (gMain.callback2 != CB2_Overworld
     || gBackupMapLayout.map == NULL
     || gSaveBlock1Ptr->pos.x < 0 || gSaveBlock1Ptr->pos.y < 0
     || gSaveBlock1Ptr->pos.x + MAP_OFFSET_W > gBackupMapLayout.width
     || gSaveBlock1Ptr->pos.y + MAP_OFFSET_H > gBackupMapLayout.height)
        return MULTIPLAYER_POC_REWARD_RESULT_NOT_IN_FIELD;
    // This experiment requires a prior ordinary save. A first save may need
    // SAVE_OVERWRITE_DIFFERENT_FILE, and cannot be a torn-write baseline.
    if (gDifferentSaveFile || !gFlashMemoryPresent || gSaveCounter == 0)
        return MULTIPLAYER_POC_REWARD_RESULT_NO_BASELINE;
    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE
     || FlagGet(FLAG_STORING_ITEMS_IN_PYRAMID_BAG))
        return MULTIPLAYER_POC_REWARD_RESULT_WRONG_BAG;
    return MULTIPLAYER_POC_REWARD_RESULT_NONE;
}

static bool8 SaveReward(void)
{
    u8 status;

    gMultiplayerPocRewardDiag.savePhase = MULTIPLAYER_POC_REWARD_SAVE_PREPARING;
    SaveMapView();
    gMultiplayerPocRewardDiag.savePhase = MULTIPLAYER_POC_REWARD_SAVE_WRITING;
    gMultiplayerPocRewardDiag.saveAttempts++;
    status = TrySavingData(SAVE_NORMAL);
    gMultiplayerPocRewardDiag.saveResult = status;
    gMultiplayerPocRewardDiag.savePhase = status == SAVE_STATUS_OK
        ? MULTIPLAYER_POC_REWARD_SAVE_SUCCEEDED
        : MULTIPLAYER_POC_REWARD_SAVE_FAILED;
    return status == SAVE_STATUS_OK;
}

static enum MultiplayerPocRewardResult SavePending(void)
{
    WriteLedger(MULTIPLAYER_POC_REWARD_PENDING);
    if (!SaveReward())
        return SetResult(MULTIPLAYER_POC_REWARD_RESULT_SAVE_FAILED);
    return SetResult(MULTIPLAYER_POC_REWARD_RESULT_PENDING);
}

static enum MultiplayerPocRewardResult Claim(void)
{
    // The single local SaveBlock1 image carries both the encrypted bag slots
    // and CLAIMED. A valid old slot has neither; a valid new slot has both.
    CpuCopy16(&gSaveBlock1Ptr->bag, &sBagBeforeClaim, sizeof(sBagBeforeClaim));
    gMultiplayerPocRewardDiag.grantAttempts++;
    if (!AddBagItem(ITEM_POTION, 1))
        return SetResult(MULTIPLAYER_POC_REWARD_RESULT_BAG_ADD_FAILED);

    WriteLedger(MULTIPLAYER_POC_REWARD_CLAIMED);
    if (!SaveReward())
    {
        CpuCopy16(&sBagBeforeClaim, &gSaveBlock1Ptr->bag, sizeof(sBagBeforeClaim));
        WriteLedger(MULTIPLAYER_POC_REWARD_PENDING);
        return SetResult(MULTIPLAYER_POC_REWARD_RESULT_SAVE_FAILED);
    }

    gMultiplayerPocRewardDiag.grantsCommitted++;
    return SetResult(MULTIPLAYER_POC_REWARD_RESULT_CLAIMED);
}

enum MultiplayerPocRewardResult MultiplayerPocReward_OnAgreedWin(u16 encounterId)
{
    enum MultiplayerPocRewardState state;
    enum MultiplayerPocRewardResult precondition;

    MultiplayerPocReward_UpdateDiag();
    if (encounterId != MULTIPLAYER_POC_REWARD_ENCOUNTER_ID)
        return SetResult(MULTIPLAYER_POC_REWARD_RESULT_WRONG_ENCOUNTER);
    state = ReadLedger();
    if (state == MULTIPLAYER_POC_REWARD_CLAIMED)
    {
        gMultiplayerPocRewardDiag.duplicateSuppressions++;
        return SetResult(MULTIPLAYER_POC_REWARD_RESULT_ALREADY_CLAIMED);
    }
    if (state == MULTIPLAYER_POC_REWARD_INVALID)
        return SetResult(MULTIPLAYER_POC_REWARD_RESULT_INVALID_LEDGER);
    precondition = CheckSavePreconditions();
    if (precondition != MULTIPLAYER_POC_REWARD_RESULT_NONE)
        return SetResult(precondition);

    if (state == MULTIPLAYER_POC_REWARD_PENDING)
        gMultiplayerPocRewardDiag.retryCount++;
    if (!CheckBagHasSpace(ITEM_POTION, 1))
        return SavePending();
    return Claim();
}

enum MultiplayerPocRewardResult MultiplayerPocReward_TryClaimPending(void)
{
    enum MultiplayerPocRewardResult precondition;

    MultiplayerPocReward_UpdateDiag();
    if (ReadLedger() != MULTIPLAYER_POC_REWARD_PENDING)
        return SetResult(MULTIPLAYER_POC_REWARD_RESULT_NO_PENDING);
    precondition = CheckSavePreconditions();
    if (precondition != MULTIPLAYER_POC_REWARD_RESULT_NONE)
        return SetResult(precondition);

    gMultiplayerPocRewardDiag.retryCount++;
    if (!CheckBagHasSpace(ITEM_POTION, 1))
        return SavePending();
    return Claim();
}

#endif // WAYFARER_MULTIPLAYER_POC
