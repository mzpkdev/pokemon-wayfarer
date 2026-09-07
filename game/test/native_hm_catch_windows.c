#include "global.h"
#include "move_relearner.h"
#include "pokemon.h"
#include "test/test.h"

#if IS_WAYFARER

struct ExpectedCatchEntry
{
    u8 level;
    u16 move;
};

struct ExpectedCatchMoves
{
    u8 level;
    u16 moves[MAX_MON_MOVES];
};

struct ExpectedCatchSpecies
{
    u16 species;
    bool8 modern;
    u8 entryCount;
    const struct ExpectedCatchEntry *entries;
    u8 movesetCount;
    const struct ExpectedCatchMoves *movesets;
};

#include "data/native_hm_catch_windows.h"

static void SelectCatchMode(bool8 modern)
{
    gSaveBlock3Ptr->challengeSettings.tx_Mode_Modern_Moves = modern;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Moves = FALSE;
}

static bool32 CatchMonKnowsMove(struct Pokemon *mon, u16 move)
{
    for (u32 slot = 0; slot < MAX_MON_MOVES; slot++)
        if (GetMonData(mon, MON_DATA_MOVE1 + slot) == move)
            return TRUE;
    return FALSE;
}

static bool32 ReminderHasMove(struct Pokemon *mon, u16 move)
{
    u16 moves[MAX_RELEARNER_MOVES];
    u32 count = GetBoxMonRelearnableLevelUpMoves(&mon->box, moves);

    for (u32 i = 0; i < count; i++)
        if (moves[i] == move)
            return TRUE;
    return FALSE;
}

TEST("Wayfarer catch windows preserve every approved ordered native and added entry")
{
    u32 id = 0;

    for (u32 i = 0; i < ARRAY_COUNT(sCatchSpecies); i++)
        PARAMETRIZE_LABEL("species %d modern %d", sCatchSpecies[i].species, sCatchSpecies[i].modern) { id = i; }

    {
        const struct ExpectedCatchSpecies *expected = &sCatchSpecies[id];
        const struct LevelUpMove *actual;

        SelectCatchMode(expected->modern);
        actual = GetSpeciesLevelUpLearnset(expected->species);
        EXPECT_LT(expected->entryCount, MAX_LEVEL_UP_MOVES);
        EXPECT_LT(expected->entryCount, MAX_RELEARNER_MOVES);
        for (u32 i = 0; i < expected->entryCount; i++)
        {
            EXPECT_EQ(actual[i].level, expected->entries[i].level);
            EXPECT_EQ(actual[i].move, expected->entries[i].move);
        }
        EXPECT_EQ(actual[expected->entryCount].move, LEVEL_UP_MOVE_END);
    }
}

TEST("Wayfarer catch windows match all four production move slots at every level in both modes")
{
    u32 id = 0;

    for (u32 i = 0; i < ARRAY_COUNT(sCatchSpecies); i++)
        PARAMETRIZE_LABEL("species %d modern %d", sCatchSpecies[i].species, sCatchSpecies[i].modern) { id = i; }

    {
        const struct ExpectedCatchSpecies *expected = &sCatchSpecies[id];
        u32 moveset = 0;

        SelectCatchMode(expected->modern);
        for (u32 level = 1; level <= MAX_LEVEL; level++)
        {
            struct Pokemon mon;

            while (moveset + 1 < expected->movesetCount && expected->movesets[moveset + 1].level <= level)
                moveset++;
            CreateMon(&mon, expected->species, level, 0, OTID_STRUCT_PRESET(0));
            GiveMonInitialMoveset(&mon);
            for (u32 slot = 0; slot < MAX_MON_MOVES; slot++)
            {
                u32 actual = GetMonData(&mon, MON_DATA_MOVE1 + slot);

                if (actual != expected->movesets[moveset].moves[slot])
                    Test_ExitWithResult(TEST_RESULT_FAIL, __LINE__,
                        ":L%s:%d: species %d mode %d level %d slot %d: got %d expected %d",
                        gTestRunnerState.test->filename, __LINE__, expected->species,
                        expected->modern, level, slot, actual, expected->movesets[moveset].moves[slot]);
            }
        }
    }
}

TEST("Wayfarer catch windows do not remove owned moves after a window or evolution")
{
    for (u32 mode = 0; mode < 2; mode++)
    {
        struct Pokemon mon;
        u32 evolved = SPECIES_AMBIPOM;
        u32 experience;

        SelectCatchMode(mode);
        CreateMon(&mon, SPECIES_AIPOM, 10, 0, OTID_STRUCT_PRESET(0));
        GiveMonInitialMoveset(&mon);
        ASSUME(CatchMonKnowsMove(&mon, MOVE_ROCK_SMASH));
        for (u32 level = 11; level <= MAX_LEVEL; level++)
        {
            enum Move offered = MonTryLearningNewMoveAtLevel(&mon, TRUE, level);

            while (offered != MOVE_NONE)
                offered = MonTryLearningNewMoveAtLevel(&mon, FALSE, level);
            EXPECT(CatchMonKnowsMove(&mon, MOVE_ROCK_SMASH));
        }
        experience = gExperienceTables[gSpeciesInfo[SPECIES_AIPOM].growthRate][MAX_LEVEL];
        SetMonData(&mon, MON_DATA_EXP, &experience);
        CalculateMonStats(&mon);
        EXPECT(CatchMonKnowsMove(&mon, MOVE_ROCK_SMASH));
        SetMonData(&mon, MON_DATA_SPECIES, &evolved);
        CalculateMonStats(&mon);
        EXPECT(CatchMonKnowsMove(&mon, MOVE_ROCK_SMASH));
        for (u32 slot = 0; slot < MAX_MON_MOVES; slot++)
            SetMonMoveSlot(&mon, MOVE_NONE, slot);
        EXPECT(!ReminderHasMove(&mon, MOVE_ROCK_SMASH));
    }
}

TEST("Wayfarer listed descendants relearn their own entries without blanket level-one copies")
{
    for (u32 mode = 0; mode < 2; mode++)
    {
        struct Pokemon mon;

        SelectCatchMode(mode);
        CreateMon(&mon, SPECIES_GOLDUCK, MAX_LEVEL, 0, OTID_STRUCT_PRESET(0));
        for (u32 slot = 0; slot < MAX_MON_MOVES; slot++)
            SetMonMoveSlot(&mon, MOVE_NONE, slot);
        EXPECT(ReminderHasMove(&mon, MOVE_SURF));
        EXPECT(ReminderHasMove(&mon, MOVE_DIVE));
        SetMonMoveSlot(&mon, MOVE_SURF, 0);
        EXPECT(!ReminderHasMove(&mon, MOVE_SURF));
        EXPECT(ReminderHasMove(&mon, MOVE_DIVE));
    }
}

#endif // IS_WAYFARER
