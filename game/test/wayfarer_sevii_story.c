#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "item.h"
#include "pokemon.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "wayfarer_sevii_story.h"
#include "test/test.h"
#include "constants/battle.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/regions.h"
#include "constants/species.h"
#include "constants/vars.h"

#if IS_WAYFARER

TEST("Wayfarer Sevii item receipts commit only after their Bag transaction")
{
    WayfarerSeviiInitPersistentState();
    ClearBag();

    EXPECT(!WayfarerSeviiTryGiveItemThenSetFlag(ITEM_NONE, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));
    EXPECT(!FlagGet(FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));

    EXPECT(WayfarerSeviiTryGiveItemThenSetFlag(ITEM_IAPAPA_BERRY, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));
    EXPECT(CheckBagHasItem(ITEM_IAPAPA_BERRY, 1));
    EXPECT(WayfarerSeviiTryGiveItemThenSetFlag(ITEM_IAPAPA_BERRY, FLAG_WAYFARER_SEVII_IAPAPA_BERRY_RECEIVED));
    EXPECT(RemoveBagItem(ITEM_IAPAPA_BERRY, 1));
    EXPECT(!CheckBagHasItem(ITEM_IAPAPA_BERRY, 1));

    EXPECT(!WayfarerSeviiTryRemoveItemThenSetFlag(ITEM_METEORITE, FLAG_WAYFARER_SEVII_METEORITE_DELIVERED));
    EXPECT(!FlagGet(FLAG_WAYFARER_SEVII_METEORITE_DELIVERED));
    EXPECT(AddBagItem(ITEM_METEORITE, 1));
    EXPECT(WayfarerSeviiTryExchangeItemForRewardThenSetFlags(ITEM_METEORITE, ITEM_MOON_STONE,
                                                               FLAG_WAYFARER_SEVII_METEORITE_DELIVERED,
                                                               FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED));
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_METEORITE_DELIVERED));
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_MOON_STONE_RECEIVED));
    EXPECT(!CheckBagHasItem(ITEM_METEORITE, 1));
    EXPECT(CheckBagHasItem(ITEM_MOON_STONE, 1));
}

TEST("Wayfarer Sevii party helpers and Selphy keep a pending request until reward delivery")
{
    u8 friendship = 201;

    WayfarerSeviiInitPersistentState();
    ClearBag();
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_MEOWTH, 20, 0, OTID_STRUCT_PLAYER_ID);
    SetMonData(&gPlayerParty[0], MON_DATA_FRIENDSHIP, &friendship);
    gPlayerPartyCount = 1;

    EXPECT(WayfarerSeviiHasPartyCapacity());
    EXPECT(WayfarerSeviiPartyHasSpecies(SPECIES_MEOWTH));
    EXPECT(WayfarerSeviiPartyMonHasSpecies(0, SPECIES_MEOWTH));
    EXPECT(WayfarerSeviiPartyHasFriendshipAtLeast(201));
    EXPECT(!WayfarerSeviiPartyHasFriendshipAtLeast(202));

    EXPECT(WayfarerSeviiStartSelphyRequest(SPECIES_MEOWTH, ITEM_NUGGET));
    EXPECT(WayfarerSeviiHasActiveSelphyRequest());
    EXPECT(!WayfarerSeviiIsSelphyRequestedSpecies(SPECIES_PIDGEY));
    EXPECT(WayfarerSeviiIsPartyMonSelphyRequested(0));
    EXPECT(WayfarerSeviiTryClaimSelphyPendingReward());
    EXPECT(CheckBagHasItem(ITEM_NUGGET, 1));
    EXPECT(!WayfarerSeviiHasActiveSelphyRequest());
    EXPECT_EQ(VarGet(VAR_WAYFARER_SEVII_SELPHY_PENDING_REWARD), ITEM_NONE);
}

TEST("Wayfarer Sevii Egg receipt remains a party-only capacity transaction")
{
    WayfarerSeviiInitPersistentState();
    ZeroPlayerPartyMons();
    gPlayerPartyCount = 0;

    EXPECT(WayfarerSeviiTryGiveEggThenSetFlag(SPECIES_TOGEPI, FLAG_WAYFARER_SEVII_EGG_RECEIVED));
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_EGG_RECEIVED));
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_TOGEPI);
    EXPECT(GetMonData(&gPlayerParty[0], MON_DATA_IS_EGG));
}

TEST("Wayfarer Sevii Moltres uses TR 55 and only resolves after a win or catch")
{
    WayfarerSeviiInitPersistentState();
    SetTrainerRating(WAYFARER_BIRD_CAPTURE_TR - 1);
    EXPECT(!WayfarerSeviiIsMoltresEligible());

    SetTrainerRating(WAYFARER_BIRD_CAPTURE_TR);
    EXPECT(WayfarerSeviiIsMoltresEligible());
    gBattleOutcome = B_OUTCOME_RAN;
    EXPECT(!WayfarerSeviiResolveMoltresBattle());
    EXPECT(!FlagGet(FLAG_WAYFARER_SEVII_MOLTRES_RESOLVED));

    gBattleOutcome = B_OUTCOME_CAUGHT;
    EXPECT(WayfarerSeviiResolveMoltresBattle());
    EXPECT(FlagGet(FLAG_WAYFARER_SEVII_MOLTRES_RESOLVED));
    EXPECT(!WayfarerSeviiIsMoltresEligible());
}

TEST("Wayfarer Sevii rival eligibility accepts any completed League")
{
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    SetGameClearStateForRegion(REGION_HOENN, FALSE);
    EXPECT(!WayfarerSeviiHasAnyLeagueClear());
    EXPECT(!WayfarerSeviiIsMainlandGiovanniComplete());

    SetGameClearStateForRegion(REGION_HOENN, TRUE);
    EXPECT(WayfarerSeviiHasAnyLeagueClear());
    SetGameClearStateForRegion(REGION_HOENN, FALSE);
}

#endif // IS_WAYFARER
