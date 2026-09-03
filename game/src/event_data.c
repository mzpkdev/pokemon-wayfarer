#include "global.h"
#include "event_data.h"
#include "pokedex.h"
#include "regions.h"
#include "wayfarer_persistence.h"
#if IS_WAYFARER
#include "data/wayfarer_common_source_constants.h"
#endif

#define SPECIAL_FLAGS_SIZE  (NUM_SPECIAL_FLAGS / 8)  // 8 flags per byte
#define TEMP_FLAGS_SIZE     (NUM_TEMP_FLAGS / 8)
#define DAILY_FLAGS_SIZE    ((NUM_DAILY_FLAGS + 7) / 8)
#define TEMP_VARS_SIZE      (NUM_TEMP_VARS * 2)      // 1/2 var per byte

EWRAM_DATA u16 gSpecialVar_0x8000 = 0;
EWRAM_DATA u16 gSpecialVar_0x8001 = 0;
EWRAM_DATA u16 gSpecialVar_0x8002 = 0;
EWRAM_DATA u16 gSpecialVar_0x8003 = 0;
EWRAM_DATA u16 gSpecialVar_0x8004 = 0;
EWRAM_DATA u16 gSpecialVar_0x8005 = 0;
EWRAM_DATA u16 gSpecialVar_0x8006 = 0;
EWRAM_DATA u16 gSpecialVar_0x8007 = 0;
EWRAM_DATA u16 gSpecialVar_0x8008 = 0;
EWRAM_DATA u16 gSpecialVar_0x8009 = 0;
EWRAM_DATA u16 gSpecialVar_0x800A = 0;
EWRAM_DATA u16 gSpecialVar_0x800B = 0;
EWRAM_DATA u16 gSpecialVar_Result = 0;
EWRAM_DATA u16 gSpecialVar_LastTalked = 0;
EWRAM_DATA u16 gSpecialVar_Facing = 0;
EWRAM_DATA u16 gSpecialVar_MonBoxId = 0;
EWRAM_DATA u16 gSpecialVar_MonBoxPos = 0;
EWRAM_DATA u16 gSpecialVar_Unused_0x8014 = 0;
EWRAM_DATA static u8 sSpecialFlags[SPECIAL_FLAGS_SIZE] = {0};

#if TESTING
#define TEST_FLAGS_SIZE     1
#define TEST_VARS_SIZE      8
EWRAM_DATA static u8 sTestFlags[TEST_FLAGS_SIZE] = {0};
EWRAM_DATA static u16 sTestVars[TEST_VARS_SIZE] = {0};
#endif // TESTING

extern u16 *const gSpecialVars[];

#if IS_WAYFARER
static u16 ResolveWayfarerCommonFlagId(u16 id)
{
    u16 index;

    if (!IS_WAYFARER_COMMON_FLAG_ID(id))
        return id;
    index = WAYFARER_COMMON_ID_INDEX(id);
    if (index >= WAYFARER_COMMON_FLAG_COUNT)
        return 0;
    return sWayfarerCommonFlagIds[index][WayfarerIsCurrentMapHoennSource()];
}

static u16 ResolveWayfarerCommonVarId(u16 id)
{
    u16 index;

    if (!IS_WAYFARER_COMMON_VAR_ID(id))
        return id;
    index = WAYFARER_COMMON_ID_INDEX(id);
    if (index >= WAYFARER_COMMON_VAR_COUNT)
        return 0;
    return sWayfarerCommonVarIds[index][WayfarerIsCurrentMapHoennSource()];
}

static u8 *GetHoennFlagPointer(u16 id, u8 *bit)
{
    u16 sourceId = HOENN_FLAG_SOURCE_ID(id);
    u16 offset;

    if (sourceId >= WAYFARER_HOENN_BADGE_FLAG_START
     && sourceId < WAYFARER_HOENN_BADGE_FLAG_START + WAYFARER_HOENN_BADGE_COUNT)
    {
        *bit = sourceId - WAYFARER_HOENN_BADGE_FLAG_START;
        return &gSaveBlock3Ptr->wayfarerHoenn.badges;
    }

    if (sourceId >= WAYFARER_HOENN_VISITED_FLAG_START
     && sourceId < WAYFARER_HOENN_VISITED_FLAG_START + WAYFARER_HOENN_VISITED_COUNT)
    {
        offset = sourceId - WAYFARER_HOENN_VISITED_FLAG_START;
        *bit = offset & 7;
        return &gSaveBlock3Ptr->wayfarerHoenn.visitedLocations[offset / 8];
    }

    if (sourceId == WAYFARER_HOENN_GAME_CLEAR_FLAG)
    {
        *bit = 0;
        return &gSaveBlock3Ptr->wayfarerHoenn.leagueFlags;
    }

    if (sourceId == WAYFARER_HOENN_CHAMPION_FLAG)
    {
        *bit = 1;
        return &gSaveBlock3Ptr->wayfarerHoenn.leagueFlags;
    }

    if (sourceId >= WAYFARER_HOENN_FLAGS_LOW_START && sourceId <= WAYFARER_HOENN_FLAGS_LOW_END)
        offset = sourceId - WAYFARER_HOENN_FLAGS_LOW_START;
    else if (sourceId >= WAYFARER_HOENN_FLAGS_HIGH_START && sourceId <= WAYFARER_HOENN_FLAGS_HIGH_END)
        offset = WAYFARER_HOENN_FLAGS_LOW_COUNT + sourceId - WAYFARER_HOENN_FLAGS_HIGH_START;
    else
        return NULL;

    *bit = offset & 7;
    return &gSaveBlock3Ptr->wayfarerHoenn.persistentFlags[offset / 8];
}
#endif

// Capped at 8: every consumer indexes 8-entry tables with the count this yields.
const u16 gBadgeFlags[NUM_BADGES_CAPPED] =
{
    FLAG_BADGE01_GET,
    FLAG_BADGE02_GET,
    FLAG_BADGE03_GET,
    FLAG_BADGE04_GET,
    FLAG_BADGE05_GET,
    FLAG_BADGE06_GET,
    FLAG_BADGE07_GET,
    FLAG_BADGE08_GET,
};

void InitEventData(void)
{
    memset(gSaveBlock1Ptr->flags, 0, sizeof(gSaveBlock1Ptr->flags));
    memset(gSaveBlock1Ptr->vars, 0, sizeof(gSaveBlock1Ptr->vars));
    memset(sSpecialFlags, 0, sizeof(sSpecialFlags));
}

void ClearTempFieldEventData(void)
{
    memset(&gSaveBlock1Ptr->flags[TEMP_FLAGS_START / 8], 0, TEMP_FLAGS_SIZE);
    memset(&gSaveBlock1Ptr->vars[TEMP_VARS_START - VARS_START], 0, TEMP_VARS_SIZE);
#if IS_WAYFARER
    memset(gSaveBlock3Ptr->wayfarerHoenn.vars, 0, TEMP_VARS_SIZE);
#endif
    FlagClear(FLAG_SYS_ENC_UP_ITEM);
    FlagClear(FLAG_SYS_ENC_DOWN_ITEM);
    WayfarerFieldMoveFlagClear(FLAG_SYS_USE_STRENGTH);
    FlagClear(FLAG_SYS_CTRL_OBJ_DELETE);
    FlagClear(FLAG_NURSE_UNION_ROOM_REMINDER);
}

void ClearDailyFlags(void)
{
#if IS_HNS
    u16 i;
    for (i = DAILY_FLAGS_START; i <= DAILY_FLAGS_END; i++)
        FlagClear(i);
#if IS_WAYFARER
    for (i = WAYFARER_HOENN_DAILY_FLAGS_START; i <= WAYFARER_HOENN_DAILY_FLAGS_END; i++)
        FlagClear(HOENN_FLAG_ID(i));
#endif
#else
    memset(&gSaveBlock1Ptr->flags[DAILY_FLAGS_START / 8], 0, DAILY_FLAGS_SIZE);
#endif
}

void DisableNationalPokedex(void)
{
    u16 *nationalDexVar = GetVarPointer(VAR_NATIONAL_DEX);
    gSaveBlock2Ptr->pokedex.nationalMagic = 0;
    *nationalDexVar = 0;
    FlagClear(FLAG_SYS_NATIONAL_DEX);
}

void EnableNationalPokedex(void)
{
    u16 *nationalDexVar = GetVarPointer(VAR_NATIONAL_DEX);
    gSaveBlock2Ptr->pokedex.nationalMagic = 0xDA;
    *nationalDexVar = 0x302;
    FlagSet(FLAG_SYS_NATIONAL_DEX);
    gSaveBlock2Ptr->pokedex.mode = DEX_MODE_NATIONAL;
    gSaveBlock2Ptr->pokedex.order = 0;
    ResetPokedexScrollPositions();
}

bool32 IsNationalPokedexEnabled(void)
{
    if (gSaveBlock2Ptr->pokedex.nationalMagic == 0xDA && VarGet(VAR_NATIONAL_DEX) == 0x302 && FlagGet(FLAG_SYS_NATIONAL_DEX))
        return TRUE;
    else
        return FALSE;
}

void DisableMysteryEvent(void)
{
    FlagClear(FLAG_SYS_MYSTERY_EVENT_ENABLE);
}

void EnableMysteryEvent(void)
{
    FlagSet(FLAG_SYS_MYSTERY_EVENT_ENABLE);
}

bool32 IsMysteryEventEnabled(void)
{
    return FlagGet(FLAG_SYS_MYSTERY_EVENT_ENABLE);
}

void DisableMysteryGift(void)
{
    FlagClear(FLAG_SYS_MYSTERY_GIFT_ENABLE);
}

void EnableMysteryGift(void)
{
    FlagSet(FLAG_SYS_MYSTERY_GIFT_ENABLE);
}

bool32 IsMysteryGiftEnabled(void)
{
    return FlagGet(FLAG_SYS_MYSTERY_GIFT_ENABLE);
}

void ClearMysteryGiftFlags(void)
{
    FlagClear(FLAG_MYSTERY_GIFT_DONE);
    FlagClear(FLAG_MYSTERY_GIFT_1);
    FlagClear(FLAG_MYSTERY_GIFT_2);
    FlagClear(FLAG_MYSTERY_GIFT_3);
    FlagClear(FLAG_MYSTERY_GIFT_4);
    FlagClear(FLAG_MYSTERY_GIFT_5);
    FlagClear(FLAG_MYSTERY_GIFT_6);
    FlagClear(FLAG_MYSTERY_GIFT_7);
    FlagClear(FLAG_MYSTERY_GIFT_8);
    FlagClear(FLAG_MYSTERY_GIFT_9);
    FlagClear(FLAG_MYSTERY_GIFT_10);
    FlagClear(FLAG_MYSTERY_GIFT_11);
    FlagClear(FLAG_MYSTERY_GIFT_12);
    FlagClear(FLAG_MYSTERY_GIFT_13);
    FlagClear(FLAG_MYSTERY_GIFT_14);
    FlagClear(FLAG_MYSTERY_GIFT_15);
}

void ClearMysteryGiftVars(void)
{
    VarSet(VAR_GIFT_PICHU_SLOT, 0);
    VarSet(VAR_GIFT_UNUSED_1, 0);
    VarSet(VAR_GIFT_UNUSED_2, 0);
    VarSet(VAR_GIFT_UNUSED_3, 0);
    VarSet(VAR_GIFT_UNUSED_4, 0);
    VarSet(VAR_GIFT_UNUSED_5, 0);
    VarSet(VAR_GIFT_UNUSED_6, 0);
    VarSet(VAR_GIFT_UNUSED_7, 0);
}

void DisableResetRTC(void)
{
    VarSet(VAR_RESET_RTC_ENABLE, 0);
    FlagClear(FLAG_SYS_RESET_RTC_ENABLE);
}

void EnableResetRTC(void)
{
    VarSet(VAR_RESET_RTC_ENABLE, 0x920);
    FlagSet(FLAG_SYS_RESET_RTC_ENABLE);
}

bool32 CanResetRTC(void)
{
    if (FlagGet(FLAG_SYS_RESET_RTC_ENABLE) && VarGet(VAR_RESET_RTC_ENABLE) == 0x920)
        return TRUE;
    else
        return FALSE;
}

u16 *GetVarPointer(u16 id)
{
#if IS_WAYFARER
    id = ResolveWayfarerCommonVarId(id);
    if (IS_HOENN_VAR_ID(id))
    {
        u16 sourceId = HOENN_VAR_SOURCE_ID(id);
        if (sourceId >= VARS_START && sourceId <= VARS_END)
            return &gSaveBlock3Ptr->wayfarerHoenn.vars[sourceId - VARS_START];
        return NULL;
    }
#endif

    if (id < VARS_START)
        return NULL;
    else if (id < SPECIAL_VARS_START)
        return &gSaveBlock1Ptr->vars[id - VARS_START];
#if TESTING
    else if (id >= TESTING_VARS_START)
        return &sTestVars[id - TESTING_VARS_START];
#endif // TESTING
    else
        return gSpecialVars[id - SPECIAL_VARS_START];
}

u16 VarGet(u16 id)
{
    u16 *ptr = GetVarPointer(id);
    if (!ptr)
        return id;
    return *ptr;
}

u16 VarGetIfExist(u16 id)
{
    u16 *ptr = GetVarPointer(id);
    if (!ptr)
        return 65535;
    return *ptr;
}

bool8 VarSet(u16 id, u16 value)
{
    u16 *ptr = GetVarPointer(id);
    if (!ptr)
        return FALSE;
    *ptr = value;
    return TRUE;
}

u16 VarGetObjectEventGraphicsId(u8 id)
{
    return VarGet(VAR_OBJ_GFX_ID_0 + id);
}

u8 *GetFlagPointer(u16 id)
{
#if IS_WAYFARER
    id = ResolveWayfarerCommonFlagId(id);
    if (IS_HOENN_FLAG_ID(id))
    {
        u8 bit;
        return GetHoennFlagPointer(id, &bit);
    }
#endif

    if (id == 0)
        return NULL;
    else if (id < SPECIAL_FLAGS_START)
        return &gSaveBlock1Ptr->flags[id / 8];
#if TESTING
    else if (id >= TESTING_FLAGS_START)
        return &sTestFlags[(id - TESTING_FLAGS_START) / 8];
#endif // TESTING
    else
        return &sSpecialFlags[(id - SPECIAL_FLAGS_START) / 8];
}

u8 FlagSet(u16 id)
{
#if IS_WAYFARER
    id = ResolveWayfarerCommonFlagId(id);
#endif
    u8 *ptr = GetFlagPointer(id);
    u8 bit = id & 7;
#if IS_WAYFARER
    if (IS_HOENN_FLAG_ID(id))
        ptr = GetHoennFlagPointer(id, &bit);
#endif
    if (ptr)
        *ptr |= 1 << bit;
    return 0;
}

u8 FlagToggle(u16 id)
{
#if IS_WAYFARER
    id = ResolveWayfarerCommonFlagId(id);
#endif
    u8 *ptr = GetFlagPointer(id);
    u8 bit = id & 7;
#if IS_WAYFARER
    if (IS_HOENN_FLAG_ID(id))
        ptr = GetHoennFlagPointer(id, &bit);
#endif
    if (ptr)
        *ptr ^= 1 << bit;
    return 0;
}

u8 FlagClear(u16 id)
{
#if IS_WAYFARER
    id = ResolveWayfarerCommonFlagId(id);
#endif
    u8 *ptr = GetFlagPointer(id);
    u8 bit = id & 7;
#if IS_WAYFARER
    if (IS_HOENN_FLAG_ID(id))
        ptr = GetHoennFlagPointer(id, &bit);
#endif
    if (ptr)
        *ptr &= ~(1 << bit);
    return 0;
}

bool8 FlagGet(u16 id)
{
#if IS_WAYFARER
    id = ResolveWayfarerCommonFlagId(id);
#endif
    u8 *ptr = GetFlagPointer(id);
    u8 bit = id & 7;
#if IS_WAYFARER
    if (IS_HOENN_FLAG_ID(id))
        ptr = GetHoennFlagPointer(id, &bit);
#endif

    if (!ptr)
        return FALSE;

    if (!(((*ptr) >> bit) & 1))
        return FALSE;

    return TRUE;
}
