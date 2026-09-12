// Johto/HNS story integration owns this manifest.  Caller addresses are the
// trainerbattle argument blocks (`ScriptLabel + 1`), never trainer ids.
#if IS_WAYFARER
#include "constants/map_event_ids.h"

extern const u8 CherryGroveCity_Silver_Battle_Cyndaquil[];
extern const u8 CherryGroveCity_Silver_Battle_Totodile[];
extern const u8 CherryGroveCity_Silver_Battle_Chikorita[];
extern const u8 AzaleaTown_Silver_Battle_Cyndaquil[];
extern const u8 AzaleaTown_Silver_Battle_Totodile[];
extern const u8 AzaleaTown_Silver_Battle_Chikorita[];
extern const u8 AzaleaTown_EventScript_SilverRetreat[];
extern const u8 BurnedTower_1F_Silver_Battle_Cyndaquil[];
extern const u8 BurnedTower_1F_Silver_Battle_Totodile[];
extern const u8 BurnedTower_1F_Silver_Battle_Chikorita[];
extern const u8 BurnedTower_1F_EventScript_SilverRetreat[];
extern const u8 GoldenrodCity_UndergroundSwitches_EventScript_Cyndaquil[];
extern const u8 GoldenrodCity_UndergroundSwitches_EventScript_Totodile[];
extern const u8 GoldenrodCity_UndergroundSwitches_EventScript_Chikorita[];
extern const u8 GoldenrodCity_UndergroundSwitches_EventScript_SilverReturnCyndaquil[];
extern const u8 GoldenrodCity_UndergroundSwitches_EventScript_SilverReturnTotodile[];
extern const u8 GoldenrodCity_UndergroundSwitches_EventScript_SilverReturnChikorita[];
extern const u8 GoldenrodCity_UndergroundSwitches_EventScript_SilverRetreat[];
extern const u8 VictoryRoadKanto_1F_Silver_Battle_Cyndaquil[];
extern const u8 VictoryRoadKanto_1F_Silver_Battle_Totodile[];
extern const u8 VictoryRoadKanto_1F_Silver_Battle_Chikorita[];
extern const u8 MtMoon_Cave_EventScript_Silver_Battle_Cyndaquil[];
extern const u8 MtMoon_Cave_EventScript_Silver_Battle_Totodile[];
extern const u8 MtMoon_Cave_EventScript_Silver_Battle_Chikorita[];
extern const u8 MtMoon_Cave_EventScript_SilverRetreat[];
extern const u8 IndigoPlateau_Silver_Battle_Cyndaquil[];
extern const u8 IndigoPlateau_Silver_Battle_Totodile[];
extern const u8 IndigoPlateau_Silver_Battle_Chikorita[];
extern const u8 IndigoPlateau_EventScript_SilverRetreat[];
extern const u8 SlowpokeWell_B1F_EventScript_ProtonBattle[];
extern const u8 SlowpokeWell_B1F_EventScript_ProtonRetreat[];
extern const u8 EventScript_WayfarerStoryLossRetreat[];
extern const u8 RocketHideout_B3F_EventScript_GruntF5[];
extern const u8 RocketHideout_B3F_EventScript_Eto[];
extern const u8 RocketHideout_B3F_EventScript_GiovanniBattle[];
extern const u8 RocketHideout_B3F_EventScript_PetrelRetreat[];
extern const u8 GoldenrodCity_RadioTower_5F_EventScript_PetrelBattle[];
extern const u8 GoldenrodCity_RadioTower_5F_EventScript_PetrelRetreat[];
extern const u8 GoldenrodRaidoTower4_EventScript_Proton[];
extern const u8 GoldenrodCity_RadioTower_4F_EventScript_ProtonRetreat[];
extern const u8 GoldenrodRaidoTower4_EventScript_ARIANA[];
extern const u8 GoldenrodCity_RadioTower_5F_EventScript_ArianaRetreat[];
extern const u8 GoldenrodCity_RadioTower_5F_EventScript_ArcherBattle[];
extern const u8 GoldenrodCity_RadioTower_5F_EventScript_ArcherRetreat[];
extern const u8 EcruteakCity_Theater_EventScript_RocketBattle[];
extern const u8 EcruteakCity_Theater_EventScript_RocketRetreat[];
extern const u8 SproutTower_3F_EventScript_SageLi_Battle[];
extern const u8 SproutTower_3F_EventScript_SageLiRetreat[];
extern const u8 Route24_EventScript_GruntBattle[];
extern const u8 Route24_EventScript_GruntRetreat[];
extern const u8 SSAqua_RoomNW_EventScript_StanlyBattle[];
extern const u8 RadioTower1F_EventScript_GruntBattle[];
extern const u8 ViridianCity_Gym_EventScript_BlueBattle[];
extern const u8 TohjoFalls_EventScript_GiovanniBattle[];

static bool8 WayfarerSilverBurnedTowerIsPending(void)
{
    return FlagGet(FLAG_JOHTO_STARTER_CHOICE_COMMITTED)
        && !FlagGet(FLAG_WAYFARER_SILVER_BURNED_TOWER_COMPLETE);
}

static bool8 WayfarerSilverAzaleaIsPending(void)
{
    // Ilex and the GS Ball may advance this shared host state independently.
    // The Wayfarer chapter retires only when its own completion flag is set.
    return FlagGet(FLAG_JOHTO_STARTER_CHOICE_COMMITTED)
        && !FlagGet(FLAG_WAYFARER_SILVER_AZALEA_COMPLETE)
        && VarGet(VAR_AZALEA_TOWN_STATE) >= 5;
}

static bool8 WayfarerSilverGoldenrodIsPending(void)
{
    // The original coord scene is state 8 and the return actor is state >= 10.
    // Both are stages of this one pending chapter; their scripts/actor controls
    // select the applicable staging without changing the occupation state.
    return FlagGet(FLAG_JOHTO_STARTER_CHOICE_COMMITTED)
        && !FlagGet(FLAG_WAYFARER_SILVER_GOLDENROD_UNDERGROUND_COMPLETE)
        && VarGet(VAR_GOLDENROD_CITY_STATE) >= 8;
}

static bool8 WayfarerSilverGoldenrodReturnIsPending(void)
{
    return WayfarerSilverGoldenrodIsPending()
        && VarGet(VAR_GOLDENROD_CITY_STATE) >= 10;
}

static bool8 WayfarerSilverMtMoonIsPending(void)
{
    return FlagGet(FLAG_JOHTO_STARTER_CHOICE_COMMITTED)
        && !FlagGet(FLAG_HIDE_MTMOON_SILVER);
}

static bool8 WayfarerSilverIndigoIsPending(void)
{
    return FlagGet(FLAG_JOHTO_STARTER_CHOICE_COMMITTED)
        && !FlagGet(FLAG_HIDE_INDIGO_PLATEAU_SILVER)
        && !FlagGet(FLAG_DAILY_BEAT_SILVER);
}
