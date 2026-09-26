#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "field_player_avatar.h"
#include "heal_location.h"
#include "item.h"
#include "new_game.h"
#include "pokemon.h"
#include "pokedex.h"
#include "randomizer.h"
#include "starter_choose.h"
#include "wayfarer_appearance.h"
#include "wayfarer_kanto_opening.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/opponents.h"
#include "constants/pokedex.h"
#include "constants/species.h"

#if IS_WAYFARER

static void StartPalletAs(u8 appearance)
{
    EXPECT(WayfarerConfirmPendingOrigin(ORIGIN_PALLET));
    EXPECT(WayfarerConfirmPendingAppearance(appearance));
    NewGameInitData();
    gSaveBlock3Ptr->challengeSettings.tx_Challenges_OneTypeChallenge = 31;
    gSaveBlock3Ptr->challengeSettings.tx_Random_Starter = FALSE;
}

static void StartPallet(void)
{
    StartPalletAs(APPEARANCE_GOLD);
}

static void StageStarter(u16 slot)
{
    EXPECT(WayfarerKanto_TryStageInterception());
    EXPECT(WayfarerKanto_CommitLabStarterChoice());
    gSpecialVar_0x8004 = slot;
    EXPECT(WayfarerKanto_CommitStarterSlot());
}

TEST("Pallet origin has stable profile bedroom recovery catalog and isolated fresh state")
{
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(ORIGIN_PALLET);
    struct WarpData recovery;

    EXPECT_EQ(ORIGIN_PALLET, 3);
    EXPECT(profile != NULL);
    EXPECT_EQ(profile->entryRegion, REGION_KANTO);
    EXPECT_EQ(profile->mapGroup, MAP_GROUP(MAP_PALLET_TOWN_REDS_HOUSE_2F_HNS));
    EXPECT_EQ(profile->mapNum, MAP_NUM(MAP_PALLET_TOWN_REDS_HOUSE_2F_HNS));
    EXPECT(WayfarerGetOriginProfile(4) == NULL);
    StartPallet();
    EXPECT(WayfarerKanto_IsPalletOrigin());
    EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_HOME);
    EXPECT_EQ(WayfarerKanto_GetStarterSlot(), KANTO_STARTER_SLOT_NONE);
    EXPECT_EQ(Dex_GetActiveRegion(), DEX_REGION_KANTO);
    EXPECT_EQ(WayfarerGetInitialRecoveryDestination(), HEAL_LOCATION_PALLET_HOME_WAYFARER);
    EXPECT_EQ(GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation), HEAL_LOCATION_PALLET_HOME_WAYFARER);
    EXPECT(IsLastHealLocationPlayerHouse());
    SetWhiteoutRespawnWarpAndHealerNPC(&recovery);
    EXPECT_EQ(recovery.mapGroup, MAP_GROUP(MAP_PALLET_TOWN_REDS_HOUSE_1F_HNS));
    EXPECT_EQ(recovery.mapNum, MAP_NUM(MAP_PALLET_TOWN_REDS_HOUSE_1F_HNS));
    EXPECT_EQ(recovery.x, 7);
    EXPECT_EQ(recovery.y, 6);
    EXPECT_EQ(gSaveBlock1Ptr->location.mapGroup, MAP_GROUP(MAP_PALLET_TOWN_REDS_HOUSE_2F_HNS));
    EXPECT_EQ(gSaveBlock1Ptr->location.mapNum, MAP_NUM(MAP_PALLET_TOWN_REDS_HOUSE_2F_HNS));
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, 5);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, 5);
    EXPECT(WayfarerPersistentStateIsValid());
    EXPECT(!FlagGet(FLAG_JOHTO_STARTER_RECEIVED));
    EXPECT(!FlagGet(FLAG_HOENN_STARTER_RECEIVED));
    EXPECT(!WayfarerCanUseRegularAqua());
    EXPECT(!WayfarerCanReceiveSlateportTicket());
}

TEST("Pallet starter requires escort staging and preserves each local counter branch")
{
    static const u16 starters[] = {SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE};
    static const u8 counters[] = {KANTO_STARTER_SLOT_CHARMANDER, KANTO_STARTER_SLOT_SQUIRTLE, KANTO_STARTER_SLOT_BULBASAUR};
    u16 slot;

    for (slot = 0; slot < ARRAY_COUNT(starters); slot++)
    {
        u32 personality;

        StartPallet();
        gSpecialVar_0x8004 = slot;
        EXPECT(!WayfarerKanto_CommitLabStarterChoice());
        EXPECT(!WayfarerKanto_CommitStarterSlot());
        EXPECT(!WayfarerKanto_TryGiveStarter());
        EXPECT_EQ(gPlayerPartyCount, 0);
        EXPECT(WayfarerKanto_TryStageInterception());
        EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_OAK_INTERCEPTION_AVAILABLE);
        EXPECT(!WayfarerKanto_CommitStarterSlot());
        EXPECT(WayfarerKanto_CommitLabStarterChoice());
        EXPECT(WayfarerKanto_CommitStarterSlot());
        EXPECT_EQ(WayfarerKanto_GetCounterSlot(), counters[slot]);
        EXPECT_EQ(GetKantoStarterPokemon(slot), starters[slot]);
        EXPECT(WayfarerPersistentStateIsValid());
        gSpecialVar_0x8004 = (slot + 1) % 3;
        EXPECT(!WayfarerKanto_CommitStarterSlot());
        EXPECT(WayfarerKanto_TryGiveStarter());
        EXPECT_EQ(gPlayerPartyCount, 1);
        EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), starters[slot]);
        EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_LEVEL), 5);
        personality = GetMonData(&gPlayerParty[0], MON_DATA_PERSONALITY);
        EXPECT(WayfarerKanto_TryGiveStarter());
        EXPECT_EQ(gPlayerPartyCount, 1);
        EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_PERSONALITY), personality);
        EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_STARTER_RECEIVED);
        EXPECT(FlagGet(FLAG_SYS_POKEMON_GET));
        EXPECT(!FlagGet(FLAG_JOHTO_STARTER_RECEIVED));
        EXPECT(!FlagGet(FLAG_HOENN_STARTER_RECEIVED));
        EXPECT(WayfarerPersistentStateIsValid());
    }
}

TEST("Pallet opening preserves every selected appearance and existing gender")
{
    static const u8 appearances[] = {APPEARANCE_GOLD, APPEARANCE_KRIS, APPEARANCE_BRENDAN, APPEARANCE_MAY};
    u32 i;

    for (i = 0; i < ARRAY_COUNT(appearances); i++)
    {
        u8 appearance = appearances[i];
        u16 graphics = WayfarerGetAppearanceGraphicsId(appearance, PLAYER_AVATAR_STATE_NORMAL);

        StartPalletAs(appearance);
        EXPECT_EQ(WayfarerGetPlayerAppearanceId(), appearance);
        EXPECT_EQ(gSaveBlock2Ptr->playerGender, WayfarerGetAppearanceProfile(appearance)->gender);
        StageStarter(KANTO_STARTER_SLOT_BULBASAUR);
        EXPECT(WayfarerKanto_TryGiveStarter());
        EXPECT(WayfarerKanto_ResolveFirstBattle());
        EXPECT(WayfarerKanto_TryReceiveParcel());
        EXPECT(WayfarerKanto_TryAcceptParcel());
        EXPECT(WayfarerKanto_CommitBlueArrival());
        EXPECT(WayfarerKanto_TryGrantPokedex());
        EXPECT(WayfarerKanto_TryGrantPokeBalls());
        EXPECT(WayfarerKanto_CommitPresentationDone());
        EXPECT(WayfarerKanto_CompleteOpening());
        EXPECT_EQ(WayfarerGetPlayerAppearanceId(), appearance);
        EXPECT_EQ(gSaveBlock2Ptr->playerGender, WayfarerGetAppearanceProfile(appearance)->gender);
        EXPECT_EQ(WayfarerGetAppearanceGraphicsId(WayfarerGetPlayerAppearanceId(), PLAYER_AVATAR_STATE_NORMAL), graphics);
        EXPECT(WayfarerPersistentStateIsValid());
    }
}

TEST("Pallet challenge substitution preserves the committed local slot")
{
    u16 slot;

    for (slot = 0; slot < KANTO_STARTER_SLOT_NONE; slot++)
    {
        u16 expected;

        StartPallet();
        gSaveBlock3Ptr->challengeSettings.tx_Challenges_OneTypeChallenge = TYPE_FIRE;
        StageStarter(slot);
        expected = GetKantoStarterPokemon(slot);
        EXPECT(WayfarerKanto_TryGiveStarter());
        EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), expected);
        EXPECT_EQ(WayfarerKanto_GetStarterSlot(), slot);
        EXPECT_EQ(WayfarerKanto_GetCounterSlot(), (slot + 1) % KANTO_STARTER_SLOT_NONE);
        EXPECT(WayfarerPersistentStateIsValid());
    }
}

#if RANDOMIZER_AVAILABLE
TEST("Pallet randomized starter uses Kanto seed and retains the local rival branch")
{
    static const u16 starters[] = {SPECIES_BULBASAUR, SPECIES_CHARMANDER, SPECIES_SQUIRTLE};
    u16 slot;

    for (slot = 0; slot < KANTO_STARTER_SLOT_NONE; slot++)
    {
        u16 expected;

        StartPallet();
        gSaveBlock3Ptr->challengeSettings.tx_Random_Starter = TRUE;
        StageStarter(slot);
        expected = RandomizeMon(RANDOMIZER_REASON_STARTER_AND_GIFT_MON,
                                GetRandomizerOption(RANDOMIZER_OPTION_SPECIES_MODE),
                                GetRandomizerSeed() ^ starters[slot], starters[slot]);
        EXPECT_EQ(GetKantoStarterPokemon(slot), expected);
        EXPECT(WayfarerKanto_TryGiveStarter());
        EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_SPECIES), expected);
        EXPECT_EQ(WayfarerKanto_GetStarterSlot(), slot);
        EXPECT_EQ(WayfarerKanto_GetCounterSlot(), (slot + 1) % KANTO_STARTER_SLOT_NONE);
    }
}
#endif

TEST("Pallet Parcel and lab rewards commit independently and exactly once")
{
    u16 johtoSlot, hoennSlot, hnsLabState;
    bool8 johtoReceived, hoennReceived;

    StartPallet();
    johtoSlot = VarGet(VAR_STARTER_MON);
    hoennSlot = VarGet(VAR_HOENN_STARTER_CHOICE);
    hnsLabState = VarGet(VAR_PALLETTOWN_LABSTATE);
    johtoReceived = FlagGet(FLAG_JOHTO_STARTER_RECEIVED);
    hoennReceived = FlagGet(FLAG_HOENN_STARTER_RECEIVED);
    EXPECT(!WayfarerKanto_TryReceiveParcel());
    EXPECT(!WayfarerKanto_ResolveFirstBattle());
    EXPECT(WayfarerKanto_TryStageInterception());
    EXPECT(WayfarerKanto_CommitLabStarterChoice());
    gSpecialVar_0x8004 = KANTO_STARTER_SLOT_BULBASAUR;
    EXPECT(WayfarerKanto_CommitStarterSlot());
    EXPECT(WayfarerKanto_TryGiveStarter());
    EXPECT(WayfarerKanto_ResolveFirstBattle());
    EXPECT(WayfarerKanto_ResolveFirstBattle());
    EXPECT(!WayfarerKanto_TryAcceptParcel());
    EXPECT(WayfarerKanto_TryReceiveParcel());
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_OAKS_PARCEL), 1);
    EXPECT(WayfarerKanto_TryReceiveParcel());
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_OAKS_PARCEL), 1);
    EXPECT(WayfarerKanto_TryAcceptParcel());
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_OAKS_PARCEL), 0);
    EXPECT(WayfarerKanto_TryAcceptParcel());
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_OAKS_PARCEL), 0);
    EXPECT(!WayfarerKanto_TryGrantPokedex());
    EXPECT(WayfarerKanto_CommitBlueArrival());
    EXPECT(WayfarerKanto_TryGrantPokedex());
    EXPECT(WayfarerHasSharedPokedex());
    EXPECT(!WayfarerKanto_CompleteOpening());
    EXPECT(WayfarerKanto_TryGrantPokeBalls());
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), 5);
    EXPECT(WayfarerKanto_TryGrantPokeBalls());
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), 5);
    EXPECT(!WayfarerKanto_CompleteOpening());
    EXPECT(WayfarerKanto_CommitPresentationDone());
    EXPECT(WayfarerKanto_CompleteOpening());
    EXPECT(WayfarerKanto_IsOpeningComplete());
    EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_COMPLETE);
    EXPECT(WayfarerPersistentStateIsValid());
    EXPECT_EQ(VarGet(VAR_STARTER_MON), johtoSlot);
    EXPECT_EQ(VarGet(VAR_HOENN_STARTER_CHOICE), hoennSlot);
    EXPECT_EQ(VarGet(VAR_PALLETTOWN_LABSTATE), hnsLabState);
    EXPECT_EQ(FlagGet(FLAG_JOHTO_STARTER_RECEIVED), johtoReceived);
    EXPECT_EQ(FlagGet(FLAG_HOENN_STARTER_RECEIVED), hoennReceived);
}

TEST("Pallet item receipts remain pending when the matching Bag pocket is full")
{
    struct BagPocket *pocket;
    u32 i;

    StartPallet();
    StageStarter(KANTO_STARTER_SLOT_CHARMANDER);
    EXPECT(WayfarerKanto_TryGiveStarter());
    EXPECT(WayfarerKanto_ResolveFirstBattle());
    pocket = &gBagPockets[POCKET_KEY_ITEMS];
    for (i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_BICYCLE, 1);
    EXPECT(!WayfarerKanto_TryReceiveParcel());
    EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_OAKS_PARCEL), 0);
    gSpecialVar_0x8004 = PALLET_RECEIPT_PARCEL_RECEIVED;
    EXPECT(!WayfarerKanto_HasReceipt());
    BagPocket_SetSlotItemIdAndCount(pocket, 0, ITEM_NONE, 0);
    EXPECT(WayfarerKanto_TryReceiveParcel());
    EXPECT(WayfarerKanto_TryAcceptParcel());
    EXPECT(WayfarerKanto_CommitBlueArrival());
    EXPECT(WayfarerKanto_TryGrantPokedex());
    pocket = &gBagPockets[POCKET_POKE_BALLS];
    for (i = 0; i < pocket->capacity; i++)
        BagPocket_SetSlotItemIdAndCount(pocket, i, ITEM_GREAT_BALL, 1);
    EXPECT(!WayfarerKanto_TryGrantPokeBalls());
    EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_POKEDEX_HANDOFF_COMPLETE);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), 0);
    gSpecialVar_0x8004 = PALLET_RECEIPT_POKE_BALLS;
    EXPECT(!WayfarerKanto_HasReceipt());
    EXPECT(!WayfarerKanto_CompleteOpening());
    BagPocket_SetSlotItemIdAndCount(pocket, 0, ITEM_NONE, 0);
    EXPECT(WayfarerKanto_TryGrantPokeBalls());
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_BALL), 5);
    EXPECT(WayfarerPersistentStateIsValid());
}

TEST("Kanto opening specials are inert for a New Bark origin")
{
    EXPECT(WayfarerConfirmPendingOrigin(ORIGIN_NEW_BARK));
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_GOLD));
    NewGameInitData();
    EXPECT(!WayfarerKanto_IsPalletOrigin());
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerPalletOpening.starterSlot, KANTO_STARTER_SLOT_NONE);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerPalletOpening.receipts, 0);
    EXPECT(!WayfarerKanto_TryStageInterception());
    EXPECT(!WayfarerKanto_CommitLabStarterChoice());
    gSpecialVar_0x8004 = KANTO_STARTER_SLOT_BULBASAUR;
    EXPECT(!WayfarerKanto_CommitStarterSlot());
    EXPECT(!WayfarerKanto_TryGiveStarter());
    EXPECT(!WayfarerKanto_TryReceiveParcel());
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerPalletOpening.starterSlot, KANTO_STARTER_SLOT_NONE);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerPalletOpening.receipts, 0);
    EXPECT(WayfarerPersistentStateIsValid());
}

TEST("Kanto Blue defeats use reserved Trainer flags apart from opening progress")
{
    u16 trainerId;

    for (trainerId = TRAINER_WAYFARER_KANTO_FIRST; trainerId <= TRAINER_WAYFARER_KANTO_LAST; trainerId++)
    {
        u8 flagsBefore[NUM_FLAG_BYTES];

        StartPallet();
        StageStarter(trainerId - TRAINER_WAYFARER_KANTO_FIRST);
        EXPECT(WayfarerKanto_TryGiveStarter());
        ClearTrainerFlag(trainerId);
        memcpy(flagsBefore, gSaveBlock1Ptr->flags, sizeof(flagsBefore));

        EXPECT(!HasTrainerBeenFought(trainerId));
        SetTrainerFlag(trainerId);
        EXPECT(HasTrainerBeenFought(trainerId));
        EXPECT(FlagGet(WAYFARER_KANTO_DEFEAT_FLAG_FIRST + trainerId - TRAINER_WAYFARER_KANTO_FIRST));
        // The battle's defeat flag never advances the opening; the lab script's
        // WayfarerKanto_ResolveFirstBattle does, on a win or an ordinary loss.
        EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_STARTER_RECEIVED);
        EXPECT(WayfarerKanto_ResolveFirstBattle());
        EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED);

        ClearTrainerFlag(trainerId);
        EXPECT(!HasTrainerBeenFought(trainerId));
        EXPECT_EQ(WayfarerKanto_GetPhase(), PALLET_OPENING_FIRST_BLUE_BATTLE_RESOLVED);
        EXPECT_EQ(memcmp(flagsBefore, gSaveBlock1Ptr->flags, sizeof(flagsBefore)), 0);
        EXPECT(WayfarerPersistentStateIsValid());
    }
}

static void LoadLabTemplates(void)
{
    static const u8 localIds[] = {
        LOCALID_PALLET_KANTO_LAB_OAK, LOCALID_PALLET_LAB_AIDE_1, LOCALID_PALLET_LAB_AIDE_2,
        LOCALID_PALLET_LAB_AIDE_3, LOCALID_PALLET_KANTO_BLUE, LOCALID_PALLET_KANTO_BULBASAUR_BALL,
        LOCALID_PALLET_KANTO_SQUIRTLE_BALL, LOCALID_PALLET_KANTO_CHARMANDER_BALL,
    };
    u32 i;

    memset(gSaveBlock1Ptr->objectEventTemplates, 0, sizeof(gSaveBlock1Ptr->objectEventTemplates));
    for (i = 0; i < ARRAY_COUNT(localIds); i++)
        gSaveBlock1Ptr->objectEventTemplates[i].localId = localIds[i];
}

static const struct ObjectEventTemplate *FindLabTemplate(u8 localId)
{
    u32 i;

    for (i = 0; i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
    {
        if (gSaveBlock1Ptr->objectEventTemplates[i].localId == localId)
            return &gSaveBlock1Ptr->objectEventTemplates[i];
    }
    return NULL;
}

TEST("Pallet lab staging rebuilds Oak, Blue, the FRLG ball table, and the Pokedex desk from saved progress")
{
    const struct ObjectEventTemplate *blue;

    StartPallet();
    LoadLabTemplates();
    EXPECT(!WayfarerKanto_StageLab());
    EXPECT(FlagGet(FLAG_TEMP_1));
    EXPECT(!FlagGet(FLAG_TEMP_2));
    EXPECT(!FlagGet(FLAG_TEMP_3) && !FlagGet(FLAG_TEMP_4) && !FlagGet(FLAG_TEMP_5));
    EXPECT(!FlagGet(FLAG_TEMP_6));
    EXPECT_EQ(WayfarerKanto_GetRemainingSlot(), KANTO_STARTER_SLOT_NONE);

    // Oak's escort arrives: the lab stages him above the door for FRLG's walk-in.
    EXPECT(WayfarerKanto_TryStageInterception());
    EXPECT(WayfarerKanto_StageLab());
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT_EQ(FindLabTemplate(LOCALID_PALLET_KANTO_LAB_OAK)->x, 13);
    EXPECT_EQ(FindLabTemplate(LOCALID_PALLET_KANTO_LAB_OAK)->y, 19);
    EXPECT(WayfarerKanto_CommitLabStarterChoice());
    EXPECT(!WayfarerKanto_StageLab());

    // Bulbasaur leaves Squirtle as Oak's last ball; Blue waits under Charmander.
    gSpecialVar_0x8004 = KANTO_STARTER_SLOT_BULBASAUR;
    EXPECT(WayfarerKanto_CommitStarterSlot());
    EXPECT(!WayfarerKanto_StageLab());
    EXPECT(!FlagGet(FLAG_TEMP_3) && !FlagGet(FLAG_TEMP_4) && !FlagGet(FLAG_TEMP_5));
    EXPECT(WayfarerKanto_TryGiveStarter());
    EXPECT_EQ(WayfarerKanto_GetRemainingSlot(), KANTO_STARTER_SLOT_SQUIRTLE);
    EXPECT(!WayfarerKanto_StageLab());
    EXPECT(FlagGet(FLAG_TEMP_3));
    EXPECT(!FlagGet(FLAG_TEMP_4));
    EXPECT(FlagGet(FLAG_TEMP_5));
    EXPECT(!FlagGet(FLAG_TEMP_2));
    blue = FindLabTemplate(LOCALID_PALLET_KANTO_BLUE);
    EXPECT_EQ(blue->x, 17);
    EXPECT_EQ(blue->y, 13);
    EXPECT_EQ(blue->movementType, MOVEMENT_TYPE_FACE_UP);

    // Blue leaves after the battle and returns only for the Parcel handoff.
    EXPECT(WayfarerKanto_ResolveFirstBattle());
    WayfarerKanto_StageLab();
    EXPECT(FlagGet(FLAG_TEMP_2));
    EXPECT(!FlagGet(FLAG_TEMP_4));
    EXPECT(WayfarerKanto_TryReceiveParcel());
    EXPECT(WayfarerKanto_TryAcceptParcel());
    EXPECT(WayfarerKanto_CommitBlueArrival());
    WayfarerKanto_StageLab();
    EXPECT(!FlagGet(FLAG_TEMP_2));
    EXPECT(!FlagGet(FLAG_TEMP_6));
    EXPECT(WayfarerKanto_TryGrantPokedex());
    WayfarerKanto_StageLab();
    EXPECT(FlagGet(FLAG_TEMP_6));
    EXPECT(WayfarerKanto_TryGrantPokeBalls());
    EXPECT(WayfarerKanto_CommitPresentationDone());
    WayfarerKanto_StageLab();
    EXPECT(FlagGet(FLAG_TEMP_2));
    EXPECT(WayfarerKanto_CompleteOpening());
    WayfarerKanto_StageLab();
    // As in FRLG, Oak's last Pokemon stays on the table after the opening.
    EXPECT(FlagGet(FLAG_TEMP_3) && !FlagGet(FLAG_TEMP_4) && FlagGet(FLAG_TEMP_5));
    EXPECT(FlagGet(FLAG_TEMP_6));
    EXPECT(!FlagGet(FLAG_TEMP_1));
}

TEST("Pallet lab remaining ball is the slot neither the player nor Blue took")
{
    static const u8 remaining[] = {
        [KANTO_STARTER_SLOT_BULBASAUR] = KANTO_STARTER_SLOT_SQUIRTLE,
        [KANTO_STARTER_SLOT_CHARMANDER] = KANTO_STARTER_SLOT_BULBASAUR,
        [KANTO_STARTER_SLOT_SQUIRTLE] = KANTO_STARTER_SLOT_CHARMANDER,
    };
    u16 slot;

    for (slot = 0; slot < KANTO_STARTER_SLOT_NONE; slot++)
    {
        StartPallet();
        StageStarter(slot);
        EXPECT_EQ(WayfarerKanto_GetRemainingSlot(), KANTO_STARTER_SLOT_NONE);
        EXPECT(WayfarerKanto_TryGiveStarter());
        EXPECT_EQ(WayfarerKanto_GetRemainingSlot(), remaining[slot]);
    }
}

TEST("Pallet lab staging is inert for a New Bark origin")
{
    EXPECT(WayfarerConfirmPendingOrigin(ORIGIN_NEW_BARK));
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_GOLD));
    NewGameInitData();
    LoadLabTemplates();
    FlagClear(FLAG_TEMP_1);
    FlagSet(FLAG_TEMP_2);
    FlagSet(FLAG_TEMP_3);
    EXPECT(!WayfarerKanto_StageLab());
    EXPECT(!FlagGet(FLAG_TEMP_1));
    EXPECT(FlagGet(FLAG_TEMP_2));
    EXPECT(FlagGet(FLAG_TEMP_3));
    EXPECT_EQ(FindLabTemplate(LOCALID_PALLET_KANTO_BLUE)->x, 0);
}

TEST("Pallet receipt validation rejects impossible state without rewriting it")
{
    StartPallet();
    gSaveBlock3Ptr->wayfarerPalletOpening.receipts = PALLET_RECEIPT_POKE_BALLS;
    EXPECT(!WayfarerPersistentStateIsValid());
    WayfarerValidatePersistentState();
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerPalletOpening.receipts, PALLET_RECEIPT_POKE_BALLS);
    gSaveBlock3Ptr->wayfarerPalletOpening.receipts = 0;
    gSaveBlock3Ptr->wayfarerPalletOpening.phase = PALLET_OPENING_PARCEL_ACCEPTED;
    EXPECT(!WayfarerPersistentStateIsValid());
    gSaveBlock3Ptr->wayfarerPalletOpening.phase = PALLET_OPENING_HOME;
    gSaveBlock3Ptr->wayfarerPalletOpening.starterSlot = KANTO_STARTER_SLOT_SQUIRTLE;
    EXPECT(!WayfarerPersistentStateIsValid());
}

#endif
