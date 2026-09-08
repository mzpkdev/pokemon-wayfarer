#include "global.h"
#include "capture_context.h"
#include "event_data.h"
#include "pokedex.h"
#include "test/battle.h"

ASSUMPTIONS
{
    ASSUME(gSpeciesInfo[SPECIES_CLEFFA].catchRate == 150);
}

WILD_BATTLE_TEST("Capture: Incapacitated catch bonus apply correcly with all gen configs")
{
    u32 expectedOdds;
    u32 recordedOdds;
    u32 status;
    u32 gen;

    PARAMETRIZE(expectedOdds = 100, status = STATUS1_SLEEP, gen = GEN_4);
    PARAMETRIZE(expectedOdds = 100, status = STATUS1_FREEZE, gen = GEN_4);
    PARAMETRIZE(expectedOdds = 125, status = STATUS1_SLEEP, gen = GEN_5);
    PARAMETRIZE(expectedOdds = 125, status = STATUS1_FREEZE, gen = GEN_5);

    GIVEN {
        WITH_CONFIG(B_INCAPACITATED_CATCH_BONUS, gen);
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CLEFFA) {Status1(status);}
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL); }
    } SCENE {
        CATCHING_CHANCE(&recordedOdds);
    } THEN {
        EXPECT_EQ(expectedOdds, recordedOdds);
    }
}

WILD_BATTLE_TEST("Capture: Low level catch bonus apply correcly with all gen configs")
{
    u32 expectedOdds;
    u32 recordedOdds;
    u32 level;
    u32 gen;

    PARAMETRIZE(expectedOdds = 50, level = 10, gen = GEN_7);
    PARAMETRIZE(expectedOdds = 50, level = 15, gen = GEN_7);
    PARAMETRIZE(expectedOdds = 50, level = 30, gen = GEN_7);
    PARAMETRIZE(expectedOdds = 100, level = 10, gen = GEN_8);
    PARAMETRIZE(expectedOdds = 75, level = 15, gen = GEN_8);
    PARAMETRIZE(expectedOdds = 50, level = 30, gen = GEN_8);
    PARAMETRIZE(expectedOdds = 80, level = 10, gen = GEN_9);
    PARAMETRIZE(expectedOdds = 50, level = 15, gen = GEN_9);
    PARAMETRIZE(expectedOdds = 50, level = 30, gen = GEN_9);

    GIVEN {
        WITH_CONFIG(B_LOW_LEVEL_CATCH_BONUS, gen);
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CLEFFA) {Level(level);}
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL); }
    } SCENE {
        CATCHING_CHANCE(&recordedOdds);
    } THEN {
        EXPECT_EQ(expectedOdds, recordedOdds);
    }
}

WILD_BATTLE_TEST("Capture: Missing badge malus apply correcly in gen 8")
{
    u32 expectedOdds = 0;
    u32 recordedOdds;
    u32 playerLevel = 0;
    u32 numBadges = 0;

    for (u32 j = 0; j < 8; j++)
    {
        PARAMETRIZE(expectedOdds = 50, playerLevel = 100, numBadges = j);
        PARAMETRIZE(expectedOdds = 5, playerLevel = 99, numBadges = j);
    }
    PARAMETRIZE(expectedOdds = 50, playerLevel = 100, numBadges = 8);
    PARAMETRIZE(expectedOdds = 50, playerLevel = 99, numBadges = 8);
    PARAMETRIZE(expectedOdds = 50, playerLevel = 21, numBadges = 8);

    GIVEN {
        for (u32 j = 0; j < 8; j++)
        {
            if (j < numBadges)
                FlagSet(FLAG_BADGE01_GET + j);
            else
                FlagClear(FLAG_BADGE01_GET + j);
        }
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_8);
        PLAYER(SPECIES_WOBBUFFET) {Level(playerLevel);}
        OPPONENT(SPECIES_CLEFFA);
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL); }
    } SCENE {
        CATCHING_CHANCE(&recordedOdds);
    } THEN {
        EXPECT_EQ(expectedOdds, recordedOdds);
    }
}

WILD_BATTLE_TEST("Capture: Missing badge malus apply correcly in gen 9")
{
    u32 expectedOdds;
    u32 recordedOdds;
    u32 level = 0;
    u32 numBadges = 0;

    PARAMETRIZE(expectedOdds = 200, level = 100, numBadges = 8);
    PARAMETRIZE(expectedOdds = 160, level = 100, numBadges = 7);
    PARAMETRIZE(expectedOdds = 128, level = 100, numBadges = 6);
    PARAMETRIZE(expectedOdds = 102, level = 100, numBadges = 5);
    PARAMETRIZE(expectedOdds = 200, level = 40, numBadges = 4);
    PARAMETRIZE(expectedOdds = 200, level = 40, numBadges = 3);
    PARAMETRIZE(expectedOdds = 160, level = 40, numBadges = 2);
    PARAMETRIZE(expectedOdds = 128, level = 40, numBadges = 1);
    PARAMETRIZE(expectedOdds = 102, level = 40, numBadges = 0);

    GIVEN {
        for (u32 j = 0; j < 8; j++)
        {
            if (j < numBadges)
                FlagSet(FLAG_BADGE01_GET + j);
            else
                FlagClear(FLAG_BADGE01_GET + j);
        }
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_9);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CLEFFA)  {Level(level);};
    } WHEN {
        TURN { USE_ITEM(player, ITEM_QUICK_BALL); }
    } SCENE {
        CATCHING_CHANCE(&recordedOdds);
    } THEN {
        EXPECT_EQ(expectedOdds, recordedOdds);
    }
}

WILD_BATTLE_TEST("Capture: when CRITICAL_CAPTURE_IF_OWNED is enabled, capture of owned pokemon always appear critical")
{
    enum Item item;
    bool32 alreadyOwned;
    u32 catchingChance;

    PARAMETRIZE(item = ITEM_POKE_BALL, alreadyOwned = FALSE);
    PARAMETRIZE(item = ITEM_QUICK_BALL, alreadyOwned = FALSE);
    PARAMETRIZE(item = ITEM_MASTER_BALL, alreadyOwned = FALSE);
    PARAMETRIZE(item = ITEM_POKE_BALL, alreadyOwned = TRUE);
    PARAMETRIZE(item = ITEM_QUICK_BALL, alreadyOwned = TRUE);
    PARAMETRIZE(item = ITEM_MASTER_BALL, alreadyOwned = TRUE);

    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_CATERPIE].catchRate > 155);
        if (alreadyOwned)
            GetSetPokedexFlag(SPECIES_CATERPIE, FLAG_SET_CAUGHT);
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        WITH_CONFIG(B_CRITICAL_CAPTURE_IF_OWNED, GEN_9);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CATERPIE);
    } WHEN {
        TURN { USE_ITEM(player, item, WITH_RNG(RNG_BALLTHROW_SHAKE, 0)); }
    } SCENE {
        CATCHING_CHANCE(&catchingChance);
        if (alreadyOwned)
        {
            ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_CRITICAL_CAPTURE_THROW);
            NOT ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_BALL_THROW);
        }
        else
        {
            NOT ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_CRITICAL_CAPTURE_THROW);
            ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_BALL_THROW);
        }
    } THEN {
        if (item == ITEM_POKE_BALL)
            EXPECT_LT(catchingChance, 255);
        else
            EXPECT_GT(catchingChance, 255);
    }
}

WILD_BATTLE_TEST("Capture: when CRITICAL_CAPTURE_IF_OWNED is enabled, failed capture of owned pokemon does not appear critical")
{
    bool32 success;
    PARAMETRIZE(success = TRUE);
    PARAMETRIZE(success = FALSE);

    GIVEN {
        GetSetPokedexFlag(SPECIES_CATERPIE, FLAG_SET_CAUGHT);
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        WITH_CONFIG(B_CRITICAL_CAPTURE_IF_OWNED, GEN_9);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CATERPIE);
    } WHEN {
        TURN { USE_ITEM(player, ITEM_POKE_BALL, WITH_RNG(RNG_BALLTHROW_SHAKE, success ? 0 : MAX_u16)); }
    } SCENE {
        if (success)
        {
            ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_CRITICAL_CAPTURE_THROW);
            NOT ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_BALL_THROW);
        }
        else
        {
            NOT ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_CRITICAL_CAPTURE_THROW);
            ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_BALL_THROW);
        }
    }
}

WILD_BATTLE_TEST("Capture: ball data is properly set in captured pokemon")
{
    u32 item = ITEM_NONE;
    for (enum PokeBall ballId = BALL_STRANGE; ballId < POKEBALL_COUNT; ballId++)
    {
        PARAMETRIZE(item = gPokeBalls[ballId].itemId);
    }

    GIVEN {
        // This checks stored ball identity, so keep every ball's capture odds positive.
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { USE_ITEM(player, item, WITH_RNG(RNG_BALLTHROW_SHAKE, 0)); }
    } SCENE {
        ONE_OF
        {
            ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_CRITICAL_CAPTURE_THROW);
            ANIMATION(ANIM_TYPE_SPECIAL, B_ANIM_BALL_THROW);
        }
    } THEN {
        EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_POKEBALL), GetItemSecondaryId(item));
    }
}

WILD_BATTLE_TEST("Capture: zero odds cannot catch even on the lowest shake roll")
{
    u32 species;
    u32 recordedOdds;

    PARAMETRIZE(species = SPECIES_WOBBUFFET);
    PARAMETRIZE(species = SPECIES_BELDUM);

    GIVEN {
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_9);
        PLAYER(SPECIES_WOBBUFFET) { Level(100); }
        OPPONENT(species) { Level(100); }
    } WHEN {
        TURN { USE_ITEM(player, ITEM_HEAVY_BALL, WITH_RNG(RNG_BALLTHROW_SHAKE, 0)); }
    } SCENE {
        CATCHING_CHANCE(&recordedOdds);
    } THEN {
        EXPECT_EQ(recordedOdds, 0);
        EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_SPECIES), SPECIES_NONE);
    }
}

WILD_BATTLE_TEST("Capture: absent player context skips comparisons but preserves independent ball bonuses")
{
    u32 item;
    u32 expectedOdds;
    PARAMETRIZE(item = ITEM_LEVEL_BALL, expectedOdds = 15);
    PARAMETRIZE(item = ITEM_LOVE_BALL, expectedOdds = IS_HNS ? 60 : 15);

    GIVEN {
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_8);
        WITH_CONFIG(B_LOW_LEVEL_CATCH_BONUS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET) { Level(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Level(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        const struct CaptureContext context = { .hasPlayerBattler = FALSE, .playerBattler = MAX_u8 };
        gLastUsedItem = item;
        EXPECT_EQ(ComputeCaptureOdds(B_POSITION_OPPONENT_LEFT, &context), expectedOdds);
    }
}

WILD_BATTLE_TEST("Capture: explicit completed turns control Quick and Timer bonuses without player data")
{
    u32 turns;
    u32 expectedOdds;
    u32 item;
    PARAMETRIZE(item = ITEM_QUICK_BALL, turns = 0, expectedOdds = B_QUICK_BALL_MODIFIER >= GEN_5 ? 250 : 200);
    PARAMETRIZE(item = ITEM_QUICK_BALL, turns = 1, expectedOdds = 50);
    PARAMETRIZE(item = ITEM_TIMER_BALL, turns = 0, expectedOdds = 50);
    PARAMETRIZE(item = ITEM_TIMER_BALL, turns = 1, expectedOdds = B_TIMER_BALL_MODIFIER >= GEN_5 ? 65 : 55);
    PARAMETRIZE(item = ITEM_TIMER_BALL, turns = 2, expectedOdds = B_TIMER_BALL_MODIFIER >= GEN_5 ? 80 : 60);
    PARAMETRIZE(item = ITEM_TIMER_BALL, turns = 255, expectedOdds = 200);

    GIVEN {
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        WITH_CONFIG(B_LOW_LEVEL_CATCH_BONUS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CLEFFA);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        const struct CaptureContext context = { .hasPlayerBattler = FALSE, .playerBattler = MAX_u8, .completedTurns = turns };
        gLastUsedItem = item;
        EXPECT_EQ(ComputeCaptureOdds(B_POSITION_OPPONENT_LEFT, &context), expectedOdds);
    }
}

WILD_BATTLE_TEST("Capture: proximity uses effective species rate before Heavy Ball flat bonus")
{
    u32 factor;
    u32 expectedOdds;
    PARAMETRIZE(factor = 11, expectedOdds = 43);
    PARAMETRIZE(factor = 15, expectedOdds = 61);
    PARAMETRIZE(factor = 20, expectedOdds = 84);

    GIVEN {
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        WITH_CONFIG(B_LOW_LEVEL_CATCH_BONUS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CLEFFA);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        const struct CaptureContext context = { .hasPlayerBattler = FALSE, .playerBattler = MAX_u8, .initialCatchFactor = 11, .catchFactor = factor };
        gLastUsedItem = ITEM_HEAVY_BALL;
        EXPECT_EQ(ComputeCaptureOdds(B_POSITION_OPPONENT_LEFT, &context), expectedOdds);
    }
}

WILD_BATTLE_TEST("Capture: Safari Balls keep legacy quantization while owned balls use proximity")
{
    u32 factor, safariBaseOdds, ownedOdds;
    PARAMETRIZE(factor = 11, safariBaseOdds = 46, ownedOdds = 50);
    PARAMETRIZE(factor = 15, safariBaseOdds = 63, ownedOdds = 68);
    PARAMETRIZE(factor = 18, safariBaseOdds = 76, ownedOdds = 81);
    PARAMETRIZE(factor = 20, safariBaseOdds = 85, ownedOdds = 90);

    GIVEN {
        WITH_CONFIG(B_MISSING_BADGE_CATCH_MALUS, GEN_7);
        WITH_CONFIG(B_LOW_LEVEL_CATCH_BONUS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CLEFFA);
    } WHEN {
        TURN { MOVE(player, MOVE_CELEBRATE); MOVE(opponent, MOVE_CELEBRATE); }
    } THEN {
        const struct CaptureContext context = { .hasPlayerBattler = TRUE, .playerBattler = B_POSITION_PLAYER_LEFT, .initialCatchFactor = 11, .catchFactor = factor };
        u32 savedFlags = gBattleTypeFlags;
        gBattleTypeFlags |= BATTLE_TYPE_SAFARI;
        gBattleStruct->safariCatchFactor = factor;
        gLastUsedItem = ITEM_SAFARI_BALL;
        EXPECT_EQ(ComputeCaptureOdds(B_POSITION_OPPONENT_LEFT, &context),
                  B_SAFARI_BALL_MODIFIER == GEN_1 ? safariBaseOdds * 2
                  : B_SAFARI_BALL_MODIFIER <= GEN_7 ? safariBaseOdds * 150 / 100 : safariBaseOdds);
        gLastUsedItem = ITEM_POKE_BALL;
        EXPECT_EQ(ComputeCaptureOdds(B_POSITION_OPPONENT_LEFT, &context), IS_HNS ? ownedOdds : safariBaseOdds);
        gBattleTypeFlags = savedFlags;
    }
}
