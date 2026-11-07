#include "global.h"
#include "event_data.h"
#include "field_specials.h"
#include "malloc.h"
#include "pokenav.h"
#include "string_util.h"
#include "strings.h"
#include "test/test.h"
#include "constants/map_types.h"
#include "constants/wayfarer_persistence.h"

// SCOTT is a Pokégear contact once HOENN's Slateport scenes register him, and his calls follow
// HOENN's story, all read from HOENN's bank (Emerald ids in HOENN_FLAG_ID).

static bool32 ScottSays(const u8 *text)
{
    u8 *call = Alloc(1000);
    u8 *expected = Alloc(1000);
    bool32 same;

    MatchCall_GetMessage(MC_HEADER_SCOTT, call);
    StringExpandPlaceholders(expected, text);
    same = StringCompare(call, expected) == 0;
    Free(call);
    Free(expected);
    return same;
}

TEST("Scott is on the Pokégear once HOENN registers him")
{
    FlagClear(HOENN_FLAG_ID(0x0D7));
    EXPECT(!MatchCall_GetEnabled(MC_HEADER_SCOTT));
    FlagSet(HOENN_FLAG_ID(0x0D7));
    EXPECT(MatchCall_GetEnabled(MC_HEADER_SCOTT));
}

TEST("Scott's calls follow HOENN's story")
{
    u32 i;
    static const u16 steps[] = {
        HOENN_FLAG_ID(0x08B), // Mt. Chimney
        HOENN_FLAG_ID(0x097), // Castform
        HOENN_FLAG_ID(0x0D4), // the orb
        HOENN_FLAG_ID(0x070), // Aqua escaped
        HOENN_FLAG_ID(0x4F7), // Juan
        HOENN_FLAG_ID(0x864), // HOENN's League
    };
    static const u8 *const calls[] = {
        MatchCall_Text_Scott2, MatchCall_Text_Scott3, MatchCall_Text_Scott4,
        MatchCall_Text_Scott5, MatchCall_Text_Scott6, MatchCall_Text_Scott7,
    };

    // The calls name the player, so give the save one.
    StringCopy(gSaveBlock2Ptr->playerName, COMPOUND_STRING("MAY"));
    for (i = 0; i < ARRAY_COUNT(steps); i++)
        FlagClear(steps[i]);
    FlagSet(HOENN_FLAG_ID(0x0D7));
    EXPECT(ScottSays(MatchCall_Text_Scott1));
    for (i = 0; i < ARRAY_COUNT(steps); i++)
    {
        FlagSet(steps[i]);
        EXPECT(ScottSays(calls[i]));
    }
}

// Winona's badge and Birch's Hall of Fame scene arm SCOTT's two calls. Each rings ten steps later
// in a town or on a route, and only once he is on the Pokégear.
TEST("Scott's Fortree and Frontier calls ring only once he is registered")
{
    u32 step;
    struct MapHeader header = gMapHeader;

    gMapHeader.mapType = MAP_TYPE_ROUTE;
    FlagClear(HOENN_FLAG_ID(0x0D7));
    FlagSet(HOENN_FLAG_ID(0x08A));
    FlagSet(HOENN_FLAG_ID(0x072));
    VarSet(HOENN_VAR_ID(0x40F3), 0);
    VarSet(HOENN_VAR_ID(0x40F5), 0);
    for (step = 0; step < 20; step++)
    {
        EXPECT(!ShouldDoScottFortreeCall());
        EXPECT(!ShouldDoScottBattleFrontierCall());
    }

    FlagSet(HOENN_FLAG_ID(0x0D7));
    for (step = 1; step < 10; step++)
        EXPECT(!ShouldDoScottFortreeCall());
    EXPECT(ShouldDoScottFortreeCall());
    for (step = 1; step < 10; step++)
        EXPECT(!ShouldDoScottBattleFrontierCall());
    EXPECT(ShouldDoScottBattleFrontierCall());

    // Neither rings indoors.
    gMapHeader.mapType = MAP_TYPE_INDOOR;
    VarSet(HOENN_VAR_ID(0x40F3), 20);
    VarSet(HOENN_VAR_ID(0x40F5), 20);
    EXPECT(!ShouldDoScottFortreeCall());
    EXPECT(!ShouldDoScottBattleFrontierCall());

    // The invitation says he waits aboard the S.S. TIDAL: once met there, it never rings.
    gMapHeader.mapType = MAP_TYPE_ROUTE;
    VarSet(HOENN_VAR_ID(0x40D4), 1);
    for (step = 0; step < 20; step++)
        EXPECT(!ShouldDoScottBattleFrontierCall());
    gMapHeader = header;
}
