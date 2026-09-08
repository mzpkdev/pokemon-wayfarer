#include "global.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_interface.h"
#include "battle_message.h"
#include "battle_scripts.h"
#include "battle_util.h"
#include "pokemon.h"
#include "random.h"
#include "trainer_rating.h"
#include "trainer_see.h"
#include "trainer_only_encounter.h"
#include "wayfarer_origin.h"
#include "window.h"
#include "wayfarer_loss_policy.h"
#include "event_data.h"
#include "pokeball.h"
#include "constants/battle_anim.h"
#include "text.h"
#include "sprite.h"
#include "sound.h"
#include "constants/songs.h"
#include "item.h"
#include "item_menu.h"
#include "party_menu.h"
#include "constants/battle_string_ids.h"

static const u8 sText_ReadyToAttack[] = _("The wild POKéMON is ready to attack!");
static const u8 sText_CreptCloser[] = _("You crept closer to the POKéMON.");
static const u8 sText_Closest[] = _("You can’t get any closer!");
static const u8 sText_Eating[] = _("The wild POKéMON calmed down.\nIt is eating the Berry.");
static const u8 sText_ProtectionRestored[] = _("Your POKéMON can protect you again!");
static const u8 sText_UsedItem[] = _("You used the item.");
static const u8 sText_CannotEscape[] = _("Can't escape!");
static const u8 sText_KnockedOut[] = _("The wild POKéMON was knocked out!");
static const u8 sText_Angry[] = _("The wild POKéMON is angry!");
static const u8 sText_Attacked[] = _("The wild POKéMON attacked you!");
static const u8 sText_WildFled[] = _("The wild POKéMON fled!");

static const u8 sText_Nervous[] = _("The wild POKéMON watches nervously.");
static const u8 sText_GrowingAngry[] = _("The wild POKéMON is getting angry!");
static const u8 sTrainerOnlyEscaped[] = _("Got away safely!");

static EWRAM_DATA struct TrainerOnlyState sState = {0};
static EWRAM_DATA u8 sPhase = 0;
static EWRAM_DATA u8 sAction = 0;
static EWRAM_DATA u16 sMessageTimer = 0;
static EWRAM_DATA u8 sRetaliationFrames = 0;

u32 TrainerOnlyRockDamage(u32 cap, u32 level, u32 maxHp, u32 hp)
{
    u32 numerator = max(1, cap), denominator = 8 * max(1, level);
    if (20 * numerator < denominator) { numerator = 1; denominator = 20; }
    else if (5 * numerator > denominator) { numerator = 1; denominator = 5; }
    return min(hp, max(1, (maxHp * numerator + denominator - 1) / denominator));
}
u32 TrainerOnlyPassiveAnger(u32 cap, u32 level)
{
    cap = max(1, cap);
    return min(15, max(3, (5 * max(1, level) + cap - 1) / cap));
}
u32 TrainerOnlyFleeChance(const struct TrainerOnlyState *state)
{
    s32 chance = 5 * state->escapeFactor + 5 * state->rocks - (state->foodTurns ? 10 : 0);
    return min(75, max(5, chance));
}
u32 TrainerOnlyEscapeChance(u32 cap, u32 level, u32 failures)
{
    return min(95, max(5, 50 * max(1, cap) / max(1, level) + 15 * min(255, failures)));
}
bool32 TrainerOnlyRunSucceeds(u32 chance, bool32 lessEscapes, u32 roll)
{
    return (!lessEscapes || !(roll & 512)) && roll % 100 < chance;
}
u32 TrainerOnlyWarningThreshold(u32 passiveAnger) { return min(75, 100 - (25 + passiveAnger)); }
void TrainerOnlyApplyApproach(struct TrainerOnlyState *state)
{
    CaptureAdvanceProximity(&state->catchFactor, &state->escapeFactor, &state->approach);
}

void TrainerOnlyApplyFood(struct TrainerOnlyState *state)
{
    state->anger = state->anger > 20 ? state->anger - 20 : 0;
    state->foodTurns = 3;
}
u32 TrainerOnlyResolveSurvivingTurn(struct TrainerOnlyState *state, u32 fleeRoll)
{
    state->anger = min(100, state->anger + TrainerOnlyPassiveAnger(state->cap, state->level));
    if (state->anger >= 100) return TRAINER_ONLY_RETALIATION;
    if (fleeRoll < TrainerOnlyFleeChance(state)) return TRAINER_ONLY_FLED;
    if (state->foodTurns) state->foodTurns--;
    state->completedTurns = min(255, state->completedTurns + 1);
    return TRAINER_ONLY_ONGOING;
}
bool32 IsTrainerOnlyEncounter(void) { return sState.active; }
void TrainerOnlyResetEncounter(void) { memset(&sState, 0, sizeof(sState)); sPhase = 0; }
void TrainerOnlyPrepareEncounter(void)
{
    TrainerOnlyResetEncounter();
    if (!TrainerOnlyCanEnterWildEncounter()) return;
    sState.active = TRUE;
    sState.cap = max(1, GetTrainerRatingSoftLevelCap());
    sState.level = max(1, GetMonData(&gEnemyParty[0], MON_DATA_LEVEL));
    sState.initialCatchFactor = CaptureInitialCatchFactor(gSpeciesInfo[GetMonData(&gEnemyParty[0], MON_DATA_SPECIES)].catchRate);
    sState.catchFactor = sState.initialCatchFactor;
    sState.escapeFactor = 3;
}
void TrainerOnlyGetCaptureContext(struct CaptureContext *context)
{
    context->hasPlayerBattler = FALSE;
    context->playerBattler = MAX_BATTLERS_COUNT;
    context->completedTurns = sState.completedTurns;
    context->initialCatchFactor = sState.initialCatchFactor;
    context->catchFactor = sState.catchFactor;
}
void TrainerOnlyCommitItem(enum TrainerOnlyAction action) { sAction = action; }

static void Message(const u8 *text, u8 next)
{
    BattlePutTextOnWindow(text, B_WIN_MSG);
    sMessageTimer = 90;
    sPhase = next;
}
static void EndEncounter(u8 outcome)
{
    gBattleOutcome = outcome;
    gCurrentActionFuncId = B_ACTION_FINISHED;
    TrainerOnlyFinishBattle();
}
static void TrainerOnlyMain(void)
{
    u32 damage, passive, escapeRoll;
    enum BattlerId player = GetBattlerAtPosition(B_POSITION_PLAYER_LEFT);
    enum BattlerId wild = GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT);
    if (gBattleControllerExecFlags) return;
    if (sMessageTimer)
    {
        if (!IsTextPrinterActiveOnWindow(B_WIN_MSG)) sMessageTimer--;
        return;
    }
    switch (sPhase)
    {
    case 0:
        passive = TrainerOnlyPassiveAnger(sState.cap, sState.level);
        if (sState.anger >= TrainerOnlyWarningThreshold(passive) && !sState.warned)
        {
            sState.warned = TRUE;
            Message(sText_ReadyToAttack, 0);
            return;
        }
        if (sState.anger < TrainerOnlyWarningThreshold(passive)) sState.warned = FALSE;
        BtlController_EmitChooseAction(player, B_COMM_TO_CONTROLLER, 0, ITEM_NONE);
        MarkBattlerForControllerExec(player);
        sPhase = 1;
        break;
    case 1:
        sAction = gBattleResources->bufferB[player][1];
        if (sAction == TRAINER_ONLY_BALL)
        {
            gSpecialVar_ItemId = ITEM_NONE;
            BtlController_EmitChooseItem(player, B_COMM_TO_CONTROLLER, gBattlePartyCurrentOrder);
            MarkBattlerForControllerExec(player);
            sPhase = 2;
            return;
        }
        sPhase = 3;
        break;
    case 2:
        if (gSpecialVar_ItemId == ITEM_NONE) { sPhase = 0; return; }
        sPhase = 3;
        break;
    case 3:
        gBattle_BG0_X = gBattle_BG0_Y = 0;
        gBattlerAttacker = player;
        gBattlerTarget = wild;
        switch (sAction)
        {
        case TRAINER_ONLY_ROCK:
            BtlController_EmitBattleAnimation(player, B_COMM_TO_CONTROLLER, B_ANIM_ROCK_THROW, 0);
            MarkBattlerForControllerExec(player);
            sPhase = 9;
            return;
        case TRAINER_ONLY_NEAR:
            damage = sState.approach;
            TrainerOnlyApplyApproach(&sState);
            Message(damage < 3 ? sText_CreptCloser : sText_Closest, 5);
            return;
        case TRAINER_ONLY_BALL:
            gLastUsedItem = gSpecialVar_ItemId;
            gBallToDisplay = gLastUsedItem;
            gBattlescriptCurrInstr = BattleScript_BallThrow;
            gCurrentActionFuncId = B_ACTION_EXEC_SCRIPT;
            sPhase = 4;
            return;
        case TRAINER_ONLY_FOOD:
            TrainerOnlyApplyFood(&sState);
            Message(sText_Eating, 5);
            return;
        case TRAINER_ONLY_RECOVERY:
#if IS_WAYFARER
            if (WayfarerCanStartOrdinaryBattle())
            {
                ArmTrainerRecoverySightSuppression();
                Message(sText_ProtectionRestored, 7);
                return;
            }
#endif
            Message(sText_UsedItem, 5);
            return;
        case TRAINER_ONLY_RUN:
            if (FlagGet(B_FLAG_NO_RUNNING))
            {
                Message(sText_CannotEscape, 0);
                return;
            }
            escapeRoll = Random();
            if (TrainerOnlyRunSucceeds(TrainerOnlyEscapeChance(sState.cap, sState.level, sState.escapeAttempts),
                                       gSaveBlock3Ptr->challengeSettings.tx_Challenges_LessEscapes, escapeRoll))
            {
                Message(sTrainerOnlyEscaped, 7);
                return;
            }
            Message(sText_CannotEscape, 5);
            sState.escapeAttempts = min(255, sState.escapeAttempts + 1);
            return;
        }
        break;
    case 9:
            damage = TrainerOnlyRockDamage(sState.cap, sState.level, gBattleMons[wild].maxHP, gBattleMons[wild].hp);
            gBattleMons[wild].hp -= damage;
            SetMonData(&gEnemyParty[0], MON_DATA_HP, &gBattleMons[wild].hp);
            UpdateHealthboxAttribute(gHealthboxSpriteIds[wild], &gEnemyParty[0], HEALTHBOX_ALL);
            if (!gBattleMons[wild].hp) { Message(sText_KnockedOut, 7); return; }
            sState.rocks = min(14, sState.rocks + 1);
            sState.anger = min(100, sState.anger + 25);
            // Reuse only the anger-mark presentation. Emitting on the real wild
            // battler anchors both animation positions to its sprite.
            gBattleCommunication[MULTISTRING_CHOOSER] = B_MSG_MON_ANGRY;
            BtlController_EmitBattleAnimation(wild, B_COMM_TO_CONTROLLER, B_ANIM_SAFARI_REACTION, 0);
            MarkBattlerForControllerExec(wild);
            sPhase = 11;
            return;
    case 11:
        Message(sText_Angry, 5);
        return;
    case 4:
        HandleAction_RunBattleScript();
        if (gCurrentActionFuncId != B_ACTION_EXEC_SCRIPT)
        {
            if (gBattleOutcome) { EndEncounter(gBattleOutcome); return; }
            sPhase = 5;
        }
        break;
    case 5:
        // Retaliation is resolved before requesting the single wild-flee roll.
        passive = sState.anger;
        damage = TrainerOnlyPassiveAnger(sState.cap, sState.level);
        damage = TrainerOnlyResolveSurvivingTurn(&sState, sState.anger + damage >= 100 ? 100 : Random() % 100);
        if (damage == TRAINER_ONLY_RETALIATION) { Message(sText_Attacked, 8); return; }
        if (damage == TRAINER_ONLY_FLED) { Message(sText_WildFled, 6); return; }
        if (passive < 30 && sState.anger >= 30 && sAction != TRAINER_ONLY_ROCK)
        {
            Message(sText_GrowingAngry, 0);
            return;
        }
        sPhase = 0;
        break;
    case 6: EndEncounter(B_OUTCOME_MON_FLED); break;
    case 7: EndEncounter(B_OUTCOME_RAN); break;
    case 8:
        sRetaliationFrames = 24;
        PlaySE(SE_M_DOUBLE_SLAP);
        sPhase = 10;
        break;
    case 10:
        if (sRetaliationFrames)
        {
            gSprites[gBattlerSpriteIds[player]].x2 = (sRetaliationFrames & 2) ? 4 : -4;
            gSprites[gBattlerSpriteIds[wild]].x2 = -min(16, sRetaliationFrames);
            sRetaliationFrames--;
            return;
        }
        gSprites[gBattlerSpriteIds[player]].x2 = 0;
        gSprites[gBattlerSpriteIds[wild]].x2 = 0;
        WayfarerMarkTrainerRetaliation();
        EndEncounter(B_OUTCOME_LOST);
        break;
    }
}
void TrainerOnlyStartController(void)
{
    sPhase = 0;
    sMessageTimer = 0;
    gBattleTurnCounter = 0;
    gBattleMainFunc = TrainerOnlyMain;
    Message(sText_Nervous, 0);
}

#if E2E_TESTING
void TrainerOnlyReadState(struct TrainerOnlyState *out) { *out = sState; }
bool32 TrainerOnlyIsActionMenu(void) { return sState.active && sPhase == 1; }
#endif
