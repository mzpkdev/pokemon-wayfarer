#include "global.h"
#include "map_layout.h"

#define MAP_LAYOUT_STORAGE_SCHEMA_V1 1
#define MAP_LAYOUT_CODEC_RAW 0

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
#endif

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

static enum MapLayoutLoadError GetRawTiles(const struct MapLayout *layout, u32 logicalBytes,
                                           const u16 **tiles)
{
    if (tiles == NULL)
        return MAP_LAYOUT_LOAD_BAD_ID;
    if (layout->mapData == NULL)
        return MAP_LAYOUT_LOAD_NO_DESCRIPTOR;

#if IS_WAYFARER
    {
        const struct MapLayoutDataDescriptor *descriptor = layout->mapData;
        const u8 *payload;
        uintptr_t payloadAddress;
        uintptr_t payloadsStart = (uintptr_t)__map_layout_payloads_start;
        uintptr_t payloadsEnd = (uintptr_t)__map_layout_payloads_end;
        u32 crc;

        if (((uintptr_t)descriptor & 3) != 0)
            return MAP_LAYOUT_LOAD_NO_DESCRIPTOR;
        if (descriptor->schemaVersion != MAP_LAYOUT_STORAGE_SCHEMA_V1)
            return MAP_LAYOUT_LOAD_BAD_SCHEMA;
        if (descriptor->codec != MAP_LAYOUT_CODEC_RAW)
            return MAP_LAYOUT_LOAD_BAD_CODEC_FLAGS;
        if (descriptor->flags != 0)
            return MAP_LAYOUT_LOAD_BAD_CODEC_FLAGS;
        if (descriptor->storedBytes == 0 || descriptor->decodedFileBytes < descriptor->logicalTileBytes
         || descriptor->logicalTileBytes != logicalBytes
         || descriptor->storedBytes != descriptor->decodedFileBytes)
            return MAP_LAYOUT_LOAD_BAD_SIZE;
        payload = descriptor->payload;
        payloadAddress = (uintptr_t)payload;
        if (payload == NULL || (payloadAddress & 3) != 0
         || payloadAddress < payloadsStart || payloadAddress >= payloadsEnd
         || descriptor->storedBytes > payloadsEnd - payloadAddress)
            return MAP_LAYOUT_LOAD_BAD_ROM_RANGE;
        crc = CalcCrc32(payload, descriptor->storedBytes);
        if (crc != descriptor->storedCrc32)
            return MAP_LAYOUT_LOAD_BAD_STORED_CRC;
        if (crc != descriptor->decodedCrc32)
            return MAP_LAYOUT_LOAD_BAD_DECODED_CRC;
        *tiles = (const u16 *)payload;
    }
#else
    *tiles = layout->mapData;
#endif
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutCopyRect(const struct MapLayout *layout,
                                          u32 x, u32 y, u32 width, u32 height,
                                          u16 *dest, u32 destTileCapacity, u32 destStride)
{
    const u16 *tiles;
    enum MapLayoutLoadError error;
    u32 logicalBytes;
    u32 row;
    u32 required;

    error = ValidateLayoutGeometry(layout, &logicalBytes);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    if (dest == NULL || width == 0 || height == 0 || x > (u32)layout->width
     || y > (u32)layout->height || width > (u32)layout->width - x
     || height > (u32)layout->height - y || destStride < width)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    if (height - 1 > UINT32_MAX / destStride)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    required = (height - 1) * destStride + width;
    if (required > destTileCapacity)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    error = GetRawTiles(layout, logicalBytes, &tiles);
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
    u32 logicalBytes;

    if (view == NULL || view->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
    error = ValidateLayoutGeometry(layout, &logicalBytes);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    error = GetRawTiles(layout, logicalBytes, &view->tiles);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    view->width = layout->width;
    view->height = layout->height;
    view->allocation = NULL;
    view->active = TRUE;
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError MapLayoutReleaseView(struct MapLayoutView *view)
{
    if (view == NULL || !view->active)
        return MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME;
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
    const u16 *tiles;
    enum MapLayoutLoadError error;
    u32 logicalBytes;

    error = ValidateLayoutGeometry(layout, &logicalBytes);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    if (tile == NULL || x >= (u32)layout->width || y >= (u32)layout->height)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    error = GetRawTiles(layout, logicalBytes, &tiles);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    *tile = tiles[y * layout->width + x];
    return MAP_LAYOUT_LOAD_OK;
}
