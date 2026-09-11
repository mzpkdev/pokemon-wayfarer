#include "global.h"
#include "main.h"
#include "title_screen.h"
#include "new_game.h"
#include "overworld.h"
#include "field_player_avatar.h"
#include "constants/maps.h"
#include "wayfarer_appearance.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/trainers.h"

#if IS_WAYFARER
TEST("Wayfarer appearances register complete state graphics and stable story genders")
{
    u32 index, state;
    for (index = 0; index < APPEARANCE_COUNT; index++)
    {
        u8 id = WayfarerGetAppearanceIdByIndex(index);
        const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(id);
        EXPECT(profile != NULL);
        EXPECT_EQ(profile->id, id);
        EXPECT_EQ(profile->gender, (id - 1) % 2);
        EXPECT(profile->frontPic < TRAINER_PIC_FRONT_COUNT);
        EXPECT(profile->backPic >= TRAINER_PIC_FRONT_COUNT);
        EXPECT(profile->backPic < TRAINER_PIC_COUNT);
        for (state = PLAYER_AVATAR_STATE_NORMAL; state <= PLAYER_AVATAR_STATE_VSSEEKER; state++)
            EXPECT(WayfarerGetAppearanceGraphicsId(id, state) < NUM_OBJ_EVENT_GFX);
    }
    EXPECT(WayfarerGetAppearanceGraphicsId(APPEARANCE_GOLD, PLAYER_AVATAR_STATE_NORMAL) > 255);
    EXPECT(WayfarerGetAppearanceProfile(APPEARANCE_NONE) == NULL);
    EXPECT_EQ(WayfarerGetAppearanceIdByIndex(0), APPEARANCE_GOLD);
    EXPECT_EQ(WayfarerGetAppearanceIdByIndex(1), APPEARANCE_KRIS);
    EXPECT_EQ(WayfarerGetAppearanceIdByIndex(2), APPEARANCE_BRENDAN);
    EXPECT_EQ(WayfarerGetAppearanceIdByIndex(3), APPEARANCE_MAY);
    EXPECT_EQ(WayfarerGetAppearanceIdByIndex(APPEARANCE_COUNT), APPEARANCE_NONE);
    EXPECT(WayfarerGetAppearanceProfile(APPEARANCE_RED) == NULL);
    EXPECT(WayfarerGetAppearanceProfile(APPEARANCE_LEAF) == NULL);
    EXPECT(WayfarerGetAppearanceProfile(255) == NULL);
    EXPECT_EQ(WayfarerGetAppearanceGraphicsId(255, 0), APPEARANCE_INVALID_GRAPHICS);
    EXPECT_EQ(WayfarerGetAppearanceGraphicsId(APPEARANCE_GOLD, 255), APPEARANCE_INVALID_GRAPHICS);
}

TEST("Wayfarer pending appearance is explicit and independent of an old save")
{
    gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = APPEARANCE_MAY;
    WayfarerResetPendingAppearance();
    EXPECT_EQ(WayfarerGetPendingAppearanceCandidate(), APPEARANCE_GOLD);
    EXPECT_EQ(WayfarerGetConfirmedPendingAppearance(), APPEARANCE_NONE);
    EXPECT(WayfarerSetPendingAppearanceCandidate(APPEARANCE_MAY));
    EXPECT_EQ(WayfarerGetConfirmedPendingAppearance(), APPEARANCE_NONE);
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_MAY));
    EXPECT_EQ(gSaveBlock2Ptr->playerGender, FEMALE);
    EXPECT(WayfarerSetPendingAppearanceCandidate(APPEARANCE_BRENDAN));
    EXPECT_EQ(WayfarerGetConfirmedPendingAppearance(), APPEARANCE_MAY);
    EXPECT(!WayfarerSetPendingAppearanceCandidate(APPEARANCE_RED));
    EXPECT(!WayfarerSetPendingAppearanceCandidate(APPEARANCE_LEAF));
    EXPECT(!WayfarerConfirmPendingAppearance(APPEARANCE_RED));
    EXPECT(!WayfarerConfirmPendingAppearance(APPEARANCE_LEAF));
    EXPECT(!WayfarerConfirmPendingAppearance(255));
    EXPECT_EQ(WayfarerGetConfirmedPendingAppearance(), APPEARANCE_NONE);
    WayfarerResetPendingAppearance();
    EXPECT_EQ(WayfarerGetPendingAppearanceCandidate(), APPEARANCE_GOLD);
}

TEST("Wayfarer all eight appearance and origin handoffs preserve challenge settings")
{
    u32 index, origin;
    for (index = 0; index < APPEARANCE_COUNT; index++)
    {
        u8 id = WayfarerGetAppearanceIdByIndex(index);
        for (origin = ORIGIN_NEW_BARK; origin <= ORIGIN_LITTLEROOT; origin++)
        {
            WayfarerResetPendingAppearance();
            EXPECT(WayfarerConfirmPendingAppearance(id));
            EXPECT(WayfarerConfirmPendingOrigin(origin));
            gSaveBlock3Ptr->challengeSettings.tx_Mode_Mints = 1;
            NewGameInitData();
            EXPECT_EQ(WayfarerGetPlayerAppearanceId(), id);
            EXPECT_EQ(gSaveBlock2Ptr->playerGender, (id - 1) % 2);
            EXPECT_EQ(WayfarerGetStartingOriginId(), origin);
            EXPECT_EQ((u32)gSaveBlock3Ptr->challengeSettings.tx_Mode_Mints, 1);
            EXPECT(WayfarerPersistentStateIsValid());
            EXPECT_EQ(WayfarerGetConfirmedPendingAppearance(), id);
            gSaveBlock2Ptr->playerGender ^= 1;
            EXPECT(!WayfarerPersistentStateIsValid());
            EXPECT_EQ(WayfarerGetPlayerAppearanceId(), APPEARANCE_NONE);
            gSaveBlock2Ptr->playerGender ^= 1;
            gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = APPEARANCE_RED;
            EXPECT(!WayfarerPersistentStateIsValid());
            gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = APPEARANCE_LEAF;
            EXPECT(!WayfarerPersistentStateIsValid());
            gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = 255;
            EXPECT(!WayfarerPersistentStateIsValid());
        }
    }
}

TEST("Wayfarer caller rejects missing zero reserved and unregistered appearance before save clearing")
{
    u32 attempt;
    MainCallback runnerCallback = gMain.callback2;
    u8 runnerState = gMain.state;
    EXPECT(WayfarerConfirmPendingOrigin(ORIGIN_NEW_BARK));
    for (attempt = 0; attempt < 5; attempt++)
    {
        MainCallback rejectedCallback;
        WayfarerResetPendingAppearance();
        if (attempt != 0)
            EXPECT(!WayfarerConfirmPendingAppearance(attempt == 1 ? APPEARANCE_NONE : attempt == 2 ? 255 : attempt));
        gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = APPEARANCE_MAY;
        gSaveBlock1Ptr->money = 12345;
        CB2_NewGame();
        rejectedCallback = gMain.callback2;
        gMain.callback2 = runnerCallback;
        gMain.state = runnerState;
        EXPECT(rejectedCallback == CB2_InitTitleScreen);
        EXPECT_EQ(gSaveBlock1Ptr->money, 12345);
        EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId, APPEARANCE_MAY);
        NewGameInitData();
        EXPECT_EQ(gSaveBlock1Ptr->money, 12345);
    }
}
TEST("Wayfarer all styles retain identity through first Hoenn arrival and regional warps")
{
    static const struct { u8 group, num, region; } destinations[] =
    {
        {MAP_GROUP(MAP_PALLET_TOWN_HNS), MAP_NUM(MAP_PALLET_TOWN_HNS), REGION_KANTO},
        {MAP_GROUP(MAP_SLATEPORT_CITY), MAP_NUM(MAP_SLATEPORT_CITY), REGION_HOENN},
        {MAP_GROUP(MAP_NEW_BARK_TOWN_HNS), MAP_NUM(MAP_NEW_BARK_TOWN_HNS), REGION_JOHTO},
    };
    u32 index, origin, destination;
    for (index = 0; index < APPEARANCE_COUNT; index++)
    {
        u8 id = WayfarerGetAppearanceIdByIndex(index);
        for (origin = ORIGIN_NEW_BARK; origin <= ORIGIN_LITTLEROOT; origin++)
        {
            EXPECT(WayfarerConfirmPendingAppearance(id));
            EXPECT(WayfarerConfirmPendingOrigin(origin));
            NewGameInitData();
            EXPECT(WayfarerPrepareHoennEntry());
            EXPECT_EQ(WayfarerGetPlayerAppearanceId(), id);
            for (destination = 0; destination < ARRAY_COUNT(destinations); destination++)
            {
                SetWarpDestinationToMapWarp(destinations[destination].group, destinations[destination].num, WARP_ID_NONE);
                WarpIntoMap();
                EXPECT_EQ(WayfarerGetSavedCurrentRegion(), destinations[destination].region);
                EXPECT_EQ(WayfarerGetPlayerAppearanceId(), id);
                EXPECT_EQ(WayfarerGetStartingOriginId(), origin);
                EXPECT_EQ(gSaveBlock2Ptr->playerGender, (id - 1) % 2);
                EXPECT(WayfarerPersistentStateIsValid());
            }
        }
    }
}

TEST("Wayfarer explicit rival graphics never borrow any saved player appearance")
{
    u32 index;
    for (index = 0; index < APPEARANCE_COUNT; index++)
    {
        u8 id = WayfarerGetAppearanceIdByIndex(index);
        EXPECT(WayfarerConfirmPendingAppearance(id));
        gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = id;
        EXPECT_EQ(GetRivalAvatarGraphicsIdByStateIdAndGender(PLAYER_AVATAR_STATE_NORMAL, MALE), OBJ_EVENT_GFX_GOLD_NORMAL_HNS);
        EXPECT_EQ(GetRivalAvatarGraphicsIdByStateIdAndGender(PLAYER_AVATAR_STATE_NORMAL, FEMALE), OBJ_EVENT_GFX_KRIS_NORMAL_HNS);
    }
}
#endif
