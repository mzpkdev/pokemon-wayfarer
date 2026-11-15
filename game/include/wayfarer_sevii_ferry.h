#ifndef GUARD_WAYFARER_SEVII_FERRY_H
#define GUARD_WAYFARER_SEVII_FERRY_H

#include "global.h"

#if IS_WAYFARER
void DrawWayfarerVermilionPortMenu(void);
u16 GetWayfarerVermilionPortChoice(void);
#define WAYFARER_VERMILION_SEVII_MAX_ROWS 3
u8 WayfarerGetVermilionSeviiDestinations(s32 *destinations);
void DrawWayfarerVermilionSeviiDestinationMenu(void);
u16 GetWayfarerVermilionSeviiDestination(void);
u16 WayfarerCanSailToBirthIsland(void);
u16 GetFerryLilycoveCharterCount(void);
u8 Ferry_GetCharterRows(u8 *ids);
void DrawFerryLilycoveCharterMenu(void);
u16 GetFerryLilycoveCharter(void);
#endif

#endif // GUARD_WAYFARER_SEVII_FERRY_H
