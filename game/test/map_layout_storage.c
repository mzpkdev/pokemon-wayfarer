#include "global.h"
#include "fieldmap.h"
#include "malloc.h"
#include "map_layout.h"
#include "overworld.h"
#include "save.h"
#include "script.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/map_layout_storage.h"

#if IS_WAYFARER

TEST("Map layout storage APIs agree for the unconnected canary")
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR));
    const struct MapLayout *layout = header->mapLayout;
    struct MapLayoutView first = {0};
    struct MapLayoutView second = {0};
    u32 tileCount = layout->width * layout->height;
    u16 *copy = Alloc(tileCount * sizeof(*copy));
    u16 tile;

    EXPECT(copy != NULL);
    EXPECT_EQ(MapLayoutReadTile(layout, 9, 11, &tile), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutCopyFull(layout, copy, tileCount, layout->width), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(copy[11 * layout->width + 9], tile);
    EXPECT_EQ(MapLayoutAcquireView(layout, &first), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(first.tiles[11 * layout->width + 9], tile);
#if MAP_LAYOUT_STORAGE_HYBRID
    EXPECT(first.compressed);
    EXPECT_EQ(MapLayoutAcquireView(layout, &second), MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME);
#else
    EXPECT(!first.compressed);
    EXPECT_EQ(MapLayoutAcquireView(layout, &second), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutReleaseView(&second), MAP_LAYOUT_LOAD_OK);
#endif
    EXPECT_EQ(MapLayoutReleaseView(&first), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutReleaseView(&first), MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME);
    EXPECT_EQ(MapLayoutCopyRect(layout, layout->width, 0, 1, 1, copy, tileCount, layout->width),
              MAP_LAYOUT_LOAD_BAD_BOUNDS);
    Free(copy);
}

TEST("Map layout storage loads connected borders through one context")
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    const struct MapLayout *layout = header->mapLayout;
    const struct MapHeader *north = GetMapHeaderFromConnection(&header->connections->connections[0]);
    const struct MapHeader *south = GetMapHeaderFromConnection(&header->connections->connections[1]);
    struct MapLayoutLoadContext context = {0};
    struct MapHeader previousHeader = gMapHeader;
    u32 tileCount = layout->width * layout->height;
    u16 *copy = Alloc(tileCount * sizeof(*copy));
    u16 expected;

    EXPECT(copy != NULL);
    EXPECT_EQ(header->connections->connections[0].direction, CONNECTION_NORTH);
    EXPECT_EQ(header->connections->connections[1].direction, CONNECTION_SOUTH);
    EXPECT_EQ(MapLayoutBeginLoadContext(header, &context), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutCopyFullWithContext(&context, layout, copy, tileCount, layout->width), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutEndLoadContext(&context), MAP_LAYOUT_LOAD_OK);
    gMapHeader = *header;
    EXPECT_EQ(InitMap(), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutReadTile(north->mapLayout, 0, north->mapLayout->height - MAP_OFFSET, &expected),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(gBackupMapLayout.map[MAP_OFFSET], expected);
    EXPECT_EQ(MapLayoutReadTile(south->mapLayout, 0, 0, &expected), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(gBackupMapLayout.map[(layout->height + MAP_OFFSET) * gBackupMapLayout.width + MAP_OFFSET],
              expected);
    gMapHeader = previousHeader;
    Free(copy);
}

TEST("Map layout storage shares scratch with a compressed connected layout")
{
    const struct MapHeader *route = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    const struct MapHeader *harbor = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR));
    const struct MapConnection connection =
    {
        .direction = CONNECTION_NORTH,
        .mapGroup = MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR),
        .mapNum = MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
    };
    const struct MapConnections connections = {1, &connection};
    struct MapHeader connectedHeader = *route;
    struct MapLayoutLoadContext context = {0};
    u16 expected;
    u16 actual;

    connectedHeader.connections = &connections;
    EXPECT_EQ(MapLayoutReadTile(harbor->mapLayout, 0, harbor->mapLayout->height - 1, &expected),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutBeginLoadContext(&connectedHeader, &context), MAP_LAYOUT_LOAD_OK);
#if MAP_LAYOUT_STORAGE_HYBRID
    EXPECT(context.scratch != NULL);
#endif
    EXPECT_EQ(MapLayoutCopyRectWithContext(&context, harbor->mapLayout,
                                           0, harbor->mapLayout->height - 1, 1, 1,
                                           &actual, 1, 1),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(actual, expected);
    EXPECT_EQ(MapLayoutEndLoadContext(&context), MAP_LAYOUT_LOAD_OK);
}

TEST("Map layout storage rejects invalid connected map IDs before lookup")
{
    const struct MapHeader *route = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    const struct MapConnection connection =
    {
        .direction = CONNECTION_NORTH,
        .mapGroup = MAP_GROUP(MAP_ROUTE101),
        .mapNum = 0xFF,
    };
    const struct MapConnections connections = {1, &connection};
    struct MapHeader invalidHeader = *route;
    struct MapLayoutLoadContext context = {0};

    invalidHeader.connections = &connections;
    EXPECT_EQ(MapLayoutBeginLoadContext(&invalidHeader, &context), MAP_LAYOUT_LOAD_BAD_ID);
    EXPECT(!context.active);
    EXPECT(context.scratch == NULL);
}

TEST("Map layout storage failure enters the terminal no-save state")
{
    MainCallback callback1 = gMain.callback1;
    MainCallback callback2 = gMain.callback2;
    void (*fieldCallback)(void) = gFieldCallback;
    bool8 (*fieldCallback2)(void) = gFieldCallback2;
    bool8 softResetDisabled = gSoftResetDisabled;

    gMapLayoutLoadError = (struct MapLayoutLoadFailure){0};
    sBackupMapData[0] = 0;
    sBackupMapData[MAX_MAP_DATA_SIZE - 1] = 0;
    AbortMapLayoutLoad(MAP_LAYOUT_LOAD_BAD_DECODED_CRC, 0x12, 0x34, 0x5678);

    EXPECT(gMapLayoutLoadError.active);
    EXPECT_EQ(gMapLayoutLoadError.error, MAP_LAYOUT_LOAD_BAD_DECODED_CRC);
    EXPECT_EQ(gMapLayoutLoadError.mapGroup, 0x12);
    EXPECT_EQ(gMapLayoutLoadError.mapNum, 0x34);
    EXPECT_EQ(gMapLayoutLoadError.layoutId, 0x5678);
    EXPECT_EQ(gMain.callback2, CB2_MapLayoutLoadError);
    EXPECT(ArePlayerFieldControlsLocked());
    EXPECT(!gSoftResetDisabled);
    EXPECT_EQ(sBackupMapData[0], MAPGRID_UNDEFINED);
    EXPECT_EQ(sBackupMapData[MAX_MAP_DATA_SIZE - 1], MAPGRID_UNDEFINED);
    EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_ERROR);

    gMapLayoutLoadError = (struct MapLayoutLoadFailure){0};
    gMain.callback1 = callback1;
    gMain.callback2 = callback2;
    gFieldCallback = fieldCallback;
    gFieldCallback2 = fieldCallback2;
    gSoftResetDisabled = softResetDisabled;
    UnlockPlayerFieldControls();
}

#endif
