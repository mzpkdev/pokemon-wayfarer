#include "global.h"
#include "fieldmap.h"
#include "malloc.h"
#include "map_layout.h"
#include "main.h"
#include "overworld.h"
#include "save.h"
#include "script.h"
#include "wayfarer_origin.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/map_layout_storage.h"
#include "constants/layouts.h"
#include "constants/heal_locations.h"
#include "constants/regions.h"
#include "constants/wayfarer_origin.h"
#include "../src/data/map_group_count.h"

#if IS_WAYFARER

extern const struct MapLayout *const gMapLayouts[];
extern const u16 *const gMapLayoutRawOracles[];
extern const struct MapHeader *const *const gMapGroups[];
extern const u8 __map_layout_payloads_start[];
extern const u8 gMapLayoutPeakTestPayload[];
extern const struct MapLayoutTestDescriptor gMapLayoutPeakTestDescriptor;
extern const struct MapLayout *const gMapLayoutPeakTestLayout;

struct HeapSnapshot
{
    u32 blockCount;
    u32 allocatedBytes;
    u32 freeBytes;
    u32 largestFreeBytes;
};

static struct HeapSnapshot TakeHeapSnapshot(void)
{
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    struct HeapSnapshot snapshot = {0};

    do
    {
        snapshot.blockCount++;
        if (block->allocated)
            snapshot.allocatedBytes += block->size;
        else
        {
            snapshot.freeBytes += block->size;
            if (block->size > snapshot.largestFreeBytes)
                snapshot.largestFreeBytes = block->size;
        }
        block = block->next;
    } while (block != head);
    return snapshot;
}

static u8 BuildOracleBackup(const struct MapHeader *header, u16 *dest)
{
    s32 width;
    s32 height;
    u8 flags;

    EXPECT_EQ(Test_InitLegacyRawMapLayoutData(header, dest, MAX_MAP_DATA_SIZE,
                                               &width, &height, &flags),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(width, header->mapLayout->width + MAP_OFFSET_W);
    EXPECT_EQ(height, header->mapLayout->height + MAP_OFFSET_H);
    return flags;
}

static void ExpectSyntheticConnectionsMatch(const struct MapConnection *connection, u32 count)
{
    const struct MapHeader *route = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    struct MapConnections connections = {count, connection};
    struct MapHeader header = *route;
    u16 *expected = Alloc(sizeof(sBackupMapData));
    u8 expectedFlags;

    EXPECT(expected != NULL);
    header.connections = &connections;
    expectedFlags = BuildOracleBackup(&header, expected);
    Test_ResetMapConnectionFlags();
    EXPECT_EQ(Test_InitMapLayoutData(&header), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(memcmp(expected, sBackupMapData, sizeof(sBackupMapData)), 0);
    EXPECT_EQ(Test_GetMapConnectionFlags(), expectedFlags);
    Free(expected);
}

static u32 TestCrc32(const u8 *data, u32 size)
{
    u32 crc = 0xFFFFFFFF;
    u32 i;

    while (size-- != 0)
    {
        crc ^= *data++;
        for (i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320 & (0 - (crc & 1)));
    }
    return crc ^ 0xFFFFFFFF;
}

#if MAP_LAYOUT_STORAGE_HYBRID
static void ExpectRejectedLzStream(const u8 *stream, u32 size, u32 decodedBytes)
{
    struct MapLayoutTestDescriptor descriptor =
    {
        .payload = stream,
        .storedBytes = size,
        .decodedFileBytes = decodedBytes,
        .logicalTileBytes = 2,
        .storedCrc32 = TestCrc32(stream, size),
        .decodedCrc32 = 0,
        .schemaVersion = 1,
        .codec = 1,
    };
    struct MapLayout layout =
    {
        .width = 1,
        .height = 1,
        .mapData = &descriptor,
    };
    u16 tile;

    Test_MapLayoutResetHooks();
    Test_MapLayoutSetPayloadBounds(stream, stream + size);
    EXPECT_EQ(MapLayoutCopyFull(&layout, &tile, 1, 1), MAP_LAYOUT_LOAD_BAD_STREAM);
    EXPECT_EQ(Test_MapLayoutGetTelemetry()->decodeCount, 0);
}
#endif

TEST("Map layout storage APIs agree for the unconnected canary")
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_FORTREE_CITY_HOUSE1), MAP_NUM(MAP_FORTREE_CITY_HOUSE1));
    const struct MapLayout *layout = header->mapLayout;
    struct MapLayoutView first = {0};
    struct MapLayoutView second = {0};
    u32 tileCount = layout->width * layout->height;
    u16 *copy = Alloc(tileCount * sizeof(*copy));
    u16 tile;

    EXPECT(copy != NULL);
    EXPECT_EQ(MapLayoutReadTile(layout, 2, 2, &tile), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutCopyFull(layout, copy, tileCount, layout->width), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(copy[2 * layout->width + 2], tile);
    EXPECT_EQ(MapLayoutAcquireView(layout, &first), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(first.tiles[2 * layout->width + 2], tile);
#if MAP_LAYOUT_STORAGE_HYBRID
    EXPECT(first.compressed);
    EXPECT_EQ(MapLayoutAcquireView(layout, &second), MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME);
#else
    EXPECT(!first.compressed);
    EXPECT_EQ(MapLayoutAcquireView(layout, &second), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutReleaseView(&second), MAP_LAYOUT_LOAD_OK);
#endif
    EXPECT_EQ(MapLayoutReleaseView(&first), MAP_LAYOUT_LOAD_OK);
    EXPECT(!first.active);
    EXPECT(first.tiles == NULL);
    EXPECT(first.allocation == NULL);
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
    const struct MapHeader *canary = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_FORTREE_CITY_HOUSE1), MAP_NUM(MAP_FORTREE_CITY_HOUSE1));
    const struct MapConnection connection =
    {
        .direction = CONNECTION_NORTH,
        .mapGroup = MAP_GROUP(MAP_FORTREE_CITY_HOUSE1),
        .mapNum = MAP_NUM(MAP_FORTREE_CITY_HOUSE1),
    };
    const struct MapConnections connections = {1, &connection};
    struct MapHeader connectedHeader = *route;
    struct MapLayoutLoadContext context = {0};
    u16 expected;
    u16 actual;

    connectedHeader.connections = &connections;
    EXPECT_EQ(MapLayoutReadTile(canary->mapLayout, 0, canary->mapLayout->height - 1, &expected),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutBeginLoadContext(&connectedHeader, &context), MAP_LAYOUT_LOAD_OK);
#if MAP_LAYOUT_STORAGE_HYBRID
    EXPECT(context.scratch != NULL);
#endif
    EXPECT_EQ(MapLayoutCopyRectWithContext(&context, canary->mapLayout,
                                           0, canary->mapLayout->height - 1, 1, 1,
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

TEST("Map layout lookup rejects invalid IDs")
{
    EXPECT(GetMapLayout(0) == NULL);
    EXPECT(GetMapLayout(MAP_LAYOUT_COUNT + 1) == NULL);
    EXPECT(GetMapLayout(LAYOUT_ROUTE101) != NULL);
}

static void ExpectCatalogLayoutsMatch(u32 first, u32 end)
{
    u16 *expected = Alloc(sizeof(sBackupMapData));
    u32 exercised = 0;
    u32 excluded = 0;
    u32 i;

    EXPECT(expected != NULL);
    if (end > MAP_LAYOUT_COUNT)
        end = MAP_LAYOUT_COUNT;
    for (i = first; i < end; i++)
    {
        const struct MapLayout *layout = gMapLayouts[i];
        struct MapHeader header = {0};

        if (layout == NULL)
        {
            EXPECT(gMapLayoutRawOracles[i] == NULL);
            Test_MgbaPrintf("{\"kind\":\"layout\",\"id\":%d,\"status\":\"unavailable\"}", i + 1);
            excluded++;
            continue;
        }
        EXPECT(gMapLayoutRawOracles[i] != NULL);
        header.mapLayout = layout;
        header.mapLayoutId = i + 1;
        EXPECT_EQ(BuildOracleBackup(&header, expected), 0);
        Test_ResetMapConnectionFlags();
        EXPECT_EQ(Test_InitMapLayoutData(&header), MAP_LAYOUT_LOAD_OK);
        EXPECT_EQ(gBackupMapLayout.width, layout->width + MAP_OFFSET_W);
        EXPECT_EQ(gBackupMapLayout.height, layout->height + MAP_OFFSET_H);
        EXPECT(gBackupMapLayout.map == sBackupMapData);
        if (memcmp(expected, sBackupMapData, sizeof(sBackupMapData)) != 0)
            Test_MgbaPrintf("catalog layout %d differs", i + 1);
        EXPECT_EQ(memcmp(expected, sBackupMapData, sizeof(sBackupMapData)), 0);
        Test_MgbaPrintf("{\"kind\":\"layout\",\"id\":%d,\"status\":\"passed\"}", i + 1);
        exercised++;
    }
    Test_MgbaPrintf("catalog differential %d-%d: %d passed, %d unavailable slots",
                    first + 1, end, exercised, excluded);
    Free(expected);
}

TEST("Map layout storage matches catalog layouts 1 through 186") { ExpectCatalogLayoutsMatch(0, 186); }
TEST("Map layout storage matches catalog layouts 187 through 372") { ExpectCatalogLayoutsMatch(186, 372); }
TEST("Map layout storage matches catalog layouts 373 through 558") { ExpectCatalogLayoutsMatch(372, 558); }
TEST("Map layout storage matches catalog layouts 559 through 744") { ExpectCatalogLayoutsMatch(558, 744); }
TEST("Map layout storage matches catalog layouts 745 through 930") { ExpectCatalogLayoutsMatch(744, 930); }
TEST("Map layout storage matches catalog layouts 931 through 1116") { ExpectCatalogLayoutsMatch(930, 1116); }
TEST("Map layout storage matches catalog layouts 1117 through 1302") { ExpectCatalogLayoutsMatch(1116, 1302); }
TEST("Map layout storage matches catalog layouts 1303 through end") { ExpectCatalogLayoutsMatch(1302, MAP_LAYOUT_COUNT); }

static void ExpectRealMapHeadersMatch(u32 firstGroup, u32 endGroup)
{
    u16 *expected = Alloc(sizeof(sBackupMapData));
    u32 exercised = 0;
    u32 absentSlots = 0;
    u32 group;
    u32 map;

    EXPECT(expected != NULL);
    for (group = firstGroup; group < endGroup; group++)
    {
        for (map = 0; map < MAP_GROUP_COUNT[group]; map++)
        {
            const struct MapHeader *header = gMapGroups[group][map];
            u8 expectedFlags;

            if (header == NULL)
            {
                Test_MgbaPrintf("{\"kind\":\"map\",\"group\":%d,\"map\":%d,\"status\":\"absent\"}", group, map);
                absentSlots++;
                continue;
            }
            EXPECT(header->mapLayout != NULL);
            expectedFlags = BuildOracleBackup(header, expected);
            Test_ResetMapConnectionFlags();
            EXPECT_EQ(Test_InitMapLayoutData((struct MapHeader *)header), MAP_LAYOUT_LOAD_OK);
            EXPECT_EQ(gBackupMapLayout.width, header->mapLayout->width + MAP_OFFSET_W);
            EXPECT_EQ(gBackupMapLayout.height, header->mapLayout->height + MAP_OFFSET_H);
            EXPECT(gBackupMapLayout.map == sBackupMapData);
            if (memcmp(expected, sBackupMapData, sizeof(sBackupMapData)) != 0)
                Test_MgbaPrintf("map header %d:%d differs", group, map);
            EXPECT_EQ(memcmp(expected, sBackupMapData, sizeof(sBackupMapData)), 0);
            EXPECT_EQ(Test_GetMapConnectionFlags(), expectedFlags);
            Test_MgbaPrintf("{\"kind\":\"map\",\"group\":%d,\"map\":%d,\"status\":\"passed\"}", group, map);
            exercised++;
        }
    }
    Test_MgbaPrintf("map groups %d-%d: %d passed, %d absent slots",
                    firstGroup, endGroup - 1, exercised, absentSlots);
    EXPECT(exercised != 0);
    Free(expected);
}

TEST("Map layout storage matches real map headers in groups 0 through 14")
{
    ExpectRealMapHeadersMatch(0, 15);
}

TEST("Map layout storage matches real map headers in groups 15 through 29")
{
    ExpectRealMapHeadersMatch(15, 30);
}

TEST("Map layout storage matches real map headers in groups 30 through 44")
{
    ExpectRealMapHeadersMatch(30, 45);
}

TEST("Map layout storage matches real map headers in groups 45 through 59")
{
    ExpectRealMapHeadersMatch(45, 60);
}

TEST("Map layout storage matches real map headers in groups 60 through 74")
{
    ExpectRealMapHeadersMatch(60, 75);
}

TEST("Map layout storage matches real map headers in groups 75 through 89")
{
    ExpectRealMapHeadersMatch(75, 90);
}

TEST("Map layout storage matches real map headers in groups 90 through 104")
{
    ExpectRealMapHeadersMatch(90, 105);
}

TEST("Map layout storage matches real map headers in groups 105 through 114")
{
    ExpectRealMapHeadersMatch(105, MAP_GROUPS_COUNT);
}

TEST("Map layout storage matches synthetic connection offsets and ordering")
{
    const struct MapHeader *route = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    const struct MapHeader *harbor = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR));
    s32 destWidth = route->mapLayout->width + MAP_OFFSET_W;
    s32 destHeight = route->mapLayout->height + MAP_OFFSET_H;
    struct MapConnection cases[] =
    {
        {CONNECTION_NORTH, 0, MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)},
        {CONNECTION_SOUTH, -3, MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)},
        {CONNECTION_NORTH, destWidth - 1 - MAP_OFFSET, MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)},
        {CONNECTION_SOUTH, -MAP_OFFSET - harbor->mapLayout->width + 1, MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)},
        {CONNECTION_WEST, 0, MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)},
        {CONNECTION_EAST, 3, MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)},
        {CONNECTION_WEST, destHeight - 1 - MAP_OFFSET, MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)},
        {CONNECTION_EAST, -MAP_OFFSET - harbor->mapLayout->height + 1, MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR)},
    };
    struct MapConnection ordered[] = {cases[0], cases[5], cases[1], cases[4]};
    struct MapConnection noOverlap =
    {
        CONNECTION_NORTH,
        destWidth,
        MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR),
        MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
    };
    struct MapConnections noOverlapConnections = {1, &noOverlap};
    struct MapHeader noOverlapHeader = *route;
    const s32 extremeOffsets[] = {0x7FFFFFFF, (-0x7FFFFFFF - 1)};
    const u8 extremeDirections[] =
    {
        CONNECTION_NORTH, CONNECTION_SOUTH, CONNECTION_WEST, CONNECTION_EAST,
    };
    u32 i;
    u32 j;

    for (i = 0; i < ARRAY_COUNT(cases); i++)
        ExpectSyntheticConnectionsMatch(&cases[i], 1);
    ExpectSyntheticConnectionsMatch(ordered, ARRAY_COUNT(ordered));

    noOverlapHeader.connections = &noOverlapConnections;
    EXPECT_EQ(Test_InitMapLayoutData(&noOverlapHeader), MAP_LAYOUT_LOAD_BAD_BOUNDS);
    for (i = 0; i < MAX_MAP_DATA_SIZE; i++)
        EXPECT_EQ(sBackupMapData[i], MAPGRID_UNDEFINED);

    for (i = 0; i < ARRAY_COUNT(extremeDirections); i++)
    {
        for (j = 0; j < ARRAY_COUNT(extremeOffsets); j++)
        {
            struct MapConnection extreme = noOverlap;
            struct MapConnections extremeConnections = {1, &extreme};
            struct MapHeader extremeHeader = *route;

            extreme.direction = extremeDirections[i];
            extreme.offset = extremeOffsets[j];
            extremeHeader.connections = &extremeConnections;
            EXPECT_EQ(Test_InitMapLayoutData(&extremeHeader), MAP_LAYOUT_LOAD_BAD_BOUNDS);
            EXPECT(CheckHeap());
        }
    }
}

TEST("Map layout storage owns one scratch allocation for current map and neighbors")
{
    const struct MapHeader *route = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    const struct MapHeader *canary = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_FORTREE_CITY_HOUSE1), MAP_NUM(MAP_FORTREE_CITY_HOUSE1));
    const struct MapConnection connection =
    {
        .direction = CONNECTION_NORTH,
        .mapGroup = MAP_GROUP(MAP_FORTREE_CITY_HOUSE1),
        .mapNum = MAP_NUM(MAP_FORTREE_CITY_HOUSE1),
    };
    const struct MapConnections connections = {1, &connection};
    struct MapHeader connectedHeader = *route;
    struct MapLayoutLoadContext context = {0};
    struct HeapSnapshot before;
    struct HeapSnapshot after;
    const struct MapLayoutTestTelemetry *telemetry;
    void *subsequent;
    u16 tile;

    connectedHeader.connections = &connections;
    Test_MapLayoutResetHooks();
    before = TakeHeapSnapshot();
    EXPECT_EQ(MapLayoutBeginLoadContext(&connectedHeader, &context), MAP_LAYOUT_LOAD_OK);
    telemetry = Test_MapLayoutGetTelemetry();
#if MAP_LAYOUT_STORAGE_HYBRID
    EXPECT_EQ(telemetry->allocationCount, 1);
    EXPECT_EQ(telemetry->requestedScratchBytes,
              Test_MapLayoutGetDescriptor(canary->mapLayout)->decodedFileBytes);
#else
    EXPECT_EQ(telemetry->allocationCount, 0);
#endif
    EXPECT_EQ(MapLayoutCopyRectWithContext(&context, route->mapLayout, 0, 0, 1, 1,
                                           &tile, 1, 1), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutCopyRectWithContext(&context, canary->mapLayout, 0, 0, 1, 1,
                                           &tile, 1, 1), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutCopyRectWithContext(&context, canary->mapLayout, 1, 1, 1, 1,
                                           &tile, 1, 1), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(telemetry->allocationCount, MAP_LAYOUT_STORAGE_HYBRID ? 1 : 0);
    EXPECT_EQ(MapLayoutEndLoadContext(&context), MAP_LAYOUT_LOAD_OK);
#if MAP_LAYOUT_STORAGE_HYBRID
    EXPECT_EQ(telemetry->releaseCount, 1);
#endif
    after = TakeHeapSnapshot();
    EXPECT_EQ(after.blockCount, before.blockCount);
    EXPECT_EQ(after.allocatedBytes, before.allocatedBytes);
    EXPECT_EQ(after.freeBytes, before.freeBytes);
    EXPECT_EQ(after.largestFreeBytes, before.largestFreeBytes);
    subsequent = Alloc(1024);
    EXPECT(subsequent != NULL);
    EXPECT(CheckMemBlock(subsequent));
    Free(subsequent);
    EXPECT(CheckHeap());
    Test_MapLayoutResetHooks();
}

TEST("Map layout storage proves peak scratch phases as current map and neighbor")
{
#if MAP_LAYOUT_STORAGE_HYBRID
    const struct MapHeader *route = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    const struct MapLayoutTestDescriptor *descriptor = &gMapLayoutPeakTestDescriptor;
    struct MapLayout peakLayout = *gMapLayoutPeakTestLayout;
    struct MapHeader peakHeader = *route;
    struct MapConnection connection =
    {
        .direction = CONNECTION_NORTH,
        .mapGroup = MAP_GROUP(MAP_ROUTE101),
        .mapNum = MAP_NUM(MAP_ROUTE101),
    };
    struct MapConnections connections = {1, &connection};
    struct MapHeader neighborHeader = *route;
    struct MapLayoutLoadContext context = {0};
    struct HeapSnapshot before = TakeHeapSnapshot();
    struct HeapSnapshot after;
    const struct MapLayoutTestTelemetry *telemetry;
    u16 tile;
    void *followOn;

    EXPECT_EQ(descriptor->decodedFileBytes, 14640);
    peakLayout.mapData = descriptor;
    peakHeader.mapLayout = &peakLayout;
    peakHeader.connections = NULL;
    neighborHeader.connections = &connections;
    Test_MapLayoutResetHooks();
    EXPECT(descriptor->payload == gMapLayoutPeakTestPayload);
    Test_MapLayoutSetPayloadBounds(__map_layout_payloads_start,
                                   descriptor->payload + descriptor->storedBytes);

    Test_MapLayoutSetAllocationLimit(descriptor->decodedFileBytes - 1);
    EXPECT_EQ(MapLayoutBeginLoadContext(&peakHeader, &context), MAP_LAYOUT_LOAD_ALLOC_FAILED);
    EXPECT(!context.active);
    EXPECT(context.scratch == NULL);

    Test_MapLayoutResetHooks();
    Test_MapLayoutSetPayloadBounds(__map_layout_payloads_start,
                                   descriptor->payload + descriptor->storedBytes);
    Test_MapLayoutSetAllocationLimit(descriptor->decodedFileBytes);
    EXPECT_EQ(MapLayoutBeginLoadContext(&peakHeader, &context), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutCopyRectWithContext(&context, &peakLayout, 0, 0, 1, 1,
                                           &tile, 1, 1), MAP_LAYOUT_LOAD_OK);
    telemetry = Test_MapLayoutGetTelemetry();
    EXPECT_EQ(telemetry->requestedScratchBytes, 14640);
    EXPECT_EQ(telemetry->allocationCount, 1);
    EXPECT_EQ(telemetry->decodeCount, 1);
    EXPECT(telemetry->beforeAllocationLargestFreeBytes >= descriptor->decodedFileBytes);
    EXPECT(telemetry->afterAllocationLargestFreeBytes < telemetry->beforeAllocationLargestFreeBytes);
    EXPECT_EQ(telemetry->afterDecodeLargestFreeBytes, telemetry->afterAllocationLargestFreeBytes);
    EXPECT_EQ(telemetry->minimumLargestFreeBytes, telemetry->afterAllocationLargestFreeBytes);
    EXPECT_EQ(MapLayoutEndLoadContext(&context), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(telemetry->afterReleaseLargestFreeBytes, telemetry->beforeAllocationLargestFreeBytes);

    Test_MapLayoutResetHooks();
    Test_MapLayoutSetPayloadBounds(__map_layout_payloads_start,
                                   descriptor->payload + descriptor->storedBytes);
    Test_SetMapConnectionHeaderOverride(&peakHeader);
    EXPECT_EQ(MapLayoutBeginLoadContext(&neighborHeader, &context), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutCopyRectWithContext(&context, &peakLayout, 0, 0, 1, 1,
                                           &tile, 1, 1), MAP_LAYOUT_LOAD_OK);
    telemetry = Test_MapLayoutGetTelemetry();
    EXPECT_EQ(telemetry->requestedScratchBytes, 14640);
    EXPECT_EQ(telemetry->allocationCount, 1);
    EXPECT_EQ(MapLayoutEndLoadContext(&context), MAP_LAYOUT_LOAD_OK);
    Test_SetMapConnectionHeaderOverride(NULL);

    after = TakeHeapSnapshot();
    EXPECT_EQ(after.blockCount, before.blockCount);
    EXPECT_EQ(after.allocatedBytes, before.allocatedBytes);
    EXPECT_EQ(after.freeBytes, before.freeBytes);
    EXPECT_EQ(after.largestFreeBytes, before.largestFreeBytes);
    followOn = Alloc(descriptor->decodedFileBytes);
    EXPECT(followOn != NULL);
    EXPECT(CheckMemBlock(followOn));
    Free(followOn);
    EXPECT(CheckHeap());
    Test_MapLayoutResetHooks();
#endif
}

TEST("Map layout storage repeated raw and compressed reads do not fragment the heap")
{
    const struct MapHeader *route = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    const struct MapHeader *canary = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_FORTREE_CITY_HOUSE1), MAP_NUM(MAP_FORTREE_CITY_HOUSE1));
    struct HeapSnapshot before = TakeHeapSnapshot();
    struct HeapSnapshot after;
    u16 tile;
    u32 i;

    for (i = 0; i < 32; i++)
    {
        EXPECT_EQ(MapLayoutReadTile(route->mapLayout, i % route->mapLayout->width, 0, &tile),
                  MAP_LAYOUT_LOAD_OK);
        EXPECT_EQ(MapLayoutReadTile(canary->mapLayout, i % canary->mapLayout->width, 0, &tile),
                  MAP_LAYOUT_LOAD_OK);
        EXPECT(CheckHeap());
    }
    after = TakeHeapSnapshot();
    EXPECT_EQ(after.blockCount, before.blockCount);
    EXPECT_EQ(after.allocatedBytes, before.allocatedBytes);
    EXPECT_EQ(after.freeBytes, before.freeBytes);
    EXPECT_EQ(after.largestFreeBytes, before.largestFreeBytes);
}

TEST("Map layout storage profiles the connection-heaviest real map")
{
    const struct MapHeader *heaviest = NULL;
    u32 heaviestCount = 0;
    u32 group;
    u32 map;
    struct HeapSnapshot before;
    struct HeapSnapshot after;
    const struct MapLayoutTestTelemetry *telemetry;

    for (group = 0; group < MAP_GROUPS_COUNT; group++)
    {
        for (map = 0; map < MAP_GROUP_COUNT[group]; map++)
        {
            const struct MapHeader *header = gMapGroups[group][map];
            u32 count = header != NULL && header->connections != NULL
                      ? header->connections->count : 0;
            if (heaviest == NULL || count > heaviestCount)
            {
                heaviest = header;
                heaviestCount = count;
            }
        }
    }
    EXPECT(heaviest != NULL);
    EXPECT(heaviestCount != 0);
    Test_MapLayoutResetHooks();
    before = TakeHeapSnapshot();
    EXPECT_EQ(Test_InitMapLayoutData((struct MapHeader *)heaviest), MAP_LAYOUT_LOAD_OK);
    telemetry = Test_MapLayoutGetTelemetry();
#if MAP_LAYOUT_STORAGE_HYBRID
    EXPECT(telemetry->openCount > 1);
    EXPECT_EQ(telemetry->decodePhaseCount, telemetry->decodeCount);
    if (telemetry->requestedScratchBytes != 0)
    {
        EXPECT_EQ(telemetry->allocationCount, 1);
        EXPECT(telemetry->decodeCount != 0);
        EXPECT(telemetry->afterDecodeLargestFreeBytes != 0);
        EXPECT(telemetry->minimumLargestFreeBytes != 0);
        EXPECT_EQ(telemetry->releaseCount, 1);
        EXPECT(telemetry->afterReleaseLargestFreeBytes >= telemetry->minimumLargestFreeBytes);
    }
    else
    {
        EXPECT_EQ(telemetry->allocationCount, 0);
        EXPECT_EQ(telemetry->decodeCount, 0);
        EXPECT_EQ(telemetry->releaseCount, 0);
    }
#else
    EXPECT_EQ(telemetry->allocationCount, 0);
    EXPECT_EQ(telemetry->decodeCount, 0);
    EXPECT_EQ(telemetry->decodePhaseCount, 0);
#endif
    after = TakeHeapSnapshot();
    EXPECT_EQ(after.blockCount, before.blockCount);
    EXPECT_EQ(after.allocatedBytes, before.allocatedBytes);
    EXPECT_EQ(after.freeBytes, before.freeBytes);
    EXPECT_EQ(after.largestFreeBytes, before.largestFreeBytes);
    EXPECT(CheckHeap());
    Test_MgbaPrintf("{\"kind\":\"memory\",\"case\":\"connection-heaviest\",\"connections\":%d,\"scratch\":%d,\"decodes\":%d,\"decodePhases\":%d,\"status\":\"passed\"}",
                    heaviestCount, telemetry->requestedScratchBytes,
                    telemetry->decodeCount, telemetry->decodePhaseCount);
    Test_MapLayoutResetHooks();
}

TEST("Map layout storage reports allocation and mid-connection failures without leaks")
{
    const struct MapHeader *route = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_ROUTE101), MAP_NUM(MAP_ROUTE101));
    u32 i;

#if MAP_LAYOUT_STORAGE_HYBRID
    const struct MapHeader *canary = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_FORTREE_CITY_HOUSE1), MAP_NUM(MAP_FORTREE_CITY_HOUSE1));
    struct MapLayoutLoadContext context = {0};

    {
        struct MapLayoutView view = {0};

        Test_MapLayoutResetHooks();
        Test_MapLayoutForceAllocationFailure(TRUE);
        EXPECT_EQ(MapLayoutAcquireView(canary->mapLayout, &view), MAP_LAYOUT_LOAD_ALLOC_FAILED);
        EXPECT(!view.active);
        EXPECT(view.tiles == NULL);
        EXPECT(view.allocation == NULL);
    }
    Test_MapLayoutResetHooks();
    Test_MapLayoutForceAllocationFailure(TRUE);
    EXPECT_EQ(MapLayoutBeginLoadContext(canary, &context), MAP_LAYOUT_LOAD_ALLOC_FAILED);
    EXPECT(!context.active);
    EXPECT(context.scratch == NULL);
    EXPECT_EQ(Test_MapLayoutGetTelemetry()->allocationCount, 0);
#endif

    Test_MapLayoutResetHooks();
    Test_MapLayoutInjectOpenError(3, MAP_LAYOUT_LOAD_INTERNAL);
    EXPECT_EQ(Test_InitMapLayoutData((struct MapHeader *)route), MAP_LAYOUT_LOAD_INTERNAL);
    EXPECT_EQ(Test_MapLayoutGetTelemetry()->openCount, 3);
    for (i = 0; i < MAX_MAP_DATA_SIZE; i++)
        EXPECT_EQ(sBackupMapData[i], MAPGRID_UNDEFINED);
    EXPECT(CheckHeap());
    Test_MapLayoutResetHooks();
}

TEST("Map layout storage rejects malformed descriptors and streams deterministically")
{
    const struct MapHeader *canary = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_FORTREE_CITY_HOUSE1), MAP_NUM(MAP_FORTREE_CITY_HOUSE1));
    const struct MapLayoutTestDescriptor original =
        *Test_MapLayoutGetDescriptor(canary->mapLayout);
    struct MapLayoutTestDescriptor descriptor = original;
    struct MapLayout layout = *canary->mapLayout;
    u16 *dest = Alloc(original.logicalTileBytes);
    u8 *payload = Alloc(original.storedBytes);
    const u16 *raw = gMapLayoutRawOracles[canary->mapLayoutId - 1];

    EXPECT(dest != NULL);
    EXPECT(payload != NULL);
    Test_MapLayoutResetHooks();
    layout.mapData = NULL;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_NO_DESCRIPTOR);

    layout.mapData = (const u8 *)&descriptor + 1;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_NO_DESCRIPTOR);

    layout.mapData = &descriptor;
    descriptor.schemaVersion = 0;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_SCHEMA);
    descriptor = original;
    descriptor.codec = 0xFF;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_CODEC_FLAGS);
    descriptor = original;
    descriptor.flags = 1;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_CODEC_FLAGS);
    descriptor = original;
    descriptor.storedBytes = 0;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_SIZE);
    descriptor = original;
    descriptor.storedBytes = MAP_LAYOUT_MAX_STORED_BYTES + 1;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_SIZE);
    descriptor = original;
    descriptor.decodedFileBytes = 0;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_SIZE);
    descriptor = original;
    descriptor.decodedFileBytes = MAP_LAYOUT_MAX_DECODED_FILE_BYTES + 1;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_SIZE);
    descriptor = original;
    descriptor.logicalTileBytes--;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_SIZE);
    descriptor = original;
    descriptor.logicalTileBytes = descriptor.decodedFileBytes + 2;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_SIZE);
    descriptor = original;
    descriptor.payload = NULL;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_ROM_RANGE);
    descriptor = original;
    descriptor.payload++;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_ROM_RANGE);
    descriptor = original;
    Test_MapLayoutSetPayloadBounds(original.payload, original.payload + original.storedBytes);
    descriptor.payload = original.payload + original.storedBytes;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_ROM_RANGE);
    Test_MapLayoutResetHooks();
    descriptor = original;
    descriptor.storedCrc32 ^= 1;
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_STORED_CRC);
    EXPECT_EQ(Test_MapLayoutGetTelemetry()->decodeCount, 0);

#if MAP_LAYOUT_STORAGE_HYBRID
    memcpy(payload, original.payload, original.storedBytes);
    descriptor = original;
    descriptor.payload = payload;
    Test_MapLayoutResetHooks();
    Test_MapLayoutSetPayloadBounds(payload, payload + original.storedBytes);
    payload[0] = 0;
    descriptor.storedCrc32 = TestCrc32(payload, original.storedBytes);
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_STREAM);
    EXPECT_EQ(Test_MapLayoutGetTelemetry()->decodeCount, 0);

    memcpy(payload, original.payload, original.storedBytes);
    descriptor = original;
    descriptor.payload = payload;
    descriptor.decodedCrc32 ^= 1;
    Test_MapLayoutResetHooks();
    Test_MapLayoutSetPayloadBounds(payload, payload + original.storedBytes);
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_DECODED_CRC);
    EXPECT_EQ(Test_MapLayoutGetTelemetry()->decodeCount, 1);
#endif

    descriptor = original;
    descriptor.codec = 0;
    descriptor.payload = (const u8 *)raw;
    descriptor.storedBytes = original.decodedFileBytes;
    descriptor.storedCrc32 = TestCrc32((const u8 *)raw, descriptor.storedBytes);
    descriptor.decodedCrc32 = descriptor.storedCrc32 ^ 1;
    Test_MapLayoutResetHooks();
    Test_MapLayoutSetPayloadBounds(raw, (const u8 *)raw + descriptor.storedBytes);
    EXPECT_EQ(MapLayoutCopyFull(&layout, dest, original.logicalTileBytes / 2, layout.width),
              MAP_LAYOUT_LOAD_BAD_DECODED_CRC);
    EXPECT_EQ(Test_MapLayoutGetTelemetry()->decodeCount, 0);

    Test_MapLayoutResetHooks();
    Free(payload);
    Free(dest);
}

TEST("Map layout storage preflight rejects every malformed LZ stream class")
{
#if MAP_LAYOUT_STORAGE_HYBRID
    static const ALIGNED(4) u8 badHeader[] = {0x00, 0x02, 0x00, 0x00};
    static const ALIGNED(4) u8 badOutputSize[] = {0x10, 0x03, 0x00, 0x00};
    static const ALIGNED(4) u8 truncatedLiteral[] = {0x10, 0x02, 0x00, 0x00, 0x00};
    static const ALIGNED(4) u8 truncatedBackref[] = {0x10, 0x02, 0x00, 0x00, 0x80, 0x00};
    static const ALIGNED(4) u8 invalidBackref[] = {0x10, 0x02, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00};
    static const ALIGNED(4) u8 outputOverrun[] = {0x10, 0x02, 0x00, 0x00, 0x40, 0xAA, 0x00, 0x00};
    static const ALIGNED(4) u8 earlyEnd[] =
    {
        0x10, 0x09, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    static const ALIGNED(4) u8 trailingData[] =
    {
        0x10, 0x02, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0x00, 0, 0, 0, 0,
    };
    static const ALIGNED(4) u8 nonzeroPadding[] =
    {
        0x10, 0x02, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0x7F,
    };

    ExpectRejectedLzStream(badHeader, sizeof(badHeader), 2);
    ExpectRejectedLzStream(badOutputSize, sizeof(badOutputSize), 2);
    ExpectRejectedLzStream(truncatedLiteral, sizeof(truncatedLiteral), 2);
    ExpectRejectedLzStream(truncatedBackref, sizeof(truncatedBackref), 2);
    ExpectRejectedLzStream(invalidBackref, sizeof(invalidBackref), 2);
    ExpectRejectedLzStream(outputOverrun, sizeof(outputOverrun), 2);
    ExpectRejectedLzStream(earlyEnd, sizeof(earlyEnd), 9);
    ExpectRejectedLzStream(trailingData, sizeof(trailingData), 2);
    ExpectRejectedLzStream(nonzeroPadding, sizeof(nonzeroPadding), 2);
    Test_MapLayoutResetHooks();
#endif
}

TEST("Map layout storage rejects geometry and destination size overflow")
{
    const struct MapHeader *canary = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_FORTREE_CITY_HOUSE1), MAP_NUM(MAP_FORTREE_CITY_HOUSE1));
    struct MapLayout layout = *canary->mapLayout;
    u16 tile;

    layout.width = 0;
    EXPECT_EQ(MapLayoutCopyFull(&layout, &tile, 1, 1), MAP_LAYOUT_LOAD_BAD_SIZE);
    layout = *canary->mapLayout;
    layout.height = 0;
    EXPECT_EQ(MapLayoutCopyFull(&layout, &tile, 1, 1), MAP_LAYOUT_LOAD_BAD_SIZE);
    layout = *canary->mapLayout;
    EXPECT_EQ(MapLayoutCopyRect(&layout, 0, 0, 0, 1, &tile, 1, 1), MAP_LAYOUT_LOAD_BAD_BOUNDS);
    EXPECT_EQ(MapLayoutCopyRect(&layout, 0, 0, 1, 1, NULL, 1, 1), MAP_LAYOUT_LOAD_BAD_BOUNDS);
    EXPECT_EQ(MapLayoutCopyRect(&layout, layout.width, 0, 1, 1, &tile, 1, 1), MAP_LAYOUT_LOAD_BAD_BOUNDS);
    EXPECT_EQ(MapLayoutCopyRect(&layout, 0, 0, 2, 2, &tile, 3, 2), MAP_LAYOUT_LOAD_BAD_BOUNDS);
    EXPECT_EQ(MapLayoutCopyRect(&layout, 0, 0, 2, 2, &tile, UINT32_MAX, UINT32_MAX),
              MAP_LAYOUT_LOAD_BAD_BOUNDS);
#if MAP_LAYOUT_STORAGE_HYBRID
    {
        struct MapLayoutLoadContext context = {.active = TRUE};
        u32 tileCount = canary->mapLayout->width * canary->mapLayout->height;
        u16 *dest = Alloc(tileCount * sizeof(*dest));

        EXPECT(dest != NULL);
        EXPECT_EQ(MapLayoutCopyFullWithContext(&context, canary->mapLayout, dest,
                                               tileCount, canary->mapLayout->width),
                  MAP_LAYOUT_LOAD_SCRATCH_LIMIT);
        Free(dest);
    }
#endif
}

TEST("Map layout storage accepts minimum geometry and rejects oversized backup grids")
{
    static const ALIGNED(4) u16 payload[] = {0x4321};
    struct MapLayoutTestDescriptor descriptor =
    {
        .payload = (const u8 *)payload,
        .storedBytes = sizeof(payload),
        .decodedFileBytes = sizeof(payload),
        .logicalTileBytes = sizeof(payload),
        .storedCrc32 = 0,
        .decodedCrc32 = 0,
        .schemaVersion = 1,
        .codec = 0,
    };
    struct MapLayout layout = {.width = 1, .height = 1, .mapData = &descriptor};
    struct MapHeader header = {.mapLayout = &layout};

    descriptor.storedCrc32 = TestCrc32((const u8 *)payload, sizeof(payload));
    descriptor.decodedCrc32 = descriptor.storedCrc32;
    Test_MapLayoutResetHooks();
    Test_MapLayoutSetPayloadBounds(payload, payload + ARRAY_COUNT(payload));
    EXPECT_EQ(Test_InitMapLayoutData(&header), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(gBackupMapLayout.width, 1 + MAP_OFFSET_W);
    EXPECT_EQ(gBackupMapLayout.height, 1 + MAP_OFFSET_H);
    EXPECT_EQ(gBackupMapLayout.map[MAP_OFFSET * gBackupMapLayout.width + MAP_OFFSET], payload[0]);

    layout.width = 100;
    layout.height = 100;
    EXPECT_EQ(Test_InitMapLayoutData(&header), MAP_LAYOUT_LOAD_BAD_BOUNDS);
    Test_MapLayoutResetHooks();
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

static void RestoreMapLayoutFailureState(MainCallback callback1, MainCallback callback2,
                                         void (*fieldCallback)(void),
                                         bool8 (*fieldCallback2)(void), bool8 softResetDisabled)
{
    gMapLayoutLoadError = (struct MapLayoutLoadFailure){0};
    gMain.callback1 = callback1;
    gMain.callback2 = callback2;
    gFieldCallback = fieldCallback;
    gFieldCallback2 = fieldCallback2;
    gSoftResetDisabled = softResetDisabled;
    UnlockPlayerFieldControls();
}

TEST("Map layout storage propagates every loader error through Hoenn entry before initialization")
{
    MainCallback callback1 = gMain.callback1;
    MainCallback callback2 = gMain.callback2;
    void (*fieldCallback)(void) = gFieldCallback;
    bool8 (*fieldCallback2)(void) = gFieldCallback2;
    bool8 softResetDisabled = gSoftResetDisabled;
    bool8 initialized = WayfarerHoennStateIsInitialized();
    enum Region savedRegion = WayfarerGetSavedCurrentRegion();
    enum MapLayoutLoadError error;

    for (error = MAP_LAYOUT_LOAD_BAD_ID; error <= MAP_LAYOUT_LOAD_INTERNAL; error++)
    {
        WayfarerSetHoennStateInitialized(FALSE);
        WayfarerSetSavedCurrentRegion(REGION_JOHTO);
        Test_MapLayoutResetHooks();
        Test_MapLayoutInjectOpenError(1, error);
        EXPECT(!Test_WayfarerPrepareHoennEntryAt(
            MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
            9, 11, HEAL_LOCATION_SLATEPORT_CITY));
        EXPECT(gMapLayoutLoadError.active);
        EXPECT_EQ(gMapLayoutLoadError.error, error);
        EXPECT_EQ(gMain.callback2, CB2_MapLayoutLoadError);
        EXPECT(ArePlayerFieldControlsLocked());
        EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_ERROR);
        EXPECT(!WayfarerHoennStateIsInitialized());
        EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_JOHTO);
        RestoreMapLayoutFailureState(callback1, callback2, fieldCallback, fieldCallback2,
                                     softResetDisabled);
    }
    WayfarerSetHoennStateInitialized(initialized);
    WayfarerSetSavedCurrentRegion(savedRegion);
    Test_MapLayoutResetHooks();
}

TEST("Map layout storage propagates every loader error through origin validation before initialization")
{
    MainCallback callback1 = gMain.callback1;
    MainCallback callback2 = gMain.callback2;
    void (*fieldCallback)(void) = gFieldCallback;
    bool8 (*fieldCallback2)(void) = gFieldCallback2;
    bool8 softResetDisabled = gSoftResetDisabled;
    u16 startingOrigin = gSaveBlock3Ptr->wayfarerHoenn.startingOriginId;
    bool8 initialized = WayfarerHoennStateIsInitialized();
    enum MapLayoutLoadError error;

    for (error = MAP_LAYOUT_LOAD_BAD_ID; error <= MAP_LAYOUT_LOAD_INTERNAL; error++)
    {
        gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = ORIGIN_NONE;
        WayfarerSetHoennStateInitialized(FALSE);
        Test_MapLayoutResetHooks();
        Test_MapLayoutInjectOpenError(1, error);
        EXPECT(!WayfarerInitializeOrigin(ORIGIN_LITTLEROOT));
        EXPECT(gMapLayoutLoadError.active);
        EXPECT_EQ(gMapLayoutLoadError.error, error);
        EXPECT_EQ(gMain.callback2, CB2_MapLayoutLoadError);
        EXPECT(ArePlayerFieldControlsLocked());
        EXPECT_EQ(TrySavingData(SAVE_NORMAL), SAVE_STATUS_ERROR);
        EXPECT_EQ(WayfarerGetStartingOriginId(), ORIGIN_NONE);
        EXPECT(!WayfarerHoennStateIsInitialized());
        RestoreMapLayoutFailureState(callback1, callback2, fieldCallback, fieldCallback2,
                                     softResetDisabled);
    }
    gSaveBlock3Ptr->wayfarerHoenn.startingOriginId = startingOrigin;
    WayfarerSetHoennStateInitialized(initialized);
    Test_MapLayoutResetHooks();
}

#endif
