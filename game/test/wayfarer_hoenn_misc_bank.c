#include "global.h"
#include "daycare.h"
#include "event_data.h"
#include "field_specials.h"
#include "fieldmap.h"
#include "landmark.h"
#include "malloc.h"
#include "overworld.h"
#include "pokenav.h"
#include "secret_base.h"
#include "string_util.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/event_bg.h"
#include "constants/field_specials.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"
#include "constants/region_map_sections.h"

#if IS_WAYFARER

void ToggleCurSecretBaseRegistry(void);
void GetCurSecretBaseRegistrationValidity(void);
void SetDeptStoreFloor(void);
void UpdateCyclingRoadState(void);

// Hoenn's secret bases, day care man, department store elevator, Cycling Road, ribbons and landmarks keep
// their state in the Hoenn bank, where the scripts read it. The C that drives them must use the same copies;
// Emerald's names read as 0 or as a live Johto/Kanto var. These spell out Emerald's ids (SYSTEM_FLAGS 0x860)
// so a wrong macro cannot hide behind itself.

#define CURRENT_SECRET_BASE  HOENN_VAR_ID(0x4054)
#define INIT_SECRET_BASE     HOENN_VAR_ID(0x4097)
#define SECRET_BASE_REGISTRY HOENN_FLAG_ID(0x10C)
#define PENDING_DAYCARE_EGG  HOENN_FLAG_ID(0x086)
#define DEPT_STORE_FLOOR     HOENN_VAR_ID(0x4043)
#define CYCLING_CHALLENGE    HOENN_VAR_ID(0x40A9)
#define CYCLING_ROAD         HOENN_FLAG_ID(0x860 + 0x2B)
#define RIBBON_GET           HOENN_FLAG_ID(0x860 + 0x3B)
#define LANDMARK_TRAINER_HILL HOENN_FLAG_ID(0x860 + 0x82)
#define LANDMARK_WINSTRATE   HOENN_FLAG_ID(0x860 + 0x43)

// The resources struct is private to pokenav.c; the menu handler only needs its substruct pointer table.
extern void *gPokenavResources;

static void StartHoenn(void)
{
    WayfarerInitPersistentState();
    WayfarerSetSavedCurrentRegion(REGION_KANTO);
    EXPECT(WayfarerPrepareHoennEntry(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
                                     9, 11, HEAL_LOCATION_SLATEPORT_CITY));
}

TEST("Hoenn misc bank: entering a secret base is tracked in the Hoenn bank and leaves Baoba's quest and Wade's item var alone")
{
    struct SecretBase saved[SECRET_BASES_COUNT];
    struct MapPosition position = {.x = 3 + MAP_OFFSET, .y = 4 + MAP_OFFSET};
    struct BgEvent bgEvent = {.x = 3, .y = 4, .kind = BG_EVENT_SECRET_BASE, .bgUnion.secretBaseId = 0x5A};
    struct MapEvents events = {.bgEventCount = 1, .bgEvents = &bgEvent};
    u16 baoba, wade;

    StartHoenn();
    memcpy(saved, gSaveBlock1Ptr->secretBases, sizeof(saved));
    memset(gSaveBlock1Ptr->secretBases, 0, sizeof(saved));
    gSaveBlock1Ptr->secretBases[3].secretBaseId = 0x5A;
    baoba = VarGet(VAR_BAOBA_QUEST_STATE);
    wade = VarGet(VAR_WADE_ITEM_HNS);

    VarSet(CURRENT_SECRET_BASE, 0);
    SetCurSecretBaseIdFromPosition(&position, &events);
    TrySetCurSecretBaseIndex();
    EXPECT(gSpecialVar_Result);
    EXPECT_EQ(VarGet(CURRENT_SECRET_BASE), 3);
    EXPECT_EQ(VarGet(VAR_BAOBA_QUEST_STATE), baoba);

    // The registry is read and written through that same index, and unlocks the Hoenn flag.
    FlagClear(SECRET_BASE_REGISTRY);
    ToggleCurSecretBaseRegistry();
    EXPECT(gSaveBlock1Ptr->secretBases[3].registryStatus == 1);
    EXPECT(FlagGet(SECRET_BASE_REGISTRY));
    GetCurSecretBaseRegistrationValidity();
    EXPECT_EQ(gSpecialVar_Result, 1);
    EXPECT_EQ(VarGet(VAR_BAOBA_QUEST_STATE), baoba);
    EXPECT_EQ(VarGet(VAR_WADE_ITEM_HNS), wade);

    memcpy(gSaveBlock1Ptr->secretBases, saved, sizeof(saved));
    VarSet(CURRENT_SECRET_BASE, 0);
}

TEST("Hoenn misc bank: a secret base's first-entry var is the Hoenn one the common script sets, not Wade's item var")
{
    u16 wade;
    u8 mapType = gMapHeader.mapType;

    StartHoenn();
    wade = VarGet(VAR_WADE_ITEM_HNS);
    gMapHeader.mapType = MAP_TYPE_SECRET_BASE;
    VarSet(INIT_SECRET_BASE, 0);
    EXPECT(!SecretBaseMapPopupEnabled());
    VarSet(INIT_SECRET_BASE, 1);
    EXPECT(SecretBaseMapPopupEnabled());
    EXPECT_EQ(VarGet(VAR_WADE_ITEM_HNS), wade);
    gMapHeader.mapType = mapType;
    VarSet(INIT_SECRET_BASE, 0);
}

TEST("Hoenn misc bank: a pending egg is flagged for Route 117's day care man and for the other day cares, and taking it clears both")
{
    struct DayCare saved = gSaveBlock1Ptr->daycare;

    StartHoenn();
    memset(&gSaveBlock1Ptr->daycare, 0, sizeof(gSaveBlock1Ptr->daycare));
    FlagClear(FLAG_PENDING_DAYCARE_EGG);
    FlagClear(PENDING_DAYCARE_EGG);

    TriggerPendingDaycareEgg();
    EXPECT(FlagGet(PENDING_DAYCARE_EGG));
    EXPECT(FlagGet(FLAG_PENDING_DAYCARE_EGG));

    RejectEggFromDayCare();
    EXPECT(!FlagGet(PENDING_DAYCARE_EGG));
    EXPECT(!FlagGet(FLAG_PENDING_DAYCARE_EGG));
    gSaveBlock1Ptr->daycare = saved;
}

TEST("Hoenn misc bank: the Lilycove elevator's floor is written to the var its attendant script reads")
{
    u16 engineFloor;

    StartHoenn();
    engineFloor = VarGet(VAR_DEPT_STORE_FLOOR);
    gSaveBlock1Ptr->dynamicWarp.mapGroup = MAP_GROUP(MAP_LILYCOVE_CITY_DEPARTMENT_STORE_3F);
    gSaveBlock1Ptr->dynamicWarp.mapNum = MAP_NUM(MAP_LILYCOVE_CITY_DEPARTMENT_STORE_3F);
    VarSet(DEPT_STORE_FLOOR, 0);
    SetDeptStoreFloor();
    EXPECT_EQ(VarGet(DEPT_STORE_FLOOR), DEPT_STORE_FLOORNUM_3F);
    EXPECT_EQ(VarGet(VAR_DEPT_STORE_FLOOR), engineFloor);
}

TEST("Hoenn misc bank: leaving Route 110's Cycling Road by any way but the north gate ends the challenge in the Hoenn bank")
{
    u16 engineState;

    StartHoenn();
    engineState = VarGet(VAR_CYCLING_CHALLENGE_STATE);
    gLastUsedWarp.mapGroup = MAP_GROUP(MAP_ROUTE110);
    gLastUsedWarp.mapNum = MAP_NUM(MAP_ROUTE110);
    VarSet(CYCLING_CHALLENGE, 2);
    UpdateCyclingRoadState();
    EXPECT_EQ(VarGet(CYCLING_CHALLENGE), 0);

    VarSet(CYCLING_CHALLENGE, 3);
    gLastUsedWarp.mapGroup = MAP_GROUP(MAP_ROUTE110_SEASIDE_CYCLING_ROAD_NORTH_ENTRANCE);
    gLastUsedWarp.mapNum = MAP_NUM(MAP_ROUTE110_SEASIDE_CYCLING_ROAD_NORTH_ENTRANCE);
    UpdateCyclingRoadState();
    EXPECT_EQ(VarGet(CYCLING_CHALLENGE), 3);
    EXPECT_EQ(VarGet(VAR_CYCLING_CHALLENGE_STATE), engineState);
    VarSet(CYCLING_CHALLENGE, 0);
}

TEST("Hoenn misc bank: flying off the Cycling Road clears Hoenn's road flag as well as the engine one")
{
    StartHoenn();
    FlagSet(CYCLING_ROAD);
    FlagSet(FLAG_SYS_CYCLING_ROAD);
    Overworld_ResetStateAfterFly();
    EXPECT(!FlagGet(CYCLING_ROAD));
    EXPECT(!FlagGet(FLAG_SYS_CYCLING_ROAD));
}

TEST("Hoenn misc bank: a Hoenn contest ribbon unlocks the PokeNav's ribbons entry")
{
    StartHoenn();
    FlagSet(FLAG_ADDED_MATCH_CALL_TO_POKENAV);
    FlagSet(FLAG_ENABLE_CONDITION);
    FlagClear(FLAG_ENABLE_RADIO);
    FlagClear(FLAG_SYS_RIBBON_GET);
    FlagClear(RIBBON_GET);
    gPokenavResources = AllocZeroed(256);
    EXPECT(PokenavCallback_Init_MainMenuCursorOnMap());
    EXPECT_EQ(GetPokenavMenuType(), POKENAV_MENU_TYPE_UNLOCK_MC);
    FreePokenavSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER);

    FlagSet(RIBBON_GET);
    EXPECT(PokenavCallback_Init_MainMenuCursorOnMap());
    EXPECT_EQ(GetPokenavMenuType(), POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS);
    FreePokenavSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER);
    FREE_AND_SET_NULL(gPokenavResources);
}

TEST("Hoenn misc bank: the Town Map names a landmark once its Hoenn flag is set")
{
    // Route 111's fifth landmark slot lists Trainer Hill, then the Winstrate family, then the Desert.
    static const u8 trainerHill[] = _("TRAINER HILL");
    static const u8 desert[] = _("DESERT");

    StartHoenn();
    FlagClear(LANDMARK_TRAINER_HILL);
    FlagClear(LANDMARK_WINSTRATE);
    EXPECT(!StringCompare(GetLandmarkName(MAPSEC_ROUTE_111, 4, 0), desert));
    FlagSet(LANDMARK_TRAINER_HILL);
    EXPECT(!StringCompare(GetLandmarkName(MAPSEC_ROUTE_111, 4, 0), trainerHill));
    FlagClear(LANDMARK_TRAINER_HILL);
}

// The Johto player's room in New Bark keeps the engine FLAG_DECORATION_* ids on its decoration objects,
// while HOENN's player room and secret bases carry the Hoenn copies (Emerald 0xAE..0xBB).
TEST("Hoenn misc bank: decoration flags follow the map: engine ids in New Bark, Hoenn's in Littleroot")
{
    struct WarpData saved = gSaveBlock1Ptr->location;

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_2F_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_NEW_BARK_TOWN_PLAYERS_HOUSE_2F_HNS);
    EXPECT_EQ(WayfarerDecorationFlag(0), FLAG_DECORATION_1);
    EXPECT_EQ(WayfarerDecorationFlag(13), FLAG_DECORATION_14);

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F);
    EXPECT_EQ(WayfarerDecorationFlag(0), HOENN_FLAG_ID(0xAE));
    EXPECT_EQ(WayfarerDecorationFlag(13), HOENN_FLAG_ID(0xBB));

    gSaveBlock1Ptr->location = saved;
}

#endif // IS_WAYFARER
