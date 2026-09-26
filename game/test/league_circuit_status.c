#include "global.h"
#include "league_circuit.h"
#include "league_run_helpers.h"
#include "league_circuit_status.h"
#include "string_util.h"
#include "wayfarer_persistence.h"
#include "test/test.h"

#if IS_WAYFARER
static void SetAllBadges(u8 count)
{
    u8 i;
    for (i = 0; i < 24; i++)
        SetBadgeStateForRegion(REGION_KANTO + i / 8, i % 8, i < count);
}

TEST("Trainer Card shows exact circuit rows and live first-clear states")
{
    static const u8 initial[] = _("Badges: 0/24\nIndigo League: Locked\n  8 badges\nSevii Masters: Locked\n  16 badges + Indigo clear\nHoenn League: Locked\n  24 badges + Masters clear");
    static const u8 allBadges[] = _("Badges: 24/24\nIndigo League: Available\n  8 badges\nSevii Masters: Locked\n  16 badges + Indigo clear\nHoenn League: Locked\n  24 badges + Masters clear");
    static const u8 indigoClear[] = _("Badges: 24/24\nIndigo League: Cleared\n  8 badges\nSevii Masters: Available\n  16 badges + Indigo clear\nHoenn League: Locked\n  24 badges + Masters clear");
    static const u8 mastersClear[] = _("Badges: 24/24\nIndigo League: Cleared\n  8 badges\nSevii Masters: Cleared\n  16 badges + Indigo clear\nHoenn League: Available\n  24 badges + Masters clear");
    static const u8 complete[] = _("Badges: 24/24\nIndigo League: Cleared\n  8 badges\nSevii Masters: Cleared\n  16 badges + Indigo clear\nHoenn League: Cleared\n  24 badges + Masters clear\nCircuit complete");
    u8 text[LEAGUE_CIRCUIT_STATUS_BUFFER_SIZE];

    WayfarerInitPersistentState();
    SetGameClearStateForRegion(REGION_KANTO, FALSE);
    SetGameClearStateForRegion(REGION_JOHTO, FALSE);
    SetAllBadges(0);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, initial), 0);
    SetAllBadges(24);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, allBadges), 0);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_INDIGO), CIRCUIT_COMMIT_FIRST_CLEAR);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, indigoClear), 0);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_MASTERS), CIRCUIT_COMMIT_FIRST_CLEAR);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, mastersClear), 0);
    EXPECT_EQ(Test_CompleteAndCommitCircuit(CIRCUIT_STAGE_HOENN), CIRCUIT_COMMIT_FIRST_CLEAR);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, complete), 0);
}
#endif
