#include "global.h"
#include "event_data.h"
#include "fieldmap.h"
#include "heal_location.h"
#include "overworld.h"
#include "script.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"

#if IS_WAYFARER

// Emerald names read as engine constants from C, so these spell out the Hoenn bank directly.
#define HOENN_FLAG_MIND_BADGE              HOENN_FLAG_ID(0x86D)
#define HOENN_FLAG_GROUDON_AWAKE           HOENN_FLAG_ID(0x06F)
#define HOENN_FLAG_AQUA_ESCAPED            HOENN_FLAG_ID(0x070)
#define HOENN_FLAG_STEVENS_DIVE            HOENN_FLAG_ID(0x07B)
#define HOENN_FLAG_MT_CHIMNEY_DONE         HOENN_FLAG_ID(0x08B)
#define HOENN_FLAG_HIDE_MOSSDEEP_MAGMA     HOENN_FLAG_ID(0x337)
#define HOENN_FLAG_HIDE_CENTER_1F_MAGMA    HOENN_FLAG_ID(0x2F4)
#define HOENN_FLAG_HIDE_CENTER_1F_STEVEN   HOENN_FLAG_ID(0x2F1)
#define HOENN_FLAG_HIDE_CENTER_2F_MAGMA    HOENN_FLAG_ID(0x35E)
#define HOENN_FLAG_HIDE_CENTER_2F_STEVEN   HOENN_FLAG_ID(0x35F)
#define HOENN_FLAG_HIDE_SUMMIT_ARCHIE      HOENN_FLAG_ID(0x394)
#define HOENN_FLAG_HIDE_SUMMIT_AQUA        HOENN_FLAG_ID(0x395)
#define HOENN_FLAG_HIDE_SEAFLOOR_GRUNT     HOENN_FLAG_ID(0x3AD)
#define HOENN_FLAG_HIDE_ROUTE_121_GRUNTS   HOENN_FLAG_ID(0x392)
#define HOENN_FLAG_RECEIVED_ORB            HOENN_FLAG_ID(0x0D4)
#define HOENN_VAR_MOSSDEEP_CITY_STATE      HOENN_VAR_ID(0x405D)
#define HOENN_VAR_ROUTE_121_STATE          HOENN_VAR_ID(0x4074)
#define HOENN_VAR_MOSSDEEP_CENTER_STATE    HOENN_VAR_ID(0x409F)
#define HOENN_VAR_SEAFLOOR_CAVERN_STATE    HOENN_VAR_ID(0x40A2)
#define HOENN_VAR_MT_PYRE_STATE            HOENN_VAR_ID(0x40B9)

static void StartHoenn(void)
{
    WayfarerInitPersistentState();
    WayfarerSetSavedCurrentRegion(REGION_KANTO);
    EXPECT(WayfarerPrepareHoennEntry(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
                                     9, 11, HEAL_LOCATION_SLATEPORT_CITY));
}

static void Enter(u8 mapGroup, u8 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    LoadObjEventTemplatesFromHeader();
    RunOnTransitionMapScript();
}

#define ENTER(map) Enter(MAP_GROUP(map), MAP_NUM(map))

static void StepOnTrigger(s16 x, s16 y)
{
    const struct MapEvents *events = gMapHeader.events;
    u32 i;

    for (i = 0; i < events->coordEventCount; i++)
    {
        if (events->coordEvents[i].x == x && events->coordEvents[i].y == y)
        {
            RunScriptImmediately(events->coordEvents[i].script);
            return;
        }
    }
    EXPECT(FALSE);
}

static void ExpectInvasion(bool8 started)
{
    EXPECT_EQ(VarGet(HOENN_VAR_MOSSDEEP_CITY_STATE), started ? 1 : 0);
    EXPECT_EQ(VarGet(HOENN_VAR_MOSSDEEP_CENTER_STATE), started ? 1 : 0);
    EXPECT_EQ(FlagGet(HOENN_FLAG_HIDE_MOSSDEEP_MAGMA), !started);
    EXPECT_EQ(FlagGet(HOENN_FLAG_HIDE_CENTER_1F_MAGMA), !started);
    EXPECT_EQ(FlagGet(HOENN_FLAG_HIDE_CENTER_2F_MAGMA), !started);
    EXPECT_EQ(FlagGet(HOENN_FLAG_HIDE_CENTER_2F_STEVEN), !started);
    EXPECT_EQ(FlagGet(HOENN_FLAG_HIDE_CENTER_1F_STEVEN), started);
}

TEST("Mt. Pyre's summit is empty of Team Aqua until Mt. Chimney is done")
{
    StartHoenn();
    EXPECT(!FlagGet(HOENN_FLAG_HIDE_SUMMIT_ARCHIE));

    ENTER(MAP_MT_PYRE_SUMMIT);
    EXPECT(FlagGet(HOENN_FLAG_HIDE_SUMMIT_ARCHIE));
    EXPECT(FlagGet(HOENN_FLAG_HIDE_SUMMIT_AQUA));
    StepOnTrigger(22, 7);
    StepOnTrigger(23, 7);
    StepOnTrigger(24, 7);
    EXPECT_EQ(VarGet(HOENN_VAR_MT_PYRE_STATE), 0);

    FlagSet(HOENN_FLAG_MT_CHIMNEY_DONE);
    ENTER(MAP_MT_PYRE_SUMMIT);
    EXPECT(!FlagGet(HOENN_FLAG_HIDE_SUMMIT_ARCHIE));
    EXPECT(!FlagGet(HOENN_FLAG_HIDE_SUMMIT_AQUA));
}

TEST("Mt. Pyre's summit leaves Team Aqua alone once the theft has played")
{
    StartHoenn();
    FlagSet(HOENN_FLAG_MT_CHIMNEY_DONE);

    // The theft hides them for good.
    VarSet(HOENN_VAR_MT_PYRE_STATE, 1);
    FlagSet(HOENN_FLAG_HIDE_SUMMIT_ARCHIE);
    FlagSet(HOENN_FLAG_HIDE_SUMMIT_AQUA);
    ENTER(MAP_MT_PYRE_SUMMIT);
    EXPECT(FlagGet(HOENN_FLAG_HIDE_SUMMIT_ARCHIE));
    EXPECT(FlagGet(HOENN_FLAG_HIDE_SUMMIT_AQUA));

    // Sootopolis shows Archie alone for the return of the orbs.
    VarSet(HOENN_VAR_MT_PYRE_STATE, 2);
    FlagClear(HOENN_FLAG_HIDE_SUMMIT_ARCHIE);
    ENTER(MAP_MT_PYRE_SUMMIT);
    EXPECT(!FlagGet(HOENN_FLAG_HIDE_SUMMIT_ARCHIE));
    EXPECT(FlagGet(HOENN_FLAG_HIDE_SUMMIT_AQUA));
}

TEST("Route 121's grunts are there only while Mt. Pyre is a true lead")
{
    StartHoenn();

    ENTER(MAP_ROUTE121);
    EXPECT(FlagGet(HOENN_FLAG_HIDE_ROUTE_121_GRUNTS));
    StepOnTrigger(25, 7);
    EXPECT_EQ(VarGet(HOENN_VAR_ROUTE_121_STATE), 0);

    FlagSet(HOENN_FLAG_MT_CHIMNEY_DONE);
    ENTER(MAP_ROUTE121);
    EXPECT(!FlagGet(HOENN_FLAG_HIDE_ROUTE_121_GRUNTS));

    FlagSet(HOENN_FLAG_RECEIVED_ORB);
    ENTER(MAP_ROUTE121);
    EXPECT(FlagGet(HOENN_FLAG_HIDE_ROUTE_121_GRUNTS));
    StepOnTrigger(25, 7);
    EXPECT_EQ(VarGet(HOENN_VAR_ROUTE_121_STATE), 0);
}

TEST("Route 121 stays clear once the grunts have moved out")
{
    StartHoenn();
    FlagSet(HOENN_FLAG_MT_CHIMNEY_DONE);
    VarSet(HOENN_VAR_ROUTE_121_STATE, 1);
    FlagSet(HOENN_FLAG_HIDE_ROUTE_121_GRUNTS);

    ENTER(MAP_ROUTE121);
    EXPECT(FlagGet(HOENN_FLAG_HIDE_ROUTE_121_GRUNTS));
}

TEST("Mossdeep stays quiet until the player holds the Mind Badge and Groudon is awake")
{
    u32 badgeFirst;

    PARAMETRIZE { badgeFirst = TRUE; }
    PARAMETRIZE { badgeFirst = FALSE; }

    StartHoenn();
    ENTER(MAP_MOSSDEEP_CITY);
    ExpectInvasion(FALSE);

    FlagSet(badgeFirst ? HOENN_FLAG_MIND_BADGE : HOENN_FLAG_GROUDON_AWAKE);
    ENTER(MAP_MOSSDEEP_CITY);
    ExpectInvasion(FALSE);

    FlagSet(badgeFirst ? HOENN_FLAG_GROUDON_AWAKE : HOENN_FLAG_MIND_BADGE);
    ENTER(MAP_MOSSDEEP_CITY);
    ExpectInvasion(TRUE);
}

TEST("The Space Center invasion starts once and never replays")
{
    StartHoenn();
    FlagSet(HOENN_FLAG_MIND_BADGE);
    FlagSet(HOENN_FLAG_GROUDON_AWAKE);
    ENTER(MAP_MOSSDEEP_CITY);
    ExpectInvasion(TRUE);

    ENTER(MAP_MOSSDEEP_CITY);
    ExpectInvasion(TRUE);

    // As the end of the invasion leaves things.
    VarSet(HOENN_VAR_MOSSDEEP_CITY_STATE, 3);
    VarSet(HOENN_VAR_MOSSDEEP_CENTER_STATE, 3);
    FlagSet(HOENN_FLAG_HIDE_MOSSDEEP_MAGMA);
    FlagSet(HOENN_FLAG_HIDE_CENTER_1F_MAGMA);
    FlagSet(HOENN_FLAG_HIDE_CENTER_2F_MAGMA);
    ENTER(MAP_MOSSDEEP_CITY);
    EXPECT_EQ(VarGet(HOENN_VAR_MOSSDEEP_CITY_STATE), 3);
    EXPECT_EQ(VarGet(HOENN_VAR_MOSSDEEP_CENTER_STATE), 3);
    EXPECT(FlagGet(HOENN_FLAG_HIDE_MOSSDEEP_MAGMA));
    EXPECT(FlagGet(HOENN_FLAG_HIDE_CENTER_1F_MAGMA));
    EXPECT(FlagGet(HOENN_FLAG_HIDE_CENTER_2F_MAGMA));
}

TEST("The Seafloor Cavern guard leaves only when both branches are done")
{
    u32 escaped, dive;

    PARAMETRIZE { escaped = FALSE; dive = FALSE; }
    PARAMETRIZE { escaped = FALSE; dive = TRUE; }
    PARAMETRIZE { escaped = TRUE; dive = FALSE; }
    PARAMETRIZE { escaped = TRUE; dive = TRUE; }

    StartHoenn();
    if (escaped)
        FlagSet(HOENN_FLAG_AQUA_ESCAPED);
    if (dive)
        FlagSet(HOENN_FLAG_STEVENS_DIVE);

    ENTER(MAP_SEAFLOOR_CAVERN_ENTRANCE);
    EXPECT_EQ(FlagGet(HOENN_FLAG_HIDE_SEAFLOOR_GRUNT), escaped && dive);
}

TEST("Room 9's scene waits for both branches")
{
    u32 escaped, dive;

    PARAMETRIZE { escaped = FALSE; dive = FALSE; }
    PARAMETRIZE { escaped = FALSE; dive = TRUE; }
    PARAMETRIZE { escaped = TRUE; dive = FALSE; }

    StartHoenn();
    if (escaped)
        FlagSet(HOENN_FLAG_AQUA_ESCAPED);
    if (dive)
        FlagSet(HOENN_FLAG_STEVENS_DIVE);

    ENTER(MAP_SEAFLOOR_CAVERN_ROOM9);
    StepOnTrigger(17, 42);
    EXPECT_EQ(VarGet(HOENN_VAR_SEAFLOOR_CAVERN_STATE), 0);
}

#endif
