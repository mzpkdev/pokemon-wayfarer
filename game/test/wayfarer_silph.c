#include "global.h"
#include "test/test.h"
#include "item.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "constants/vars.h"

#if IS_WAYFARER
TEST("Wayfarer Silph key and state remain local")
{
    EXPECT_NE(ITEM_SILPH_CARD_KEY, ITEM_CARD_KEY);
    EXPECT_EQ(GetItemPocket(ITEM_SILPH_CARD_KEY), POCKET_KEY_ITEMS);
    EXPECT_EQ(GetItemImportance(ITEM_SILPH_CARD_KEY), 1);
    EXPECT_EQ(FLAG_HIDE_SILPH_CO_5F_CARD_KEY, FLAG_SILPH_CARD_KEY_RECEIVED_HNS);
    EXPECT_NE(FLAG_SILPH_CARD_KEY_RECEIVED_HNS, FLAG_GOT_UP_GRADE);
    EXPECT_NE(FLAG_SILPH_MASTER_BALL_REWARD_PENDING_HNS, FLAG_SILPH_MASTER_BALL_RECEIVED_HNS);
    EXPECT_NE(VAR_SILPH_ELEVATOR_FLOOR_HNS, VAR_SILPH_GIOVANNI_SCENE_HNS);
}
#endif
