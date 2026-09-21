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
#include "data/map_group_count.h"
#include "constants/rgb.h"

#define MAP_LAYOUT_STORAGE_SCHEMA_V1 1
#define MAP_LAYOUT_CODEC_RAW 0
#define MAP_LAYOUT_CODEC_GBA_LZ77 1

struct MapLayoutDataDescriptor
{
    const u8 *payload;
    u32 storedBytes;
    u32 decodedFileBytes;
    u32 logicalTileBytes;
    u32 storedCrc32;
    u32 decodedCrc32;
    u8 schemaVersion;
    u8 codec;
    u16 flags;
};

STATIC_ASSERT(sizeof(struct MapLayoutDataDescriptor) == 28, MapLayoutDataDescriptor_size);
STATIC_ASSERT(sizeof(struct MapLayout) == 28, MapLayout_size);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, payload) == 0x00, MapLayoutDataDescriptor_payload);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, storedBytes) == 0x04, MapLayoutDataDescriptor_storedBytes);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, decodedFileBytes) == 0x08, MapLayoutDataDescriptor_decodedFileBytes);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, logicalTileBytes) == 0x0C, MapLayoutDataDescriptor_logicalTileBytes);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, storedCrc32) == 0x10, MapLayoutDataDescriptor_storedCrc32);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, decodedCrc32) == 0x14, MapLayoutDataDescriptor_decodedCrc32);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, schemaVersion) == 0x18, MapLayoutDataDescriptor_schemaVersion);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, codec) == 0x19, MapLayoutDataDescriptor_codec);
STATIC_ASSERT(offsetof(struct MapLayoutDataDescriptor, flags) == 0x1A, MapLayoutDataDescriptor_flags);

#if IS_WAYFARER
extern const u8 __map_layout_payloads_start[];
extern const u8 __map_layout_payloads_end[];
static bool8 sCompressedViewActive;
#endif

EWRAM_DATA struct MapLayoutLoadFailure gMapLayoutLoadError = {0};

static const u8 sMapLayoutErrorGlyphs['Z' - '0' + 1][5] =
{
    ['0' - '0'] = {0x3E, 0x51, 0x49, 0x45, 0x3E},
    ['1' - '0'] = {0x00, 0x42, 0x7F, 0x40, 0x00},
    ['2' - '0'] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3' - '0'] = {0x21, 0x41, 0x45, 0x4B, 0x31},
    ['4' - '0'] = {0x18, 0x14, 0x12, 0x7F, 0x10},
    ['5' - '0'] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6' - '0'] = {0x3C, 0x4A, 0x49, 0x49, 0x30},
    ['7' - '0'] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8' - '0'] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9' - '0'] = {0x06, 0x49, 0x49, 0x29, 0x1E},
    [':' - '0'] = {0x00, 0x36, 0x36, 0x00, 0x00},
    ['A' - '0'] = {0x7E, 0x11, 0x11, 0x11, 0x7E},
    ['B' - '0'] = {0x7F, 0x49, 0x49, 0x49, 0x36},
    ['C' - '0'] = {0x3E, 0x41, 0x41, 0x41, 0x22},
    ['D' - '0'] = {0x7F, 0x41, 0x41, 0x22, 0x1C},
    ['E' - '0'] = {0x7F, 0x49, 0x49, 0x49, 0x41},
    ['F' - '0'] = {0x7F, 0x09, 0x09, 0x09, 0x01},
    ['G' - '0'] = {0x3E, 0x41, 0x49, 0x49, 0x7A},
    ['L' - '0'] = {0x7F, 0x40, 0x40, 0x40, 0x40},
    ['M' - '0'] = {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    ['N' - '0'] = {0x7F, 0x04, 0x08, 0x10, 0x7F},
    ['O' - '0'] = {0x3E, 0x41, 0x41, 0x41, 0x3E},
    ['P' - '0'] = {0x7F, 0x09, 0x09, 0x09, 0x06},
    ['R' - '0'] = {0x7F, 0x09, 0x19, 0x29, 0x46},
    ['T' - '0'] = {0x01, 0x01, 0x7F, 0x01, 0x01},
    ['U' - '0'] = {0x3F, 0x40, 0x40, 0x40, 0x3F},
    ['X' - '0'] = {0x63, 0x14, 0x08, 0x14, 0x63},
    ['Y' - '0'] = {0x07, 0x08, 0x70, 0x08, 0x07},
};

static void DrawMapLayoutErrorChar(char character, u32 x, u32 y)
{
    volatile u16 *framebuffer = (volatile u16 *)VRAM;
    const u8 *glyph;
    u32 column;
    u32 row;

    if (character < '0' || character > 'Z')
        return;
    glyph = sMapLayoutErrorGlyphs[character - '0'];
    for (column = 0; column < 5; column++)
    {
        for (row = 0; row < 7; row++)
        {
            if (glyph[column] & (1 << row))
            {
                u32 pixelX = x + column * 2;
                u32 pixelY = y + row * 2;
                framebuffer[pixelY * DISPLAY_WIDTH + pixelX] = RGB_WHITE;
                framebuffer[pixelY * DISPLAY_WIDTH + pixelX + 1] = RGB_WHITE;
                framebuffer[(pixelY + 1) * DISPLAY_WIDTH + pixelX] = RGB_WHITE;
                framebuffer[(pixelY + 1) * DISPLAY_WIDTH + pixelX + 1] = RGB_WHITE;
            }
        }
    }
}

static void DrawMapLayoutErrorText(const char *text, u32 x, u32 y)
{
    while (*text != '\0')
    {
        DrawMapLayoutErrorChar(*text++, x, y);
        x += 12;
    }
}

static void DrawMapLayoutErrorHex(u32 value, u32 digits, u32 x, u32 y)
{
    static const char sHex[] = "0123456789ABCDEF";
    u32 digit;

    for (digit = 0; digit < digits; digit++)
    {
        u32 shift = (digits - digit - 1) * 4;
        DrawMapLayoutErrorChar(sHex[(value >> shift) & 0xF], x + digit * 12, y);
    }
}

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
    CpuFastFill16(RGB_BLACK, (void *)VRAM, DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(u16));
    REG_BG2PA = 1 << 8;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2PD = 1 << 8;
    REG_BG2X = 0;
    REG_BG2Y = 0;
    DrawMapLayoutErrorText("MAP DATA ERROR", 42, 24);
    DrawMapLayoutErrorText("ERROR: 0X", 30, 56);
    DrawMapLayoutErrorHex(gMapLayoutLoadError.error, 2, 138, 56);
    DrawMapLayoutErrorText("MAP: 0X", 30, 84);
    DrawMapLayoutErrorHex((u16)gMapLayoutLoadError.mapGroup, 4, 114, 84);
    DrawMapLayoutErrorChar(':', 162, 84);
    DrawMapLayoutErrorHex((u16)gMapLayoutLoadError.mapNum, 4, 174, 84);
    DrawMapLayoutErrorText("LAYOUT: 0X", 30, 112);
    DrawMapLayoutErrorHex(gMapLayoutLoadError.layoutId, 4, 150, 112);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_3 | DISPCNT_BG2_ON);
    gMain.state = 1;
}

static u32 CalcCrc32(const u8 *data, u32 size)
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

    if (descriptor->storedBytes < 4 || stream[0] != 0x10
     || ((u32)stream[1] | (u32)stream[2] << 8 | (u32)stream[3] << 16) != descriptor->decodedFileBytes)
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
                                                  bool8 verifyPayload,
                                                  const struct MapLayoutDataDescriptor **descriptorOut)
{
    const struct MapLayoutDataDescriptor *descriptor;
    uintptr_t payloadAddress;
    uintptr_t payloadsStart = (uintptr_t)__map_layout_payloads_start;
    uintptr_t payloadsEnd = (uintptr_t)__map_layout_payloads_end;

    if (layout->mapData == NULL)
        return MAP_LAYOUT_LOAD_NO_DESCRIPTOR;
    descriptor = layout->mapData;
    if (((uintptr_t)descriptor & 3) != 0)
        return MAP_LAYOUT_LOAD_NO_DESCRIPTOR;
    if (descriptor->schemaVersion != MAP_LAYOUT_STORAGE_SCHEMA_V1)
        return MAP_LAYOUT_LOAD_BAD_SCHEMA;
    if (descriptor->codec > MAP_LAYOUT_CODEC_GBA_LZ77 || descriptor->flags != 0)
        return MAP_LAYOUT_LOAD_BAD_CODEC_FLAGS;
    if (descriptor->storedBytes == 0 || descriptor->storedBytes > MAP_LAYOUT_MAX_STORED_BYTES
     || descriptor->decodedFileBytes == 0 || descriptor->decodedFileBytes > MAP_LAYOUT_MAX_DECODED_FILE_BYTES
     || descriptor->logicalTileBytes != logicalBytes || (descriptor->logicalTileBytes & 1) != 0
     || descriptor->logicalTileBytes > descriptor->decodedFileBytes
     || (descriptor->codec == MAP_LAYOUT_CODEC_RAW && descriptor->storedBytes != descriptor->decodedFileBytes))
        return MAP_LAYOUT_LOAD_BAD_SIZE;
    payloadAddress = (uintptr_t)descriptor->payload;
    if (descriptor->payload == NULL || (payloadAddress & 3) != 0
     || payloadAddress < payloadsStart || payloadAddress >= payloadsEnd
     || descriptor->storedBytes > payloadsEnd - payloadAddress)
        return MAP_LAYOUT_LOAD_BAD_ROM_RANGE;
    if (verifyPayload && CalcCrc32(descriptor->payload, descriptor->storedBytes) != descriptor->storedCrc32)
        return MAP_LAYOUT_LOAD_BAD_STORED_CRC;
    if (verifyPayload && descriptor->codec == MAP_LAYOUT_CODEC_GBA_LZ77)
    {
        enum MapLayoutLoadError error = PreflightLz77(descriptor);
        if (error != MAP_LAYOUT_LOAD_OK)
            return error;
    }
    else if (verifyPayload && descriptor->storedCrc32 != descriptor->decodedCrc32)
    {
        return MAP_LAYOUT_LOAD_BAD_DECODED_CRC;
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
        error = ValidateDescriptor(layout, logicalBytes, FALSE, &descriptor);
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

static enum MapLayoutLoadError OpenTiles(const struct MapLayout *layout, void *scratch,
                                         u32 scratchCapacity, const u16 **tiles,
                                         bool8 *compressed)
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
        error = ValidateDescriptor(layout, logicalBytes, TRUE, &descriptor);
        if (error != MAP_LAYOUT_LOAD_OK)
            return error;
        if (descriptor->codec == MAP_LAYOUT_CODEC_RAW)
        {
            *tiles = (const u16 *)descriptor->payload;
            if (compressed != NULL)
                *compressed = FALSE;
            return MAP_LAYOUT_LOAD_OK;
        }
        if (scratch == NULL || descriptor->decodedFileBytes > scratchCapacity)
            return MAP_LAYOUT_LOAD_SCRATCH_LIMIT;
        FastLZ77UnCompWram((const u32 *)descriptor->payload, scratch);
        if (CalcCrc32(scratch, descriptor->decodedFileBytes) != descriptor->decodedCrc32)
            return MAP_LAYOUT_LOAD_BAD_DECODED_CRC;
        *tiles = scratch;
        if (compressed != NULL)
            *compressed = TRUE;
    }
#else
    *tiles = layout->mapData;
    if (compressed != NULL)
        *compressed = FALSE;
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
    context->scratch = required == 0 ? NULL : Alloc(required);
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
     || height - 1 > UINT32_MAX / destStride)
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
    context->scratch = maximum == 0 ? NULL : Alloc(maximum);
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
    Free(context->scratch);
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
    error = OpenTiles(layout, context->scratch, context->capacity, &tiles, NULL);
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
    view->allocation = required == 0 ? NULL : Alloc(required);
    if (required != 0 && view->allocation == NULL)
        return MAP_LAYOUT_LOAD_ALLOC_FAILED;
    error = OpenTiles(layout, view->allocation, required, &view->tiles, &view->compressed);
    if (error != MAP_LAYOUT_LOAD_OK)
    {
        Free(view->allocation);
        view->allocation = NULL;
        return error;
    }
    view->width = layout->width;
    view->height = layout->height;
    view->active = TRUE;
#if IS_WAYFARER
    if (view->compressed)
        sCompressedViewActive = TRUE;
#endif
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutReleaseView(struct MapLayoutView *view)
{
    if (view == NULL || !view->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
#if IS_WAYFARER
    if (view->compressed)
        sCompressedViewActive = FALSE;
#endif
    Free(view->allocation);
    view->tiles = NULL;
    view->width = 0;
    view->height = 0;
    view->allocation = NULL;
    view->active = FALSE;
    view->compressed = FALSE;
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
    error = OpenTiles(layout, context.scratch, context.capacity, &tiles, NULL);
    if (error == MAP_LAYOUT_LOAD_OK)
        *tile = tiles[y * layout->width + x];
    MapLayoutEndLoadContext(&context);
    return error;
}
