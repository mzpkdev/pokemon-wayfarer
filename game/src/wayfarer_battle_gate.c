#include "global.h"
#include "battle_setup.h"
#include "follower_npc.h"
#include "trainer_only_encounter.h"
#include "wayfarer_loss_policy.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "main.h"
#include "overworld.h"
#include "script.h"
#include "title_screen.h"
#include "wayfarer_battle_gate.h"
#include "wayfarer_origin.h"

bool8 TrainerOnlyCanEnterWildEncounter(void)
{
#if IS_WAYFARER
    return !WayfarerCanStartOrdinaryBattle()
        && WayfarerAllowsOrdinaryPartyExhaustion()
        && !FollowerNPCIsBattlePartner()
        && !(B_FLAG_FORCE_DOUBLE_WILD != 0 && FlagGet(B_FLAG_FORCE_DOUBLE_WILD))
        && !BattleSetup_IsUnidentifiedGhostEncounter();
#else
    return FALSE;
#endif
}

void WayfarerAbortEmptyPartyBattle(void)
{
#if IS_WAYFARER
    const struct WarpData *recovery;

    TrainerOnlyResetEncounter();

    // An unexpected battle cannot run its success continuation or loss rewards.
    ScriptContext_Init();
    FlagClear(FLAG_SAFE_FOLLOWER_MOVEMENT);
    if (!WayfarerEnsureRecoveryDestination())
    {
        SetMainCallback2(CB2_InitTitleScreen);
        return;
    }
    recovery = &gSaveBlock1Ptr->lastHealLocation;
    SetWarpDestination(recovery->mapGroup, recovery->mapNum, recovery->warpId,
                       recovery->x, recovery->y);
    DoWarp();
#endif
}
