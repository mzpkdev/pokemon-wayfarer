#include "global.h"
#include "event_data.h"
#include "credits.h"
#include "heal_location.h"
#include "league_circuit.h"
#include "main.h"
#include "trainer_rating.h"
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

TEST("League circuit accepts mixed badges without badges in the host region")
{
    SetRegionalBadges(REGION_JOHTO, 4);
    SetRegionalBadges(REGION_HOENN, 4);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_KANTO);
    EXPECT(IsEligibleForLeague(REGION_KANTO));
    EXPECT(!IsEligibleForLeague(REGION_JOHTO));
    EXPECT(!IsEligibleForLeague(REGION_HOENN));
    EXPECT(TryRecordLeagueClear(REGION_KANTO));
    SetRegionalBadges(REGION_JOHTO, 0);
    SetRegionalBadges(REGION_KANTO, 8);
    SetRegionalBadges(REGION_HOENN, 8);
    EXPECT(IsEligibleForLeague(REGION_JOHTO));
    EXPECT(TryRecordLeagueClear(REGION_JOHTO));
    EXPECT(!IsEligibleForLeague(REGION_HOENN));
    SetRegionalBadges(REGION_JOHTO, 8);
    EXPECT(IsEligibleForLeague(REGION_HOENN));
    EXPECT(TryRecordLeagueClear(REGION_HOENN));
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_NONE);
    EXPECT_EQ(GetBadgeCertificationCap(), 24);
    EXPECT(!IsEligibleForLeague(REGION_NONE));
    EXPECT(!IsEligibleForLeague(REGION_HOENN));
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));
}

TEST("League circuit postpones each cap and permits retry after its required clear")
{
    enum Region region;
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT_EQ(GetRequiredLeagueRegion(), region);
        EXPECT_EQ(GetBadgeCertificationCap(), 8 * region);
        SetRegionalBadges(region, 7);
        EXPECT(!IsEligibleForLeague(region));
        EXPECT(!TryRecordLeagueClear(region));
        EXPECT(CanChallengeGymForBadge(region, 7));
        SetRegionalBadges(region, 8);
        EXPECT(CanChallengeGymForBadge(region, 7));
        if (region != REGION_HOENN)
            EXPECT(!CanChallengeGymForBadge(region + 1, 0));
        EXPECT(TryRecordLeagueClear(region));
        if (region != REGION_HOENN)
            EXPECT(CanChallengeGymForBadge(region + 1, 0));
    }
    EXPECT(!CanChallengeGymForBadge(REGION_NONE, 0));
    EXPECT(!CanChallengeGymForBadge(REGION_KANTO, 8));
}

TEST("League circuit fails safely with excess badges and out-of-order regional clears")
{
    SetRegionalBadges(REGION_KANTO, 8);
    SetRegionalBadges(REGION_JOHTO, 8);
    SetRegionalBadges(REGION_HOENN, 7);
    EXPECT(!CanChallengeGymForBadge(REGION_HOENN, 7));
    EXPECT(!TryRecordLeagueClear(REGION_JOHTO));
    EXPECT(!TryRecordLeagueClear(REGION_HOENN));
    SetChampionStateForRegion(REGION_JOHTO, TRUE);
    SetChampionStateForRegion(REGION_HOENN, TRUE);
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_KANTO);
    EXPECT_EQ(GetBadgeCertificationCap(), 8);
    EXPECT(!CanChallengeGymForBadge(REGION_HOENN, 7));
    EXPECT(TryRecordLeagueClear(REGION_KANTO));
    EXPECT_EQ(GetRequiredLeagueRegion(), REGION_NONE);
}

TEST("League circuit clear preserves every badge and unrelated regional campaign state")
{
    enum Region region, other;
    u8 badge;
    SetRegionalBadges(REGION_KANTO, 8);
    SetRegionalBadges(REGION_JOHTO, 8);
    SetRegionalBadges(REGION_HOENN, 8);
    FlagSet(HOENN_FLAG_ID(0x52));
    FlagSet(0x52);
    VarSet(HOENN_VAR_ID(0x4001), 1234);
    VarSet(0x4001, 5678);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT(TryRecordLeagueClear(region));
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

TEST("League circuit Hall of Fame handoff consumes the committed region once")
{
    ConsumeRecordedLeagueClearRegion();
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
    SetRegionalBadges(REGION_HOENN, 8);
    EXPECT(TryRecordLeagueClear(REGION_KANTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_KANTO);
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
    EXPECT(!TryRecordLeagueClear(REGION_JOHTO));
    EXPECT_EQ(ConsumeRecordedLeagueClearRegion(), REGION_NONE);
}

#if WAYFARER_LEAGUE_CIRCUIT_ENABLED
extern int GameClear(void);

TEST("League circuit Hall of Fame records the cleared tier despite shared venue provenance")
{
    MainCallback testCallback = gMain.callback2;
    enum Region region;
    const struct HealLocation *heal;

    SetRegionalBadges(REGION_KANTO, 8);
    SetRegionalBadges(REGION_JOHTO, 8);
    SetRegionalBadges(REGION_HOENN, 8);
    gSaveBlock1Ptr->location.mapGroup = MAP_GROUP(MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
    gSaveBlock1Ptr->location.mapNum = MAP_NUM(MAP_POKEMON_LEAGUE_HALL_OF_FAME_HNS);
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        EXPECT(TryRecordLeagueClear(region));
        GameClear();
        SetMainCallback2(testCallback);
        EXPECT(GetGameClearStateForRegion(region));
        if (region == REGION_KANTO)
            EXPECT(!GetChampionStateForRegion(REGION_JOHTO));
        if (region != REGION_HOENN)
            EXPECT(!GetChampionStateForRegion(REGION_HOENN));
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

TEST("League circuit Hall of Fame rejects unrecorded calls without regional writes")
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
