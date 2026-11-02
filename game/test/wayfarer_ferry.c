#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "fake_rtc.h"
#include "field_specials.h"
#include "heal_location.h"
#include "item.h"
#include "overworld.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "rtc.h"
#include "wayfarer_daily_slots.h"
#include "wayfarer_ferry.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "wayfarer_sevii_ferry.h"
#include "test/test.h"
#include "constants/event_objects.h"
#include "constants/field_specials.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/script_commands.h"
#include "constants/moves.h"
#include "constants/opponents.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/vars.h"
#include "constants/wayfarer_ferry.h"
#include "constants/wayfarer_persistence.h"

#if IS_WAYFARER

// The S.S. ferries (.product/specs/ss-ferries.md): the schedule, the voyage state, sailing and service.

#define HOURLY_PORTS FERRY_PORT_COUNT

static void ResetFerryFixture(void)
{
    struct WayfarerVoyage none = {0};

    WayfarerInitPersistentState();
    ClearBag();
    Test_Ferry_SetVoyage(&none);
    FlagClear(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED);
    FlagClear(FLAG_SYS_GAME_CLEAR);
    VarSet(VAR_SSAQUA_STATE, 0);
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_PALLET;
    FakeRtc_ManuallySetTime(1, 8, 0, 0);
}

static void PutEveryShipInService(void)
{
    EXPECT(AddBagItem(ITEM_SS_TICKET, 1));
    FlagSet(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED);
}

TEST("Ferry: the schedule table returns the by-day and by-night ship of every port at every hour boundary")
{
    // by day = 8:00 up to 20:00. At One Island and Melemele the berth is empty 0:00 up to 4:00.
    static const u8 sDay[FERRY_PORT_COUNT] =
    {
        [FERRY_PORT_OLIVINE] = FERRY_SHIP_AQUA, [FERRY_PORT_VERMILION] = FERRY_SHIP_AQUA,
        [FERRY_PORT_SLATEPORT] = FERRY_SHIP_TIDAL, [FERRY_PORT_LILYCOVE] = FERRY_SHIP_AQUA,
        [FERRY_PORT_SEVEN_ISLAND] = FERRY_SHIP_ANNE, [FERRY_PORT_ONE_ISLAND] = FERRY_SHIP_ANNE,
        [FERRY_PORT_MELEMELE] = FERRY_SHIP_ANNE,
    };
    static const u8 sNight[FERRY_PORT_COUNT] =
    {
        [FERRY_PORT_OLIVINE] = FERRY_SHIP_TIDAL, [FERRY_PORT_VERMILION] = FERRY_SHIP_ANNE,
        [FERRY_PORT_SLATEPORT] = FERRY_SHIP_AQUA, [FERRY_PORT_LILYCOVE] = FERRY_SHIP_TIDAL,
        [FERRY_PORT_SEVEN_ISLAND] = FERRY_SHIP_TIDAL, [FERRY_PORT_ONE_ISLAND] = FERRY_SHIP_ANNE,
        [FERRY_PORT_MELEMELE] = FERRY_SHIP_ANNE,
    };
    u32 port, hour;

    ResetFerryFixture();
    PutEveryShipInService();
    for (port = 0; port < FERRY_PORT_COUNT; port++)
    {
        bool32 anchoredAtNight = port == FERRY_PORT_ONE_ISLAND || port == FERRY_PORT_MELEMELE;

        // 3:59 (hour 3), 4:00, 7:59, 8:00, 19:59 and 20:00.
        EXPECT_EQ(Ferry_ShipAt(port, 3), anchoredAtNight ? FERRY_SHIP_NONE : sNight[port]);
        EXPECT_EQ(Ferry_ShipAt(port, 4), sNight[port]);
        EXPECT_EQ(Ferry_ShipAt(port, 7), sNight[port]);
        EXPECT_EQ(Ferry_ShipAt(port, 8), sDay[port]);
        EXPECT_EQ(Ferry_ShipAt(port, 19), sDay[port]);
        EXPECT_EQ(Ferry_ShipAt(port, 20), sNight[port]);
        for (hour = 0; hour < HOURS_PER_DAY; hour++)
        {
            u8 expected = hour >= 8 && hour < 20 ? sDay[port] : sNight[port];

            if (anchoredAtNight && hour < 4)
                expected = FERRY_SHIP_NONE;
            EXPECT_EQ(Ferry_ShipAt(port, hour), expected);
        }
    }
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_NONE, 12), FERRY_SHIP_NONE);
}

TEST("Ferry: ships not yet in service are returned as none, whatever the table says")
{
    ResetFerryFixture();
    // No captain's reward: the Anne is out. The Aqua and the Tidal are in service even without a Ticket
    // (the Ticket gates boarding at the gangway, not the berth).
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_VERMILION, 12), FERRY_SHIP_AQUA);
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_VERMILION, 22), FERRY_SHIP_NONE);
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_SEVEN_ISLAND, 12), FERRY_SHIP_NONE);
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_SEVEN_ISLAND, 22), FERRY_SHIP_TIDAL);
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_SLATEPORT, 12), FERRY_SHIP_TIDAL);
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_ONE_ISLAND, 12), FERRY_SHIP_NONE);
    // A Johto start whose maiden voyage is pending has no Aqua anywhere.
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_VERMILION, 12), FERRY_SHIP_NONE);
    EXPECT_EQ(Ferry_ScheduledShip(FERRY_PORT_VERMILION, 12), FERRY_SHIP_AQUA);
}

TEST("Ferry: only a Johto start's pending maiden voyage keeps the Aqua out of service, and the Ticket is never used up")
{
    static const u16 sStarts[] = {ORIGIN_NEW_BARK, ORIGIN_LITTLEROOT, ORIGIN_PALLET};
    u32 start, state, ticket;

    ResetFerryFixture();
    for (start = 0; start < ARRAY_COUNT(sStarts); start++)
    {
        gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = sStarts[start];
        for (state = 0; state <= 9; state++)
        {
            VarSet(VAR_SSAQUA_STATE, state);
            for (ticket = 0; ticket <= 1; ticket++)
            {
                bool32 pending = sStarts[start] == ORIGIN_NEW_BARK && state < 8;

                ClearBag();
                if (ticket)
                    EXPECT(AddBagItem(ITEM_SS_TICKET, 1));
                EXPECT_EQ(Ferry_IsMaidenVoyagePending(), pending);
                EXPECT_EQ(Ferry_ShipInService(FERRY_SHIP_AQUA), !pending);
                EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_OLIVINE, 12), pending ? FERRY_SHIP_NONE : FERRY_SHIP_AQUA);
                EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_SS_TICKET), ticket);
            }
        }
    }
}

TEST("Ferry: the Tidal sails for a Ticket holder without Hoenn's League, and the Anne waits for the captain's reward")
{
    ResetFerryFixture();
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT(Ferry_ShipInService(FERRY_SHIP_TIDAL));
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_SLATEPORT, 12), FERRY_SHIP_TIDAL);
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_LILYCOVE, 22), FERRY_SHIP_TIDAL);
    EXPECT(AddBagItem(ITEM_SS_TICKET, 1));
    EXPECT(!Ferry_ShipInService(FERRY_SHIP_ANNE));
    FlagSet(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED);
    EXPECT(Ferry_ShipInService(FERRY_SHIP_ANNE));
    EXPECT_EQ(Ferry_ShipAt(FERRY_PORT_VERMILION, 22), FERRY_SHIP_ANNE);
    EXPECT(!Ferry_ShipInService(FERRY_SHIP_NONE));
}

TEST("Ferry: the Birth Island trip needs the Aurora Ticket alone")
{
    ResetFerryFixture();
    EXPECT(!WayfarerCanSailToBirthIsland());
    // No start gate and no S.S. Ticket: the Aurora Ticket is the condition, as in FireRed and LeafGreen.
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    EXPECT(AddBagItem(ITEM_AURORA_TICKET, 1));
    EXPECT(WayfarerCanSailToBirthIsland());
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_PALLET;
    EXPECT(WayfarerCanSailToBirthIsland());
    EXPECT(RemoveBagItem(ITEM_AURORA_TICKET, 1));
    EXPECT(!WayfarerCanSailToBirthIsland());
    // Navel Rock keeps its league-win and Mystic Ticket check.
    EXPECT(AddBagItem(ITEM_MYSTIC_TICKET, 1));
    EXPECT(!WayfarerCanSailToNavelRock());
    FlagSet(FLAG_SYS_GAME_CLEAR);
    EXPECT(WayfarerCanSailToNavelRock());
}

TEST("Ferry: boarding sets every field of the voyage state and keeps a pending Anne departure")
{
    const struct WayfarerVoyage *voyage = Ferry_GetVoyage();
    u32 port;

    ResetFerryFixture();
    EXPECT_EQ(sizeof(struct WayfarerVoyage), 12);
    gPokemonStoragePtr->dailySlots.stampDay = 0x1234;
    EXPECT(!Ferry_IsAboard());
    // Each ship boards at each of its own ports, at the loop index of that port.
    EXPECT(Ferry_BoardAt(FERRY_PORT_LILYCOVE, FERRY_SHIP_AQUA, 5000));
    EXPECT_EQ((u32)voyage->ship, FERRY_SHIP_AQUA);
    EXPECT_EQ((u32)voyage->stop, 3);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_PORT);
    EXPECT_EQ(voyage->eventAt, 5000 + FERRY_PORT_MINUTES);
    EXPECT_EQ(voyage->slotDay, 0x1234);
    EXPECT_EQ((u32)voyage->annePending, 0);
    // Two hosts are drawn (the draw has its own test); nothing is earned or settled yet.
    EXPECT(voyage->hosts[0] != 0 && voyage->hosts[1] != 0);
    EXPECT_EQ((u32)voyage->cabin, 0);
    EXPECT_EQ((u32)voyage->hostsDone, 0);
    EXPECT(Ferry_IsAboard());
    EXPECT(!Ferry_IsAtSea());

    EXPECT(Ferry_BoardAt(FERRY_PORT_MELEMELE, FERRY_SHIP_ANNE, 100));
    EXPECT_EQ((u32)voyage->stop, 2);
    EXPECT(Ferry_BoardAt(FERRY_PORT_OLIVINE, FERRY_SHIP_TIDAL, 100));
    EXPECT_EQ((u32)voyage->stop, 3);
    for (port = 0; port < FERRY_PORT_COUNT; port++)
    {
        u8 stop = Ferry_GetPortStop(FERRY_SHIP_AQUA, port);

        EXPECT_EQ(Ferry_BoardAt(port, FERRY_SHIP_AQUA, 0), stop != 0xFF);
    }
    // A ship doesn't call where it doesn't sail, and an invalid ship is refused.
    EXPECT(!Ferry_BoardAt(FERRY_PORT_MELEMELE, FERRY_SHIP_AQUA, 0));
    EXPECT(!Ferry_BoardAt(FERRY_PORT_VERMILION, FERRY_SHIP_NONE, 0));

    Ferry_ClearVoyage();
    EXPECT_EQ((u32)voyage->ship, FERRY_SHIP_NONE);
    EXPECT_EQ(voyage->eventAt, 0);
}

TEST("Ferry: the loops run in order and wrap")
{
    u32 stop;

    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_AQUA, 0), FERRY_PORT_OLIVINE);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_AQUA, 1), FERRY_PORT_VERMILION);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_AQUA, 2), FERRY_PORT_SLATEPORT);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_AQUA, 3), FERRY_PORT_LILYCOVE);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_AQUA, 4), FERRY_PORT_OLIVINE);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_ANNE, 0), FERRY_PORT_VERMILION);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_ANNE, 1), FERRY_PORT_ONE_ISLAND);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_ANNE, 2), FERRY_PORT_MELEMELE);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_ANNE, 3), FERRY_PORT_SEVEN_ISLAND);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_TIDAL, 0), FERRY_PORT_SLATEPORT);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_TIDAL, 1), FERRY_PORT_LILYCOVE);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_TIDAL, 2), FERRY_PORT_SEVEN_ISLAND);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_TIDAL, 3), FERRY_PORT_OLIVINE);
    for (stop = 0; stop < FERRY_LOOP_LENGTH; stop++)
        EXPECT_EQ(Ferry_GetPortStop(FERRY_SHIP_TIDAL, Ferry_GetLoopPort(FERRY_SHIP_TIDAL, stop)), stop);
}

TEST("Ferry: a ship casts off half an in-game hour after boarding and arrives six hours later")
{
    const struct WayfarerVoyage *voyage = Ferry_GetVoyage();
    u32 t0 = 10000;

    ResetFerryFixture();
    EXPECT(Ferry_BoardAt(FERRY_PORT_SLATEPORT, FERRY_SHIP_TIDAL, t0));
    EXPECT_EQ(Ferry_Advance(t0), FERRY_ANNOUNCE_NONE);
    EXPECT_EQ(Ferry_Advance(t0 + 29), FERRY_ANNOUNCE_NONE);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_PORT);

    // Cast-off: at sea, the next port of the loop, eventAt += 360.
    EXPECT_EQ(Ferry_Advance(t0 + 30), FERRY_ANNOUNCE_CAST_OFF);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_SEA);
    EXPECT_EQ((u32)voyage->stop, 1);
    EXPECT_EQ(voyage->eventAt, t0 + 30 + 360);
    EXPECT(Ferry_IsAtSea());
    EXPECT_EQ(Ferry_Advance(t0 + 30 + 359), FERRY_ANNOUNCE_NONE);

    // Arrival: in port, eventAt += 30. The schedule is never consulted.
    EXPECT_EQ(Ferry_Advance(t0 + 30 + 360), FERRY_ANNOUNCE_ARRIVAL);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_PORT);
    EXPECT_EQ((u32)voyage->stop, 1);
    EXPECT_EQ(voyage->eventAt, t0 + 30 + 360 + 30);
    EXPECT(!Ferry_IsAtSea());

    // Nothing happens aboard no ship.
    Ferry_ClearVoyage();
    EXPECT_EQ(Ferry_Advance(t0 + 100000), FERRY_ANNOUNCE_NONE);
}

TEST("Ferry: elapsed transitions are applied in order after a long battle and only the last is announced")
{
    const struct WayfarerVoyage *voyage = Ferry_GetVoyage();
    u32 t0 = 10000;
    u32 castOff = t0 + 30;
    u32 arrival = castOff + 360;
    u32 secondCastOff = arrival + 30;

    ResetFerryFixture();
    // Late past the first cast-off and the arrival: the ship is in port again, announced as an arrival.
    EXPECT(Ferry_BoardAt(FERRY_PORT_OLIVINE, FERRY_SHIP_AQUA, t0));
    EXPECT_EQ(Ferry_Advance(arrival + 5), FERRY_ANNOUNCE_ARRIVAL);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_PORT);
    EXPECT_EQ((u32)voyage->stop, 1);
    EXPECT_EQ(voyage->eventAt, secondCastOff);

    // Late past the second cast-off too: at sea toward the third port, announced as the cast-off.
    EXPECT(Ferry_BoardAt(FERRY_PORT_OLIVINE, FERRY_SHIP_AQUA, t0));
    EXPECT_EQ(Ferry_Advance(secondCastOff + 7), FERRY_ANNOUNCE_CAST_OFF);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_SEA);
    EXPECT_EQ((u32)voyage->stop, 2);
    EXPECT_EQ(voyage->eventAt, secondCastOff + 360);

    // A whole loop and more: events keep their schedule, they are not reset by when they were noticed.
    EXPECT(Ferry_BoardAt(FERRY_PORT_OLIVINE, FERRY_SHIP_AQUA, t0));
    EXPECT_EQ(Ferry_Advance(t0 + 4 * (30 + 360)), FERRY_ANNOUNCE_ARRIVAL);
    EXPECT_EQ((u32)voyage->stop, 0);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_PORT);
    EXPECT_EQ(voyage->eventAt, t0 + 4 * (30 + 360) + 30);
    // Many days away: the ship is back on the same loop position it would have reached.
    EXPECT(Ferry_BoardAt(FERRY_PORT_OLIVINE, FERRY_SHIP_AQUA, t0));
    EXPECT_EQ(Ferry_Advance(t0 + 4 * (30 + 360) * 1000 + 30 + 360 + 2), FERRY_ANNOUNCE_ARRIVAL);
    EXPECT_EQ((u32)voyage->stop, 1);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_PORT);
    EXPECT_EQ(voyage->eventAt, t0 + 4 * (30 + 360) * 1000 + 30 + 360 + 30);
}

TEST("Ferry: the in-game clock feeds the minute count the voyage schedules against")
{
    u32 earlier, later;

    ResetFerryFixture();
    FakeRtc_ManuallySetTime(1, 8, 0, 0);
    earlier = Ferry_GetNow();
    // days x 1440 + hours x 60 + minutes.
    EXPECT_EQ(earlier % (24 * 60), 8 * 60);
    FakeRtc_ManuallySetTime(3, 9, 15, 0);
    later = Ferry_GetNow();
    EXPECT_EQ(later % (24 * 60), 9 * 60 + 15);
    EXPECT_EQ(later - earlier, 2 * 24 * 60 + 75);
    // Playing on advances the count one minute per five seconds of play (12 in-game seconds a second).
    FakeRtc_AdvanceTimeBy(0, 0, 30, 0);
    EXPECT_EQ(Ferry_GetNow(), later + 30);
}

TEST("Ferry: the Anne's first departure is armed by the reward, boards at Vermilion and is spent at cast-off")
{
    const struct WayfarerVoyage *voyage = Ferry_GetVoyage();

    ResetFerryFixture();
    // Armed away from the Anne: the Vermilion sailor offers her, whatever the table says.
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS);
    FlagSet(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED);
    EXPECT(AddBagItem(ITEM_SS_TICKET, 1));
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
    EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_AQUA);
    EXPECT(!Ferry_ArmAnneDeparture());
    EXPECT(voyage->annePending != 0);
    EXPECT(Ferry_IsAnnePendingAtVermilion());
    EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_ANNE);

    // Aboard the Anne when the captain rewards: the voyage is set as for boarding at Vermilion, cast-off in half an hour.
    Ferry_ClearVoyage();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_SSANNE_CAPTAINS_OFFICE);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_SSANNE_CAPTAINS_OFFICE);
    EXPECT(Ferry_ArmAnneDeparture());
    EXPECT_EQ((u32)voyage->ship, FERRY_SHIP_ANNE);
    EXPECT_EQ((u32)voyage->stop, 0);
    EXPECT_EQ((u32)voyage->phase, FERRY_PHASE_PORT);
    EXPECT_EQ(voyage->eventAt, Ferry_GetNow() + FERRY_PORT_MINUTES);
    EXPECT(voyage->annePending != 0);

    // Casting off spends the arming: from then on the Anne follows the table.
    EXPECT_EQ(Ferry_Advance(Ferry_GetNow() + FERRY_PORT_MINUTES), FERRY_ANNOUNCE_CAST_OFF);
    EXPECT(voyage->annePending == 0);
    EXPECT_EQ(Ferry_GetLoopPort(FERRY_SHIP_ANNE, voyage->stop), FERRY_PORT_ONE_ISLAND);
}

TEST("Ferry: a berth offers the ship the table has in at this hour")
{
    ResetFerryFixture();
    PutEveryShipInService();
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS);
    EXPECT_EQ(Ferry_GetBerthPort(), FERRY_PORT_VERMILION);
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
    EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_AQUA);
    FakeRtc_ManuallySetTime(1, 20, 0, 0);
    EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_ANNE);
    // The Anne anchors offshore at One Island from 0:00 to 4:00.
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_ONE_ISLAND_HARBOR);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_ONE_ISLAND_HARBOR);
    FakeRtc_ManuallySetTime(1, 2, 0, 0);
    EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_NONE);
    FakeRtc_ManuallySetTime(1, 4, 0, 0);
    EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_ANNE);
    // The Slateport berth closes during the Aqua escape, whatever the table says.
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_SLATEPORT_CITY_HARBOR);
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
    EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_TIDAL);
    FlagSet(HOENN_FLAG_ID(0x389)); // FLAG_HIDE_SLATEPORT_CITY_HARBOR_PATRONS
    EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_NONE);
    FlagClear(HOENN_FLAG_ID(0x389));
}

TEST("Ferry: ship maps and ports are told apart from every other map")
{
    EXPECT_EQ(Ferry_GetShipOfMap(MAP_GROUP(MAP_SSAQUA_1F_HNS), MAP_NUM(MAP_SSAQUA_1F_HNS)), FERRY_SHIP_AQUA);
    EXPECT_EQ(Ferry_GetShipOfMap(MAP_GROUP(MAP_SSAQUA_ROOM_SW_HNS), MAP_NUM(MAP_SSAQUA_ROOM_SW_HNS)), FERRY_SHIP_AQUA);
    EXPECT_EQ(Ferry_GetShipOfMap(MAP_GROUP(MAP_SSANNE_1F_CORRIDOR), MAP_NUM(MAP_SSANNE_1F_CORRIDOR)), FERRY_SHIP_ANNE);
    EXPECT_EQ(Ferry_GetShipOfMap(MAP_GROUP(MAP_SSANNE_1F_ROOM6), MAP_NUM(MAP_SSANNE_1F_ROOM6)), FERRY_SHIP_ANNE);
    EXPECT_EQ(Ferry_GetShipOfMap(MAP_GROUP(MAP_SS_TIDAL_ROOMS), MAP_NUM(MAP_SS_TIDAL_ROOMS)), FERRY_SHIP_TIDAL);
    // The Anne's exterior is not part of the ship; the berths are not ships.
    EXPECT(!Ferry_IsShipMap(MAP_GROUP(MAP_SSANNE_EXTERIOR), MAP_NUM(MAP_SSANNE_EXTERIOR)));
    EXPECT(!Ferry_IsShipMap(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)));
    EXPECT_EQ(Ferry_GetPortOfMap(MAP_GROUP(MAP_MELEMELE_ISLE_HNS), MAP_NUM(MAP_MELEMELE_ISLE_HNS)), FERRY_PORT_MELEMELE);
    EXPECT_EQ(Ferry_GetPortOfMap(MAP_GROUP(MAP_LILYCOVE_CITY_HARBOR), MAP_NUM(MAP_LILYCOVE_CITY_HARBOR)), FERRY_PORT_LILYCOVE);
    EXPECT_EQ(Ferry_GetPortOfMap(MAP_GROUP(MAP_SSAQUA_1F_HNS), MAP_NUM(MAP_SSAQUA_1F_HNS)), FERRY_PORT_NONE);
}

TEST("Ferry: a blackout aboard clears the voyage")
{
    ResetFerryFixture();
    EXPECT(Ferry_BoardAt(FERRY_PORT_VERMILION, FERRY_SHIP_AQUA, 1000));
    Ferry_OnBlackout();
    EXPECT(!Ferry_IsAboard());
    // Ashore, a blackout changes nothing.
    Ferry_OnBlackout();
    EXPECT(!Ferry_IsAboard());
}

TEST("Ferry: the Tidal's Town Map location follows the voyage state")
{
    struct WayfarerVoyage voyage = {0};
    s8 group = 0, num = 0;
    s16 x = 0, y = 0;

    ResetFerryFixture();
    // With no voyage: Slateport.
    EXPECT_EQ(GetSSTidalLocation(&group, &num, &x, &y), SS_TIDAL_LOCATION_SLATEPORT);

    voyage.ship = FERRY_SHIP_TIDAL;
    // Slateport, Lilycove, Seven Island, Olivine: at, or sailing to, each.
    voyage.stop = 0;
    Test_Ferry_SetVoyage(&voyage);
    EXPECT_EQ(GetSSTidalLocation(&group, &num, &x, &y), SS_TIDAL_LOCATION_SLATEPORT);
    voyage.stop = 1;
    Test_Ferry_SetVoyage(&voyage);
    EXPECT_EQ(GetSSTidalLocation(&group, &num, &x, &y), SS_TIDAL_LOCATION_LILYCOVE);
    voyage.stop = 2;
    Test_Ferry_SetVoyage(&voyage);
    EXPECT_EQ(GetSSTidalLocation(&group, &num, &x, &y), SS_TIDAL_LOCATION_LILYCOVE);
    voyage.stop = 3;
    voyage.phase = FERRY_PHASE_SEA;
    Test_Ferry_SetVoyage(&voyage);
    EXPECT_EQ(GetSSTidalLocation(&group, &num, &x, &y), SS_TIDAL_LOCATION_SLATEPORT);
    // The retired cruise never counts steps.
    EXPECT(!CountSSTidalStep(1000));
}

TEST("Ferry: loading a ship map aboard changes neither the saved region nor the visited regions")
{
    struct WayfarerVoyage voyage = {0};

    ResetFerryFixture();
    WayfarerSetSavedCurrentRegion(REGION_KANTO);
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_KANTO);
    EXPECT(!GetRegionVisitedState(REGION_HOENN));

    // Aboard, the Tidal's Hoenn-source maps and the Aqua's Johto maps leave the region alone.
    EXPECT(Ferry_BoardAt(FERRY_PORT_VERMILION, FERRY_SHIP_AQUA, 0));
    WayfarerUpdateHnsRegionContextForMap(MAP_GROUP(MAP_SS_TIDAL_CORRIDOR), MAP_NUM(MAP_SS_TIDAL_CORRIDOR));
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_KANTO);
    EXPECT(!GetRegionVisitedState(REGION_HOENN));
    WayfarerUpdateHnsRegionContextForMap(MAP_GROUP(MAP_SSAQUA_1F_HNS), MAP_NUM(MAP_SSAQUA_1F_HNS));
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_KANTO);
    EXPECT(!GetRegionVisitedState(REGION_JOHTO));

    // A ship map's rules apply to ship maps only: a port map still sets its own region.
    WayfarerUpdateHnsRegionContextForMap(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR));
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_HOENN);

    // Ashore the same Tidal map is a normal Hoenn-source map.
    WayfarerSetSavedCurrentRegion(REGION_KANTO);
    Test_Ferry_SetVoyage(&voyage);
    WayfarerUpdateHnsRegionContextForMap(MAP_GROUP(MAP_SS_TIDAL_CORRIDOR), MAP_NUM(MAP_SS_TIDAL_CORRIDOR));
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_HOENN);
}

TEST("Ferry: initializing Hoenn's state touches neither the saved region nor the respawn")
{
    struct WarpData healBefore;

    ResetFerryFixture();
    WayfarerSetSavedCurrentRegion(REGION_JOHTO);
    SetLastHealLocationWarp(HEAL_LOCATION_OLIVINE_CITY_HNS);
    healBefore = gSaveBlock1Ptr->lastHealLocation;
    EXPECT(!WayfarerHoennStateIsInitialized());
    EXPECT(!GetRegionVisitedState(REGION_HOENN));

    // Boarding the Tidal at Olivine: the state is ready before the first warp onto a Hoenn-source map.
    EXPECT(WayfarerInitializeHoennState(MAP_GROUP(MAP_SS_TIDAL_CORRIDOR), MAP_NUM(MAP_SS_TIDAL_CORRIDOR), 1, 10));
    EXPECT(WayfarerHoennStateIsInitialized());
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_JOHTO);
    EXPECT(!GetRegionVisitedState(REGION_HOENN));
    EXPECT_EQ(memcmp(&healBefore, &gSaveBlock1Ptr->lastHealLocation, sizeof(healBefore)), 0);

    // Going ashore at Slateport then arrives in Hoenn: the region and the respawn are set then.
    EXPECT(WayfarerPrepareHoennEntry(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
                                     9, 11, HEAL_LOCATION_SLATEPORT_CITY));
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_HOENN);
    EXPECT(GetRegionVisitedState(REGION_HOENN));
    EXPECT(memcmp(&healBefore, &gSaveBlock1Ptr->lastHealLocation, sizeof(healBefore)) != 0);
}

TEST("Ferry: the Kanto and Hoenn starts hide the maiden voyage's story objects, the Johto start keeps them")
{
    static const u16 sStarts[] = {ORIGIN_PALLET, ORIGIN_LITTLEROOT, ORIGIN_NEW_BARK};
    u32 start;

    for (start = 0; start < ARRAY_COUNT(sStarts); start++)
    {
        bool32 hidden = sStarts[start] != ORIGIN_NEW_BARK;

        ResetFerryFixture();
        FlagClear(FLAG_HIDE_SSAQUA_CAPTAINS_ROOM_GRANDDAUGHTER);
        FlagClear(FLAG_HIDE_SSAQUA_ROOM_SSE_GRANDDAUGHTER);
        gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NONE;
        EXPECT(WayfarerInitializeOrigin(sStarts[start]));
        EXPECT_EQ(FlagGet(FLAG_HIDE_SSAQUA_CAPTAINS_ROOM_GRANDDAUGHTER), hidden);
        EXPECT_EQ(FlagGet(FLAG_HIDE_SSAQUA_ROOM_SSE_GRANDDAUGHTER), hidden);
    }
}

TEST("Ferry: a refused Hoenn state initialization or arrival changes nothing")
{
    ResetFerryFixture();
    WayfarerSetSavedCurrentRegion(REGION_JOHTO);
    // A map that isn't Hoenn-source.
    EXPECT(!WayfarerInitializeHoennState(MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS), 8, 9));
    EXPECT(!WayfarerHoennStateIsInitialized());
    // A wall tile on the Tidal corridor.
    EXPECT(!WayfarerInitializeHoennState(MAP_GROUP(MAP_SS_TIDAL_CORRIDOR), MAP_NUM(MAP_SS_TIDAL_CORRIDOR), 0, 0));
    EXPECT(!WayfarerHoennStateIsInitialized());
    EXPECT(!WayfarerPrepareHoennEntry(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
                                      9, 11, HEAL_LOCATION_OLIVINE_CITY_HNS));
    EXPECT(!WayfarerHoennStateIsInitialized());
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_JOHTO);
}

TEST("Ferry: the Lilycove arrival tile is a valid Hoenn arrival, so going ashore there is never refused")
{
    ResetFerryFixture();
    EXPECT(WayfarerPrepareHoennEntry(MAP_GROUP(MAP_LILYCOVE_CITY_HARBOR), MAP_NUM(MAP_LILYCOVE_CITY_HARBOR),
                                     8, 11, HEAL_LOCATION_LILYCOVE_CITY));
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_HOENN);
    EXPECT(WayfarerHoennStateIsInitialized());
}


static void StandAt(u32 mapGroup, u32 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
}

static void LoadBerth(u32 mapGroup, u32 mapNum, u32 hour)
{
    StandAt(mapGroup, mapNum);
    FakeRtc_ManuallySetTime(1, hour, 0, 0);
    Ferry_OnMapLoad();
}

TEST("Ferry: each dock berth shows the scheduled ship's graphics by day and by night, or hides when empty")
{
    ResetFerryFixture();
    PutEveryShipInService();
    // Olivine: Aqua by day, Tidal by night.
    LoadBerth(MAP_GROUP(MAP_OLIVINE_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_OLIVINE_CITY_PORT_INSIDE_HNS), 12);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_AQUA_HNS);
    EXPECT(!FlagGet(FLAG_TEMP_1F));
    LoadBerth(MAP_GROUP(MAP_OLIVINE_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_OLIVINE_CITY_PORT_INSIDE_HNS), 22);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_TIDAL);
    // Vermilion: Aqua by day, the Anne by night, one row further out so she clears the sailor and the walkway.
    LoadBerth(MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS), 12);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_AQUA_HNS);
    EXPECT(!FlagGet(FLAG_TEMP_1F));
    LoadBerth(MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS), 22);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_ANNE);
    EXPECT(!FlagGet(FLAG_TEMP_1F));
    // Slateport: Tidal by day, Aqua by night; Lilycove: Aqua by day, Tidal by night.
    LoadBerth(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR), 12);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_TIDAL);
    LoadBerth(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR), 22);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_AQUA_HNS);
    LoadBerth(MAP_GROUP(MAP_LILYCOVE_CITY_HARBOR), MAP_NUM(MAP_LILYCOVE_CITY_HARBOR), 12);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_AQUA_HNS);
    LoadBerth(MAP_GROUP(MAP_LILYCOVE_CITY_HARBOR), MAP_NUM(MAP_LILYCOVE_CITY_HARBOR), 22);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_TIDAL);
    // An empty berth hides its ship: the Anne is out of service until her reward.
    FlagClear(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED);
    LoadBerth(MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS), 22);
    EXPECT(FlagGet(FLAG_TEMP_1F));
    // The maiden voyage keeps Olivine's Aqua in the water even at night, for the Johto start.
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    LoadBerth(MAP_GROUP(MAP_OLIVINE_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_OLIVINE_CITY_PORT_INSIDE_HNS), 22);
    EXPECT_EQ(VarGet(VAR_OBJ_GFX_ID_E), OBJ_EVENT_GFX_SS_AQUA_HNS);
    EXPECT(!FlagGet(FLAG_TEMP_1F));
    // The berth's flag is not left set for the next map.
    LoadBerth(MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS), 22);
}

TEST("Ferry: a berth whose ship changes is removed and spawned again, never re-skinned in place")
{
    // Re-skinning keeps the old sprite's tile allocation (60 tiles), and the Anne needs 128.
    EXPECT(Ferry_BerthNeedsRespawn(FERRY_SHIP_AQUA + 1, FERRY_SHIP_ANNE));
    EXPECT(Ferry_BerthNeedsRespawn(FERRY_SHIP_ANNE + 1, FERRY_SHIP_AQUA));
    EXPECT(Ferry_BerthNeedsRespawn(FERRY_SHIP_AQUA + 1, FERRY_SHIP_NONE));
    EXPECT(!Ferry_BerthNeedsRespawn(FERRY_SHIP_AQUA + 1, FERRY_SHIP_AQUA));
    EXPECT(!Ferry_BerthNeedsRespawn(0, FERRY_SHIP_AQUA));
}

TEST("Ferry: the launch ports board the ship at anchor, and an empty berth refuses")
{
    const struct WayfarerVoyage *voyage = Ferry_GetVoyage();

    ResetFerryFixture();
    PutEveryShipInService();
    // One Island by day: the Anne, stop 1 of her loop.
    StandAt(MAP_GROUP(MAP_ONE_ISLAND_HARBOR), MAP_NUM(MAP_ONE_ISLAND_HARBOR));
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
        EXPECT(Ferry_Board());
    EXPECT_EQ((u32)voyage->ship, FERRY_SHIP_ANNE);
    EXPECT_EQ((u32)voyage->stop, 1);
    Ferry_ClearVoyage();
    // ... but 0:00-4:00 the berth is empty and boarding is refused.
    FakeRtc_ManuallySetTime(1, 2, 0, 0);
        EXPECT_EQ(Ferry_GetBerthShip(), FERRY_SHIP_NONE);
    EXPECT(!Ferry_Board());
    EXPECT(!Ferry_IsAboard());
    // Seven Island: the Anne by day, the Tidal by night (which readies Hoenn's state for its maps).
    StandAt(MAP_GROUP(MAP_SEVEN_ISLAND_HARBOR), MAP_NUM(MAP_SEVEN_ISLAND_HARBOR));
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
        EXPECT(Ferry_Board());
    EXPECT_EQ((u32)voyage->ship, FERRY_SHIP_ANNE);
    EXPECT_EQ((u32)voyage->stop, 3);
    Ferry_ClearVoyage();
    FakeRtc_ManuallySetTime(1, 22, 0, 0);
        EXPECT(Ferry_BufferBoarding());
    EXPECT(Ferry_Board());
    EXPECT_EQ((u32)voyage->ship, FERRY_SHIP_TIDAL);
    EXPECT_EQ((u32)voyage->stop, 2);
    EXPECT(WayfarerHoennStateIsInitialized());
    Ferry_ClearVoyage();
    // Melemele: the Anne, stop 2.
    StandAt(MAP_GROUP(MAP_MELEMELE_ISLE_HNS), MAP_NUM(MAP_MELEMELE_ISLE_HNS));
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
        EXPECT(Ferry_Board());
    EXPECT_EQ((u32)voyage->ship, FERRY_SHIP_ANNE);
    EXPECT_EQ((u32)voyage->stop, 2);
    // Boarding needs a ship in service: without the reward there is nothing to board at Melemele.
    Ferry_ClearVoyage();
    FlagClear(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED);
        EXPECT(!Ferry_Board());
}

TEST("Ferry: a blackout aboard returns the player to the last heal location with the voyage cleared")
{
    ResetFerryFixture();
    SetLastHealLocationWarp(HEAL_LOCATION_OLIVINE_CITY_HNS);
    EXPECT(Ferry_BoardAt(FERRY_PORT_VERMILION, FERRY_SHIP_AQUA, 1000));
    SetWarpDestinationToLastHealLocation();
    EXPECT(!Ferry_IsAboard());
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapNum, MAP_NUM(MAP_OLIVINE_CITY_HNS));
}

TEST("Ferry: the Anne's first departure: getting off before cast-off leaves the player in Vermilion with it cleared")
{
    const struct WayfarerVoyage *voyage = Ferry_GetVoyage();

    ResetFerryFixture();
    FlagSet(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED);
    StandAt(MAP_GROUP(MAP_SSANNE_CAPTAINS_OFFICE), MAP_NUM(MAP_SSANNE_CAPTAINS_OFFICE));
    EXPECT(Ferry_ArmAnneDeparture());
    EXPECT(voyage->annePending != 0);
    // Walking down the gangway: the Vermilion berth map loads with the voyage still set.
    StandAt(MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS));
    Ferry_OnMapLoad();
    EXPECT(!Ferry_IsAboard());
    EXPECT(voyage->annePending == 0);
    EXPECT(!Ferry_IsAnnePendingAtVermilion());
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapNum, MAP_NUM(MAP_VERMILION_CITY_HNS));
}

TEST("Ferry: going ashore at Melemele by the gangway runs the Alola first-visit flags once")
{
    ResetFerryFixture();
    FlagClear(FLAG_VISITED_ALOLA);
    FlagClear(FLAG_HIDE_TAPU_KOKO);
    FlagClear(FLAG_HIDE_FLYOVER);
    EXPECT(Ferry_BoardAt(FERRY_PORT_SEVEN_ISLAND, FERRY_SHIP_ANNE, 0));
    StandAt(MAP_GROUP(MAP_SSANNE_1F_CORRIDOR), MAP_NUM(MAP_SSANNE_1F_CORRIDOR));
    StandAt(MAP_GROUP(MAP_MELEMELE_ISLE_HNS), MAP_NUM(MAP_MELEMELE_ISLE_HNS));
    Ferry_OnMapLoad();
    EXPECT(!Ferry_IsAboard());
    EXPECT(FlagGet(FLAG_HIDE_TAPU_KOKO));
    EXPECT(FlagGet(FLAG_HIDE_FLYOVER));
    EXPECT(FlagGet(FLAG_HIDE_SAMSONOAK_INTRO));
    EXPECT_EQ(gSaveBlock1Ptr->lastHealLocation.mapNum, MAP_NUM(MAP_MELEMELE_ISLE_HNS));
}

TEST("Ferry: Lilycove's menu rows map to the ship, each charter and Cancel in order")
{
    u8 ids[5];

    ResetFerryFixture();
    StandAt(MAP_GROUP(MAP_LILYCOVE_CITY_HARBOR), MAP_NUM(MAP_LILYCOVE_CITY_HARBOR));
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
    // A ship in port and Scott's invite: the Aqua first, then the Frontier, then Cancel.
    FlagSet(HOENN_FLAG_ID(0x1D0));
    EXPECT_EQ(Ferry_GetCharterRows(ids), 3);
    EXPECT_EQ(ids[0], FERRY_CHARTER_SHIP);
    EXPECT_EQ(ids[1], FERRY_CHARTER_BATTLE_FRONTIER);
    EXPECT_EQ(ids[2], FERRY_CHARTER_CANCEL);
    // Every charter at once.
    EXPECT(AddBagItem(ITEM_EON_TICKET, 1));
    EXPECT(AddBagItem(ITEM_OLD_SEA_MAP, 1));
    EXPECT_EQ(Ferry_GetCharterRows(ids), 5);
    EXPECT_EQ(ids[1], FERRY_CHARTER_SOUTHERN_ISLAND);
    EXPECT_EQ(ids[2], FERRY_CHARTER_FARAWAY_ISLAND);
    EXPECT_EQ(ids[3], FERRY_CHARTER_BATTLE_FRONTIER);
    EXPECT_EQ(ids[4], FERRY_CHARTER_CANCEL);
    // An empty berth (the Anne is out of service at night here) drops the ship row; the ids stay fixed.
    FakeRtc_ManuallySetTime(1, 22, 0, 0);
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    VarSet(VAR_SSAQUA_STATE, 0);
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
    EXPECT_EQ(Ferry_GetCharterRows(ids), 4);
    EXPECT_EQ(ids[0], FERRY_CHARTER_SOUTHERN_ISLAND);
    EXPECT_EQ(ids[3], FERRY_CHARTER_CANCEL);
    FlagClear(HOENN_FLAG_ID(0x1D0));
}

TEST("Ferry: the Lilycove charters are offered by their conditions, and none means boarding directly")
{
    ResetFerryFixture();
    EXPECT_EQ(GetFerryLilycoveCharterCount(), 0);
    EXPECT(AddBagItem(ITEM_EON_TICKET, 1));
    EXPECT_EQ(GetFerryLilycoveCharterCount(), 1);
    EXPECT(AddBagItem(ITEM_OLD_SEA_MAP, 1));
    EXPECT_EQ(GetFerryLilycoveCharterCount(), 2);
    // The Battle Frontier is Scott's invite: the Hoenn-bank flag his scene on the Tidal sets.
    FlagSet(HOENN_FLAG_ID(0x1D0));
    EXPECT_EQ(GetFerryLilycoveCharterCount(), 3);
    EXPECT(RemoveBagItem(ITEM_EON_TICKET, 1));
    EXPECT(RemoveBagItem(ITEM_OLD_SEA_MAP, 1));
    EXPECT_EQ(GetFerryLilycoveCharterCount(), 1);
    FlagClear(HOENN_FLAG_ID(0x1D0));
    EXPECT_EQ(GetFerryLilycoveCharterCount(), 0);
    // No game-clear gate and no FLAG_ENABLE_SHIP_* flags.
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
    EXPECT(AddBagItem(ITEM_EON_TICKET, 1));
    EXPECT_EQ(GetFerryLilycoveCharterCount(), 1);
}


// ---------- Cabins ----------

extern MainCallback Test_EndTrainerBattleCallback(void);

static void GiveParty(u32 species, u32 level)
{
    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], species, level, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[0]);
    gPlayerPartyCount = 1;
}

static void AddToParty(u32 species, u32 level)
{
    CreateMon(&gPlayerParty[gPlayerPartyCount], species, level, 0, OTID_STRUCT_PLAYER_ID);
    CalculateMonStats(&gPlayerParty[gPlayerPartyCount]);
    gPlayerPartyCount++;
}

static void SetHp(u32 slot, u16 hp)
{
    SetMonData(&gPlayerParty[slot], MON_DATA_HP, &hp);
}

// Boards a ship and puts the two drawn hosts to the ones given (table indexes), params at the list's first entry.
static void BoardWithHosts(u8 ship, u8 first, u8 second)
{
    struct WayfarerVoyage voyage = *Ferry_GetVoyage();

    EXPECT(Ferry_BoardAt(Ferry_GetLoopPort(ship, 0), ship, Ferry_GetNow()));
    voyage = *Ferry_GetVoyage();
    voyage.hosts[0] = first + 1;
    voyage.hosts[1] = second + 1;
    voyage.hostParams[0] = 0;
    voyage.hostParams[1] = 0;
    Test_Ferry_SetVoyage(&voyage);
}

TEST("Ferry: boarding draws two distinct hosts of the ship's own pool, with a parameter each, and saves them")
{
    const struct WayfarerVoyage *voyage = Ferry_GetVoyage();
    u8 ships[] = {FERRY_SHIP_AQUA, FERRY_SHIP_ANNE, FERRY_SHIP_TIDAL};
    u32 ship, attempt, host, pool;
    bool32 sawDifferentPair = FALSE;
    u8 firstPair = 0;

    ResetFerryFixture();
    for (ship = 0; ship < ARRAY_COUNT(ships); ship++)
    {
        pool = 0;
        for (host = 0; host < Test_Ferry_HostCount(); host++)
            if (Test_Ferry_HostShip(host) == ships[ship])
                pool++;
        EXPECT(pool >= 7);
        for (attempt = 0; attempt < 40; attempt++)
        {
            u32 slot;

            EXPECT(Ferry_BoardAt(Ferry_GetLoopPort(ships[ship], 0), ships[ship], 100));
            EXPECT(voyage->hosts[0] != voyage->hosts[1]);
            for (slot = 0; slot < 2; slot++)
            {
                EXPECT(voyage->hosts[slot] >= 1 && voyage->hosts[slot] <= Test_Ferry_HostCount());
                EXPECT_EQ(Test_Ferry_HostShip(voyage->hosts[slot] - 1), ships[ship]);
                EXPECT(voyage->hostParams[slot] < Test_Ferry_HostParamCount(voyage->hosts[slot] - 1));
            }
            if (attempt == 0)
                firstPair = voyage->hosts[0] * 64 + voyage->hosts[1];
            else if (voyage->hosts[0] * 64 + voyage->hosts[1] != firstPair)
                sawDifferentPair = TRUE;
        }
    }
    // The draw is random (not the same pair every time), and what was drawn stays through the voyage's events.
    EXPECT(sawDifferentPair);
    {
        u8 hosts0 = voyage->hosts[0], hosts1 = voyage->hosts[1], param0 = voyage->hostParams[0];

        Ferry_Advance(voyage->eventAt + FERRY_SEA_MINUTES + FERRY_PORT_MINUTES);
        EXPECT_EQ(voyage->hosts[0], hosts0);
        EXPECT_EQ(voyage->hosts[1], hosts1);
        EXPECT_EQ(voyage->hostParams[0], param0);
    }
}

TEST("Ferry: the Anne's, Aqua's and Tidal's pools carry the spec's kinds")
{
    ResetFerryFixture();
    EXPECT(Test_Ferry_FindHost(FERRY_SHIP_ANNE, FERRY_REQ_BATTLE) != 0xFF);
    EXPECT(Test_Ferry_FindHost(FERRY_SHIP_ANNE, FERRY_REQ_REGION) != 0xFF);
    EXPECT(Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_MOVE) != 0xFF);
    EXPECT(Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_ITEM) != 0xFF);
    EXPECT(Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_COIN) != 0xFF);
    EXPECT(Test_Ferry_FindHost(FERRY_SHIP_TIDAL, FERRY_REQ_DOUBLE_BATTLE) != 0xFF);
    EXPECT(Test_Ferry_FindHost(FERRY_SHIP_TIDAL, FERRY_REQ_FRIENDSHIP) != 0xFF);
    EXPECT(Test_Ferry_FindHost(FERRY_SHIP_TIDAL, FERRY_REQ_COLOUR) != 0xFF);
}

TEST("Ferry: a request is met by an item in the Bag, which is taken, or a key item, which is not")
{
    ResetFerryFixture();
    EXPECT(!Ferry_RequestMet(FERRY_REQ_ITEM, ITEM_ORAN_BERRY));
    EXPECT(AddBagItem(ITEM_ORAN_BERRY, 1));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_ITEM, ITEM_PECHA_BERRY));
    EXPECT(Ferry_RequestMet(FERRY_REQ_ITEM, ITEM_ORAN_BERRY));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_ORAN_BERRY), 0);
    EXPECT(!Ferry_RequestMet(FERRY_REQ_ITEM, ITEM_ORAN_BERRY));

    EXPECT(!Ferry_RequestMet(FERRY_REQ_KEY_ITEM, ITEM_POKE_FLUTE));
    EXPECT(AddBagItem(ITEM_POKE_FLUTE, 1));
    EXPECT(Ferry_RequestMet(FERRY_REQ_KEY_ITEM, ITEM_POKE_FLUTE));
    EXPECT_EQ(CountTotalItemQuantityInBag(ITEM_POKE_FLUTE), 1);
}

TEST("Ferry: a request is met by a party Pokemon's type, species, colour, move or fully evolved form, never an Egg's")
{
    u16 move = MOVE_CUT;
    u8 egg = TRUE;

    ResetFerryFixture();
    GiveParty(SPECIES_MACHAMP, 40);
    EXPECT(Ferry_RequestMet(FERRY_REQ_TYPE, TYPE_FIGHTING));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_TYPE, TYPE_FIRE));
    EXPECT(Ferry_RequestMet(FERRY_REQ_SPECIES, SPECIES_MACHAMP));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_SPECIES, SPECIES_MACHOP));
    EXPECT(Ferry_RequestMet(FERRY_REQ_COLOUR, gSpeciesInfo[SPECIES_MACHAMP].bodyColor));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_COLOUR, gSpeciesInfo[SPECIES_MACHAMP].bodyColor == BODY_COLOR_PINK ? BODY_COLOR_BLUE : BODY_COLOR_PINK));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_MOVE, MOVE_CUT));
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE1, &move);
    EXPECT(Ferry_RequestMet(FERRY_REQ_MOVE, MOVE_CUT));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_MOVE, MOVE_FLASH));
    EXPECT(Ferry_RequestMet(FERRY_REQ_EVOLVED, 0));

    // Not fully evolved (Machop evolves), and no pre-evolution (Tauros stands alone).
    GiveParty(SPECIES_MACHOP, 5);
    EXPECT(!Ferry_RequestMet(FERRY_REQ_EVOLVED, 0));
    GiveParty(SPECIES_TAUROS, 30);
    EXPECT(!Ferry_RequestMet(FERRY_REQ_EVOLVED, 0));

    // An Egg shows nothing: not its type, species, colour or form.
    GiveParty(SPECIES_MACHAMP, 40);
    SetMonData(&gPlayerParty[0], MON_DATA_IS_EGG, &egg);
    EXPECT(!Ferry_RequestMet(FERRY_REQ_TYPE, TYPE_FIGHTING));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_SPECIES, SPECIES_MACHAMP));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_COLOUR, gSpeciesInfo[SPECIES_MACHAMP].bodyColor));
    EXPECT(!Ferry_RequestMet(FERRY_REQ_EVOLVED, 0));
    // A second party Pokemon is looked at too.
    AddToParty(SPECIES_PIKACHU, 20);
    EXPECT(Ferry_RequestMet(FERRY_REQ_TYPE, TYPE_ELECTRIC));
}

TEST("Ferry: a request is met by a Pokemon from the start region's Pokedex or one that adores the player")
{
    u8 friendship = 219;

    ResetFerryFixture();
    GiveParty(SPECIES_CHIKORITA, 10);
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NEW_BARK;
    EXPECT(Ferry_RequestMet(FERRY_REQ_REGION, 0));
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_PALLET;
    EXPECT(!Ferry_RequestMet(FERRY_REQ_REGION, 0));
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_LITTLEROOT;
    EXPECT(!Ferry_RequestMet(FERRY_REQ_REGION, 0));
    GiveParty(SPECIES_TREECKO, 10);
    EXPECT(Ferry_RequestMet(FERRY_REQ_REGION, 0));
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_PALLET;
    GiveParty(SPECIES_PIKACHU, 10);
    EXPECT(Ferry_RequestMet(FERRY_REQ_REGION, 0));

    SetMonData(&gPlayerParty[0], MON_DATA_FRIENDSHIP, &friendship);
    EXPECT(!Ferry_RequestMet(FERRY_REQ_FRIENDSHIP, 0));
    friendship = 220;
    SetMonData(&gPlayerParty[0], MON_DATA_FRIENDSHIP, &friendship);
    EXPECT(Ferry_RequestMet(FERRY_REQ_FRIENDSHIP, 0));
}

TEST("Ferry: the coin flip comes up both ways, and the quiz takes the right answer only")
{
    u32 i, heads = 0;
    u8 host;

    ResetFerryFixture();
    for (i = 0; i < 100; i++)
        heads += Ferry_RequestMet(FERRY_REQ_COIN, 0);
    EXPECT(heads > 10 && heads < 90);

    host = Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_QUIZ);
    BoardWithHosts(FERRY_SHIP_AQUA, host, Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_COIN));
    Test_Ferry_StandBeforeHost(host);
    for (i = 0; i < Test_Ferry_HostParamCount(host); i++)
    {
        u32 choice, right = 0;
        struct WayfarerVoyage voyage = *Ferry_GetVoyage();

        voyage.hostParams[0] = i;
        Test_Ferry_SetVoyage(&voyage);
        Ferry_BufferQuiz();
        for (choice = 0; choice < 3; choice++)
        {
            gSpecialVar_0x8004 = choice;
            right += Ferry_QuizCorrect();
        }
        EXPECT_EQ(right, 1);
    }
}

TEST("Ferry: talking to a host: the request while open, small talk once settled, the bed or the sleep once earned")
{
    u8 mover = 0, fisher = 0;
    struct WayfarerVoyage voyage;

    ResetFerryFixture();
    mover = Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_MOVE);
    fisher = Test_Ferry_FindHost(FERRY_SHIP_TIDAL, FERRY_REQ_FRIENDSHIP);
    BoardWithHosts(FERRY_SHIP_AQUA, mover, Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_COIN));
    // Standing before somebody who isn't one of the voyage's hosts.
    Test_Ferry_StandBeforeHost(Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_QUIZ));
    EXPECT_EQ(Ferry_HostTalk(), FERRY_HOST_NONE);

    Test_Ferry_StandBeforeHost(mover);
    GiveParty(SPECIES_MACHAMP, 40);
    EXPECT_EQ(Ferry_HostTalk(), FERRY_HOST_ASK);
    // Not met: a retry kind stays open.
    EXPECT(!Ferry_CheckRequest());
    EXPECT(!Ferry_IsHostOneTry());
    Ferry_HostMissed();
    EXPECT_EQ((u32)Ferry_GetVoyage()->hostsDone, 0);
    EXPECT_EQ(Ferry_HostTalk(), FERRY_HOST_ASK);
    // Met: the cabin is the player's and the request is settled; the Aqua's cabins have no bed, so they offer the sleep.
    Ferry_HostMet();
    EXPECT_EQ((u32)Ferry_GetVoyage()->cabin, 1);
    EXPECT_EQ((u32)Ferry_GetVoyage()->hostsDone, 1);
    EXPECT_EQ(Ferry_HostTalk(), FERRY_HOST_SLEEP);

    // The other host is still open; earning one cabin settles nothing else.
    Test_Ferry_StandBeforeHost(Test_Ferry_FindHost(FERRY_SHIP_AQUA, FERRY_REQ_COIN));
    EXPECT_EQ(Ferry_HostTalk(), FERRY_HOST_ASK);
    EXPECT(Ferry_IsHostOneTry());
    // A one-try kind is settled when failed: small talk afterwards, and the cabin isn't theirs.
    Ferry_HostMissed();
    EXPECT_EQ((u32)Ferry_GetVoyage()->hostsDone, 3);
    EXPECT_EQ((u32)Ferry_GetVoyage()->cabin, 1);
    EXPECT_EQ(Ferry_HostTalk(), FERRY_HOST_CHAT);

    // On the Tidal the breeder's cabin has the bed: earning it makes them point at it.
    BoardWithHosts(FERRY_SHIP_TIDAL, fisher, Test_Ferry_FindHost(FERRY_SHIP_TIDAL, FERRY_REQ_COLOUR));
    Test_Ferry_StandBeforeHost(fisher);
    Ferry_HostMet();
    EXPECT_EQ(Ferry_HostTalk(), FERRY_HOST_BED);
    EXPECT(Ferry_IsBedMine());
    voyage = *Ferry_GetVoyage();
    voyage.cabin = 0;
    Test_Ferry_SetVoyage(&voyage);
    EXPECT(!Ferry_IsBedMine());
}

TEST("Ferry: a lost host battle leaves the player aboard with the fainted at 1 HP, and never whites out")
{
    u8 host;
    u32 i;

    ResetFerryFixture();
    host = Test_Ferry_FindHost(FERRY_SHIP_ANNE, FERRY_REQ_BATTLE);
    BoardWithHosts(FERRY_SHIP_ANNE, host, Test_Ferry_FindHost(FERRY_SHIP_ANNE, FERRY_REQ_QUIZ));
    Test_Ferry_StandBeforeHost(host);
    GiveParty(SPECIES_MACHAMP, 40);
    AddToParty(SPECIES_PIKACHU, 20);
    AddToParty(SPECIES_PIKACHU, 20);
    EXPECT_EQ(Ferry_HostTalk(), FERRY_HOST_ASK);
    EXPECT(Ferry_IsHostOneTry());

    EXPECT(Ferry_SetUpHostBattle());
    EXPECT(FlagGet(FLAG_NO_WHITEOUT));
    EXPECT(TRAINER_BATTLE_PARAM.opponentA != TRAINER_NONE);
    // The party is beaten (one Pokemon is left with an HP of 7).
    SetHp(0, 0);
    SetHp(1, 7);
    SetHp(2, 0);
    gBattleTypeFlags = BATTLE_TYPE_TRAINER;
    gBattleOutcome = B_OUTCOME_LOST;
    EXPECT(Test_EndTrainerBattleCallback() == CB2_ReturnToFieldContinueScriptPlayMapMusic);
    EXPECT(!Ferry_EndHostBattle());
    EXPECT(!FlagGet(FLAG_NO_WHITEOUT));
    EXPECT_EQ(GetMonData(&gPlayerParty[0], MON_DATA_HP), 1);
    EXPECT_EQ(GetMonData(&gPlayerParty[1], MON_DATA_HP), 7);
    EXPECT_EQ(GetMonData(&gPlayerParty[2], MON_DATA_HP), 1);
    Ferry_HostMissed();
    EXPECT_EQ((u32)Ferry_GetVoyage()->hostsDone, 1);
    EXPECT_EQ((u32)Ferry_GetVoyage()->cabin, 0);
    EXPECT(Ferry_IsAboard());

    // Without the flag the same loss whites out (so the flag is what keeps the player aboard).
    for (i = 0; i < 3; i++)
        SetHp(i, 0);
    EXPECT(Test_EndTrainerBattleCallback() == CB2_WhiteOut);
}

TEST("Ferry: a won host battle earns the cabin and leaves the borrowed trainer's own flag as it was")
{
    u8 host;
    u16 trainer;

    ResetFerryFixture();
    host = Test_Ferry_FindHost(FERRY_SHIP_TIDAL, FERRY_REQ_DOUBLE_BATTLE);
    BoardWithHosts(FERRY_SHIP_TIDAL, host, Test_Ferry_FindHost(FERRY_SHIP_TIDAL, FERRY_REQ_COLOUR));
    Test_Ferry_StandBeforeHost(host);
    // A double battle needs two Pokemon able to fight.
    GiveParty(SPECIES_MACHAMP, 40);
    EXPECT(!Ferry_SetUpHostBattle());
    EXPECT(!FlagGet(FLAG_NO_WHITEOUT));
    AddToParty(SPECIES_PIKACHU, 20);
    EXPECT(Ferry_SetUpHostBattle());
    trainer = TRAINER_BATTLE_PARAM.opponentA;
    EXPECT(!HasTrainerBeenFought(trainer));
    gBattleTypeFlags = BATTLE_TYPE_TRAINER | BATTLE_TYPE_DOUBLE;
    gBattleOutcome = B_OUTCOME_WON;
    Test_EndTrainerBattleCallback();
    EXPECT(HasTrainerBeenFought(trainer));
    EXPECT(Ferry_EndHostBattle());
    EXPECT(!HasTrainerBeenFought(trainer));
    EXPECT(!FlagGet(FLAG_NO_WHITEOUT));
    Ferry_HostMet();
    EXPECT_EQ((u32)Ferry_GetVoyage()->cabin, 1);

    // A trainer already beaten elsewhere stays beaten.
    SetTrainerFlag(trainer);
    EXPECT(Ferry_SetUpHostBattle());
    Test_EndTrainerBattleCallback();
    EXPECT(Ferry_EndHostBattle());
    EXPECT(HasTrainerBeenFought(trainer));
    ClearTrainerFlag(trainer);
}

// Puts the voyage at sea, its next event `minutes` of in-game time ahead of now.
static void SailWithEventIn(u32 minutes)
{
    struct WayfarerVoyage voyage = *Ferry_GetVoyage();

    voyage.phase = FERRY_PHASE_SEA;
    voyage.eventAt = Ferry_GetNow() + minutes;
    Test_Ferry_SetVoyage(&voyage);
}

TEST("Ferry: sleeping at sea moves the clock to the arrival exactly, and refuses in port")
{
    const struct WayfarerVoyage *voyage = Ferry_GetVoyage();
    u32 minutes, before;

    ResetFerryFixture();
    EXPECT(Ferry_BoardAt(FERRY_PORT_VERMILION, FERRY_SHIP_AQUA, Ferry_GetNow()));
    // In port: nothing moves.
    FakeRtc_ManuallySetTime(1, 9, 13, 20);
    before = Ferry_GetNow();
    EXPECT(Ferry_SleepUntilEvent() == FALSE);
    EXPECT_EQ(Ferry_GetNow(), before);
    EXPECT_EQ(Ferry_Sleep(), FALSE);
    EXPECT_EQ(Ferry_GetNow(), before);

    // At sea: from any point of the leg, and across midnight, the clock lands on eventAt and no later.
    for (minutes = 1; minutes <= FERRY_SEA_MINUTES; minutes += 59)
    {
        FakeRtc_ManuallySetTime(1, 21, 40, 0);
        SailWithEventIn(minutes);
        EXPECT(Ferry_SleepUntilEvent());
        EXPECT_EQ(Ferry_GetNow(), voyage->eventAt);
        // The arrival is then due, and it's the voyage's, not the sleep's: it still has to be run.
        EXPECT(Ferry_IsAtSea());
        EXPECT_EQ(Ferry_Advance(Ferry_GetNow()), FERRY_ANNOUNCE_ARRIVAL);
        EXPECT(!Ferry_IsAtSea());
        EXPECT_EQ(voyage->eventAt, Ferry_GetNow() + FERRY_PORT_MINUTES);
    }
    // Sleeping again at the arrival minute changes nothing, and the port stay is not skipped.
    FakeRtc_ManuallySetTime(1, 12, 0, 0);
    before = Ferry_GetNow();
    SailWithEventIn(0);
    EXPECT(Ferry_SleepUntilEvent());
    EXPECT_EQ(Ferry_GetNow(), before);
}

TEST("Ferry: the daily slots' day holds while aboard and is released ashore")
{
    ResetFerryFixture();
    gDailySlotsDebugFlags = DAILY_DEBUG_DAY;
    gDailySlotsDebugDay = 10;
    DailySlots_OnMapLoad();
    EXPECT_EQ(DailySlots_GetDay(), 10);
    EXPECT_EQ(gPokemonStoragePtr->dailySlots.stampDay, 10);

    EXPECT(Ferry_BoardAt(FERRY_PORT_VERMILION, FERRY_SHIP_AQUA, Ferry_GetNow()));
    EXPECT_EQ(Ferry_GetVoyage()->slotDay, 10);
    // Days pass aboard, and nothing new starts: the draw's day, and so every pick and snapshot stamped from it, stays.
    gDailySlotsDebugDay = 12;
    EXPECT_EQ(DailySlots_GetDay(), 10);
    DailySlots_OnMapLoad();
    EXPECT_EQ(gPokemonStoragePtr->dailySlots.stampDay, 10);

    Ferry_ClearVoyage();
    EXPECT_EQ(DailySlots_GetDay(), 12);
    DailySlots_OnMapLoad();
    EXPECT_EQ(gPokemonStoragePtr->dailySlots.stampDay, 12);
    gDailySlotsDebugFlags = 0;
}

TEST("Ferry: a ship's beds no longer heal")
{
    extern const u8 SSTidalRooms_EventScript_Bed[];
    extern const u8 SSAquaRooms_EventScript_Bed[];
    extern const u8 Ferry_EventScript_Bed[];
    extern const u8 Common_EventScript_OutOfCenterPartyHeal[];
    extern const u8 Route119_WeatherInstitute_1F_EventScript_Bed[];
    const u8 *scripts[] = {SSTidalRooms_EventScript_Bed, SSAquaRooms_EventScript_Bed, Ferry_EventScript_Bed};
    u32 i, offset;

    const u8 *healPointer = Common_EventScript_OutOfCenterPartyHeal;
    bool32 found;

    // Neither bed script calls the party heal. The Ferry bed is where both go, and it is checked through its
    // whole stretch of the script (the bed, the sleep question and its answers).
    for (i = 0; i < ARRAY_COUNT(scripts); i++)
    {
        u32 length = i == 2 ? 200 : 8;

        found = FALSE;
        for (offset = 0; offset + 4 <= length; offset++)
            found |= memcmp(scripts[i] + offset, &healPointer, 4) == 0;
        EXPECT(!found);
    }
    // The same scan does find the call in an ordinary bed (a bed of Route 119's Weather Institute).
    found = FALSE;
    for (offset = 0; offset < 24; offset++)
        found |= memcmp(Route119_WeatherInstitute_1F_EventScript_Bed + offset, &healPointer, 4) == 0;
    EXPECT(found);
    // The beds go to the Ferry bed.
    EXPECT_EQ(scripts[0][0], SCR_OP_GOTO);
    EXPECT_EQ(scripts[1][0], SCR_OP_GOTO);
}

#endif // IS_WAYFARER
