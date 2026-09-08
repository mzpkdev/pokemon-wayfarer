#include "global.h"
#include "event_data.h"
#include "wayfarer_celadon_hideout.h"
#include "wayfarer_origin.h"
#include "constants/map_groups.h"

#if IS_WAYFARER

extern const u8 CeladonGameCorner_EventScript_HideoutGrunt[];
extern const u8 CeladonGameCorner_EventScript_HideoutGrunt_TrainerBattle[];
extern const u8 RocketHideout_B1F_EventScript_Grunt1[];
extern const u8 RocketHideout_B1F_EventScript_Grunt1_TrainerBattle[];
extern const u8 RocketHideout_B1F_EventScript_Grunt2[];
extern const u8 RocketHideout_B1F_EventScript_Grunt2_TrainerBattle[];
extern const u8 RocketHideout_B1F_EventScript_Grunt3[];
extern const u8 RocketHideout_B1F_EventScript_Grunt3_TrainerBattle[];
extern const u8 RocketHideout_B1F_EventScript_Grunt4[];
extern const u8 RocketHideout_B1F_EventScript_Grunt4_TrainerBattle[];
extern const u8 RocketHideout_B1F_EventScript_Grunt5[];
extern const u8 RocketHideout_B1F_EventScript_Grunt5_TrainerBattle[];
extern const u8 RocketHideout_B2F_EventScript_Grunt[];
extern const u8 RocketHideout_B2F_EventScript_Grunt_TrainerBattle[];
extern const u8 RocketHideout_B3F_EventScript_Grunt1[];
extern const u8 RocketHideout_B3F_EventScript_Grunt1_TrainerBattle[];
extern const u8 RocketHideout_B3F_EventScript_Grunt2[];
extern const u8 RocketHideout_B3F_EventScript_Grunt2_TrainerBattle[];
extern const u8 RocketHideout_B4F_EventScript_Grunt1[];
extern const u8 RocketHideout_B4F_EventScript_Grunt1_TrainerBattle[];
extern const u8 RocketHideout_B4F_EventScript_Grunt2[];
extern const u8 RocketHideout_B4F_EventScript_Grunt2_TrainerBattle[];
extern const u8 RocketHideout_B4F_EventScript_Grunt3[];
extern const u8 RocketHideout_B4F_EventScript_Grunt3_TrainerBattle[];
extern const u8 WayfarerMtMoon_EventScript_Grunt1[];
extern const u8 WayfarerMtMoon_EventScript_Grunt1TrainerBattle[];
extern const u8 WayfarerMtMoon_EventScript_Grunt2[];
extern const u8 WayfarerMtMoon_EventScript_Grunt2TrainerBattle[];
extern const u8 WayfarerMtMoon_EventScript_Grunt3[];
extern const u8 WayfarerMtMoon_EventScript_Grunt3TrainerBattle[];
extern const u8 WayfarerMtMoon_EventScript_Grunt4[];
extern const u8 WayfarerMtMoon_EventScript_Grunt4TrainerBattle[];

struct WayfarerTrainerBattleScript
{
    const u8 *scriptStart;
    const u8 *trainerBattle;
};

static const struct WayfarerTrainerBattleScript sCeladonHideoutTrainerBattleScripts[] =
{
    {CeladonGameCorner_EventScript_HideoutGrunt, CeladonGameCorner_EventScript_HideoutGrunt_TrainerBattle},
    {RocketHideout_B1F_EventScript_Grunt1, RocketHideout_B1F_EventScript_Grunt1_TrainerBattle},
    {RocketHideout_B1F_EventScript_Grunt2, RocketHideout_B1F_EventScript_Grunt2_TrainerBattle},
    {RocketHideout_B1F_EventScript_Grunt3, RocketHideout_B1F_EventScript_Grunt3_TrainerBattle},
    {RocketHideout_B1F_EventScript_Grunt4, RocketHideout_B1F_EventScript_Grunt4_TrainerBattle},
    {RocketHideout_B1F_EventScript_Grunt5, RocketHideout_B1F_EventScript_Grunt5_TrainerBattle},
    {RocketHideout_B2F_EventScript_Grunt, RocketHideout_B2F_EventScript_Grunt_TrainerBattle},
    {RocketHideout_B3F_EventScript_Grunt1, RocketHideout_B3F_EventScript_Grunt1_TrainerBattle},
    {RocketHideout_B3F_EventScript_Grunt2, RocketHideout_B3F_EventScript_Grunt2_TrainerBattle},
    {RocketHideout_B4F_EventScript_Grunt1, RocketHideout_B4F_EventScript_Grunt1_TrainerBattle},
    {RocketHideout_B4F_EventScript_Grunt2, RocketHideout_B4F_EventScript_Grunt2_TrainerBattle},
    {RocketHideout_B4F_EventScript_Grunt3, RocketHideout_B4F_EventScript_Grunt3_TrainerBattle},
    {WayfarerMtMoon_EventScript_Grunt1, WayfarerMtMoon_EventScript_Grunt1TrainerBattle},
    {WayfarerMtMoon_EventScript_Grunt2, WayfarerMtMoon_EventScript_Grunt2TrainerBattle},
    {WayfarerMtMoon_EventScript_Grunt3, WayfarerMtMoon_EventScript_Grunt3TrainerBattle},
    {WayfarerMtMoon_EventScript_Grunt4, WayfarerMtMoon_EventScript_Grunt4TrainerBattle},
};

u16 WayfarerCanStartOrdinaryBattleForScript(void)
{
    return WayfarerCanStartOrdinaryBattle();
}

const u8 *WayfarerResolveCeladonHideoutTrainerBattleScript(const u8 *scriptStart)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sCeladonHideoutTrainerBattleScripts); i++)
    {
        if (scriptStart == sCeladonHideoutTrainerBattleScripts[i].scriptStart)
            return sCeladonHideoutTrainerBattleScripts[i].trainerBattle;
    }

    return NULL;
}

u16 WayfarerGetRocketHideoutElevatorFloorForScript(void)
{
    // The current dynamic return identifies the Hideout floor that supplied
    // the elevator door. Keep this state separate from FRLG's generic
    // VAR_ELEVATOR_FLOOR, which aliases active HNS state.
    if (gSaveBlock1Ptr->dynamicWarp.mapGroup != MAP_GROUP(MAP_ROCKET_HIDEOUT_B1F))
        return 3;

    switch (gSaveBlock1Ptr->dynamicWarp.mapNum)
    {
    case MAP_NUM(MAP_ROCKET_HIDEOUT_B1F):
        return 3;
    case MAP_NUM(MAP_ROCKET_HIDEOUT_B2F):
        return 2;
    case MAP_NUM(MAP_ROCKET_HIDEOUT_B4F):
        return 0;
    default:
        return 3;
    }
}

#endif // IS_WAYFARER
