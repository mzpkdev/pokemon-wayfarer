#include "global.h"
#include "field_player_avatar.h"
#include "event_object_movement.h"
#include "sprite.h"
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

TEST("Wayfarer current movement graphics resolve all four saved appearances")
{
    u8 index, state;
    u8 oldAppearance = gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId;
    u8 oldFlags = gPlayerAvatar.flags;
    u8 oldGender = gPlayerAvatar.gender;
    u8 oldSavedGender = gSaveBlock2Ptr->playerGender;
    for (index = 0; index < APPEARANCE_COUNT; index++)
    {
        u8 id = WayfarerGetAppearanceIdByIndex(index);
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
    gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = APPEARANCE_BRENDAN;
    EXPECT_EQ(GetPlayerAvatarGraphicsIdByStateIdAndGender(PLAYER_AVATAR_STATE_NORMAL, MALE), OBJ_EVENT_GFX_GOLD_NORMAL_HNS);
    EXPECT_EQ(GetRivalAvatarGraphicsIdByStateIdAndGender(PLAYER_AVATAR_STATE_NORMAL, FEMALE), OBJ_EVENT_GFX_KRIS_NORMAL_HNS);
    gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = oldAppearance;
}
TEST("Wayfarer return-to-field reset keeps movement mode before object respawn")
{
    struct PlayerAvatar oldAvatar = gPlayerAvatar;
    const struct MapEvents *oldEvents = gMapHeader.events;
    bool8 active[OBJECT_EVENTS_COUNT];
    u8 state, i;

    // Isolate the reset boundary from map-specific NPC spawning. The emulator
    // journey additionally exercises this path with an active surfing player.
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        active[i] = gObjectEvents[i].active;
        gObjectEvents[i].active = FALSE;
    }
    gMapHeader.events = NULL;
    for (state = PLAYER_AVATAR_STATE_NORMAL; state <= PLAYER_AVATAR_STATE_UNDERWATER; state++)
    {
        ResetSpriteData();
        gPlayerAvatar.flags = (1 << state) | PLAYER_AVATAR_FLAG_CONTROLLABLE;
        gPlayerAvatar.runningState = 0x7F;
        SpawnObjectEventsOnReturnToField(0, 0);
        EXPECT_EQ(gPlayerAvatar.flags, 1 << state);
        EXPECT_EQ((u8)gPlayerAvatar.runningState, 0);
    }
    ResetSpriteData();
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        gObjectEvents[i].active = active[i];
    gMapHeader.events = oldEvents;
    gPlayerAvatar = oldAvatar;
}

#endif
