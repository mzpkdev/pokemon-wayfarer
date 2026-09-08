#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "data.h"
#include "item_menu.h"
#include "main.h"
#include "palette.h"
#include "party_menu.h"
#include "constants/party_menu.h"
#include "reshow_battle_screen.h"
#include "sound.h"
#include "text.h"
#include "trainer_only_encounter.h"
#include "window.h"
#include "constants/trainers.h"
#include "constants/songs.h"
#include "constants/rgb.h"

static const u8 sText_Menu[] = _("Rock{CLEAR_TO 56}Bag\nGo Near{CLEAR_TO 56}Run");
static const u8 sText_Prompt[] = _("What will you do?");

static void RunCommand(enum BattlerId battler);
void TrainerOnlyBufferExecCompleted(enum BattlerId battler)
{
    gBattlerControllerFuncs[battler] = RunCommand;
    MarkBattleControllerIdleOnLocal(battler);
}
static void ChooseAction(enum BattlerId battler)
{
    u8 cursor = gActionSelectionCursor[battler];
    if (HandleLastUsedBallCycleInput(battler, TRAINER_ONLY_QUICK_BALL))
        return;
    if (B_LAST_USED_BALL == TRUE && B_LAST_USED_BALL_CYCLE == FALSE
        && JOY_NEW(B_LAST_USED_BALL_BUTTON) && CanThrowLastUsedBall())
    {
        PlaySE(SE_SELECT);
        TryHideLastUsedBall();
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, TRAINER_ONLY_QUICK_BALL, 0);
        BtlController_Complete(battler);
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        static const u8 actions[] = {TRAINER_ONLY_ROCK, TRAINER_ONLY_BALL, TRAINER_ONLY_NEAR, TRAINER_ONLY_RUN};
        PlaySE(SE_SELECT);
        TryHideLastUsedBall();
        BtlController_EmitTwoReturnValues(battler, B_COMM_TO_ENGINE, actions[cursor], 0);
        BtlController_Complete(battler);
        return;
    }
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT)) cursor ^= 1;
    if (JOY_NEW(DPAD_UP | DPAD_DOWN)) cursor ^= 2;
    if (JOY_NEW(B_BUTTON)) cursor = 3;
    if (cursor != gActionSelectionCursor[battler])
    {
        PlaySE(SE_SELECT);
        ActionSelectionDestroyCursorAt(gActionSelectionCursor[battler]);
        gActionSelectionCursor[battler] = cursor;
        ActionSelectionCreateCursorAt(cursor, 0);
    }
}
static void WaitForBag(enum BattlerId battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        BtlController_EmitOneReturnValue(battler, B_COMM_TO_ENGINE, gSpecialVar_ItemId);
        BtlController_Complete(battler);
    }
}
static void OpenBag(enum BattlerId battler)
{
    if (!gPaletteFade.active)
    {
        gBattlerControllerFuncs[battler] = WaitForBag;
        ReshowBattleScreenDummy();
        FreeAllWindowBuffers();
        GoToBagMenu(ITEMMENULOCATION_BATTLE, POCKET_POKE_BALLS, CB2_SetUpReshowBattleScreenAfterMenu2);
    }
}
static void WaitForPartySelection(enum BattlerId battler)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade.active)
    {
        // PARTY_ACTION_SEND_MON_TO_BOX writes the selected slot (or its
        // cancellation sentinel) directly. It does not use the item-menu
        // callback flag, so forwarding that flag would turn every selection
        // into a cancellation.
        BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, gSelectedMonPartyId, gBattlePartyCurrentOrder);
        BtlController_Complete(battler);
    }
}
static void OpenCapturePartyMenu(enum BattlerId battler)
{
    if (!gPaletteFade.active)
    {
        gBattlerControllerFuncs[battler] = WaitForPartySelection;
        FreeAllWindowBuffers();
        OpenPartyMenuInBattle(PARTY_ACTION_SEND_MON_TO_BOX);
    }
}
static void ChooseCapturePartyMon(enum BattlerId battler)
{
    // The capture delivery script owns swaps and cancellation; this controller
    // only selects an actual party slot, never an active trainer Pokemon.
    for (u32 i = 0; i < ARRAY_COUNT(gBattlePartyCurrentOrder); i++)
        gBattlePartyCurrentOrder[i] = gBattleResources->bufferA[battler][4 + i];
    gBattleStruct->battlerPreventingSwitchout = 0;
    gBattleStruct->prevSelectedPartySlot = gBattleResources->bufferA[battler][2];
    gBattleStruct->abilityPreventingSwitchout = ABILITY_NONE;
    gBattlerInMenuId = battler;
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gBattlerControllerFuncs[battler] = OpenCapturePartyMenu;
}
static void RunCommand(enum BattlerId battler)
{
    if (!IsBattleControllerActiveOnLocal(battler)) return;
    switch (gBattleResources->bufferA[battler][0])
    {
    case CONTROLLER_DRAWTRAINERPIC:
    {
        enum TrainerPicID pic = gSaveBlock2Ptr->playerGender == FEMALE ? TRAINER_BACK_PIC_PLAYER_FEMALE : TRAINER_BACK_PIC_PLAYER_MALE;
        BtlController_HandleDrawTrainerPic(battler, pic, FALSE, 80, 80 + 4 * (8 - gTrainerBacksprites[pic].coordinates.size), 30);
        break;
    }
    case CONTROLLER_CHOOSEACTION:
        BattlePutTextOnWindow(sText_Menu, B_WIN_ACTION_MENU);
        BattlePutTextOnWindow(sText_Prompt, B_WIN_ACTION_PROMPT);
        for (u32 i = 0; i < 4; i++) ActionSelectionDestroyCursorAt(i);
        TryRestoreLastUsedBall();
        ActionSelectionCreateCursorAt(gActionSelectionCursor[battler], 0);
        gBattle_BG0_X = 0;
        gBattle_BG0_Y = DISPLAY_HEIGHT;
        gBattlerControllerFuncs[battler] = ChooseAction;
        break;
    case CONTROLLER_OPENBAG:
        gBattlerInMenuId = battler;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gBattlerControllerFuncs[battler] = OpenBag;
        break;
    case CONTROLLER_CHOOSEPOKEMON:
        if ((gBattleResources->bufferA[battler][1] & 0xF) == PARTY_ACTION_SEND_MON_TO_BOX)
            ChooseCapturePartyMon(battler);
        else
        {
            gSelectedMonPartyId = PARTY_SIZE + 1;
            BtlController_EmitChosenMonReturnValue(battler, B_COMM_TO_ENGINE, PARTY_SIZE, NULL);
            BtlController_Complete(battler);
        }
        break;
    case CONTROLLER_BALLTHROWANIM: BtlController_HandleBallThrowAnim(battler); break;
    case CONTROLLER_PRINTSTRING: BtlController_HandlePrintString(battler); break;
    case CONTROLLER_PRINTSTRINGPLAYERONLY: BtlController_HandlePrintStringPlayerOnly(battler); break;
    case CONTROLLER_PLAYSE: BtlController_HandlePlaySE(battler); break;
    case CONTROLLER_PLAYFANFAREORBGM: BtlController_HandlePlayFanfareOrBGM(battler); break;
    case CONTROLLER_INTROSLIDE: BtlController_HandleIntroSlide(battler); break;
    case CONTROLLER_BATTLEANIMATION: BtlController_HandleBattleAnimation(battler); break;
    default: BtlController_Complete(battler); break;
    }
}
void SetControllerToTrainerOnly(enum BattlerId battler)
{
    gBattlerBattleController[battler] = BATTLE_CONTROLLER_TRAINER_ONLY;
    gBattlerControllerEndFuncs[battler] = TrainerOnlyBufferExecCompleted;
    gBattlerControllerFuncs[battler] = RunCommand;
}

#if E2E_TESTING
bool32 E2ETest_GetTrainerOnlyActionMenuState(u8 *cursor)
{
    enum BattlerId player = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
    if (!IsTrainerOnlyEncounter() || gBattlerControllerFuncs[player] != ChooseAction) return FALSE;
    *cursor = gActionSelectionCursor[player];
    return TRUE;
}
#endif
