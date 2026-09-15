#ifndef GUARD_WAYFARER_SEVII_STATE_H
#define GUARD_WAYFARER_SEVII_STATE_H

#include "global.h"

// Story owns the SaveBlock3 payload behind these bounds-safe accessors. The
// ordinary Trainer runtime uses a 64-family, two-bit stage bank and a separate
// pending-ready bit bank; no caller receives the aggregate itself.
u8 WayfarerSeviiRematchStageGet(u8 family);
void WayfarerSeviiRematchStageSet(u8 family, u8 stage);
bool8 WayfarerSeviiRematchPendingGet(u8 family);
void WayfarerSeviiRematchPendingSet(u8 family, bool8 pending);
void WayfarerSeviiRematchClearAllPending(void);

#endif // GUARD_WAYFARER_SEVII_STATE_H
