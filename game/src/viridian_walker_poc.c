#include "global.h"
#include "viridian_walker_poc.h"
#include "viridian_walker_world.h"

#if IS_WAYFARER && VIRIDIAN_WALKER_POC

#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "malloc.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "constants/event_object_movement.h"
#include "constants/map_event_ids.h"

#define WALKER_MAX_TILES (56 * 50)
#define WALKER_SEARCH_SLICE 8
#define WALKER_BLOCKED_LIMIT 4

// A bounded 11,200-byte workspace handles every supported outdoor map.
struct WalkerWork {
    u16 queue[WALKER_MAX_TILES];
    u8 back[WALKER_MAX_TILES];
    u8 path[WALKER_MAX_TILES];
};

EWRAM_DATA struct ViridianWalkerDebug gViridianWalkerDebug = {0};
static EWRAM_DATA struct WalkerWork *sWork = NULL;
static EWRAM_DATA u16 sRead = 0;
static EWRAM_DATA u16 sWrite = 0;
static EWRAM_DATA u16 sRemaining = 0;
static EWRAM_DATA u16 sWait = 0;
static EWRAM_DATA u8 sGoal = 0;
static EWRAM_DATA u8 sBlocked = 0;
static EWRAM_DATA u8 sInStep = FALSE;
static EWRAM_DATA u8 sExitStep = FALSE;
static EWRAM_DATA u8 sDoorStep = FALSE;
static EWRAM_DATA u8 sExitReported = FALSE;
static EWRAM_DATA u8 sProxyActive = FALSE;
static EWRAM_DATA u8 sProxyStep = FALSE;
static EWRAM_DATA u8 sPostSeamStep = FALSE;
static EWRAM_DATA u8 sProxyDirection = DIR_NONE;
static EWRAM_DATA u8 sGrassEmoted = FALSE;
static EWRAM_DATA u8 sInitialized = FALSE;
static EWRAM_DATA u16 sSearchStartFrame = 0;
static EWRAM_DATA s16 sWidth = 0;
static EWRAM_DATA s16 sHeight = 0;
static EWRAM_DATA u16 sTileCount = 0;
static EWRAM_DATA u16 sActiveMap = 0;
static EWRAM_DATA u8 sActiveMapValid = FALSE;

static const s8 sDx[5] = {0, 0, 0, -1, 1};
static const s8 sDy[5] = {0, 1, -1, 0, 0};

static bool8 CanWalkFrom(struct ObjectEvent *walker, s16 x, s16 y, enum Direction direction);

static u32 ScanlineStamp(void)
{
    u32 first, second;
    u16 vcount;
    do
    {
        first = *(volatile u32 *)&gMain.vblankCounter1;
        vcount = REG_VCOUNT;
        second = *(volatile u32 *)&gMain.vblankCounter1;
    } while (first != second);
    return first * 228 + (vcount + 68) % 228;
}

void ViridianWalker_OnHeapReset(void)
{
    // InitHeap immediately overwrites the allocator headers. Never Free this
    // pointer afterwards; the next field frame allocates a fresh workspace.
    sWork = NULL;
    sInitialized = FALSE;
}

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

static bool8 OnMap(void)
{
    u16 map = CurrentMap();
    return map == MAP_VIRIDIAN_CITY_HNS || map == MAP_ROUTE2_HNS || map == MAP_ROUTE1_HNS;
}

static bool8 IsSupportedMap(u16 map)
{
    return map == MAP_VIRIDIAN_CITY_HNS || map == MAP_ROUTE2_HNS || map == MAP_ROUTE1_HNS;
}

static u8 LocalId(u16 map)
{
    if (map == MAP_VIRIDIAN_CITY_HNS)
        return LOCALID_VIRIDIAN_WALKER_POC;
    if (map == MAP_ROUTE2_HNS)
        return LOCALID_ROUTE2_WALKER_POC;
    return LOCALID_ROUTE1_WALKER_POC;
}

static bool8 IsWalkerIdentity(const struct ObjectEvent *objectEvent, u16 map)
{
    return objectEvent->active && IsSupportedMap(map)
        && objectEvent->mapGroup == MAP_GROUP(map)
        && objectEvent->mapNum == MAP_NUM(map)
        && objectEvent->localId == LocalId(map);
}

static struct ObjectEvent *FindActorOnMap(u16 map)
{
    u8 i;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
        if (IsWalkerIdentity(&gObjectEvents[i], map))
            return &gObjectEvents[i];
    return NULL;
}

bool8 ViridianWalker_IsObject(const struct ObjectEvent *objectEvent)
{
    return IsWalkerIdentity(objectEvent, MAP_VIRIDIAN_CITY_HNS)
        || IsWalkerIdentity(objectEvent, MAP_ROUTE2_HNS)
        || IsWalkerIdentity(objectEvent, MAP_ROUTE1_HNS);
}

bool8 ViridianWalker_HasVisibleActorOnMap(u16 map)
{
    struct ObjectEvent *walker = FindActorOnMap(map);
    return walker != NULL && !walker->invisible && !walker->offScreen;
}

static struct ObjectEvent *GetWalker(void)
{
    u8 id;
    if (TryGetObjectEventIdByLocalIdAndMap(LocalId(CurrentMap()),
        gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, &id))
        return NULL;
    return &gObjectEvents[id];
}

static bool8 RecoverWalker(struct ObjectEvent *walker)
{
    const struct ViridianWalkerWorldState *world = ViridianWorld_Get();
    static const s8 offsets[][3] = {
        {0, 0, DIR_NONE}, {0, -1, DIR_NORTH}, {-1, 0, DIR_WEST},
        {1, 0, DIR_EAST}, {0, 1, DIR_SOUTH},
    };
    s16 x, y;
    s16 savedX = world->x + MAP_OFFSET;
    s16 savedY = world->y + MAP_OFFSET;
    u8 elevation = MapGridGetElevationAt(savedX, savedY);
    u8 i;
    ObjectEventClearHeldMovementIfActive(walker);
    for (i = 0; i < ARRAY_COUNT(offsets); i++)
    {
        x = world->x + MAP_OFFSET + offsets[i][0];
        y = world->y + MAP_OFFSET + offsets[i][1];
        if (x < MAP_OFFSET || y < MAP_OFFSET || x >= sWidth + MAP_OFFSET || y >= sHeight + MAP_OFFSET)
            continue;
        if (MapGridGetCollisionAt(x, y) != 0 || MapGridGetElevationAt(x, y) != elevation)
            continue;
        if (i != 0 && !CanWalkFrom(walker, savedX, savedY, offsets[i][2]))
            continue;
        if (GetObjectObjectCollidesWith(walker, x, y, FALSE) == OBJECT_EVENTS_COUNT)
            break;
    }
    if (i == ARRAY_COUNT(offsets))
        return FALSE;
    MoveObjectEventToMapCoords(walker, x, y);
    walker->currentElevation = walker->previousElevation = MapGridGetElevationAt(x, y);
    walker->invisible = FALSE;
    return TRUE;
}

static u16 TileIndex(s16 x, s16 y)
{
    return y * sWidth + x;
}

static void NextGoal(struct ObjectEvent *walker)
{
    u16 map = CurrentMap();
    if (map == MAP_VIRIDIAN_CITY_HNS)
        sGoal = ViridianWorld_Get()->destinationMap == MAP_ROUTE1_HNS
            ? WALKER_GOAL_ROUTE1 : WALKER_GOAL_ROUTE2;
    else if (map == MAP_ROUTE1_HNS)
        sGoal = sGoal == WALKER_GOAL_ROUTE1_SPOT
            ? WALKER_GOAL_ROUTE1_RETURN : WALKER_GOAL_ROUTE1_SPOT;
    else
        sGoal = ViridianWorld_Get()->destinationMap == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS
            ? WALKER_GOAL_ROUTE2_GATE : WALKER_GOAL_ROUTE2_RETURN;
    sBlocked = 0;
    gViridianWalkerDebug.goal = sGoal;
    VarSet(VAR_TEMP_2, sGoal);
    ViridianWorld_ActorMoved(walker->currentCoords.x - MAP_OFFSET,
        walker->currentCoords.y - MAP_OFFSET, sGoal);
    gViridianWalkerDebug.phase = WALKER_PHASE_SEARCH;
    sRead = sWrite = 0;
}

static void BeginSearch(struct ObjectEvent *walker)
{
    s16 x = walker->currentCoords.x - MAP_OFFSET;
    s16 y = walker->currentCoords.y - MAP_OFFSET;
    if (x < 0 || x >= sWidth || y < 0 || y >= sHeight)
    {
        NextGoal(walker);
        return;
    }
    memset(sWork->back, 0xFF, sizeof(sWork->back));
    sWork->queue[0] = TileIndex(x, y);
    sWork->back[sWork->queue[0]] = DIR_NONE;
    sRead = 0;
    sWrite = 1;
    gViridianWalkerDebug.searches++;
    gViridianWalkerDebug.searchNodes = 0;
    gViridianWalkerDebug.searchFrames = 0;
    sSearchStartFrame = gMain.vblankCounter1;
    gViridianWalkerDebug.phase = WALKER_PHASE_SEARCH;
}

static bool8 CanWalkFrom(struct ObjectEvent *walker, s16 x, s16 y, enum Direction direction)
{
    struct ObjectEvent probe = *walker;
    probe.currentCoords.x = x;
    probe.currentCoords.y = y;
    probe.currentElevation = MapGridGetElevationAt(x, y);
    probe.currentMetatileBehavior = MapGridGetMetatileBehaviorAt(x, y);
    return GetCollisionAtCoords(&probe, x + sDx[direction], y + sDy[direction], direction) == COLLISION_NONE;
}

static bool8 IsGoalTile(struct ObjectEvent *walker, s16 x, s16 y)
{
    u8 i;
    u16 destination;
    switch (sGoal)
    {
    case WALKER_GOAL_GRASS:
        if (gViridianWalkerDebug.usedGrassFallback)
            return x == 34 && y == 34; // decorative green strip east of the Center
        return MetatileBehavior_IsTallGrass(MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET));
    case WALKER_GOAL_MART:
        destination = MAP_VIRIDIAN_CITY_MART_HNS;
        break;
    case WALKER_GOAL_CENTER:
        destination = MAP_VIRIDIAN_CITY_POKEMON_CENTER_HNS;
        break;
    case WALKER_GOAL_ROUTE1:
        return CurrentMap() == MAP_VIRIDIAN_CITY_HNS && y == sHeight - 1
            && GetMapBorderIdAt(x + MAP_OFFSET, y + MAP_OFFSET + 1) != CONNECTION_INVALID
            && CanWalkFrom(walker, x + MAP_OFFSET, y + MAP_OFFSET, DIR_SOUTH);
    case WALKER_GOAL_ROUTE2:
        return CurrentMap() == MAP_VIRIDIAN_CITY_HNS && y == 0
            && GetMapBorderIdAt(x + MAP_OFFSET, y + MAP_OFFSET - 1) != CONNECTION_INVALID
            && CanWalkFrom(walker, x + MAP_OFFSET, y + MAP_OFFSET, DIR_NORTH);
    case WALKER_GOAL_ROUTE1_SPOT:
        return CurrentMap() == MAP_ROUTE1_HNS && x == 31 && y == 15;
    case WALKER_GOAL_ROUTE1_RETURN:
        return CurrentMap() == MAP_ROUTE1_HNS && y == 0
            && GetMapBorderIdAt(x + MAP_OFFSET, y + MAP_OFFSET - 1) != CONNECTION_INVALID
            && CanWalkFrom(walker, x + MAP_OFFSET, y + MAP_OFFSET, DIR_NORTH);
    case WALKER_GOAL_ROUTE2_RETURN:
        return CurrentMap() == MAP_ROUTE2_HNS && y == sHeight - 1
            && GetMapBorderIdAt(x + MAP_OFFSET, y + MAP_OFFSET + 1) != CONNECTION_INVALID
            && CanWalkFrom(walker, x + MAP_OFFSET, y + MAP_OFFSET, DIR_SOUTH);
    case WALKER_GOAL_ROUTE2_GATE:
        destination = MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS;
        break;
    default:
        return FALSE;
    }
    for (i = 0; i < gMapHeader.events->warpCount; i++)
    {
        const struct WarpEvent *warp = &gMapHeader.events->warps[i];
        if (warp->mapGroup == MAP_GROUP(destination) && warp->mapNum == MAP_NUM(destination)
            && warp->x == x && warp->y + 1 == y)
            return TRUE;
    }
    return FALSE;
}

static void FinishSearch(u16 target)
{
    u16 count = 0;
    u16 source = sWork->queue[0];
    gViridianWalkerDebug.goalX = target % sWidth + MAP_OFFSET;
    gViridianWalkerDebug.goalY = target / sWidth + MAP_OFFSET;
    while (target != source && count < sTileCount)
    {
        u8 direction = sWork->back[target];
        sWork->path[count++] = direction;
        target -= sDx[direction] + sDy[direction] * sWidth;
    }
    sRemaining = count;
    gViridianWalkerDebug.phase = WALKER_PHASE_WALK;
}

static void SearchSlice(struct ObjectEvent *walker)
{
    u8 expanded = 0;
    u16 frameAtStart = gMain.vblankCounter1;
    u32 scanlineAtStart = ScanlineStamp();
    u32 elapsedScanlines;
    while (sRead < sWrite && expanded < WALKER_SEARCH_SLICE)
    {
        u16 tile = sWork->queue[sRead++];
        s16 x = tile % sWidth;
        s16 y = tile / sWidth;
        u8 dir;
        expanded++;
        gViridianWalkerDebug.searchNodes++;
        if (IsGoalTile(walker, x, y))
        {
            FinishSearch(tile);
            break;
        }
        for (dir = DIR_SOUTH; dir <= DIR_EAST; dir++)
        {
            s16 nx = x + sDx[dir];
            s16 ny = y + sDy[dir];
            u16 next;
            struct ObjectEvent probe;
            if (nx < 0 || nx >= sWidth || ny < 0 || ny >= sHeight)
                continue;
            next = TileIndex(nx, ny);
            if (sWork->back[next] != 0xFF)
                continue;
            probe = *walker;
            probe.currentCoords.x = x + MAP_OFFSET;
            probe.currentCoords.y = y + MAP_OFFSET;
            probe.currentElevation = MapGridGetElevationAt(probe.currentCoords.x, probe.currentCoords.y);
            probe.currentMetatileBehavior = MapGridGetMetatileBehaviorAt(probe.currentCoords.x, probe.currentCoords.y);
            if (GetCollisionAtCoords(&probe, nx + MAP_OFFSET, ny + MAP_OFFSET, dir) != COLLISION_NONE)
                continue;
            sWork->back[next] = dir;
            sWork->queue[sWrite++] = next;
        }
    }
    if (expanded > gViridianWalkerDebug.maxNodesPerFrame)
        gViridianWalkerDebug.maxNodesPerFrame = expanded;
    gViridianWalkerDebug.searchFrames = gMain.vblankCounter1 - sSearchStartFrame + 1;
    if ((u16)(gMain.vblankCounter1 - frameAtStart) > gViridianWalkerDebug.maxSliceVblanks)
        gViridianWalkerDebug.maxSliceVblanks = gMain.vblankCounter1 - frameAtStart;
    elapsedScanlines = ScanlineStamp() - scanlineAtStart;
    if (elapsedScanlines > gViridianWalkerDebug.maxSliceScanlines)
        gViridianWalkerDebug.maxSliceScanlines = elapsedScanlines;
    if (sRead == sWrite && gViridianWalkerDebug.phase == WALKER_PHASE_SEARCH)
        NextGoal(walker);
}

static enum Direction ExitDirection(void)
{
    if (sGoal == WALKER_GOAL_ROUTE1 || sGoal == WALKER_GOAL_ROUTE2_RETURN)
        return DIR_SOUTH;
    if (sGoal == WALKER_GOAL_ROUTE2 || sGoal == WALKER_GOAL_ROUTE1_RETURN)
        return DIR_NORTH;
    return DIR_WEST;
}

static const struct MapConnection *FindExitConnection(u16 destination, enum Connection direction)
{
    const struct MapConnections *connections = gMapHeader.connections;
    s32 i;
    if (connections == NULL)
        return NULL;
    for (i = 0; i < connections->count; i++)
    {
        const struct MapConnection *connection = &connections->connections[i];
        if (connection->direction == direction && connection->mapGroup == MAP_GROUP(destination)
         && connection->mapNum == MAP_NUM(destination))
            return connection;
    }
    return NULL;
}

static const struct MapConnection *FindBorderConnection(u16 destination)
{
    const struct MapConnection *connection = FindExitConnection(destination, CONNECTION_NORTH);
    if (connection == NULL)
        connection = FindExitConnection(destination, CONNECTION_SOUTH);
    return connection;
}

static bool8 ProjectWorldToBorder(const struct MapConnection *connection, u8 worldX, u8 worldY,
    s16 *x, s16 *y, u8 *direction)
{
    const struct MapHeader *destination = GetMapHeaderFromConnection(connection);
    *x = worldX + connection->offset;
    if (connection->direction == CONNECTION_NORTH)
    {
        *y = worldY - destination->mapLayout->height;
        *direction = DIR_NORTH;
        return *x >= 0 && *x < sWidth && *y < 0 && *y >= -MAP_OFFSET;
    }
    *y = worldY + sHeight;
    *direction = DIR_SOUTH;
    return *x >= 0 && *x < sWidth && *y >= sHeight && *y < sHeight + MAP_OFFSET;
}

static bool8 ProjectBorderToWorld(const struct MapConnection *connection, s16 x, s16 y,
    u8 *worldX, u8 *worldY)
{
    const struct MapHeader *destination = GetMapHeaderFromConnection(connection);
    s16 destX = x - connection->offset;
    s16 destY = connection->direction == CONNECTION_NORTH
        ? y + destination->mapLayout->height : y - sHeight;
    if (destX < 0 || destX >= destination->mapLayout->width
     || destY < 0 || destY >= destination->mapLayout->height)
        return FALSE;
    *worldX = destX;
    *worldY = destY;
    return TRUE;
}

static bool8 ProxyHeadsIntoPlayerMap(u16 worldMap, u8 goal)
{
    u16 playerMap = CurrentMap();
    return (worldMap == MAP_VIRIDIAN_CITY_HNS
             && ((playerMap == MAP_ROUTE2_HNS && goal == WALKER_GOAL_ROUTE2)
              || (playerMap == MAP_ROUTE1_HNS && goal == WALKER_GOAL_ROUTE1)))
        || (playerMap == MAP_VIRIDIAN_CITY_HNS
             && ((worldMap == MAP_ROUTE2_HNS && goal == WALKER_GOAL_ROUTE2_RETURN)
              || (worldMap == MAP_ROUTE1_HNS && goal == WALKER_GOAL_ROUTE1_RETURN)));
}

static u8 ProxyDirection(const struct MapConnection *connection, u16 worldMap, u8 goal)
{
    u8 outward = connection->direction == CONNECTION_NORTH ? DIR_NORTH : DIR_SOUTH;
    if (ProxyHeadsIntoPlayerMap(worldMap, goal))
        return outward == DIR_NORTH ? DIR_SOUTH : DIR_NORTH;
    return outward;
}

static bool8 ProxyWouldBeVisible(s16 x, s16 y)
{
    x += MAP_OFFSET;
    y += MAP_OFFSET;
    return x >= gSaveBlock1Ptr->pos.x - 2 && x < gSaveBlock1Ptr->pos.x + 16
        && y >= gSaveBlock1Ptr->pos.y - 2 && y <= gSaveBlock1Ptr->pos.y + 16;
}

static bool8 ReportExit(struct ObjectEvent *walker)
{
    s16 x = walker->currentCoords.x - MAP_OFFSET;
    u16 map = CurrentMap();
    u16 destination;
    enum Connection direction;
    u8 arrival;
    const struct MapConnection *connection;
    const struct MapHeader *destinationHeader;
    s16 destX, destY;
    if (map == MAP_VIRIDIAN_CITY_HNS && sGoal == WALKER_GOAL_ROUTE2)
        destination = MAP_ROUTE2_HNS;
    else if (map == MAP_VIRIDIAN_CITY_HNS && sGoal == WALKER_GOAL_ROUTE1)
        destination = MAP_ROUTE1_HNS;
    else if ((map == MAP_ROUTE2_HNS && sGoal == WALKER_GOAL_ROUTE2_RETURN)
          || (map == MAP_ROUTE1_HNS && sGoal == WALKER_GOAL_ROUTE1_RETURN))
        destination = MAP_VIRIDIAN_CITY_HNS;
    else
        return FALSE;
    direction = ExitDirection() == DIR_NORTH ? CONNECTION_NORTH : CONNECTION_SOUTH;
    connection = FindExitConnection(destination, direction);
    if (connection == NULL)
        return FALSE;
    destinationHeader = GetMapHeaderFromConnection(connection);
    destX = x - connection->offset;
    destY = direction == CONNECTION_NORTH ? destinationHeader->mapLayout->height - 1 : 0;
    if (destX < 0 || destX >= destinationHeader->mapLayout->width)
        return FALSE;
    arrival = direction == CONNECTION_NORTH ? WORLD_ARRIVAL_SOUTH : WORLD_ARRIVAL_NORTH;
    ViridianWorld_ActorCrossedExit(destination, arrival, x, destX, destY);
    sExitReported = TRUE;
    sProxyActive = TRUE;
    sProxyStep = TRUE;
    sProxyDirection = ExitDirection();
    return TRUE;
}

static void CompleteGoal(struct ObjectEvent *walker)
{
    s16 x = walker->currentCoords.x - MAP_OFFSET;
    gViridianWalkerDebug.completed++;
    gViridianWalkerDebug.lastCompletedGoal = sGoal;
    sGrassEmoted = FALSE;
    sWait = sGoal == WALKER_GOAL_GRASS || sGoal == WALKER_GOAL_ROUTE1_SPOT ? 120 : 180;
    if (sGoal == WALKER_GOAL_GRASS || sGoal == WALKER_GOAL_ROUTE1_SPOT)
    {
        ViridianWorld_ActorAtSpot(x, walker->currentCoords.y - MAP_OFFSET, sGoal);
        gViridianWalkerDebug.phase = WALKER_PHASE_WAIT;
    }
    else if (sGoal == WALKER_GOAL_ROUTE2_GATE)
    {
        ViridianWorld_ActorInside(MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS);
        RemoveObjectEvent(walker);
        Free(sWork);
        sWork = NULL;
        sInitialized = FALSE;
        gViridianWalkerDebug.phase = WALKER_PHASE_INACTIVE;
    }
    else
    {
        sExitReported = FALSE;
        RemoveObjectEvent(walker);
        Free(sWork);
        sWork = NULL;
        sInitialized = FALSE;
        gViridianWalkerDebug.phase = WALKER_PHASE_INACTIVE;
    }
}

static void Replan(struct ObjectEvent *walker)
{
    gViridianWalkerDebug.replans++;
    gViridianWalkerDebug.blockedSteps++;
    sInStep = FALSE;
    if (++sBlocked >= WALKER_BLOCKED_LIMIT)
        NextGoal(walker);
    else
        BeginSearch(walker);
}

static void WalkStep(struct ObjectEvent *walker)
{
    u8 dir;
    s16 nx, ny;
    if (walker->frozen)
        return;
    if (sInStep)
    {
        if (!ObjectEventClearHeldMovementIfFinished(walker))
            return;
        sInStep = FALSE;
        if (walker->currentCoords.x != gViridianWalkerDebug.nextX
         || walker->currentCoords.y != gViridianWalkerDebug.nextY)
        {
            Replan(walker);
            return;
        }
        if (sExitStep)
        {
            sExitStep = FALSE;
            CompleteGoal(walker);
            return;
        }
        if (sDoorStep)
        {
            sDoorStep = FALSE;
            CompleteGoal(walker);
            return;
        }
        ViridianWorld_ActorMoved(walker->currentCoords.x - MAP_OFFSET,
            walker->currentCoords.y - MAP_OFFSET, sGoal);
    }
    if (sRemaining == 0)
    {
        if (sGoal == WALKER_GOAL_GRASS || sGoal == WALKER_GOAL_ROUTE1_SPOT)
        {
            CompleteGoal(walker);
            return;
        }
        if (sGoal == WALKER_GOAL_MART || sGoal == WALKER_GOAL_CENTER
         || sGoal == WALKER_GOAL_ROUTE2_GATE)
        {
            dir = DIR_NORTH;
            sDoorStep = TRUE;
        }
        else
        {
            dir = ExitDirection();
            sExitStep = TRUE;
        }
    }
    else
        dir = sWork->path[sRemaining - 1];
    nx = walker->currentCoords.x + sDx[dir];
    ny = walker->currentCoords.y + sDy[dir];
    if (sDoorStep)
    {
        // Outdoor door metatiles are flagged impassable. Scripted movement may
        // cross that one known warp tile, but never an occupied tile.
        if (GetObjectObjectCollidesWith(walker, nx, ny, FALSE) != OBJECT_EVENTS_COUNT)
        {
            sDoorStep = FALSE;
            Replan(walker);
            return;
        }
    }
    else if (GetCollisionAtCoords(walker, nx, ny, dir) != COLLISION_NONE)
    {
        sExitStep = FALSE;
        Replan(walker);
        return;
    }
    if (ObjectEventSetHeldMovement(walker, GetWalkNormalMovementAction(dir)))
        return;
    if (!sExitStep && !sDoorStep)
        sRemaining--;
    gViridianWalkerDebug.nextX = nx;
    gViridianWalkerDebug.nextY = ny;
    sInStep = TRUE;
    if (sExitStep && !ReportExit(walker))
    {
        ObjectEventClearHeldMovementIfActive(walker);
        sExitStep = FALSE;
        Replan(walker);
    }
}

static void StopProxy(struct ObjectEvent *walker)
{
    if (walker != NULL)
        RemoveObjectEvent(walker);
    if (sWork != NULL)
        Free(sWork);
    sWork = NULL;
    sInitialized = FALSE;
    sProxyActive = sProxyStep = sExitStep = sInStep = FALSE;
    gViridianWalkerDebug.phase = WALKER_PHASE_INACTIVE;
}

static bool8 RecoverProxy(struct ObjectEvent *walker)
{
    const struct ViridianWalkerWorldState *world = ViridianWorld_Get();
    const struct MapConnection *connection = FindBorderConnection(world->currentMap);
    s16 x, y;
    if (connection == NULL || !ProjectWorldToBorder(connection, world->x, world->y,
        &x, &y, &sProxyDirection))
        return FALSE;
    ObjectEventClearHeldMovementIfActive(walker);
    MoveObjectEventToMapCoords(walker, x + MAP_OFFSET, y + MAP_OFFSET);
    walker->currentElevation = walker->previousElevation = MapGridGetElevationAt(
        x + MAP_OFFSET, y + MAP_OFFSET);
    walker->invisible = FALSE;
    sGoal = world->goal;
    gViridianWalkerDebug.goal = sGoal;
    sProxyDirection = ProxyDirection(connection, world->currentMap, sGoal);
    sProxyStep = sExitStep = sInStep = FALSE;
    sInitialized = TRUE;
    return TRUE;
}

static void UpdateProxy(struct ObjectEvent *walker)
{
    const struct ViridianWalkerWorldState *world = ViridianWorld_Get();
    const struct MapConnection *connection;
    s16 nx, ny;
    u8 worldX, worldY;
    if (!sInitialized && !RecoverProxy(walker))
    {
        StopProxy(walker);
        return;
    }
    if (walker->frozen)
        return;
    if (sProxyStep)
    {
        u8 wasExitStep = sExitStep;
        if (!ObjectEventClearHeldMovementIfFinished(walker))
            return;
        sProxyStep = sInStep = FALSE;
        if (wasExitStep)
        {
            sExitStep = FALSE;
            gViridianWalkerDebug.completed++;
            gViridianWalkerDebug.lastCompletedGoal = sGoal;
            if (sWork != NULL)
            {
                Free(sWork);
                sWork = NULL;
            }
        }
        else
        {
            connection = FindBorderConnection(world->currentMap);
            if (connection != NULL && ProjectBorderToWorld(connection,
                walker->currentCoords.x - MAP_OFFSET, walker->currentCoords.y - MAP_OFFSET,
                &worldX, &worldY))
                ViridianWorld_ActorMovedAcrossSeam(worldX, worldY, sGoal);
        }
    }
    if (walker->offScreen)
    {
        StopProxy(walker);
        return;
    }
    connection = FindBorderConnection(world->currentMap);
    if (connection == NULL)
    {
        StopProxy(walker);
        return;
    }
    nx = walker->currentCoords.x + sDx[sProxyDirection];
    ny = walker->currentCoords.y + sDy[sProxyDirection];
    if (ProxyHeadsIntoPlayerMap(world->currentMap, sGoal)
     && ((connection->direction == CONNECTION_NORTH && ny == MAP_OFFSET)
      || (connection->direction == CONNECTION_SOUTH && ny == sHeight - 1 + MAP_OFFSET)))
    {
        if (GetCollisionAtCoords(walker, nx, ny, sProxyDirection) != COLLISION_NONE)
            return;
        if (ObjectEventSetHeldMovement(walker, GetWalkNormalMovementAction(sProxyDirection)))
            return;
        ViridianWorld_ActorEnteredPlayerMapFromProxy(CurrentMap(),
            connection->direction == CONNECTION_NORTH ? WORLD_ARRIVAL_NORTH : WORLD_ARRIVAL_SOUTH,
            world->x, nx - MAP_OFFSET, ny - MAP_OFFSET);
        sProxyActive = sProxyStep = FALSE;
        sPostSeamStep = TRUE;
        sInitialized = FALSE;
        return;
    }
    if (!ProjectBorderToWorld(connection, nx - MAP_OFFSET, ny - MAP_OFFSET, &worldX, &worldY))
    {
        StopProxy(walker);
        return;
    }
    if (GetCollisionAtCoords(walker, nx, ny, sProxyDirection) != COLLISION_NONE)
    {
        // Once collision data ends, retire only after the sprite itself has
        // left the viewport. A blocking actor keeps us in place until clear.
        if (walker->offScreen)
            StopProxy(walker);
        return;
    }
    if (ObjectEventSetHeldMovement(walker, GetWalkNormalMovementAction(sProxyDirection)))
        return;
    ViridianWorld_ActorMovedAcrossSeam(worldX, worldY, sGoal);
    sProxyStep = TRUE;
    gViridianWalkerDebug.nextX = nx;
    gViridianWalkerDebug.nextY = ny;
    gViridianWalkerDebug.phase = WALKER_PHASE_WALK;
}

static void DiscoverGrass(void)
{
    s16 x, y;
    gViridianWalkerDebug.grassTilesFound = 0;
    gViridianWalkerDebug.usedGrassFallback = 0;
    if (CurrentMap() != MAP_VIRIDIAN_CITY_HNS)
        return;
    for (y = 0; y < sHeight; y++)
        for (x = 0; x < sWidth; x++)
            if (MetatileBehavior_IsTallGrass(MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET)))
                gViridianWalkerDebug.grassTilesFound++;
    if (gViridianWalkerDebug.grassTilesFound == 0)
        gViridianWalkerDebug.usedGrassFallback = 1;
}

static void RetagActor(struct ObjectEvent *walker, u16 map)
{
    walker->mapGroup = MAP_GROUP(map);
    walker->mapNum = MAP_NUM(map);
    walker->localId = LocalId(map);
    walker->initialCoords = walker->currentCoords;
}

void ViridianWalker_Update(void)
{
    struct ObjectEvent *walker;
    const struct ViridianWalkerWorldState *world;
    const struct MapConnection *connection;
    u16 map;
    if (gSaveBlock1Ptr == NULL)
        return;
    map = CurrentMap();
    if (OnMap())
    {
        sWidth = gMapHeader.mapLayout->width;
        sHeight = gMapHeader.mapLayout->height;
        sTileCount = sWidth * sHeight;
    }
    if (!sActiveMapValid || sActiveMap != map)
    {
        u16 oldMap = sActiveMap;
        struct ObjectEvent *oldWalker = sActiveMapValid ? FindActorOnMap(oldMap) : NULL;
        bool8 migrateLocal = FALSE;
        bool8 migrateProxy = FALSE;
        u8 direction = DIR_NONE;
        s16 borderX, borderY;
        // Camera connections can retain the source object's slot during the
        // seam. Its full source-map identity distinguishes it from the actor
        // freshly spawned on the destination map.
        world = ViridianWorld_Get();
        if (oldWalker != NULL && IsSupportedMap(map) && gCamera.active)
        {
            if (sProxyActive && world->currentMap == map)
                migrateLocal = TRUE;
            else if (world->currentMap == oldMap && !oldWalker->offScreen)
            {
                connection = FindBorderConnection(oldMap);
                if (connection != NULL && ProjectWorldToBorder(connection, world->x, world->y,
                    &borderX, &borderY, &direction))
                    migrateProxy = TRUE;
            }
        }
        if (oldWalker != NULL)
        {
            if (migrateLocal || migrateProxy)
                RetagActor(oldWalker, map);
            else
                RemoveObjectEvent(oldWalker);
        }
        if (sWork != NULL)
            Free(sWork);
        sWork = NULL;
        sInitialized = FALSE;
        sPostSeamStep = migrateLocal && oldWalker->heldMovementActive;
        sProxyActive = migrateProxy;
        sProxyStep = migrateProxy && oldWalker->heldMovementActive;
        sProxyDirection = migrateProxy
            ? ProxyDirection(connection, world->currentMap, world->goal) : DIR_NONE;
        sInStep = sExitStep = sDoorStep = sExitReported = FALSE;
        sRemaining = sRead = sWrite = sWait = 0;
        sBlocked = 0;
        if (migrateProxy)
            sInitialized = TRUE;
        sActiveMap = map;
        sActiveMapValid = TRUE;
        gViridianWalkerDebug.phase = WALKER_PHASE_INACTIVE;
    }
    if (!OnMap())
    {
        return;
    }
    if (sTileCount > WALKER_MAX_TILES)
        return;
    world = ViridianWorld_Get();
    gViridianWalkerDebug.currentMap = map;
    gViridianWalkerDebug.localId = LocalId(map);
    walker = GetWalker();
    if (!ViridianWorld_IsOnPlayerMap())
    {
        s16 borderX, borderY;
        bool8 canShowProxy = FALSE;
        connection = FindBorderConnection(world->currentMap);
        if (connection != NULL && world->state == WORLD_STATE_TRAVELLING
         && ProjectWorldToBorder(connection, world->x, world->y,
            &borderX, &borderY, &sProxyDirection)
         && ProxyWouldBeVisible(borderX, borderY))
            canShowProxy = TRUE;
        if (canShowProxy)
            sProxyDirection = ProxyDirection(connection, world->currentMap, world->goal);
        if (!sProxyActive && canShowProxy)
        {
            sProxyActive = TRUE;
            sInitialized = FALSE;
        }
        if (sProxyActive && walker != NULL)
            UpdateProxy(walker);
        else if (walker != NULL)
            RemoveObjectEvent(walker);
        else if (canShowProxy)
        {
            TrySpawnObjectEvent(LocalId(map), MAP_NUM(map), MAP_GROUP(map));
            walker = GetWalker();
            if (walker != NULL)
            {
                sProxyActive = TRUE;
                sInitialized = FALSE;
                UpdateProxy(walker);
            }
        }
        return;
    }
    if (sPostSeamStep && walker != NULL)
    {
        if (!ObjectEventClearHeldMovementIfFinished(walker))
            return;
        sPostSeamStep = FALSE;
    }
    if (walker == NULL)
    {
        TrySpawnObjectEvent(LocalId(map), MAP_NUM(map), MAP_GROUP(map));
        walker = GetWalker();
        sInitialized = FALSE;
    }
    if (walker == NULL)
        return;
    if (!sInitialized)
    {
        if (!RecoverWalker(walker))
            return;
        sWork = Alloc(sizeof(*sWork));
        if (sWork == NULL)
            return;
        DiscoverGrass();
        sInStep = sExitStep = sDoorStep = sExitReported = FALSE;
        sGrassEmoted = FALSE;
        sRemaining = sRead = sWrite = sWait = 0;
        sBlocked = 0;
        if (map == MAP_VIRIDIAN_CITY_HNS && world->goal == WALKER_GOAL_GRASS
         && world->state == WORLD_STATE_AT_SPOT)
            sGoal = WALKER_GOAL_GRASS;
        else if (map == MAP_ROUTE1_HNS && world->goal == WALKER_GOAL_ROUTE1_SPOT
         && world->state == WORLD_STATE_AT_SPOT && world->dwell != 0)
            sGoal = WALKER_GOAL_ROUTE1_SPOT;
        else if (map == MAP_ROUTE1_HNS)
            sGoal = world->goal == WALKER_GOAL_ROUTE1_RETURN
                ? WALKER_GOAL_ROUTE1_RETURN : WALKER_GOAL_ROUTE1_SPOT;
        else if (map == MAP_ROUTE2_HNS)
            sGoal = world->destinationMap == MAP_GATE_ROUTE2_VIRIDIAN_FOREST_HNS
                ? WALKER_GOAL_ROUTE2_GATE : WALKER_GOAL_ROUTE2_RETURN;
        else
            sGoal = world->destinationMap == MAP_ROUTE1_HNS
                ? WALKER_GOAL_ROUTE1 : WALKER_GOAL_ROUTE2;
        VarSet(VAR_TEMP_2, sGoal);
        gViridianWalkerDebug.goal = sGoal;
        if (world->state == WORLD_STATE_AT_SPOT && world->dwell != 0
         && (sGoal == WALKER_GOAL_GRASS || sGoal == WALKER_GOAL_ROUTE1_SPOT))
        {
            sWait = 120;
            sGrassEmoted = FALSE;
            gViridianWalkerDebug.phase = WALKER_PHASE_WAIT;
        }
        else
            BeginSearch(walker);
        sInitialized = TRUE;
    }
    gViridianWalkerDebug.x = walker->currentCoords.x;
    gViridianWalkerDebug.y = walker->currentCoords.y;
    if (walker->frozen)
        return;
    switch (gViridianWalkerDebug.phase)
    {
    case WALKER_PHASE_SEARCH:
        if (sWrite == 0)
            BeginSearch(walker);
        else
            SearchSlice(walker);
        break;
    case WALKER_PHASE_WALK:
        WalkStep(walker);
        break;
    case WALKER_PHASE_WAIT:
        if (sWait != 0)
            sWait--;
        else if (!sGrassEmoted)
        {
            if (!ObjectEventSetHeldMovement(walker, MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK))
                sGrassEmoted = TRUE;
        }
        else if (ObjectEventClearHeldMovementIfFinished(walker))
            NextGoal(walker);
        break;
    }
}

#endif
