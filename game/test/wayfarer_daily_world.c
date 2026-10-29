#include "global.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "malloc.h"
#include "overworld.h"
#include "pokemon_storage_system.h"
#include "sprite.h"
#include "wayfarer_daily_slots.h"
#include "wayfarer_walker_beats.h"
#include "test/test.h"
#include "constants/event_objects.h"

#if IS_WAYFARER

// Daily world slots meeting the world around them (.product/specs/daily-world-slots.md: The day,
// The map-load hook, Known limitations): a day change on a camera-transition load, and walkers
// that keep off every slot's tile.

#define TEST_SEED   0x5EEDF00D
#define WALKER      1           // the gObjectEvents row the walker tests step with
#define SPAWN_TRIES 60

struct Snapshot
{
    struct MapHeader header;
    struct WarpData location;
    struct WayfarerDailySlots daily;
    struct ObjectEventTemplate templates[OBJECT_EVENT_TEMPLATES_COUNT];
    u8 flags[sizeof(gSaveBlock1Ptr->flags)];
    struct ObjectEvent objects[OBJECT_EVENTS_COUNT];
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
    memcpy(snapshot->objects, gObjectEvents, sizeof(snapshot->objects));
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
    memcpy(gObjectEvents, snapshot->objects, sizeof(snapshot->objects));
    gDailySlotsDebugDay = snapshot->debugDay;
    gDailySlotsDebugSeed = snapshot->debugSeed;
    gDailySlotsDebugFlags = snapshot->debugFlags;
    Free(snapshot);
}

static void PinDraws(u32 day)
{
    gDailySlotsDebugFlags = DAILY_DEBUG_DAY | DAILY_DEBUG_SEED;
    gDailySlotsDebugDay = day;
    gDailySlotsDebugSeed = TEST_SEED;
}

// A map load: the templates, the day and the slots' hooks, as every load runs them.
static void EnterMap(u8 mapGroup, u8 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(mapGroup, mapNum);
    LoadObjEventTemplatesFromHeader();
}

// The same, with the map grid the walkers read.
static void EnterMapWithGrid(u8 mapGroup, u8 mapNum)
{
    EnterMap(mapGroup, mapNum);
    ASSUME(InitMap() == MAP_LAYOUT_LOAD_OK);
}

// An active object of a slot's map, as the field keeps it after a crossing; no sprite behind it.
static struct ObjectEvent *PlaceObject(u8 index, u8 mapGroup, u8 mapNum, u8 localId)
{
    struct ObjectEvent *object = &gObjectEvents[index];

    memset(object, 0, sizeof(*object));
    object->active = TRUE;
    object->localId = localId;
    object->mapGroup = mapGroup;
    object->mapNum = mapNum;
    object->graphicsId = OBJ_EVENT_GFX_YOUNGSTER;
    return object;
}

// The first trainer slot's first object, whose map is not the one of the first ball spot.
static u16 FirstSlotOtherThan(u8 mapGroup, u8 mapNum)
{
    u16 slot;

    for (slot = 0; slot < WORLD_TRAINER_SLOT_COUNT; slot++)
    {
        if (DailyTrainers_GetSlot(slot)->mapGroup != mapGroup || DailyTrainers_GetSlot(slot)->mapNum != mapNum)
            return slot;
    }
    return NO_TRAINER_SLOT;
}

static u16 FirstBallSpot(void)
{
    u16 i;

    for (i = 0; i < WORLD_ITEM_SPOT_COUNT; i++)
    {
        if (!WORLD_ITEM_ATTR_HIDDEN(DailyItems_GetSpot(i)->attrs))
            return i;
    }
    return NO_ITEM_SPOT;
}

TEST("Daily slots: a camera-transition load that starts a new day removes the slot objects left on screen")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 spot = FirstBallSpot();
    const struct WorldItemSpot *ball = DailyItems_GetSpot(spot);
    u16 slot = FirstSlotOtherThan(ball->mapGroup, ball->mapNum);
    const struct WorldTrainerSlot *trainer = DailyTrainers_GetSlot(slot);
    struct ObjectEvent *ballObject, *trainerObject, *otherObject;

    ASSUME(spot != NO_ITEM_SPOT && slot != NO_TRAINER_SLOT);
    ResetSpriteData();
    DailySlots_InitNewGame();

    // Day one, standing in the ball's map: a ball, a trainer slot's object of another map, and an
    // object that is no slot's, all on screen.
    PinDraws(3000);
    EnterMap(ball->mapGroup, ball->mapNum);
    ballObject = PlaceObject(2, ball->mapGroup, ball->mapNum, ball->id);
    trainerObject = PlaceObject(3, trainer->mapGroup, trainer->mapNum, trainer->localId[0]);
    otherObject = PlaceObject(4, ball->mapGroup, ball->mapNum, 0xF0);

    // A crossing on the same day changes no draw: nothing is touched.
    EnterMap(trainer->mapGroup, trainer->mapNum);
    DailySlots_OnCameraTransition();
    EXPECT(ballObject->active);
    EXPECT(trainerObject->active);
    EXPECT(otherObject->active);

    // A crossing that starts the next day removes every slot's object, the one of the map left
    // behind too, and keeps the rest.
    PinDraws(3001);
    EnterMap(ball->mapGroup, ball->mapNum);
    DailySlots_OnCameraTransition();
    EXPECT(!ballObject->active);
    EXPECT(!trainerObject->active);
    EXPECT(otherObject->active);

    // The refresh is once per load: objects spawned afterwards on the same day stay.
    ballObject = PlaceObject(2, ball->mapGroup, ball->mapNum, ball->id);
    DailySlots_OnCameraTransition();
    EXPECT(ballObject->active);

    // A warp's load starts days too, but resets every object itself: only a camera transition asks.
    PinDraws(3002);
    EnterMap(trainer->mapGroup, trainer->mapNum);
    EXPECT(ballObject->active);

    ResetSpriteData();
    RestoreSnapshot(snapshot);
}

// Whether a walker standing on (x, y) (MAP_OFFSET coordinates) may take a step.
static bool32 CanStep(s16 x, s16 y, u8 dir)
{
    struct ObjectEvent *walker = &gObjectEvents[WALKER];

    memset(walker, 0, sizeof(*walker));
    walker->active = TRUE;
    walker->localId = 0xF1;
    walker->currentCoords.x = walker->previousCoords.x = walker->initialCoords.x = x;
    walker->currentCoords.y = walker->previousCoords.y = walker->initialCoords.y = y;
    walker->currentElevation = walker->previousElevation = MapGridGetElevationAt(x, y);
    walker->currentMetatileBehavior = MapGridGetMetatileBehaviorAt(x, y);
    return WayfarerWalkers_BeatCanStep(walker, dir);
}

// A slot's tile (MAP_OFFSET coordinates) that a walker can leave for a neighbour but may not enter
// from it: the neighbour's direction and tile. The reverse step shows the map really lets the walker
// move there, so the refusal is the slot's.
static bool32 FindLeavableTile(s16 x, s16 y, u8 *dir, s16 *nx, s16 *ny)
{
    static const s8 dx[5] = {0, 0, 0, -1, 1};
    static const s8 dy[5] = {0, 1, -1, 0, 0};
    u8 d;

    for (d = DIR_SOUTH; d <= DIR_EAST; d++)
    {
        if (CanStep(x, y, d) && !DailySlots_IsSlotTile(x + dx[d], y + dy[d]))
        {
            *dir = GetOppositeDirection(d);
            *nx = x + dx[d];
            *ny = y + dy[d];
            return TRUE;
        }
    }
    return FALSE;
}

TEST("Daily slots: a walker never steps onto a trainer slot's tile, whether or not its object is there")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 slot, tries = 0;
    bool32 checked = FALSE;

    DailySlots_InitNewGame();
    PinDraws(3100);
    for (slot = 0; slot < WORLD_TRAINER_SLOT_COUNT && tries < SPAWN_TRIES && !checked; slot++)
    {
        const struct WorldTrainerSlot *row = DailyTrainers_GetSlot(slot);
        const struct ObjectEventTemplate *template;
        u8 dir;
        s16 x, y, nx, ny;

        tries++;
        EnterMapWithGrid(row->mapGroup, row->mapNum);
        template = &gSaveBlock1Ptr->objectEventTemplates[row->localId[0] - 1];
        x = template->x + MAP_OFFSET;
        y = template->y + MAP_OFFSET;
        // No object stands there: nothing but the slot's own claim stops a step onto it.
        gObjectEvents[WALKER].active = FALSE;
        EXPECT_EQ(GetObjectEventIdByXY(x, y), OBJECT_EVENTS_COUNT);
        if (!FindLeavableTile(x, y, &dir, &nx, &ny))
            continue;
        EXPECT(DailySlots_IsSlotTile(x, y));
        EXPECT(!CanStep(nx, ny, dir));
        // A slot that is empty today, or cleared, keeps its tile too.
        DailyTrainers_SetCleared(slot);
        EXPECT(!CanStep(nx, ny, dir));
        checked = TRUE;
    }
    ASSUME(checked);

    RestoreSnapshot(snapshot);
}

TEST("Daily slots: a walker never steps onto an item ball's tile, picked up or not")
{
    struct Snapshot *snapshot = TakeSnapshot();
    u16 spot, tries = 0;
    bool32 checked = FALSE;

    DailySlots_InitNewGame();
    PinDraws(3200);
    for (spot = 0; spot < WORLD_ITEM_SPOT_COUNT && tries < SPAWN_TRIES && !checked; spot++)
    {
        const struct WorldItemSpot *row = DailyItems_GetSpot(spot);
        const struct ObjectEventTemplate *template;
        u8 dir;
        s16 x, y, nx, ny;

        if (WORLD_ITEM_ATTR_HIDDEN(row->attrs))
            continue;
        tries++;
        EnterMapWithGrid(row->mapGroup, row->mapNum);
        template = &gSaveBlock1Ptr->objectEventTemplates[row->id - 1];
        x = template->x + MAP_OFFSET;
        y = template->y + MAP_OFFSET;
        // No object stands there: nothing but the slot's own claim stops a step onto it.
        gObjectEvents[WALKER].active = FALSE;
        EXPECT_EQ(GetObjectEventIdByXY(x, y), OBJECT_EVENTS_COUNT);
        if (!FindLeavableTile(x, y, &dir, &nx, &ny))
            continue;
        EXPECT(DailySlots_IsSlotTile(x, y));
        EXPECT(!CanStep(nx, ny, dir));
        // Picked up, its object is gone for the day; the tile is still the spot's.
        DailyItems_SetCleared(spot);
        EXPECT(!CanStep(nx, ny, dir));
        checked = TRUE;
    }
    ASSUME(checked);

    RestoreSnapshot(snapshot);
}

#endif // IS_WAYFARER
