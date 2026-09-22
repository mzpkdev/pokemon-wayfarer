#include "global.h"
#include "battle_pyramid.h"
#include "decoration.h"
#include "event_data.h"
#include "fieldmap.h"
#include "malloc.h"
#include "map_layout.h"
#include "overworld.h"
#include "save.h"
#include "secret_base.h"
#include "trainer_hill.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/decorations.h"
#include "constants/layouts.h"
#include "constants/metatile_labels.h"
#include "constants/trainer_hill.h"

#if IS_WAYFARER

#define PYRAMID_TEMPLATE_COUNT 16
#define PYRAMID_LAYOUT_OPTION_COUNT 8

extern const struct MapLayout *const gMapLayouts[];

static void FillUndefined(u16 *map)
{
    CpuFastFill16(MAPGRID_UNDEFINED, map, MAX_MAP_DATA_SIZE * sizeof(*map));
}

TEST("Map layout special consumers keep raw exceptions explicit")
{
    u32 layoutId;

    for (layoutId = LAYOUT_SECRET_BASE_RED_CAVE1;
         layoutId <= LAYOUT_SECRET_BASE_SHRUB4;
         layoutId++)
        EXPECT_EQ(Test_MapLayoutGetDescriptor(gMapLayouts[layoutId - 1])->codec, 0);
    for (layoutId = LAYOUT_TRAINER_HILL_1F_HNS;
         layoutId <= LAYOUT_TRAINER_HILL_4F_HNS;
         layoutId++)
        EXPECT_EQ(Test_MapLayoutGetDescriptor(gMapLayouts[layoutId - 1])->codec, 0);
    for (layoutId = LAYOUT_BATTLE_PYRAMID_SQUARE01_HNS;
         layoutId <= LAYOUT_BATTLE_PYRAMID_SQUARE16_HNS;
         layoutId++)
        EXPECT_EQ(Test_MapLayoutGetDescriptor(gMapLayouts[layoutId - 1])->codec, 0);
}

TEST("Map layout special consumers generate every Battle Pyramid template identically")
{
    u16 *expected = Alloc(MAX_MAP_DATA_SIZE * sizeof(*expected));
    struct MapHeader previousHeader = gMapHeader;
    u8 layoutOptions[NUM_PYRAMID_FLOOR_SQUARES];
    bool8 covered[NUM_PYRAMID_FLOOR_SQUARES][PYRAMID_LAYOUT_OPTION_COUNT];
    u16 templateId;
    u16 pattern;
    u16 option;

    EXPECT(expected != NULL);
    for (templateId = 0; templateId < PYRAMID_TEMPLATE_COUNT; templateId++)
    {
        memset(covered, FALSE, sizeof(covered));
        for (pattern = 0; pattern < 3; pattern++)
        {
            for (option = 0; option < PYRAMID_LAYOUT_OPTION_COUNT; option++)
            {
                u8 entrance = option + (pattern == 1 ? 8 : 0);
                u8 exit = (entrance + NUM_PYRAMID_FLOOR_SQUARES / 2)
                          % NUM_PYRAMID_FLOOR_SQUARES;
                s16 expectedX;
                s16 expectedY;
                u32 expectedWidth;
                u32 expectedHeight;
                u32 i;

                // The generator composes 16 independent square choices. The
                // rotation exhausts every square/option pair; uniform and
                // paired-stride patterns cover correlated selectable tuples.
                for (i = 0; i < ARRAY_COUNT(layoutOptions); i++)
                {
                    if (pattern == 0)
                        layoutOptions[i] = (option + i) % PYRAMID_LAYOUT_OPTION_COUNT;
                    else if (pattern == 1)
                        layoutOptions[i] = option;
                    else
                        layoutOptions[i] = (option + (i / 2) * 3)
                                           % PYRAMID_LAYOUT_OPTION_COUNT;
                    covered[i][layoutOptions[i]] = TRUE;
                }

                FillUndefined(expected);
                Test_MapLayoutResetHooks();
                Test_MapLayoutUseRawOracle(TRUE);
                gSaveBlock1Ptr->pos.x = -1;
                gSaveBlock1Ptr->pos.y = -1;
                EXPECT_EQ(Test_GenerateBattlePyramidFloorLayout(expected, templateId,
                                                                 layoutOptions, entrance,
                                                                 exit, FALSE),
                          MAP_LAYOUT_LOAD_OK);
                expectedX = gSaveBlock1Ptr->pos.x;
                expectedY = gSaveBlock1Ptr->pos.y;
                expectedWidth = gBackupMapLayout.width;
                expectedHeight = gBackupMapLayout.height;

                FillUndefined(sBackupMapData);
                Test_MapLayoutResetHooks();
                gSaveBlock1Ptr->pos.x = -1;
                gSaveBlock1Ptr->pos.y = -1;
                EXPECT_EQ(Test_GenerateBattlePyramidFloorLayout(sBackupMapData, templateId,
                                                                 layoutOptions, entrance,
                                                                 exit, FALSE),
                          MAP_LAYOUT_LOAD_OK);
                EXPECT(gBackupMapLayout.map == sBackupMapData);
                EXPECT_EQ(gBackupMapLayout.width, expectedWidth);
                EXPECT_EQ(gBackupMapLayout.height, expectedHeight);
                EXPECT_EQ(gSaveBlock1Ptr->pos.x, expectedX);
                EXPECT_EQ(gSaveBlock1Ptr->pos.y, expectedY);
                EXPECT_EQ(memcmp(expected, sBackupMapData,
                                 MAX_MAP_DATA_SIZE * sizeof(*expected)), 0);
            }
        }
        for (option = 0; option < PYRAMID_LAYOUT_OPTION_COUNT; option++)
            for (pattern = 0; pattern < NUM_PYRAMID_FLOOR_SQUARES; pattern++)
                EXPECT(covered[pattern][option]);
    }

    FillUndefined(sBackupMapData);
    Test_MapLayoutResetHooks();
    gSaveBlock1Ptr->pos.x = 41;
    gSaveBlock1Ptr->pos.y = 73;
    memset(layoutOptions, 0, sizeof(layoutOptions));
    EXPECT_EQ(Test_GenerateBattlePyramidFloorLayout(sBackupMapData, 0, layoutOptions,
                                                     0, 1, TRUE),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(gSaveBlock1Ptr->pos.x, 41);
    EXPECT_EQ(gSaveBlock1Ptr->pos.y, 73);
    EXPECT_EQ(Test_GenerateBattlePyramidFloorLayout(sBackupMapData,
                                                     PYRAMID_TEMPLATE_COUNT, layoutOptions,
                                                     0, 1, FALSE),
              MAP_LAYOUT_LOAD_BAD_BOUNDS);
    gMapHeader = previousHeader;
    Test_MapLayoutResetHooks();
    Free(expected);
}

TEST("Map layout special consumers generate every Trainer Hill floor and mode identically")
{
    static const u16 sFloorMaps[] =
    {
        MAP_TRAINER_HILL_ENTRANCE_HNS,
        MAP_TRAINER_HILL_1F_HNS,
        MAP_TRAINER_HILL_2F_HNS,
        MAP_TRAINER_HILL_3F_HNS,
        MAP_TRAINER_HILL_4F_HNS,
        MAP_TRAINER_HILL_ROOF_HNS,
    };
    u16 *expected = Alloc(MAX_MAP_DATA_SIZE * sizeof(*expected));
    struct MapHeader previousHeader = gMapHeader;
    u8 previousMode = gSaveBlock1Ptr->trainerHill.mode;
    s8 previousGroup = gSaveBlock1Ptr->location.mapGroup;
    s8 previousNum = gSaveBlock1Ptr->location.mapNum;
    u32 floor;
    u32 mode;

    EXPECT(expected != NULL);
    for (floor = 0; floor < ARRAY_COUNT(sFloorMaps); floor++)
    {
        const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(
            MAP_GROUP(sFloorMaps[floor]), MAP_NUM(sFloorMaps[floor]));

        EXPECT(header != NULL);
        for (mode = 0; mode < NUM_TRAINER_HILL_MODES; mode++)
        {
            u32 expectedWidth;
            u32 expectedHeight;

            gMapHeader = *header;
            gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(sFloorMaps[floor]);
            gSaveBlock1Ptr->location.mapNum = MAP_NUM(sFloorMaps[floor]);
            gSaveBlock1Ptr->trainerHill.mode = mode;
            memset(gSaveBlock1Ptr->mapView, 0, sizeof(gSaveBlock1Ptr->mapView));
            FillUndefined(expected);
            FillUndefined(sBackupMapData);
            Test_MapLayoutResetHooks();
            Test_MapLayoutUseRawOracle(TRUE);
            EXPECT_EQ(GenerateTrainerHillFloorLayout(expected), MAP_LAYOUT_LOAD_OK);
            if (floor == 0 || floor == ARRAY_COUNT(sFloorMaps) - 1)
                memcpy(expected, sBackupMapData, MAX_MAP_DATA_SIZE * sizeof(*expected));
            expectedWidth = gBackupMapLayout.width;
            expectedHeight = gBackupMapLayout.height;

            FillUndefined(sBackupMapData);
            Test_MapLayoutResetHooks();
            memset(gSaveBlock1Ptr->mapView, 0, sizeof(gSaveBlock1Ptr->mapView));
            EXPECT_EQ(GenerateTrainerHillFloorLayout(sBackupMapData), MAP_LAYOUT_LOAD_OK);
            EXPECT(gBackupMapLayout.map == sBackupMapData);
            EXPECT_EQ(gBackupMapLayout.width, expectedWidth);
            EXPECT_EQ(gBackupMapLayout.height, expectedHeight);
            EXPECT_EQ(memcmp(expected, sBackupMapData,
                             MAX_MAP_DATA_SIZE * sizeof(*expected)), 0);
            if (floor != 0 && floor != ARRAY_COUNT(sFloorMaps) - 1)
                EXPECT(LoadTrainerHillFloorObjectEventScripts());
        }
    }
    gSaveBlock1Ptr->trainerHill.mode = previousMode;
    gSaveBlock1Ptr->location.mapGroup = previousGroup;
    gSaveBlock1Ptr->location.mapNum = previousNum;
    gMapHeader = previousHeader;
    Test_MapLayoutResetHooks();
    Free(expected);
}

TEST("Map layout special consumers search every Secret Base immutable layout identically")
{
    struct MapHeader previousHeader = gMapHeader;
    s16 previousGroup = gSaveBlock1Ptr->location.mapGroup;
    s16 previousNum = gSaveBlock1Ptr->location.mapNum;
    u32 mapNum;

    for (mapNum = MAP_NUM(MAP_SECRET_BASE_RED_CAVE1);
         mapNum <= MAP_NUM(MAP_SECRET_BASE_SHRUB4);
         mapNum++)
    {
        const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(
            MAP_GROUP(MAP_SECRET_BASE_RED_CAVE1), mapNum);
        s16 rawPcX = -1;
        s16 rawPcY = -1;
        s16 actualPcX = -1;
        s16 actualPcY = -1;
        s16 rawEntranceX = -1;
        s16 rawEntranceY = -1;
        s16 actualEntranceX = -1;
        s16 actualEntranceY = -1;
        u16 entranceTile;
        u16 actualEntranceTile;

        EXPECT(header != NULL);
        EXPECT(header->events != NULL);
        EXPECT(header->events->warpCount > 0);
        gMapHeader = *header;
        gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_SECRET_BASE_RED_CAVE1);
        gSaveBlock1Ptr->location.mapNum = mapNum;

        Test_MapLayoutResetHooks();
        Test_MapLayoutUseRawOracle(TRUE);
        EXPECT_EQ(Test_FindSecretBaseMetatile(&rawPcX, &rawPcY, METATILE_SecretBase_PC),
                  MAP_LAYOUT_LOAD_OK);
        EXPECT_EQ(MapLayoutReadTile(header->mapLayout,
                                    header->events->warps[0].x,
                                    header->events->warps[0].y,
                                    &entranceTile),
                  MAP_LAYOUT_LOAD_OK);
        EXPECT_EQ(Test_FindSecretBaseMetatile(&rawEntranceX, &rawEntranceY,
                                               entranceTile & MAPGRID_METATILE_ID_MASK),
                  MAP_LAYOUT_LOAD_OK);

        Test_MapLayoutResetHooks();
        EXPECT_EQ(Test_FindSecretBaseMetatile(&actualPcX, &actualPcY, METATILE_SecretBase_PC),
                  MAP_LAYOUT_LOAD_OK);
        EXPECT_EQ(MapLayoutReadTile(header->mapLayout,
                                    header->events->warps[0].x,
                                    header->events->warps[0].y,
                                    &actualEntranceTile),
                  MAP_LAYOUT_LOAD_OK);
        EXPECT_EQ(Test_FindSecretBaseMetatile(&actualEntranceX, &actualEntranceY,
                                               actualEntranceTile & MAPGRID_METATILE_ID_MASK),
                  MAP_LAYOUT_LOAD_OK);
        EXPECT_EQ(actualPcX, rawPcX);
        EXPECT_EQ(actualPcY, rawPcY);
        EXPECT_EQ(actualEntranceTile, entranceTile);
        EXPECT_EQ(actualEntranceX, rawEntranceX);
        EXPECT_EQ(actualEntranceY, rawEntranceY);
    }

    gSaveBlock1Ptr->location.mapGroup = previousGroup;
    gSaveBlock1Ptr->location.mapNum = previousNum;
    gMapHeader = previousHeader;
    Test_MapLayoutResetHooks();
}

TEST("Map layout special consumers preserve Secret Base appearance across saved reload")
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_SECRET_BASE_RED_CAVE1), MAP_NUM(MAP_SECRET_BASE_RED_CAVE1));
    struct MapHeader previousHeader = gMapHeader;
    struct SecretBase savedBase = gSaveBlock1Ptr->secretBases[1];
    s16 previousGroup = gSaveBlock1Ptr->location.mapGroup;
    s16 previousNum = gSaveBlock1Ptr->location.mapNum;
    u16 *expected = Alloc(MAX_MAP_DATA_SIZE * sizeof(*expected));
    s16 pcX = -1;
    s16 pcY = -1;
    s16 rawAfterPcX = -1;
    s16 rawAfterPcY = -1;
    s16 rawAfterEntranceX = -1;
    s16 rawAfterEntranceY = -1;
    s16 actualAfterPcX = -1;
    s16 actualAfterPcY = -1;
    s16 actualAfterEntranceX = -1;
    s16 actualAfterEntranceY = -1;
    u16 entranceTile;
    u16 spriteAuthoredTile;
    u32 spriteIndex;

    EXPECT(expected != NULL);
    gMapHeader = *header;
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_SECRET_BASE_RED_CAVE1);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_SECRET_BASE_RED_CAVE1);
    memset(&gSaveBlock1Ptr->secretBases[1], 0, sizeof(gSaveBlock1Ptr->secretBases[1]));
    gSaveBlock1Ptr->secretBases[1].secretBaseId = 1;
    VarSet(VAR_CURRENT_SECRET_BASE, 1);

    Test_MapLayoutResetHooks();
    Test_MapLayoutUseRawOracle(TRUE);
    EXPECT_EQ(Test_FindSecretBaseMetatile(&pcX, &pcY, METATILE_SecretBase_PC),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(MapLayoutReadTile(header->mapLayout,
                                header->events->warps[0].x,
                                header->events->warps[0].y,
                                &entranceTile),
              MAP_LAYOUT_LOAD_OK);
    gSaveBlock1Ptr->secretBases[1].decorations[0] = DECOR_SMALL_DESK;
    gSaveBlock1Ptr->secretBases[1].decorationPositions[0] = (pcX << 4) | pcY;
    gSaveBlock1Ptr->secretBases[1].decorations[1] = DECOR_PICHU_DOLL;
    gSaveBlock1Ptr->secretBases[1].decorationPositions[1] =
        (header->events->warps[0].x << 4) | header->events->warps[0].y;
    memset(gSaveBlock1Ptr->mapView, 0, sizeof(gSaveBlock1Ptr->mapView));
    gSaveBlock1Ptr->pos.x = MAP_OFFSET;
    gSaveBlock1Ptr->pos.y = MAP_OFFSET;
    EXPECT_EQ(InitMapFromSavedGame(), MAP_LAYOUT_LOAD_OK);
    spriteIndex = (header->events->warps[0].y + MAP_OFFSET) * gBackupMapLayout.width
                  + header->events->warps[0].x + MAP_OFFSET;
    spriteAuthoredTile = sBackupMapData[spriteIndex];
    EXPECT_EQ(sBackupMapData[spriteIndex], spriteAuthoredTile);
    EXPECT_EQ(Test_FindSecretBaseMetatile(&rawAfterPcX, &rawAfterPcY,
                                           METATILE_SecretBase_PC),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(Test_FindSecretBaseMetatile(&rawAfterEntranceX, &rawAfterEntranceY,
                                           entranceTile & MAPGRID_METATILE_ID_MASK),
              MAP_LAYOUT_LOAD_OK);
    memcpy(expected, sBackupMapData, MAX_MAP_DATA_SIZE * sizeof(*expected));
    SaveMapView();
    EXPECT_EQ(InitMapFromSavedGame(), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(memcmp(expected, sBackupMapData,
                     MAX_MAP_DATA_SIZE * sizeof(*expected)), 0);

    Test_MapLayoutResetHooks();
    memset(gSaveBlock1Ptr->mapView, 0, sizeof(gSaveBlock1Ptr->mapView));
    EXPECT_EQ(InitMapFromSavedGame(), MAP_LAYOUT_LOAD_OK);
    SaveMapView();
    EXPECT_EQ(InitMapFromSavedGame(), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(sBackupMapData[spriteIndex], spriteAuthoredTile);
    EXPECT_EQ(Test_FindSecretBaseMetatile(&actualAfterPcX, &actualAfterPcY,
                                           METATILE_SecretBase_PC),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(Test_FindSecretBaseMetatile(&actualAfterEntranceX, &actualAfterEntranceY,
                                           entranceTile & MAPGRID_METATILE_ID_MASK),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(actualAfterPcX, rawAfterPcX);
    EXPECT_EQ(actualAfterPcY, rawAfterPcY);
    EXPECT_EQ(actualAfterEntranceX, rawAfterEntranceX);
    EXPECT_EQ(actualAfterEntranceY, rawAfterEntranceY);
    EXPECT_EQ(memcmp(expected, sBackupMapData,
                     MAX_MAP_DATA_SIZE * sizeof(*expected)), 0);
    EXPECT_EQ(MapGridGetMetatileIdAt(pcX + MAP_OFFSET, pcY + MAP_OFFSET),
              METATILE_SecretBase_RegisterPC);

    gSaveBlock1Ptr->secretBases[1] = savedBase;
    gSaveBlock1Ptr->location.mapGroup = previousGroup;
    gSaveBlock1Ptr->location.mapNum = previousNum;
    gMapHeader = previousHeader;
    Test_MapLayoutResetHooks();
    Free(expected);
}

TEST("Map layout special consumers restore decoration tiles from immutable authored data")
{
    const struct MapHeader *header = Overworld_GetMapHeaderByGroupAndId(
        MAP_GROUP(MAP_SECRET_BASE_RED_CAVE1), MAP_NUM(MAP_SECRET_BASE_RED_CAVE1));
    struct MapHeader previousHeader = gMapHeader;
    u16 *expected = Alloc(MAX_MAP_DATA_SIZE * sizeof(*expected));
    u16 *expectedPlaced = Alloc(MAX_MAP_DATA_SIZE * sizeof(*expectedPlaced));
    s16 x = -1;
    s16 y = -1;
    u32 tileX;
    u32 tileY;
    u32 spriteIndex;
    u16 spriteBefore;

    EXPECT(expected != NULL);
    EXPECT(expectedPlaced != NULL);
    gMapHeader = *header;

    Test_MapLayoutResetHooks();
    Test_MapLayoutUseRawOracle(TRUE);
    EXPECT_EQ(Test_FindSecretBaseMetatile(&x, &y, METATILE_SecretBase_PC),
              MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(Test_InitMapLayoutData(&gMapHeader), MAP_LAYOUT_LOAD_OK);
    memcpy(expected, sBackupMapData, MAX_MAP_DATA_SIZE * sizeof(*expected));
    EXPECT_EQ(Test_ApplyDecoration(x, y, DECOR_PRETTY_DESK), MAP_LAYOUT_LOAD_OK);
    memcpy(expectedPlaced, sBackupMapData, MAX_MAP_DATA_SIZE * sizeof(*expectedPlaced));
    EXPECT(memcmp(expectedPlaced, expected,
                  MAX_MAP_DATA_SIZE * sizeof(*expectedPlaced)) != 0);
    EXPECT_EQ(Test_RemoveDecoration(x, y, DECOR_PRETTY_DESK), MAP_LAYOUT_LOAD_OK);
    for (tileY = 0; tileY < 3; tileY++)
    {
        for (tileX = 0; tileX < 3; tileX++)
        {
            u16 authoredTile;

            EXPECT_EQ(MapLayoutReadTile(gMapHeader.mapLayout, x + tileX, y - tileY,
                                        &authoredTile), MAP_LAYOUT_LOAD_OK);
            EXPECT_EQ(MapGridGetMetatileIdAt(x + MAP_OFFSET + tileX,
                                             y + MAP_OFFSET - tileY),
                      authoredTile & MAPGRID_METATILE_ID_MASK);
        }
    }
    memcpy(expected, sBackupMapData, MAX_MAP_DATA_SIZE * sizeof(*expected));

    Test_MapLayoutResetHooks();
    EXPECT_EQ(Test_InitMapLayoutData(&gMapHeader), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(Test_ApplyDecoration(x, y, DECOR_PRETTY_DESK), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(memcmp(expectedPlaced, sBackupMapData,
                     MAX_MAP_DATA_SIZE * sizeof(*expectedPlaced)), 0);
    EXPECT_EQ(Test_RemoveDecoration(x, y, DECOR_PRETTY_DESK), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(memcmp(expected, sBackupMapData,
                     MAX_MAP_DATA_SIZE * sizeof(*expected)), 0);

    spriteIndex = (y + MAP_OFFSET) * gBackupMapLayout.width + x + MAP_OFFSET;
    spriteBefore = sBackupMapData[spriteIndex];
    EXPECT_EQ(Test_ApplyDecoration(x, y, DECOR_PICHU_DOLL), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(sBackupMapData[spriteIndex], spriteBefore);
    EXPECT_EQ(Test_RemoveDecoration(x, y, DECOR_PICHU_DOLL), MAP_LAYOUT_LOAD_OK);
    EXPECT_EQ(sBackupMapData[spriteIndex], spriteBefore);
    EXPECT_EQ(Test_RestoreDecorationTiles(-1, y, 1, 1, FALSE),
              MAP_LAYOUT_LOAD_BAD_BOUNDS);

    gMapHeader = previousHeader;
    Test_MapLayoutResetHooks();
    Free(expectedPlaced);
    Free(expected);
}

#endif // IS_WAYFARER
