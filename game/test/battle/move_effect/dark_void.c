#include "global.h"
#include "test/battle.h"

ASSUMPTIONS
{
    ASSUME(GetMoveEffect(MOVE_DARK_VOID) == EFFECT_DARK_VOID);
}

SINGLE_BATTLE_TEST("Dark Void inflicts the configured number of sleep turns")
{
    u32 turns = 0, count, maxTurns;

    if (B_SLEEP_TURNS >= GEN_5)
        maxTurns = 3;
    else if (B_SLEEP_TURNS >= GEN_3)
        maxTurns = 4;
    else
        maxTurns = 7;

    for (count = 1; count <= maxTurns; count++)
        PARAMETRIZE { turns = count; }
    PASSES_RANDOMLY(1, maxTurns, RNG_SLEEP_TURNS);
    GIVEN {
        PLAYER(SPECIES_DARKRAI);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DARK_VOID); MOVE(opponent, MOVE_CELEBRATE); }
        for (count = 0; count < turns; ++count)
            TURN {}
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DARK_VOID, player);
        ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_SLP, opponent);
        MESSAGE("The opposing WOBBUFFET fell asleep!");
        STATUS_ICON(opponent, sleep: TRUE);
        for (count = 0; count < turns; ++count)
        {
            if (count < turns - 1)
                MESSAGE("The opposing WOBBUFFET is fast asleep.");
            ANIMATION(ANIM_TYPE_STATUS, B_ANIM_STATUS_SLP, opponent);
        }
        MESSAGE("The opposing WOBBUFFET woke up!");
        STATUS_ICON(opponent, none: TRUE);
    }
}

TO_DO_BATTLE_TEST("Dark Void can only be used by Darkrai (Gen7+)");
TO_DO_BATTLE_TEST("Dark Void can be used by Pokémon other than Darkrai (Gen4-6)");
TO_DO_BATTLE_TEST("Dark Void can be used by a Pokémon transformed into Darkrai");
