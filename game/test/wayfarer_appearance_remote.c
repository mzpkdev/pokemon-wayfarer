#include "global.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "wayfarer_appearance.h"
#include "test/test.h"
#include "constants/event_objects.h"

#if IS_WAYFARER
TEST("Wayfarer remote FRLG avatars retain complete legacy descriptors for every local style")
{
    u8 index, gender;
    u8 oldAppearance = gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId;
    u8 oldGender = gSaveBlock2Ptr->playerGender;
    for (index = 0; index < APPEARANCE_COUNT; index++)
    {
        u8 id = WayfarerGetAppearanceIdByIndex(index);
        gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = id;
        gSaveBlock2Ptr->playerGender = WayfarerGetAppearanceProfile(id)->gender;
        for (gender = MALE; gender <= FEMALE; gender++)
        {
            u16 graphicsId = GetFRLGAvatarGraphicsIdByGender(gender);
            const struct ObjectEventGraphicsInfo *info = GetObjectEventGraphicsInfo(graphicsId);
            EXPECT_EQ(graphicsId, gender == MALE ? OBJ_EVENT_GFX_RED : OBJ_EVENT_GFX_LEAF);
            EXPECT(info != NULL);
            if (info != NULL)
            {
                EXPECT_EQ(info->paletteTag, OBJ_EVENT_PAL_TAG_RED_LEAF);
                EXPECT_EQ(info->width, 16);
                EXPECT_EQ(info->height, 32);
                EXPECT(info->images != NULL);
                EXPECT(info->anims != NULL);
            }
        }
    }
    gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = oldAppearance;
    gSaveBlock2Ptr->playerGender = oldGender;
}
#endif
