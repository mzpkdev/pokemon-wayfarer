#include "global.h"
#include "trainer_scaler.h"

bool32 ValidateTrainerScaler(const struct TrainerScalerAnchor *anchors, u32 count)
{
    u32 i;

    if (anchors == NULL || count == 0 || anchors[0].tr != 0)
        return FALSE;
    for (i = 1; i < count; i++)
        if (anchors[i].tr <= anchors[i - 1].tr || anchors[i].value < anchors[i - 1].value)
            return FALSE;
    return TRUE;
}

u16 EvaluateTrainerScaler(const struct TrainerScalerAnchor *anchors, u32 count, u32 rating, bool32 step)
{
    u32 i;

    // Invalid content must be rejected by its owner before preparation.
    assertf(anchors != NULL && count != 0);
    for (i = 1; i < count; i++)
    {
        if (rating < anchors[i].tr)
        {
            u32 width = anchors[i].tr - anchors[i - 1].tr;
            u64 rise;

            if (step)
                return anchors[i - 1].value;
            rise = (u64)(rating - anchors[i - 1].tr) * (anchors[i].value - anchors[i - 1].value);
            return anchors[i - 1].value + (2 * rise + width) / (2 * (u64)width);
        }
    }
    return anchors[count - 1].value;
}
