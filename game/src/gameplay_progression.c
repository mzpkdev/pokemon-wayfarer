#include "global.h"
#include "gameplay_progression.h"

struct ProgressionCurve
{
    const u8 (*points)[2];
    u8 count;
};

#include "gameplay_progression.inc"

bool32 EvaluateGameplayCurve(u16 curveId, u32 rating, u8 *out)
{
    const struct ProgressionCurve *curve;
    const u8 (*point)[2];
    u32 remaining;
    if (curveId >= ARRAY_COUNT(sProgressionCurves) || out == NULL)
        return FALSE;
    curve = &sProgressionCurves[curveId];
    point = curve->points;
    remaining = curve->count;
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
