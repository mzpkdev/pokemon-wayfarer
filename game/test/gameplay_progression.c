#include "global.h"
#include "gameplay_progression.h"
#include "test/test.h"

TEST("Gameplay progression rejects invalid IDs without modifying output")
{
    u8 output = 37;
    EXPECT_EQ(EvaluateGameplayCurve(0xFFFF, 40, &output), FALSE);
    EXPECT_EQ(output, 37);
    EXPECT_EQ(EvaluateGameplayCurve(GAMEPLAY_CURVE_SOFT_CAP, 40, NULL), FALSE);
}

TEST("Gameplay progression shares exact points and clamps oversized input")
{
    static const u8 ratings[] = { 0, 4, 8, 16, 30, 40, 55, 65, 80 };
    static const u8 expected[] = { 15, 16, 18, 23, 30, 42, 60, 80, 100 };
    u32 i;
    u8 output = 0;
    for (i = 0; i < ARRAY_COUNT(ratings); i++)
    {
        EXPECT_EQ(EvaluateGameplayCurve(GAMEPLAY_CURVE_SOFT_CAP, ratings[i], &output), TRUE);
        EXPECT_EQ(output, expected[i]);
    }
    EXPECT_EQ(EvaluateGameplayCurve(GAMEPLAY_CURVE_SOFT_CAP, 0xFFFFFFFF, &output), TRUE);
    EXPECT_EQ(output, 100);
}
