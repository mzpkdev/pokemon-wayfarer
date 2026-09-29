#include "global.h"
#include "notable_trainers.h"
#include "test/test.h"
#include "constants/notable_trainers.h"
#include "constants/opponents.h"
#include "constants/opponents_hns.h"
#include "constants/wayfarer_indigo_trainers.h"
#include "constants/wayfarer_kanto_trainers.h"
#include "constants/wayfarer_viridian_trainers.h"
#include "constants/species.h"
#include "constants/moves.h"

#if IS_WAYFARER

TEST("Notable trainer catalog has 38 valid identities and explicit encounter aliases")
{
    u32 i;
    EXPECT(IsNotableTrainerCatalogValid());
    for (i = 1; i <= NOTABLE_TRAINER_COUNT; i++)
        EXPECT_EQ(GetNotableTrainerById(i)->characterId, i);
    EXPECT(GetNotableTrainerById(0) == NULL);
    EXPECT(GetNotableTrainerById(39) == NULL);
    EXPECT_EQ(GetNotableTrainerForEncounter(TRAINER_VIRIDIAN_GYM_GIOVANNI_HNS)->characterId, NOTABLE_TRAINER_GIOVANNI);
    EXPECT_EQ(GetNotableTrainerForEncounter(TRAINER_WAYFARER_KANTO_BLUE_BULBASAUR)->characterId, NOTABLE_TRAINER_BLUE);
    EXPECT_EQ(GetNotableTrainerForEncounter(TRAINER_WAYFARER_INDIGO_BLUE)->characterId, NOTABLE_TRAINER_BLUE);
    EXPECT_EQ(GetNotableTrainerForEncounter(TRAINER_TATE_AND_LIZA_1)->characterId, NOTABLE_TRAINER_TATE_LIZA);
    EXPECT(GetNotableTrainerForEncounter(TRAINER_BLUE_HNS) == NULL);
    EXPECT(GetNotableTrainerForEncounter(TRAINER_NONE) == NULL);
}

TEST("Notable growth stays deterministic and respects archetype endpoints")
{
    const struct NotableTrainer *brock = GetNotableTrainerById(NOTABLE_TRAINER_BROCK);
    const struct NotableTrainer *blue = GetNotableTrainerById(NOTABLE_TRAINER_BLUE);
    const struct NotableTrainer *lance = GetNotableTrainerById(NOTABLE_TRAINER_LANCE);
    u32 id, world;
    EXPECT_EQ(GetNotableTrainerRating(brock, 0), 25);
    EXPECT_EQ(GetNotableTrainerRating(brock, 40), 44);
    EXPECT_EQ(GetNotableTrainerRating(brock, 80), 63);
    EXPECT_EQ(GetNotableTrainerRating(brock, 160), 100);
    EXPECT_EQ(GetNotableTrainerRating(blue, 0), 0);
    EXPECT_EQ(GetNotableTrainerRating(blue, 20), 26);
    EXPECT_EQ(GetNotableTrainerRating(blue, 40), 49);
    EXPECT_EQ(GetNotableTrainerRating(blue, 160), 170);
    EXPECT_EQ(GetNotableTrainerRating(lance, 0), 200);
    EXPECT_EQ(GetNotableTrainerRating(lance, 300), 200);
    for (id = 1; id <= NOTABLE_TRAINER_COUNT; id++)
    {
        const struct NotableTrainer *trainer = GetNotableTrainerById(id);
        u32 previous = GetNotableTrainerRating(trainer, 0);
        EXPECT_EQ(previous, trainer->startTR);
        for (world = 1; world <= 200; world++)
        {
            u32 rating = GetNotableTrainerRating(trainer, world);
            EXPECT(rating >= previous);
            EXPECT(rating <= trainer->peakTR);
            previous = rating;
        }
        EXPECT_EQ(GetNotableTrainerRating(trainer, 300), trainer->peakTR);
    }
}

TEST("Notable snapshots keep stable roster slots separate from battle order")
{
    struct NotableTrainerSnapshot snapshot;
    EXPECT(ResolveNotableTrainerSnapshot(GetNotableTrainerById(NOTABLE_TRAINER_BROCK), 0, FALSE, &snapshot));
    EXPECT_EQ(snapshot.trainerTR, 25);
    EXPECT_EQ(snapshot.teamSize, 2);
    EXPECT_EQ(snapshot.members[0].species, SPECIES_ONIX);
    EXPECT_EQ(snapshot.members[0].lvl, 18);
    EXPECT_EQ(snapshot.members[1].species, SPECIES_GEODUDE);
    EXPECT_EQ(snapshot.members[1].lvl, 16);
    EXPECT_EQ(snapshot.battleOrder[0], 1);
    EXPECT_EQ(snapshot.battleOrder[1], 0);
    EXPECT_EQ(snapshot.aceCount, 1);
    EXPECT(ResolveNotableTrainerSnapshot(GetNotableTrainerById(NOTABLE_TRAINER_BLUE), 0, FALSE, &snapshot));
    EXPECT_EQ(snapshot.teamSize, 1);
    EXPECT_EQ(snapshot.members[0].species, SPECIES_EEVEE);
    EXPECT_EQ(snapshot.members[0].lvl, 5);
    EXPECT(ResolveNotableTrainerSnapshot(GetNotableTrainerById(NOTABLE_TRAINER_TATE_LIZA), 0, FALSE, &snapshot));
    EXPECT_EQ(snapshot.teamSize, 2);
    EXPECT_EQ(snapshot.battleOrder[0], 1); // Liza's Lunatone
    EXPECT_EQ(snapshot.battleOrder[1], 0); // Tate's Solrock
    EXPECT_EQ(snapshot.aceCount, 2);
}

TEST("Every notable trainer resolves a complete legal snapshot across progression")
{
    static const u32 checkpoints[] = {0, 40, 80, 120, 160};
    struct NotableTrainerSnapshot snapshot;
    u32 id, checkpoint, slot, position;
    for (id = 1; id <= NOTABLE_TRAINER_COUNT; id++)
    {
        const struct NotableTrainer *trainer = GetNotableTrainerById(id);
        for (checkpoint = 0; checkpoint < ARRAY_COUNT(checkpoints); checkpoint++)
        {
            u8 seen = 0;
            EXPECT(ResolveNotableTrainerSnapshot(trainer, checkpoints[checkpoint], FALSE, &snapshot));
            EXPECT(snapshot.teamSize >= 1 && snapshot.teamSize <= PARTY_SIZE);
            EXPECT(snapshot.aiFlags != 0);
            for (slot = 0; slot < snapshot.teamSize; slot++)
            {
                EXPECT(snapshot.members[slot].species != SPECIES_NONE);
                EXPECT(snapshot.members[slot].lvl >= 1 && snapshot.members[slot].lvl <= 100);
                EXPECT(snapshot.members[slot].moves[0] != MOVE_NONE);
            }
            for (position = 0; position < snapshot.teamSize; position++)
            {
                u8 rosterSlot = snapshot.battleOrder[position];
                EXPECT(rosterSlot < snapshot.teamSize);
                EXPECT(!(seen & (1 << rosterSlot)));
                seen |= 1 << rosterSlot;
            }
            EXPECT_EQ(seen, (1 << snapshot.teamSize) - 1);
            EXPECT_EQ(snapshot.battleOrder[snapshot.teamSize - 1], 0);
        }
    }
}

#endif // IS_WAYFARER
