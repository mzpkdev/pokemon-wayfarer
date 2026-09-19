#include "global.h"
#include "test/test.h"
#include "constants/opponents.h"
#include "wayfarer_coast_trainers.h"

#if IS_WAYFARER

TEST("Coast staged rematches keep the first Vs. Seeker party after the initial battle")
{
    const u16 trainerId = TRAINER_WAYFARER_COAST_PICNICKER_MISSY;

    gSaveBlock3Ptr->wayfarerCoast.vars[WAYFARER_COAST_REMATCH_STAGE_VAR_LOW] = 0;
    gSaveBlock3Ptr->wayfarerCoast.vars[WAYFARER_COAST_REMATCH_STAGE_VAR_HIGH] = 0;
    WayfarerCoastRematchClearAllReady();

    // Initial battles never mark a compact Coast family ready, so their
    // completion cannot skip the first staged party.
    EXPECT_EQ(WayfarerCoastRematchGetOpponent(trainerId),
              TRAINER_WAYFARER_COAST_PICNICKER_MISSY_2);
    EXPECT(!WayfarerCoastRematchCompleteIfReady(trainerId));
    EXPECT_EQ(WayfarerCoastRematchGetOpponent(trainerId),
              TRAINER_WAYFARER_COAST_PICNICKER_MISSY_2);

    WayfarerCoastRematchSetReady(trainerId);
    EXPECT(WayfarerCoastRematchCompleteIfReady(trainerId));
    EXPECT_EQ(WayfarerCoastRematchGetOpponent(trainerId),
              TRAINER_WAYFARER_COAST_PICNICKER_MISSY_3);

    // The final authored stage remains the reusable rematch party.
    WayfarerCoastRematchSetReady(trainerId);
    EXPECT(WayfarerCoastRematchCompleteIfReady(trainerId));
    EXPECT_EQ(WayfarerCoastRematchGetOpponent(trainerId),
              TRAINER_WAYFARER_COAST_PICNICKER_MISSY_3);
}

#endif  // IS_WAYFARER
