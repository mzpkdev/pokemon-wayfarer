#include "global.h"
#include "event_data.h"
#include "item.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/items.h"
#include "constants/maps.h"

#if IS_WAYFARER

TEST("Wayfarer New Bark Aqua policy depends only on its real maiden voyage")
{
    u32 state;
    u32 receipt;
    u32 ticket;

    WayfarerInitPersistentState();
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    // Read the HNS voyage bank even while the attendant runs on a Hoenn map.
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_SLATEPORT_CITY_HARBOR);
    VarSet(HOENN_VAR_ID(0x408B), 99);
    for (state = 0; state <= 9; state++)
    {
        VarSet(VAR_SSAQUA_STATE, state);
        for (receipt = 0; receipt <= 1; receipt++)
        {
            if (receipt)
                FlagSet(FLAG_HOENN_STARTER_RECEIVED);
            else
                FlagClear(FLAG_HOENN_STARTER_RECEIVED);
            for (ticket = 0; ticket <= 1; ticket++)
            {
                if (ticket)
                    EXPECT(AddBagItem(ITEM_SS_TICKET, 1));
                EXPECT_EQ(WayfarerCanUseRegularAqua(), state >= 8);
                EXPECT(!WayfarerCanReceiveSlateportTicket());
                EXPECT(WayfarerCanUseAquaMaidenVoyage());
                EXPECT_EQ(VarGet(VAR_SSAQUA_STATE), state);
                EXPECT_EQ(VarGet(HOENN_VAR_ID(0x408B)), 99);
                EXPECT_EQ(FlagGet(FLAG_HOENN_STARTER_RECEIVED), receipt);
                EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SS_TICKET), ticket);
                if (ticket)
                    EXPECT(RemoveBagItem(ITEM_SS_TICKET, 1));
            }
        }
    }
}

TEST("Wayfarer Littleroot Aqua policy unlocks on partner receipt without maiden rewards")
{
    u32 state;
    u32 receipt;
    u32 ticket;

    WayfarerInitPersistentState();
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_LITTLEROOT;
    FlagSet(FLAG_HIDE_SSAQUA_1F_GRANDPA);
    FlagClear(FLAG_HIDE_SSAQUA_SAILOR);
    VarSet(VAR_STARTER_MON, 2);
    for (state = 0; state <= 9; state++)
    {
        VarSet(VAR_SSAQUA_STATE, state);
        for (receipt = 0; receipt <= 1; receipt++)
        {
            if (receipt)
                FlagSet(FLAG_HOENN_STARTER_RECEIVED);
            else
                FlagClear(FLAG_HOENN_STARTER_RECEIVED);
            for (ticket = 0; ticket <= 1; ticket++)
            {
                if (ticket)
                    EXPECT(AddBagItem(ITEM_SS_TICKET, 1));
                EXPECT_EQ(WayfarerCanUseRegularAqua(), receipt);
                EXPECT_EQ(WayfarerCanReceiveSlateportTicket(), receipt);
                EXPECT(!WayfarerCanUseAquaMaidenVoyage());
                EXPECT_EQ(VarGet(VAR_SSAQUA_STATE), state);
                EXPECT_EQ(VarGet(VAR_STARTER_MON), 2);
                EXPECT_EQ(FlagGet(FLAG_HOENN_STARTER_RECEIVED), receipt);
                EXPECT(FlagGet(FLAG_HIDE_SSAQUA_1F_GRANDPA));
                EXPECT(!FlagGet(FLAG_HIDE_SSAQUA_SAILOR));
                EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SS_TICKET), ticket);
                EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_METAL_COAT), 0);
                if (ticket)
                    EXPECT(RemoveBagItem(ITEM_SS_TICKET, 1));
            }
        }
    }
}

TEST("Wayfarer unknown origins cannot inherit stock Aqua travel policies")
{
    WayfarerInitPersistentState();
    VarSet(VAR_SSAQUA_STATE, 8);
    FlagSet(FLAG_HOENN_STARTER_RECEIVED);
    EXPECT(AddBagItem(ITEM_SS_TICKET, 1));
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = 0xFFFF;
    EXPECT(!WayfarerCanUseRegularAqua());
    EXPECT(!WayfarerCanReceiveSlateportTicket());
    EXPECT(!WayfarerCanUseAquaMaidenVoyage());
    EXPECT_EQ(VarGet(VAR_SSAQUA_STATE), 8);
    EXPECT(FlagGet(FLAG_HOENN_STARTER_RECEIVED));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SS_TICKET), 1);
}

#endif
