#include "global.h"
#include "event_data.h"
#include "script.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "constants/vars.h"
#include "constants/wayfarer_persistence.h"

#if IS_WAYFARER

// These scripts assemble against Emerald's ids in the Hoenn bank. C sees the engine's names, where
// Emerald's mean something else or nothing, so the ids here are Emerald's numbers.
#define BADGE03_GET                    HOENN_FLAG_ID(0x869)
#define RECEIVED_TM_SHOCK_WAVE         HOENN_FLAG_ID(0xA7)
#define GOT_TM_THUNDERBOLT             HOENN_FLAG_ID(0xD1)
#define WATTSON_REMATCH_AVAILABLE      HOENN_FLAG_ID(0x5B)
#define HIDE_MAUVILLE_CITY_WATTSON     HOENN_FLAG_ID(0x390)
#define HIDE_MAUVILLE_GYM_WATTSON      HOENN_FLAG_ID(0x391)
#define DEFEATED_WALLY_MAUVILLE        HOENN_FLAG_ID(0xBE)
#define HIDE_MAUVILLE_CITY_WALLY       HOENN_FLAG_ID(0x324)
#define HIDE_MAUVILLE_CITY_WALLYS_UNCLE HOENN_FLAG_ID(0x325)
#define HIDE_PETALBURG_GYM_GREETER     HOENN_FLAG_ID(0x30D)
#define PETALBURG_MART_EXPANDED_ITEMS  HOENN_FLAG_ID(0x128)
#define HIDE_SS_TIDAL_BRINEY           HOENN_FLAG_ID(0x3B6)
#define IS_CHAMPION                    HOENN_FLAG_ID(0x87F)
#define DEFEATED_PETALBURG_GYM         HOENN_FLAG_ID(0x4F4)
#define RECEIVED_POKENAV               HOENN_FLAG_ID(0xBC)
#define PETALBURG_GYM_STATE            HOENN_VAR_ID(0x4085)
#define LAVARIDGE_TOWN_STATE           HOENN_VAR_ID(0x4053)
#define BRINEY_LOCATION                HOENN_VAR_ID(0x4096)

extern const u8 MauvilleCity_Gym_EventScript_PlaceWattson[];
extern const u8 MauvilleCity_EventScript_PlaceWally[];
extern const u8 Common_EventScript_ReadyPetalburgGymForBattle[];
extern const u8 LavaridgeTown_EventScript_TryArmRivalScene[];
extern const u8 Common_EventScript_UpdateBrineyLocation[];
extern const u8 EverGrandeCity_HallOfFame_EventScript_SetGameClearFlags[];

// Route 104, Dewford and Route 109, each with his boat, then his house and Peeko.
static const u16 sBrineyHideFlags[] =
{
    HOENN_FLAG_ID(0x2E2), HOENN_FLAG_ID(0x2E6),
    HOENN_FLAG_ID(0x2E4), HOENN_FLAG_ID(0x2E7),
    HOENN_FLAG_ID(0x2E5), HOENN_FLAG_ID(0x2E8),
    HOENN_FLAG_ID(0x2E3), HOENN_FLAG_ID(0x371),
};

static void PlaceWattson(bool32 badge, bool32 shockWave, bool32 thunderbolt)
{
    (badge ? FlagSet : FlagClear)(BADGE03_GET);
    (shockWave ? FlagSet : FlagClear)(RECEIVED_TM_SHOCK_WAVE);
    (thunderbolt ? FlagSet : FlagClear)(GOT_TM_THUNDERBOLT);
    RunScriptImmediately(MauvilleCity_Gym_EventScript_PlaceWattson);
}

static bool32 WattsonIsOutside(void)
{
    EXPECT_NE(!!FlagGet(HIDE_MAUVILLE_CITY_WATTSON), !!FlagGet(HIDE_MAUVILLE_GYM_WATTSON));
    return FlagGet(HIDE_MAUVILLE_GYM_WATTSON);
}

TEST("Wattson waits in his Gym until the badge is won and TM Shock Wave taken")
{
    FlagSet(HIDE_MAUVILLE_GYM_WATTSON);
    FlagClear(HIDE_MAUVILLE_CITY_WATTSON);

    PlaceWattson(FALSE, FALSE, FALSE);
    EXPECT(!WattsonIsOutside());
    // A full Bag in the badge scene leaves the TM with him.
    PlaceWattson(TRUE, FALSE, FALSE);
    EXPECT(!WattsonIsOutside());
}

TEST("Wattson asks outside after the TM, with or without Norman's badge")
{
    PlaceWattson(TRUE, TRUE, FALSE);
    EXPECT(WattsonIsOutside());
}

TEST("Wattson returns to his Gym for good once TM Thunderbolt is claimed")
{
    FlagClear(WATTSON_REMATCH_AVAILABLE);
    PlaceWattson(TRUE, TRUE, FALSE);
    EXPECT(!FlagGet(WATTSON_REMATCH_AVAILABLE));

    PlaceWattson(TRUE, TRUE, TRUE);
    EXPECT(!WattsonIsOutside());
    EXPECT(FlagGet(WATTSON_REMATCH_AVAILABLE));

    PlaceWattson(TRUE, TRUE, TRUE);
    EXPECT(!WattsonIsOutside());
}

TEST("Wally and his uncle come to Mauville only after the catching tutorial")
{
    FlagClear(DEFEATED_WALLY_MAUVILLE);
    VarSet(PETALBURG_GYM_STATE, 0);
    RunScriptImmediately(MauvilleCity_EventScript_PlaceWally);
    EXPECT(FlagGet(HIDE_MAUVILLE_CITY_WALLY));
    EXPECT(FlagGet(HIDE_MAUVILLE_CITY_WALLYS_UNCLE));

    VarSet(PETALBURG_GYM_STATE, 6);
    RunScriptImmediately(MauvilleCity_EventScript_PlaceWally);
    EXPECT(!FlagGet(HIDE_MAUVILLE_CITY_WALLY));
    EXPECT(!FlagGet(HIDE_MAUVILLE_CITY_WALLYS_UNCLE));
}

TEST("Readying Petalburg Gym shows its guide and stocks the Mart")
{
    // Common scripts pick the Hoenn bank from the map the player stands on.
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_PETALBURG_CITY_GYM);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_PETALBURG_CITY_GYM);
    FlagSet(HIDE_PETALBURG_GYM_GREETER);
    FlagClear(PETALBURG_MART_EXPANDED_ITEMS);
    RunScriptImmediately(Common_EventScript_ReadyPetalburgGymForBattle);
    EXPECT(!FlagGet(HIDE_PETALBURG_GYM_GREETER));
    EXPECT(FlagGet(PETALBURG_MART_EXPANDED_ITEMS));
}

TEST("Mr. Briney sails past Norman's defeat until the Hall of Fame boards him on the S.S. Tidal")
{
    u32 i;

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_EVER_GRANDE_CITY_HALL_OF_FAME);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_EVER_GRANDE_CITY_HALL_OF_FAME);
    FlagSet(DEFEATED_PETALBURG_GYM);
    FlagSet(RECEIVED_POKENAV);
    FlagSet(HIDE_SS_TIDAL_BRINEY);
    FlagClear(IS_CHAMPION);
    for (i = 0; i < ARRAY_COUNT(sBrineyHideFlags); i++)
        FlagSet(sBrineyHideFlags[i]);
    FlagClear(HOENN_FLAG_ID(0x2E6)); // His boat waits at Route 104.
    VarSet(BRINEY_LOCATION, 0);

    RunScriptImmediately(Common_EventScript_UpdateBrineyLocation);
    EXPECT_EQ(VarGet(BRINEY_LOCATION), 1);

    RunScriptImmediately(EverGrandeCity_HallOfFame_EventScript_SetGameClearFlags);
    EXPECT(FlagGet(IS_CHAMPION));
    EXPECT(!FlagGet(HIDE_SS_TIDAL_BRINEY));
    for (i = 0; i < ARRAY_COUNT(sBrineyHideFlags); i++)
        EXPECT(FlagGet(sBrineyHideFlags[i]));
    EXPECT_EQ(VarGet(BRINEY_LOCATION), 0);

    // Pokemon Centers track him on the way in, and find nothing left to restore.
    RunScriptImmediately(Common_EventScript_UpdateBrineyLocation);
    EXPECT_EQ(VarGet(BRINEY_LOCATION), 0);
}

static void WalkOutOnto(s16 x, s16 y)
{
    gSaveBlock1Ptr->pos.x = x;
    gSaveBlock1Ptr->pos.y = y;
    VarSet(LAVARIDGE_TOWN_STATE, 4);
    RunScriptImmediately(LavaridgeTown_EventScript_TryArmRivalScene);
}

TEST("The Lavaridge rival's scene stays parked without a Hoenn starter")
{
    VarSet(VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_NONE);
    WalkOutOnto(5, 15);
    EXPECT_EQ(VarGet(LAVARIDGE_TOWN_STATE), 4);
    WalkOutOnto(9, 6);
    EXPECT_EQ(VarGet(LAVARIDGE_TOWN_STATE), 4);
}

TEST("The Lavaridge rival's scene arms only at the Gym door and the Pokemon Center's front door")
{
    VarSet(VAR_HOENN_STARTER_CHOICE, HOENN_STARTER_CHOICE_TORCHIC);
    WalkOutOnto(5, 15);
    EXPECT_EQ(VarGet(LAVARIDGE_TOWN_STATE), 1);
    WalkOutOnto(9, 6);
    EXPECT_EQ(VarGet(LAVARIDGE_TOWN_STATE), 1);

    // The Pokemon Center's back exit, the Fly landing, and the road from Route 112.
    WalkOutOnto(9, 2);
    EXPECT_EQ(VarGet(LAVARIDGE_TOWN_STATE), 4);
    WalkOutOnto(9, 7);
    EXPECT_EQ(VarGet(LAVARIDGE_TOWN_STATE), 4);
    WalkOutOnto(0, 10);
    EXPECT_EQ(VarGet(LAVARIDGE_TOWN_STATE), 4);
}

#endif // IS_WAYFARER
