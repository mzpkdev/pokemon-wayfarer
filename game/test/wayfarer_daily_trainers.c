#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "malloc.h"
#include "new_game.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "script.h"
#include "wayfarer_daily_slots.h"
#include "wayfarer_sevii_rematches.h"
#include "test/test.h"
#include "constants/flags.h"
#include "constants/maps.h"
#include "constants/opponents.h"
#include "constants/trainer_types.h"

#if IS_WAYFARER

// Daily world slots, trainer half (.product/specs/daily-world-slots.md, Trainers and Save state).

#define TEST_SEED   0x5EEDF00D

// Stands in for a trainerbattle's after-battle event script.
static EWRAM_DATA u8 sAfterBattleScript[1] = {0};

struct Snapshot
{
    struct MapHeader header;
    struct WarpData location;
    struct WayfarerDailySlots daily;
    struct ObjectEventTemplate templates[OBJECT_EVENT_TEMPLATES_COUNT];
    u8 flags[sizeof(gSaveBlock1Ptr->flags)];
    u8 rematches[MAX_REMATCH_ENTRIES];
    struct ObjectEvent object;
    u32 debugDay, debugSeed;
    u8 debugFlags, debugAbsence;
    u8 selected;
    u16 vars[VARS_COUNT];
    struct SaveBlock3 sb3;
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
    memcpy(snapshot->rematches, gSaveBlock1Ptr->trainerRematches, sizeof(snapshot->rematches));
    snapshot->object = gObjectEvents[0];
    snapshot->debugDay = gDailySlotsDebugDay;
    snapshot->debugSeed = gDailySlotsDebugSeed;
    snapshot->debugFlags = gDailySlotsDebugFlags;
    snapshot->debugAbsence = gDailySlotsDebugAbsence;
    snapshot->selected = gSelectedObjectEvent;
    memcpy(snapshot->vars, gSaveBlock1Ptr->vars, sizeof(snapshot->vars));
    snapshot->sb3 = *gSaveBlock3Ptr;
    return snapshot;
}

static void RestoreSnapshot(struct Snapshot *snapshot)
{
    gMapHeader = snapshot->header;
    gSaveBlock1Ptr->location = snapshot->location;
    gPokemonStoragePtr->dailySlots = snapshot->daily;
    memcpy(gSaveBlock1Ptr->objectEventTemplates, snapshot->templates, sizeof(snapshot->templates));
    memcpy(gSaveBlock1Ptr->flags, snapshot->flags, sizeof(snapshot->flags));
    memcpy(gSaveBlock1Ptr->trainerRematches, snapshot->rematches, sizeof(snapshot->rematches));
    gObjectEvents[0] = snapshot->object;
    gDailySlotsDebugDay = snapshot->debugDay;
    gDailySlotsDebugSeed = snapshot->debugSeed;
    gDailySlotsDebugFlags = snapshot->debugFlags;
    gDailySlotsDebugAbsence = snapshot->debugAbsence;
    gSelectedObjectEvent = snapshot->selected;
    memcpy(gSaveBlock1Ptr->vars, snapshot->vars, sizeof(snapshot->vars));
    *gSaveBlock3Ptr = snapshot->sb3;
    Free(snapshot);
}

static void PinDraws(u32 day, u32 seed)
{
    gDailySlotsDebugFlags = DAILY_DEBUG_DAY | DAILY_DEBUG_SEED;
    gDailySlotsDebugDay = day;
    gDailySlotsDebugSeed = seed;
}

// A new day: clears the day's bits and takes the snapshots, as the first map load of the day does.
static void StartDay(u32 day)
{
    PinDraws(day, TEST_SEED);
    DailySlots_OnMapLoad();
}

static void EnterMap(u8 mapGroup, u8 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    LoadObjEventTemplatesFromHeader();
}

static void EnterSlotMap(u16 slot)
{
    EnterMap(DailyTrainers_GetSlot(slot)->mapGroup, DailyTrainers_GetSlot(slot)->mapNum);
}

static const struct ObjectEventTemplate *RomTemplate(u16 slot, u8 object)
{
    const struct WorldTrainerSlot *row = DailyTrainers_GetSlot(slot);

    return &Overworld_GetMapHeaderByGroupAndId(row->mapGroup, row->mapNum)->events->objectEvents[row->localId[object] - 1];
}

static struct ObjectEventTemplate *LiveTemplate(u16 slot, u8 object)
{
    return &gSaveBlock1Ptr->objectEventTemplates[DailyTrainers_GetSlot(slot)->localId[object] - 1];
}

static void DefeatSlot(u16 slot)
{
    SetTrainerFlag(DailyTrainers_GetSlot(slot)->trainerId);
}

static void DefeatGroup(u8 group)
{
    u16 i;

    for (i = 0; i < DailyTrainers_GroupSize(group); i++)
        DefeatSlot(DailyTrainers_GroupSlot(group, i));
}

// The first group (from `start`) with at least `minSlots` slots, singles or pairs, spread over `minMaps` maps.
static u8 FindGroup(u8 start, u16 minSlots, bool32 pairs, u8 minMaps)
{
    u8 group;

    for (group = start; group < WORLD_TRAINER_GROUP_COUNT; group++)
    {
        u16 size = DailyTrainers_GroupSize(group), i, j;
        u8 maps = 0;

        if (size < minSlots || (DailyTrainers_GetSlot(DailyTrainers_GroupSlot(group, 0))->flags & WORLD_TRAINER_SLOT_PAIR ? 1 : 0) != (pairs ? 1 : 0))
            continue;
        for (i = 0; i < size; i++)
        {
            const struct WorldTrainerSlot *a = DailyTrainers_GetSlot(DailyTrainers_GroupSlot(group, i));
            bool32 seen = FALSE;

            for (j = 0; j < i; j++)
            {
                const struct WorldTrainerSlot *b = DailyTrainers_GetSlot(DailyTrainers_GroupSlot(group, j));

                seen |= a->mapGroup == b->mapGroup && a->mapNum == b->mapNum;
            }
            maps += !seen;
        }
        if (maps >= minMaps)
            return group;
    }
    return 0xFF;
}

// Number of positions whose occupant is a wanderer (not home, not away), and of away ones.
static void Count(const u16 *out, u16 size, u16 *placed, u16 *away)
{
    u16 i;

    *placed = *away = 0;
    for (i = 0; i < size; i++)
    {
        if (out[i] == DAILY_OCCUPANT_AWAY)
            (*away)++;
        else if (out[i] != DAILY_OCCUPANT_HOME)
            (*placed)++;
    }
}

static bool32 SameOccupants(const u16 *a, const u16 *b, u16 size)
{
    u16 i;

    for (i = 0; i < size; i++)
    {
        if (a[i] != b[i])
            return FALSE;
    }
    return TRUE;
}

TEST("Daily trainers: the slot tables are consistent")
{
    u16 slot, total = 0;
    u8 group;

    for (group = 0; group < WORLD_TRAINER_GROUP_COUNT; group++)
    {
        u16 i;

        EXPECT_GT(DailyTrainers_GroupSize(group), 0);
        EXPECT_LE(DailyTrainers_GroupSize(group), WORLD_TRAINER_GROUP_SLOTS_MAX);
        total += DailyTrainers_GroupSize(group);
        for (i = 0; i < DailyTrainers_GroupSize(group); i++)
        {
            slot = DailyTrainers_GroupSlot(group, i);
            EXPECT_EQ(DailyTrainers_GroupOf(slot), group);
            if (i > 0)
                EXPECT_GT(slot, DailyTrainers_GroupSlot(group, i - 1));
        }
    }
    // A home-range slot has no group; every other slot is in exactly one.
    for (slot = 0; slot < WORLD_TRAINER_SLOT_COUNT; slot++)
    {
        if (DailyTrainers_IsHomeRange(slot))
        {
            EXPECT_EQ(DailyTrainers_GetSlot(slot)->group, WORLD_TRAINER_GROUP_HOME);
            EXPECT_EQ(DailyTrainers_RangeSize(slot), 1);
            total++;
        }
        else
            EXPECT_LT(DailyTrainers_GetSlot(slot)->group, WORLD_TRAINER_GROUP_COUNT);
        EXPECT_LE(WORLD_TRAINER_SLOT_ABSENCE(DailyTrainers_GetSlot(slot)->flags), 7);
        EXPECT_LE(DailyTrainers_AbsencePercent(slot), 100);
    }
    EXPECT_EQ(total, WORLD_TRAINER_SLOT_COUNT);
    for (slot = 0; slot < WORLD_TRAINER_SLOT_COUNT; slot++)
    {
        const struct WorldTrainerSlot *row = DailyTrainers_GetSlot(slot);
        const struct ObjectEventTemplate *first = RomTemplate(slot, 0);

        EXPECT_EQ(DailyTrainers_FindSlot(row->mapGroup, row->mapNum, row->localId[0]), slot);
        EXPECT_EQ(first->graphicsId, row->gfx[0]);
        EXPECT(first->script == row->script[0]);
        EXPECT(first->trainerType == TRAINER_TYPE_NORMAL || first->trainerType == TRAINER_TYPE_SEE_ALL_DIRECTIONS);
        if (row->flags & WORLD_TRAINER_SLOT_PAIR)
        {
            EXPECT_NE(row->localId[1], 0);
            EXPECT_EQ(DailyTrainers_FindSlot(row->mapGroup, row->mapNum, row->localId[1]), slot);
            EXPECT_EQ(RomTemplate(slot, 1)->graphicsId, row->gfx[1]);
            EXPECT(RomTemplate(slot, 1)->script == row->script[1]);
        }
        else
            EXPECT_EQ(row->localId[1], 0);
    }
    EXPECT_EQ(DailyTrainers_FindSlot(0, 0, 1), NO_TRAINER_SLOT);
}

TEST("Daily trainers: the save struct fits its budget and New Game clears everything")
{
    struct Snapshot *snapshot = TakeSnapshot();
    struct WayfarerDailySlots *daily = &gPokemonStoragePtr->dailySlots;
    u16 slot = 0;

    EXPECT_LE(sizeof(struct WayfarerDailySlots), 330);
    // A day with cleared slots, a freeze and snapshots, then New Game.
    StartDay(700);
    DailyTrainers_SetCleared(slot);
    memset(daily->homeBeatenToday, 0xFF, sizeof(daily->homeBeatenToday));
    memset(daily->readyAtDayStart, 0xFF, sizeof(daily->readyAtDayStart));
    memset(daily->giftAtDayStart, 0xFF, sizeof(daily->giftAtDayStart));
    DailySlots_InitNewGame();
    EXPECT_EQ(daily->stampDay, DAILY_STAMP_NONE);
    EXPECT(!DailyTrainers_IsCleared(slot));
    EXPECT(!DailyTrainers_IsHomeBeaten(slot));
    EXPECT(!DailyContacts_ReadyAtDayStart(3));
    EXPECT(!DailyContacts_GiftAtDayStart(3));
    // The last trainer slot's bits are in range.
    DailyTrainers_SetCleared(WORLD_TRAINER_SLOT_COUNT - 1);
    EXPECT(DailyTrainers_IsCleared(WORLD_TRAINER_SLOT_COUNT - 1));
    EXPECT(!DailyTrainers_IsCleared(WORLD_TRAINER_SLOT_COUNT - 2));

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: a new day clears the cleared and home-beaten bits")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 slot = 5;

    DailySlots_InitNewGame();
    StartDay(800);
    DailyTrainers_SetCleared(slot);
    DailyTrainers_SetCleared(slot + 1);
    gPokemonStoragePtr->dailySlots.homeBeatenToday[0] |= 1 << 5;
    EXPECT(DailyTrainers_IsHomeBeaten(5));
    // The same day keeps them across map loads.
    DailySlots_OnMapLoad();
    EXPECT(DailyTrainers_IsCleared(slot));
    EXPECT(DailyTrainers_IsHomeBeaten(5));
    StartDay(801);
    EXPECT(!DailyTrainers_IsCleared(slot));
    EXPECT(!DailyTrainers_IsCleared(slot + 1));
    EXPECT(!DailyTrainers_IsHomeBeaten(5));

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: home slots show their home trainer until it is beaten")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 3, FALSE, 1);
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], size, i, placed, away;

    ASSUME(group != 0xFF);
    size = DailyTrainers_GroupSize(group);
    DailySlots_InitNewGame();
    StartDay(900);
    DailyTrainers_ResolveGroup(group, out);
    for (i = 0; i < size; i++)
        EXPECT_EQ(out[i], DAILY_OCCUPANT_HOME);
    // Beating one home trainer makes its slot rotate (tomorrow, when it is not beaten today): the first day
    // after, a group of one wanderer has either that wanderer at home or away.
    DefeatSlot(DailyTrainers_GroupSlot(group, 1));
    StartDay(901);
    DailyTrainers_ResolveGroup(group, out);
    for (i = 0; i < size; i++)
        EXPECT(i == 1 ? (out[i] == DailyTrainers_GroupSlot(group, 1) || out[i] == DAILY_OCCUPANT_AWAY) : out[i] == DAILY_OCCUPANT_HOME);
    Count(out, size, &placed, &away);
    EXPECT_EQ(placed + away, 1);

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: the shuffle repeats, changes with the day and the save, and places each wanderer once")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 6, FALSE, 3);
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], again[WORLD_TRAINER_GROUP_SLOTS_MAX], first[WORLD_TRAINER_GROUP_SLOTS_MAX];
    u16 size, i, j, day, differing = 0, placedTotal = 0, awayTotal = 0, placed, away;
    const u16 days = 300;

    ASSUME(group != 0xFF);
    size = DailyTrainers_GroupSize(group);
    DailySlots_InitNewGame();
    StartDay(1000);
    DefeatGroup(group);
    StartDay(1001);
    DailyTrainers_ResolveGroup(group, first);
    DailyTrainers_ResolveGroup(group, again);
    EXPECT(SameOccupants(first, again, size));
    for (day = 1001; day < 1001 + days; day++)
    {
        StartDay(day);
        DailyTrainers_ResolveGroup(group, out);
        // Every wanderer is at most once in the group; a slot holds a wanderer of the group.
        for (i = 0; i < size; i++)
        {
            if (out[i] == DAILY_OCCUPANT_AWAY)
                continue;
            EXPECT_EQ(DailyTrainers_GroupOf(out[i]), group);
            for (j = i + 1; j < size; j++)
                EXPECT_NE(out[i], out[j]);
        }
        Count(out, size, &placed, &away);
        EXPECT_EQ(placed + away, size);
        placedTotal += placed;
        awayTotal += away;
        differing += !SameOccupants(out, first, size);
    }
    EXPECT_GT(differing, days * 9 / 10);
    // About a quarter of the rotating slots are empty (25 percent, within 4 points over the sample).
    EXPECT_GT(awayTotal * 100, (u32)(placedTotal + awayTotal) * 21);
    EXPECT_LT(awayTotal * 100, (u32)(placedTotal + awayTotal) * 29);
    // The save's seed shuffles differently on the same day.
    StartDay(1001);
    gDailySlotsDebugSeed = TEST_SEED ^ 0x1234567;
    DailyTrainers_ResolveGroup(group, out);
    EXPECT(!SameOccupants(out, first, size));

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: every trainer is in at most one place a day across the world")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group;
    u16 day, slot;
    static EWRAM_DATA u8 seen[(WORLD_TRAINER_SLOT_COUNT + 7) / 8] = {0};

    DailySlots_InitNewGame();
    StartDay(1500);
    for (group = 0; group < WORLD_TRAINER_GROUP_COUNT; group++)
        DefeatGroup(group);
    for (day = 1501; day < 1521; day++)
    {
        StartDay(day);
        memset(seen, 0, sizeof(seen));
        for (group = 0; group < WORLD_TRAINER_GROUP_COUNT; group++)
        {
            u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], i;

            DailyTrainers_ResolveGroup(group, out);
            for (i = 0; i < DailyTrainers_GroupSize(group); i++)
            {
                if (out[i] == DAILY_OCCUPANT_AWAY)
                    continue;
                EXPECT_NE(out[i], DAILY_OCCUPANT_HOME);
                slot = out[i];
                EXPECT(!(seen[slot / 8] & (1 << (slot % 8))));
                seen[slot / 8] |= 1 << (slot % 8);
            }
        }
    }

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: pairs rotate as pairs among pairs")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 2, TRUE, 1);
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], size, i, day;
    bool32 swapped = FALSE;

    ASSUME(group != 0xFF);
    size = DailyTrainers_GroupSize(group);
    DailySlots_InitNewGame();
    StartDay(1600);
    DefeatGroup(group);
    for (day = 1601; day < 1700 && !swapped; day++)
    {
        StartDay(day);
        DailyTrainers_ResolveGroup(group, out);
        for (i = 0; i < size; i++)
        {
            u16 slot = DailyTrainers_GroupSlot(group, i);
            const struct WorldTrainerSlot *home = DailyTrainers_GetSlot(slot);

            EXPECT(home->flags & WORLD_TRAINER_SLOT_PAIR);
            if (out[i] == DAILY_OCCUPANT_AWAY || out[i] == slot)
                continue;
            // A different pair stands in this slot: both objects take that pair's sprites and scripts.
            EnterSlotMap(slot);
            EXPECT_EQ(LiveTemplate(slot, 0)->graphicsId, DailyTrainers_GetSlot(out[i])->gfx[0]);
            EXPECT_EQ(LiveTemplate(slot, 1)->graphicsId, DailyTrainers_GetSlot(out[i])->gfx[1]);
            EXPECT(LiveTemplate(slot, 0)->script == DailyTrainers_GetSlot(out[i])->script[0]);
            EXPECT(LiveTemplate(slot, 1)->script == DailyTrainers_GetSlot(out[i])->script[1]);
            // The slot keeps its own place and sight.
            EXPECT_EQ(LiveTemplate(slot, 0)->x, RomTemplate(slot, 0)->x);
            EXPECT_EQ(LiveTemplate(slot, 1)->y, RomTemplate(slot, 1)->y);
            swapped = TRUE;
        }
    }
    EXPECT(swapped);

    RestoreSnapshot(snapshot);
}

// A rotating slot's wanderer on a day it is there: the slot, the occupant and the day.
static bool32 FindWanderer(u8 group, u32 firstDay, u16 *slot, u16 *occupant, u32 *day)
{
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], i;

    for (*day = firstDay; *day < firstDay + 200; (*day)++)
    {
        StartDay(*day);
        DailyTrainers_ResolveGroup(group, out);
        for (i = 0; i < DailyTrainers_GroupSize(group); i++)
        {
            if (out[i] != DAILY_OCCUPANT_AWAY && out[i] != DAILY_OCCUPANT_HOME && out[i] != DailyTrainers_GroupSlot(group, i))
            {
                *slot = DailyTrainers_GroupSlot(group, i);
                *occupant = out[i];
                return TRUE;
            }
        }
    }
    return FALSE;
}

TEST("Daily trainers: the map hook gives a slot the wanderer's sprite and script and keeps its place")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 4, FALSE, 2);
    u16 slot, occupant;
    u32 day;
    const struct ObjectEventTemplate *rom;
    struct ObjectEventTemplate *live;

    ASSUME(group != 0xFF);
    DailySlots_InitNewGame();
    StartDay(2000);
    DefeatGroup(group);
    ASSUME(FindWanderer(group, 2001, &slot, &occupant, &day));
    EnterSlotMap(slot);
    rom = RomTemplate(slot, 0);
    live = LiveTemplate(slot, 0);
    EXPECT_EQ(live->graphicsId, DailyTrainers_GetSlot(occupant)->gfx[0]);
    EXPECT(live->script == DailyTrainers_GetSlot(occupant)->script[0]);
    EXPECT(live->script != rom->script);
    EXPECT_EQ(live->x, rom->x);
    EXPECT_EQ(live->y, rom->y);
    EXPECT_EQ(live->movementType, rom->movementType);
    EXPECT_EQ(live->trainerType, rom->trainerType);
    EXPECT_EQ(live->trainerRange_berryTreeId, rom->trainerRange_berryTreeId);
    EXPECT_EQ(live->flagId, rom->flagId);
    EXPECT(!DailyTrainers_HideTemplate(live));
    EXPECT(DailyTrainers_IsSlotTile(live->x + MAP_OFFSET, live->y + MAP_OFFSET));
    EXPECT(!DailyTrainers_IsSlotTile(-1, -1));
    // Reloading the map the same day gives the same occupant; so does a pinned-then-restored clock.
    EnterSlotMap(slot);
    EXPECT_EQ(LiveTemplate(slot, 0)->graphicsId, DailyTrainers_GetSlot(occupant)->gfx[0]);
    // The next day draws again: the occupant it resolves is what the hook applies.
    StartDay(day + 1);
    EnterSlotMap(slot);
    occupant = DailyTrainers_ResolveOccupant(slot);
    if (occupant != DAILY_OCCUPANT_AWAY && occupant != DAILY_OCCUPANT_HOME)
        EXPECT(LiveTemplate(slot, 0)->script == DailyTrainers_GetSlot(occupant)->script[0]);

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: an empty slot's template never spawns and a placed or cleared slot does")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 6, FALSE, 3);
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], size, i, day;
    bool32 sawAway = FALSE, sawPlaced = FALSE;

    ASSUME(group != 0xFF);
    size = DailyTrainers_GroupSize(group);
    DailySlots_InitNewGame();
    StartDay(2100);
    DefeatGroup(group);
    for (day = 2101; day < 2140; day++)
    {
        StartDay(day);
        DailyTrainers_ResolveGroup(group, out);
        for (i = 0; i < size; i++)
        {
            u16 slot = DailyTrainers_GroupSlot(group, i);

            EnterSlotMap(slot);
            if (out[i] == DAILY_OCCUPANT_AWAY)
            {
                EXPECT(DailyTrainers_HideTemplate(LiveTemplate(slot, 0)));
                sawAway = TRUE;
            }
            else
            {
                EXPECT(!DailyTrainers_HideTemplate(LiveTemplate(slot, 0)));
                // Battled today: the occupant stays and speaks.
                DailyTrainers_SetCleared(slot);
                EXPECT(!DailyTrainers_HideTemplate(LiveTemplate(slot, 0)));
                sawPlaced = TRUE;
            }
        }
    }
    EXPECT(sawAway);
    EXPECT(sawPlaced);

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: every map keeps its objects, places and sight, and holds all its rotating slots")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 slot, day = 0;

    DailySlots_InitNewGame();
    StartDay(2200);
    for (slot = 0; slot < WORLD_TRAINER_SLOT_COUNT; slot++)
        DefeatSlot(slot);
    for (slot = 0; slot < WORLD_TRAINER_SLOT_COUNT; slot++)
    {
        const struct WorldTrainerSlot *row = DailyTrainers_GetSlot(slot);
        const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(row->mapGroup, row->mapNum);
        u8 i;
        u16 count = 0, other;

        if (slot > 0 && DailyTrainers_GetSlot(slot - 1)->mapGroup == row->mapGroup && DailyTrainers_GetSlot(slot - 1)->mapNum == row->mapNum)
            continue;
        StartDay(2201 + (day++ % 7));
        EnterSlotMap(slot);
        for (i = 0; i < header->events->objectEventCount; i++)
        {
            const struct ObjectEventTemplate *rom = &header->events->objectEvents[i];
            const struct ObjectEventTemplate *live = &gSaveBlock1Ptr->objectEventTemplates[i];

            EXPECT_EQ(live->localId, rom->localId);
            EXPECT_EQ(live->x, rom->x);
            EXPECT_EQ(live->y, rom->y);
            EXPECT_EQ(live->elevation, rom->elevation);
            EXPECT_EQ(live->movementType, rom->movementType);
            EXPECT_EQ(live->trainerType, rom->trainerType);
            EXPECT_EQ(live->trainerRange_berryTreeId, rom->trainerRange_berryTreeId);
            // (Item balls on the same map lose their flag to the item hook; a trainer slot keeps its own.)
            if (DailyTrainers_FindSlot(row->mapGroup, row->mapNum, rom->localId) != NO_TRAINER_SLOT)
                EXPECT_EQ(live->flagId, rom->flagId);
        }
        // Every slot of the map is rotating now (all trainers defeated and none beaten today).
        for (other = slot; other < WORLD_TRAINER_SLOT_COUNT && DailyTrainers_GetSlot(other)->mapGroup == row->mapGroup && DailyTrainers_GetSlot(other)->mapNum == row->mapNum; other++)
            count++;
        EXPECT_EQ(DailyTrainers_MapOccupantCount(), count);
    }

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: beating a home trainer freezes its slot for the day and a wanderer starts the next")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 4, FALSE, 2);
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], size, i, slot, trainer;
    const struct WorldTrainerSlot *row;
    u8 map, num;
    u8 local;

    ASSUME(group != 0xFF);
    size = DailyTrainers_GroupSize(group);
    DailySlots_InitNewGame();
    StartDay(3000);
    DefeatSlot(DailyTrainers_GroupSlot(group, 0));
    DefeatSlot(DailyTrainers_GroupSlot(group, 1));
    StartDay(3001);
    // Beat the third home trainer: set up the battle on its object, win, and the game flags it as it does today.
    slot = DailyTrainers_GroupSlot(group, 2);
    row = DailyTrainers_GetSlot(slot);
    EnterSlotMap(slot);
    gSpecialVar_LastTalked = row->localId[0];
    EXPECT_EQ(DailyTrainers_NoteBattle(0, row->localId[0], row->trainerId), row->trainerId);
    EXPECT(!DailyTrainers_NotedRotating(0));
    SetTrainerFlag(row->trainerId);
    DailyTrainers_OnTrainersWon();
    // A home trainer's first win runs its one-time after-battle script as ever.
    TRAINER_BATTLE_PARAM.battleScriptRetAddrA = sAfterBattleScript;
    EXPECT(BattleSetup_GetTrainerPostBattleScript() == sAfterBattleScript);
    EXPECT(DailyTrainers_IsCleared(slot));
    EXPECT(DailyTrainers_IsHomeBeaten(slot));
    // Today the slot stays its home trainer's, and nobody calls it a place to go: it was battled today.
    DailyTrainers_ResolveGroup(group, out);
    EXPECT_EQ(out[2], DAILY_OCCUPANT_HOME);
    EXPECT(!DailyTrainers_WhereIsTrainerToday(row->trainerId, &map, &num, &local));
    EnterSlotMap(slot);
    EXPECT(LiveTemplate(slot, 0)->script == row->script[0]);
    // The day's shuffle did not change by the win: the other slots still resolve the same.
    EXPECT_EQ(out[0] == DAILY_OCCUPANT_HOME, FALSE);
    EXPECT_EQ(out[1] == DAILY_OCCUPANT_HOME, FALSE);
    // Tomorrow its home trainer wanders and the slot is open again.
    StartDay(3002);
    EXPECT(!DailyTrainers_IsCleared(slot));
    EXPECT(!DailyTrainers_IsHomeBeaten(slot));
    DailyTrainers_ResolveGroup(group, out);
    EXPECT_NE(out[2], DAILY_OCCUPANT_HOME);
    trainer = row->trainerId;
    EXPECT(trainer != 0);
    for (i = 0; i < size; i++)
        EXPECT(i <= 2 ? out[i] != DAILY_OCCUPANT_HOME : out[i] == DAILY_OCCUPANT_HOME);

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: a win in a rotating slot clears only the slot and a loss clears nothing")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 4, FALSE, 2);
    u16 slot, occupant, wrongOpponent;
    u32 day;
    const struct WorldTrainerSlot *row, *wanderer;
    u8 i;

    ASSUME(group != 0xFF);
    DailySlots_InitNewGame();
    StartDay(4000);
    DefeatGroup(group);
    ASSUME(FindWanderer(group, 4001, &slot, &occupant, &day));
    row = DailyTrainers_GetSlot(slot);
    wanderer = DailyTrainers_GetSlot(occupant);
    EnterSlotMap(slot);
    // A battle with someone else's trainer id on this object is another script's battle: nothing is noted.
    wrongOpponent = row->trainerId;
    EXPECT_NE(wrongOpponent, wanderer->trainerId);
    EXPECT_EQ(DailyTrainers_NoteBattle(0, row->localId[0], wrongOpponent), wrongOpponent);
    EXPECT(!DailyTrainers_NotedRotating(0));
    DailyTrainers_OnTrainersWon();
    EXPECT(!DailyTrainers_IsCleared(slot));
    // The wanderer's battle: a loss (no win callback) clears nothing...
    DailyTrainers_NoteBattle(0, row->localId[0], wanderer->trainerId);
    EXPECT(DailyTrainers_NotedRotating(0));
    EXPECT(!DailyTrainers_IsCleared(slot));
    EXPECT(!DailyTrainers_IsHomeBeaten(slot));
    // ...a win clears the slot and only the slot; the permanent flags stay as they were.
    DailyTrainers_OnTrainersWon();
    EXPECT(DailyTrainers_IsCleared(slot));
    EXPECT(!DailyTrainers_IsHomeBeaten(slot));
    // The after-battle script is chosen after the win callback, and a wanderer's is skipped: the mark outlives the win.
    EXPECT(DailyTrainers_NotedRotating(0));
    TRAINER_BATTLE_PARAM.battleScriptRetAddrA = sAfterBattleScript;
    EXPECT(BattleSetup_GetTrainerPostBattleScript() == EventScript_TryGetTrainerScript);
    for (i = 0; i < DailyTrainers_GroupSize(group); i++)
        EXPECT(!DailyTrainers_IsHomeBeaten(DailyTrainers_GroupSlot(group, i)));
    // The same occupant still stands there after a reload and speaks (the slot is cleared).
    EnterSlotMap(slot);
    EXPECT(LiveTemplate(slot, 0)->script == wanderer->script[0]);
    EXPECT(DailyTrainers_IsCleared(slot));
    // The next day it is open again, with the day's own draw.
    StartDay(day + 1);
    EXPECT(!DailyTrainers_IsCleared(slot));

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: the battle scripts read the slot, so a wanderer approaches by sight until cleared")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 4, FALSE, 2);
    u16 slot, occupant;
    u32 day;
    const struct WorldTrainerSlot *row;
    struct ObjectEvent *object = &gObjectEvents[0];

    ASSUME(group != 0xFF);
    DailySlots_InitNewGame();
    StartDay(5000);
    DefeatGroup(group);
    ASSUME(FindWanderer(group, 5001, &slot, &occupant, &day));
    row = DailyTrainers_GetSlot(slot);
    EnterSlotMap(slot);
    memset(object, 0, sizeof(*object));
    object->active = TRUE;
    object->localId = row->localId[0];
    object->mapGroup = row->mapGroup;
    object->mapNum = row->mapNum;
    gSelectedObjectEvent = 0;
    TRAINER_BATTLE_PARAM.opponentA = DailyTrainers_GetSlot(occupant)->trainerId;
    // The trainer's permanent flag is set, yet the wanderer is not done: it battles (and approaches by sight).
    EXPECT(HasTrainerBeenFought(DailyTrainers_GetSlot(occupant)->trainerId));
    EXPECT(DailyTrainers_ObjectIsRotating(0));
    EXPECT(!DailyTrainers_ObjectIsDone(0));
    EXPECT_EQ(GetTrainerFlag(), FALSE);
    // Rematch battles are not offered at a rotating slot.
    EXPECT_EQ(ShouldTryRematchBattle(), FALSE);
    // After the win it is done for the day.
    DailyTrainers_SetCleared(slot);
    EXPECT(DailyTrainers_ObjectIsDone(0));
    EXPECT_EQ(GetTrainerFlag(), TRUE);
    // A home object (not rewritten) keeps the permanent flag's meaning.
    object->localId = 255;
    EXPECT(!DailyTrainers_ObjectIsRotating(0));
    EXPECT_EQ(GetTrainerFlag(), HasTrainerBeenFought(TRAINER_BATTLE_PARAM.opponentA));

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: Continue rebuilds the occupants from the saved day")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 4, FALSE, 2);
    u16 slot, occupant, other;
    u32 day;
    const struct WorldTrainerSlot *wanderer;
    struct ObjectEvent *object = &gObjectEvents[0];

    ASSUME(group != 0xFF);
    DailySlots_InitNewGame();
    StartDay(6000);
    DefeatGroup(group);
    ASSUME(FindWanderer(group, 6001, &slot, &occupant, &day));
    wanderer = DailyTrainers_GetSlot(occupant);
    EnterSlotMap(slot);
    // Save, reset: the templates come back from the save, the scripts are the authored ones and the EWRAM table is empty.
    other = DailyTrainers_GroupSlot(group, 0) == slot ? DailyTrainers_GroupSlot(group, 1) : DailyTrainers_GroupSlot(group, 0);
    memset(object, 0, sizeof(*object));
    object->active = TRUE;
    object->localId = DailyTrainers_GetSlot(slot)->localId[0];
    object->mapGroup = DailyTrainers_GetSlot(slot)->mapGroup;
    object->mapNum = DailyTrainers_GetSlot(slot)->mapNum;
    gSelectedObjectEvent = 0;
    EXPECT(DailyTrainers_ObjectIsRotating(0));
    // The real clock has moved on: Continue applies no day change, it rebuilds with the stamped day.
    PinDraws(day + 40, TEST_SEED);
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(DailyTrainers_GetSlot(slot)->mapGroup, DailyTrainers_GetSlot(slot)->mapNum);
    LoadSaveblockObjEventScripts();
    EXPECT(LiveTemplate(slot, 0)->script == RomTemplate(slot, 0)->script);
    DailyTrainers_RewriteTemplates();
    EXPECT(DailyTrainers_ObjectIsRotating(0));
    EXPECT_EQ(LiveTemplate(slot, 0)->graphicsId, wanderer->gfx[0]);
    EXPECT(LiveTemplate(slot, 0)->script == wanderer->script[0]);
    EXPECT_EQ(gPokemonStoragePtr->dailySlots.stampDay, (u16)day);
    // Another map empties the table; coming back through Continue rebuilds it.
    EnterSlotMap(other);
    if (DailyTrainers_GetSlot(other)->mapNum == DailyTrainers_GetSlot(slot)->mapNum && DailyTrainers_GetSlot(other)->mapGroup == DailyTrainers_GetSlot(slot)->mapGroup)
        EXPECT(DailyTrainers_MapOccupantCount() > 0);
    else
        EXPECT(!DailyTrainers_ObjectIsRotating(0));

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: ready and gift trainers are always placed, from the day-start snapshot only")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group, contactOne = DAILY_NO_CONTACT, contactTwo = DAILY_NO_CONTACT;
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], size, i, day, slotOne = 0, slotTwo = 0, placedOne = 0;

    for (group = 0; group < WORLD_TRAINER_GROUP_COUNT; group++)
    {
        contactOne = contactTwo = DAILY_NO_CONTACT;
        if (DailyTrainers_GroupSize(group) < 4)
            continue;
        for (i = 0; i < DailyTrainers_GroupSize(group); i++)
        {
            u16 slot = DailyTrainers_GroupSlot(group, i);
            u8 contact = DailyContacts_IndexOf(DailyTrainers_GetSlot(slot)->trainerId);

            if (contact == DAILY_NO_CONTACT || contact >= REMATCH_SPECIAL_TRAINER_START)
                continue;
            if (contactOne == DAILY_NO_CONTACT)
                contactOne = contact, slotOne = slot;
            else if (contactTwo == DAILY_NO_CONTACT)
                contactTwo = contact, slotTwo = slot;
        }
        if (contactTwo != DAILY_NO_CONTACT)
            break;
    }
    ASSUME(group < WORLD_TRAINER_GROUP_COUNT);
    size = DailyTrainers_GroupSize(group);
    DailySlots_InitNewGame();
    StartDay(7000);
    DefeatGroup(group);
    // Ready or holding a gift as the day starts: placed every day, however the empty draws fall.
    for (day = 7001; day < 7301; day++)
    {
        bool32 foundOne = FALSE, foundTwo = FALSE;

        StartDay(day);
        memset(gPokemonStoragePtr->dailySlots.readyAtDayStart, 0, sizeof(gPokemonStoragePtr->dailySlots.readyAtDayStart));
        memset(gPokemonStoragePtr->dailySlots.giftAtDayStart, 0, sizeof(gPokemonStoragePtr->dailySlots.giftAtDayStart));
        gPokemonStoragePtr->dailySlots.readyAtDayStart[contactOne / 8] |= 1 << (contactOne % 8);
        gPokemonStoragePtr->dailySlots.giftAtDayStart[contactTwo / 8] |= 1 << (contactTwo % 8);
        DailyTrainers_ResolveGroup(group, out);
        for (i = 0; i < size; i++)
        {
            foundOne |= out[i] == slotOne;
            foundTwo |= out[i] == slotTwo;
        }
        EXPECT(foundOne);
        EXPECT(foundTwo);
        placedOne += foundOne;
    }
    EXPECT_EQ(placedOne, 300);
    // Readiness changing mid-day is never read: the arrangement of the day stays.
    StartDay(7400);
    DailyTrainers_ResolveGroup(group, out);
    {
        u16 before[WORLD_TRAINER_GROUP_SLOTS_MAX];

        memcpy(before, out, sizeof(before));
        gSaveBlock1Ptr->trainerRematches[TrainerIdToRematchTableId(gRematchTable, DailyTrainers_GetSlot(slotOne)->trainerId)] = 1;
        FlagSet(FLAG_WADE_HAS_ITEM_HNS);
        DailyTrainers_ResolveGroup(group, out);
        EXPECT(SameOccupants(before, out, size));
    }

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: the day-start snapshots take who is ready and who holds a gift")
{
    struct Snapshot *snapshot = TakeSnapshot();
    s32 wade = TrainerIdToRematchTableId(gRematchTable, TRAINER_WADE_HNS);
    u8 contact = DailyContacts_IndexOf(TRAINER_WADE_HNS);

    ASSUME(wade >= 0);
    EXPECT_EQ(contact, wade);
    DailySlots_InitNewGame();
    gSaveBlock1Ptr->trainerRematches[wade] = 1;
    FlagSet(FLAG_WADE_HAS_ITEM_HNS);
    StartDay(8000);
    EXPECT(DailyContacts_ReadyAtDayStart(contact));
    EXPECT(DailyContacts_GiftAtDayStart(contact));
    EXPECT(!DailyContacts_ReadyAtDayStart(contact + 1));
    // They are fixed for the day: later readiness, a cleared flag or a gift handed over change nothing...
    gSaveBlock1Ptr->trainerRematches[wade] = 0;
    FlagClear(FLAG_WADE_HAS_ITEM_HNS);
    DailySlots_OnMapLoad();
    EXPECT(DailyContacts_ReadyAtDayStart(contact));
    EXPECT(DailyContacts_GiftAtDayStart(contact));
    // ...until the next day starts.
    StartDay(8001);
    EXPECT(!DailyContacts_ReadyAtDayStart(contact));
    EXPECT(!DailyContacts_GiftAtDayStart(contact));
    // A trainer who is not a contact has no index.
    EXPECT_EQ(DailyContacts_IndexOf(TRAINER_FALKNER_1_HNS), DAILY_NO_CONTACT);

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: an occupant holding a gift hands it over before the battle")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 wadeSlot = DailyTrainers_FindSlot(MAP_GROUP(MAP_ROUTE31_HNS), MAP_NUM(MAP_ROUTE31_HNS), 1);
    u8 group;
    u16 slot = NO_TRAINER_SLOT, occupant = NO_TRAINER_SLOT;
    u32 day;
    struct ObjectEvent *object = &gObjectEvents[0];

    ASSUME(wadeSlot != NO_TRAINER_SLOT);
    group = DailyTrainers_GroupOf(wadeSlot);
    DailySlots_InitNewGame();
    StartDay(9000);
    DefeatGroup(group);
    // Day after day until Wade stands in a slot of the group.
    for (day = 9001; day < 9400 && slot == NO_TRAINER_SLOT; day++)
    {
        u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], i;

        StartDay(day);
        DailyTrainers_ResolveGroup(group, out);
        for (i = 0; i < DailyTrainers_GroupSize(group); i++)
        {
            if (out[i] == wadeSlot)
            {
                slot = DailyTrainers_GroupSlot(group, i);
                occupant = out[i];
            }
        }
    }
    ASSUME(slot != NO_TRAINER_SLOT);
    EnterSlotMap(slot);
    memset(object, 0, sizeof(*object));
    object->active = TRUE;
    object->localId = DailyTrainers_GetSlot(slot)->localId[0];
    object->mapGroup = DailyTrainers_GetSlot(slot)->mapGroup;
    object->mapNum = DailyTrainers_GetSlot(slot)->mapNum;
    gSelectedObjectEvent = 0;
    EXPECT_EQ(occupant, wadeSlot);
    EXPECT(!DailyTrainers_ObjectIsDone(0));
    EXPECT_EQ(GetTrainerFlag(), FALSE);
    // While Wade holds a gift the script's flag check reads done, so it falls through to the hand-off.
    FlagSet(FLAG_WADE_HAS_ITEM_HNS);
    EXPECT(DailyTrainers_ObjectIsDone(0));
    EXPECT_EQ(GetTrainerFlag(), TRUE);
    FlagClear(FLAG_WADE_HAS_ITEM_HNS);
    EXPECT_EQ(GetTrainerFlag(), FALSE);

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: WhereIsTrainerToday names today's place and none for a trainer who is away or battled")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 group = FindGroup(0, 4, FALSE, 2);
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], size, i, day, anyAway = 0, anyHere = 0;
    u8 map, num, local;

    ASSUME(group != 0xFF);
    size = DailyTrainers_GroupSize(group);
    DailySlots_InitNewGame();
    StartDay(10000);
    // Not beaten: waiting at home.
    {
        u16 home = DailyTrainers_GroupSlot(group, 0);
        const struct WorldTrainerSlot *row = DailyTrainers_GetSlot(home);

        EXPECT(DailyTrainers_WhereIsTrainerToday(row->trainerId, &map, &num, &local));
        EXPECT_EQ(map, row->mapGroup);
        EXPECT_EQ(num, row->mapNum);
        EXPECT_EQ(local, row->localId[0]);
    }
    DefeatGroup(group);
    for (day = 10001; day < 10060; day++)
    {
        StartDay(day);
        DailyTrainers_ResolveGroup(group, out);
        for (i = 0; i < size; i++)
        {
            u16 wanderer = DailyTrainers_GroupSlot(group, i);
            bool32 found = DailyTrainers_WhereIsTrainerToday(DailyTrainers_GetSlot(wanderer)->trainerId, &map, &num, &local);
            u16 j, place = NO_TRAINER_SLOT;

            for (j = 0; j < size; j++)
            {
                if (out[j] == wanderer)
                    place = DailyTrainers_GroupSlot(group, j);
            }
            EXPECT_EQ(found, place != NO_TRAINER_SLOT);
            if (found)
            {
                EXPECT_EQ(map, DailyTrainers_GetSlot(place)->mapGroup);
                EXPECT_EQ(num, DailyTrainers_GetSlot(place)->mapNum);
                EXPECT_EQ(local, DailyTrainers_GetSlot(place)->localId[0]);
                anyHere++;
                // Battled today: no call names it.
                DailyTrainers_SetCleared(place);
                EXPECT(!DailyTrainers_WhereIsTrainerToday(DailyTrainers_GetSlot(wanderer)->trainerId, &map, &num, &local));
                gPokemonStoragePtr->dailySlots.clearedToday[(WORLD_ITEM_SPOT_COUNT + place) / 8] &= ~(1 << ((WORLD_ITEM_SPOT_COUNT + place) % 8));
            }
            else
                anyAway++;
        }
    }
    EXPECT_GT(anyAway, 0);
    EXPECT_GT(anyHere, 0);

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: a contact that is not daily stands at its home and a home-range trainer is placed in its own slot")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u8 map, num, local;

    DailySlots_InitNewGame();
    StartDay(11000);
    // Joey (Route 30) is hidden by a story flag, so he is fixed at home: no slot, a home row.
    EXPECT(DailyTrainers_FindSlot(MAP_GROUP(MAP_ROUTE30_HNS), MAP_NUM(MAP_ROUTE30_HNS), 2) == NO_TRAINER_SLOT);
    EXPECT(DailyTrainers_WhereIsTrainerToday(TRAINER_JOEY_HNS, &map, &num, &local));
    EXPECT_EQ(map, MAP_GROUP(MAP_ROUTE30_HNS));
    EXPECT_EQ(num, MAP_NUM(MAP_ROUTE30_HNS));
    EXPECT_EQ(local, 2);
    SetTrainerFlag(TRAINER_JOEY_HNS);
    StartDay(11001);
    EXPECT(DailyTrainers_WhereIsTrainerToday(TRAINER_JOEY_HNS, &map, &num, &local));
    // Anthony (Route 33) talks about the tunnel he came through, so he is bound to his place: he has a slot of his own
    // and rotates in a group of one.
    EXPECT(DailyTrainers_FindSlot(MAP_GROUP(MAP_ROUTE33_HNS), MAP_NUM(MAP_ROUTE33_HNS), 1) != NO_TRAINER_SLOT);
    // A trainer that is no slot and no fixed contact is nowhere.
    EXPECT(!DailyTrainers_WhereIsTrainerToday(TRAINER_FALKNER_1_HNS, &map, &num, &local));

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: a home-range trainer is at home or away each day, battled once a day, and a ready one is always home")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 slot = DailyTrainers_FindSlot(MAP_GROUP(MAP_ROUTE33_HNS), MAP_NUM(MAP_ROUTE33_HNS), 1);
    const struct WorldTrainerSlot *row;
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], day, home = 0, away = 0;
    u8 map, num, local, contact;

    ASSUME(slot != NO_TRAINER_SLOT);
    row = DailyTrainers_GetSlot(slot);
    EXPECT(DailyTrainers_IsHomeRange(slot));
    EXPECT_EQ(DailyTrainers_RangeSize(slot), 1);
    DailySlots_InitNewGame();
    StartDay(12000);
    // Undefeated: waiting at home as ever.
    DailyTrainers_ResolveRange(slot, out);
    EXPECT_EQ(out[0], DAILY_OCCUPANT_HOME);
    // Beaten for the first time today: still at home today (frozen), battled, cleared.
    EnterSlotMap(slot);
    gSpecialVar_LastTalked = row->localId[0];
    EXPECT_EQ(DailyTrainers_NoteBattle(0, row->localId[0], row->trainerId), row->trainerId);
    SetTrainerFlag(row->trainerId);
    DailyTrainers_OnTrainersWon();
    EXPECT(DailyTrainers_IsCleared(slot));
    DailyTrainers_ResolveRange(slot, out);
    EXPECT_EQ(out[0], DAILY_OCCUPANT_HOME);
    // From the next day it is present (as its own occupant) or away by the 25 percent roll, and open to one battle a day.
    for (day = 12001; day < 12301; day++)
    {
        StartDay(day);
        DailyTrainers_ResolveRange(slot, out);
        EXPECT(out[0] == slot || out[0] == DAILY_OCCUPANT_AWAY);
        EXPECT(!DailyTrainers_IsCleared(slot));
        if (out[0] == slot)
        {
            home++;
            EnterSlotMap(slot);
            EXPECT(!DailyTrainers_HideTemplate(LiveTemplate(slot, 0)));
            EXPECT(LiveTemplate(slot, 0)->script == row->script[0]);
        }
        else
            away++;
    }
    EXPECT_GT(away * 100, (u32)(home + away) * 17);
    EXPECT_LT(away * 100, (u32)(home + away) * 33);
    // Ready or holding a gift as the day started: placed every day, in its own empty slot when the roll says empty.
    contact = DailyContacts_IndexOf(row->trainerId);
    ASSUME(contact != DAILY_NO_CONTACT);
    for (day = 12301; day < 12501; day++)
    {
        StartDay(day);
        gPokemonStoragePtr->dailySlots.readyAtDayStart[contact / 8] |= 1 << (contact % 8);
        DailyTrainers_ResolveRange(slot, out);
        EXPECT_EQ(out[0], slot);
        EXPECT(DailyTrainers_WhereIsTrainerToday(row->trainerId, &map, &num, &local));
        EXPECT_EQ(local, row->localId[0]);
    }
    // A win in its slot on a day it stands there clears it for that day only.
    StartDay(12501);
    DailyTrainers_ResolveRange(slot, out);
    if (out[0] == slot)
    {
        EnterSlotMap(slot);
        EXPECT_EQ(DailyTrainers_NoteBattle(0, row->localId[0], row->trainerId), DailyTrainers_LatestTeam(row->trainerId));
        EXPECT(DailyTrainers_NotedRotating(0));
        DailyTrainers_OnTrainersWon();
        EXPECT(DailyTrainers_IsCleared(slot));
        EXPECT(!DailyTrainers_WhereIsTrainerToday(row->trainerId, &map, &num, &local));
    }
    StartDay(12502);
    EXPECT(!DailyTrainers_IsCleared(slot));

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: a slot with no absence always has someone, battleable once a day after the first win")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 slot = DailyTrainers_FindSlot(MAP_GROUP(MAP_ROUTE33_HNS), MAP_NUM(MAP_ROUTE33_HNS), 1);
    u8 group = FindGroup(0, 4, FALSE, 2);
    const struct WorldTrainerSlot *row;
    u16 out[WORLD_TRAINER_GROUP_SLOTS_MAX], day, i;
    u8 map, num, local;

    ASSUME(slot != NO_TRAINER_SLOT);
    ASSUME(group != 0xFF);
    row = DailyTrainers_GetSlot(slot);
    // The generated tables give every slot the default 25 percent today.
    EXPECT_EQ(DailyTrainers_AbsencePercent(slot), 25);
    gDailySlotsDebugFlags = DAILY_DEBUG_DAY | DAILY_DEBUG_SEED | DAILY_DEBUG_ABSENCE;
    gDailySlotsDebugAbsence = 0;
    DailySlots_InitNewGame();
    StartDay(13000);
    gDailySlotsDebugFlags |= DAILY_DEBUG_ABSENCE;
    EXPECT_EQ(DailyTrainers_AbsencePercent(slot), 0);
    SetTrainerFlag(row->trainerId);
    for (day = 13001; day < 13201; day++)
    {
        StartDay(day);
        gDailySlotsDebugFlags |= DAILY_DEBUG_ABSENCE;
        // A home range: the trainer is there every day, never away, and the slot is open for one battle.
        DailyTrainers_ResolveRange(slot, out);
        EXPECT_EQ(out[0], slot);
        EXPECT(!DailyTrainers_IsCleared(slot));
        EnterSlotMap(slot);
        EXPECT(!DailyTrainers_HideTemplate(LiveTemplate(slot, 0)));
        EXPECT_EQ(DailyTrainers_NoteBattle(0, row->localId[0], row->trainerId), DailyTrainers_LatestTeam(row->trainerId));
        DailyTrainers_OnTrainersWon();
        EXPECT(DailyTrainers_IsCleared(slot));
        EXPECT(!DailyTrainers_WhereIsTrainerToday(row->trainerId, &map, &num, &local));
        // Clearing is for the day: a second note of the same slot is harmless and the slot stays cleared.
        EXPECT(DailyTrainers_IsCleared(slot));
    }
    // A class range with no absence places every wanderer every day.
    DailySlots_InitNewGame();
    StartDay(13300);
    DefeatGroup(group);
    for (day = 13301; day < 13361; day++)
    {
        u16 placed = 0;

        StartDay(day);
        gDailySlotsDebugFlags |= DAILY_DEBUG_ABSENCE;
        DailyTrainers_ResolveGroup(group, out);
        for (i = 0; i < DailyTrainers_GroupSize(group); i++)
        {
            EXPECT_NE(out[i], DAILY_OCCUPANT_AWAY);
            placed += out[i] != DAILY_OCCUPANT_HOME;
        }
        EXPECT_EQ(placed, DailyTrainers_GroupSize(group));
    }
    // And at 100 percent nobody is ever there.
    gDailySlotsDebugAbsence = 100;
    StartDay(13400);
    gDailySlotsDebugFlags |= DAILY_DEBUG_ABSENCE;
    DailyTrainers_ResolveGroup(group, out);
    for (i = 0; i < DailyTrainers_GroupSize(group); i++)
        EXPECT_EQ(out[i], DAILY_OCCUPANT_AWAY);

    RestoreSnapshot(snapshot);
}

TEST("Daily trainers: a wanderer fights with its latest rematch team")
{
    struct Snapshot *snapshot = TakeSnapshot();
    s32 wade = TrainerIdToRematchTableId(gRematchTable, TRAINER_WADE_HNS);
    u16 second = gRematchTable[wade].trainerIds[1];

    ASSUME(wade >= 0);
    ASSUME(second != 0);
    // Nothing beaten beyond the first team: the first team.
    ClearTrainerFlag(second);
    EXPECT_EQ(DailyTrainers_LatestTeam(TRAINER_WADE_HNS), TRAINER_WADE_HNS);
    // Once the second team has been beaten it is the latest.
    SetTrainerFlag(second);
    EXPECT_EQ(DailyTrainers_LatestTeam(TRAINER_WADE_HNS), second);
    // A trainer with no rematch team fights as itself.
    EXPECT_EQ(DailyTrainers_LatestTeam(TRAINER_FALKNER_1_HNS), TRAINER_FALKNER_1_HNS);

    RestoreSnapshot(snapshot);
}

#endif // IS_WAYFARER
