#include "global.h"
#include "battle.h"
#include "battle_util.h"
#include "league_halls.h"
#include "test/battle.h"
#include "config/league_circuit.h"

#if WAYFARER_LEAGUE_EVENTS

SINGLE_BATTLE_TEST("League hall rooms expire after five turns")
{
    enum LeagueId league;
    u8 match;

    PARAMETRIZE { league = LEAGUE_ID_INDIGO; match = 2; } // Trick Room
    PARAMETRIZE { league = LEAGUE_ID_HOENN; match = 0; } // Magic Room
    PARAMETRIZE { league = LEAGUE_ID_MASTERS; match = 3; } // Wonder Room

    BuildLeagueHallStartingStatuses(GetLeagueHall(league, match), &gStartingStatuses);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
        TURN {}
        TURN {}
        TURN {}
        TURN {}
    } SCENE {
        if (league == LEAGUE_ID_INDIGO)
            MESSAGE("The twisted dimensions returned to normal!");
        else if (league == LEAGUE_ID_HOENN)
            MESSAGE("Magic Room wore off, and held items' effects returned to normal!");
        else
            MESSAGE("Wonder Room wore off, and DEFENSE and SP. DEF stats returned to normal!");
    } THEN {
        EXPECT_EQ(gFieldTimers.trickRoomTimer, 0);
        EXPECT_EQ(gFieldTimers.magicRoomTimer, 0);
        EXPECT_EQ(gFieldTimers.wonderRoomTimer, 0);
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("League hall terrain expires after five turns")
{
    enum LeagueId league;
    u8 match;

    PARAMETRIZE { league = LEAGUE_ID_MASTERS; match = 0; } // Psychic Terrain
    PARAMETRIZE { league = LEAGUE_ID_MASTERS; match = 1; } // Grassy Terrain
    PARAMETRIZE { league = LEAGUE_ID_HOENN; match = 2; } // Misty Terrain

    BuildLeagueHallStartingStatuses(GetLeagueHall(league, match), &gStartingStatuses);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
        TURN {}
        TURN {}
        TURN {}
        TURN {}
    } SCENE {
        if (league == LEAGUE_ID_HOENN)
            MESSAGE("The mist disappeared from the battlefield.");
        else if (match == 0)
            MESSAGE("The weirdness disappeared from the battlefield!");
        else
            MESSAGE("The grass disappeared from the battlefield.");
    } THEN {
        EXPECT_EQ(gFieldTimers.terrainTimer, 0);
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("League hall Tailwind protects both sides for four turns")
{
    BuildLeagueHallStartingStatuses(GetLeagueHall(LEAGUE_ID_INDIGO, 3), &gStartingStatuses);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
        TURN {}
        TURN {}
        TURN {}
    } SCENE {
        MESSAGE("Your team's Tailwind petered out!");
        MESSAGE("The opposing team's Tailwind petered out!");
    } THEN {
        EXPECT_EQ(gSideTimers[B_SIDE_PLAYER].tailwindTimer, 0);
        EXPECT_EQ(gSideTimers[B_SIDE_OPPONENT].tailwindTimer, 0);
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("League hall Sea of Fire burns both sides for four turns")
{
    BuildLeagueHallStartingStatuses(GetLeagueHall(LEAGUE_ID_HOENN, 3), &gStartingStatuses);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(1000); HP(1000); }
        OPPONENT(SPECIES_WOBBUFFET) { MaxHP(1000); HP(1000); }
    } WHEN {
        TURN {}
        TURN {}
        TURN {}
        TURN {}
    } SCENE {
        MESSAGE("WOBBUFFET was hurt by the sea of fire!");
        MESSAGE("The opposing WOBBUFFET was hurt by the sea of fire!");
        MESSAGE("The sea of fire around your team disappeared!");
        MESSAGE("The sea of fire around the opposing team disappeared!");
    } THEN {
        EXPECT_EQ(gSideTimers[B_SIDE_PLAYER].seaOfFireTimer, 0);
        EXPECT_EQ(gSideTimers[B_SIDE_OPPONENT].seaOfFireTimer, 0);
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("League hall Sticky Web slows both opening leads")
{
    BuildLeagueHallStartingStatuses(GetLeagueHall(LEAGUE_ID_HOENN, 1), &gStartingStatuses);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        MESSAGE("WOBBUFFET was caught in a sticky web!");
        MESSAGE("The opposing WOBBUFFET was caught in a sticky web!");
    } THEN {
        EXPECT_EQ(player->statStages[STAT_SPEED], DEFAULT_STAT_STAGE - 1);
        EXPECT_EQ(opponent->statStages[STAT_SPEED], DEFAULT_STAT_STAGE - 1);
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("League hall Stealth Rock damages both opening leads")
{
    BuildLeagueHallStartingStatuses(GetLeagueHall(LEAGUE_ID_MASTERS, 2), &gStartingStatuses);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {}
    } SCENE {
        MESSAGE("Pointed stones dug into WOBBUFFET!");
        MESSAGE("Pointed stones dug into the opposing WOBBUFFET!");
    } THEN {
        EXPECT_LT(player->hp, player->maxHP);
        EXPECT_LT(opponent->hp, opponent->maxHP);
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("League hall opening Stealth Rock lets a fainted lead be replaced")
{
    BuildLeagueHallStartingStatuses(GetLeagueHall(LEAGUE_ID_MASTERS, 2), &gStartingStatuses);
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { HP(1); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        // This replacement happens before the first move-selection turn.
        TURN { SKIP_TURN(player); SEND_OUT(player, 1); }
    } SCENE {
        MESSAGE("Pointed stones dug into WOBBUFFET!");
        MESSAGE("WOBBUFFET fainted!");
        MESSAGE("Go! WYNAUT!");
        MESSAGE("Pointed stones dug into WYNAUT!");
    } THEN {
        EXPECT_EQ(player->species, SPECIES_WYNAUT);
        EXPECT_LT(player->hp, player->maxHP);
        ResetStartingStatuses();
    }
}

#endif
