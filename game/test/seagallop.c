#include "global.h"
#include "item.h"
#include "seagallop.h"
#include "test/test.h"

#if IS_FRLG
static void FillSeviiTestKeyItemsPocket(u32 freeSlots)
{
    struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];

    ClearBag();
    for (u32 i = 0; i + freeSlots < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_BICYCLE, 1);
}

TEST("Seagallop: original trip preflight reserves every missing Key Item slot")
{
    FillSeviiTestKeyItemsPocket(3);
    EXPECT(CanStartOriginalSeviiTrip());

    FillSeviiTestKeyItemsPocket(2);
    EXPECT(!CanStartOriginalSeviiTrip());
}

TEST("Seagallop: original trip preflight counts only missing forced items")
{
    FillSeviiTestKeyItemsPocket(3);
    EXPECT(AddBagItem(ITEM_TOWN_MAP, 1));
    EXPECT(AddBagItem(ITEM_RAINBOW_PASS, 1));
    EXPECT(CanStartOriginalSeviiTrip());

    FillSeviiTestKeyItemsPocket(3);
    EXPECT(AddBagItem(ITEM_TOWN_MAP, 1));
    EXPECT(AddBagItem(ITEM_TRI_PASS, 1));
    EXPECT(CanStartOriginalSeviiTrip());
}

TEST("Seagallop: original trip preflight accepts a fully reconciled pocket")
{
    FillSeviiTestKeyItemsPocket(0);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 0, ITEM_TOWN_MAP, 1);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 1, ITEM_METEORITE, 1);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 2, ITEM_RAINBOW_PASS, 1);
    EXPECT(CanStartOriginalSeviiTrip());
}

TEST("Seagallop: Ruby handoff atomically replaces the Tri-Pass in a full pocket")
{
    FillSeviiTestKeyItemsPocket(0);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 0, ITEM_RUBY, 1);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 1, ITEM_TRI_PASS, 1);

    EXPECT(TryCompleteSeviiRubyHandoff());
    EXPECT(!CheckBagHasItem(ITEM_RUBY, 1));
    EXPECT(!CheckBagHasItem(ITEM_TRI_PASS, 1));
    EXPECT(CheckBagHasItem(ITEM_RAINBOW_PASS, 1));
}

TEST("Seagallop: Ruby handoff preserves a preexisting Rainbow Pass and Tri-Pass")
{
    FillSeviiTestKeyItemsPocket(0);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 0, ITEM_RUBY, 1);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 1, ITEM_TRI_PASS, 1);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 2, ITEM_RAINBOW_PASS, 1);

    EXPECT(TryCompleteSeviiRubyHandoff());
    EXPECT(!CheckBagHasItem(ITEM_RUBY, 1));
    EXPECT(CheckBagHasItem(ITEM_TRI_PASS, 1));
    EXPECT(CheckBagHasItem(ITEM_RAINBOW_PASS, 1));
}

TEST("Seagallop: Ruby handoff handles a missing Tri-Pass")
{
    FillSeviiTestKeyItemsPocket(0);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 0, ITEM_RUBY, 1);

    EXPECT(TryCompleteSeviiRubyHandoff());
    EXPECT(!CheckBagHasItem(ITEM_RUBY, 1));
    EXPECT(CheckBagHasItem(ITEM_RAINBOW_PASS, 1));
}

TEST("Seagallop: Ruby handoff fails without consuming another item when Ruby is missing")
{
    FillSeviiTestKeyItemsPocket(0);
    BagPocket_SetSlotItemIdAndCount(&gBagPockets[POCKET_KEY_ITEMS], 0, ITEM_TRI_PASS, 1);

    EXPECT(!TryCompleteSeviiRubyHandoff());
    EXPECT(CheckBagHasItem(ITEM_TRI_PASS, 1));
    EXPECT(!CheckBagHasItem(ITEM_RAINBOW_PASS, 1));
}
#endif
