#include "global.h"

#if WAYFARER_MULTIPLAYER_POC

#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "fieldmap.h"
#include "link.h"
#include "main.h"
#include "multiplayer_poc.h"
#include "multiplayer_poc_battle.h"
#include "multiplayer_poc_menu.h"
#include "multiplayer_poc_reward.h"
#include "overworld.h"
#include "palette.h"
#include "script.h"
#include "sprite.h"

#define POC_CONNECT_TIMEOUT_FRAMES 600
#define POC_PEER_TIMEOUT_FRAMES 180
#define POC_SPRITE_RETRY_FRAMES 30
#define POC_AWAY_HEARTBEAT_INTERVAL 16
#define POC_CLOSE_TIMEOUT_FRAMES 240
#define POC_SESSION_TIMEOUT_FRAMES 1800

const volatile struct MultiplayerPocBuildTag gMultiplayerPocBuildTag
    __attribute__((used, section(".rodata.multiplayer_poc_build"))) =
{
    .marker = {'W', 'F', 'P', 'B', 'I', 'D', 'v', '1'},
    .id = {0},
    .tail = {'B', 'I', 'D', '!'},
};

EWRAM_DATA volatile struct MultiplayerPocDiag gMultiplayerPocDiag;
EWRAM_DATA volatile struct MultiplayerPocSessionDiag gMultiplayerPocSessionDiag;

static EWRAM_DATA u32 sStartFrame;
static EWRAM_DATA u32 sLastPeerFrame;
static EWRAM_DATA u32 sNextSpriteRetryFrame;
static EWRAM_DATA u8 sPeerSpriteId;
static EWRAM_DATA u8 sPendingError;
static EWRAM_DATA bool8 sLocalInField;
static EWRAM_DATA bool8 sPeerInField;
static EWRAM_DATA bool8 sInitialized;
static EWRAM_DATA bool8 sConsumeSelect;
static EWRAM_DATA bool8 sConsumeClaim;
static EWRAM_DATA bool8 sSessionWanted;
static EWRAM_DATA bool8 sLocalSaveHold;
static EWRAM_DATA bool8 sPeerSaveRequested;
static EWRAM_DATA bool8 sBattleHold;
static EWRAM_DATA bool8 sPeerBuildMatches;
static EWRAM_DATA bool8 sPeerHelloAck;
static EWRAM_DATA u8 sHelloPage;
static EWRAM_DATA u8 sHelloPagesSeen;
static EWRAM_DATA u8 sPeerBuildId[16];
static EWRAM_DATA u32 sCloseStartFrame;
static EWRAM_DATA u32 sSessionDeadline;
static EWRAM_DATA u32 sSaveRequestFrame;

STATIC_ASSERT(sizeof(struct MultiplayerPocDiag) == 80, MultiplayerPocDiagSize);
STATIC_ASSERT(sizeof(struct MultiplayerPocSessionDiag) == 72, MultiplayerPocSessionDiagSize);

static void SetSessionStatus(enum MultiplayerPocSessionStatus status)
{
    gMultiplayerPocSessionDiag.status = status;
}

static void SetSessionError(enum MultiplayerPocError error)
{
    gMultiplayerPocSessionDiag.error = error;
    gMultiplayerPocDiag.error = error;
}

static u8 GetBuildIdByte(u8 index)
{
    return gMultiplayerPocBuildTag.id[index];
}

static void InitPocIfNeeded(void)
{
    if (sInitialized)
        return;
    gMultiplayerPocDiag.magic = MULTIPLAYER_POC_DIAG_MAGIC;
    gMultiplayerPocDiag.version = MULTIPLAYER_POC_DIAG_VERSION;
    gMultiplayerPocDiag.size = sizeof(struct MultiplayerPocDiag);
    gMultiplayerPocDiag.peerId = 0xFFFFFFFF;
    gMultiplayerPocDiag.peerMap = 0xFFFFFFFF;
    gMultiplayerPocSessionDiag.magic = MULTIPLAYER_POC_SESSION_DIAG_MAGIC;
    gMultiplayerPocSessionDiag.version = 1;
    gMultiplayerPocSessionDiag.size = sizeof(struct MultiplayerPocSessionDiag);
    for (u8 i = 0; i < 4; i++)
        gMultiplayerPocSessionDiag.buildId[i] = GetBuildIdByte(i * 4)
            | (GetBuildIdByte(i * 4 + 1) << 8)
            | (GetBuildIdByte(i * 4 + 2) << 16)
            | ((u32)GetBuildIdByte(i * 4 + 3) << 24);
    sPeerSpriteId = MAX_SPRITES;
    sInitialized = TRUE;
}

static void SpriteCB_MultiplayerPoc(struct Sprite *sprite)
{
    // The ordinary overworld remains in control of the local camera and input.
}

static bool8 HasPeerSprite(void)
{
    return sPeerSpriteId < MAX_SPRITES
        && gSprites[sPeerSpriteId].inUse
        && gSprites[sPeerSpriteId].callback == SpriteCB_MultiplayerPoc;
}

static void ClearPeerSprite(void)
{
    if (HasPeerSprite())
        DestroySprite(&gSprites[sPeerSpriteId]);
    sPeerSpriteId = MAX_SPRITES;
    gMultiplayerPocDiag.peerVisible = FALSE;
}

static void StopPoc(enum MultiplayerPocState state, enum MultiplayerPocError error)
{
    MultiplayerPocBattle_OnPresenceLost();
    ClearPeerSprite();
    CloseLink();
    ClearLinkCallback();
    SetSuppressLinkErrorMessage(FALSE);
    gLinkType = 0;
    gMultiplayerPocDiag.state = state;
    gMultiplayerPocDiag.error = error;
    sPendingError = MULTIPLAYER_POC_ERROR_NONE;
    sPeerInField = FALSE;
    if (state == MULTIPLAYER_POC_FAILED)
    {
        sSessionWanted = FALSE;
        SetSessionStatus(MULTIPLAYER_POC_SESSION_FAILED);
        SetSessionError(error);
    }
    else
    {
        SetSessionStatus(MULTIPLAYER_POC_SESSION_IDLE);
        SetSessionError(MULTIPLAYER_POC_ERROR_NONE);
    }
    gMultiplayerPocSessionDiag.wanted = sSessionWanted;
}

bool8 MultiplayerPoc_IsRunning(void)
{
    return gMultiplayerPocDiag.state == MULTIPLAYER_POC_CONNECTING
        || gMultiplayerPocDiag.state == MULTIPLAYER_POC_HELLO
        || gMultiplayerPocDiag.state == MULTIPLAYER_POC_ACTIVE
        || gMultiplayerPocDiag.state == MULTIPLAYER_POC_SUSPENDING;
}

void MultiplayerPoc_BattleDetach(void)
{
    ClearPeerSprite();
    ClearLinkCallback();
    SetSuppressLinkErrorMessage(FALSE);
    gMultiplayerPocDiag.state = MULTIPLAYER_POC_IDLE;
    gMultiplayerPocDiag.error = MULTIPLAYER_POC_ERROR_NONE;
    sPendingError = MULTIPLAYER_POC_ERROR_NONE;
    sPeerInField = FALSE;
    sBattleHold = sSessionWanted;
    SetSessionStatus(sBattleHold ? MULTIPLAYER_POC_SESSION_BATTLE_HOLD
                                 : MULTIPLAYER_POC_SESSION_IDLE);
}

void MultiplayerPoc_BattleFinished(void)
{
    sBattleHold = FALSE;
    if (sSessionWanted)
    {
        sSessionDeadline = gMultiplayerPocDiag.frame + POC_SESSION_TIMEOUT_FRAMES;
        SetSessionStatus(MULTIPLAYER_POC_SESSION_RECONNECTING);
    }
    else
        SetSessionStatus(MULTIPLAYER_POC_SESSION_IDLE);
}

static bool8 CanOpenCable(void)
{
    return gMain.callback2 == CB2_Overworld
        && !gPaletteFade.active
        && !ArePlayerFieldControlsLocked()
        && IsPlayerStandingStill()
        && !MultiplayerPocBattle_OwnsTransport();
}

static void OpenPocCable(void)
{
    ClearPeerSprite();
    gMultiplayerPocDiag.state = MULTIPLAYER_POC_CONNECTING;
    gMultiplayerPocDiag.error = MULTIPLAYER_POC_ERROR_NONE;
    gMultiplayerPocDiag.peerId = 0xFFFFFFFF;
    gMultiplayerPocDiag.peerMap = 0xFFFFFFFF;
    gMultiplayerPocDiag.txCount = 0;
    gMultiplayerPocDiag.rxCount = 0;
    gMultiplayerPocDiag.lastCmd = 0;
    sPendingError = MULTIPLAYER_POC_ERROR_NONE;
    sPeerInField = FALSE;
    sPeerSaveRequested = FALSE;
    sPeerBuildMatches = FALSE;
    sPeerHelloAck = FALSE;
    sHelloPagesSeen = 0;
    sHelloPage = 0;
    gMultiplayerPocSessionDiag.helloPagesSeen = 0;
    gMultiplayerPocSessionDiag.helloPeerConfirmed = 0;
    gMultiplayerPocSessionDiag.buildMatch = 0;
    gMultiplayerPocSessionDiag.attemptCount++;
    sStartFrame = gMultiplayerPocDiag.frame;
    SetSessionStatus(gMultiplayerPocSessionDiag.attemptCount > 1
                     ? MULTIPLAYER_POC_SESSION_RECONNECTING
                     : MULTIPLAYER_POC_SESSION_CONNECTING);

    gWirelessCommType = 0;
    gLinkType = LINKTYPE_WAYFARER_POC;
    SetSerialCallback(SerialCB);
    OpenLinkTimed();
    SetSuppressLinkErrorMessage(TRUE);
    ResetLinkPlayers();
}

bool8 MultiplayerPoc_Connect(void)
{
    u8 i;
    u8 nonzero = 0;

    InitPocIfNeeded();
    if (sLocalSaveHold || sBattleHold || MultiplayerPocBattle_OwnsTransport())
        return FALSE;
    // An ELF loaded without the post-link ROM stamp contains the zero
    // placeholder. Never let two such images authenticate one another.
    for (i = 0; i < 16; i++)
        nonzero |= GetBuildIdByte(i);
    if (!nonzero)
    {
        sSessionWanted = FALSE;
        gMultiplayerPocSessionDiag.wanted = FALSE;
        gMultiplayerPocDiag.state = MULTIPLAYER_POC_FAILED;
        SetSessionStatus(MULTIPLAYER_POC_SESSION_FAILED);
        SetSessionError(MULTIPLAYER_POC_ERROR_BUILD_MISMATCH);
        return FALSE;
    }
    sSessionWanted = TRUE;
    gMultiplayerPocSessionDiag.wanted = TRUE;
    SetSessionError(MULTIPLAYER_POC_ERROR_NONE);
    if (MultiplayerPoc_IsRunning())
        return TRUE;
    sSessionDeadline = gMultiplayerPocDiag.frame + POC_SESSION_TIMEOUT_FRAMES;
    if (CanOpenCable())
        OpenPocCable();
    else
        SetSessionStatus(MULTIPLAYER_POC_SESSION_RECONNECTING);
    return TRUE;
}

void MultiplayerPoc_Leave(void)
{
    InitPocIfNeeded();
    sSessionWanted = FALSE;
    sLocalSaveHold = FALSE;
    sBattleHold = FALSE;
    sPeerSaveRequested = FALSE;
    gMultiplayerPocSessionDiag.wanted = FALSE;
    gMultiplayerPocSessionDiag.localSaveHold = FALSE;
    gMultiplayerPocSessionDiag.peerSaveRequested = FALSE;
    StopPoc(MULTIPLAYER_POC_IDLE, MULTIPLAYER_POC_ERROR_NONE);
}

static void BeginClosingForSave(void)
{
    if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_SUSPENDING)
        return;
    if (!MultiplayerPoc_LinkIsOpen())
    {
        gMultiplayerPocDiag.state = MULTIPLAYER_POC_IDLE;
        SetSessionStatus(sLocalSaveHold ? MULTIPLAYER_POC_SESSION_SAVE_HOLD
                                        : MULTIPLAYER_POC_SESSION_RECONNECTING);
        return;
    }
    MultiplayerPocBattle_OnPresenceLost();
    ClearPeerSprite();
    gMultiplayerPocDiag.state = MULTIPLAYER_POC_SUSPENDING;
    SetSessionStatus(MULTIPLAYER_POC_SESSION_SUSPENDING);
    sCloseStartFrame = gMultiplayerPocDiag.frame;
    ClearLinkCallback();
    SetCloseLinkCallback();
}

void MultiplayerPoc_BeginLocalSave(void)
{
    InitPocIfNeeded();
    if (sLocalSaveHold)
        return;
    sLocalSaveHold = TRUE;
    gMultiplayerPocSessionDiag.localSaveHold = TRUE;
    sSaveRequestFrame = gMultiplayerPocDiag.frame;
    if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_ACTIVE)
        SetSessionStatus(MULTIPLAYER_POC_SESSION_SUSPENDING);
    else if (MultiplayerPoc_IsRunning())
        BeginClosingForSave();
    else
        SetSessionStatus(MULTIPLAYER_POC_SESSION_SAVE_HOLD);
}

bool8 MultiplayerPoc_IsLocalSaveReady(void)
{
    return sLocalSaveHold && !MultiplayerPoc_LinkIsOpen()
        && !MultiplayerPocBattle_OwnsTransport();
}

void MultiplayerPoc_EndLocalSave(void)
{
    if (!sLocalSaveHold)
        return;
    sLocalSaveHold = FALSE;
    gMultiplayerPocSessionDiag.localSaveHold = FALSE;
    if (sSessionWanted)
    {
        sSessionDeadline = gMultiplayerPocDiag.frame + POC_SESSION_TIMEOUT_FRAMES;
        SetSessionStatus(MULTIPLAYER_POC_SESSION_RECONNECTING);
    }
    else
        SetSessionStatus(MULTIPLAYER_POC_SESSION_IDLE);
}

enum MultiplayerPocSessionStatus MultiplayerPoc_GetStatus(void)
{
    return gMultiplayerPocSessionDiag.status;
}

enum MultiplayerPocError MultiplayerPoc_GetError(void)
{
    return gMultiplayerPocSessionDiag.error;
}

bool8 MultiplayerPoc_TryToggle(void)
{
    // FieldGetPlayerInput counts held SELECT frames and interprets its release
    // as a registered-item tap. Consume the complete debug chord.
    if (sConsumeSelect)
    {
        if (!JOY_HELD(SELECT_BUTTON))
            sConsumeSelect = FALSE;
        return TRUE;
    }

    if (sConsumeClaim)
    {
        if (!JOY_HELD(A_BUTTON))
            sConsumeClaim = FALSE;
        return TRUE;
    }

    if (MultiplayerPocBattle_TryRequest())
        return TRUE;

    // Explicit local retry after making bag space. It does not implicitly
    // connect, start a battle, or accept a reward on the other cartridge.
    if (JOY_NEW(A_BUTTON)
     && (gMain.heldKeys & (L_BUTTON | R_BUTTON)) == (L_BUTTON | R_BUTTON)
     && !ArePlayerFieldControlsLocked()
     && IsPlayerStandingStill()
     && !MultiplayerPocBattle_OwnsTransport()
     && !sSessionWanted
     && !MultiplayerPoc_LinkIsOpen())
    {
        sConsumeClaim = TRUE;
        MultiplayerPocReward_TryClaimPending();
        return TRUE;
    }

    if (!JOY_NEW(SELECT_BUTTON)
     || (gMain.heldKeys & (L_BUTTON | R_BUTTON)) != (L_BUTTON | R_BUTTON))
        return FALSE;

    sConsumeSelect = TRUE;
    if (sSessionWanted)
        MultiplayerPoc_Leave();
    else
        MultiplayerPoc_Connect();
    return TRUE;
}

static void CaptureLocalState(void)
{
    const struct ObjectEvent *player;

    sLocalInField = FALSE;
    if (gMain.callback2 != CB2_Overworld
     || gPlayerAvatar.objectEventId >= OBJECT_EVENTS_COUNT)
        return;

    player = &gObjectEvents[gPlayerAvatar.objectEventId];
    if (!player->active)
        return;

    gMultiplayerPocDiag.localMap =
        (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    gMultiplayerPocDiag.localX = player->currentCoords.x;
    gMultiplayerPocDiag.localY = player->currentCoords.y;
    gMultiplayerPocDiag.localFacing = GetPlayerFacingDirection();
    sLocalInField = TRUE;
}

static void UpdatePeerSprite(void)
{
    struct Sprite *sprite;
    const struct ObjectEventGraphicsInfo *graphics;
    u16 graphicsId;
    s16 x, y;

    if (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE
     || !sLocalInField
     || !sPeerInField
     || gMultiplayerPocDiag.frame - sLastPeerFrame > POC_PEER_TIMEOUT_FRAMES
     || gMultiplayerPocDiag.peerMap != gMultiplayerPocDiag.localMap
     || gMapHeader.mapLayout == NULL
     || gMultiplayerPocDiag.peerX < MAP_OFFSET
     || gMultiplayerPocDiag.peerY < MAP_OFFSET
     || gMultiplayerPocDiag.peerX >= gMapHeader.mapLayout->width + MAP_OFFSET
     || gMultiplayerPocDiag.peerY >= gMapHeader.mapLayout->height + MAP_OFFSET)
    {
        ClearPeerSprite();
        return;
    }

    if (!HasPeerSprite())
    {
        if (gMultiplayerPocDiag.frame < sNextSpriteRetryFrame)
            return;
        graphicsId = GetPlayerAvatarGraphicsIdByStateIdAndGender(
            PLAYER_AVATAR_STATE_NORMAL, gLinkPlayers[gMultiplayerPocDiag.peerId].gender);
        sPeerSpriteId = CreateObjectGraphicsSprite(graphicsId,
                                                   SpriteCB_MultiplayerPoc, 0, 0, 1);
        if (!HasPeerSprite())
        {
            sPeerSpriteId = MAX_SPRITES;
            sNextSpriteRetryFrame = gMultiplayerPocDiag.frame + POC_SPRITE_RETRY_FRAMES;
            return;
        }
        StartSpriteAnim(&gSprites[sPeerSpriteId],
                        GetFaceDirectionAnimNum(gMultiplayerPocDiag.peerFacing));
    }

    sprite = &gSprites[sPeerSpriteId];
    graphics = GetObjectEventGraphicsInfo(
        GetPlayerAvatarGraphicsIdByStateIdAndGender(
            PLAYER_AVATAR_STATE_NORMAL, gLinkPlayers[gMultiplayerPocDiag.peerId].gender));
    // Standard map object sprites add gSpriteCoordOffset during OAM assembly.
    // The peer is a standalone sprite, so opt it into the same camera offset.
    sprite->coordOffsetEnabled = TRUE;
    sprite->centerToCornerVecX = -(graphics->width >> 1);
    sprite->centerToCornerVecY = -(graphics->height >> 1);
    SetSpritePosToMapCoords(gMultiplayerPocDiag.peerX,
                            gMultiplayerPocDiag.peerY, &x, &y);
    sprite->x = x + 8;
    sprite->y = y + 16 + sprite->centerToCornerVecY;
    UpdateObjectEventSpriteInvisibility(sprite, FALSE);
    gMultiplayerPocDiag.peerVisible = !sprite->invisible;
}

static void FinishCableClose(void)
{
    ClearLinkCallback();
    SetSuppressLinkErrorMessage(FALSE);
    gLinkType = 0;
    gMultiplayerPocDiag.state = MULTIPLAYER_POC_IDLE;
    gMultiplayerPocSessionDiag.closeCount++;
    sPeerSaveRequested = FALSE;
    gMultiplayerPocSessionDiag.peerSaveRequested = FALSE;
    if (sLocalSaveHold)
        SetSessionStatus(MULTIPLAYER_POC_SESSION_SAVE_HOLD);
    else if (sSessionWanted)
        SetSessionStatus(MULTIPLAYER_POC_SESSION_RECONNECTING);
    else
        SetSessionStatus(MULTIPLAYER_POC_SESSION_IDLE);
}

static void RetryAfterCableLoss(enum MultiplayerPocError error)
{
    MultiplayerPocBattle_OnPresenceLost();
    ClearPeerSprite();
    CloseLink();
    FinishCableClose();
    if (!sSessionWanted)
    {
        StopPoc(MULTIPLAYER_POC_FAILED, error);
        return;
    }
    gMultiplayerPocSessionDiag.reconnectCount++;
    SetSessionError(error);
    if (sSessionDeadline == 0)
        sSessionDeadline = gMultiplayerPocDiag.frame + POC_SESSION_TIMEOUT_FRAMES;
}

static void ActivateAfterHello(void)
{
    if (!sPeerBuildMatches || !sPeerHelloAck)
        return;
    gMultiplayerPocDiag.state = MULTIPLAYER_POC_ACTIVE;
    SetSessionStatus(MULTIPLAYER_POC_SESSION_ACTIVE);
    SetSessionError(MULTIPLAYER_POC_ERROR_NONE);
    sLastPeerFrame = gMultiplayerPocDiag.frame;
    gMultiplayerPocSessionDiag.buildMatch = TRUE;
    sSessionDeadline = 0;
}

void MultiplayerPoc_Update(void)
{
    u8 status;

    InitPocIfNeeded();
    MultiplayerPocReward_UpdateDiag();
    gMultiplayerPocDiag.frame++;
    CaptureLocalState();
    MultiplayerPocBattle_Update();
    MultiplayerPocMenu_Update();
    if (sBattleHold || MultiplayerPocBattle_OwnsTransport())
        return;

    if (MultiplayerPoc_IsRunning() && sPendingError)
    {
        StopPoc(MULTIPLAYER_POC_FAILED, sPendingError);
        return;
    }

    if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_SUSPENDING)
    {
        if (!MultiplayerPoc_LinkIsOpen())
            FinishCableClose();
        else if (HasLinkErrorOccurred()
              || gMultiplayerPocDiag.frame - sCloseStartFrame > POC_CLOSE_TIMEOUT_FRAMES)
        {
            CloseLink();
            FinishCableClose();
        }
        return;
    }

    if (MultiplayerPoc_IsRunning() && HasLinkErrorOccurred())
    {
        RetryAfterCableLoss(MULTIPLAYER_POC_ERROR_TRANSPORT);
        return;
    }

    if (!MultiplayerPoc_IsRunning())
    {
        if (sLocalSaveHold)
            SetSessionStatus(MULTIPLAYER_POC_SESSION_SAVE_HOLD);
        else if (sSessionWanted && !MultiplayerPoc_LinkIsOpen())
        {
            if (gMultiplayerPocDiag.frame >= sSessionDeadline)
                StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_SESSION_TIMEOUT);
            else if (CanOpenCable())
                OpenPocCable();
        }
        return;
    }

    if ((sLocalSaveHold
      && (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE
       || gMultiplayerPocDiag.frame - sSaveRequestFrame >= 8))
     || sPeerSaveRequested
     || (gMultiplayerPocDiag.peerId < 2
      && gReadyToCloseLink[gMultiplayerPocDiag.peerId]))
    {
        if (!sLocalSaveHold)
            sSessionDeadline = gMultiplayerPocDiag.frame + POC_SESSION_TIMEOUT_FRAMES;
        BeginClosingForSave();
        return;
    }

    if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_CONNECTING)
    {
        if (gMultiplayerPocDiag.frame - sStartFrame > POC_CONNECT_TIMEOUT_FRAMES)
        {
            RetryAfterCableLoss(MULTIPLAYER_POC_ERROR_CONNECT_TIMEOUT);
            return;
        }
        if (GetLinkPlayerCount_2() > 2)
        {
            StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_PLAYER_COUNT);
            return;
        }
        if (GetLinkPlayerCount_2() == 2 && IsLinkMaster())
            CheckShouldAdvanceLinkState();

        status = GetLinkPlayerDataExchangeStatusTimed(2, 2);
        if (status == EXCHANGE_DIFF_SELECTIONS)
        {
            StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_LINK_TYPE);
            return;
        }
        if (status == EXCHANGE_WRONG_NUM_PLAYERS)
        {
            StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_PLAYER_COUNT);
            return;
        }
        if (status != EXCHANGE_COMPLETE)
            return;

        gMultiplayerPocDiag.localId = GetMultiplayerId();
        gMultiplayerPocDiag.peerId = gMultiplayerPocDiag.localId ^ 1;
        if (gLinkPlayers[gMultiplayerPocDiag.localId].linkType != LINKTYPE_WAYFARER_POC
         || gLinkPlayers[gMultiplayerPocDiag.peerId].linkType != LINKTYPE_WAYFARER_POC)
        {
            StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_LINK_TYPE);
            return;
        }
        gMultiplayerPocDiag.state = MULTIPLAYER_POC_HELLO;
        sLastPeerFrame = gMultiplayerPocDiag.frame;
        StartSendingKeysToLink();
        return;
    }

    if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_HELLO)
    {
        if (gMultiplayerPocDiag.frame - sStartFrame > POC_CONNECT_TIMEOUT_FRAMES)
            RetryAfterCableLoss(MULTIPLAYER_POC_ERROR_CONNECT_TIMEOUT);
        else if (GetLinkPlayerCount_2() != 2 || !gReceivedRemoteLinkPlayers)
            RetryAfterCableLoss(MULTIPLAYER_POC_ERROR_PEER_TIMEOUT);
        return;
    }

    if (GetLinkPlayerCount_2() != 2
     || !gReceivedRemoteLinkPlayers
     || gMultiplayerPocDiag.frame - sLastPeerFrame > POC_PEER_TIMEOUT_FRAMES)
    {
        RetryAfterCableLoss(MULTIPLAYER_POC_ERROR_PEER_TIMEOUT);
        return;
    }

    UpdatePeerSprite();
}

void MultiplayerPoc_BuildSendCmd(u16 *cmd)
{
    u8 i;

    if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_HELLO)
    {
        u8 page = sHelloPage++ % 3;

        cmd[2] = MULTIPLAYER_POC_PACKET_MAGIC;
        cmd[3] = MULTIPLAYER_POC_PROTOCOL_VERSION;
        cmd[4] = MULTIPLAYER_POC_HELLO_MARKER | page
               | (sPeerBuildMatches ? 0x80 : 0);
        for (i = 0; i < 3; i++)
        {
            u8 index = page * 6 + i * 2;
            u16 word = index < 16 ? GetBuildIdByte(index) : 0;

            if (index + 1 < 16)
                word |= GetBuildIdByte(index + 1) << 8;
            cmd[5 + i] = word;
        }
        gMultiplayerPocDiag.txCount++;
        return;
    }

    if (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE)
        return;

    cmd[2] = MULTIPLAYER_POC_PACKET_MAGIC;
    cmd[3] = MULTIPLAYER_POC_PROTOCOL_VERSION;
    cmd[4] = gMultiplayerPocDiag.localMap;
    cmd[5] = gMultiplayerPocDiag.localX;
    cmd[6] = gMultiplayerPocDiag.localY;
    cmd[7] = (gMultiplayerPocDiag.localFacing & 0xF) | (sLocalInField << 8)
           | (sLocalSaveHold ? MULTIPLAYER_POC_SAVE_REQUEST_BIT : 0)
           | MULTIPLAYER_POC_HELLO_ACK_BIT;
    gMultiplayerPocDiag.txCount++;
}

bool8 MultiplayerPoc_ShouldSendCmd(void)
{
    u32 battleState = gMultiplayerPocBattleDiag.state;

    if (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE)
        return TRUE;
    if (sLocalSaveHold)
        return TRUE;
    if (gMultiplayerPocDiag.rxCount == 0)
        return TRUE;
    // Both players consent at full rate before handing the cable to battle.
    if (battleState >= POC_BATTLE_START && battleState <= POC_BATTLE_GO)
        return TRUE;
    if (sLocalInField && sPeerInField)
        return TRUE;
    // Full-screen menus and solo battles update their main callback more
    // slowly. Empty link commands let their receive queue drain while a
    // periodic snapshot still satisfies the presence watchdog.
    return (gMultiplayerPocDiag.frame & (POC_AWAY_HEARTBEAT_INTERVAL - 1)) == 0;
}

void MultiplayerPoc_ReceiveCmd(u8 playerId, const u16 *cmd)
{
    u8 i;

    if (playerId != gMultiplayerPocDiag.peerId
     || (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE
      && gMultiplayerPocDiag.state != MULTIPLAYER_POC_HELLO))
        return;

    gMultiplayerPocDiag.lastCmd = cmd[0];
    if (cmd[2] != MULTIPLAYER_POC_PACKET_MAGIC
     || cmd[3] != MULTIPLAYER_POC_PROTOCOL_VERSION)
    {
        sPendingError = MULTIPLAYER_POC_ERROR_PROTOCOL;
        return;
    }

    if ((cmd[4] & 0xFF00) == MULTIPLAYER_POC_HELLO_MARKER)
    {
        u8 page = cmd[4] & 0x7F;

        if (page > 2)
        {
            sPendingError = MULTIPLAYER_POC_ERROR_PROTOCOL;
            return;
        }
        for (i = 0; i < 3; i++)
        {
            u8 index = page * 6 + i * 2;

            if (index < 16)
                sPeerBuildId[index] = cmd[5 + i] & 0xFF;
            if (index + 1 < 16)
                sPeerBuildId[index + 1] = cmd[5 + i] >> 8;
        }
        sHelloPagesSeen |= 1 << page;
        gMultiplayerPocSessionDiag.helloPagesSeen = sHelloPagesSeen;
        if (sHelloPagesSeen == 7 && !sPeerBuildMatches)
        {
            for (i = 0; i < 16; i++)
            {
                if (sPeerBuildId[i] != GetBuildIdByte(i))
                {
                    sPendingError = MULTIPLAYER_POC_ERROR_BUILD_MISMATCH;
                    return;
                }
            }
            sPeerBuildMatches = TRUE;
            gMultiplayerPocSessionDiag.buildMatch = TRUE;
        }
        if (cmd[4] & 0x80)
        {
            sPeerHelloAck = TRUE;
            gMultiplayerPocSessionDiag.helloPeerConfirmed = TRUE;
        }
        sLastPeerFrame = gMultiplayerPocDiag.frame;
        if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_HELLO)
            ActivateAfterHello();
        return;
    }

    if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_HELLO)
    {
        if (cmd[7] & MULTIPLAYER_POC_HELLO_ACK_BIT)
        {
            sPeerHelloAck = TRUE;
            gMultiplayerPocSessionDiag.helloPeerConfirmed = TRUE;
            ActivateAfterHello();
        }
        return;
    }

    gMultiplayerPocDiag.peerMap = cmd[4];
    gMultiplayerPocDiag.peerX = (s16)cmd[5];
    gMultiplayerPocDiag.peerY = (s16)cmd[6];
    gMultiplayerPocDiag.peerFacing = cmd[7] & 0xF;
    if (gMultiplayerPocDiag.peerFacing < DIR_SOUTH
     || gMultiplayerPocDiag.peerFacing > DIR_EAST)
        gMultiplayerPocDiag.peerFacing = DIR_SOUTH;
    sPeerInField = (cmd[7] & 0x100) != 0;
    sPeerSaveRequested = (cmd[7] & MULTIPLAYER_POC_SAVE_REQUEST_BIT) != 0;
    gMultiplayerPocSessionDiag.peerSaveRequested = sPeerSaveRequested;
    sLastPeerFrame = gMultiplayerPocDiag.frame;
    gMultiplayerPocDiag.rxCount++;
    MultiplayerPocBattle_OnPeerWord(cmd[1]);
    if (HasPeerSprite()
     && gSprites[sPeerSpriteId].animNum
        != GetFaceDirectionAnimNum(gMultiplayerPocDiag.peerFacing))
        StartSpriteAnim(&gSprites[sPeerSpriteId],
                        GetFaceDirectionAnimNum(gMultiplayerPocDiag.peerFacing));
}

#endif // WAYFARER_MULTIPLAYER_POC
