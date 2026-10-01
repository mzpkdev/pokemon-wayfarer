#include "global.h"
#include "config/notable_trainers.h"
#include "league_selection.h"
#include "notable_trainers.h"
#include "test/test.h"
#include "constants/notable_trainers.h"
#include "constants/wayfarer_indigo_trainers.h"

#if IS_WAYFARER && WAYFARER_V0_TRAINERS

static const u32 sNoRecentLeagueLineup[LEAGUE_LINEUP_SIZE] = {0};

TEST("League registry keeps selected people tied to their own presentation and source")
{
    u32 i, eligible = 0;
    EXPECT(IsLeagueTrainerRegistryValid());
    EXPECT_EQ(GetLeagueTrainerCount(), NOTABLE_TRAINER_COUNT);
    for (i = 1; i <= GetLeagueTrainerCount(); i++)
    {
        const struct LeagueTrainer *entry = GetLeagueTrainer(i);
        EXPECT_EQ(entry->characterId, i);
        EXPECT_EQ(entry->presentationId, i);
        EXPECT(GetNotableTrainerForEncounter(entry->sourceTrainerId) == GetNotableTrainerById(i));
        if (entry->enabled)
            eligible++;
    }
    EXPECT_EQ(eligible, 37);
    EXPECT_EQ(GetLeagueTrainer(NOTABLE_TRAINER_TATE_LIZA)->enabled, FALSE);
    EXPECT_EQ(GetLeagueTrainer(NOTABLE_TRAINER_LORELEI)->sourceTrainerId, TRAINER_WAYFARER_INDIGO_LORELEI);
    EXPECT(GetLeagueTrainer(0) == NULL);
    EXPECT(GetLeagueTrainer(39) == NULL);
    EXPECT(GetLeagueTrainerByIndex(38) == NULL);
}

TEST("Indigo lineup uses travel, aloof cutoff, score ties, and ascending battle TR")
{
    struct LeagueLineupSelection lineup;
    static const u32 earlyExpected[] = {NOTABLE_TRAINER_KOGA, NOTABLE_TRAINER_BRUNO,
        NOTABLE_TRAINER_KAREN, NOTABLE_TRAINER_WALLACE, NOTABLE_TRAINER_STEVEN};
    static const u32 lateExpected[] = {NOTABLE_TRAINER_MORTY, NOTABLE_TRAINER_SABRINA,
        NOTABLE_TRAINER_CLAIR, NOTABLE_TRAINER_STEVEN, NOTABLE_TRAINER_LANCE};
    u32 i;

    EXPECT(SelectLeagueLineup(LEAGUE_ID_INDIGO, 0, sNoRecentLeagueLineup, 0, 0, &lineup));
    EXPECT_EQ(lineup.count, LEAGUE_LINEUP_SIZE);
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
        EXPECT_EQ(lineup.members[i].characterId, earlyExpected[i]);
    EXPECT_EQ(lineup.members[3].willingness, 90); // Wallace travels from Hoenn.
    EXPECT_EQ(lineup.members[4].leagueScore, 45);

    EXPECT(SelectLeagueLineup(LEAGUE_ID_INDIGO, 160, sNoRecentLeagueLineup, 0, 0, &lineup));
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
    {
        EXPECT_EQ(lineup.members[i].characterId, lateExpected[i]);
        if (i != 0)
            EXPECT(lineup.members[i - 1].trainerTR <= lineup.members[i].trainerTR);
    }
}

TEST("Most recently resolved lineup applies fatigue before scoring")
{
    static const u32 recent[] = {NOTABLE_TRAINER_MORTY, NOTABLE_TRAINER_SABRINA,
        NOTABLE_TRAINER_CLAIR, NOTABLE_TRAINER_STEVEN, NOTABLE_TRAINER_LANCE};
    static const u32 expected[] = {NOTABLE_TRAINER_KAREN, NOTABLE_TRAINER_GIOVANNI,
        NOTABLE_TRAINER_JASMINE, NOTABLE_TRAINER_BLUE, NOTABLE_TRAINER_WALLACE};
    struct LeagueLineupSelection lineup;
    u32 i;
    EXPECT(SelectLeagueLineup(LEAGUE_ID_INDIGO, 160, recent, 0, 0, &lineup));
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
        EXPECT_EQ(lineup.members[i].characterId, expected[i]);
}

TEST("Hoenn selects local entrants independently of Indigo's location")
{
    struct LeagueLineupSelection lineup;
    static const u32 expected[] = {NOTABLE_TRAINER_PHOEBE, NOTABLE_TRAINER_GLACIA,
        NOTABLE_TRAINER_DRAKE, NOTABLE_TRAINER_WALLACE, NOTABLE_TRAINER_STEVEN};
    u32 i;
    EXPECT(SelectLeagueLineup(LEAGUE_ID_HOENN, 0, sNoRecentLeagueLineup, 0, 0, &lineup));
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
        EXPECT_EQ(lineup.members[i].characterId, expected[i]);
    EXPECT_EQ(lineup.members[3].willingness, 100); // Wallace is home in Hoenn.
}

TEST("Masters guarantees seats to notable trainers with both reign flags")
{
    struct LeagueLineupSelection lineup;
    static const u32 recent[] = {NOTABLE_TRAINER_BROCK, NOTABLE_TRAINER_MISTY,
        NOTABLE_TRAINER_LT_SURGE, NOTABLE_TRAINER_ERIKA, NOTABLE_TRAINER_JANINE};
    static const u32 expected[] = {NOTABLE_TRAINER_LT_SURGE, NOTABLE_TRAINER_BROCK,
        NOTABLE_TRAINER_JANINE, NOTABLE_TRAINER_MISTY, NOTABLE_TRAINER_ERIKA};
    u64 masters = (1ULL << (NOTABLE_TRAINER_BROCK - 1))
                | (1ULL << (NOTABLE_TRAINER_MISTY - 1))
                | (1ULL << (NOTABLE_TRAINER_LT_SURGE - 1))
                | (1ULL << (NOTABLE_TRAINER_ERIKA - 1))
                | (1ULL << (NOTABLE_TRAINER_JANINE - 1));
    u32 i;
    EXPECT(SelectLeagueLineup(LEAGUE_ID_MASTERS, 160, sNoRecentLeagueLineup, masters, masters, &lineup));
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
        EXPECT_EQ(lineup.members[i].characterId, expected[i]);
    EXPECT(SelectLeagueLineup(LEAGUE_ID_MASTERS, 160, recent, masters, masters, &lineup));
    for (i = 0; i < LEAGUE_LINEUP_SIZE; i++)
        EXPECT_EQ(lineup.members[i].characterId, expected[i]); // Fatigue cannot remove a guaranteed seat.
    EXPECT(SelectLeagueLineup(LEAGUE_ID_MASTERS, 0, sNoRecentLeagueLineup, 0, 0, &lineup));
    EXPECT_EQ(lineup.members[4].characterId, NOTABLE_TRAINER_LANCE); // Aloof rule is off here.
}

TEST("League selection rejects malformed history without a partial result")
{
    static const u32 partialRecent[] = {NOTABLE_TRAINER_BROCK, 0, 0, 0, 0};
    static const u32 duplicateRecent[] = {1, 2, 3, 4, 4};
    static const u32 ineligibleRecent[] = {1, 2, 3, 4, NOTABLE_TRAINER_TATE_LIZA};
    struct LeagueLineupSelection lineup;
    EXPECT(!SelectLeagueLineup(LEAGUE_ID_NONE, 80, sNoRecentLeagueLineup, 0, 0, &lineup));
    EXPECT_EQ(lineup.count, 0);
    EXPECT(!SelectLeagueLineup(LEAGUE_ID_HOENN, 80, partialRecent, 0, 0, &lineup));
    EXPECT(!SelectLeagueLineup(LEAGUE_ID_HOENN, 80, duplicateRecent, 0, 0, &lineup));
    EXPECT(!SelectLeagueLineup(LEAGUE_ID_HOENN, 80, ineligibleRecent, 0, 0, &lineup));
    EXPECT(!SelectLeagueLineup(LEAGUE_ID_HOENN, 80, sNoRecentLeagueLineup, 1ULL << 38, 0, &lineup));
    EXPECT_EQ(lineup.count, 0);
}

#endif // IS_WAYFARER && WAYFARER_V0_TRAINERS
