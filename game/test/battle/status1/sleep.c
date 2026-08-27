#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Sleep prevents the battler from using a move for the configured number of turns")
{
    u32 turns = 0, j, maxTurns;

    if (B_SLEEP_TURNS >= GEN_5)
        maxTurns = 3;
    else if (B_SLEEP_TURNS >= GEN_3)
        maxTurns = 4;
    else
        maxTurns = 7;

    for (j = 1; j <= maxTurns; j++)
        PARAMETRIZE { turns = j; }
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Status1(STATUS1_SLEEP_TURN(turns)); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        for (j = 0; j < turns; j++)
            TURN { MOVE(player, MOVE_CELEBRATE); }
    } SCENE {
        for (j = 0; j < turns - 1; j++)
            MESSAGE("WOBBUFFET is fast asleep.");
        MESSAGE("WOBBUFFET woke up!");
        STATUS_ICON(player, none: TRUE);
        MESSAGE("WOBBUFFET used CELEBRATE!");
    }
}

SINGLE_BATTLE_TEST("Sleep: Spore affects grass types (Gen1-5)")
{
    GIVEN {
        WITH_CONFIG(B_POWDER_GRASS, GEN_5);
        ASSUME(IsPowderMove(MOVE_SPORE));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CHIKORITA);
    } WHEN {
        TURN { MOVE(player, MOVE_SPORE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SPORE, player);
    }
}

SINGLE_BATTLE_TEST("Sleep: Spore doesn't affect grass types (Gen6+)")
{
    GIVEN {
        WITH_CONFIG(B_POWDER_GRASS, GEN_6);
        ASSUME(IsPowderMove(MOVE_SPORE));
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CHIKORITA);
    } WHEN {
        TURN { MOVE(player, MOVE_SPORE); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_SPORE, player);
    }
}

AI_SINGLE_BATTLE_TEST("AI avoids hypnosis when it can not put target to sleep")
{
    u32 species;
    enum Ability ability;

    PARAMETRIZE { species = SPECIES_HOOTHOOT; ability = ABILITY_INSOMNIA; }
    PARAMETRIZE { species = SPECIES_MANKEY; ability = ABILITY_VITAL_SPIRIT; }
    PARAMETRIZE { species = SPECIES_KOMALA; ability = ABILITY_COMATOSE; }
    PARAMETRIZE { species = SPECIES_NACLI; ability = ABILITY_PURIFYING_SALT; }

    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_OMNISCIENT);
        PLAYER(species) { Ability(ability); }
        OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE, MOVE_HYPNOSIS); }
    } WHEN {
        TURN { SCORE_EQ(opponent, MOVE_CELEBRATE, MOVE_HYPNOSIS); } // Both get -10
    }
}
