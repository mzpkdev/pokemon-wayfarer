#include "global.h"
#include "field_effect.h"
#include "sprite.h"
#include "test/test.h"
#include "constants/field_effects.h"

#if IS_WAYFARER

TEST("Wayfarer Trainer sight does not leak an active exclamation effect when sprites are full")
{
    u32 i;

    ResetSpriteData();
    FieldEffectActiveListClear();
    for (i = 0; i < MAX_SPRITES; i++)
        EXPECT_NE(CreateSprite(&gDummySpriteTemplate, 0, 0, 0), MAX_SPRITES);

    FieldEffectStart(FLDEFF_EXCLAMATION_MARK_ICON);
    EXPECT(!FieldEffectActiveListContains(FLDEFF_EXCLAMATION_MARK_ICON));

    ResetSpriteData();
    FieldEffectActiveListClear();
}

#endif // IS_WAYFARER
