#include "global.h"
#include "wayfarer_origin.h"
#if IS_WAYFARER
#include "event_data.h"
#include "field_screen_effect.h"
#include "field_special_scene.h"
#include "data/map_group_count.h"
#include "heal_location.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokedex.h"
#include "script.h"
#include "wayfarer_persistence.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"
#include "constants/pokedex.h"

extern const u8 WayfarerHoennOrigin_EventScript_InitializeBaseline[];

static EWRAM_DATA u16 sCandidateOrigin = ORIGIN_NONE;
static EWRAM_DATA u16 sConfirmedOrigin = ORIGIN_NONE;
#if TESTING
static EWRAM_DATA const struct WayfarerOriginProfile *sTestProfile = NULL;
#endif

static u8 NewBarkRecovery(void) { return HEAL_LOCATION_NEW_BARK_TOWN_PLAYERS_HOUSE_2F_HNS; }
static u8 LittlerootRecovery(void)
{
    return gSaveBlock2Ptr->playerGender == FEMALE
        ? HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F
        : HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F;
}
static bool8 NewBarkAqua(void) { return VarGet(VAR_SSAQUA_STATE) >= 8; }
static bool8 LittlerootAqua(void) { return FlagGet(FLAG_HOENN_STARTER_RECEIVED); }
static bool8 NoTicket(void) { return FALSE; }
static void InitializeNewBark(void) { Dex_SetActiveRegion(DEX_REGION_JOHTO); }
static void InitializeLittleroot(void)
{
    Dex_SetActiveRegion(DEX_REGION_HOENN);
    VarSet(VAR_NEWBARK_TOWN_STATE, 2);
    VarSet(VAR_NEWBARKTOWN_LABSTATE, 0);
    FlagSet(FLAG_HIDE_LAB_POLICEMAN);
    FlagClear(FLAG_HIDE_SILVER_NEWBARKTOWN);
    RunScriptImmediately(WayfarerHoennOrigin_EventScript_InitializeBaseline);
    WayfarerSetHoennStateInitialized(TRUE);
}

static const struct WayfarerOriginProfile sOriginProfiles[] =
{
    {
        .id = ORIGIN_NEW_BARK, .selectionLabel = COMPOUND_STRING("JOHTO"), .entryRegion = REGION_JOHTO,
        .mapGroup = MAP_GROUP(MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_2F_HNS),
        .mapNum = MAP_NUM(MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_2F_HNS), .warpId = 1,
        .scenePolicies = {ORIGIN_SCENE_NATIVE, ORIGIN_SCENE_NATIVE, ORIGIN_SCENE_VISITOR, ORIGIN_SCENE_VISITOR},
        .initialRecovery = NewBarkRecovery, .initialize = InitializeNewBark,
        .openingCallback = FieldCB_WarpExitFadeFromBlack,
        .regularAqua = NewBarkAqua, .slateportTicket = NoTicket, .maidenVoyage = TRUE,
    },
    {
        .id = ORIGIN_LITTLEROOT, .selectionLabel = COMPOUND_STRING("HOENN"), .entryRegion = REGION_HOENN,
        .mapGroup = MAP_GROUP(MAP_INSIDE_OF_TRUCK), .mapNum = MAP_NUM(MAP_INSIDE_OF_TRUCK),
        .warpId = WARP_ID_NONE, .x = -1, .y = -1,
        .scenePolicies = {ORIGIN_SCENE_VISITOR, ORIGIN_SCENE_VISITOR, ORIGIN_SCENE_NATIVE, ORIGIN_SCENE_NATIVE},
        .initialRecovery = LittlerootRecovery, .initialize = InitializeLittleroot,
        .openingCallback = ExecuteTruckSequence,
        .regularAqua = LittlerootAqua, .slateportTicket = LittlerootAqua, .maidenVoyage = FALSE,
    },
};

const struct WayfarerOriginProfile *WayfarerGetOriginProfile(u16 id)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sOriginProfiles); i++)
        if (sOriginProfiles[i].id == id)
            return &sOriginProfiles[i];
#if TESTING
    if (sTestProfile != NULL && sTestProfile->id == id)
        return sTestProfile;
#endif
    return NULL;
}

static bool8 IsProfileValid(const struct WayfarerOriginProfile *profile)
{
    u32 i;
    const struct MapHeader *map;
    s16 x, y;
    if (profile == NULL || profile->id == ORIGIN_NONE || profile->initialRecovery == NULL
     || profile->initialize == NULL || profile->openingCallback == NULL || profile->regularAqua == NULL || profile->slateportTicket == NULL
     || GetHealLocation(profile->initialRecovery()) == NULL
     || (profile->entryRegion != REGION_JOHTO && profile->entryRegion != REGION_HOENN && profile->entryRegion != REGION_KANTO)
     || profile->mapGroup < 0 || profile->mapGroup >= MAP_GROUPS_COUNT
     || profile->mapNum < 0 || profile->mapNum >= MAP_GROUP_COUNT[profile->mapGroup]
     || WayfarerGetRegionForMap(profile->mapGroup, profile->mapNum) != profile->entryRegion)
        return FALSE;
    map = Overworld_GetMapHeaderByGroupAndId(profile->mapGroup, profile->mapNum);
    if (map == NULL || map->mapLayout == NULL || map->events == NULL || map->mapLayout->map == NULL)
        return FALSE;
    if (profile->warpId != WARP_ID_NONE)
    {
        if (profile->warpId < 0 || profile->warpId >= map->events->warpCount)
            return FALSE;
        x = map->events->warps[profile->warpId].x;
        y = map->events->warps[profile->warpId].y;
    }
    else if (profile->x == -1 && profile->y == -1)
    {
        x = map->mapLayout->width / 2;
        y = map->mapLayout->height / 2;
    }
    else
    {
        if (profile->x > 127 || profile->y > 127)
            return FALSE;
        x = profile->x;
        y = profile->y;
    }
    if (x < 0 || y < 0 || x >= map->mapLayout->width || y >= map->mapLayout->height)
        return FALSE;
    if (UNPACK_COLLISION(map->mapLayout->map[y * map->mapLayout->width + x]) != 0)
        return FALSE;
    for (i = 0; i < ORIGIN_SCENES_COUNT; i++)
        if (profile->scenePolicies[i] > ORIGIN_SCENE_AUTHORED
         || (profile->scenePolicies[i] == ORIGIN_SCENE_AUTHORED && profile->authoredScene == NULL))
            return FALSE;
    return TRUE;
}

void WayfarerResetPendingOrigin(void)
{
    sCandidateOrigin = ORIGIN_NEW_BARK;
    sConfirmedOrigin = ORIGIN_NONE;
}
bool8 WayfarerSetPendingOriginCandidate(u16 id)
{
    sConfirmedOrigin = ORIGIN_NONE;
    if (!IsProfileValid(WayfarerGetOriginProfile(id)))
        return FALSE;
    sCandidateOrigin = id;
    return TRUE;
}
u16 WayfarerGetPendingOriginCandidate(void) { return sCandidateOrigin; }
bool8 WayfarerConfirmPendingOrigin(u16 id)
{
    sConfirmedOrigin = ORIGIN_NONE;
    if (!WayfarerSetPendingOriginCandidate(id))
        return FALSE;
    sConfirmedOrigin = id;
    return TRUE;
}
u16 WayfarerGetConfirmedPendingOrigin(void) { return sConfirmedOrigin; }
u16 WayfarerGetStartingOriginId(void) { return gSaveBlock3Ptr->wayfarerHoenn.startingOriginId; }

bool8 WayfarerInitializeOrigin(u16 id)
{
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(id);
    if (WayfarerGetStartingOriginId() != ORIGIN_NONE || !IsProfileValid(profile))
        return FALSE;
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = id;
    profile->initialize();
    gSaveBlock3Ptr->wayfarerHoenn.fallbackHealLocation = profile->initialRecovery();
    SetLastHealLocationWarp(gSaveBlock3Ptr->wayfarerHoenn.fallbackHealLocation);
    gSaveBlock3Ptr->wayfarerHoenn.visitedRegions = 1 << profile->entryRegion;
    WayfarerSetSavedCurrentRegion(profile->entryRegion);
    SetWarpDestination(profile->mapGroup, profile->mapNum, profile->warpId, profile->x, profile->y);
    sConfirmedOrigin = ORIGIN_NONE;
    return TRUE;
}
u8 WayfarerGetOriginScenePolicy(u8 scene)
{
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(WayfarerGetStartingOriginId());
    if (profile == NULL || scene >= ORIGIN_SCENES_COUNT)
        return ORIGIN_SCENE_AUTHORED;
    return profile->scenePolicies[scene];
}
void WayfarerEnterOriginOpening(void)
{
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(WayfarerGetStartingOriginId());
    if (profile != NULL)
        profile->openingCallback();
}
u16 WayfarerDispatchOriginScene(void)
{
    u8 scene = gSpecialVar_0x8004;
    u8 policy = WayfarerGetOriginScenePolicy(scene);
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(WayfarerGetStartingOriginId());
    if (policy == ORIGIN_SCENE_AUTHORED && profile != NULL && profile->authoredScene != NULL)
    {
        const u8 *script = profile->authoredScene(scene);
        if (script != NULL)
            ScriptContext_SetupScript(script);
    }
    return policy;
}
u16 WayfarerUsesNativeJohtoHousehold(void) { return WayfarerGetOriginScenePolicy(ORIGIN_SCENE_JOHTO_HOUSEHOLD) == ORIGIN_SCENE_NATIVE; }
u16 WayfarerUsesNativeHoennHousehold(void) { return WayfarerGetOriginScenePolicy(ORIGIN_SCENE_HOENN_HOUSEHOLD) == ORIGIN_SCENE_NATIVE; }
u16 WayfarerUsesNativeJohtoOpening(void) { return WayfarerGetOriginScenePolicy(ORIGIN_SCENE_JOHTO_PROFESSOR) == ORIGIN_SCENE_NATIVE; }
u16 WayfarerUsesNativeHoennOpening(void) { return WayfarerGetOriginScenePolicy(ORIGIN_SCENE_HOENN_RESCUE) == ORIGIN_SCENE_NATIVE; }
u16 WayfarerCanUseRegularAqua(void)
{
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(WayfarerGetStartingOriginId());
    return profile != NULL && profile->regularAqua();
}
u16 WayfarerCanReceiveSlateportTicket(void)
{
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(WayfarerGetStartingOriginId());
    return profile != NULL && profile->slateportTicket();
}
u16 WayfarerCanUseAquaMaidenVoyage(void)
{
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(WayfarerGetStartingOriginId());
    return profile != NULL && profile->maidenVoyage;
}
void WayfarerGrantSharedEquipment(void)
{
    FlagSet(FLAG_SYS_POKENAV_GET);
    FlagSet(FLAG_HAS_MATCH_CALL);
    FlagSet(FLAG_RECEIVED_POKENAV);
    FlagSet(FLAG_ADDED_MATCH_CALL_TO_POKENAV);
}
void WayfarerGrantRunningShoes(void)
{
    FlagSet(FLAG_SYS_B_DASH);
    FlagSet(FLAG_RECEIVED_RUNNING_SHOES);
}
u16 WayfarerHasSharedPokedex(void) { return FlagGet(FLAG_SYS_POKEDEX_GET); }
void WayfarerGrantSharedPokedex(void) { FlagSet(FLAG_SYS_POKEDEX_GET); }
bool8 WayfarerCanStartOrdinaryBattle(void)
{
    u32 i;
    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE
         && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG)
         && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
            return TRUE;
    return FALSE;
}
u8 WayfarerGetInitialRecoveryDestination(void)
{
    const struct WayfarerOriginProfile *profile = WayfarerGetOriginProfile(WayfarerGetStartingOriginId());
    return profile == NULL ? HEAL_LOCATION_NONE : profile->initialRecovery();
}
bool8 WayfarerReplaceRecoveryDestination(u8 healLocationId)
{
    if (GetHealLocation(healLocationId) == NULL)
        return FALSE;
    SetLastHealLocationWarp(healLocationId);
    gSaveBlock3Ptr->wayfarerHoenn.fallbackHealLocation = healLocationId;
    return TRUE;
}
bool8 WayfarerEnsureRecoveryDestination(void)
{
    u8 fallback;
    if (GetHealLocationIndexByWarpData(&gSaveBlock1Ptr->lastHealLocation) != HEAL_LOCATION_NONE)
        return TRUE;
    fallback = gSaveBlock3Ptr->wayfarerHoenn.fallbackHealLocation;
    if (GetHealLocation(fallback) == NULL)
        fallback = WayfarerGetInitialRecoveryDestination();
    if (GetHealLocation(fallback) == NULL)
        return FALSE;
    SetLastHealLocationWarp(fallback);
    return TRUE;
}
#if TESTING
bool8 Test_WayfarerRegisterOriginProfile(const struct WayfarerOriginProfile *profile)
{
    if (profile == NULL)
    {
        sTestProfile = NULL;
        return TRUE;
    }
    if (profile->id <= ORIGIN_LITTLEROOT || !IsProfileValid(profile))
        return FALSE;
    sTestProfile = profile;
    return TRUE;
}
#endif
#endif
