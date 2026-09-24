#ifndef GUARD_WAYFARER_CELADON_HIDEOUT_H
#define GUARD_WAYFARER_CELADON_HIDEOUT_H

#include "global.h"

#if IS_WAYFARER
u16 WayfarerCanStartOrdinaryBattleForScript(void);
u16 WayfarerGetRocketHideoutElevatorFloorForScript(void);
const u8 *WayfarerResolveCeladonHideoutTrainerBattleScript(const u8 *scriptStart);
u16 WayfarerCeladonHideoutTrainerDefeatFlag(u16 trainerId);
#endif

#endif // GUARD_WAYFARER_CELADON_HIDEOUT_H
