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

#define HOENN_ENTRY(_caller, _loss, _trigger, _key, _scene, _policy, _dialogue, _flags, _map, _local, _elevation, _width, _height, _x, _y, _eligible) \
    { \
        .caller = (_caller), .lossRedirect = (_loss), .triggerScript = (_trigger), \
        .stableKey = (_key), .sceneId = (_scene), .policy = (_policy), .dialogue = (_dialogue), .flags = (_flags), \
        .mapGroup = MAP_GROUP(_map), .mapNum = MAP_NUM(_map), .localId = (_local), .elevation = (_elevation), \
        .activationWidth = (_width), .activationHeight = (_height), .x = (_x), .y = (_y), .isNarrativelyEligible = (_eligible), \
    }

const struct WayfarerStoryEncounter gWayfarerStoryHoennEncounters[] =
{
    // Authored tutorial: refusal precedes lock/starter presentation, and retains
    // Emerald's existing loss route rather than granting a field-return battle.
    HOENN_ENTRY(Route103_EventScript_StartMayBattleTreecko + 1, NULL, Route103_EventScript_Rival, 0, WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_ORDINARY, 0, MAP_ROUTE103, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute103Pending),
    HOENN_ENTRY(Route103_EventScript_StartMayBattleTorchic + 1, NULL, NULL, 0, WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_ORDINARY, 0, MAP_ROUTE103, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute103Pending),
    HOENN_ENTRY(Route103_EventScript_StartMayBattleMudkip + 1, NULL, NULL, 0, WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_ORDINARY, 0, MAP_ROUTE103, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute103Pending),
    HOENN_ENTRY(Route103_EventScript_StartBrendanBattleTreecko + 1, NULL, NULL, 0, WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_ORDINARY, 0, MAP_ROUTE103, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute103Pending),
    HOENN_ENTRY(Route103_EventScript_StartBrendanBattleTorchic + 1, NULL, NULL, 0, WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_ORDINARY, 0, MAP_ROUTE103, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute103Pending),
    HOENN_ENTRY(Route103_EventScript_StartBrendanBattleMudkip + 1, NULL, NULL, 0, WAYFARER_STORY_SCENE_ROUTE103_TUTORIAL, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_ORDINARY, 0, MAP_ROUTE103, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute103Pending),

    // Rustboro's eligible Match Call conversation stays authored; only either explicit battle prompt refuses without a party.
    HOENN_ENTRY(NULL, NULL, NULL, 1, WAYFARER_STORY_SCENE_RUSTBORO_RIVAL_CONVERSATION, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_RIVAL, 0, MAP_RUSTBORO_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRustboroRivalPending),
    HOENN_ENTRY(RustboroCity_EventScript_BattleMayTreecko + 1, RustboroCity_EventScript_WayfarerRivalLossRetreat, RustboroCity_EventScript_RivalTrigger0, 1, WAYFARER_STORY_SCENE_RUSTBORO_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_RUSTBORO_CITY, 0, 3, 8, 1, 12, 53, WayfarerHoennRustboroRivalPending),
    HOENN_ENTRY(RustboroCity_EventScript_BattleMayTorchic + 1, RustboroCity_EventScript_WayfarerRivalLossRetreat, NULL, 1, WAYFARER_STORY_SCENE_RUSTBORO_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_RUSTBORO_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRustboroRivalPending),
    HOENN_ENTRY(RustboroCity_EventScript_BattleMayMudkip + 1, RustboroCity_EventScript_WayfarerRivalLossRetreat, NULL, 1, WAYFARER_STORY_SCENE_RUSTBORO_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_RUSTBORO_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRustboroRivalPending),
    HOENN_ENTRY(RustboroCity_EventScript_BattleBrendanTreecko + 1, RustboroCity_EventScript_WayfarerRivalLossRetreat, NULL, 1, WAYFARER_STORY_SCENE_RUSTBORO_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_RUSTBORO_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRustboroRivalPending),
    HOENN_ENTRY(RustboroCity_EventScript_BattleBrendanTorchic + 1, RustboroCity_EventScript_WayfarerRivalLossRetreat, NULL, 1, WAYFARER_STORY_SCENE_RUSTBORO_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_RUSTBORO_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRustboroRivalPending),
    HOENN_ENTRY(RustboroCity_EventScript_BattleBrendanMudkip + 1, RustboroCity_EventScript_WayfarerRivalLossRetreat, NULL, 1, WAYFARER_STORY_SCENE_RUSTBORO_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_RUSTBORO_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRustboroRivalPending),

    // Route 110's walking rival is native; restore it only at its real template after recovery. Its bike remains script-only and hidden.
    HOENN_ENTRY(Route110_EventScript_MayBattleTreecko + 1, Route110_EventScript_WayfarerRivalLossRetreat, Route110_EventScript_RivalScene, 2, WAYFARER_STORY_SCENE_ROUTE110_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_ROUTE110, WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL, 3, 3, 1, 33, 56, WayfarerHoennRoute110RivalPending),
    HOENN_ENTRY(Route110_EventScript_MayBattleTorchic + 1, Route110_EventScript_WayfarerRivalLossRetreat, NULL, 2, WAYFARER_STORY_SCENE_ROUTE110_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE110, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute110RivalPending),
    HOENN_ENTRY(Route110_EventScript_MayBattleMudkip + 1, Route110_EventScript_WayfarerRivalLossRetreat, NULL, 2, WAYFARER_STORY_SCENE_ROUTE110_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE110, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute110RivalPending),
    HOENN_ENTRY(Route110_EventScript_BrendanBattleTreecko + 1, Route110_EventScript_WayfarerRivalLossRetreat, NULL, 2, WAYFARER_STORY_SCENE_ROUTE110_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE110, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute110RivalPending),
    HOENN_ENTRY(Route110_EventScript_BrendanBattleTorchic + 1, Route110_EventScript_WayfarerRivalLossRetreat, NULL, 2, WAYFARER_STORY_SCENE_ROUTE110_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE110, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute110RivalPending),
    HOENN_ENTRY(Route110_EventScript_BrendanBattleMudkip + 1, Route110_EventScript_WayfarerRivalLossRetreat, NULL, 2, WAYFARER_STORY_SCENE_ROUTE110_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE110, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute110RivalPending),
    HOENN_ENTRY(NULL, NULL, NULL, 2, WAYFARER_STORY_SCENE_ROUTE110_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE, MAP_ROUTE110, WAYFARER_HOENN_LOCALID_ROUTE110_RIVAL_ON_BIKE, 3, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute110RivalPending),

    // Route 119 has two authored approach paths; its script provisions the actor and Fly remains a victory-only reward.
    HOENN_ENTRY(Route119_EventScript_BattleMayTreecko + 1, Route119_EventScript_WayfarerRivalLossRetreat, Route119_EventScript_RivalEncounter, 3, WAYFARER_STORY_SCENE_ROUTE119_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_ROUTE119, WAYFARER_HOENN_LOCALID_ROUTE119_RIVAL, 0, 2, 1, 25, 31, WayfarerHoennRoute119RivalPending),
    HOENN_ENTRY(Route119_EventScript_BattleMayTorchic + 1, Route119_EventScript_WayfarerRivalLossRetreat, NULL, 3, WAYFARER_STORY_SCENE_ROUTE119_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE119, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute119RivalPending),
    HOENN_ENTRY(Route119_EventScript_BattleMayMudkip + 1, Route119_EventScript_WayfarerRivalLossRetreat, NULL, 3, WAYFARER_STORY_SCENE_ROUTE119_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE119, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute119RivalPending),
    HOENN_ENTRY(Route119_EventScript_BattleBrendanTreecko + 1, Route119_EventScript_WayfarerRivalLossRetreat, NULL, 3, WAYFARER_STORY_SCENE_ROUTE119_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE119, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute119RivalPending),
    HOENN_ENTRY(Route119_EventScript_BattleBrendanTorchic + 1, Route119_EventScript_WayfarerRivalLossRetreat, NULL, 3, WAYFARER_STORY_SCENE_ROUTE119_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE119, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute119RivalPending),
    HOENN_ENTRY(Route119_EventScript_BattleBrendanMudkip + 1, Route119_EventScript_WayfarerRivalLossRetreat, NULL, 3, WAYFARER_STORY_SCENE_ROUTE119_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE119, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute119RivalPending),
    HOENN_ENTRY(NULL, NULL, NULL, 3, WAYFARER_STORY_SCENE_ROUTE119_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_HIDE_WHILE_UNUSABLE, MAP_ROUTE119, WAYFARER_HOENN_LOCALID_ROUTE119_RIVAL_ON_BIKE, 4, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRoute119RivalPending),

    // Optional rivals retain their chapter; no decline, decor, departure, or call state is written on suppression.
    HOENN_ENTRY(LilycoveCity_EventScript_BattleMayTreecko + 1, LilycoveCity_EventScript_WayfarerRivalLossRetreat, NULL, 4, WAYFARER_STORY_SCENE_LILYCOVE_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, MAP_LILYCOVE_CITY, WAYFARER_HOENN_LOCALID_LILYCOVE_RIVAL, 5, 0, 0, 27, 7, WayfarerHoennLilycoveRivalPending),
    HOENN_ENTRY(LilycoveCity_EventScript_BattleMayTorchic + 1, LilycoveCity_EventScript_WayfarerRivalLossRetreat, NULL, 4, WAYFARER_STORY_SCENE_LILYCOVE_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_LILYCOVE_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennLilycoveRivalPending),
    HOENN_ENTRY(LilycoveCity_EventScript_BattleMayMudkip + 1, LilycoveCity_EventScript_WayfarerRivalLossRetreat, NULL, 4, WAYFARER_STORY_SCENE_LILYCOVE_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_LILYCOVE_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennLilycoveRivalPending),
    HOENN_ENTRY(LilycoveCity_EventScript_BattleBrendanTreecko + 1, LilycoveCity_EventScript_WayfarerRivalLossRetreat, NULL, 4, WAYFARER_STORY_SCENE_LILYCOVE_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_LILYCOVE_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennLilycoveRivalPending),
    HOENN_ENTRY(LilycoveCity_EventScript_BattleBrendanTorchic + 1, LilycoveCity_EventScript_WayfarerRivalLossRetreat, NULL, 4, WAYFARER_STORY_SCENE_LILYCOVE_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_LILYCOVE_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennLilycoveRivalPending),
    HOENN_ENTRY(LilycoveCity_EventScript_BattleBrendanMudkip + 1, LilycoveCity_EventScript_WayfarerRivalLossRetreat, NULL, 4, WAYFARER_STORY_SCENE_LILYCOVE_RIVAL, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_LILYCOVE_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennLilycoveRivalPending),

    HOENN_ENTRY(MauvilleCity_EventScript_BattleWallyTrainerBattle + 1, MauvilleCity_EventScript_WayfarerWallyLossRetreat, NULL, 5, WAYFARER_STORY_SCENE_MAUVILLE_WALLY, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, MAP_MAUVILLE_CITY, WAYFARER_HOENN_LOCALID_MAUVILLE_WALLY, 3, 0, 0, 8, 6, WayfarerHoennMauvilleWallyPending),

    // Victory Road keeps Emerald's loss route and League/rematch behavior; only no-party chapter/rematch start is deferred.
    HOENN_ENTRY(VictoryRoad_1F_EventScript_WallyEntranceTrainerBattle + 1, NULL, VictoryRoad_1F_EventScript_WallyBattleTrigger1, 6, WAYFARER_STORY_SCENE_VICTORY_ROAD_WALLY, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_VICTORY_ROAD_1F, 0, 4, 2, 1, 2, 23, WayfarerHoennVictoryRoadWallyPending),
    HOENN_ENTRY(VictoryRoad_1F_EventScript_ExitWally + 1, NULL, NULL, 6, WAYFARER_STORY_SCENE_VICTORY_ROAD_WALLY, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, WAYFARER_STORY_FLAG_TRANSIENT_OBJECT, MAP_VICTORY_ROAD_1F, WAYFARER_HOENN_LOCALID_VICTORY_ROAD_EXIT_WALLY, 3, 0, 0, 31, 9, WayfarerHoennVictoryRoadWallyRematchEligible),
    HOENN_ENTRY(VictoryRoad_1F_EventScript_RematchWally + 1, NULL, NULL, 6, WAYFARER_STORY_SCENE_VICTORY_ROAD_WALLY, WAYFARER_STORY_POLICY_DEFERRED_RIVAL, WAYFARER_STORY_DIALOGUE_RIVAL, 0, MAP_VICTORY_ROAD_1F, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennVictoryRoadWallyRematchEligible),

    // This scripted party substitution is not an ordinary trainer battle.
    HOENN_ENTRY(NULL, NULL, PetalburgCity_EventScript_WallyTutorial, 0, WAYFARER_STORY_SCENE_PETALBURG_WALLY_TUTORIAL, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_ORDINARY, WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_PETALBURG_CITY, 0, WAYFARER_STORY_ANY_ELEVATION, 30, 30, 0, 0, NULL),

    // Local objectives retain their victory writers.  Only audited single callers opt into a field retry.
    HOENN_ENTRY(NULL, NULL, SlateportCity_OceanicMuseum_2F_EventScript_CaptStern, 0, WAYFARER_STORY_SCENE_OCEANIC_MUSEUM_STERN, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_STERN, 0, MAP_SLATEPORT_CITY_OCEANIC_MUSEUM_2F, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennMuseumPending),
    HOENN_ENTRY(RusturfTunnel_EventScript_GruntTrainerBattle + 1, RusturfTunnel_EventScript_WayfarerGruntLossRetreat, NULL, 20, WAYFARER_STORY_SCENE_RUSTURF_AQUA, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_AQUA_GUARD, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_RUSTURF_TUNNEL, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennRusturfRescuePending),
    HOENN_ENTRY(MtChimney_EventScript_MaxieTrainerBattle + 1, MtChimney_EventScript_WayfarerMaxieLossRetreat, MtChimney_EventScript_Maxie, 21, WAYFARER_STORY_SCENE_MT_CHIMNEY_MAXIE, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_MAGMA_GUARD, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_MT_CHIMNEY, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennMtChimneyPending),
    HOENN_ENTRY(MagmaHideout_4F_EventScript_MaxieTrainerBattle + 1, MagmaHideout_4F_EventScript_WayfarerMaxieLossRetreat, MagmaHideout_4F_EventScript_Maxie, 22, WAYFARER_STORY_SCENE_MAGMA_HIDEOUT_MAXIE, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_MAGMA_GUARD, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_MAGMA_HIDEOUT_4F, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennMagmaHideoutPending),
    HOENN_ENTRY(AquaHideout_B2F_EventScript_Matt + 1, EventScript_WayfarerStoryLossRetreat, NULL, 23, WAYFARER_STORY_SCENE_AQUA_HIDEOUT_MATT, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_AQUA_GUARD, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_AQUA_HIDEOUT_B2F, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennAquaHideoutPending),

    // The ordinary manifest owns the five security grunts. Shelly alone owns this completion writer and field retry.
    HOENN_ENTRY(Route119_WeatherInstitute_2F_EventScript_Shelly + 1, EventScript_WayfarerStoryLossRetreat, NULL, 24, WAYFARER_STORY_SCENE_WEATHER_INSTITUTE_SHELLY, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_AQUA_GUARD, WAYFARER_STORY_FLAG_LOSS_RETURN, MAP_ROUTE119_WEATHER_INSTITUTE_2F, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennWeatherInstitutePending),

    // The invasion chain and Steven's partner offer are separately gated in the map scripts.
    HOENN_ENTRY(MossdeepCity_SpaceCenter_1F_EventScript_Grunt2TrainerBattle + 1, NULL, NULL, 25, WAYFARER_STORY_SCENE_SPACE_CENTER_STAIR_GUARD, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_MAGMA_GUARD, 0, MAP_MOSSDEEP_CITY_SPACE_CENTER_1F, 0, WAYFARER_STORY_ANY_ELEVATION, 0, 0, WAYFARER_STORY_NO_COORD, WAYFARER_STORY_NO_COORD, WayfarerHoennSpaceCenterStairGuardPending),
    HOENN_ENTRY(NULL, NULL, MossdeepCity_SpaceCenter_2F_EventScript_ThreeMagmaGrunts, 0, WAYFARER_STORY_SCENE_SPACE_CENTER_GRUNTS, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_MAGMA_GUARD, WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_MOSSDEEP_CITY_SPACE_CENTER_2F, 0, WAYFARER_STORY_ANY_ELEVATION, 16, 10, 0, 0, WayfarerHoennSpaceCenterGruntsPending),
    HOENN_ENTRY(NULL, NULL, MossdeepCity_SpaceCenter_2F_EventScript_Steven, 0, WAYFARER_STORY_SCENE_SPACE_CENTER_OFFER, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_PARTNER, WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_MOSSDEEP_CITY_SPACE_CENTER_2F, 0, 3, 3, 3, 0, 7, WayfarerHoennSpaceCenterPartnerPending),

    HOENN_ENTRY(SeafloorCavern_Room9_EventScript_ArchieTrainerBattle + 1, SeafloorCavern_Room9_EventScript_WayfarerArchieLossRetreat, SeafloorCavern_Room9_EventScript_ArchieAwakenKyogre, 30, WAYFARER_STORY_SCENE_SEAFLOOR_ARCHIE, WAYFARER_STORY_POLICY_OBJECTIVE_GUARD, WAYFARER_STORY_DIALOGUE_AQUA_GUARD, WAYFARER_STORY_FLAG_LOSS_RETURN | WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_SEAFLOOR_CAVERN_ROOM9, 0, 3, 1, 1, 17, 42, WayfarerHoennSeafloorPending),
    HOENN_ENTRY(NULL, NULL, Route120_EventScript_Steven, 0, WAYFARER_STORY_SCENE_ROUTE120_KECLEON, WAYFARER_STORY_POLICY_EXCEPTION, WAYFARER_STORY_DIALOGUE_INVESTIGATE, WAYFARER_STORY_FLAG_REARM_ON_LEAVE, MAP_ROUTE120, 0, 4, 4, 4, 11, 14, WayfarerHoennKecleonInvestigationPending),
};

const u32 gWayfarerStoryHoennEncounterCount = ARRAY_COUNT(gWayfarerStoryHoennEncounters);

#undef HOENN_ENTRY

#else

const struct WayfarerStoryEncounter gWayfarerStoryHoennEncounters[] =
{
    {0},
};
const u32 gWayfarerStoryHoennEncounterCount = 0;

#endif
