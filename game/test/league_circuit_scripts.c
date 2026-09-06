#include "global.h"
#include "event_data.h"
#include "league_circuit.h"
#include "script.h"
#include "wayfarer_persistence.h"
#include "test/test.h"
#include "config/league_circuit.h"

#if IS_WAYFARER && WAYFARER_LEAGUE_CIRCUIT_ENABLED
extern const u8 LeagueCircuit_EventScript_PostponeGym[];
extern const u8 PewterCity_Gym_EventScript_Brock[];
extern const u8 CeruleanCity_Gym_EventScript_Misty[];
extern const u8 VermilionCity_Gym_EventScript_Surge[];
extern const u8 CeladonCity_Gym_EventScript_Erika[];
extern const u8 SaffronCity_Gym_EventScript_Sabrina[];
extern const u8 FuchsiaCity_Gym_EventScript_Janine[];
extern const u8 SeafoamIslands_Gym_EventScript_Blaine[];
extern const u8 ViridianCity_Gym_EventScript_Blue[];
extern const u8 VioletCity_Gym_EventScript_Falkner[];
extern const u8 AzaleaTown_Gym_EventScript_Bugsy[];
extern const u8 GoldenrodCity_Gym_EventScript_Whitney[];
extern const u8 EcruteakCity_Gym_EventScript_Morty[];
extern const u8 CianwoodGym_EventScript_Chuck[];
extern const u8 OlivineCity_Gym_EventScript_Jasmine[];
extern const u8 MahoganyTown_Gym_EventScript_Pryce[];
extern const u8 BlackthornGym_EventScript_Clair[];
extern const u8 RustboroCity_Gym_EventScript_Roxanne[];
extern const u8 DewfordTown_Gym_EventScript_Brawly[];
extern const u8 MauvilleCity_Gym_EventScript_Wattson[];
extern const u8 LavaridgeTown_Gym_1F_EventScript_Flannery[];
extern const u8 PetalburgCity_Gym_EventScript_NormanBattle[];
extern const u8 FortreeCity_Gym_EventScript_Winona[];
extern const u8 MossdeepCity_Gym_EventScript_TateAndLiza[];
extern const u8 SootopolisCity_Gym_1F_EventScript_Juan[];
extern const u8 GoldenrodCity_Gym_EventScript_WhitneyBadge[];
extern const u8 DragonsDen_Shrine_EventScript_ElderOfferQuiz[];
extern u16 LeagueCircuit_CanChallengeGym(void);

static const struct
{
    const u8 *script;
    enum Region region;
    u8 badge;
} sGymEntries[] =
{
    { PewterCity_Gym_EventScript_Brock, REGION_KANTO, 0 },
    { CeruleanCity_Gym_EventScript_Misty, REGION_KANTO, 1 },
    { VermilionCity_Gym_EventScript_Surge, REGION_KANTO, 2 },
    { CeladonCity_Gym_EventScript_Erika, REGION_KANTO, 3 },
    { SaffronCity_Gym_EventScript_Sabrina, REGION_KANTO, 4 },
    { FuchsiaCity_Gym_EventScript_Janine, REGION_KANTO, 5 },
    { SeafoamIslands_Gym_EventScript_Blaine, REGION_KANTO, 6 },
    { ViridianCity_Gym_EventScript_Blue, REGION_KANTO, 7 },
    { VioletCity_Gym_EventScript_Falkner, REGION_JOHTO, 0 },
    { AzaleaTown_Gym_EventScript_Bugsy, REGION_JOHTO, 1 },
    { GoldenrodCity_Gym_EventScript_Whitney, REGION_JOHTO, 2 },
    { EcruteakCity_Gym_EventScript_Morty, REGION_JOHTO, 3 },
    { CianwoodGym_EventScript_Chuck, REGION_JOHTO, 4 },
    { OlivineCity_Gym_EventScript_Jasmine, REGION_JOHTO, 5 },
    { MahoganyTown_Gym_EventScript_Pryce, REGION_JOHTO, 6 },
    { BlackthornGym_EventScript_Clair, REGION_JOHTO, 7 },
    { RustboroCity_Gym_EventScript_Roxanne, REGION_HOENN, 0 },
    { DewfordTown_Gym_EventScript_Brawly, REGION_HOENN, 1 },
    { MauvilleCity_Gym_EventScript_Wattson, REGION_HOENN, 2 },
    { LavaridgeTown_Gym_1F_EventScript_Flannery, REGION_HOENN, 3 },
    { PetalburgCity_Gym_EventScript_NormanBattle, REGION_HOENN, 4 },
    { FortreeCity_Gym_EventScript_Winona, REGION_HOENN, 5 },
    { MossdeepCity_Gym_EventScript_TateAndLiza, REGION_HOENN, 6 },
    { SootopolisCity_Gym_1F_EventScript_Juan, REGION_HOENN, 7 },
    { GoldenrodCity_Gym_EventScript_WhitneyBadge, REGION_JOHTO, 2 },
    { DragonsDen_Shrine_EventScript_ElderOfferQuiz, REGION_JOHTO, 7 },
};

static void SeedBadgesExcept(u8 count, enum Region omittedRegion, u8 omittedBadge)
{
    enum Region region;
    u8 badge;
    for (region = REGION_KANTO; region <= REGION_HOENN; region++)
    {
        for (badge = 0; badge < 8; badge++)
        {
            bool8 earned = count != 0 && (region != omittedRegion || badge != omittedBadge);
            SetBadgeStateForRegion(region, badge, earned);
            if (earned)
                count--;
        }
    }
}

TEST("League circuit real Gym scripts postpone all 24 badges before any save or battle effect")
{
    struct ScriptContext ctx;
    enum Region required;
    u8 entry;
    for (required = REGION_KANTO; required <= REGION_JOHTO; required++)
    {
        for (entry = 0; entry < ARRAY_COUNT(sGymEntries); entry++)
        {
            SetGameClearStateForRegion(REGION_KANTO, required == REGION_JOHTO);
            SetGameClearStateForRegion(REGION_JOHTO, FALSE);
            SetGameClearStateForRegion(REGION_HOENN, FALSE);
            SeedBadgesExcept(required == REGION_KANTO ? 8 : 16, sGymEntries[entry].region, sGymEntries[entry].badge);
            // Stop at specialvar. The special is explicitly instrumented, so
            // the script has already supplied its region and badge arguments
            // without reaching any Gym battle or save-affecting command.
            EXPECT(RunScriptImmediatelyUntilEffect(SCREFF_V1 | SCREFF_ANY, sGymEntries[entry].script, &ctx));
            EXPECT_EQ(gSpecialVar_0x8004, sGymEntries[entry].region);
            EXPECT_EQ(gSpecialVar_0x8005, sGymEntries[entry].badge);
            EXPECT_EQ(LeagueCircuit_CanChallengeGym(), FALSE);
            EXPECT_EQ(GetGlobalBadgeCount(), required == REGION_KANTO ? 8 : 16);
            EXPECT(!GetBadgeStateForRegion(sGymEntries[entry].region, sGymEntries[entry].badge));
            EXPECT(TryRecordLeagueClear(required));
            EXPECT(RunScriptImmediatelyUntilEffect(SCREFF_V1 | SCREFF_ANY, sGymEntries[entry].script, &ctx));
            EXPECT_EQ(LeagueCircuit_CanChallengeGym(), TRUE);
        }
    }
}

TEST("League circuit real Gym scripts admit the final badge below each cap and already earned badges")
{
    struct ScriptContext ctx;
    u8 tier, entry;
    for (tier = 1; tier <= 3; tier++)
    {
        SetGameClearStateForRegion(REGION_KANTO, tier >= 2);
        SetGameClearStateForRegion(REGION_JOHTO, tier >= 3);
        SetGameClearStateForRegion(REGION_HOENN, FALSE);
        for (entry = 0; entry < ARRAY_COUNT(sGymEntries); entry++)
        {
            SeedBadgesExcept(8 * tier - 1, sGymEntries[entry].region, sGymEntries[entry].badge);
            EXPECT(RunScriptImmediatelyUntilEffect(SCREFF_V1 | SCREFF_ANY, sGymEntries[entry].script, &ctx));
            EXPECT_EQ(LeagueCircuit_CanChallengeGym(), TRUE);
            SetBadgeStateForRegion(sGymEntries[entry].region, sGymEntries[entry].badge, TRUE);
            EXPECT_EQ(GetGlobalBadgeCount(), 8 * tier);
            EXPECT(RunScriptImmediatelyUntilEffect(SCREFF_V1 | SCREFF_ANY, sGymEntries[entry].script, &ctx));
            EXPECT_EQ(LeagueCircuit_CanChallengeGym(), TRUE);
        }
    }
}
#endif
