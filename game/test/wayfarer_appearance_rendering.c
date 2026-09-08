#include "global.h"
#include "battle.h"
#include "contest.h"
#include "contest_util.h"
#include "event_object_movement.h"
#include "constants/event_objects.h"
#include "battle_controllers.h"
#include "link.h"
#include "pokemon.h"
#include "trainer_pokemon_sprites.h"
#include "wayfarer_appearance.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/trainers.h"

#if IS_WAYFARER
TEST("Wayfarer local trainer pictures read saved identity while legacy mappers stay gender based")
{
    u32 id;
    WayfarerInitPersistentState();
    for (id = APPEARANCE_GOLD; id <= APPEARANCE_MAY; id++)
    {
        const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(id);
        WayfarerResetPendingAppearance();
        EXPECT(WayfarerConfirmPendingAppearance(id == APPEARANCE_GOLD ? APPEARANCE_RED : APPEARANCE_GOLD));
        gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = id;
        gSaveBlock2Ptr->playerGender = profile->gender;
        EXPECT_EQ(GetLocalPlayerFrontTrainerPicId(), profile->frontPic);
        EXPECT_EQ(GetLocalPlayerBackTrainerPicId(), profile->backPic);
        EXPECT_EQ(PlayerGenderToFrontTrainerPicId(MALE), TRAINER_PIC_FRONT_GOLD_HNS);
        EXPECT_EQ(PlayerGenderToFrontTrainerPicId(FEMALE), TRAINER_PIC_FRONT_KRIS_HNS);
        EXPECT_EQ(PlayerGenderToFrontTrainerPicId_Debug(MALE, TRUE), TRAINER_PIC_FRONT_GOLD_HNS);
        EXPECT_EQ(PlayerGenderToFrontTrainerPicId_Debug(FEMALE, TRUE), TRAINER_PIC_FRONT_KRIS_HNS);
    }
}

TEST("Wayfarer live link local back pictures bypass gender-only link records for every style")
{
    u32 id;
    u32 oldBattleTypeFlags = gBattleTypeFlags;
    struct LinkPlayer oldLinkPlayer = gLinkPlayers[0];
    WayfarerInitPersistentState();
    gLinkPlayers[0].gender = FEMALE;
    gLinkPlayers[0].version = VERSION_FIRE_RED;
    for (id = APPEARANCE_GOLD; id <= APPEARANCE_MAY; id++)
    {
        const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(id);
        gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = id;
        gSaveBlock2Ptr->playerGender = profile->gender;
        gBattleTypeFlags = BATTLE_TYPE_LINK | BATTLE_TYPE_TRAINER;
        EXPECT_EQ(PlayerGetTrainerBackPicId(), profile->backPic);
        EXPECT_EQ(LinkPlayerGetTrainerPicId(0), TRAINER_PIC_BACK_KRIS_HNS);
        gBattleTypeFlags = BATTLE_TYPE_TRAINER;
        EXPECT_EQ(PlayerGetTrainerBackPicId(), profile->backPic);
        gBattleTypeFlags = BATTLE_TYPE_RECORDED | BATTLE_TYPE_TRAINER;
        EXPECT_EQ(PlayerGetTrainerBackPicId(), profile->gender == MALE ? TRAINER_PIC_BACK_GOLD_HNS : TRAINER_PIC_BACK_KRIS_HNS);
    }
    gLinkPlayers[0] = oldLinkPlayer;
    gBattleTypeFlags = oldBattleTypeFlags;
}
TEST("Wayfarer contest graphics retain full local IDs and unchanged remote records")
{
    u32 id, local, contestant;
    u8 oldPlayer = gContestPlayerMonIndex;
    u8 oldGraphics[CONTESTANT_COUNT];
    WayfarerInitPersistentState();
    for (contestant = 0; contestant < CONTESTANT_COUNT; contestant++)
    {
        oldGraphics[contestant] = gContestMons[contestant].trainerGfxId;
        gContestMons[contestant].trainerGfxId = OBJ_EVENT_GFX_LINK_MAY;
    }
    for (id = APPEARANCE_GOLD; id <= APPEARANCE_MAY; id++)
    {
        gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = id;
        gSaveBlock2Ptr->playerGender = WayfarerGetAppearanceProfile(id)->gender;
        for (local = 0; local < CONTESTANT_COUNT; local++)
        {
            gContestPlayerMonIndex = local;
            for (contestant = 0; contestant < CONTESTANT_COUNT; contestant++)
            {
                EXPECT_EQ(GetContestTrainerGraphicsId(contestant), contestant == local ? WayfarerGetAppearanceGraphicsId(id, PLAYER_AVATAR_STATE_NORMAL) : OBJ_EVENT_GFX_LINK_MAY);
                EXPECT_EQ(gContestMons[contestant].trainerGfxId, OBJ_EVENT_GFX_LINK_MAY);
            }
        }
    }
    for (contestant = 0; contestant < CONTESTANT_COUNT; contestant++)
        gContestMons[contestant].trainerGfxId = oldGraphics[contestant];
    gContestPlayerMonIndex = oldPlayer;
}
TEST("Wayfarer frontier head crops use uncompressed 16 by 32 normal frames")
{
    u32 id;
    for (id = APPEARANCE_GOLD; id <= APPEARANCE_MAY; id++)
    {
        const struct ObjectEventGraphicsInfo *graphics = GetObjectEventGraphicsInfo(WayfarerGetAppearanceGraphicsId(id, PLAYER_AVATAR_STATE_NORMAL));
        EXPECT_EQ((u32)graphics->compressed, FALSE);
        EXPECT_EQ(graphics->width, 16);
        EXPECT_EQ(graphics->height, 32);
        EXPECT(graphics->images != NULL);
        EXPECT(graphics->images[0].data != NULL);
        EXPECT(graphics->images[0].size >= 0x80);
    }
}
#endif
