#include "global.h"
#include "event_data.h"
#include "load_save.h"
#include "main.h"
#include "save.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "gba/flash_internal.h"
#include "constants/opponents.h"
#include "constants/maps.h"

#if IS_WAYFARER

EWRAM_DATA static struct SaveSector sWayfarerTestSector = {0};
EWRAM_DATA static u8 sWayfarerTestChunks[NUM_SECTORS_PER_SLOT][SAVE_BLOCK_3_CHUNK_SIZE] = {0};
EWRAM_DATA static u8 sWayfarerExpectedSaveBlock3[sizeof(struct SaveBlock3)] = {0};

TEST("Wayfarer Hoenn variables use an isolated full-size bank")
{
    const u16 first = HOENN_VAR_ID(0x4000);
    const u16 last = HOENN_VAR_ID(0x40FF);

    EXPECT_NE(first, 0);
    EXPECT_NE(first, VARS_START);
    EXPECT(VarSet(first, 1234));
    EXPECT(VarSet(last, 5678));
    EXPECT_EQ(VarGet(first), 1234);
    EXPECT_EQ(VarGet(last), 5678);
    EXPECT_EQ(VarGet(VARS_START), 0);

    ClearTempFieldEventData();
    EXPECT_EQ(VarGet(first), 0);
    EXPECT_EQ(VarGet(last), 5678);
}

TEST("Wayfarer common-script source follows the map catalog")
{
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F);
    EXPECT(WayfarerIsCurrentMapHoennSource());

    // HnS reuses Hoenn geography for several maps; source must not be inferred
    // from the region-map section.
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_BATTLE_FRONTIER_OUTSIDE_WEST_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_BATTLE_FRONTIER_OUTSIDE_WEST_HNS);
    EXPECT(!WayfarerIsCurrentMapHoennSource());
}

TEST("Wayfarer field move flags follow map source and reset both banks")
{
    const u16 hoennStrength = HOENN_FLAG_ID(WAYFARER_HOENN_USE_STRENGTH_FLAG);
    const u16 hoennFlash = HOENN_FLAG_ID(WAYFARER_HOENN_USE_FLASH_FLAG);

    WayfarerFieldMoveFlagClear(FLAG_SYS_USE_STRENGTH);
    WayfarerFieldMoveFlagClear(FLAG_SYS_USE_FLASH);
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F);

    // Hoenn's common Strength script writes this source-mapped flag.
    FlagSet(hoennStrength);
    WayfarerFieldMoveFlagSet(FLAG_SYS_USE_FLASH);
    EXPECT(WayfarerFieldMoveFlagGet(FLAG_SYS_USE_STRENGTH));
    EXPECT(WayfarerFieldMoveFlagGet(FLAG_SYS_USE_FLASH));
    EXPECT(!FlagGet(FLAG_SYS_USE_STRENGTH));
    EXPECT(!FlagGet(FLAG_SYS_USE_FLASH));
    EXPECT(FlagGet(hoennFlash));

    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_NEW_BARK_TOWN_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_NEW_BARK_TOWN_HNS);
    EXPECT(!WayfarerFieldMoveFlagGet(FLAG_SYS_USE_STRENGTH));
    WayfarerFieldMoveFlagSet(FLAG_SYS_USE_STRENGTH);
    EXPECT(FlagGet(FLAG_SYS_USE_STRENGTH));

    WayfarerFieldMoveFlagClear(FLAG_SYS_USE_STRENGTH);
    WayfarerFieldMoveFlagClear(FLAG_SYS_USE_FLASH);
    EXPECT(!FlagGet(FLAG_SYS_USE_STRENGTH));
    EXPECT(!FlagGet(hoennStrength));
    EXPECT(!FlagGet(hoennFlash));
}

TEST("Wayfarer Hoenn persistent flags route without touching HnS flags")
{
    const u16 hoennFlag = HOENN_FLAG_ID(0x52);

    EXPECT_NE(hoennFlag, 0);
    EXPECT_NE(hoennFlag, 0x52);
    FlagSet(hoennFlag);
    EXPECT(FlagGet(hoennFlag));
    EXPECT(!FlagGet(0x52));
    FlagToggle(hoennFlag);
    EXPECT(!FlagGet(hoennFlag));
}

TEST("Wayfarer clears Hoenn daily flags separately")
{
    const u16 storyFlag = HOENN_FLAG_ID(0x52);
    const u16 dailyFlag = HOENN_FLAG_ID(WAYFARER_HOENN_DAILY_FLAGS_START + 1);

    FlagSet(storyFlag);
    FlagSet(dailyFlag);
    ClearDailyFlags();
    EXPECT(FlagGet(storyFlag));
    EXPECT(!FlagGet(dailyFlag));
}

TEST("Wayfarer regional badges and League state are isolated")
{
    SetChampionStateForRegion(REGION_JOHTO, FALSE);
    SetChampionStateForRegion(REGION_KANTO, FALSE);
    SetChampionStateForRegion(REGION_HOENN, FALSE);
    SetBadgeStateForRegion(REGION_JOHTO, 0, TRUE);
    SetBadgeStateForRegion(REGION_KANTO, 1, TRUE);
    SetBadgeStateForRegion(REGION_HOENN, 2, TRUE);
    EXPECT_EQ(GetBadgeCountForRegion(REGION_JOHTO), 1);
    EXPECT_EQ(GetBadgeCountForRegion(REGION_KANTO), 1);
    EXPECT_EQ(GetBadgeCountForRegion(REGION_HOENN), 1);

    SetChampionStateForRegion(REGION_JOHTO, TRUE);
    SetChampionStateForRegion(REGION_HOENN, TRUE);
    SetChampionStateForRegion(REGION_HOENN, FALSE);
    EXPECT(GetChampionStateForRegion(REGION_JOHTO));
    EXPECT(!GetChampionStateForRegion(REGION_HOENN));

    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    EXPECT(GetGameClearStateForRegion(REGION_JOHTO));
    EXPECT(!GetGameClearStateForRegion(REGION_KANTO));
    EXPECT(!GetGameClearStateForRegion(REGION_HOENN));

    SetGameClearStateForRegion(REGION_KANTO, TRUE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    EXPECT(!GetGameClearStateForRegion(REGION_JOHTO));
    EXPECT(GetGameClearStateForRegion(REGION_KANTO));
    EXPECT(!GetGameClearStateForRegion(REGION_HOENN));

    FlagClear(FLAG_SYS_GAME_CLEAR);
    SetGameClearStateForRegion(REGION_HOENN, TRUE);
    EXPECT(!FlagGet(FLAG_SYS_GAME_CLEAR));
    SetGameClearStateForRegion(REGION_HOENN, FALSE);
    EXPECT(GetGameClearStateForRegion(REGION_KANTO));
    EXPECT(!GetGameClearStateForRegion(REGION_HOENN));
}

TEST("Wayfarer Hoenn visited state is region aware")
{
    const u16 littleroot = HOENN_FLAG_ID(WAYFARER_HOENN_VISITED_FLAG_START);

    WayfarerInitPersistentState();
    EXPECT(GetRegionVisitedState(REGION_JOHTO));
    EXPECT(!GetRegionVisitedState(REGION_KANTO));
    EXPECT(!GetRegionVisitedState(REGION_HOENN));
    SetLocationVisitedStateForRegion(REGION_HOENN, littleroot, FALSE);
    EXPECT(!GetRegionVisitedState(REGION_HOENN));
    SetRegionVisitedState(REGION_KANTO, TRUE);
    EXPECT(GetRegionVisitedState(REGION_KANTO));
    EXPECT(FlagGet(FLAG_VISITED_KANTO));
    SetLocationVisitedStateForRegion(REGION_HOENN, littleroot, TRUE);
    EXPECT(GetRegionVisitedState(REGION_HOENN));
    EXPECT(GetLocationVisitedStateForRegion(REGION_HOENN, littleroot));
    EXPECT(!GetLocationVisitedStateForRegion(REGION_JOHTO, littleroot));
}

TEST("Wayfarer Hoenn Trainer defeat bits accept only mapped Hoenn ids")
{
    const u16 first = WAYFARER_HOENN_TRAINER_OFFSET + 1;
    const u16 last = WAYFARER_HOENN_TRAINER_OFFSET + WAYFARER_HOENN_TRAINERS_COUNT - 1;

    WayfarerHoennTrainerFlagSet(first);
    WayfarerHoennTrainerFlagSet(last);
    EXPECT(WayfarerHoennTrainerFlagGet(first));
    EXPECT(WayfarerHoennTrainerFlagGet(last));
    EXPECT(!WayfarerHoennTrainerFlagGet(WAYFARER_HOENN_TRAINER_OFFSET));
    EXPECT(!WayfarerHoennTrainerFlagGet(TRAINERS_COUNT_WAYFARER));
    WayfarerHoennTrainerFlagClear(first);
    EXPECT(!WayfarerHoennTrainerFlagGet(first));
    EXPECT(WayfarerHoennTrainerFlagGet(last));
}

TEST("Wayfarer initialization clears all Hoenn state as one unit")
{
    FlagSet(HOENN_FLAG_ID(0x52));
    VarSet(HOENN_VAR_ID(0x40FF), 1234);
    SetBadgeStateForRegion(REGION_HOENN, 0, TRUE);
    WayfarerHoennTrainerFlagSet(WAYFARER_HOENN_TRAINER_OFFSET + 1);
    WayfarerSetHoennStateInitialized(TRUE);
    WayfarerSetSavedCurrentRegion(REGION_HOENN);

    WayfarerInitPersistentState();
    EXPECT(!FlagGet(HOENN_FLAG_ID(0x52)));
    EXPECT_EQ(VarGet(HOENN_VAR_ID(0x40FF)), 0);
    EXPECT_EQ(GetBadgeCountForRegion(REGION_HOENN), 0);
    EXPECT(!WayfarerHoennTrainerFlagGet(WAYFARER_HOENN_TRAINER_OFFSET + 1));
    EXPECT(!WayfarerHoennStateIsInitialized());
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_JOHTO);
}

TEST("Wayfarer invalid saved region falls back safely")
{
    gSaveBlock1Ptr->location.mapGroup = -1;
    gSaveBlock1Ptr->location.mapNum = -1;
    gSaveBlock3Ptr->wayfarerHoenn.currentRegion = REGION_NONE;
    WayfarerValidatePersistentState();
    EXPECT_EQ(WayfarerGetSavedCurrentRegion(), REGION_JOHTO);
}

TEST("Wayfarer SaveBlock3 chunks round trip without replacing sector payloads")
{
    u8 *saved = (u8 *)gSaveBlock3Ptr;
    u32 i;
    u32 sectorId;
    u32 lastUsedSector = (sizeof(struct SaveBlock3) - 1) / SAVE_BLOCK_3_CHUNK_SIZE;

    for (i = 0; i < sizeof(sWayfarerExpectedSaveBlock3); i++)
        sWayfarerExpectedSaveBlock3[i] = (i * 37 + 11) & 0xFF;
    memcpy(saved, sWayfarerExpectedSaveBlock3, sizeof(sWayfarerExpectedSaveBlock3));

    for (sectorId = 0; sectorId < NUM_SECTORS_PER_SLOT; sectorId++)
    {
        memset(&sWayfarerTestSector, 0xA5, sizeof(sWayfarerTestSector));
        Test_CopySaveBlock3ToSector(sectorId, &sWayfarerTestSector);
        memcpy(sWayfarerTestChunks[sectorId], sWayfarerTestSector.saveBlock3Chunk, SAVE_BLOCK_3_CHUNK_SIZE);
        EXPECT_EQ(sWayfarerTestSector.data[0], 0xA5);
        EXPECT_EQ(sWayfarerTestSector.data[SECTOR_DATA_SIZE - 1], 0xA5);
    }

    memset(saved, 0, sizeof(sWayfarerExpectedSaveBlock3));
    for (sectorId = 0; sectorId < NUM_SECTORS_PER_SLOT; sectorId++)
    {
        memcpy(sWayfarerTestSector.saveBlock3Chunk, sWayfarerTestChunks[sectorId], SAVE_BLOCK_3_CHUNK_SIZE);
        Test_CopySaveBlock3FromSector(sectorId, &sWayfarerTestSector);
    }

    for (i = 0; i < sizeof(sWayfarerExpectedSaveBlock3); i++)
        EXPECT_EQ(saved[i], sWayfarerExpectedSaveBlock3[i]);
    EXPECT(Test_GetSaveBlock3ChunkSize(lastUsedSector) > 0);
    EXPECT_EQ(Test_GetSaveBlock3ChunkSize(lastUsedSector + 1), 0);
}

TEST("Wayfarer incremental partial save commits and reloads every SaveBlock3 chunk")
{
    u8 *saveBlock3Bytes = (u8 *)gSaveBlock3Ptr;
    u8 *storageBytes = (u8 *)gPokemonStoragePtr;
    u32 i;
    u32 j;
    u32 sectorId;
    u32 physicalSector;
    u32 storageOffset;
    u32 chunkOffset;
    u32 chunkSize;
    u32 lastUsedSector = (sizeof(struct SaveBlock3) - 1) / SAVE_BLOCK_3_CHUNK_SIZE;
    u32 incrementalCalls = 0;
    u8 loadStatus;
    bool32 incrementalFinished = FALSE;
    bool32 flashLayoutValid = TRUE;
    bool32 saveBlock3RoundTrip = TRUE;
    bool32 storageRoundTrip = TRUE;

    CheckForFlashMemory();
    if (gFlashMemoryPresent != TRUE)
    {
        // The headless test ROM may be probed before mGBA infers its save
        // type. IdentifyFlash has still installed the default 1M handlers.
        gFlashMemoryPresent = TRUE;
        InitFlashTimer();
    }
    ASSUME(gPokemonStoragePtr != NULL);

    ClearSaveData();
    Save_ResetSaveCounters();
    ClearSav1();
    ClearSav2();
    ClearSav3();
    memset(storageBytes, 0, sizeof(*gPokemonStoragePtr));

    // Seed a complete valid slot. Incremental saves replace the active slot,
    // so the untouched storage sectors need to exist before the partial save.
    gSaveBlock1Ptr->saveVersionMagic = SAVE_VERSION_MAGIC;
    gSaveBlock1Ptr->saveVersion = SAVE_VERSION;
    gSaveBlock1Ptr->location.mapGroup = 0;
    gSaveBlock1Ptr->location.mapNum = 0;
    WayfarerInitPersistentState();
    for (i = 0; i < sizeof(*gPokemonStoragePtr); i++)
        storageBytes[i] = (i * 13 + 0x31) & 0xFF;
    HandleSavingData(SAVE_NORMAL);

    // Change every byte carried by the sidecar, while making its validated
    // metadata deliberate so loading does not normalize the expected image.
    for (i = 0; i < sizeof(sWayfarerExpectedSaveBlock3); i++)
        saveBlock3Bytes[i] = (i * 37 + 0x5B) & 0xFF;
    gSaveBlock3Ptr->wayfarerHoenn.magic = WAYFARER_HOENN_STATE_MAGIC;
    gSaveBlock3Ptr->wayfarerHoenn.initialized = TRUE;
    gSaveBlock3Ptr->wayfarerHoenn.currentRegion = REGION_JOHTO;
    gSaveBlock3Ptr->wayfarerHoenn.visitedRegions = 1 << REGION_JOHTO;
    memcpy(sWayfarerExpectedSaveBlock3, saveBlock3Bytes, sizeof(sWayfarerExpectedSaveBlock3));

    // If a storage sector payload is accidentally replaced instead of merely
    // carrying a new sidecar, this different in-memory image will expose it.
    memset(storageBytes, 0xC7, sizeof(*gPokemonStoragePtr));
    if (WriteSaveBlock2())
        flashLayoutValid = FALSE;
    while (!incrementalFinished && incrementalCalls < NUM_SECTORS_PER_SLOT + 2)
    {
        incrementalFinished = WriteSaveBlock1Sector();
        incrementalCalls++;
    }
    if (!incrementalFinished
     || incrementalCalls != lastUsedSector + 1
     || gDamagedSaveSectors != 0)
        flashLayoutValid = FALSE;

    // Inspect the active rotated slot directly. This checks the deferred
    // sector-4 signature as well as each SaveBlock3 chunk written beyond it.
    for (sectorId = 0; sectorId < NUM_SECTORS_PER_SLOT; sectorId++)
    {
        physicalSector = (sectorId + gLastWrittenSector) % NUM_SECTORS_PER_SLOT;
        physicalSector += NUM_SECTORS_PER_SLOT * (gSaveCounter % NUM_SAVE_SLOTS);
        ReadFlash(physicalSector, 0, (u8 *)&sWayfarerTestSector, SECTOR_SIZE);
        if (sWayfarerTestSector.id != sectorId
         || sWayfarerTestSector.signature != SECTOR_SIGNATURE
         || sWayfarerTestSector.counter != gSaveCounter)
            flashLayoutValid = FALSE;

        chunkSize = Test_GetSaveBlock3ChunkSize(sectorId);
        chunkOffset = sectorId * SAVE_BLOCK_3_CHUNK_SIZE;
        for (j = 0; j < chunkSize; j++)
        {
            if (sWayfarerTestSector.saveBlock3Chunk[j] != sWayfarerExpectedSaveBlock3[chunkOffset + j])
                flashLayoutValid = FALSE;
        }

        if (sectorId >= SECTOR_ID_PKMN_STORAGE_START)
        {
            storageOffset = (sectorId - SECTOR_ID_PKMN_STORAGE_START) * SECTOR_DATA_SIZE;
            for (j = 0; j < SECTOR_DATA_SIZE && storageOffset + j < sizeof(*gPokemonStoragePtr); j++)
            {
                if (sWayfarerTestSector.data[j] != ((storageOffset + j) * 13 + 0x31) % 256)
                    flashLayoutValid = FALSE;
            }
        }
    }

    ClearSav1();
    ClearSav2();
    ClearSav3();
    memset(storageBytes, 0, sizeof(*gPokemonStoragePtr));
    loadStatus = LoadGameSave(SAVE_NORMAL);

    for (i = 0; i < sizeof(sWayfarerExpectedSaveBlock3); i++)
    {
        if (saveBlock3Bytes[i] != sWayfarerExpectedSaveBlock3[i])
            saveBlock3RoundTrip = FALSE;
    }
    for (i = 0; i < sizeof(*gPokemonStoragePtr); i++)
    {
        if (storageBytes[i] != (i * 13 + 0x31) % 256)
            storageRoundTrip = FALSE;
    }

    // Leave the runner's private flash image clean even when the assertions
    // below report a regression.
    ClearSaveData();
    Save_ResetSaveCounters();

    EXPECT(flashLayoutValid);
    EXPECT_EQ(loadStatus, SAVE_STATUS_OK);
    EXPECT_EQ(gSaveFileStatus, SAVE_STATUS_OK);
    EXPECT(saveBlock3RoundTrip);
    EXPECT(storageRoundTrip);
    EXPECT(lastUsedSector >= SECTOR_ID_PKMN_STORAGE_START);
}

TEST("Wayfarer Hoenn persistent state stays under one KiB")
{
    EXPECT_LT(sizeof(gSaveBlock3Ptr->wayfarerHoenn), 1024);
    EXPECT_LE(sizeof(struct SaveBlock3), 1624);
}

#endif
