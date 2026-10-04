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
#include "fake_rtc.h"
#include "script.h"
#include "sprite.h"
#include "wayfarer_ambience.h"
#include "wayfarer_walker_beats.h"
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
// The spawn period's frames: spawns at 0, story scenes and culls at
// STORY_PHASE, the follower rule at FOLLOWER_PHASE, so no frame carries two
// of them (each is about 7 to 10 scanlines); beats keep off all three.
#define STORY_PHASE             2
#define FOLLOWER_PHASE          4
#define WALKER_SPAWN_BACKOFF    64      // frames before retrying a spawn that found no room
#define WALKER_NEAR_RADIUS      3
#define WALKER_LANE_SCAN        10
#define WALKER_YIELD_FRAMES     40      // the player pushing against a walker this long
#define WALKER_YIELD_BLOCKS     2       // steps blocked by the player before backing off
#define WALKER_BACKOFFS_MAX     3       // back-offs per visit before handing off
#define WALKER_BACKOFF_WAIT     180
#define WALKER_WALKOFF_FRAMES   1200    // a walk-off gives up (vanishes) after this long (the
                                        // spec's value, doubled for slow walking although
                                        // walk-offs walk at normal speed)
#define WALKER_AWAY_DISTANCE    3
#define WALKER_STRIP_BLOCKED_FRAMES 30  // a strip actor that can't step on is removed after this long
#define WALKER_JOB_SCANLINES    32      // a frame's world-job work: a trip search's slice
#define WALKER_JOB_NODES        4       // graph nodes per search call between clock checks
#define WALKER_SPAWN_CLEARANCE  2       // never spawn this close to the player
#define WALKER_SPAWN_AHEAD      4       // nor this far straight ahead of them
#define WANDER_PAUSE_TICKS      3
#define BROWSE_INTERVAL_TICKS   8
#define BROWSE_FLOOR_EVERY      3

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
    u8 template;
    u8 stripDir;        // strip actors: the direction they walk
    u16 wait;
    u8 frames;          // frames into the current dwell tick
    u8 dwellTicks;      // dwell ticks towards one heartbeat of local dwell
    u16 t;              // template tick counter; frames into a walk-off
    u16 k;              // template move counter
    u8 pauseTicks;
    u8 failures;        // consecutive failed searches
    u8 playerBlocks;    // steps blocked by the player
    u8 backOffs;        // back-offs this visit
    u8 pushFrames;      // frames the player has pushed against this walker
    u8 capacityWaits;   // exits put off because the next map was full
    u8 ambienceLatches; // AMBIENCE_LATCH_*: kept here, not in the heap block, so they survive a menu
    bool8 truncated:1;
    bool8 atSpot:1;         // the stay started: dwell and the template run
    bool8 stripToward:1;    // walking into the player's map
    bool8 actionPending:1;
    bool8 justLeaving:1;
    // Beats (kept here so a heap reset can't lose them: the ambience block
    // may already be overwritten when the heap-reset hook runs).
    bool8 beatActive:1;     // a beat is running (mirrors the block's state)
    bool8 backingOff:1;     // a back-off or its wait: no beat starts
    bool8 leavePending:1;   // the stay ended while a beat ran: its leaving decision comes after it
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

// World work the walkers spread over field frames, like the heartbeat: a
// watched trainer's routine advance (one spot choice per frame) and the
// search for a trip's first edge (a slice per frame). One search at a time.
struct WalkerAdvance
{
    u8 slot;
    u8 attempt;
    u8 attemptMax;
    u8 padding;
};

// The context fields an advance reads, as the queue's first advance started
// (league state, badges and rating only change in scripts and battles, when
// the queue waits, and a league result flushes it first).
struct WalkerAdvanceContext
{
    u32 worldProgress;
    u32 provisionalMask;
    u32 risingMask;
};

enum
{
    EDGE_JOB_IDLE,
    EDGE_JOB_RUNNING,
    EDGE_JOB_DONE,
};

struct WalkerEdgeJob
{
    u8 slot;
    u8 status;
    bool8 live;         // the search is started in sJobWorkspace
    u8 padding;
    u16 source;         // the record's node and destination it answers for
    u16 dest;
    u16 edge;
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
static EWRAM_DATA u16 sRestoreMap = 0;      // the map the Continue restore is for
static EWRAM_DATA struct WalkerAdvance sAdvances[WALKER_ACTOR_COUNT] = {0};
static EWRAM_DATA u8 sAdvanceCount = 0;
static EWRAM_DATA struct WalkerAdvanceContext sAdvanceContext = {0};
static EWRAM_DATA struct WalkerEdgeJob sEdgeJob = {0};
static EWRAM_DATA void *sJobWorkspace = NULL;

#define REACT_KEY_STALE 0   // never a react key (ReactKey): the next frame checks

// Notable ambience (spec: notable-ambience.md): every beat's state lives in
// one heap block, allocated while walkers exist and dropped (never freed) on
// heap resets, so the beats cost no EWRAM: the mechanics-test build has
// almost none left. Only the latches live in the actors (they must survive
// a menu). The debug block comes first: verify.py reads it at sAmbience.
struct WalkerAmbience
{
    struct WalkerAmbienceDebug debug;
    struct AmbienceWalker select[WALKER_ACTOR_COUNT];
    struct WalkerBeatRun run[WALKER_ACTOR_COUNT];
    u8 tickFrames[WALKER_ACTOR_COUNT];      // frames into the current ambience tick
    u8 adjacentTicks[WALKER_ACTOR_COUNT];   // ticks the player has stood adjacent, facing, not pushing
    bool8 keepWalking[WALKER_ACTOR_COUNT];  // the running beat runs alongside the walk (hum)
    // The one companion on the map (global: one per map).
    u8 companionOwner;      // the actor whose beat brought it out, or NO_ACTOR
    u8 companionObject;     // its gObjectEvents index
    u16 companionGfx;       // OBJ_EVENT_MON + species: the object must still show it
    u16 companionSheet;     // a sprite sheet tag "companion out" preloaded, TAG_NONE when none
    u16 companionPlan[WALKER_ACTOR_COUNT];  // the graphics the last room check chose, per actor
    // Frame cost (no beat may add a lag frame).
    u32 needFacts[AMBIENCE_CLASS_REACT + 1];    // the facts the rows of this class and above read
    u32 reactKey[WALKER_ACTOR_COUNT];       // the last react check's inputs (ReactKey), per actor
    u32 pendingFrame;       // when pendingCount was counted (FollowerFreeSlots)
    u16 pendingNode;        // and for which map
    u8 pendingCount;
    u8 pendingLocals;
    u8 decisionWait[WALKER_ACTOR_COUNT];    // frames a full decision has waited for a frame with room
    u8 reactWait[WALKER_ACTOR_COUNT];       // react samples a changed key has waited for a frame with room
    u8 lateFrames[WALKER_ACTOR_COUNT];      // frames in a row the running beat sat out (a late frame)
    u8 spawnDeferred;       // spawn periods a spawn attempt was put off for want of room (BusyWorkWaits)
    u8 followerDeferred;    // the same for the follower rule
    u8 storyDeferred;       // and for the story scene check and the slot cull
    u8 stepDecided;         // bit per actor: the step decision at the current boundary was made (a
                            // beat it started held the walker there): not counted again
    bool8 frameClaimed;     // a full decision or a costly beat start took this frame
    bool8 reactChecked;     // a react check ran this frame
    bool8 pendingValid;
};

static EWRAM_DATA struct WalkerAmbience *sAmbience = NULL;

STATIC_ASSERT(sizeof(struct WayfarerWalkerActorDebug) == 16, WalkerActorDebugSize);
STATIC_ASSERT(sizeof(struct WayfarerWalkersDebug) == 64 + 16 * WALKER_ACTOR_COUNT + 40, WalkersDebugSize);
STATIC_ASSERT(DIR_NORTHEAST <= 15, WalkerMoveFitsANibble);
STATIC_ASSERT(WALKER_MAX_TILES < STATE_LAYER, WalkerTileFitsAState);
// The ambience pointer is paid for by packing the actors (176 bytes before).
STATIC_ASSERT(sizeof(struct WalkerActor) * WALKER_ACTOR_COUNT + sizeof(struct WalkerAmbience *) <= 176, WalkerActorsNoLarger);
STATIC_ASSERT(WALKER_ACTOR_COUNT <= 4, AmbienceGreetedBitsPerActor);
// AmbienceFrame's react samples (ReactPhase: odd frames) never meet a busy one.
STATIC_ASSERT(WALKER_SPAWN_PERIOD == 2 * WALKER_ACTOR_COUNT && STORY_PHASE % 2 == 0 && FOLLOWER_PHASE % 2 == 0,
              ReactChecksAvoidBusyFrames);  // AMBIENCE_LATCH_GREETED_SHIFT: 4 bits

// DIR_SOUTH..DIR_EAST are 1..4; the stair diagonals DIR_SOUTHWEST..DIR_NORTHEAST 5..8.
static const s8 sDx[9] = {0, 0, 0, -1, 1, -1, 1, -1, 1};
static const s8 sDy[9] = {0, 1, -1, 0, 0, 1, 1, -1, -1};
// The walking command for each move: a diagonal is a west or east step on stairs.
static const u8 sMoveCommand[9] = {DIR_NONE, DIR_SOUTH, DIR_NORTH, DIR_WEST, DIR_EAST, DIR_WEST, DIR_EAST, DIR_WEST, DIR_EAST};
static const u8 sOpposite[5] = {DIR_NONE, DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST};

static void RequestSearch(struct WalkerActor *actor);
static void ChooseGoal(struct WalkerActor *actor, struct ObjectEvent *obj);
static void InterruptBeat(struct WalkerActor *actor);
static void ForgetBeat(struct WalkerActor *actor);
static void EndKeepWalkingBeat(struct WalkerActor *actor);
static bool8 IsStepDecided(const struct WalkerActor *actor);
static void SetStepDecided(const struct WalkerActor *actor, bool8 decided);
static void InitAmbienceFor(u8 index);
static bool8 DecideBeat(struct WalkerActor *actor, struct ObjectEvent *obj, u8 decision, u32 facts);
static s16 FollowerFreeSlots(u8 *wantedOut);

// ---------------------------------------------------------------------------
// Small helpers

static u32 ScanlineStamp(void)
{
    u32 first, second;
    u16 vcount, pending;
    // Line 160 starts VBlank, but the interrupt that counts the frame can
    // come a little later (it does in SkyEmu): a stamp read there would be a
    // whole frame (228 lines) early. Wait that line out (at most one line).
    do
    {
        first = *(volatile u32 *)&gMain.vblankCounter1;
        vcount = REG_VCOUNT;
        pending = REG_IF & INTR_FLAG_VBLANK;
        second = *(volatile u32 *)&gMain.vblankCounter1;
    } while (first != second || vcount == 160);
    // Later in VBlank with the interrupt still not taken (interrupts off):
    // the counter is one behind.
    if (pending && vcount > 160)
        first++;
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

static EWRAM_DATA u32 sLastUpdateScanlines = 0;
static EWRAM_DATA u32 sLastUpdateVblank = 0;

// This frame's update only: a map load's frame (a warp) has none.
u32 WayfarerWalkers_LastUpdateScanlines(void)
{
    return sLastUpdateVblank == gMain.vblankCounter1 ? sLastUpdateScanlines : 0;
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

static void CancelEdgeJob(void);

static void ForgetActor(struct WalkerActor *actor)
{
    ForgetBeat(actor);
    AbortSearchFor(actor);
    if (actor->mode != WALKER_MODE_NONE && sEdgeJob.status != EDGE_JOB_IDLE && sEdgeJob.slot == actor->slot)
        CancelEdgeJob();
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
    u8 i, index = ActorIndex(actor);

    memset(actor, 0, sizeof(*actor));
    actor->mode = WALKER_MODE_LOCAL;
    actor->slot = slot;
    actor->objectId = objectId;
    actor->phase = WALKER_PHASE_PLAN;
    actor->goalEdge = NO_EDGE;
    // A new walker in this actor: nobody has greeted it yet.
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
        sActors[i].ambienceLatches &= ~(1 << (AMBIENCE_LATCH_GREETED_SHIFT + index));
    if (sAmbience != NULL)
        InitAmbienceFor(index);
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
    case WORLD_ARRIVAL_WATER:
        // Landed from across the water: on the shore tile of the way back.
        if (crossing < node->edgeCount)
        {
            const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[node->firstEdge + crossing];
            if (edge->kind == WORLD_EDGE_WATER)
                return NearestSpawnTile(edge->a, edge->b, 0, WALKER_NEAR_RADIUS, x, y);
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
    if (actor->mode == WALKER_MODE_LEAVING)
    {
        RemoveActor(actor);     // no way out: vanish in place
        return;
    }
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

static void SearchSlice(u32 limit)
{
    struct WalkerActor *actor = &sActors[sSearch.actor];
    struct ObjectEvent *obj = ActorObject(actor);
    u16 width = MapWidth(), height = MapHeight();
    u32 start = ScanlineStamp(), elapsed;
    u8 expanded = 0;

    while (sSearch.read != sSearch.write && expanded < WALKER_SEARCH_SLICE
        && (expanded == 0 || ScanlinesSince(start) < limit))
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
// spent: scanlines this frame's world job already took of the shared slice.
// The lines a slice may still take this frame: its own cap, and no more than
// the frame has left before its end margin (WayfarerWalkers_FrameLinesLeft):
// the walkers' update runs 110 to 190 lines into a busy frame, so a full
// slice there would overrun it. A slice always makes one step of progress.
static u32 SliceLimit(u32 cap, u32 spent)
{
    u32 left = WayfarerWalkers_FrameLinesLeft();
    cap = spent < cap ? cap - spent : 0;
    return left < cap ? left : cap;
}

static void RunScheduler(u32 spent)
{
    u8 i;
    if (spent >= WALKER_SLICE_SCANLINES)
        return;
    if (sSearch.actor == NO_ACTOR)
    {
        for (i = 0; i < WALKER_ACTOR_COUNT; i++)
        {
            if ((sActors[i].mode == WALKER_MODE_LOCAL || sActors[i].mode == WALKER_MODE_LEAVING)
             && sActors[i].phase == WALKER_PHASE_QUEUED
             && sActors[i].stepKind == STEP_NONE)
                break;
        }
        if (i == WALKER_ACTOR_COUNT || !EnsureWorkspace())
            return;
        BeginSearch(&sActors[i]);
        if (sSearch.actor == NO_ACTOR)
            return;
    }
    SearchSlice(SliceLimit(WALKER_SLICE_SCANLINES, spent));
}

static void RequestSearch(struct WalkerActor *actor)
{
    AbortSearchFor(actor);
    actor->phase = WALKER_PHASE_QUEUED;
}

// ---------------------------------------------------------------------------
// World jobs: a watched trainer's routine advance and a trip's first edge,
// spread over field frames so no frame carries a long search or several
// spot choices (the heartbeat's rule). The heartbeat never runs alongside
// them: the walkers' AI waits while one is pending, and a map load, a save
// or the league hook completes the advances first (FlushWorldJobs).

static bool8 IsAdvancing(u8 slot)
{
    u8 i;
    for (i = 0; i < sAdvanceCount; i++)
    {
        if (sAdvances[i].slot == slot)
            return TRUE;
    }
    return FALSE;
}

static u32 AdvancingMask(void)
{
    u32 mask = 0;
    u8 i;
    for (i = 0; i < sAdvanceCount; i++)
        mask |= 1u << sAdvances[i].slot;
    return mask;
}

// The routine's next step for a trainer whose stay ended while watched.
static void QueueAdvance(u8 slot)
{
    struct WayfarerWorldContext ctx;
    struct WalkerAdvance *job;

    if (IsAdvancing(slot))
        return;
    gWayfarerWalkersDebug.localAdvances++;
    if (sAdvanceCount >= ARRAY_COUNT(sAdvances))
    {
        // One per actor: never full. At once if it were.
        BuildContext(&ctx);
        WorldSim_AdvanceRoutine(State(), slot, &ctx, NULL);
        return;
    }
    if (sAdvanceCount == 0)
    {
        BuildContext(&ctx);
        sAdvanceContext.worldProgress = ctx.worldProgress;
        sAdvanceContext.provisionalMask = ctx.provisionalMask;
        sAdvanceContext.risingMask = ctx.risingMask;
    }
    job = &sAdvances[sAdvanceCount++];
    job->slot = slot;
    job->attempt = job->attemptMax = 0;
    gWayfarerWalkersDebug.worldJobs++;
}

// One spot choice of the oldest advance.
static void StepAdvance(void)
{
    struct WalkerAdvance *job = &sAdvances[0];
    struct WayfarerWorldContext ctx = {0};

    ctx.worldProgress = sAdvanceContext.worldProgress;
    ctx.provisionalMask = sAdvanceContext.provisionalMask;
    ctx.risingMask = sAdvanceContext.risingMask;
    if (!WorldSim_IsSimulated(&State()->records[job->slot])
     || WorldSim_AdvanceRoutineStep(State(), job->slot, &ctx, NULL, &job->attempt, &job->attemptMax))
    {
        sAdvanceCount--;
        memmove(&sAdvances[0], &sAdvances[1], sizeof(sAdvances[0]) * sAdvanceCount);
    }
}

static void FreeJobWorkspace(void)
{
    if (sJobWorkspace != NULL)
        Free(sJobWorkspace);
    sJobWorkspace = NULL;
}

static void CancelEdgeJob(void)
{
    sEdgeJob.status = EDGE_JOB_IDLE;
    sEdgeJob.live = FALSE;
    FreeJobWorkspace();
}

// The first edge of the actor's trip: TRUE with *edge once known (the cached
// path answers at once); FALSE while its search runs in slices, or while
// another actor's does and the cache can't answer.
static bool8 PlanNextEdge(struct WalkerActor *actor, u16 *edge)
{
    const struct WayfarerWorldRecord *record = Record(actor);
    u16 dest = WorldSim_DestNode(State(), actor->slot);

    if (sEdgeJob.status != EDGE_JOB_IDLE && sEdgeJob.slot == actor->slot)
    {
        if (sEdgeJob.status == EDGE_JOB_RUNNING)
            return FALSE;
        sEdgeJob.status = EDGE_JOB_IDLE;
        if (sEdgeJob.source == record->node && sEdgeJob.dest == dest)
        {
            *edge = sEdgeJob.edge;
            return TRUE;
        }
    }
    if (WorldSim_NextEdgeBegin(State(), actor->slot, NULL, FALSE, edge))
        return TRUE;
    // A finished answer its owner never came back for (it left, or was
    // handed off) doesn't hold the search: its path is cached anyway.
    if (sEdgeJob.status == EDGE_JOB_DONE)
        CancelEdgeJob();
    if (sEdgeJob.status != EDGE_JOB_IDLE)
        return FALSE;   // one search at a time
    if (sJobWorkspace == NULL)
        sJobWorkspace = Alloc(WorldSim_WorkspaceSize());
    if (sJobWorkspace == NULL)
    {
        actor->wait = WALKER_RETRY_FRAMES;
        return FALSE;
    }
    sEdgeJob.slot = actor->slot;
    sEdgeJob.status = EDGE_JOB_RUNNING;
    sEdgeJob.live = FALSE;
    sEdgeJob.source = record->node;
    sEdgeJob.dest = dest;
    gWayfarerWalkersDebug.worldJobs++;
    return FALSE;
}

// A slice of the running trip search, within the frame's job budget.
static void RunEdgeJob(u32 start)
{
    const struct WayfarerWorldRecord *record = &State()->records[sEdgeJob.slot];
    u16 edge = NO_EDGE;
    bool8 done = FALSE;
    u32 limit;

    if (record->node != sEdgeJob.source || WorldSim_DestNode(State(), sEdgeJob.slot) != sEdgeJob.dest)
    {
        // The question changed (it shouldn't while its actor waits): ask anew.
        sEdgeJob.source = record->node;
        sEdgeJob.dest = WorldSim_DestNode(State(), sEdgeJob.slot);
        sEdgeJob.live = FALSE;
    }
    if (sJobWorkspace == NULL)
    {
        // Lost to a heap reset: the search starts again in a new one.
        sJobWorkspace = Alloc(WorldSim_WorkspaceSize());
        sEdgeJob.live = FALSE;
        if (sJobWorkspace == NULL)
            return;
    }
    // One unit every frame (progress), more while the slice has room.
    limit = SliceLimit(WALKER_JOB_SCANLINES, 0);
    if (!sEdgeJob.live)
    {
        sEdgeJob.live = TRUE;
        done = WorldSim_NextEdgeBegin(State(), sEdgeJob.slot, sJobWorkspace, FALSE, &edge);
    }
    else
    {
        done = WorldSim_NextEdgeRun(State(), sEdgeJob.slot, sJobWorkspace, WALKER_JOB_NODES, &edge, NULL);
    }
    while (!done && ScanlinesSince(start) < limit)
        done = WorldSim_NextEdgeRun(State(), sEdgeJob.slot, sJobWorkspace, WALKER_JOB_NODES, &edge, NULL);
    if (done)
    {
        sEdgeJob.status = EDGE_JOB_DONE;
        sEdgeJob.live = FALSE;
        sEdgeJob.edge = edge;
        FreeJobWorkspace();
    }
}

// This frame's world job: one spot choice (heavy: TRUE, and the grid search
// sits this frame out), or a slice of the trip search.
// This frame's world job: one spot choice, or a slice of the trip search.
// Returns the scanlines it took; the grid search gets what is left of the
// shared slice (WALKER_SLICE_SCANLINES), and none after a spot choice.
static u32 RunWorldJobs(void)
{
    u32 start = ScanlineStamp(), cost;
    bool8 spotChoice = FALSE;

    if (sAdvanceCount != 0)
    {
        StepAdvance();
        spotChoice = TRUE;
    }
    else if (sEdgeJob.status == EDGE_JOB_RUNNING)
    {
        RunEdgeJob(start);
    }
    else
    {
        return 0;
    }
    cost = ScanlinesSince(start);
    if (cost > gWayfarerWalkersDebug.maxJobScanlines)
        gWayfarerWalkersDebug.maxJobScanlines = cost > 0xFFFF ? 0xFFFF : cost;
    // A spot choice can take most of a slice on its own: no grid search after it.
    return (spotChoice || cost >= WALKER_SLICE_SCANLINES) ? WALKER_SLICE_SCANLINES : cost;
}

void WayfarerWalkers_FlushWorldJobs(void)
{
    while (sAdvanceCount != 0)
        StepAdvance();
    // A search is only a question: whoever asked asks again.
    if (sEdgeJob.status != EDGE_JOB_IDLE)
        CancelEdgeJob();
}

static void DropWorldJobs(void)
{
    sAdvanceCount = 0;
    CancelEdgeJob();
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

// Walkers stroll (slow, 32 frames a tile) unless they have a reason to
// hurry: a walk-off, a back-off from the player, a Gym visitor just leaving
// (it must never block the entrance), a strip actor, or the step out of (or
// into) the map through a side lane. Spec: notable-ambience.md, "Pace".
static bool8 WalksNormalSpeed(const struct WalkerActor *actor, u8 kind)
{
    return actor->mode == WALKER_MODE_LEAVING || actor->mode == WALKER_MODE_STRIP
        || actor->goalKind == WALKER_GOAL_AWAY || actor->justLeaving
        || kind == STEP_EXIT_EDGE || kind == STEP_ENTER || kind == STEP_STRIP;
}

// Start one walking step: a straight one (overwrite cleared) or a stair
// diagonal (overwrite set, as GetCollisionAtCoords would for the player).
static bool8 StartWalk(struct WalkerActor *actor, struct ObjectEvent *obj, u8 move, u8 kind)
{
    u8 command = sMoveCommand[move];
    u8 action = WalksNormalSpeed(actor, kind) ? GetWalkNormalMovementAction(command) : GetWalkSlowMovementAction(command);
    obj->directionOverwrite = move > DIR_EAST ? move : DIR_NONE;
    if (ObjectEventSetHeldMovement(obj, action))
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
        return;
    }
    // The templates' emotes are beats now (grass_rustle, water_bite and
    // water_wait, chat_talk): see the ambience section.
    actor->template = WorldSim_DefaultTemplate(spot, record->activity);
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

// A handed-off walker walks to the nearest way out of its node (a map side
// or a door, away from the player) and is removed there or once out of
// view. It no longer stands for its record: the record moves on at the
// heartbeats as if the walker had vanished. With no exit, it vanishes now.
static void StartWalkOff(struct WalkerActor *actor, u16 node)
{
    struct ObjectEvent *obj = ActorObject(actor);
    const struct WayfarerWorldNode *data;
    s16 x = obj->currentCoords.x - MAP_OFFSET, y = obj->currentCoords.y - MAP_OFFSET;
    u16 e, best = NO_EDGE, bestDistance = 0xFFFF;

    InterruptBeat(actor);
    // A handed-off walker no longer plans trips: free the shared search.
    if (sEdgeJob.status != EDGE_JOB_IDLE && sEdgeJob.slot == actor->slot)
        CancelEdgeJob();

    if (node >= gWayfarerWorldNodeCount || WorldSim_NodeMap(node) != CurrentMap() || !IsVisible(obj))
    {
        RemoveActor(actor);
        return;
    }
    data = &gWayfarerWorldNodes[node];
    for (e = data->firstEdge; e < data->firstEdge + data->edgeCount; e++)
    {
        const struct WayfarerWorldEdge *edge = &gWayfarerWorldEdges[e];
        s16 ex, ey;
        u16 distance;
        switch (edge->kind)
        {
        case WORLD_EDGE_NORTH: ex = (edge->a + edge->b) / 2; ey = 0; break;
        case WORLD_EDGE_SOUTH: ex = (edge->a + edge->b) / 2; ey = MapHeight() - 1; break;
        case WORLD_EDGE_WEST:  ex = 0; ey = (edge->a + edge->b) / 2; break;
        case WORLD_EDGE_EAST:  ex = MapWidth() - 1; ey = (edge->a + edge->b) / 2; break;
        case WORLD_EDGE_WARP:
            ex = edge->a;
            ey = edge->b;
            // Never the door the player stands at.
            if (PlayerDistance(ex + MAP_OFFSET, ey + MAP_OFFSET) <= 1)
                continue;
            break;
        default:
            continue;   // transit and the like aren't walked to
        }
        distance = Distance(x, y, ex, ey);
        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = e;
        }
    }
    if (best == NO_EDGE)
    {
        RemoveActor(actor);
        return;
    }
    actor->mode = WALKER_MODE_LEAVING;
    actor->justLeaving = FALSE;
    actor->leavePending = FALSE;
    actor->atSpot = FALSE;
    actor->blocked = 0;
    actor->pushFrames = 0;
    actor->t = 0;
    SetEdgeGoal(actor, best);
}

static void EndWalkOff(struct WalkerActor *actor, bool8 walkedOff)
{
    if (walkedOff)
        gWayfarerWalkersDebug.walkOffs++;
    RemoveActor(actor);
}

// Hand off for the rest of this visit: the record stays where it is and
// moves on at the heartbeats; no new actor until the next map load.
static void YieldVisit(struct WalkerActor *actor)
{
    InterruptBeat(actor);
    if (actor->justLeaving)
    {
        FinishLeaving(actor);
        return;
    }
    sYielded |= 1u << actor->slot;
    gWayfarerWalkersDebug.handoffs++;
    StartWalkOff(actor, Record(actor)->node);
}

// A Gym visitor leaves at once: the visit ends and the record moves on.
static void FinishLeaving(struct WalkerActor *actor)
{
    struct WayfarerWorldRecord *record = Record(actor);
    const struct WayfarerWorldSpot *spot = DestSpot(record, actor->slot);
    u16 edge = NO_EDGE, node = record->node;

    InterruptBeat(actor);
    // Story scenes and slot culls run with the AI paused, possibly while the
    // map load's heartbeat is still running: then the record is left alone
    // (no forced finish, no long frame). The visit's one-heartbeat dwell
    // ends it at the next heartbeat, and the visitor isn't shown again.
    if (!WayfarerWorld_IsHeartbeatPending())
    {
        if (spot != NULL && spot->kind == WORLD_SPOT_GYM && spot->node == record->node)
            edge = ExitEdgeForWarp(record->node, spot->x, spot->y);
        if (edge != NO_EDGE)
        {
            // Out of the Gym only while the map outside has room (the
            // off-screen hop rule); the routine moves on either way.
            if (WorldSim_HopAllowed(State(), actor->slot, edge))
                WorldSim_TakeEdge(State(), actor->slot, edge, gWayfarerWorldEdges[edge].c);
            else
                sYielded |= 1u << actor->slot;  // still inside: never shown again this visit
            QueueAdvance(actor->slot);
        }
    }
    sVisitorsSeen |= 1u << actor->slot;
    gWayfarerWalkersDebug.visitorsVanished++;
    StartWalkOff(actor, node);
}

// The next map is full (the off-screen hop rule, which an exit commits like
// a hop): wait where it stands, as an off-screen trainer waits a heartbeat;
// still full on the next try, hand off and let the heartbeats route round.
static void CapacityBlocked(struct WalkerActor *actor)
{
    gWayfarerWalkersDebug.capacityWaits++;
    if (actor->capacityWaits++ != 0)
    {
        YieldVisit(actor);
        return;
    }
    actor->phase = actor->atSpot ? WALKER_PHASE_TEMPLATE : WALKER_PHASE_IDLE;
    actor->goalKind = WALKER_GOAL_NONE;
    actor->goalEdge = NO_EDGE;
    actor->wait = WALKER_UNREACHABLE_WAIT;
}

// Make room for the player: walk to an open tile away from them, wait, then
// carry on. After a few back-offs in one visit, hand off instead.
static void StartBackOff(struct WalkerActor *actor)
{
    struct ObjectEvent *player = Player();
    InterruptBeat(actor);
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
    actor->backingOff = TRUE;   // until the next plan (ChooseGoal)
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

    actor->backingOff = FALSE;  // a back-off's wait is over
    // The routine's next step is still being chosen (a spot choice a frame).
    if (IsAdvancing(actor->slot))
        return;
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
    // A trip's first search can expand hundreds of nodes: it runs a slice a
    // frame (RunWorldJobs) while the actor waits here.
    if (!PlanNextEdge(actor, &edge))
        return;
    if (edge == NO_EDGE)
    {
        // No path within the bound: stand still; the heartbeat moves the
        // record on once the player has left.
        actor->phase = WALKER_PHASE_IDLE;
        actor->wait = WALKER_UNREACHABLE_WAIT << WALKER_FAILS_MAX;
        return;
    }
    if (gWayfarerWorldEdges[edge].kind == WORLD_EDGE_WATER)
    {
        // An off-screen-only link (across water): nobody is seen making it.
        // Hand off and walk out of view; the record crosses at a heartbeat.
        YieldVisit(actor);
        return;
    }
    if (!WorldSim_HopAllowed(State(), actor->slot, edge))
    {
        CapacityBlocked(actor);
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
    if (actor->mode == WALKER_MODE_LEAVING)
    {
        // Leaving and still in the way: vanish rather than block.
        if (byPlayer || ++actor->blocked >= WALKER_BLOCKED_LIMIT)
            RemoveActor(actor);
        else
            RequestSearch(actor);
        return;
    }
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

    if (actor->mode == WALKER_MODE_LEAVING)
    {
        // At the way out (the lane or beside the door): gone.
        EndWalkOff(actor, TRUE);
        return;
    }
    switch (actor->goalKind)
    {
    case WALKER_GOAL_TILE:
        EndKeepWalkingBeat(actor);
        if (record->state == WORLD_STATE_TRAVELLING && WorldSim_DestNode(State(), actor->slot) == record->node)
        {
            WorldSim_Arrive(State(), actor->slot);
            gWayfarerWalkersDebug.arrivals++;
            actor->atSpot = FALSE;
        }
        if (!actor->atSpot)
        {
            // The stay starts: the template, then the arrival's decision point.
            actor->atSpot = TRUE;
            actor->frames = actor->dwellTicks = 0;
            StartTemplate(actor, obj);
            DecideBeat(actor, obj, AMBIENCE_DECIDE_ARRIVE, AMBIENCE_FACT_ARRIVING);
            break;
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
        // The map beyond may have filled while it walked to the lane.
        if (!WorldSim_HopAllowed(State(), actor->slot, actor->goalEdge))
        {
            CapacityBlocked(actor);
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
        bool8 allowed;
        if (door ? GetObjectEventIdByXY(wx, wy) != OBJECT_EVENTS_COUNT || IsPlayerTile(wx, wy)
                 : ProbeFromHere(obj, dir) != dir)
        {
            OnBlocked(actor, IsPlayerAhead(obj, dir));
            return;
        }
        // A Gym visitor always walks out (it must never block the
        // entrance); anyone else waits while the map beyond is full.
        allowed = WorldSim_HopAllowed(State(), actor->slot, edgeIndex);
        if (!allowed && !actor->justLeaving)
        {
            CapacityBlocked(actor);
            return;
        }
        if (!StartWalk(actor, obj, dir, STEP_EXIT_WARP))
            return;
        if (allowed)
        {
            WorldSim_TakeEdge(State(), actor->slot, edgeIndex, gWayfarerWorldEdges[edgeIndex].c);
            NoteEntry(actor->slot, edgeIndex, gWayfarerWorldEdges[edgeIndex].c);
        }
        else
        {
            // The visitor walked out but its record stays inside (the map
            // beyond is full) with a new destination: no longer a Gym visit,
            // so mark it handed off, or it would respawn in view.
            sYielded |= 1u << actor->slot;
        }
        // A Gym visit ends as the visitor walks out: the record moves on
        // rather than heading back to the Gym (a spot choice per frame).
        if (actor->justLeaving)
            QueueAdvance(actor->slot);
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
        bool8 byPlayer = move >= DIR_SOUTH && move <= DIR_NORTHEAST && IsPlayerAhead(obj, sMoveCommand[move]);
        // Blocked by an object that isn't the player (not a wall): a react
        // decision with the blocked fact (blocked_sigh).
        if (!byPlayer && move >= DIR_SOUTH && move <= DIR_NORTHEAST
         && GetObjectEventIdByXY(obj->currentCoords.x + sDx[move], obj->currentCoords.y + sDy[move]) != OBJECT_EVENTS_COUNT)
            DecideBeat(actor, obj, AMBIENCE_DECIDE_REACT, AMBIENCE_FACT_BLOCKED);
        OnBlocked(actor, byPlayer);
        return;
    }
    // A step boundary: a decision point (a beat that stops the walk takes
    // this frame; hum walks on). Once per boundary: the walk resuming here
    // after that beat isn't another one (s and steps-since-beat count steps).
    if (!IsStepDecided(actor) && DecideBeat(actor, obj, AMBIENCE_DECIDE_STEP, 0))
    {
        SetStepDecided(actor, TRUE);
        return;
    }
    if (!StartWalk(actor, obj, move, STEP_WALK))
        return;
    SetStepDecided(actor, FALSE);
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
        // Out of the map: keep walking out of view in the connection strip
        // (strip actors run no beats).
        EndKeepWalkingBeat(actor);
        actor->mode = WALKER_MODE_STRIP;
        actor->blocked = 0;
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

// A strip actor walks straight only. Blocked straight ahead (a 1-wide lane
// whose next tile is a wall, or the player standing in it), it would stand
// in the seam, maybe across the player's way: after a moment it goes. Its
// record was handed off when it stepped out (or stays beyond the seam).
static void StripBlocked(struct WalkerActor *actor)
{
    if (++actor->blocked < WALKER_STRIP_BLOCKED_FRAMES)
        return;
    gWayfarerWalkersDebug.stripTimeouts++;
    RemoveActor(actor);
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
        // No lane here, or the player's map is full (the hop rule): it
        // turns round and walks out of view instead of stepping in.
        if (edge == NO_EDGE || !WorldSim_HopAllowed(State(), actor->slot, edge))
        {
            actor->stripToward = FALSE;
            return;
        }
        if (ProbeFromHere(obj, dir) != dir)
        {
            StripBlocked(actor);
            return;
        }
        if (!StartWalk(actor, obj, dir, STEP_ENTER))
            return;
        actor->blocked = 0;
        WorldSim_TakeEdge(State(), actor->slot, edge, coord);
        gWayfarerWalkersDebug.stripEntries++;
        return;
    }

    // Past the loaded grid there is no collision data: keep walking until
    // the sprite has left the viewport.
    if (nx >= 0 && ny >= 0 && nx < gBackupMapLayout.width && ny < gBackupMapLayout.height
     && ProbeFromHere(obj, dir) != dir)
    {
        StripBlocked(actor);
        return;
    }
    if (StartWalk(actor, obj, dir, STEP_STRIP))
        actor->blocked = 0;
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
        InterruptBeat(actor);   // the map changed
        if (actor->mode == WALKER_MODE_LEAVING)
        {
            RemoveActor(actor);
            continue;
        }
        if (WayfarerWorld_IsSlotPending(actor->slot))
        {
            // Released to the heartbeat at the transition (off the new map
            // and out of view): it would be dropped below whether or not its
            // record has hopped yet, so drop it without waiting for that.
            RemoveActor(actor);
            continue;
        }
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
        actor->blocked = 0;
        actor->stripToward = heading;
        actor->phase = WALKER_PHASE_WALK;
        gWayfarerWalkersDebug.rebases++;
    }
}

// The player standing against a walker (adjacent, facing it).
static bool8 IsPlayerPushing(const struct ObjectEvent *obj)
{
    struct ObjectEvent *player = Player();
    s16 px = player->currentCoords.x, py = player->currentCoords.y;
    if (Distance(px, py, obj->currentCoords.x, obj->currentCoords.y) != 1)
        return FALSE;
    return player->facingDirection == DirectionTowards(px, py, obj->currentCoords.x, obj->currentCoords.y);
}

// Pushing for real: facing the walker with that direction held.
static bool8 IsPlayerPressingInto(const struct ObjectEvent *obj)
{
    static const u16 sDirKeys[5] = {0, DPAD_DOWN, DPAD_UP, DPAD_LEFT, DPAD_RIGHT};
    u8 facing = Player()->facingDirection;
    return IsPlayerPushing(obj) && facing >= DIR_SOUTH && facing <= DIR_EAST && (gMain.heldKeys & sDirKeys[facing]);
}

// ---------------------------------------------------------------------------
// Notable ambience: context, decision points and interruptions
// (spec: notable-ambience.md, "Context", "Selection", "Interruptions"). The
// selection is wayfarer_ambience.c's; the primitives run in
// wayfarer_walker_beats.c. Only local actors (not Gym visitors just leaving)
// run beats. While a beat runs the walker keeps its yield checks and its
// dwell clock, but takes no template move, step or plan (searches still
// finish in the background); a keep-walking beat (hum) runs alongside.

#define COMPANION_LOCALID WALKER_COMPANION_LOCALID

static void InitAmbienceFor(u8 index)
{
    Ambience_InitWalker(&sAmbience->select[index]);
    // A bow the actor's last beat left to a script would be lost with the
    // reset (with the field locked it stays the script's).
    if (!ArePlayerFieldControlsLocked())
        WalkerBeats_ReleaseNpc(&sAmbience->run[index]);
    memset(&sAmbience->run[index], 0, sizeof(sAmbience->run[index]));
    sAmbience->stepDecided &= ~(1 << index);
    sAmbience->lateFrames[index] = 0;
    sAmbience->tickFrames[index] = 0;
    sAmbience->adjacentTicks[index] = 0;
    sAmbience->keepWalking[index] = FALSE;
    sAmbience->debug.running[index] = AMBIENCE_BEAT_NONE;
    sAmbience->reactKey[index] = REACT_KEY_STALE;   // the first frame checks
    sAmbience->decisionWait[index] = 0;
    sAmbience->reactWait[index] = 0;
}

// The block, allocated on demand (retried next frame if the heap is full:
// beats just don't run meanwhile).
static bool8 EnsureAmbience(void)
{
    u8 i;
    if (sAmbience != NULL)
        return TRUE;
    sAmbience = AllocZeroed(sizeof(*sAmbience));
    if (sAmbience == NULL)
        return FALSE;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
        InitAmbienceFor(i);
    sAmbience->companionOwner = NO_ACTOR;
    sAmbience->companionObject = OBJECT_EVENTS_COUNT;
    sAmbience->companionSheet = TAG_NONE;
    // The facts each decision's candidate rows read (react checks read only
    // the react rows', the others add the transition rows', and the idle
    // rows' once an idle beat can win): the context gathers only those.
    for (i = 0; i < gWayfarerAmbienceBeatCount; i++)
    {
        const struct WayfarerAmbienceBeat *row = &gWayfarerAmbienceBeats[i];
        u8 cls;
        for (cls = AMBIENCE_CLASS_IDLE; cls <= row->cls && cls <= AMBIENCE_CLASS_REACT; cls++)
            sAmbience->needFacts[cls] |= row->all | row->any;
    }
    return TRUE;
}

static bool8 BeatRunning(const struct WalkerActor *actor)
{
    return sAmbience != NULL && sAmbience->select[ActorIndex(actor)].beat != AMBIENCE_BEAT_NONE;
}

// A running beat that stops the walker (all but the keep-walking ones).
static bool8 BeatHoldsWalker(const struct WalkerActor *actor)
{
    return BeatRunning(actor) && !sAmbience->keepWalking[ActorIndex(actor)];
}

// The actor's object while it is still the actor's (NULL once the engine dropped it).
static struct ObjectEvent *OwnObject(const struct WalkerActor *actor)
{
    struct ObjectEvent *obj = ActorObject(actor);
    return (obj->active && obj->localId == WALKER_LOCALID_BASE + ActorIndex(actor)) ? obj : NULL;
}

// Frame cost. The field frame reaches the walkers 110 to 190 scanlines in
// (SkyEmu, a busy outdoor map), and the rest of the frame after them takes
// 16 to 56: most frames have 20 to 40 lines to spare. So costly work (a
// decision where an idle beat can win, an icon, a field effect, the
// companion) takes a frame only with room left for it, one per frame, never
// the walkers' busy frames (IsBusyFrame), and waits a bounded time for one.
#define FRAME_END_MARGIN        64  // the rest of the field frame: 16 to 56 lines (wayfarer_world.c
                                    // keeps 40 for its slices; beats are optional, so more here)
#define DECISION_LINES          24  // a full decision: the whole pool and maybe companion_room
#define DECISION_WAIT_FRAMES    60  // a dwell tick
#define REACT_LINES             12  // a react check: the context and the react rows
#define BEAT_LATE_FRAMES        4   // a running beat sits out at most this many late frames in a row
#define REACT_WAIT_SAMPLES      8   // react samples (one per walker every 8 frames) a check waits for room

// The busy frames: the walkers' own (spawns, story scenes and culls, the
// follower rule) and the engine's periodic time-of-day update, which
// re-blends the palettes after this hook (OverworldBasic), and the frame
// after it (the re-blend's weather work: the heaviest of the two).
static bool8 IsBusyFrame(void)
{
    return sSpawnTimer == 0 || sSpawnTimer == STORY_PHASE || sSpawnTimer == FOLLOWER_PHASE || gTimeUpdateCounter <= 1
        || gTimeUpdateCounter >= (s16)(SECONDS_PER_MINUTE * 60 / FakeRtc_GetSecondsRatio());
}

// The next frame is a busy one (a beat start's sprite spills into it).
bool8 WayfarerWalkers_NextFrameBusy(void)
{
    u8 next = sSpawnTimer + 1 >= WALKER_SPAWN_PERIOD ? 0 : sSpawnTimer + 1;
    return next == 0 || next == STORY_PHASE || next == FOLLOWER_PHASE || gTimeUpdateCounter <= 2;
}

u16 WayfarerWalkers_FrameLinesLeft(void)
{
    u16 line;
    // Past this frame's VBlank already: none.
    if (gMain.vblankCounter1 != sLastUpdateVblank)
        return 0;
    line = (REG_VCOUNT + 68) % 228;
    return line + FRAME_END_MARGIN >= 228 ? 0 : 228 - FRAME_END_MARGIN - line;
}

bool8 WayfarerWalkers_ClaimFrame(u16 lines, bool8 force)
{
    if (sAmbience == NULL || sAmbience->frameClaimed || IsBusyFrame())
        return FALSE;
    if (!force && WayfarerWalkers_FrameLinesLeft() < lines)
        return FALSE;
    sAmbience->frameClaimed = TRUE;
    return TRUE;
}

static void LogBeat(const struct WalkerActor *actor, u8 beat, u8 event)
{
    struct WalkerAmbienceDebug *debug = &sAmbience->debug;
    struct WalkerBeatLog *entry = &debug->log[debug->logCount % WALKER_BEAT_LOG_SIZE];
    entry->frame = gWayfarerWalkersDebug.frames;
    entry->actor = ActorIndex(actor);
    entry->slot = actor->slot;
    entry->beat = beat;
    entry->event = event;
    debug->logCount++;
}

// ---------------------------------------------------------------------------
// The ace companion (spec: notable-ambience.md, "Companion"). One per map,
// owned by the actor whose beat brought it out, held in the ambience block
// (owner, object, graphics) and found by its dynamic local id otherwise. It
// is the lowest-priority object: it only comes out with room to spare
// (companion_room), and it goes away at once when a map object, a walker or
// the following Pokemon needs the room, when its object vanishes, or when its
// beat ends or is interrupted. Only "companion out", "companion in" and the
// beat's end touch it from the runner.

#define FOLLOWER_FREE_SLOTS     2   // the follower rule hides it below this many free slots
#define COMPANION_SPARE_PALETTES 1  // palettes kept free besides the companion's own (the beat's icons)

// The object view the engine keeps (RemoveObjectEventIfOutsideView): a
// companion outside it would be culled at once.
static bool8 IsInObjectView(s16 x, s16 y)
{
    s16 px = gSaveBlock1Ptr->pos.x, py = gSaveBlock1Ptr->pos.y;
    return x >= px - 2 && x <= px + 17 && y >= py && y <= py + 16;
}

static u8 FreeSpritePalettes(void)
{
    u8 i, count = 0;
    for (i = gReservedSpritePaletteCount; i < 16; i++)
    {
        if (GetSpritePaletteTagByPaletteNum(i) == TAG_NONE)
            count++;
    }
    return count;
}

// The live companion object, or NULL (none out, or its object is gone).
static struct ObjectEvent *CompanionObject(void)
{
    struct ObjectEvent *obj;
    if (sAmbience == NULL || sAmbience->companionOwner == NO_ACTOR || sAmbience->companionObject >= OBJECT_EVENTS_COUNT)
        return NULL;
    obj = &gObjectEvents[sAmbience->companionObject];
    if (!obj->active || obj->localId != COMPANION_LOCALID || obj->graphicsId != sAmbience->companionGfx)
        return NULL;
    return obj;
}

// The map's one companion is taken: reserved by a companion beat as it
// starts (so a second walker's decision finds it busy), or out.
static bool8 IsCompanionOut(void)
{
    return sAmbience != NULL && sAmbience->companionOwner != NO_ACTOR;
}

// Its object was spawned (it may have vanished since: CompanionObject).
static bool8 IsCompanionSpawned(void)
{
    return IsCompanionOut() && sAmbience->companionObject < OBJECT_EVENTS_COUNT;
}

static void ReleaseCompanionSheet(void);

// Puts the companion away: removes its object (only while it is still the
// companion's) and frees the global state; a reservation that never spawned
// just ends. Called with live field sprites.
static void PutCompanionAway(bool8 early, u8 reason)
{
    struct ObjectEvent *obj;
    if (!IsCompanionOut())
        return;
    if (IsCompanionSpawned())
    {
        obj = CompanionObject();
        if (obj != NULL)
            RemoveObjectEvent(obj);
        if (early)
            sAmbience->debug.companionAway[reason]++;
        else
            sAmbience->debug.companionsIn++;
    }
    else
    {
        ReleaseCompanionSheet();
    }
    sAmbience->companionOwner = NO_ACTOR;
    sAmbience->companionObject = OBJECT_EVENTS_COUNT;
}

// Puts the spawned companion away before its beat does, and interrupts that beat.
static void SendCompanionAway(u8 reason)
{
    u8 owner;
    if (!IsCompanionSpawned())
        return;
    owner = sAmbience->companionOwner;
    PutCompanionAway(TRUE, reason);
    if (owner < WALKER_ACTOR_COUNT && sActors[owner].mode != WALKER_MODE_NONE)
        InterruptBeat(&sActors[owner]);
}

// The field is being torn down (a heap reset, a Continue): sprite calls are
// unsafe (the sprites may already belong to a menu or a battle), so any
// object with the companion's local id is just switched off. Nothing
// respawns an inactive object, and the field's reload clears its sprite and
// palette.
static void DropCompanionObjects(void)
{
    u8 i;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (gObjectEvents[i].active && gObjectEvents[i].localId == COMPANION_LOCALID)
        {
            gObjectEvents[i].active = FALSE;
            gObjectEvents[i].graphicsId = 0;
        }
    }
    if (sAmbience != NULL)
    {
        sAmbience->companionOwner = NO_ACTOR;
        sAmbience->companionObject = OBJECT_EVENTS_COUNT;
        sAmbience->companionSheet = TAG_NONE;  // the reload frees every sheet
    }
}

// A sheet preloaded for "companion out" goes again unless a sprite uses it
// (the spawned companion does; its removal frees it as usual).
static void ReleaseCompanionSheet(void)
{
    u16 tileStart;
    if (sAmbience == NULL || sAmbience->companionSheet == TAG_NONE)
        return;
    tileStart = GetSpriteTileStartByTag(sAmbience->companionSheet);
    if (tileStart != TAG_NONE)
        FieldEffectFreeTilesIfUnused(tileStart);
    sAmbience->companionSheet = TAG_NONE;
}

// The tiles beside the walker, in the order a companion takes them: its
// sides first, then behind it, then ahead.
static const u8 sCompanionDirs[5][4] = {
    [DIR_NONE]  = {DIR_WEST, DIR_EAST, DIR_SOUTH, DIR_NORTH},
    [DIR_SOUTH] = {DIR_WEST, DIR_EAST, DIR_NORTH, DIR_SOUTH},
    [DIR_NORTH] = {DIR_WEST, DIR_EAST, DIR_SOUTH, DIR_NORTH},
    [DIR_WEST]  = {DIR_NORTH, DIR_SOUTH, DIR_EAST, DIR_WEST},
    [DIR_EAST]  = {DIR_NORTH, DIR_SOUTH, DIR_WEST, DIR_EAST},
};

// A free tile with nobody on it, not the player's (MAP_OFFSET coords).
static bool8 IsEmptyTile(s16 x, s16 y)
{
    return IsStandable(x, y) && GetObjectEventIdByXY(x, y) == OBJECT_EVENTS_COUNT && !IsPlayerTile(x, y);
}

// A 64x64 follower sprite stands bottom-aligned and centred on its tile,
// overhanging the tiles beside it and three rows above. Its "2x2 area" here:
// it only stands west or east of the walker (never above or below, where it
// would cover the walker), and its tile, the tile beyond it away from the
// walker and the two tiles north of those must all be free.
static bool8 HasBigArea(s16 x, s16 y, u8 dir)
{
    s16 bx = x + sDx[dir];
    if (dir != DIR_WEST && dir != DIR_EAST)
        return FALSE;
    return IsEmptyTile(x, y - 1) && IsEmptyTile(bx, y) && IsEmptyTile(bx, y - 1);
}

// companion_room (spec, "Companion"): WALKER_COMPANION_DENY_COUNT when it
// holds, with the tile (MAP_OFFSET coords) and the graphics of the first
// candidate that fits; otherwise the first condition that failed. onlyGfx:
// 0, or the one candidate (by graphics) to try.
static u8 CompanionRoom(const struct WalkerActor *actor, struct ObjectEvent *obj, s16 *outX, s16 *outY, u16 *outGfx,
                        u16 onlyGfx)
{
    const struct WayfarerAmbienceTrainer *trainer = &gWayfarerAmbienceTrainers[actor->slot];
    const struct WayfarerWorldSpot *spot;
    struct ObjectEvent *player = Player();
    s16 aheadX = player->currentCoords.x, aheadY = player->currentCoords.y;
    u8 i, j, tiles = 0, tested = 0, facing = obj->facingDirection;
    bool8 outdoors;

    if (!actor->atSpot || actor->phase != WALKER_PHASE_TEMPLATE || actor->mode != WALKER_MODE_LOCAL)
        return WALKER_COMPANION_DENY_PLACE;
    spot = DestSpot(Record(actor), actor->slot);
    if (spot == NULL || spot->kind == WORLD_SPOT_STORE || spot->kind == WORLD_SPOT_GAME_CORNER
     || spot->kind == WORLD_SPOT_CENTER_COUNTER || spot->kind == WORLD_SPOT_CENTER_SIDE)
        return WALKER_COMPANION_DENY_PLACE;
    if (IsCompanionSpawned() || (IsCompanionOut() && sAmbience->companionOwner != ActorIndex(actor)))
        return WALKER_COMPANION_DENY_BUSY;
    // After spawning it the walker rule's free slots must remain.
    if (FreeObjectSlots() < SPAWN_FREE_SLOTS + 1)
        return WALKER_COMPANION_DENY_SLOTS;
    // The follower rule ignores the companion (it gives way instead): with
    // it counted, the rule must still keep the follower out.
    if (FollowerFreeSlots(NULL) - 1 < FOLLOWER_FREE_SLOTS)
        return WALKER_COMPANION_DENY_FOLLOWER;
    // Its dynamic palette must load (a full table isn't handled: an out-of-
    // bounds write), and the beat's icon needs one more.
    if (FreeSpritePalettes() < 1 + COMPANION_SPARE_PALETTES)
        return WALKER_COMPANION_DENY_PALETTE;

    // The free tiles beside the walker it may stand on (bits by order),
    // each worked out only when a candidate gets to it: standable by the
    // walker's own step rules (elevation included), in the engine's object
    // view, not the player's tile (current or previous) nor the tile
    // straight ahead of the player.
    if (player->facingDirection >= DIR_SOUTH && player->facingDirection <= DIR_EAST)
    {
        aheadX += sDx[player->facingDirection];
        aheadY += sDy[player->facingDirection];
    }
    if (facing > DIR_EAST)
        facing = DIR_NONE;
    outdoors = IsMapTypeOutdoors(gMapHeader.mapType);
    // The first candidate that fits; a big one (outdoors only, as the engine
    // hides 64x64 followers indoors) needs its 2x2 area.
    for (i = 0; i < trainer->companionCount && i < AMBIENCE_COMPANION_MAX; i++)
    {
        u16 entry = trainer->companions[i];
        bool8 big = (entry & AMBIENCE_COMPANION_BIG) != 0;
        if ((entry & AMBIENCE_COMPANION_SPECIES_MASK) == SPECIES_NONE || (big && !outdoors)
         || (onlyGfx != 0 && (entry & AMBIENCE_COMPANION_SPECIES_MASK) + OBJ_EVENT_MON != onlyGfx))
            continue;
        for (j = 0; j < 4; j++)
        {
            u8 dir = sCompanionDirs[facing][j];
            s16 x = obj->currentCoords.x + sDx[dir], y = obj->currentCoords.y + sDy[dir];
            if (!(tested & (1 << j)))
            {
                tested |= 1 << j;
                if ((x != aheadX || y != aheadY) && IsInObjectView(x, y) && WayfarerWalkers_BeatCanStep(obj, dir))
                    tiles |= 1 << j;
            }
            if (!(tiles & (1 << j)) || (big && !HasBigArea(x, y, dir)))
                continue;
            *outX = x;
            *outY = y;
            *outGfx = (entry & AMBIENCE_COMPANION_SPECIES_MASK) + OBJ_EVENT_MON;
            return WALKER_COMPANION_DENY_COUNT;
        }
    }
    return WALKER_COMPANION_DENY_TILE;
}

// The actor whose object this is (a local walker), or NULL.
static struct WalkerActor *ActorOfObject(const struct ObjectEvent *obj)
{
    u8 index;
    if (obj == NULL || !WayfarerWalkers_IsActorObject(obj))
        return NULL;
    index = obj->localId - WALKER_LOCALID_BASE;
    if (sActors[index].mode == WALKER_MODE_NONE || ActorObject(&sActors[index]) != obj)
        return NULL;
    return &sActors[index];
}

// The sheet tag a spawn of these graphics looks up (LoadSheetGraphicsInfo),
// or TAG_NONE when its spawn decompresses nothing: an uncompressed sheet,
// or one already in VRAM (the player's follower, a map object).
static u16 CompanionSheetToLoad(u16 gfx)
{
    const struct ObjectEventGraphicsInfo *info;
    u16 tag;
    if (!OW_GFX_COMPRESS || gfx == 0)
        return TAG_NONE;
    info = GetObjectEventGraphicsInfo(gfx);
    if (!info->compressed && info->tileTag == TAG_NONE)
        return TAG_NONE;
    tag = info->tileTag != TAG_NONE ? info->tileTag : COMP_OW_TILE_TAG_BASE + gfx;
    return GetSpriteTileStartByTag(tag) != TAG_NONE ? TAG_NONE : tag;
}

void WayfarerWalkers_CompanionPrepare(struct ObjectEvent *obj)
{
    struct WalkerActor *actor = ActorOfObject(obj);
    u16 gfx, tag;

    // Only for its own reservation (another walker's preloaded sheet stays).
    if (!OW_GFX_COMPRESS || actor == NULL || !IsCompanionOut() || IsCompanionSpawned()
     || sAmbience->companionOwner != ActorIndex(actor))
        return;
    ReleaseCompanionSheet();
    gfx = sAmbience->companionPlan[ActorIndex(actor)];
    // Only a sheet loaded here is released again.
    tag = CompanionSheetToLoad(gfx);
    if (tag == TAG_NONE)
        return;
    sAmbience->debug.companionPrepares++;
    if (LoadSheetGraphicsInfo(GetObjectEventGraphicsInfo(gfx), gfx, NULL) == tag && GetSpriteTileStartByTag(tag) != TAG_NONE)
        sAmbience->companionSheet = tag;
}

u8 WayfarerWalkers_CompanionOut(struct ObjectEvent *obj, bool8 mayPrepare)
{
    struct WalkerActor *actor = ActorOfObject(obj);
    struct ObjectEvent *companion;
    s16 x, y;
    u16 gfx, plan;
    u8 id, elevation, room;

    if (actor == NULL || !IsCompanionOut() || IsCompanionSpawned() || sAmbience->companionOwner != ActorIndex(actor))
        return WALKER_COMPANION_OUT_FAILED;
    // The prepared species while it still fits (its sheet is in VRAM, so the
    // spawn decompresses nothing); else the first candidate that fits now.
    plan = sAmbience->companionPlan[ActorIndex(actor)];
    room = CompanionRoom(actor, obj, &x, &y, &gfx, plan);
    if (room == WALKER_COMPANION_DENY_TILE && plan != 0)
        room = CompanionRoom(actor, obj, &x, &y, &gfx, 0);
    if (room != WALKER_COMPANION_DENY_COUNT)
    {
        ReleaseCompanionSheet();
        sAmbience->debug.companionOutFailed++;
        return WALKER_COMPANION_OUT_FAILED;
    }
    if (gfx != plan && CompanionSheetToLoad(gfx) != TAG_NONE)
    {
        // Its sheet would be decompressed in this frame: it gets a prepare
        // frame of its own first.
        sAmbience->companionPlan[ActorIndex(actor)] = gfx;
        if (mayPrepare)
            return WALKER_COMPANION_OUT_PREPARE;
        ReleaseCompanionSheet();
        sAmbience->debug.companionOutFailed++;
        return WALKER_COMPANION_OUT_FAILED;
    }
    elevation = MapGridGetElevationAt(x, y);
    if (elevation == 15)
        elevation = obj->currentElevation;
    // No slot, no sprite or no VRAM: OBJECT_EVENTS_COUNT, nothing spawned.
    id = SpawnSpecialObjectEventParameterized(gfx, MOVEMENT_TYPE_NONE, COMPANION_LOCALID, x, y, elevation);
    ReleaseCompanionSheet();
    if (id >= OBJECT_EVENTS_COUNT)
    {
        sAmbience->debug.companionOutFailed++;
        return WALKER_COMPANION_OUT_FAILED;
    }
    companion = &gObjectEvents[id];
    ObjectEventTurn(companion, DirectionTowards(x, y, obj->currentCoords.x, obj->currentCoords.y));
    sAmbience->companionObject = id;
    sAmbience->companionGfx = gfx;
    sAmbience->debug.companionsOut++;
    return WALKER_COMPANION_OUT_DONE;
}

u8 WayfarerWalkers_ObjectSlot(const struct ObjectEvent *obj)
{
    struct WalkerActor *actor = ActorOfObject(obj);
    return actor != NULL ? actor->slot : SLOT_NONE;
}

void WayfarerWalkers_CompanionIn(struct ObjectEvent *obj)
{
    struct WalkerActor *actor = ActorOfObject(obj);
    if (actor != NULL && IsCompanionSpawned() && sAmbience->companionOwner == ActorIndex(actor))
        PutCompanionAway(FALSE, 0);
}

struct ObjectEvent *WayfarerWalkers_Companion(const struct ObjectEvent *obj)
{
    struct WalkerActor *actor = ActorOfObject(obj);
    if (actor == NULL || !IsCompanionOut() || sAmbience->companionOwner != ActorIndex(actor))
        return NULL;
    return CompanionObject();
}

// Every walker frame while a companion is out or reserved, controls locked
// or not: strays go (a companion object that isn't the live one), and the
// live companion gives way when its object vanished or the slots run short.
// With none out it costs nothing: every other way a companion object can
// outlive its beat (a save, a warp, a heap reset) switches it off already
// (DropCompanionObjects), and a frame of the walkers' update can tip a busy
// frame (a seam's heartbeat) into a lag frame.
static void CompanionFrame(void)
{
    u8 i, freeSlots = 0, owner;
    if (!IsCompanionOut())
        return;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *obj = &gObjectEvents[i];
        if (!obj->active)
        {
            freeSlots++;
            continue;
        }
        if (obj->localId == COMPANION_LOCALID
         && !(IsCompanionOut() && sAmbience->companionObject == i && obj->graphicsId == sAmbience->companionGfx))
        {
            RemoveObjectEvent(obj);
            freeSlots++;
            if (sAmbience != NULL)
                sAmbience->debug.companionStrays++;
        }
    }
    owner = sAmbience->companionOwner;
    if (owner >= WALKER_ACTOR_COUNT || sActors[owner].mode != WALKER_MODE_LOCAL || !BeatRunning(&sActors[owner]))
        PutCompanionAway(TRUE, WALKER_COMPANION_AWAY_INTERRUPTED);  // no beat holds it any more
    else if (!IsCompanionSpawned())
        return;     // reserved: its beat is bringing it out
    else if (CompanionObject() == NULL)
        SendCompanionAway(WALKER_COMPANION_AWAY_VANISHED);
    else if (freeSlots < SPAWN_FREE_SLOTS)
        SendCompanionAway(WALKER_COMPANION_AWAY_SLOTS);
}

// What the walker faces: water, grass, a counter, or an NPC (an object that
// isn't the player, the follower, a walker or the companion).
static u32 FacingFacts(const struct ObjectEvent *obj)
{
    u8 dir = obj->facingDirection, id, behavior;
    s16 x, y;

    if (dir < DIR_SOUTH || dir > DIR_EAST)
        return 0;
    x = obj->currentCoords.x + sDx[dir];
    y = obj->currentCoords.y + sDy[dir];
    id = GetObjectEventIdByXY(x, y);
    if (id < OBJECT_EVENTS_COUNT)
    {
        const struct ObjectEvent *other = &gObjectEvents[id];
        if (!other->isPlayer && other->localId != OBJ_EVENT_ID_FOLLOWER && !WayfarerWalkers_IsActorObject(other)
         && other->localId != COMPANION_LOCALID)
            return AMBIENCE_FACT_FACING_NPC;
    }
    behavior = MapGridGetMetatileBehaviorAt(x, y);
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) || MetatileBehavior_IsSurfableFishableWater(behavior))
        return AMBIENCE_FACT_FACING_WATER;
    if (MetatileBehavior_IsTallGrass(behavior) || MetatileBehavior_IsLongGrass(behavior))
        return AMBIENCE_FACT_FACING_GRASS;
    if (MetatileBehavior_IsCounter(behavior))
        return AMBIENCE_FACT_FACING_COUNTER;
    return 0;
}

static u8 CapDistance(u16 distance)
{
    return distance >= AMBIENCE_DISTANCE_NONE ? AMBIENCE_DISTANCE_NONE - 1 : distance;
}

#define FACING_FACTS (AMBIENCE_FACT_FACING_WATER | AMBIENCE_FACT_FACING_GRASS | AMBIENCE_FACT_FACING_NPC \
                    | AMBIENCE_FACT_FACING_COUNTER)

// The facts the candidate rows of a decision read (EnsureAmbience).
static u32 NeededFacts(u8 decision, bool8 idleCanWin)
{
    if (decision == AMBIENCE_DECIDE_REACT)
        return sAmbience->needFacts[AMBIENCE_CLASS_REACT];
    return sAmbience->needFacts[idleCanWin ? AMBIENCE_CLASS_IDLE : AMBIENCE_CLASS_TRANSITION];
}

// facts: the decision's own (ARRIVING, LEAVING, BLOCKED). need: the facts
// the candidate rows read; the costly ones (what the walker faces) are only
// gathered when they are among them.
static void BuildAmbienceContext(const struct WalkerActor *actor, const struct ObjectEvent *obj, u32 facts,
                                 struct AmbienceContext *ctx, u32 need)
{
    const struct WayfarerWorldRecord *record = Record(actor);
    bool8 dwelling = actor->atSpot && actor->phase == WALKER_PHASE_TEMPLATE;
    u8 index = ActorIndex(actor), i, relation;
    u8 ungreetedDistance[AMBIENCE_RELATION_COLLEAGUE + 1], ungreetedActor[AMBIENCE_RELATION_COLLEAGUE + 1];

    if (actor->phase == WALKER_PHASE_WALK && actor->stepKind == STEP_NONE)
        facts |= AMBIENCE_FACT_WALKING;
    if (dwelling)
        facts |= AMBIENCE_FACT_DWELLING;
    if (IsMapTypeOutdoors(gMapHeader.mapType))
        facts |= AMBIENCE_FACT_OUTDOORS;
    if (need & FACING_FACTS)
        facts |= FacingFacts(obj);
    // AMBIENCE_FACT_COMPANION_ROOM: added by DecideBeat where it can matter.
    ctx->facts = facts;
    ctx->spotKind = 0xFF;
    ctx->spotActivities = 0xFF;
    if (dwelling)
    {
        const struct WayfarerWorldSpot *spot = DestSpot(record, actor->slot);
        if (spot != NULL)
        {
            ctx->spotKind = spot->kind;
            ctx->spotActivities = spot->activities;
        }
    }
    ctx->activity = record->activity;
    ctx->playerDistance = IsVisible(obj) ? CapDistance(PlayerDistance(obj->currentCoords.x, obj->currentCoords.y))
                                         : AMBIENCE_DISTANCE_NONE;
    ctx->adjacentTicks = sAmbience->adjacentTicks[index];
    // The nearest other walker of each relation (NONE: any), preferring
    // the nearest this walker hasn't greeted yet: a greeting (once per pair)
    // then goes to the next one of that relation, not to nobody.
    memset(ctx->notableDistance, AMBIENCE_DISTANCE_NONE, sizeof(ctx->notableDistance));
    memset(ctx->notableActor, AMBIENCE_ACTOR_NONE, sizeof(ctx->notableActor));
    memset(ungreetedDistance, AMBIENCE_DISTANCE_NONE, sizeof(ungreetedDistance));
    memset(ungreetedActor, AMBIENCE_ACTOR_NONE, sizeof(ungreetedActor));
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        const struct WalkerActor *other = &sActors[i];
        const struct ObjectEvent *otherObj;
        u8 distance, r, slots[2];
        bool8 greeted;
        if (i == index || other->mode != WALKER_MODE_LOCAL)
            continue;
        otherObj = ActorObject(other);
        distance = CapDistance(Distance(obj->currentCoords.x, obj->currentCoords.y,
                                        otherObj->currentCoords.x, otherObj->currentCoords.y));
        greeted = (actor->ambienceLatches & (1 << (AMBIENCE_LATCH_GREETED_SHIFT + i))) != 0;
        slots[0] = AMBIENCE_RELATION_NONE;
        slots[1] = gWayfarerAmbienceRelations[actor->slot][other->slot];
        for (r = 0; r < 2; r++)
        {
            relation = slots[r];
            if (r == 1 && (relation == AMBIENCE_RELATION_NONE || relation > AMBIENCE_RELATION_COLLEAGUE))
                continue;
            if (distance < ctx->notableDistance[relation])
            {
                ctx->notableDistance[relation] = distance;
                ctx->notableActor[relation] = i;
            }
            if (!greeted && distance < ungreetedDistance[relation])
            {
                ungreetedDistance[relation] = distance;
                ungreetedActor[relation] = i;
            }
        }
    }
    for (relation = 0; relation <= AMBIENCE_RELATION_COLLEAGUE; relation++)
    {
        if (ungreetedActor[relation] != AMBIENCE_ACTOR_NONE)
        {
            ctx->notableDistance[relation] = ungreetedDistance[relation];
            ctx->notableActor[relation] = ungreetedActor[relation];
        }
    }
}

// The spot's facing while the walker stands at a stand/sit/browse spot (the
// templates that face it), else DIR_NONE.
static u8 SpotFacing(const struct WalkerActor *actor)
{
    const struct WayfarerWorldSpot *spot;
    if (!actor->atSpot || (actor->template != WORLD_TEMPLATE_STAND_AND_FACE && actor->template != WORLD_TEMPLATE_SIT_OR_IDLE
                           && actor->template != WORLD_TEMPLATE_BROWSE))
        return DIR_NONE;
    spot = DestSpot(Record(actor), actor->slot);
    if (spot == NULL || spot->facing < DIR_SOUTH || spot->facing > DIR_EAST)
        return DIR_NONE;
    return spot->facing;
}

static bool8 CanStartBeat(const struct WalkerActor *actor, struct ObjectEvent *obj, u8 decision)
{
    // Never during a back-off or the wait after it.
    if (sAmbience == NULL || actor->mode != WALKER_MODE_LOCAL || actor->justLeaving || BeatRunning(actor)
     || actor->backingOff || actor->goalKind == WALKER_GOAL_AWAY
     || actor->stepKind != STEP_NONE || IsPlayerPressingInto(obj))
        return FALSE;
    // The arrival's face action (StartTemplate) may still be under way: the
    // beat's first action waits for it. Otherwise nothing may be pending (a
    // stopped beat's last step, a template turn).
    return !actor->actionPending || decision == AMBIENCE_DECIDE_ARRIVE;
}

static void StartBeat(struct WalkerActor *actor, struct ObjectEvent *obj, u8 beat)
{
    u8 index = ActorIndex(actor);
    struct AmbienceWalker *select = &sAmbience->select[index];
    const struct WayfarerWorldRecord *record = Record(actor);
    const struct WayfarerWorldSpot *spot = DestSpot(record, actor->slot);
    struct ObjectEvent *other = NULL;
    u8 startFacing = obj->facingDirection, otherSlot = SLOT_NONE;

    if (select->other < WALKER_ACTOR_COUNT && sActors[select->other].mode != WALKER_MODE_NONE)
    {
        other = OwnObject(&sActors[select->other]);
        otherSlot = sActors[select->other].slot;
    }
    // The arrival's turn to the spot (StartTemplate) may still be queued: the
    // beat starts from the facing it is turning to, so look_back and an
    // interruption turn it back to the spot, not away from it.
    if (actor->actionPending && SpotFacing(actor) != DIR_NONE)
        startFacing = SpotFacing(actor);
    // Targets are captured now: the spot's facing ("target", "away") and
    // the partner. k was advanced by the pick: its parity at the start.
    WalkerBeats_Start(&sAmbience->run[index], beat, startFacing,
                      (spot != NULL && spot->node == record->node) ? spot->facing : DIR_NONE,
                      other, otherSlot, select->beatCounter - 1);
    actor->beatActive = TRUE;
    // A companion beat takes the map's companion now: another walker's
    // decision finds it busy (one prepared sheet, one spawn at a time).
    if (gWayfarerAmbienceBeats[beat].flags & AMBIENCE_FLAG_NEEDS_COMPANION)
    {
        sAmbience->companionOwner = index;
        sAmbience->companionObject = OBJECT_EVENTS_COUNT;
    }
    sAmbience->keepWalking[index] = (gWayfarerAmbienceBeats[beat].flags & AMBIENCE_FLAG_KEEP_WALKING) != 0;
    sAmbience->debug.started[gWayfarerAmbienceBeats[beat].cls]++;
    sAmbience->debug.running[index] = beat;
    LogBeat(actor, beat, WALKER_BEAT_EVENT_START);
}

static bool8 DecideWith(struct WalkerActor *actor, struct ObjectEvent *obj, u8 decision, const struct AmbienceContext *ctx)
{
    u8 beat = Ambience_Select(&sAmbience->select[ActorIndex(actor)], &actor->ambienceLatches, actor->slot, ctx, decision);
    if (beat == AMBIENCE_BEAT_NONE)
        return FALSE;
    StartBeat(actor, obj, beat);
    return !sAmbience->keepWalking[ActorIndex(actor)];
}

// Whether a companion beat could be picked here if companion_room held (its
// cooldown over, its when, who and limits holding): only then is the room
// (tiles, slots, palettes, the follower rule) worked out. At an arrival or a
// leaving a transition beat (arrive_look, leave_turn) usually holds: a row
// of a higher class than idle that holds off cooldown always wins the pick.
static bool8 CompanionBeatCouldWin(const struct WalkerActor *actor, const struct AmbienceContext *ctx, u8 decision)
{
    const struct AmbienceWalker *select = &sAmbience->select[ActorIndex(actor)];
    struct AmbienceContext withRoom = *ctx;
    u8 beat;

    if (decision == AMBIENCE_DECIDE_ARRIVE || decision == AMBIENCE_DECIDE_LEAVE)
    {
        for (beat = 0; beat < gWayfarerAmbienceBeatCount; beat++)
        {
            if (gWayfarerAmbienceBeats[beat].cls > AMBIENCE_CLASS_IDLE && select->cooldown[beat] == 0
             && Ambience_BeatHolds(beat, actor->slot, ctx, actor->ambienceLatches))
                return FALSE;
        }
    }
    withRoom.facts |= AMBIENCE_FACT_COMPANION_ROOM;
    for (beat = 0; beat < gWayfarerAmbienceBeatCount; beat++)
    {
        if ((gWayfarerAmbienceBeats[beat].flags & AMBIENCE_FLAG_NEEDS_COMPANION) && select->cooldown[beat] == 0
         && Ambience_BeatHolds(beat, actor->slot, &withRoom, actor->ambienceLatches))
            return TRUE;
    }
    return FALSE;
}

// A decision point. TRUE when a beat that stops the walker started.
// companion_room is only worked out while dwelling, at a decision where an
// idle beat can win (the companion beats are idle ones): about once every
// four dwell ticks past the quiet gap, never every frame.
static bool8 DecideBeat(struct WalkerActor *actor, struct ObjectEvent *obj, u8 decision, u32 facts)
{
    struct AmbienceContext ctx;
    bool8 idle;
    if (!CanStartBeat(actor, obj, decision))
        return FALSE;
    // Only a decision where an idle beat can win reads the whole pool (and
    // maybe companion_room): it takes the frame (see DecisionWaits).
    idle = Ambience_IdleCanWin(&sAmbience->select[ActorIndex(actor)], actor->slot, decision);
    if (idle)
        sAmbience->frameClaimed = TRUE;
    BuildAmbienceContext(actor, obj, facts, &ctx, NeededFacts(decision, idle));
    if ((ctx.facts & AMBIENCE_FACT_DWELLING) && idle && CompanionBeatCouldWin(actor, &ctx, decision))
    {
        s16 x, y;
        u16 gfx;
        u8 room = CompanionRoom(actor, obj, &x, &y, &gfx, 0);
        if (room == WALKER_COMPANION_DENY_COUNT)
        {
            ctx.facts |= AMBIENCE_FACT_COMPANION_ROOM;
            sAmbience->companionPlan[ActorIndex(actor)] = gfx;
        }
        else
            sAmbience->debug.companionDenied[room]++;
    }
    return DecideWith(actor, obj, decision, &ctx);
}

// Before a decision point where an idle beat can win (the costly kind): TRUE
// when this frame has no room for it (another took the frame, it is the spawn
// frame, or too few lines are left); the caller tries again next frame, for
// at most DECISION_WAIT_FRAMES. The other decisions are cheap and never wait.
static bool8 DecisionWaits(struct WalkerActor *actor, struct ObjectEvent *obj, u8 decision)
{
    u8 index = ActorIndex(actor);
    if (!CanStartBeat(actor, obj, decision) || !Ambience_IdleCanWin(&sAmbience->select[index], actor->slot, decision))
    {
        if (sAmbience != NULL)
            sAmbience->decisionWait[index] = 0;
        return FALSE;
    }
    if (WayfarerWalkers_ClaimFrame(DECISION_LINES, sAmbience->decisionWait[index] >= DECISION_WAIT_FRAMES))
    {
        sAmbience->decisionWait[index] = 0;
        return FALSE;
    }
    if (sAmbience->decisionWait[index] != 0xFF)
        sAmbience->decisionWait[index]++;
    return TRUE;
}

// The running beat ended (interrupted: stopped at once, facing restored).
// replan: a walker the beat left a tile off its spot walks back to it.
static void EndBeat(struct WalkerActor *actor, bool8 interrupted, bool8 replan)
{
    u8 index = ActorIndex(actor), beat;
    struct ObjectEvent *obj;
    bool8 pending, keepWalking;
    s8 dx, dy;

    if (!BeatRunning(actor))
        return;
    beat = sAmbience->select[index].beat;
    obj = OwnObject(actor);
    // A keep-walking beat (hum) never turns the walker: its walk does.
    keepWalking = sAmbience->keepWalking[index];
    // A script or a menu may hold the field (objects frozen): no held movements then.
    pending = WalkerBeats_Stop(&sAmbience->run[index], obj, interrupted && !keepWalking, ArePlayerFieldControlsLocked());
    WalkerBeats_Displacement(&sAmbience->run[index], &dx, &dy);
    LogBeat(actor, beat, interrupted ? WALKER_BEAT_EVENT_INTERRUPT : WALKER_BEAT_EVENT_END);
    if (interrupted)
        sAmbience->debug.interrupted++;
    else
        sAmbience->debug.ended++;
    // Its companion (or its reservation) goes with it ("companion in" may
    // have put it away), and a sheet preloaded for it that it never used.
    if (IsCompanionOut() && sAmbience->companionOwner == index)
        PutCompanionAway(interrupted, WALKER_COMPANION_AWAY_INTERRUPTED);
    if (!IsCompanionOut() && (gWayfarerAmbienceBeats[beat].flags & AMBIENCE_FLAG_NEEDS_COMPANION))
        ReleaseCompanionSheet();
    Ambience_EndBeat(&sAmbience->select[index]);
    sAmbience->keepWalking[index] = FALSE;
    sAmbience->debug.running[index] = AMBIENCE_BEAT_NONE;
    actor->beatActive = FALSE;
    if (obj == NULL)
        return;
    // A walk or a jump still under way (it can't be cancelled), or the
    // restored facing's turn: the walker waits for it, then turns back.
    // Under a keep-walking beat the held movement is the walker's own step.
    if (pending || (!keepWalking && obj->heldMovementActive))
        actor->actionPending = TRUE;
    if (replan && (dx != 0 || dy != 0) && actor->mode == WALKER_MODE_LOCAL && actor->atSpot)
    {
        SetTileGoal(actor, obj, obj->currentCoords.x - MAP_OFFSET - dx, obj->currentCoords.y - MAP_OFFSET - dy);
    }
    else if (!interrupted && !actor->actionPending && actor->mode == WALKER_MODE_LOCAL
          && actor->phase == WALKER_PHASE_TEMPLATE && SpotFacing(actor) != DIR_NONE
          && obj->facingDirection != SpotFacing(actor))
    {
        // A beat that ended facing elsewhere (meditate, study_ground...):
        // back to the spot's facing for the rest of the stay.
        ObjectEventClearHeldMovementIfFinished(obj);
        FaceDirection(actor, obj, SpotFacing(actor));
    }
}

static void InterruptBeat(struct WalkerActor *actor)
{
    EndBeat(actor, TRUE, TRUE);
}

// The walk a keep-walking beat (hum) runs alongside is over (the stay
// starts, or the walker stepped out into the strip): the beat ends with it,
// so the arrival's decision isn't lost to it.
static void EndKeepWalkingBeat(struct WalkerActor *actor)
{
    if (BeatRunning(actor) && sAmbience->keepWalking[ActorIndex(actor)])
        EndBeat(actor, FALSE, FALSE);
}

// A bow a beat stopped under a lock left to the script ends once the field
// is free again, whatever became of the beat's walker since.
static void ReleaseBeatNpcs(void)
{
    u8 i;
    if (sAmbience == NULL)
        return;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sAmbience->select[i].beat == AMBIENCE_BEAT_NONE)
            WalkerBeats_ReleaseNpc(&sAmbience->run[i]);
    }
}

// The actor goes away (removed, forgotten, a warp).
static void ForgetBeat(struct WalkerActor *actor)
{
    if (actor->mode != WALKER_MODE_NONE)
        EndBeat(actor, TRUE, FALSE);
}

static void InterruptAllBeats(void)
{
    u8 i;
    if (sAmbience == NULL)
        return;
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        if (sActors[i].mode != WALKER_MODE_NONE)
            InterruptBeat(&sActors[i]);
    }
}

bool8 WayfarerWalkers_BeatCanStep(struct ObjectEvent *obj, u8 dir)
{
    if (dir < DIR_SOUTH || dir > DIR_EAST)
        return FALSE;
    return IsFreeTile(obj->currentCoords.x + sDx[dir], obj->currentCoords.y + sDy[dir])
        && ProbeFromHere(obj, dir) == dir;
}

// What a react check reads, folded into a key that is cheap to make every
// frame: the player's and the walkers' tiles (distances, the player in
// view), the walker's latches, adjacency ticks, walking or dwelling, the
// activity and whether a beat can start. A key that changed means a react
// condition may have just come true (spec, "Selection").
// The frame of the spawn period a walker's react check may run in.
static u8 ReactPhase(u8 index)
{
    return 1 + 2 * index;
}

static u32 ReactKey(const struct WalkerActor *actor, const struct ObjectEvent *obj, bool8 canStart)
{
    const struct ObjectEvent *player = Player();
    u8 index = ActorIndex(actor), i;
    u32 key = (u16)player->currentCoords.x | ((u32)(u16)player->currentCoords.y << 16);

    key = key * 33 ^ ((u16)obj->currentCoords.x | ((u32)(u16)obj->currentCoords.y << 16));
    key = key * 33 ^ ((u16)gSaveBlock1Ptr->pos.x | ((u32)(u16)gSaveBlock1Ptr->pos.y << 16));
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        const struct ObjectEvent *other;
        if (i == index || sActors[i].mode != WALKER_MODE_LOCAL)
            continue;
        other = ActorObject(&sActors[i]);
        key = key * 33 ^ (i | ((u32)(u8)other->currentCoords.x << 8) | ((u32)(u8)other->currentCoords.y << 16));
    }
    key = key * 33 ^ (actor->ambienceLatches | (sAmbience->adjacentTicks[index] << 8)
                      | (Record(actor)->activity << 16)
                      | ((actor->phase == WALKER_PHASE_WALK && actor->stepKind == STEP_NONE) << 24)
                      | ((actor->atSpot && actor->phase == WALKER_PHASE_TEMPLATE) << 25)
                      | (canStart << 26));
    return key | 1;
}

// A react row on cooldown that the coming tick frees: the react check must
// run then even with nothing else changed.
static bool8 ReactCooldownEnds(const struct AmbienceWalker *select)
{
    u8 beat;
    for (beat = 0; beat < gWayfarerAmbienceBeatCount; beat++)
    {
        if (select->cooldown[beat] == 1 && gWayfarerAmbienceBeats[beat].cls == AMBIENCE_CLASS_REACT)
            return TRUE;
    }
    return FALSE;
}

// Every frame for a local actor: the ambience tick, the running beat, the
// latches and the react decision point. The react check (the latches and
// the react rows) only runs when its inputs changed (ReactKey) or a react
// row's cooldown ran out: within 4 frames of its condition first holding. It
// is put off while another walker's check or a costly start has the frame.
static void AmbienceFrame(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    u8 index = ActorIndex(actor);
    struct AmbienceContext ctx;
    bool8 canStart;
    u32 key;

    if (sAmbience == NULL && !EnsureAmbience())
        return;
    // Most frames of an idle walker: nothing but the tick count.
    if (++sAmbience->tickFrames[index] < AMBIENCE_TICK_FRAMES && !actor->beatActive
     && sSpawnTimer != ReactPhase(index))
        return;
    if (sAmbience->tickFrames[index] >= AMBIENCE_TICK_FRAMES)
    {
        sAmbience->tickFrames[index] = 0;
        if (ReactCooldownEnds(&sAmbience->select[index]))
            sAmbience->reactKey[index] = REACT_KEY_STALE;
        Ambience_Tick(&sAmbience->select[index]);
        // Standing next to the walker, facing it, without pushing.
        if (IsPlayerPushing(obj) && !IsPlayerPressingInto(obj))
        {
            if (sAmbience->adjacentTicks[index] != 0xFF)
                sAmbience->adjacentTicks[index]++;
        }
        else
        {
            sAmbience->adjacentTicks[index] = 0;
        }
    }
    if (BeatRunning(actor))
    {
        // Pushed against: the beat stops at once (the yield rule follows).
        if (IsPlayerPressingInto(obj))
        {
            InterruptBeat(actor);
        }
        else if (WayfarerWalkers_FrameLinesLeft() == 0 && sAmbience->lateFrames[index] < BEAT_LATE_FRAMES)
        {
            // A frame that reached the walkers late (the engine's own work
            // before them ran long) has no line to spare: the beat sits it
            // out (its movement under way goes on), a few frames at most.
            sAmbience->lateFrames[index]++;
        }
        else
        {
            // The walkers' busy frames start no new step, so only a frame
            // that can start one ends a run of sat-out frames: a step can
            // always start within BEAT_LATE_FRAMES + 1 non-busy frames.
            u8 result;
            bool8 busy = IsBusyFrame();
            if (!busy)
                sAmbience->lateFrames[index] = 0;
            result = WalkerBeats_Run(&sAmbience->run[index], obj, IsVisible(obj), busy, &sAmbience->debug);
            if (result != WALKER_BEAT_RUNNING)
                EndBeat(actor, FALSE, TRUE);
        }
    }
    if (actor->mode != WALKER_MODE_LOCAL || actor->justLeaving)
        return;
    // Each walker looks once a spawn period, in its own odd frame (clear of
    // the busy ones): about a line a frame saved per walker, for at most 8
    // frames (133 ms) of delay.
    if (sSpawnTimer != ReactPhase(index) || sAmbience->reactChecked || sAmbience->frameClaimed || IsBusyFrame())
        return;     // a later frame sees the change
    canStart = CanStartBeat(actor, obj, AMBIENCE_DECIDE_REACT);
    key = ReactKey(actor, obj, canStart);
    if (key == sAmbience->reactKey[index])
        return;
    // A check needs a frame with room for it too (a few samples at most).
    if (sAmbience->reactWait[index] < REACT_WAIT_SAMPLES && WayfarerWalkers_FrameLinesLeft() < REACT_LINES)
    {
        sAmbience->reactWait[index]++;
        return;
    }
    sAmbience->reactWait[index] = 0;
    sAmbience->reactChecked = TRUE;
    BuildAmbienceContext(actor, obj, 0, &ctx, sAmbience->needFacts[AMBIENCE_CLASS_REACT]);
    Ambience_UpdateLatches(&actor->ambienceLatches, actor->slot, &ctx);
    if (canStart)
        DecideWith(actor, obj, AMBIENCE_DECIDE_REACT, &ctx);
    // As this check leaves them: the latches it set or cleared, the beat it started.
    sAmbience->reactKey[index] = ReactKey(actor, obj, canStart && !BeatRunning(actor));
}

static bool8 IsStepDecided(const struct WalkerActor *actor)
{
    return sAmbience != NULL && (sAmbience->stepDecided & (1 << ActorIndex(actor)));
}

static void SetStepDecided(const struct WalkerActor *actor, bool8 decided)
{
    if (sAmbience == NULL)
        return;
    if (decided)
        sAmbience->stepDecided |= 1 << ActorIndex(actor);
    else
        sAmbience->stepDecided &= ~(1 << ActorIndex(actor));
}

#define AMBIENCE_DECISION_NONE 0xFF

// The decision WalkStep is about to make (OnGoalReached's, at the path's
// end): a step boundary not decided yet, the arrival when the stay starts
// there, or none (a truncated path re-plans; a walk back to the spot, a
// wander or browse move, a back-off and the exits have no decision).
static u8 WalkDecision(const struct WalkerActor *actor)
{
    const struct WayfarerWorldRecord *record;
    if (actor->pathPos < actor->pathLen)
        return IsStepDecided(actor) ? AMBIENCE_DECISION_NONE : AMBIENCE_DECIDE_STEP;
    if (actor->truncated || actor->goalKind != WALKER_GOAL_TILE)
        return AMBIENCE_DECISION_NONE;
    record = Record(actor);
    if (!actor->atSpot
     || (record->state == WORLD_STATE_TRAVELLING && WorldSim_DestNode(State(), actor->slot) == record->node))
        return AMBIENCE_DECIDE_ARRIVE;
    return AMBIENCE_DECISION_NONE;
}

// A stopped beat's walk or jump is over: turn back to its start facing.
static void RestoreBeatFacing(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    if (sAmbience == NULL || BeatRunning(actor))
        return;
    WalkerBeats_RestoreFacing(&sAmbience->run[ActorIndex(actor)], obj);
    if (obj->heldMovementActive)
        actor->actionPending = TRUE;
}

// ---------------------------------------------------------------------------
// Behaviour templates and local dwell

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

// Whether the next dwell tick reaches a decision point: a dwell decision
// (template phase, no beat holding the walker) or the stay's end (leaving).
// Dwell and leaving decisions count t alike (Ambience_IdleCanWin).
static bool8 TickDecides(const struct WalkerActor *actor)
{
    const struct WayfarerWorldRecord *record = Record(actor);
    if (actor->phase == WALKER_PHASE_TEMPLATE && !BeatHoldsWalker(actor))
        return TRUE;
    return record->state == WORLD_STATE_DWELLING && actor->dwellTicks + 1 >= WORLD_LOCAL_DWELL_TICKS && record->dwell <= 1;
}

// Dwell also runs down in local time while watched: one heartbeat of dwell
// per WORLD_LOCAL_DWELL_TICKS dwell ticks; at 0 the routine advances.
static bool8 LocalDwellTick(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    struct WayfarerWorldRecord *record = Record(actor);

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
    // The next step's spot choices run a frame apart (RunWorldJobs); the
    // actor plans once they are done.
    QueueAdvance(actor->slot);
    actor->capacityWaits = 0;
    actor->atSpot = FALSE;
    actor->k = actor->t = 0;
    actor->pauseTicks = 0;
    Replan(actor, 0);
    // The stay ended: the leaving decision point (the plan waits for its
    // beat). With a beat still running it comes once that beat ends.
    if (BeatRunning(actor))
        actor->leavePending = TRUE;
    else
        DecideBeat(actor, obj, AMBIENCE_DECIDE_LEAVE, AMBIENCE_FACT_LEAVING);
    return TRUE;
}

// A walk-off: out of view, pushed against, or too slow, it vanishes.
static void UpdateLeaving(struct WalkerActor *actor, struct ObjectEvent *obj)
{
    if (!IsVisible(obj))
    {
        EndWalkOff(actor, TRUE);
        return;
    }
    // Still pushed against as long as a yield takes (not just the push that
    // started the walk-off): vanish.
    if (IsPlayerPressingInto(obj))
        actor->pushFrames++;
    else
        actor->pushFrames = 0;
    if (++actor->t >= WALKER_WALKOFF_FRAMES || actor->pushFrames >= WALKER_YIELD_FRAMES)
    {
        RemoveActor(actor);
        return;
    }
    switch (actor->phase)
    {
    case WALKER_PHASE_PLAN:
    case WALKER_PHASE_IDLE:
        RequestSearch(actor);
        break;
    case WALKER_PHASE_WALK:
        WalkStep(actor, obj);
        break;
    default:
        break;
    }
}

static void UpdateActor(struct WalkerActor *actor)
{
    struct ObjectEvent *obj = ActorObject(actor);
    struct WayfarerWorldRecord *record = Record(actor);
    u8 decision;

    if (obj->frozen)
        return;
    if (actor->mode == WALKER_MODE_LOCAL)
        AmbienceFrame(actor, obj);
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
        RestoreBeatFacing(actor, obj);
        if (actor->actionPending)
            return;
    }
    if (actor->mode == WALKER_MODE_STRIP)
    {
        UpdateStrip(actor, obj);
        return;
    }
    if (actor->mode == WALKER_MODE_LEAVING)
    {
        UpdateLeaving(actor, obj);
        return;
    }
    if (!WorldSim_IsSimulated(record) || WorldSim_NodeMap(record->node) != CurrentMap()
     || (record->state == WORLD_STATE_DWELLING && IsLeaderHomeInGym(actor->slot))
     || !InMap(obj->currentCoords.x - MAP_OFFSET, obj->currentCoords.y - MAP_OFFSET))
    {
        RemoveActor(actor);
        return;
    }

    // Never trap the player: pushed against for a while, make room (also
    // mid-search: the back-off's own search request aborts it safely).
    // Pushing means pressing into the walker: a player just standing next
    // to it (player_lingers) doesn't make it back off.
    if (IsPlayerPressingInto(obj))
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

    // Dwell ticks: the template's clock and the local dwell. A tick that
    // reaches a decision point skips the busy frames (its decision and the
    // template move cost 6 to 12 lines, too many on top of a spawn frame or
    // the time-of-day update; at most a few frames), and a costly decision
    // waits for a frame with room (a frame or so).
    if (actor->atSpot && ++actor->frames >= WORLD_DWELL_TICK_FRAMES
     && (!TickDecides(actor) || (!IsBusyFrame() && !DecisionWaits(actor, obj, AMBIENCE_DECIDE_DWELL))))
    {
        actor->frames = 0;
        actor->t++;
        if (LocalDwellTick(actor, obj))
            return;
        // A dwell tick is a decision point; a beat that starts takes the
        // template's turn.
        if (actor->phase == WALKER_PHASE_TEMPLATE && !BeatHoldsWalker(actor)
         && !DecideBeat(actor, obj, AMBIENCE_DECIDE_DWELL, 0))
            TemplateTick(actor, obj);
    }
    else if (actor->atSpot && actor->frames >= WORLD_DWELL_TICK_FRAMES)
    {
        actor->frames = WORLD_DWELL_TICK_FRAMES - 1;    // the tick comes next frame
    }
    // No template move, step or plan while a beat holds the walker.
    if (BeatHoldsWalker(actor))
        return;
    // The stay ended while a beat ran: its leaving moment, before the plan.
    if (actor->leavePending && !BeatRunning(actor))
    {
        if (DecisionWaits(actor, obj, AMBIENCE_DECIDE_LEAVE))
            return;
        actor->leavePending = FALSE;
        if (DecideBeat(actor, obj, AMBIENCE_DECIDE_LEAVE, AMBIENCE_FACT_LEAVING))
            return;
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
        // A step boundary or the arrival: a costly decision waits for a
        // frame with room.
        decision = WalkDecision(actor);
        if (decision == AMBIENCE_DECIDE_ARRIVE)
            EndKeepWalkingBeat(actor);  // the walk is over: the arrival decides now
        if (decision != AMBIENCE_DECISION_NONE && DecisionWaits(actor, obj, decision))
            break;
        WalkStep(actor, obj);
        break;
    }
}

// ---------------------------------------------------------------------------
// Following Pokémon, frozen mask and the local actor block

// Free object slots with every wanted actor spawned and the follower out.
// The companion isn't counted: it gives way instead (CompanionRoom and
// UpdateFollower keep one more slot for it).
static s16 FollowerFreeSlots(u8 *wantedOut)
{
    u8 i, others = 0, wanted, slot;

    wanted = CountActors(WALKER_MODE_LOCAL) + CountActors(WALKER_MODE_STRIP);
    if (sMapNode != WORLD_NODE_NONE)
    {
        u8 capacity = WorldSim_MapCapacity(sMapNode), locals = CountActors(WALKER_MODE_LOCAL);
        u8 pending = 0, room = capacity > locals ? capacity - locals : 0;
        // The candidates (a pass over every trainer) as the follower rule
        // counted them less than a spawn period ago, for the same map and
        // walkers: companion_room checks reuse them (UpdateFollower, every
        // spawn period, counts again and puts a companion away if needed).
        if (sAmbience != NULL && sAmbience->pendingValid && sAmbience->pendingNode == sMapNode
         && sAmbience->pendingLocals == locals
         && gWayfarerWalkersDebug.frames - sAmbience->pendingFrame < WALKER_SPAWN_PERIOD)
        {
            pending = sAmbience->pendingCount;
        }
        else
        {
            for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT && pending < room; slot++)
            {
                if (IsSpawnCandidate(slot))
                    pending++;
            }
            if (sAmbience != NULL)
            {
                sAmbience->pendingValid = TRUE;
                sAmbience->pendingNode = sMapNode;
                sAmbience->pendingLocals = locals;
                sAmbience->pendingCount = pending;
                sAmbience->pendingFrame = gWayfarerWalkersDebug.frames;
            }
        }
        wanted += pending;
    }
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        const struct ObjectEvent *obj = &gObjectEvents[i];
        if (obj->active && obj->localId != OBJ_EVENT_ID_FOLLOWER && obj->localId != COMPANION_LOCALID
         && !WayfarerWalkers_IsActorObject(obj))
            others++;
    }
    if (wantedOut != NULL)
        *wantedOut = wanted;
    return OBJECT_EVENTS_COUNT - others - wanted - 1;
}

static void UpdateFollower(void)
{
    u8 wanted;
    bool8 hide;
    s16 freeSlots = FollowerFreeSlots(&wanted);

    // The companion never makes the rule hide the follower: once the rule
    // has no slot to spare for it, it goes.
    if (IsCompanionSpawned() && freeSlots - 1 < FOLLOWER_FREE_SLOTS)
        SendCompanionAway(WALKER_COMPANION_AWAY_FOLLOWER);
    hide = wanted != 0 && freeSlots < FOLLOWER_FREE_SLOTS;
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

// Off-screen actors give their slot back when the map's own objects need it;
// the companion goes before any of them.
static void CullForSlots(void)
{
    s8 i;
    if (FreeObjectSlots() > 1)
        return;
    if (IsCompanionSpawned())
    {
        SendCompanionAway(WALKER_COMPANION_AWAY_SLOTS);
        return;
    }
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
        // A walker leaving no longer stands for its record.
        if (actor->mode != WALKER_MODE_LEAVING)
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
    // Whether the follower's hide flag is ours goes into any save, so a
    // Continue without a warp (temp flags kept) can clear it again.
    if (sFollowerFlagOurs)
        state->walkerFlags |= WORLD_WALKER_FLAG_FOLLOWER_HIDDEN;
    else
        state->walkerFlags &= ~WORLD_WALKER_FLAG_FOLLOWER_HIDDEN;
    // Trainers with an actor (or visible in a strip), or whose routine
    // advance is still under way, are left alone by the heartbeat.
    gWayfarerWorldFrozenMask = mask | AdvancingMask();
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
    // A real map change (a warp or a seam) clears the temp flags; the first
    // frame after a Continue without a warp keeps them, and with them the
    // saved ownership of the follower's hide flag.
    bool8 flagsCleared = sTransition != TRANSITION_NONE || sActiveMapValid;

    if (sTransition == TRANSITION_CAMERA && sActiveMapValid)
        OnSeam();
    else
        ForgetAllActors();
    // The Continue restore is for the saved map only: dropped once the
    // player is anywhere else (a Continue warp elsewhere, or out through a
    // door before it spawned), so it never seats a trainer on another map.
    if (sRestorePending && map != sRestoreMap)
    {
        sRestorePending = FALSE;
        sPendingCount = 0;
    }
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
    if (flagsCleared)
    {
        sHideFollower = FALSE;
        sFollowerFlagOurs = FALSE;
    }
}

// ---------------------------------------------------------------------------
// Hooks

// A spawn attempt, the story scene check with the slot cull and the follower
// rule (their spawn-period frames) wait a period when the frame has too few
// lines left for them, at most BUSY_DEFER_PERIODS in a row (24 frames): a
// frame the engine's own work made late, plus a busy frame's 10 to 15 lines,
// overruns, and a beat's sprites (an icon, an effect, the companion) take a
// few lines more every frame they show. Only while the ambience block exists.
#define BUSY_WORK_LINES     16
#define BUSY_DEFER_PERIODS  3
static bool8 BusyWorkWaits(u8 *deferred)
{
    if (deferred == NULL || *deferred >= BUSY_DEFER_PERIODS || WayfarerWalkers_FrameLinesLeft() >= BUSY_WORK_LINES)
    {
        if (deferred != NULL)
            *deferred = 0;
        return FALSE;
    }
    (*deferred)++;
    return TRUE;
}

void WayfarerWalkers_Update(void)
{
    u8 i;
    u16 map;
    u32 start;

    sLastUpdateScanlines = 0;
    sLastUpdateVblank = gMain.vblankCounter1;
    if (gSaveBlock1Ptr == NULL || gMapHeader.mapLayout == NULL || gMapHeader.events == NULL)
        return;
    start = ScanlineStamp();
    if (sAmbience != NULL)
        sAmbience->frameClaimed = sAmbience->reactChecked = FALSE;
    gWayfarerWalkersDebug.frames++;
    map = CurrentMap();
    if (!sActiveMapValid || sActiveMap != map || sTransition != TRANSITION_NONE)
        OnMapChanged(map);
    ValidateActors();
    if (sRestorePending && !sAdoptChecked)
        AdoptRestoredObjects();
    CompanionFrame();

    if (++sSpawnTimer >= WALKER_SPAWN_PERIOD)
        sSpawnTimer = 0;
    if (sSpawnTimer == STORY_PHASE && !BusyWorkWaits(sAmbience != NULL ? &sAmbience->storyDeferred : NULL))
    {
        // Story objects can appear during a script, with the controls locked.
        CheckStoryScenes();
        CullForSlots();
    }

    // Talks, menus and scripts lock the field controls: the AI and its dwell
    // stay suspended (the engine still finishes a step already under way).
    // So do they while the map load's heartbeat is still running (from a
    // few frames to a few dozen in busy frames): the walkers' world writes
    // and spawns then come after it, as when it ran inside the map load,
    // and it gets the frame's spare time to itself.
    // A script, a menu or a story scene stops every running beat at once.
    if (ArePlayerFieldControlsLocked())
        InterruptAllBeats();
    if (!ArePlayerFieldControlsLocked())
        ReleaseBeatNpcs();
    if (!ArePlayerFieldControlsLocked() && !WayfarerWorld_IsHeartbeatPending())
    {
        RunScheduler(RunWorldJobs());
        for (i = 0; i < WALKER_ACTOR_COUNT; i++)
        {
            if (sActors[i].mode != WALKER_MODE_NONE)
                UpdateActor(&sActors[i]);
        }
        if (sSpawnTimer == 0 && !BusyWorkWaits(sAmbience != NULL ? &sAmbience->spawnDeferred : NULL))
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
        }
        if (sSpawnTimer == FOLLOWER_PHASE && !BusyWorkWaits(sAmbience != NULL ? &sAmbience->followerDeferred : NULL))
            UpdateFollower();
    }
    PublishState();
    {
        u32 cost = ScanlinesSince(start);
        sLastUpdateScanlines = cost;
        if (cost > gWayfarerWalkersDebug.maxUpdateScanlines)
            gWayfarerWalkersDebug.maxUpdateScanlines = cost;
    }
}

void WayfarerWalkers_OnWarp(void)
{
    // The warp resets every object; the records stay on their nodes and the
    // heartbeat that follows treats them as off-screen. A routine advance
    // still under way completes first (behind the fade), after any
    // heartbeat still pending, which it was waiting for.
    if (WayfarerWorld_IsHeartbeatPending())
    {
        gWayfarerWorldDebug.pendingAtLoad++;
        WayfarerWorld_FinishHeartbeat();
    }
    WayfarerWalkers_FlushWorldJobs();
    // The warp resets every object (and the field may already be torn
    // down): the companion is dropped, not removed.
    DropCompanionObjects();
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
    // A routine advance under way stays queued across the seam (no spot
    // choices in the seam's frame): its trainer stays frozen for the
    // heartbeat, and it resumes once the heartbeat is done. A trip search
    // is only a question; whoever asked asks again.
    if (sEdgeJob.status != EDGE_JOB_IDLE)
        CancelEdgeJob();
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
    gWayfarerWorldFrozenMask |= AdvancingMask();
}

void WayfarerWalkers_OnContinue(void)
{
    struct WayfarerWorldState *state = State();
    u8 i, slot, x, y, facing;

    // A save may have captured a companion: switched off before the saved
    // objects' sprites come back (never respawned, so no palette load).
    DropCompanionObjects();
    ForgetAllActors();
    DropWorldJobs();
    memset(sRecentEntries, 0, sizeof(sRecentEntries));
    sActiveMapValid = FALSE;
    sTransition = TRANSITION_NONE;
    sPendingCount = 0;
    sRestoreMap = CurrentMap();
    // The saved temp flag comes back as it was: whether it is the walkers'
    // own is saved beside it (a map script's stays the script's).
    sFollowerFlagOurs = (state->walkerFlags & WORLD_WALKER_FLAG_FOLLOWER_HIDDEN) && FlagGet(FLAG_TEMP_HIDE_FOLLOWER);
    sFollowerRemoved = sFollowerFlagOurs;
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
    DropCompanionObjects();
    ForgetAllActors();
    DropWorldJobs();
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
    // InitHeap overwrites the allocator: never Free these pointers again,
    // and never read the ambience block here either: MoveSaveBlocks_ResetHeap
    // has already copied the save blocks over the heap. Each actor re-plans
    // from its current tile. A menu, battle or save tears the field down: a
    // step under way would replay with no collision check when the sprites
    // come back, so it ends here, on its destination tile.
    sWork = NULL;
    sSearch.actor = NO_ACTOR;
    sJobWorkspace = NULL;   // a trip search under way starts again
    sAmbience = NULL;       // counters and cooldowns start again in a new block
    sEdgeJob.live = FALSE;
    gWayfarerWalkersDebug.heapResets++;
    // Beats are stopped before any heap reset (the field-controls lock: menus,
    // scripts, battles; warps and Continue forget the actors). Whatever is
    // left ends with plain field writes from data outside the heap: the
    // companion's object is switched off (RemoveObjectEvent isn't safe here:
    // the field's sprites may already be a menu's or a battle's; nothing
    // respawns it), a nurse left bowing stands up, facing locks go.
    DropCompanionObjects();
    WalkerBeats_OnHeapReset();
    for (i = 0; i < WALKER_ACTOR_COUNT; i++)
    {
        struct WalkerActor *actor = &sActors[i];
        struct ObjectEvent *obj;
        if (actor->mode == WALKER_MODE_NONE)
            continue;
        obj = ActorObject(actor);
        if (obj->active && obj->localId == WALKER_LOCALID_BASE + i)
        {
            obj->facingDirectionLocked = FALSE;
            SettleHeldMovement(obj);
            actor->actionPending = FALSE;
        }
        if (actor->beatActive)
        {
            // A beat nothing stopped (its state went with the block): the
            // walker walks back to its spot (a beat may have left it a tile
            // off) and faces it again there.
            actor->beatActive = FALSE;
            actor->leavePending = FALSE;
            if (actor->mode == WALKER_MODE_LOCAL && actor->atSpot)
            {
                actor->phase = WALKER_PHASE_PLAN;
                actor->wait = 0;
            }
        }
        if ((actor->mode == WALKER_MODE_LOCAL || actor->mode == WALKER_MODE_LEAVING)
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
        {
            // The objects of a map spawn as it loads. A leader still to act
            // in the heartbeat may be on the way home: decide from the
            // finished heartbeat, as before it was spread over frames. (Gyms
            // are entered by warp, so this runs behind the fade.)
            if (WayfarerWorld_IsSlotPending(slot))
                WayfarerWorld_FinishHeartbeatEarly();
            return !IsLeaderObjectShown(slot);
        }
    }
    return FALSE;
}

bool8 WayfarerWalkers_HideFollower(void)
{
    return sHideFollower;
}

#endif // IS_WAYFARER
