#include "global.h"
#include "trainer_tower.h"
#include "test/test.h"
#include "constants/items.h"

#if IS_WAYFARER
TEST("Wayfarer Trainer Tower normalizes legal levels and has fixed source prizes")
{
    EXPECT_EQ(WayfarerTrainerTowerNormalizeLevel(0), 1);
    EXPECT_EQ(WayfarerTrainerTowerNormalizeLevel(1), 1);
    EXPECT_EQ(WayfarerTrainerTowerNormalizeLevel(100), 100);
    EXPECT_EQ(WayfarerTrainerTowerNormalizeLevel(255), 100);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(CHALLENGE_TYPE_SINGLE), ITEM_UP_GRADE);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(CHALLENGE_TYPE_DOUBLE), ITEM_DRAGON_SCALE);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(CHALLENGE_TYPE_KNOCKOUT), ITEM_METAL_COAT);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(CHALLENGE_TYPE_MIXED), ITEM_KINGS_ROCK);
    EXPECT_EQ(WayfarerTrainerTowerGetPrize(NUM_TOWER_CHALLENGE_TYPES), ITEM_NONE);
}

TEST("Wayfarer Trainer Tower mixed format selects the frozen floor variants")
{
    static const u8 expected[MAX_TRAINER_TOWER_FLOORS] =
    {
        CHALLENGE_TYPE_SINGLE,
        CHALLENGE_TYPE_DOUBLE,
        CHALLENGE_TYPE_KNOCKOUT,
        CHALLENGE_TYPE_DOUBLE,
        CHALLENGE_TYPE_DOUBLE,
        CHALLENGE_TYPE_KNOCKOUT,
        CHALLENGE_TYPE_DOUBLE,
        CHALLENGE_TYPE_KNOCKOUT,
    };
    u8 floor;

    for (floor = 0; floor < MAX_TRAINER_TOWER_FLOORS; floor++)
        EXPECT_EQ(WayfarerTrainerTowerGetFloorChallengeType(CHALLENGE_TYPE_MIXED, floor), expected[floor]);
    EXPECT_EQ(WayfarerTrainerTowerGetFloorChallengeType(CHALLENGE_TYPE_MIXED, MAX_TRAINER_TOWER_FLOORS), NUM_TOWER_CHALLENGE_TYPES);
}

TEST("Wayfarer Trainer Tower records use zero as empty and only retain improvements")
{
    struct WayfarerSeviiTrainerTowerRecords records = {0};

    EXPECT(!WayfarerTrainerTowerRecordTime(&records, CHALLENGE_TYPE_SINGLE, 0));
    EXPECT(!WayfarerTrainerTowerRecordTime(&records, NUM_TOWER_CHALLENGE_TYPES, 60));
    EXPECT(WayfarerTrainerTowerRecordTime(&records, CHALLENGE_TYPE_SINGLE, 600));
    EXPECT_EQ(records.bestTime[CHALLENGE_TYPE_SINGLE], 600);
    EXPECT_EQ(records.completedMask, 1 << CHALLENGE_TYPE_SINGLE);
    EXPECT(!WayfarerTrainerTowerRecordTime(&records, CHALLENGE_TYPE_SINGLE, 600));
    EXPECT(!WayfarerTrainerTowerRecordTime(&records, CHALLENGE_TYPE_SINGLE, 601));
    EXPECT(WayfarerTrainerTowerRecordTime(&records, CHALLENGE_TYPE_SINGLE, 599));
    EXPECT_EQ(records.bestTime[CHALLENGE_TYPE_SINGLE], 599);
    EXPECT(!WayfarerTrainerTowerRecordTime(&records, CHALLENGE_TYPE_DOUBLE, TRAINER_TOWER_MAX_TIME + 1));
}
#endif
