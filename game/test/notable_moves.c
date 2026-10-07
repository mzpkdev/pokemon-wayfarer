#include "global.h"
#include "data.h"
#include "notable_moves.h"
#include "test/test.h"
#include "constants/moves.h"
#include "constants/species.h"

#if IS_WAYFARER

TEST("Notable evolution uses authored nonlevel thresholds and game numeric thresholds")
{
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_STEELIX, 34), SPECIES_ONIX);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_STEELIX, 35), SPECIES_STEELIX);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_GOLEM, 12), SPECIES_GEODUDE);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_GOLEM, 25), SPECIES_GRAVELER);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_GOLEM, 38), SPECIES_GOLEM);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_PIKACHU, 1), SPECIES_PIKACHU);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_JYNX, 1), SPECIES_JYNX);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_ONIX, 100), SPECIES_ONIX);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_PORYGON_Z, 41), SPECIES_PORYGON);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_PORYGON_Z, 42), SPECIES_PORYGON2);
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_PORYGON_Z, 52), SPECIES_PORYGON_Z);
}

TEST("Notable pool claims natural moves and fills then replaces oldest unclaimed moves")
{
    struct TrainerMon member = {.species = SPECIES_ONIX, .lvl = 20};
    const struct NotableMovePoolEntry pool[] = {
        {MOVE_BIND, 0}, {MOVE_ROCK_TOMB, 0}, {MOVE_STEALTH_ROCK, 0},
        {MOVE_SANDSTORM, 20},
    };

    EXPECT(ResolveNotableTrainerMoves(&member, 1, 1, pool, ARRAY_COUNT(pool), FALSE));
    EXPECT_EQ(member.moves[0], MOVE_BIND);
    EXPECT_EQ(member.moves[1], MOVE_STEALTH_ROCK);
    EXPECT_EQ(member.moves[2], MOVE_ROCK_TOMB);
    EXPECT_EQ(member.moves[3], MOVE_SANDSTORM);
}

TEST("Notable pool routes one entry to an ace before an earlier filler")
{
    struct TrainerMon members[2] = {
        {.species = SPECIES_ONIX, .lvl = 20},
        {.species = SPECIES_ONIX, .lvl = 20},
    };
    const struct NotableMovePoolEntry pool[] = {{MOVE_BIND, 0}};

    EXPECT(ResolveNotableTrainerMoves(members, 2, 2, pool, ARRAY_COUNT(pool), FALSE));
    EXPECT_EQ(members[0].moves[0], MOVE_RAGE);
    EXPECT_EQ(members[1].moves[0], MOVE_BIND);
}

TEST("Notable low-count teams accept future ace bits in the full roster mask")
{
    struct TrainerMon members[2] = {
        {.species = SPECIES_ONIX, .lvl = 20},
        {.species = SPECIES_ONIX, .lvl = 20},
    };
    const struct NotableMovePoolEntry pool[] = {{MOVE_BIND, 0}};

    // Brock's six-slot mask has active slot 0 and a later, locked ace at slot 5.
    EXPECT(ResolveNotableTrainerMoves(members, 2, (1 << 0) | (1 << 5), pool, ARRAY_COUNT(pool), FALSE));
    EXPECT_EQ(members[0].moves[0], MOVE_BIND);
    EXPECT_EQ(members[1].moves[0], MOVE_RAGE);
    EXPECT(!ResolveNotableTrainerMoves(members, 2, 1 << PARTY_SIZE, pool, ARRAY_COUNT(pool), FALSE));
}

TEST("Notable pool respects from levels and skip option")
{
    struct TrainerMon member = {.species = SPECIES_ONIX, .lvl = 19};
    const struct NotableMovePoolEntry pool[] = {{MOVE_SANDSTORM, 20}};

    EXPECT(ResolveNotableTrainerMoves(&member, 1, 1, pool, ARRAY_COUNT(pool), FALSE));
    EXPECT_EQ(member.moves[0], MOVE_ROCK_TOMB);
    EXPECT_EQ(member.moves[3], MOVE_ROCK_POLISH);
    member.lvl = 20;
    EXPECT(ResolveNotableTrainerMoves(&member, 1, 1, pool, ARRAY_COUNT(pool), TRUE));
    EXPECT_EQ(member.moves[0], MOVE_RAGE);
    EXPECT_EQ(member.moves[3], MOVE_GYRO_BALL);
}

TEST("Notable pool recognizes earlier level moves and base egg moves")
{
    struct TrainerMon members[2] = {
        {.species = SPECIES_STARMIE, .lvl = 30},
        {.species = SPECIES_ONIX, .lvl = 20},
    };
    const struct NotableMovePoolEntry pool[] = {
        {MOVE_PSYWAVE, 0}, // Staryu learns this, but Starmie does not.
        {MOVE_FLAIL, 0},   // Onix can receive this only as an egg move.
        {MOVE_FLAIL, 20},
    };

    EXPECT(ResolveNotableTrainerMoves(members, 2, 1, pool, ARRAY_COUNT(pool), FALSE));
    EXPECT_EQ(members[0].moves[0], MOVE_PSYWAVE);
    EXPECT_EQ(members[1].moves[0], MOVE_FLAIL);
}

TEST("Notable egg moves include hatchling ancestry without scaling parties to babies")
{
    struct TrainerMon member = {.species = SPECIES_PIKACHU, .lvl = 20};
    const struct NotableMovePoolEntry pool[] = {{MOVE_WISH, 20}};

    EXPECT(ResolveNotableTrainerMoves(&member, 1, 1, pool, ARRAY_COUNT(pool), FALSE));
    EXPECT_EQ(member.species, SPECIES_PIKACHU);
    EXPECT_EQ(member.moves[0], MOVE_WISH); // Pichu's egg move.
    EXPECT_EQ(StepDownSpeciesToLevel(SPECIES_PIKACHU, 1), SPECIES_PIKACHU);
    member.species = SPECIES_RAICHU;
    EXPECT(ResolveNotableTrainerMoves(&member, 1, 1, pool, ARRAY_COUNT(pool), FALSE));
    EXPECT_EQ(member.moves[0], MOVE_WISH);
}

TEST("Notable baby egg moves still require their authored from level")
{
    struct TrainerMon member = {.species = SPECIES_PIKACHU, .lvl = 19};
    const struct NotableMovePoolEntry pool[] = {{MOVE_WISH, 0}, {MOVE_WISH, 20}};
    u32 i;

    EXPECT(ResolveNotableTrainerMoves(&member, 1, 1, pool, ARRAY_COUNT(pool), FALSE));
    for (i = 0; i < MAX_MON_MOVES; i++)
        EXPECT_NE(member.moves[i], MOVE_WISH);
    member.lvl = 20;
    EXPECT(ResolveNotableTrainerMoves(&member, 1, 1, pool, ARRAY_COUNT(pool), FALSE));
    EXPECT_EQ(member.moves[0], MOVE_WISH);
}

#endif
