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
#include "multiplayer_poc_reward.h"
#include "overworld.h"
#include "script.h"
#include "sprite.h"

#define POC_CONNECT_TIMEOUT_FRAMES 600
#define POC_PEER_TIMEOUT_FRAMES 180
#define POC_SPRITE_RETRY_FRAMES 30
#define POC_AWAY_HEARTBEAT_INTERVAL 16

EWRAM_DATA volatile struct MultiplayerPocDiag gMultiplayerPocDiag;

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

STATIC_ASSERT(sizeof(struct MultiplayerPocDiag) == 80, MultiplayerPocDiagSize);

static void InitPocIfNeeded(void)
{
    if (sInitialized)
        return;
    gMultiplayerPocDiag.magic = MULTIPLAYER_POC_DIAG_MAGIC;
    gMultiplayerPocDiag.version = MULTIPLAYER_POC_DIAG_VERSION;
    gMultiplayerPocDiag.size = sizeof(struct MultiplayerPocDiag);
    gMultiplayerPocDiag.peerId = 0xFFFFFFFF;
    gMultiplayerPocDiag.peerMap = 0xFFFFFFFF;
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
}

bool8 MultiplayerPoc_IsRunning(void)
{
    return gMultiplayerPocDiag.state == MULTIPLAYER_POC_CONNECTING
        || gMultiplayerPocDiag.state == MULTIPLAYER_POC_ACTIVE;
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
     && !MultiplayerPocBattle_OwnsTransport())
    {
        sConsumeClaim = TRUE;
        MultiplayerPocReward_TryClaimPending();
        return TRUE;
    }

    if (!JOY_NEW(SELECT_BUTTON)
     || (gMain.heldKeys & (L_BUTTON | R_BUTTON)) != (L_BUTTON | R_BUTTON))
        return FALSE;

    sConsumeSelect = TRUE;
    if (MultiplayerPoc_IsRunning())
    {
        StopPoc(MULTIPLAYER_POC_IDLE, MULTIPLAYER_POC_ERROR_NONE);
        return TRUE;
    }

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
    sStartFrame = gMultiplayerPocDiag.frame;

    // This is Emerald's ordinary multi-serial cable transport. Both carts need
    // this PoC ROM; the link type is deliberately separate from Cable Club.
    gWirelessCommType = 0;
    gLinkType = LINKTYPE_WAYFARER_POC;
    // Normal intro installs this callback. The E2E direct-overworld path skips
    // intro, so install the ordinary serial ISR explicitly for both paths.
    SetSerialCallback(SerialCB);
    OpenLinkTimed();
    SetSuppressLinkErrorMessage(TRUE);
    ResetLinkPlayers();
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

void MultiplayerPoc_Update(void)
{
    u8 status;

    InitPocIfNeeded();
    MultiplayerPocReward_UpdateDiag();
    gMultiplayerPocDiag.frame++;
    CaptureLocalState();
    if (MultiplayerPoc_IsRunning() && sPendingError)
    {
        StopPoc(MULTIPLAYER_POC_FAILED, sPendingError);
        return;
    }
    if (MultiplayerPoc_IsRunning() && HasLinkErrorOccurred())
    {
        StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_TRANSPORT);
        return;
    }

    MultiplayerPocBattle_Update();
    if (MultiplayerPocBattle_OwnsTransport() || !MultiplayerPoc_IsRunning())
        return;

    if (gMultiplayerPocDiag.state == MULTIPLAYER_POC_CONNECTING)
    {
        if (gMultiplayerPocDiag.frame - sStartFrame > POC_CONNECT_TIMEOUT_FRAMES)
        {
            StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_CONNECT_TIMEOUT);
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
        gMultiplayerPocDiag.state = MULTIPLAYER_POC_ACTIVE;
        sLastPeerFrame = gMultiplayerPocDiag.frame;
        StartSendingKeysToLink();
        return;
    }

    if (GetLinkPlayerCount_2() != 2
     || !gReceivedRemoteLinkPlayers
     || gMultiplayerPocDiag.frame - sLastPeerFrame > POC_PEER_TIMEOUT_FRAMES)
    {
        StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_PEER_TIMEOUT);
        return;
    }

    UpdatePeerSprite();
}

void MultiplayerPoc_BuildSendCmd(u16 *cmd)
{
    if (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE)
        return;

    cmd[2] = MULTIPLAYER_POC_PACKET_MAGIC;
    cmd[3] = MULTIPLAYER_POC_PROTOCOL_VERSION;
    cmd[4] = gMultiplayerPocDiag.localMap;
    cmd[5] = gMultiplayerPocDiag.localX;
    cmd[6] = gMultiplayerPocDiag.localY;
    cmd[7] = (gMultiplayerPocDiag.localFacing & 0xF) | (sLocalInField << 8);
    gMultiplayerPocDiag.txCount++;
}

bool8 MultiplayerPoc_ShouldSendCmd(void)
{
    u32 battleState = gMultiplayerPocBattleDiag.state;

    if (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE)
        return TRUE;
    if (gMultiplayerPocDiag.rxCount == 0)
        return TRUE;
    // Both players consent at full rate before handing the cable to battle.
    if (battleState >= POC_BATTLE_OFFER && battleState <= POC_BATTLE_GO)
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
    if (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE
     || playerId != gMultiplayerPocDiag.peerId)
        return;

    gMultiplayerPocDiag.lastCmd = cmd[0];
    if (cmd[2] != MULTIPLAYER_POC_PACKET_MAGIC
     || cmd[3] != MULTIPLAYER_POC_PROTOCOL_VERSION)
    {
        sPendingError = MULTIPLAYER_POC_ERROR_PROTOCOL;
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
