#include "global.h"
#include "test/test.h"
#include "trainer_party_scaling.h"

// Original phase-B baseline bodies are an independent timing reference.
// TEST code generation differs from release LTO; report these separately.
u8 BeforeGetLeagueScalingBaseline(u32 rating)
{
    static const u8 anchors[][2] = {{0, 15}, {4, 16}, {8, 18}, {16, 23}, {30, 30}, {40, 42}, {55, 60}, {65, 80}, {80, 100}};
    u32 i;
    rating = min(rating, 80);
    for (i = 1; i < ARRAY_COUNT(anchors); i++)
    {
        if (rating <= anchors[i][0])
        {
            u32 width = anchors[i][0] - anchors[i - 1][0];
            u32 rise = (rating - anchors[i - 1][0]) * (anchors[i][1] - anchors[i - 1][1]);
            return anchors[i - 1][1] + (2 * rise + width) / (2 * width);
        }
    }
    return 100;
}

u8 BeforeGetLeagueScalingLevel(u32 rating, s8 encounterOffset, s8 slotOffset)
{
    s32 level = BeforeGetLeagueScalingBaseline(rating) + encounterOffset + slotOffset;
    return min(max(level, 1), 100);
}

u8 BeforeGetTrainerScalingLevel(u32 rating, u32 authoredLevel, u32 policy)
{
    static const u8 anchors[][2] = {{0, 7}, {4, 8}, {8, 10}, {16, 15}, {30, 22}, {40, 34}, {55, 52}, {65, 72}, {80, 92}};
    u32 i;
    s32 adjustment, level;
    rating = min(rating, 80);
    authoredLevel = min(max(authoredLevel, 1), 100);
    adjustment = (s32)authoredLevel - 5;
    adjustment = adjustment < 0 ? -((-adjustment + 2) / 5) : (adjustment + 2) / 5;
    adjustment = min(max(adjustment, -1), 8);
    level = 92;
    for (i = 1; i < ARRAY_COUNT(anchors); i++)
    {
        if (rating <= anchors[i][0])
        {
            u32 width = anchors[i][0] - anchors[i - 1][0];
            u32 rise = (rating - anchors[i - 1][0]) * (anchors[i][1] - anchors[i - 1][1]);
            level = anchors[i - 1][1] + (2 * rise + width) / (2 * width);
            break;
        }
    }
    return min(max(level + adjustment + (policy == TRAINER_SCALING_GYM_MEMBER ? 2 : 0), 1), 100);
}

u8 BeforeGetGymLeaderScalingLevel(u32 rating, s8 levelOffset)
{
    static const u8 anchors[][2] = {{0, 15}, {4, 16}, {8, 18}, {16, 23}, {30, 30}, {40, 42}, {55, 60}, {65, 80}, {80, 100}};
    u32 i;
    s32 level = 100;

    rating = min(rating, 80);
    for (i = 1; i < ARRAY_COUNT(anchors); i++)
    {
        if (rating <= anchors[i][0])
        {
            u32 width = anchors[i][0] - anchors[i - 1][0];
            u32 rise = (rating - anchors[i - 1][0]) * (anchors[i][1] - anchors[i - 1][1]);
            level = anchors[i - 1][1] + (2 * rise + width) / (2 * width);
            break;
        }
    }
    return min(max(level + levelOffset, 1), 100);
}

#define MEASURE_QUERY(result, cycles, expression) do { \
    u16 ime = REG_IME; \
    u16 timer2Control, timer3Control, timer2Count, timer3Count; \
    REG_IME = 0; \
    timer2Control = REG_TM2CNT_H; \
    timer3Control = REG_TM3CNT_H; \
    timer2Count = REG_TM2CNT_L; \
    timer3Count = REG_TM3CNT_L; \
    CycleCountStart(); \
    result = (expression); \
    cycles = CycleCountEnd(); \
    REG_TM2CNT_L = timer2Count; \
    REG_TM3CNT_L = timer3Count; \
    REG_TM3CNT_H = timer3Control; \
    REG_TM2CNT_H = timer2Control; \
    REG_IME = ime; \
} while (0)

TEST("Gameplay progression reports bounded query timing against phase B baseline")
{
    u8 (*volatile beforeBaseline)(u32) = BeforeGetLeagueScalingBaseline;
    u8 (*volatile afterBaseline)(u32) = GetLeagueScalingBaseline;
    u8 (*volatile beforeLeague)(u32, s8, s8) = BeforeGetLeagueScalingLevel;
    u8 (*volatile afterLeague)(u32, s8, s8) = GetLeagueScalingLevel;
    u8 (*volatile beforeOrdinary)(u32, u32, u32) = BeforeGetTrainerScalingLevel;
    u8 (*volatile afterOrdinary)(u32, u32, u32) = GetTrainerScalingLevel;
    u8 (*volatile beforeGym)(u32, s8) = BeforeGetGymLeaderScalingLevel;
    u8 (*volatile afterGym)(u32, s8) = GetGymLeaderScalingLevel;
    u32 rating;
    s32 maximum[4] = { 0, 0, 0, 0 };
    u32 beforeCycles, afterCycles;
    volatile u8 beforeValue, afterValue;
    for (rating = 0; rating <= 80; rating++)
    {
        MEASURE_QUERY(beforeValue, beforeCycles, beforeBaseline(rating));
        MEASURE_QUERY(afterValue, afterCycles, afterBaseline(rating));
        maximum[0] = max(maximum[0], (s32)afterCycles - (s32)beforeCycles);
        EXPECT_EQ(beforeValue, afterValue);
        MEASURE_QUERY(beforeValue, beforeCycles, beforeLeague(rating, 4, -2));
        MEASURE_QUERY(afterValue, afterCycles, afterLeague(rating, 4, -2));
        maximum[1] = max(maximum[1], (s32)afterCycles - (s32)beforeCycles);
        EXPECT_EQ(beforeValue, afterValue);
        MEASURE_QUERY(beforeValue, beforeCycles, beforeOrdinary(rating, 50, TRAINER_SCALING_GYM_MEMBER));
        MEASURE_QUERY(afterValue, afterCycles, afterOrdinary(rating, 50, TRAINER_SCALING_GYM_MEMBER));
        maximum[2] = max(maximum[2], (s32)afterCycles - (s32)beforeCycles);
        EXPECT_EQ(beforeValue, afterValue);
        MEASURE_QUERY(beforeValue, beforeCycles, beforeGym(rating, -2));
        MEASURE_QUERY(afterValue, afterCycles, afterGym(rating, -2));
        maximum[3] = max(maximum[3], (s32)afterCycles - (s32)beforeCycles);
        EXPECT_EQ(beforeValue, afterValue);
    }
    Test_MgbaPrintf("Progression TEST maximum cycle deltas: league baseline=%d level=%d ordinary=%d Gym=%d", maximum[0], maximum[1], maximum[2], maximum[3]);
}
