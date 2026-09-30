#ifndef GUARD_MULTIPLAYER_POC_H
#define GUARD_MULTIPLAYER_POC_H

#if WAYFARER_MULTIPLAYER_POC

#define MULTIPLAYER_POC_DIAG_MAGIC 0x504F4331 // "POC1"
#define MULTIPLAYER_POC_DIAG_VERSION 1
#define MULTIPLAYER_POC_PROTOCOL_VERSION 2
#define MULTIPLAYER_POC_PACKET_MAGIC 0x5750 // "WP"
#define LINKTYPE_WAYFARER_POC 0x7711

enum MultiplayerPocState
{
    MULTIPLAYER_POC_IDLE,
    MULTIPLAYER_POC_CONNECTING,
    MULTIPLAYER_POC_ACTIVE,
    MULTIPLAYER_POC_FAILED,
};

enum MultiplayerPocError
{
    MULTIPLAYER_POC_ERROR_NONE,
    MULTIPLAYER_POC_ERROR_CONNECT_TIMEOUT,
    MULTIPLAYER_POC_ERROR_TRANSPORT,
    MULTIPLAYER_POC_ERROR_LINK_TYPE,
    MULTIPLAYER_POC_ERROR_PROTOCOL,
    MULTIPLAYER_POC_ERROR_PEER_TIMEOUT,
    MULTIPLAYER_POC_ERROR_PLAYER_COUNT,
};

// Fixed 32-bit fields make this diagnostic readable from an ordinary emulator's
// symbol table and memory bus. It is not part of the multiplayer protocol.
struct MultiplayerPocDiag
{
    u32 magic;
    u32 version;
    u32 size;
    u32 state;
    u32 error;
    u32 localId;
    u32 peerId;
    u32 localMap; // mapGroup << 8 | mapNum
    u32 peerMap;
    s32 localX;
    s32 localY;
    s32 peerX;
    s32 peerY;
    u32 localFacing;
    u32 peerFacing;
    u32 txCount;
    u32 rxCount;
    u32 lastCmd;
    u32 peerVisible;
    u32 frame;
};

extern volatile struct MultiplayerPocDiag gMultiplayerPocDiag;

bool8 MultiplayerPoc_TryToggle(void);
void MultiplayerPoc_Update(void);
void MultiplayerPoc_BuildSendCmd(u16 *cmd);
bool8 MultiplayerPoc_ShouldSendCmd(void);
void MultiplayerPoc_ReceiveCmd(u8 playerId, const u16 *cmd);
bool8 MultiplayerPoc_IsRunning(void);
void MultiplayerPoc_BattleDetach(void);

#endif

#endif // GUARD_MULTIPLAYER_POC_H
