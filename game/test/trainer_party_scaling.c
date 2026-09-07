#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_util2.h"
#include "data.h"
#include "debug.h"
#include "challenge_menu.h"
#include "event_data.h"
#include "malloc.h"
#include "random.h"
#include "randomizer.h"
#include "test/test.h"
#include "trainer_party_scaling.h"
#include "trainer_rating.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/item.h"
#include "constants/opponents.h"

#if IS_WAYFARER

static const u8 sBaselineOracle[] = {
    7, 7, 8, 8, 8, 9, 9, 10, 10, 11, 11, 12, 13, 13, 14, 14, 15,
    16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24,
    26, 27, 28, 29, 30, 32, 33, 34, 35, 36, 38, 39, 40, 41, 42, 44,
    45, 46, 47, 48, 50, 51, 52, 54, 56, 58, 60, 62, 64, 66, 68, 70,
    72, 73, 75, 76, 77, 79, 80, 81, 83, 84, 85, 87, 88, 89, 91, 92,
};

static u32 ScalingLevelOracle(u32 rating, s32 level, bool32 gym)
{
    s32 adjustment, nearest = -1, distance = 1000;
    for (adjustment = -1; adjustment <= 8; adjustment++)
    {
        s32 error = 5 * adjustment - (level - 5);
        if (error < 0)
            error = -error;
        if (error < distance)
        {
            nearest = adjustment;
            distance = error;
        }
    }
    return min(100, sBaselineOracle[min(rating, 80)] + nearest + 2 * gym);
}

static const struct TrainerMon sGymLeaderPlanTestParty[PARTY_SIZE] = {
    {.species = SPECIES_GEODUDE},
    {.species = SPECIES_ONIX},
    {.species = SPECIES_KABUTO},
    {.species = SPECIES_OMANYTE},
    {.species = SPECIES_NOSEPASS},
    {.species = SPECIES_AERODACTYL},
};

// Retention is source order.  Construction deliberately puts the ace last.
static const struct GymLeaderScalingRoster sGymLeaderPlanTestRoster = {
    .trainerId = TRAINER_FALKNER_1_HNS,
    .ownerId = TRAINER_FALKNER_1_HNS,
    .party = sGymLeaderPlanTestParty,
    .legacyParty = sGymLeaderPlanTestParty,
    .legacyPartySize = 2,
    .slots = {
        {.battleOrder = 5, .isAce = TRUE,  .levelOffset =  0, .movePolicy = GYM_LEADER_MOVE_AUTHORED},
        {.battleOrder = 0, .isAce = FALSE, .levelOffset = -1, .movePolicy = GYM_LEADER_MOVE_LEVEL_UP},
        {.battleOrder = 3, .isAce = FALSE, .levelOffset = -2, .movePolicy = GYM_LEADER_MOVE_AUTHORED},
        {.battleOrder = 1, .isAce = FALSE, .levelOffset = -1, .movePolicy = GYM_LEADER_MOVE_LEVEL_UP},
        {.battleOrder = 4, .isAce = FALSE, .levelOffset = -2, .movePolicy = GYM_LEADER_MOVE_AUTHORED},
        {.battleOrder = 2, .isAce = FALSE, .levelOffset = -1, .movePolicy = GYM_LEADER_MOVE_LEVEL_UP},
    },
};

static const struct GymLeaderScalingRoster sTateLizaPlanTestRoster = {
    .trainerId = TRAINER_TATE_AND_LIZA_1,
    .ownerId = TRAINER_TATE_AND_LIZA_1,
    .party = sGymLeaderPlanTestParty,
    .legacyParty = sGymLeaderPlanTestParty,
    .legacyPartySize = 2,
    .isDoubleBattle = TRUE,
    .slots = {
        {.battleOrder = 0, .isAce = TRUE,  .levelOffset =  0, .movePolicy = GYM_LEADER_MOVE_AUTHORED},
        {.battleOrder = 1, .isAce = TRUE,  .levelOffset =  0, .movePolicy = GYM_LEADER_MOVE_AUTHORED},
        {.battleOrder = 2, .isAce = FALSE, .levelOffset = -1, .movePolicy = GYM_LEADER_MOVE_LEVEL_UP},
        {.battleOrder = 3, .isAce = FALSE, .levelOffset = -1, .movePolicy = GYM_LEADER_MOVE_LEVEL_UP},
        {.battleOrder = 4, .isAce = FALSE, .levelOffset = -2, .movePolicy = GYM_LEADER_MOVE_LEVEL_UP},
        {.battleOrder = 5, .isAce = FALSE, .levelOffset = -2, .movePolicy = GYM_LEADER_MOVE_LEVEL_UP},
    },
};

TEST("Gym Leader scaling has independent party thresholds and a monotonic cap-seeded curve")
{
    static const u8 anchors[][2] = {{0, 15}, {4, 16}, {8, 18}, {16, 23}, {30, 30}, {40, 42}, {55, 60}, {65, 80}, {80, 100}};
    u32 rating;
    u8 previous = 0;

    EXPECT_EQ(GetGymLeaderScalingPartySize(0), 2);
    EXPECT_EQ(GetGymLeaderScalingPartySize(7), 2);
    EXPECT_EQ(GetGymLeaderScalingPartySize(8), 3);
    EXPECT_EQ(GetGymLeaderScalingPartySize(21), 3);
    EXPECT_EQ(GetGymLeaderScalingPartySize(22), 4);
    EXPECT_EQ(GetGymLeaderScalingPartySize(33), 4);
    EXPECT_EQ(GetGymLeaderScalingPartySize(34), 5);
    EXPECT_EQ(GetGymLeaderScalingPartySize(39), 5);
    EXPECT_EQ(GetGymLeaderScalingPartySize(40), 6);
    EXPECT_EQ(GetGymLeaderScalingPartySize(80), 6);
    EXPECT_EQ(GetGymLeaderScalingPartySize(65535), 6);

    for (rating = 0; rating <= 80; rating++)
    {
        u8 level = GetGymLeaderScalingLevel(rating, 0);
        EXPECT_GE(level, previous);
        EXPECT_GE(GetGymLeaderScalingLevel(rating, -2), 1);
        EXPECT_EQ(GetGymLeaderScalingLevel(rating, -1), level - 1);
        previous = level;
    }
    for (rating = 0; rating < ARRAY_COUNT(anchors); rating++)
        EXPECT_EQ(GetGymLeaderScalingLevel(anchors[rating][0], 0), anchors[rating][1]);
    EXPECT_EQ(GetGymLeaderScalingLevel(0, -2), 13);
    EXPECT_EQ(GetGymLeaderScalingLevel(0, -1), 14);
    EXPECT_EQ(GetGymLeaderScalingLevel(40, -2), 40);
    EXPECT_EQ(GetGymLeaderScalingLevel(40, -1), 41);
    EXPECT_EQ(GetGymLeaderScalingLevel(80, -2), 98);
    EXPECT_EQ(GetGymLeaderScalingLevel(80, -1), 99);
    EXPECT_EQ(GetGymLeaderScalingLevel(2, 0), 16); // Exact half rounds upward.
    EXPECT_EQ(GetGymLeaderScalingLevel(12, 0), 21); // Another exact half.
    EXPECT_EQ(GetGymLeaderScalingLevel(65535, 0), 100);
}

TEST("Gym Leader plans retain prefixes then reorder source indices without changing their levels")
{
    static const u8 ratings[] = {0, 8, 22, 34, 40};
    static const u8 expectedCounts[] = {2, 3, 4, 5, 6};
    static const u8 expectedSources[][PARTY_SIZE] = {
        {1, 0},
        {1, 2, 0},
        {1, 3, 2, 0},
        {1, 3, 2, 4, 0},
        {1, 3, 5, 2, 4, 0},
    };
    struct GymLeaderScalingPlan plan;
    u32 row, i;
    rng_value_t before = gRngValue;

    for (row = 0; row < ARRAY_COUNT(ratings); row++)
    {
        EXPECT(BuildGymLeaderScalingPlan(&sGymLeaderPlanTestRoster, ratings[row], &plan));
        EXPECT_EQ(plan.count, expectedCounts[row]);
        for (i = 0; i < plan.count; i++)
        {
            u8 sourceIndex = plan.sourceIndices[i];
            EXPECT_EQ(sourceIndex, expectedSources[row][i]);
            EXPECT_EQ(plan.levels[i], GetGymLeaderScalingLevel(ratings[row], sGymLeaderPlanTestRoster.slots[sourceIndex].levelOffset));
        }
        for (; i < PARTY_SIZE; i++)
        {
            EXPECT_EQ(plan.sourceIndices[i], 0);
            EXPECT_EQ(plan.levels[i], 0);
        }
    }
    EXPECT_EQ(memcmp(&gRngValue, &before, sizeof(before)), 0);
}

TEST("Gym Leader plans preserve Tate and Liza's two ace opening pair")
{
    struct GymLeaderScalingPlan plan;

    EXPECT(BuildGymLeaderScalingPlan(&sTateLizaPlanTestRoster, 0, &plan));
    EXPECT_EQ(plan.count, 2);
    EXPECT_EQ(plan.sourceIndices[0], 0);
    EXPECT_EQ(plan.sourceIndices[1], 1);
    EXPECT_EQ(plan.levels[0], 15);
    EXPECT_EQ(plan.levels[1], 15);
    EXPECT(BuildGymLeaderScalingPlan(&sTateLizaPlanTestRoster, 40, &plan));
    EXPECT_EQ(plan.count, PARTY_SIZE);
    for (u32 i = 0; i < PARTY_SIZE; i++)
        EXPECT_EQ(plan.sourceIndices[i], i);
}

TEST("Gym Leader plans reject malformed metadata before construction")
{
    struct GymLeaderScalingRoster invalid = sGymLeaderPlanTestRoster;
    struct GymLeaderScalingPlan plan;

    invalid.slots[5].battleOrder = invalid.slots[4].battleOrder;
    EXPECT(!BuildGymLeaderScalingPlan(&invalid, 0, &plan));
    invalid = sGymLeaderPlanTestRoster;
    invalid.slots[0].isAce = FALSE;
    EXPECT(!BuildGymLeaderScalingPlan(&invalid, 0, &plan));
    invalid = sGymLeaderPlanTestRoster;
    invalid.slots[1].levelOffset = 0;
    EXPECT(!BuildGymLeaderScalingPlan(&invalid, 0, &plan));
    invalid = sGymLeaderPlanTestRoster;
    invalid.legacyParty = NULL;
    EXPECT(!BuildGymLeaderScalingPlan(&invalid, 0, &plan));
}

TEST("Trainer scaling matches an independent oracle for every Rating and authored level")
{
    u32 level, rating, policy;
    for (policy = TRAINER_SCALING_ORDINARY; policy <= TRAINER_SCALING_GYM_MEMBER; policy++)
    {
        for (level = 1; level <= 100; level++)
        {
            u32 previous = 0;
            for (rating = 0; rating <= 80; rating++)
            {
                u32 actual = GetTrainerScalingLevel(rating, level, policy);
                EXPECT_EQ(actual, ScalingLevelOracle(rating, level, policy == TRAINER_SCALING_GYM_MEMBER));
                EXPECT_GE(actual, previous);
                EXPECT_GE(actual, 1);
                EXPECT_LE(actual, 100);
                previous = actual;
            }
            EXPECT_EQ(GetTrainerScalingLevel(65535, level, policy), previous);
        }
    }
}

TEST("Trainer scaling projection and predecessors consume no random draws")
{
    rng_value_t before = gRngValue;
    GetTrainerScalingLevel(17, 2, TRAINER_SCALING_GYM_MEMBER);
    ResolveTrainerScalingSpecies(SPECIES_CHARIZARD, 7);
    GetTrainerScalingPolicy(TRAINER_ROD_HNS);
    EXPECT_EQ(memcmp(&gRngValue, &before, sizeof(before)), 0);
}

TEST("Trainer scaling reverses numeric chains without wild floors or forward evolution")
{
    EXPECT_EQ(ResolveTrainerScalingSpecies(SPECIES_CHARIZARD, 15), SPECIES_CHARMANDER);
    EXPECT_EQ(ResolveTrainerScalingSpecies(SPECIES_CHARIZARD, 16), SPECIES_CHARMELEON);
    EXPECT_EQ(ResolveTrainerScalingSpecies(SPECIES_CHARIZARD, 35), SPECIES_CHARMELEON);
    EXPECT_EQ(ResolveTrainerScalingSpecies(SPECIES_CHARIZARD, 36), SPECIES_CHARIZARD);
    EXPECT_EQ(ResolveTrainerScalingSpecies(SPECIES_CHARMANDER, 100), SPECIES_CHARMANDER);
    EXPECT_EQ(ResolveTrainerScalingSpecies(SPECIES_SCYTHER, 7), SPECIES_SCYTHER);
    EXPECT_EQ(ResolveTrainerScalingSpecies(SPECIES_RAICHU, 7), SPECIES_RAICHU);
    EXPECT_EQ(ResolveTrainerScalingSpecies(SPECIES_RAICHU_ALOLA, 7), SPECIES_RAICHU_ALOLA);
}

TEST("Trainer scaling legal abilities use the final species")
{
    u32 slot = GetTrainerScalingAbility(SPECIES_BAGON, ABILITY_INTIMIDATE, 123);
    EXPECT_EQ(gSpeciesInfo[SPECIES_BAGON].abilities[slot], ABILITY_ROCK_HEAD);
    slot = GetTrainerScalingAbility(SPECIES_BAGON, ABILITY_SHEER_FORCE, 123);
    EXPECT_EQ(gSpeciesInfo[SPECIES_BAGON].abilities[slot], ABILITY_SHEER_FORCE);
    for (u32 hash = 0; hash < 32; hash++)
    {
        slot = GetTrainerScalingAbility(SPECIES_CATERPIE, ABILITY_NONE, hash);
        EXPECT_LT(slot, ARRAY_COUNT(gSpeciesInfo[SPECIES_CATERPIE].abilities));
        EXPECT_NE(gSpeciesInfo[SPECIES_CATERPIE].abilities[slot], ABILITY_NONE);
    }
}

TEST("Trainer scaling context excludes facilities recordings tutorials and external battles")
{
    static const u32 excluded[] = {
        BATTLE_TYPE_LINK, BATTLE_TYPE_RECORDED, BATTLE_TYPE_RECORDED_LINK,
        BATTLE_TYPE_BATTLE_TOWER, BATTLE_TYPE_DOME, BATTLE_TYPE_PALACE,
        BATTLE_TYPE_ARENA, BATTLE_TYPE_FACTORY, BATTLE_TYPE_PIKE, BATTLE_TYPE_PYRAMID,
        BATTLE_TYPE_TRAINER_HILL, BATTLE_TYPE_EREADER_TRAINER, BATTLE_TYPE_SECRET_BASE,
        BATTLE_TYPE_FIRST_BATTLE, BATTLE_TYPE_CATCH_TUTORIAL, BATTLE_TYPE_POKEDUDE,
        BATTLE_TYPE_INGAME_PARTNER,
    };
    EXPECT(!IsTrainerScalingBattleContext(0));
    for (u32 i = 0; i < ARRAY_COUNT(excluded); i++)
        EXPECT(!IsTrainerScalingBattleContext(BATTLE_TYPE_TRAINER | excluded[i]));
    if (B_TRAINER_PARTY_SCALING)
    {
        EXPECT(IsTrainerScalingBattleContext(BATTLE_TYPE_TRAINER));
        EXPECT(IsTrainerScalingBattleContext(BATTLE_TYPE_TRAINER | BATTLE_TYPE_TWO_OPPONENTS));
    }
}

TEST("Trainer scaling snapshot stays fixed through reconstruction and resets for a retry")
{
    SetTrainerRating(0);
    ResetTrainerScalingSnapshot();
    EXPECT_EQ(GetTrainerScalingSnapshot(), 0);
    SetTrainerRating(80);
    EXPECT_EQ(GetTrainerScalingSnapshot(), 0);
    ResetTrainerScalingSnapshot();
    EXPECT_EQ(GetTrainerScalingSnapshot(), 80);
    ResetTrainerScalingSnapshot();
}

TEST("Trainer scaling policies identify roles by ID and fail closed for invalid IDs")
{
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_JOEY_2_HNS), TRAINER_SCALING_ORDINARY);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_JOEY_5_HNS), TRAINER_SCALING_ORDINARY);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_ROD_HNS), TRAINER_SCALING_GYM_MEMBER);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_FALKNER_1_HNS), TRAINER_SCALING_GYM_LEADER);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_FALKNER_2_HNS), TRAINER_SCALING_EXCLUDED);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_ARCHER_HNS), TRAINER_SCALING_EXCLUDED);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_PROTON_2_HNS), TRAINER_SCALING_EXCLUDED);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_NONE), TRAINER_SCALING_EXCLUDED);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINERS_COUNT), TRAINER_SCALING_EXCLUDED);
    EXPECT_EQ(GetTrainerScalingPolicy(TRAINER_PARTNER(1)), TRAINER_SCALING_EXCLUDED);
    EXPECT_EQ(GetTrainerScalingPolicy(65535), TRAINER_SCALING_EXCLUDED);
}

TEST("Trainer scaling move exceptions require the whole tuple at its active learnset threshold")
{
    u32 modern;
    PARAMETRIZE { modern = FALSE; }
    PARAMETRIZE { modern = TRUE; }
    gSaveBlock3Ptr->challengeSettings.tx_Mode_Modern_Moves = modern;
    struct TrainerMon entry = { .species = SPECIES_CHARMANDER };
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(entry.species);
    u32 threshold = 0;
    EXPECT(!CanRetainTrainerScalingMoves(&entry, entry.species, 100));
    for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        bool32 repeated = FALSE;
        for (u32 j = 0; j < i; j++)
            repeated |= learnset[j].move == learnset[i].move;
        if (learnset[i].level > 1 && !repeated)
        {
            entry.moves[0] = learnset[i].move;
            threshold = learnset[i].level;
            break;
        }
    }
    EXPECT_GT(threshold, 1);
    EXPECT(!CanRetainTrainerScalingMoves(&entry, entry.species, threshold - 1));
    EXPECT(CanRetainTrainerScalingMoves(&entry, entry.species, threshold));
    EXPECT(!CanRetainTrainerScalingMoves(&entry, SPECIES_CHARMELEON, 100));
    entry.moves[1] = MOVE_SPLASH;
    EXPECT(!CanRetainTrainerScalingMoves(&entry, entry.species, 100));
    EXPECT(!HasTrainerScalingMoveException(TRAINER_JOEY_2_HNS, DIFFICULTY_NORMAL, 0));
}

static void PrepareScalingPartyTest(u32 rating)
{
    SetTrainerRating(rating);
    gIsDebugBattle = FALSE;
    ResetTrainerScalingSnapshot();
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = FALSE;
}

static bool32 HasLegalLevelMoves(struct Pokemon *mon)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(GetMonData(mon, MON_DATA_SPECIES));
    u32 level = GetMonData(mon, MON_DATA_LEVEL), usable = 0;
    for (u32 slot = 0; slot < MAX_MON_MOVES; slot++)
    {
        u32 move = GetMonData(mon, MON_DATA_MOVE1 + slot);
        bool32 found = FALSE;
        if (move == MOVE_NONE)
            continue;
        for (u32 entry = 0; learnset[entry].move != LEVEL_UP_MOVE_END; entry++)
            if (learnset[entry].move == move && learnset[entry].level <= level)
                found = TRUE;
        if (!found)
            return FALSE;
        usable += GetMonData(mon, MON_DATA_PP1 + slot) != 0;
    }
    return usable != 0;
}

TEST("Trainer scaling constructs reversed parties with legal moves and retained authored fields")
{
    ASSUME(B_TRAINER_PARTY_SCALING);
    u32 modern;
    PARAMETRIZE { modern = FALSE; }
    PARAMETRIZE { modern = TRUE; }
    PrepareScalingPartyTest(0);
    gSaveBlock3Ptr->challengeSettings.tx_Mode_Modern_Moves = modern;
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, TRAINER_JOEY_2_HNS, TRUE, BATTLE_TYPE_TRAINER), 3);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_CHARMANDER);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 15);
    EXPECT_EQ(GetMonAbility(&party[0]), ABILITY_BLAZE);
    EXPECT_EQ(GetMonGender(&party[0]), MON_FEMALE);
    EXPECT_EQ(GetNature(&party[0]), NATURE_ADAMANT);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_HELD_ITEM), ITEM_LEFTOVERS);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_FRIENDSHIP), 42);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_POKEBALL), BALL_MASTER);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_GIGANTAMAX_FACTOR), FALSE);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_HP_IV), 21);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPDEF_IV), 26);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_HP_EV), 20);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPEED_EV), 32);
    EXPECT_EQ(GetMonData(&party[1], MON_DATA_SPECIES), SPECIES_CHARMANDER);
    EXPECT_EQ(GetMonData(&party[2], MON_DATA_SPECIES), SPECIES_SCYTHER);
    EXPECT_EQ(GetMonData(&party[2], MON_DATA_LEVEL), 7);
    for (u32 i = 0; i < 3; i++)
        EXPECT(HasLegalLevelMoves(&party[i]));
    EXPECT_EQ(GetTrainerStructFromId(TRAINER_JOEY_2_HNS)->party[0].lvl, 60);
    EXPECT_EQ(GetTrainerStructFromId(TRAINER_JOEY_2_HNS)->party[0].species, SPECIES_CHARIZARD);
    Free(party);
}

TEST("Trainer scaling resolves aliases pools and difficulty before projecting selected slots")
{
    ASSUME(B_TRAINER_PARTY_SCALING);
    PrepareScalingPartyTest(0);
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, TRAINER_JOEY_3_HNS, TRUE, BATTLE_TYPE_TRAINER), 3);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_CHARMANDER);
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(party, TRAINER_JOEY_4_HNS, TRUE, BATTLE_TYPE_TRAINER), 2);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_CHARMANDER);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 15);
    EXPECT_EQ(GetMonData(&party[1], MON_DATA_SPECIES), SPECIES_SCYTHER);
    EXPECT_EQ(GetMonData(&party[1], MON_DATA_LEVEL), 14);
    SetCurrentDifficultyLevel(DIFFICULTY_HARD);
    CreateNPCTrainerPartyForOpponent(party, TRAINER_JOEY_5_HNS, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_CHARMANDER);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 15);
    SetCurrentDifficultyLevel(DIFFICULTY_NORMAL);
    CreateNPCTrainerPartyForOpponent(party, TRAINER_JOEY_5_HNS, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_RATTATA);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 7);
    Free(party);
}

TEST("Trainer scaling constructs production roster samples from Johto Kanto and Hoenn")
{
    static const u16 trainers[] = {TRAINER_ABE_HNS, TRAINER_QUINN_HNS, TRAINER_SAWYER_1};
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    u32 rating, trainer;

    ASSUME(B_TRAINER_PARTY_SCALING);
    for (rating = 0; rating <= 80; rating += 40)
    {
        PrepareScalingPartyTest(rating);
        for (trainer = 0; trainer < ARRAY_COUNT(trainers); trainer++)
        {
            u32 count = CreateNPCTrainerPartyForOpponent(party, trainers[trainer], TRUE, BATTLE_TYPE_TRAINER);
            EXPECT_EQ(count, GetTrainerStructFromId(trainers[trainer])->partySize);
            EXPECT(HasLegalLevelMoves(&party[0]));
            EXPECT_NE(GetMonData(&party[0], MON_DATA_LEVEL), 0);
        }
    }
    Free(party);
}

TEST("Trainer scaling keeps mixed opponents independent and shares the battle Rating")
{
    ASSUME(B_TRAINER_PARTY_SCALING);
    PrepareScalingPartyTest(0);
    u32 flags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS;
    gBattleTypeFlags = flags;
    AllocateBattleResources();
    CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_ROD_HNS, TRUE, flags);
    SetTrainerRating(80);
    CreateNPCTrainerPartyForOpponent(&gEnemyParty[3], TRAINER_JOEY_2_HNS, FALSE, flags);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 9);
    EXPECT_EQ(GetMonData(&gEnemyParty[3], MON_DATA_LEVEL), 15);
    EXPECT_EQ(GetMonData(&gEnemyParty[5], MON_DATA_LEVEL), 7);
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanDynamax, (1 << 0) | (1 << 3));
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanTera, (1 << 0) | (1 << 3));
    CreateNPCTrainerPartyForOpponent(&gEnemyParty[3], TRAINER_FALKNER_1_HNS, FALSE, flags);
    EXPECT_EQ(GetMonData(&gEnemyParty[3], MON_DATA_LEVEL), 60);
    EXPECT_EQ(GetMonData(&gEnemyParty[3], MON_DATA_SPECIES), SPECIES_CHARIZARD);
    EXPECT_EQ(GetMonData(&gEnemyParty[3], MON_DATA_MOVE1), MOVE_HYPER_BEAM);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 9);
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanDynamax, 1 << 0);
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanTera, 1 << 0);
    ResetTrainerScalingSnapshot();
    CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_ROD_HNS, TRUE, flags);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 94);
    FreeBattleResources();
}

TEST("Trainer scaling leaves raw player partner debug and recorded construction authored")
{
    PrepareScalingPartyTest(0);
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    const struct Trainer *trainer = GetTrainerStructFromId(TRAINER_JOEY_2_HNS);
    CreateNPCTrainerPartyFromTrainer(party, trainer, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 60);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_CHARIZARD);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_MOVE1), MOVE_HYPER_BEAM);
    CreateNPCTrainerPartyForOpponent(party, TRAINER_JOEY_2_HNS, TRUE, BATTLE_TYPE_TRAINER | BATTLE_TYPE_RECORDED);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_LEVEL), 60);
    EXPECT_EQ(GetMonData(&party[0], MON_DATA_SPECIES), SPECIES_CHARIZARD);
    Free(party);
}

TEST("Trainer scaling leaves excluded facility gimmick state untouched")
{
    u32 flags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_FRONTIER;
    AllocateBattleResources();
    gBattleStruct->opponentMonCanDynamax = (1 << PARTY_SIZE) - 1;
    gBattleStruct->opponentMonCanTera = (1 << PARTY_SIZE) - 1;
    CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_JOEY_2_HNS, TRUE, flags);
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanDynamax, (1 << PARTY_SIZE) - 1);
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanTera, (1 << PARTY_SIZE) - 1);
    FreeBattleResources();
}

TEST("Trainer scaling preserves second opponent gimmick slots outside scaling contexts")
{
    u32 battleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_RECORDED;

    gBattleTypeFlags = battleTypeFlags;
    AllocateBattleResources();
    CreateNPCTrainerPartyForOpponent(&gEnemyParty[PARTY_SIZE / 2], TRAINER_JOEY_2_HNS, FALSE, battleTypeFlags);
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanDynamax, 1 << (PARTY_SIZE / 2));
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanTera, 1 << (PARTY_SIZE / 2));
    FreeBattleResources();
}

TEST("Trainer scaling randomization retains the authored mapping inputs and scales levels")
{
    ASSUME(B_TRAINER_PARTY_SCALING);
    PrepareScalingPartyTest(0);
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = TRUE;
    struct Pokemon *party = AllocZeroed(PARTY_SIZE * sizeof(*party));
    const struct Trainer *trainer = GetTrainerStructFromId(TRAINER_JOEY_2_HNS);
    u16 expected[3];
    for (u32 i = 0; i < 3; i++)
        expected[i] = RandomizeTrainerMon(trainer->trainerClass, i, 3, trainer->party[i].species);
    CreateNPCTrainerPartyForOpponent(party, TRAINER_JOEY_2_HNS, TRUE, BATTLE_TYPE_TRAINER);
    for (u32 i = 0; i < 3; i++)
    {
        EXPECT_EQ(GetMonData(&party[i], MON_DATA_SPECIES), expected[i]);
        EXPECT_EQ(GetMonData(&party[i], MON_DATA_LEVEL), i == 2 ? 7 : 15);
        EXPECT(HasLegalLevelMoves(&party[i]));
        u32 ability = GetMonAbility(&party[i]);
        bool32 valid = FALSE;
        for (u32 slot = 0; slot < ARRAY_COUNT(gSpeciesInfo[expected[i]].abilities); slot++)
            valid |= ability == gSpeciesInfo[expected[i]].abilities[slot] && ability != ABILITY_NONE;
        EXPECT(valid);
        u32 ratio = gSpeciesInfo[expected[i]].genderRatio;
        if (ratio == MON_MALE || ratio == MON_FEMALE || ratio == MON_GENDERLESS)
            EXPECT_EQ(GetMonGender(&party[i]), ratio);
    }
    gSaveBlock3Ptr->challengeSettings.tx_Random_Trainer = FALSE;
    Free(party);
}

TEST("Trainer scaling preserves defeat flags and ignores player party levels")
{
    ASSUME(B_TRAINER_PARTY_SCALING);
    PrepareScalingPartyTest(40);
    SetTrainerFlag(TRAINER_JOEY_2_HNS);
    CreateMon(&gPlayerParty[0], SPECIES_MAGIKARP, 100, 0, OTID_STRUCT_RANDOM_NO_SHINY);
    CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_JOEY_2_HNS, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 42);
    EXPECT(HasTrainerBeenFought(TRAINER_JOEY_2_HNS));
    CreateMon(&gPlayerParty[0], SPECIES_MAGIKARP, 1, 0, OTID_STRUCT_RANDOM_NO_SHINY);
    CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_JOEY_2_HNS, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 42);
    EXPECT(HasTrainerBeenFought(TRAINER_JOEY_2_HNS));
    ClearTrainerFlag(TRAINER_JOEY_2_HNS);
}

TEST("Trainer scaling challenge IV and EV options do not replace Rating levels")
{
    ASSUME(B_TRAINER_PARTY_SCALING);
    PrepareScalingPartyTest(80);
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = 1;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = 1;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_LevelCap = 1;
    CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_JOEY_2_HNS, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 100);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_HP_IV), GetCurrentTrainerIVs());
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_HP_EV), GetCurrentTrainerEVs());
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingIVs = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_TrainerScalingEVs = FALSE;
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_LevelCap = FALSE;
}

TEST("Trainer scaling rejects invalid opponents without altering supplied parties")
{
    PrepareScalingPartyTest(0);
    CreateMon(&gEnemyParty[0], SPECIES_MAGIKARP, 10, 0, OTID_STRUCT_RANDOM_NO_SHINY);
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, 65535, TRUE, BATTLE_TYPE_TRAINER), 0);
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_PARTNER(1), TRUE, BATTLE_TYPE_TRAINER), 0);
    EXPECT_EQ(CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_NONE, TRUE, BATTLE_TYPE_TRAINER), 0);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), SPECIES_MAGIKARP);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 10);
}

TEST("Trainer scaling rollback bypasses level species and move changes together")
{
    ASSUME(!B_TRAINER_PARTY_SCALING);
    PrepareScalingPartyTest(0);
    AllocateBattleResources();
    gBattleStruct->opponentMonCanDynamax = (1 << PARTY_SIZE) - 1;
    gBattleStruct->opponentMonCanTera = (1 << PARTY_SIZE) - 1;
    CreateNPCTrainerPartyForOpponent(gEnemyParty, TRAINER_JOEY_2_HNS, TRUE, BATTLE_TYPE_TRAINER);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_SPECIES), SPECIES_CHARIZARD);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_LEVEL), 60);
    EXPECT_EQ(GetMonData(&gEnemyParty[0], MON_DATA_MOVE1), MOVE_HYPER_BEAM);
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanDynamax, (1 << PARTY_SIZE) - 1);
    EXPECT_EQ((u32)gBattleStruct->opponentMonCanTera, (1 << PARTY_SIZE) - 1);
    FreeBattleResources();
}

#endif
