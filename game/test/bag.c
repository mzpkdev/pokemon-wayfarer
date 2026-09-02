#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "item_menu.h"
#include "pokemon.h"
#include "test/overworld_script.h"
#include "test/test.h"

#if IS_FRLG
static const u16 sStandardRodContributorFlags[] =
{
    FLAG_GOT_OLD_ROD,
    FLAG_GOT_GOOD_ROD,
    FLAG_GOT_SUPER_ROD,
};
#elif IS_HNS
static const u16 sStandardRodContributorFlags[] =
{
    FLAG_STANDARD_ROD_ROUTE32_CONTRIBUTED,
    FLAG_STANDARD_ROD_OLIVINE_CONTRIBUTED,
    FLAG_STANDARD_ROD_ROUTE12_CONTRIBUTED,
};
STATIC_ASSERT(FLAG_STANDARD_ROD_ROUTE32_CONTRIBUTED == 0x304, StandardRodRoute32FlagId);
STATIC_ASSERT(FLAG_STANDARD_ROD_OLIVINE_CONTRIBUTED == 0x305, StandardRodOlivineFlagId);
STATIC_ASSERT(FLAG_STANDARD_ROD_ROUTE12_CONTRIBUTED == 0x306, StandardRodRoute12FlagId);
STATIC_ASSERT(FLAG_HNS_MAGNET_TRAIN_RESTORATION_STARTED == 0x307, MagnetTrainRestorationStartedFlagId);
#else
static const u16 sStandardRodContributorFlags[] =
{
    FLAG_RECEIVED_OLD_ROD,
    FLAG_RECEIVED_GOOD_ROD,
    FLAG_RECEIVED_SUPER_ROD,
};
#endif

static const u8 sStandardRodVisitOrders[][3] =
{
    {0, 1, 2},
    {0, 2, 1},
    {1, 0, 2},
    {1, 2, 0},
    {2, 0, 1},
    {2, 1, 0},
};

static void ResetStandardRodTestState(void)
{
    ClearBag();
    for (u32 i = 0; i < ARRAY_COUNT(sStandardRodContributorFlags); i++)
        FlagClear(sStandardRodContributorFlags[i]);
    gSaveBlock1Ptr->registeredItem = ITEM_NONE;
    gSaveBlock3Ptr->registeredItemHold = ITEM_NONE;
}

static u32 FindKeyItemSlot(enum Item itemId)
{
    struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];

    for (u32 i = 0; i < pocket->capacity; i++)
    {
        if (BagPocket_GetSlotData(pocket, i).itemId == itemId)
            return i;
    }
    return pocket->capacity;
}

static void FillEmptyKeyItemSlots(void)
{
    struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];

    for (u32 i = 0; i < pocket->capacity; i++)
    {
        if (BagPocket_GetSlotData(pocket, i).itemId == ITEM_NONE)
            BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_BICYCLE, 1);
    }
}

TEST("Standard Rod: every giver visit order awards Old, Good, then Super")
{
    static const enum Item expectedAwards[] = {ITEM_OLD_ROD, ITEM_GOOD_ROD, ITEM_SUPER_ROD};

    for (u32 order = 0; order < ARRAY_COUNT(sStandardRodVisitOrders); order++)
    {
        enum Item awardedItem;

        ResetStandardRodTestState();
        for (u32 step = 0; step < ARRAY_COUNT(expectedAwards); step++)
        {
            u16 flag = sStandardRodContributorFlags[sStandardRodVisitOrders[order][step]];

            EXPECT_EQ(TryAwardStandardRod(flag, &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
            EXPECT_EQ(awardedItem, expectedAwards[step]);
            EXPECT(FlagGet(flag));
            EXPECT_EQ(CountTotalItemQuantityInBag(expectedAwards[step]), 1);
            if (step > 0)
                EXPECT_EQ(CountTotalItemQuantityInBag(expectedAwards[step - 1]), 0);
        }
    }
}

TEST("Standard Rod: a contributor cannot award twice")
{
    enum Item awardedItem;

    ResetStandardRodTestState();
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[0], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[0], &awardedItem), STANDARD_ROD_AWARD_ALREADY_CONTRIBUTED);
    EXPECT_EQ(awardedItem, ITEM_NONE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_OLD_ROD), 1);
    EXPECT(!FlagGet(sStandardRodContributorFlags[1]));
    EXPECT(!FlagGet(sStandardRodContributorFlags[2]));
}

TEST("Standard Rod: a full Key Items pocket rejects only the first award")
{
    enum Item awardedItem;

    ResetStandardRodTestState();
    FillEmptyKeyItemSlots();
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[0], &awardedItem), STANDARD_ROD_AWARD_NO_SPACE);
    EXPECT_EQ(awardedItem, ITEM_NONE);
    EXPECT(!FlagGet(sStandardRodContributorFlags[0]));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_OLD_ROD), 0);
}

TEST("Standard Rod: full-pocket upgrades reuse the exact encrypted Bag slot")
{
    struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];
    enum Item awardedItem;
    u32 rodSlot;

    ResetStandardRodTestState();
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[0], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    rodSlot = FindKeyItemSlot(ITEM_OLD_ROD);
    ASSUME(rodSlot < pocket->capacity);
    FillEmptyKeyItemSlots();

    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[1], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    EXPECT_EQ(awardedItem, ITEM_GOOD_ROD);
    EXPECT_EQ(FindKeyItemSlot(ITEM_GOOD_ROD), rodSlot);
    EXPECT_EQ(BagPocket_GetSlotData(pocket, rodSlot).quantity, 1);

    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[2], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    EXPECT_EQ(awardedItem, ITEM_SUPER_ROD);
    EXPECT_EQ(FindKeyItemSlot(ITEM_SUPER_ROD), rodSlot);
    EXPECT_EQ(BagPocket_GetSlotData(pocket, rodSlot).quantity, 1);
}

TEST("Standard Rod: upgrades transfer both registered-item shortcuts independently")
{
    enum Item awardedItem;

    ResetStandardRodTestState();
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[0], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    gSaveBlock1Ptr->registeredItem = ITEM_OLD_ROD;
    gSaveBlock3Ptr->registeredItemHold = ITEM_BICYCLE;
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[1], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    EXPECT_EQ(gSaveBlock1Ptr->registeredItem, ITEM_GOOD_ROD);
    EXPECT_EQ(gSaveBlock3Ptr->registeredItemHold, ITEM_BICYCLE);

    gSaveBlock1Ptr->registeredItem = ITEM_BICYCLE;
    gSaveBlock3Ptr->registeredItemHold = ITEM_GOOD_ROD;
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[2], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    EXPECT_EQ(gSaveBlock1Ptr->registeredItem, ITEM_BICYCLE);
    EXPECT_EQ(gSaveBlock3Ptr->registeredItemHold, ITEM_SUPER_ROD);
}

TEST("Standard Rod: upgrades transfer both registered-item shortcuts together")
{
    enum Item awardedItem;

    ResetStandardRodTestState();
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[0], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    gSaveBlock1Ptr->registeredItem = ITEM_OLD_ROD;
    gSaveBlock3Ptr->registeredItemHold = ITEM_OLD_ROD;

    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[1], &awardedItem), STANDARD_ROD_AWARD_SUCCESS);
    EXPECT_EQ(gSaveBlock1Ptr->registeredItem, ITEM_GOOD_ROD);
    EXPECT_EQ(gSaveBlock3Ptr->registeredItemHold, ITEM_GOOD_ROD);
}

TEST("Standard Rod: invalid state rolls back Bag shortcuts and contributor flag")
{
    struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];
    enum Item awardedItem;

    ResetStandardRodTestState();
    FlagSet(sStandardRodContributorFlags[0]);
    BagPocket_SetSlotItemIdAndCount(pocket, 7, ITEM_OLD_ROD, 2);
    gSaveBlock1Ptr->registeredItem = ITEM_OLD_ROD;
    gSaveBlock3Ptr->registeredItemHold = ITEM_OLD_ROD;

    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[1], &awardedItem), STANDARD_ROD_AWARD_INVALID_STATE);
    EXPECT_EQ(awardedItem, ITEM_NONE);
    EXPECT_EQ(BagPocket_GetSlotData(pocket, 7).itemId, ITEM_OLD_ROD);
    EXPECT_EQ(BagPocket_GetSlotData(pocket, 7).quantity, 2);
    EXPECT_EQ(gSaveBlock1Ptr->registeredItem, ITEM_OLD_ROD);
    EXPECT_EQ(gSaveBlock3Ptr->registeredItemHold, ITEM_OLD_ROD);
    EXPECT(!FlagGet(sStandardRodContributorFlags[1]));
}

TEST("Standard Rod: invalid contributor flags and mismatched progression do not mutate state")
{
    enum Item awardedItem;

    ResetStandardRodTestState();
    EXPECT_EQ(TryAwardStandardRod(FLAG_TEMP_1, &awardedItem), STANDARD_ROD_AWARD_INVALID_STATE);
    EXPECT_EQ(awardedItem, ITEM_NONE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_OLD_ROD), 0);

    FlagSet(sStandardRodContributorFlags[0]);
    EXPECT_EQ(TryAwardStandardRod(sStandardRodContributorFlags[1], &awardedItem), STANDARD_ROD_AWARD_INVALID_STATE);
    EXPECT(!FlagGet(sStandardRodContributorFlags[1]));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_GOOD_ROD), 0);
}

TEST("Standard Rod: script wrapper reports the outcome and dynamic awarded item")
{
    ResetStandardRodTestState();
    gSpecialVar_0x8004 = sStandardRodContributorFlags[0];
    Script_TryAwardStandardRod();

    EXPECT_EQ(gSpecialVar_Result, STANDARD_ROD_AWARD_SUCCESS);
    EXPECT_EQ(gSpecialVar_0x8005, ITEM_OLD_ROD);
    EXPECT(FlagGet(sStandardRodContributorFlags[0]));
}

#if IS_HNS
TEST("Magnet Train: Lost Item atomically becomes the Pass in a full Key Items pocket")
{
    struct BagPocket *pocket = &gBagPockets[POCKET_KEY_ITEMS];

    ClearBag();
    BagPocket_SetSlotItemIdAndCount(pocket, 0, ITEM_LOST_ITEM, 1);
    FillEmptyKeyItemSlots();

    EXPECT(TrySwapLostItemForPass());
    EXPECT(!CheckBagHasItem(ITEM_LOST_ITEM, 1));
    EXPECT(CheckBagHasItem(ITEM_PASS, 1));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_PASS), 1);
}

TEST("Magnet Train: Lost Item handoff fails without mutating the Bag when the item is missing")
{
    ClearBag();
    FillEmptyKeyItemSlots();

    EXPECT(!TrySwapLostItemForPass());
    EXPECT(!CheckBagHasItem(ITEM_LOST_ITEM, 1));
    EXPECT(!CheckBagHasItem(ITEM_PASS, 1));
}

TEST("Magnet Train: preexisting Pass satisfies the handoff without duplication")
{
    ClearBag();
    EXPECT(AddBagItem(ITEM_LOST_ITEM, 2));
    EXPECT(AddBagItem(ITEM_PASS, 1));
    FillEmptyKeyItemSlots();

    RUN_OVERWORLD_SCRIPT(
        specialvar VAR_RESULT, TrySwapLostItemForPass;
    );

    EXPECT(gSpecialVar_Result);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_LOST_ITEM), 0);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_PASS), 1);
}
#endif

TEST("TMs and HMs are sorted correctly in the bag")
{
    struct BagPocket *pocket = &gBagPockets[POCKET_TM_HM];

    ASSUME(GetItemPocket(ITEM_HM07) == POCKET_TM_HM);
    ASSUME(GetItemPocket(ITEM_TM25) == POCKET_TM_HM);
    ASSUME(GetItemPocket(ITEM_TM14) == POCKET_TM_HM);
    ASSUME(GetItemPocket(ITEM_TM42) == POCKET_TM_HM);
    ASSUME(GetItemPocket(ITEM_HM05) == POCKET_TM_HM);
    ASSUME(GetItemPocket(ITEM_TM05) == POCKET_TM_HM);
    ASSUME(GetItemPocket(ITEM_TM01) == POCKET_TM_HM);
    ASSUME(GetItemPocket(ITEM_HM02) == POCKET_TM_HM);

    /*
     * Note: I would add a test to make sure that TMs are sorted correctly by move name,
     * but downstream users are likely to rearrange TMs so this would just be a nuisance.
     */

    RUN_OVERWORLD_SCRIPT(
        additem ITEM_HM07;
        additem ITEM_TM25;
        additem ITEM_TM14;
        additem ITEM_TM42;
        additem ITEM_HM05;
        additem ITEM_TM05;
        additem ITEM_TM01;
        additem ITEM_HM02;
    );

    SortItemsInBag(&gBagPockets[POCKET_TM_HM], SORT_BY_INDEX);

    EXPECT_EQ(pocket->itemSlots[0].itemId, ITEM_TM01);
    EXPECT_EQ(pocket->itemSlots[1].itemId, ITEM_TM05);
    EXPECT_EQ(pocket->itemSlots[2].itemId, ITEM_TM14);
    EXPECT_EQ(pocket->itemSlots[3].itemId, ITEM_TM25);
    EXPECT_EQ(pocket->itemSlots[4].itemId, ITEM_TM42);
    EXPECT_EQ(pocket->itemSlots[5].itemId, ITEM_HM02);
    EXPECT_EQ(pocket->itemSlots[6].itemId, ITEM_HM05);
    EXPECT_EQ(pocket->itemSlots[7].itemId, ITEM_HM07);
    EXPECT_EQ(pocket->itemSlots[8].itemId, ITEM_NONE);
}

TEST("Berries are sorted correctly in the bag")
{
    struct BagPocket *pocket = &gBagPockets[POCKET_BERRIES];

    ASSUME(GetItemPocket(ITEM_POMEG_BERRY) == POCKET_BERRIES);
    ASSUME(GetItemPocket(ITEM_MAGOST_BERRY) == POCKET_BERRIES);
    ASSUME(GetItemPocket(ITEM_KELPSY_BERRY) == POCKET_BERRIES);
    ASSUME(GetItemPocket(ITEM_MICLE_BERRY) == POCKET_BERRIES);
    ASSUME(GetItemPocket(ITEM_CHARTI_BERRY) == POCKET_BERRIES);
    ASSUME(GetItemPocket(ITEM_GANLON_BERRY) == POCKET_BERRIES);
    ASSUME(GetItemPocket(ITEM_ORAN_BERRY) == POCKET_BERRIES);
    ASSUME(GetItemPocket(ITEM_CHERI_BERRY) == POCKET_BERRIES);

    RUN_OVERWORLD_SCRIPT(
        additem ITEM_POMEG_BERRY;
        additem ITEM_MAGOST_BERRY;
        additem ITEM_KELPSY_BERRY;
        additem ITEM_MICLE_BERRY;
        additem ITEM_CHARTI_BERRY;
        additem ITEM_GANLON_BERRY;
        additem ITEM_ORAN_BERRY;
        additem ITEM_CHERI_BERRY;
    );

    SortItemsInBag(&gBagPockets[POCKET_BERRIES], SORT_BY_INDEX);

    EXPECT_EQ(pocket->itemSlots[0].itemId, ITEM_CHERI_BERRY);
    EXPECT_EQ(pocket->itemSlots[1].itemId, ITEM_ORAN_BERRY);
    EXPECT_EQ(pocket->itemSlots[2].itemId, ITEM_POMEG_BERRY);
    EXPECT_EQ(pocket->itemSlots[3].itemId, ITEM_KELPSY_BERRY);
    EXPECT_EQ(pocket->itemSlots[4].itemId, ITEM_MAGOST_BERRY);
    EXPECT_EQ(pocket->itemSlots[5].itemId, ITEM_CHARTI_BERRY);
    EXPECT_EQ(pocket->itemSlots[6].itemId, ITEM_GANLON_BERRY);
    EXPECT_EQ(pocket->itemSlots[7].itemId, ITEM_MICLE_BERRY);
    EXPECT_EQ(pocket->itemSlots[8].itemId, ITEM_NONE);

    SortItemsInBag(&gBagPockets[POCKET_BERRIES], SORT_ALPHABETICALLY);

    EXPECT_EQ(pocket->itemSlots[0].itemId, ITEM_CHARTI_BERRY);
    EXPECT_EQ(pocket->itemSlots[1].itemId, ITEM_CHERI_BERRY);
    EXPECT_EQ(pocket->itemSlots[2].itemId, ITEM_GANLON_BERRY);
    EXPECT_EQ(pocket->itemSlots[3].itemId, ITEM_KELPSY_BERRY);
    EXPECT_EQ(pocket->itemSlots[4].itemId, ITEM_MAGOST_BERRY);
    EXPECT_EQ(pocket->itemSlots[5].itemId, ITEM_MICLE_BERRY);
    EXPECT_EQ(pocket->itemSlots[6].itemId, ITEM_ORAN_BERRY);
    EXPECT_EQ(pocket->itemSlots[7].itemId, ITEM_POMEG_BERRY);
    EXPECT_EQ(pocket->itemSlots[8].itemId, ITEM_NONE);
}

TEST("Items are correctly sorted and compacted in the bag")
{
    struct BagPocket *pocket = &gBagPockets[POCKET_ITEMS];
    memset(pocket->itemSlots, 0, sizeof(gSaveBlock1Ptr->bag.items));

    ASSUME(GetItemPocket(ITEM_NUGGET) == POCKET_ITEMS);
    ASSUME(GetItemPocket(ITEM_BIG_NUGGET) == POCKET_ITEMS);
    ASSUME(GetItemPocket(ITEM_TINY_MUSHROOM) == POCKET_ITEMS);
    ASSUME(GetItemPocket(ITEM_BIG_MUSHROOM) == POCKET_ITEMS);
    ASSUME(GetItemPocket(ITEM_PEARL) == POCKET_ITEMS);
    ASSUME(GetItemPocket(ITEM_BIG_PEARL) == POCKET_ITEMS);

    RUN_OVERWORLD_SCRIPT(
        additem ITEM_NUGGET;
        additem ITEM_BIG_NUGGET;
        additem ITEM_TINY_MUSHROOM;
        additem ITEM_BIG_MUSHROOM;
        additem ITEM_PEARL;
        additem ITEM_BIG_PEARL;
    );

    EXPECT_EQ(pocket->itemSlots[0].itemId, ITEM_NUGGET);
    EXPECT_EQ(pocket->itemSlots[0].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[1].itemId, ITEM_BIG_NUGGET);
    EXPECT_EQ(pocket->itemSlots[1].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[2].itemId, ITEM_TINY_MUSHROOM);
    EXPECT_EQ(pocket->itemSlots[2].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[3].itemId, ITEM_BIG_MUSHROOM);
    EXPECT_EQ(pocket->itemSlots[3].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[4].itemId, ITEM_PEARL);
    EXPECT_EQ(pocket->itemSlots[4].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[5].itemId, ITEM_BIG_PEARL);
    EXPECT_EQ(pocket->itemSlots[5].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[6].itemId, ITEM_NONE);

    SortItemsInBag(&gBagPockets[POCKET_ITEMS], SORT_ALPHABETICALLY);

    EXPECT_EQ(pocket->itemSlots[0].itemId, ITEM_BIG_MUSHROOM);
    EXPECT_EQ(pocket->itemSlots[1].itemId, ITEM_BIG_NUGGET);
    EXPECT_EQ(pocket->itemSlots[2].itemId, ITEM_BIG_PEARL);
    EXPECT_EQ(pocket->itemSlots[3].itemId, ITEM_NUGGET);
    EXPECT_EQ(pocket->itemSlots[4].itemId, ITEM_PEARL);
    EXPECT_EQ(pocket->itemSlots[5].itemId, ITEM_TINY_MUSHROOM);
    EXPECT_EQ(pocket->itemSlots[6].itemId, ITEM_NONE);

    // Try removing the big items, check that everything is compacted correctly

    RUN_OVERWORLD_SCRIPT(
        removeitem ITEM_BIG_NUGGET;
        removeitem ITEM_BIG_MUSHROOM;
        removeitem ITEM_BIG_PEARL;
    );

    CompactItemsInBagPocket(POCKET_ITEMS);

    EXPECT_EQ(pocket->itemSlots[0].itemId, ITEM_NUGGET);
    EXPECT_EQ(pocket->itemSlots[0].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[1].itemId, ITEM_PEARL);
    EXPECT_EQ(pocket->itemSlots[1].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[2].itemId, ITEM_TINY_MUSHROOM);
    EXPECT_EQ(pocket->itemSlots[2].quantity, 1);
    EXPECT_EQ(pocket->itemSlots[3].itemId, ITEM_NONE);
    EXPECT_EQ(pocket->itemSlots[4].itemId, ITEM_NONE);
    EXPECT_EQ(pocket->itemSlots[5].itemId, ITEM_NONE);
    EXPECT_EQ(pocket->itemSlots[6].itemId, ITEM_NONE);
}
