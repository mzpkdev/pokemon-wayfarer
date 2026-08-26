#include "global.h"
#include "event_data.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("SetStartingStatus can start Spikes on the opposing side", s16 damage)
{
    u16 startingHazard;
    u32 divisor;

    PARAMETRIZE { startingHazard = STARTING_STATUS_SPIKES_OPPONENT_L1; divisor = 8; }
    PARAMETRIZE { startingHazard = STARTING_STATUS_SPIKES_OPPONENT_L2; divisor = 6; }
    PARAMETRIZE { startingHazard = STARTING_STATUS_SPIKES_OPPONENT_L3; divisor = 4; }

    SetStartingStatus(startingHazard);

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        MESSAGE("2 sent out WYNAUT!");
        s32 maxHP = GetMonData(&OPPONENT_PARTY[1], MON_DATA_MAX_HP);
        HP_BAR(opponent, damage: maxHP / divisor);
        MESSAGE("The opposing WYNAUT was hurt by the spikes!");
    } FINALLY {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting Toxic Spikes poison the opposing switch-in")
{
    SetStartingStatus(STARTING_STATUS_TOXIC_SPIKES_OPPONENT_L1);

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        MESSAGE("2 sent out WYNAUT!");
        MESSAGE("The opposing WYNAUT was poisoned!");
        STATUS_ICON(opponent, poison: TRUE);
        NOT STATUS_ICON(opponent, badPoison: TRUE);
    } THEN {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting Toxic Spikes badly poison the opposing switch-in")
{
    SetStartingStatus(STARTING_STATUS_TOXIC_SPIKES_OPPONENT_L2);

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        MESSAGE("2 sent out WYNAUT!");
        MESSAGE("The opposing WYNAUT was badly poisoned!");
        STATUS_ICON(opponent, badPoison: TRUE);
    } THEN {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting Sticky Web lowers Speed on entry")
{
    SetStartingStatus(STARTING_STATUS_STICKY_WEB_OPPONENT);

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_RATICATE);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        MESSAGE("2 sent out WYNAUT!");
        MESSAGE("The opposing WYNAUT was caught in a sticky web!");
        MESSAGE("The opposing WYNAUT's SPEED fell!");
    } THEN {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting Stealth Rock damages the opposing switch-in")
{
    SetStartingStatus(STARTING_STATUS_STEALTH_ROCK_OPPONENT);

    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_CHARIZARD].types[0] == TYPE_FIRE);
        ASSUME(gSpeciesInfo[SPECIES_CHARIZARD].types[1] == TYPE_FLYING);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CHARIZARD);
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        MESSAGE("2 sent out CHARIZARD!");
        s32 maxHP = GetMonData(&OPPONENT_PARTY[1], MON_DATA_MAX_HP);
        HP_BAR(opponent, damage: maxHP / 2);
        MESSAGE("Pointed stones dug into the opposing CHARIZARD!");
    } THEN {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting sharp steel damages the opposing switch-in")
{
    SetStartingStatus(STARTING_STATUS_SHARP_STEEL_OPPONENT);

    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_SYLVEON].types[0] == TYPE_FAIRY);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_SYLVEON);
    } WHEN {
        TURN { SWITCH(opponent, 1); }
    } SCENE {
        MESSAGE("2 sent out SYLVEON!");
        s32 maxHP = GetMonData(&OPPONENT_PARTY[1], MON_DATA_MAX_HP);
        HP_BAR(opponent, damage: maxHP / 4);
        MESSAGE("The sharp steel bit into the opposing SYLVEON!");
    } THEN {
        ResetStartingStatuses();
    }
}

// Player-side hazard tests

SINGLE_BATTLE_TEST("SetStartingStatus can start Spikes on the player side", s16 damage)
{
    u16 startingHazard;
    u32 divisor;

    PARAMETRIZE { startingHazard = STARTING_STATUS_SPIKES_PLAYER_L1; divisor = 8; }
    PARAMETRIZE { startingHazard = STARTING_STATUS_SPIKES_PLAYER_L2; divisor = 6; }
    PARAMETRIZE { startingHazard = STARTING_STATUS_SPIKES_PLAYER_L3; divisor = 4; }

    SetStartingStatus(startingHazard);

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        MESSAGE("Go! WYNAUT!");
        s32 maxHP = GetMonData(&PLAYER_PARTY[1], MON_DATA_MAX_HP);
        HP_BAR(player, damage: maxHP / divisor);
        MESSAGE("WYNAUT was hurt by the spikes!");
    } FINALLY {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting Toxic Spikes poison the player's switch-in")
{
    SetStartingStatus(STARTING_STATUS_TOXIC_SPIKES_PLAYER_L1);

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        MESSAGE("Go! WYNAUT!");
        MESSAGE("WYNAUT was poisoned!");
        STATUS_ICON(player, poison: TRUE);
        NOT STATUS_ICON(player, badPoison: TRUE);
    } THEN {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting Toxic Spikes badly poison the player's switch-in")
{
    SetStartingStatus(STARTING_STATUS_TOXIC_SPIKES_PLAYER_L2);

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        MESSAGE("Go! WYNAUT!");
        MESSAGE("WYNAUT was badly poisoned!");
        STATUS_ICON(player, badPoison: TRUE);
    } THEN {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting Sticky Web lowers Speed on player's entry")
{
    SetStartingStatus(STARTING_STATUS_STICKY_WEB_PLAYER);

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        MESSAGE("Go! WYNAUT!");
        MESSAGE("WYNAUT was caught in a sticky web!");
        MESSAGE("WYNAUT's SPEED fell!");
    } THEN {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting Stealth Rock damages the player's switch-in")
{
    SetStartingStatus(STARTING_STATUS_STEALTH_ROCK_PLAYER);

    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_CHARIZARD].types[0] == TYPE_FIRE);
        ASSUME(gSpeciesInfo[SPECIES_CHARIZARD].types[1] == TYPE_FLYING);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_CHARIZARD);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        MESSAGE("Go! CHARIZARD!");
        s32 maxHP = GetMonData(&PLAYER_PARTY[1], MON_DATA_MAX_HP);
        HP_BAR(player, damage: maxHP / 2);
        MESSAGE("Pointed stones dug into CHARIZARD!");
    } THEN {
        ResetStartingStatuses();
    }
}

SINGLE_BATTLE_TEST("Starting sharp steel damages the player's switch-in")
{
    SetStartingStatus(STARTING_STATUS_SHARP_STEEL_PLAYER);

    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_SYLVEON].types[0] == TYPE_FAIRY);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_SYLVEON);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { SWITCH(player, 1); }
    } SCENE {
        MESSAGE("Go! SYLVEON!");
        s32 maxHP = GetMonData(&PLAYER_PARTY[1], MON_DATA_MAX_HP);
        HP_BAR(player, damage: maxHP / 4);
        MESSAGE("The sharp steel bit into SYLVEON!");
    } THEN {
        ResetStartingStatuses();
    }
}
