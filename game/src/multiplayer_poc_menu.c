#include "global.h"

#if WAYFARER_MULTIPLAYER_POC
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "main.h"
#include "map_name_popup.h"
#include "menu.h"
#include "multiplayer_poc.h"
#include "multiplayer_poc_battle.h"
#include "multiplayer_poc_menu.h"
#include "multiplayer_poc_reward.h"
#include "overworld.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

enum MultiplayerMenuAction
{
    ACTION_CONNECT,
    ACTION_INVITE,
    ACTION_ACCEPT,
    ACTION_DECLINE,
    ACTION_CANCEL,
    ACTION_LEAVE,
    ACTION_CLAIM,
    ACTION_BACK,
};

static const u8 sText_Title[] = _("MULTIPLAYER");
static const u8 sText_Offline[] = _("Not connected");
static const u8 sText_Connecting[] = _("Connecting...");
static const u8 sText_Connected[] = _("Connected");
static const u8 sText_Suspending[] = _("Disconnecting to save...");
static const u8 sText_Saving[] = _("Saving locally...");
static const u8 sText_Reconnecting[] = _("Reconnecting...");
static const u8 sText_Battle[] = _("Shared battle in progress");
static const u8 sText_Failed[] = _("Connection ended");
static const u8 sText_ConnectBoth[] = _("Choose Connect on both games.");
static const u8 sText_Wait[] = _("Waiting for your friend...");
static const u8 sText_Nearby[] = _("Your friend is nearby.");
static const u8 sText_OtherMap[] = _("Your friend is on another map.");
static const u8 sText_Busy[] = _("Your friend is busy.");
static const u8 sText_Incoming[] = _("Your friend invited you to battle.");
static const u8 sText_Rentals[] = _("Team up against NPCs with rental teams.");
static const u8 sText_Outgoing[] = _("Waiting for your friend to accept.");
static const u8 sText_Starting[] = _("Preparing your shared battle...");
static const u8 sText_Mismatch[] = _("Use the same built ROM on both games.");
static const u8 sText_Timeout[] = _("No response. Try connecting again.");
static const u8 sText_LinkLost[] = _("Check the connection and try again.");
static const u8 sText_Unavailable[] = _("Your friend must be here and ready.");
static const u8 sText_Connect[] = _("Connect");
static const u8 sText_Invite[] = _("Invite to co-op battle");
static const u8 sText_Accept[] = _("Accept invitation");
static const u8 sText_Decline[] = _("Decline invitation");
static const u8 sText_Cancel[] = _("Cancel invitation");
static const u8 sText_Leave[] = _("Leave session");
static const u8 sText_Claim[] = _("Claim pending reward");
static const u8 sText_Back[] = _("Back");

static const u8 *const sActionTexts[] =
{
    [ACTION_CONNECT] = sText_Connect,
    [ACTION_INVITE] = sText_Invite,
    [ACTION_ACCEPT] = sText_Accept,
    [ACTION_DECLINE] = sText_Decline,
    [ACTION_CANCEL] = sText_Cancel,
    [ACTION_LEAVE] = sText_Leave,
    [ACTION_CLAIM] = sText_Claim,
    [ACTION_BACK] = sText_Back,
};

static const struct WindowTemplate sWindow =
{
    .bg = 0, .tilemapLeft = 1, .tilemapTop = 2,
    .width = 28, .height = 17, .paletteNum = 15, .baseBlock = 8,
};

static EWRAM_INIT u8 sWindowId = WINDOW_NONE;
static EWRAM_DATA u8 sActions[4];
static EWRAM_DATA u8 sActionCount;
static EWRAM_DATA u32 sLastView;
static EWRAM_DATA bool8 sActionFailed;
static EWRAM_DATA bool8 sIncomingShown;

static void Task_Menu(u8 taskId);
static void Task_Action(u8 taskId);

static bool8 FieldReady(void)
{
    return gMain.callback2 == CB2_Overworld && !gPaletteFade.active
        && !ArePlayerFieldControlsLocked() && IsPlayerStandingStill();
}

static u32 ViewKey(void)
{
    return MultiplayerPoc_GetStatus()
        | (MultiplayerPoc_GetError() << 4)
        | (MultiplayerPocBattle_GetInviteStatus() << 8)
        | (gMultiplayerPocDiag.peerVisible << 12)
        | ((gMultiplayerPocDiag.peerMap != gMultiplayerPocDiag.localMap) << 13)
        | (sActionFailed << 14)
        | (MultiplayerPocReward_GetState() << 16);
}

static void AddAction(u8 action)
{
    if (sActionCount < ARRAY_COUNT(sActions))
        sActions[sActionCount++] = action;
}

static void DrawMenu(void)
{
    enum MultiplayerPocSessionStatus status = MultiplayerPoc_GetStatus();
    enum MultiplayerPocInviteStatus invite = MultiplayerPocBattle_GetInviteStatus();
    enum MultiplayerPocError error = MultiplayerPoc_GetError();
    const u8 *stateText = sText_Offline;
    const u8 *detail = sText_ConnectBoth;
    u8 i;

    sActionCount = 0;
    switch (status)
    {
    case MULTIPLAYER_POC_SESSION_CONNECTING:
        stateText = sText_Connecting;
        detail = sText_Wait;
        break;
    case MULTIPLAYER_POC_SESSION_ACTIVE:
        stateText = sText_Connected;
        detail = gMultiplayerPocDiag.peerMap != gMultiplayerPocDiag.localMap
            ? sText_OtherMap : gMultiplayerPocDiag.peerVisible ? sText_Nearby : sText_Busy;
        break;
    case MULTIPLAYER_POC_SESSION_SUSPENDING:
        stateText = sText_Suspending;
        detail = sText_Wait;
        break;
    case MULTIPLAYER_POC_SESSION_SAVE_HOLD:
        stateText = sText_Saving;
        detail = sText_Wait;
        break;
    case MULTIPLAYER_POC_SESSION_RECONNECTING:
        stateText = sText_Reconnecting;
        detail = sText_Wait;
        break;
    case MULTIPLAYER_POC_SESSION_BATTLE_HOLD:
        stateText = sText_Battle;
        detail = sText_Wait;
        break;
    case MULTIPLAYER_POC_SESSION_FAILED:
        stateText = sText_Failed;
        detail = error == MULTIPLAYER_POC_ERROR_BUILD_MISMATCH || error == MULTIPLAYER_POC_ERROR_PROTOCOL
            ? sText_Mismatch : error == MULTIPLAYER_POC_ERROR_SESSION_TIMEOUT
                || error == MULTIPLAYER_POC_ERROR_CONNECT_TIMEOUT
            ? sText_Timeout : sText_LinkLost;
        break;
    case MULTIPLAYER_POC_SESSION_IDLE:
        break;
    }
    if (status == MULTIPLAYER_POC_SESSION_IDLE || status == MULTIPLAYER_POC_SESSION_FAILED)
    {
        AddAction(ACTION_CONNECT);
        if (!MultiplayerPoc_LinkIsOpen() && MultiplayerPocReward_GetState() == MULTIPLAYER_POC_REWARD_PENDING)
            AddAction(ACTION_CLAIM);
    }
    else
    {
        if (status == MULTIPLAYER_POC_SESSION_ACTIVE)
        {
            switch (invite)
            {
            case POC_INVITE_INCOMING:
                sIncomingShown = TRUE;
                detail = sText_Incoming;
                AddAction(ACTION_ACCEPT);
                AddAction(ACTION_DECLINE);
                break;
            case POC_INVITE_OUTGOING:
                detail = sText_Outgoing;
                AddAction(ACTION_CANCEL);
                break;
            case POC_INVITE_STARTING:
                detail = sText_Starting;
                AddAction(ACTION_CANCEL);
                break;
            case POC_INVITE_NONE:
                AddAction(ACTION_INVITE);
                break;
            }
        }
        AddAction(ACTION_LEAVE);
    }
    AddAction(ACTION_BACK);
    if (sActionFailed)
        detail = sText_Unavailable;

    FillWindowPixelBuffer(sWindowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(sWindowId, FONT_NORMAL, sText_Title, 8, 0, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sWindowId, FONT_NORMAL, stateText, 8, 18, TEXT_SKIP_DRAW, NULL);
    AddTextPrinterParameterized(sWindowId, FONT_SMALL, detail, 8, 36, TEXT_SKIP_DRAW, NULL);
    if (invite != POC_INVITE_NONE)
        AddTextPrinterParameterized(sWindowId, FONT_SMALL, sText_Rentals, 8, 48, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < sActionCount; i++)
        AddTextPrinterParameterized(sWindowId, FONT_NORMAL, sActionTexts[sActions[i]], 12, 62 + i * 16, TEXT_SKIP_DRAW, NULL);
    InitMenuNormal(sWindowId, FONT_NORMAL, 0, 62, 16, sActionCount, 0);
    CopyWindowToVram(sWindowId, COPYWIN_FULL);
    sLastView = ViewKey();
}

void MultiplayerPocMenu_Open(void)
{
    u8 taskId;

    if (sWindowId != WINDOW_NONE || FuncIsActiveTask(Task_Action)
     || GetTaskCount() >= NUM_TASKS || !FieldReady())
        return;
    HideMapNamePopUpWindow();
    sWindowId = AddWindow(&sWindow);
    if (sWindowId == WINDOW_NONE)
        return;
    taskId = CreateTask(Task_Menu, 0x50);
    gTasks[taskId].data[2] = TRUE;
    FreezeObjectEvents();
    PlayerFreeze();
    StopPlayerAvatar();
    LockPlayerFieldControls();
    LoadMessageBoxAndBorderGfx();
    DrawStdWindowFrame(sWindowId, FALSE);
    DrawMenu();
}

static void CloseMenu(void)
{
    ClearStdWindowAndFrameToTransparent(sWindowId, TRUE);
    RemoveWindow(sWindowId);
    sWindowId = WINDOW_NONE;
    ScriptUnfreezeObjectEvents();
    UnlockPlayerFieldControls();
}

static void Task_Menu(u8 taskId)
{
    s8 input;
    u8 action;

    if (ViewKey() != sLastView)
    {
        DrawMenu();
        gTasks[taskId].data[2] = TRUE;
        return; // Never reinterpret an A press against a newly changed menu.
    }
    // An invitation may arrive on the same frame as an unrelated A press.
    // Require release after opening or replacing choices before accepting input.
    if (gTasks[taskId].data[2])
    {
        if (!(gMain.heldKeys & (A_BUTTON | B_BUTTON | DPAD_ANY)))
            gTasks[taskId].data[2] = FALSE;
        return;
    }
    input = Menu_ProcessInput();
    if (input == MENU_NOTHING_CHOSEN)
        return;
    action = input == MENU_B_PRESSED ? ACTION_BACK : sActions[input];
    PlaySE(SE_SELECT);
    CloseMenu();
    if (action == ACTION_BACK)
    {
        sActionFailed = FALSE;
        DestroyTask(taskId);
        return;
    }
    gTasks[taskId].data[0] = action;
    gTasks[taskId].data[1] = 0;
    gTasks[taskId].func = Task_Action;
}

static void Task_Action(u8 taskId)
{
    bool8 accepted = TRUE;
    u8 action = gTasks[taskId].data[0];

    // A later field update owns the transition, after window/task cleanup.
    if (++gTasks[taskId].data[1] == 1)
        return;
    if (!FieldReady())
    {
        if (gTasks[taskId].data[1] > 120)
            DestroyTask(taskId);
        return;
    }
    DestroyTask(taskId);
    switch (action)
    {
    case ACTION_CONNECT: accepted = MultiplayerPoc_Connect(); break;
    case ACTION_INVITE: accepted = MultiplayerPocBattle_RequestInvite(); break;
    case ACTION_ACCEPT: accepted = MultiplayerPocBattle_AcceptInvite(); break;
    case ACTION_DECLINE: accepted = MultiplayerPocBattle_DeclineInvite(); break;
    case ACTION_CANCEL: accepted = MultiplayerPocBattle_CancelInvite(); break;
    case ACTION_LEAVE: MultiplayerPoc_Leave(); break;
    case ACTION_CLAIM:
        if (!MultiplayerPoc_LinkIsOpen())
            MultiplayerPocReward_TryClaimPending();
        break;
    default: break;
    }
    sActionFailed = !accepted;
    if (!accepted)
        MultiplayerPocMenu_Open();
}

void MultiplayerPocMenu_Update(void)
{
    enum MultiplayerPocInviteStatus invite = MultiplayerPocBattle_GetInviteStatus();

    if (invite != POC_INVITE_INCOMING)
        sIncomingShown = FALSE;
    else if (!sIncomingShown && FieldReady()
          && sWindowId == WINDOW_NONE && !FuncIsActiveTask(Task_Action))
    {
        sActionFailed = FALSE;
        MultiplayerPocMenu_Open();
        sIncomingShown = sWindowId != WINDOW_NONE;
    }
}
#endif
