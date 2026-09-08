#include "global.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "heal_location.h"
#include "new_game.h"
#include "money.h"
#include "trainer_rating.h"
#include "script.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokedex.h"
#include "wayfarer_origin.h"
#include "wayfarer_appearance.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"
#include "constants/pokedex.h"
#include "constants/species.h"

#if IS_WAYFARER

#define TEST_ORIGIN 0x7FFE
#define TEST_ORIGIN_MILESTONE VAR_UNUSED_HNS_0x40FE
static u8 CustomRecovery(void) { return HEAL_LOCATION_OLIVINE_CITY_HNS; }
static bool8 CustomTravel(void) { return VarGet(TEST_ORIGIN_MILESTONE) == 2; }
static void CustomSetup(void)
{
    VarSet(TEST_ORIGIN_MILESTONE, 1);
    CreateMon(&gPlayerParty[0], SPECIES_PIDGEY, 5, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[0]);
    gPlayerPartyCount = 1;
}
extern const u8 Test_WayfarerOrigin_AuthoredScene[];
extern const u8 Test_WayfarerOrigin_InterceptScene[];
static const u8 *CustomScene(u8 scene) { return Test_WayfarerOrigin_AuthoredScene; }
static void CustomOpening(void) { VarSet(VAR_UNUSED_HNS_0x40FC, 1); }
static const struct WayfarerOriginProfile sCustomOrigin =
{
    .id = TEST_ORIGIN, .selectionLabel = NULL, .entryRegion = REGION_JOHTO,
    .mapGroup = MAP_GROUP(MAP_OLIVINE_CITY_HNS), .mapNum = MAP_NUM(MAP_OLIVINE_CITY_HNS),
    .warpId = WARP_ID_NONE, .x = 20, .y = 20,
    .scenePolicies = {ORIGIN_SCENE_AUTHORED, ORIGIN_SCENE_VISITOR, ORIGIN_SCENE_VISITOR, ORIGIN_SCENE_VISITOR},
    .initialRecovery = CustomRecovery, .initialize = CustomSetup,
    .openingCallback = CustomOpening, .authoredScene = CustomScene,
    .regularAqua = CustomTravel, .slateportTicket = CustomTravel,
};

TEST("Wayfarer pending origin requires explicit confirmation and fresh flow resets it")
{
    WayfarerResetPendingOrigin();
    EXPECT_EQ(WayfarerGetPendingOriginCandidate(), ORIGIN_NEW_BARK);
    EXPECT_EQ(WayfarerGetConfirmedPendingOrigin(), ORIGIN_NONE);
    EXPECT(WayfarerSetPendingOriginCandidate(ORIGIN_LITTLEROOT));
    EXPECT_EQ(WayfarerGetConfirmedPendingOrigin(), ORIGIN_NONE);
    EXPECT(WayfarerConfirmPendingOrigin(ORIGIN_LITTLEROOT));
    EXPECT_EQ(WayfarerGetConfirmedPendingOrigin(), ORIGIN_LITTLEROOT);
    EXPECT(!WayfarerConfirmPendingOrigin(0xFFFF));
    EXPECT_EQ(WayfarerGetConfirmedPendingOrigin(), ORIGIN_NONE);
    WayfarerResetPendingOrigin();
    EXPECT_EQ(WayfarerGetConfirmedPendingOrigin(), ORIGIN_NONE);
    EXPECT_EQ(WayfarerGetOriginProfile(ORIGIN_NEW_BARK)->id, 1);
    EXPECT_EQ(WayfarerGetOriginProfile(ORIGIN_LITTLEROOT)->id, 2);
}

TEST("Wayfarer new origins initialize independent regional state and recovery")
{
    u16 origin;
    u8 gender;
    for (origin = ORIGIN_NEW_BARK; origin <= ORIGIN_LITTLEROOT; origin++)
    {
        for (gender = MALE; gender <= FEMALE; gender++)
        {
            gSaveBlock2Ptr->playerGender = gender;
            gSaveBlock3Ptr->challengeSettings.tx_Mode_Mints = 1;
            EXPECT(WayfarerConfirmPendingOrigin(origin));
            EXPECT(WayfarerConfirmPendingAppearance(gender == FEMALE ? APPEARANCE_KRIS : APPEARANCE_GOLD));
            NewGameInitData();
            EXPECT_EQ(WayfarerGetStartingOriginId(), origin);
            EXPECT_EQ(Dex_GetActiveRegion(), origin == ORIGIN_LITTLEROOT ? DEX_REGION_HOENN : DEX_REGION_JOHTO);
            EXPECT(WayfarerPersistentStateIsValid());
            EXPECT_EQ((u32)gSaveBlock3Ptr->challengeSettings.tx_Mode_Mints, 1);
            EXPECT_EQ(gPlayerPartyCount, 0);
            EXPECT_EQ(GetMoney(&gSaveBlock1Ptr->money), 3000);
            EXPECT_EQ(GetTrainerRating(), 0);
            EXPECT_EQ(GetBadgeCountForRegion(REGION_KANTO), 0);
            EXPECT(!GetGameClearStateForRegion(REGION_JOHTO));
            EXPECT(!GetGameClearStateForRegion(REGION_HOENN));
            EXPECT_EQ(GetBadgeCountForRegion(REGION_JOHTO), 0);
            EXPECT_EQ(GetBadgeCountForRegion(REGION_HOENN), 0);
            EXPECT(!GetGameClearStateForRegion(REGION_KANTO));
            EXPECT_EQ(VarGet(VAR_HOENN_STARTER_CHOICE), HOENN_STARTER_CHOICE_NONE);
            EXPECT(!FlagGet(FLAG_HOENN_STARTER_RECEIVED));
            EXPECT(!FlagGet(FLAG_JOHTO_STARTER_CHOICE_COMMITTED));
            EXPECT(!FlagGet(FLAG_JOHTO_STARTER_RECEIVED));
            EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.hnsRegionContext, REGION_JOHTO);
            EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation), WayfarerGetInitialRecoveryDestination());
            EXPECT_EQ(GetRegionVisitedState(REGION_JOHTO), origin == ORIGIN_NEW_BARK);
            EXPECT_EQ(GetRegionVisitedState(REGION_HOENN), origin == ORIGIN_LITTLEROOT);
            EXPECT_EQ(WayfarerHoennStateIsInitialized(), origin == ORIGIN_LITTLEROOT);
            EXPECT(!WayfarerInitializeOrigin(origin));
            if (origin == ORIGIN_LITTLEROOT)
            {
                EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_INSIDE_OF_TRUCK));
                EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_INSIDE_OF_TRUCK));
                EXPECT_EQ(VarGet(VAR_NEWBARK_TOWN_STATE), 2);
                EXPECT_EQ(VarGet(VAR_NEWBARKTOWN_LABSTATE), 0);
                EXPECT_EQ(VarGet(VAR_SSAQUA_STATE), 0);
            }
        }
    }
}

TEST("Wayfarer test-only same-region origin owns setup scenes travel and replaceable recovery")
{
    EXPECT(Test_WayfarerRegisterOriginProfile(&sCustomOrigin));
    EXPECT(WayfarerConfirmPendingOrigin(TEST_ORIGIN));
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_GOLD));
    NewGameInitData();
    EXPECT_EQ(WayfarerGetStartingOriginId(), TEST_ORIGIN);
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_OLIVINE_CITY_HNS));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_OLIVINE_CITY_HNS));
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, 20);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, 20);
    EXPECT_EQ(VarGet(TEST_ORIGIN_MILESTONE), 1);
    EXPECT_EQ(VarGet(VAR_UNUSED_HNS_0x40FC), 0);
    WayfarerEnterOriginOpening();
    EXPECT_EQ(VarGet(VAR_UNUSED_HNS_0x40FC), 1);
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT(!WayfarerUsesNativeJohtoHousehold());
    EXPECT(!WayfarerUsesNativeJohtoOpening());
    EXPECT_EQ(WayfarerGetOriginScenePolicy(ORIGIN_SCENE_JOHTO_HOUSEHOLD), ORIGIN_SCENE_AUTHORED);
    EXPECT(!WayfarerCanUseRegularAqua());
    ScriptContext_SetupScript(Test_WayfarerOrigin_InterceptScene);
    ScriptContext_RunScript();
    EXPECT_EQ(VarGet(TEST_ORIGIN_MILESTONE), 2);
    EXPECT_EQ(VarGet(VAR_UNUSED_HNS_0x40FD), 0);
    VarSet(TEST_ORIGIN_MILESTONE, 1);
    RunScriptImmediately(Test_WayfarerOrigin_InterceptScene);
    EXPECT_EQ(VarGet(VAR_UNUSED_HNS_0x40FD), 0);
    ScriptContext_RunScript();
    EXPECT_EQ(VarGet(TEST_ORIGIN_MILESTONE), 2);
    EXPECT(WayfarerCanUseRegularAqua());
    EXPECT_EQ(VarGet(VAR_SSAQUA_STATE), 0);
    EXPECT(!FlagGet(FLAG_JOHTO_STARTER_RECEIVED));
    EXPECT(!FlagGet(FLAG_HOENN_STARTER_RECEIVED));
    EXPECT(!WayfarerInitializeOrigin(TEST_ORIGIN));
    WayfarerValidatePersistentState();
    EXPECT_EQ(VarGet(TEST_ORIGIN_MILESTONE), 2);
    EXPECT_EQ(gPlayerPartyCount, 1);
    EXPECT(WayfarerReplaceRecoveryDestination(HEAL_LOCATION_SLATEPORT_CITY));
    memset(&gSaveBlock1Ptr->lastHealLocation, 0, sizeof(gSaveBlock1Ptr->lastHealLocation));
    EXPECT(WayfarerEnsureRecoveryDestination());
    EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation), HEAL_LOCATION_SLATEPORT_CITY);
    EXPECT(!WayfarerReplaceRecoveryDestination(0));
    EXPECT(Test_WayfarerRegisterOriginProfile(NULL));
    EXPECT(WayfarerGetOriginProfile(TEST_ORIGIN) == NULL);
}

TEST("Wayfarer authored Hoenn scenes intercept real rescue lab and household scripts")
{
    extern const u8 Route101_EventScript_BirchsBag[];
    extern const u8 LittlerootTown_ProfessorBirchsLab_EventScript_GiveStarterEvent[];
    extern const u8 PlayersHouse_1F_EventScript_Mom[];
    static const u8 *const entryScripts[] =
    {
        Route101_EventScript_BirchsBag,
        LittlerootTown_ProfessorBirchsLab_EventScript_GiveStarterEvent,
        PlayersHouse_1F_EventScript_Mom,
    };
    struct WayfarerOriginProfile custom = sCustomOrigin;
    struct WayfarerHoennPersistentState before;
    u32 i;

    custom.scenePolicies[ORIGIN_SCENE_HOENN_RESCUE] = ORIGIN_SCENE_AUTHORED;
    custom.scenePolicies[ORIGIN_SCENE_HOENN_HOUSEHOLD] = ORIGIN_SCENE_AUTHORED;
    EXPECT(Test_WayfarerRegisterOriginProfile(&custom));
    EXPECT(WayfarerConfirmPendingOrigin(TEST_ORIGIN));
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_GOLD));
    NewGameInitData();
    memcpy(&before, &gSaveBlock3Ptr->wayfarerHoenn, sizeof(before));
    for (i = 0; i < ARRAY_COUNT(entryScripts); i++)
    {
        VarSet(TEST_ORIGIN_MILESTONE, 1);
        ScriptContext_SetupScript(entryScripts[i]);
        ScriptContext_RunScript();
        EXPECT_EQ(VarGet(TEST_ORIGIN_MILESTONE), 2);
        EXPECT_EQ(memcmp(&before, &gSaveBlock3Ptr->wayfarerHoenn, sizeof(before)), 0);
        EXPECT_EQ(gPlayerPartyCount, 1);
        EXPECT(!FlagGet(FLAG_JOHTO_STARTER_RECEIVED));
        EXPECT(!FlagGet(FLAG_HOENN_STARTER_RECEIVED));
    }
    EXPECT(Test_WayfarerRegisterOriginProfile(NULL));
}

TEST("Wayfarer native Hoenn first Dex exposes starter records and later handoffs preserve catalogs")
{
    EXPECT(WayfarerConfirmPendingOrigin(ORIGIN_LITTLEROOT));
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_GOLD));
    NewGameInitData();
    GetSetPokedexFlag(NATIONAL_DEX_TORCHIC, FLAG_SET_SEEN);
    GetSetPokedexFlag(NATIONAL_DEX_TORCHIC, FLAG_SET_CAUGHT);
    GetSetPokedexFlag(NATIONAL_DEX_ZIGZAGOON, FLAG_SET_SEEN);
    WayfarerGrantSharedPokedex();
    EXPECT(WayfarerHasSharedPokedex());
    EXPECT_EQ(Dex_GetActiveRegion(), DEX_REGION_HOENN);
    EXPECT_EQ(Dex_GetRegionalVisibleProgress(FLAG_GET_SEEN), 2);
    EXPECT_EQ(Dex_GetRegionalVisibleProgress(FLAG_GET_CAUGHT), 1);
    EXPECT(Dex_UpgradeToNational());
    EXPECT(Dex_GrantNationalExtension(DEX_REGION_HOENN));
    EXPECT(Dex_SetActiveRegion(DEX_REGION_KANTO));
    WayfarerGrantSharedPokedex();
    WayfarerGrantSharedPokedex();
    EXPECT_EQ(Dex_GetActiveRegion(), DEX_REGION_KANTO);
    EXPECT(Dex_HasNationalUpgrade());
    EXPECT(Dex_HasNationalExtension(DEX_REGION_HOENN));
    EXPECT(GetSetPokedexFlag(NATIONAL_DEX_TORCHIC, FLAG_GET_CAUGHT));
}

TEST("Wayfarer profile registration rejects unsafe entries and missing authored handlers")
{
    struct WayfarerOriginProfile invalid = sCustomOrigin;
    invalid.mapGroup = -1;
    EXPECT(!Test_WayfarerRegisterOriginProfile(&invalid));
    invalid = sCustomOrigin;
    invalid.warpId = 127;
    EXPECT(!Test_WayfarerRegisterOriginProfile(&invalid));
    invalid = sCustomOrigin;
    invalid.x = 128;
    EXPECT(!Test_WayfarerRegisterOriginProfile(&invalid));
    invalid = sCustomOrigin;
    invalid.x = 22;
    invalid.y = 13;
    EXPECT(!Test_WayfarerRegisterOriginProfile(&invalid));
    invalid = sCustomOrigin;
    invalid.authoredScene = NULL;
    EXPECT(!Test_WayfarerRegisterOriginProfile(&invalid));
    invalid = sCustomOrigin;
    invalid.id = ORIGIN_NEW_BARK;
    EXPECT(!Test_WayfarerRegisterOriginProfile(&invalid));
    EXPECT(Test_WayfarerRegisterOriginProfile(NULL));
}

TEST("Wayfarer usable-party battle permission survives missing story milestones")
{
    ZeroPlayerPartyMons();
    EXPECT(!WayfarerCanStartOrdinaryBattle());
    CreateMon(&gPlayerParty[0], SPECIES_PIDGEY, 5, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[0]);
    EXPECT(WayfarerCanStartOrdinaryBattle());
    SetMonData(&gPlayerParty[0], MON_DATA_HP, &(u16){0});
    EXPECT(!WayfarerCanStartOrdinaryBattle());
}
#endif
