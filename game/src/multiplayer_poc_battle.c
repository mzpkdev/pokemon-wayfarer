#include "global.h"

#if WAYFARER_MULTIPLAYER_POC

#include "battle.h"
#include "battle_gfx_sfx_util.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_util2.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "link.h"
#include "main.h"
#include "multiplayer_poc.h"
#include "multiplayer_poc_battle.h"
#include "multiplayer_poc_reward.h"
#include "overworld.h"
#include "palette.h"
#include "pokemon.h"
#include "script.h"
#include "task.h"
#include "constants/battle.h"
#include "constants/moves.h"
#include "constants/species.h"

#define POC_BATTLE_WORD_BASE 0xB000
#define POC_BATTLE_WORD_TOKEN_MASK 0x0FF0
#define POC_BATTLE_CONSENT_TIMEOUT 1800
#define POC_BATTLE_RESULT_TIMEOUT 1800
#define POC_BATTLE_GO_HOLD_FRAMES 8
#define POC_BATTLE_RESULT_HOLD_FRAMES 30
#define POC_BATTLE_CANCEL_ACK_HOLD_FRAMES 64
#define POC_BATTLE_CANCEL_TIMEOUT 180

enum PocBattleWord
{
    POC_WORD_IDLE,
    POC_WORD_OFFER,
    POC_WORD_READY,
    POC_WORD_START,
    POC_WORD_START_ACK,
    POC_WORD_FINAL_GO,
    POC_WORD_RESULT_WIN,
    POC_WORD_RESULT_OTHER,
    POC_WORD_RESULT_ACK_WIN,
    POC_WORD_RESULT_ACK_OTHER,
    POC_WORD_DECLINE,
    POC_WORD_CANCEL,
    POC_WORD_CANCEL_ACK,
};

EWRAM_DATA volatile struct MultiplayerPocBattleDiag gMultiplayerPocBattleDiag;

static EWRAM_DATA struct Pokemon sOriginalParty[PARTY_SIZE];
static EWRAM_DATA u8 sOriginalPartyCount;
static EWRAM_DATA u8 sOriginalFrontierLevel;
static EWRAM_DATA bool8 sOriginalDisableRecordBattle;
static EWRAM_DATA bool8 sPartySaved;
static EWRAM_DATA bool8 sInitialized;
static EWRAM_DATA bool8 sPeerResultSeen;
static EWRAM_DATA bool8 sPeerAckSeen;
static EWRAM_DATA bool8 sRewardQueued;
static EWRAM_DATA bool8 sLocalConsent;
static EWRAM_DATA bool8 sPeerConsent;
static EWRAM_DATA bool8 sPeerReadySeen;
static EWRAM_DATA bool8 sCancelAckSeen;
static EWRAM_DATA bool8 sRejectedTokenValid;
static EWRAM_DATA bool8 sOfferBarrier;
static EWRAM_DATA bool8 sBattleDetached;
static EWRAM_DATA u8 sCancelWord;
static EWRAM_DATA u8 sNextInviteSequence;
static EWRAM_DATA u16 sToken;
static EWRAM_DATA u16 sRejectedToken;
static EWRAM_DATA u32 sStateStartFrame;
static EWRAM_DATA u32 sGoFrames;
static EWRAM_DATA u32 sResultAckFrame;

STATIC_ASSERT(sizeof(struct MultiplayerPocBattleDiag) == 24 * sizeof(u32), MultiplayerPocBattleDiagSize);

static void CB2_PocBattleReturn(void);

static void InitIfNeeded(void)
{
    if (sInitialized)
        return;
    gMultiplayerPocBattleDiag.magic = MULTIPLAYER_POC_BATTLE_DIAG_MAGIC;
    gMultiplayerPocBattleDiag.version = 1;
    gMultiplayerPocBattleDiag.size = sizeof(struct MultiplayerPocBattleDiag);
    gMultiplayerPocBattleDiag.encounterId = MULTIPLAYER_POC_BATTLE_ENCOUNTER_ID;
    gMultiplayerPocBattleDiag.localPlayerId = 0xFFFFFFFF;
    gMultiplayerPocBattleDiag.localBattlerId = 0xFFFFFFFF;
    gMultiplayerPocBattleDiag.peerOutcome = 0xFFFFFFFF;
    sInitialized = TRUE;
}

static u32 PartyHash(const struct Pokemon *party, u8 count)
{
    const u8 *bytes = (const u8 *)party;
    u32 hash = 2166136261u;
    u32 i;

    for (i = 0; i < sizeof(struct Pokemon) * PARTY_SIZE; i++)
        hash = (hash ^ bytes[i]) * 16777619u;
    return (hash ^ count) * 16777619u;
}

static bool8 CanStayInvited(void)
{
    return gMultiplayerPocDiag.state == MULTIPLAYER_POC_ACTIVE
        && gMultiplayerPocDiag.localId < 2
        && gMultiplayerPocDiag.peerId < 2
        && gMultiplayerPocDiag.localMap == gMultiplayerPocDiag.peerMap;
}

static bool8 CanCommitBattle(void)
{
    return CanStayInvited()
        && gMultiplayerPocDiag.peerVisible
        && gMain.callback2 == CB2_Overworld
        && !gPaletteFade.active
        && !ArePlayerFieldControlsLocked()
        && IsPlayerStandingStill();
}

static bool8 IsIdleForInvite(void)
{
    return gMultiplayerPocBattleDiag.state == POC_BATTLE_IDLE
        || gMultiplayerPocBattleDiag.state == POC_BATTLE_COMPLETE
        || gMultiplayerPocBattleDiag.state == POC_BATTLE_ABORTED;
}

static void BeginInvite(u16 token, bool8 localConsent)
{
    gMultiplayerPocBattleDiag.state = POC_BATTLE_OFFER;
    gMultiplayerPocBattleDiag.error = POC_BATTLE_ERROR_NONE;
    gMultiplayerPocBattleDiag.localPlayerId = gMultiplayerPocDiag.localId;
    gMultiplayerPocBattleDiag.localBattlerId = gMultiplayerPocDiag.localId ? 2 : 0;
    gMultiplayerPocBattleDiag.peerWord = 0;
    gMultiplayerPocBattleDiag.localMoveChoices = 0;
    gMultiplayerPocBattleDiag.localExecutedMoves = 0;
    gMultiplayerPocBattleDiag.localOutcome = 0;
    gMultiplayerPocBattleDiag.peerOutcome = 0xFFFFFFFF;
    gMultiplayerPocBattleDiag.resultAgreed = FALSE;
    gMultiplayerPocBattleDiag.rewardResult = 0;
    sPartySaved = FALSE;
    sPeerResultSeen = FALSE;
    sPeerAckSeen = FALSE;
    sRewardQueued = FALSE;
    sLocalConsent = localConsent;
    sPeerConsent = !localConsent;
    sPeerReadySeen = FALSE;
    sToken = token;
    sBattleDetached = FALSE;
    sStateStartFrame = gMultiplayerPocBattleDiag.frame;
}

bool8 MultiplayerPocBattle_CanInvite(void)
{
    InitIfNeeded();
    return IsIdleForInvite() && !sOfferBarrier && CanStayInvited();
}

bool8 MultiplayerPocBattle_CanAccept(void)
{
    InitIfNeeded();
    return gMultiplayerPocBattleDiag.state == POC_BATTLE_OFFER
        && sPeerConsent && !sLocalConsent && CanStayInvited();
}

bool8 MultiplayerPocBattle_RequestInvite(void)
{
    u16 token;

    if (!MultiplayerPocBattle_CanInvite())
        return FALSE;
    sNextInviteSequence = (sNextInviteSequence + 1) & 0x7F;
    if (sNextInviteSequence == 0)
        sNextInviteSequence = 1;
    token = (gMultiplayerPocDiag.localId << 11) | (sNextInviteSequence << 4);
    BeginInvite(token, TRUE);
    return TRUE;
}

bool8 MultiplayerPocBattle_AcceptInvite(void)
{
    if (!MultiplayerPocBattle_CanAccept())
        return FALSE;
    sLocalConsent = TRUE;
    return TRUE;
}

static void BeginCancellation(u8 word, enum MultiplayerPocBattleError error)
{
    sRejectedToken = sToken;
    sRejectedTokenValid = TRUE;
    sOfferBarrier = TRUE;
    sCancelWord = word;
    sCancelAckSeen = FALSE;
    sStateStartFrame = gMultiplayerPocBattleDiag.frame;
    gMultiplayerPocBattleDiag.error = error;
    gMultiplayerPocBattleDiag.state = POC_BATTLE_CANCELING;
}

bool8 MultiplayerPocBattle_DeclineInvite(void)
{
    if (!MultiplayerPocBattle_CanAccept())
        return FALSE;
    BeginCancellation(POC_WORD_DECLINE, POC_BATTLE_ERROR_DECLINED);
    return TRUE;
}

bool8 MultiplayerPocBattle_CancelInvite(void)
{
    InitIfNeeded();
    if ((gMultiplayerPocBattleDiag.state != POC_BATTLE_OFFER
      && gMultiplayerPocBattleDiag.state != POC_BATTLE_READY)
     || !sLocalConsent)
        return FALSE;
    BeginCancellation(POC_WORD_CANCEL, POC_BATTLE_ERROR_CANCELED);
    return TRUE;
}

enum MultiplayerPocInviteStatus MultiplayerPocBattle_GetInviteStatus(void)
{
    InitIfNeeded();
    if (gMultiplayerPocBattleDiag.state == POC_BATTLE_OFFER)
    {
        if (!sLocalConsent)
            return POC_INVITE_INCOMING;
        return sPeerConsent ? POC_INVITE_STARTING : POC_INVITE_OUTGOING;
    }
    if (gMultiplayerPocBattleDiag.state >= POC_BATTLE_READY
     && gMultiplayerPocBattleDiag.state <= POC_BATTLE_GO)
        return POC_INVITE_STARTING;
    return POC_INVITE_NONE;
}

bool8 MultiplayerPocBattle_TryRequest(void)
{
    if (!JOY_NEW(START_BUTTON)
     || (gMain.heldKeys & (L_BUTTON | R_BUTTON)) != (L_BUTTON | R_BUTTON))
        return FALSE;
    if (MultiplayerPocBattle_CanAccept())
        return MultiplayerPocBattle_AcceptInvite();
    return MultiplayerPocBattle_RequestInvite();
}

static u8 GetPhaseForState(void)
{
    switch (gMultiplayerPocBattleDiag.state)
    {
    case POC_BATTLE_OFFER: return sLocalConsent ? POC_WORD_OFFER : POC_WORD_IDLE;
    case POC_BATTLE_READY: return POC_WORD_READY;
    case POC_BATTLE_START: return POC_WORD_START;
    case POC_BATTLE_ACK: return POC_WORD_START_ACK;
    case POC_BATTLE_GO: return POC_WORD_FINAL_GO;
    case POC_BATTLE_CANCELING: return sCancelWord;
    case POC_BATTLE_RESULT:
        if (sPeerResultSeen)
            return gMultiplayerPocBattleDiag.localOutcome == B_OUTCOME_WON
                ? POC_WORD_RESULT_ACK_WIN : POC_WORD_RESULT_ACK_OTHER;
        return gMultiplayerPocBattleDiag.localOutcome == B_OUTCOME_WON
            ? POC_WORD_RESULT_WIN : POC_WORD_RESULT_OTHER;
    default: return POC_WORD_IDLE;
    }
}

u16 MultiplayerPocBattle_GetTxWord(void)
{
    InitIfNeeded();
    return POC_BATTLE_WORD_BASE | sToken | GetPhaseForState();
}

void MultiplayerPocBattle_OnPeerWord(u16 word)
{
    u8 phase;
    u16 token;

    InitIfNeeded();
    if ((word & 0xF000) != POC_BATTLE_WORD_BASE)
        return;
    phase = word & 0xF;
    token = word & POC_BATTLE_WORD_TOKEN_MASK;
    gMultiplayerPocBattleDiag.peerWord = word;

    if (phase == POC_WORD_IDLE)
    {
        // After cancellation, wait for the peer's idle word before accepting
        // any new offer from packets queued in the old transaction.
        if (gMultiplayerPocBattleDiag.state == POC_BATTLE_ABORTED)
            sOfferBarrier = FALSE;
        return;
    }

    if (phase == POC_WORD_OFFER)
    {
        // The token's origin must be the cable peer, not stale local loopback.
        if ((token >> 11) != gMultiplayerPocDiag.peerId
         || sOfferBarrier
         || (sRejectedTokenValid && token == sRejectedToken))
            return;
        if (IsIdleForInvite())
        {
            if (CanStayInvited())
                BeginInvite(token, FALSE);
            return;
        }
        if (gMultiplayerPocBattleDiag.state == POC_BATTLE_OFFER)
        {
            if (token == sToken)
                sPeerConsent = TRUE;
            else if (sLocalConsent && gMultiplayerPocBattleDiag.localPlayerId == 0)
                sPeerConsent = TRUE; // Player 0's token wins simultaneous offers.
            else if (sLocalConsent && gMultiplayerPocBattleDiag.localPlayerId == 1)
            {
                sToken = token;
                sPeerConsent = TRUE;
            }
        }
        return;
    }

    if (token != sToken)
    {
        // In a simultaneous invitation, player 0 may have seen our OFFER and
        // moved to READY before player 1 receives player 0's OFFER. Its READY
        // carries the winning player-0 token and proves its explicit invite.
        if (phase == POC_WORD_READY
         && gMultiplayerPocBattleDiag.state == POC_BATTLE_OFFER
         && sLocalConsent
         && gMultiplayerPocBattleDiag.localPlayerId == 1
         && gMultiplayerPocDiag.peerId == 0
         && (token >> 11) == 0
         && !sOfferBarrier
         && (!sRejectedTokenValid || token != sRejectedToken)
         && CanStayInvited())
        {
            sToken = token;
            sPeerConsent = TRUE;
            sPeerReadySeen = TRUE;
        }
        return;
    }
    if (phase == POC_WORD_DECLINE || phase == POC_WORD_CANCEL)
    {
        if (gMultiplayerPocBattleDiag.state >= POC_BATTLE_OFFER
         && gMultiplayerPocBattleDiag.state <= POC_BATTLE_GO)
            BeginCancellation(POC_WORD_CANCEL_ACK,
                              phase == POC_WORD_DECLINE ? POC_BATTLE_ERROR_DECLINED : POC_BATTLE_ERROR_CANCELED);
        else if (gMultiplayerPocBattleDiag.state == POC_BATTLE_CANCELING)
            sCancelWord = POC_WORD_CANCEL_ACK;
        return;
    }
    if (gMultiplayerPocBattleDiag.state == POC_BATTLE_CANCELING)
    {
        if (phase == POC_WORD_CANCEL_ACK)
            sCancelAckSeen = TRUE;
        return;
    }

    switch (gMultiplayerPocBattleDiag.state)
    {
    case POC_BATTLE_OFFER:
        if (phase == POC_WORD_READY)
        {
            sPeerConsent = TRUE;
            sPeerReadySeen = TRUE;
        }
        break;
    case POC_BATTLE_READY:
        if (phase == POC_WORD_READY)
            sPeerReadySeen = TRUE;
        if (gMultiplayerPocBattleDiag.localPlayerId == 0 && sPeerReadySeen && CanCommitBattle())
            gMultiplayerPocBattleDiag.state = POC_BATTLE_START;
        else if (gMultiplayerPocBattleDiag.localPlayerId == 1 && phase == POC_WORD_START && CanCommitBattle())
            gMultiplayerPocBattleDiag.state = POC_BATTLE_ACK;
        break;
    case POC_BATTLE_START:
        if (phase == POC_WORD_START_ACK && CanCommitBattle())
        {
            gMultiplayerPocBattleDiag.state = POC_BATTLE_GO;
            sGoFrames = 0;
        }
        break;
    case POC_BATTLE_ACK:
        if (phase == POC_WORD_FINAL_GO && CanCommitBattle())
            gMultiplayerPocBattleDiag.state = POC_BATTLE_SETUP;
        break;
    case POC_BATTLE_RESULT:
        if (phase == POC_WORD_RESULT_WIN || phase == POC_WORD_RESULT_OTHER
         || phase == POC_WORD_RESULT_ACK_WIN || phase == POC_WORD_RESULT_ACK_OTHER)
        {
            u32 peerOutcome = (phase == POC_WORD_RESULT_WIN || phase == POC_WORD_RESULT_ACK_WIN)
                ? B_OUTCOME_WON : B_OUTCOME_LOST;
            if (sPeerResultSeen && gMultiplayerPocBattleDiag.peerOutcome != peerOutcome)
            {
                gMultiplayerPocBattleDiag.error = POC_BATTLE_ERROR_RESULT_MISMATCH;
                break;
            }
            gMultiplayerPocBattleDiag.peerOutcome = peerOutcome;
            sPeerResultSeen = TRUE;
            if (phase == POC_WORD_RESULT_ACK_WIN || phase == POC_WORD_RESULT_ACK_OTHER)
            {
                if (!sPeerAckSeen)
                {
                    sPeerAckSeen = TRUE;
                    sResultAckFrame = gMultiplayerPocBattleDiag.frame;
                }
            }
        }
        break;
    }
}

static void MakeFixtureMon(struct Pokemon *mon, u16 species, u8 level, u32 personality,
                           u16 move1, u16 move2)
{
    u8 iv = 20;
    u8 i;

    CreateMonWithIVs(mon, species, level, personality, OTID_STRUCT_PRESET(0x13572468), 20);
    // CreateMonWithIVs can be capped by a save's challenge settings.
    for (i = 0; i < NUM_STATS; i++)
        SetMonData(mon, MON_DATA_HP_IV + i, &iv);
    SetMonMoveSlot(mon, move1, 0);
    SetMonMoveSlot(mon, move2, 1);
    SetMonMoveSlot(mon, MOVE_NONE, 2);
    SetMonMoveSlot(mon, MOVE_NONE, 3);
    CalculateMonStats(mon);
}

static void SetupFixedFixture(void)
{
    ZeroPlayerPartyMons();
    // Three slots are needed for the stock Tower multi party exchange.
    MakeFixtureMon(&gPlayerParty[0], SPECIES_PIKACHU, 25, 0x13572469,
                   MOVE_THUNDER_SHOCK, MOVE_QUICK_ATTACK);
    MakeFixtureMon(&gPlayerParty[1], SPECIES_BULBASAUR, 25, 0x1357246A,
                   MOVE_VINE_WHIP, MOVE_TACKLE);
    MakeFixtureMon(&gPlayerParty[2], SPECIES_RATTATA, 25, 0x1357246B,
                   MOVE_QUICK_ATTACK, MOVE_TACKLE);
    gPlayerPartyCount = 3;

    ZeroEnemyPartyMons();
    MakeFixtureMon(&gEnemyParty[0], SPECIES_CATERPIE, 12, 0x24681357,
                   MOVE_TACKLE, MOVE_GROWL);
    MakeFixtureMon(&gEnemyParty[3], SPECIES_WEEDLE, 12, 0x24681358,
                   MOVE_TACKLE, MOVE_GROWL);
    gEnemyPartyCount = 2;
}

static void StartBattle(void)
{
    sOriginalPartyCount = gPlayerPartyCount;
    memcpy(sOriginalParty, gPlayerParty, sizeof(sOriginalParty));
    sOriginalFrontierLevel = gSaveBlock2Ptr->frontier.lvlMode;
    sOriginalDisableRecordBattle = gSaveBlock2Ptr->frontier.disableRecordBattle;
    sPartySaved = TRUE;
    gMultiplayerPocBattleDiag.beforePartyCount = sOriginalPartyCount;
    gMultiplayerPocBattleDiag.beforePartyHash = PartyHash(sOriginalParty, sOriginalPartyCount);

    SaveLinkPlayers(2);
    sBattleDetached = TRUE;
    MultiplayerPoc_BattleDetach();
    gLinkType = LINKTYPE_BATTLE;
    gSaveBlock2Ptr->frontier.lvlMode = FRONTIER_LVL_50;
    SetupFixedFixture();
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_BATTLE_TOWER
                     | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_LINK | BATTLE_TYPE_MULTI
                     | BATTLE_TYPE_TOWER_LINK_MULTI;
    TRAINER_BATTLE_PARAM.opponentA = 0;
    TRAINER_BATTLE_PARAM.opponentB = 1;
    gMain.savedCallback = CB2_PocBattleReturn;
    CleanupOverworldWindowsAndTilemaps();
    SetMainCallback2(CB2_InitBattle);
}

static void RestorePersonalState(void)
{
    if (!sPartySaved)
        return;
    memcpy(gPlayerParty, sOriginalParty, sizeof(sOriginalParty));
    gPlayerPartyCount = sOriginalPartyCount;
    gSaveBlock2Ptr->frontier.lvlMode = sOriginalFrontierLevel;
    gSaveBlock2Ptr->frontier.disableRecordBattle = sOriginalDisableRecordBattle;
    gMultiplayerPocBattleDiag.afterPartyCount = gPlayerPartyCount;
    gMultiplayerPocBattleDiag.afterPartyHash = PartyHash(gPlayerParty, gPlayerPartyCount);
    sPartySaved = FALSE;
}

static void ReturnToIndependentField(void)
{
    ClearLinkCallback();
    CloseLink();
    SetSuppressLinkErrorMessage(FALSE);
    gLinkType = 0;
    RestorePersonalState();
    gBattleTypeFlags = 0;
    gMain.inBattle = FALSE;
    gMain.callback1 = CB1_Overworld;
    ResetTasks();
    gFieldCallback = FieldCB_ReturnToFieldNoScriptCheckMusic;
    gMultiplayerPocBattleDiag.state = POC_BATTLE_RETURNING;
    SetMainCallback2(CB2_ReturnToField);
}

static void AbortBattle(enum MultiplayerPocBattleError error)
{
    gMultiplayerPocBattleDiag.error = error;
    gMultiplayerPocBattleDiag.resultAgreed = FALSE;
    sRewardQueued = FALSE;
    // The stock link error handler can arrive during party preview or battle.
    // The saved party is EWRAM, outside the heap reset on battle entry.
    FreeBattleResources();
    FreeBattleSpritesData();
    FreeMonSpritesGfx();
    ReturnToIndependentField();
}

void MultiplayerPocBattle_AbortFromLinkError(void)
{
    if (MultiplayerPocBattle_OwnsTransport())
        AbortBattle(POC_BATTLE_ERROR_LINK_LOST);
}

void MultiplayerPocBattle_OnPresenceLost(void)
{
    sOfferBarrier = FALSE;
    if (gMultiplayerPocBattleDiag.state >= POC_BATTLE_OFFER
     && gMultiplayerPocBattleDiag.state <= POC_BATTLE_GO)
    {
        sRejectedToken = sToken;
        sRejectedTokenValid = TRUE;
        gMultiplayerPocBattleDiag.state = POC_BATTLE_ABORTED;
        gMultiplayerPocBattleDiag.error = POC_BATTLE_ERROR_LINK_LOST;
    }
    else if (gMultiplayerPocBattleDiag.state == POC_BATTLE_CANCELING)
        gMultiplayerPocBattleDiag.state = POC_BATTLE_ABORTED;
}

bool8 MultiplayerPocBattle_OwnsTransport(void)
{
    return gMultiplayerPocBattleDiag.state >= POC_BATTLE_SETUP
        && gMultiplayerPocBattleDiag.state <= POC_BATTLE_RESULT;
}

bool8 MultiplayerPocBattle_IsResultExchange(void)
{
    return gMultiplayerPocBattleDiag.state == POC_BATTLE_RESULT;
}

bool8 MultiplayerPocBattle_IsExperimentActive(void)
{
    return gMultiplayerPocBattleDiag.state == POC_BATTLE_SETUP
        || gMultiplayerPocBattleDiag.state == POC_BATTLE_IN_PROGRESS;
}

void MultiplayerPocBattle_Update(void)
{
    InitIfNeeded();
    gMultiplayerPocBattleDiag.frame++;

    if (gMultiplayerPocBattleDiag.state == POC_BATTLE_OFFER
     || gMultiplayerPocBattleDiag.state == POC_BATTLE_READY)
    {
        if (!CanStayInvited())
        {
            sRejectedToken = sToken;
            sRejectedTokenValid = TRUE;
            gMultiplayerPocBattleDiag.state = POC_BATTLE_ABORTED;
            gMultiplayerPocBattleDiag.error = POC_BATTLE_ERROR_LINK_LOST;
            return;
        }
        if (gMultiplayerPocBattleDiag.frame - sStateStartFrame > POC_BATTLE_CONSENT_TIMEOUT)
        {
            BeginCancellation(POC_WORD_CANCEL, POC_BATTLE_ERROR_CONSENT_TIMEOUT);
            return;
        }
        if (gMultiplayerPocBattleDiag.state == POC_BATTLE_OFFER
         && sLocalConsent && sPeerConsent && CanCommitBattle())
            gMultiplayerPocBattleDiag.state = POC_BATTLE_READY;
        if (gMultiplayerPocBattleDiag.state == POC_BATTLE_READY
         && gMultiplayerPocBattleDiag.localPlayerId == 0
         && sPeerReadySeen && CanCommitBattle())
            gMultiplayerPocBattleDiag.state = POC_BATTLE_START;
    }
    else if (gMultiplayerPocBattleDiag.state >= POC_BATTLE_START
          && gMultiplayerPocBattleDiag.state <= POC_BATTLE_GO)
    {
        if (!CanCommitBattle())
        {
            BeginCancellation(POC_WORD_CANCEL, POC_BATTLE_ERROR_CANCELED);
            return;
        }
        if (gMultiplayerPocBattleDiag.frame - sStateStartFrame > POC_BATTLE_CONSENT_TIMEOUT)
        {
            BeginCancellation(POC_WORD_CANCEL, POC_BATTLE_ERROR_CONSENT_TIMEOUT);
            return;
        }
        if (gMultiplayerPocBattleDiag.state == POC_BATTLE_GO
         && ++sGoFrames >= POC_BATTLE_GO_HOLD_FRAMES)
            gMultiplayerPocBattleDiag.state = POC_BATTLE_SETUP;
    }
    else if (gMultiplayerPocBattleDiag.state == POC_BATTLE_CANCELING)
    {
        u32 hold = sCancelWord == POC_WORD_CANCEL_ACK
            ? POC_BATTLE_CANCEL_ACK_HOLD_FRAMES : POC_BATTLE_CANCEL_TIMEOUT;

        if ((sCancelWord != POC_WORD_CANCEL_ACK && sCancelAckSeen)
         || gMultiplayerPocBattleDiag.frame - sStateStartFrame >= hold)
            gMultiplayerPocBattleDiag.state = POC_BATTLE_ABORTED;
    }

    if (gMultiplayerPocBattleDiag.state == POC_BATTLE_SETUP)
    {
        if (!sPartySaved)
            StartBattle();
        else if (gMain.callback2 == BattleMainCB2)
            gMultiplayerPocBattleDiag.state = POC_BATTLE_IN_PROGRESS;
    }
    else if (gMultiplayerPocBattleDiag.state == POC_BATTLE_IN_PROGRESS)
    {
        if (HasLinkErrorOccurred() || !gReceivedRemoteLinkPlayers)
            AbortBattle(POC_BATTLE_ERROR_LINK_LOST);
    }
    else if (gMultiplayerPocBattleDiag.state == POC_BATTLE_RESULT)
    {
        if ((HasLinkErrorOccurred() && !sPeerAckSeen)
         || gMultiplayerPocBattleDiag.frame - sStateStartFrame > POC_BATTLE_RESULT_TIMEOUT)
        {
            gMultiplayerPocBattleDiag.error = POC_BATTLE_ERROR_RESULT_TIMEOUT;
            ReturnToIndependentField();
        }
        else if (sPeerAckSeen
              && (HasLinkErrorOccurred()
               || gMultiplayerPocBattleDiag.frame - sResultAckFrame >= POC_BATTLE_RESULT_HOLD_FRAMES))
        {
            if ((gMultiplayerPocBattleDiag.localOutcome == B_OUTCOME_WON)
             != (gMultiplayerPocBattleDiag.peerOutcome == B_OUTCOME_WON))
                gMultiplayerPocBattleDiag.error = POC_BATTLE_ERROR_RESULT_MISMATCH;
            gMultiplayerPocBattleDiag.resultAgreed =
                gMultiplayerPocBattleDiag.localOutcome == B_OUTCOME_WON
                && gMultiplayerPocBattleDiag.peerOutcome == B_OUTCOME_WON
                && gMultiplayerPocBattleDiag.error == POC_BATTLE_ERROR_NONE;
            sRewardQueued = gMultiplayerPocBattleDiag.resultAgreed;
            ReturnToIndependentField();
        }
    }
    else if (gMultiplayerPocBattleDiag.state == POC_BATTLE_RETURNING
          && gMain.callback2 == CB2_Overworld
          && !gPaletteFade.active
          && !ArePlayerFieldControlsLocked()
          && IsPlayerStandingStill())
    {
        gMultiplayerPocBattleDiag.state = gMultiplayerPocBattleDiag.error
            ? POC_BATTLE_ABORTED : POC_BATTLE_COMPLETE;
        if (sRewardQueued)
        {
            sRewardQueued = FALSE;
            gMultiplayerPocBattleDiag.rewardResult =
                MultiplayerPocReward_OnAgreedWin(MULTIPLAYER_POC_REWARD_ENCOUNTER_ID);
        }
    }

    if (sBattleDetached
     && (gMultiplayerPocBattleDiag.state == POC_BATTLE_COMPLETE
      || gMultiplayerPocBattleDiag.state == POC_BATTLE_ABORTED)
     && gMain.callback2 == CB2_Overworld
     && !gPaletteFade.active
     && !ArePlayerFieldControlsLocked()
     && IsPlayerStandingStill())
    {
        sBattleDetached = FALSE;
        MultiplayerPoc_BattleFinished();
    }
}

void MultiplayerPocBattle_RecordMoveChoice(u8 battler, u16 move, u16 turn)
{
    if (!MultiplayerPocBattle_IsExperimentActive()
     || battler != gMultiplayerPocBattleDiag.localBattlerId)
        return;
    gMultiplayerPocBattleDiag.localMoveChoices++;
    gMultiplayerPocBattleDiag.localLastChosenMove = move;
    gMultiplayerPocBattleDiag.localLastChosenTurn = turn;
}

void MultiplayerPocBattle_RecordMoveAnimation(u8 battler, u16 move, u16 turn)
{
    if (!MultiplayerPocBattle_IsExperimentActive()
     || battler != gMultiplayerPocBattleDiag.localBattlerId)
        return;
    gMultiplayerPocBattleDiag.localExecutedMoves++;
    gMultiplayerPocBattleDiag.localLastExecutedMove = move;
    gMultiplayerPocBattleDiag.localLastExecutedTurn = turn;
}

static void CB2_PocBattleReturn(void)
{
    if (gMultiplayerPocBattleDiag.state != POC_BATTLE_RESULT)
    {
        gMultiplayerPocBattleDiag.localOutcome = gBattleOutcome & 0x7F;
        gMultiplayerPocBattleDiag.peerOutcome = 0xFFFFFFFF;
        sPeerResultSeen = FALSE;
        sPeerAckSeen = FALSE;
        sStateStartFrame = gMultiplayerPocBattleDiag.frame;
        ResetTasks();
        ClearLinkCallback();
        StartSendingKeysToLink();
        gMain.callback1 = NULL;
        gMultiplayerPocBattleDiag.state = POC_BATTLE_RESULT;
    }
}

#endif // WAYFARER_MULTIPLAYER_POC
