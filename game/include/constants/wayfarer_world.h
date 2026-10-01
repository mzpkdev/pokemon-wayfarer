#ifndef GUARD_CONSTANTS_WAYFARER_WORLD_H
#define GUARD_CONSTANTS_WAYFARER_WORLD_H

// Notable world simulation: shared values for the generated walker graph and
// spot tables (tools/wayfarer_world), the off-screen simulation and the local
// actor. See .product/specs/notable-world-simulation.md and notable-spots.md.

#define WORLD_SCHEMA_VERSION        1

// The notables with a Full 9-frame overworld sprite, in catalog order. The
// spec counted 27, but Bruno's and Koga's sheets are face-only (48x32), so 25.
#define WORLD_SIM_TRAINER_COUNT     25
#define WORLD_LOCAL_ACTOR_COUNT     3
#define WORLD_LOCAL_ACTOR_NONE      63

// Record field widths: node is 12 bits and destId 14 bits. The top value of
// each is reserved as "none", so at most 4,095 nodes and 16,383 spots exist.
#define WORLD_MAX_NODES             4095
#define WORLD_MAX_SPOTS             16383
#define WORLD_NODE_NONE             0xFFF
#define WORLD_SPOT_NONE             0x3FFF

#define WORLD_CYCLE_MAX_STEPS       4
#define WORLD_FAVOURITE_MAX         3

// Placeholders from the specs.
#define WORLD_RADIUS_NARROW         3
#define WORLD_RADIUS_TRAVELLER      8
#define WORLD_CAPACITY_INTERIOR     1
#define WORLD_CAPACITY_OUTDOOR      3
#define WORLD_LIFE_EVENT_STEPS      2
#define WORLD_LOCAL_DWELL_TICKS     10  // dwell ticks per heartbeat of local dwell
#define WORLD_DWELL_TICK_FRAMES     60

enum WorldActivity
{
    WORLD_ACTIVITY_CARE,
    WORLD_ACTIVITY_SHOP,
    WORLD_ACTIVITY_GAMBLE,
    WORLD_ACTIVITY_TRAIN,
    WORLD_ACTIVITY_RELAX,
    WORLD_ACTIVITY_FISH,
    WORLD_ACTIVITY_VISIT,
    WORLD_ACTIVITY_STUDY,
    WORLD_ACTIVITY_HOME,
    WORLD_ACTIVITY_SIGHTSEE,
    WORLD_ACTIVITY_LIE_LOW,
    WORLD_ACTIVITY_COUNT,
    WORLD_ACTIVITY_NONE = 15,
};

// Spot kinds as the spot table stores them. The spots spec's kinds 1 and 7
// split into two each (counter and side; square and bench).
enum WorldSpotKind
{
    WORLD_SPOT_CENTER_COUNTER,
    WORLD_SPOT_CENTER_SIDE,
    WORLD_SPOT_STORE,
    WORLD_SPOT_GAME_CORNER,
    WORLD_SPOT_GYM,
    WORLD_SPOT_TALL_GRASS,
    WORLD_SPOT_WATER_EDGE,
    WORLD_SPOT_SQUARE,
    WORLD_SPOT_BENCH,
    WORLD_SPOT_NPC_CHAT,
    WORLD_SPOT_NAMED,
    WORLD_SPOT_KIND_COUNT,
};

// WayfarerWorldSpot.flags
#define WORLD_SPOT_FLAG_PUBLIC          (1 << 0) // closed to aloof trainers
#define WORLD_SPOT_FLAG_CAPACITY_2      (1 << 1) // named spot holding two trainers
#define WORLD_SPOT_TEMPLATE_SHIFT       2        // named spot template override
#define WORLD_SPOT_TEMPLATE_MASK        (3 << WORLD_SPOT_TEMPLATE_SHIFT)

enum WorldTemplate
{
    WORLD_TEMPLATE_DEFAULT,      // named spots: from the activity
    WORLD_TEMPLATE_STAND_AND_FACE,
    WORLD_TEMPLATE_SIT_OR_IDLE,
    WORLD_TEMPLATE_WANDER,
    WORLD_TEMPLATE_BROWSE,
    WORLD_TEMPLATE_JUST_LEAVING,
};

enum WorldEmote
{
    WORLD_EMOTE_NONE,
    WORLD_EMOTE_EXCLAMATION,
    WORLD_EMOTE_ELLIPSIS,
};

// An edge leaves its source node by a map side, a warp, or a transit link.
enum WorldEdgeKind
{
    WORLD_EDGE_NONE,
    WORLD_EDGE_NORTH,
    WORLD_EDGE_SOUTH,
    WORLD_EDGE_EAST,
    WORLD_EDGE_WEST,
    WORLD_EDGE_WARP,
    WORLD_EDGE_TRANSIT,
};

// How a trainer entered their current node; a map side is the side of the
// current map they crossed.
enum WorldArrival
{
    WORLD_ARRIVAL_NONE,
    WORLD_ARRIVAL_NORTH,
    WORLD_ARRIVAL_SOUTH,
    WORLD_ARRIVAL_EAST,
    WORLD_ARRIVAL_WEST,
    WORLD_ARRIVAL_DOOR,
    WORLD_ARRIVAL_TRANSIT,
    WORLD_ARRIVAL_COUNT,
};

enum WorldState
{
    WORLD_STATE_TRAVELLING,
    WORLD_STATE_DWELLING,
    WORLD_STATE_AWAY_LEAGUE,
    WORLD_STATE_AWAY_PARTNER,
    WORLD_STATE_PINNED,
    WORLD_STATE_HOME_LOCKED,
    WORLD_STATE_COUNT,
};

enum WorldDestKind
{
    WORLD_DEST_NONE,
    WORLD_DEST_SPOT,
    WORLD_DEST_HOME,
    WORLD_DEST_RESERVED_HAUNT,
};

enum WorldLifeEvent
{
    WORLD_LIFE_NONE,
    WORLD_LIFE_RECOVERING,
    WORLD_LIFE_CELEBRATING,
    WORLD_LIFE_BROODING,
};

enum WorldRegion
{
    WORLD_REGION_KANTO,
    WORLD_REGION_JOHTO,
    WORLD_REGION_HOENN,
    WORLD_REGION_SEVII,
    WORLD_REGION_NONE = 0xFF,
};

// WayfarerWorldNode.flags
#define WORLD_NODE_FLAG_INTERIOR        (1 << 0)
#define WORLD_NODE_REGION_SHIFT         1
#define WORLD_NODE_REGION_MASK          (3 << WORLD_NODE_REGION_SHIFT)

// WayfarerWorldTrainer.flags
#define WORLD_TRAINER_FLAG_TRAVELLER    (1 << 0)
#define WORLD_TRAINER_FLAG_ALOOF        (1 << 1)
#define WORLD_TRAINER_FLAG_GYM_LEADER   (1 << 2)

// stayBits
#define WORLD_STAY_EGG_FOLLOW_UP        (1 << 0)
#define WORLD_STAY_COURIER_TRAIL        (1 << 1)

#endif // GUARD_CONSTANTS_WAYFARER_WORLD_H
