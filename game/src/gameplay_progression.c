#include "global.h"
#include "gameplay_progression.h"

#include "gameplay_progression.inc"

bool32 EvaluateGameplayCurve(u16 curveId, u32 rating, u8 *out)
{
    const u8 (*point)[2];
    u32 remaining;
    if (out == NULL || !GetGameplayProgressionCurve(curveId, &point, &remaining))
        return FALSE;
    rating = min(rating, 80);
    while (--remaining)
    {
        if (rating <= point[1][0])
        {
            u32 width = point[1][0] - point[0][0];
            u32 rise = (rating - point[0][0]) * (point[1][1] - point[0][1]);
            *out = point[0][1] + (2 * rise + width) / (2 * width);
            return TRUE;
        }
        point++;
    }
    return FALSE;
}
