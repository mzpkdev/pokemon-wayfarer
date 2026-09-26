#include "global.h"
#include "pokemon.h"
#include "wayfarer_local_adventures.h"

#if IS_WAYFARER

extern const u8 ViridianCity_Gym_EventScript_Jason[];
extern const u8 ViridianCity_Gym_EventScript_Jason_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Cole[];
extern const u8 ViridianCity_Gym_EventScript_Cole_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Atsushi[];
extern const u8 ViridianCity_Gym_EventScript_Atsushi_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Kiyo[];
extern const u8 ViridianCity_Gym_EventScript_Kiyo_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Takashi[];
extern const u8 ViridianCity_Gym_EventScript_Takashi_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Samuel[];
extern const u8 ViridianCity_Gym_EventScript_Samuel_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Yuji[];
extern const u8 ViridianCity_Gym_EventScript_Yuji_TrainerBattle[];
extern const u8 ViridianCity_Gym_EventScript_Warren[];
extern const u8 ViridianCity_Gym_EventScript_Warren_TrainerBattle[];

extern const u8 WayfarerMtMoon_EventScript_Grunt1[];
extern const u8 WayfarerMtMoon_EventScript_Grunt1TrainerBattle[];
extern const u8 WayfarerMtMoon_EventScript_Grunt2[];
extern const u8 WayfarerMtMoon_EventScript_Grunt2TrainerBattle[];
extern const u8 WayfarerMtMoon_EventScript_Grunt3[];
extern const u8 WayfarerMtMoon_EventScript_Grunt3TrainerBattle[];
extern const u8 WayfarerMtMoon_EventScript_Grunt4[];
extern const u8 WayfarerMtMoon_EventScript_Grunt4TrainerBattle[];
extern const u8 SilphCo_2F_EventScript_Connor[];
extern const u8 SilphCo_2F_EventScript_Connor_TrainerBattle[];
extern const u8 SilphCo_2F_EventScript_Jerry[];
extern const u8 SilphCo_2F_EventScript_Jerry_TrainerBattle[];
extern const u8 SilphCo_2F_EventScript_Grunt1[];
extern const u8 SilphCo_2F_EventScript_Grunt1_TrainerBattle[];
extern const u8 SilphCo_2F_EventScript_Grunt2[];
extern const u8 SilphCo_2F_EventScript_Grunt2_TrainerBattle[];
extern const u8 SilphCo_3F_EventScript_Jose[];
extern const u8 SilphCo_3F_EventScript_Jose_TrainerBattle[];
extern const u8 SilphCo_3F_EventScript_Grunt[];
extern const u8 SilphCo_3F_EventScript_Grunt_TrainerBattle[];
extern const u8 SilphCo_4F_EventScript_Rodney[];
extern const u8 SilphCo_4F_EventScript_Rodney_TrainerBattle[];
extern const u8 SilphCo_4F_EventScript_Grunt1[];
extern const u8 SilphCo_4F_EventScript_Grunt1_TrainerBattle[];
extern const u8 SilphCo_4F_EventScript_Grunt2[];
extern const u8 SilphCo_4F_EventScript_Grunt2_TrainerBattle[];
extern const u8 SilphCo_5F_EventScript_Beau[];
extern const u8 SilphCo_5F_EventScript_Beau_TrainerBattle[];
extern const u8 SilphCo_5F_EventScript_Grunt1[];
extern const u8 SilphCo_5F_EventScript_Grunt1_TrainerBattle[];
extern const u8 SilphCo_5F_EventScript_Grunt2[];
extern const u8 SilphCo_5F_EventScript_Grunt2_TrainerBattle[];
extern const u8 SilphCo_5F_EventScript_Dalton[];
extern const u8 SilphCo_5F_EventScript_Dalton_TrainerBattle[];
extern const u8 SilphCo_6F_EventScript_Taylor[];
extern const u8 SilphCo_6F_EventScript_Taylor_TrainerBattle[];
extern const u8 SilphCo_6F_EventScript_Grunt1[];
extern const u8 SilphCo_6F_EventScript_Grunt1_TrainerBattle[];
extern const u8 SilphCo_6F_EventScript_Grunt2[];
extern const u8 SilphCo_6F_EventScript_Grunt2_TrainerBattle[];
extern const u8 SilphCo_7F_EventScript_Joshua[];
extern const u8 SilphCo_7F_EventScript_Joshua_TrainerBattle[];
extern const u8 SilphCo_7F_EventScript_Grunt1[];
extern const u8 SilphCo_7F_EventScript_Grunt1_TrainerBattle[];
extern const u8 SilphCo_7F_EventScript_Grunt2[];
extern const u8 SilphCo_7F_EventScript_Grunt2_TrainerBattle[];
extern const u8 SilphCo_7F_EventScript_Grunt3[];
extern const u8 SilphCo_7F_EventScript_Grunt3_TrainerBattle[];
extern const u8 SilphCo_8F_EventScript_Parker[];
extern const u8 SilphCo_8F_EventScript_Parker_TrainerBattle[];
extern const u8 SilphCo_8F_EventScript_Grunt1[];
extern const u8 SilphCo_8F_EventScript_Grunt1_TrainerBattle[];
extern const u8 SilphCo_8F_EventScript_Grunt2[];
extern const u8 SilphCo_8F_EventScript_Grunt2_TrainerBattle[];
extern const u8 SilphCo_9F_EventScript_Ed[];
extern const u8 SilphCo_9F_EventScript_Ed_TrainerBattle[];
extern const u8 SilphCo_9F_EventScript_Grunt1[];
extern const u8 SilphCo_9F_EventScript_Grunt1_TrainerBattle[];
extern const u8 SilphCo_9F_EventScript_Grunt2[];
extern const u8 SilphCo_9F_EventScript_Grunt2_TrainerBattle[];
extern const u8 SilphCo_10F_EventScript_Travis[];
extern const u8 SilphCo_10F_EventScript_Travis_TrainerBattle[];
extern const u8 SilphCo_10F_EventScript_Grunt[];
extern const u8 SilphCo_10F_EventScript_Grunt_TrainerBattle[];
extern const u8 SilphCo_11F_EventScript_Grunt1[];
extern const u8 SilphCo_11F_EventScript_Grunt1_TrainerBattle[];
extern const u8 SilphCo_11F_EventScript_Grunt2[];
extern const u8 SilphCo_11F_EventScript_Grunt2_TrainerBattle[];

struct LocalAdventureSightScript { const u8 *start; const u8 *battle; };
static const struct LocalAdventureSightScript sSightScripts[] =
{
    {ViridianCity_Gym_EventScript_Jason, ViridianCity_Gym_EventScript_Jason_TrainerBattle},
    {ViridianCity_Gym_EventScript_Cole, ViridianCity_Gym_EventScript_Cole_TrainerBattle},
    {ViridianCity_Gym_EventScript_Atsushi, ViridianCity_Gym_EventScript_Atsushi_TrainerBattle},
    {ViridianCity_Gym_EventScript_Kiyo, ViridianCity_Gym_EventScript_Kiyo_TrainerBattle},
    {ViridianCity_Gym_EventScript_Takashi, ViridianCity_Gym_EventScript_Takashi_TrainerBattle},
    {ViridianCity_Gym_EventScript_Samuel, ViridianCity_Gym_EventScript_Samuel_TrainerBattle},
    {ViridianCity_Gym_EventScript_Yuji, ViridianCity_Gym_EventScript_Yuji_TrainerBattle},
    {ViridianCity_Gym_EventScript_Warren, ViridianCity_Gym_EventScript_Warren_TrainerBattle},
    {WayfarerMtMoon_EventScript_Grunt1, WayfarerMtMoon_EventScript_Grunt1TrainerBattle},
    {WayfarerMtMoon_EventScript_Grunt2, WayfarerMtMoon_EventScript_Grunt2TrainerBattle},
    {WayfarerMtMoon_EventScript_Grunt3, WayfarerMtMoon_EventScript_Grunt3TrainerBattle},
    {WayfarerMtMoon_EventScript_Grunt4, WayfarerMtMoon_EventScript_Grunt4TrainerBattle},
    {SilphCo_2F_EventScript_Connor, SilphCo_2F_EventScript_Connor_TrainerBattle},
    {SilphCo_2F_EventScript_Jerry, SilphCo_2F_EventScript_Jerry_TrainerBattle},
    {SilphCo_2F_EventScript_Grunt1, SilphCo_2F_EventScript_Grunt1_TrainerBattle},
    {SilphCo_2F_EventScript_Grunt2, SilphCo_2F_EventScript_Grunt2_TrainerBattle},
    {SilphCo_3F_EventScript_Jose, SilphCo_3F_EventScript_Jose_TrainerBattle},
    {SilphCo_3F_EventScript_Grunt, SilphCo_3F_EventScript_Grunt_TrainerBattle},
    {SilphCo_4F_EventScript_Rodney, SilphCo_4F_EventScript_Rodney_TrainerBattle},
    {SilphCo_4F_EventScript_Grunt1, SilphCo_4F_EventScript_Grunt1_TrainerBattle},
    {SilphCo_4F_EventScript_Grunt2, SilphCo_4F_EventScript_Grunt2_TrainerBattle},
    {SilphCo_5F_EventScript_Beau, SilphCo_5F_EventScript_Beau_TrainerBattle},
    {SilphCo_5F_EventScript_Grunt1, SilphCo_5F_EventScript_Grunt1_TrainerBattle},
    {SilphCo_5F_EventScript_Grunt2, SilphCo_5F_EventScript_Grunt2_TrainerBattle},
    {SilphCo_5F_EventScript_Dalton, SilphCo_5F_EventScript_Dalton_TrainerBattle},
    {SilphCo_6F_EventScript_Taylor, SilphCo_6F_EventScript_Taylor_TrainerBattle},
    {SilphCo_6F_EventScript_Grunt1, SilphCo_6F_EventScript_Grunt1_TrainerBattle},
    {SilphCo_6F_EventScript_Grunt2, SilphCo_6F_EventScript_Grunt2_TrainerBattle},
    {SilphCo_7F_EventScript_Joshua, SilphCo_7F_EventScript_Joshua_TrainerBattle},
    {SilphCo_7F_EventScript_Grunt1, SilphCo_7F_EventScript_Grunt1_TrainerBattle},
    {SilphCo_7F_EventScript_Grunt2, SilphCo_7F_EventScript_Grunt2_TrainerBattle},
    {SilphCo_7F_EventScript_Grunt3, SilphCo_7F_EventScript_Grunt3_TrainerBattle},
    {SilphCo_8F_EventScript_Parker, SilphCo_8F_EventScript_Parker_TrainerBattle},
    {SilphCo_8F_EventScript_Grunt1, SilphCo_8F_EventScript_Grunt1_TrainerBattle},
    {SilphCo_8F_EventScript_Grunt2, SilphCo_8F_EventScript_Grunt2_TrainerBattle},
    {SilphCo_9F_EventScript_Ed, SilphCo_9F_EventScript_Ed_TrainerBattle},
    {SilphCo_9F_EventScript_Grunt1, SilphCo_9F_EventScript_Grunt1_TrainerBattle},
    {SilphCo_9F_EventScript_Grunt2, SilphCo_9F_EventScript_Grunt2_TrainerBattle},
    {SilphCo_10F_EventScript_Travis, SilphCo_10F_EventScript_Travis_TrainerBattle},
    {SilphCo_10F_EventScript_Grunt, SilphCo_10F_EventScript_Grunt_TrainerBattle},
    {SilphCo_11F_EventScript_Grunt1, SilphCo_11F_EventScript_Grunt1_TrainerBattle},
    {SilphCo_11F_EventScript_Grunt2, SilphCo_11F_EventScript_Grunt2_TrainerBattle},
};

const u8 *WayfarerResolveLocalAdventureTrainerBattleScript(const u8 *scriptStart)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sSightScripts); i++)
        if (scriptStart == sSightScripts[i].start)
            return sSightScripts[i].battle;
    return NULL;
}

#endif // IS_WAYFARER
