#include "global.h"
#include "string_util.h"
#include "strings.h"
#include "wayfarer_origin.h"
#include "wayfarer_appearance.h"
#include "test/test.h"
#include "constants/maps.h"

#if IS_WAYFARER
TEST("Wayfarer rival names follow script source for both origins and appearances")
{
    u16 origin;
    u8 index, appearance;
    u8 gender;
    u8 text[32];

    for (origin = ORIGIN_NEW_BARK; origin <= ORIGIN_LITTLEROOT; origin++)
    {
        gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = origin;
        for (index = 0; index < APPEARANCE_COUNT; index++)
        {
            appearance = WayfarerGetAppearanceIdByIndex(index);
            EXPECT(WayfarerConfirmPendingAppearance(appearance));
            gSaveBlock3Ptr->wayfarerHoenn.playerAppearanceId = appearance;
            gender = WayfarerGetAppearanceProfile(appearance)->gender;
            StringCopy(gSaveBlock2Ptr->rivalName, gText_ExpandedPlaceholder_Silver);
            gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB);
            gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB);
            StringExpandPlaceholders(text, COMPOUND_STRING("{RIVAL}"));
            EXPECT_EQ(StringCompare(text, gender == MALE
                ? gText_ExpandedPlaceholder_May : gText_ExpandedPlaceholder_Brendan), 0);
            EXPECT_EQ(StringCompare(gSaveBlock2Ptr->rivalName, gText_ExpandedPlaceholder_Silver), 0);

            gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_NEW_BARK_TOWN_LAB_HNS);
            gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_NEW_BARK_TOWN_LAB_HNS);
            StringExpandPlaceholders(text, COMPOUND_STRING("{RIVAL}"));
            EXPECT_EQ(StringCompare(text, gText_ExpandedPlaceholder_Silver), 0);

            StringCopy(gSaveBlock2Ptr->rivalName, COMPOUND_STRING("CUSTOM"));
            StringExpandPlaceholders(text, COMPOUND_STRING("{RIVAL}"));
            EXPECT_EQ(StringCompare(text, COMPOUND_STRING("CUSTOM")), 0);
        }
    }
}
#endif
