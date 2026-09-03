#include "global.h"
#include "nuzlocke.h"
#include "test/test.h"
#include "constants/map_groups.h"
#include "constants/region_map_sections.h"

#if IS_WAYFARER

enum
{
    HOENN_MAPSEC_PETALBURG_CITY = 7,
    HOENN_MAPSEC_AQUA_HIDEOUT = 193,
    HOENN_MAPSEC_TRAINER_HILL = 218,
    HOENN_MAPSEC_NONE = 219,
};

static void SetCurrentMap(u16 map)
{
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(map);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(map);
}

TEST("Wayfarer Hoenn Nuzlocke flags do not alias low HNS section values")
{
    u8 hnsFlags[sizeof(gSaveBlock3Ptr->challengeSettings.nuzlockeEncounterFlags)];

    SetCurrentMap(MAP_NEW_BARK_TOWN_HNS);
    NuzlockeFlagClear(MAPSEC_ROUTE_1);
    NuzlockeFlagSet(MAPSEC_ROUTE_1);
    EXPECT(NuzlockeFlagGet(MAPSEC_ROUTE_1));
    memcpy(hnsFlags,
           gSaveBlock3Ptr->challengeSettings.nuzlockeEncounterFlags,
           sizeof(hnsFlags));

    SetCurrentMap(MAP_PETALBURG_CITY);
    NuzlockeFlagClear(HOENN_MAPSEC_PETALBURG_CITY);
    EXPECT(!NuzlockeFlagGet(HOENN_MAPSEC_PETALBURG_CITY));
    NuzlockeFlagSet(HOENN_MAPSEC_PETALBURG_CITY);
    EXPECT(NuzlockeFlagGet(HOENN_MAPSEC_PETALBURG_CITY));
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.nuzlockeEncounterFlags[0] & (1 << 7), 1 << 7);
    EXPECT_EQ(memcmp(hnsFlags,
                     gSaveBlock3Ptr->challengeSettings.nuzlockeEncounterFlags,
                     sizeof(hnsFlags)),
              0);

    SetCurrentMap(MAP_NEW_BARK_TOWN_HNS);
    EXPECT(NuzlockeFlagGet(MAPSEC_ROUTE_1));
}

TEST("Wayfarer Hoenn Nuzlocke storage safely covers high raw sections")
{
    u8 hnsFlags[sizeof(gSaveBlock3Ptr->challengeSettings.nuzlockeEncounterFlags)];
    u8 hoennFlags[sizeof(gSaveBlock3Ptr->wayfarerHoenn.nuzlockeEncounterFlags)];

    SetCurrentMap(MAP_PETALBURG_CITY);
    NuzlockeFlagClear(HOENN_MAPSEC_AQUA_HIDEOUT);
    NuzlockeFlagClear(HOENN_MAPSEC_TRAINER_HILL);
    memcpy(hnsFlags,
           gSaveBlock3Ptr->challengeSettings.nuzlockeEncounterFlags,
           sizeof(hnsFlags));

    NuzlockeFlagSet(HOENN_MAPSEC_AQUA_HIDEOUT);
    NuzlockeFlagSet(HOENN_MAPSEC_TRAINER_HILL);
    EXPECT(NuzlockeFlagGet(HOENN_MAPSEC_AQUA_HIDEOUT));
    EXPECT(NuzlockeFlagGet(HOENN_MAPSEC_TRAINER_HILL));
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.nuzlockeEncounterFlags[24] & (1 << 1), 1 << 1);
    EXPECT_EQ(gSaveBlock3Ptr->wayfarerHoenn.nuzlockeEncounterFlags[27] & (1 << 2), 1 << 2);

    NuzlockeFlagClear(HOENN_MAPSEC_AQUA_HIDEOUT);
    EXPECT(!NuzlockeFlagGet(HOENN_MAPSEC_AQUA_HIDEOUT));
    EXPECT(NuzlockeFlagGet(HOENN_MAPSEC_TRAINER_HILL));

    // MAPSEC_NONE and values outside the raw Hoenn range are safe no-ops.
    memcpy(hoennFlags,
           gSaveBlock3Ptr->wayfarerHoenn.nuzlockeEncounterFlags,
           sizeof(hoennFlags));
    NuzlockeFlagSet(HOENN_MAPSEC_NONE);
    NuzlockeFlagSet(0xFFFF);
    EXPECT(!NuzlockeFlagGet(HOENN_MAPSEC_NONE));
    EXPECT(!NuzlockeFlagGet(0xFFFF));
    EXPECT_EQ(memcmp(hoennFlags,
                     gSaveBlock3Ptr->wayfarerHoenn.nuzlockeEncounterFlags,
                     sizeof(hoennFlags)),
              0);
    EXPECT_EQ(memcmp(hnsFlags,
                     gSaveBlock3Ptr->challengeSettings.nuzlockeEncounterFlags,
                     sizeof(hnsFlags)),
              0);
}

TEST("Wayfarer HNS Nuzlocke mapping remains unchanged")
{
    SetCurrentMap(MAP_NEW_BARK_TOWN_HNS);
    NuzlockeFlagClear(MAPSEC_ROUTE_1);
    NuzlockeFlagClear(MAPSEC_ROUTE_2);

    EXPECT(!NuzlockeFlagGet(MAPSEC_ROUTE_1));
    EXPECT(!NuzlockeFlagGet(MAPSEC_ROUTE_2));
    NuzlockeFlagSet(MAPSEC_ROUTE_1);
    EXPECT(NuzlockeFlagGet(MAPSEC_ROUTE_1));
    EXPECT(!NuzlockeFlagGet(MAPSEC_ROUTE_2));
    NuzlockeFlagClear(MAPSEC_ROUTE_1);
    EXPECT(!NuzlockeFlagGet(MAPSEC_ROUTE_1));
}

#endif
