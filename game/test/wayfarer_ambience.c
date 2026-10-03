#include "global.h"
#include "test/test.h"
#include "wayfarer_ambience.h"
#include "wayfarer_world_data.h"
#include "constants/notable_trainers.h"

// Notable ambience beat selection on the target (src/wayfarer_ambience.c).
// The full matrix runs on the host (tools/wayfarer_world_report/ambience);
// these repeat a few checks with the ROM's compiler and tables. Locals only:
// the mechanics-test build has almost no EWRAM to spare.

#if IS_WAYFARER

static u8 SlotOf(u16 characterId)
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (gWayfarerWorldTrainers[slot].characterId == characterId)
            break;
    }
    return slot;
}

// The pool row with these traits (the tables carry no names).
static u8 FindBeat(u8 cls, u32 all, u8 flags, u16 tagsAny, u16 tagsNone)
{
    u8 beat;
    for (beat = 0; beat < gWayfarerAmbienceBeatCount; beat++)
    {
        const struct WayfarerAmbienceBeat *row = &gWayfarerAmbienceBeats[beat];
        if (row->cls == cls && row->all == all && row->flags == flags
         && row->tagsAny == tagsAny && row->tagsNone == tagsNone)
            return beat;
    }
    return AMBIENCE_BEAT_NONE;
}

static void InitContext(struct AmbienceContext *ctx, u32 facts)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->facts = facts;
    ctx->spotKind = 0xFF;
    ctx->spotActivities = 0xFF;
    ctx->activity = WORLD_ACTIVITY_NONE;
    ctx->playerDistance = AMBIENCE_DISTANCE_NONE;
    memset(ctx->notableDistance, AMBIENCE_DISTANCE_NONE, sizeof(ctx->notableDistance));
    memset(ctx->notableActor, AMBIENCE_ACTOR_NONE, sizeof(ctx->notableActor));
}

TEST("Ambience: notice_player fires once per approach and ignores the quiet gap")
{
    u8 brock = SlotOf(NOTABLE_TRAINER_BROCK);
    u8 notice = FindBeat(AMBIENCE_CLASS_REACT, AMBIENCE_FACT_PLAYER_WITHIN, AMBIENCE_FLAG_ONCE_PER_APPROACH, 0, AMBIENCE_TAG_STOIC);
    struct AmbienceContext ctx;
    struct AmbienceWalker walker;
    u8 latches = 0;

    EXPECT(brock < WORLD_SIM_TRAINER_COUNT);
    EXPECT(notice != AMBIENCE_BEAT_NONE);
    Ambience_InitWalker(&walker);
    InitContext(&ctx, AMBIENCE_FACT_WALKING);
    ctx.playerDistance = 2;
    EXPECT_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), notice);
    Ambience_EndBeat(&walker);
    memset(walker.cooldown, 0, sizeof(walker.cooldown));
    Ambience_UpdateLatches(&latches, brock, &ctx);
    EXPECT_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), AMBIENCE_BEAT_NONE);
    ctx.playerDistance = AMBIENCE_DISTANCE_NONE;
    Ambience_UpdateLatches(&latches, brock, &ctx);
    ctx.playerDistance = 3;
    EXPECT_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), notice);
}

TEST("Ambience: notice_player on cooldown uses up the approach, and a fresh walker's steps count as passed")
{
    u8 brock = SlotOf(NOTABLE_TRAINER_BROCK);
    u8 notice = FindBeat(AMBIENCE_CLASS_REACT, AMBIENCE_FACT_PLAYER_WITHIN, AMBIENCE_FLAG_ONCE_PER_APPROACH, 0, AMBIENCE_TAG_STOIC);
    struct AmbienceContext ctx;
    struct AmbienceWalker walker;
    u8 latches = 0;

    EXPECT(notice != AMBIENCE_BEAT_NONE);
    Ambience_InitWalker(&walker);
    EXPECT_EQ(walker.stepsSinceBeat, 0xFF);
    walker.cooldown[notice] = 1;
    InitContext(&ctx, AMBIENCE_FACT_WALKING);
    ctx.playerDistance = 2;
    EXPECT_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), AMBIENCE_BEAT_NONE);
    EXPECT_EQ(latches, AMBIENCE_LATCH_APPROACH);
    Ambience_Tick(&walker);
    Ambience_UpdateLatches(&latches, brock, &ctx);
    EXPECT_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), AMBIENCE_BEAT_NONE);
}

TEST("Ambience: the quiet gap holds idle beats back, three times as long for a stoic trainer")
{
    u8 brock = SlotOf(NOTABLE_TRAINER_BROCK), sabrina = SlotOf(NOTABLE_TRAINER_SABRINA);
    u8 c = gWayfarerWorldTrainers[sabrina].characterId - 1;
    struct AmbienceContext ctx;
    struct AmbienceWalker walker;
    u8 latches = 0;

    EXPECT_EQ(Ambience_QuietGap(brock), AMBIENCE_GAP_BASE + AMBIENCE_GAP_STEP * ((gWayfarerWorldTrainers[brock].characterId - 1) % 3));
    EXPECT_EQ(Ambience_QuietGap(sabrina), 3 * (AMBIENCE_GAP_BASE + AMBIENCE_GAP_STEP * (c % 3)));

    // Sabrina dwelling at a Gym: meditate, once the gap has passed and the odds allow.
    InitContext(&ctx, AMBIENCE_FACT_DWELLING);
    ctx.spotKind = WORLD_SPOT_GYM;
    Ambience_InitWalker(&walker);
    walker.gapTicks = Ambience_QuietGap(sabrina) - 1;
    walker.decisionCounter = (u8)(2 * AMBIENCE_IDLE_DECISIONS - 1 - c % AMBIENCE_IDLE_DECISIONS);
    EXPECT_EQ(Ambience_Select(&walker, &latches, sabrina, &ctx, AMBIENCE_DECIDE_DWELL), AMBIENCE_BEAT_NONE);
    walker.gapTicks = Ambience_QuietGap(sabrina);
    walker.decisionCounter = (u8)(2 * AMBIENCE_IDLE_DECISIONS - 1 - c % AMBIENCE_IDLE_DECISIONS);
    EXPECT(Ambience_Select(&walker, &latches, sabrina, &ctx, AMBIENCE_DECIDE_DWELL) != AMBIENCE_BEAT_NONE);
}

TEST("Ambience: water_bite holds only facing water at a water's edge")
{
    u8 misty = SlotOf(NOTABLE_TRAINER_MISTY);
    u8 bite = FindBeat(AMBIENCE_CLASS_IDLE, AMBIENCE_FACT_DWELLING | AMBIENCE_FACT_FACING_WATER | AMBIENCE_FACT_SPOT, 0, 0, 0);
    struct AmbienceContext ctx;

    EXPECT(bite != AMBIENCE_BEAT_NONE);
    InitContext(&ctx, AMBIENCE_FACT_DWELLING | AMBIENCE_FACT_FACING_WATER);
    ctx.spotKind = WORLD_SPOT_WATER_EDGE;
    EXPECT(Ambience_BeatHolds(bite, misty, &ctx, 0));
    ctx.facts = AMBIENCE_FACT_DWELLING;
    EXPECT(!Ambience_BeatHolds(bite, misty, &ctx, 0));
    ctx.facts = AMBIENCE_FACT_DWELLING | AMBIENCE_FACT_FACING_WATER;
    ctx.spotKind = WORLD_SPOT_TALL_GRASS;
    EXPECT(!Ambience_BeatHolds(bite, misty, &ctx, 0));
}

TEST("Ambience: the same calls give the same beats")
{
    u8 slot = SlotOf(NOTABLE_TRAINER_BROCK);
    struct AmbienceContext ctx;
    struct AmbienceWalker a, b;
    u8 latchesA = 0, latchesB = 0, beatA, beatB;
    u32 i, beats = 0;

    Ambience_InitWalker(&a);
    Ambience_InitWalker(&b);
    for (i = 0; i < 400; i++)
    {
        InitContext(&ctx, (i % 100) < 50 ? AMBIENCE_FACT_WALKING : AMBIENCE_FACT_DWELLING | AMBIENCE_FACT_OUTDOORS);
        ctx.spotKind = (i % 100) < 50 ? 0xFF : WORLD_SPOT_SQUARE;
        ctx.playerDistance = (i % 70) < 10 ? (i % 70) / 2 : AMBIENCE_DISTANCE_NONE;
        Ambience_Tick(&a);
        Ambience_Tick(&b);
        Ambience_UpdateLatches(&latchesA, slot, &ctx);
        Ambience_UpdateLatches(&latchesB, slot, &ctx);
        if (a.beat != AMBIENCE_BEAT_NONE && (i % 4) == 0)
            Ambience_EndBeat(&a);
        if (b.beat != AMBIENCE_BEAT_NONE && (i % 4) == 0)
            Ambience_EndBeat(&b);
        if (a.beat != AMBIENCE_BEAT_NONE || b.beat != AMBIENCE_BEAT_NONE)
        {
            EXPECT_EQ(a.beat, b.beat);
            continue;
        }
        beatA = Ambience_Select(&a, &latchesA, slot, &ctx, (i % 100) < 50 ? AMBIENCE_DECIDE_STEP : AMBIENCE_DECIDE_DWELL);
        beatB = Ambience_Select(&b, &latchesB, slot, &ctx, (i % 100) < 50 ? AMBIENCE_DECIDE_STEP : AMBIENCE_DECIDE_DWELL);
        EXPECT_EQ(beatA, beatB);
        beats += beatA != AMBIENCE_BEAT_NONE;
    }
    EXPECT(beats > 0);
}

#endif // IS_WAYFARER
