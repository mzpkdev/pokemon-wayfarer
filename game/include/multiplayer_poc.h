#ifndef GUARD_MULTIPLAYER_POC_H
#define GUARD_MULTIPLAYER_POC_H

#if WAYFARER_MULTIPLAYER_POC

#define MULTIPLAYER_POC_DIAG_MAGIC 0x504F4331 // "POC1"
#define MULTIPLAYER_POC_DIAG_VERSION 1
#define MULTIPLAYER_POC_PROTOCOL_VERSION 3
#define MULTIPLAYER_POC_PACKET_MAGIC 0x5750 // "WP"
#define LINKTYPE_WAYFARER_POC 0x7711
#define MULTIPLAYER_POC_SAVE_REQUEST_BIT (1 << 9)
#define MULTIPLAYER_POC_HELLO_ACK_BIT (1 << 10)
#define MULTIPLAYER_POC_HELLO_MARKER 0xA500
#define MULTIPLAYER_POC_SESSION_DIAG_MAGIC 0x53455331 // "SES1"

enum MultiplayerPocState
{
    MULTIPLAYER_POC_IDLE,
    MULTIPLAYER_POC_CONNECTING,
    MULTIPLAYER_POC_ACTIVE,
    MULTIPLAYER_POC_FAILED,
    MULTIPLAYER_POC_HELLO,
    MULTIPLAYER_POC_SUSPENDING,
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
    MULTIPLAYER_POC_ERROR_BUILD_MISMATCH,
    MULTIPLAYER_POC_ERROR_SESSION_TIMEOUT,
};

enum MultiplayerPocSessionStatus
{
    MULTIPLAYER_POC_SESSION_IDLE,
    MULTIPLAYER_POC_SESSION_CONNECTING,
    MULTIPLAYER_POC_SESSION_ACTIVE,
    MULTIPLAYER_POC_SESSION_SUSPENDING,
    MULTIPLAYER_POC_SESSION_SAVE_HOLD,
    MULTIPLAYER_POC_SESSION_RECONNECTING,
    MULTIPLAYER_POC_SESSION_BATTLE_HOLD,
    MULTIPLAYER_POC_SESSION_FAILED,
};

struct MultiplayerPocSessionDiag
{
    u32 magic;
    u32 version;
    u32 size;
    u32 status;
    u32 error;
    u32 wanted;
    u32 localSaveHold;
    u32 peerSaveRequested;
    u32 closeCount;
    u32 reconnectCount;
    u32 attemptCount;
    u32 helloPagesSeen;
    u32 helloPeerConfirmed;
    u32 buildMatch;
    u32 buildId[4];
};

// Patched after the ROM is built. The digest covers the complete ROM with
// these 16 bytes zeroed, including all compiled content and build settings.
struct MultiplayerPocBuildTag
{
    u8 marker[8];
    u8 id[16];
    u8 tail[4];
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
extern volatile struct MultiplayerPocSessionDiag gMultiplayerPocSessionDiag;
extern const volatile struct MultiplayerPocBuildTag gMultiplayerPocBuildTag;

bool8 MultiplayerPoc_TryToggle(void);
void MultiplayerPoc_Update(void);
void MultiplayerPoc_BuildSendCmd(u16 *cmd);
bool8 MultiplayerPoc_ShouldSendCmd(void);
void MultiplayerPoc_ReceiveCmd(u8 playerId, const u16 *cmd);
bool8 MultiplayerPoc_IsRunning(void);
void MultiplayerPoc_BattleDetach(void);
void MultiplayerPoc_BattleFinished(void);
bool8 MultiplayerPoc_Connect(void);
void MultiplayerPoc_Leave(void);
void MultiplayerPoc_BeginLocalSave(void);
bool8 MultiplayerPoc_IsLocalSaveReady(void);
void MultiplayerPoc_EndLocalSave(void);
enum MultiplayerPocSessionStatus MultiplayerPoc_GetStatus(void);
enum MultiplayerPocError MultiplayerPoc_GetError(void);
bool8 MultiplayerPoc_LinkIsOpen(void);

#endif

#endif // GUARD_MULTIPLAYER_POC_H
