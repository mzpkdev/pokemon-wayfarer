#include "global.h"
#include "credits.h"
#include "event_data.h"
#include "heal_location.h"
#include "league_circuit.h"
#include "main.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "config/league_circuit.h"
#include "constants/heal_locations.h"
#include "constants/maps.h"

#if IS_WAYFARER
static void SetRegionalBadges(enum Region region, u8 count)
{
    u8 i;

    for (i = 0; i < 8; i++)
        SetBadgeStateForRegion(region, i, i < count);
}

static void SetTotalBadges(u8 count)
{
    u8 i;

    for (i = 0; i < 24; i++)
        SetBadgeStateForRegion(REGION_KANTO + i / 8, i % 8, i < count);
}

TEST("League circuit aggregates all 729 regional badge distributions without duplicates")
{
    u8 kanto, johto, hoenn;

    for (kanto = 0; kanto <= 8; kanto++)
    {
        SetRegionalBadges(REGION_KANTO, kanto);
        for (johto = 0; johto <= 8; johto++)
        {
            SetRegionalBadges(REGION_JOHTO, johto);
            for (hoenn = 0; hoenn <= 8; hoenn++)
            {
                SetRegionalBadges(REGION_HOENN, hoenn);
                EXPECT_EQ(GetGlobalBadgeCount(), kanto + johto + hoenn);
                SetRegionalBadges(REGION_HOENN, hoenn);
                EXPECT_EQ(GetGlobalBadgeCount(), kanto + johto + hoenn);
            }
        }
    }
}

TEST("League circuit permits all 24 badges without any League clear")
{
    enum Region region;

    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        SetRegionalBadges(region, 8);
        EXPECT_EQ(GetGlobalBadgeCount(), 8 * region);
        EXPECT(!GetChampionStateForRegion(REGION_KANTO));
        EXPECT(!GetChampionStateForRegion(REGION_JOHTO));
        EXPECT(!GetChampionStateForRegion(REGION_HOENN));
    }

    EXPECT_EQ(GetGlobalBadgeCount(), 24);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_KANTO);
    EXPECT(IsEligibleForLeague(REGION_KANTO));
    EXPECT(!IsEligibleForLeague(REGION_JOHTO));
    EXPECT(!IsEligibleForLeague(REGION_HOENN));
}

TEST("League circuit checks exact badge thresholds and preceding clears")
{
    SetTotalBadges(7);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_KANTO), LEAGUE_ADMISSION_NEEDS_8_BADGES);
    EXPECT(!IsEligibleForLeague(REGION_KANTO));
    SetTotalBadges(8);
    EXPECT(IsEligibleForLeague(REGION_KANTO));

    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_JOHTO), LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR);
    SetGameClearStateForRegion(REGION_KANTO, TRUE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_JOHTO), LEAGUE_ADMISSION_NEEDS_16_BADGES);
    SetTotalBadges(15);
    EXPECT(!IsEligibleForLeague(REGION_JOHTO));
    SetTotalBadges(16);
    EXPECT(IsEligibleForLeague(REGION_JOHTO));

    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_HOENN), LEAGUE_ADMISSION_NEEDS_JOHTO_CLEAR);
    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_HOENN), LEAGUE_ADMISSION_NEEDS_24_BADGES);
    SetTotalBadges(23);
    EXPECT(!IsEligibleForLeague(REGION_HOENN));
    SetTotalBadges(24);
    EXPECT(IsEligibleForLeague(REGION_HOENN));
}

TEST("League circuit accepts mixed badges and exact thresholds without host badges")
{
    SetRegionalBadges(REGION_JOHTO, 4);
    SetRegionalBadges(REGION_HOENN, 4);
    EXPECT_EQ(GetGlobalBadgeCount(), 8);
    EXPECT(IsEligibleForLeague(REGION_KANTO));
    EXPECT(TryRecordLeagueClear(REGION_KANTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);

    SetRegionalBadges(REGION_JOHTO, 0);
    SetRegionalBadges(REGION_KANTO, 8);
    SetRegionalBadges(REGION_HOENN, 8);
    EXPECT_EQ(GetGlobalBadgeCount(), 16);
    EXPECT(IsEligibleForLeague(REGION_JOHTO));
    EXPECT(TryRecordLeagueClear(REGION_JOHTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_JOHTO);

    SetRegionalBadges(REGION_JOHTO, 8);
    EXPECT_EQ(GetGlobalBadgeCount(), 24);
    EXPECT(IsEligibleForLeague(REGION_HOENN));
    EXPECT(TryRecordLeagueClear(REGION_HOENN));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_HOENN);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_NONE);
}

TEST("League circuit clear order stays Kanto then Johto then Hoenn with all badges earned")
{
    enum Region region;

    SetTotalBadges(24);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT_EQ(GetRequiredLeagueRegion(), region);
        EXPECT(IsEligibleForLeague(region));
        if (region != REGION_KANTO)
            EXPECT(!IsEligibleForLeague(region - 1));
        if (region != REGION_HOENN)
            EXPECT(!IsEligibleForLeague(region + 1));
        EXPECT(TryRecordLeagueClear(region));
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), region);
    }
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_NONE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_NONE), LEAGUE_ADMISSION_UNAVAILABLE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_HOENN), LEAGUE_ADMISSION_UNAVAILABLE);
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));
}

TEST("League circuit rejects mixed out-of-order clear states")
{
    SetTotalBadges(24);
    EXPECT(!TryRecordLeagueClear(REGION_JOHTO));
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));

    SetGameClearStateForRegion(REGION_JOHTO, TRUE);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_KANTO);
    EXPECT(IsEligibleForLeague(REGION_KANTO));
    EXPECT(!IsEligibleForLeague(REGION_JOHTO));
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_JOHTO), LEAGUE_ADMISSION_UNAVAILABLE);
    EXPECT_EQ(GetLeagueAdmissionRequirement(REGION_HOENN), LEAGUE_ADMISSION_NEEDS_KANTO_CLEAR);

    EXPECT(TryRecordLeagueClear(REGION_KANTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_HOENN);
    EXPECT(IsEligibleForLeague(REGION_HOENN));
}

TEST("League circuit clear preserves every badge and unrelated regional state")
{
    enum Region region, other;
    u8 badge;

    SetTotalBadges(24);
    FlagSet(HOENN_FLAG_ID(0x52));
    FlagSet(0x52);
    VarSet(HOENN_VAR_ID(0x4001), 1234);
    VarSet(0x4001, 5678);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT(TryRecordLeagueClear(region));
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), region);
        EXPECT(GetGameClearStateForRegion(region));
        EXPECT(!TryRecordLeagueClear(region));
        for (other = REGION_KANTO; other <= REGION_HOENN; other++)
        {
            EXPECT_EQ(GetChampionStateForRegion(other), other <= region);
            EXPECT_EQ(GetGameClearStateForRegion(other), other <= region);
            for (badge = 0; badge < 8; badge++)
                EXPECT(GetBadgeStateForRegion(other, badge));
        }
        EXPECT(FlagGet(HOENN_FLAG_ID(0x52)));
        EXPECT(FlagGet(0x52));
        EXPECT_EQ(VarGet(HOENN_VAR_ID(0x4001)), 1234);
        EXPECT_EQ(VarGet(0x4001), 5678);
    }
}

TEST("League circuit clear handoff is one-shot and ignores rejected clears")
{
    ConsumeRecordedLeagueClearRegion();
    SetTotalBadges(24);
    EXPECT(TryRecordLeagueClear(REGION_KANTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
    EXPECT(TryRecordLeagueClear(REGION_JOHTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_JOHTO);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
}

#if WAYFARER_LEAGUE_CIRCUIT_ENABLED
extern int GameClear(void);

TEST("League circuit Hall of Fame consumes each recorded tier and sets its return warp")
{
    MainCallback testCallback = gMain.callback2;
    enum Region region;
    const struct HealLocation *heal;

    SetTotalBadges(24);
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT(TryRecordLeagueClear(region));
        GameClear();
        SetMainCallback2(testCallback);
        EXPECT(GetGameClearStateForRegion(region));
        EXPECT_EQ(GetGlobalBadgeCount(), 24);
        heal = GetHealLocation(region == REGION_HOENN
            ? HEAL_LOCATION_EVER_GRANDE_CITY_POKEMON_LEAGUE : HEAL_LOCATION_INDIGO_PLATEAU_HNS);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.mapGroup, heal->mapGroup);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.mapNum, heal->mapNum);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.x, heal->x);
        EXPECT_EQ(gSaveBlock1Ptr->continueGameWarp.y, heal->y);
        EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
    }
}

TEST("League circuit Hall of Fame rejects a missing clear handoff without state changes")
{
    MainCallback testCallback = gMain.callback2;

    ConsumeRecordedLeagueClearRegion();
    GameClear();
    EXPECT(gMain.callback2 == testCallback);
    EXPECT(!GetChampionStateForRegion(REGION_KANTO));
    EXPECT(!GetChampionStateForRegion(REGION_JOHTO));
    EXPECT(!GetChampionStateForRegion(REGION_HOENN));
}
#endif
#endif
