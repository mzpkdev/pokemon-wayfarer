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
