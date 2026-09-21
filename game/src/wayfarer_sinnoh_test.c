#include "global.h"
#include "fieldmap.h"
#include "map_layout.h"
#include "overworld.h"
#include "wayfarer_persistence.h"
#include "wayfarer_sinnoh_test.h"
#include "data/map_group_count.h"

#if IS_WAYFARER && (TESTING || defined(E2E_TESTING))
// This deliberately lives outside every SaveBlock. It is a test traversal
// return token, never a heal point, origin, ticket, or story checkpoint.
static EWRAM_DATA struct
{
    bool8 active;
    s16 mapGroup;
    s16 mapNum;
    s16 x;
    s16 y;
} sSinnohTestReturn;

bool8 EnterSinnohForTest(s16 mapGroup, s16 mapNum, s16 x, s16 y)
{
    const struct MapHeader *mapHeader;
    const struct MapHeader *returnMapHeader;
    u16 block;

    if (sSinnohTestReturn.active || mapGroup < 0 || mapGroup >= MAP_GROUPS_COUNT
     || mapNum < 0 || mapNum >= MAP_GROUP_COUNT[mapGroup] || x < 0 || y < 0 || x > 127 || y > 127
     || gSaveBlock1Ptr->location.mapGroup < 0 || gSaveBlock1Ptr->location.mapGroup >= MAP_GROUPS_COUNT
     || gSaveBlock1Ptr->location.mapNum < 0
     || gSaveBlock1Ptr->location.mapNum >= MAP_GROUP_COUNT[gSaveBlock1Ptr->location.mapGroup]
     || WayfarerGetRegionForMap(mapGroup, mapNum) != REGION_SINNOH)
        return FALSE;
    returnMapHeader = Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->location.mapGroup,
                                                         gSaveBlock1Ptr->location.mapNum);
    if (returnMapHeader == NULL || returnMapHeader->mapLayout == NULL
     || gSaveBlock1Ptr->pos.x < 0 || gSaveBlock1Ptr->pos.y < 0
     || gSaveBlock1Ptr->pos.x >= returnMapHeader->mapLayout->width
     || gSaveBlock1Ptr->pos.y >= returnMapHeader->mapLayout->height)
        return FALSE;
    mapHeader = Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    if (mapHeader == NULL || mapHeader->mapLayout == NULL || x >= mapHeader->mapLayout->width || y >= mapHeader->mapLayout->height)
        return FALSE;
    if (MapLayoutReadTile(mapHeader->mapLayout, x, y, &block) != MAP_LAYOUT_LOAD_OK)
        return FALSE;
    if (UNPACK_COLLISION(block) != 0)
        return FALSE;

    sSinnohTestReturn.active = TRUE;
    sSinnohTestReturn.mapGroup = gSaveBlock1Ptr->location.mapGroup;
    sSinnohTestReturn.mapNum = gSaveBlock1Ptr->location.mapNum;
    sSinnohTestReturn.x = gSaveBlock1Ptr->pos.x;
    sSinnohTestReturn.y = gSaveBlock1Ptr->pos.y;
    SetWarpDestination(mapGroup, mapNum, WARP_ID_NONE, x, y);
    WarpIntoMap();
    return TRUE;
}

bool8 ReturnFromSinnohTest(void)
{
    if (!sSinnohTestReturn.active)
        return FALSE;

    // WarpData coordinates are signed bytes, while authored maps and the saved
    // player position may exceed 127. Load the saved map through an encodable
    // placeholder, then restore the full-width coordinates before field init.
    SetWarpDestination(sSinnohTestReturn.mapGroup, sSinnohTestReturn.mapNum, WARP_ID_NONE, 0, 0);
    WarpIntoMap();
    gSaveBlock1Ptr->pos.x = sSinnohTestReturn.x;
    gSaveBlock1Ptr->pos.y = sSinnohTestReturn.y;
    sSinnohTestReturn.active = FALSE;
    return TRUE;
}

bool8 IsSinnohTestTraversalActive(void)
{
    return sSinnohTestReturn.active;
}
#endif
