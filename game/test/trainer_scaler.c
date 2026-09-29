#include "global.h"
#include "test/test.h"
#include "trainer_scaler.h"

TEST("Trainer scaler interpolates half-up and holds step anchors")
{
    static const struct TrainerScalerAnchor anchors[] = {{0, 5}, {20, 14}, {40, 28}, {80, 50}, {160, 100}};
    EXPECT(ValidateTrainerScaler(anchors, ARRAY_COUNT(anchors)));
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 0, FALSE), 5);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 10, FALSE), 10);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 20, FALSE), 14);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 30, FALSE), 21);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 19, TRUE), 5);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 20, TRUE), 14);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 39, TRUE), 14);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 0xFFFFFFFF, FALSE), 100);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 0xFFFFFFFF, TRUE), 100);
}

TEST("Trainer scaler handles wide arithmetic without wrapping")
{
    static const struct TrainerScalerAnchor anchors[] = {{0, 0}, {0xFFFFFFFE, 65535}};
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 0x7FFFFFFF, FALSE), 32768);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 0xFFFFFFFE, FALSE), 65535);
    EXPECT_EQ(EvaluateTrainerScaler(anchors, ARRAY_COUNT(anchors), 0xFFFFFFFF, FALSE), 65535);
}

TEST("Trainer scaler rejects malformed authored curves")
{
    static const struct TrainerScalerAnchor start[] = {{1, 1}};
    static const struct TrainerScalerAnchor repeated[] = {{0, 1}, {0, 2}};
    static const struct TrainerScalerAnchor descending[] = {{0, 2}, {1, 1}};
    static const struct TrainerScalerAnchor constant[] = {{0, 7}};
    EXPECT(!ValidateTrainerScaler(NULL, 0));
    EXPECT(!ValidateTrainerScaler(start, ARRAY_COUNT(start)));
    EXPECT(!ValidateTrainerScaler(repeated, ARRAY_COUNT(repeated)));
    EXPECT(!ValidateTrainerScaler(descending, ARRAY_COUNT(descending)));
    EXPECT(ValidateTrainerScaler(constant, ARRAY_COUNT(constant)));
    EXPECT_EQ(EvaluateTrainerScaler(constant, ARRAY_COUNT(constant), 999, FALSE), 7);
}
