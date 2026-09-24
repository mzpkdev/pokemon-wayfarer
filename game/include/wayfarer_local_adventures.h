#ifndef GUARD_WAYFARER_LOCAL_ADVENTURES_H
#define GUARD_WAYFARER_LOCAL_ADVENTURES_H

#include "global.h"

#if IS_WAYFARER
u16 WayfarerCanStartOrdinaryBattleForScript(void);
const u8 *WayfarerResolveLocalAdventureTrainerBattleScript(const u8 *scriptStart);
#endif

#endif // GUARD_WAYFARER_LOCAL_ADVENTURES_H
