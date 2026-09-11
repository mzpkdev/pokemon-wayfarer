#ifndef GUARD_WAYFARER_SEVII_FERRY_H
#define GUARD_WAYFARER_SEVII_FERRY_H

#include "global.h"

#if IS_WAYFARER
void DrawWayfarerVermilionPortMenu(void);
u16 GetWayfarerVermilionPortChoice(void);
void DrawWayfarerVermilionSeviiDestinationMenu(void);
u16 GetWayfarerVermilionSeviiDestination(void);
u16 WayfarerCanSailToBirthIsland(void);
u16 WayfarerCanSailToNavelRock(void);
#endif

#endif // GUARD_WAYFARER_SEVII_FERRY_H
