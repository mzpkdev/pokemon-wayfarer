#include "global.h"
#include "battle_pyramid.h"
#include "bg.h"
#include "fieldmap.h"
#include "fldeff.h"
#include "fldeff_misc.h"
#include "frontier_util.h"
#include "menu.h"
#include "map_layout.h"
#include "mirage_tower.h"
#include "overworld.h"
#include "palette.h"
#include "pokenav.h"
#include "script.h"
#include "secret_base.h"
#include "trainer_hill.h"
#include "tv.h"
#include "constants/rgb.h"
#include "constants/layouts.h"
#include "constants/metatile_behaviors.h"
#include "constants/metatile_behaviors_frlg.h"
#include "wild_encounter.h"

struct ConnectionFlags
{
    u8 south:1;
    u8 north:1;
    u8 west:1;
    u8 east:1;
};

EWRAM_DATA u16 ALIGNED(4) sBackupMapData[MAX_MAP_DATA_SIZE] = {0};
EWRAM_DATA struct MapHeader gMapHeader = {0};
EWRAM_DATA struct Camera gCamera = {0};
EWRAM_DATA static struct ConnectionFlags sMapConnectionFlags = {0};

COMMON_DATA struct BackupMapLayout gBackupMapLayout = {0};

static const struct ConnectionFlags sDummyConnectionFlags = {0};
#if TESTING
static const struct MapHeader *sTestMapConnectionHeaderOverride;
#endif

static u32 NormalizeFrlgMetatileBehavior(u32 metatileBehavior)
{
    // FRLG stores several engine behaviors at different IDs. Keep values that
    // already share the Emerald/HNS meaning unchanged.
    switch (metatileBehavior)
    {
    case MB_FRLG_FAST_WATER:              return MB_FAST_WATER;
    case MB_FRLG_CYCLING_ROAD_WATER:      return MB_CYCLING_ROAD_WATER;
    case MB_FRLG_STRENGTH_BUTTON:         return MB_STRENGTH_BUTTON;
    case MB_FRLG_ICE:                     return MB_ICE;
    case MB_FRLG_THIN_ICE:                return MB_THIN_ICE;
    case MB_FRLG_CRACKED_ICE:             return MB_CRACKED_ICE;
    case MB_FRLG_HOT_SPRINGS:             return MB_HOT_SPRINGS;
    case MB_FRLG_ROCK_STAIRS:             return MB_ROCK_STAIRS;
    case MB_FRLG_FALL_WARP:               return MB_MT_PYRE_HOLE;
    case MB_FRLG_REGULAR_WARP:            return MB_NON_ANIMATED_DOOR;
    case MB_FRLG_UP_RIGHT_STAIR_WARP:     return MB_UP_RIGHT_STAIR_WARP;
    case MB_FRLG_UP_LEFT_STAIR_WARP:      return MB_UP_LEFT_STAIR_WARP;
    case MB_FRLG_DOWN_RIGHT_STAIR_WARP:   return MB_DOWN_RIGHT_STAIR_WARP;
    case MB_FRLG_DOWN_LEFT_STAIR_WARP:    return MB_DOWN_LEFT_STAIR_WARP;
    case MB_FRLG_UNION_ROOM_WARP:         return MB_BRIDGE_OVER_OCEAN;
    case MB_FRLG_SIGNPOST:                return MB_SIGNPOST;
    case MB_FRLG_POKEMON_CENTER_SIGN:     return MB_POKEMON_CENTER_SIGN;
    case MB_FRLG_POKEMART_SIGN:           return MB_POKEMART_SIGN;
    case MB_FRLG_CABINET:                 return MB_CABINET;
    case MB_FRLG_KITCHEN:                 return MB_KITCHEN;
    case MB_FRLG_DRESSER:                 return MB_DRESSER;
    case MB_FRLG_SNACKS:                  return MB_SNACKS;
    case MB_FRLG_CABLE_CLUB_WIRELESS_MONITOR: return MB_CABLE_CLUB_WIRELESS_MONITOR;
    case MB_FRLG_BATTLE_RECORDS:          return MB_BATTLE_RECORDS;
    case MB_FRLG_FOOD:                    return MB_FOOD;
    case MB_FRLG_INDIGO_PLATEAU_SIGN_1:   return MB_INDIGO_PLATEAU_SIGN_1;
    case MB_FRLG_INDIGO_PLATEAU_SIGN_2:   return MB_INDIGO_PLATEAU_SIGN_2;
    case MB_FRLG_BLUEPRINTS:              return MB_BLUEPRINTS;
    case MB_FRLG_PAINTING:                return MB_PAINTING;
    case MB_FRLG_POWER_PLANT_MACHINE:     return MB_POWER_PLANT_MACHINE;
    case MB_FRLG_TELEPHONE:               return MB_TELEPHONE;
    case MB_FRLG_COMPUTER:                return MB_COMPUTER;
    case MB_FRLG_ADVERTISING_POSTER:      return MB_ADVERTISING_POSTER;
    case MB_FRLG_FOOD_SMELLS_TASTY:       return MB_FOOD_SMELLS_TASTY;
    case MB_FRLG_TRASH_BIN:               return MB_TRASH_CAN;
    case MB_FRLG_CUP:                     return MB_CUP;
    case MB_FRLG_PORTHOLE:                return MB_PORTHOLE;
    case MB_FRLG_WINDOW:                  return MB_WINDOW;
    case MB_FRLG_BLINKING_LIGHTS:         return MB_BLINKING_LIGHTS;
    case MB_FRLG_NEATLY_LINED_UP_TOOLS:   return MB_NEATLY_LINED_UP_TOOLS;
    case MB_FRLG_IMPRESSIVE_MACHINE:      return MB_IMPRESSIVE_MACHINE;
    case MB_FRLG_VIDEO_GAME:              return MB_VIDEO_GAME;
    case MB_FRLG_BURGLARY:                return MB_BURGLARY;
    case MB_FRLG_TRAINER_TOWER_MONITOR:   return MB_TRAINER_TOWER_MONITOR;
    case MB_FRLG_CYCLING_ROAD_PULL_DOWN:  return MB_CYCLING_ROAD_PULL_DOWN;
    case MB_FRLG_CYCLING_ROAD_PULL_DOWN_GRASS: return MB_CYCLING_ROAD_PULL_DOWN_GRASS;
    default:                              return metatileBehavior;
    }
}

static enum MapLayoutLoadError InitMapLayoutData(struct MapHeader *mapHeader);
static enum MapLayoutLoadError InitBackupMapLayoutData(const struct MapLayout *mapLayout, struct MapLayoutLoadContext *context);
static enum MapLayoutLoadError FillSouthConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset, struct MapLayoutLoadContext *context);
static enum MapLayoutLoadError FillNorthConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset, struct MapLayoutLoadContext *context);
static enum MapLayoutLoadError FillWestConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset, struct MapLayoutLoadContext *context);
static enum MapLayoutLoadError FillEastConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset, struct MapLayoutLoadContext *context);
static enum MapLayoutLoadError InitBackupMapLayoutConnections(struct MapHeader *mapHeader, struct MapLayoutLoadContext *context);
static void LoadSavedMapView(void);
static bool8 SkipCopyingMetatileFromSavedMap(u16 *mapBlock, u16 mapWidth, u8 yMode);
static const struct MapConnection *GetIncomingConnection(enum Connection direction, int x, int y);
static bool8 IsPosInIncomingConnectingMap(enum Connection direction, int x, int y, const struct MapConnection *connection);
static bool8 IsCoordInIncomingConnectingMap(int coord, int srcMax, int destMax, int offset);

static inline u16 GetBorderBlockAt(int x, int y)
{
    const struct MapLayout *mapLayout = gMapHeader.mapLayout;

    if (mapLayout->layoutVersion == LAYOUT_VERSION_FRLG)
    {
        s32 xprime;
        s32 yprime;

        xprime = x - MAP_OFFSET;
        xprime += 8 * mapLayout->borderWidth;
        xprime %= mapLayout->borderWidth;

        yprime = y - MAP_OFFSET;
        yprime += 8 * mapLayout->borderHeight;
        yprime %= mapLayout->borderHeight;

        return mapLayout->border[xprime + yprime * mapLayout->borderWidth] | MAPGRID_COLLISION_MASK;
    }

    int i = (x + 1) & 1;
    i += ((y + 1) & 1) * 2;
    return gMapHeader.mapLayout->border[i] | MAPGRID_IMPASSABLE;
}

#define AreCoordsWithinMapGridBounds(x, y) (x >= 0 && x < gBackupMapLayout.width && y >= 0 && y < gBackupMapLayout.height)

#define GetMapGridBlockAt(x, y) (AreCoordsWithinMapGridBounds(x, y) ? gBackupMapLayout.map[x + gBackupMapLayout.width * y] : GetBorderBlockAt(x, y))

// Masks/shifts for metatile attributes
// This is the format of the data stored in each data/tilesets/*/*/metatile_attributes.bin file
static const u32 sMetatileAttrMasks[METATILE_ATTRIBUTE_COUNT] = {
    [METATILE_ATTRIBUTE_BEHAVIOR]       = METATILE_ATTR_BEHAVIOR_MASK_FRLG, // Bits 0-8
    [METATILE_ATTRIBUTE_TERRAIN]        = 0x00000000,
    [METATILE_ATTRIBUTE_2]              = 0x0003c000, // Bits 14-17
    [METATILE_ATTRIBUTE_3]              = 0x00fc0000, // Bits 18-23
    [METATILE_ATTRIBUTE_ENCOUNTER_TYPE] = 0x07000000, // Bits 24-26
    [METATILE_ATTRIBUTE_5]              = 0x18000000, // Bits 27-28
    [METATILE_ATTRIBUTE_LAYER_TYPE]     = METATILE_ATTR_LAYER_MASK_FRLG, // Bits 29-30
    [METATILE_ATTRIBUTE_7]              = 0x80000000  // Bit  31
};

static const u8 sMetatileAttrShifts[METATILE_ATTRIBUTE_COUNT] = {
    [METATILE_ATTRIBUTE_BEHAVIOR]       = METATILE_ATTR_BEHAVIOR_SHIFT_FRLG,
    [METATILE_ATTRIBUTE_TERRAIN]        = 9,
    [METATILE_ATTRIBUTE_2]              = 14,
    [METATILE_ATTRIBUTE_3]              = 18,
    [METATILE_ATTRIBUTE_ENCOUNTER_TYPE] = 24,
    [METATILE_ATTRIBUTE_5]              = 27,
    [METATILE_ATTRIBUTE_LAYER_TYPE]     = METATILE_ATTR_LAYER_SHIFT_FRLG,
    [METATILE_ATTRIBUTE_7]              = 31
};

static const u32 sMetatileAttrMasksEmerald[METATILE_ATTRIBUTE_COUNT] = {

    [METATILE_ATTRIBUTE_BEHAVIOR]       = METATILE_ATTR_BEHAVIOR_MASK,
    [METATILE_ATTRIBUTE_TERRAIN]        = 0xFFFFFFFF,
    [METATILE_ATTRIBUTE_2]              = 0xFFFFFFFF,
    [METATILE_ATTRIBUTE_3]              = 0xFFFFFFFF,
    [METATILE_ATTRIBUTE_ENCOUNTER_TYPE] = 0xFFFFFFFF,
    [METATILE_ATTRIBUTE_5]              = 0xFFFFFFFF,
    [METATILE_ATTRIBUTE_LAYER_TYPE]     = METATILE_ATTR_LAYER_MASK,
    [METATILE_ATTRIBUTE_7]              = 0xFFFFFFFF
};

static const u8 sMetatileAttrShiftsEmerald[METATILE_ATTRIBUTE_COUNT] = {

    [METATILE_ATTRIBUTE_BEHAVIOR]       = METATILE_ATTR_BEHAVIOR_SHIFT,
    [METATILE_ATTRIBUTE_TERRAIN]        = 0,
    [METATILE_ATTRIBUTE_2]              = 0,
    [METATILE_ATTRIBUTE_3]              = 0,
    [METATILE_ATTRIBUTE_ENCOUNTER_TYPE] = 0,
    [METATILE_ATTRIBUTE_5]              = 0,
    [METATILE_ATTRIBUTE_LAYER_TYPE]     = METATILE_ATTR_LAYER_SHIFT,
    [METATILE_ATTRIBUTE_7]              = 0
};

const struct MapHeader *const GetMapHeaderFromConnection(const struct MapConnection *connection)
{
#if TESTING
    if (sTestMapConnectionHeaderOverride != NULL)
        return sTestMapConnectionHeaderOverride;
#endif
    return Overworld_GetMapHeaderByGroupAndId(connection->mapGroup, connection->mapNum);
}

enum MapLayoutLoadError InitMap(void)
{
    enum MapLayoutLoadError error = InitMapLayoutData(&gMapHeader);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    SetOccupiedSecretBaseEntranceMetatiles(gMapHeader.events);
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError InitMapFromSavedGame(void)
{
    enum MapLayoutLoadError error = InitMapLayoutData(&gMapHeader);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    error = InitSecretBaseAppearance(FALSE);
    if (error != MAP_LAYOUT_LOAD_OK)
        return error;
    SetOccupiedSecretBaseEntranceMetatiles(gMapHeader.events);
    LoadSavedMapView();
    UpdateTVScreensOnMap(gBackupMapLayout.width, gBackupMapLayout.height);
    return MAP_LAYOUT_LOAD_OK;
}

enum MapLayoutLoadError InitBattlePyramidMap(bool8 setPlayerPosition)
{
    CpuFastFill16(MAPGRID_UNDEFINED, sBackupMapData, sizeof(sBackupMapData));
    return GenerateBattlePyramidFloorLayout(sBackupMapData, setPlayerPosition);
}

enum MapLayoutLoadError InitTrainerHillMap(void)
{
    CpuFastFill16(MAPGRID_UNDEFINED, sBackupMapData, sizeof(sBackupMapData));
    return GenerateTrainerHillFloorLayout(sBackupMapData);
}

static enum MapLayoutLoadError InitMapLayoutData(struct MapHeader *mapHeader)
{
    struct MapLayout const *mapLayout;
    struct MapLayoutLoadContext context = {0};
    enum MapLayoutLoadError error;
    int width;
    int height;
    if (mapHeader == NULL || mapHeader->mapLayout == NULL)
        return MAP_LAYOUT_LOAD_BAD_ID;
    mapLayout = mapHeader->mapLayout;
    CpuFastFill16(MAPGRID_UNDEFINED, sBackupMapData, sizeof(sBackupMapData));
    gBackupMapLayout.map = sBackupMapData;
    if (mapLayout->width <= 0 || mapLayout->height <= 0
     || mapLayout->width > INT_MAX - MAP_OFFSET_W
     || mapLayout->height > INT_MAX - MAP_OFFSET_H)
        return MAP_LAYOUT_LOAD_BAD_SIZE;
    width = mapLayout->width + MAP_OFFSET_W;
    gBackupMapLayout.width = width;
    height = mapLayout->height + MAP_OFFSET_H;
    gBackupMapLayout.height = height;
    if ((u32)width > MAX_MAP_DATA_SIZE / (u32)height)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    error = MapLayoutBeginLoadContext(mapHeader, &context);
    if (error == MAP_LAYOUT_LOAD_OK)
    {
        error = InitBackupMapLayoutData(mapLayout, &context);
        if (error == MAP_LAYOUT_LOAD_OK)
            error = InitBackupMapLayoutConnections(mapHeader, &context);
        MapLayoutEndLoadContext(&context);
    }
    if (error != MAP_LAYOUT_LOAD_OK)
        CpuFastFill16(MAPGRID_UNDEFINED, sBackupMapData, sizeof(sBackupMapData));
    return error;
}

#if TESTING && IS_WAYFARER
extern const u16 *const gMapLayoutRawOracles[];

// Retained pre-abstraction raw loader used only as the Wayfarer differential oracle.
// Keep this independent of MapLayoutCopy* and the descriptor decoder.
static enum MapLayoutLoadError LegacyRawCopyConnection(u16 *dest, s32 destWidth,
    s32 destHeight, const struct MapHeader *connectedMapHeader,
    s32 x, s32 y, s32 x2, s32 y2, s32 width, s32 height)
{
    const struct MapLayout *layout;
    const u16 *source;
    s32 row;

    if (connectedMapHeader == NULL || connectedMapHeader->mapLayout == NULL
     || connectedMapHeader->mapLayoutId == 0 || connectedMapHeader->mapLayoutId > MAP_LAYOUT_COUNT)
        return MAP_LAYOUT_LOAD_BAD_ID;
    layout = connectedMapHeader->mapLayout;
    source = gMapLayoutRawOracles[connectedMapHeader->mapLayoutId - 1];
    if (source == NULL || x < 0 || y < 0 || x2 < 0 || y2 < 0 || width <= 0 || height <= 0
     || x > destWidth - width || y > destHeight - height
     || x2 > layout->width - width || y2 > layout->height - height)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    source += layout->width * y2 + x2;
    dest += destWidth * y + x;
    for (row = 0; row < height; row++)
    {
        CpuCopy16(source, dest, width * sizeof(u16));
        source += layout->width;
        dest += destWidth;
    }
    return MAP_LAYOUT_LOAD_OK;
}
#endif

#if TESTING

enum MapLayoutLoadError Test_InitMapLayoutData(struct MapHeader *mapHeader)
{
    return InitMapLayoutData(mapHeader);
}

#if IS_WAYFARER
static enum MapLayoutLoadError LegacyRawFillConnection(u16 *dest, s32 destWidth,
    s32 destHeight, const struct MapHeader *mapHeader,
    const struct MapConnection *connection)
{
    const struct MapHeader *connected = GetMapHeaderFromConnection(connection);
    const struct MapLayout *layout;
    s32 offset = connection->offset;
    s32 x, y, x2, y2, width, height;

    if (connected == NULL || connected->mapLayout == NULL)
        return MAP_LAYOUT_LOAD_BAD_ID;
    layout = connected->mapLayout;
    switch (connection->direction)
    {
    case CONNECTION_SOUTH:
    case CONNECTION_NORTH:
        x = offset + MAP_OFFSET;
        if (x < 0)
        {
            x2 = -x;
            width = min(x + layout->width, destWidth);
            x = 0;
        }
        else
        {
            x2 = 0;
            width = min(layout->width, destWidth - x);
        }
        if (connection->direction == CONNECTION_SOUTH)
        {
            y = mapHeader->mapLayout->height + MAP_OFFSET;
            y2 = 0;
        }
        else
        {
            y = 0;
            y2 = layout->height - MAP_OFFSET;
        }
        return LegacyRawCopyConnection(dest, destWidth, destHeight, connected,
                                       x, y, x2, y2, width, MAP_OFFSET);
    case CONNECTION_WEST:
    case CONNECTION_EAST:
        y = offset + MAP_OFFSET;
        if (y < 0)
        {
            y2 = -y;
            height = min(y + layout->height, destHeight);
            y = 0;
        }
        else
        {
            y2 = 0;
            height = min(layout->height, destHeight - y);
        }
        if (connection->direction == CONNECTION_WEST)
        {
            x = 0;
            x2 = layout->width - MAP_OFFSET;
            width = MAP_OFFSET;
        }
        else
        {
            x = mapHeader->mapLayout->width + MAP_OFFSET;
            x2 = 0;
            width = MAP_OFFSET + 1;
        }
        return LegacyRawCopyConnection(dest, destWidth, destHeight, connected,
                                       x, y, x2, y2, width, height);
    default:
        return MAP_LAYOUT_LOAD_OK;
    }
}

enum MapLayoutLoadError Test_InitLegacyRawMapLayoutData(const struct MapHeader *mapHeader,
                                                        u16 *dest, u32 destCapacity,
                                                        s32 *width, s32 *height, u8 *connectionFlags)
{
    const struct MapLayout *layout;
    const u16 *source;
    s32 row;
    s32 i;

    if (mapHeader == NULL || mapHeader->mapLayout == NULL || dest == NULL
     || width == NULL || height == NULL || connectionFlags == NULL
     || mapHeader->mapLayoutId == 0 || mapHeader->mapLayoutId > MAP_LAYOUT_COUNT)
        return MAP_LAYOUT_LOAD_BAD_ID;
    layout = mapHeader->mapLayout;
    source = gMapLayoutRawOracles[mapHeader->mapLayoutId - 1];
    if (source == NULL)
        return MAP_LAYOUT_LOAD_NO_DESCRIPTOR;
    *width = layout->width + MAP_OFFSET_W;
    *height = layout->height + MAP_OFFSET_H;
    if (*width <= 0 || *height <= 0 || (u32)*width > destCapacity / (u32)*height)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    for (i = 0; i < (s32)destCapacity; i++)
        dest[i] = MAPGRID_UNDEFINED;
    for (row = 0; row < layout->height; row++)
        CpuCopy16(&source[row * layout->width],
                  &dest[(row + MAP_OFFSET) * *width + MAP_OFFSET],
                  layout->width * sizeof(u16));
    *connectionFlags = 0;
    if (mapHeader->connections != NULL)
    {
        for (i = 0; i < mapHeader->connections->count; i++)
        {
            const struct MapConnection *connection = &mapHeader->connections->connections[i];
            enum MapLayoutLoadError error = LegacyRawFillConnection(dest, *width, *height,
                                                                     mapHeader, connection);
            if (error != MAP_LAYOUT_LOAD_OK)
                return error;
            switch (connection->direction)
            {
            case CONNECTION_SOUTH: *connectionFlags |= 1; break;
            case CONNECTION_NORTH: *connectionFlags |= 2; break;
            case CONNECTION_WEST:  *connectionFlags |= 4; break;
            case CONNECTION_EAST:  *connectionFlags |= 8; break;
            }
        }
    }
    return MAP_LAYOUT_LOAD_OK;
}
#endif

void Test_ResetMapConnectionFlags(void)
{
    sMapConnectionFlags = sDummyConnectionFlags;
}

void Test_SetMapConnectionHeaderOverride(const struct MapHeader *mapHeader)
{
    sTestMapConnectionHeaderOverride = mapHeader;
}

u8 Test_GetMapConnectionFlags(void)
{
    return sMapConnectionFlags.south
         | sMapConnectionFlags.north << 1
         | sMapConnectionFlags.west << 2
         | sMapConnectionFlags.east << 3;
}
#endif

static enum MapLayoutLoadError InitBackupMapLayoutData(const struct MapLayout *mapLayout, struct MapLayoutLoadContext *context)
{
    u16 *dest;
    dest = gBackupMapLayout.map;
    dest += gBackupMapLayout.width * 7 + MAP_OFFSET;
    return MapLayoutCopyFullWithContext(context, mapLayout, dest,
                                        MAX_MAP_DATA_SIZE - (dest - sBackupMapData),
                                        gBackupMapLayout.width);
}

static enum MapLayoutLoadError InitBackupMapLayoutConnections(struct MapHeader *mapHeader, struct MapLayoutLoadContext *context)
{
    int count;
    const struct MapConnection *connection;
    int i;
    enum MapLayoutLoadError error = MAP_LAYOUT_LOAD_OK;

    if (mapHeader->connections)
    {
        count = mapHeader->connections->count;
        connection = mapHeader->connections->connections;
        sMapConnectionFlags = sDummyConnectionFlags;
        for (i = 0; i < count; i++, connection++)
        {
            struct MapHeader const *cMap = GetMapHeaderFromConnection(connection);
            s32 offset = connection->offset;
            switch (connection->direction)
            {
            case CONNECTION_SOUTH:
                error = FillSouthConnection(mapHeader, cMap, offset, context);
                if (error != MAP_LAYOUT_LOAD_OK)
                    return error;
                sMapConnectionFlags.south = TRUE;
                break;
            case CONNECTION_NORTH:
                error = FillNorthConnection(mapHeader, cMap, offset, context);
                if (error != MAP_LAYOUT_LOAD_OK)
                    return error;
                sMapConnectionFlags.north = TRUE;
                break;
            case CONNECTION_WEST:
                error = FillWestConnection(mapHeader, cMap, offset, context);
                if (error != MAP_LAYOUT_LOAD_OK)
                    return error;
                sMapConnectionFlags.west = TRUE;
                break;
            case CONNECTION_EAST:
                error = FillEastConnection(mapHeader, cMap, offset, context);
                if (error != MAP_LAYOUT_LOAD_OK)
                    return error;
                sMapConnectionFlags.east = TRUE;
                break;
            }
        }
    }
    return error;
}

static enum MapLayoutLoadError FillConnection(int x, int y, struct MapHeader const *connectedMapHeader, int x2, int y2, int width, int height, struct MapLayoutLoadContext *context)
{
    u16 *dest;

    if (x < 0 || y < 0 || width <= 0 || height <= 0
     || x >= gBackupMapLayout.width || y >= gBackupMapLayout.height
     || width > gBackupMapLayout.width - x || height > gBackupMapLayout.height - y)
        return MAP_LAYOUT_LOAD_BAD_BOUNDS;
    dest = &gBackupMapLayout.map[gBackupMapLayout.width * y + x];
    return MapLayoutCopyRectWithContext(context, connectedMapHeader->mapLayout,
                                        x2, y2, width, height, dest,
                                        MAX_MAP_DATA_SIZE - (dest - sBackupMapData),
                                        gBackupMapLayout.width);
}

static enum MapLayoutLoadError FillSouthConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset, struct MapLayoutLoadContext *context)
{
    s64 x;
    s64 x2;
    s64 width;
    s64 cWidth;

    if (connectedMapHeader)
    {
        cWidth = connectedMapHeader->mapLayout->width;
        x = (s64)offset + MAP_OFFSET;
        x2 = 0;
        width = cWidth;
        if (x < 0)
        {
            x2 = -x;
            width -= x2;
            x = 0;
        }
        if (x >= gBackupMapLayout.width || x2 >= cWidth || width <= 0)
            return MAP_LAYOUT_LOAD_BAD_BOUNDS;
        if (width > gBackupMapLayout.width - x)
            width = gBackupMapLayout.width - x;

        return FillConnection(
            x, mapHeader->mapLayout->height + MAP_OFFSET,
            connectedMapHeader,
            x2, /*y2*/ 0,
            width, /*height*/ MAP_OFFSET, context);
    }
    return MAP_LAYOUT_LOAD_OK;
}

static enum MapLayoutLoadError FillNorthConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset, struct MapLayoutLoadContext *context)
{
    s64 x;
    s64 x2;
    s64 width;
    s64 cWidth;
    int cHeight;

    if (connectedMapHeader)
    {
        cWidth = connectedMapHeader->mapLayout->width;
        cHeight = connectedMapHeader->mapLayout->height;
        x = (s64)offset + MAP_OFFSET;
        x2 = 0;
        width = cWidth;
        if (x < 0)
        {
            x2 = -x;
            width -= x2;
            x = 0;
        }
        if (x >= gBackupMapLayout.width || x2 >= cWidth || width <= 0)
            return MAP_LAYOUT_LOAD_BAD_BOUNDS;
        if (width > gBackupMapLayout.width - x)
            width = gBackupMapLayout.width - x;

        return FillConnection(
            x, /*y*/ 0,
            connectedMapHeader,
            x2, cHeight - MAP_OFFSET,
            width, /*height*/ MAP_OFFSET, context);

    }
    return MAP_LAYOUT_LOAD_OK;
}

static enum MapLayoutLoadError FillWestConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset, struct MapLayoutLoadContext *context)
{
    s64 y;
    s64 y2;
    s64 height;
    int cWidth;
    s64 cHeight;
    if (connectedMapHeader)
    {
        cWidth = connectedMapHeader->mapLayout->width;
        cHeight = connectedMapHeader->mapLayout->height;
        y = (s64)offset + MAP_OFFSET;
        y2 = 0;
        height = cHeight;
        if (y < 0)
        {
            y2 = -y;
            height -= y2;
            y = 0;
        }
        if (y >= gBackupMapLayout.height || y2 >= cHeight || height <= 0)
            return MAP_LAYOUT_LOAD_BAD_BOUNDS;
        if (height > gBackupMapLayout.height - y)
            height = gBackupMapLayout.height - y;

        return FillConnection(
            /*x*/ 0, y,
            connectedMapHeader,
            cWidth - MAP_OFFSET, y2,
            /*width*/ MAP_OFFSET, height, context);
    }
    return MAP_LAYOUT_LOAD_OK;
}

static enum MapLayoutLoadError FillEastConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset, struct MapLayoutLoadContext *context)
{
    int x;
    s64 y;
    s64 y2;
    s64 height;
    s64 cHeight;
    if (connectedMapHeader)
    {
        cHeight = connectedMapHeader->mapLayout->height;
        x = mapHeader->mapLayout->width + MAP_OFFSET;
        y = (s64)offset + MAP_OFFSET;
        y2 = 0;
        height = cHeight;
        if (y < 0)
        {
            y2 = -y;
            height -= y2;
            y = 0;
        }
        if (y >= gBackupMapLayout.height || y2 >= cHeight || height <= 0)
            return MAP_LAYOUT_LOAD_BAD_BOUNDS;
        if (height > gBackupMapLayout.height - y)
            height = gBackupMapLayout.height - y;

        return FillConnection(
            x, y,
            connectedMapHeader,
            /*x2*/ 0, y2,
            /*width*/ MAP_OFFSET + 1, height, context);
    }
    return MAP_LAYOUT_LOAD_OK;
}

u8 MapGridGetElevationAt(int x, int y)
{
    u16 block = GetMapGridBlockAt(x, y);

    if (block == MAPGRID_UNDEFINED)
        return 0;

    return UNPACK_ELEVATION(block);
}

u8 MapGridGetCollisionAt(int x, int y)
{
    u16 block = GetMapGridBlockAt(x, y);

    if (block == MAPGRID_UNDEFINED)
        return TRUE;

    return UNPACK_COLLISION(block);
}

u32 GetNumTilesInPrimary(struct MapLayout const *mapLayout)
{
    switch (mapLayout->layoutVersion)
    {
    case LAYOUT_VERSION_FRLG: return NUM_TILES_IN_PRIMARY;
    case LAYOUT_VERSION_HNS:  return NUM_TILES_IN_PRIMARY;
    default:                  return NUM_TILES_IN_PRIMARY_EMERALD;
    }
}

u32 GetNumMetatilesInPrimary(struct MapLayout const *mapLayout)
{
    switch (mapLayout->layoutVersion)
    {
    case LAYOUT_VERSION_FRLG: return NUM_METATILES_IN_PRIMARY;
    case LAYOUT_VERSION_HNS:  return NUM_METATILES_IN_PRIMARY;
    default:                  return NUM_METATILES_IN_PRIMARY_EMERALD;
    }
}

u32 GetNumPalsInPrimary(struct MapLayout const *mapLayout)
{
    switch (mapLayout->layoutVersion)
    {
    case LAYOUT_VERSION_FRLG: return NUM_PALS_IN_PRIMARY;
    case LAYOUT_VERSION_HNS:  return NUM_PALS_IN_PRIMARY;
    default:                  return NUM_PALS_IN_PRIMARY_EMERALD;
    }
}

u32 MapGridGetMetatileIdAt(int x, int y)
{
    u16 block = GetMapGridBlockAt(x, y);

    if (block == MAPGRID_UNDEFINED)
        return GetBorderBlockAt(x, y) & MAPGRID_METATILE_ID_MASK;

    return UNPACK_METATILE(block);
}

u32 MapGridGetMetatileAttributeAt(s16 x, s16 y, u8 attributeType)
{
    u16 metatileId = MapGridGetMetatileIdAt(x, y);
    return GetAttributeByMetatileIdAndMapLayout(metatileId, attributeType, gMapHeader.mapLayout->layoutVersion);
}

u32 MapGridGetMetatileBehaviorAt(int x, int y)
{
    return MapGridGetMetatileAttributeAt(x, y, METATILE_ATTRIBUTE_BEHAVIOR);
}

u8 MapGridGetMetatileLayerTypeAt(int x, int y)
{
    return MapGridGetMetatileAttributeAt(x, y, METATILE_ATTRIBUTE_LAYER_TYPE);
}

void MapGridSetMetatileIdAt(int x, int y, u16 metatile)
{
    int i;
    if (AreCoordsWithinMapGridBounds(x, y))
    {
        i = x + y * gBackupMapLayout.width;

        // Elevation is ignored in the argument, but copy metatile ID and collision
        gBackupMapLayout.map[i] = (gBackupMapLayout.map[i] & MAPGRID_ELEVATION_MASK) | (metatile & ~MAPGRID_ELEVATION_MASK);
    }
}

void MapGridSetMetatileEntryAt(int x, int y, u16 metatile)
{
    int i;
    if (AreCoordsWithinMapGridBounds(x, y))
    {
        i = x + gBackupMapLayout.width * y;
        gBackupMapLayout.map[i] = metatile;
    }
}

u32 ExtractMetatileAttribute(u32 attributes, u8 attributeType, u8 layoutVersion)
{
    u32 attribute;

    if (attributeType >= METATILE_ATTRIBUTE_COUNT)
        return attributes;

    if (layoutVersion == LAYOUT_VERSION_FRLG)
    {
        attribute = (attributes & sMetatileAttrMasks[attributeType]) >> sMetatileAttrShifts[attributeType];
        if (attributeType == METATILE_ATTRIBUTE_BEHAVIOR)
            return NormalizeFrlgMetatileBehavior(attribute);
        return attribute;
    }

    return (attributes & sMetatileAttrMasksEmerald[attributeType]) >> sMetatileAttrShiftsEmerald[attributeType];
}

static u32 GetAttributeByMetatileIdAndMapLayoutFrlg(u16 metatile, u8 attributeType)
{
    u32 attribute;
    if (metatile < GetNumMetatilesInPrimary(gMapHeader.mapLayout))
    {
        const u32 *attributes = (const u32*)gMapHeader.mapLayout->primaryTileset->metatileAttributes;
        attribute = attributes[metatile];
    }
    else if (metatile < NUM_METATILES_TOTAL)
    {
        const u32 *attributes = (const u32*) gMapHeader.mapLayout->secondaryTileset->metatileAttributes;
        metatile -= GetNumMetatilesInPrimary(gMapHeader.mapLayout);
        attribute = attributes[metatile];
    }
    else
    {
        return MB_INVALID;
    }

    return ExtractMetatileAttribute(attribute, attributeType, LAYOUT_VERSION_FRLG);
}

u32 GetAttributeByMetatileIdAndMapLayout(u16 metatile, u8 attributeType, u8 layoutVersion)
{
    u32 attribute;

    if (layoutVersion == LAYOUT_VERSION_FRLG)
        return GetAttributeByMetatileIdAndMapLayoutFrlg(metatile, attributeType);

    if (metatile < GetNumMetatilesInPrimary(gMapHeader.mapLayout))
    {
        const u16 *attributes = (const u16*)gMapHeader.mapLayout->primaryTileset->metatileAttributes;
        attribute = attributes[metatile];
    }
    else if (metatile < NUM_METATILES_TOTAL)
    {
        const u16 *attributes = (const u16*)gMapHeader.mapLayout->secondaryTileset->metatileAttributes;
        metatile -= GetNumMetatilesInPrimary(gMapHeader.mapLayout);
        attribute = attributes[metatile];
    }
    else
    {
        return MB_INVALID;
    }

    return ExtractMetatileAttribute(attribute, attributeType, FALSE);
}

void SaveMapView(void)
{
    int i, j;
    int x, y;
    u16 *mapView;
    int width;
    mapView = gSaveBlock1Ptr->mapView;
    width = gBackupMapLayout.width;
    x = gSaveBlock1Ptr->pos.x;
    y = gSaveBlock1Ptr->pos.y;
    for (i = y; i < y + MAP_OFFSET_H; i++)
    {
        for (j = x; j < x + MAP_OFFSET_W; j++)
            *mapView++ = sBackupMapData[width * i + j];
    }
}

static bool32 SavedMapViewIsEmpty(void)
{
    u16 i;
    u32 marker = 0;

#ifndef UBFIX
    // BUG: This loop extends past the bounds of the mapView array. Its size is only 0x100.
    for (i = 0; i < 0x200; i++)
        marker |= gSaveBlock1Ptr->mapView[i];
#else
    // UBFIX: Only iterate over 0x100
    for (i = 0; i < ARRAY_COUNT(gSaveBlock1Ptr->mapView); i++)
        marker |= gSaveBlock1Ptr->mapView[i];
#endif


    if (marker == 0)
        return TRUE;
    else
        return FALSE;
}

static void ClearSavedMapView(void)
{
    CpuFill16(0, gSaveBlock1Ptr->mapView, sizeof(gSaveBlock1Ptr->mapView));
}

static void LoadSavedMapView(void)
{
    u8 yMode;
    int i, j;
    int x, y;
    u16 *mapView;
    int width;
    mapView = gSaveBlock1Ptr->mapView;
    if (!SavedMapViewIsEmpty())
    {
        width = gBackupMapLayout.width;
        x = gSaveBlock1Ptr->pos.x;
        y = gSaveBlock1Ptr->pos.y;
        for (i = y; i < y + MAP_OFFSET_H; i++)
        {
            if (i == y && i != 0)
                yMode = 0;
            else if (i == y + MAP_OFFSET_H - 1 && i != gMapHeader.mapLayout->height - 1)
                yMode = 1;
            else
                yMode = 0xFF;

            for (j = x; j < x + MAP_OFFSET_W; j++)
            {
                if (!SkipCopyingMetatileFromSavedMap(&sBackupMapData[j + width * i], width, yMode))
                    sBackupMapData[j + width * i] = *mapView;
                mapView++;
            }
        }
        for (j = x; j < x + MAP_OFFSET_W; j++)
        {
            if (y != 0)
                FixLongGrassMetatilesWindowTop(j, y - 1);
            if (i < gMapHeader.mapLayout->height - 1)
                FixLongGrassMetatilesWindowBottom(j, y + MAP_OFFSET_H - 1);
        }
        ClearSavedMapView();
    }
}

static void MoveMapViewToBackup(enum Connection direction)
{
    int width;
    u16 *mapView;
    int x0, y0;
    int x2, y2;
    u16 *src, *dest;
    int srci, desti;
    int r9, r8;
    int x, y;
    int i, j;
    mapView = gSaveBlock1Ptr->mapView;
    width = gBackupMapLayout.width;
    r9 = 0;
    r8 = 0;
    x0 = gSaveBlock1Ptr->pos.x;
    y0 = gSaveBlock1Ptr->pos.y;
    x2 = MAP_OFFSET_W;
    y2 = MAP_OFFSET_H;

    switch (direction)
    {
    case CONNECTION_NORTH:
        y0 += 1;
        y2 = MAP_OFFSET_H - 1;
        break;
    case CONNECTION_SOUTH:
        r8 = 1;
        y2 = MAP_OFFSET_H - 1;
        break;
    case CONNECTION_WEST:
        x0 += 1;
        x2 = MAP_OFFSET_W - 1;
        break;
    case CONNECTION_EAST:
        r9 = 1;
        x2 = MAP_OFFSET_W - 1;
        break;
    default:
        break;
    }

    for (y = 0; y < y2; y++)
    {
        i = 0;
        j = 0;
        for (x = 0; x < x2; x++)
        {
            desti = width * (y + y0);
            srci = (y + r8) * MAP_OFFSET_W + r9;
            src = &mapView[srci + i];
            dest = &sBackupMapData[x0 + desti + j];
            *dest = *src;
            i++;
            j++;
        }
    }

    ClearSavedMapView();
}

enum Connection GetMapBorderIdAt(int x, int y)
{
    if (GetMapGridBlockAt(x, y) == MAPGRID_UNDEFINED)
        return CONNECTION_INVALID;

    if (x >= (gBackupMapLayout.width - (MAP_OFFSET + 1)))
    {
        if (!sMapConnectionFlags.east)
            return CONNECTION_INVALID;

        return CONNECTION_EAST;
    }
    else if (x < MAP_OFFSET)
    {
        if (!sMapConnectionFlags.west)
            return CONNECTION_INVALID;

        return CONNECTION_WEST;
    }
    else if (y >= (gBackupMapLayout.height - MAP_OFFSET))
    {
        if (!sMapConnectionFlags.south)
            return CONNECTION_INVALID;

        return CONNECTION_SOUTH;
    }
    else if (y < MAP_OFFSET)
    {
        if (!sMapConnectionFlags.north)
            return CONNECTION_INVALID;

        return CONNECTION_NORTH;
    }
    else
    {
        return CONNECTION_NONE;
    }
}

enum Connection GetPostCameraMoveMapBorderId(int x, int y)
{
    return GetMapBorderIdAt(gSaveBlock1Ptr->pos.x + MAP_OFFSET + x, gSaveBlock1Ptr->pos.y + MAP_OFFSET + y);
}

bool32 CanCameraMoveInDirection(enum Direction direction)
{
    int x, y;
    x = gSaveBlock1Ptr->pos.x + MAP_OFFSET + gDirectionToVectors[direction].x;
    y = gSaveBlock1Ptr->pos.y + MAP_OFFSET + gDirectionToVectors[direction].y;

    if (GetMapBorderIdAt(x, y) == CONNECTION_INVALID)
        return FALSE;

    return TRUE;
}

static void SetPositionFromConnection(const struct MapConnection *connection, enum Connection direction, int x, int y)
{
    struct MapHeader const *mapHeader = GetMapHeaderFromConnection(connection);

    switch (direction)
    {
    case CONNECTION_EAST:
        gSaveBlock1Ptr->pos.x = -x;
        gSaveBlock1Ptr->pos.y -= connection->offset;
        break;
    case CONNECTION_WEST:
        gSaveBlock1Ptr->pos.x = mapHeader->mapLayout->width;
        gSaveBlock1Ptr->pos.y -= connection->offset;
        break;
    case CONNECTION_SOUTH:
        gSaveBlock1Ptr->pos.x -= connection->offset;
        gSaveBlock1Ptr->pos.y = -y;
        break;
    case CONNECTION_NORTH:
        gSaveBlock1Ptr->pos.x -= connection->offset;
        gSaveBlock1Ptr->pos.y = mapHeader->mapLayout->height;
        break;
    default:
        errorf("invalid direction: %d", direction);
        break;
    }
}

bool8 CameraMove(int x, int y)
{
    enum Connection direction;
    const struct MapConnection *connection;
    int old_x, old_y;
    gCamera.active = FALSE;
    direction = GetPostCameraMoveMapBorderId(x, y);
    if (direction == CONNECTION_NONE || direction == CONNECTION_INVALID)
    {
        gSaveBlock1Ptr->pos.x += x;
        gSaveBlock1Ptr->pos.y += y;
    }
    else
    {
        SaveMapView();
        ClearMirageTowerPulseBlendEffect();
        old_x = gSaveBlock1Ptr->pos.x;
        old_y = gSaveBlock1Ptr->pos.y;
        connection = GetIncomingConnection(direction, gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
        assertf(connection)
        {
            return gCamera.active;
        }

        SetPositionFromConnection(connection, direction, x, y);
        LoadMapFromCameraTransition(connection->mapGroup, connection->mapNum);
        gCamera.active = TRUE;
        gCamera.x = old_x - gSaveBlock1Ptr->pos.x;
        gCamera.y = old_y - gSaveBlock1Ptr->pos.y;
        gSaveBlock1Ptr->pos.x += x;
        gSaveBlock1Ptr->pos.y += y;
        MoveMapViewToBackup(direction);
    }

    return gCamera.active;
}

static const struct MapConnection *GetIncomingConnection(enum Connection direction, int x, int y)
{
    int count;
    int i;
    const struct MapConnection *connection;
    const struct MapConnections *connections = gMapHeader.connections;

#ifdef UBFIX // UB: Multiple possible null dereferences
    if (connections == NULL || connections->connections == NULL)
        return NULL;
#endif
    count = connections->count;
    connection = connections->connections;
    for (i = 0; i < count; i++, connection++)
    {
        if (connection->direction == direction && IsPosInIncomingConnectingMap(direction, x, y, connection) == TRUE)
            return connection;
    }
    return NULL;
}

static bool8 IsPosInIncomingConnectingMap(enum Connection direction, int x, int y, const struct MapConnection *connection)
{
    struct MapHeader const *mapHeader;
    mapHeader = GetMapHeaderFromConnection(connection);
    switch (direction)
    {
    case CONNECTION_SOUTH:
    case CONNECTION_NORTH:
        return IsCoordInIncomingConnectingMap(x, gMapHeader.mapLayout->width, mapHeader->mapLayout->width, connection->offset);
    case CONNECTION_WEST:
    case CONNECTION_EAST:
        return IsCoordInIncomingConnectingMap(y, gMapHeader.mapLayout->height, mapHeader->mapLayout->height, connection->offset);
    default:
        return FALSE;
    }
}

static bool8 IsCoordInIncomingConnectingMap(int coord, int srcMax, int destMax, int offset)
{
    int offset2;
    offset2 = offset;

    if (offset2 < 0)
        offset2 = 0;

    if (destMax + offset < srcMax)
        srcMax = destMax + offset;

    if (offset2 <= coord && coord <= srcMax)
        return TRUE;

    return FALSE;
}

static int IsCoordInConnectingMap(int coord, int max)
{
    if (coord >= 0 && coord < max)
        return TRUE;

    return FALSE;
}

static int IsPosInConnectingMap(const struct MapConnection *connection, int x, int y)
{
    struct MapHeader const *mapHeader;
    mapHeader = GetMapHeaderFromConnection(connection);
    switch (connection->direction)
    {
    case CONNECTION_SOUTH:
    case CONNECTION_NORTH:
        return IsCoordInConnectingMap(x - connection->offset, mapHeader->mapLayout->width);
    case CONNECTION_WEST:
    case CONNECTION_EAST:
        return IsCoordInConnectingMap(y - connection->offset, mapHeader->mapLayout->height);
    }
    return FALSE;
}

const struct MapConnection *GetMapConnectionAtPos(s16 x, s16 y)
{
    int count;
    const struct MapConnection *connection;
    int i;
    enum Connection direction;
    if (!gMapHeader.connections)
    {
        return NULL;
    }
    else
    {
        count = gMapHeader.connections->count;
        connection = gMapHeader.connections->connections;
        for (i = 0; i < count; i++, connection++)
        {
            direction = connection->direction;
            if ((direction == CONNECTION_DIVE || direction == CONNECTION_EMERGE)
             || (direction == CONNECTION_NORTH && y > MAP_OFFSET - 1)
             || (direction == CONNECTION_SOUTH && y < gMapHeader.mapLayout->height + MAP_OFFSET)
             || (direction == CONNECTION_WEST && x > MAP_OFFSET - 1)
             || (direction == CONNECTION_EAST && x < gMapHeader.mapLayout->width + MAP_OFFSET))
            {
                continue;
            }
            if (IsPosInConnectingMap(connection, x - MAP_OFFSET, y - MAP_OFFSET) == TRUE)
            {
                return connection;
            }
        }
    }
    return NULL;
}

void SetCameraFocusCoords(u16 x, u16 y)
{
    gSaveBlock1Ptr->pos.x = x - MAP_OFFSET;
    gSaveBlock1Ptr->pos.y = y - MAP_OFFSET;
}

void GetCameraFocusCoords(u16 *x, u16 *y)
{
    *x = gSaveBlock1Ptr->pos.x + MAP_OFFSET;
    *y = gSaveBlock1Ptr->pos.y + MAP_OFFSET;
}

static void UNUSED SetCameraCoords(u16 x, u16 y)
{
    gSaveBlock1Ptr->pos.x = x;
    gSaveBlock1Ptr->pos.y = y;
}

void GetCameraCoords(u16 *x, u16 *y)
{
    *x = gSaveBlock1Ptr->pos.x;
    *y = gSaveBlock1Ptr->pos.y;
}

void MapGridSetMetatileImpassabilityAt(int x, int y, bool32 impassable)
{
    if (AreCoordsWithinMapGridBounds(x, y))
    {
        if (impassable)
            gBackupMapLayout.map[x + gBackupMapLayout.width * y] |= MAPGRID_COLLISION_MASK;
        else
            gBackupMapLayout.map[x + gBackupMapLayout.width * y] &= ~MAPGRID_COLLISION_MASK;
    }
}

static bool8 SkipCopyingMetatileFromSavedMap(u16 *mapBlock, u16 mapWidth, u8 yMode)
{
    if (yMode == 0xFF)
        return FALSE;

    if (yMode == 0)
        mapBlock -= mapWidth;
    else
        mapBlock += mapWidth;

    if (IsLargeBreakableDecoration(UNPACK_METATILE(*mapBlock), yMode) == TRUE)
        return TRUE;
    return FALSE;
}

static void CopyTilesetToVram(struct Tileset const *tileset, u16 numTiles, u16 offset)
{
    if (tileset)
    {
        if (!tileset->isCompressed)
            LoadBgTiles(2, tileset->tiles, numTiles * 32, offset);
        else
            DecompressAndCopyTileDataToVram(2, tileset->tiles, numTiles * 32, offset, 0);
    }
}

static void CopyTilesetToVramUsingHeap(struct Tileset const *tileset, u16 numTiles, u16 offset)
{
    if (tileset)
    {
        if (!tileset->isCompressed)
            LoadBgTiles(2, tileset->tiles, numTiles * 32, offset);
        else
            DecompressAndLoadBgGfxUsingHeap(2, tileset->tiles, numTiles * 32, offset, 0);
    }
}

// Below two are dummied functions from FRLG, used to tint the overworld palettes for the Quest Log
static void ApplyGlobalTintToPaletteEntries(u16 offset, u16 size)
{

}

static void UNUSED ApplyGlobalTintToPaletteSlot(u8 slot, u8 count)
{

}

static void LoadTilesetPalette(struct Tileset const *tileset, u16 destOffset, u16 size, bool8 skipFaded, u32 numPalsInPrimary)
{
    if (tileset)
    {
        if (tileset->isSecondary == FALSE)
        {
            if (skipFaded)
                CpuFastCopy(tileset->palettes, &gPlttBufferUnfaded[destOffset], size); // always word-aligned
            else
                LoadPaletteFast(tileset->palettes, destOffset, size);
            gPlttBufferFaded[destOffset] = gPlttBufferUnfaded[destOffset] = RGB_BLACK;
            ApplyGlobalTintToPaletteEntries(destOffset + 1, (size - 2) >> 1);
        }
        else if (tileset->isSecondary == TRUE)
        {
            // All 'gTilesetPalettes_' arrays should have ALIGNED(4) in them,
            // but we use SmartCopy here just in case they don't
            if (skipFaded)
                CpuCopy16(tileset->palettes[numPalsInPrimary], &gPlttBufferUnfaded[destOffset], size);
            else
                LoadPaletteFast(tileset->palettes[numPalsInPrimary], destOffset, size);
        }
        else
        {
            LoadPalette((const u16 *)tileset->palettes, destOffset, size);
            ApplyGlobalTintToPaletteEntries(destOffset, size >> 1);
        }
    }
}

void CopyPrimaryTilesetToVram(struct MapLayout const *mapLayout)
{
    CopyTilesetToVram(mapLayout->primaryTileset, GetNumTilesInPrimary(mapLayout), 0);
}

void CopyPrimaryTilesetToVramUsingHeap(struct MapLayout const *mapLayout)
{
    CopyTilesetToVramUsingHeap(mapLayout->primaryTileset, GetNumTilesInPrimary(mapLayout), 0);
}

void CopySecondaryTilesetToVram(struct MapLayout const *mapLayout)
{
    CopyTilesetToVram(mapLayout->secondaryTileset, NUM_TILES_TOTAL - GetNumTilesInPrimary(mapLayout), GetNumTilesInPrimary(mapLayout));
}

void CopySecondaryTilesetToVramUsingHeap(struct MapLayout const *mapLayout)
{
    CopyTilesetToVramUsingHeap(mapLayout->secondaryTileset, NUM_TILES_TOTAL - GetNumTilesInPrimary(mapLayout), GetNumTilesInPrimary(mapLayout));
}

void LoadPrimaryTilesetPalette(struct MapLayout const *mapLayout, bool8 skipFaded)
{
    LoadTilesetPalette(mapLayout->primaryTileset, 0, GetNumPalsInPrimary(mapLayout) * PLTT_SIZE_4BPP, skipFaded, GetNumPalsInPrimary(mapLayout));
}

void LoadSecondaryTilesetPalette(struct MapLayout const *mapLayout, bool8 skipFaded)
{
    LoadTilesetPalette(mapLayout->secondaryTileset, GetNumPalsInPrimary(mapLayout) * 16, (NUM_PALS_TOTAL - GetNumPalsInPrimary(mapLayout)) * PLTT_SIZE_4BPP, skipFaded, GetNumPalsInPrimary(mapLayout));
}

void CopyMapTilesetsToVram(struct MapLayout const *mapLayout)
{
    if (mapLayout)
    {
        CopyTilesetToVramUsingHeap(mapLayout->primaryTileset, GetNumTilesInPrimary(mapLayout), 0);
        CopyTilesetToVramUsingHeap(mapLayout->secondaryTileset, NUM_TILES_TOTAL - GetNumTilesInPrimary(mapLayout), GetNumTilesInPrimary(mapLayout));
    }
}

void LoadMapTilesetPalettes(struct MapLayout const *mapLayout)
{
    if (mapLayout)
    {
        LoadPrimaryTilesetPalette(mapLayout, FALSE);
        LoadSecondaryTilesetPalette(mapLayout, FALSE);
    }
}
