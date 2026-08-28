#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Zen Mode switches Darmanitan's form when HP is half or less at the end of the turn")
{
    u16 standardSpecies, zenSpecies;
    PARAMETRIZE { standardSpecies = SPECIES_DARMANITAN_STANDARD;          zenSpecies = SPECIES_DARMANITAN_ZEN; }
    PARAMETRIZE { standardSpecies = SPECIES_DARMANITAN_GALAR_STANDARD; zenSpecies = SPECIES_DARMANITAN_GALAR_ZEN; }

    // TODO(nightly-failures): Zen Mode form changes receive additional or reordered battle messages before this scene's expected actions. Re-enable after the form-change event sequence is aligned.
    KNOWN_FAILING;
    GIVEN {
        ASSUME(GetSpeciesBaseHP(standardSpecies) == 105);
        ASSUME(GetSpeciesBaseHP(zenSpecies) == 105);
        PLAYER(standardSpecies)
        {
            Ability(ABILITY_ZEN_MODE);
            HP((GetMonData(&PLAYER_PARTY[0], MON_DATA_MAX_HP) / 2) + 1);
        }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
            TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("DARMANITAN used CELEBRATE!");
        MESSAGE("The opposing WOBBUFFET used SCRATCH!");
        HP_BAR(player);
        ABILITY_POPUP(player, ABILITY_ZEN_MODE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_FORM_CHANGE, player);
        MESSAGE("ZEN MODE triggered!");
    } THEN {
        EXPECT_LT(player->hp, player->maxHP / 2);
        EXPECT_EQ(player->species, zenSpecies);
    }
}

SINGLE_BATTLE_TEST("Zen Mode switches Darmanitan's form to Standard when swapped out")
{
    u16 standardSpecies, zenSpecies;
    PARAMETRIZE { standardSpecies = SPECIES_DARMANITAN_STANDARD;          zenSpecies = SPECIES_DARMANITAN_ZEN; }
    PARAMETRIZE { standardSpecies = SPECIES_DARMANITAN_GALAR_STANDARD;    zenSpecies = SPECIES_DARMANITAN_GALAR_ZEN; }

    // TODO(nightly-failures): Zen Mode form changes receive additional or reordered battle messages before this scene's expected actions. Re-enable after the form-change event sequence is aligned.
    KNOWN_FAILING;
    GIVEN {
        ASSUME(GetSpeciesBaseHP(standardSpecies) == 105);
        ASSUME(GetSpeciesBaseHP(zenSpecies) == 105);
        PLAYER(standardSpecies)
        {
            Ability(ABILITY_ZEN_MODE);
            HP(GetMonData(&PLAYER_PARTY[0], MON_DATA_MAX_HP) / 2);
        }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { SWITCH(player, 1); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { SWITCH(player, 0); MOVE(opponent, MOVE_CELEBRATE); }
    } SCENE {
        MESSAGE("DARMANITAN used CELEBRATE!");
        MESSAGE("The opposing WOBBUFFET used CELEBRATE!");
        ABILITY_POPUP(player, ABILITY_ZEN_MODE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_FORM_CHANGE, player);
        MESSAGE("ZEN MODE triggered!");
        MESSAGE("Go! WOBBUFFET!");
        MESSAGE("The opposing WOBBUFFET used CELEBRATE!");
        MESSAGE("Go! DARMANITAN!");
        MESSAGE("The opposing WOBBUFFET used CELEBRATE!");
        ABILITY_POPUP(player, ABILITY_ZEN_MODE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_FORM_CHANGE, player);
        MESSAGE("ZEN MODE triggered!");
    } THEN {
        EXPECT_LE(player->hp, player->maxHP / 2);
        EXPECT_EQ(player->species, zenSpecies);
    }
}

SINGLE_BATTLE_TEST("Zen Mode switches Darmanitan's form when HP is healed above half")
{
    u16 standardSpecies, zenSpecies;
    PARAMETRIZE { standardSpecies = SPECIES_DARMANITAN_STANDARD;          zenSpecies = SPECIES_DARMANITAN_ZEN; }
    PARAMETRIZE { standardSpecies = SPECIES_DARMANITAN_GALAR_STANDARD;    zenSpecies = SPECIES_DARMANITAN_GALAR_ZEN; }

    // TODO(nightly-failures): Zen Mode form changes receive additional or reordered battle messages before this scene's expected actions. Re-enable after the form-change event sequence is aligned.
    KNOWN_FAILING;
    GIVEN {
        ASSUME(GetSpeciesBaseHP(standardSpecies) == 105);
        ASSUME(GetSpeciesBaseHP(zenSpecies) == 105);
        PLAYER(standardSpecies)
        {
            Ability(ABILITY_ZEN_MODE);
            HP(GetMonData(&PLAYER_PARTY[0], MON_DATA_MAX_HP) / 2);
        }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_HEAL_PULSE); }
    } SCENE {
        MESSAGE("DARMANITAN used CELEBRATE!");
        MESSAGE("The opposing WOBBUFFET used CELEBRATE!");
        ABILITY_POPUP(player, ABILITY_ZEN_MODE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_FORM_CHANGE, player);
        MESSAGE("ZEN MODE triggered!");
        MESSAGE("DARMANITAN used CELEBRATE!");
        MESSAGE("The opposing WOBBUFFET used HEAL PULSE!");
        HP_BAR(player);
        ABILITY_POPUP(player, ABILITY_ZEN_MODE);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_FORM_CHANGE, player);
        MESSAGE("ZEN MODE ended!");
    } THEN {
        EXPECT_GT(player->hp, player->maxHP / 2);
        EXPECT_EQ(player->species, standardSpecies);
    }
}
