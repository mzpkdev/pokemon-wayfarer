#ifndef GUARD_WAYFARER_SEVII_REMATCHES_H
#define GUARD_WAYFARER_SEVII_REMATCHES_H

#include "global.h"

#define WAYFARER_SEVII_REMATCH_STAGES 5

struct WayfarerSeviiRematchFamily
{
    u16 trainerIds[WAYFARER_SEVII_REMATCH_STAGES];
};

extern const struct WayfarerSeviiRematchFamily gWayfarerSeviiRematchFamilies[WAYFARER_SEVII_REMATCH_FAMILY_COUNT];

bool8 WayfarerSeviiRematchHasFamily(u16 trainerId);
bool8 WayfarerSeviiRematchIsReady(u16 trainerId);
void WayfarerSeviiRematchSetReady(u16 trainerId);
void WayfarerSeviiRematchClearReady(u16 trainerId);
void WayfarerSeviiRematchClearAllReady(void);
u16 WayfarerSeviiRematchGetOpponent(u16 trainerId);
void WayfarerSeviiRematchAdvance(u16 trainerId);
s32 WayfarerSeviiRematchFamilyIndex(u16 trainerId);

#endif // GUARD_WAYFARER_SEVII_REMATCHES_H
