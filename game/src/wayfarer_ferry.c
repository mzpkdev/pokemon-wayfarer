// The S.S. ferries (Wayfarer only). Spec: .product/specs/ss-ferries.md.
//
// The schedule is a pure table. The voyage state lives in PokemonStorage after the daily
// slots (struct WayfarerVoyage) and is C-side only, so HnS and Hoenn maps read the same state
// through specials. Cabins, hosts and the daily-slot freeze are other parts of the spec: this
// file keeps their fields in the saved struct and leaves them zero.

#include "global.h"
#include "wayfarer_ferry.h"

#if IS_WAYFARER

#include "event_data.h"
#include "event_object_movement.h"
#include "field_screen_effect.h"
#include "item.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "constants/characters.h"
#include "constants/event_objects.h"
#include "constants/flags.h"
#include "constants/heal_locations.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/rtc.h"
#include "constants/siirtc.h"
#include "constants/vars.h"
#include "constants/wayfarer_persistence.h"

extern const u8 Alola_EventScript_FirstVisitFlags[];
extern const u8 Ferry_EventScript_PA_CastOff[];
extern const u8 Ferry_EventScript_PA_CastOffAlola[];
extern const u8 Ferry_EventScript_PA_Arrival[];
extern const u8 Ferry_EventScript_PA_ArrivalAlola[];

#define FERRY_SLATEPORT_PATRONS_FLAG HOENN_FLAG_ID(0x389)  // FLAG_HIDE_SLATEPORT_CITY_HARBOR_PATRONS
#define FERRY_BERTH_HIDE_FLAG        FLAG_TEMP_1F          // the berth ship object's flag; cleared on every map load
#define FERRY_CYCLE_MINUTES          (4 * (FERRY_PORT_MINUTES + FERRY_SEA_MINUTES))
#define MINUTES_PER_DAY              (HOURS_PER_DAY * MINUTES_PER_HOUR)

STATIC_ASSERT(MAP_NUM(MAP_SSAQUA_ROOM_SW_HNS) - MAP_NUM(MAP_SSAQUA_1F_HNS) + 1 == 11, FerryAquaMapRange);
STATIC_ASSERT(MAP_NUM(MAP_SSANNE_1F_ROOM6) - MAP_NUM(MAP_SSANNE_1F_CORRIDOR) + 1 == 25, FerryAnneMapRange);
STATIC_ASSERT(MAP_NUM(MAP_SS_TIDAL_ROOMS) - MAP_NUM(MAP_SS_TIDAL_CORRIDOR) + 1 == 3, FerryTidalMapRange);

struct FerryPort
{
    u8 mapGroup;
    u8 mapNum;
    s8 x;                 // the arrival tile
    s8 y;
    u8 healLocation;
    u8 kind;              // FERRY_BERTH_*
    bool8 hoenn;          // a Hoenn-source map: going ashore arrives in Hoenn
    u8 shipLocalId;       // the berth's ship object (docks only)
    u8 dayShip;
    u8 nightShip;
    bool8 emptyAtDeepNight;  // no ship 0:00-4:00 (the Anne anchors offshore)
    const u8 *name;
};

static const u8 sNameOlivine[] = _("OLIVINE CITY");
static const u8 sNameVermilion[] = _("VERMILION CITY");
static const u8 sNameSlateport[] = _("SLATEPORT CITY");
static const u8 sNameLilycove[] = _("LILYCOVE CITY");
static const u8 sNameOneIsland[] = _("ONE ISLAND");
static const u8 sNameSevenIsland[] = _("SEVEN ISLAND");
static const u8 sNameMelemele[] = _("MELEMELE ISLAND");

static const u8 sNameNone[] = _("");

static const u8 sShipNameAqua[] = _("S.S. AQUA");
static const u8 sShipNameAnne[] = _("S.S. ANNE");
static const u8 sShipNameTidal[] = _("S.S. TIDAL");

// Arrival tiles: the Seagallop's own tile (8,5) at the Sevii harbors stays free; Melemele's tile is
// beside the launch sailor, off the Route 13 boat's tile (25,33) and the Samson Oak trigger path.
static const struct FerryPort sPorts[FERRY_PORT_COUNT] =
{
    [FERRY_PORT_OLIVINE] =
    {
        MAP_GROUP(MAP_OLIVINE_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_OLIVINE_CITY_PORT_INSIDE_HNS), 8, 16,
        HEAL_LOCATION_OLIVINE_CITY_HNS, FERRY_BERTH_DOCK, FALSE, LOCALID_OLIVINEPORT_BOAT,
        FERRY_SHIP_AQUA, FERRY_SHIP_TIDAL, FALSE, sNameOlivine,
    },
    [FERRY_PORT_VERMILION] =
    {
        MAP_GROUP(MAP_VERMILION_CITY_PORT_INSIDE_HNS), MAP_NUM(MAP_VERMILION_CITY_PORT_INSIDE_HNS), 8, 9,
        HEAL_LOCATION_VERMILION_CITY_HNS, FERRY_BERTH_DOCK, FALSE, LOCALID_VERMILIONMARINA_BOAT,
        FERRY_SHIP_AQUA, FERRY_SHIP_ANNE, FALSE, sNameVermilion,
    },
    [FERRY_PORT_SLATEPORT] =
    {
        MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR), 9, 11,
        HEAL_LOCATION_SLATEPORT_CITY, FERRY_BERTH_DOCK, TRUE, LOCALID_SLATEPORT_HARBOR_SS_TIDAL,
        FERRY_SHIP_TIDAL, FERRY_SHIP_AQUA, FALSE, sNameSlateport,
    },
    [FERRY_PORT_LILYCOVE] =
    {
        MAP_GROUP(MAP_LILYCOVE_CITY_HARBOR), MAP_NUM(MAP_LILYCOVE_CITY_HARBOR), 8, 11,
        HEAL_LOCATION_LILYCOVE_CITY, FERRY_BERTH_DOCK, TRUE, LOCALID_LILYCOVE_HARBOR_SS_TIDAL,
        FERRY_SHIP_AQUA, FERRY_SHIP_TIDAL, FALSE, sNameLilycove,
    },
    [FERRY_PORT_ONE_ISLAND] =
    {
        MAP_GROUP(MAP_ONE_ISLAND_HARBOR), MAP_NUM(MAP_ONE_ISLAND_HARBOR), 9, 4,
        HEAL_LOCATION_ONE_ISLAND, FERRY_BERTH_LAUNCH, FALSE, 0,
        FERRY_SHIP_ANNE, FERRY_SHIP_ANNE, TRUE, sNameOneIsland,
    },
    [FERRY_PORT_SEVEN_ISLAND] =
    {
        MAP_GROUP(MAP_SEVEN_ISLAND_HARBOR), MAP_NUM(MAP_SEVEN_ISLAND_HARBOR), 9, 4,
        HEAL_LOCATION_SEVEN_ISLAND, FERRY_BERTH_LAUNCH, FALSE, 0,
        FERRY_SHIP_ANNE, FERRY_SHIP_TIDAL, FALSE, sNameSevenIsland,
    },
    [FERRY_PORT_MELEMELE] =
    {
        MAP_GROUP(MAP_MELEMELE_ISLE_HNS), MAP_NUM(MAP_MELEMELE_ISLE_HNS), 23, 33,
        HEAL_LOCATION_MELEMELE_ISLE_HNS, FERRY_BERTH_LAUNCH, FALSE, 0,
        FERRY_SHIP_ANNE, FERRY_SHIP_ANNE, TRUE, sNameMelemele,
    },
};

static const u8 sLoops[][FERRY_LOOP_LENGTH] =
{
    [FERRY_SHIP_AQUA] = { FERRY_PORT_OLIVINE, FERRY_PORT_VERMILION, FERRY_PORT_SLATEPORT, FERRY_PORT_LILYCOVE },
    [FERRY_SHIP_ANNE] = { FERRY_PORT_VERMILION, FERRY_PORT_ONE_ISLAND, FERRY_PORT_MELEMELE, FERRY_PORT_SEVEN_ISLAND },
    [FERRY_SHIP_TIDAL] = { FERRY_PORT_SLATEPORT, FERRY_PORT_LILYCOVE, FERRY_PORT_SEVEN_ISLAND, FERRY_PORT_OLIVINE },
};

struct FerryShip
{
    const u8 *name;
    u8 mapGroup;
    u8 firstMap;
    u8 lastMap;
    u8 entryGroup;
    u8 entryNum;
    s8 entryX;
    s8 entryY;
    u16 graphicsId;       // the berth object
    u8 homePort;          // where a ship map with no voyage lets you off
};

static const struct FerryShip sShips[] =
{
    [FERRY_SHIP_AQUA] =
    {
        sShipNameAqua, MAP_GROUP(MAP_SSAQUA_1F_HNS), MAP_NUM(MAP_SSAQUA_1F_HNS), MAP_NUM(MAP_SSAQUA_ROOM_SW_HNS),
        MAP_GROUP(MAP_SSAQUA_1F_HNS), MAP_NUM(MAP_SSAQUA_1F_HNS), 29, 3, OBJ_EVENT_GFX_SS_AQUA_HNS, FERRY_PORT_VERMILION,
    },
    [FERRY_SHIP_ANNE] =
    {
        sShipNameAnne, MAP_GROUP(MAP_SSANNE_1F_CORRIDOR), MAP_NUM(MAP_SSANNE_1F_CORRIDOR), MAP_NUM(MAP_SSANNE_1F_ROOM6),
        MAP_GROUP(MAP_SSANNE_1F_CORRIDOR), MAP_NUM(MAP_SSANNE_1F_CORRIDOR), 19, 2, OBJ_EVENT_GFX_SS_ANNE, FERRY_PORT_VERMILION,
    },
    [FERRY_SHIP_TIDAL] =
    {
        sShipNameTidal, MAP_GROUP(MAP_SS_TIDAL_CORRIDOR), MAP_NUM(MAP_SS_TIDAL_CORRIDOR), MAP_NUM(MAP_SS_TIDAL_ROOMS),
        MAP_GROUP(MAP_SS_TIDAL_CORRIDOR), MAP_NUM(MAP_SS_TIDAL_CORRIDOR), 1, 10, OBJ_EVENT_GFX_SS_TIDAL, FERRY_PORT_SLATEPORT,
    },
};

// The berth object the minute refresh still has to apply (a dialogue holds it back).
static EWRAM_DATA u8 sBerthShownPlusOne = 0;  // the ship the berth shows, plus one; zero until the berth is set up
static EWRAM_DATA bool8 sBerthDirty = FALSE;

static struct WayfarerVoyage *Voyage(void)
{
    return &gPokemonStoragePtr->voyage;
}

const struct WayfarerVoyage *Ferry_GetVoyage(void)
{
    return Voyage();
}

#if TESTING
void Test_Ferry_SetVoyage(const struct WayfarerVoyage *voyage)
{
    *Voyage() = *voyage;
}
#endif

// ---------- The schedule ----------

u8 Ferry_ScheduledShip(u8 port, u8 hour)
{
    const struct FerryPort *info;

    if (port >= FERRY_PORT_COUNT)
        return FERRY_SHIP_NONE;
    info = &sPorts[port];
    if (hour >= DAY_HOUR_BEGIN && hour < DAY_HOUR_END)
        return info->dayShip;
    if (info->emptyAtDeepNight && hour >= NIGHT_HOUR_BEGIN && hour < NIGHT_HOUR_END)
        return FERRY_SHIP_NONE;
    return info->nightShip;
}

bool8 Ferry_IsMaidenVoyagePending(void)
{
    return WayfarerCanUseAquaMaidenVoyage() && VarGet(VAR_SSAQUA_STATE) < 8;
}

bool8 Ferry_ShipInService(u8 ship)
{
    switch (ship)
    {
    case FERRY_SHIP_AQUA:
        return CheckBagHasItem(ITEM_SS_TICKET, 1) && !Ferry_IsMaidenVoyagePending();
    case FERRY_SHIP_ANNE:
        return FlagGet(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED);
    case FERRY_SHIP_TIDAL:
        return TRUE;
    }
    return FALSE;
}

u8 Ferry_ShipAt(u8 port, u8 hour)
{
    u8 ship = Ferry_ScheduledShip(port, hour);

    return ship != FERRY_SHIP_NONE && Ferry_ShipInService(ship) ? ship : FERRY_SHIP_NONE;
}

const u8 *Ferry_GetShipName(u8 ship)
{
    return ship >= FERRY_SHIP_AQUA && ship <= FERRY_SHIP_TIDAL ? sShips[ship].name : sNameNone;
}

u8 Ferry_GetPortStop(u8 ship, u8 port)
{
    u8 stop;

    if (ship == FERRY_SHIP_NONE || ship > FERRY_SHIP_TIDAL)
        return 0xFF;
    for (stop = 0; stop < FERRY_LOOP_LENGTH; stop++)
        if (sLoops[ship][stop] == port)
            return stop;
    return 0xFF;
}

u8 Ferry_GetLoopPort(u8 ship, u8 stop)
{
    if (ship == FERRY_SHIP_NONE || ship > FERRY_SHIP_TIDAL)
        return FERRY_PORT_NONE;
    return sLoops[ship][stop % FERRY_LOOP_LENGTH];
}

// ---------- Maps ----------

u8 Ferry_GetPortOfMap(s16 mapGroup, s16 mapNum)
{
    u8 port;

    for (port = 0; port < FERRY_PORT_COUNT; port++)
        if (sPorts[port].mapGroup == mapGroup && sPorts[port].mapNum == mapNum)
            return port;
    return FERRY_PORT_NONE;
}

u8 Ferry_GetShipOfMap(s16 mapGroup, s16 mapNum)
{
    u8 ship;

    for (ship = FERRY_SHIP_AQUA; ship <= FERRY_SHIP_TIDAL; ship++)
        if (sShips[ship].mapGroup == mapGroup && mapNum >= sShips[ship].firstMap && mapNum <= sShips[ship].lastMap)
            return ship;
    return FERRY_SHIP_NONE;
}

bool8 Ferry_IsShipMap(s16 mapGroup, s16 mapNum)
{
    return Ferry_GetShipOfMap(mapGroup, mapNum) != FERRY_SHIP_NONE;
}

// ---------- Voyage state ----------

bool8 Ferry_IsAboard(void)
{
    return Voyage()->ship != FERRY_SHIP_NONE;
}

bool8 Ferry_IsAtSea(void)
{
    return Voyage()->ship != FERRY_SHIP_NONE && Voyage()->phase == FERRY_PHASE_SEA;
}

// The in-game minute count the voyage schedules against: days x 1440 + hours x 60 + minutes.
u32 Ferry_GetNow(void)
{
    RtcCalcLocalTime();
    return (u32)gLocalTime.days * MINUTES_PER_DAY + gLocalTime.hours * MINUTES_PER_HOUR + gLocalTime.minutes;
}

void Ferry_InitNewGame(void)
{
    memset(Voyage(), 0, sizeof(struct WayfarerVoyage));
}

void Ferry_ClearVoyage(void)
{
    memset(Voyage(), 0, sizeof(struct WayfarerVoyage));
    sBerthDirty = TRUE;
}

// A blackout aboard clears the voyage; the player returns to the last heal location as on any ship.
void Ferry_OnBlackout(void)
{
    if (Ferry_IsAboard())
        Ferry_ClearVoyage();
}

// Boarding sets every field. The cabin fields (hosts, request parameters, cabin, settled requests)
// stay zero until the cabins part draws them; slotDay is frozen at the daily slots' current stamp so
// boarding never starts a new day by itself.
bool8 Ferry_BoardAt(u8 port, u8 ship, u32 now)
{
    struct WayfarerVoyage *voyage = Voyage();
    u8 stop = Ferry_GetPortStop(ship, port);
    bool8 annePending = voyage->annePending;

    if (stop == 0xFF)
        return FALSE;
    memset(voyage, 0, sizeof(*voyage));
    voyage->ship = ship;
    voyage->stop = stop;
    voyage->phase = FERRY_PHASE_PORT;
    voyage->eventAt = now + FERRY_PORT_MINUTES;
    voyage->slotDay = gPokemonStoragePtr->dailySlots.stampDay;
    voyage->annePending = annePending;
    return TRUE;
}

// Applies every elapsed transition in order. Events are scheduled, not reset by when they're noticed:
// cast-off adds a leg and arrival adds the port stay to eventAt. Only the last one is announced.
u8 Ferry_Advance(u32 now)
{
    struct WayfarerVoyage *voyage = Voyage();
    u8 announce = FERRY_ANNOUNCE_NONE;

    if (voyage->ship == FERRY_SHIP_NONE)
        return announce;
    // A save loaded after many days: whole cycles bring the ship back to the same place, so skip them.
    if (now > voyage->eventAt + 2 * FERRY_CYCLE_MINUTES)
        voyage->eventAt += ((now - voyage->eventAt) / FERRY_CYCLE_MINUTES - 1) * FERRY_CYCLE_MINUTES;
    while (now >= voyage->eventAt)
    {
        if (voyage->phase == FERRY_PHASE_PORT)
        {
            voyage->phase = FERRY_PHASE_SEA;
            voyage->stop = (voyage->stop + 1) % FERRY_LOOP_LENGTH;
            voyage->eventAt += FERRY_SEA_MINUTES;
            voyage->annePending = FALSE;
            announce = FERRY_ANNOUNCE_CAST_OFF;
        }
        else
        {
            voyage->phase = FERRY_PHASE_PORT;
            voyage->eventAt += FERRY_PORT_MINUTES;
            announce = FERRY_ANNOUNCE_ARRIVAL;
        }
    }
    return announce;
}

// ---------- The Anne's first departure ----------

static bool8 IsOnAnneMap(void)
{
    return Ferry_GetShipOfMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) == FERRY_SHIP_ANNE;
}

// The captain's reward arms the Anne's departure. Aboard the Anne already (the adventure), the voyage is
// set as for boarding at Vermilion, the table ignored this once. Cast-off is half an in-game hour away.
bool8 Ferry_ArmAnneDeparture(void)
{
    bool8 aboard = IsOnAnneMap();

    if (aboard && !Ferry_BoardAt(FERRY_PORT_VERMILION, FERRY_SHIP_ANNE, Ferry_GetNow()))
        return FALSE;
    Voyage()->annePending = TRUE;
    return aboard;
}

void Ferry_ArmAnneDepartureScript(void)
{
    Ferry_ArmAnneDeparture();
}

u16 Ferry_IsAnnePendingAtVermilion(void)
{
    return Voyage()->annePending && Voyage()->ship == FERRY_SHIP_NONE
        && Ferry_GetPortOfMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) == FERRY_PORT_VERMILION;
}

// ---------- Berths ----------

static u8 GetBerthPort(void)
{
    return Ferry_GetPortOfMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

static bool8 IsBerthClosed(u8 port)
{
    // The Aqua escape (from VAR_SLATEPORT_HARBOR_STATE 1 until the Mossdeep Gym) shuts Slateport's berth.
    return port == FERRY_PORT_SLATEPORT && FlagGet(FERRY_SLATEPORT_PATRONS_FLAG);
}

// What the berth shows now. The maiden voyage keeps Olivine's Aqua in the water whatever the table says.
static u8 GetShownShip(u8 port)
{
    if (IsBerthClosed(port))
        return FERRY_SHIP_NONE;
    if (port == FERRY_PORT_OLIVINE && Ferry_IsMaidenVoyagePending())
        return FERRY_SHIP_AQUA;
    return Ferry_ShipAt(port, GetTimeOfDayHours());
}

u16 Ferry_GetBerthPort(void)
{
    return GetBerthPort();
}

// What the sailor offers: the ship in. Before the Anne's first departure Vermilion offers her whatever the table says.
u16 Ferry_GetBerthShip(void)
{
    u8 port = GetBerthPort();

    if (port == FERRY_PORT_NONE || IsBerthClosed(port))
        return FERRY_SHIP_NONE;
    if (Ferry_IsAnnePendingAtVermilion())
        return FERRY_SHIP_ANNE;
    return GetShownShip(port);
}

static void ApplyBerth(bool8 live)
{
    u8 port = GetBerthPort();
    u8 ship;
    u16 graphicsId;
    u8 objectId;

    sBerthDirty = FALSE;
    if (port == FERRY_PORT_NONE || sPorts[port].kind != FERRY_BERTH_DOCK)
        return;
    ship = GetShownShip(port);
    graphicsId = sShips[ship == FERRY_SHIP_NONE ? FERRY_SHIP_AQUA : ship].graphicsId;
    VarSet(VAR_OBJ_GFX_ID_E, graphicsId);
    if (ship == FERRY_SHIP_NONE)
        FlagSet(FERRY_BERTH_HIDE_FLAG);
    else
        FlagClear(FERRY_BERTH_HIDE_FLAG);
    sBerthShownPlusOne = ship + 1;
    sBerthDirty = FALSE;
    if (!live)
        return;
    if (ship == FERRY_SHIP_NONE)
    {
        RemoveObjectEventByLocalIdAndMap(sPorts[port].shipLocalId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
        return;
    }
    if (!TryGetObjectEventIdByLocalIdAndMap(sPorts[port].shipLocalId, gSaveBlock1Ptr->location.mapNum,
                                            gSaveBlock1Ptr->location.mapGroup, &objectId))
        ObjectEventSetGraphicsId(&gObjectEvents[objectId], graphicsId);
    else
        TrySpawnObjectEvent(sPorts[port].shipLocalId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup);
}

// Leaving a ship map with the voyage still set means the player walked off by a warp (the Anne's dynamic
// gangway warps): that is going ashore. The port's heal location becomes the respawn, the voyage clears,
// and Melemele's first visit sets Alola's flags (the same block Route 13's boat runs).
static void FinishDisembarkByWarp(void)
{
    u8 port = GetBerthPort();

    if (port != FERRY_PORT_NONE)
    {
        if (port == FERRY_PORT_MELEMELE && !FlagGet(FLAG_VISITED_ALOLA))
            RunScriptImmediately(Alola_EventScript_FirstVisitFlags);
        SetLastHealLocationWarp(sPorts[port].healLocation);
    }
    Ferry_ClearVoyage();
}

// A berth reads the table when its map loads, after the temp flags are cleared and before objects spawn.
void Ferry_OnMapLoad(void)
{
    if (Ferry_IsAboard() && !Ferry_IsShipMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum))
        FinishDisembarkByWarp();
    sBerthShownPlusOne = 0;
    sBerthDirty = FALSE;
    ApplyBerth(FALSE);
}

// Every in-game minute (the overworld's time-of-day refresh): a changed answer is applied the next time
// the player has the field, so a dialogue with the sailor is never cut short.
void Ferry_OnMinute(void)
{
    u8 port = GetBerthPort();

    if (port != FERRY_PORT_NONE && sPorts[port].kind == FERRY_BERTH_DOCK && GetShownShip(port) + 1 != sBerthShownPlusOne)
        sBerthDirty = TRUE;
}

// ---------- The Anne's dynamic warp ----------

// In port the corridor's gangway leads to the arrival tile of the port at `stop`. At sea it points back at
// the corridor's own entry tile, so taking the gangway only returns the player there. With no voyage the
// Anne's home port is Vermilion.
void Ferry_RefreshAnneWarp(void)
{
    const struct WayfarerVoyage *voyage = Voyage();
    const struct FerryPort *port = &sPorts[sShips[FERRY_SHIP_ANNE].homePort];
    const struct FerryShip *anne = &sShips[FERRY_SHIP_ANNE];

    if (voyage->ship == FERRY_SHIP_ANNE)
    {
        if (voyage->phase == FERRY_PHASE_SEA)
        {
            SetDynamicWarpWithCoords(0, anne->entryGroup, anne->entryNum, WARP_ID_NONE, anne->entryX, anne->entryY);
            return;
        }
        port = &sPorts[Ferry_GetLoopPort(FERRY_SHIP_ANNE, voyage->stop)];
    }
    SetDynamicWarpWithCoords(0, port->mapGroup, port->mapNum, WARP_ID_NONE, port->x, port->y);
}

// ---------- Announcements ----------

static const u8 *GetPortName(u8 port)
{
    return port < FERRY_PORT_COUNT ? sPorts[port].name : sNameNone;
}

// The field poll: acts only when the player has the field (it runs from the field input, so never in a script,
// a menu or a battle), while aboard on a ship map. Returns TRUE after starting the announcement script.
bool8 Ferry_TryStartFieldEvent(void)
{
    const struct WayfarerVoyage *voyage = Voyage();
    u8 announce;
    u8 port;
    const u8 *script;

    if (sBerthDirty)
        ApplyBerth(TRUE);
    if (voyage->ship == FERRY_SHIP_NONE
     || !Ferry_IsShipMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum))
        return FALSE;
    if (Ferry_GetNow() < voyage->eventAt)
        return FALSE;
    announce = Ferry_Advance(Ferry_GetNow());
    if (announce == FERRY_ANNOUNCE_NONE)
        return FALSE;
    if (voyage->ship == FERRY_SHIP_ANNE)
        Ferry_RefreshAnneWarp();
    port = Ferry_GetLoopPort(voyage->ship, voyage->stop);
    StringCopy(gStringVar1, GetPortName(port));
    if (announce == FERRY_ANNOUNCE_CAST_OFF)
        script = voyage->ship == FERRY_SHIP_ANNE && port == FERRY_PORT_MELEMELE ? Ferry_EventScript_PA_CastOffAlola : Ferry_EventScript_PA_CastOff;
    else
        script = voyage->ship == FERRY_SHIP_ANNE && port == FERRY_PORT_MELEMELE ? Ferry_EventScript_PA_ArrivalAlola : Ferry_EventScript_PA_Arrival;
    ScriptContext_SetupScript(script);
    return TRUE;
}

// ---------- Boarding ----------

u16 Ferry_BufferNextShip(void)
{
    u8 port = GetBerthPort();
    u32 hour = GetTimeOfDayHours();
    u32 step;

    if (port == FERRY_PORT_NONE)
        return FALSE;
    for (step = 1; step <= HOURS_PER_DAY; step++)
    {
        u8 next = (hour + step) % HOURS_PER_DAY;
        u8 ship = Ferry_ShipAt(port, next);

        if (ship != FERRY_SHIP_NONE)
        {
            u8 *end = ConvertIntToDecimalStringN(gStringVar2, next, STR_CONV_MODE_LEFT_ALIGN, 2);

            *end++ = CHAR_COLON;
            *end++ = CHAR_0;
            *end++ = CHAR_0;
            *end = EOS;
            StringCopy(gStringVar1, sShips[ship].name);
            return TRUE;
        }
    }
    return FALSE;
}

u16 Ferry_BufferBoarding(void)
{
    u8 ship = Ferry_GetBerthShip();
    u8 port = GetBerthPort();
    u8 stop = Ferry_GetPortStop(ship, port);

    if (stop == 0xFF)
        return FALSE;
    StringCopy(gStringVar1, sShips[ship].name);
    StringCopy(gStringVar2, GetPortName(Ferry_GetLoopPort(ship, stop + 1)));
    return TRUE;
}

// Boards the ship in at this berth. Boarding the Tidal initializes Hoenn's state first (its maps are
// Hoenn-source); a refusal changes nothing.
u16 Ferry_Board(void)
{
    u8 ship = Ferry_GetBerthShip();
    u8 port = GetBerthPort();

    if (ship == FERRY_SHIP_NONE || Ferry_GetPortStop(ship, port) == 0xFF)
        return FALSE;
    if (ship == FERRY_SHIP_TIDAL && !WayfarerInitializeHoennState(sShips[ship].entryGroup, sShips[ship].entryNum,
                                                                 sShips[ship].entryX, sShips[ship].entryY))
        return FALSE;
    return Ferry_BoardAt(port, ship, Ferry_GetNow());
}

void Ferry_WarpAboard(void)
{
    const struct FerryShip *ship = &sShips[Voyage()->ship];

    SetWarpDestination(ship->entryGroup, ship->entryNum, WARP_ID_NONE, ship->entryX, ship->entryY);
    DoWarp();
    ResetInitialPlayerAvatarState();
}

// ---------- Going ashore ----------

u16 Ferry_IsAtSeaForScript(void)
{
    return Ferry_IsAtSea();
}

u16 Ferry_IsAboardForScript(void)
{
    return Ferry_IsAboard();
}

// The port the exit leads to: the ship's current stop in port, or the ship map's fixed home port with no voyage.
static u8 GetDisembarkPort(void)
{
    const struct WayfarerVoyage *voyage = Voyage();
    u8 ship;

    if (voyage->ship != FERRY_SHIP_NONE)
        return voyage->phase == FERRY_PHASE_PORT ? Ferry_GetLoopPort(voyage->ship, voyage->stop) : FERRY_PORT_NONE;
    ship = Ferry_GetShipOfMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
    return ship == FERRY_SHIP_NONE ? FERRY_PORT_NONE : sShips[ship].homePort;
}

u16 Ferry_CanDisembark(void)
{
    return GetDisembarkPort() != FERRY_PORT_NONE;
}

void Ferry_BufferArrivalPort(void)
{
    StringCopy(gStringVar1, GetPortName(GetDisembarkPort()));
}

static s8 GetArrivalY(u8 port)
{
    // Prof. Oak stands at Olivine's arrival tile until his scene is over.
    if (port == FERRY_PORT_OLIVINE && !FlagGet(FLAG_HIDE_OLIVINE_PORT_OAK))
        return sPorts[port].y - 1;
    return sPorts[port].y;
}

// Going ashore by an exit sailor (the Aqua and the Tidal): Hoenn arrival setup when the port is in Hoenn, the respawn, clear the voyage, warp. A
// refused Hoenn arrival changes nothing.
u16 Ferry_Disembark(void)
{
    u8 portId = GetDisembarkPort();
    const struct FerryPort *port;
    s8 y;

    if (portId == FERRY_PORT_NONE)
        return FALSE;
    port = &sPorts[portId];
    y = GetArrivalY(portId);
    if (port->hoenn && !WayfarerPrepareHoennEntry(port->mapGroup, port->mapNum, port->x, y, port->healLocation))
        return FALSE;
    SetLastHealLocationWarp(port->healLocation);
    Ferry_ClearVoyage();
    SetWarpDestination(port->mapGroup, port->mapNum, WARP_ID_NONE, port->x, y);
    DoWarp();
    ResetInitialPlayerAvatarState();
    return TRUE;
}

#endif // IS_WAYFARER
