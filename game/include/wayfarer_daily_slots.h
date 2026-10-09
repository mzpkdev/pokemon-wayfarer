#ifndef GUARD_WAYFARER_DAILY_SLOTS_H
#define GUARD_WAYFARER_DAILY_SLOTS_H

// Daily world slots (Wayfarer only): the day, deterministic draws, the saved
// per-day state and the item half of the slots. Specs:
// .product/specs/daily-world-slots.md and .product/specs/world-items.md.
//
// The saved state sits in PokemonStorage beside the world state. The trainer
// half appends its own fields after clearedToday (homeBeatenToday, the phone
// snapshots) and its slots after the item spots in the same bitset.

#include "gba/types.h"
#include "constants/world_item_spots.h"

struct ObjectEventTemplate;
struct BgEvent;
struct ScriptContext;

#define DAILY_SLOT_COUNT        WORLD_ITEM_SPOT_COUNT   // item spots now; trainer slots follow them later
#define DAILY_STAMP_NONE        0xFFFF                  // a day no real stamp equals: the first map load starts a fresh day

struct WayfarerDailySlots
{
    u16 stampDay;                                        // low 16 bits of the day the bits below belong to
    u8 clearedToday[(DAILY_SLOT_COUNT + 7) / 8];         // item spots picked up today
};

// Kinds of deterministic draw (the `kind` input of the hash).
enum DailyRollKind
{
    DAILY_ROLL_ITEM_EMPTY,
    DAILY_ROLL_ITEM_TIER,
    DAILY_ROLL_ITEM_PICK,
    DAILY_ROLL_ITEM_GROUP,
};

// Shape of a spot row's attrs.
#define WORLD_ITEM_SPOT_BALL        0
#define WORLD_ITEM_SPOT_HIDDEN      1
#define WORLD_ITEM_SPOT_DYNAMIC     0
#define WORLD_ITEM_SPOT_PRIZE       1

enum WorldItemTier
{
    WORLD_ITEM_TIER_ROAD,
    WORLD_ITEM_TIER_WILDS,
    WORLD_ITEM_TIER_OUTLANDS,
    WORLD_ITEM_TIER_COUNT,
};

enum WorldItemRegion
{
    WORLD_ITEM_REGION_KANTO,
    WORLD_ITEM_REGION_SEVII,
    WORLD_ITEM_REGION_JOHTO,
    WORLD_ITEM_REGION_ALOLA,
    WORLD_ITEM_REGION_SINJOH,
    WORLD_ITEM_REGION_HOENN,
    WORLD_ITEM_REGION_COUNT,
};

// attrs: bit 0 hidden, bit 1 prize, bits 2-3 reach tier, bits 4-6 region.
#define WORLD_ITEM_ATTRS(hidden, prize, tier, region) ((hidden) | ((prize) << 1) | ((tier) << 2) | ((region) << 4))
#define WORLD_ITEM_ATTR_HIDDEN(attrs)   ((attrs) & 1)
#define WORLD_ITEM_ATTR_PRIZE(attrs)    (((attrs) >> 1) & 1)
#define WORLD_ITEM_ATTR_TIER(attrs)     (((attrs) >> 2) & 3)
#define WORLD_ITEM_ATTR_REGION(attrs)   (((attrs) >> 4) & 7)

// Pool entries that stand for a whole group, picked among equally.
#define WORLD_ITEM_GROUP_STONE  0xFFFE
#define WORLD_ITEM_GROUP_TM     0xFFFD

struct WorldItemSpot
{
    u8 mapGroup;
    u8 mapNum;
    u8 id;      // local id of a ball, index among the map's bg events for a hidden item
    u8 attrs;
    u16 prize;  // the prize item, ITEM_NONE for a dynamic spot
    u16 flag;   // a prize's permanent flag exactly as the map assembles it, 0 for a dynamic spot
};

struct WorldItemPoolEntry
{
    u16 item;
    u8 weight;
};

struct WorldItemPool
{
    const struct WorldItemPoolEntry *entries;
    u8 count;
    u16 totalWeight;
};

#define NO_ITEM_SPOT 0xFFFF

// Day and draws.
u32 Hash32(u32 seed, u32 day, u32 kind, u32 a, u32 b);
u32 DailySlots_GetDay(void);
u32 DailySlots_GetSeed(void);
u32 DailySlots_Roll(enum DailyRollKind kind, u32 a, u32 b);
void DailySlots_InitNewGame(void);
void DailySlots_OnMapLoad(void);

// Item spots.
u16 DailyItems_FindSpot(u8 mapGroup, u8 mapNum, bool8 hidden, u8 id);
const struct WorldItemSpot *DailyItems_GetSpot(u16 index);
u16 DailyItems_ResolveSpot(u16 index);
u16 DailyItems_ResolveSpotTier(u16 index, u8 *poolTier);
bool8 DailyItems_IsCleared(u16 index);
void DailyItems_SetCleared(u16 index);
void DailyItems_RewriteTemplates(void);
bool8 DailyItems_HideTemplate(const struct ObjectEventTemplate *template);
bool8 DailyItems_ResolveHidden(u8 mapGroup, u8 mapNum, const struct BgEvent *bgEvent, u16 *item, u16 *flagId, u16 *spotIndex);
void DailyItems_SetPendingHidden(u16 spotIndex, u16 item, u16 flagId);
void DailyItems_ClearPendingHidden(void);
void DailyItems_PickedUpHidden(void);
// Natives of the shared ball script (item_ball_scripts_wayfarer.inc).
void DailyItems_ResolveBall(struct ScriptContext *ctx);
void DailyItems_MarkBallCleared(struct ScriptContext *ctx);
void DailyItems_ClearPendingHidden_NativeCall(struct ScriptContext *ctx);

#if TESTING || defined(E2E_TESTING)
#define DAILY_DEBUG_DAY     (1 << 0)
#define DAILY_DEBUG_SEED    (1 << 1)
#define DAILY_DEBUG_NO_EMPTY (1 << 2)   // no spot has an empty day: a journey needs the find to be there
// Debug and E2E builds can pin the day and the save seed so any draw is reproducible.
extern u32 gDailySlotsDebugDay;
extern u32 gDailySlotsDebugSeed;
extern u8 gDailySlotsDebugFlags;
// What the last managed pickup gave and how many there were since boot, for journeys to observe.
extern u16 gDailySlotsDebugFoundItem;
extern u16 gDailySlotsDebugFoundCount;
#endif

#endif // GUARD_WAYFARER_DAILY_SLOTS_H
