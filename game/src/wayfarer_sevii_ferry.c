#include "global.h"

#if IS_WAYFARER

#include "event_data.h"
#include "item.h"
#include "malloc.h"
#include "script_menu.h"
#include "string_util.h"
#include "strings.h"
#include "wayfarer_origin.h"
#include "constants/items.h"
#include "constants/script_menu.h"
#include "constants/seagallop.h"

enum WayfarerVermilionPortChoice
{
    WAYFARER_VERMILION_PORT_SEVII = 0,
    WAYFARER_VERMILION_PORT_OTHER_DESTINATIONS = 1,
    WAYFARER_VERMILION_PORT_CANCEL = 2,
};

static const u8 sTextSeviiIslands[] = _("SEVII ISLANDS");
static const u8 sTextOtherDestinations[] = _("OTHER DESTINATIONS");
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
    return WayfarerCanUseRegularAqua() && CheckBagHasItem(ITEM_AURORA_TICKET, 1);
}

u16 WayfarerCanSailToNavelRock(void)
{
    return FlagGet(FLAG_SYS_GAME_CLEAR) && CheckBagHasItem(ITEM_MYSTIC_TICKET, 1);
}

void DrawWayfarerVermilionPortMenu(void)
{
    static const u8 *const sLabels[] =
    {
        sTextSeviiIslands,
        sTextOtherDestinations,
        gText_Cancel2,
    };
    static const s32 sDestinations[] =
    {
        WAYFARER_VERMILION_PORT_SEVII,
        WAYFARER_VERMILION_PORT_OTHER_DESTINATIONS,
        WAYFARER_VERMILION_PORT_CANCEL,
    };

    ScriptMenu_MultichoiceDynamic(15, 4, ARRAY_COUNT(sLabels),
                                  CreateMenuItems(sLabels, sDestinations, ARRAY_COUNT(sLabels)),
                                  FALSE, ARRAY_COUNT(sLabels), 0, DYN_MULTICHOICE_CB_NONE);
}

u16 GetWayfarerVermilionPortChoice(void)
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
