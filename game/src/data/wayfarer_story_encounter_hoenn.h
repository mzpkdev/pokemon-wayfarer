// Hoenn's registry covers only current Emerald maps.  The raw local IDs below are
// map-event ordinals (mapjson emits object index + 1); the companion static audit
// validates them against the source maps so a reordered map cannot silently hide
// the wrong actor.
#if IS_WAYFARER

#define WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL 28
#define WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL_ON_BIKE 29
#define WAYFARER_HOENN_LOCALID_ROUTE119_RIVAL 16
#define WAYFARER_HOENN_LOCALID_ROUTE119_RIVAL_ON_BIKE 25
#define WAYFARER_HOENN_LOCALID_LILYCOVE_RIVAL 17
#define WAYFARER_HOENN_LOCALID_MAUVILLE_WALLY 6
#define WAYFARER_HOENN_LOCALID_VICTORY_ROAD_EXIT_WALLY 7

// These are Emerald's source-state IDs after its namespace translation. The
// Wayfarer executable otherwise sees the HNS definitions with the same names;
// use local constants so Hoenn story retirement reads the state authored by the
// port without redefining HNS symbols for this translation unit. Keep these in
// sync with data/wayfarer_hoenn_source_constants.inc.
enum
{
    WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_RIVAL_ROUTE103 = 0x6082,
    WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_RIVAL_RUSTBORO = 0x60D3,
    WAYFARER_HOENN_SOURCE_FLAG_MET_RIVAL_LILYCOVE = 0x6124,
    WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_WALLY_MAUVILLE = 0x60BE,
    WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_WALLY_VICTORY_ROAD = 0x607E,
    WAYFARER_HOENN_SOURCE_FLAG_SYS_GAME_CLEAR = 0x6864,
    WAYFARER_HOENN_SOURCE_FLAG_DELIVERED_DEVON_GOODS = 0x6095,
    WAYFARER_HOENN_SOURCE_FLAG_RECOVERED_DEVON_GOODS = 0x608F,
    WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY = 0x608B,
    WAYFARER_HOENN_SOURCE_FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT = 0x606F,
    WAYFARER_HOENN_SOURCE_FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE = 0x6070,
    WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_GRUNT_SPACE_CENTER_1F = 0x60BF,
    WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_MAGMA_SPACE_CENTER = 0x6075,
    WAYFARER_HOENN_SOURCE_FLAG_RECEIVED_DEVON_SCOPE = 0x611D,
    WAYFARER_HOENN_SOURCE_VAR_ROUTE110_STATE = 0x7069,
    WAYFARER_HOENN_SOURCE_VAR_ROUTE119_STATE = 0x7072,
    WAYFARER_HOENN_SOURCE_VAR_WEATHER_INSTITUTE_STATE = 0x70B3,
    WAYFARER_HOENN_SOURCE_VAR_MOSSDEEP_CITY_STATE = 0x705D,
    WAYFARER_HOENN_SOURCE_VAR_MOSSDEEP_SPACE_CENTER_STATE = 0x709F,
    WAYFARER_HOENN_SOURCE_VAR_SEAFLOOR_CAVERN_STATE = 0x70A2,
};

extern const u8 EventScript_WayfarerStoryLossRetreat[];

extern const u8 Route103_EventScript_Rival[];
extern const u8 Route103_EventScript_StartMayBattleTreecko[];
extern const u8 Route103_EventScript_StartMayBattleTorchic[];
extern const u8 Route103_EventScript_StartMayBattleMudkip[];
extern const u8 Route103_EventScript_StartBrendanBattleTreecko[];
extern const u8 Route103_EventScript_StartBrendanBattleTorchic[];
extern const u8 Route103_EventScript_StartBrendanBattleMudkip[];

extern const u8 RustboroCity_EventScript_RivalTrigger0[];
extern const u8 RustboroCity_EventScript_BattleMayTreecko[];
extern const u8 RustboroCity_EventScript_BattleMayTorchic[];
extern const u8 RustboroCity_EventScript_BattleMayMudkip[];
extern const u8 RustboroCity_EventScript_BattleBrendanTreecko[];
extern const u8 RustboroCity_EventScript_BattleBrendanTorchic[];
extern const u8 RustboroCity_EventScript_BattleBrendanMudkip[];
extern const u8 RustboroCity_EventScript_WayfarerRivalLossRetreat[];

extern const u8 Route110_EventScript_RivalScene[];
extern const u8 Route110_EventScript_MayBattleTreecko[];
extern const u8 Route110_EventScript_MayBattleTorchic[];
extern const u8 Route110_EventScript_MayBattleMudkip[];
extern const u8 Route110_EventScript_BrendanBattleTreecko[];
extern const u8 Route110_EventScript_BrendanBattleTorchic[];
extern const u8 Route110_EventScript_BrendanBattleMudkip[];
extern const u8 Route110_EventScript_WayfarerRivalLossRetreat[];

extern const u8 Route119_EventScript_RivalEncounter[];
extern const u8 Route119_EventScript_BattleMayTreecko[];
extern const u8 Route119_EventScript_BattleMayTorchic[];
extern const u8 Route119_EventScript_BattleMayMudkip[];
extern const u8 Route119_EventScript_BattleBrendanTreecko[];
extern const u8 Route119_EventScript_BattleBrendanTorchic[];
extern const u8 Route119_EventScript_BattleBrendanMudkip[];
extern const u8 Route119_EventScript_WayfarerRivalLossRetreat[];

extern const u8 LilycoveCity_EventScript_BattleMayTreecko[];
extern const u8 LilycoveCity_EventScript_BattleMayTorchic[];
extern const u8 LilycoveCity_EventScript_BattleMayMudkip[];
extern const u8 LilycoveCity_EventScript_BattleBrendanTreecko[];
extern const u8 LilycoveCity_EventScript_BattleBrendanTorchic[];
extern const u8 LilycoveCity_EventScript_BattleBrendanMudkip[];
extern const u8 LilycoveCity_EventScript_WayfarerRivalLossRetreat[];

extern const u8 MauvilleCity_EventScript_BattleWallyTrainerBattle[];
extern const u8 MauvilleCity_EventScript_WayfarerWallyLossRetreat[];

extern const u8 VictoryRoad_1F_EventScript_WallyBattleTrigger1[];
extern const u8 VictoryRoad_1F_EventScript_WallyEntranceTrainerBattle[];
extern const u8 VictoryRoad_1F_EventScript_ExitWally[];
extern const u8 VictoryRoad_1F_EventScript_RematchWally[];

extern const u8 PetalburgCity_EventScript_WallyTutorial[];

extern const u8 SlateportCity_OceanicMuseum_2F_EventScript_CaptStern[];
extern const u8 RusturfTunnel_EventScript_GruntTrainerBattle[];
extern const u8 RusturfTunnel_EventScript_WayfarerGruntLossRetreat[];
extern const u8 MtChimney_EventScript_Maxie[];
extern const u8 MtChimney_EventScript_MaxieTrainerBattle[];
extern const u8 MtChimney_EventScript_WayfarerMaxieLossRetreat[];
extern const u8 MagmaHideout_4F_EventScript_Maxie[];
extern const u8 MagmaHideout_4F_EventScript_MaxieTrainerBattle[];
extern const u8 MagmaHideout_4F_EventScript_WayfarerMaxieLossRetreat[];
extern const u8 AquaHideout_B2F_EventScript_Matt[];
extern const u8 Route119_WeatherInstitute_2F_EventScript_Shelly[];
extern const u8 MossdeepCity_SpaceCenter_1F_EventScript_Grunt2TrainerBattle[];
extern const u8 MossdeepCity_SpaceCenter_2F_EventScript_ThreeMagmaGrunts[];
extern const u8 MossdeepCity_SpaceCenter_2F_EventScript_Steven[];
extern const u8 SeafloorCavern_Room9_EventScript_ArchieAwakenKyogre[];
extern const u8 SeafloorCavern_Room9_EventScript_ArchieTrainerBattle[];
extern const u8 SeafloorCavern_Room9_EventScript_WayfarerArchieLossRetreat[];
extern const u8 Route120_EventScript_Steven[];

static bool8 WayfarerHoennRoute103Pending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_RIVAL_ROUTE103);
}

static bool8 WayfarerHoennRustboroRivalPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_RIVAL_RUSTBORO);
}

static bool8 WayfarerHoennRoute110RivalPending(void)
{
    return VarGet(WAYFARER_HOENN_SOURCE_VAR_ROUTE110_STATE) == 0;
}

static bool8 WayfarerHoennRoute119RivalPending(void)
{
    return VarGet(WAYFARER_HOENN_SOURCE_VAR_ROUTE119_STATE) == 0;
}

static bool8 WayfarerHoennLilycoveRivalPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_MET_RIVAL_LILYCOVE);
}

static bool8 WayfarerHoennMauvilleWallyPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_WALLY_MAUVILLE);
}

static bool8 WayfarerHoennVictoryRoadWallyPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_WALLY_VICTORY_ROAD);
}

static bool8 WayfarerHoennVictoryRoadWallyRematchEligible(void)
{
    return FlagGet(WAYFARER_HOENN_SOURCE_FLAG_SYS_GAME_CLEAR);
}

static bool8 WayfarerHoennMuseumPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_DELIVERED_DEVON_GOODS);
}

static bool8 WayfarerHoennRusturfRescuePending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_RECOVERED_DEVON_GOODS);
}

static bool8 WayfarerHoennMtChimneyPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY);
}

static bool8 WayfarerHoennMagmaHideoutPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT);
}

static bool8 WayfarerHoennAquaHideoutPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE);
}

static bool8 WayfarerHoennWeatherInstitutePending(void)
{
    return VarGet(WAYFARER_HOENN_SOURCE_VAR_WEATHER_INSTITUTE_STATE) == 0;
}

static bool8 WayfarerHoennSpaceCenterGruntsPending(void)
{
    return VarGet(WAYFARER_HOENN_SOURCE_VAR_MOSSDEEP_SPACE_CENTER_STATE) == 1;
}

static bool8 WayfarerHoennSpaceCenterStairGuardPending(void)
{
    return VarGet(WAYFARER_HOENN_SOURCE_VAR_MOSSDEEP_CITY_STATE) == 2
        && !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_GRUNT_SPACE_CENTER_1F);
}

static bool8 WayfarerHoennSpaceCenterPartnerPending(void)
{
    return VarGet(WAYFARER_HOENN_SOURCE_VAR_MOSSDEEP_SPACE_CENTER_STATE) == 2
        && !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_DEFEATED_MAGMA_SPACE_CENTER);
}

static bool8 WayfarerHoennSeafloorPending(void)
{
    return VarGet(WAYFARER_HOENN_SOURCE_VAR_SEAFLOOR_CAVERN_STATE) == 0;
}

static bool8 WayfarerHoennKecleonInvestigationPending(void)
{
    return !FlagGet(WAYFARER_HOENN_SOURCE_FLAG_RECEIVED_DEVON_SCOPE);
}

const struct WayfarerStoryEncounterDescriptor gWayfarerStoryHoennDescriptors[] =
{
    { .lossRedirect = NULL, .triggerScript = Route103_EventScript_Rival, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_ORDINARY, .flags = 0, .mapGroup = MAP_GROUP(MAP_ROUTE103), .mapNum = MAP_NUM(MAP_ROUTE103), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRoute103Pending },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_ORDINARY, .flags = 0, .mapGroup = MAP_GROUP(MAP_ROUTE103), .mapNum = MAP_NUM(MAP_ROUTE103), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRoute103Pending },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 1, .sceneId = WAYFARER_STORY_SCENE_RUSTBORO_RIVAL_CONVERSATION, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = 0, .mapGroup = MAP_GROUP(MAP_RUSTBORO_CITY), .mapNum = MAP_NUM(MAP_RUSTBORO_CITY), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRustboroRivalPending },
    { .lossRedirect = RustboroCity_EventScript_WayfarerRivalLossRetreat, .triggerScript = RustboroCity_EventScript_RivalTrigger0, .stableKey = 1, .sceneId = WAYFARER_STORY_SCENE_RUSTBORO_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_RUSTBORO_CITY), .mapNum = MAP_NUM(MAP_RUSTBORO_CITY), .localId = 0, .elevation = 3, .activationWidth = 8, .activationHeight = 1, .x = 12, .y = 53, .isNarrativelyEligible = WayfarerHoennRustboroRivalPending },
    { .lossRedirect = RustboroCity_EventScript_WayfarerRivalLossRetreat, .triggerScript = NULL, .stableKey = 1, .sceneId = WAYFARER_STORY_SCENE_RUSTBORO_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_RUSTBORO_CITY), .mapNum = MAP_NUM(MAP_RUSTBORO_CITY), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRustboroRivalPending },
    { .lossRedirect = Route110_EventScript_WayfarerRivalLossRetreat, .triggerScript = Route110_EventScript_RivalScene, .stableKey = 2, .sceneId = WAYFARER_STORY_SCENE_ROUTE110_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_ROUTE110), .mapNum = MAP_NUM(MAP_ROUTE110), .localId = WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL, .elevation = 3, .activationWidth = 3, .activationHeight = 1, .x = 33, .y = 56, .isNarrativelyEligible = WayfarerHoennRoute110RivalPending },
    { .lossRedirect = Route110_EventScript_WayfarerRivalLossRetreat, .triggerScript = NULL, .stableKey = 2, .sceneId = WAYFARER_STORY_SCENE_ROUTE110_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_ROUTE110), .mapNum = MAP_NUM(MAP_ROUTE110), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRoute110RivalPending },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 2, .sceneId = WAYFARER_STORY_SCENE_ROUTE110_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE, .mapGroup = MAP_GROUP(MAP_ROUTE110), .mapNum = MAP_NUM(MAP_ROUTE110), .localId = WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL_ON_BIKE, .elevation = 3, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRoute110RivalPending },
    { .lossRedirect = Route119_EventScript_WayfarerRivalLossRetreat, .triggerScript = Route119_EventScript_RivalEncounter, .stableKey = 3, .sceneId = WAYFARER_STORY_SCENE_ROUTE119_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_ROUTE119), .mapNum = MAP_NUM(MAP_ROUTE119), .localId = WAYFARER_HOENN_LOCALID_ROUTE119_RIVAL, .elevation = 0, .activationWidth = 2, .activationHeight = 1, .x = 25, .y = 31, .isNarrativelyEligible = WayfarerHoennRoute119RivalPending },
    { .lossRedirect = Route119_EventScript_WayfarerRivalLossRetreat, .triggerScript = NULL, .stableKey = 3, .sceneId = WAYFARER_STORY_SCENE_ROUTE119_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_ROUTE119), .mapNum = MAP_NUM(MAP_ROUTE119), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRoute119RivalPending },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 3, .sceneId = WAYFARER_STORY_SCENE_ROUTE119_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE, .mapGroup = MAP_GROUP(MAP_ROUTE119), .mapNum = MAP_NUM(MAP_ROUTE119), .localId = WAYFARER_HOENN_LOCALID_ROUTE119_RIVAL_ON_BIKE, .elevation = 4, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRoute119RivalPending },
    { .lossRedirect = LilycoveCity_EventScript_WayfarerRivalLossRetreat, .triggerScript = NULL, .stableKey = 4, .sceneId = WAYFARER_STORY_SCENE_LILYCOVE_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, .mapGroup = MAP_GROUP(MAP_LILYCOVE_CITY), .mapNum = MAP_NUM(MAP_LILYCOVE_CITY), .localId = WAYFARER_HOENN_LOCALID_LILYCOVE_RIVAL, .elevation = 5, .activationWidth = 0, .activationHeight = 0, .x = 27, .y = 7, .isNarrativelyEligible = WayfarerHoennLilycoveRivalPending },
    { .lossRedirect = LilycoveCity_EventScript_WayfarerRivalLossRetreat, .triggerScript = NULL, .stableKey = 4, .sceneId = WAYFARER_STORY_SCENE_LILYCOVE_RIVAL, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_LILYCOVE_CITY), .mapNum = MAP_NUM(MAP_LILYCOVE_CITY), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennLilycoveRivalPending },
    { .lossRedirect = MauvilleCity_EventScript_WayfarerWallyLossRetreat, .triggerScript = NULL, .stableKey = 5, .sceneId = WAYFARER_STORY_SCENE_MAUVILLE_WALLY, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, .mapGroup = MAP_GROUP(MAP_MAUVILLE_CITY), .mapNum = MAP_NUM(MAP_MAUVILLE_CITY), .localId = WAYFARER_HOENN_LOCALID_MAUVILLE_WALLY, .elevation = 3, .activationWidth = 0, .activationHeight = 0, .x = 8, .y = 6, .isNarrativelyEligible = WayfarerHoennMauvilleWallyPending },
    { .lossRedirect = NULL, .triggerScript = VictoryRoad_1F_EventScript_WallyBattleTrigger1, .stableKey = 6, .sceneId = WAYFARER_STORY_SCENE_VICTORY_ROAD_WALLY, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_1F), .mapNum = MAP_NUM(MAP_VICTORY_ROAD_1F), .localId = 0, .elevation = 4, .activationWidth = 2, .activationHeight = 1, .x = 2, .y = 23, .isNarrativelyEligible = WayfarerHoennVictoryRoadWallyPending },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 6, .sceneId = WAYFARER_STORY_SCENE_VICTORY_ROAD_WALLY, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_1F), .mapNum = MAP_NUM(MAP_VICTORY_ROAD_1F), .localId = WAYFARER_HOENN_LOCALID_VICTORY_ROAD_EXIT_WALLY, .elevation = 3, .activationWidth = 0, .activationHeight = 0, .x = 31, .y = 9, .isNarrativelyEligible = WayfarerHoennVictoryRoadWallyRematchEligible },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 6, .sceneId = WAYFARER_STORY_SCENE_VICTORY_ROAD_WALLY, .policy = WAYFARER_STORY_POLICY_DEFERRED_RIVAL, .dialogue = WAYFARER_STORY_DIALOGUE_RIVAL, .flags = 0, .mapGroup = MAP_GROUP(MAP_VICTORY_ROAD_1F), .mapNum = MAP_NUM(MAP_VICTORY_ROAD_1F), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennVictoryRoadWallyRematchEligible },
    { .lossRedirect = NULL, .triggerScript = PetalburgCity_EventScript_WallyTutorial, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_PETALBURG_WALLY_TUTORIAL, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_ORDINARY, .flags = WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_PETALBURG_CITY), .mapNum = MAP_NUM(MAP_PETALBURG_CITY), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 30, .activationHeight = 30, .x = 0, .y = 0, .isNarrativelyEligible = NULL },
    { .lossRedirect = NULL, .triggerScript = SlateportCity_OceanicMuseum_2F_EventScript_CaptStern, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_OCEANIC_MUSEUM_STERN, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_STERN, .flags = 0, .mapGroup = MAP_GROUP(MAP_SLATEPORT_CITY_OCEANIC_MUSEUM_2F), .mapNum = MAP_NUM(MAP_SLATEPORT_CITY_OCEANIC_MUSEUM_2F), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennMuseumPending },
    { .lossRedirect = RusturfTunnel_EventScript_WayfarerGruntLossRetreat, .triggerScript = NULL, .stableKey = 20, .sceneId = WAYFARER_STORY_SCENE_RUSTURF_AQUA, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_AQUA_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_RUSTURF_TUNNEL), .mapNum = MAP_NUM(MAP_RUSTURF_TUNNEL), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennRusturfRescuePending },
    { .lossRedirect = MtChimney_EventScript_WayfarerMaxieLossRetreat, .triggerScript = MtChimney_EventScript_Maxie, .stableKey = 21, .sceneId = WAYFARER_STORY_SCENE_MT_CHIMNEY_MAXIE, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_MAGMA_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_MT_CHIMNEY), .mapNum = MAP_NUM(MAP_MT_CHIMNEY), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennMtChimneyPending },
    { .lossRedirect = MagmaHideout_4F_EventScript_WayfarerMaxieLossRetreat, .triggerScript = MagmaHideout_4F_EventScript_Maxie, .stableKey = 22, .sceneId = WAYFARER_STORY_SCENE_MAGMA_HIDEOUT_MAXIE, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_MAGMA_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_MAGMA_HIDEOUT_4F), .mapNum = MAP_NUM(MAP_MAGMA_HIDEOUT_4F), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennMagmaHideoutPending },
    { .lossRedirect = EventScript_WayfarerStoryLossRetreat, .triggerScript = NULL, .stableKey = 23, .sceneId = WAYFARER_STORY_SCENE_AQUA_HIDEOUT_MATT, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_AQUA_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_AQUA_HIDEOUT_B2F), .mapNum = MAP_NUM(MAP_AQUA_HIDEOUT_B2F), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennAquaHideoutPending },
    { .lossRedirect = EventScript_WayfarerStoryLossRetreat, .triggerScript = NULL, .stableKey = 24, .sceneId = WAYFARER_STORY_SCENE_WEATHER_INSTITUTE_SHELLY, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_AQUA_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN, .mapGroup = MAP_GROUP(MAP_ROUTE119_WEATHER_INSTITUTE_2F), .mapNum = MAP_NUM(MAP_ROUTE119_WEATHER_INSTITUTE_2F), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennWeatherInstitutePending },
    { .lossRedirect = NULL, .triggerScript = NULL, .stableKey = 25, .sceneId = WAYFARER_STORY_SCENE_SPACE_CENTER_STAIR_GUARD, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_MAGMA_GUARD, .flags = 0, .mapGroup = MAP_GROUP(MAP_MOSSDEEP_CITY_SPACE_CENTER_1F), .mapNum = MAP_NUM(MAP_MOSSDEEP_CITY_SPACE_CENTER_1F), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 0, .activationHeight = 0, .x = WAYFARER_STORY_NO_COORD, .y = WAYFARER_STORY_NO_COORD, .isNarrativelyEligible = WayfarerHoennSpaceCenterStairGuardPending },
    { .lossRedirect = NULL, .triggerScript = MossdeepCity_SpaceCenter_2F_EventScript_ThreeMagmaGrunts, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_SPACE_CENTER_GRUNTS, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_MAGMA_GUARD, .flags = WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_MOSSDEEP_CITY_SPACE_CENTER_2F), .mapNum = MAP_NUM(MAP_MOSSDEEP_CITY_SPACE_CENTER_2F), .localId = 0, .elevation = WAYFARER_STORY_ANY_ELEVATION, .activationWidth = 16, .activationHeight = 10, .x = 0, .y = 0, .isNarrativelyEligible = WayfarerHoennSpaceCenterGruntsPending },
    { .lossRedirect = NULL, .triggerScript = MossdeepCity_SpaceCenter_2F_EventScript_Steven, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_SPACE_CENTER_OFFER, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_PARTNER, .flags = WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_MOSSDEEP_CITY_SPACE_CENTER_2F), .mapNum = MAP_NUM(MAP_MOSSDEEP_CITY_SPACE_CENTER_2F), .localId = 0, .elevation = 3, .activationWidth = 3, .activationHeight = 3, .x = 0, .y = 7, .isNarrativelyEligible = WayfarerHoennSpaceCenterPartnerPending },
    { .lossRedirect = SeafloorCavern_Room9_EventScript_WayfarerArchieLossRetreat, .triggerScript = SeafloorCavern_Room9_EventScript_ArchieAwakenKyogre, .stableKey = 30, .sceneId = WAYFARER_STORY_SCENE_SEAFLOOR_ARCHIE, .policy = WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, .dialogue = WAYFARER_STORY_DIALOGUE_AQUA_GUARD, .flags = WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_SEAFLOOR_CAVERN_ROOM9), .mapNum = MAP_NUM(MAP_SEAFLOOR_CAVERN_ROOM9), .localId = 0, .elevation = 3, .activationWidth = 1, .activationHeight = 1, .x = 17, .y = 42, .isNarrativelyEligible = WayfarerHoennSeafloorPending },
    { .lossRedirect = NULL, .triggerScript = Route120_EventScript_Steven, .stableKey = 0, .sceneId = WAYFARER_STORY_SCENE_ROUTE120_KECLEON, .policy = WAYFARER_STORY_POLICY_EXCEPTION, .dialogue = WAYFARER_STORY_DIALOGUE_INVESTIGATE, .flags = WAYFARER_STORY_FLAG_REARM_ON_LEAVE, .mapGroup = MAP_GROUP(MAP_ROUTE120), .mapNum = MAP_NUM(MAP_ROUTE120), .localId = 0, .elevation = 4, .activationWidth = 4, .activationHeight = 4, .x = 11, .y = 14, .isNarrativelyEligible = WayfarerHoennKecleonInvestigationPending },
};
const u32 gWayfarerStoryHoennDescriptorCount = ARRAY_COUNT(gWayfarerStoryHoennDescriptors);
const u8 *const gWayfarerStoryHoennCallers[] =
{
    Route103_EventScript_StartMayBattleTreecko + 1,
    Route103_EventScript_StartMayBattleTorchic + 1,
    Route103_EventScript_StartMayBattleMudkip + 1,
    Route103_EventScript_StartBrendanBattleTreecko + 1,
    Route103_EventScript_StartBrendanBattleTorchic + 1,
    Route103_EventScript_StartBrendanBattleMudkip + 1,
    NULL,
    RustboroCity_EventScript_BattleMayTreecko + 1,
    RustboroCity_EventScript_BattleMayTorchic + 1,
    RustboroCity_EventScript_BattleMayMudkip + 1,
    RustboroCity_EventScript_BattleBrendanTreecko + 1,
    RustboroCity_EventScript_BattleBrendanTorchic + 1,
    RustboroCity_EventScript_BattleBrendanMudkip + 1,
    Route110_EventScript_MayBattleTreecko + 1,
    Route110_EventScript_MayBattleTorchic + 1,
    Route110_EventScript_MayBattleMudkip + 1,
    Route110_EventScript_BrendanBattleTreecko + 1,
    Route110_EventScript_BrendanBattleTorchic + 1,
    Route110_EventScript_BrendanBattleMudkip + 1,
    NULL,
    Route119_EventScript_BattleMayTreecko + 1,
    Route119_EventScript_BattleMayTorchic + 1,
    Route119_EventScript_BattleMayMudkip + 1,
    Route119_EventScript_BattleBrendanTreecko + 1,
    Route119_EventScript_BattleBrendanTorchic + 1,
    Route119_EventScript_BattleBrendanMudkip + 1,
    NULL,
    LilycoveCity_EventScript_BattleMayTreecko + 1,
    LilycoveCity_EventScript_BattleMayTorchic + 1,
    LilycoveCity_EventScript_BattleMayMudkip + 1,
    LilycoveCity_EventScript_BattleBrendanTreecko + 1,
    LilycoveCity_EventScript_BattleBrendanTorchic + 1,
    LilycoveCity_EventScript_BattleBrendanMudkip + 1,
    MauvilleCity_EventScript_BattleWallyTrainerBattle + 1,
    VictoryRoad_1F_EventScript_WallyEntranceTrainerBattle + 1,
    VictoryRoad_1F_EventScript_ExitWally + 1,
    VictoryRoad_1F_EventScript_RematchWally + 1,
    NULL,
    NULL,
    RusturfTunnel_EventScript_GruntTrainerBattle + 1,
    MtChimney_EventScript_MaxieTrainerBattle + 1,
    MagmaHideout_4F_EventScript_MaxieTrainerBattle + 1,
    AquaHideout_B2F_EventScript_Matt + 1,
    Route119_WeatherInstitute_2F_EventScript_Shelly + 1,
    MossdeepCity_SpaceCenter_1F_EventScript_Grunt2TrainerBattle + 1,
    NULL,
    NULL,
    SeafloorCavern_Room9_EventScript_ArchieTrainerBattle + 1,
    NULL,
};
const u8 gWayfarerStoryHoennDescriptorIndices[] =
{
    0,
    1,
    1,
    1,
    1,
    1,
    2,
    3,
    4,
    4,
    4,
    4,
    4,
    5,
    6,
    6,
    6,
    6,
    6,
    7,
    8,
    9,
    9,
    9,
    9,
    9,
    10,
    11,
    12,
    12,
    12,
    12,
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
    25,
    26,
    27,
    28,
};
const u32 gWayfarerStoryHoennEncounterCount = ARRAY_COUNT(gWayfarerStoryHoennCallers);
#else
const struct WayfarerStoryEncounterDescriptor gWayfarerStoryHoennDescriptors[] = {0};
const u32 gWayfarerStoryHoennDescriptorCount = 0;
const u8 *const gWayfarerStoryHoennCallers[] = {NULL};
const u8 gWayfarerStoryHoennDescriptorIndices[] = {0};
const u32 gWayfarerStoryHoennEncounterCount = 0;
#endif
