#include "global.h"
#include "event_data.h"
#include "field_control_avatar.h"
#include "fieldmap.h"
#include "item_use.h"
#include "malloc.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "save.h"
#include "task.h"
#include "test/test.h"
#include "wayfarer_persistence.h"
#include "wild_encounter.h"
#include "constants/event_bg.h"
#include "constants/event_objects.h"
#include "constants/maps.h"

#if IS_WAYFARER

extern const u8 EventScript_RockSmash[];

#define SEVII_ITEM_BALL_COUNT   39
#define SEVII_HIDDEN_ITEM_COUNT 58
#define SEVII_FIRST_PICKUP_SLOT WAYFARER_SEVII_ID_INDEX(FLAG_WAYFARER_SEVII_ITEM_MT_EMBER_EXTERIOR_ULTRA_BALL)

// Every Sevii map with a restored item ball or hidden item.
static const u16 sSeviiPickupMaps[] =
{
    MAP_MT_EMBER_EXTERIOR,
    MAP_THREE_ISLAND_BERRY_FOREST,
    MAP_FOUR_ISLAND_ICEFALL_CAVE_1F,
    MAP_FOUR_ISLAND_ICEFALL_CAVE_B1F,
    MAP_FIVE_ISLAND_ROCKET_WAREHOUSE,
    MAP_FIVE_ISLAND_LOST_CAVE_ROOM10,
    MAP_FIVE_ISLAND_LOST_CAVE_ROOM11,
    MAP_FIVE_ISLAND_LOST_CAVE_ROOM12,
    MAP_FIVE_ISLAND_LOST_CAVE_ROOM13,
    MAP_FIVE_ISLAND_LOST_CAVE_ROOM14,
    MAP_THREE_ISLAND_DUNSPARCE_TUNNEL,
    MAP_TWO_ISLAND,
    MAP_THREE_ISLAND,
    MAP_FOUR_ISLAND,
    MAP_SIX_ISLAND,
    MAP_ONE_ISLAND_KINDLE_ROAD,
    MAP_ONE_ISLAND_TREASURE_BEACH,
    MAP_TWO_ISLAND_CAPE_BRINK,
    MAP_THREE_ISLAND_BOND_BRIDGE,
    MAP_FIVE_ISLAND_RESORT_GORGEOUS,
    MAP_FIVE_ISLAND_MEADOW,
    MAP_FIVE_ISLAND_MEMORIAL_PILLAR,
    MAP_SIX_ISLAND_OUTCAST_ISLAND,
    MAP_SIX_ISLAND_GREEN_PATH,
    MAP_SIX_ISLAND_WATER_PATH,
    MAP_SIX_ISLAND_RUIN_VALLEY,
    MAP_SEVEN_ISLAND_TRAINER_TOWER,
    MAP_SEVEN_ISLAND_SEVAULT_CANYON_ENTRANCE,
    MAP_SEVEN_ISLAND_SEVAULT_CANYON,
    MAP_SEVEN_ISLAND_TANOBY_RUINS,
    MAP_SEVEN_ISLAND_SEVAULT_CANYON_HOUSE,
};

// Every Sevii map with a Rock Smash table.
static const u16 sSeviiRockSmashMaps[] =
{
    MAP_MT_EMBER_EXTERIOR,
    MAP_MT_EMBER_RUBY_PATH_1F,
    MAP_MT_EMBER_RUBY_PATH_B1F,
    MAP_MT_EMBER_RUBY_PATH_B1F_STAIRS,
    MAP_MT_EMBER_RUBY_PATH_B2F,
    MAP_MT_EMBER_RUBY_PATH_B2F_STAIRS,
    MAP_MT_EMBER_RUBY_PATH_B3F,
    MAP_MT_EMBER_SUMMIT_PATH_2F,
    MAP_ONE_ISLAND_KINDLE_ROAD,
    MAP_SEVEN_ISLAND_SEVAULT_CANYON,
};

static const struct MapEvents *GetSeviiMapEvents(u16 mapId)
{
    return Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(mapId), MAP_NUM(mapId))->events;
}

static bool32 IsSeviiPickupFlag(u16 flagId)
{
    return IS_WAYFARER_SEVII_FLAG_ID(flagId)
        && WAYFARER_SEVII_ID_INDEX(flagId) >= SEVII_FIRST_PICKUP_SLOT
        && WAYFARER_SEVII_ID_INDEX(flagId) < WAYFARER_SEVII_FLAG_COUNT;
}

// Marks a pickup flag as seen; fails for a non-pickup flag or a duplicate.
static bool32 MarkSeviiPickupFlag(u8 *seen, u16 flagId)
{
    u32 slot = WAYFARER_SEVII_ID_INDEX(flagId);

    if (!IsSeviiPickupFlag(flagId) || (seen[slot / 8] & (1 << (slot % 8))))
        return FALSE;
    seen[slot / 8] |= 1 << (slot % 8);
    return TRUE;
}

// Collects every restored pickup flag, failing on a duplicate.
static u32 CollectSeviiPickupFlags(u16 *flags, u32 *itemBalls, u32 *hiddenItems)
{
    u8 seen[WAYFARER_SEVII_FLAG_BYTES] = {0};
    u32 count = 0;

    *itemBalls = 0;
    *hiddenItems = 0;
    for (u32 m = 0; m < ARRAY_COUNT(sSeviiPickupMaps); m++)
    {
        const struct MapEvents *events = GetSeviiMapEvents(sSeviiPickupMaps[m]);
        for (u32 i = 0; i < events->objectEventCount; i++)
        {
            u16 flagId = events->objectEvents[i].flagId;
            if (events->objectEvents[i].graphicsId != OBJ_EVENT_GFX_ITEM_BALL)
                continue;
            if (!MarkSeviiPickupFlag(seen, flagId))
                return 0;
            flags[count++] = flagId;
            (*itemBalls)++;
        }
        for (u32 i = 0; i < events->bgEventCount; i++)
        {
            u16 flagId;
            if (events->bgEvents[i].kind != BG_EVENT_HIDDEN_ITEM)
                continue;
            flagId = GetHiddenItemFlagId(&events->bgEvents[i]);
            if (!MarkSeviiPickupFlag(seen, flagId))
                return 0;
            flags[count++] = flagId;
            (*hiddenItems)++;
        }
    }
    return count;
}

TEST("Wayfarer Sevii hidden items decode the Sevii marker to a Sevii bank flag")
{
    struct BgEvent bgEvent = {0};

    bgEvent.kind = BG_EVENT_HIDDEN_ITEM;
    bgEvent.bgUnion.hiddenItem.hiddenItemId = WAYFARER_SEVII_HIDDEN_ITEM_MARKER | 157;
    EXPECT_EQ(GetHiddenItemFlagId(&bgEvent), WAYFARER_SEVII_FLAG_ID(157));
    bgEvent.bgUnion.hiddenItem.hiddenItemId = WAYFARER_SEVII_HIDDEN_ITEM_MARKER | WAYFARER_SEVII_ID_INDEX(FLAG_WAYFARER_SEVII_HIDDEN_MT_EMBER_EXTERIOR_0);
    EXPECT_EQ(GetHiddenItemFlagId(&bgEvent), FLAG_WAYFARER_SEVII_HIDDEN_MT_EMBER_EXTERIOR_0);
    // The highest Emerald hidden-item flag keeps its Hoenn decoding.
    bgEvent.bgUnion.hiddenItem.hiddenItemId = WAYFARER_HOENN_HIDDEN_ITEM_MARKER | 0x95F;
    EXPECT_EQ(GetHiddenItemFlagId(&bgEvent), HOENN_FLAG_ID(0x95F));
    bgEvent.bgUnion.hiddenItem.hiddenItemId = 5;
    EXPECT_EQ(GetHiddenItemFlagId(&bgEvent), FLAG_HIDDEN_ITEMS_START + 5);
}

TEST("Wayfarer Sevii pickups keep unique flags across a save and reload")
{
    u16 *flags = Alloc(sizeof(u16) * (SEVII_ITEM_BALL_COUNT + SEVII_HIDDEN_ITEM_COUNT));
    u8 (*chunks)[SAVE_BLOCK_3_CHUNK_SIZE] = Alloc(NUM_SECTORS_PER_SLOT * SAVE_BLOCK_3_CHUNK_SIZE);
    struct SaveSector *sector = Alloc(sizeof(*sector));
    u32 itemBalls, hiddenItems, count, setCount = 0;

    ASSUME(flags != NULL && chunks != NULL && sector != NULL);
    count = CollectSeviiPickupFlags(flags, &itemBalls, &hiddenItems);
    EXPECT_EQ(itemBalls, SEVII_ITEM_BALL_COUNT);
    EXPECT_EQ(hiddenItems, SEVII_HIDDEN_ITEM_COUNT);
    EXPECT_EQ(count, SEVII_ITEM_BALL_COUNT + SEVII_HIDDEN_ITEM_COUNT);

    WayfarerSeviiInitPersistentState();
    for (u32 i = 0; i < count; i++)
    {
        EXPECT(!FlagGet(flags[i]));
        FlagSet(flags[i]);
    }

    for (u32 sectorId = 0; sectorId < NUM_SECTORS_PER_SLOT; sectorId++)
    {
        Test_CopySaveBlock3ToSector(sectorId, sector);
        memcpy(chunks[sectorId], sector->saveBlock3Chunk, SAVE_BLOCK_3_CHUNK_SIZE);
    }
    WayfarerSeviiInitPersistentState();
    EXPECT(!FlagGet(flags[0]));
    for (u32 sectorId = 0; sectorId < NUM_SECTORS_PER_SLOT; sectorId++)
    {
        memcpy(sector->saveBlock3Chunk, chunks[sectorId], SAVE_BLOCK_3_CHUNK_SIZE);
        Test_CopySaveBlock3FromSector(sectorId, sector);
    }

    for (u32 i = 0; i < count; i++)
        EXPECT(FlagGet(flags[i]));
    // Nothing else in the pickup range was set, so no two pickups alias.
    for (u32 slot = SEVII_FIRST_PICKUP_SLOT; slot < WAYFARER_SEVII_FLAG_COUNT; slot++)
        setCount += FlagGet(WAYFARER_SEVII_FLAG_ID(slot));
    EXPECT_EQ(setCount, count);

    WayfarerSeviiInitPersistentState();
    Free(sector);
    Free(chunks);
    Free(flags);
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

TEST("Wayfarer Itemfinder finds every Sevii hidden item until it is picked up")
{
    struct MapHeader mapHeader = gMapHeader;
    struct PlayerAvatar playerAvatar = gPlayerAvatar;
    struct ObjectEvent playerObject = gObjectEvents[0];
    struct WarpData location = gSaveBlock1Ptr->location;
    u8 taskId = CreateTask(TaskDummy, 0);
    u32 checked = 0;
    bool32 passed = TRUE;

    WayfarerSeviiInitPersistentState();
    gPlayerAvatar.objectEventId = 0;
    for (u32 m = 0; m < ARRAY_COUNT(sSeviiPickupMaps); m++)
    {
        u16 mapId = sSeviiPickupMaps[m];
        const struct MapEvents *events;

        gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(mapId);
        gSaveBlock1Ptr->location.mapNum = MAP_NUM(mapId);
        gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(mapId), MAP_NUM(mapId));
        events = gMapHeader.events;
        for (u32 i = 0; i < events->bgEventCount; i++)
        {
            const struct BgEvent *bgEvent = &events->bgEvents[i];
            if (bgEvent->kind != BG_EVENT_HIDDEN_ITEM)
                continue;
            gObjectEvents[0].currentCoords.x = bgEvent->x + MAP_OFFSET;
            gObjectEvents[0].currentCoords.y = bgEvent->y + MAP_OFFSET;
            passed &= ItemfinderFindsItemUnderfoot(events, taskId);
            FlagSet(GetHiddenItemFlagId(bgEvent));
            passed &= !ItemfinderFindsItemUnderfoot(events, taskId);
            checked++;
        }
    }

    DestroyTask(taskId);
    WayfarerSeviiInitPersistentState();
    gMapHeader = mapHeader;
    gPlayerAvatar = playerAvatar;
    gObjectEvents[0] = playerObject;
    gSaveBlock1Ptr->location = location;
    EXPECT(passed);
    EXPECT_EQ(checked, SEVII_HIDDEN_ITEM_COUNT);
}

TEST("Wayfarer Sevii Rock Smash rocks can start the map's Rock Smash encounter")
{
    u16 mapId = MAP_UNDEFINED;
    for (u32 i = 0; i < ARRAY_COUNT(sSeviiRockSmashMaps); i++)
        PARAMETRIZE(mapId = sSeviiRockSmashMaps[i]);

    struct Pokemon *playerParty = Alloc(sizeof(gPlayerParty));
    struct Pokemon *enemyParty = Alloc(sizeof(gEnemyParty));
    struct MapHeader mapHeader = gMapHeader;
    struct WarpData location = gSaveBlock1Ptr->location;
    u8 playerPartyCount = gPlayerPartyCount;
    u16 repelSteps = VarGet(VAR_REPEL_STEP_COUNT);
    u16 specialResult = gSpecialVar_Result;
    const struct MapEvents *events = GetSeviiMapEvents(mapId);
    bool32 hasRock = FALSE, started = FALSE;
    u16 species;

    ASSUME(playerParty != NULL && enemyParty != NULL);
    memcpy(playerParty, gPlayerParty, sizeof(gPlayerParty));
    memcpy(enemyParty, gEnemyParty, sizeof(gEnemyParty));

    for (u32 i = 0; i < events->objectEventCount; i++)
        hasRock |= events->objectEvents[i].script == EventScript_RockSmash;

    ZeroPlayerPartyMons();
    CreateMon(&gPlayerParty[0], SPECIES_MACHOP, 5, 0, OTID_STRUCT_PLAYER_ID);
    CalculatePlayerPartyCount();
    ZeroEnemyPartyMons();
    FlagClear(FLAG_ENABLE_TRAINER_ONLY_ENCOUNTERS);
    DisableWildEncounters(FALSE);
    VarSet(VAR_REPEL_STEP_COUNT, 0);
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(mapId);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(mapId);
    gMapHeader = *Overworld_GetMapHeaderByGroupAndId(MAP_GROUP(mapId), MAP_NUM(mapId));
    SetWildStartInterceptionForTesting(TRUE);
    for (u32 seed = 0; seed < 256 && !started; seed++)
    {
        SeedRng(seed);
        RockSmashWildEncounter();
        started = gSpecialVar_Result;
    }
    species = GetMonData(&gEnemyParty[0], MON_DATA_SPECIES);
    SetWildStartInterceptionForTesting(FALSE);

    memcpy(gPlayerParty, playerParty, sizeof(gPlayerParty));
    memcpy(gEnemyParty, enemyParty, sizeof(gEnemyParty));
    gPlayerPartyCount = playerPartyCount;
    gMapHeader = mapHeader;
    gSaveBlock1Ptr->location = location;
    VarSet(VAR_REPEL_STEP_COUNT, repelSteps);
    gSpecialVar_Result = specialResult;
    Free(enemyParty);
    Free(playerParty);

    EXPECT(hasRock);
    EXPECT(started);
    EXPECT_NE(species, SPECIES_NONE);
}

#endif // IS_WAYFARER
