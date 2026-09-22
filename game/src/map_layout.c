#include "global.h"
#include "decompress.h"
#include "fieldmap.h"
#include "gpu_regs.h"
#include "malloc.h"
#include "main.h"
#include "map_layout.h"
#include "overworld.h"
#include "script.h"
#include "task.h"
#include "constants/map_groups.h"
#include "constants/map_layout_storage.h"
#include "constants/layouts.h"
#include "data/map_group_count.h"
#if IS_WAYFARER

#define MAP_LAYOUT_CODEC_RAW 0
#define MAP_LAYOUT_CODEC_GBA_LZ77 1

struct MapLayoutDataDescriptor
{
    const u8 *payload;
    u32 storedBytes;
    u32 decodedFileBytes;
    u8 codec;
    u8 padding;
    u16 reserved;
};

STATIC_ASSERT(sizeof(struct MapLayoutDataDescriptor) == 16, MapLayoutDataDescriptor_size);
STATIC_ASSERT(sizeof(struct MapLayout) == 28, MapLayout_size);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, payload) == 0x00, MapLayoutDataDescriptor_payload);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, storedBytes) == 0x04, MapLayoutDataDescriptor_storedBytes);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, decodedFileBytes) == 0x08, MapLayoutDataDescriptor_decodedFileBytes);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, codec) == 0x0C, MapLayoutDataDescriptor_codec);

extern const u8 __map_layout_payloads_start[];
extern const u8 __map_layout_payloads_end[];
static bool8 sCompressedViewActive;
#if TESTING
extern const struct MapLayout *const gMapLayouts[];
extern const u16 *const gMapLayoutRawOracles[];
static const u8 *sTestPayloadsStart;
static const u8 *sTestPayloadsEnd;
static bool8 sTestForceAllocationFailure;
static bool8 sTestUseRawOracle;
static u32 sTestAllocationLimit;
static u32 sTestInjectedOpenCall;
static enum MapLayoutLoadError sTestInjectedOpenError;
static struct MapLayoutTestTelemetry sTestTelemetry;

static u32 Test_GetLargestFreeBlock(void)
{
    const struct MemBlock *head = HeapHead();
    const struct MemBlock *block = head;
    u32 largest = 0;

    do
    {
        if (!block->allocated && block->size > largest)
            largest = block->size;
        block = block->next;
    } while (block != head);
    return largest;
}

static void Test_RecordLargestFree(u32 *phase)
{
    *phase = Test_GetLargestFreeBlock();
    if (sTestTelemetry.minimumLargestFreeBytes == 0
     || *phase < sTestTelemetry.minimumLargestFreeBytes)
        sTestTelemetry.minimumLargestFreeBytes = *phase;
}

static const u16 *Test_GetRawOracle(const struct MapLayout *layout)
{
    u32 i;

    if (!sTestUseRawOracle)
        return NULL;
    for (i = 0; i < MAP_LAYOUT_COUNT; i++)
        if (gMapLayouts[i] == layout)
            return gMapLayoutRawOracles[i];
    return NULL;
}
#endif
#endif // IS_WAYFARER

EWRAM_DATA struct MapLayoutLoadFailure gMapLayoutLoadError = {0};

void AbortMapLayoutLoad(enum MapLayoutLoadError error, s16 mapGroup, s16 mapNum,
                        u16 layoutId)
{
    if (error == MAP_LAYOUT_LOAD_OK || gMapLayoutLoadError.active)
        return;
    gMapLayoutLoadError.error = error;
    gMapLayoutLoadError.mapGroup = mapGroup;
    gMapLayoutLoadError.mapNum = mapNum;
    gMapLayoutLoadError.layoutId = layoutId;
    gMapLayoutLoadError.active = TRUE;
    CpuFastFill16(MAPGRID_UNDEFINED, sBackupMapData, sizeof(sBackupMapData));
    ScriptContext_Stop();
    LockPlayerFieldControls();
    gFieldCallback = NULL;
    gFieldCallback2 = NULL;
    gMain.callback1 = NULL;
    ResetTasks();
    gSoftResetDisabled = FALSE;
    gMain.state = 0;
    SetMainCallback2(CB2_MapLayoutLoadError);
}

void CB2_MapLayoutLoadError(void)
{
    if (gMain.state != 0)
        return;
    SetVBlankCallback(NULL);
    SetHBlankCallback(NULL);
    SetGpuReg(REG_OFFSET_DISPCNT, 0);
    CpuFastFill16(0, (void *)VRAM, DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(u16));
    REG_BG2PA = 1 << 8;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 1 << 8;
    REG_BG2X = 0;
    REG_BG2Y = 0;
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_3 | DISPCNT_BG2_ON);
    gMain.state = 1;
}

static enum MapLayoutLoadError ValidateLayoutGeometry(const struct MapLayout *layout, u32 *logicalBytes)
{
    if (layout == NULL || logicalBytes == NULL)
        return MAP_LAYOUT_LOAD_BAD_ID;
    if (layout->width <= 0 || layout->height <= 0
     || (u32)layout->width > UINT32_MAX / (u32)layout->height
     || (u32)layout->width * (u32)layout->height > UINT32_MAX / sizeof(u16))
        return MAP_LAYOUT_LOAD_BAD_SIZE;
    *logicalBytes = (u32)layout->width * (u32)layout->height * sizeof(u16);
    return MAP_LAYOUT_LOAD_OK;
}

#if IS_WAYFARER
static enum MapLayoutLoadError PreflightLz77(const struct MapLayoutDataDescriptor *descriptor)
{
    const u8 *stream = descriptor->payload;
    u32 position = 4;
    u32 output = 0;

    if (descriptor->storedBytes < 4 || (descriptor->storedBytes & 3) != 0
     || stream[0] != 0x10
     || ((u32)stream[1] | (u32)stream[2] << 8 | (u32)stream[3] << 16)
            != descriptor->decodedFileBytes)
        return MAP_LAYOUT_LOAD_BAD_STREAM;
    while (output < descriptor->decodedFileBytes)
    {
        u8 flags;
        u32 bit;

        if (position >= descriptor->storedBytes)
            return MAP_LAYOUT_LOAD_BAD_STREAM;
        flags = stream[position++];
        for (bit = 0; bit < 8 && output < descriptor->decodedFileBytes; bit++)
        {
            if (flags & (0x80 >> bit))
            {
                u32 token;
                u32 length;
                u32 distance;

                if (descriptor->storedBytes - position < 2)
                    return MAP_LAYOUT_LOAD_BAD_STREAM;
                token = (u32)stream[position] << 8 | stream[position + 1];
                position += 2;
                length = (token >> 12) + 3;
                distance = (token & 0xFFF) + 1;
                if (distance > output || length > descriptor->decodedFileBytes - output)
                    return MAP_LAYOUT_LOAD_BAD_STREAM;
                output += length;
            }
            else
            {
                if (position >= descriptor->storedBytes)
                    return MAP_LAYOUT_LOAD_BAD_STREAM;
                position++;
                output++;
            }
        }
    }
    if (((position + 3) & ~3) != descriptor->storedBytes)
        return MAP_LAYOUT_LOAD_BAD_STREAM;
    while (position < descriptor->storedBytes)
        if (stream[position++] != 0)
            return MAP_LAYOUT_LOAD_BAD_STREAM;
    return MAP_LAYOUT_LOAD_OK;
}

static enum MapLayoutLoadError ValidateDescriptor(const struct MapLayout *layout, u32 logicalBytes,
                                                  const struct MapLayoutDataDescriptor **descriptorOut)
{
    const struct MapLayoutDataDescriptor *descriptor;
    uintptr_t payloadAddress;
    uintptr_t payloadsStart = (uintptr_t)__map_layout_payloads_start;
    uintptr_t payloadsEnd = (uintptr_t)__map_layout_payloads_end;

#if TESTING
    if (sTestPayloadsStart != NULL)
    {
        payloadsStart = (uintptr_t)sTestPayloadsStart;
        payloadsEnd = (uintptr_t)sTestPayloadsEnd;
    }
#endif
    if (layout->mapData == NULL)
        return MAP_LAYOUT_LOAD_NO_DESCRIPTOR;
    descriptor = layout->mapData;
    if (((uintptr_t)descriptor & 3) != 0)
        return MAP_LAYOUT_LOAD_NO_DESCRIPTOR;
    if (descriptor->codec > MAP_LAYOUT_CODEC_GBA_LZ77
     || descriptor->storedBytes == 0
     || descriptor->storedBytes > MAP_LAYOUT_MAX_STORED_BYTES
     || descriptor->decodedFileBytes == 0
     || descriptor->decodedFileBytes > MAP_LAYOUT_MAX_DECODED_FILE_BYTES
     || descriptor->decodedFileBytes < logicalBytes
     || (descriptor->codec == MAP_LAYOUT_CODEC_RAW
      && descriptor->storedBytes != descriptor->decodedFileBytes))
        return MAP_LAYOUT_LOAD_BAD_SIZE;
    payloadAddress = (uintptr_t)descriptor->payload;
    if (descriptor->payload == NULL || (payloadAddress & 3) != 0
     || payloadAddress < payloadsStart || payloadAddress >= payloadsEnd
     || descriptor->storedBytes > payloadsEnd - payloadAddress)
        return MAP_LAYOUT_LOAD_BAD_ROM_RANGE;
    if (descriptor->codec == MAP_LAYOUT_CODEC_GBA_LZ77)
    {
        enum MapLayoutLoadError error = PreflightLz77(descriptor);
        if (error != MAP_LAYOUT_LOAD_OK)
            return error;
    }
    *descriptorOut = descriptor;
    return MAP_LAYOUT_LOAD_OK;
}
#endif

static enum MapLayoutLoadError GetRequiredScratch(const struct MapLayout *layout, u32 *required)
{
    u32 logicalBytes;
    enum MapLayoutLoadError error = ValidateLayoutGeometry(layout, &logicalBytes);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
#if IS_WAYFARER
    {
        const struct MapLayoutDataDescriptor *descriptor;
#if TESTING
        if (Test_GetRawOracle(layout) != NULL)
        {
            *required = 0;
            return MAP_LAYOUT_LOAD_OK;
        }
#endif
        error = ValidateDescriptor(layout, logicalBytes, &descriptor);
        if (error != MAP_LAYOUT_LOAD_OK)
            return error;
        *required = descriptor->codec == MAP_LAYOUT_CODEC_GBA_LZ77 ? descriptor->decodedFileBytes : 0;
    }
#else
    if (layout->mapData == NULL)
        return MAP_LAYOUT_LOAD_NO_DESCRIPTOR;
    *required = 0;
#endif
    return MAP_LAYOUT_LOAD_OK;
}

static void *AllocateScratch(u32 size)
{
    void *scratch;

#if IS_WAYFARER && TESTING
    sTestTelemetry.requestedScratchBytes = size;
    Test_RecordLargestFree(&sTestTelemetry.beforeAllocationLargestFreeBytes);
    if (size == 0 || sTestForceAllocationFailure
     || (sTestAllocationLimit != 0 && size > sTestAllocationLimit))
        return NULL;
    scratch = Alloc(size);
    if (scratch != NULL)
        sTestTelemetry.allocationCount++;
    Test_RecordLargestFree(&sTestTelemetry.afterAllocationLargestFreeBytes);
#else
    scratch = size == 0 ? NULL : Alloc(size);
#endif
    return scratch;
}

static void ReleaseScratch(void *scratch)
{
    Free(scratch);
#if IS_WAYFARER && TESTING
    if (scratch != NULL)
        sTestTelemetry.releaseCount++;
    Test_RecordLargestFree(&sTestTelemetry.afterReleaseLargestFreeBytes);
#endif
}

static enum MapLayoutLoadError OpenTiles(const struct MapLayout *layout, void *scratch,
                                         u32 scratchCapacity, const u16 **tiles)
{
    u32 logicalBytes;
    enum MapLayoutLoadError error = ValidateLayoutGeometry(layout, &logicalBytes);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    if (tiles == NULL)
        return MAP_LAYOUT_LOAD_INTERNAL;
#if IS_WAYFARER
    {
        const struct MapLayoutDataDescriptor *descriptor;
#if TESTING
        sTestTelemetry.openCount++;
        if (sTestInjectedOpenCall != 0 && sTestTelemetry.openCount == sTestInjectedOpenCall)
            return sTestInjectedOpenError;
        if (Test_GetRawOracle(layout) != NULL)
        {
            *tiles = Test_GetRawOracle(layout);
            return MAP_LAYOUT_LOAD_OK;
        }
#endif
        error = ValidateDescriptor(layout, logicalBytes, &descriptor);
        if (error != MAP_LAYOUT_LOAD_OK)
            return error;
        if (descriptor->codec == MAP_LAYOUT_CODEC_RAW)
        {
            *tiles = (const u16 *)descriptor->payload;
            return MAP_LAYOUT_LOAD_OK;
        }
        if (scratch == NULL || descriptor->decodedFileBytes > scratchCapacity)
            return MAP_LAYOUT_LOAD_SCRATCH_LIMIT;
#if TESTING
        sTestTelemetry.decodeCount++;
#endif
        FastLZ77UnCompWram((const u32 *)descriptor->payload, scratch);
#if TESTING
        Test_RecordLargestFree(&sTestTelemetry.afterDecodeLargestFreeBytes);
#endif
        *tiles = scratch;
    }
#else
    *tiles = layout->mapData;
#endif
    return MAP_LAYOUT_LOAD_OK;
}

static enum MapLayoutLoadError BeginContextForLayout(const struct MapLayout *layout,
                                                     struct MapLayoutLoadContext *context)
{
    enum MapLayoutLoadError error;
    u32 required;
    if (context == NULL || context->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
    error = GetRequiredScratch(layout, &required);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    context->scratch = AllocateScratch(required);
    if (required != 0 && context->scratch == NULL)
        return MAP_LAYOUT_LOAD_ALLOC_FAILED;
    context->capacity = required;
    context->active = TRUE;
    return MAP_LAYOUT_LOAD_OK;
}

static enum MapLayoutLoadError ValidateRectArguments(const struct MapLayout *layout,
                                          u32 x, u32 y, u32 width, u32 height,
                                          const u16 *dest, u32 destTileCapacity, u32 destStride)
{
    u32 logicalBytes;
    u32 required;
    enum MapLayoutLoadError error = ValidateLayoutGeometry(layout, &logicalBytes);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    if (dest == NULL || width == 0 || height == 0 || x > (u32)layout->width
     || y > (u32)layout->height || width > (u32)layout->width - x
     || height > (u32)layout->height - y || destStride < width
     || height - 1 > (UINT32_MAX - width) / destStride)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    required = (height - 1) * destStride + width;
    return required > destTileCapacity ? MAP_LAYOUT_LOAD_BAD_BOUNDS : MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutBeginLoadContext(const struct MapHeader *mapHeader,
                                                  struct MapLayoutLoadContext *context)
{
    enum MapLayoutLoadError error;
    u32 required;
    u32 maximum;
    u32 i;

    if (context == NULL || context->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
    if (mapHeader == NULL || mapHeader->mapLayout == NULL)
        return MAP_LAYOUT_LOAD_BAD_ID;
    error = GetRequiredScratch(mapHeader->mapLayout, &maximum);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    if (mapHeader->connections != NULL)
    {
        if (mapHeader->connections->count < 0
         || (mapHeader->connections->count != 0 && mapHeader->connections->connections == NULL))
            return MAP_LAYOUT_LOAD_BAD_ID;
        for (i = 0; i < mapHeader->connections->count; i++)
        {
            const struct MapConnection *connection = &mapHeader->connections->connections[i];
            const struct MapHeader *connected;

            if (connection->mapGroup >= MAP_GROUPS_COUNT
             || connection->mapNum >= MAP_GROUP_COUNT[connection->mapGroup])
                return MAP_LAYOUT_LOAD_BAD_ID;
            connected = GetMapHeaderFromConnection(connection);
            if (connected == NULL || connected->mapLayout == NULL)
                return MAP_LAYOUT_LOAD_BAD_ID;
            error = GetRequiredScratch(connected->mapLayout, &required);
            if (error != MAP_LAYOUT_LOAD_OK)
                return error;
            if (required > maximum)
                maximum = required;
        }
    }
    if (maximum > MAP_LAYOUT_MAX_DECODED_FILE_BYTES)
        return MAP_LAYOUT_LOAD_SCRATCH_LIMIT;
    context->scratch = AllocateScratch(maximum);
    if (maximum != 0 && context->scratch == NULL)
        return MAP_LAYOUT_LOAD_ALLOC_FAILED;
    context->capacity = maximum;
    context->active = TRUE;
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutEndLoadContext(struct MapLayoutLoadContext *context)
{
    if (context == NULL || !context->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
    ReleaseScratch(context->scratch);
    context->scratch = NULL;
    context->capacity = 0;
    context->active = FALSE;
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutCopyRectWithContext(struct MapLayoutLoadContext *context,
                                          const struct MapLayout *layout,
                                          u32 x, u32 y, u32 width, u32 height,
                                          u16 *dest, u32 destTileCapacity, u32 destStride)
{
    const u16 *tiles;
    enum MapLayoutLoadError error;
    u32 row;

    if (context == NULL || !context->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
    error = ValidateRectArguments(layout, x, y, width, height, dest, destTileCapacity, destStride);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    error = OpenTiles(layout, context->scratch, context->capacity, &tiles);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    tiles += y * layout->width + x;
    for (row = 0; row < height; row++)
    {
        CpuCopy16(tiles, dest, width * sizeof(u16));
        tiles += layout->width;
        dest += destStride;
    }
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutCopyFullWithContext(struct MapLayoutLoadContext *context,
                                          const struct MapLayout *layout, u16 *dest,
                                          u32 destTileCapacity, u32 destStride)
{
    if (layout == NULL)
        return MAP_LAYOUT_LOAD_BAD_ID;
    return MapLayoutCopyRectWithContext(context, layout, 0, 0, layout->width, layout->height,
                                        dest, destTileCapacity, destStride);
}

enum MapLayoutLoadError MapLayoutCopyRect(const struct MapLayout *layout,
                                          u32 x, u32 y, u32 width, u32 height,
                                          u16 *dest, u32 destTileCapacity, u32 destStride)
{
    struct MapLayoutLoadContext context = {0};
    enum MapLayoutLoadError error = ValidateRectArguments(layout, x, y, width, height,
                                                          dest, destTileCapacity, destStride);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    error = BeginContextForLayout(layout, &context);
    if (error == MAP_LAYOUT_LOAD_OK)
    {
        error = MapLayoutCopyRectWithContext(&context, layout, x, y, width, height,
                                             dest, destTileCapacity, destStride);
        MapLayoutEndLoadContext(&context);
    }
    return error;
}

enum MapLayoutLoadError MapLayoutCopyFull(const struct MapLayout *layout, u16 *dest,
                                          u32 destTileCapacity, u32 destStride)
{
    if (layout == NULL)
        return MAP_LAYOUT_LOAD_BAD_ID;
    return MapLayoutCopyRect(layout, 0, 0, layout->width, layout->height,
                             dest, destTileCapacity, destStride);
}

enum MapLayoutLoadError MapLayoutAcquireView(const struct MapLayout *layout,
                                             struct MapLayoutView *view)
{
    enum MapLayoutLoadError error;
    u32 required;

    if (view == NULL || view->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
    error = GetRequiredScratch(layout, &required);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
#if IS_WAYFARER
    if (required != 0 && sCompressedViewActive)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
#endif
    view->allocation = AllocateScratch(required);
    if (required != 0 && view->allocation == NULL)
        return MAP_LAYOUT_LOAD_ALLOC_FAILED;
    error = OpenTiles(layout, view->allocation, required, &view->tiles);
    if (error != MAP_LAYOUT_LOAD_OK)
    {
        ReleaseScratch(view->allocation);
        view->allocation = NULL;
        return error;
    }
    view->width = layout->width;
    view->height = layout->height;
    view->active = TRUE;
#if IS_WAYFARER
    if (view->allocation != NULL)
        sCompressedViewActive = TRUE;
#endif
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutReleaseView(struct MapLayoutView *view)
{
    if (view == NULL || !view->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
#if IS_WAYFARER
    if (view->allocation != NULL)
        sCompressedViewActive = FALSE;
#endif
    ReleaseScratch(view->allocation);
    view->tiles = NULL;
    view->width = 0;
    view->height = 0;
    view->allocation = NULL;
    view->active = FALSE;
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutReadTile(const struct MapLayout *layout, u32 x, u32 y,
                                          u16 *tile)
{
    struct MapLayoutLoadContext context = {0};
    const u16 *tiles;
    enum MapLayoutLoadError error;
    u32 logicalBytes;

    error = ValidateLayoutGeometry(layout, &logicalBytes);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    if (tile == NULL || x >= (u32)layout->width || y >= (u32)layout->height)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    error = BeginContextForLayout(layout, &context);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    error = OpenTiles(layout, context.scratch, context.capacity, &tiles);
    if (error == MAP_LAYOUT_LOAD_OK)
        *tile = tiles[y * layout->width + x];
    MapLayoutEndLoadContext(&context);
    return error;
}

#if IS_WAYFARER && TESTING
void Test_MapLayoutResetHooks(void)
{
    sCompressedViewActive = FALSE;
    sTestPayloadsStart = NULL;
    sTestPayloadsEnd = NULL;
    sTestForceAllocationFailure = FALSE;
    sTestUseRawOracle = FALSE;
    sTestAllocationLimit = 0;
    sTestInjectedOpenCall = 0;
    sTestInjectedOpenError = MAP_LAYOUT_LOAD_OK;
    sTestTelemetry = (struct MapLayoutTestTelemetry){0};
}

void Test_MapLayoutSetPayloadBounds(const void *start, const void *end)
{
    sTestPayloadsStart = start;
    sTestPayloadsEnd = end;
}

void Test_MapLayoutForceAllocationFailure(bool8 enabled)
{
    sTestForceAllocationFailure = enabled;
}

void Test_MapLayoutSetAllocationLimit(u32 maximumBytes)
{
    sTestAllocationLimit = maximumBytes;
}

void Test_MapLayoutInjectOpenError(u32 call, enum MapLayoutLoadError error)
{
    sTestInjectedOpenCall = call;
    sTestInjectedOpenError = error;
}

void Test_MapLayoutUseRawOracle(bool8 enabled)
{
    sTestUseRawOracle = enabled;
}

const struct MapLayoutTestTelemetry *Test_MapLayoutGetTelemetry(void)
{
    return &sTestTelemetry;
}

const struct MapLayoutTestDescriptor *Test_MapLayoutGetDescriptor(const struct MapLayout *layout)
{
    return layout == NULL ? NULL : layout->mapData;
}
#endif
