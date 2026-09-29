#include "global.h"
#include "config/league_circuit.h"
#include "league_circuit.h"
#include "league_run_helpers.h"
#include "league_circuit_status.h"
#include "string_util.h"
#include "trainer_rating.h"
#include "wayfarer_persistence.h"
#include "test/test.h"

#if IS_WAYFARER
static void SetAllBadges(u8 count)
{
    u8 i;
    for (i = 0; i < 24; i++)
        SetBadgeStateForRegion(REGION_KANTO + i / 8, i % 8, i < count);
}

TEST("Trainer Card shows TR and regional gates with independent league wins")
{
    static const u8 initial[] = _("Badges: 0/24\nIndigo League: Locked\n  TR 80 + Kanto/Johto badge\nSevii Masters: Locked\n  TR 80 + both league clears\nHoenn League: Locked\n  TR 80 + Hoenn badge");
    static const u8 allBadges[] = _("Badges: 24/24\nIndigo League: Available\n  TR 80 + Kanto/Johto badge\nSevii Masters: Locked\n  TR 80 + both league clears\nHoenn League: Available\n  TR 80 + Hoenn badge");
    static const u8 indigoClear[] = _("Badges: 24/24\nIndigo League: Cleared\n  TR 80 + Kanto/Johto badge\nSevii Masters: Locked\n  TR 80 + both league clears\nHoenn League: Available\n  TR 80 + Hoenn badge");
    static const u8 mastersAvailable[] = _("Badges: 24/24\nIndigo League: Cleared\n  TR 80 + Kanto/Johto badge\nSevii Masters: Available\n  TR 80 + both league clears\nHoenn League: Cleared\n  TR 80 + Hoenn badge");
    static const u8 complete[] = _("Badges: 24/24\nIndigo League: Cleared\n  TR 80 + Kanto/Johto badge\nSevii Masters: Cleared\n  TR 80 + both league clears\nHoenn League: Cleared\n  TR 80 + Hoenn badge\nCircuit complete");
#if WAYFARER_LEAGUE_EVENTS
    static const u8 accepted[] = _("Badges: 24/24\nIndigo League: Cleared\n  TR 80 + Kanto/Johto badge\nSevii Masters: Accepted\n  TR 80 + both league clears\nHoenn League: Cleared\n  TR 80 + Hoenn badge\nCircuit complete");
#endif
    u8 text[LEAGUE_CIRCUIT_STATUS_BUFFER_SIZE];

    WayfarerInitPersistentState();
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    SetAllBadges(0);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, initial), 0);
    SetTrainerRating(80);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, initial), 0);
    SetAllBadges(24);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, allBadges), 0);
    SetCircuitClearForTesting(CIRCUIT_STAGE_INDIGO, TRUE);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, indigoClear), 0);
    SetCircuitClearForTesting(CIRCUIT_STAGE_HOENN, TRUE);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, mastersAvailable), 0);
    SetCircuitClearForTesting(CIRCUIT_STAGE_MASTERS, TRUE);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, complete), 0);
#if WAYFARER_LEAGUE_EVENTS
    EXPECT(Test_AdmitCircuitRun(CIRCUIT_STAGE_MASTERS));
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, accepted), 0);
#endif
}
#endif
