#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "easy_chat.h"
#include "event_data.h"
#include "item.h"
#include "malloc.h"
#include "menu.h"
#include "overworld.h"
#include "sound.h"
#include "strings.h"
#include "string_util.h"
#include "trainer_tower.h"
#include "util.h"
#include "wayfarer_persistence.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/songs.h"
#include "constants/trainers.h"
#include "constants/trainer_tower.h"
#include "constants/vars.h"

#if IS_WAYFARER && FREE_TRAINER_TOWER
#error "Wayfarer Trainer Tower content requires FREE_TRAINER_TOWER == FALSE"
#endif

#define CURR_FLOOR sTrainerTowerState->data.floors[sTrainerTowerState->floorIdx]
#define TRAINER_TOWER gSaveBlock1Ptr->trainerTower[gSaveBlock1Ptr->towerChallengeId]

struct TrainerTowerOpponent
{
    /* 0x00 */ u8 name[11];
    /* 0x0C */ u16 speechWin[6];
    /* 0x18 */ u16 speechLose[6];
    /* 0x24 */ u16 speechWin2[6];
    /* 0x30 */ u16 speechLose2[6];
    /* 0x3C */ u8 battleType;
    /* 0x3D */ u8 facilityClass;
    /* 0x3E */ u8 textColor;
};

struct SinglesTrainerInfo
{
    u16 objGfx;
    u8 facilityClass;
    bool8 gender;
};

struct DoublesTrainerInfo
{
    u16 objGfx1;
    u16 objGfx2;
    u8 facilityClass;
    bool8 gender1;
    bool8 gender2;
};

struct TrainerEncounterMusicPairs
{
    u8 facilityClass;
    u8 musicId;
};

static EWRAM_DATA struct TrainerTowerState * sTrainerTowerState = NULL;
static EWRAM_DATA struct TrainerTowerOpponent * sTrainerTowerOpponent = NULL;

#if IS_WAYFARER
// This intentionally lives only in EWRAM. A reset or reload discards an
// in-progress course, while the story-owned record payload remains saved.
struct WayfarerTrainerTowerRun
{
    struct Pokemon partySnapshot[PARTY_SIZE];
    u32 timer;
    u8 partyCount;
    u8 challengeType;
    u8 localSetId;
    u8 normalizedLevel;
    u8 floorIdx;
    u8 knockoutOpponent;
    u8 clearedFloors;
    bool8 active;
    bool8 snapshotValid;
    bool8 ownerSpoken;
    bool8 timeChecked;
    bool8 lossPending;
};

static EWRAM_DATA struct WayfarerTrainerTowerRun sWayfarerTrainerTowerRun = {0};
#endif

static void SetUpTrainerTowerDataStruct(void);
static void FreeTrainerTowerDataStruct(void);
static void InitTrainerTowerFloor(void);
static void SetTrainerTowerNPCGraphics(void);
static void TT_ConvertEasyChatMessageToString(u16 *ecWords, u8 *dest);
static void BufferTowerOpponentSpeech(void);
static void TrainerTowerGetOpponentTextColor(u8 battleType, u8 facilityClass);
static void DoTrainerTowerBattle(void);
static void TrainerTowerGetChallengeType(void);
static void TrainerTowerAddFloorCleared(void);
static void GetFloorAlreadyCleared(void);
static void StartTrainerTowerChallenge(void);
static void GetOwnerState(void);
static void GiveChallengePrize(void);
static void CheckFinalTime(void);
static void TrainerTowerResumeTimer(void);
static void TrainerTowerSetPlayerLost(void);
static void GetTrainerTowerChallengeStatus(void);
static void GetCurrentTime(void);
static void ShowResultsBoard(void);
static void CloseResultsBoard(void);
static void TrainerTowerGetDoublesEligiblity(void);
static void TrainerTowerGetNumFloors(void);
static void ShouldWarpToCounter(void);
static void PlayTrainerTowerEncounterMusic(void);
static void HasSpokenToOwner(void);
static void BuildEnemyParty(void);
static s32 GetPartyMaxLevel(void);
static void ValidateOrResetCurTrainerTowerRecord(void);
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
static u32 GetTrainerTowerRecordTime(u32 *);
static void SetTrainerTowerRecordTime(u32 *, u32);
#endif //FREE_TRAINER_TOWER
static void TrainerTowerCheckEligibility(void);
static void TrainerTowerAbandonChallenge(void);
static void TrainerTowerCheckPendingPrize(void);
static void TrainerTowerClaimPendingPrize(void);
static void TrainerTowerCheckActive(void);
static void TrainerTowerRestoreAndCloseRun(void);
static u8 GetTrainerTowerChallengeType(void);
static void HealTrainerTowerParty(void);
#if IS_WAYFARER
static void SyncTrainerTowerPendingPrize(const struct WayfarerSeviiTrainerTowerRecords *records);
#endif

const u8 gText_XMinYZSec[] = _("{STR_VAR_1}MIN. {STR_VAR_2}.{STR_VAR_3}SEC.");
static const u8 sTextNoRecord[] = _("--:--.--");

extern const u8 gText_TimeBoard[];


const u8 *const gTrainerTowerChallengeTypeTexts[NUM_TOWER_CHALLENGE_TYPES] =
{
    gText_Single,
    gText_Double,
    gText_Knockout,
    gText_Mixed
};

static const struct SinglesTrainerInfo sSingleBattleTrainerInfo[] = {
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_HEX_MANIAC,          MALE},
    {OBJ_EVENT_GFX_BOY,            FACILITY_CLASS_RICH_BOY,            MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_GUITARIST,           MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_KINDLER,             MALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_BUG_MANIAC,          MALE},
    {OBJ_EVENT_GFX_BOY,            FACILITY_CLASS_SCHOOL_KID_M,        MALE},
    {OBJ_EVENT_GFX_WOMAN_1_FRLG,   FACILITY_CLASS_SCHOOL_KID_F,      FEMALE},
    {OBJ_EVENT_GFX_BALDING_MAN,    FACILITY_CLASS_POKEFAN_M,           MALE},
    {OBJ_EVENT_GFX_WOMAN_3_FRLG,   FACILITY_CLASS_POKEFAN_F,         FEMALE},
    {OBJ_EVENT_GFX_OLD_MAN_1,      FACILITY_CLASS_EXPERT_M,            MALE},
    {OBJ_EVENT_GFX_OLD_WOMAN_FRLG, FACILITY_CLASS_EXPERT_F,          FEMALE},
    {OBJ_EVENT_GFX_COOLTRAINER_M,  FACILITY_CLASS_DRAGON_TAMER,        MALE},
    {OBJ_EVENT_GFX_LITTLE_BOY_FRLG, FACILITY_CLASS_NINJA_BOY,           MALE},
    {OBJ_EVENT_GFX_BATTLE_GIRL,    FACILITY_CLASS_BATTLE_GIRL,       FEMALE},
    {OBJ_EVENT_GFX_BEAUTY_FRLG,    FACILITY_CLASS_PARASOL_LADY,      FEMALE},
    {OBJ_EVENT_GFX_FAT_MAN_FRLG,   FACILITY_CLASS_COLLECTOR,           MALE},
    {OBJ_EVENT_GFX_YOUNGSTER_FRLG, FACILITY_CLASS_YOUNGSTER_FRLG,           MALE},
    {OBJ_EVENT_GFX_BUG_CATCHER_FRLG, FACILITY_CLASS_BUG_CATCHER_FRLG,         MALE},
    {OBJ_EVENT_GFX_LASS_FRLG,      FACILITY_CLASS_LASS_FRLG,              FEMALE},
    {OBJ_EVENT_GFX_SAILOR_FRLG,    FACILITY_CLASS_SAILOR_FRLG,              MALE},
    {OBJ_EVENT_GFX_CAMPER_FRLG,    FACILITY_CLASS_CAMPER_FRLG,              MALE},
    {OBJ_EVENT_GFX_PICNICKER_FRLG, FACILITY_CLASS_PICNICKER_FRLG,         FEMALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_POKEMANIAC,          MALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_SUPER_NERD_FRLG,          MALE},
    {OBJ_EVENT_GFX_HIKER_FRLG,     FACILITY_CLASS_HIKER_FRLG,               MALE},
    {OBJ_EVENT_GFX_BIKER,          FACILITY_CLASS_BIKER_FRLG,               MALE},
    {OBJ_EVENT_GFX_SUPER_NERD,     FACILITY_CLASS_BURGLAR_FRLG,             MALE},
    {OBJ_EVENT_GFX_BALDING_MAN,    FACILITY_CLASS_ENGINEER_FRLG,            MALE},
    {OBJ_EVENT_GFX_FISHER,         FACILITY_CLASS_FISHERMAN_FRLG,           MALE},
    {OBJ_EVENT_GFX_SWIMMER_M_LAND, FACILITY_CLASS_SWIMMER_M_FRLG,           MALE},
    {OBJ_EVENT_GFX_BIKER,          FACILITY_CLASS_CUE_BALL_FRLG,            MALE},
    {OBJ_EVENT_GFX_BEAUTY_FRLG,    FACILITY_CLASS_BEAUTY_FRLG,            FEMALE},
    {OBJ_EVENT_GFX_SWIMMER_F_LAND, FACILITY_CLASS_SWIMMER_F_FRLG,         FEMALE},
    {OBJ_EVENT_GFX_BOY,            FACILITY_CLASS_PSYCHIC_M_FRLG,           MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_ROCKER_FRLG,              MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_JUGGLER_FRLG,             MALE},
    {OBJ_EVENT_GFX_ROCKER,         FACILITY_CLASS_BIRD_KEEPER,         MALE},
    {OBJ_EVENT_GFX_BLACKBELT,      FACILITY_CLASS_BLACK_BELT_FRLG,          MALE},
    {OBJ_EVENT_GFX_SCIENTIST,      FACILITY_CLASS_SCIENTIST_FRLG,           MALE},
    {OBJ_EVENT_GFX_COOLTRAINER_M,  FACILITY_CLASS_COOLTRAINER_M_FRLG,       MALE},
    {OBJ_EVENT_GFX_COOLTRAINER_F,  FACILITY_CLASS_COOLTRAINER_F_FRLG,     FEMALE},
    {OBJ_EVENT_GFX_GENTLEMAN,      FACILITY_CLASS_GENTLEMAN_FRLG,           MALE},
    {OBJ_EVENT_GFX_WOMAN_1_FRLG,   FACILITY_CLASS_PSYCHIC_F_FRLG,         FEMALE},
    {OBJ_EVENT_GFX_BATTLE_GIRL,    FACILITY_CLASS_CRUSH_GIRL_FRLG,        FEMALE},
    {OBJ_EVENT_GFX_TUBER_F_FRLG,   FACILITY_CLASS_TUBER_FRLG,             FEMALE},
    {OBJ_EVENT_GFX_WOMAN_2_FRLG,   FACILITY_CLASS_PKMN_BREEDER_FRLG,      FEMALE},
    {OBJ_EVENT_GFX_CAMPER_FRLG,    FACILITY_CLASS_PKMN_RANGER_M_FRLG,       MALE},
    {OBJ_EVENT_GFX_PICNICKER_FRLG, FACILITY_CLASS_PKMN_RANGER_F_FRLG,     FEMALE},
    {OBJ_EVENT_GFX_WOMAN_2_FRLG,   FACILITY_CLASS_AROMA_LADY_FRLG,        FEMALE},
    {OBJ_EVENT_GFX_HIKER_FRLG,     FACILITY_CLASS_RUIN_MANIAC,         MALE},
    {OBJ_EVENT_GFX_WOMAN_2_FRLG,   FACILITY_CLASS_LADY_FRLG,              FEMALE}
};

static const struct DoublesTrainerInfo sDoubleBattleTrainerInfo[] = {
    {OBJ_EVENT_GFX_BEAUTY_FRLG,    OBJ_EVENT_GFX_WOMAN_1_FRLG,   FACILITY_CLASS_SR_AND_JR,       FEMALE, FEMALE},
    {OBJ_EVENT_GFX_OLD_MAN_1,      OBJ_EVENT_GFX_OLD_WOMAN_FRLG, FACILITY_CLASS_OLD_COUPLE,        MALE, FEMALE},
    {OBJ_EVENT_GFX_LITTLE_GIRL_FRLG, OBJ_EVENT_GFX_LITTLE_GIRL_FRLG,    FACILITY_CLASS_TWINS_FRLG,           FEMALE, FEMALE},
    {OBJ_EVENT_GFX_COOLTRAINER_M,  OBJ_EVENT_GFX_COOLTRAINER_F,  FACILITY_CLASS_COOL_COUPLE_FRLG,       MALE, FEMALE},
    {OBJ_EVENT_GFX_BEAUTY_FRLG,    OBJ_EVENT_GFX_MAN,            FACILITY_CLASS_YOUNG_COUPLE_FRLG,    FEMALE,   MALE},
    {OBJ_EVENT_GFX_BATTLE_GIRL,    OBJ_EVENT_GFX_BLACKBELT,      FACILITY_CLASS_CRUSH_KIN_FRLG,       FEMALE,   MALE},
    {OBJ_EVENT_GFX_SWIMMER_F_LAND, OBJ_EVENT_GFX_TUBER_M_LAND,   FACILITY_CLASS_SIS_AND_BRO_FRLG,     FEMALE,   MALE}
};

static const struct TrainerEncounterMusicPairs sTrainerEncounterMusicLUT[] = {
    {FACILITY_CLASS_AQUA_GRUNT_F,           TRAINER_ENCOUNTER_MUSIC_AQUA},
    {FACILITY_CLASS_RUNNING_TRIATHLETE_F,   TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_LEADER_BRAWLY,          TRAINER_ENCOUNTER_MUSIC_TWINS},
    {FACILITY_CLASS_MAGMA_LEADER_MAXIE,     TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_POKEFAN_F,              TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_PSYCHIC_M_FRLG,              TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_PICNICKER_FRLG,              TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_GUITARIST,              TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_RUNNING_TRIATHLETE_M,   TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_CAMPER_FRLG,                 TRAINER_ENCOUNTER_MUSIC_TWINS},
    {FACILITY_CLASS_CYCLING_TRIATHLETE_F,   TRAINER_ENCOUNTER_MUSIC_TWINS},
    {FACILITY_CLASS_BEAUTY_FRLG,                 TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_INTERVIEWER,            TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_YOUNGSTER_FRLG,              TRAINER_ENCOUNTER_MUSIC_RICH},
    {FACILITY_CLASS_MAGMA_GRUNT_F,          TRAINER_ENCOUNTER_MUSIC_SWIMMER},
    {FACILITY_CLASS_SUPER_NERD_FRLG,             TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_ENGINEER_FRLG,               TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_SAILOR_FRLG,                 TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_BUG_MANIAC,             TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_KINDLER,                TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_BATTLE_GIRL,            TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {FACILITY_CLASS_COLLECTOR,              TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {FACILITY_CLASS_NINJA_BOY,              TRAINER_ENCOUNTER_MUSIC_MAGMA},
    {FACILITY_CLASS_PARASOL_LADY,           TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_EXPERT_F,               TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_RICH_BOY,               TRAINER_ENCOUNTER_MUSIC_RICH},
    {FACILITY_CLASS_HEX_MANIAC,             TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_SWIMMER_F_FRLG,              TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_CYCLING_TRIATHLETE_M,   TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_SWIMMER_M_FRLG,              TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_DRAGON_TAMER,           TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_BIKER_FRLG,                  TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_SWIMMING_TRIATHLETE_M,  TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS},
    {FACILITY_CLASS_CUE_BALL_FRLG,               TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_SWIMMING_TRIATHLETE_F,  TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_POKEMANIAC,             TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_BUG_CATCHER_FRLG,            TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_OLD_COUPLE,             TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_LEADER_ROXANNE,         TRAINER_ENCOUNTER_MUSIC_TWINS},
    {FACILITY_CLASS_BURGLAR_FRLG,                TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_SCHOOL_KID_M,           TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_LEADER_TATE_AND_LIZA,   TRAINER_ENCOUNTER_MUSIC_INTENSE},
    {FACILITY_CLASS_POKEFAN_M,              TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_EXPERT_M,               TRAINER_ENCOUNTER_MUSIC_COOL},
    {FACILITY_CLASS_MAGMA_GRUNT_M,          TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_SR_AND_JR,              TRAINER_ENCOUNTER_MUSIC_HIKER},
    {FACILITY_CLASS_LASS_FRLG,                   TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_SCHOOL_KID_F,           TRAINER_ENCOUNTER_MUSIC_MALE},
    {FACILITY_CLASS_FISHERMAN_FRLG,              TRAINER_ENCOUNTER_MUSIC_GIRL},
    {FACILITY_CLASS_HIKER_FRLG,                  TRAINER_ENCOUNTER_MUSIC_SWIMMER},
    {FACILITY_CLASS_ELITE_FOUR_SIDNEY,      TRAINER_ENCOUNTER_MUSIC_FEMALE},
    {FACILITY_CLASS_ELITE_FOUR_PHOEBE,      TRAINER_ENCOUNTER_MUSIC_MALE}
};

static const struct WindowTemplate sTimeBoardWindowTemplate[] = {
    {
        .bg = 0,
        .tilemapLeft = 3,
        .tilemapTop = 1,
        .width = 27,
        .height = 18,
        .paletteNum = 15,
        .baseBlock = 0x001
    }, DUMMY_WIN_TEMPLATE
};

static const u32 sUnusedValue = 112;

static const u8 sTextColors[3] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static void (*const sTrainerTowerFunctions[])(void) = {
    [TRAINER_TOWER_FUNC_INIT_FLOOR]             = InitTrainerTowerFloor,
    [TRAINER_TOWER_FUNC_GET_SPEECH]             = BufferTowerOpponentSpeech,
    [TRAINER_TOWER_FUNC_DO_BATTLE]              = DoTrainerTowerBattle,
    [TRAINER_TOWER_FUNC_GET_CHALLENGE_TYPE]     = TrainerTowerGetChallengeType,
    [TRAINER_TOWER_FUNC_CLEARED_FLOOR]          = TrainerTowerAddFloorCleared,
    [TRAINER_TOWER_FUNC_GET_FLOOR_CLEARED]      = GetFloorAlreadyCleared,
    [TRAINER_TOWER_FUNC_START_CHALLENGE]        = StartTrainerTowerChallenge,
    [TRAINER_TOWER_FUNC_GET_OWNER_STATE]        = GetOwnerState,
    [TRAINER_TOWER_FUNC_GIVE_PRIZE]             = GiveChallengePrize,
    [TRAINER_TOWER_FUNC_CHECK_FINAL_TIME]       = CheckFinalTime,
    [TRAINER_TOWER_FUNC_RESUME_TIMER]           = TrainerTowerResumeTimer,
    [TRAINER_TOWER_FUNC_SET_LOST]               = TrainerTowerSetPlayerLost,
    [TRAINER_TOWER_FUNC_GET_CHALLENGE_STATUS]   = GetTrainerTowerChallengeStatus,
    [TRAINER_TOWER_FUNC_GET_TIME]               = GetCurrentTime,
    [TRAINER_TOWER_FUNC_SHOW_RESULTS]           = ShowResultsBoard,
    [TRAINER_TOWER_FUNC_CLOSE_RESULTS]          = CloseResultsBoard,
    [TRAINER_TOWER_FUNC_CHECK_DOUBLES]          = TrainerTowerGetDoublesEligiblity,
    [TRAINER_TOWER_FUNC_GET_NUM_FLOORS]         = TrainerTowerGetNumFloors,
    [TRAINER_TOWER_FUNC_SHOULD_WARP_TO_COUNTER] = ShouldWarpToCounter,
    [TRAINER_TOWER_FUNC_ENCOUNTER_MUSIC]        = PlayTrainerTowerEncounterMusic,
    [TRAINER_TOWER_FUNC_GET_BEAT_CHALLENGE]     = HasSpokenToOwner,
    [TRAINER_TOWER_FUNC_CHECK_ELIGIBILITY]      = TrainerTowerCheckEligibility,
    [TRAINER_TOWER_FUNC_ABANDON_CHALLENGE]      = TrainerTowerAbandonChallenge,
    [TRAINER_TOWER_FUNC_CHECK_PENDING_PRIZE]    = TrainerTowerCheckPendingPrize,
    [TRAINER_TOWER_FUNC_CLAIM_PENDING_PRIZE]    = TrainerTowerClaimPendingPrize,
    [TRAINER_TOWER_FUNC_CHECK_ACTIVE]           = TrainerTowerCheckActive,
};

// - 1 excludes Mixed challenge, which just uses one of the 3 other types
static const u16 sFloorLayouts[MAX_TRAINER_TOWER_FLOORS][NUM_TOWER_CHALLENGE_TYPES - 1] = {
    {LAYOUT_TRAINER_TOWER_1F, LAYOUT_TRAINER_TOWER_1F_DOUBLES, LAYOUT_TRAINER_TOWER_1F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_2F, LAYOUT_TRAINER_TOWER_2F_DOUBLES, LAYOUT_TRAINER_TOWER_2F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_3F, LAYOUT_TRAINER_TOWER_3F_DOUBLES, LAYOUT_TRAINER_TOWER_3F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_4F, LAYOUT_TRAINER_TOWER_4F_DOUBLES, LAYOUT_TRAINER_TOWER_4F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_5F, LAYOUT_TRAINER_TOWER_5F_DOUBLES, LAYOUT_TRAINER_TOWER_5F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_6F, LAYOUT_TRAINER_TOWER_6F_DOUBLES, LAYOUT_TRAINER_TOWER_6F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_7F, LAYOUT_TRAINER_TOWER_7F_DOUBLES, LAYOUT_TRAINER_TOWER_7F_KNOCKOUT},
    {LAYOUT_TRAINER_TOWER_8F, LAYOUT_TRAINER_TOWER_8F_DOUBLES, LAYOUT_TRAINER_TOWER_8F_KNOCKOUT}
};

static const u16 sPrizeList[] = {
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CARBOS,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_BRIGHT_POWDER,
    ITEM_WHITE_HERB,
    ITEM_MENTAL_HERB,
    ITEM_CHOICE_BAND,
    ITEM_KINGS_ROCK,
    ITEM_SCOPE_LENS,
    ITEM_METAL_COAT,
    ITEM_DRAGON_SCALE,
    ITEM_UP_GRADE
};

static const u16 sTrainerTowerEncounterMusic[] = {
    [TRAINER_ENCOUNTER_MUSIC_MALE]        = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_FEMALE]      = MUS_RG_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_GIRL]        = MUS_RG_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_SUSPICIOUS]  = MUS_RG_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_INTENSE]     = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_COOL]        = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_AQUA]        = MUS_RG_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_MAGMA]       = MUS_RG_ENCOUNTER_ROCKET,
    [TRAINER_ENCOUNTER_MUSIC_SWIMMER]     = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_TWINS]       = MUS_RG_ENCOUNTER_GIRL,
    [TRAINER_ENCOUNTER_MUSIC_ELITE_FOUR]  = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_HIKER]       = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_INTERVIEWER] = MUS_RG_ENCOUNTER_BOY,
    [TRAINER_ENCOUNTER_MUSIC_RICH]        = MUS_RG_ENCOUNTER_BOY
};

// The trainer only uses two Pokemon from the encoded pool, based on the current floor
static const u8 sSingleBattleChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][2] = {
    {0, 2},
    {1, 3},
    {2, 4},
    {3, 5},
    {4, 1},
    {5, 2},
    {0, 3},
    {1, 4}
};

// Each trainer only uses one Pokemon from the encoded pool, based on the current floor
static const u8 sDoubleBattleChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][2] = {
    {0, 1},
    {1, 3},
    {2, 0},
    {3, 4},
    {4, 2},
    {5, 2},
    {0, 3},
    {1, 5}
};

// Each trainer only uses one Pokemon from the encoded pool, based on the current floor
static const u8 sKnockoutChallengeMonIdxs[MAX_TRAINER_TOWER_FLOORS][3] = {
    {0, 2, 4},
    {1, 3, 5},
    {2, 3, 1},
    {3, 4, 0},
    {4, 1, 2},
    {5, 0, 3},
    {0, 5, 2},
    {1, 4, 5}
};

extern const struct EReaderTrainerTowerSetSubstruct gTrainerTowerLocalHeader;
extern const struct TrainerTowerFloor *const gTrainerTowerFloors[][MAX_TRAINER_TOWER_FLOORS];

bool8 WayfarerTrainerTowerIsChallengeActive(void)
{
#if IS_WAYFARER
    return sWayfarerTrainerTowerRun.active;
#else
    return FALSE;
#endif
}

bool8 WayfarerTrainerTowerIsSaveAllowed(void)
{
    return !WayfarerTrainerTowerIsChallengeActive();
}

#if IS_WAYFARER
void WayfarerTrainerTowerResetTransientState(void)
{
    ClearTrainerHillVBlankCounter();
    memset(&sWayfarerTrainerTowerRun, 0, sizeof(sWayfarerTrainerTowerRun));
}

u8 WayfarerTrainerTowerGetUsablePartyCount(void)
{
    u8 count = 0;
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        u32 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);
        if (species != SPECIES_NONE && species != SPECIES_EGG
         && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
            count++;
    }
    return count;
}

u8 WayfarerTrainerTowerNormalizeLevel(u8 highestUsableLevel)
{
    if (highestUsableLevel == 0)
        return 1;
    if (highestUsableLevel > MAX_LEVEL)
        return MAX_LEVEL;
    return highestUsableLevel;
}

u8 WayfarerTrainerTowerGetFloorChallengeType(u8 challengeType, u8 floor)
{
    if (challengeType >= NUM_TOWER_CHALLENGE_TYPES || floor >= MAX_TRAINER_TOWER_FLOORS)
        return NUM_TOWER_CHALLENGE_TYPES;
    return gTrainerTowerFloors[challengeType][floor]->challengeType;
}

u16 WayfarerTrainerTowerGetPrize(u8 challengeType)
{
    static const u16 sWayfarerPrizeByChallenge[NUM_TOWER_CHALLENGE_TYPES] =
    {
        [CHALLENGE_TYPE_SINGLE] = ITEM_UP_GRADE,
        [CHALLENGE_TYPE_DOUBLE] = ITEM_DRAGON_SCALE,
        [CHALLENGE_TYPE_KNOCKOUT] = ITEM_METAL_COAT,
        [CHALLENGE_TYPE_MIXED] = ITEM_KINGS_ROCK,
    };

    if (challengeType >= NUM_TOWER_CHALLENGE_TYPES)
        return ITEM_NONE;
    return sWayfarerPrizeByChallenge[challengeType];
}

bool8 WayfarerTrainerTowerRecordTime(struct WayfarerSeviiTrainerTowerRecords *records, u8 challengeType, u32 time)
{
    u8 bit;

    if (records == NULL || challengeType >= NUM_TOWER_CHALLENGE_TYPES
     || time == 0 || time > TRAINER_TOWER_MAX_TIME)
        return FALSE;

    bit = 1 << challengeType;
    if (!(records->completedMask & bit) || records->bestTime[challengeType] == 0
     || time < records->bestTime[challengeType])
    {
        records->bestTime[challengeType] = time;
        records->completedMask |= bit;
        return TRUE;
    }
    return FALSE;
}
#endif

void WayfarerTrainerTowerFormatTime(u32 time, u16 *minutes, u8 *seconds, u8 *centiseconds)
{
    if (time > TRAINER_TOWER_MAX_TIME)
        time = TRAINER_TOWER_MAX_TIME;
    *minutes = time / (60 * 60);
    time %= 60 * 60;
    *seconds = time / 60;
    *centiseconds = (time % 60) * 168 / 100;
}

// The authoritative persistent payload exists only in Wayfarer SaveBlock3.
#if IS_WAYFARER
static void SyncTrainerTowerPendingPrize(const struct WayfarerSeviiTrainerTowerRecords *records)
{
    VarSet(VAR_WAYFARER_SEVII_TRAINER_TOWER_PENDING_PRIZE,
           records == NULL ? ITEM_NONE : records->pendingPrize);
}
#endif

static void HealTrainerTowerParty(void)
{
    u8 i;

    for (i = 0; i < gPlayerPartyCount; i++)
        HealPokemon(&gPlayerParty[i]);
}

static u8 GetTrainerTowerChallengeType(void)
{
#if IS_WAYFARER
    if (sWayfarerTrainerTowerRun.active)
        return sWayfarerTrainerTowerRun.challengeType;
#endif
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    if (gSaveBlock1Ptr->towerChallengeId < NUM_TOWER_CHALLENGE_TYPES)
        return gSaveBlock1Ptr->towerChallengeId;
#endif
    return CHALLENGE_TYPE_SINGLE;
}

void CallTrainerTowerFunc(void)
{
    if (gSpecialVar_0x8004 >= ARRAY_COUNT(sTrainerTowerFunctions))
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    SetUpTrainerTowerDataStruct();
    sTrainerTowerFunctions[gSpecialVar_0x8004]();
    FreeTrainerTowerDataStruct();
}

u8 GetTrainerTowerOpponentClass(void)
{
    return gFacilityClassToTrainerClass[sTrainerTowerOpponent->facilityClass];
}

void GetTrainerTowerOpponentName(u8 *dest)
{
    StringCopyN(dest, sTrainerTowerOpponent->name, 11);
}

u8 GetTrainerTowerTrainerFrontSpriteId(void)
{
    return gFacilityClassToPicIndex[sTrainerTowerOpponent->facilityClass];
}

void InitTrainerTowerBattleStruct(void)
{
    u16 trainerId;
    int i;

    SetUpTrainerTowerDataStruct();
    sTrainerTowerOpponent = AllocZeroed(sizeof(*sTrainerTowerOpponent));
    trainerId = VarGet(VAR_TEMP_1);
    StringCopyN(sTrainerTowerOpponent->name, CURR_FLOOR.trainers[trainerId].name, 11);

    for (i = 0; i < 6; i++)
    {
        sTrainerTowerOpponent->speechWin[i] = CURR_FLOOR.trainers[trainerId].speechWin[i];
        sTrainerTowerOpponent->speechLose[i] = CURR_FLOOR.trainers[trainerId].speechLose[i];

        if (CURR_FLOOR.challengeType == CHALLENGE_TYPE_DOUBLE)
        {
            sTrainerTowerOpponent->speechWin2[i] = CURR_FLOOR.trainers[trainerId + 1].speechWin[i];
            sTrainerTowerOpponent->speechLose2[i] = CURR_FLOOR.trainers[trainerId + 1].speechLose[i];
        }
    }

    sTrainerTowerOpponent->battleType = CURR_FLOOR.challengeType;
    sTrainerTowerOpponent->facilityClass = CURR_FLOOR.trainers[trainerId].facilityClass;
    sTrainerTowerOpponent->textColor = CURR_FLOOR.trainers[trainerId].textColor;
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    SetTrainerHillVBlankCounter(&TRAINER_TOWER.timer);
#elif IS_WAYFARER
    if (sWayfarerTrainerTowerRun.active)
        SetTrainerHillVBlankCounter(&sWayfarerTrainerTowerRun.timer);
#endif //FREE_TRAINER_TOWER
    FreeTrainerTowerDataStruct();
}

void FreeTrainerTowerBattleStruct(void)
{
    FREE_AND_SET_NULL(sTrainerTowerOpponent);
}

void GetTrainerTowerOpponentWinText(u8 *dest, u8 opponentIdx)
{
    VarSet(VAR_TEMP_3, opponentIdx);
    TrainerTowerGetOpponentTextColor(sTrainerTowerOpponent->battleType, sTrainerTowerOpponent->facilityClass);
    if (opponentIdx == 0)
        TT_ConvertEasyChatMessageToString(sTrainerTowerOpponent->speechWin, dest);
    else
        TT_ConvertEasyChatMessageToString(sTrainerTowerOpponent->speechWin2, dest);
}

void GetTrainerTowerOpponentLoseText(u8 *dest, u8 opponentIdx)
{
    VarSet(VAR_TEMP_3, opponentIdx);
    TrainerTowerGetOpponentTextColor(sTrainerTowerOpponent->battleType, sTrainerTowerOpponent->facilityClass);
    if (opponentIdx == 0)
        TT_ConvertEasyChatMessageToString(sTrainerTowerOpponent->speechLose, dest);
    else
        TT_ConvertEasyChatMessageToString(sTrainerTowerOpponent->speechLose2, dest);
}

static void SetUpTrainerTowerDataStruct(void)
{
    u32 challengeType = GetTrainerTowerChallengeType();
    s32 i;
    const struct TrainerTowerFloor *const * floors_p;

    sTrainerTowerState = AllocZeroed(sizeof(*sTrainerTowerState));
    sTrainerTowerState->floorIdx = gMapHeader.mapLayoutId - LAYOUT_TRAINER_TOWER_1F;
    // if (ReadTrainerTowerAndValidate() == TRUE)
    //     CEReaderTool_LoadTrainerTower(&sTrainerTowerState->data);
    // else
    // {
        struct TrainerTowerState * ttstate_p = sTrainerTowerState;
        const struct EReaderTrainerTowerSetSubstruct * header_p = &gTrainerTowerLocalHeader;
        memcpy(&ttstate_p->data, header_p, sizeof(struct EReaderTrainerTowerSetSubstruct));
        floors_p = gTrainerTowerFloors[challengeType];
        for (i = 0; i < MAX_TRAINER_TOWER_FLOORS; i++)
        {
            *(sTrainerTowerState->data.floors + i) = *(floors_p[i]); // manual pointer arithmetic needed to match
        }
        sTrainerTowerState->data.checksum = CalcByteArraySum((void *)sTrainerTowerState->data.floors, sizeof(sTrainerTowerState->data.floors));
        ValidateOrResetCurTrainerTowerRecord();
    // }
}

static void FreeTrainerTowerDataStruct(void)
{
    TRY_FREE_AND_SET_NULL(sTrainerTowerState);
}

static void InitTrainerTowerFloor(void)
{
    if (gMapHeader.mapLayoutId - LAYOUT_TRAINER_TOWER_LOBBY > sTrainerTowerState->data.numFloors)
    {
        gSpecialVar_Result = 3; // Skip past usable challenge types
        SetCurrentMapLayout(LAYOUT_TRAINER_TOWER_ROOF);
    }
    else
    {
#if IS_WAYFARER
        if (sWayfarerTrainerTowerRun.active)
            sWayfarerTrainerTowerRun.floorIdx = sTrainerTowerState->floorIdx;
#endif
        gSpecialVar_Result = CURR_FLOOR.challengeType;
        SetCurrentMapLayout(sFloorLayouts[sTrainerTowerState->floorIdx][gSpecialVar_Result]);
        SetTrainerTowerNPCGraphics();
    }
}

static void SetTrainerTowerNPCGraphics(void)
{
    s32 i, j;
    u16 trainerGfx1, trainerGfx2, facilityClass;
    switch (CURR_FLOOR.challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
        facilityClass = CURR_FLOOR.trainers[0].facilityClass;
        for (i = 0; i < NELEMS(sSingleBattleTrainerInfo); i++)
        {
            if (sSingleBattleTrainerInfo[i].facilityClass == facilityClass)
                break;
        }

        if (i != NELEMS(sSingleBattleTrainerInfo))
            trainerGfx1 = sSingleBattleTrainerInfo[i].objGfx;
        else
            trainerGfx1 = OBJ_EVENT_GFX_YOUNGSTER_FRLG;

        VarSet(VAR_OBJ_GFX_ID_1, trainerGfx1);
        break;
    case CHALLENGE_TYPE_DOUBLE:
        facilityClass = CURR_FLOOR.trainers[0].facilityClass;
        for (i = 0; i < NELEMS(sDoubleBattleTrainerInfo); i++)
        {
            if (sDoubleBattleTrainerInfo[i].facilityClass == facilityClass)
                break;
        }
        if (i != NELEMS(sDoubleBattleTrainerInfo))
        {
            trainerGfx1  = sDoubleBattleTrainerInfo[i].objGfx1;
            trainerGfx2 = sDoubleBattleTrainerInfo[i].objGfx2;
        }
        else
        {
            trainerGfx1  = OBJ_EVENT_GFX_YOUNGSTER_FRLG;
            trainerGfx2 = OBJ_EVENT_GFX_YOUNGSTER_FRLG;
        }
        VarSet(VAR_OBJ_GFX_ID_0, trainerGfx1);
        VarSet(VAR_OBJ_GFX_ID_3, trainerGfx2);
        break;
    case CHALLENGE_TYPE_KNOCKOUT:
        for (j = 0; j < MAX_TRAINERS_PER_FLOOR; j++)
        {
            facilityClass = CURR_FLOOR.trainers[j].facilityClass;
            for (i = 0; i < NELEMS(sSingleBattleTrainerInfo); i++)
            {
                if (sSingleBattleTrainerInfo[i].facilityClass == facilityClass)
                    break;
            }

            if (i != NELEMS(sSingleBattleTrainerInfo))
                trainerGfx1 = sSingleBattleTrainerInfo[i].objGfx;
            else
                trainerGfx1 = OBJ_EVENT_GFX_YOUNGSTER;

            switch (j)
            {
            case 0:
                VarSet(VAR_OBJ_GFX_ID_2, trainerGfx1);
                break;
            case 1:
                VarSet(VAR_OBJ_GFX_ID_0, trainerGfx1);
                break;
            case 2:
                VarSet(VAR_OBJ_GFX_ID_1, trainerGfx1);
                break;
            }
        }
    }
}

static void TT_ConvertEasyChatMessageToString(u16 *ecWords, u8 *dest)
{
    s32 i;
    ConvertEasyChatWordsToString(dest, ecWords, 3, 2);
    if ((unsigned)GetStringWidth(FONT_NORMAL, dest, -1) > 196)
    {
        // Has to be printed 2x3
        ConvertEasyChatWordsToString(dest, ecWords, 2, 3);
        // Skip line 1
        i = 0;
        while (dest[i++] != CHAR_NEWLINE)
            ;
        // Skip line 2
        while (dest[i] != CHAR_NEWLINE)
            i++;
        // Replace \n with \l at the end of line 2
        dest[i] = CHAR_PROMPT_SCROLL;
    }
}

static void BufferTowerOpponentSpeech(void)
{
    u16 trainerId = gSpecialVar_0x8006;
    u8 facilityClass;
    u8 challengeType = CURR_FLOOR.challengeType;
    
    if (challengeType != CHALLENGE_TYPE_DOUBLE)
        facilityClass = CURR_FLOOR.trainers[trainerId].facilityClass;
    else
        facilityClass = CURR_FLOOR.trainers[0].facilityClass;

    switch (gSpecialVar_0x8005)
    {
    case TRAINER_TOWER_TEXT_INTRO:
        TrainerTowerGetOpponentTextColor(challengeType, facilityClass);
        TT_ConvertEasyChatMessageToString(CURR_FLOOR.trainers[trainerId].speechBefore, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_PLAYER_LOST:
        TrainerTowerGetOpponentTextColor(challengeType, facilityClass);
        TT_ConvertEasyChatMessageToString(CURR_FLOOR.trainers[trainerId].speechWin, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_PLAYER_WON:
        TrainerTowerGetOpponentTextColor(challengeType, facilityClass);
        TT_ConvertEasyChatMessageToString(CURR_FLOOR.trainers[trainerId].speechLose, gStringVar4);
        break;
    case TRAINER_TOWER_TEXT_AFTER:
        TT_ConvertEasyChatMessageToString(CURR_FLOOR.trainers[trainerId].speechAfter, gStringVar4);
        break;
    }
}

static void TrainerTowerGetOpponentTextColor(u8 challengeType, u8 facilityClass)
{
    // u16 gender = MALE;
    // int i;
    // switch (challengeType)
    // {
    // case CHALLENGE_TYPE_SINGLE:
    // case CHALLENGE_TYPE_KNOCKOUT:
    //     for (i = 0; i < NELEMS(sSingleBattleTrainerInfo); i++)
    //     {
    //         if (sSingleBattleTrainerInfo[i].facilityClass == facilityClass)
    //             break;
    //     }
    //     if (i != NELEMS(sSingleBattleTrainerInfo))
    //         gender = sSingleBattleTrainerInfo[i].gender;
    //     break;
    // case CHALLENGE_TYPE_DOUBLE:
    //     for (i = 0; i < NELEMS(sDoubleBattleTrainerInfo); i++)
    //     {
    //         if (sDoubleBattleTrainerInfo[i].facilityClass == facilityClass)
    //             break;
    //     }
    //     if (i != NELEMS(sDoubleBattleTrainerInfo))
    //     {
    //         if (VarGet(VAR_TEMP_3))
    //             gender = sDoubleBattleTrainerInfo[i].gender2;
    //         else
    //             gender = sDoubleBattleTrainerInfo[i].gender1;
    //     }
    //     break;
    // }
    // gSpecialVar_PrevTextColor = gSpecialVar_TextColor;
    // gSpecialVar_TextColor = gender;
}

static void CB2_EndTrainerTowerBattle(void)
{
    SetMainCallback2(CB2_ReturnToFieldContinueScriptPlayMapMusic);
}

static void Task_DoTrainerTowerBattle(u8 taskId)
{
    if (IsBattleTransitionDone() == TRUE)
    {
        gMain.savedCallback = CB2_EndTrainerTowerBattle;
        CleanupOverworldWindowsAndTilemaps();
        SetMainCallback2(CB2_InitBattle);
        DestroyTask(taskId);
    }
}

static void DoTrainerTowerBattle(void)
{
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_TRAINER_TOWER;

    if (CURR_FLOOR.challengeType == CHALLENGE_TYPE_DOUBLE)
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;

    TRAINER_BATTLE_PARAM.opponentA = 0;
    BuildEnemyParty();
    CreateTask(Task_DoTrainerTowerBattle, 1);
    PlayMapChosenOrBattleBGM(0);
    BattleTransition_StartOnField(GetSpecialBattleTransition(B_TRANSITION_GROUP_TRAINER_HILL));
}

static void TrainerTowerGetChallengeType(void)
{
    if (!gSpecialVar_0x8005)
        gSpecialVar_Result = CURR_FLOOR.challengeType;
}

static void TrainerTowerAddFloorCleared(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    TRAINER_TOWER.floorsCleared++;
#elif IS_WAYFARER
    if (sWayfarerTrainerTowerRun.active && sWayfarerTrainerTowerRun.floorIdx < MAX_TRAINER_TOWER_FLOORS)
        sWayfarerTrainerTowerRun.clearedFloors |= 1 << sWayfarerTrainerTowerRun.floorIdx;
#endif //FREE_TRAINER_TOWER
}

// So the player can safely go back through defeated floors to use the Poke Center (or exit challenge)
static void GetFloorAlreadyCleared(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    u16 mapLayoutId = gMapHeader.mapLayoutId;
    if (mapLayoutId - LAYOUT_TRAINER_TOWER_1F == TRAINER_TOWER.floorsCleared 
     && mapLayoutId - LAYOUT_TRAINER_TOWER_LOBBY <= CURR_FLOOR.floorIdx)
        gSpecialVar_Result = FALSE;
    else
        gSpecialVar_Result = TRUE;
#else
    gSpecialVar_Result = TRUE;
#endif //FREE_TRAINER_TOWER
#if IS_WAYFARER
    if (sWayfarerTrainerTowerRun.active && sWayfarerTrainerTowerRun.floorIdx < MAX_TRAINER_TOWER_FLOORS)
        gSpecialVar_Result = (sWayfarerTrainerTowerRun.clearedFloors & (1 << sWayfarerTrainerTowerRun.floorIdx)) != 0;
#endif
}

static void StartTrainerTowerChallenge(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    gSaveBlock1Ptr->towerChallengeId = gSpecialVar_0x8005;
    if (gSaveBlock1Ptr->towerChallengeId >= NUM_TOWER_CHALLENGE_TYPES)
        gSaveBlock1Ptr->towerChallengeId = 0;
    ValidateOrResetCurTrainerTowerRecord();
    TRAINER_TOWER.validated = TRUE;
    TRAINER_TOWER.floorsCleared = 0;
    SetTrainerHillVBlankCounter(&TRAINER_TOWER.timer);
    TRAINER_TOWER.timer = 0;
    TRAINER_TOWER.spokeToOwner = FALSE;
    TRAINER_TOWER.checkedFinalTime = FALSE;
#elif IS_WAYFARER
    u8 challengeType = gSpecialVar_0x8005;
    u8 neededMons = (challengeType == CHALLENGE_TYPE_DOUBLE || challengeType == CHALLENGE_TYPE_MIXED) ? 2 : 1;
    struct WayfarerSeviiTrainerTowerRecords *records = WayfarerSevii_GetTrainerTowerRecords();

    gSpecialVar_Result = FALSE;
    if (challengeType >= NUM_TOWER_CHALLENGE_TYPES || sWayfarerTrainerTowerRun.active
     || records == NULL || records->pendingPrize != ITEM_NONE
     || WayfarerTrainerTowerGetUsablePartyCount() < neededMons)
        return;

    HealTrainerTowerParty();
    memcpy(sWayfarerTrainerTowerRun.partySnapshot, gPlayerParty, sizeof(gPlayerParty));
    sWayfarerTrainerTowerRun.partyCount = gPlayerPartyCount;
    sWayfarerTrainerTowerRun.challengeType = challengeType;
    sWayfarerTrainerTowerRun.localSetId = gTrainerTowerLocalHeader.id;
    sWayfarerTrainerTowerRun.normalizedLevel = GetPartyMaxLevel();
    sWayfarerTrainerTowerRun.floorIdx = 0;
    sWayfarerTrainerTowerRun.knockoutOpponent = 0;
    sWayfarerTrainerTowerRun.clearedFloors = 0;
    sWayfarerTrainerTowerRun.timer = 0;
    sWayfarerTrainerTowerRun.active = TRUE;
    sWayfarerTrainerTowerRun.snapshotValid = TRUE;
    sWayfarerTrainerTowerRun.ownerSpoken = FALSE;
    sWayfarerTrainerTowerRun.timeChecked = FALSE;
    sWayfarerTrainerTowerRun.lossPending = FALSE;
    SetTrainerHillVBlankCounter(&sWayfarerTrainerTowerRun.timer);
    gSpecialVar_Result = TRUE;
#endif //FREE_TRAINER_TOWER
}

static void GetOwnerState(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    ClearTrainerHillVBlankCounter();
    gSpecialVar_Result = 0;

    if (TRAINER_TOWER.spokeToOwner)
        gSpecialVar_Result++;
    if (TRAINER_TOWER.receivedPrize && TRAINER_TOWER.checkedFinalTime)
        gSpecialVar_Result++;

    TRAINER_TOWER.spokeToOwner = TRUE;
#elif IS_WAYFARER
    gSpecialVar_Result = 0;
    if (!sWayfarerTrainerTowerRun.active)
    {
        gSpecialVar_Result = 2;
        return;
    }
    ClearTrainerHillVBlankCounter();
    if (sWayfarerTrainerTowerRun.ownerSpoken)
        gSpecialVar_Result++;
    if (sWayfarerTrainerTowerRun.timeChecked)
        gSpecialVar_Result++;
    sWayfarerTrainerTowerRun.ownerSpoken = TRUE;
#else
    gSpecialVar_Result = 2;
#endif //FREE_TRAINER_TOWER
}

static void GiveChallengePrize(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    u16 itemId = sPrizeList[sTrainerTowerState->data.floors->prize];

    if (TRAINER_TOWER.receivedPrize)
    {
        gSpecialVar_Result = 2;
    }
    else if (AddBagItem(itemId, 1) == TRUE)
    {
        CopyItemName(itemId, gStringVar2);
        TRAINER_TOWER.receivedPrize = TRUE;
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }
#elif IS_WAYFARER
    struct WayfarerSeviiTrainerTowerRecords *records = WayfarerSevii_GetTrainerTowerRecords();
    u16 itemId = WayfarerTrainerTowerGetPrize(sWayfarerTrainerTowerRun.challengeType);

    gSpecialVar_Result = 2;
    if (!sWayfarerTrainerTowerRun.active || records == NULL || itemId == ITEM_NONE)
        return;
    SyncTrainerTowerPendingPrize(records);
    if (AddBagItem(itemId, 1))
    {
        CopyItemName(itemId, gStringVar2);
        gSpecialVar_Result = 0;
    }
    else
    {
        records->pendingPrize = itemId;
        SyncTrainerTowerPendingPrize(records);
        gSpecialVar_Result = 1;
    }
    TrainerTowerRestoreAndCloseRun();
#else
    gSpecialVar_Result = 0;
#endif //FREE_TRAINER_TOWER
}

static void CheckFinalTime(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    if (TRAINER_TOWER.checkedFinalTime)
    {
        gSpecialVar_Result = 2;
    }
    else if (GetTrainerTowerRecordTime(&TRAINER_TOWER.bestTime) > TRAINER_TOWER.timer)
    {
        SetTrainerTowerRecordTime(&TRAINER_TOWER.bestTime, TRAINER_TOWER.timer);
        gSpecialVar_Result = 0;
    }
    else
    {
        gSpecialVar_Result = 1;
    }

    TRAINER_TOWER.checkedFinalTime = TRUE;
#elif IS_WAYFARER
    struct WayfarerSeviiTrainerTowerRecords *records = WayfarerSevii_GetTrainerTowerRecords();

    gSpecialVar_Result = 2;
    if (!sWayfarerTrainerTowerRun.active || records == NULL)
        return;
    ClearTrainerHillVBlankCounter();
    if (!sWayfarerTrainerTowerRun.timeChecked)
    {
        if (sWayfarerTrainerTowerRun.timer > TRAINER_TOWER_MAX_TIME)
            sWayfarerTrainerTowerRun.timer = TRAINER_TOWER_MAX_TIME;
        gSpecialVar_Result = WayfarerTrainerTowerRecordTime(records, sWayfarerTrainerTowerRun.challengeType,
                                                             sWayfarerTrainerTowerRun.timer) ? 0 : 1;
        sWayfarerTrainerTowerRun.timeChecked = TRUE;
    }
#else
    gSpecialVar_Result = 0;
#endif //FREE_TRAINER_TOWER
}

static void TrainerTowerResumeTimer(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    if (!TRAINER_TOWER.spokeToOwner)
    {
        if (TRAINER_TOWER.timer >= TRAINER_TOWER_MAX_TIME)
            TRAINER_TOWER.timer = TRAINER_TOWER_MAX_TIME;
        else
            SetTrainerHillVBlankCounter(&TRAINER_TOWER.timer);
    }
#endif //FREE_TRAINER_TOWER
#if IS_WAYFARER
    if (sWayfarerTrainerTowerRun.active && !sWayfarerTrainerTowerRun.ownerSpoken)
    {
        if (sWayfarerTrainerTowerRun.timer >= TRAINER_TOWER_MAX_TIME)
            sWayfarerTrainerTowerRun.timer = TRAINER_TOWER_MAX_TIME;
        else
            SetTrainerHillVBlankCounter(&sWayfarerTrainerTowerRun.timer);
    }
#endif
}

static void TrainerTowerSetPlayerLost(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    TRAINER_TOWER.hasLost = TRUE;
#elif IS_WAYFARER
    if (sWayfarerTrainerTowerRun.active)
    {
        TrainerTowerRestoreAndCloseRun();
        sWayfarerTrainerTowerRun.lossPending = TRUE;
    }
#endif //FREE_TRAINER_TOWER
}

static void GetTrainerTowerChallengeStatus(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    if (TRAINER_TOWER.hasLost)
    {
        TRAINER_TOWER.hasLost = FALSE;
        gSpecialVar_Result = TT_CHALLENGE_STATUS_LOST;
    }
    else if (TRAINER_TOWER.unkA_4)
    {
        TRAINER_TOWER.unkA_4 = FALSE;
        gSpecialVar_Result = TT_CHALLENGE_STATUS_UNK;
    }
    else
    {
        gSpecialVar_Result = TT_CHALLENGE_STATUS_NORMAL;
    }
#else
    gSpecialVar_Result = TT_CHALLENGE_STATUS_NORMAL;
#endif //FREE_TRAINER_TOWER
#if IS_WAYFARER
    if (sWayfarerTrainerTowerRun.lossPending)
    {
        sWayfarerTrainerTowerRun.lossPending = FALSE;
        gSpecialVar_Result = TT_CHALLENGE_STATUS_LOST;
    }
#endif
}

#define PRINT_TOWER_TIME(src) ({                                                           \
    u16 minutes;                                                                           \
    u8 seconds, centiseconds;                                                              \
                                                                                           \
    WayfarerTrainerTowerFormatTime((src), &minutes, &seconds, &centiseconds);             \
                                                                                           \
    ConvertIntToDecimalStringN(gStringVar1, minutes, STR_CONV_MODE_RIGHT_ALIGN, 2);        \
    ConvertIntToDecimalStringN(gStringVar2, seconds, STR_CONV_MODE_RIGHT_ALIGN, 2);        \
    ConvertIntToDecimalStringN(gStringVar3, centiseconds, STR_CONV_MODE_LEADING_ZEROS, 2); \
})

static void GetCurrentTime(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    if (TRAINER_TOWER.timer >= TRAINER_TOWER_MAX_TIME)
    {
        ClearTrainerHillVBlankCounter();
        TRAINER_TOWER.timer = TRAINER_TOWER_MAX_TIME;
    }

    PRINT_TOWER_TIME(TRAINER_TOWER.timer);
#elif IS_WAYFARER
    if (sWayfarerTrainerTowerRun.timer >= TRAINER_TOWER_MAX_TIME)
    {
        ClearTrainerHillVBlankCounter();
        sWayfarerTrainerTowerRun.timer = TRAINER_TOWER_MAX_TIME;
    }
    PRINT_TOWER_TIME(sWayfarerTrainerTowerRun.timer);
#else
    ConvertIntToDecimalStringN(gStringVar1, 0, STR_CONV_MODE_RIGHT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, 0, STR_CONV_MODE_RIGHT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar3, 0, STR_CONV_MODE_LEADING_ZEROS, 2);
#endif //FREE_TRAINER_TOWER
}

static void ShowResultsBoard(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    u8 windowId;
    s32 i;

    ValidateOrResetCurTrainerTowerRecord();
    windowId = AddWindow(sTimeBoardWindowTemplate);
    LoadMessageBoxAndBorderGfx();
    DrawStdWindowFrame(windowId, FALSE);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_TimeBoard, 74, 0, TEXT_SKIP_DRAW, NULL);

    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        PRINT_TOWER_TIME(GetTrainerTowerRecordTime(&TRAINER_TOWER.bestTime));

        StringExpandPlaceholders(gStringVar4, gText_XMinYZSec);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gTrainerTowerChallengeTypeTexts[i], 24, 36 + 20 * i, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 96, 46 + 20 * i, TEXT_SKIP_DRAW, NULL);
    }

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
    VarSet(VAR_TEMP_1, windowId);
#elif IS_WAYFARER
    u8 windowId;
    u8 i;
    struct WayfarerSeviiTrainerTowerRecords *records = WayfarerSevii_GetTrainerTowerRecords();

    if (records == NULL)
        return;
    windowId = AddWindow(sTimeBoardWindowTemplate);
    LoadMessageBoxAndBorderGfx();
    DrawStdWindowFrame(windowId, FALSE);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gText_TimeBoard, 74, 0, TEXT_SKIP_DRAW, NULL);
    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        if ((records->completedMask & (1 << i))
         && records->bestTime[i] > 0
         && records->bestTime[i] <= TRAINER_TOWER_MAX_TIME)
            PRINT_TOWER_TIME(records->bestTime[i]);
        else
            StringCopy(gStringVar4, sTextNoRecord);
        if ((records->completedMask & (1 << i))
         && records->bestTime[i] > 0
         && records->bestTime[i] <= TRAINER_TOWER_MAX_TIME)
            StringExpandPlaceholders(gStringVar4, gText_XMinYZSec);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gTrainerTowerChallengeTypeTexts[i], 24, 36 + 20 * i, TEXT_SKIP_DRAW, NULL);
        AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 96, 46 + 20 * i, TEXT_SKIP_DRAW, NULL);
    }
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
    VarSet(VAR_TEMP_1, windowId);
#endif //FREE_TRAINER_TOWER
}

static void CloseResultsBoard(void)
{
#if (FREE_TRAINER_TOWER == FALSE && IS_FRLG) || IS_WAYFARER
    u8 windowId = VarGet(VAR_TEMP_1);
    ClearStdWindowAndFrameToTransparent(windowId, TRUE);
    RemoveWindow(windowId);
#endif //FREE_TRAINER_TOWER
}

static void TrainerTowerCheckEligibility(void)
{
#if IS_WAYFARER
    u8 challengeType = gSpecialVar_0x8005;
    u8 neededMons = (challengeType == CHALLENGE_TYPE_DOUBLE || challengeType == CHALLENGE_TYPE_MIXED) ? 2 : 1;
    struct WayfarerSeviiTrainerTowerRecords *records = WayfarerSevii_GetTrainerTowerRecords();

    gSpecialVar_Result = challengeType < NUM_TOWER_CHALLENGE_TYPES
                      && !sWayfarerTrainerTowerRun.active
                      && records != NULL
                      && records->pendingPrize == ITEM_NONE
                      && WayfarerTrainerTowerGetUsablePartyCount() >= neededMons;
#else
    gSpecialVar_Result = TRUE;
#endif
}

static void TrainerTowerRestoreAndCloseRun(void)
{
#if IS_WAYFARER
    ClearTrainerHillVBlankCounter();
    if (sWayfarerTrainerTowerRun.snapshotValid)
    {
        memcpy(gPlayerParty, sWayfarerTrainerTowerRun.partySnapshot, sizeof(gPlayerParty));
        gPlayerPartyCount = sWayfarerTrainerTowerRun.partyCount;
    }
    sWayfarerTrainerTowerRun.active = FALSE;
    sWayfarerTrainerTowerRun.snapshotValid = FALSE;
    sWayfarerTrainerTowerRun.ownerSpoken = FALSE;
    sWayfarerTrainerTowerRun.timeChecked = FALSE;
    sWayfarerTrainerTowerRun.timer = 0;
    sWayfarerTrainerTowerRun.clearedFloors = 0;
    sWayfarerTrainerTowerRun.floorIdx = 0;
    sWayfarerTrainerTowerRun.knockoutOpponent = 0;
    sWayfarerTrainerTowerRun.localSetId = 0;
    sWayfarerTrainerTowerRun.normalizedLevel = 0;
#endif
}

static void TrainerTowerAbandonChallenge(void)
{
#if IS_WAYFARER
    if (sWayfarerTrainerTowerRun.active)
        TrainerTowerRestoreAndCloseRun();
    gSpecialVar_Result = TRUE;
#else
    gSpecialVar_Result = FALSE;
#endif
}

static void TrainerTowerCheckPendingPrize(void)
{
#if IS_WAYFARER
    struct WayfarerSeviiTrainerTowerRecords *records = WayfarerSevii_GetTrainerTowerRecords();
    SyncTrainerTowerPendingPrize(records);
    gSpecialVar_Result = records != NULL && records->pendingPrize != ITEM_NONE;
#else
    gSpecialVar_Result = FALSE;
#endif
}

static void TrainerTowerClaimPendingPrize(void)
{
#if IS_WAYFARER
    struct WayfarerSeviiTrainerTowerRecords *records = WayfarerSevii_GetTrainerTowerRecords();

    gSpecialVar_Result = FALSE;
    if (records == NULL || records->pendingPrize == ITEM_NONE)
        return;
    // The lobby wrapper performs giveitem first. This finalizer is deliberately
    // non-transactional: it can never make a second Bag insertion.
    records->pendingPrize = ITEM_NONE;
    SyncTrainerTowerPendingPrize(records);
    gSpecialVar_Result = TRUE;
#else
    gSpecialVar_Result = FALSE;
#endif
}

static void TrainerTowerCheckActive(void)
{
    gSpecialVar_Result = WayfarerTrainerTowerIsChallengeActive();
}

static void TrainerTowerGetDoublesEligiblity(void)
{
#if IS_WAYFARER
    gSpecialVar_Result = WayfarerTrainerTowerGetUsablePartyCount() >= 2;
#else
    gSpecialVar_Result = GetMonsStateToDoubles();
#endif
}


static void TrainerTowerGetNumFloors(void)
{
    if (sTrainerTowerState->data.numFloors != sTrainerTowerState->data.floors[0].floorIdx)
    {
        ConvertIntToDecimalStringN(gStringVar1, sTrainerTowerState->data.numFloors, STR_CONV_MODE_LEFT_ALIGN, 1);
        gSpecialVar_Result = TRUE;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

// Dummied? equivalent to gSpecialVar_Result = FALSE
// If it were to return TRUE the player would be warped back to the lobby
static void ShouldWarpToCounter(void)
{
    if (gMapHeader.mapLayoutId == LAYOUT_TRAINER_TOWER_LOBBY && VarGet(VAR_MAP_SCENE_TRAINER_TOWER) == 0)
        gSpecialVar_Result = FALSE;
    else
        gSpecialVar_Result = FALSE;
}

static void PlayTrainerTowerEncounterMusic(void)
{
    s32 i;
    u16 idx = VarGet(VAR_TEMP_1);
    u8 facilityClass = CURR_FLOOR.trainers[idx].facilityClass;

    for (i = 0; i < ARRAY_COUNT(sTrainerEncounterMusicLUT); i++)
    {
        if (sTrainerEncounterMusicLUT[i].facilityClass == gFacilityClassToTrainerClass[facilityClass])
            break;
    }

    if (i != ARRAY_COUNT(sTrainerEncounterMusicLUT))
    {
        idx = sTrainerEncounterMusicLUT[i].musicId;
    }
    else
    {
        idx = 0;
    }
    PlayNewMapMusic(sTrainerTowerEncounterMusic[idx]);
}

static void HasSpokenToOwner(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    gSpecialVar_Result = TRAINER_TOWER.spokeToOwner;
#elif IS_WAYFARER
    gSpecialVar_Result = sWayfarerTrainerTowerRun.ownerSpoken;
#endif //FREE_TRAINER_TOWER
}

static void BuildEnemyParty(void)
{
    u16 trainerIdx = VarGet(VAR_TEMP_1);
    s32 level = GetPartyMaxLevel();
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    u8 floorIdx = TRAINER_TOWER.floorsCleared;
#elif IS_WAYFARER
    u8 floorIdx = sWayfarerTrainerTowerRun.floorIdx;
#else
    u8 floorIdx = gMapHeader.mapLayoutId - LAYOUT_TRAINER_TOWER_1F;
#endif //FREE_TRAINER_TOWER
    s32 i;
    u8 monIdx;

#if IS_WAYFARER
    if (sWayfarerTrainerTowerRun.active)
        level = sWayfarerTrainerTowerRun.normalizedLevel;
#endif

    ZeroEnemyPartyMons();

    switch (CURR_FLOOR.challengeType)
    {
    case CHALLENGE_TYPE_SINGLE:
    default:
        for (i = 0; i < 2; i++)
        {
            monIdx = sSingleBattleChallengeMonIdxs[floorIdx][i];
            {
                struct BattleTowerPokemon mon = CURR_FLOOR.trainers[trainerIdx].mons[monIdx];
                mon.level = level;
                CreateBattleTowerMon(&gEnemyParty[i], &mon);
            }
        }
        break;
    case CHALLENGE_TYPE_DOUBLE:
        monIdx = sDoubleBattleChallengeMonIdxs[floorIdx][0];
        {
            struct BattleTowerPokemon mon = CURR_FLOOR.trainers[0].mons[monIdx];
            mon.level = level;
            CreateBattleTowerMon(&gEnemyParty[0], &mon);
        }

        monIdx = sDoubleBattleChallengeMonIdxs[floorIdx][1];
        {
            struct BattleTowerPokemon mon = CURR_FLOOR.trainers[1].mons[monIdx];
            mon.level = level;
            CreateBattleTowerMon(&gEnemyParty[1], &mon);
        }
        break;
    case CHALLENGE_TYPE_KNOCKOUT:
        monIdx = sKnockoutChallengeMonIdxs[floorIdx][trainerIdx];
        {
            struct BattleTowerPokemon mon = CURR_FLOOR.trainers[trainerIdx].mons[monIdx];
            mon.level = level;
            CreateBattleTowerMon(&gEnemyParty[0], &mon);
        }
#if IS_WAYFARER
        sWayfarerTrainerTowerRun.knockoutOpponent = trainerIdx;
#endif
        break;
    }
}

static s32 GetPartyMaxLevel(void)
{
    s32 topLevel = 0;
    s32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL) != SPECIES_NONE
         && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG, NULL) != SPECIES_EGG
         && GetMonData(&gPlayerParty[i], MON_DATA_HP, NULL) != 0)
        {
            s32 currLevel = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL, NULL);
            if (currLevel > topLevel)
                topLevel = currLevel;
        }
    }

#if IS_WAYFARER
    return WayfarerTrainerTowerNormalizeLevel(topLevel);
#else
    return topLevel;
#endif
}

static void ValidateOrResetCurTrainerTowerRecord(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    if (TRAINER_TOWER.unk9 != sTrainerTowerState->data.id)
    {
        TRAINER_TOWER.unk9 = sTrainerTowerState->data.id;
        SetTrainerTowerRecordTime(&TRAINER_TOWER.bestTime, TRAINER_TOWER_MAX_TIME);
        TRAINER_TOWER.receivedPrize = FALSE;
    }
#endif //FREE_TRAINER_TOWER
}

void PrintTrainerTowerRecords(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    s32 i;
    u8 windowId = 0;

    SetUpTrainerTowerDataStruct();
    FillWindowPixelRect(0, PIXEL_FILL(0), 0, 0, 216, 144);
    ValidateOrResetCurTrainerTowerRecord();
    AddTextPrinterParameterized3(0, FONT_NORMAL, 0x4a, 0, sTextColors, 0, gText_TimeBoard);

    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        PRINT_TOWER_TIME(GetTrainerTowerRecordTime(&gSaveBlock1Ptr->trainerTower[i].bestTime));
        StringExpandPlaceholders(gStringVar4, gText_XMinYZSec);
        AddTextPrinterParameterized3(windowId, FONT_NORMAL, 0x18, 0x24 + 0x14 * i, sTextColors, 0, gTrainerTowerChallengeTypeTexts[i]);
        AddTextPrinterParameterized3(windowId, FONT_NORMAL, 0x60, 0x24 + 0x14 * i, sTextColors, 0, gStringVar4);
    }

    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);
    FreeTrainerTowerDataStruct();
#elif IS_WAYFARER
    u8 i;
    struct WayfarerSeviiTrainerTowerRecords *records = WayfarerSevii_GetTrainerTowerRecords();

    if (records == NULL)
        return;
    FillWindowPixelRect(0, PIXEL_FILL(0), 0, 0, 216, 144);
    AddTextPrinterParameterized3(0, FONT_NORMAL, 0x4a, 0, sTextColors, 0, gText_TimeBoard);
    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        if ((records->completedMask & (1 << i))
         && records->bestTime[i] > 0
         && records->bestTime[i] <= TRAINER_TOWER_MAX_TIME)
        {
            PRINT_TOWER_TIME(records->bestTime[i]);
            StringExpandPlaceholders(gStringVar4, gText_XMinYZSec);
        }
        else
            StringCopy(gStringVar4, sTextNoRecord);
        AddTextPrinterParameterized3(0, FONT_NORMAL, 0x18, 0x24 + 0x14 * i, sTextColors, 0, gTrainerTowerChallengeTypeTexts[i]);
        AddTextPrinterParameterized3(0, FONT_NORMAL, 0x60, 0x24 + 0x14 * i, sTextColors, 0, gStringVar4);
    }
    PutWindowTilemap(0);
    CopyWindowToVram(0, COPYWIN_FULL);
#endif //FREE_TRAINER_TOWER
}

#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
static u32 GetTrainerTowerRecordTime(u32 *counter)
{
    return *counter;
}

static void SetTrainerTowerRecordTime(u32 *counter, u32 value)
{
    *counter = value;
}
#endif //FREE_TRAINER_TOWER

void ResetTrainerTowerResults(void)
{
#if FREE_TRAINER_TOWER == FALSE && IS_FRLG
    s32 i;

    for (i = 0; i < NUM_TOWER_CHALLENGE_TYPES; i++)
    {
        SetTrainerTowerRecordTime(&gSaveBlock1Ptr->trainerTower[i].bestTime, TRAINER_TOWER_MAX_TIME);
    }
#endif //FREE_TRAINER_TOWER
}
