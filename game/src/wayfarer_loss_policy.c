#include "global.h"
#include "wayfarer_loss_policy.h"
#include "battle.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "bug_contest.h"
#include "event_data.h"
#include "league_circuit.h"
#include "main.h"
#include "nuzlocke.h"
#include "overworld.h"
#include "safari_zone.h"
#include "trainer_hill.h"
#include "wayfarer_persistence.h"
#include "constants/heal_locations.h"
#include "constants/regions.h"

#if IS_WAYFARER
extern bool8 BattleSetup_IsOrdinaryWildCaller(void);

static EWRAM_DATA const u8 *sTrainerLossRedirect = NULL;
static EWRAM_DATA enum WayfarerRecoveryCause sRecoveryCause = WAYFARER_RECOVERY_NONE;
static EWRAM_DATA bool8 sBattleLossMoneyHandled = FALSE;
#endif

bool8 WayfarerAllowsOrdinaryPartyExhaustion(void)
{
#if IS_WAYFARER
    return !IsNuzlockeActive() && !IsNuzlockeEasyActive()
        && !gSaveBlock3Ptr->challengeSettings.tx_Challenges_NuzlockeHardcore
        && GetActiveLeagueRunRegion() == REGION_NONE
        && !GetSafariZoneFlag() && !GetBugContestFlag()
        && !CurrentBattlePyramidLocation() && !InBattlePike() && !InTrainerHillChallenge();
#else
    return FALSE;
#endif
}

bool8 WayfarerAllowsUnprotectedStorage(void)
{
    return WayfarerAllowsOrdinaryPartyExhaustion();
}

bool8 WayfarerShouldContinuePartyDefeat(void)
{
#if IS_WAYFARER
    const u32 allowedFlags = BATTLE_TYPE_IS_MASTER | BATTLE_TYPE_TRAINER;
    if (!WayfarerAllowsOrdinaryPartyExhaustion()
     || gBattleOutcome != B_OUTCOME_LOST
     || (gBattleTypeFlags & ~allowedFlags))
        return FALSE;
    if (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        return sTrainerLossRedirect != NULL;
    return BattleSetup_IsOrdinaryWildCaller();
#else
    return FALSE;
#endif
}

bool8 WayfarerShouldRetreatFromSupportedTrainerOutcome(void)
{
#if IS_WAYFARER
    const u32 allowedFlags = BATTLE_TYPE_IS_MASTER | BATTLE_TYPE_TRAINER;

    // A matching redirect is a caller-level contract. Within the same ordinary
    // field configuration, only an actual win may enter authored post-battle
    // success code; loss, draw, forfeit and abnormal endings all use its safe
    // retreat. Challenge and partner routing remains unchanged.
    return WayfarerAllowsOrdinaryPartyExhaustion()
        && sTrainerLossRedirect != NULL
        && (gBattleTypeFlags & BATTLE_TYPE_TRAINER)
        && !(gBattleTypeFlags & ~allowedFlags)
        && gBattleOutcome != B_OUTCOME_WON;
#else
    return FALSE;
#endif
}

void WayfarerResetLossContext(void)
{
#if IS_WAYFARER
    sTrainerLossRedirect = NULL;
    sBattleLossMoneyHandled = FALSE;
#endif
}

void WayfarerSetTrainerLossRedirect(const u8 *script)
{
#if IS_WAYFARER
    sTrainerLossRedirect = script;
#endif
}

const u8 *WayfarerGetTrainerLossRedirect(void)
{
#if IS_WAYFARER
    return sTrainerLossRedirect;
#else
    return NULL;
#endif
}

void WayfarerRecordBattleLossMoney(void)
{
#if IS_WAYFARER
    sBattleLossMoneyHandled = TRUE;
#endif
}

bool8 WayfarerBattleLossMoneyWasHandled(void)
{
#if IS_WAYFARER
    return sBattleLossMoneyHandled;
#else
    return FALSE;
#endif
}

void WayfarerMarkTrainerRetaliation(void)
{
#if IS_WAYFARER
    sRecoveryCause = WAYFARER_RECOVERY_TRAINER_RETALIATION;
#endif
}

void WayfarerBeginTrainerRetaliationRecovery(void)
{
#if IS_WAYFARER
    WayfarerMarkTrainerRetaliation();
    // Field-poison whiteouts choose Lavaridge before entering the shared whiteout
    // callback. Retaliation enters that callback directly, so retain the same
    // established, region-scoped recovery route here.
    if (WayfarerShouldWhiteOutToLavaridge())
        SetLastHealLocationWarp(HEAL_LOCATION_LAVARIDGE_TOWN);
    SetMainCallback2(CB2_WhiteOut);
#endif
}

bool8 WayfarerRecoveryIsTrainerRetaliation(void)
{
#if IS_WAYFARER
    return sRecoveryCause == WAYFARER_RECOVERY_TRAINER_RETALIATION;
#else
    return FALSE;
#endif
}

void WayfarerClearRecoveryCause(void)
{
#if IS_WAYFARER
    sRecoveryCause = WAYFARER_RECOVERY_NONE;
#endif
}
