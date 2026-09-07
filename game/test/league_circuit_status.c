#include "global.h"
#include "league_circuit.h"
#include "league_circuit_status.h"
#include "string_util.h"
#include "wayfarer_persistence.h"
#include "test/test.h"

#if IS_WAYFARER
TEST("League circuit status reports every target and qualification then completion")
{
    static const enum Region regions[] = { REGION_KANTO, REGION_JOHTO, REGION_HOENN };
    static const u8 complete[] = _("Badges: 24/24\nCertification cap: 24\nCircuit complete");
    static const u8 completePaged[] = _("Badges: 24/24\nCertification cap: 24\pCircuit complete");
    static const u8 waitingKanto[] = _("Badges: 0/24\nCertification cap: 8\nNext: Kanto League\nBadge target: 8");
    static const u8 waitingJohto[] = _("Badges: 8/24\nCertification cap: 16\nNext: Johto League\nBadge target: 16");
    static const u8 waitingHoenn[] = _("Badges: 16/24\nCertification cap: 24\nNext: Hoenn League\nBadge target: 24");
    static const u8 qualifiedKanto[] = _("Badges: 8/24\nCertification cap: 8\nNext: Kanto League\nQualified!");
    static const u8 qualifiedJohto[] = _("Badges: 16/24\nCertification cap: 16\nNext: Johto League\nQualified!");
    static const u8 qualifiedHoenn[] = _("Badges: 24/24\nCertification cap: 24\nNext: Hoenn League\nQualified!");
    static const u8 *const waiting[] =
    {
        waitingKanto,
        waitingJohto,
        waitingHoenn,
    };
    static const u8 *const qualified[] =
    {
        qualifiedKanto,
        qualifiedJohto,
        qualifiedHoenn,
    };
    u8 text[LEAGUE_CIRCUIT_STATUS_BUFFER_SIZE];
    u8 region;
    u8 badge;

    WayfarerInitPersistentState();
    for (region = 0; region < ARRAY_COUNT(regions); region++)
    {
        SetChampionStateForRegion(regions[region], FALSE);
        for (badge = 0; badge < 8; badge++)
            SetBadgeStateForRegion(regions[region], badge, FALSE);
    }
    for (region = 0; region < ARRAY_COUNT(regions); region++)
    {
        FormatLeagueCircuitStatus(text, FALSE);
        EXPECT_EQ(StringCompare(text, waiting[region]), 0);
        for (badge = 0; badge < 8; badge++)
            SetBadgeStateForRegion(regions[region], badge, TRUE);
        FormatLeagueCircuitStatus(text, FALSE);
        EXPECT_EQ(StringCompare(text, qualified[region]), 0);
        EXPECT(TryRecordLeagueClear(regions[region]));
    }
    FormatLeagueCircuitStatus(text, FALSE);
    EXPECT_EQ(StringCompare(text, complete), 0);
    BufferLeagueCircuitStatus();
    EXPECT_EQ(StringCompare(gStringVar4, completePaged), 0);
}
#endif
