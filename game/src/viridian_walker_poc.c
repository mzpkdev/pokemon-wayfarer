#include "global.h"
#include "viridian_walker_poc.h"

#if IS_WAYFARER && VIRIDIAN_WALKER_POC

#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "malloc.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "constants/event_object_movement.h"
#include "constants/map_event_ids.h"

#define WALKER_WIDTH 56
#define WALKER_HEIGHT 50
#define WALKER_TILES (WALKER_WIDTH * WALKER_HEIGHT)
#define WALKER_SEARCH_SLICE 8
#define WALKER_BLOCKED_LIMIT 4

// 11,200 bytes on the engine heap while Viridian is loaded; no bulk EWRAM/IWRAM static data.
struct WalkerWork {
    u16 queue[WALKER_TILES];
    u8 back[WALKER_TILES];
    u8 path[WALKER_TILES];
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
static EWRAM_DATA u8 sEntryStep = FALSE;
static EWRAM_DATA u8 sGrassEmoted = FALSE;
static EWRAM_DATA u8 sInitialized = FALSE;
static EWRAM_DATA u8 sRecoverAfterHeapReset = FALSE;
static EWRAM_DATA u16 sSearchStartFrame = 0;
static EWRAM_DATA s16 sDoorX = 0;
static EWRAM_DATA s16 sDoorY = 0;

static const s8 sDx[5] = {0, 0, 0, -1, 1};
static const s8 sDy[5] = {0, 1, -1, 0, 0};

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
    if (sWork != NULL)
        sRecoverAfterHeapReset = TRUE;
    sWork = NULL;
    sInitialized = FALSE;
}

static bool8 OnMap(void)
{
    return gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(MAP_VIRIDIAN_CITY_HNS)
        && gSaveBlock1Ptr->location.mapNum == MAP_NUM(MAP_VIRIDIAN_CITY_HNS);
}

bool8 ViridianWalker_IsObject(const struct ObjectEvent *objectEvent)
{
    return OnMap() && objectEvent->localId == VIRIDIAN_WALKER_LOCAL_ID;
}

static struct ObjectEvent *GetWalker(void)
{
    u8 id;
    if (TryGetObjectEventIdByLocalIdAndMap(VIRIDIAN_WALKER_LOCAL_ID,
        MAP_NUM(MAP_VIRIDIAN_CITY_HNS), MAP_GROUP(MAP_VIRIDIAN_CITY_HNS), &id))
        return NULL;
    return &gObjectEvents[id];
}

static bool8 RecoverWalker(struct ObjectEvent *walker)
{
    static const s16 sSafeSidewalk[][2] = {
        {32 + MAP_OFFSET, 37 + MAP_OFFSET},
        {33 + MAP_OFFSET, 37 + MAP_OFFSET},
        {31 + MAP_OFFSET, 37 + MAP_OFFSET},
        {32 + MAP_OFFSET, 38 + MAP_OFFSET},
    };
    s16 x = walker->currentCoords.x;
    s16 y = walker->currentCoords.y;
    u8 i;
    ObjectEventClearHeldMovementIfActive(walker);
    if (walker->invisible || x < MAP_OFFSET || x >= WALKER_WIDTH + MAP_OFFSET
     || y < MAP_OFFSET || y >= WALKER_HEIGHT + MAP_OFFSET)
    {
        for (i = 0; i < ARRAY_COUNT(sSafeSidewalk); i++)
        {
            x = sSafeSidewalk[i][0];
            y = sSafeSidewalk[i][1];
            if (GetObjectObjectCollidesWith(walker, x, y, FALSE) == OBJECT_EVENTS_COUNT)
                break;
        }
        if (i == ARRAY_COUNT(sSafeSidewalk))
            return FALSE;
    }
    MoveObjectEventToMapCoords(walker, x, y);
    walker->currentElevation = walker->previousElevation = MapGridGetElevationAt(x, y);
    walker->invisible = FALSE;
    sRecoverAfterHeapReset = FALSE;
    return TRUE;
}

static u16 TileIndex(s16 x, s16 y)
{
    return y * WALKER_WIDTH + x;
}

static void NextGoal(void)
{
    sGoal = (sGoal + 1) % 6;
    sBlocked = 0;
    gViridianWalkerDebug.goal = sGoal;
    VarSet(VAR_TEMP_2, sGoal);
    gViridianWalkerDebug.phase = WALKER_PHASE_SEARCH;
    sRead = sWrite = 0;
}

static void BeginSearch(struct ObjectEvent *walker)
{
    s16 x = walker->currentCoords.x - MAP_OFFSET;
    s16 y = walker->currentCoords.y - MAP_OFFSET;
    if (x < 0 || x >= WALKER_WIDTH || y < 0 || y >= WALKER_HEIGHT)
    {
        NextGoal();
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
        return y == WALKER_HEIGHT - 1
            && GetMapBorderIdAt(x + MAP_OFFSET, y + MAP_OFFSET + 1) != CONNECTION_INVALID
            && CanWalkFrom(walker, x + MAP_OFFSET, y + MAP_OFFSET, DIR_SOUTH);
    case WALKER_GOAL_ROUTE2:
        return y == 0
            && GetMapBorderIdAt(x + MAP_OFFSET, y + MAP_OFFSET - 1) != CONNECTION_INVALID
            && CanWalkFrom(walker, x + MAP_OFFSET, y + MAP_OFFSET, DIR_NORTH);
    default:
        return x == 0
            && GetMapBorderIdAt(x + MAP_OFFSET - 1, y + MAP_OFFSET) != CONNECTION_INVALID
            && CanWalkFrom(walker, x + MAP_OFFSET, y + MAP_OFFSET, DIR_WEST);
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
    gViridianWalkerDebug.goalX = target % WALKER_WIDTH + MAP_OFFSET;
    gViridianWalkerDebug.goalY = target / WALKER_WIDTH + MAP_OFFSET;
    while (target != source && count < WALKER_TILES)
    {
        u8 direction = sWork->back[target];
        sWork->path[count++] = direction;
        target -= sDx[direction] + sDy[direction] * WALKER_WIDTH;
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
        s16 x = tile % WALKER_WIDTH;
        s16 y = tile / WALKER_WIDTH;
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
            if (nx < 0 || nx >= WALKER_WIDTH || ny < 0 || ny >= WALKER_HEIGHT)
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
        NextGoal();
}

static enum Direction ExitDirection(void)
{
    if (sGoal == WALKER_GOAL_ROUTE1)
        return DIR_SOUTH;
    if (sGoal == WALKER_GOAL_ROUTE2)
        return DIR_NORTH;
    return DIR_WEST;
}

static void CompleteGoal(struct ObjectEvent *walker)
{
    gViridianWalkerDebug.completed++;
    gViridianWalkerDebug.lastCompletedGoal = sGoal;
    sGrassEmoted = FALSE;
    sWait = sGoal == WALKER_GOAL_GRASS ? 120 : 180;
    if (sGoal == WALKER_GOAL_GRASS)
        gViridianWalkerDebug.phase = WALKER_PHASE_WAIT;
    else
    {
        sDoorX = walker->currentCoords.x;
        sDoorY = walker->currentCoords.y;
        walker->invisible = TRUE;
        // An invisible active event still collides. Park it in the unused
        // corner of the map grid while the visit/travel timer runs.
        MoveObjectEventToMapCoords(walker, 0, 0);
        gViridianWalkerDebug.phase = WALKER_PHASE_HIDDEN;
    }
}

static bool8 FindEntrance(struct ObjectEvent *walker, u8 fromGoal, s16 *x, s16 *y, enum Direction *inward)
{
    s16 i;
    u8 incoming = fromGoal == WALKER_GOAL_ROUTE1 ? WALKER_GOAL_ROUTE2
        : fromGoal == WALKER_GOAL_ROUTE2 ? WALKER_GOAL_ROUTE22 : WALKER_GOAL_ROUTE1;
    for (i = 0; i < (incoming == WALKER_GOAL_ROUTE22 ? WALKER_HEIGHT : WALKER_WIDTH); i++)
    {
        s16 px = incoming == WALKER_GOAL_ROUTE22 ? 0 : i;
        s16 py = incoming == WALKER_GOAL_ROUTE1 ? WALKER_HEIGHT - 1
            : incoming == WALKER_GOAL_ROUTE2 ? 0 : i;
        s16 gx = px + MAP_OFFSET;
        s16 gy = py + MAP_OFFSET;
        enum Direction outward = incoming == WALKER_GOAL_ROUTE1 ? DIR_SOUTH
            : incoming == WALKER_GOAL_ROUTE2 ? DIR_NORTH : DIR_WEST;
        if (MapGridGetCollisionAt(gx, gy)
         || GetMapBorderIdAt(gx + sDx[outward], gy + sDy[outward]) == CONNECTION_INVALID
         || !CanWalkFrom(walker, gx, gy, outward)
         || !CanWalkFrom(walker, gx + sDx[outward], gy + sDy[outward], GetOppositeDirection(outward))
         || GetObjectObjectCollidesWith(walker, gx, gy, FALSE) != OBJECT_EVENTS_COUNT)
            continue;
        *x = gx;
        *y = gy;
        *inward = incoming == WALKER_GOAL_ROUTE1 ? DIR_NORTH
            : incoming == WALKER_GOAL_ROUTE2 ? DIR_SOUTH : DIR_EAST;
        return TRUE;
    }
    return FALSE;
}

static void Replan(struct ObjectEvent *walker)
{
    gViridianWalkerDebug.replans++;
    gViridianWalkerDebug.blockedSteps++;
    sInStep = FALSE;
    if (++sBlocked >= WALKER_BLOCKED_LIMIT)
        NextGoal();
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
        if (sEntryStep)
        {
            sEntryStep = FALSE;
            NextGoal();
            return;
        }
    }
    if (sRemaining == 0)
    {
        if (sGoal == WALKER_GOAL_GRASS)
        {
            CompleteGoal(walker);
            return;
        }
        if (sGoal < WALKER_GOAL_ROUTE1)
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
}

static void DiscoverGrass(void)
{
    s16 x, y;
    for (y = 0; y < WALKER_HEIGHT; y++)
        for (x = 0; x < WALKER_WIDTH; x++)
            if (MetatileBehavior_IsTallGrass(MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET)))
                gViridianWalkerDebug.grassTilesFound++;
    if (gViridianWalkerDebug.grassTilesFound == 0)
        gViridianWalkerDebug.usedGrassFallback = 1;
}

void ViridianWalker_Update(void)
{
    struct ObjectEvent *walker;
    if (!OnMap())
    {
        if (sWork != NULL)
        {
            Free(sWork);
            sWork = NULL;
        }
        sInitialized = FALSE;
        sRecoverAfterHeapReset = FALSE;
        sInStep = sExitStep = sDoorStep = sEntryStep = FALSE;
        sRemaining = sRead = sWrite = sWait = 0;
        sBlocked = 0;
        gViridianWalkerDebug.phase = WALKER_PHASE_INACTIVE;
        return;
    }
    walker = GetWalker();
    if (walker == NULL)
    {
        TrySpawnObjectEvent(VIRIDIAN_WALKER_LOCAL_ID,
            MAP_NUM(MAP_VIRIDIAN_CITY_HNS), MAP_GROUP(MAP_VIRIDIAN_CITY_HNS));
        walker = GetWalker();
    }
    if (walker == NULL)
        return;
    if (sRecoverAfterHeapReset && !RecoverWalker(walker))
        return;
    if (!sInitialized)
    {
        sWork = Alloc(sizeof(*sWork));
        if (sWork == NULL)
            return;
        memset(&gViridianWalkerDebug, 0, sizeof(gViridianWalkerDebug));
        DiscoverGrass();
        sInStep = sExitStep = sDoorStep = sEntryStep = FALSE;
        sRemaining = sRead = sWrite = sWait = 0;
        sBlocked = 0;
        sGoal = WALKER_GOAL_GRASS;
        VarSet(VAR_TEMP_2, sGoal);
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
            NextGoal();
        break;
    case WALKER_PHASE_HIDDEN:
        if (sWait != 0)
            sWait--;
        else
        {
            if (sGoal < WALKER_GOAL_ROUTE1)
            {
                s16 x = sDoorX;
                s16 y = sDoorY + 1;
                if (GetObjectObjectCollidesWith(walker, x, y, FALSE) != OBJECT_EVENTS_COUNT)
                    break;
                MoveObjectEventToMapCoords(walker, x, y);
                walker->currentElevation = walker->previousElevation = MapGridGetElevationAt(x, y);
                walker->invisible = FALSE;
                NextGoal();
            }
            else
            {
                s16 x, y;
                enum Direction inward;
                if (!FindEntrance(walker, sGoal, &x, &y, &inward)
                 || GetObjectObjectCollidesWith(walker, x, y, FALSE) != OBJECT_EVENTS_COUNT)
                    break;
                MoveObjectEventToMapCoords(walker, x - sDx[inward], y - sDy[inward]);
                walker->currentElevation = walker->previousElevation = MapGridGetElevationAt(x, y);
                walker->invisible = FALSE;
                if (ObjectEventSetHeldMovement(walker, GetWalkNormalMovementAction(inward)))
                    break;
                gViridianWalkerDebug.nextX = x;
                gViridianWalkerDebug.nextY = y;
                sEntryStep = sInStep = TRUE;
                gViridianWalkerDebug.phase = WALKER_PHASE_WALK;
            }
        }
        break;
    }
}

#endif
