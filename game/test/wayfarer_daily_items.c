#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_control_avatar.h"
#include "fieldmap.h"
#include "item.h"
#include "item_use.h"
#include "malloc.h"
#include "new_game.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "task.h"
#include "wayfarer_appearance.h"
#include "wayfarer_daily_slots.h"
#include "wayfarer_persistence.h"
#include "wayfarer_origin.h"
#include "test/test.h"
#include "constants/event_bg.h"
#include "constants/items.h"
#include "constants/maps.h"

#if IS_WAYFARER

// Daily world slots, item half (.product/specs/daily-world-slots.md, Items; world-items.md).

extern const u8 DailyItems_EventScript_Ball[];

struct Snapshot
{
    struct MapHeader header;
    struct WarpData location;
    struct WayfarerDailySlots daily;
    struct ObjectEventTemplate templates[OBJECT_EVENT_TEMPLATES_COUNT];
    u8 flags[sizeof(gSaveBlock1Ptr->flags)];
    u32 debugDay, debugSeed;
    u8 debugFlags;
};

static struct Snapshot *TakeSnapshot(void)
{
    struct Snapshot *snapshot = Alloc(sizeof(*snapshot));

    ASSUME(snapshot != NULL);
    snapshot->header = gMapHeader;
    snapshot->location = gSaveBlock1Ptr->location;
    snapshot->daily = gPokemonStoragePtr->dailySlots;
    memcpy(snapshot->templates, gSaveBlock1Ptr->objectEventTemplates, sizeof(snapshot->templates));
    memcpy(snapshot->flags, gSaveBlock1Ptr->flags, sizeof(snapshot->flags));
    snapshot->debugDay = gDailySlotsDebugDay;
    snapshot->debugSeed = gDailySlotsDebugSeed;
    snapshot->debugFlags = gDailySlotsDebugFlags;
    return snapshot;
}

static void RestoreSnapshot(struct Snapshot *snapshot)
{
    gMapHeader = snapshot->header;
    gSaveBlock1Ptr->location = snapshot->location;
    gPokemonStoragePtr->dailySlots = snapshot->daily;
    memcpy(gSaveBlock1Ptr->objectEventTemplates, snapshot->templates, sizeof(snapshot->templates));
    memcpy(gSaveBlock1Ptr->flags, snapshot->flags, sizeof(snapshot->flags));
    gDailySlotsDebugDay = snapshot->debugDay;
    gDailySlotsDebugSeed = snapshot->debugSeed;
    gDailySlotsDebugFlags = snapshot->debugFlags;
    Free(snapshot);
}

static void PinDraws(u32 day, u32 seed)
{
    gDailySlotsDebugFlags = DAILY_DEBUG_DAY | DAILY_DEBUG_SEED;
    gDailySlotsDebugDay = day;
    gDailySlotsDebugSeed = seed;
}

static void EnterMap(u8 mapGroup, u8 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    LoadObjEventTemplatesFromHeader();
}

static void EnterSpotMap(u16 index)
{
    const struct WorldItemSpot *spot = DailyItems_GetSpot(index);

    EnterMap(spot->mapGroup, spot->mapNum);
}

static struct ObjectEventTemplate *BallTemplate(u16 index)
{
    return &gSaveBlock1Ptr->objectEventTemplates[DailyItems_GetSpot(index)->id - 1];
}

static const struct ObjectEventTemplate *RomTemplate(u16 index)
{
    return &Overworld_GetMapHeaderByGroupAndId(DailyItems_GetSpot(index)->mapGroup, DailyItems_GetSpot(index)->mapNum)->events
        ->objectEvents[DailyItems_GetSpot(index)->id - 1];
}

static const struct BgEvent *HiddenEvent(u16 index)
{
    const struct WorldItemSpot *spot = DailyItems_GetSpot(index);

    return &Overworld_GetMapHeaderByGroupAndId(spot->mapGroup, spot->mapNum)->events->bgEvents[spot->id];
}

// The first spot (from `start`) of a kind: ball or hidden, prize or dynamic.
static u16 FindSpot(bool32 hidden, bool32 prize, u16 start)
{
    u16 i;

    for (i = start; i < WORLD_ITEM_SPOT_COUNT; i++)
    {
        const struct WorldItemSpot *spot = DailyItems_GetSpot(i);

        if (WORLD_ITEM_ATTR_HIDDEN(spot->attrs) == hidden && WORLD_ITEM_ATTR_PRIZE(spot->attrs) == prize)
            return i;
    }
    return NO_ITEM_SPOT;
}

// A dynamic ball whose find is there on `day`: the ball spot and the day to use.
static u16 DynamicBallWithFind(u32 seed, u32 *day)
{
    u16 index = FindSpot(FALSE, FALSE, 0);

    for (*day = 1000; *day < 1100; (*day)++)
    {
        PinDraws(*day, seed);
        gPokemonStoragePtr->dailySlots.stampDay = *day;
        if (DailyItems_ResolveSpot(index) != ITEM_NONE)
            return index;
    }
    return NO_ITEM_SPOT;
}

TEST("Daily slots: the hash is a fixed mix of seed, day, kind and both inputs")
{
    EXPECT_EQ(Hash32(1, 2, 3, 4, 5), Hash32(1, 2, 3, 4, 5));
    EXPECT_NE(Hash32(1, 2, 3, 4, 5), Hash32(2, 2, 3, 4, 5));
    EXPECT_NE(Hash32(1, 2, 3, 4, 5), Hash32(1, 3, 3, 4, 5));
    EXPECT_NE(Hash32(1, 2, 3, 4, 5), Hash32(1, 2, 4, 4, 5));
    EXPECT_NE(Hash32(1, 2, 3, 4, 5), Hash32(1, 2, 3, 5, 5));
    EXPECT_NE(Hash32(1, 2, 3, 4, 5), Hash32(1, 2, 3, 4, 6));
    // Pinned: a changed mix would reshuffle every save's spots.
    EXPECT_EQ(Hash32(0, 0, 0, 0, 0), 0x176842CD);
    EXPECT_EQ(Hash32(0x12345678, 20000, DAILY_ROLL_ITEM_PICK, 123, 2), 0x3D5EAC93);
}

TEST("Daily slots: the save struct, new game and the day change")
{
    struct Snapshot *snapshot = TakeSnapshot();
    struct WayfarerDailySlots *daily = &gPokemonStoragePtr->dailySlots;

    PinDraws(500, 77);
    DailySlots_InitNewGame();
    EXPECT_EQ(daily->stampDay, DAILY_STAMP_NONE);
    DailySlots_OnMapLoad();
    EXPECT_EQ(daily->stampDay, 500);

    DailyItems_SetCleared(0);
    DailyItems_SetCleared(WORLD_ITEM_SPOT_COUNT - 1);
    EXPECT(DailyItems_IsCleared(0));
    EXPECT(DailyItems_IsCleared(WORLD_ITEM_SPOT_COUNT - 1));
    EXPECT(!DailyItems_IsCleared(1));
    // The same day keeps the bits across later map loads.
    DailySlots_OnMapLoad();
    EXPECT(DailyItems_IsCleared(0));
    // A new day clears them.
    PinDraws(501, 77);
    DailySlots_OnMapLoad();
    EXPECT_EQ(daily->stampDay, 501);
    EXPECT(!DailyItems_IsCleared(0));
    EXPECT(!DailyItems_IsCleared(WORLD_ITEM_SPOT_COUNT - 1));
    // Only the low 16 bits of the day are compared.
    DailyItems_SetCleared(3);
    PinDraws(501 + 0x10000, 77);
    DailySlots_OnMapLoad();
    EXPECT(DailyItems_IsCleared(3));
    // The sentinel is not a real day.
    PinDraws(DAILY_STAMP_NONE - 1, 77);
    DailySlots_OnMapLoad();
    EXPECT_EQ(daily->stampDay, DAILY_STAMP_NONE - 1);

    RestoreSnapshot(snapshot);
}

TEST("Daily slots: New Game on the same day inherits no cleared spot")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u32 day;
    u16 index;

    index = DynamicBallWithFind(5, &day);
    ASSUME(index != NO_ITEM_SPOT);
    PinDraws(day, 5);
    DailySlots_InitNewGame();
    DailySlots_OnMapLoad();
    DailyItems_SetCleared(index);
    EnterSpotMap(index);
    EXPECT(DailyItems_HideTemplate(BallTemplate(index)));

    // A New Game on the same day: the init clears the bits and its warp into the first map restarts the day.
    memset(gPokemonStoragePtr->dailySlots.clearedToday, 0xFF, sizeof(gPokemonStoragePtr->dailySlots.clearedToday));
    EXPECT(WayfarerConfirmPendingOrigin(ORIGIN_NEW_BARK));
    EXPECT(WayfarerConfirmPendingAppearance(APPEARANCE_GOLD));
    NewGameInitData();
    EXPECT_EQ(gPokemonStoragePtr->dailySlots.stampDay, DAILY_STAMP_NONE);
    EXPECT(!DailyItems_IsCleared(index));
    PinDraws(day, 5);
    // The first map load starts the day and the ball spawns as on any fresh day.
    EnterSpotMap(index);
    EXPECT_EQ(gPokemonStoragePtr->dailySlots.stampDay, day);
    EXPECT(!DailyItems_IsCleared(index));
    EXPECT(!DailyItems_HideTemplate(BallTemplate(index)));

    RestoreSnapshot(snapshot);
}

TEST("Daily slots: a draw repeats across reload and save and changes with the day")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 first[64], second[64], i, changed = 0;
    struct WayfarerDailySlots saved;

    PinDraws(2000, 0xA5A5A5A5);
    DailySlots_InitNewGame();
    DailySlots_OnMapLoad();
    for (i = 0; i < ARRAY_COUNT(first); i++)
        first[i] = DailyItems_ResolveSpot(i * 7);
    // Save, wipe and reload the struct: the draws come back unchanged.
    saved = gPokemonStoragePtr->dailySlots;
    memset(&gPokemonStoragePtr->dailySlots, 0, sizeof(saved));
    gPokemonStoragePtr->dailySlots = saved;
    for (i = 0; i < ARRAY_COUNT(second); i++)
        EXPECT_EQ(DailyItems_ResolveSpot(i * 7), first[i]);
    // The next day draws again.
    PinDraws(2001, 0xA5A5A5A5);
    DailySlots_OnMapLoad();
    for (i = 0; i < ARRAY_COUNT(second); i++)
        changed += DailyItems_ResolveSpot(i * 7) != first[i];
    EXPECT_GT(changed, ARRAY_COUNT(first) / 2);
    // Another save's seed draws differently on the same day.
    changed = 0;
    PinDraws(2000, 0x5A5A5A5A);
    gPokemonStoragePtr->dailySlots.stampDay = 2000;
    for (i = 0; i < ARRAY_COUNT(second); i++)
        changed += DailyItems_ResolveSpot(i * 7) != first[i];
    EXPECT_GT(changed, ARRAY_COUNT(first) / 2);

    RestoreSnapshot(snapshot);
}

// Empty and tier odds over many days stay within tolerance of the spec's numbers.
TEST("Daily slots: empty days and tier odds follow the spec within tolerance")
{
    struct Snapshot *snapshot = TakeSnapshot();
    static const u8 sOdds[WORLD_ITEM_TIER_COUNT][3] = { {70, 25, 5}, {50, 38, 12}, {30, 45, 25} };
    u32 reachTier = 0;
    u16 index = NO_ITEM_SPOT, i;
    u32 day, empty = 0, tiers[3] = {0};
    const u32 days = 4000;

    for (i = 0; i < WORLD_ITEM_TIER_COUNT; i++)
        PARAMETRIZE(reachTier = i);
    for (i = 0; i < WORLD_ITEM_SPOT_COUNT; i++)
    {
        if (WORLD_ITEM_ATTR_TIER(DailyItems_GetSpot(i)->attrs) == reachTier && !WORLD_ITEM_ATTR_PRIZE(DailyItems_GetSpot(i)->attrs))
        {
            index = i;
            break;
        }
    }
    ASSUME(index != NO_ITEM_SPOT);
    for (day = 0; day < days; day++)
    {
        u8 tier;

        PinDraws(day, 0xC0FFEE);
        gPokemonStoragePtr->dailySlots.stampDay = day;
        if (DailyItems_ResolveSpotTier(index, &tier) == ITEM_NONE)
        {
            empty++;
            EXPECT_EQ(tier, 0xFF);
        }
        else
            tiers[tier]++;
    }
    // 25% empty, within 3 points; the rest split by the reach's odds, within 4 points of the live days.
    EXPECT_GT(empty, days * 22 / 100);
    EXPECT_LT(empty, days * 28 / 100);
    for (i = 0; i < 3; i++)
    {
        u32 live = days - empty;
        u32 target = live * sOdds[reachTier][i] / 100;
        EXPECT_GT(tiers[i] + live * 4 / 100, target);
        EXPECT_LT(tiers[i], target + live * 4 / 100);
    }

    RestoreSnapshot(snapshot);
}

TEST("Daily slots: an untaken prize ball keeps its template and flag and gives the prize")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index = FindSpot(FALSE, TRUE, 0);
    u32 checked = 0;

    PinDraws(3000, 9);
    DailySlots_InitNewGame();
    for (; index != NO_ITEM_SPOT; index = FindSpot(FALSE, TRUE, index + 1))
    {
        const struct WorldItemSpot *spot = DailyItems_GetSpot(index);
        struct ObjectEventTemplate *template;

        DailySlots_OnMapLoad();
        EnterSpotMap(index);
        template = BallTemplate(index);
        // The authored flag stays, the authored script never runs, and the ball is never hidden.
        EXPECT_EQ(template->flagId, RomTemplate(index)->flagId);
        EXPECT_NE(template->flagId, 0);
        EXPECT(template->script == DailyItems_EventScript_Ball);
        EXPECT(!DailyItems_HideTemplate(template));
        gSpecialVar_LastTalked = spot->id;
        DailyItems_ResolveBall(NULL);
        EXPECT_EQ(gSpecialVar_0x8000, spot->prize);
        EXPECT_NE(spot->prize, ITEM_NONE);
        checked++;
    }
    EXPECT_GT(checked, 100);
    RestoreSnapshot(snapshot);
}

TEST("Daily slots: an untaken prize hidden item gives the prize, never the authored item")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index;
    u32 checked = 0, replaced = 0;

    PinDraws(3001, 9);
    DailySlots_InitNewGame();
    DailySlots_OnMapLoad();
    for (index = FindSpot(TRUE, TRUE, 0); index != NO_ITEM_SPOT; index = FindSpot(TRUE, TRUE, index + 1))
    {
        const struct WorldItemSpot *spot = DailyItems_GetSpot(index);
        const struct BgEvent *bgEvent = HiddenEvent(index);
        u16 item, flagId, found;

        EnterSpotMap(index);
        EXPECT(DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, bgEvent, &item, &flagId, &found));
        EXPECT_EQ(item, spot->prize);
        EXPECT_EQ(found, index);
        // The spot's own flag stays: picking it up sets it for good.
        EXPECT_EQ(flagId, spot->flag);
        EXPECT_NE(flagId, 0);
        replaced += bgEvent->bgUnion.hiddenItem.item != spot->prize;
        checked++;
    }
    EXPECT_GT(checked, 60);
    EXPECT_GT(replaced, 30);
    RestoreSnapshot(snapshot);
}

TEST("Daily slots: a dynamic ball stays visible after reload and refills next day with no permanent flag")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u32 day, next;
    u16 index;
    u8 flagsBefore[sizeof(gSaveBlock1Ptr->flags)];

    index = DynamicBallWithFind(11, &day);
    ASSUME(index != NO_ITEM_SPOT);
    PinDraws(day, 11);
    DailySlots_InitNewGame();
    EnterSpotMap(index);
    memcpy(flagsBefore, gSaveBlock1Ptr->flags, sizeof(flagsBefore));
    EXPECT_EQ(BallTemplate(index)->flagId, 0);
    EXPECT(BallTemplate(index)->script == DailyItems_EventScript_Ball);
    EXPECT(!DailyItems_HideTemplate(BallTemplate(index)));

    // Reload on the same day (save and Continue): still there, nothing resolved differently.
    EnterSpotMap(index);
    EXPECT(!DailyItems_HideTemplate(BallTemplate(index)));
    LoadSaveblockObjEventScripts();
    DailyItems_RewriteTemplates();
    EXPECT(BallTemplate(index)->script == DailyItems_EventScript_Ball);
    EXPECT(!DailyItems_HideTemplate(BallTemplate(index)));

    // Picked up: gone for the day on every spawn check, reload and re-entry.
    gSpecialVar_LastTalked = DailyItems_GetSpot(index)->id;
    DailyItems_MarkBallCleared(NULL);
    EXPECT(DailyItems_HideTemplate(BallTemplate(index)));
    EnterSpotMap(index);
    EXPECT(DailyItems_HideTemplate(BallTemplate(index)));
    LoadSaveblockObjEventScripts();
    DailyItems_RewriteTemplates();
    EXPECT(DailyItems_HideTemplate(BallTemplate(index)));
    EXPECT_EQ(memcmp(flagsBefore, gSaveBlock1Ptr->flags, sizeof(flagsBefore)), 0);

    // The next day it draws again (on a day it is not empty).
    for (next = day + 1; next < day + 60; next++)
    {
        PinDraws(next, 11);
        DailySlots_OnMapLoad();
        if (DailyItems_ResolveSpot(index) != ITEM_NONE)
            break;
    }
    EnterSpotMap(index);
    EXPECT(!DailyItems_IsCleared(index));
    EXPECT(!DailyItems_HideTemplate(BallTemplate(index)));
    EXPECT_EQ(memcmp(flagsBefore, gSaveBlock1Ptr->flags, sizeof(flagsBefore)), 0);

    RestoreSnapshot(snapshot);
}

TEST("Daily slots: an empty day's ball never spawns")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index = FindSpot(FALSE, FALSE, 0);
    u32 day;
    bool32 found = FALSE;

    for (day = 100; day < 200 && !found; day++)
    {
        PinDraws(day, 21);
        DailySlots_InitNewGame();
        DailySlots_OnMapLoad();
        found = DailyItems_ResolveSpot(index) == ITEM_NONE;
    }
    ASSUME(found);
    EnterSpotMap(index);
    EXPECT(DailyItems_HideTemplate(BallTemplate(index)));
    RestoreSnapshot(snapshot);
}

TEST("Daily slots: a taken prize ball draws like a dynamic spot and stays gone once picked up")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index = FindSpot(FALSE, TRUE, 0);
    u32 day;
    bool32 found = FALSE;

    ASSUME(index != NO_ITEM_SPOT);
    for (day = 300; day < 400 && !found; day++)
    {
        PinDraws(day, 31);
        DailySlots_InitNewGame();
        DailySlots_OnMapLoad();
        found = DailyItems_ResolveSpot(index) != ITEM_NONE;
    }
    ASSUME(found);
    EnterSpotMap(index);
    FlagSet(RomTemplate(index)->flagId);
    // The next map load after the prize was taken: no flag, today's find.
    EnterSpotMap(index);
    EXPECT_EQ(BallTemplate(index)->flagId, 0);
    EXPECT(!DailyItems_HideTemplate(BallTemplate(index)));
    gSpecialVar_LastTalked = DailyItems_GetSpot(index)->id;
    DailyItems_ResolveBall(NULL);
    EXPECT_EQ(gSpecialVar_0x8000, DailyItems_ResolveSpot(index));
    EXPECT_NE(gSpecialVar_0x8000, DailyItems_GetSpot(index)->prize);
    DailyItems_MarkBallCleared(NULL);
    EnterSpotMap(index);
    EXPECT(DailyItems_HideTemplate(BallTemplate(index)));
    RestoreSnapshot(snapshot);
}

// The prize pickup sets the flag and today's bit; the next day the spot draws dynamically.
TEST("Daily slots: a picked-up prize ball becomes dynamic the next day")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index = FindSpot(FALSE, TRUE, 0);
    u32 day = 700;

    ASSUME(index != NO_ITEM_SPOT);
    PinDraws(day, 41);
    DailySlots_InitNewGame();
    EnterSpotMap(index);
    EXPECT_NE(BallTemplate(index)->flagId, 0);
    // Std_FindItem's removeobject sets the template's flag, the script's tail the spot's bit.
    FlagSet(BallTemplate(index)->flagId);
    gSpecialVar_LastTalked = DailyItems_GetSpot(index)->id;
    DailyItems_MarkBallCleared(NULL);
    EXPECT(DailyItems_IsCleared(index));
    EnterSpotMap(index);
    EXPECT_EQ(BallTemplate(index)->flagId, 0);
    EXPECT(DailyItems_HideTemplate(BallTemplate(index)));
    // Next day: the bit is gone and the spot resolves like any dynamic one.
    for (day = 701; day < 760; day++)
    {
        PinDraws(day, 41);
        DailySlots_OnMapLoad();
        if (DailyItems_ResolveSpot(index) != ITEM_NONE)
            break;
    }
    EnterSpotMap(index);
    EXPECT(!DailyItems_HideTemplate(BallTemplate(index)));
    EXPECT_NE(DailyItems_ResolveSpot(index), DailyItems_GetSpot(index)->prize);
    RestoreSnapshot(snapshot);
}

TEST("Daily slots: Continue rebuilds today's balls from the saved day without a day change")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u32 day;
    u16 index;

    index = DynamicBallWithFind(51, &day);
    ASSUME(index != NO_ITEM_SPOT);
    PinDraws(day, 51);
    DailySlots_InitNewGame();
    EnterSpotMap(index);
    // The real clock moved on, but Continue applies no day change: the stamp stays.
    PinDraws(day + 5, 51);
    LoadSaveblockObjEventScripts();
    DailyItems_RewriteTemplates();
    EXPECT_EQ(gPokemonStoragePtr->dailySlots.stampDay, day);
    EXPECT(BallTemplate(index)->script == DailyItems_EventScript_Ball);
    EXPECT(!DailyItems_HideTemplate(BallTemplate(index)));
    // The next map load starts the new day.
    EnterSpotMap(index);
    EXPECT_EQ(gPokemonStoragePtr->dailySlots.stampDay, day + 5);
    RestoreSnapshot(snapshot);
}

TEST("Daily slots: a dynamic hidden item resolves, is picked up once a day and sets no flag")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index = FindSpot(TRUE, FALSE, 0);
    u32 day;
    bool32 found = FALSE;
    u8 flagsBefore[sizeof(gSaveBlock1Ptr->flags)];
    u16 item, flagId, spotIndex;
    const struct WorldItemSpot *spot;

    ASSUME(index != NO_ITEM_SPOT);
    spot = DailyItems_GetSpot(index);
    for (day = 900; day < 1000 && !found; day++)
    {
        PinDraws(day, 61);
        DailySlots_InitNewGame();
        DailySlots_OnMapLoad();
        found = DailyItems_ResolveSpot(index) != ITEM_NONE;
    }
    ASSUME(found);
    EnterSpotMap(index);
    memcpy(flagsBefore, gSaveBlock1Ptr->flags, sizeof(flagsBefore));
    EXPECT(DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, HiddenEvent(index), &item, &flagId, &spotIndex));
    EXPECT_EQ(item, DailyItems_ResolveSpot(index));
    EXPECT_EQ(flagId, 0);
    EXPECT_EQ(spotIndex, index);

    DailyItems_SetPendingHidden(spotIndex, item, flagId);
    gSpecialVar_0x8004 = flagId;
    gSpecialVar_0x8005 = item;
    DailyItems_PickedUpHidden();
    EXPECT(DailyItems_IsCleared(index));
    EXPECT(!DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, HiddenEvent(index), &item, &flagId, NULL));
    EXPECT_EQ(memcmp(flagsBefore, gSaveBlock1Ptr->flags, sizeof(flagsBefore)), 0);
    // A pickup without a pending hidden spot (an unmanaged one) clears nothing more.
    DailyItems_PickedUpHidden();
    // Next day it is back.
    for (day = 901; day < 960; day++)
    {
        PinDraws(day, 61);
        DailySlots_OnMapLoad();
        if (DailyItems_ResolveSpot(index) != ITEM_NONE)
            break;
    }
    EXPECT(DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, HiddenEvent(index), &item, &flagId, NULL));
    RestoreSnapshot(snapshot);
}

TEST("Daily slots: a taken prize hidden item draws like a dynamic spot")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index = FindSpot(TRUE, TRUE, 0);
    const struct WorldItemSpot *spot = DailyItems_GetSpot(index);
    const struct BgEvent *bgEvent = HiddenEvent(index);
    u16 item, flagId, found;
    u32 day;
    bool32 present = FALSE;

    PinDraws(1200, 71);
    DailySlots_InitNewGame();
    DailySlots_OnMapLoad();
    EnterSpotMap(index);
    EXPECT(DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, bgEvent, &item, &flagId, &found));
    EXPECT_EQ(item, spot->prize);
    // Pick it up: the flag and today's bit.
    FlagSet(flagId);
    DailyItems_SetPendingHidden(found, item, flagId);
    gSpecialVar_0x8004 = flagId;
    gSpecialVar_0x8005 = item;
    DailyItems_PickedUpHidden();
    EXPECT(!DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, bgEvent, &item, &flagId, NULL));
    for (day = 1201; day < 1260 && !present; day++)
    {
        PinDraws(day, 71);
        DailySlots_OnMapLoad();
        present = DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, bgEvent, &item, &flagId, NULL);
    }
    EXPECT(present);
    EXPECT_EQ(flagId, 0);
    EXPECT_NE(item, spot->prize);
    RestoreSnapshot(snapshot);
}

TEST("Daily slots: a Hoenn hidden prize keeps a Hoenn flag that no story flag aliases")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index;
    u32 checked = 0;

    PinDraws(3100, 9);
    DailySlots_InitNewGame();
    DailySlots_OnMapLoad();
    for (index = FindSpot(TRUE, TRUE, 0); index != NO_ITEM_SPOT; index = FindSpot(TRUE, TRUE, index + 1))
    {
        const struct WorldItemSpot *spot = DailyItems_GetSpot(index);
        const struct BgEvent *bgEvent = HiddenEvent(index);
        u16 item, flagId, flagBefore;

        if (WORLD_ITEM_ATTR_REGION(spot->attrs) != WORLD_ITEM_REGION_HOENN)
            continue;
        EnterSpotMap(index);
        // The Hoenn flag namespace; the packed bg event decodes to a different, aliased story flag instead.
        EXPECT(IS_HOENN_FLAG_ID(spot->flag));
        EXPECT_NE(GetHiddenItemFlagId(bgEvent), spot->flag);
        // Setting the aliased story flag must not hide the prize.
        flagBefore = FlagGet(GetHiddenItemFlagId(bgEvent));
        FlagSet(GetHiddenItemFlagId(bgEvent));
        EXPECT(DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, bgEvent, &item, &flagId, NULL));
        EXPECT_EQ(item, spot->prize);
        EXPECT_EQ(flagId, spot->flag);
        if (!flagBefore)
            FlagClear(GetHiddenItemFlagId(bgEvent));
        // Its own flag does.
        FlagSet(spot->flag);
        EXPECT(!DailyItems_ResolveHidden(spot->mapGroup, spot->mapNum, bgEvent, &item, &flagId, NULL) || item != spot->prize);
        FlagClear(spot->flag);
        checked++;
    }
    EXPECT_GT(checked, 10);
    RestoreSnapshot(snapshot);
}

TEST("Daily slots: a stale pending hidden pickup marks nothing")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 index = FindSpot(TRUE, FALSE, 0);

    PinDraws(3200, 9);
    DailySlots_InitNewGame();
    DailySlots_OnMapLoad();
    // The full-bag path clears the pending pickup.
    DailyItems_SetPendingHidden(index, ITEM_POTION, 0);
    DailyItems_ClearPendingHidden();
    gSpecialVar_0x8004 = 0;
    DailyItems_PickedUpHidden();
    EXPECT(!DailyItems_IsCleared(index));
    // A special call for something else (Silph's Card Key doors) does not match the pending flag.
    DailyItems_SetPendingHidden(index, ITEM_POTION, 0);
    gSpecialVar_0x8004 = 0x123;
    DailyItems_PickedUpHidden();
    EXPECT(!DailyItems_IsCleared(index));
    // The pending value is gone after that call, so the real pickup needs its own facing check.
    gSpecialVar_0x8004 = 0;
    DailyItems_PickedUpHidden();
    EXPECT(!DailyItems_IsCleared(index));
    DailyItems_SetPendingHidden(index, ITEM_POTION, 0);
    DailyItems_PickedUpHidden();
    EXPECT(DailyItems_IsCleared(index));
    RestoreSnapshot(snapshot);
}

static bool32 ItemfinderFindsItemUnderfoot(const struct MapEvents *events, u8 taskId)
{
    gTasks[taskId].data[0] = 0;
    gTasks[taskId].data[1] = 0;
    gTasks[taskId].data[2] = FALSE;
    return ItemfinderCheckForHiddenItems(events, taskId)
        && gTasks[taskId].data[0] == 0
        && gTasks[taskId].data[1] == 0;
}

TEST("Daily slots: the Itemfinder sees only the resolved hidden items")
{
    struct Snapshot *snapshot = TakeSnapshot();
    struct PlayerAvatar playerAvatar = gPlayerAvatar;
    struct ObjectEvent playerObject = gObjectEvents[0];
    u8 taskId = CreateTask(TaskDummy, 0);
    u16 index;
    u32 checked = 0, seen = 0, hidden = 0;
    bool32 passed = TRUE;

    PinDraws(1500, 81);
    DailySlots_InitNewGame();
    DailySlots_OnMapLoad();
    gPlayerAvatar.objectEventId = 0;
    for (index = 0; index < WORLD_ITEM_SPOT_COUNT; index++)
    {
        const struct WorldItemSpot *spot = DailyItems_GetSpot(index);
        const struct BgEvent *bgEvent;
        bool32 expected;

        if (!WORLD_ITEM_ATTR_HIDDEN(spot->attrs))
            continue;
        EnterSpotMap(index);
        bgEvent = HiddenEvent(index);
        gObjectEvents[0].currentCoords.x = bgEvent->x + MAP_OFFSET;
        gObjectEvents[0].currentCoords.y = bgEvent->y + MAP_OFFSET;
        // An untaken prize is always there; a dynamic spot only when today's find is.
        expected = WORLD_ITEM_ATTR_PRIZE(spot->attrs) || DailyItems_ResolveSpot(index) != ITEM_NONE;
        // Other spots on this map sit elsewhere; look only at this tile.
        passed &= ItemfinderFindsItemUnderfoot(gMapHeader.events, taskId) == expected;
        if (expected)
        {
            seen++;
            DailyItems_SetCleared(index);
            if (WORLD_ITEM_ATTR_PRIZE(spot->attrs))
                FlagSet(spot->flag);
            passed &= !ItemfinderFindsItemUnderfoot(gMapHeader.events, taskId);
        }
        hidden++;
        checked++;
    }
    DestroyTask(taskId);
    gPlayerAvatar = playerAvatar;
    gObjectEvents[0] = playerObject;
    RestoreSnapshot(snapshot);
    EXPECT(passed);
    EXPECT_EQ(checked, hidden);
    // A quarter of the dynamic ones are empty today, so not every hidden item was seen.
    EXPECT_LT(seen, hidden);
    EXPECT_GT(seen, hidden / 2);
}

#endif // IS_WAYFARER
