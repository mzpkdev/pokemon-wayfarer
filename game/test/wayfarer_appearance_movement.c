#include "global.h"
#include "field_player_avatar.h"
#include "wayfarer_appearance.h"
#include "test/test.h"
#include "constants/event_objects.h"

#if IS_WAYFARER
TEST("Wayfarer sprite recreation preserves Acro and underwater state even with shared graphics")
{
    u8 oldFlags = gPlayerAvatar.flags;
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_ACRO_BIKE | PLAYER_AVATAR_FLAG_CONTROLLABLE;
    EXPECT_EQ(GetPlayerAvatarStateTransitionByGraphicsId(OBJ_EVENT_GFX_RED_BIKE, MALE), PLAYER_AVATAR_FLAG_ACRO_BIKE);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_CONTROLLABLE;
    EXPECT_EQ(GetPlayerAvatarStateTransitionByGraphicsId(OBJ_EVENT_GFX_RED_BIKE, MALE), PLAYER_AVATAR_FLAG_MACH_BIKE);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_UNDERWATER | PLAYER_AVATAR_FLAG_CONTROLLABLE;
    EXPECT_EQ(GetPlayerAvatarStateTransitionByGraphicsId(OBJ_EVENT_GFX_GREEN_SURF, FEMALE), PLAYER_AVATAR_FLAG_UNDERWATER);
    gPlayerAvatar.flags = PLAYER_AVATAR_FLAG_SURFING | PLAYER_AVATAR_FLAG_CONTROLLABLE;
    EXPECT_EQ(GetPlayerAvatarStateTransitionByGraphicsId(OBJ_EVENT_GFX_GREEN_SURF, FEMALE), PLAYER_AVATAR_FLAG_SURFING);
    gPlayerAvatar.flags = oldFlags;
}

TEST("Wayfarer current movement graphics resolve all six saved appearances")
{
    u8 id, state;
    u8 oldAppearance = gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId;
    u8 oldFlags = gPlayerAvatar.flags;
    u8 oldGender = gPlayerAvatar.gender;
    u8 oldSavedGender = gSaveBlock2Ptr->playerGender;
    for (id = APPEARANCE_GOLD; id <= APPEARANCE_MAY; id++)
    {
        gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = id;
        gPlayerAvatar.gender = WayfarerGetAppearanceProfile(id)->gender;
        gSaveBlock2Ptr->playerGender = gPlayerAvatar.gender;
        for (state = PLAYER_AVATAR_STATE_NORMAL; state <= PLAYER_AVATAR_STATE_UNDERWATER; state++)
        {
            gPlayerAvatar.flags = (1 << state) | PLAYER_AVATAR_FLAG_CONTROLLABLE;
            EXPECT_EQ(GetPlayerAvatarGraphicsIdByCurrentState(), WayfarerGetAppearanceGraphicsId(id, state));
        }
    }
    gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = oldAppearance;
    gPlayerAvatar.flags = oldFlags;
    gPlayerAvatar.gender = oldGender;
    gSaveBlock2Ptr->playerGender = oldSavedGender;
}

TEST("Wayfarer explicit gender and rival graphics never borrow the local appearance")
{
    u8 oldAppearance = gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId;
    gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = APPEARANCE_RED;
    EXPECT_EQ(GetPlayerAvatarGraphicsIdByStateIdAndGender(PLAYER_AVATAR_STATE_NORMAL, MALE), OBJ_EVENT_GFX_GOLD_NORMAL_HNS);
    EXPECT_EQ(GetRivalAvatarGraphicsIdByStateIdAndGender(PLAYER_AVATAR_STATE_NORMAL, FEMALE), OBJ_EVENT_GFX_KRIS_NORMAL_HNS);
    gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = oldAppearance;
}
#endif
