#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "main.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "wayfarer_loss_policy.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"
#include "constants/wayfarer_persistence.h"
#include "test/test.h"

#if IS_WAYFARER
static const u8 sLossRedirect[] = {0};

TEST("Wayfarer trainer field loss requires an explicit redirect and real defeat")
{
    WayfarerResetLossContext();
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gBattleOutcome = B_OUTCOME_LOST;
    EXPECT(!WayfarerShouldContinuePartyDefeat());
    WayfarerSetTrainerLossRedirect(sLossRedirect);
    EXPECT(WayfarerShouldContinuePartyDefeat());
    gBattleOutcome = B_OUTCOME_WON;
    EXPECT(!WayfarerShouldContinuePartyDefeat());
    gBattleOutcome = B_OUTCOME_DREW;
    EXPECT(!WayfarerShouldContinuePartyDefeat());
    gBattleOutcome = B_OUTCOME_LOST;
    gBattleTypeFlags |= BATTLE_TYPE_INGAME_PARTNER;
    EXPECT(!WayfarerShouldContinuePartyDefeat());
    WayfarerResetLossContext();
    EXPECT_EQ(WayfarerGetTrainerLossRedirect(), NULL);
}

TEST("Wayfarer loss policy retains hardcore exhaustion routing")
{
    EXPECT(WayfarerAllowsOrdinaryPartyExhaustion());
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_NuzlockeHardcore = TRUE;
    EXPECT(!WayfarerAllowsOrdinaryPartyExhaustion());
    EXPECT(!WayfarerAllowsUnprotectedStorage());
}

TEST("Wayfarer retaliation recovery cause survives battle context teardown")
{
    MainCallback testCallback = gMain.callback2;

    WayfarerClearRecoveryCause();
    EXPECT(!WayfarerRecoveryIsTrainerRetaliation());
    WayfarerMarkTrainerRetaliation();
    EXPECT(gMain.callback2 == testCallback);
    EXPECT(WayfarerRecoveryIsTrainerRetaliation());
    WayfarerBeginTrainerRetaliationRecovery();
    EXPECT(gMain.callback2 == CB2_WhiteOut);
    SetMainCallback2(testCallback);
    WayfarerRecordBattleLossMoney();
    EXPECT(WayfarerBattleLossMoneyWasHandled());
    WayfarerResetLossContext();
    EXPECT(!WayfarerBattleLossMoneyWasHandled());
    EXPECT(WayfarerRecoveryIsTrainerRetaliation());
    WayfarerClearRecoveryCause();
    EXPECT(!WayfarerRecoveryIsTrainerRetaliation());
}

TEST("Wayfarer trainer retaliation keeps the established regional recovery route")
{
    MainCallback testCallback = gMain.callback2;

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_SLATEPORT_CITY);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_SLATEPORT_CITY);
    SetLastHealLocationWarp(HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    FlagSet(HOENN_FLAG_ID(WAYFARER_HOENN_WHITEOUT_TO_LAVARIDGE_FLAG));
    WayfarerBeginTrainerRetaliationRecovery();
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapGroup, MAP_GROUP(MAP_LAVARIDGE_TOWN));
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapNum, MAP_NUM(MAP_LAVARIDGE_TOWN));
    EXPECT(gMain.callback2 == CB2_WhiteOut);

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_OLIVINE_CITY_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_OLIVINE_CITY_HNS);
    SetLastHealLocationWarp(HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    WayfarerBeginTrainerRetaliationRecovery();
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapGroup,
              MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F));
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapNum,
              MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F));
    SetMainCallback2(testCallback);
    FlagClear(HOENN_FLAG_ID(WAYFARER_HOENN_WHITEOUT_TO_LAVARIDGE_FLAG));
    WayfarerClearRecoveryCause();
}

TEST("Storage compaction recounts sparse and fully empty parties")
{
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[5], SPECIES_RATTATA, 5, 0, OTID_STRUCT_PLAYER_ID);
    gPlayerPartyCount = PARTY_SIZE;
    CompactPartySlots();
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), SPECIES_RATTATA);
    EXPECT_EQ(GetMonData(&gPlayerParty[5], MON_DATA_SPECIES), SPECIES_NONE);
    ZeroMonData(&gPlayerParty[0]);
    CompactPartySlots();
    EXPECT_EQ(gPlayerPartyCount, 0);
}
#endif
