// Local actors for the notable world simulation (stage 3): the walkers on
// the player's map. They spawn from the saved world records after a map
// load, walk tile by tile with the engine's collision checks, report every
// exit and arrival back to the record, play the spots' behaviour templates,
// and stay visible in a connection strip until they walk out of view.
// Spec: .product/specs/notable-world-simulation.md and notable-spots.md.
// Lessons from the travel proof of concept (branch task/viridian-walker-poc):
// a bounded grid search at eight expansions per field update, re-planning
// when blocked, the door-step exception, the seam rebase, and dropping the
// search workspace on heap resets.
//
// The grid search follows the engine's own step rules, the same ones the
// walker graph generator (tools/wayfarer_world/graph.py) floods with:
// GetCollisionAtCoords (with its sideways-stair diagonal steps), ledges and
// water as walls, and the elevation an object settles on after a step
// (ObjectEventUpdateElevation: the tile's, unless the tile is 15).
//
// A walker never traps the player: pushed against, or blocked by the
// player, it backs off to an open tile away from them, and after a few
// tries (or with nowhere to go) it hands off to its record and leaves for
// the rest of the visit.

#include "global.h"
#include "wayfarer_walkers.h"

#if IS_WAYFARER
#include "event_data.h"
#include "event_object_movement.h"
#include "field_effect.h"
#include "fieldmap.h"
#include "malloc.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "script.h"
#include "wayfarer_world.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/field_effects.h"

#define WALKER_MAX_TILES        7320    // ROUTE47_HNS, the largest in-scope layout
#define WALKER_QUEUE_SIZE       1024    // ring queue; a grid frontier stays far below
#define WALKER_LAYER_MAX        64      // second-elevation states on elevation-15 tiles
#define WALKER_PATH_MAX         192     // steps per plan; longer walks re-plan on the way
#define WALKER_SEARCH_SLICE     8       // node expansions per field update, at most
#define WALKER_SLICE_SCANLINES  48      // and no more once a slice has run this long:
                                        // the overworld's own frame must still fit
#define WALKER_AWAY_NODES       160     // a back-off search gives up after this many
#define WALKER_BLOCKED_LIMIT    4       // blocked steps before the goal is dropped
#define WALKER_RETRY_FRAMES     30
#define WALKER_UNREACHABLE_WAIT 240     // stand still (or play the template), then retry
#define WALKER_FAILS_MAX        3       // the retry wait doubles up to this many failures
#define WALKER_SPAWN_PERIOD     8
#define WALKER_SPAWN_BACKOFF    64      // frames before retrying a spawn that found no room
#define WALKER_NEAR_RADIUS      3
#define WALKER_LANE_SCAN        10
#define WALKER_YIELD_FRAMES     40      // the player pushing against a walker this long
#define WALKER_YIELD_BLOCKS     2       // steps blocked by the player before backing off
#define WALKER_BACKOFFS_MAX     3       // back-offs per visit before handing off
#define WALKER_BACKOFF_WAIT     180
#define WALKER_AWAY_DISTANCE    3
#define WALKER_SPAWN_CLEARANCE  2       // never spawn this close to the player
#define WALKER_SPAWN_AHEAD      4       // nor this far straight ahead of them
#define WANDER_PAUSE_TICKS      3
#define BROWSE_INTERVAL_TICKS   8
#define BROWSE_FLOOR_EVERY      3

#define EMOTE_ELLIPSIS_FRAME 5     // FOLLOWER_EMOTION_PENSIVE: the "..." bubble
#define NO_ACTOR    0xFF
#define NO_EDGE     0xFFFF
#define SLOT_NONE   0xFF
#define TILE_UNSEEN 0xFF
#define STATE_LAYER 0x8000
#define LAYER_NONE  0xFF

enum
{
    STEP_NONE,
    STEP_WALK,      // a path step: check where it landed
    STEP_EXIT_EDGE, // out through a map side: becomes a strip actor
    STEP_EXIT_WARP, // into a warp: removed afterwards
    STEP_STRIP,     // a strip actor's step
    STEP_ENTER,     // a strip actor stepping into the player's map
    STEP_SETTLE,    // a step carried across a map change or a menu
};

enum
{
    TRANSITION_NONE,
    TRANSITION_CAMERA,
    TRANSITION_WARP,
};

struct WalkerActor
{
    u8 mode;
    u8 slot;
    u8 objectId;
    u8 phase;
    u8 goalKind;
    u8 goalDir;         // edge: the side's direction
    u8 goalX;           // tile goal, warp tile, or the player's tile for a back-off
    u8 goalY;
    u16 goalEdge;
    u8 laneA;
    u8 laneB;
    u8 blocked;
    u8 stepKind;
    s16 nextX;          // where the current step lands (MAP_OFFSET coords)
    s16 nextY;
    u8 pathLen;
    u8 pathPos;
    bool8 truncated;
    u8 template;
    u8 emote;
    bool8 atSpot;       // the stay started: dwell and the template run
    u8 stripDir;        // strip actors: the direction they walk
    bool8 stripToward;  // walking into the player's map
    u16 wait;
    u8 frames;          // frames into the current dwell tick
    u8 dwellTicks;      // dwell ticks towards one heartbeat of local dwell
    u16 t;              // template tick counter
    u16 k;              // template move counter
    u8 pauseTicks;
    bool8 actionPending;
    bool8 justLeaving;
    u8 failures;        // consecutive failed searches
    u8 playerBlocks;    // steps blocked by the player
    u8 backOffs;        // back-offs this visit
    u8 pushFrames;      // frames the player has pushed against this walker
    u8 padding;
};

// Shared heap workspace for the one grid search that runs at a time.
// tiles[]: TILE_UNSEEN, or the move into the tile (DIR_SOUTH..DIR_NORTHEAST,
// low nibble) and the elevation the walker settles on there (high nibble).
struct WalkerWork
{
    u8 tiles[WALKER_MAX_TILES];
    // An elevation-15 tile (a bridge) carries the walker's elevation, so it
    // can be reached in two states (over and under): the second one lives
    // here. Queue entries and states are a tile index, or STATE_LAYER | an
    // index into this table.
    u16 layerTile[WALKER_LAYER_MAX];
    u8 layerValue[WALKER_LAYER_MAX];    // as tiles[]
    u8 layerCount;
    // Whether each state was reached from a second state: bits by tile for
    // tiles[] (written with the tile, so never cleared), bytes for layers.
    u8 fromLayer[(WALKER_MAX_TILES + 7) / 8];
    bool8 layerFromLayer[WALKER_LAYER_MAX];
    u16 queue[WALKER_QUEUE_SIZE];
    u8 path[WALKER_ACTOR_COUNT][WALKER_PATH_MAX];
};

struct WalkerSearch
{
    u8 actor;
    u16 read;
    u16 write;
    u16 source;
    u16 expanded;
    u16 startFrame;
};

// How an actor just handed off into a map (RAM only). Reaching the
// destination node makes the record Dwelling and clears its arrival, so
// without this a trainer followed through a door would spawn at their spot
// instead of walking in.
struct RecentEntry
{
    u16 map;
    u8 slot;
    u8 arrival;
    u8 crossing;
    u8 padding;
};

struct PendingActor
{
    u8 slot;
    u8 x;
    u8 y;
    u8 facing;
};

EWRAM_DATA struct WayfarerWalkersDebug gWayfarerWalkersDebug = {0};
static EWRAM_DATA struct WalkerActor sActors[WALKER_ACTOR_COUNT] = {0};
static EWRAM_DATA struct WalkerWork *sWork = NULL;
static EWRAM_DATA struct WalkerSearch sSearch = {0};
static EWRAM_DATA struct PendingActor sPending[WORLD_LOCAL_ACTOR_COUNT] = {0};
static EWRAM_DATA u8 sPendingCount = 0;
static EWRAM_DATA bool8 sRestorePending = FALSE;
static EWRAM_DATA bool8 sAdoptChecked = FALSE;
static EWRAM_DATA u16 sActiveMap = 0;
static EWRAM_DATA bool8 sActiveMapValid = FALSE;
static EWRAM_DATA u8 sTransition = TRANSITION_NONE;
static EWRAM_DATA u16 sMapNode = 0;  // set on every map change
static EWRAM_DATA u32 sVisitorsSeen = 0;    // Gym visitors spawned during this entry
static EWRAM_DATA u32 sYielded = 0;         // handed off for the rest of this visit
static EWRAM_DATA bool8 sHideFollower = FALSE;
static EWRAM_DATA bool8 sFollowerFlagOurs = FALSE;
static EWRAM_DATA bool8 sFollowerRemoved = FALSE;   // survives map changes: a seam keeps it removed
static EWRAM_DATA u8 sSpawnTimer = 0;
static EWRAM_DATA u8 sSpawnBackoff = 0;
static EWRAM_DATA struct RecentEntry sRecentEntries[WALKER_ACTOR_COUNT] = {0};

STATIC_ASSERT(sizeof(struct WayfarerWalkerActorDebug) == 16, WalkerActorDebugSize);
STATIC_ASSERT(sizeof(struct WayfarerWalkersDebug) == 64 + 16 * WALKER_ACTOR_COUNT + 32, WalkersDebugSize);
STATIC_ASSERT(DIR_NORTHEAST <= 15, WalkerMoveFitsANibble);
STATIC_ASSERT(WALKER_MAX_TILES < STATE_LAYER, WalkerTileFitsAState);

// DIR_SOUTH..DIR_EAST are 1..4; the stair diagonals DIR_SOUTHWEST..DIR_NORTHEAST 5..8.
static const s8 sDx[9] = {0, 0, 0, -1, 1, -1, 1, -1, 1};
static const s8 sDy[9] = {0, 1, -1, 0, 0, 1, 1, -1, -1};
// The walking command for each move: a diagonal is a west or east step on stairs.
static const u8 sMoveCommand[9] = {DIR_NONE, DIR_SOUTH, DIR_NORTH, DIR_WEST, DIR_EAST, DIR_WEST, DIR_EAST, DIR_WEST, DIR_EAST};
static const u8 sOpposite[5] = {DIR_NONE, DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST};

static void RequestSearch(struct WalkerActor *actor);
static void ChooseGoal(struct WalkerActor *actor, struct ObjectEvent *obj);

// ---------------------------------------------------------------------------
// Small helpers

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

// Scanlines since a stamp. The VBlank interrupt that counts frames can run a
// few lines after VCOUNT reaches 160, so a stamp taken in between reads one
// frame early: a negative span counts as 0.
static u32 ScanlinesSince(u32 start)
{
    s32 span = ScanlineStamp() - start;
    return span < 0 ? 0 : span;
}

u32 WayfarerWalkers_ScanlineStamp(void)
{
    return ScanlineStamp();
}

u32 WayfarerWalkers_ScanlinesSince(u32 start)
{
    return ScanlinesSince(start);
}

void WayfarerWalkers_NoteHeartbeat(u32 scanlines, u32 contextScanlines)
{
    gWayfarerWalkersDebug.lastContextScanlines = contextScanlines;
    gWayfarerWalkersDebug.lastHeartbeatScanlines = scanlines;
    if (scanlines > gWayfarerWalkersDebug.maxHeartbeatScanlines)
        gWayfarerWalkersDebug.maxHeartbeatScanlines = scanlines;
}

static u16 CurrentMap(void)
{
    return (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
}

static struct WayfarerWorldState *State(void)
{
    return WayfarerWorld_GetState();
}

static struct WayfarerWorldRecord *Record(const struct WalkerActor *actor)
{
    return &State()->records[actor->slot];
}

static const struct WayfarerWorldTrainer *Trainer(u8 slot)
{
    return &gWayfarerWorldTrainers[slot];
}

static u8 CatalogIndex(u8 slot)
{
    return Trainer(slot)->characterId - 1;  // c in the spots spec
}

static s16 MapWidth(void)
{
    return gMapHeader.mapLayout->width;
}

static s16 MapHeight(void)
{
    return gMapHeader.mapLayout->height;
}

static bool8 InMap(s16 x, s16 y)
{
    return x >= 0 && y >= 0 && x < MapWidth() && y < MapHeight();
}

static u8 ActorIndex(const struct WalkerActor *actor)
{
    return actor - sActors;
}

static struct ObjectEvent *ActorObject(const struct WalkerActor *actor)
{
    return &gObjectEvents[actor->objectId];
}

static struct ObjectEvent *Player(void)
{
    return &gObjectEvents[gPlayerAvatar.objectEventId];
}

static u16 Distance(s16 x1, s16 y1, s16 x2, s16 y2)
{
    return (x1 > x2 ? x1 - x2 : x2 - x1) + (y1 > y2 ? y1 - y2 : y2 - y1);
}

// MAP_OFFSET coords.
static u16 PlayerDistance(s16 x, s16 y)
{
    return Distance(x, y, Player()->currentCoords.x, Player()->currentCoords.y);
}

static bool8 IsLeaderHomeInGym(u8 slot)
{
    const struct WayfarerWorldRecord *record = &State()->records[slot];
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);
    return (trainer->flags & WORLD_TRAINER_FLAG_GYM_LEADER) && record->destKind == WORLD_DEST_HOME
        && record->node == trainer->gymNode;
}

// A Gym's own leader object: shown while home-locked or dwelling at home in
// the Gym; while away or pinned, shown only while still unbeaten.
static bool8 IsLeaderObjectShown(u8 slot)
{
    const struct WayfarerWorldRecord *record = &State()->records[slot];
    if (record->state == WORLD_STATE_HOME_LOCKED)
        return TRUE;
    if (WorldSim_IsSimulated(record))
        return record->state == WORLD_STATE_DWELLING && IsLeaderHomeInGym(slot);
    return WayfarerWorld_IsLeaderUnbeaten(slot);
}

static bool8 IsTrainerSprite(u8 slot, u16 gfx)
{
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);
    return gfx == trainer->graphicsId
        || (trainer->altGraphicsIds[0] != 0 && gfx == trainer->altGraphicsIds[0])
        || (trainer->altGraphicsIds[1] != 0 && gfx == trainer->altGraphicsIds[1]);
}

static bool8 IsPlayerTile(s16 x, s16 y)
{
    struct ObjectEvent *player = Player();
    return (player->currentCoords.x == x && player->currentCoords.y == y)
        || (player->previousCoords.x == x && player->previousCoords.y == y);
}

// Ledges and water are walls for walkers (the generator's rule too).
static bool8 IsBarrier(u8 behavior)
{
    return MetatileBehavior_IsSurfableWaterOrUnderwater(behavior)
        || MetatileBehavior_IsJumpNorth(behavior) || MetatileBehavior_IsJumpSouth(behavior)
        || MetatileBehavior_IsJumpEast(behavior) || MetatileBehavior_IsJumpWest(behavior);
}

// A tile a walker may stand on (MAP_OFFSET coords).
static bool8 IsStandable(s16 x, s16 y)
{
    if (!InMap(x - MAP_OFFSET, y - MAP_OFFSET) || MapGridGetCollisionAt(x, y) != 0)
        return FALSE;
    return !IsBarrier(MapGridGetMetatileBehaviorAt(x, y));
}

static bool8 IsFreeTile(s16 x, s16 y)
{
    return IsStandable(x, y) && GetObjectEventIdByXY(x, y) == OBJECT_EVENTS_COUNT && !IsPlayerTile(x, y);
}

// Where a walker may appear: free, not beside the player and not straight
// ahead of them, so a spawn never stands in the player's way.
static bool8 IsSpawnTile(s16 x, s16 y)
{
    struct ObjectEvent *player = Player();
    s16 px = player->currentCoords.x, py = player->currentCoords.y;
    u8 facing = player->facingDirection;
    u8 i;

    if (!IsFreeTile(x, y) || Distance(x, y, px, py) < WALKER_SPAWN_CLEARANCE)
        return FALSE;
    if (facing >= DIR_SOUTH && facing <= DIR_EAST)
    {
        for (i = 1; i <= WALKER_SPAWN_AHEAD; i++)
        {
            if (x == px + sDx[facing] * i && y == py + sDy[facing] * i)
                return FALSE;
        }
    }
    return TRUE;
}

// A step onto or off these may be a diagonal (GetSidewaysStairsCollision).
static bool8 IsSidewaysStairs(u8 behavior)
{
    return MetatileBehavior_IsSidewaysStairsLeftSideAny(behavior) || MetatileBehavior_IsSidewaysStairsRightSideAny(behavior);
}

// The elevation an object settles on at a tile (ObjectEventUpdateElevation
// once a step is over: the tile's own, except on elevation 15).
static u8 RestElevation(s16 x, s16 y, u8 carried)
{
    u8 elevation = MapGridGetElevationAt(x, y);
    return elevation == 15 ? carried : elevation;
}

// One step from (x, y) (MAP_OFFSET coords) for an object at the given
// elevation, by the engine's GetCollisionAtCoords: the move (DIR_SOUTH..
// DIR_EAST, or a stair diagonal DIR_SOUTHWEST..DIR_NORTHEAST), or DIR_NONE.
// The object's own fields are put back as they were.
static u8 ProbeStep(struct ObjectEvent *obj, s16 x, s16 y, u8 elevation, u8 dir)
{
    struct Coords16 coords = obj->currentCoords;
    u8 savedElevation = obj->currentElevation;
    u8 behavior = obj->currentMetatileBehavior;
    u8 overwrite = obj->directionOverwrite;
    s16 nx = x + sDx[dir], ny = y + sDy[dir];
    u8 move = DIR_NONE;

    if (IsBarrier(MapGridGetMetatileBehaviorAt(nx, ny)))
        return DIR_NONE;
    obj->currentCoords.x = x;
    obj->currentCoords.y = y;
    obj->currentElevation = elevation;
    obj->currentMetatileBehavior = MapGridGetMetatileBehaviorAt(x, y);
    if (GetCollisionAtCoords(obj, nx, ny, dir) == COLLISION_NONE)
        move = obj->directionOverwrite != DIR_NONE ? obj->directionOverwrite : dir;
    obj->currentCoords = coords;
    obj->currentElevation = savedElevation;
    obj->currentMetatileBehavior = behavior;
    obj->directionOverwrite = overwrite;
    if (move > DIR_EAST)
    {
        // The stair diagonal lands beside the step: it must be walkable too.
        s16 dx = x + sDx[move], dy = y + sDy[move];
        if (move > DIR_NORTHEAST || MapGridGetCollisionAt(dx, dy) != 0 || IsBarrier(MapGridGetMetatileBehaviorAt(dx, dy)))
            move = DIR_NONE;
    }
    return move;
}

// A step from where the object stands now.
static u8 ProbeFromHere(struct ObjectEvent *obj, u8 dir)
{
    return ProbeStep(obj, obj->currentCoords.x, obj->currentCoords.y, obj->currentElevation, dir);
}

static bool8 IsVisible(const struct ObjectEvent *obj)
{
    s16 x = obj->currentCoords.x, y = obj->currentCoords.y;
    s16 px = gSaveBlock1Ptr->pos.x, py = gSaveBlock1Ptr->pos.y;
    return x >= px - 1 && x <= px + MAP_OFFSET * 2 + 1 && y >= py + 1 && y <= py + MAP_OFFSET * 2 - 1;
}

static u8 DirectionTowards(s16 x, s16 y, s16 tx, s16 ty)
{
    if (ty > y) return DIR_SOUTH;
    if (ty < y) return DIR_NORTH;
    if (tx < x) return DIR_WEST;
    if (tx > x) return DIR_EAST;
    return DIR_NONE;
}

static u8 EdgeDirection(u8 kind)
{
    switch (kind)
    {
    case WORLD_EDGE_NORTH: return DIR_NORTH;
    case WORLD_EDGE_SOUTH: return DIR_SOUTH;
    case WORLD_EDGE_EAST:  return DIR_EAST;
    case WORLD_EDGE_WEST:  return DIR_WEST;
    }
    return DIR_NONE;
}

static u8 EdgeKindForDirection(u8 dir)
{
    switch (dir)
    {
    case DIR_NORTH: return WORLD_EDGE_NORTH;
    case DIR_SOUTH: return WORLD_EDGE_SOUTH;
    case DIR_EAST:  return WORLD_EDGE_EAST;
    case DIR_WEST:  return WORLD_EDGE_WEST;
    }
    return WORLD_EDGE_NONE;
}

static u16 FirstNodeOfMap(u16 map)
{
    u16 node;
    for (node = 0; node < gWayfarerWorldNodeCount; node++)
    {
        if (gWayfarerWorldNodes[node].map == map)
            return node;
    }
    return WORLD_NODE_NONE;
}

// Spots follow map order and each map's nodes are contiguous, so the first
// spot of a map is a lower bound on the node index.
static u16 FirstSpotFromNode(u16 node)
{
    u16 low = 0, high = gWayfarerWorldSpotCount;
    while (low < high)
    {
        u16 mid = (low + high) / 2;
        if (gWayfarerWorldSpots[mid].node < node)
            low = mid + 1;
        else
            high = mid;
    }
    return low;
}

static void NoteEntry(u8 slot, u16 edgeIndex, u8 crossing)
{
    const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[edgeIndex];
    u8 i, free = 0;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sRecentEntries[i].slot == slot || sRecentEntries[i].map == 0)
        {
            free = i;
            break;
        }
    }
    if (i == WALKER_ACTOR_COUNT)
    {
        // Full: drop the oldest.
        memmove(&sRecentEntries[0], &sRecentEntries[1], sizeof(sRecentEntries[0]) * (WALKER_ACTOR_COUNT - 1));
        free = WALKER_ACTOR_COUNT - 1;
    }
    sRecentEntries[free].map = WorldSim_NodeMap(edge->target) + 1;  // 0 marks unused
    sRecentEntries[free].slot = slot;
    sRecentEntries[free].arrival = WorldSim_ArrivalForEdge(edge->kind);
    sRecentEntries[free].crossing = crossing;
}

static const struct RecentEntry *FindEntry(u8 slot, u16 map)
{
    u8 i;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sRecentEntries[i].map == map + 1 && sRecentEntries[i].slot == slot)
            return &sRecentEntries[i];
    }
    return NULL;
}

static void ForgetEntry(u8 slot)
{
    u8 i;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sRecentEntries[i].slot == slot)
            sRecentEntries[i].map = 0;
    }
}

// A map load keeps only the entries into the new map: a trainer who walked
// in just ahead of the player is found walking in.
static void KeepEntriesFor(u16 map)
{
    u8 i;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sRecentEntries[i].map != map + 1)
            sRecentEntries[i].map = 0;
    }
}

static void BuildContext(struct WayfarerWorldContext *ctx)
{
    WayfarerWorld_BuildContext(ctx);
}

// A step under way when the field is torn down (a menu, a battle, a save)
// restarts from its first frame when the sprite is rebuilt and shifts the
// object once more, with no collision check. The object already stands on
// the step's destination tile: end the movement there.
static void SettleHeldMovement(struct ObjectEvent *obj)
{
    obj->movementActionId = MOVEMENT_ACTION_NONE;
    obj->heldMovementActive = FALSE;
    obj->heldMovementFinished = FALSE;
    obj->directionOverwrite = DIR_NONE;
    // Collision also counts the step's start tile (previousCoords) until
    // the next step: let go of it. (The engine settles the elevation from
    // both tiles on the object's next frame; no map grid is read here, as
    // Continue can run this before the map is loaded.)
    ShiftStillObjectEventCoords(obj);
}

// ---------------------------------------------------------------------------
// Actor lifetime

static struct WalkerActor *FindActorForSlot(u8 slot)
{
    u8 i;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sActors[i].mode != WALKER_MODE_NONE && sActors[i].slot == slot)
            return &sActors[i];
    }
    return NULL;
}

static u8 CountActors(u8 mode)
{
    u8 i, count = 0;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sActors[i].mode == mode)
            count++;
    }
    return count;
}

static void AbortSearchFor(struct WalkerActor *actor)
{
    if (sSearch.actor == ActorIndex(actor))
        sSearch.actor = NO_ACTOR;
}

static void ForgetActor(struct WalkerActor *actor)
{
    AbortSearchFor(actor);
    memset(actor, 0, sizeof(*actor));
    actor->slot = SLOT_NONE;
}

static void RemoveActor(struct WalkerActor *actor)
{
    struct ObjectEvent *obj = ActorObject(actor);
    if (obj->active && obj->localId == WALKER_LOCALID_BASE + ActorIndex(actor))
        RemoveObjectEvent(obj);
    gWayfarerWalkersDebug.removals++;
    ForgetActor(actor);
}

static void ForgetAllActors(void)
{
    u8 i;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
        ForgetActor(&sActors[i]);
    sSearch.actor = NO_ACTOR;
}

static void InitActor(struct WalkerActor *actor, u8 slot, u8 objectId)
{
    memset(actor, 0, sizeof(*actor));
    actor->mode = WALKER_MODE_LOCAL;
    actor->slot = slot;
    actor->objectId = objectId;
    actor->phase = WALKER_PHASE_PLAN;
    actor->goalEdge = NO_EDGE;
}

static u8 SpawnElevation(s16 x, s16 y)
{
    u8 elevation = MapGridGetElevationAt(x, y);
    if (elevation == 15)
        elevation = Player()->currentElevation;
    return elevation;
}

// x, y: map tile without MAP_OFFSET.
static struct WalkerActor *SpawnActor(u8 slot, s16 x, s16 y, u8 facing)
{
    struct WalkerActor *actor = NULL;
    struct ObjectEvent *obj;
    u8 i, objectId;

    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sActors[i].mode == WALKER_MODE_NONE)
        {
            actor = &sActors[i];
            break;
        }
    }
    if (actor == NULL)
        return NULL;
    objectId = SpawnSpecialObjectEventParameterized(Trainer(slot)->graphicsId, MOVEMENT_TYPE_NONE,
                                                    WALKER_LOCALID_BASE + i, x + MAP_OFFSET, y + MAP_OFFSET,
                                                    SpawnElevation(x + MAP_OFFSET, y + MAP_OFFSET));
    if (objectId >= OBJECT_EVENTS_COUNT)
        return NULL;
    InitActor(actor, slot, objectId);
    obj = &gObjectEvents[objectId];
    if (facing >= DIR_SOUTH && facing <= DIR_EAST)
        ObjectEventTurn(obj, facing);
    gWayfarerWalkersDebug.spawns++;
    return actor;
}

// ---------------------------------------------------------------------------
// Spawn positions (world to local)

// The first spawn tile within the radius, nearest first, in a fixed order:
// south rows first (doors open south), then west before east.
static bool8 NearestSpawnTile(s16 cx, s16 cy, u8 minDistance, u8 radius, s16 *outX, s16 *outY)
{
    s16 d, dy, side;
    for (d = minDistance; d <= radius; d++)
    {
        for (dy = d; dy >= -d; dy--)
        {
            s16 rest = d - (dy < 0 ? -dy : dy);
            for (side = 0; side < (rest == 0 ? 1 : 2); side++)
            {
                s16 x = cx + (side == 0 ? -rest : rest), y = cy + dy;
                if (IsSpawnTile(x + MAP_OFFSET, y + MAP_OFFSET))
                {
                    *outX = x;
                    *outY = y;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

// A lane tile on the entered side, at the crossing or the nearest spawn tile.
static bool8 LaneTile(u8 arrival, u8 crossing, s16 *outX, s16 *outY, u8 *facing)
{
    s16 i;
    for (i = 0; i <= WALKER_LANE_SCAN; i++)
    {
        s16 along = crossing + ((i & 1) ? -((i + 1) / 2) : (i / 2));
        s16 x, y;
        switch (arrival)
        {
        case WORLD_ARRIVAL_NORTH: x = along; y = 0; *facing = DIR_SOUTH; break;
        case WORLD_ARRIVAL_SOUTH: x = along; y = MapHeight() - 1; *facing = DIR_NORTH; break;
        case WORLD_ARRIVAL_WEST:  x = 0; y = along; *facing = DIR_EAST; break;
        default:                  x = MapWidth() - 1; y = along; *facing = DIR_WEST; break;
        }
        if (IsSpawnTile(x + MAP_OFFSET, y + MAP_OFFSET))
        {
            *outX = x;
            *outY = y;
            return TRUE;
        }
    }
    return FALSE;
}

static const struct WayfarerWorldSpot *DestSpot(const struct WayfarerWorldRecord *record, u8 slot)
{
    if (record->destKind == WORLD_DEST_SPOT
     || (record->destKind == WORLD_DEST_HOME && !(Trainer(slot)->flags & WORLD_TRAINER_FLAG_GYM_LEADER)))
    {
        if (record->destId < gWayfarerWorldSpotCount)
            return &gWayfarerWorldSpots[record->destId];
    }
    return NULL;
}

static bool8 IsGymVisit(const struct WayfarerWorldRecord *record, u8 slot)
{
    const struct WayfarerWorldSpot *spot = DestSpot(record, slot);
    return spot != NULL && spot->kind == WORLD_SPOT_GYM && spot->node == record->node;
}

// Gym "just leaving": an approach tile, nearest the exit first, never on or
// beside the player's arrival tile nor ahead of them. A 1-wide entrance has
// no such tile: the visitor isn't shown this entry.
static bool8 VisitorTile(const struct WayfarerWorldSpot *spot, s16 *outX, s16 *outY, u8 *facing)
{
    u16 i;
    for (i = 0; i < spot->dataCount; i++)
    {
        const u8 *tile = gWayfarerWorldAreaTiles[spot->dataStart + i];
        if (IsSpawnTile(tile[0] + MAP_OFFSET, tile[1] + MAP_OFFSET))
        {
            *outX = tile[0];
            *outY = tile[1];
            *facing = DirectionTowards(tile[0], tile[1], spot->x, spot->y);
            return TRUE;
        }
    }
    return FALSE;
}

// The first node of a node's map (each map's nodes are contiguous).
static u16 MapFirstNode(u16 node)
{
    while (node > 0 && gWayfarerWorldNodes[node - 1].map == gWayfarerWorldNodes[node].map)
        node--;
    return node;
}

// The first non-Gym spot of this node: somewhere a trainer plausibly was.
static u16 FirstSpotOfNode(u16 node)
{
    u16 map = WorldSim_NodeMap(node), spot;
    for (spot = FirstSpotFromNode(MapFirstNode(node));
         spot < gWayfarerWorldSpotCount && WorldSim_NodeMap(gWayfarerWorldSpots[spot].node) == map; spot++)
    {
        if (gWayfarerWorldSpots[spot].node == node && gWayfarerWorldSpots[spot].kind != WORLD_SPOT_GYM)
            return spot;
    }
    return WORLD_SPOT_NONE;
}

static bool8 SpawnTile(u8 slot, s16 *x, s16 *y, u8 *facing)
{
    const struct WayfarerWorldRecord *record = &State()->records[slot];
    const struct WayfarerWorldSpot *spot = DestSpot(record, slot);
    const struct WayfarerWorldNode *node = &gWayfarerWorldNodes[record->node];
    u8 arrival = record->arrival, crossing = record->crossing;
    const struct RecentEntry *entry = FindEntry(slot, CurrentMap());

    *facing = DIR_SOUTH;
    if (entry != NULL && record->state == WORLD_STATE_DWELLING && !IsGymVisit(record, slot))
    {
        // Just arrived by walking in: enter as a traveller would.
        arrival = entry->arrival;
        crossing = entry->crossing;
    }
    else if (record->state == WORLD_STATE_DWELLING)
    {
        if (spot != NULL && spot->node == record->node)
        {
            if (spot->kind == WORLD_SPOT_GYM)
                return VisitorTile(spot, x, y, facing);
            if (spot->facing >= DIR_SOUTH && spot->facing <= DIR_EAST)
                *facing = spot->facing;
            return NearestSpawnTile(spot->x, spot->y, 0, WALKER_NEAR_RADIUS, x, y);
        }
        return NearestSpawnTile(node->x, node->y, 0, WALKER_NEAR_RADIUS, x, y);
    }

    switch (arrival)
    {
    case WORLD_ARRIVAL_NORTH:
    case WORLD_ARRIVAL_SOUTH:
    case WORLD_ARRIVAL_EAST:
    case WORLD_ARRIVAL_WEST:
        return LaneTile(arrival, crossing, x, y, facing);
    case WORLD_ARRIVAL_DOOR:
    case WORLD_ARRIVAL_TRANSIT:
        if (crossing < gMapHeader.events->warpCount)
        {
            const struct WarpEvent *warp = &gMapHeader.events->warps[crossing];
            if (NearestSpawnTile(warp->x, warp->y, 1, WALKER_NEAR_RADIUS, x, y))
            {
                *facing = DirectionTowards(warp->x, warp->y, *x, *y);
                if (*facing == DIR_NONE)
                    *facing = DIR_SOUTH;
                return TRUE;
            }
            return FALSE;
        }
        break;
    }
    // Travelling without an arrival (their stay here just ended off-screen):
    // the first spot of this node, as somewhere they plausibly were.
    {
        u16 first = FirstSpotOfNode(record->node);
        if (first != WORLD_SPOT_NONE)
            return NearestSpawnTile(gWayfarerWorldSpots[first].x, gWayfarerWorldSpots[first].y, 0, WALKER_NEAR_RADIUS, x, y);
    }
    return NearestSpawnTile(node->x, node->y, 0, WALKER_NEAR_RADIUS, x, y);
}

// Story scenes win: a visible template object (or live object) drawn with
// any of the trainer's sprites on this map keeps the actor away. A Gym
// Leader's own object is matched by its local id, not here.
static bool8 IsStorySuppressed(u8 slot)
{
    const struct WayfarerWorldTrainer *trainer = Trainer(slot);
    bool8 ownGym = trainer->leaderLocalId != 0 && WorldSim_NodeMap(trainer->gymNode) == CurrentMap();
    u8 i;

    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        const struct ObjectEvent *obj = &gObjectEvents[i];
        if (obj->active && !obj->invisible && IsTrainerSprite(slot, obj->graphicsId) && !WayfarerWalkers_IsActorObject(obj)
         && !(ownGym && obj->localId == trainer->leaderLocalId))
            return TRUE;
    }
    if (gMapHeader.events != NULL)
    {
        for (i = 0; i < gMapHeader.events->objectEventCount && i < OBJECT_EVENT_TEMPLATES_COUNT; i++)
        {
            const struct ObjectEventTemplate *template = &gSaveBlock1Ptr->objectEventTemplates[i];
            if (IsTrainerSprite(slot, template->graphicsId) && !FlagGet(template->flagId)
             && !(ownGym && template->localId == trainer->leaderLocalId))
                return TRUE;
        }
    }
    return FALSE;
}

static bool8 IsSpawnCandidate(u8 slot)
{
    const struct WayfarerWorldRecord *record = &State()->records[slot];
    if (!WorldSim_IsSimulated(record) || WorldSim_NodeMap(record->node) != CurrentMap())
        return FALSE;
    if (FindActorForSlot(slot) != NULL || IsLeaderHomeInGym(slot) || (sYielded & (1u << slot)))
        return FALSE;
    if (IsGymVisit(record, slot) && (sVisitorsSeen & (1u << slot)))
        return FALSE;
    return TRUE;
}

static u8 FreeObjectSlots(void)
{
    u8 i, count = 0;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (!gObjectEvents[i].active)
            count++;
    }
    return count;
}

// Spawning leaves at least this many object slots free for the map's own
// objects (Cut trees, boulders, trainers) coming into view.
#define SPAWN_FREE_SLOTS 3

// Spawns the restored local actor block first (Continue), then records on
// this map in priority order, up to the map's capacity and the free slots.
static void TrySpawn(void)
{
    u8 order[WORLD_SIM_TRAINER_COUNT], hops[WORLD_SIM_TRAINER_COUNT];
    u8 count = 0, slot, i, j, room, capacity, locals;
    bool8 spawned = FALSE;

    if (sMapNode == WORLD_NODE_NONE)
        return;
    capacity = WorldSim_MapCapacity(sMapNode);

    if (sRestorePending)
    {
        for (i = 0; i < sPendingCount; i++)
        {
            struct PendingActor *pending = &sPending[i];
            const struct WayfarerWorldRecord *record = &State()->records[pending->slot];
            s16 x = pending->x, y = pending->y;
            if (FindActorForSlot(pending->slot) != NULL || !WorldSim_IsSimulated(record)
             || WorldSim_NodeMap(record->node) != CurrentMap() || FreeObjectSlots() < SPAWN_FREE_SLOTS)
                continue;
            // The saved tile, unless something stands there now (a warped Continue).
            if (!IsFreeTile(x + MAP_OFFSET, y + MAP_OFFSET) && !NearestSpawnTile(pending->x, pending->y, 1, WALKER_NEAR_RADIUS, &x, &y))
                continue;
            if (SpawnActor(pending->slot, x, y, pending->facing) != NULL)
                gWayfarerWalkersDebug.restores++;
        }
        sRestorePending = FALSE;
        sPendingCount = 0;
    }

    locals = CountActors(WALKER_MODE_LOCAL);
    if (locals >= capacity || FreeObjectSlots() < SPAWN_FREE_SLOTS)
        return;
    room = capacity - locals;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (IsSpawnCandidate(slot))
            order[count++] = slot;
    }
    if (count == 0)
        return;

    // Priority order (fewer hops home first, then catalog order) only
    // matters when there are more candidates than room.
    if (count > room)
    {
        // A table lookup: no search workspace.
        for (i = 0; i < count; i++)
            hops[order[i]] = WorldSim_HomeHops(State(), order[i]);
        for (i = 1; i < count; i++)
        {
            u8 value = order[i];
            for (j = i; j > 0 && hops[order[j - 1]] > hops[value]; j--)
                order[j] = order[j - 1];
            order[j] = value;
        }
    }

    for (i = 0; i < count && room > 0; i++)
    {
        s16 x, y;
        u8 facing;
        struct WalkerActor *actor;

        slot = order[i];
        if (FreeObjectSlots() < SPAWN_FREE_SLOTS)
            break;
        if (IsStorySuppressed(slot))
        {
            gWayfarerWalkersDebug.storySuppressed++;
            sYielded |= 1u << slot;
            continue;
        }
        if (!SpawnTile(slot, &x, &y, &facing))
        {
            if (IsGymVisit(&State()->records[slot], slot))
                sVisitorsSeen |= 1u << slot;  // no tile off the player's path: not this entry
            continue;
        }
        actor = SpawnActor(slot, x, y, facing);
        if (actor == NULL)
            break;
        spawned = TRUE;
        ForgetEntry(slot);
        if (IsGymVisit(Record(actor), slot))
        {
            actor->justLeaving = TRUE;
            sVisitorsSeen |= 1u << slot;
        }
        room--;
    }
    if (!spawned)
        sSpawnBackoff = WALKER_SPAWN_BACKOFF;
}

// On Continue without a warp the engine restores the saved objects itself;
// adopt the walkers among them instead of respawning (no flicker).
static void AdoptRestoredObjects(void)
{
    u8 i, p;

    sAdoptChecked = TRUE;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *obj = &gObjectEvents[i];
        u8 index;
        bool8 adopted = FALSE;

        if (!obj->active || obj->localId < WALKER_LOCALID_BASE || obj->localId >= WALKER_LOCALID_BASE + WALKER_ACTOR_COUNT)
            continue;
        index = obj->localId - WALKER_LOCALID_BASE;
        for (p = 0; p < sPendingCount && sRestorePending; p++)
        {
            struct PendingActor *pending = &sPending[p];
            if (pending->slot == SLOT_NONE || Trainer(pending->slot)->graphicsId != obj->graphicsId
             || obj->currentCoords.x != pending->x + MAP_OFFSET || obj->currentCoords.y != pending->y + MAP_OFFSET
             || sActors[index].mode != WALKER_MODE_NONE)
                continue;
            InitActor(&sActors[index], pending->slot, i);
            if (pending->facing >= DIR_SOUTH && pending->facing <= DIR_EAST)
                ObjectEventTurn(obj, pending->facing);
            pending->slot = SLOT_NONE;
            gWayfarerWalkersDebug.restores++;
            adopted = TRUE;
            break;
        }
        if (!adopted)
            RemoveObjectEvent(obj);
    }
    // Entries consumed by adoption are skipped by TrySpawn.
    for (p = 0; p < sPendingCount; p++)
    {
        if (sPending[p].slot == SLOT_NONE)
        {
            sPending[p] = sPending[sPendingCount - 1];
            sPendingCount--;
            p--;
        }
    }
}

// ---------------------------------------------------------------------------
// Shared grid search

static bool8 EnsureWorkspace(void)
{
    if (sWork == NULL)
        sWork = Alloc(sizeof(struct WalkerWork));
    gWayfarerWalkersDebug.workspaceBytes = sizeof(struct WalkerWork);
    return sWork != NULL;
}

// An open tile: three or more standable neighbours (not a corridor).
static bool8 IsOpenTile(s16 x, s16 y)
{
    u8 dir, open = 0;
    for (dir = DIR_SOUTH; dir <= DIR_EAST; dir++)
    {
        if (IsStandable(x + MAP_OFFSET + sDx[dir], y + MAP_OFFSET + sDy[dir]))
            open++;
    }
    return open >= 3;
}

// Is (x, y) (no offset) where the actor's goal is met? elevation: the
// walker's settled elevation there.
static bool8 IsGoalTile(struct WalkerActor *actor, struct ObjectEvent *obj, s16 x, s16 y, u8 elevation)
{
    switch (actor->goalKind)
    {
    case WALKER_GOAL_TILE:
        return x == actor->goalX && y == actor->goalY;
    case WALKER_GOAL_EDGE:
    {
        s16 along;
        switch (actor->goalDir)
        {
        case DIR_NORTH: if (y != 0) return FALSE; along = x; break;
        case DIR_SOUTH: if (y != MapHeight() - 1) return FALSE; along = x; break;
        case DIR_WEST:  if (x != 0) return FALSE; along = y; break;
        default:        if (x != MapWidth() - 1) return FALSE; along = y; break;
        }
        return along >= actor->laneA && along <= actor->laneB
            && ProbeStep(obj, x + MAP_OFFSET, y + MAP_OFFSET, elevation, actor->goalDir) == actor->goalDir;
    }
    case WALKER_GOAL_WARP:
    {
        u8 dir;
        if (Distance(x, y, actor->goalX, actor->goalY) != 1)
            return FALSE;
        // Door tiles carry the collision bit: the step in is the exception.
        if (MapGridGetCollisionAt(actor->goalX + MAP_OFFSET, actor->goalY + MAP_OFFSET) != 0)
            return TRUE;
        dir = DirectionTowards(x, y, actor->goalX, actor->goalY);
        return ProbeStep(obj, x + MAP_OFFSET, y + MAP_OFFSET, elevation, dir) == dir;
    }
    case WALKER_GOAL_AWAY:
        // Back off: an open tile well away from the player (their tile is goalX/Y).
        return Distance(x, y, actor->goalX, actor->goalY) >= WALKER_AWAY_DISTANCE && IsOpenTile(x, y);
    }
    return FALSE;
}

static u16 StateTile(u16 state)
{
    return (state & STATE_LAYER) ? sWork->layerTile[state & ~STATE_LAYER] : state;
}

static u8 StateValue(u16 state)
{
    return (state & STATE_LAYER) ? sWork->layerValue[state & ~STATE_LAYER] : sWork->tiles[state];
}

static u8 FindLayer(u16 tile)
{
    u8 i;
    for (i = 0; i < sWork->layerCount; i++)
    {
        if (sWork->layerTile[i] == tile)
            return i;
    }
    return LAYER_NONE;
}

// The state a path came from: the tile the move came from, in the state
// recorded when this one was reached.
static u16 PreviousState(u16 state)
{
    u16 tile = StateTile(state);
    u8 move = StateValue(state) & 0xF;
    u16 previous = tile - (sDx[move] + sDy[move] * MapWidth());
    bool8 fromLayer = (state & STATE_LAYER) ? sWork->layerFromLayer[state & ~STATE_LAYER]
                                            : (sWork->fromLayer[tile >> 3] >> (tile & 7)) & 1;
    return fromLayer ? (STATE_LAYER | FindLayer(previous)) : previous;
}

static void SetFromLayer(u16 tile, bool8 fromLayer)
{
    if (fromLayer)
        sWork->fromLayer[tile >> 3] |= 1 << (tile & 7);
    else
        sWork->fromLayer[tile >> 3] &= ~(1 << (tile & 7));
}

static void FinishSearch(struct WalkerActor *actor, u16 goal)
{
    u16 length = 0, state = goal, take, i;
    u8 *path = sWork->path[ActorIndex(actor)];
    u8 last[WALKER_PATH_MAX];
    u32 start = ScanlineStamp(), cost;

    // One walk back from the goal: the moves come out last first, so keep
    // the most recent WALKER_PATH_MAX of them (the path's first steps).
    while (state != sSearch.source && length < WALKER_MAX_TILES + WALKER_LAYER_MAX)
    {
        last[length % WALKER_PATH_MAX] = StateValue(state) & 0xF;
        state = PreviousState(state);
        length++;
    }
    take = length < WALKER_PATH_MAX ? length : WALKER_PATH_MAX;
    for (i = 0; i < take; i++)
        path[i] = last[(length - 1 - i) % WALKER_PATH_MAX];
    cost = ScanlinesSince(start);
    if (cost > gWayfarerWalkersDebug.maxFinishScanlines)
        gWayfarerWalkersDebug.maxFinishScanlines = cost;
    actor->pathLen = take;
    actor->pathPos = 0;
    actor->truncated = take < length;
    actor->phase = WALKER_PHASE_WALK;
    actor->failures = 0;
    sSearch.actor = NO_ACTOR;

    gWayfarerWalkersDebug.lastSearchNodes = sSearch.expanded;
    if (sSearch.expanded > gWayfarerWalkersDebug.maxSearchNodes)
        gWayfarerWalkersDebug.maxSearchNodes = sSearch.expanded;
    gWayfarerWalkersDebug.lastSearchFrames = gMain.vblankCounter1 - sSearch.startFrame + 1;
    if (gWayfarerWalkersDebug.lastSearchFrames > gWayfarerWalkersDebug.maxSearchFrames)
        gWayfarerWalkersDebug.maxSearchFrames = gWayfarerWalkersDebug.lastSearchFrames;
}

static void StartBackOff(struct WalkerActor *actor);
static void YieldVisit(struct WalkerActor *actor);
static void FinishLeaving(struct WalkerActor *actor);

static void FailSearch(struct WalkerActor *actor)
{
    struct ObjectEvent *obj = ActorObject(actor);
    u16 wait;

    sSearch.actor = NO_ACTOR;
    gWayfarerWalkersDebug.searchFails++;
    gWayfarerWalkersDebug.lastFailNodes = sSearch.expanded;
    if (actor->justLeaving)
    {
        // A Gym visitor that can't reach the exit (the player is in the
        // way) leaves anyway: it must never block the entrance.
        FinishLeaving(actor);
        return;
    }
    if (actor->goalKind == WALKER_GOAL_AWAY)
    {
        // Nowhere to back off to: hand off and leave for this visit.
        YieldVisit(actor);
        return;
    }
    if (PlayerDistance(obj->currentCoords.x, obj->currentCoords.y) <= WALKER_NEAR_RADIUS)
    {
        // Possibly cut off by the player: make room rather than wait.
        StartBackOff(actor);
        return;
    }
    // Unreachable for now: stand still (or keep playing the template), then
    // try again, less and less often. The record moves on at a heartbeat
    // once the player leaves.
    if (actor->failures < WALKER_FAILS_MAX)
        actor->failures++;
    wait = WALKER_UNREACHABLE_WAIT << actor->failures;
    actor->phase = actor->atSpot ? WALKER_PHASE_TEMPLATE : WALKER_PHASE_IDLE;
    actor->goalKind = actor->atSpot ? WALKER_GOAL_NONE : actor->goalKind;
    actor->wait = wait;
}

static void BeginSearch(struct WalkerActor *actor)
{
    struct ObjectEvent *obj = ActorObject(actor);
    s16 x = obj->currentCoords.x - MAP_OFFSET, y = obj->currentCoords.y - MAP_OFFSET;

    sSearch.actor = ActorIndex(actor);
    actor->phase = WALKER_PHASE_SEARCH;
    gWayfarerWalkersDebug.searches++;
    if (!InMap(x, y) || MapWidth() * MapHeight() > WALKER_MAX_TILES)
    {
        FailSearch(actor);
        return;
    }
    memset(sWork->tiles, TILE_UNSEEN, MapWidth() * MapHeight());
    sSearch.source = y * MapWidth() + x;
    sSearch.read = 0;
    sSearch.write = 1;
    sSearch.expanded = 0;
    sSearch.startFrame = gMain.vblankCounter1;
    sWork->queue[0] = sSearch.source;
    sWork->tiles[sSearch.source] = (obj->currentElevation & 0xF) << 4;
    sWork->layerCount = 0;
}

static void SearchSlice(void)
{
    struct WalkerActor *actor = &sActors[sSearch.actor];
    struct ObjectEvent *obj = ActorObject(actor);
    u16 width = MapWidth(), height = MapHeight();
    u32 start = ScanlineStamp(), elapsed;
    u8 expanded = 0;

    while (sSearch.read != sSearch.write && expanded < WALKER_SEARCH_SLICE
        && (expanded == 0 || ScanlinesSince(start) < WALKER_SLICE_SCANLINES))
    {
        u16 state = sWork->queue[sSearch.read % WALKER_QUEUE_SIZE], tile = StateTile(state);
        s16 x = tile % width, y = tile / width;
        u8 elevation = StateValue(state) >> 4;
        bool8 onStairs = IsSidewaysStairs(MapGridGetMetatileBehaviorAt(x + MAP_OFFSET, y + MAP_OFFSET));
        u8 dir;

        sSearch.read++;
        sSearch.expanded++;
        expanded++;
        if (IsGoalTile(actor, obj, x, y, elevation))
        {
            FinishSearch(actor, state);
            break;
        }
        for (dir = DIR_SOUTH; dir <= DIR_EAST; dir++)
        {
            s16 nx = x + sDx[dir], ny = y + sDy[dir];
            u16 next;
            u8 move, nextElevation;
            // Off sideways stairs a step is straight: a neighbour already
            // seen (or off the map) needs no collision probe, unless it is a
            // bridge tile with a second state still free.
            if (!onStairs && (nx < 0 || ny < 0 || nx >= width || ny >= height
                              || (sWork->tiles[ny * width + nx] != TILE_UNSEEN
                                  && MapGridGetElevationAt(nx + MAP_OFFSET, ny + MAP_OFFSET) != 15
                                  && !IsSidewaysStairs(MapGridGetMetatileBehaviorAt(nx + MAP_OFFSET, ny + MAP_OFFSET)))))
                continue;
            move = ProbeStep(obj, x + MAP_OFFSET, y + MAP_OFFSET, elevation, dir);
            if (move == DIR_NONE)
                continue;
            nx = x + sDx[move];
            ny = y + sDy[move];
            if (nx < 0 || ny < 0 || nx >= width || ny >= height)
                continue;
            next = ny * width + nx;
            if ((u16)(sSearch.write - sSearch.read) >= WALKER_QUEUE_SIZE)
                continue;
            nextElevation = RestElevation(nx + MAP_OFFSET, ny + MAP_OFFSET, elevation);
            if (sWork->tiles[next] == TILE_UNSEEN)
            {
                sWork->tiles[next] = move | (nextElevation << 4);
                SetFromLayer(next, (state & STATE_LAYER) != 0);
                sWork->queue[sSearch.write % WALKER_QUEUE_SIZE] = next;
            }
            else if ((sWork->tiles[next] >> 4) != nextElevation && sWork->layerCount < WALKER_LAYER_MAX
                  && MapGridGetElevationAt(nx + MAP_OFFSET, ny + MAP_OFFSET) == 15 && FindLayer(next) == LAYER_NONE)
            {
                // The bridge tile's other state (over or under).
                u8 layer = sWork->layerCount++;
                sWork->layerTile[layer] = next;
                sWork->layerValue[layer] = move | (nextElevation << 4);
                sWork->layerFromLayer[layer] = (state & STATE_LAYER) != 0;
                sWork->queue[sSearch.write % WALKER_QUEUE_SIZE] = STATE_LAYER | layer;
            }
            else
            {
                continue;
            }
            sSearch.write++;
        }
    }

    elapsed = ScanlinesSince(start);
    if (elapsed > gWayfarerWalkersDebug.maxSliceScanlines)
        gWayfarerWalkersDebug.maxSliceScanlines = elapsed;
    if (expanded > gWayfarerWalkersDebug.maxSliceNodes)
        gWayfarerWalkersDebug.maxSliceNodes = expanded;
    if (sSearch.actor != NO_ACTOR
     && (sSearch.read == sSearch.write || (actor->goalKind == WALKER_GOAL_AWAY && sSearch.expanded >= WALKER_AWAY_NODES)))
        FailSearch(actor);
}

// Exactly one search at a time across all actors; requests are served in
// actor order, which is spawn (priority) order.
static void RunScheduler(void)
{
    u8 i;
    if (sSearch.actor == NO_ACTOR)
    {
        for (i = 0; i < WALKER_ACTOR_COUNT; i++)
        {
            if (sActors[i].mode == WALKER_MODE_LOCAL && sActors[i].phase == WALKER_PHASE_QUEUED
             && sActors[i].stepKind == STEP_NONE)
                break;
        }
        if (i == WALKER_ACTOR_COUNT || !EnsureWorkspace())
            return;
        BeginSearch(&sActors[i]);
        if (sSearch.actor == NO_ACTOR)
            return;
    }
    SearchSlice();
}

static void RequestSearch(struct WalkerActor *actor)
{
    AbortSearchFor(actor);
    actor->phase = WALKER_PHASE_QUEUED;
}

// ---------------------------------------------------------------------------
// Goals and handoffs (local to world)

static u16 ExitEdgeForWarp(u16 node, u8 x, u8 y)
{
    const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[node];
    u16 e;
    for (e = data->firstEdge; e < data->firstEdge + data->edgeCount; e++)
    {
        const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[e];
        if ((edge->kind == WORLD_EDGE_WARP || edge->kind == WORLD_EDGE_TRANSIT) && edge->a == x && edge->b == y)
            return e;
    }
    return NO_EDGE;
}

// The way out for a Gym visitor: the spot's exit warp, or a sibling exit
// mat of the same door (Saffron's three) when the player stands on it.
static u16 VisitorExitEdge(const struct WayfarerWorldRecord *record, const struct WayfarerWorldSpot *spot,
                           const struct ObjectEvent *obj)
{
    const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[record->node];
    u16 exit = ExitEdgeForWarp(record->node, spot->x, spot->y), best = exit, e, bestDistance = 0xFFFF;

    if (exit == NO_EDGE || !IsPlayerTile(spot->x + MAP_OFFSET, spot->y + MAP_OFFSET))
        return exit;
    for (e = data->firstEdge; e < data->firstEdge + data->edgeCount; e++)
    {
        const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[e];
        u16 distance;
        if (edge->kind != WORLD_EDGE_WARP || edge->target != gWayfarerWorldEdges[exit].target
         || IsPlayerTile(edge->a + MAP_OFFSET, edge->b + MAP_OFFSET))
            continue;
        distance = Distance(edge->a + MAP_OFFSET, edge->b + MAP_OFFSET, obj->currentCoords.x, obj->currentCoords.y);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = e;
        }
    }
    return best;
}

static void FaceDirection(struct WalkerActor *actor, struct ObjectEvent *obj, u8 dir)
{
    if (dir < DIR_SOUTH || dir > DIR_EAST || obj->facingDirection == dir)
        return;
    if (!ObjectEventSetHeldMovement(obj, GetFaceDirectionMovementAction(dir)))
        actor->actionPending = TRUE;
}

// Start one walking step: a straight one (overwrite cleared) or a stair
// diagonal (overwrite set, as GetCollisionAtCoords would for the player).
static bool8 StartWalk(struct WalkerActor *actor, struct ObjectEvent *obj, u8 move, u8 kind)
{
    u8 command = sMoveCommand[move];
    obj->directionOverwrite = move > DIR_EAST ? move : DIR_NONE;
    if (ObjectEventSetHeldMovement(obj, GetWalkNormalMovementAction(command)))
    {
        obj->directionOverwrite = DIR_NONE;
        return FALSE;
    }
    actor->nextX = obj->currentCoords.x + sDx[move];
    actor->nextY = obj->currentCoords.y + sDy[move];
    actor->stepKind = kind;
    return TRUE;
}

static void StartTemplate(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    const struct WayfarerWorldRecord *record = Record(actor);
    const struct WayfarerWorldSpot *spot = DestSpot(record, actor->slot);

    actor->phase = WALKER_PHASE_TEMPLATE;
    actor->goalKind = WALKER_GOAL_NONE;
    actor->pauseTicks = 0;
    if (spot == NULL)
    {
        actor->template = WORLD_TEMPLATE_SIT_OR_IDLE;
        actor->emote = WORLD_EMOTE_NONE;
        return;
    }
    actor->template = WorldSim_DefaultTemplate(spot, record->activity);
    actor->emote = WorldSim_TemplateEmote(spot, record->activity);
    if (actor->template == WORLD_TEMPLATE_STAND_AND_FACE || actor->template == WORLD_TEMPLATE_SIT_OR_IDLE
     || actor->template == WORLD_TEMPLATE_BROWSE)
        FaceDirection(actor, obj, spot->facing);
}

static void SetTileGoal(struct WalkerActor *actor, struct ObjectEvent *obj, u8 x, u8 y)
{
    actor->goalKind = WALKER_GOAL_TILE;
    actor->goalX = x;
    actor->goalY = y;
    actor->goalEdge = NO_EDGE;
    actor->pathLen = actor->pathPos = 0;
    actor->truncated = FALSE;
    if (obj->currentCoords.x == x + MAP_OFFSET && obj->currentCoords.y == y + MAP_OFFSET)
        actor->phase = WALKER_PHASE_WALK;  // path of length 0: the goal action runs next
    else
        RequestSearch(actor);
}

static void SetEdgeGoal(struct WalkerActor *actor, u16 edgeIndex)
{
    const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[edgeIndex];
    actor->goalEdge = edgeIndex;
    actor->pathLen = actor->pathPos = 0;
    actor->truncated = FALSE;
    if (edge->kind == WORLD_EDGE_WARP || edge->kind == WORLD_EDGE_TRANSIT)
    {
        actor->goalKind = WALKER_GOAL_WARP;
        actor->goalX = edge->a;
        actor->goalY = edge->b;
    }
    else
    {
        actor->goalKind = WALKER_GOAL_EDGE;
        actor->goalDir = EdgeDirection(edge->kind);
        actor->laneA = edge->a;
        actor->laneB = edge->b;
    }
    RequestSearch(actor);
}

// Hand off for the rest of this visit: the record stays where it is and
// moves on at the heartbeats; no new actor until the next map load.
static void YieldVisit(struct WalkerActor *actor)
{
    if (actor->justLeaving)
    {
        FinishLeaving(actor);
        return;
    }
    sYielded |= 1u << actor->slot;
    gWayfarerWalkersDebug.handoffs++;
    RemoveActor(actor);
}

// A Gym visitor leaves at once: the visit ends and the record moves on.
static void FinishLeaving(struct WalkerActor *actor)
{
    struct WayfarerWorldRecord *record = Record(actor);
    const struct WayfarerWorldSpot *spot = DestSpot(record, actor->slot);
    struct WayfarerWorldContext ctx;
    u16 edge = NO_EDGE;

    if (spot != NULL && spot->kind == WORLD_SPOT_GYM && spot->node == record->node)
        edge = ExitEdgeForWarp(record->node, spot->x, spot->y);
    if (edge != NO_EDGE)
    {
        WorldSim_TakeEdge(State(), actor->slot, edge, gWayfarerWorldEdges[edge].c);
        BuildContext(&ctx);
        WorldSim_AdvanceRoutine(State(), actor->slot, &ctx, NULL);
        gWayfarerWalkersDebug.localAdvances++;
    }
    sVisitorsSeen |= 1u << actor->slot;
    gWayfarerWalkersDebug.visitorsVanished++;
    RemoveActor(actor);
}

// Make room for the player: walk to an open tile away from them, wait, then
// carry on. After a few back-offs in one visit, hand off instead.
static void StartBackOff(struct WalkerActor *actor)
{
    struct ObjectEvent *player = Player();
    if (actor->justLeaving)
    {
        FinishLeaving(actor);
        return;
    }
    if (actor->backOffs >= WALKER_BACKOFFS_MAX)
    {
        YieldVisit(actor);
        return;
    }
    actor->backOffs++;
    actor->playerBlocks = 0;
    actor->pushFrames = 0;
    actor->blocked = 0;
    gWayfarerWalkersDebug.backOffs++;
    actor->goalKind = WALKER_GOAL_AWAY;
    actor->goalEdge = NO_EDGE;
    actor->goalX = player->currentCoords.x - MAP_OFFSET;
    actor->goalY = player->currentCoords.y - MAP_OFFSET;
    actor->pathLen = actor->pathPos = 0;
    actor->truncated = FALSE;
    if (IsGoalTile(actor, ActorObject(actor), ActorObject(actor)->currentCoords.x - MAP_OFFSET,
                   ActorObject(actor)->currentCoords.y - MAP_OFFSET, ActorObject(actor)->currentElevation))
    {
        actor->phase = WALKER_PHASE_IDLE;
        actor->wait = WALKER_BACKOFF_WAIT;
        return;
    }
    RequestSearch(actor);
}

static void ChooseGoal(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    struct WayfarerWorldRecord *record = Record(actor);
    const struct WayfarerWorldSpot *spot = DestSpot(record, actor->slot);
    u16 destNode, edge;
    void *workspace;

    actor->blocked = 0;
    if (record->state == WORLD_STATE_DWELLING)
    {
        if (spot != NULL && (actor->justLeaving || IsGymVisit(record, actor->slot)))
        {
            // Just leaving: straight out through the Gym's exit warp.
            actor->justLeaving = TRUE;
            edge = VisitorExitEdge(record, spot, obj);
            if (edge != NO_EDGE)
                SetEdgeGoal(actor, edge);
            else
                FinishLeaving(actor);   // never stand on the exit mat
            return;
        }
        else if (spot != NULL && spot->node == record->node)
        {
            SetTileGoal(actor, obj, spot->x, spot->y);
            return;
        }
        else if (record->destKind == WORLD_DEST_NONE)
        {
            actor->atSpot = TRUE;
            StartTemplate(actor, obj);
            return;
        }
    }

    destNode = WorldSim_DestNode(State(), actor->slot);
    if (destNode == record->node && spot != NULL)
    {
        SetTileGoal(actor, obj, spot->x, spot->y);
        return;
    }
    workspace = Alloc(WorldSim_WorkspaceSize());
    if (workspace == NULL)
    {
        actor->wait = WALKER_RETRY_FRAMES;
        return;
    }
    edge = WorldSim_NextEdge(State(), actor->slot, workspace, NULL);
    Free(workspace);
    if (edge == NO_EDGE)
    {
        // No path within the bound: stand still; the heartbeat moves the
        // record on once the player has left.
        actor->phase = WALKER_PHASE_IDLE;
        actor->wait = WALKER_UNREACHABLE_WAIT << WALKER_FAILS_MAX;
        return;
    }
    SetEdgeGoal(actor, edge);
}

static void Replan(struct WalkerActor *actor, u16 wait)
{
    gWayfarerWalkersDebug.replans++;
    AbortSearchFor(actor);
    actor->phase = WALKER_PHASE_PLAN;
    actor->wait = wait;
}

// Is the player (or the follower) on the tile a step from here would enter?
static bool8 IsPlayerAhead(struct ObjectEvent *obj, u8 dir)
{
    s16 x = obj->currentCoords.x + sDx[dir], y = obj->currentCoords.y + sDy[dir];
    u8 id = GetObjectEventIdByXY(x, y);
    if (IsPlayerTile(x, y))
        return TRUE;
    return id < OBJECT_EVENTS_COUNT && gObjectEvents[id].localId == OBJ_EVENT_ID_FOLLOWER;
}

static void OnBlocked(struct WalkerActor *actor, bool8 byPlayer)
{
    gWayfarerWalkersDebug.blockedSteps++;
    if (byPlayer && ++actor->playerBlocks >= WALKER_YIELD_BLOCKS)
    {
        StartBackOff(actor);
        return;
    }
    if (++actor->blocked >= WALKER_BLOCKED_LIMIT)
    {
        // Four blocked steps drop the goal and re-plan from scratch.
        actor->blocked = 0;
        Replan(actor, WALKER_RETRY_FRAMES);
        return;
    }
    gWayfarerWalkersDebug.replans++;
    if (actor->goalKind == WALKER_GOAL_NONE)
        Replan(actor, WALKER_RETRY_FRAMES);
    else
        RequestSearch(actor);
}

static void OnGoalReached(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    struct WayfarerWorldRecord *record = Record(actor);
    s16 x = obj->currentCoords.x, y = obj->currentCoords.y;

    switch (actor->goalKind)
    {
    case WALKER_GOAL_TILE:
        if (record->state == WORLD_STATE_TRAVELLING && WorldSim_DestNode(State(), actor->slot) == record->node)
        {
            WorldSim_Arrive(State(), actor->slot);
            gWayfarerWalkersDebug.arrivals++;
            actor->atSpot = FALSE;
        }
        if (!actor->atSpot)
        {
            actor->atSpot = TRUE;
            actor->frames = actor->dwellTicks = 0;
        }
        StartTemplate(actor, obj);
        break;
    case WALKER_GOAL_AWAY:
        // Out of the player's way: wait a while before carrying on.
        actor->goalKind = WALKER_GOAL_NONE;
        actor->phase = WALKER_PHASE_IDLE;
        actor->wait = WALKER_BACKOFF_WAIT;
        break;
    case WALKER_GOAL_EDGE:
    {
        u8 dir = actor->goalDir;
        const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[actor->goalEdge];
        u8 along = (dir == DIR_NORTH || dir == DIR_SOUTH) ? x - MAP_OFFSET : y - MAP_OFFSET;
        if (ProbeFromHere(obj, dir) != dir)
        {
            OnBlocked(actor, IsPlayerAhead(obj, dir));
            return;
        }
        if (!StartWalk(actor, obj, dir, STEP_EXIT_EDGE))
            return;
        // The handoff commits as the exit step starts, so a player crossing
        // at once can't outrun it.
        WorldSim_TakeEdge(State(), actor->slot, actor->goalEdge, WorldSim_LaneCrossing(edge, along));
        NoteEntry(actor->slot, actor->goalEdge, WorldSim_LaneCrossing(edge, along));
        gWayfarerWalkersDebug.edgeExits++;
        actor->phase = WALKER_PHASE_EXIT;
        actor->atSpot = FALSE;
        break;
    }
    case WALKER_GOAL_WARP:
    {
        s16 wx = actor->goalX + MAP_OFFSET, wy = actor->goalY + MAP_OFFSET;
        u8 dir = DirectionTowards(x, y, wx, wy);
        u16 edgeIndex = actor->goalEdge;
        bool8 door = MapGridGetCollisionAt(wx, wy) != 0;
        if (door ? GetObjectEventIdByXY(wx, wy) != OBJECT_EVENTS_COUNT || IsPlayerTile(wx, wy)
                 : ProbeFromHere(obj, dir) != dir)
        {
            OnBlocked(actor, IsPlayerAhead(obj, dir));
            return;
        }
        if (!StartWalk(actor, obj, dir, STEP_EXIT_WARP))
            return;
        WorldSim_TakeEdge(State(), actor->slot, edgeIndex, gWayfarerWorldEdges[edgeIndex].c);
        NoteEntry(actor->slot, edgeIndex, gWayfarerWorldEdges[edgeIndex].c);
        if (actor->justLeaving)
        {
            // A Gym visit ends as the visitor walks out: the record moves on
            // rather than heading back to the Gym.
            struct WayfarerWorldContext ctx;
            BuildContext(&ctx);
            WorldSim_AdvanceRoutine(State(), actor->slot, &ctx, NULL);
            gWayfarerWalkersDebug.localAdvances++;
        }
        gWayfarerWalkersDebug.warpExits++;
        actor->phase = WALKER_PHASE_EXIT;
        break;
    }
    default:
        actor->phase = WALKER_PHASE_PLAN;
        break;
    }
}

static void WalkStep(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    u8 move;

    if (actor->pathPos >= actor->pathLen)
    {
        if (actor->truncated)
            RequestSearch(actor);
        else
            OnGoalReached(actor, obj);
        return;
    }
    if (sWork == NULL)
    {
        RequestSearch(actor);
        return;
    }
    move = sWork->path[ActorIndex(actor)][actor->pathPos];
    if (move < DIR_SOUTH || move > DIR_NORTHEAST || ProbeFromHere(obj, sMoveCommand[move]) != move)
    {
        OnBlocked(actor, move >= DIR_SOUTH && move <= DIR_NORTHEAST && IsPlayerAhead(obj, sMoveCommand[move]));
        return;
    }
    if (!StartWalk(actor, obj, move, STEP_WALK))
        return;
    actor->pathPos++;
}

static void FinishStep(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    u8 kind = actor->stepKind;
    actor->stepKind = STEP_NONE;
    obj->directionOverwrite = DIR_NONE;
    switch (kind)
    {
    case STEP_WALK:
        if (obj->currentCoords.x != actor->nextX || obj->currentCoords.y != actor->nextY)
            OnBlocked(actor, FALSE);
        else
            actor->blocked = 0;
        break;
    case STEP_EXIT_EDGE:
        // Out of the map: keep walking out of view in the connection strip.
        actor->mode = WALKER_MODE_STRIP;
        actor->stripDir = actor->goalDir;
        actor->stripToward = FALSE;
        actor->phase = WALKER_PHASE_WALK;
        break;
    case STEP_EXIT_WARP:
        RemoveActor(actor);
        break;
    case STEP_ENTER:
        actor->mode = WALKER_MODE_LOCAL;
        actor->phase = WALKER_PHASE_PLAN;
        actor->wait = 0;
        break;
    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// Connection strips and the seam

// The side of the player's map a strip actor stands beyond.
static u8 StripSide(const struct ObjectEvent *obj)
{
    s16 x = obj->currentCoords.x - MAP_OFFSET, y = obj->currentCoords.y - MAP_OFFSET;
    if (y < 0) return DIR_NORTH;
    if (y >= MapHeight()) return DIR_SOUTH;
    if (x < 0) return DIR_WEST;
    if (x >= MapWidth()) return DIR_EAST;
    return DIR_NONE;
}

// The record's edge into the player's map through this side, at coord.
static u16 EdgeIntoPlayerMap(u16 node, u8 side, s16 coord)
{
    const struct WayfarerWorldNode *data = &gWayfarerWorldNodes[node];
    u8 kind = EdgeKindForDirection(sOpposite[side]);
    u16 e;

    for (e = data->firstEdge; e < data->firstEdge + data->edgeCount; e++)
    {
        const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[e];
        if (edge->kind == kind && WorldSim_NodeMap(edge->target) == CurrentMap()
         && coord >= edge->c && coord <= edge->c + (edge->b - edge->a))
            return e;
    }
    return NO_EDGE;
}

static void UpdateStrip(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    u8 side = StripSide(obj), dir;
    s16 nx, ny;

    if (!IsVisible(obj) || side == DIR_NONE)
    {
        RemoveActor(actor);
        return;
    }
    dir = actor->stripToward ? sOpposite[side] : side;
    nx = obj->currentCoords.x + sDx[dir];
    ny = obj->currentCoords.y + sDy[dir];

    if (actor->stripToward && InMap(nx - MAP_OFFSET, ny - MAP_OFFSET))
    {
        s16 coord = (dir == DIR_NORTH || dir == DIR_SOUTH) ? nx - MAP_OFFSET : ny - MAP_OFFSET;
        u16 edge = EdgeIntoPlayerMap(Record(actor)->node, side, coord);
        if (edge == NO_EDGE)
        {
            actor->stripToward = FALSE;
            return;
        }
        if (ProbeFromHere(obj, dir) != dir || !StartWalk(actor, obj, dir, STEP_ENTER))
            return;
        WorldSim_TakeEdge(State(), actor->slot, edge, coord);
        gWayfarerWalkersDebug.stripEntries++;
        return;
    }

    // Past the loaded grid there is no collision data: keep walking until
    // the sprite has left the viewport.
    if (nx >= 0 && ny >= 0 && nx < gBackupMapLayout.width && ny < gBackupMapLayout.height
     && ProbeFromHere(obj, dir) != dir)
        return;
    StartWalk(actor, obj, dir, STEP_STRIP);
}

// A camera transition kept every actor's object (coordinates shifted with
// the camera). Actors whose record is on the new map and who stand inside it
// are rebased into it without a respawn; the others stay in the strip while
// visible.
static void OnSeam(void)
{
    u8 i;
    u16 map = CurrentMap();

    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        struct WalkerActor *actor = &sActors[i];
        struct ObjectEvent *obj;
        const struct WayfarerWorldRecord *record;
        bool8 heading = FALSE;

        if (actor->mode == WALKER_MODE_NONE)
            continue;
        obj = ActorObject(actor);
        record = Record(actor);
        AbortSearchFor(actor);
        if (actor->stepKind != STEP_NONE)
            actor->stepKind = (actor->stepKind == STEP_EXIT_WARP) ? STEP_EXIT_WARP : STEP_SETTLE;
        if (WorldSim_IsSimulated(record) && WorldSim_NodeMap(record->node) == map)
        {
            // Only an actor actually standing in the new map: one whose
            // record the heartbeat moved here from out of view is respawned
            // by the record rules instead.
            if (!InMap(obj->currentCoords.x - MAP_OFFSET, obj->currentCoords.y - MAP_OFFSET))
            {
                RemoveActor(actor);
                continue;
            }
            obj->mapGroup = MAP_GROUP(map);
            obj->mapNum = MAP_NUM(map);
            obj->initialCoords = obj->currentCoords;
            actor->mode = WALKER_MODE_LOCAL;
            actor->phase = WALKER_PHASE_PLAN;
            actor->wait = 0;
            ForgetEntry(actor->slot);
            gWayfarerWalkersDebug.rebases++;
            continue;
        }
        if (!IsVisible(obj) || StripSide(obj) == DIR_NONE)
        {
            RemoveActor(actor);
            continue;
        }
        // The player crossed ahead of a walker heading this way: it follows
        // into the player's map; otherwise it walks off out of view.
        if (actor->mode == WALKER_MODE_LOCAL && actor->goalKind == WALKER_GOAL_EDGE && actor->goalEdge != NO_EDGE
         && WorldSim_NodeMap(gWayfarerWorldEdges[actor->goalEdge].target) == map)
            heading = TRUE;
        actor->mode = WALKER_MODE_STRIP;
        actor->stripToward = heading;
        actor->phase = WALKER_PHASE_WALK;
        gWayfarerWalkersDebug.rebases++;
    }
}

// ---------------------------------------------------------------------------
// Behaviour templates and local dwell

static void ShowEmote(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    if (actor->emote == WORLD_EMOTE_EXCLAMATION)
    {
        if (!ObjectEventSetHeldMovement(obj, MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK))
        {
            actor->actionPending = TRUE;
            gWayfarerWalkersDebug.emotes++;
        }
    }
    else if (actor->emote == WORLD_EMOTE_ELLIPSIS && !FieldEffectActiveListContains(FLDEFF_EMOTE))
    {
        // The follower emote sheet's "..." frame.
        gFieldEffectArguments[0] = obj->localId;
        gFieldEffectArguments[1] = obj->mapNum;
        gFieldEffectArguments[2] = obj->mapGroup;
        gFieldEffectArguments[7] = EMOTE_ELLIPSIS_FRAME;
        FieldEffectStart(FLDEFF_EMOTE);
        gWayfarerWalkersDebug.emotes++;
    }
}

static bool8 IsTileTaken(s16 x, s16 y)
{
    return !IsStandable(x + MAP_OFFSET, y + MAP_OFFSET)
        || GetObjectEventIdByXY(x + MAP_OFFSET, y + MAP_OFFSET) != OBJECT_EVENTS_COUNT
        || IsPlayerTile(x + MAP_OFFSET, y + MAP_OFFSET);
}

static void WanderMove(struct WalkerActor *actor, struct ObjectEvent *obj, const struct WayfarerWorldSpot *spot)
{
    u16 n = spot->dataCount, i, index;
    if (n < 2)
        return;
    index = WorldSim_TemplateIndex(CatalogIndex(actor->slot), actor->k++, n);
    for (i = 0; i < n; i++, index = (index + 1) % n)
    {
        const u8 *tile = gWayfarerWorldAreaTiles[spot->dataStart + index];
        if (tile[0] + MAP_OFFSET == obj->currentCoords.x && tile[1] + MAP_OFFSET == obj->currentCoords.y)
            continue;
        if (IsTileTaken(tile[0], tile[1]))
            continue;
        gWayfarerWalkersDebug.templateMoves++;
        SetTileGoal(actor, obj, tile[0], tile[1]);
        return;
    }
}

// Another floor of the same store, under its capacity, every third move.
static bool8 BrowseOtherFloor(struct WalkerActor *actor, const struct WayfarerWorldSpot *spot)
{
    const struct WayfarerWorldRecord *record = Record(actor);
    u16 floors = spot->dataCount, f, index, s;

    if (floors < 2)
        return FALSE;
    index = WorldSim_TemplateIndex(CatalogIndex(actor->slot), actor->k / BROWSE_FLOOR_EVERY, floors);
    for (f = 0; f < floors; f++, index = (index + 1) % floors)
    {
        u16 floorNode = gWayfarerWorldStoreFloors[spot->dataStart + index];
        if (floorNode == record->node)
            continue;
        if (WorldSim_Occupancy(State(), WorldSim_NodeMap(floorNode), actor->slot) >= WorldSim_MapCapacity(floorNode))
            continue;
        for (s = FirstSpotFromNode(MapFirstNode(floorNode)); s < gWayfarerWorldSpotCount; s++)
        {
            const struct WayfarerWorldSpot *other = &gWayfarerWorldSpots[s];
            if (WorldSim_NodeMap(other->node) != WorldSim_NodeMap(floorNode))
                break;
            if (other->node == floorNode && other->kind == WORLD_SPOT_STORE && !WorldSim_SpotTaken(State(), s, actor->slot))
            {
                WorldSim_ChangeSpot(State(), actor->slot, s);
                gWayfarerWalkersDebug.floorChanges++;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void BrowseMove(struct WalkerActor *actor, struct ObjectEvent *obj, const struct WayfarerWorldSpot *spot)
{
    const struct WayfarerWorldRecord *record = Record(actor);
    u16 current = record->destId, first = current, last = current, n, i, index;

    actor->k++;
    if (actor->k % BROWSE_FLOOR_EVERY == 0 && BrowseOtherFloor(actor, spot))
    {
        actor->atSpot = FALSE;
        Replan(actor, 0);
        return;
    }
    // The store's shelves on this floor: a contiguous run of the spot table.
    while (first > 0 && gWayfarerWorldSpots[first - 1].node == spot->node && gWayfarerWorldSpots[first - 1].kind == WORLD_SPOT_STORE)
        first--;
    while (last + 1 < gWayfarerWorldSpotCount && gWayfarerWorldSpots[last + 1].node == spot->node
        && gWayfarerWorldSpots[last + 1].kind == WORLD_SPOT_STORE)
        last++;
    n = last - first + 1;
    if (n < 2)
        return;
    index = WorldSim_TemplateIndex(CatalogIndex(actor->slot), actor->k, n);
    for (i = 0; i < n; i++, index = (index + 1) % n)
    {
        u16 shelf = first + index;
        const struct WayfarerWorldSpot *data = &gWayfarerWorldSpots[shelf];
        if (shelf == current || WorldSim_SpotTaken(State(), shelf, actor->slot) || IsTileTaken(data->x, data->y))
            continue;
        WorldSim_ChangeSpot(State(), actor->slot, shelf);
        gWayfarerWalkersDebug.templateMoves++;
        SetTileGoal(actor, obj, data->x, data->y);
        return;
    }
}

static void TemplateTick(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    const struct WayfarerWorldSpot *spot = DestSpot(Record(actor), actor->slot);

    if (actor->emote != WORLD_EMOTE_NONE && WorldSim_IsEmoteTick(CatalogIndex(actor->slot), actor->t))
        ShowEmote(actor, obj);
    if (spot == NULL || actor->actionPending)
        return;
    switch (actor->template)
    {
    case WORLD_TEMPLATE_WANDER:
        if (++actor->pauseTicks >= WANDER_PAUSE_TICKS)
        {
            actor->pauseTicks = 0;
            WanderMove(actor, obj, spot);
        }
        else
        {
            // Look around during the pause.
            FaceDirection(actor, obj, ((CatalogIndex(actor->slot) + actor->t) % 4) + DIR_SOUTH);
        }
        break;
    case WORLD_TEMPLATE_BROWSE:
        if (++actor->pauseTicks >= BROWSE_INTERVAL_TICKS)
        {
            actor->pauseTicks = 0;
            BrowseMove(actor, obj, spot);
        }
        break;
    }
}

// Dwell also runs down in local time while watched: one heartbeat of dwell
// per WORLD_LOCAL_DWELL_TICKS dwell ticks; at 0 the routine advances.
static bool8 LocalDwellTick(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    struct WayfarerWorldRecord *record = Record(actor);
    struct WayfarerWorldContext ctx;

    if (record->state != WORLD_STATE_DWELLING || !actor->atSpot)
        return FALSE;
    if (++actor->dwellTicks < WORLD_LOCAL_DWELL_TICKS)
        return FALSE;
    actor->dwellTicks = 0;
    gWayfarerWalkersDebug.localDwellBeats++;
    if (record->dwell > 0)
        record->dwell--;
    if (record->dwell != 0)
        return FALSE;
    BuildContext(&ctx);
    WorldSim_AdvanceRoutine(State(), actor->slot, &ctx, NULL);
    gWayfarerWalkersDebug.localAdvances++;
    actor->atSpot = FALSE;
    actor->k = actor->t = 0;
    actor->pauseTicks = 0;
    Replan(actor, 0);
    return TRUE;
}

// The player pushing against a standing walker (adjacent, facing it).
static bool8 IsPlayerPushing(const struct ObjectEvent *obj)
{
    struct ObjectEvent *player = Player();
    s16 px = player->currentCoords.x, py = player->currentCoords.y;
    if (Distance(px, py, obj->currentCoords.x, obj->currentCoords.y) != 1)
        return FALSE;
    return player->facingDirection == DirectionTowards(px, py, obj->currentCoords.x, obj->currentCoords.y);
}

static void UpdateActor(struct WalkerActor *actor)
{
    struct ObjectEvent *obj = ActorObject(actor);
    struct WayfarerWorldRecord *record = Record(actor);

    if (obj->frozen)
        return;
    if (actor->stepKind != STEP_NONE)
    {
        if (!ObjectEventClearHeldMovementIfFinished(obj))
            return;
        FinishStep(actor, obj);
        if (actor->mode == WALKER_MODE_NONE)
            return;
    }
    if (actor->actionPending)
    {
        if (!ObjectEventClearHeldMovementIfFinished(obj))
            return;
        actor->actionPending = FALSE;
    }
    if (actor->mode == WALKER_MODE_STRIP)
    {
        UpdateStrip(actor, obj);
        return;
    }
    if (!WorldSim_IsSimulated(record) || WorldSim_NodeMap(record->node) != CurrentMap()
     || (record->state == WORLD_STATE_DWELLING && IsLeaderHomeInGym(actor->slot))
     || !InMap(obj->currentCoords.x - MAP_OFFSET, obj->currentCoords.y - MAP_OFFSET))
    {
        RemoveActor(actor);
        return;
    }

    // Never trap the player: pushed against for a while, make room.
    if (actor->phase != WALKER_PHASE_SEARCH && IsPlayerPushing(obj))
    {
        if (++actor->pushFrames >= WALKER_YIELD_FRAMES)
        {
            StartBackOff(actor);
            return;
        }
    }
    else
    {
        actor->pushFrames = 0;
    }

    // Dwell ticks: the template's clock and the local dwell.
    if (actor->atSpot && ++actor->frames >= WORLD_DWELL_TICK_FRAMES)
    {
        actor->frames = 0;
        actor->t++;
        if (LocalDwellTick(actor, obj))
            return;
        if (actor->phase == WALKER_PHASE_TEMPLATE)
            TemplateTick(actor, obj);
    }

    switch (actor->phase)
    {
    case WALKER_PHASE_PLAN:
        if (actor->wait != 0)
            actor->wait--;
        else
            ChooseGoal(actor, obj);
        break;
    case WALKER_PHASE_IDLE:
        if (actor->wait != 0)
            actor->wait--;
        else
            actor->phase = WALKER_PHASE_PLAN;
        break;
    case WALKER_PHASE_TEMPLATE:
        // An unreachable template move waits here, then the goal is retried.
        if (actor->wait != 0 && --actor->wait == 0 && actor->goalKind == WALKER_GOAL_NONE)
            ChooseGoal(actor, obj);
        break;
    case WALKER_PHASE_WALK:
        WalkStep(actor, obj);
        break;
    }
}

// ---------------------------------------------------------------------------
// Following Pokémon, frozen mask and the local actor block

static void UpdateFollower(void)
{
    u8 i, others = 0, wanted, slot;
    bool8 hide;
    s16 freeSlots;

    wanted = CountActors(WALKER_MODE_LOCAL) + CountActors(WALKER_MODE_STRIP);
    if (sMapNode != WORLD_NODE_NONE)
    {
        u8 capacity = WorldSim_MapCapacity(sMapNode), locals = CountActors(WALKER_MODE_LOCAL);
        u8 pending = 0, room = capacity > locals ? capacity - locals : 0;
        for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT && pending < room; slot++)
        {
            if (IsSpawnCandidate(slot))
                pending++;
        }
        wanted += pending;
    }
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        const struct ObjectEvent *obj = &gObjectEvents[i];
        if (obj->active && obj->localId != OBJ_EVENT_ID_FOLLOWER && !WayfarerWalkers_IsActorObject(obj))
            others++;
    }
    // Free slots with every actor spawned and the follower out.
    freeSlots = OBJECT_EVENTS_COUNT - others - wanted - 1;
    hide = wanted != 0 && freeSlots < 2;
    if (hide && !sFollowerFlagOurs && !FlagGet(FLAG_TEMP_HIDE_FOLLOWER))
    {
        // Only our own use of the temp flag is ever cleared again: maps whose
        // scripts set it (Fortree and Mossdeep Gyms, Trainer Hill...) keep it.
        FlagSet(FLAG_TEMP_HIDE_FOLLOWER);
        sFollowerFlagOurs = TRUE;
        sFollowerRemoved = TRUE;
        RemoveFollowingPokemon();
    }
    else if (!hide && sFollowerFlagOurs)
    {
        FlagClear(FLAG_TEMP_HIDE_FOLLOWER);
        sFollowerFlagOurs = FALSE;
        sFollowerRemoved = FALSE;
        UpdateFollowingPokemon();
    }
    else if (!hide && sFollowerRemoved && !FlagGet(FLAG_TEMP_HIDE_FOLLOWER))
    {
        // A map seam cleared our flag but nothing respawns the follower
        // there (a warp would): bring it back once there's room.
        sFollowerRemoved = FALSE;
        if (GetFollowerObject() == NULL)
            UpdateFollowingPokemon();
    }
    sHideFollower = hide;
    gWayfarerWalkersDebug.followerHidden = sFollowerFlagOurs;
}

// Off-screen actors give their slot back when the map's own objects need it.
static void CullForSlots(void)
{
    s8 i;
    if (FreeObjectSlots() > 1)
        return;
    for (i = WALKER_ACTOR_COUNT - 1; i >= 0; i--)
    {
        struct WalkerActor *actor = &sActors[i];
        if (actor->mode == WALKER_MODE_LOCAL && actor->stepKind == STEP_NONE && !IsVisible(ActorObject(actor)))
        {
            gWayfarerWalkersDebug.culls++;
            YieldVisit(actor);
            return;
        }
    }
}

// A story object that appeared after the walker (a script's addobject)
// wins: the walker leaves.
static void CheckStoryScenes(void)
{
    u8 i;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        struct WalkerActor *actor = &sActors[i];
        if (actor->mode == WALKER_MODE_LOCAL && actor->stepKind == STEP_NONE && IsStorySuppressed(actor->slot))
        {
            gWayfarerWalkersDebug.storySuppressed++;
            YieldVisit(actor);
        }
    }
}

static void PublishState(void)
{
    struct WayfarerWorldState *state = State();
    u32 mask = 0;
    u8 i, entry = 0;

    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        struct WalkerActor *actor = &sActors[i];
        struct WayfarerWalkerActorDebug *debug = &gWayfarerWalkersDebug.actors[i];
        memset(debug, 0, sizeof(*debug));
        debug->slot = SLOT_NONE;
        if (actor->mode == WALKER_MODE_NONE)
            continue;
        mask |= 1u << actor->slot;
        debug->slot = actor->slot;
        debug->mode = actor->mode;
        debug->phase = actor->phase;
        debug->goalKind = actor->goalKind;
        debug->objectId = actor->objectId;
        debug->template = actor->template;
        debug->atSpot = actor->atSpot;
        debug->blocked = actor->blocked;
        debug->x = ActorObject(actor)->currentCoords.x - MAP_OFFSET;
        debug->y = ActorObject(actor)->currentCoords.y - MAP_OFFSET;
        debug->goalX = actor->goalX;
        debug->goalY = actor->goalY;
        debug->goalEdge = actor->goalEdge;

        // The local actor block follows live actors so any save captures them.
        if (!sRestorePending && actor->mode == WALKER_MODE_LOCAL && entry < WORLD_LOCAL_ACTOR_COUNT
         && actor->stepKind != STEP_EXIT_EDGE && actor->stepKind != STEP_EXIT_WARP
         && WorldSim_NodeMap(state->records[actor->slot].node) == CurrentMap()
         && InMap(debug->x, debug->y))
        {
            WorldSim_SetLocalActor(state, entry++, actor->slot, debug->x, debug->y, ActorObject(actor)->facingDirection);
        }
    }
    if (!sRestorePending)
    {
        for (; entry < WORLD_LOCAL_ACTOR_COUNT; entry++)
        {
            u8 *raw = &state->localActors[entry * WORLD_LOCAL_ACTOR_BYTES];
            raw[0] = WORLD_LOCAL_ACTOR_NONE;
            raw[1] = raw[2] = 0;
        }
    }
    // Trainers with an actor (or visible in a strip) are left alone by the heartbeat.
    gWayfarerWorldFrozenMask = mask;
    gWayfarerWalkersDebug.activeActors = CountActors(WALKER_MODE_LOCAL) + CountActors(WALKER_MODE_STRIP);
    gWayfarerWalkersDebug.currentMap = CurrentMap();
    gWayfarerWalkersDebug.worldState = (u32)state;
}

// Objects the engine dropped (a script's removeobject, a reset) are forgotten.
static void ValidateActors(void)
{
    u8 i;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        struct WalkerActor *actor = &sActors[i];
        const struct ObjectEvent *obj;
        if (actor->mode == WALKER_MODE_NONE)
            continue;
        obj = ActorObject(actor);
        if (!obj->active || obj->localId != WALKER_LOCALID_BASE + i || obj->graphicsId != Trainer(actor->slot)->graphicsId)
            ForgetActor(actor);
    }
}

static void OnMapChanged(u16 map)
{
    if (sTransition == TRANSITION_CAMERA && sActiveMapValid)
        OnSeam();
    else
        ForgetAllActors();
    sSearch.actor = NO_ACTOR;
    sActiveMap = map;
    sActiveMapValid = TRUE;
    sTransition = TRANSITION_NONE;
    sMapNode = FirstNodeOfMap(map);
    sVisitorsSeen = 0;
    sYielded = 0;
    sSpawnTimer = 0;
    sSpawnBackoff = 0;
    // Map loads clear temp flags: the follower flag isn't ours any more.
    sHideFollower = FALSE;
    sFollowerFlagOurs = FALSE;
}

// ---------------------------------------------------------------------------
// Hooks

void WayfarerWalkers_Update(void)
{
    u8 i;
    u16 map;
    u32 start;

    if (gSaveBlock1Ptr == NULL || gMapHeader.mapLayout == NULL || gMapHeader.events == NULL)
        return;
    start = ScanlineStamp();
    gWayfarerWalkersDebug.frames++;
    map = CurrentMap();
    if (!sActiveMapValid || sActiveMap != map || sTransition != TRANSITION_NONE)
        OnMapChanged(map);
    ValidateActors();
    if (sRestorePending && !sAdoptChecked)
        AdoptRestoredObjects();

    if (++sSpawnTimer >= WALKER_SPAWN_PERIOD)
    {
        sSpawnTimer = 0;
        // Story objects can appear during a script, with the controls locked.
        CheckStoryScenes();
        CullForSlots();
    }

    // Talks, menus and scripts lock the field controls: the AI and its dwell
    // stay suspended (the engine still finishes a step already under way).
    if (!ArePlayerFieldControlsLocked())
    {
        RunScheduler();
        for (i = 0; i < WALKER_ACTOR_COUNT; i++)
        {
            if (sActors[i].mode != WALKER_MODE_NONE)
                UpdateActor(&sActors[i]);
        }
        if (sSpawnTimer == 0)
        {
            if (sSpawnBackoff > WALKER_SPAWN_PERIOD)
            {
                sSpawnBackoff -= WALKER_SPAWN_PERIOD;
            }
            else
            {
                u32 spawnStart = ScanlineStamp(), spawnCost;
                sSpawnBackoff = 0;
                TrySpawn();
                spawnCost = ScanlinesSince(spawnStart);
                if (spawnCost > gWayfarerWalkersDebug.maxSpawnScanlines)
                    gWayfarerWalkersDebug.maxSpawnScanlines = spawnCost;
            }
            UpdateFollower();
        }
    }
    PublishState();
    {
        u32 cost = ScanlinesSince(start);
        if (cost > gWayfarerWalkersDebug.maxUpdateScanlines)
            gWayfarerWalkersDebug.maxUpdateScanlines = cost;
    }
}

void WayfarerWalkers_OnWarp(void)
{
    // The warp resets every object; the records stay on their nodes and the
    // heartbeat that follows treats them as off-screen.
    ForgetAllActors();
    KeepEntriesFor(CurrentMap());
    gWayfarerWorldFrozenMask = 0;
    sTransition = TRANSITION_WARP;
}

void WayfarerWalkers_OnCameraTransition(void)
{
    // Objects survive a camera transition (OnSeam sorts them out next frame).
    // Before the heartbeat, unfreeze the trainers whose actor will be dropped:
    // not on the new map and out of view once the camera follows the player.
    // Object coordinates aren't shifted yet, but offsets from the player are.
    const struct ObjectEvent *player = Player();
    u16 map = CurrentMap();
    u8 i;

    sTransition = TRANSITION_CAMERA;
    KeepEntriesFor(map);
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        const struct WalkerActor *actor = &sActors[i];
        const struct ObjectEvent *obj;
        s16 dx, dy;
        if (actor->mode == WALKER_MODE_NONE)
            continue;
        obj = ActorObject(actor);
        dx = obj->currentCoords.x - player->currentCoords.x;
        dy = obj->currentCoords.y - player->currentCoords.y;
        if (WorldSim_NodeMap(State()->records[actor->slot].node) != map
         && (dx < -(MAP_OFFSET + 1) || dx > MAP_OFFSET + 1 || dy < -(MAP_OFFSET - 1) || dy > MAP_OFFSET - 1))
            gWayfarerWorldFrozenMask &= ~(1u << actor->slot);
    }
}

void WayfarerWalkers_OnContinue(void)
{
    struct WayfarerWorldState *state = State();
    u8 i, slot, x, y, facing;

    ForgetAllActors();
    memset(sRecentEntries, 0, sizeof(sRecentEntries));
    sActiveMapValid = FALSE;
    sPendingCount = 0;
    for (i = 0; i < WORLD_LOCAL_ACTOR_COUNT; i++)
    {
        if (WorldSim_GetLocalActor(state, i, &slot, &x, &y, &facing))
        {
            sPending[sPendingCount].slot = slot;
            sPending[sPendingCount].x = x;
            sPending[sPendingCount].y = y;
            sPending[sPendingCount].facing = facing;
            sPendingCount++;
        }
    }
    // Used once, for the restored map, and then cleared.
    for (i = 0; i < WORLD_LOCAL_ACTOR_COUNT; i++)
    {
        u8 *raw = &state->localActors[i * WORLD_LOCAL_ACTOR_BYTES];
        raw[0] = WORLD_LOCAL_ACTOR_NONE;
        raw[1] = raw[2] = 0;
    }
    // The saved objects come back with any step that was under way at save
    // time: end it before their sprites are rebuilt.
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (WayfarerWalkers_IsActorObject(&gObjectEvents[i]))
            SettleHeldMovement(&gObjectEvents[i]);
    }
    sRestorePending = TRUE;
    sAdoptChecked = FALSE;
}

void WayfarerWalkers_Reset(void)
{
    ForgetAllActors();
    memset(sRecentEntries, 0, sizeof(sRecentEntries));
    sPendingCount = 0;
    sRestorePending = FALSE;
    sAdoptChecked = FALSE;
    sActiveMapValid = FALSE;
    sTransition = TRANSITION_NONE;
    sHideFollower = FALSE;
    sFollowerFlagOurs = FALSE;
    sFollowerRemoved = FALSE;
    sVisitorsSeen = 0;
    sYielded = 0;
}

void WayfarerWalkers_OnHeapReset(void)
{
    u8 i;
    // InitHeap overwrites the allocator: never Free this pointer again. Each
    // actor re-plans from its current tile. A menu, battle or save tears the
    // field down: a step under way would replay with no collision check when
    // the sprites come back, so it ends here, on its destination tile.
    sWork = NULL;
    sSearch.actor = NO_ACTOR;
    gWayfarerWalkersDebug.heapResets++;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        struct WalkerActor *actor = &sActors[i];
        struct ObjectEvent *obj;
        if (actor->mode == WALKER_MODE_NONE)
            continue;
        obj = ActorObject(actor);
        if (obj->active && obj->localId == WALKER_LOCALID_BASE + i)
        {
            SettleHeldMovement(obj);
            actor->actionPending = FALSE;
        }
        if (actor->mode == WALKER_MODE_LOCAL
         && (actor->phase == WALKER_PHASE_QUEUED || actor->phase == WALKER_PHASE_SEARCH || actor->phase == WALKER_PHASE_WALK))
        {
            actor->phase = WALKER_PHASE_PLAN;
            actor->wait = 0;
        }
    }
}

bool8 WayfarerWalkers_IsActorObject(const struct ObjectEvent *objectEvent)
{
    return objectEvent->active && objectEvent->localId >= WALKER_LOCALID_BASE
        && objectEvent->localId < WALKER_LOCALID_BASE + WALKER_ACTOR_COUNT;
}

bool8 WayfarerWalkers_HideTemplate(const struct ObjectEventTemplate *template)
{
    u16 map;
    u8 slot;

    if (gSaveBlock1Ptr == NULL)
        return FALSE;
    map = CurrentMap();
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        const struct WayfarerWorldTrainer *trainer = Trainer(slot);
        if (trainer->leaderLocalId == 0 || trainer->gymNode == WORLD_NODE_NONE)
            continue;
        if (template->localId == trainer->leaderLocalId && WorldSim_NodeMap(trainer->gymNode) == map)
            return !IsLeaderObjectShown(slot);
    }
    return FALSE;
}

bool8 WayfarerWalkers_HideFollower(void)
{
    return sHideFollower;
}

#endif // IS_WAYFARER
