#ifndef GUARD_MAP_LAYOUT_H
#define GUARD_MAP_LAYOUT_H

#include "global.h"

enum MapLayoutLoadError
{
    MAP_LAYOUT_LOAD_OK = 0x00,
    MAP_LAYOUT_LOAD_BAD_ID = 0x01,
    MAP_LAYOUT_LOAD_NO_DESCRIPTOR = 0x02,
    MAP_LAYOUT_LOAD_BAD_SCHEMA = 0x03,
    MAP_LAYOUT_LOAD_BAD_CODEC_FLAGS = 0x04,
    MAP_LAYOUT_LOAD_BAD_ROM_RANGE = 0x05,
    MAP_LAYOUT_LOAD_BAD_SIZE = 0x06,
    MAP_LAYOUT_LOAD_BAD_BOUNDS = 0x07,
    MAP_LAYOUT_LOAD_BAD_STREAM = 0x08,
    MAP_LAYOUT_LOAD_BAD_STORED_CRC = 0x09,
    MAP_LAYOUT_LOAD_BAD_DECODED_CRC = 0x0A,
    MAP_LAYOUT_LOAD_SCRATCH_LIMIT = 0x0B,
    MAP_LAYOUT_LOAD_ALLOC_FAILED = 0x0C,
    MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME = 0x0D,
    MAP_LAYOUT_LOAD_INTERNAL = 0x0E,
};

struct MapLayoutView
{
    const u16 *tiles;
    u32 width;
    u32 height;
    void *allocation;
    bool8 active;
};

enum MapLayoutLoadError MapLayoutCopyFull(const struct MapLayout *layout, u16 *dest,
                                          u32 destTileCapacity, u32 destStride);
enum MapLayoutLoadError MapLayoutCopyRect(const struct MapLayout *layout,
                                          u32 x, u32 y, u32 width, u32 height,
                                          u16 *dest, u32 destTileCapacity, u32 destStride);
enum MapLayoutLoadError MapLayoutAcquireView(const struct MapLayout *layout,
                                             struct MapLayoutView *view);
enum MapLayoutLoadError MapLayoutReleaseView(struct MapLayoutView *view);
enum MapLayoutLoadError MapLayoutReadTile(const struct MapLayout *layout, u32 x, u32 y,
                                          u16 *tile);

#endif // GUARD_MAP_LAYOUT_H
