#include "global.h"

#if WAYFARER_MULTIPLAYER_POC

#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "link.h"
#include "main.h"
#include "multiplayer_poc.h"
#include "overworld.h"
#include "sprite.h"

#define POC_PACKET_MAGIC 0x5750 // "WP"
#define POC_CONNECT_TIMEOUT_FRAMES 600
#define POC_PEER_TIMEOUT_FRAMES 180
#define POC_SPRITE_RETRY_FRAMES 30

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

STATIC_ASSERT(sizeof(struct MultiplayerPocDiag) == 80, MultiplayerPocDiagSize);

static void InitPocIfNeeded(void)
{
    if (sInitialized)
        return;
    gMultiplayerPocDiag.magic = MULTIPLAYER_POC_DIAG_MAGIC;
    gMultiplayerPocDiag.version = MULTIPLAYER_POC_PROTOCOL_VERSION;
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
    gMultiplayerPocDiag.frame++;
    CaptureLocalState();

    if (!MultiplayerPoc_IsRunning())
        return;

    if (sPendingError)
    {
        StopPoc(MULTIPLAYER_POC_FAILED, sPendingError);
        return;
    }
    if (HasLinkErrorOccurred())
    {
        StopPoc(MULTIPLAYER_POC_FAILED, MULTIPLAYER_POC_ERROR_TRANSPORT);
        return;
    }

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

    cmd[2] = POC_PACKET_MAGIC;
    cmd[3] = MULTIPLAYER_POC_PROTOCOL_VERSION;
    cmd[4] = gMultiplayerPocDiag.localMap;
    cmd[5] = gMultiplayerPocDiag.localX;
    cmd[6] = gMultiplayerPocDiag.localY;
    cmd[7] = (gMultiplayerPocDiag.localFacing & 0xF) | (sLocalInField << 8);
    gMultiplayerPocDiag.txCount++;
}

void MultiplayerPoc_ReceiveCmd(u8 playerId, const u16 *cmd)
{
    if (gMultiplayerPocDiag.state != MULTIPLAYER_POC_ACTIVE
     || playerId != gMultiplayerPocDiag.peerId)
        return;

    gMultiplayerPocDiag.lastCmd = cmd[0];
    if (cmd[2] != POC_PACKET_MAGIC
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
    if (HasPeerSprite()
     && gSprites[sPeerSpriteId].animNum
        != GetFaceDirectionAnimNum(gMultiplayerPocDiag.peerFacing))
        StartSpriteAnim(&gSprites[sPeerSpriteId],
                        GetFaceDirectionAnimNum(gMultiplayerPocDiag.peerFacing));
}

#endif // WAYFARER_MULTIPLAYER_POC
