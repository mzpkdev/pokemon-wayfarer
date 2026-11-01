#include "global.h"

#if IS_WAYFARER

#include "event_data.h"
#include "item.h"
#include "malloc.h"
#include "script_menu.h"
#include "string_util.h"
#include "strings.h"
#include "wayfarer_ferry.h"
#include "wayfarer_origin.h"
#include "wayfarer_ss_anne.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/script_menu.h"
#include "constants/seagallop.h"

enum WayfarerVermilionPortChoice
{
    WAYFARER_VERMILION_PORT_FERRY = 0,
    WAYFARER_VERMILION_PORT_SEVII = 1,
    WAYFARER_VERMILION_PORT_SS_ANNE = 2,
    WAYFARER_VERMILION_PORT_CANCEL = 3,
};

static const u8 sTextSeviiIslands[] = _("SEVII ISLANDS");
static const u8 sTextBoardSSAnne[] = _("BOARD S.S. ANNE");
static const u8 sTextOneIsland[] = _("ONE ISLAND");

static struct ListMenuItem *CreateMenuItems(const u8 *const *labels, const s32 *destinations, u8 count)
{
    struct ListMenuItem *items = AllocZeroed(sizeof(*items) * count);
    u8 i;

    for (i = 0; i < count; i++)
    {
        u8 *name = Alloc(100);
        StringCopy(name, labels[i]);
        items[i].name = name;
        items[i].id = destinations[i];
    }
    return items;
}

u16 WayfarerCanSailToBirthIsland(void)
{
    return CheckBagHasItem(ITEM_AURORA_TICKET, 1);
}

u16 WayfarerCanSailToNavelRock(void)
{
    return FlagGet(FLAG_SYS_GAME_CLEAR) && CheckBagHasItem(ITEM_MYSTIC_TICKET, 1);
}

// Vermilion's sailor runs the S.S. berth, the Seagallop and, until the captain's reward, the S.S. Anne
// adventure. The ship row is missing when the berth is empty.
void DrawWayfarerVermilionPortMenu(void)
{
    const u8 *labels[4];
    s32 destinations[4];
    u8 count = 0;
    u8 ship = Ferry_GetBerthShip();

    if (ship != FERRY_SHIP_NONE)
    {
        labels[count] = Ferry_GetShipName(ship);
        destinations[count++] = WAYFARER_VERMILION_PORT_FERRY;
    }
    labels[count] = sTextSeviiIslands;
    destinations[count++] = WAYFARER_VERMILION_PORT_SEVII;
    if (!FlagGet(FLAG_WAYFARER_SS_ANNE_CAPTAIN_REWARDED))
    {
        labels[count] = sTextBoardSSAnne;
        destinations[count++] = WAYFARER_VERMILION_PORT_SS_ANNE;
    }
    labels[count] = gText_Cancel2;
    destinations[count++] = WAYFARER_VERMILION_PORT_CANCEL;

    ScriptMenu_MultichoiceDynamic(15, 4, count, CreateMenuItems(labels, destinations, count),
                                  FALSE, count, 0, DYN_MULTICHOICE_CB_NONE);
}

u16 GetWayfarerVermilionPortChoice(void)
{
    return gSpecialVar_Result;
}

// Lilycove's attendant also owns the event-island charters. Each is offered while its condition holds:
// Southern Island with the Eon Ticket, Faraway Island with the Old Sea Map and the Battle Frontier once
// Scott has invited the player (no game-clear gate and no FLAG_ENABLE_SHIP_* flags).
static bool8 IsCharterAvailable(u8 charter)
{
    switch (charter)
    {
    case FERRY_CHARTER_SOUTHERN_ISLAND:
        return CheckBagHasItem(ITEM_EON_TICKET, 1);
    case FERRY_CHARTER_FARAWAY_ISLAND:
        return CheckBagHasItem(ITEM_OLD_SEA_MAP, 1);
    case FERRY_CHARTER_BATTLE_FRONTIER:
        return FlagGet(FLAG_MET_SCOTT_ON_SS_TIDAL);
    }
    return FALSE;
}

u16 GetFerryLilycoveCharterCount(void)
{
    u8 charter;
    u8 count = 0;

    for (charter = FERRY_CHARTER_SOUTHERN_ISLAND; charter <= FERRY_CHARTER_BATTLE_FRONTIER; charter++)
        if (IsCharterAvailable(charter))
            count++;
    return count;
}

void DrawFerryLilycoveCharterMenu(void)
{
    static const u8 sTextSouthernIsland[] = _("SOUTHERN ISLAND");
    static const u8 sTextFarawayIsland[] = _("FARAWAY ISLAND");
    static const u8 sTextBattleFrontier[] = _("BATTLE FRONTIER");
    const u8 *labels[5];
    s32 destinations[5];
    u8 count = 0;
    u8 ship = Ferry_GetBerthShip();

    if (ship != FERRY_SHIP_NONE)
    {
        labels[count] = Ferry_GetShipName(ship);
        destinations[count++] = FERRY_CHARTER_SHIP;
    }
    if (IsCharterAvailable(FERRY_CHARTER_SOUTHERN_ISLAND))
    {
        labels[count] = sTextSouthernIsland;
        destinations[count++] = FERRY_CHARTER_SOUTHERN_ISLAND;
    }
    if (IsCharterAvailable(FERRY_CHARTER_FARAWAY_ISLAND))
    {
        labels[count] = sTextFarawayIsland;
        destinations[count++] = FERRY_CHARTER_FARAWAY_ISLAND;
    }
    if (IsCharterAvailable(FERRY_CHARTER_BATTLE_FRONTIER))
    {
        labels[count] = sTextBattleFrontier;
        destinations[count++] = FERRY_CHARTER_BATTLE_FRONTIER;
    }
    labels[count] = gText_Cancel2;
    destinations[count++] = FERRY_CHARTER_CANCEL;

    ScriptMenu_MultichoiceDynamic(15, 4, count, CreateMenuItems(labels, destinations, count),
                                  FALSE, count, 0, DYN_MULTICHOICE_CB_NONE);
}

u16 GetFerryLilycoveCharter(void)
{
    return gSpecialVar_Result;
}

void DrawWayfarerVermilionSeviiDestinationMenu(void)
{
    const u8 *labels[4];
    s32 destinations[4];
    u8 count = 0;

    // Destination IDs are stored with each visible row.  Ticket-gated destinations
    // can therefore never reindex the ordinary One Island entry.
    labels[count] = sTextOneIsland;
    destinations[count++] = SEAGALLOP_ONE_ISLAND;
    if (WayfarerCanSailToBirthIsland())
    {
        labels[count] = gText_BirthIsland;
        destinations[count++] = SEAGALLOP_BIRTH_ISLAND;
    }
    if (WayfarerCanSailToNavelRock())
    {
        labels[count] = gText_NavelRock;
        destinations[count++] = SEAGALLOP_NAVEL_ROCK;
    }
    labels[count] = gText_Cancel2;
    destinations[count++] = MULTI_B_PRESSED;

    ScriptMenu_MultichoiceDynamic(15, 4, count, CreateMenuItems(labels, destinations, count),
                                  FALSE, count, 0, DYN_MULTICHOICE_CB_NONE);
}

u16 GetWayfarerVermilionSeviiDestination(void)
{
    return gSpecialVar_Result;
}

#endif // IS_WAYFARER
