#include "global.h"
#include "event_data.h"
#include "heal_location.h"
#include "item.h"
#include "overworld.h"
#include "pokemon.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/opponents.h"
#include "constants/species.h"

#if IS_WAYFARER

#define HOENN_FLAG_HIDE_SLATEPORT_CAPTAIN_STERN HOENN_FLAG_ID(0x349)
#define HOENN_FLAG_HIDE_SLATEPORT_SS_TIDAL      HOENN_FLAG_ID(0x35C)
#define HOENN_FLAG_HIDE_ROUTE_101_BIRCH         HOENN_FLAG_ID(0x381)
#define HOENN_VAR_SLATEPORT_HARBOR_STATE        HOENN_VAR_ID(0x40A0)

static const struct MapHeader *GetSlateportHarborHeader(void)
{
    return Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR),
                                               MAP_NUM(MAP_SLATEPORT_CITY_HARBOR));
}

static bool8 HarborHasCoordEventAt(s16 x, s16 y)
{
    const struct MapEvents *events = GetSlateportHarborHeader()->events;
    u32 i;

    for (i = 0; i < events->coordEventCount; i++)
    {
        if (events->coordEvents[i].x == x && events->coordEvents[i].y == y)
            return TRUE;
    }
    return FALSE;
}

static void ExpectWalkableHarborTile(s16 x, s16 y)
{
    const struct MapLayout *layout = GetSlateportHarborHeader()->mapLayout;
    u16 metatile = layout->map[y * layout->width + x];

    EXPECT_EQ(UNPACK_COLLISION(metatile), 0);
}

TEST("Wayfarer Hoenn entry destination is safe and has an open path to Slateport")
{
    const struct MapHeader *header = GetSlateportHarborHeader();
    const struct MapEvents *events = header->events;
    bool8 foundExit = FALSE;
    u32 i;

    EXPECT(header != NULL);
    EXPECT(header->mapLayout != NULL);
    EXPECT(events != NULL);
    EXPECT(!HarborHasCoordEventAt(9, 11));

    // The approved arrival tile and a direct route to the ordinary south exit.
    ExpectWalkableHarborTile(9, 11);
    EXPECT_EQ(UNPACK_ELEVATION(header->mapLayout->map[11 * header->mapLayout->width + 9]),
              ELEVATION_DEFAULT);
    ExpectWalkableHarborTile(9, 12);
    ExpectWalkableHarborTile(9, 13);
    ExpectWalkableHarborTile(9, 14);
    ExpectWalkableHarborTile(10, 14);
    ExpectWalkableHarborTile(11, 14);

    for (i = 0; i < events->warpCount; i++)
    {
        if (events->warps[i].x == 11 && events->warps[i].y == 14
         && events->warps[i].mapGroup == MAP_GROUP(MAP_SLATEPORT_CITY)
         && events->warps[i].mapNum == MAP_NUM(MAP_SLATEPORT_CITY))
        {
            foundExit = TRUE;
            break;
        }
    }
    EXPECT(foundExit);
}

TEST("Wayfarer first Hoenn entry establishes only the Hoenn baseline")
{
    struct Pokemon partyBefore;
    const struct HealLocation *slateportHeal = GetHealLocation(HEAL_LOCATION_SLATEPORT_CITY);
    const u16 hnsProgressFlag = FLAG_VISITED_KANTO;
    const u16 hnsProgressVar = VAR_SSAQUA_STATE;
    const u32 money = 54321;

    WayfarerInitPersistentState();
    WayfarerSetSavedCurrentRegion(REGION_KANTO);
    FlagSet(hnsProgressFlag);
    VarSet(hnsProgressVar, 8);
    gSaveBlock1Ptr->money = money;
    EXPECT(AddBagItem(ITEM_SS_TICKET, 1));
    CreateMon(&gPlayerParty[0], SPECIES_TYPHLOSION, 42, 0, OTID_STRUCT_PLAYER_ID);
    memcpy(&partyBefore, &gPlayerParty[0], sizeof(partyBefore));

    EXPECT(WayfarerPrepareHoennEntry());

    EXPECT(WayfarerHoennStateIsInitialized());
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_HOENN);
    EXPECT(GetRegionVisitedState(REGION_HOENN));
    EXPECT_EQ(VarGet(VAR_HOENN_STARTER_CHOICE), HOENN_STARTER_CHOICE_NONE);
    EXPECT(!FlagGet(FLAG_HOENN_STARTER_RECEIVED));
    EXPECT(FlagGet(HOENN_FLAG_HIDE_SLATEPORT_CAPTAIN_STERN));
    EXPECT(FlagGet(HOENN_FLAG_HIDE_SLATEPORT_SS_TIDAL));
    EXPECT(FlagGet(HOENN_FLAG_HIDE_ROUTE_101_BIRCH));
    EXPECT_EQ(VarGet(HOENN_VAR_SLATEPORT_HARBOR_STATE), 0);
    EXPECT(!GetChampionStateForRegion(REGION_HOENN));

    EXPECT(FlagGet(hnsProgressFlag));
    EXPECT_EQ(VarGet(hnsProgressVar), 8);
    EXPECT_EQ(gSaveBlock1Ptr->money, money);
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SS_TICKET), 1);
    EXPECT_EQ(memcmp(&partyBefore, &gPlayerParty[0], sizeof(partyBefore)), 0);

    EXPECT(slateportHeal != NULL);
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapGroup, slateportHeal->mapGroup);
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapNum, slateportHeal->mapNum);
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.x, slateportHeal->x);
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.y, slateportHeal->y);
}

TEST("Wayfarer repeat Hoenn entry preserves campaign progress and refreshes travel state")
{
    const u16 progressFlag = HOENN_FLAG_ID(0x4FB);
    const u16 progressVar = HOENN_VAR_SLATEPORT_HARBOR_STATE;
    const u16 trainerId = WAYFARER_HOENN_TRAINER_OFFSET + 1;
    const struct HealLocation *slateportHeal = GetHealLocation(HEAL_LOCATION_SLATEPORT_CITY);

    WayfarerInitPersistentState();
    EXPECT(WayfarerPrepareHoennEntry());
    FlagSet(progressFlag);
    VarSet(progressVar, 2);
    SetBadgeStateForRegion(REGION_HOENN, 3, TRUE);
    WayfarerHoennTrainerFlagSet(trainerId);

    WayfarerSetSavedCurrentRegion(REGION_KANTO);
    SetLastHealLocationWarp(HEAL_LOCATION_NEW_BARK_TOWN_HNS);
    EXPECT(WayfarerPrepareHoennEntry());

    EXPECT(WayfarerHoennStateIsInitialized());
    EXPECT(FlagGet(progressFlag));
    EXPECT_EQ(VarGet(progressVar), 2);
    EXPECT(GetBadgeStateForRegion(REGION_HOENN, 3));
    EXPECT(WayfarerHoennTrainerFlagGet(trainerId));
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_HOENN);
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapGroup, slateportHeal->mapGroup);
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapNum, slateportHeal->mapNum);
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.x, slateportHeal->x);
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.y, slateportHeal->y);
}

TEST("Wayfarer Hoenn entry preflight failure is atomic")
{
    struct WayfarerHoennPersistentState stateBefore;
    struct WarpData healBefore;

    WayfarerInitPersistentState();
    WayfarerSetSavedCurrentRegion(REGION_KANTO);
    SetLastHealLocationWarp(HEAL_LOCATION_NEW_BARK_TOWN_HNS);
    memcpy(&stateBefore, &gSaveBlock3Ptr->wayfarerHoenn, sizeof(stateBefore));
    memcpy(&healBefore, &gSaveBlock1Ptr->lastHealLocation, sizeof(healBefore));

    // (8, 11) is a real harbor coordinate event and therefore an invalid
    // arrival tile even though its collision permits walking.
    EXPECT(!Test_WayfarerPrepareHoennEntryAt(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR),
                                             MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
                                             8, 11, HEAL_LOCATION_SLATEPORT_CITY));
    EXPECT_EQ(memcmp(&stateBefore, &gSaveBlock3Ptr->wayfarerHoenn, sizeof(stateBefore)), 0);
    EXPECT_EQ(memcmp(&healBefore, &gSaveBlock1Ptr->lastHealLocation, sizeof(healBefore)), 0);

    EXPECT(!Test_WayfarerPrepareHoennEntryAt(MAP_GROUP(MAP_NEW_BARK_TOWN_HNS),
                                             MAP_NUM(MAP_NEW_BARK_TOWN_HNS),
                                             9, 11, HEAL_LOCATION_SLATEPORT_CITY));
    EXPECT_EQ(memcmp(&stateBefore, &gSaveBlock3Ptr->wayfarerHoenn, sizeof(stateBefore)), 0);

    EXPECT(!Test_WayfarerPrepareHoennEntryAt(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR),
                                             MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
                                             9, 11, HEAL_LOCATION_NONE));
    EXPECT_EQ(memcmp(&stateBefore, &gSaveBlock3Ptr->wayfarerHoenn, sizeof(stateBefore)), 0);
}

#endif
