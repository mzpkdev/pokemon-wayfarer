#include "global.h"
#include "fieldmap.h"
#include "test/test.h"
#include "constants/metatile_behaviors.h"
#include "constants/metatile_behaviors_frlg.h"

TEST("FRLG behavior attributes normalize values whose engine meanings differ")
{
    EXPECT_EQ(ExtractMetatileAttribute(MB_FRLG_FAST_WATER, METATILE_ATTRIBUTE_BEHAVIOR, LAYOUT_VERSION_FRLG), MB_FAST_WATER);
    EXPECT_EQ(ExtractMetatileAttribute(MB_FRLG_REGULAR_WARP, METATILE_ATTRIBUTE_BEHAVIOR, LAYOUT_VERSION_FRLG), MB_NON_ANIMATED_DOOR);
    EXPECT_EQ(ExtractMetatileAttribute(MB_FRLG_FALL_WARP, METATILE_ATTRIBUTE_BEHAVIOR, LAYOUT_VERSION_FRLG), MB_MT_PYRE_HOLE);
    EXPECT_EQ(ExtractMetatileAttribute(MB_FRLG_UP_RIGHT_STAIR_WARP, METATILE_ATTRIBUTE_BEHAVIOR, LAYOUT_VERSION_FRLG), MB_UP_RIGHT_STAIR_WARP);
    EXPECT_EQ(ExtractMetatileAttribute(MB_FRLG_CABINET, METATILE_ATTRIBUTE_BEHAVIOR, LAYOUT_VERSION_FRLG), MB_CABINET);
}

TEST("FRLG normalization leaves non-behavior attributes and HNS behavior IDs unchanged")
{
    u32 attributes = MB_FRLG_FAST_WATER | (METATILE_LAYER_TYPE_SPLIT << METATILE_ATTR_LAYER_SHIFT_FRLG);

    EXPECT_EQ(ExtractMetatileAttribute(attributes, METATILE_ATTRIBUTE_LAYER_TYPE, LAYOUT_VERSION_FRLG), METATILE_LAYER_TYPE_SPLIT);
    EXPECT_EQ(ExtractMetatileAttribute(MB_SEAWEED, METATILE_ATTRIBUTE_BEHAVIOR, LAYOUT_VERSION_HNS), MB_SEAWEED);
}
