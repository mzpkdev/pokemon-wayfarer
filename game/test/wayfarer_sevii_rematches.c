#include "global.h"
#include "battle_setup.h"
#include "test/test.h"
#include "constants/opponents.h"
#include "wayfarer_sevii_rematches.h"
#include "wayfarer_sevii_state.h"

#if IS_WAYFARER

TEST("Sevii Vs. Seeker registry is local, complete, and uses generated IDs")
{
    u32 i;
    u32 familyCount = 0;

    for (i = 0; i < WAYFARER_SEVII_REMATCH_FAMILY_COUNT; i++)
    {
        u32 stage;

        EXPECT_NE(gWayfarerSeviiRematchFamilies[i].trainerIds[0], TRAINER_NONE);
        EXPECT_EQ(WayfarerSeviiRematchFamilyIndex(gWayfarerSeviiRematchFamilies[i].trainerIds[0]), i);
        for (stage = 0; stage < WAYFARER_SEVII_REMATCH_STAGES; stage++)
            EXPECT_NE(gWayfarerSeviiRematchFamilies[i].trainerIds[stage], TRAINER_NONE);
        familyCount++;
    }
    EXPECT_EQ(familyCount, 64);

    EXPECT(WayfarerSeviiRematchHasFamily(TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY));
    EXPECT(!WayfarerSeviiRematchHasFamily(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_JOCELYN));

    EXPECT(!WayfarerSeviiRematchIsReady(TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY));
    WayfarerSeviiRematchSetReady(TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY);
    EXPECT(WayfarerSeviiRematchIsReady(TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY));
    WayfarerSeviiRematchClearReady(TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY);
    EXPECT(!WayfarerSeviiRematchIsReady(TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY));

    WayfarerSeviiRematchStageSet(WayfarerSeviiRematchFamilyIndex(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON), 0);
    EXPECT_EQ(WayfarerSeviiRematchGetOpponent(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON),
              TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON_2);
    WayfarerSeviiRematchAdvance(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    EXPECT_EQ(WayfarerSeviiRematchGetOpponent(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON),
              TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON_3);
    WayfarerSeviiRematchAdvance(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    EXPECT_EQ(WayfarerSeviiRematchGetOpponent(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON),
              TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON_3);

    WayfarerSeviiRematchStageSet(WayfarerSeviiRematchFamilyIndex(TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY), 0);
    EXPECT_EQ(WayfarerSeviiRematchGetOpponent(TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY),
              TRAINER_WAYFARER_SEVII_FISHERMAN_TOMMY);

    // Both source objects use this one generated base identity, so readiness
    // and stage selection remain atomic for the double battle.
    EXPECT(WayfarerSeviiRematchHasFamily(TRAINER_WAYFARER_SEVII_CRUSH_KIN_MIK_KIA));
}

TEST("Sevii rematch battle accepts a ready compact family after opponent selection")
{
    WayfarerSeviiRematchClearReady(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    TRAINER_BATTLE_PARAM.opponentA = TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON_2;
    EXPECT(!IsTrainerReadyForRematch());

    WayfarerSeviiRematchSetReady(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    EXPECT(IsTrainerReadyForRematch());

    WayfarerSeviiRematchClearReady(TRAINER_WAYFARER_SEVII_CRUSH_GIRL_SHARON);
    EXPECT(!IsTrainerReadyForRematch());
}

#endif // IS_WAYFARER
