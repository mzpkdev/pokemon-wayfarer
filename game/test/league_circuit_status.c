#include "global.h"
#include "league_circuit.h"
#include "league_run_helpers.h"
#include "league_circuit_status.h"
#include "string_util.h"
#include "wayfarer_persistence.h"
#include "test/test.h"

#if IS_WAYFARER
static const enum Region sRegions[] = { REGION_KANTO, REGION_JOHTO, REGION_HOENN };

static void ResetCircuitFacts(void)
{
    u8 region;
    u8 badge;

    ConsumeRecordedLeagueClearRegion();
    WayfarerInitPersistentState();
    for (region = 0; region < ARRAY_COUNT(sRegions); region++)
    {
        SetChampionStateForRegion(sRegions[region], FALSE);
        for (badge = 0; badge < 8; badge++)
            SetBadgeStateForRegion(sRegions[region], badge, FALSE);
    }
}

static void SetBadgeCount(enum Region region, u8 count)
{
    u8 badge;

    for (badge = 0; badge < 8; badge++)
        SetBadgeStateForRegion(region, badge, badge < count);
}

static void SetBadgeDistribution(u8 kanto, u8 johto, u8 hoenn)
{
    SetBadgeCount(REGION_KANTO, kanto);
    SetBadgeCount(REGION_JOHTO, johto);
    SetBadgeCount(REGION_HOENN, hoenn);
}

TEST("League circuit status reports mixed badge thresholds and prior clear requirements")
{
    static const u8 initial[] = _("Badges: 0/24\nKanto: Locked\n  8 badges + no prior clear\nJohto: Locked\n  16 badges + Kanto clear\nHoenn: Locked\n  24 badges + Kanto + Johto clears");
    static const u8 mixedEight[] = _("Badges: 8/24\nKanto: Available\n  8 badges + no prior clear\nJohto: Locked\n  16 badges + Kanto clear\nHoenn: Locked\n  24 badges + Kanto + Johto clears");
    static const u8 fifteen[] = _("Badges: 15/24\nKanto: Available\n  8 badges + no prior clear\nJohto: Locked\n  16 badges + Kanto clear\nHoenn: Locked\n  24 badges + Kanto + Johto clears");
    static const u8 sixteenWithoutClear[] = _("Badges: 16/24\nKanto: Available\n  8 badges + no prior clear\nJohto: Locked\n  16 badges + Kanto clear\nHoenn: Locked\n  24 badges + Kanto + Johto clears");
    static const u8 sixteenWithKantoClear[] = _("Badges: 16/24\nKanto: Cleared\n  8 badges + no prior clear\nJohto: Available\n  16 badges + Kanto clear\nHoenn: Locked\n  24 badges + Kanto + Johto clears");
    u8 text[LEAGUE_CIRCUIT_STATUS_BUFFER_SIZE];

    ResetCircuitFacts();
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, initial), 0);

    SetBadgeDistribution(0, 4, 4);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, mixedEight), 0);

    SetBadgeDistribution(7, 4, 4);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, fifteen), 0);

    SetBadgeDistribution(8, 4, 4);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, sixteenWithoutClear), 0);

    EXPECT(Test_CompleteAndRecordLeague(REGION_KANTO));
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, sixteenWithKantoClear), 0);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);
}

TEST("League circuit status reports all badges before consecutive League clears")
{
    static const u8 allBadgesNoClears[] = _("Badges: 24/24\nKanto: Available\n  8 badges + no prior clear\nJohto: Locked\n  16 badges + Kanto clear\nHoenn: Locked\n  24 badges + Kanto + Johto clears");
    static const u8 kantoCleared[] = _("Badges: 24/24\nKanto: Cleared\n  8 badges + no prior clear\nJohto: Available\n  16 badges + Kanto clear\nHoenn: Locked\n  24 badges + Kanto + Johto clears");
    static const u8 johtoCleared[] = _("Badges: 24/24\nKanto: Cleared\n  8 badges + no prior clear\nJohto: Cleared\n  16 badges + Kanto clear\nHoenn: Available\n  24 badges + Kanto + Johto clears");
    static const u8 complete[] = _("Badges: 24/24\nKanto: Cleared\n  8 badges + no prior clear\nJohto: Cleared\n  16 badges + Kanto clear\nHoenn: Cleared\n  24 badges + Kanto + Johto clears\nCircuit complete");
    u8 text[LEAGUE_CIRCUIT_STATUS_BUFFER_SIZE];

    ResetCircuitFacts();
    SetBadgeDistribution(8, 8, 8);
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, allBadgesNoClears), 0);

    EXPECT(Test_CompleteAndRecordLeague(REGION_KANTO));
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, kantoCleared), 0);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);

    EXPECT(Test_CompleteAndRecordLeague(REGION_JOHTO));
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, johtoCleared), 0);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_JOHTO);

    EXPECT(Test_CompleteAndRecordLeague(REGION_HOENN));
    FormatLeagueCircuitStatus(text);
    EXPECT_EQ(StringCompare(text, complete), 0);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_HOENN);
}
#endif
