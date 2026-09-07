#include "global.h"
#include "battle.h"
#include "data.h"
#include "debug.h"
#include "event_data.h"
#include "item.h"
#include "mom_savings.h"
#include "money.h"
#include "test/battle.h"
#include "trainer_party_scaling.h"
#include "trainer_rating.h"
#include "constants/opponents.h"

#if IS_WAYFARER

static void SeedScalingBattle(u32 rating, bool32 modern)
{
    gIsDebugBattle = FALSE;
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    SetTrainerRating(rating);
    ResetTrainerScalingSnapshot();
    gSaveBlock3Ptr->challengeSettings.tx_Mode_Modern_Moves = modern;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_LevelCap = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_ExpMultiplier = OPTIONS_EXP_MULTIPLIER_1X;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = FALSE;
}

static void ConstructCurrentOpponent(u16 trainerId)
{
    struct Pokemon *mon = gBattleTestRunnerState->data.currentMon;
    CreateNPCTrainerPartyForOpponent(mon, trainerId, FALSE, BATTLE_TYPE_TRAINER);
    u16 moves[MAX_MON_MOVES];
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        moves[i] = GetMonData(mon, MON_DATA_MOVE1 + i);
    Moves_(__LINE__, moves);
}

SINGLE_BATTLE_TEST("Trainer scaling generated moves execute at early middle and late Ratings")
{
    u32 rating = 0, modern = FALSE;
    PARAMETRIZE { rating = 0; modern = FALSE; }
    PARAMETRIZE { rating = 0; modern = TRUE; }
    PARAMETRIZE { rating = 40; modern = FALSE; }
    PARAMETRIZE { rating = 40; modern = TRUE; }
    PARAMETRIZE { rating = 80; modern = FALSE; }
    PARAMETRIZE { rating = 80; modern = TRUE; }
    GIVEN {
        ASSUME(B_TRAINER_PARTY_SCALING);
        SeedScalingBattle(rating, modern);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_RATTATA) {
            ConstructCurrentOpponent(TRAINER_JOEY_5_HNS);
        }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, moveSlot: 0); }
    } THEN {
        EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), rating == 0 ? 7 : rating == 40 ? 34 : 92);
        EXPECT_NE(GetMonData(&gEnemyParty[0], MON_DATA_MOVE1), MOVE_NONE);
    }
}

AI_SINGLE_BATTLE_TEST("Trainer scaling battle XP uses effective levels and money keeps authored levels", u32 initialExp)
{
    u32 rating = 0, level = 7, playerLevel = 10;
    PARAMETRIZE { rating = 0; level = 7; playerLevel = 10; }
    PARAMETRIZE { rating = 40; level = 34; playerLevel = 40; }
    PARAMETRIZE { rating = 80; level = 92; playerLevel = 90; }
    GIVEN {
        ASSUME(B_TRAINER_PARTY_SCALING);
        ASSUME(B_SCALED_EXP == GEN_3);
        ASSUME(B_TRAINER_EXP_MULTIPLIER == GEN_3);
        SeedScalingBattle(rating, TRUE);
        Mom_EnableSaving(FALSE);
        SetMoney(&gSaveBlock1Ptr->money, 0);
        RemoveBagItem(ITEM_EXP_CHARM, 99);
        gBattleTestRunnerState->data.recordedBattle.opponentA = TRAINER_JOEY_5_HNS;
        PLAYER(SPECIES_WOBBUFFET) {
            Level(playerLevel);
            u32 otId = READ_OTID_FROM_SAVE;
            SetMonData(gBattleTestRunnerState->data.currentMon, MON_DATA_OT_ID, &otId);
            SetMonData(gBattleTestRunnerState->data.currentMon, MON_DATA_OT_NAME, gSaveBlock2Ptr->playerName);
            results[i].initialExp = GetMonData(gBattleTestRunnerState->data.currentMon, MON_DATA_EXP);
        }
        OPPONENT(SPECIES_RATTATA) {
            ConstructCurrentOpponent(TRAINER_JOEY_5_HNS);
            HP(1);
        }
    } WHEN {
        TURN { MOVE(player, MOVE_EXTREME_SPEED); }
    } THEN {
        u32 expectedExp = (gSpeciesInfo[SPECIES_RATTATA].expYield * level / 7) * 3 / 2;
        u32 moneyRate = gTrainerClasses[TRAINER_CLASS_YOUNGSTER].money ?: 5;
        EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_EXP) - results[i].initialExp, expectedExp);
        EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 4 * 5 * moneyRate);
        EXPECT_EQ(GetTrainerStructFromId(TRAINER_JOEY_5_HNS)->party[0].lvl, 5);
    }
}

ONE_VS_TWO_BATTLE_TEST("Trainer scaling mixed Gym and boss doubles execute their constructed parties")
{
    GIVEN {
        ASSUME(B_TRAINER_PARTY_SCALING);
        SeedScalingBattle(0, TRUE);
        MULTI_PLAYER(SPECIES_WOBBUFFET);
        MULTI_PLAYER(SPECIES_WOBBUFFET);
        MULTI_OPPONENT_A(SPECIES_RATTATA) {
            ConstructCurrentOpponent(TRAINER_ROD_HNS);
        }
        MULTI_OPPONENT_B(SPECIES_CHARIZARD) {
            ConstructCurrentOpponent(TRAINER_FALKNER_1_HNS);
        }
    } WHEN {
        TURN {
            MOVE(playerLeft, MOVE_CELEBRATE);
            MOVE(playerRight, MOVE_CELEBRATE);
            MOVE(opponentLeft, moveSlot: 0, target: playerLeft);
            MOVE(opponentRight, MOVE_HYPER_BEAM, target: playerRight);
        }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_HYPER_BEAM, opponentRight);
        HP_BAR(playerRight);
    } THEN {
        EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 9);
        EXPECT_EQ(GetMonData(&gEnemyParty[3], MON_DATA_LEVEL), 60);
        EXPECT_EQ(GetMonData(&gEnemyParty[3], MON_DATA_SPECIES), SPECIES_CHARIZARD);
    }
}

#endif
