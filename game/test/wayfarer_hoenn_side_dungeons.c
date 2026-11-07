#include "global.h"
#include "braille_puzzles.h"
#include "event_data.h"
#include "fake_rtc.h"
#include "field_specials.h"
#include "mirage_tower.h"
#include "overworld.h"
#include "rtc.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"

#if IS_WAYFARER

// Hoenn's side dungeons keep their state in the Hoenn bank, where the map scripts read it. The C
// that drives them must use the same copies; Emerald's names read as 0 or as a live Johto/Kanto var.
// These spell out Emerald's ids (SYSTEM_FLAGS 0x860) so a wrong macro cannot hide behind itself.

#define SHOAL_TIDE        HOENN_FLAG_ID(0x89A)
#define SHOAL_ITEM        HOENN_FLAG_ID(0x8BF)
#define RUSTURF_OPENED    HOENN_FLAG_ID(0x0C7)
#define RUSTURF_ROCK_1    HOENN_FLAG_ID(0x3A3)
#define RUSTURF_ROCK_2    HOENN_FLAG_ID(0x3A4)
#define RUSTURF_STATE     HOENN_VAR_ID(0x409A)
#define MIRAGE_VISIBLE    HOENN_FLAG_ID(0x14E)
#define MIRAGE_FORCE      HOENN_FLAG_ID(0x09D)
#define MIRAGE_STATE      HOENN_VAR_ID(0x40CB)
#define BRAILLE_DIG       HOENN_FLAG_ID(0x8AF)
#define REGIROCK_DONE     HOENN_FLAG_ID(0x8B0)
#define REGICE_DONE       HOENN_FLAG_ID(0x8B1)
#define REGISTEEL_DONE    HOENN_FLAG_ID(0x8B2)
#define REGICE_STEPS_1    HOENN_VAR_ID(0x403B)
#define REGICE_STEPS_2    HOENN_VAR_ID(0x403C)
#define REGICE_STEPS_3    HOENN_VAR_ID(0x403D)

bool8 TryUpdateRusturfTunnelState(void);
void SetShoalItemFlag(u16 unused);
void UpdateShoalTideFlag(void);
void SetMirageTowerVisibility(void);
bool8 IsMirageTowerVisible(void);

static void StartHoenn(void)
{
    WayfarerInitPersistentState();
    WayfarerSetSavedCurrentRegion(REGION_KANTO);
    EXPECT(WayfarerPrepareHoennEntry(MAP_GROUP(MAP_SLATEPORT_CITY_HARBOR), MAP_NUM(MAP_SLATEPORT_CITY_HARBOR),
                                     9, 11, HEAL_LOCATION_SLATEPORT_CITY));
}

static void StandOn(u32 mapGroup, u32 mapNum, s16 x, s16 y)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
    gSaveBlock1Ptr->pos.x = x;
    gSaveBlock1Ptr->pos.y = y;
}

TEST("Hoenn side dungeons: Shoal Cave's tide follows the hour in the Hoenn bank, and the Lake of Rage tide still follows it too")
{
    // Emerald's tide table: high at 21-02 and 09-14, low otherwise.
    static const u8 highTide[HOURS_PER_DAY] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1};
    struct SiiRtcInfo clock = gSaveBlock3Ptr->fakeRTC;
    struct WarpData lastWarp = gLastUsedWarp;
    u32 hour;

    StartHoenn();
    gLastUsedWarp.mapGroup = MAP_GROUP(MAP_ROUTE111);
    gLastUsedWarp.mapNum = MAP_NUM(MAP_ROUTE111);
    for (hour = 0; hour < HOURS_PER_DAY; hour++)
    {
        FakeRtc_AdvanceTimeBy(0, 1, 0, 0);
        UpdateShoalTideFlag();
        RtcCalcLocalTime();
        EXPECT_EQ(FlagGet(SHOAL_TIDE), highTide[gLocalTime.hours]);
        EXPECT_EQ(FlagGet(FLAG_SYS_LAKE_OF_RAGE_TIDE), highTide[gLocalTime.hours]);
    }
    gLastUsedWarp = lastWarp;
    gSaveBlock3Ptr->fakeRTC = clock;
    RtcCalcLocalTime();
}

TEST("Hoenn side dungeons: the daily update puts Shoal Cave's items back in the Hoenn bank")
{
    StartHoenn();
    FlagClear(SHOAL_ITEM);
    SetShoalItemFlag(1);
    EXPECT(FlagGet(SHOAL_ITEM));
}

TEST("Hoenn side dungeons: Rock Smash in Rusturf Tunnel starts the clearing scene without touching Dana's item var")
{
    u16 danaItem;

    StartHoenn();
    danaItem = VarGet(VAR_DANA_ITEM_HNS);
    StandOn(MAP_GROUP(MAP_RUSTURF_TUNNEL), MAP_NUM(MAP_RUSTURF_TUNNEL), 0, 0);
    FlagClear(RUSTURF_OPENED);
    FlagClear(RUSTURF_ROCK_1);
    FlagClear(RUSTURF_ROCK_2);
    EXPECT(!TryUpdateRusturfTunnelState());

    FlagSet(RUSTURF_ROCK_1);
    EXPECT(TryUpdateRusturfTunnelState());
    EXPECT_EQ(VarGet(RUSTURF_STATE), 4);
    FlagClear(RUSTURF_ROCK_1);
    FlagSet(RUSTURF_ROCK_2);
    EXPECT(TryUpdateRusturfTunnelState());
    EXPECT_EQ(VarGet(RUSTURF_STATE), 5);
    EXPECT_EQ(VarGet(VAR_DANA_ITEM_HNS), danaItem);

    // Once the tunnel is open the scene never starts again.
    VarSet(RUSTURF_STATE, 6);
    FlagSet(RUSTURF_OPENED);
    EXPECT(!TryUpdateRusturfTunnelState());
    EXPECT_EQ(VarGet(RUSTURF_STATE), 6);
}

TEST("Hoenn side dungeons: the Mirage Tower shows from the Hoenn flags and goes once its event is done")
{
    StartHoenn();
    StandOn(MAP_GROUP(MAP_ROUTE111), MAP_NUM(MAP_ROUTE111), 0, 0);
    VarSet(MIRAGE_STATE, 0);
    FlagClear(MIRAGE_VISIBLE);
    FlagSet(MIRAGE_FORCE);
    SetMirageTowerVisibility();
    EXPECT(FlagGet(MIRAGE_VISIBLE));
    EXPECT(IsMirageTowerVisible());
    ClearMirageTowerPulseBlendEffect();

    VarSet(MIRAGE_STATE, 3);
    SetMirageTowerVisibility();
    EXPECT(!FlagGet(MIRAGE_VISIBLE));
    EXPECT(!IsMirageTowerVisible());
    FlagClear(MIRAGE_FORCE);
}

TEST("Hoenn side dungeons: each braille chamber stays solved from the Hoenn bank")
{
    StartHoenn();
    FlagClear(BRAILLE_DIG);
    FlagClear(REGIROCK_DONE);
    FlagClear(REGISTEEL_DONE);

    StandOn(MAP_GROUP(MAP_SEALED_CHAMBER_OUTER_ROOM), MAP_NUM(MAP_SEALED_CHAMBER_OUTER_ROOM), 10, 3);
    EXPECT(ShouldDoBrailleDigEffect());
    FlagSet(BRAILLE_DIG);
    EXPECT(!ShouldDoBrailleDigEffect());

    StandOn(MAP_GROUP(MAP_DESERT_RUINS), MAP_NUM(MAP_DESERT_RUINS), 6, 23);
    EXPECT(ShouldDoBrailleRegirockEffect());
    FlagSet(REGIROCK_DONE);
    EXPECT(!ShouldDoBrailleRegirockEffect());

    StandOn(MAP_GROUP(MAP_ANCIENT_TOMB), MAP_NUM(MAP_ANCIENT_TOMB), 8, 25);
    EXPECT(ShouldDoBrailleRegisteelEffect());
    FlagSet(REGISTEEL_DONE);
    EXPECT(!ShouldDoBrailleRegisteelEffect());
}

TEST("Hoenn side dungeons: the Regice lap is counted in the Hoenn vars, not Kanto's Resort Gorgeous and Heracross vars")
{
    u16 gorgeous, heracross;

    StartHoenn();
    gorgeous = VarGet(VAR_RESORT_GORGEOUS_REWARD);
    heracross = VarGet(VAR_HERACROSS_SIZE_RECORD);
    VarSet(REGICE_STEPS_1, 0);
    VarSet(REGICE_STEPS_2, 0);
    VarSet(REGICE_STEPS_3, 0);
    FlagClear(REGICE_DONE);
    FlagSet(FLAG_TEMP_REGICE_PUZZLE_STARTED);
    FlagClear(FLAG_TEMP_REGICE_PUZZLE_FAILED);

    StandOn(MAP_GROUP(MAP_ISLAND_CAVE), MAP_NUM(MAP_ISLAND_CAVE), 4, 21); // the first step of the lap
    EXPECT(!ShouldDoBrailleRegicePuzzle());
    EXPECT_EQ(VarGet(REGICE_STEPS_1), 1);
    StandOn(MAP_GROUP(MAP_ISLAND_CAVE), MAP_NUM(MAP_ISLAND_CAVE), 5, 21);
    EXPECT(!ShouldDoBrailleRegicePuzzle());
    EXPECT_EQ(VarGet(REGICE_STEPS_1), 3);
    EXPECT_EQ(VarGet(VAR_RESORT_GORGEOUS_REWARD), gorgeous);
    EXPECT_EQ(VarGet(VAR_HERACROSS_SIZE_RECORD), heracross);

    // A solved puzzle stops counting.
    FlagSet(REGICE_DONE);
    StandOn(MAP_GROUP(MAP_ISLAND_CAVE), MAP_NUM(MAP_ISLAND_CAVE), 6, 21);
    EXPECT(!ShouldDoBrailleRegicePuzzle());
    EXPECT_EQ(VarGet(REGICE_STEPS_1), 3);
    FlagClear(FLAG_TEMP_REGICE_PUZZLE_STARTED);
}

#endif // IS_WAYFARER
