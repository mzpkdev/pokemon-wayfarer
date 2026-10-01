#include "global.h"
#include "test/test.h"
#include "notable_ai.h"
#include "notable_trainers.h"
#include "constants/battle_ai.h"

TEST("Notable AI adds skill tiers at trainer rating boundaries")
{
    struct NotableTrainer trainer = {.playStyle = NOTABLE_STYLE_TACTICIAN};
    struct NotableTrainerSnapshot snapshot = {.trainer = &trainer, .aceCount = 1};
    static const u32 ratings[] = {0, 29, 30, 69, 70, 109, 110, 100000};
    u32 i;
    for (i = 0; i < ARRAY_COUNT(ratings); i++)
    {
        u64 flags;
        snapshot.trainerTR = ratings[i];
        flags = ResolveNotableTrainerAi(&snapshot, FALSE, FALSE);
        EXPECT_EQ(flags & AI_FLAG_BASIC_TRAINER, AI_FLAG_BASIC_TRAINER);
        EXPECT_EQ((flags & AI_FLAG_ASSUME_STAB) != 0, ratings[i] >= 30);
        EXPECT_EQ((flags & AI_FLAG_SMART_SWITCHING) != 0, ratings[i] >= 70);
        EXPECT_EQ((flags & AI_FLAG_PREDICT_MOVE) != 0, ratings[i] >= 110);
        EXPECT(flags & AI_FLAG_ACE_POKEMON);
        EXPECT(!(flags & AI_FLAG_DOUBLE_ACE_POKEMON));
    }
}

TEST("Notable AI preserves styles, double battles and randomized ace precedence")
{
    struct NotableTrainer trainer = {.playStyle = NOTABLE_STYLE_BOMBER, .bossOmniscient = TRUE};
    struct NotableTrainerSnapshot snapshot = {.trainer = &trainer, .trainerTR = 200, .aceCount = 2};
    u64 flags = ResolveNotableTrainerAi(&snapshot, FALSE, TRUE);
    EXPECT(flags & AI_FLAG_RISKY);
    EXPECT(flags & AI_FLAG_WILL_SUICIDE);
    EXPECT(flags & AI_FLAG_OMNISCIENT);
    EXPECT(flags & AI_FLAG_DOUBLE_BATTLE);
    EXPECT(flags & AI_FLAG_DOUBLE_ACE_POKEMON);
    EXPECT(!(flags & AI_FLAG_ACE_POKEMON));
    flags = ResolveNotableTrainerAi(&snapshot, TRUE, TRUE);
    EXPECT(!(flags & (AI_FLAG_ACE_POKEMON | AI_FLAG_DOUBLE_ACE_POKEMON)));
    EXPECT(flags & AI_FLAG_DOUBLE_BATTLE);
    EXPECT(flags & AI_FLAG_PREDICT_MOVE);
    trainer.playStyle = NOTABLE_STYLE_TURTLE;
    flags = ResolveNotableTrainerAi(&snapshot, FALSE, FALSE);
    EXPECT(flags & AI_FLAG_CONSERVATIVE);
    EXPECT(!(flags & AI_FLAG_RISKY));
    snapshot.aceCount = 0;
    EXPECT_EQ(ResolveNotableTrainerAi(&snapshot, FALSE, FALSE), 0);
    EXPECT_EQ(ResolveNotableTrainerAi(NULL, FALSE, FALSE), 0);
}
