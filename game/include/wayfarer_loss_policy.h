#ifndef GUARD_WAYFARER_LOSS_POLICY_H
#define GUARD_WAYFARER_LOSS_POLICY_H

#include "global.h"

enum WayfarerRecoveryCause
{
    WAYFARER_RECOVERY_NONE,
    WAYFARER_RECOVERY_TRAINER_RETALIATION,
};

bool8 WayfarerAllowsOrdinaryPartyExhaustion(void);
bool8 WayfarerAllowsUnprotectedStorage(void);
bool8 WayfarerShouldContinuePartyDefeat(void);
void WayfarerResetLossContext(void);
void WayfarerSetTrainerLossRedirect(const u8 *script);
const u8 *WayfarerGetTrainerLossRedirect(void);
void WayfarerMarkTrainerRetaliation(void);
void WayfarerBeginTrainerRetaliationRecovery(void);
bool8 WayfarerRecoveryIsTrainerRetaliation(void);
void WayfarerClearRecoveryCause(void);
void WayfarerRecordBattleLossMoney(void);
bool8 WayfarerBattleLossMoneyWasHandled(void);

#endif
