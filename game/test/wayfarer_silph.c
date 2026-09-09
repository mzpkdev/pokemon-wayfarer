#include "global.h"
#include "test/test.h"
#include "item.h"
#include "event_data.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "constants/vars.h"

#if IS_WAYFARER
TEST("Wayfarer Silph key remains independent of Goldenrod access")
{
    EXPECT_EQ(ITEM_CARD_KEY, 750);
    EXPECT_EQ(ITEM_HM09, 901);
    EXPECT_EQ(ITEM_SILPH_CARD_KEY, 902);
    EXPECT_EQ(ITEMS_COUNT, 903);
    EXPECT_EQ(GetItemPocket(ITEM_SILPH_CARD_KEY), POCKET_KEY_ITEMS);
    EXPECT_EQ(GetItemImportance(ITEM_SILPH_CARD_KEY), 1);
    EXPECT_EQ(VAR_SILPH_ELEVATOR_FLOOR_HNS, 0x40DA);
    EXPECT_EQ(VAR_SILPH_GIOVANNI_SCENE_HNS, 0x40DB);
    EXPECT_EQ(FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS, 0x4F8);
    EXPECT_EQ(FLAG_SILPH_MASTER_BALL_RECEIVED_HNS, 0x4F9);
}
#endif
