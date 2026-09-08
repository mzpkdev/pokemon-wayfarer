#include "global.h"
#include "capture_context.h"
#include "test/test.h"

TEST("Capture proximity is neutral at entry for every species rate")
{
    for (u32 rate = 0; rate <= 255; rate++)
    {
        u32 initial = CaptureInitialCatchFactor(rate);
        EXPECT_GE(initial, 1);
        EXPECT_LE(initial, 20);
        EXPECT_EQ(CaptureApplyProximity(rate, initial, initial), rate);
        EXPECT_EQ(CaptureApplyProximity(rate, 0, 0), rate);
    }
}

TEST("Capture proximity preserves low factors, fractional floors and rates above 255")
{
    EXPECT_EQ(CaptureInitialCatchFactor(3), 1);
    EXPECT_EQ(CaptureInitialCatchFactor(64), 5);
    EXPECT_EQ(CaptureInitialCatchFactor(255), 20);
    EXPECT_EQ(CaptureApplyProximity(3, 1, 20), 60);
    EXPECT_EQ(CaptureApplyProximity(64, 5, 9), 115);
    EXPECT_EQ(CaptureApplyProximity(64, 5, 12), 153);
    EXPECT_EQ(CaptureApplyProximity(64, 5, 14), 179);
    EXPECT_EQ(CaptureApplyProximity(64, 5, 15), 192);
    EXPECT_EQ(CaptureApplyProximity(150, 11, 20), 272);
    EXPECT_EQ(CaptureApplyProximity(255, 20, 20), 255);
}
