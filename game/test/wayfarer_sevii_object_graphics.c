#include "global.h"
#include "event_object_movement.h"
#include "test/test.h"
#include "constants/event_objects.h"

#if IS_WAYFARER

static const u16 sOrdinarySeviiTrainerGraphics[] = {
    OBJ_EVENT_GFX_BATTLE_GIRL,
    OBJ_EVENT_GFX_BEAUTY_FRLG,
    OBJ_EVENT_GFX_BLACKBELT,
    OBJ_EVENT_GFX_BOY,
    OBJ_EVENT_GFX_BUG_CATCHER_FRLG,
    OBJ_EVENT_GFX_CAMPER_FRLG,
    OBJ_EVENT_GFX_COOLTRAINER_F,
    OBJ_EVENT_GFX_COOLTRAINER_M,
    OBJ_EVENT_GFX_FISHER,
    OBJ_EVENT_GFX_GENTLEMAN_FRLG,
    OBJ_EVENT_GFX_HIKER_FRLG,
    OBJ_EVENT_GFX_LASS_FRLG,
    OBJ_EVENT_GFX_LITTLE_GIRL_FRLG,
    OBJ_EVENT_GFX_MAN,
    OBJ_EVENT_GFX_PICNICKER_FRLG,
    OBJ_EVENT_GFX_ROCKER,
    OBJ_EVENT_GFX_SUPER_NERD,
    OBJ_EVENT_GFX_SWIMMER_F_LAND,
    OBJ_EVENT_GFX_SWIMMER_F_WATER,
    OBJ_EVENT_GFX_SWIMMER_M_LAND,
    OBJ_EVENT_GFX_SWIMMER_M_WATER,
    OBJ_EVENT_GFX_TUBER_F_FRLG,
    OBJ_EVENT_GFX_TUBER_M_WATER,
    OBJ_EVENT_GFX_WOMAN_1_FRLG,
    OBJ_EVENT_GFX_WOMAN_2_FRLG,
    OBJ_EVENT_GFX_YOUNGSTER_FRLG,
};

TEST("Wayfarer resolves every projected ordinary Sevii Trainer graphics family")
{
    u32 i;

    EXPECT_EQ(ARRAY_COUNT(sOrdinarySeviiTrainerGraphics), 26);
    for (i = 0; i < ARRAY_COUNT(sOrdinarySeviiTrainerGraphics); i++)
    {
        const struct ObjectEventGraphicsInfo *info = GetObjectEventGraphicsInfo(sOrdinarySeviiTrainerGraphics[i]);

        EXPECT(info != NULL);
        EXPECT(info->images != NULL);
        EXPECT(info->images[0].data != NULL);
    }
}

TEST("Wayfarer does not expose unrelated FRLG object graphics")
{
    EXPECT(GetObjectEventGraphicsInfo(OBJ_EVENT_GFX_RED_NORMAL) == NULL);
    EXPECT(GetObjectEventGraphicsInfo(OBJ_EVENT_GFX_SS_ANNE) == NULL);
}

TEST("Wayfarer exposes the selected Hideout Giovanni object graphics")
{
    const struct ObjectEventGraphicsInfo *info = GetObjectEventGraphicsInfo(OBJ_EVENT_GFX_GIOVANNI);

    EXPECT(info != NULL);
    EXPECT(info->images != NULL);
    EXPECT(info->images[0].data != NULL);
}

#endif // IS_WAYFARER
