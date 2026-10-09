#include "global.h"
#include "event_data.h"
#include "field_control_avatar.h"
#include "fieldmap.h"
#include "item_use.h"
#include "malloc.h"
#include "overworld.h"
#include "task.h"
#include "test/test.h"
#include "wayfarer_persistence.h"
#include "constants/event_bg.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "../src/data/map_group_count.h"

#if IS_WAYFARER

// Wayfarer's C code sees the HnS flag table, so Emerald's hidden-item flags are
// written as their source ids: FLAG_HIDDEN_ITEMS_START through the last of them.
#define EMERALD_FLAG_HIDDEN_ITEMS_START 0x1F4
#define EMERALD_FLAG_HIDDEN_ITEMS_END   0x263
#define HOENN_HIDDEN_ITEM_COUNT         112

struct HoennHiddenItemSpot
{
    u16 mapId;
    u16 x, y;
    u16 item;
    u16 sourceFlag;
};

// Hidden items whose flags once decoded into unrelated low HnS story flags.
static const struct HoennHiddenItemSpot sHoennHiddenItemSpots[] =
{
    // FLAG_HIDDEN_ITEM_ROUTE_120_RARE_CANDY_1
    { MAP_ROUTE120, 9, 1, ITEM_RARE_CANDY, EMERALD_FLAG_HIDDEN_ITEMS_START + 0x47 },
    // FLAG_HIDDEN_ITEM_TRICK_HOUSE_NUGGET
    { MAP_ROUTE110_TRICK_HOUSE_END, 0, 0, ITEM_NUGGET, EMERALD_FLAG_HIDDEN_ITEMS_START + 0x01 },
    // FLAG_HIDDEN_ITEM_ABANDONED_SHIP_RM_1_KEY
    { MAP_ABANDONED_SHIP_HIDDEN_FLOOR_ROOMS, 0, 0, ITEM_KEY_TO_ROOM_1, EMERALD_FLAG_HIDDEN_ITEMS_START + 0x1F },
    // FLAG_HIDDEN_ITEM_NAVEL_ROCK_TOP_SACRED_ASH
    { MAP_NAVEL_ROCK_TOP, 0, 0, ITEM_SACRED_ASH, EMERALD_FLAG_HIDDEN_ITEMS_START + 0x6D },
};

static bool32 IsHoennSourceMap(u16 mapGroup, u16 mapNum)
{
    struct WarpData location = gSaveBlock1Ptr->location;
    bool32 isHoenn;

    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    isHoenn = WayfarerIsCurrentMapHoennSource();
    gSaveBlock1Ptr->location = location;
    return isHoenn;
}

static const struct BgEvent *FindHiddenItem(const struct HoennHiddenItemSpot *spot)
{
    const struct MapEvents *events = Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(spot->mapId), MAP_NUM(spot->mapId))->events;

    for (u32 i = 0; i < events->bgEventCount; i++)
    {
        const struct BgEvent *bgEvent = &events->bgEvents[i];
        if (bgEvent->kind == BG_EVENT_HIDDEN_ITEM && bgEvent->bgUnion.hiddenItem.item == spot->item
         && (spot->x == 0 || (bgEvent->x == spot->x && bgEvent->y == spot->y)))
            return bgEvent;
    }
    return NULL;
}

TEST("Wayfarer Hoenn hidden items decode to their own Hoenn flag and a single item")
{
    u8 *seen = AllocZeroed(EMERALD_FLAG_HIDDEN_ITEMS_END - EMERALD_FLAG_HIDDEN_ITEMS_START + 1);
    u32 count = 0;
    bool32 passed = TRUE;

    ASSUME(seen != NULL);
    for (u32 group = 0; group < MAP_GROUPS_COUNT; group++)
    {
        for (u32 num = 0; num < MAP_GROUP_COUNT[group]; num++)
        {
            const struct MapEvents *events;

            if (!IsHoennSourceMap(group, num))
                continue;
            events = Overworld_GetMapHeaderByGroupAndId(group, num)->events;
            for (u32 i = 0; i < events->bgEventCount; i++)
            {
                const struct BgEvent *bgEvent = &events->bgEvents[i];
                u16 flagId, sourceId;

                if (bgEvent->kind != BG_EVENT_HIDDEN_ITEM)
                    continue;
                flagId = GetHiddenItemFlagId(bgEvent);
                sourceId = HOENN_FLAG_SOURCE_ID(flagId);
                passed &= IS_HOENN_FLAG_ID(flagId);
                passed &= sourceId >= EMERALD_FLAG_HIDDEN_ITEMS_START && sourceId <= EMERALD_FLAG_HIDDEN_ITEMS_END;
                passed &= bgEvent->bgUnion.hiddenItem.quantity == 1;
                if (passed)
                    passed &= seen[sourceId - EMERALD_FLAG_HIDDEN_ITEMS_START]++ == 0;
                count++;
            }
        }
    }

    Free(seen);
    EXPECT(passed);
    EXPECT_EQ(count, HOENN_HIDDEN_ITEM_COUNT);
}

TEST("Wayfarer Hoenn hidden items keep their authored Emerald flags")
{
    for (u32 i = 0; i < ARRAY_COUNT(sHoennHiddenItemSpots); i++)
    {
        const struct BgEvent *bgEvent = FindHiddenItem(&sHoennHiddenItemSpots[i]);

        ASSUME(bgEvent != NULL);
        EXPECT_EQ(GetHiddenItemFlagId(bgEvent), HOENN_FLAG_ID(sHoennHiddenItemSpots[i].sourceFlag));
        EXPECT_EQ((u32)bgEvent->bgUnion.hiddenItem.quantity, 1);
    }
}

static bool32 ItemfinderFindsItemUnderfoot(const struct MapEvents *events, u8 taskId)
{
    gTasks[taskId].data[0] = 0;
    gTasks[taskId].data[1] = 0;
    gTasks[taskId].data[2] = FALSE;
    return ItemfinderCheckForHiddenItems(events, taskId)
        && gTasks[taskId].data[0] == 0
        && gTasks[taskId].data[1] == 0;
}

TEST("Wayfarer Itemfinder tracks each Hoenn hidden item by its own Hoenn flag")
{
    struct MapHeader mapHeader = gMapHeader;
    struct PlayerAvatar playerAvatar = gPlayerAvatar;
    struct ObjectEvent playerObject = gObjectEvents[0];
    u8 taskId = CreateTask(TaskDummy, 0);
    u32 checked = 0;
    bool32 passed = TRUE;

    gPlayerAvatar.objectEventId = 0;
    for (u32 group = 0; group < MAP_GROUPS_COUNT; group++)
    {
        for (u32 num = 0; num < MAP_GROUP_COUNT[group]; num++)
        {
            const struct MapEvents *events;

            if (!IsHoennSourceMap(group, num))
                continue;
            gMapHeader = *Overworld_GetMapHeaderByGroupAndId(group, num);
            events = gMapHeader.events;
            for (u32 i = 0; i < events->bgEventCount; i++)
            {
                const struct BgEvent *bgEvent = &events->bgEvents[i];
                u16 flagId;

                if (bgEvent->kind != BG_EVENT_HIDDEN_ITEM)
                    continue;
                flagId = GetHiddenItemFlagId(bgEvent);
                gObjectEvents[0].currentCoords.x = bgEvent->x + MAP_OFFSET;
                gObjectEvents[0].currentCoords.y = bgEvent->y + MAP_OFFSET;
                FlagClear(flagId);
                passed &= ItemfinderFindsItemUnderfoot(events, taskId);
                FlagSet(flagId);
                passed &= !ItemfinderFindsItemUnderfoot(events, taskId);
                FlagClear(flagId);
                checked++;
            }
        }
    }

    DestroyTask(taskId);
    gMapHeader = mapHeader;
    gPlayerAvatar = playerAvatar;
    gObjectEvents[0] = playerObject;
    EXPECT(passed);
    EXPECT_EQ(checked, HOENN_HIDDEN_ITEM_COUNT);
}

#endif // IS_WAYFARER
