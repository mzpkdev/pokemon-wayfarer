#include "global.h"
#include "event_data.h"
#include "field_screen_effect.h"
#include "main.h"
#include "overworld.h"
#include "script.h"
#include "title_screen.h"
#include "wayfarer_battle_gate.h"
#include "wayfarer_origin.h"

void WayfarerAbortEmptyPartyBattle(void)
{
#if IS_WAYFARER
    const struct WarpData *recovery;

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
