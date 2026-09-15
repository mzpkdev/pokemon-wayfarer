#include "global.h"
#include "battle_setup.h"
#include "wayfarer_sevii_rematches.h"
#include "wayfarer_sevii_state.h"

#if IS_WAYFARER

#include "constants/opponents.h"

#define SEVII_REMATCH(a, b, c, d, e) {{a, b, c, d, e}}

const struct WayfarerSeviiRematchFamily gWayfarerSeviiRematchFamilies[WAYFARER_SEVII_REMATCH_FAMILY_COUNT] =
{
#include "data/wayfarer_sevii_rematches.h"
};

#undef SEVII_REMATCH

s32 WayfarerSeviiRematchFamilyIndex(u16 trainerId)
{
    s32 i;
    s32 stage;

    for (i = 0; i < WAYFARER_SEVII_REMATCH_FAMILY_COUNT; i++)
    {
        for (stage = 0; stage < WAYFARER_SEVII_REMATCH_STAGES; stage++)
        {
            if (gWayfarerSeviiRematchFamilies[i].trainerIds[stage] == trainerId)
                return i;
        }
    }

    return -1;
}

bool8 WayfarerSeviiRematchHasFamily(u16 trainerId)
{
    return WayfarerSeviiRematchFamilyIndex(trainerId) != -1;
}

bool8 WayfarerSeviiRematchIsReady(u16 trainerId)
{
    s32 family = WayfarerSeviiRematchFamilyIndex(trainerId);

    return family != -1 && WayfarerSeviiRematchPendingGet(family);
}

void WayfarerSeviiRematchSetReady(u16 trainerId)
{
    s32 family = WayfarerSeviiRematchFamilyIndex(trainerId);

    if (family != -1)
        WayfarerSeviiRematchPendingSet(family, TRUE);
}

void WayfarerSeviiRematchClearReady(u16 trainerId)
{
    s32 family = WayfarerSeviiRematchFamilyIndex(trainerId);

    if (family != -1)
        WayfarerSeviiRematchPendingSet(family, FALSE);
}

void WayfarerSeviiRematchClearAllReady(void)
{
    WayfarerSeviiRematchClearAllPending();
}

u16 WayfarerSeviiRematchGetOpponent(u16 trainerId)
{
    s32 family = WayfarerSeviiRematchFamilyIndex(trainerId);

    if (family == -1)
        return trainerId;

    u8 stage = WayfarerSeviiRematchStageGet(family);

    if (stage >= WAYFARER_SEVII_REMATCH_STAGES - 1)
        stage = 0;
    return gWayfarerSeviiRematchFamilies[family].trainerIds[stage + 1];
}

void WayfarerSeviiRematchAdvance(u16 trainerId)
{
    s32 family = WayfarerSeviiRematchFamilyIndex(trainerId);

    if (family != -1)
    {
        u8 stage = WayfarerSeviiRematchStageGet(family);

        if (stage < WAYFARER_SEVII_REMATCH_STAGES - 2)
            WayfarerSeviiRematchStageSet(family, stage + 1);
    }
}

#endif // IS_WAYFARER
