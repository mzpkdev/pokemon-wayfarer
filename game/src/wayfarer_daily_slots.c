// Daily world slots, item half (Wayfarer only). Every dynamic item spot and
// every taken prize spot draws a find each day; a prize spot gives its prize
// once. Specs: .product/specs/daily-world-slots.md (The day, Deterministic
// draws, Save state, The map-load hook, Items) and .product/specs/world-items.md.
//
// A spot's find is a pure function of the save's seed, the stamped day and the
// spot's slot index, so nothing about the draw is stored: reloading, saving
// or leaving and returning on the same day always gives the same result. Only
// the spots picked up today are saved (clearedToday).

#include "global.h"
#include "wayfarer_daily_slots.h"

#if IS_WAYFARER
#include "constants/items.h"
#include "constants/maps.h"
#include "event_data.h"
#include "field_control_avatar.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "rtc.h"
#include "script.h"
#include "data/item_slots/pools.h"
#include "data/item_slots/spots.h"

extern const u8 DailyItems_EventScript_Ball[];

#if TESTING || defined(E2E_TESTING)
EWRAM_DATA u32 gDailySlotsDebugDay = 0;
EWRAM_DATA u32 gDailySlotsDebugSeed = 0;
EWRAM_DATA u8 gDailySlotsDebugFlags = 0;
EWRAM_DATA u16 gDailySlotsDebugFoundItem = 0;
EWRAM_DATA u16 gDailySlotsDebugFoundCount = 0;
#endif

// The hidden item the player is picking up: its spot (plus one; zero is none), item
// and flag as the facing check resolved them. Only lives from the facing check to
// the pickup script; SetHiddenItemFlag marks the spot only when the script's
// variables still describe this pickup (Silph's Card Key doors share that special).
static EWRAM_DATA u16 sPendingHiddenPlusOne = 0;
static EWRAM_DATA u16 sPendingHiddenItem = 0;
static EWRAM_DATA u16 sPendingHiddenFlag = 0;

// ---------- the day and the draws ----------
static u32 RotateRight(u32 value, u32 bits)
{
    return (value >> bits) | (value << (32 - bits));
}

static u32 MixIn(u32 h, u32 k)
{
    k *= 0xCC9E2D51;
    k = RotateRight(k, 17);
    k *= 0x1B873593;
    h ^= k;
    h = RotateRight(h, 19);
    return h * 5 + 0xE6546B64;
}

// A fixed integer mix (murmur3's block step and finaliser): no state.
u32 Hash32(u32 seed, u32 day, u32 kind, u32 a, u32 b)
{
    u32 h = seed;

    h = MixIn(h, day);
    h = MixIn(h, kind);
    h = MixIn(h, a);
    h = MixIn(h, b);
    h ^= h >> 16;
    h *= 0x85EBCA6B;
    h ^= h >> 13;
    h *= 0xC2B2AE35;
    h ^= h >> 16;
    return h;
}

static struct WayfarerDailySlots *Daily(void)
{
    return &gPokemonStoragePtr->dailySlots;
}

u32 DailySlots_GetDay(void)
{
#if TESTING || defined(E2E_TESTING)
    if (gDailySlotsDebugFlags & DAILY_DEBUG_DAY)
        return gDailySlotsDebugDay;
#endif
    // The clock is read fresh: a map load runs this before the time-based events do.
    RtcCalcLocalTime();
    return RtcGetLocalDayCount();
}

u32 DailySlots_GetSeed(void)
{
#if TESTING || defined(E2E_TESTING)
    if (gDailySlotsDebugFlags & DAILY_DEBUG_SEED)
        return gDailySlotsDebugSeed;
#endif
    return READ_OTID_FROM_SAVE;
}

// Draws key on the stamped day, which is what the save's cleared bits belong to.
u32 DailySlots_Roll(enum DailyRollKind kind, u32 a, u32 b)
{
    return Hash32(DailySlots_GetSeed(), Daily()->stampDay, kind, a, b);
}

// New Game doesn't clear PokemonStorage beyond the boxes, so the state clears
// itself. The first map load sees a stamp no real day equals and starts a day.
void DailySlots_InitNewGame(void)
{
    memset(Daily(), 0, sizeof(struct WayfarerDailySlots));
    Daily()->stampDay = DAILY_STAMP_NONE;
}

// A new day takes effect on the next map load (never on Continue).
void DailySlots_OnMapLoad(void)
{
    u16 day = (u16)DailySlots_GetDay();
    struct WayfarerDailySlots *daily = Daily();

    sPendingHiddenPlusOne = 0;
    if (daily->stampDay != day)
    {
        memset(daily->clearedToday, 0, sizeof(daily->clearedToday));
        daily->stampDay = day;
    }
}

// ---------- item spots ----------
static u32 SpotKey(const struct WorldItemSpot *spot)
{
    return (spot->mapGroup << 17) | (spot->mapNum << 9) | (WORLD_ITEM_ATTR_HIDDEN(spot->attrs) << 8) | spot->id;
}

static u32 MakeKey(u8 mapGroup, u8 mapNum, bool8 hidden, u8 id)
{
    return (mapGroup << 17) | (mapNum << 9) | ((hidden != FALSE) << 8) | id;
}

// First row whose key is not below `key`.
static u16 LowerBound(u32 key)
{
    u16 low = 0, high = WORLD_ITEM_SPOT_COUNT;

    while (low < high)
    {
        u16 middle = (low + high) / 2;

        if (SpotKey(&sWorldItemSpots[middle]) < key)
            low = middle + 1;
        else
            high = middle;
    }
    return low;
}

u16 DailyItems_FindSpot(u8 mapGroup, u8 mapNum, bool8 hidden, u8 id)
{
    u32 key = MakeKey(mapGroup, mapNum, hidden, id);
    u16 index = LowerBound(key);

    if (index < WORLD_ITEM_SPOT_COUNT && SpotKey(&sWorldItemSpots[index]) == key)
        return index;
    return NO_ITEM_SPOT;
}

const struct WorldItemSpot *DailyItems_GetSpot(u16 index)
{
    return &sWorldItemSpots[index];
}

bool8 DailyItems_IsCleared(u16 index)
{
    return (Daily()->clearedToday[index / 8] >> (index % 8)) & 1;
}

void DailyItems_SetCleared(u16 index)
{
    Daily()->clearedToday[index / 8] |= 1 << (index % 8);
}

// Today's find of a spot, ignoring whether it was picked up: ITEM_NONE on an
// empty day, otherwise a tier from the spot's reach, then an item by weight.
u16 DailyItems_ResolveSpot(u16 index)
{
    u8 poolTier;

    return DailyItems_ResolveSpotTier(index, &poolTier);
}

// The same, also reporting the pool tier rolled (0 Regular, 1 Better, 2 Special; 0xFF on an empty day).
u16 DailyItems_ResolveSpotTier(u16 index, u8 *poolTierOut)
{
    const struct WorldItemSpot *spot = &sWorldItemSpots[index];
    u32 tier = WORLD_ITEM_ATTR_TIER(spot->attrs);
    u32 region = WORLD_ITEM_ATTR_REGION(spot->attrs);
    const u8 *odds = sWorldItemTierOdds[tier];
    u32 roll, poolTier, weight;
    const struct WorldItemPool *pool;
    u32 i;
    u16 item;

    *poolTierOut = 0xFF;
#if TESTING || defined(E2E_TESTING)
    if (!(gDailySlotsDebugFlags & DAILY_DEBUG_NO_EMPTY))
#endif
    if (DailySlots_Roll(DAILY_ROLL_ITEM_EMPTY, index, 0) % 100 < WORLD_ITEM_EMPTY_PERCENT)
        return ITEM_NONE;
    roll = DailySlots_Roll(DAILY_ROLL_ITEM_TIER, index, 0) % 100;
    poolTier = roll < odds[0] ? 0 : roll < odds[1] ? 1 : 2;
    *poolTierOut = poolTier;
    pool = &sWorldItemPools[region][poolTier];
    weight = DailySlots_Roll(DAILY_ROLL_ITEM_PICK, index, poolTier) % pool->totalWeight;
    for (i = 0; i < pool->count; i++)
    {
        if (weight < pool->entries[i].weight)
            break;
        weight -= pool->entries[i].weight;
    }
    item = pool->entries[i].item;
    if (item == WORLD_ITEM_GROUP_STONE)
        return sWorldItemStones[DailySlots_Roll(DAILY_ROLL_ITEM_GROUP, index, poolTier) % ARRAY_COUNT(sWorldItemStones)];
    if (item == WORLD_ITEM_GROUP_TM)
        return sWorldItemDynamicTms[DailySlots_Roll(DAILY_ROLL_ITEM_GROUP, index, poolTier) % ARRAY_COUNT(sWorldItemDynamicTms)];
    return item;
}

static bool8 IsPrize(const struct WorldItemSpot *spot)
{
    return WORLD_ITEM_ATTR_PRIZE(spot->attrs);
}

static bool8 IsHidden(const struct WorldItemSpot *spot)
{
    return WORLD_ITEM_ATTR_HIDDEN(spot->attrs);
}

// The ball spot of the current map for a template's local id.
static u16 CurrentMapBallSpot(u8 localId)
{
    return DailyItems_FindSpot(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, FALSE, localId);
}

// Map load, and Continue after the saved scripts are reset to the authored
// ones: an untaken prize keeps its authored template and permanent flag;
// every other ball spot loses its flag, so the pickup's removeobject sets
// nothing permanent. All of them run the shared ball script.
void DailyItems_RewriteTemplates(void)
{
    u16 index = LowerBound(MakeKey(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, FALSE, 0));

    for (; index < WORLD_ITEM_SPOT_COUNT; index++)
    {
        const struct WorldItemSpot *spot = &sWorldItemSpots[index];
        struct ObjectEventTemplate *template;

        if (spot->mapGroup != gSaveBlock1Ptr->location.mapGroup || spot->mapNum != gSaveBlock1Ptr->location.mapNum)
            break;
        if (IsHidden(spot) || spot->id > gMapHeader.events->objectEventCount)
            continue;
        template = &gSaveBlock1Ptr->objectEventTemplates[spot->id - 1];
        template->script = DailyItems_EventScript_Ball;
        // The generated flag is the one the map assembled; an untaken prize keeps it, any other spot has none.
        template->flagId = (IsPrize(spot) && !FlagGet(spot->flag)) ? spot->flag : 0;
    }
}

// Spawn check (TrySpawnObjectEvents): a dynamic or taken-prize ball spawns only
// when today's find is there and not picked up. An untaken prize keeps its
// flag, which already hides it once taken, so it is never hidden here.
bool8 DailyItems_HideTemplate(const struct ObjectEventTemplate *template)
{
    u16 index;

    if (template->flagId != 0)
        return FALSE;
    index = CurrentMapBallSpot(template->localId);
    if (index == NO_ITEM_SPOT)
        return FALSE;
    return DailyItems_IsCleared(index) || DailyItems_ResolveSpot(index) == ITEM_NONE;
}

// The item a ball gives now: the prize until it is taken, then today's find.
static u16 BallItem(u16 index, u8 localId)
{
    const struct WorldItemSpot *spot = &sWorldItemSpots[index];

    if (IsPrize(spot) && !FlagGet(spot->flag))
        return spot->prize;
    return DailyItems_ResolveSpot(index);
}

// Hidden items. Stateless, so an Itemfinder sweep of a connected map resolves
// the neighbour's spots the same way. Present when the find is there and not
// yet picked up (an untaken prize: when its permanent flag is unset).
bool8 DailyItems_ResolveHidden(u8 mapGroup, u8 mapNum, const struct BgEvent *bgEvent, u16 *item, u16 *flagId, u16 *spotIndex)
{
    const struct MapHeader *header = (mapGroup == gSaveBlock1Ptr->location.mapGroup && mapNum == gSaveBlock1Ptr->location.mapNum)
        ? &gMapHeader : Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    u16 index;
    const struct WorldItemSpot *spot;

    if (spotIndex != NULL)
        *spotIndex = NO_ITEM_SPOT;
    *item = bgEvent->bgUnion.hiddenItem.item;
    index = DailyItems_FindSpot(mapGroup, mapNum, TRUE, bgEvent - header->events->bgEvents);
    if (index == NO_ITEM_SPOT)
    {
        // A fixed spot stays on its authored path.
        *flagId = GetHiddenItemFlagId(bgEvent);
        return !FlagGet(*flagId);
    }
    spot = &sWorldItemSpots[index];
    if (spotIndex != NULL)
        *spotIndex = index;
    // A prize's flag comes from the generated table: the packed bg event cannot carry a Hoenn flag.
    *flagId = spot->flag;
    if (IsPrize(spot) && !FlagGet(spot->flag))
    {
        *item = spot->prize;
        return TRUE;
    }
    *flagId = 0;
    *item = DailyItems_ResolveSpot(index);
    return *item != ITEM_NONE && !DailyItems_IsCleared(index);
}

void DailyItems_SetPendingHidden(u16 spotIndex, u16 item, u16 flagId)
{
    sPendingHiddenPlusOne = spotIndex == NO_ITEM_SPOT ? 0 : spotIndex + 1;
    sPendingHiddenItem = item;
    sPendingHiddenFlag = flagId;
}

// The full-bag path of the hidden item script: nothing was picked up.
void DailyItems_ClearPendingHidden(void)
{
    sPendingHiddenPlusOne = 0;
}

void DailyItems_ClearPendingHidden_NativeCall(struct ScriptContext *ctx)
{
    DailyItems_ClearPendingHidden();
}

// SetHiddenItemFlag runs after the item went into the bag. Silph's Card Key doors
// call it too, so only a pickup whose flag is still the pending one counts (the item can
// change under the randomizer, the flag cannot).
void DailyItems_PickedUpHidden(void)
{
    if (sPendingHiddenPlusOne != 0 && gSpecialVar_0x8004 == sPendingHiddenFlag)
    {
        DailyItems_SetCleared(sPendingHiddenPlusOne - 1);
#if TESTING || defined(E2E_TESTING)
        gDailySlotsDebugFoundItem = sPendingHiddenItem;
        gDailySlotsDebugFoundCount++;
#endif
    }
    sPendingHiddenPlusOne = 0;
}

// Script natives of the shared ball script (data/scripts/item_ball_scripts_wayfarer.inc).
void DailyItems_ResolveBall(struct ScriptContext *ctx)
{
    u16 index = CurrentMapBallSpot(gSpecialVar_LastTalked);

    gSpecialVar_0x8000 = index == NO_ITEM_SPOT ? ITEM_NONE : BallItem(index, gSpecialVar_LastTalked);
    gSpecialVar_0x8001 = 1;
}

void DailyItems_MarkBallCleared(struct ScriptContext *ctx)
{
    u16 index = CurrentMapBallSpot(gSpecialVar_LastTalked);

    if (index != NO_ITEM_SPOT)
    {
        DailyItems_SetCleared(index);
#if TESTING || defined(E2E_TESTING)
        gDailySlotsDebugFoundItem = gSpecialVar_0x8000;
        gDailySlotsDebugFoundCount++;
#endif
    }
}

#endif // IS_WAYFARER
