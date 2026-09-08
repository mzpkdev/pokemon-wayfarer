#include "global.h"
#include "wayfarer_appearance.h"
#if IS_WAYFARER
#include "constants/event_objects.h"
#include "constants/trainers.h"

static EWRAM_DATA struct
{
    u8 candidate;
    u8 confirmed;
    bool8 isConfirmed;
} sPendingAppearance = {0};

static const struct WayfarerAppearanceProfile sProfiles[] =
{
    {
        .id = APPEARANCE_GOLD, .gender = MALE,
        .frontPic = TRAINER_PIC_FRONT_GOLD_HNS, .backPic = TRAINER_PIC_BACK_GOLD_HNS,
        .paletteTag = OBJ_EVENT_PAL_TAG_GOLD_HNS,
        .graphicsIds = {
            [PLAYER_AVATAR_STATE_NORMAL] = OBJ_EVENT_GFX_GOLD_NORMAL_HNS,
            [PLAYER_AVATAR_STATE_MACH_BIKE] = OBJ_EVENT_GFX_GOLD_MACH_BIKE_HNS,
            [PLAYER_AVATAR_STATE_ACRO_BIKE] = OBJ_EVENT_GFX_GOLD_ACRO_BIKE_HNS,
            [PLAYER_AVATAR_STATE_SURFING] = OBJ_EVENT_GFX_GOLD_SURFING_HNS,
            [PLAYER_AVATAR_STATE_UNDERWATER] = OBJ_EVENT_GFX_GOLD_UNDERWATER_HNS,
            [PLAYER_AVATAR_STATE_FIELD_MOVE] = OBJ_EVENT_GFX_GOLD_FIELD_MOVE_HNS,
            [PLAYER_AVATAR_STATE_FISHING] = OBJ_EVENT_GFX_GOLD_FISHING_HNS,
            [PLAYER_AVATAR_STATE_WATERING] = OBJ_EVENT_GFX_GOLD_WATERING_HNS,
            [PLAYER_AVATAR_STATE_VSSEEKER] = OBJ_EVENT_GFX_GOLD_FIELD_MOVE_HNS,
        },
    },
    {
        .id = APPEARANCE_KRIS, .gender = FEMALE,
        .frontPic = TRAINER_PIC_FRONT_KRIS_HNS, .backPic = TRAINER_PIC_BACK_KRIS_HNS,
        .paletteTag = OBJ_EVENT_PAL_TAG_KRIS_HNS,
        .graphicsIds = {
            [PLAYER_AVATAR_STATE_NORMAL] = OBJ_EVENT_GFX_KRIS_NORMAL_HNS,
            [PLAYER_AVATAR_STATE_MACH_BIKE] = OBJ_EVENT_GFX_KRIS_MACH_BIKE_HNS,
            [PLAYER_AVATAR_STATE_ACRO_BIKE] = OBJ_EVENT_GFX_KRIS_ACRO_BIKE_HNS,
            [PLAYER_AVATAR_STATE_SURFING] = OBJ_EVENT_GFX_KRIS_SURFING_HNS,
            [PLAYER_AVATAR_STATE_UNDERWATER] = OBJ_EVENT_GFX_KRIS_UNDERWATER_HNS,
            [PLAYER_AVATAR_STATE_FIELD_MOVE] = OBJ_EVENT_GFX_KRIS_FIELD_MOVE_HNS,
            [PLAYER_AVATAR_STATE_FISHING] = OBJ_EVENT_GFX_KRIS_FISHING_HNS,
            [PLAYER_AVATAR_STATE_WATERING] = OBJ_EVENT_GFX_KRIS_WATERING_HNS,
            [PLAYER_AVATAR_STATE_VSSEEKER] = OBJ_EVENT_GFX_KRIS_FIELD_MOVE_HNS,
        },
    },
    {
        .id = APPEARANCE_RED, .gender = MALE,
        .frontPic = TRAINER_PIC_FRONT_RED, .backPic = TRAINER_PIC_BACK_RED,
        .paletteTag = OBJ_EVENT_PAL_TAG_PLAYER_RED,
        .graphicsIds = {
            [PLAYER_AVATAR_STATE_NORMAL] = OBJ_EVENT_GFX_RED_NORMAL,
            [PLAYER_AVATAR_STATE_MACH_BIKE] = OBJ_EVENT_GFX_RED_BIKE,
            [PLAYER_AVATAR_STATE_ACRO_BIKE] = OBJ_EVENT_GFX_RED_ACRO_BIKE_WAYFARER,
            [PLAYER_AVATAR_STATE_SURFING] = OBJ_EVENT_GFX_RED_SURF,
            [PLAYER_AVATAR_STATE_UNDERWATER] = OBJ_EVENT_GFX_RED_UNDERWATER_WAYFARER,
            [PLAYER_AVATAR_STATE_FIELD_MOVE] = OBJ_EVENT_GFX_RED_FIELD_MOVE,
            [PLAYER_AVATAR_STATE_FISHING] = OBJ_EVENT_GFX_RED_FISH,
            [PLAYER_AVATAR_STATE_WATERING] = OBJ_EVENT_GFX_RED_WATERING_WAYFARER,
            [PLAYER_AVATAR_STATE_VSSEEKER] = OBJ_EVENT_GFX_RED_VS_SEEKER,
        },
    },
    {
        .id = APPEARANCE_LEAF, .gender = FEMALE,
        .frontPic = TRAINER_PIC_FRONT_LEAF, .backPic = TRAINER_PIC_BACK_LEAF,
        .paletteTag = OBJ_EVENT_PAL_TAG_PLAYER_GREEN,
        .graphicsIds = {
            [PLAYER_AVATAR_STATE_NORMAL] = OBJ_EVENT_GFX_GREEN_NORMAL,
            [PLAYER_AVATAR_STATE_MACH_BIKE] = OBJ_EVENT_GFX_GREEN_BIKE,
            [PLAYER_AVATAR_STATE_ACRO_BIKE] = OBJ_EVENT_GFX_GREEN_ACRO_BIKE_WAYFARER,
            [PLAYER_AVATAR_STATE_SURFING] = OBJ_EVENT_GFX_GREEN_SURF,
            [PLAYER_AVATAR_STATE_UNDERWATER] = OBJ_EVENT_GFX_GREEN_UNDERWATER_WAYFARER,
            [PLAYER_AVATAR_STATE_FIELD_MOVE] = OBJ_EVENT_GFX_GREEN_FIELD_MOVE,
            [PLAYER_AVATAR_STATE_FISHING] = OBJ_EVENT_GFX_GREEN_FISH,
            [PLAYER_AVATAR_STATE_WATERING] = OBJ_EVENT_GFX_GREEN_WATERING_WAYFARER,
            [PLAYER_AVATAR_STATE_VSSEEKER] = OBJ_EVENT_GFX_GREEN_VS_SEEKER,
        },
    },
    {
        .id = APPEARANCE_BRENDAN, .gender = MALE,
        .frontPic = TRAINER_PIC_FRONT_BRENDAN, .backPic = TRAINER_PIC_BACK_BRENDAN,
        .paletteTag = OBJ_EVENT_PAL_TAG_BRENDAN,
        .graphicsIds = {
            [PLAYER_AVATAR_STATE_NORMAL] = OBJ_EVENT_GFX_BRENDAN_NORMAL,
            [PLAYER_AVATAR_STATE_MACH_BIKE] = OBJ_EVENT_GFX_BRENDAN_MACH_BIKE,
            [PLAYER_AVATAR_STATE_ACRO_BIKE] = OBJ_EVENT_GFX_BRENDAN_ACRO_BIKE,
            [PLAYER_AVATAR_STATE_SURFING] = OBJ_EVENT_GFX_BRENDAN_SURFING,
            [PLAYER_AVATAR_STATE_UNDERWATER] = OBJ_EVENT_GFX_BRENDAN_UNDERWATER,
            [PLAYER_AVATAR_STATE_FIELD_MOVE] = OBJ_EVENT_GFX_BRENDAN_FIELD_MOVE,
            [PLAYER_AVATAR_STATE_FISHING] = OBJ_EVENT_GFX_BRENDAN_FISHING,
            [PLAYER_AVATAR_STATE_WATERING] = OBJ_EVENT_GFX_BRENDAN_WATERING,
            [PLAYER_AVATAR_STATE_VSSEEKER] = OBJ_EVENT_GFX_BRENDAN_FIELD_MOVE,
        },
    },
    {
        .id = APPEARANCE_MAY, .gender = FEMALE,
        .frontPic = TRAINER_PIC_FRONT_MAY, .backPic = TRAINER_PIC_BACK_MAY,
        .paletteTag = OBJ_EVENT_PAL_TAG_MAY,
        .graphicsIds = {
            [PLAYER_AVATAR_STATE_NORMAL] = OBJ_EVENT_GFX_MAY_NORMAL,
            [PLAYER_AVATAR_STATE_MACH_BIKE] = OBJ_EVENT_GFX_MAY_MACH_BIKE,
            [PLAYER_AVATAR_STATE_ACRO_BIKE] = OBJ_EVENT_GFX_MAY_ACRO_BIKE,
            [PLAYER_AVATAR_STATE_SURFING] = OBJ_EVENT_GFX_MAY_SURFING,
            [PLAYER_AVATAR_STATE_UNDERWATER] = OBJ_EVENT_GFX_MAY_UNDERWATER,
            [PLAYER_AVATAR_STATE_FIELD_MOVE] = OBJ_EVENT_GFX_MAY_FIELD_MOVE,
            [PLAYER_AVATAR_STATE_FISHING] = OBJ_EVENT_GFX_MAY_FISHING,
            [PLAYER_AVATAR_STATE_WATERING] = OBJ_EVENT_GFX_MAY_WATERING,
            [PLAYER_AVATAR_STATE_VSSEEKER] = OBJ_EVENT_GFX_MAY_FIELD_MOVE,
        },
    },
};

const struct WayfarerAppearanceProfile *WayfarerGetAppearanceProfile(u8 id)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sProfiles); i++)
        if (sProfiles[i].id == id)
            return &sProfiles[i];
    return NULL;
}

u8 WayfarerGetPlayerAppearanceId(void)
{
    const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId);
    if (profile == NULL || profile->gender != gSaveBlock2Ptr->playerGender)
        return APPEARANCE_NONE;
    return profile->id;
}

u16 WayfarerGetAppearanceGraphicsId(u8 id, u8 state)
{
    const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(id);
    if (profile == NULL || state >= ARRAY_COUNT(profile->graphicsIds))
        return APPEARANCE_INVALID_GRAPHICS;
    return profile->graphicsIds[state];
}

u16 WayfarerGetAppearanceFrontPic(u8 id)
{
    const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(id);
    return profile == NULL ? APPEARANCE_INVALID_GRAPHICS : profile->frontPic;
}

u16 WayfarerGetAppearanceBackPic(u8 id)
{
    const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(id);
    return profile == NULL ? APPEARANCE_INVALID_GRAPHICS : profile->backPic;
}

u16 WayfarerGetAppearancePaletteTag(u8 id)
{
    const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(id);
    return profile == NULL ? APPEARANCE_INVALID_GRAPHICS : profile->paletteTag;
}

void WayfarerResetPendingAppearance(void)
{
    sPendingAppearance.candidate = APPEARANCE_GOLD;
    sPendingAppearance.confirmed = APPEARANCE_NONE;
    sPendingAppearance.isConfirmed = FALSE;
}

bool8 WayfarerSetPendingAppearanceCandidate(u8 id)
{
    if (WayfarerGetAppearanceProfile(id) == NULL)
        return FALSE;
    sPendingAppearance.candidate = id;
    return TRUE;
}

u8 WayfarerGetPendingAppearanceCandidate(void)
{
    return sPendingAppearance.candidate;
}

bool8 WayfarerConfirmPendingAppearance(u8 id)
{
    const struct WayfarerAppearanceProfile *profile = WayfarerGetAppearanceProfile(id);
    sPendingAppearance.confirmed = APPEARANCE_NONE;
    sPendingAppearance.isConfirmed = FALSE;
    if (profile == NULL)
        return FALSE;
    sPendingAppearance.candidate = id;
    sPendingAppearance.confirmed = id;
    sPendingAppearance.isConfirmed = TRUE;
    gSaveBlock2Ptr->playerGender = profile->gender;
    return TRUE;
}

u8 WayfarerGetConfirmedPendingAppearance(void)
{
    if (!sPendingAppearance.isConfirmed || WayfarerGetAppearanceProfile(sPendingAppearance.confirmed) == NULL)
        return APPEARANCE_NONE;
    return sPendingAppearance.confirmed;
}
#endif
