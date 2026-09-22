#ifndef GUARD_MAP_LAYOUT_H
#define GUARD_MAP_LAYOUT_H

#include "global.h"
#include "constants/map_layout_storage.h"

enum MapLayoutLoadError
{
    MAP_LAYOUT_LOAD_OK = 0x00,
    MAP_LAYOUT_LOAD_BAD_ID = 0x01,
    MAP_LAYOUT_LOAD_NO_DESCRIPTOR = 0x02,
    MAP_LAYOUT_LOAD_BAD_ROM_RANGE = 0x03,
    MAP_LAYOUT_LOAD_BAD_SIZE = 0x04,
    MAP_LAYOUT_LOAD_BAD_BOUNDS = 0x05,
    MAP_LAYOUT_LOAD_BAD_STREAM = 0x06,
    MAP_LAYOUT_LOAD_SCRATCH_LIMIT = 0x07,
    MAP_LAYOUT_LOAD_ALLOC_FAILED = 0x08,
    MAP_LAYOUT_LOAD_BAD_VIEW_LIFETIME = 0x09,
    MAP_LAYOUT_LOAD_INTERNAL = 0x0A,
};

struct MapLayoutView
{
    const u16 *tiles;
    u32 width;
    u32 height;
    void *allocation;
    bool8 active;
};

struct MapLayoutLoadContext
{
    void *scratch;
    u32 capacity;
    bool8 active;
};

struct MapLayoutLoadFailure
{
    enum MapLayoutLoadError error;
    s16 mapGroup;
    s16 mapNum;
    u16 layoutId;
    bool8 active;
};

extern struct MapLayoutLoadFailure gMapLayoutLoadError;

void AbortMapLayoutLoad(enum MapLayoutLoadError error, s16 mapGroup, s16 mapNum,
                        u16 layoutId);
void CB2_MapLayoutLoadError(void);

enum MapLayoutLoadError MapLayoutBeginLoadContext(const struct MapHeader *mapHeader,
                                                  struct MapLayoutLoadContext *context);
enum MapLayoutLoadError MapLayoutEndLoadContext(struct MapLayoutLoadContext *context);
enum MapLayoutLoadError MapLayoutCopyFullWithContext(struct MapLayoutLoadContext *context,
                                          const struct MapLayout *layout, u16 *dest,
                                          u32 destTileCapacity, u32 destStride);
enum MapLayoutLoadError MapLayoutCopyRectWithContext(struct MapLayoutLoadContext *context,
                                          const struct MapLayout *layout,
                                          u32 x, u32 y, u32 width, u32 height,
                                          u16 *dest, u32 destTileCapacity, u32 destStride);

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

#if TESTING && IS_WAYFARER
struct MapLayoutTestDescriptor
{
    const u8 *payload;
    u32 storedBytes;
    u32 decodedFileBytes;
    u8 codec;
    u8 padding;
    u16 reserved;
};

struct MapLayoutTestTelemetry
{
    u32 requestedScratchBytes;
    u32 allocationCount;
    u32 releaseCount;
    u32 openCount;
    u32 decodeCount;
    u32 beforeAllocationLargestFreeBytes;
    u32 afterAllocationLargestFreeBytes;
    u32 afterDecodeLargestFreeBytes;
    u32 afterReleaseLargestFreeBytes;
    u32 minimumLargestFreeBytes;
};

void Test_MapLayoutResetHooks(void);
void Test_MapLayoutSetPayloadBounds(const void *start, const void *end);
void Test_MapLayoutForceAllocationFailure(bool8 enabled);
void Test_MapLayoutSetAllocationLimit(u32 maximumBytes);
void Test_MapLayoutInjectOpenError(u32 call, enum MapLayoutLoadError error);
void Test_MapLayoutUseRawOracle(bool8 enabled);
const struct MapLayoutTestTelemetry *Test_MapLayoutGetTelemetry(void);
const struct MapLayoutTestDescriptor *Test_MapLayoutGetDescriptor(const struct MapLayout *layout);
#endif

#endif // GUARD_MAP_LAYOUT_H
