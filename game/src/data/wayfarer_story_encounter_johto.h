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

const struct WayfarerStoryEncounterDescriptor gWayfarerStoryJohtoDescriptors[] =
{
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 1001, .sceneId = WAYFARER_STORY_SCENE_SILVER_CHERRYGROVE, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_CHERRYGROVE_CITY_HNS), .mapNum = MAP_NUM(MAP_CHERRYGROVE_CITY_HNS), .localId = 0, .elevation = 0, .activationWidth = 1, .activationHeight = 2, .x = 56, .y = 10, .isNarrativelyEligible = NULL },
    { .lossRedirect = AzaleaTown_EventScript_SilverRetreat, .triggerScript = NULL, .stableKey = 1002, .sceneId = WAYFARER_STORY_SCENE_SILVER_AZALEA, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_AZALEA_TOWN_HNS), .mapNum = MAP_NUM(MAP_AZALEA_TOWN_HNS), .localId = 0, .elevation = 0, .activationWidth = 1, .activationHeight = 2, .x = 11, .y = 16, .isNarrativelyEligible = WayfarerSilverAzaleaIsPending },
    { .lossRedirect = BurnedTower_1F_EventScript_SilverRetreat, .triggerScript = NULL, .stableKey = 1003, .sceneId = WAYFARER_STORY_SCENE_SILVER_BURNED_TOWER, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, .mapGroup = MAP_GROUP(MAP_BURNED_TOWER_1F_HNS), .mapNum = MAP_NUM(MAP_BURNED_TOWER_1F_HNS), .localId = LOCALID_BURNED_TOWER_SILVER, .elevation = 0, .activationWidth = 0, .activationHeight = 0, .x = 18, .y = 12, .isNarrativelyEligible = WayfarerSilverBurnedTowerIsPending },
    { .lossRedirect = GoldenrodCity_UndergroundSwitches_EventScript_SilverRetreat, .triggerScript = NULL, .stableKey = 1004, .sceneId = WAYFARER_STORY_SCENE_SILVER_GOLDENROD_UNDERGROUND, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_GOLDENROD_CITY_UNDERGROUND_SWITCHES_HNS), .mapNum = MAP_NUM(MAP_GOLDENROD_CITY_UNDERGROUND_SWITCHES_HNS), .localId = 0, .elevation = 0, .activationWidth = 1, .activationHeight = 2, .x = 26, .y = 5, .isNarrativelyEligible = WayfarerSilverGoldenrodIsPending },
    { .lossRedirect = GoldenrodCity_UndergroundSwitches_EventScript_SilverRetreat, .triggerScript = NULL, .stableKey = 1004, .sceneId = WAYFARER_STORY_SCENE_SILVER_GOLDENROD_UNDERGROUND, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, .mapGroup = MAP_GROUP(MAP_GOLDENROD_CITY_UNDERGROUND_SWITCHES_HNS), .mapNum = MAP_NUM(MAP_GOLDENROD_CITY_UNDERGROUND_SWITCHES_HNS), .localId = LOCALID_GOLDENROD_UNDERGROUND_SILVER, .elevation = 0, .activationWidth = 0, .activationHeight = 0, .x = 35, .y = 2, .isNarrativelyEligible = WayfarerSilverGoldenrodReturnIsPending },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 1005, .sceneId = WAYFARER_STORY_SCENE_SILVER_VICTORY_ROAD, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_LEAGUE, .flags = WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_KANTO_1F_HNS), .mapNum = MAP_NUM(MAP_VICTORY_ROAD_KANTO_1F_HNS), .localId = 0, .elevation = 0, .activationWidth = 3, .activationHeight = 1, .x = 27, .y = 7, .isNarrativelyEligible = NULL },
    { .lossRedirect = MtMoon_Cave_EventScript_SilverRetreat, .triggerScript = NULL, .stableKey = 1006, .sceneId = WAYFARER_STORY_SCENE_SILVER_MT_MOON, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, .mapGroup = MAP_GROUP(MAP_MT_MOON_CAVE_HNS), .mapNum = MAP_NUM(MAP_MT_MOON_CAVE_HNS), .localId = LOCALID_MTMOON_SILVER, .elevation = 0, .activationWidth = 0, .activationHeight = 0, .x = 9, .y = 11, .isNarrativelyEligible = WayfarerSilverMtMoonIsPending },
    { .lossRedirect = IndigoPlateau_EventScript_SilverRetreat, .triggerScript = NULL, .stableKey = 1007, .sceneId = WAYFARER_STORY_SCENE_SILVER_INDIGO, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, .mapGroup = MAP_GROUP(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS), .mapNum = MAP_NUM(MAP_INDIGO_PLATEAU_POKEMON_CENTER_HNS), .localId = LOCALID_INDIGO_PLATEAU_SILVER, .elevation = 0, .activationWidth = 0, .activationHeight = 0, .x = 32, .y = 8, .isNarrativelyEligible = WayfarerSilverIndigoIsPending },
    { .lossRedirect = SlowpokeWell_B1F_EventScript_ProtonRetreat, .triggerScript = NULL, .stableKey = 1101, .sceneId = WAYFARER_STORY_SCENE_SLOWPOKE_WELL_PROTON, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_SLOWPOKE_WELL_B1F_HNS), .mapNum = MAP_NUM(MAP_SLOWPOKE_WELL_B1F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = EventScript_WayfarerStoryLossRetreat, .triggerScript = NULL, .stableKey = 1102, .sceneId = WAYFARER_STORY_SCENE_MAHOGANY_PASSWORD_GRUNT_F, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT, .mapGroup = MAP_GROUP(MAP_ROCKET_HIDEOUT_B3F_HNS), .mapNum = MAP_NUM(MAP_ROCKET_HIDEOUT_B3F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = EventScript_WayfarerStoryLossRetreat, .triggerScript = NULL, .stableKey = 1103, .sceneId = WAYFARER_STORY_SCENE_MAHOGANY_PASSWORD_GRUNT_M, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT, .mapGroup = MAP_GROUP(MAP_ROCKET_HIDEOUT_B3F_HNS), .mapNum = MAP_NUM(MAP_ROCKET_HIDEOUT_B3F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = RocketHideout_B3F_EventScript_PetrelRetreat, .triggerScript = NULL, .stableKey = 1104, .sceneId = WAYFARER_STORY_SCENE_MAHOGANY_PETREL, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_DISGUISED_DIRECTOR, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_ROCKET_HIDEOUT_B3F_HNS), .mapNum = MAP_NUM(MAP_ROCKET_HIDEOUT_B3F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = GoldenrodCity_RadioTower_5F_EventScript_PetrelRetreat, .triggerScript = NULL, .stableKey = 1105, .sceneId = WAYFARER_STORY_SCENE_RADIO_FAKE_DIRECTOR_PETREL, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_DISGUISED_DIRECTOR, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_GOLDENROD_CITY_RADIO_TOWER_5F_HNS), .mapNum = MAP_NUM(MAP_GOLDENROD_CITY_RADIO_TOWER_5F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = GoldenrodCity_RadioTower_4F_EventScript_ProtonRetreat, .triggerScript = NULL, .stableKey = 1106, .sceneId = WAYFARER_STORY_SCENE_RADIO_EXECUTIVE_PROTON, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT, .mapGroup = MAP_GROUP(MAP_GOLDENROD_CITY_RADIO_TOWER_4F_HNS), .mapNum = MAP_NUM(MAP_GOLDENROD_CITY_RADIO_TOWER_4F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = GoldenrodCity_RadioTower_5F_EventScript_ArianaRetreat, .triggerScript = NULL, .stableKey = 1107, .sceneId = WAYFARER_STORY_SCENE_RADIO_EXECUTIVE_ARIANA, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_ALLOW_POST_BATTLE_TEXT, .mapGroup = MAP_GROUP(MAP_GOLDENROD_CITY_RADIO_TOWER_5F_HNS), .mapNum = MAP_NUM(MAP_GOLDENROD_CITY_RADIO_TOWER_5F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = GoldenrodCity_RadioTower_5F_EventScript_ArcherRetreat, .triggerScript = NULL, .stableKey = 1108, .sceneId = WAYFARER_STORY_SCENE_RADIO_ARCHER, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_GOLDENROD_CITY_RADIO_TOWER_5F_HNS), .mapNum = MAP_NUM(MAP_GOLDENROD_CITY_RADIO_TOWER_5F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = EcruteakCity_Theater_EventScript_RocketRetreat, .triggerScript = NULL, .stableKey = 1109, .sceneId = WAYFARER_STORY_SCENE_THEATER_ROCKET, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_ECRUTEAK_CITY_THEATER_HNS), .mapNum = MAP_NUM(MAP_ECRUTEAK_CITY_THEATER_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = SproutTower_3F_EventScript_SageLiRetreat, .triggerScript = NULL, .stableKey = 1110, .sceneId = WAYFARER_STORY_SCENE_SPROUT_ELDER_LI, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_TRIAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_SPROUT_TOWER_3F_HNS), .mapNum = MAP_NUM(MAP_SPROUT_TOWER_3F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = Route24_EventScript_GruntRetreat, .triggerScript = NULL, .stableKey = 1111, .sceneId = WAYFARER_STORY_SCENE_ROUTE24_ROCKET, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_ROUTE24_HNS), .mapNum = MAP_NUM(MAP_ROUTE24_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = EventScript_WayfarerStoryLossRetreat, .triggerScript = NULL, .stableKey = 28196, .sceneId = WAYFARER_STORY_SCENE_SSAQUA_STANLY, .policy = WAYFARER_STORY_POLICY_ORDINARY, .dialogue = WAYFARER_STORY_DIALOGUE_ORDINARY, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_SSAQUA_ROOM_NW_HNS), .mapNum = MAP_NUM(MAP_SSAQUA_ROOM_NW_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = EventScript_WayfarerStoryLossRetreat, .triggerScript = NULL, .stableKey = 52962, .sceneId = WAYFARER_STORY_SCENE_RADIO_1F_GRUNT, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_GOLDENROD_CITY_RADIO_TOWER_1F_HNS), .mapNum = MAP_NUM(MAP_GOLDENROD_CITY_RADIO_TOWER_1F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 1201, .sceneId = WAYFARER_STORY_SCENE_VIRIDIAN_BLUE, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_GYM, .flags = 0, .mapGroup = MAP_GROUP(MAP_VIRIDIAN_CITY_GYM_HNS), .mapNum = MAP_NUM(MAP_VIRIDIAN_CITY_GYM_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 1202, .sceneId = WAYFARER_STORY_SCENE_TOHJO_GIOVANNI_EXCEPTION, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = 0, .mapGroup = MAP_GROUP(MAP_TOHJO_FALLS_GIOVANNI_ROOM_HNS), .mapNum = MAP_NUM(MAP_TOHJO_FALLS_GIOVANNI_ROOM_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_KIMONO_TRIAL, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_TRIAL, .flags = 0, .mapGroup = MAP_GROUP(MAP_ECRUTEAK_CITY_THEATER_HNS), .mapNum = MAP_NUM(MAP_ECRUTEAK_CITY_THEATER_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_MAHOGANY_CONFRONTATION, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_ROCKET_GUARD, .flags = 0, .mapGroup = MAP_GROUP(MAP_ROCKET_HIDEOUT_B2F_HNS), .mapNum = MAP_NUM(MAP_ROCKET_HIDEOUT_B2F_HNS), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = NULL },
};
const u32 gWayfarerStoryJohtoDescriptorCount = ARRAY_COUNT(gWayfarerStoryJohtoDescriptors);
const u8 *const gWayfarerStoryJohtoCallers[] =
{
    CherryGroveCity_Silver_Battle_Cyndaquil + 1,
    CherryGroveCity_Silver_Battle_Totodile + 1,
    CherryGroveCity_Silver_Battle_Chikorita + 1,
    AzaleaTown_Silver_Battle_Cyndaquil + 1,
    AzaleaTown_Silver_Battle_Totodile + 1,
    AzaleaTown_Silver_Battle_Chikorita + 1,
    BurnedTower_1F_Silver_Battle_Cyndaquil + 1,
    BurnedTower_1F_Silver_Battle_Totodile + 1,
    BurnedTower_1F_Silver_Battle_Chikorita + 1,
    GoldenrodCity_UndergroundSwitches_EventScript_Cyndaquil + 1,
    GoldenrodCity_UndergroundSwitches_EventScript_Totodile + 1,
    GoldenrodCity_UndergroundSwitches_EventScript_Chikorita + 1,
    GoldenrodCity_UndergroundSwitches_EventScript_SilverReturnCyndaquil + 1,
    GoldenrodCity_UndergroundSwitches_EventScript_SilverReturnTotodile + 1,
    GoldenrodCity_UndergroundSwitches_EventScript_SilverReturnChikorita + 1,
    VictoryRoadKanto_1F_Silver_Battle_Cyndaquil + 1,
    VictoryRoadKanto_1F_Silver_Battle_Totodile + 1,
    VictoryRoadKanto_1F_Silver_Battle_Chikorita + 1,
    MtMoon_Cave_EventScript_Silver_Battle_Cyndaquil + 1,
    MtMoon_Cave_EventScript_Silver_Battle_Totodile + 1,
    MtMoon_Cave_EventScript_Silver_Battle_Chikorita + 1,
    IndigoPlateau_Silver_Battle_Cyndaquil + 1,
    IndigoPlateau_Silver_Battle_Totodile + 1,
    IndigoPlateau_Silver_Battle_Chikorita + 1,
    SlowpokeWell_B1F_EventScript_ProtonBattle + 1,
    RocketHideout_B3F_EventScript_GruntF5 + 1,
    RocketHideout_B3F_EventScript_Eto + 1,
    RocketHideout_B3F_EventScript_GiovanniBattle + 1,
    GoldenrodCity_RadioTower_5F_EventScript_PetrelBattle + 1,
    GoldenrodRaidoTower4_EventScript_Proton + 1,
    GoldenrodRaidoTower4_EventScript_ARIANA + 1,
    GoldenrodCity_RadioTower_5F_EventScript_ArcherBattle + 1,
    EcruteakCity_Theater_EventScript_RocketBattle + 1,
    SproutTower_3F_EventScript_SageLi_Battle + 1,
    Route24_EventScript_GruntBattle + 1,
    SSAqua_RoomNW_EventScript_StanlyBattle + 1,
    RadioTower1F_EventScript_GruntBattle + 1,
    ViridianCity_Gym_EventScript_BlueBattle + 1,
    TohjoFalls_EventScript_GiovanniBattle + 1,
    NULL,
    NULL,
};
const u8 gWayfarerStoryJohtoDescriptorIndices[] =
{
    0,
    0,
    0,
    1,
    1,
    1,
    2,
    2,
    2,
    3,
    3,
    3,
    4,
    4,
    4,
    5,
    5,
    5,
    6,
    6,
    6,
    7,
    7,
    7,
    8,
    9,
    10,
    11,
    12,
    13,
    14,
    15,
    16,
    17,
    18,
    19,
    20,
    21,
    22,
    23,
    24,
};
const u32 gWayfarerStoryJohtoEncounterCount = ARRAY_COUNT(gWayfarerStoryJohtoCallers);
#else
const struct WayfarerStoryEncounterDescriptor gWayfarerStoryJohtoDescriptors[] = {0};
const u32 gWayfarerStoryJohtoDescriptorCount = 0;
const u8 *const gWayfarerStoryJohtoCallers[] = {NULL};
const u8 gWayfarerStoryJohtoDescriptorIndices[] = {0};
const u32 gWayfarerStoryJohtoEncounterCount = 0;
#endif
