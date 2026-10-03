// Host tests of the beat selection (src/wayfarer_ambience.c) against the
// real generated tables. test_ambience_select.py builds it with prelude.h
// and a generated ambience_beats.h (BEAT_<id> from the pool's comments).

#include <stdio.h>
#include <string.h>
#include "wayfarer_ambience.h"
#include "wayfarer_world_data.h"
#include "constants/notable_trainers.h"
#include "ambience_beats.h"

static int sFailures;

#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); sFailures++; } } while (0)
#define CHECK_EQ(a, b) do { long _a = (long)(a), _b = (long)(b); if (_a != _b) { \
    printf("FAIL %s:%d: %s == %s (%ld != %ld)\n", __FILE__, __LINE__, #a, #b, _a, _b); sFailures++; } } while (0)

static u8 Slot(u16 characterId)
{
    u8 slot;
    for (slot = 0; slot < WORLD_SIM_TRAINER_COUNT; slot++)
    {
        if (gWayfarerWorldTrainers[slot].characterId == characterId)
            return slot;
    }
    printf("FAIL: no slot for trainer %u\n", characterId);
    sFailures++;
    return 0;
}

static u8 Catalog(u8 slot)
{
    return gWayfarerWorldTrainers[slot].characterId - 1;
}

static struct AmbienceContext Ctx(u32 facts)
{
    struct AmbienceContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.facts = facts;
    ctx.spotKind = 0xFF;
    ctx.spotActivities = 0xFF;
    ctx.activity = WORLD_ACTIVITY_NONE;
    ctx.playerDistance = AMBIENCE_DISTANCE_NONE;
    memset(ctx.notableDistance, AMBIENCE_DISTANCE_NONE, sizeof(ctx.notableDistance));
    memset(ctx.notableActor, AMBIENCE_ACTOR_NONE, sizeof(ctx.notableActor));
    return ctx;
}

static struct AmbienceContext Dwelling(u8 spotKind, u32 facts)
{
    struct AmbienceContext ctx = Ctx(AMBIENCE_FACT_DWELLING | facts);
    ctx.spotKind = spotKind;
    return ctx;
}

// A walker past its quiet gap.
static struct AmbienceWalker Ready(void)
{
    struct AmbienceWalker walker;
    Ambience_InitWalker(&walker);
    walker.gapTicks = 0xFF;
    return walker;
}

// Sets t so the next ARRIVE/LEAVE/DWELL decision passes (pass) or fails the idle odds.
static void AimDwellOdds(struct AmbienceWalker *walker, u8 slot, bool8 pass)
{
    u8 t = 0;
    while (((Catalog(slot) + t + 1) % AMBIENCE_IDLE_DECISIONS == 0) != pass)
        t++;
    walker->decisionCounter = t;
}

// Runs one decision and returns the beat, leaving no beat running and no
// cooldown or gap behind (to look at selection alone).
static u8 Peek(struct AmbienceWalker *walker, u8 *latches, u8 slot, const struct AmbienceContext *ctx, u8 decision)
{
    u8 beat = Ambience_Select(walker, latches, slot, ctx, decision);
    walker->beat = AMBIENCE_BEAT_NONE;
    walker->other = AMBIENCE_ACTOR_NONE;
    return beat;
}

static void TestInit(void)
{
    struct AmbienceWalker walker;
    u32 i;
    memset(&walker, 0xAA, sizeof(walker));
    Ambience_InitWalker(&walker);
    CHECK_EQ(walker.beat, AMBIENCE_BEAT_NONE);
    CHECK_EQ(walker.other, AMBIENCE_ACTOR_NONE);
    CHECK_EQ(walker.beatCounter + walker.decisionCounter + walker.stepCount + walker.stepsSinceBeat + walker.gapTicks, 0);
    for (i = 0; i < AMBIENCE_MAX_BEATS; i++)
        CHECK_EQ(walker.cooldown[i], 0);
}

static void TestGate(void)
{
    u8 erika = Slot(NOTABLE_TRAINER_ERIKA), brock = Slot(NOTABLE_TRAINER_BROCK);
    u8 sabrina = Slot(NOTABLE_TRAINER_SABRINA);
    struct AmbienceContext ctx = Ctx(AMBIENCE_FACT_DWELLING);
    struct AmbienceWalker walker;
    u8 latches = 0, gap = Ambience_QuietGap(erika);

    // The quiet gap: 16 + 4 * (c mod 3), three times that for a stoic trainer.
    CHECK_EQ(gap, 16 + 4 * (Catalog(erika) % 3));
    CHECK_EQ(Ambience_QuietGap(brock), 16 + 4 * (Catalog(brock) % 3));
    CHECK_EQ(Ambience_QuietGap(sabrina), 3 * (16 + 4 * (Catalog(sabrina) % 3)));

    // Erika dwelling: flourish is her only beat. The gap blocks it.
    Ambience_InitWalker(&walker);
    walker.gapTicks = gap - 1;
    AimDwellOdds(&walker, erika, TRUE);
    CHECK_EQ(Peek(&walker, &latches, erika, &ctx, AMBIENCE_DECIDE_DWELL), AMBIENCE_BEAT_NONE);
    walker.gapTicks = gap;
    AimDwellOdds(&walker, erika, TRUE);
    CHECK_EQ(Peek(&walker, &latches, erika, &ctx, AMBIENCE_DECIDE_DWELL), BEAT_FLOURISH);

    // A stoic trainer waits three times as long (Sabrina meditates at a Gym).
    ctx = Dwelling(WORLD_SPOT_GYM, 0);
    Ambience_InitWalker(&walker);
    walker.gapTicks = Ambience_QuietGap(sabrina) / 3;
    AimDwellOdds(&walker, sabrina, TRUE);
    CHECK_EQ(Peek(&walker, &latches, sabrina, &ctx, AMBIENCE_DECIDE_DWELL), AMBIENCE_BEAT_NONE);
    walker.gapTicks = Ambience_QuietGap(sabrina) - 1;
    AimDwellOdds(&walker, sabrina, TRUE);
    CHECK_EQ(Peek(&walker, &latches, sabrina, &ctx, AMBIENCE_DECIDE_DWELL), AMBIENCE_BEAT_NONE);
    walker.gapTicks = Ambience_QuietGap(sabrina);
    AimDwellOdds(&walker, sabrina, TRUE);
    CHECK_EQ(Peek(&walker, &latches, sabrina, &ctx, AMBIENCE_DECIDE_DWELL), BEAT_MEDITATE);

    // React beats ignore the gap; transition beats do not.
    ctx = Ctx(AMBIENCE_FACT_WALKING);
    ctx.playerDistance = 2;
    Ambience_InitWalker(&walker);
    CHECK_EQ(Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), BEAT_NOTICE_PLAYER);
    latches = 0;
    ctx = Dwelling(WORLD_SPOT_SQUARE, AMBIENCE_FACT_LEAVING);
    Ambience_InitWalker(&walker);
    CHECK_EQ(Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_LEAVE), AMBIENCE_BEAT_NONE);
    walker.gapTicks = 0xFF;
    CHECK_EQ(Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_LEAVE), BEAT_LEAVE_TURN);
}

static void TestCounters(void)
{
    u8 brock = Slot(NOTABLE_TRAINER_BROCK);
    struct AmbienceContext ctx = Ctx(AMBIENCE_FACT_WALKING);
    struct AmbienceWalker walker;
    u8 latches = 0;

    // The counters advance first, whether or not the gate lets a beat through.
    Ambience_InitWalker(&walker);
    Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_STEP);
    CHECK_EQ(walker.stepCount, 1);
    CHECK_EQ(walker.stepsSinceBeat, 1);
    CHECK_EQ(walker.decisionCounter, 0);
    ctx = Dwelling(WORLD_SPOT_SQUARE, 0);
    Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_DWELL);
    Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_ARRIVE);
    Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_LEAVE);
    CHECK_EQ(walker.decisionCounter, 3);
    CHECK_EQ(walker.stepCount, 1);
    // Nothing advances at a react check.
    Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT);
    CHECK_EQ(walker.decisionCounter, 3);
    CHECK_EQ(walker.stepCount, 1);
    CHECK_EQ(walker.stepsSinceBeat, 1);
    CHECK_EQ(walker.beatCounter, 0);
}

static void TestClassPriority(void)
{
    u8 brock = Slot(NOTABLE_TRAINER_BROCK);
    struct AmbienceContext ctx = Dwelling(WORLD_SPOT_SQUARE, AMBIENCE_FACT_OUTDOORS | AMBIENCE_FACT_LEAVING);
    struct AmbienceWalker walker = Ready();
    u8 latches = 0, beat;

    // Idle (people_watch, study_ground), transition (leave_turn) and react
    // (notice_player) all hold: react wins.
    ctx.playerDistance = 3;
    AimDwellOdds(&walker, brock, TRUE);
    CHECK_EQ(Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_LEAVE), BEAT_NOTICE_PLAYER);
    // Without the player, transition beats beat idle ones, even when the odds would pass.
    latches = 0;
    ctx.playerDistance = AMBIENCE_DISTANCE_NONE;
    AimDwellOdds(&walker, brock, TRUE);
    CHECK_EQ(Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_LEAVE), BEAT_LEAVE_TURN);
    // Transition beats ignore the idle odds.
    AimDwellOdds(&walker, brock, FALSE);
    CHECK_EQ(Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_LEAVE), BEAT_LEAVE_TURN);
    // Only idle beats left: one of them, when the odds pass.
    ctx.facts &= ~AMBIENCE_FACT_LEAVING;
    AimDwellOdds(&walker, brock, TRUE);
    beat = Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_DWELL);
    CHECK(beat == BEAT_PEOPLE_WATCH || beat == BEAT_STUDY_GROUND);
    AimDwellOdds(&walker, brock, FALSE);
    CHECK_EQ(Peek(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_DWELL), AMBIENCE_BEAT_NONE);
}

static void TestIdleOdds(void)
{
    u8 erika = Slot(NOTABLE_TRAINER_ERIKA), brock = Slot(NOTABLE_TRAINER_BROCK);
    struct AmbienceContext ctx = Ctx(AMBIENCE_FACT_DWELLING);
    struct AmbienceWalker walker = Ready();
    u8 latches = 0, beat;
    u32 t, s, since, fired = 0;

    // Dwell ticks: fire exactly when (c + t) mod 4 == 0, t counting this decision (from 1).
    for (t = 1; t <= 16; t++)
    {
        beat = Peek(&walker, &latches, erika, &ctx, AMBIENCE_DECIDE_DWELL);
        CHECK_EQ(walker.decisionCounter, t);
        CHECK_EQ(beat != AMBIENCE_BEAT_NONE, (Catalog(erika) + t) % 4 == 0);
        fired += beat != AMBIENCE_BEAT_NONE;
    }
    CHECK_EQ(fired, 4);

    // Walking: (c + s) mod 10 == 0 with at least 6 steps since the last beat.
    // Brock walking: look_around and hum (cheerful).
    ctx = Ctx(AMBIENCE_FACT_WALKING);
    walker = Ready();
    since = 0;
    fired = 0;
    for (s = 1; s <= 60; s++)
    {
        since++;
        beat = Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_STEP);
        CHECK_EQ(beat != AMBIENCE_BEAT_NONE, (Catalog(brock) + s) % 10 == 0 && since >= 6);
        if (beat != AMBIENCE_BEAT_NONE)
        {
            CHECK(beat == BEAT_LOOK_AROUND || beat == BEAT_HUM);
            Ambience_EndBeat(&walker);
            memset(walker.cooldown, 0, sizeof(walker.cooldown));
            walker.gapTicks = 0xFF;
            since = 0;
            fired++;
        }
    }
    CHECK_EQ(fired, 6);

    // Too few steps since the last beat: the 10th-step window passes unused.
    walker = Ready();
    walker.stepCount = (u8)((20 - Catalog(brock) % 10 - 1) % 10);   // the next step hits the window
    walker.stepsSinceBeat = 4;
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_STEP), AMBIENCE_BEAT_NONE);
    CHECK_EQ((Catalog(brock) + walker.stepCount) % 10, 0);
    walker.stepCount = (u8)((20 - Catalog(brock) % 10 - 1) % 10);
    walker.stepsSinceBeat = 5;
    CHECK(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_STEP) != AMBIENCE_BEAT_NONE);
}

static void TestPick(void)
{
    u8 misty = Slot(NOTABLE_TRAINER_MISTY), jasmine = Slot(NOTABLE_TRAINER_JASMINE);
    struct AmbienceContext ctx = Dwelling(WORLD_SPOT_WATER_EDGE, AMBIENCE_FACT_FACING_WATER);
    struct AmbienceWalker walker = Ready();
    // Pool order, the preferred beat (Misty: admire_water) twice.
    static const u8 mistyList[] = {BEAT_WATER_BITE, BEAT_WATER_WAIT, BEAT_ADMIRE_WATER, BEAT_ADMIRE_WATER};
    static const u8 jasmineList[] = {BEAT_WATER_BITE, BEAT_WATER_WAIT, BEAT_ADMIRE_WATER};
    u8 latches = 0, beat, k;
    u32 admire = 0;

    CHECK_EQ(gWayfarerAmbienceTrainers[misty].preferred, BEAT_ADMIRE_WATER);
    CHECK_EQ(gWayfarerAmbienceTrainers[jasmine].preferred, AMBIENCE_PREFERRED_NONE);
    for (k = 0; k < 12; k++)
    {
        AimDwellOdds(&walker, misty, TRUE);
        CHECK_EQ(walker.beatCounter, k);
        beat = Peek(&walker, &latches, misty, &ctx, AMBIENCE_DECIDE_DWELL);
        CHECK_EQ(beat, mistyList[(Catalog(misty) + 7 * k) % 4]);
        CHECK_EQ(walker.beatCounter, k + 1);
        admire += beat == BEAT_ADMIRE_WATER;
    }
    CHECK_EQ(admire, 6);    // half the entries

    walker = Ready();
    admire = 0;
    for (k = 0; k < 12; k++)
    {
        AimDwellOdds(&walker, jasmine, TRUE);
        beat = Peek(&walker, &latches, jasmine, &ctx, AMBIENCE_DECIDE_DWELL);
        CHECK_EQ(beat, jasmineList[(Catalog(jasmine) + 7 * k) % 3]);
        admire += beat == BEAT_ADMIRE_WATER;
    }
    CHECK_EQ(admire, 4);    // a third
}

static void TestCooldown(void)
{
    u8 erika = Slot(NOTABLE_TRAINER_ERIKA);
    struct AmbienceContext ctx = Ctx(AMBIENCE_FACT_DWELLING);
    struct AmbienceWalker walker = Ready();
    u8 latches = 0, cooldown = gWayfarerAmbienceBeats[BEAT_FLOURISH].cooldown;
    u32 i;

    AimDwellOdds(&walker, erika, TRUE);
    CHECK_EQ(Ambience_Select(&walker, &latches, erika, &ctx, AMBIENCE_DECIDE_DWELL), BEAT_FLOURISH);
    CHECK_EQ(walker.beat, BEAT_FLOURISH);
    Ambience_EndBeat(&walker);
    CHECK_EQ(walker.beat, AMBIENCE_BEAT_NONE);
    CHECK_EQ(walker.other, AMBIENCE_ACTOR_NONE);
    CHECK_EQ(walker.gapTicks, 0);
    CHECK_EQ(walker.stepsSinceBeat, 0);
    CHECK_EQ(walker.cooldown[BEAT_FLOURISH], cooldown);
    CHECK_EQ(cooldown, AMBIENCE_DEFAULT_COOLDOWN);
    for (i = 1; i < cooldown; i++)
        Ambience_Tick(&walker);
    CHECK_EQ(walker.cooldown[BEAT_FLOURISH], 1);
    CHECK(walker.gapTicks >= Ambience_QuietGap(erika));
    AimDwellOdds(&walker, erika, TRUE);
    CHECK_EQ(Peek(&walker, &latches, erika, &ctx, AMBIENCE_DECIDE_DWELL), AMBIENCE_BEAT_NONE);
    Ambience_Tick(&walker);
    CHECK_EQ(walker.cooldown[BEAT_FLOURISH], 0);
    AimDwellOdds(&walker, erika, TRUE);
    CHECK_EQ(Peek(&walker, &latches, erika, &ctx, AMBIENCE_DECIDE_DWELL), BEAT_FLOURISH);
    // The gap saturates instead of wrapping.
    for (i = 0; i < 300; i++)
        Ambience_Tick(&walker);
    CHECK_EQ(walker.gapTicks, 0xFF);
}

static void TestContext(void)
{
    u8 brock = Slot(NOTABLE_TRAINER_BROCK), blaine = Slot(NOTABLE_TRAINER_BLAINE);
    u8 sabrina = Slot(NOTABLE_TRAINER_SABRINA), morty = Slot(NOTABLE_TRAINER_MORTY);
    u8 blue = Slot(NOTABLE_TRAINER_BLUE);
    struct AmbienceContext ctx;

    // water_bite: only facing water at a water's edge, while dwelling.
    ctx = Dwelling(WORLD_SPOT_WATER_EDGE, AMBIENCE_FACT_FACING_WATER);
    CHECK(Ambience_BeatHolds(BEAT_WATER_BITE, brock, &ctx, 0));
    ctx = Dwelling(WORLD_SPOT_WATER_EDGE, 0);
    CHECK(!Ambience_BeatHolds(BEAT_WATER_BITE, brock, &ctx, 0));
    ctx = Dwelling(WORLD_SPOT_TALL_GRASS, AMBIENCE_FACT_FACING_WATER);
    CHECK(!Ambience_BeatHolds(BEAT_WATER_BITE, brock, &ctx, 0));
    ctx = Ctx(AMBIENCE_FACT_WALKING | AMBIENCE_FACT_FACING_WATER);
    ctx.spotKind = WORLD_SPOT_WATER_EDGE;
    CHECK(!Ambience_BeatHolds(BEAT_WATER_BITE, brock, &ctx, 0));

    // grass_chase: not for an elder.
    ctx = Dwelling(WORLD_SPOT_TALL_GRASS, AMBIENCE_FACT_FACING_GRASS);
    CHECK(Ambience_BeatHolds(BEAT_GRASS_CHASE, brock, &ctx, 0));
    CHECK(!Ambience_BeatHolds(BEAT_GRASS_CHASE, blaine, &ctx, 0));
    CHECK(Ambience_BeatHolds(BEAT_GRASS_RUSTLE, blaine, &ctx, 0));

    // notice_player not for a stoic trainer; stare_down only for one.
    ctx = Ctx(AMBIENCE_FACT_WALKING);
    ctx.playerDistance = 3;
    CHECK(Ambience_BeatHolds(BEAT_NOTICE_PLAYER, brock, &ctx, 0));
    CHECK(!Ambience_BeatHolds(BEAT_NOTICE_PLAYER, sabrina, &ctx, 0));
    CHECK(Ambience_BeatHolds(BEAT_STARE_DOWN, sabrina, &ctx, 0));
    CHECK(!Ambience_BeatHolds(BEAT_STARE_DOWN, brock, &ctx, 0));
    ctx.playerDistance = 4;
    CHECK(!Ambience_BeatHolds(BEAT_NOTICE_PLAYER, brock, &ctx, 0));
    ctx.playerDistance = 3;
    ctx.facts = 0;  // neither walking nor dwelling: notice_player's any-term fails
    CHECK(!Ambience_BeatHolds(BEAT_NOTICE_PLAYER, brock, &ctx, 0));

    // meditate: dwelling, not at a store or Game Corner.
    ctx = Dwelling(WORLD_SPOT_SQUARE, 0);
    CHECK(Ambience_BeatHolds(BEAT_MEDITATE, morty, &ctx, 0));
    ctx = Dwelling(WORLD_SPOT_STORE, 0);
    CHECK(!Ambience_BeatHolds(BEAT_MEDITATE, morty, &ctx, 0));
    ctx = Dwelling(WORLD_SPOT_GAME_CORNER, 0);
    CHECK(!Ambience_BeatHolds(BEAT_MEDITATE, morty, &ctx, 0));
    ctx = Ctx(AMBIENCE_FACT_WALKING);
    CHECK(!Ambience_BeatHolds(BEAT_MEDITATE, morty, &ctx, 0));
    ctx = Dwelling(WORLD_SPOT_SQUARE, 0);
    CHECK(!Ambience_BeatHolds(BEAT_MEDITATE, brock, &ctx, 0));

    // swagger: dwelling, or the player within 4.
    ctx = Dwelling(WORLD_SPOT_SQUARE, 0);
    CHECK(Ambience_BeatHolds(BEAT_SWAGGER, blue, &ctx, 0));
    ctx = Ctx(AMBIENCE_FACT_WALKING);
    CHECK(!Ambience_BeatHolds(BEAT_SWAGGER, blue, &ctx, 0));
    ctx.playerDistance = 4;
    CHECK(Ambience_BeatHolds(BEAT_SWAGGER, blue, &ctx, 0));
    ctx.playerDistance = 5;
    CHECK(!Ambience_BeatHolds(BEAT_SWAGGER, blue, &ctx, 0));
    ctx.playerDistance = 4;
    CHECK(!Ambience_BeatHolds(BEAT_SWAGGER, brock, &ctx, 0));

    // spot named:sightsee: either of a named spot's activities.
    ctx = Dwelling(WORLD_SPOT_NAMED, 0);
    ctx.spotActivities = WORLD_ACTIVITY_STUDY | (WORLD_ACTIVITY_SIGHTSEE << 4);
    CHECK(Ambience_BeatHolds(BEAT_TAKE_IN_VIEW, brock, &ctx, 0));
    ctx.spotActivities = WORLD_ACTIVITY_SIGHTSEE | (WORLD_ACTIVITY_NONE << 4);
    CHECK(Ambience_BeatHolds(BEAT_TAKE_IN_VIEW, brock, &ctx, 0));
    ctx.spotActivities = WORLD_ACTIVITY_STUDY | (WORLD_ACTIVITY_NONE << 4);
    CHECK(!Ambience_BeatHolds(BEAT_TAKE_IN_VIEW, brock, &ctx, 0));
    ctx.spotActivities = 0xFF;
    CHECK(!Ambience_BeatHolds(BEAT_TAKE_IN_VIEW, brock, &ctx, 0));
    // An unnamed spot's kind never matches by activity.
    ctx = Dwelling(WORLD_SPOT_SQUARE, 0);
    ctx.spotActivities = WORLD_ACTIVITY_SIGHTSEE;
    CHECK(!Ambience_BeatHolds(BEAT_TAKE_IN_VIEW, brock, &ctx, 0));
    // Named spots don't match spotKinds by kind (people_watch: square or bench).
    ctx = Dwelling(WORLD_SPOT_NAMED, 0);
    ctx.spotActivities = WORLD_ACTIVITY_SIGHTSEE;
    CHECK(!Ambience_BeatHolds(BEAT_PEOPLE_WATCH, brock, &ctx, 0));

    // activity: doze for dreamy Erika, relaxing or training.
    ctx = Dwelling(WORLD_SPOT_BENCH, 0);
    ctx.activity = WORLD_ACTIVITY_RELAX;
    CHECK(Ambience_BeatHolds(BEAT_DOZE, Slot(NOTABLE_TRAINER_ERIKA), &ctx, 0));
    ctx.activity = WORLD_ACTIVITY_SHOP;
    CHECK(!Ambience_BeatHolds(BEAT_DOZE, Slot(NOTABLE_TRAINER_ERIKA), &ctx, 0));

    // companion beats need a companion and the room for one.
    ctx = Dwelling(WORLD_SPOT_TALL_GRASS, AMBIENCE_FACT_OUTDOORS);
    ctx.activity = WORLD_ACTIVITY_TRAIN;
    CHECK(!Ambience_BeatHolds(BEAT_ACE_PLAY, brock, &ctx, 0));
    ctx.facts |= AMBIENCE_FACT_COMPANION_ROOM;
    CHECK(Ambience_BeatHolds(BEAT_ACE_PLAY, brock, &ctx, 0));
    CHECK(Ambience_BeatHolds(BEAT_ACE_SPAR, blue, &ctx, 0));
    CHECK(!Ambience_BeatHolds(BEAT_ACE_SPAR, brock, &ctx, 0));

    // Out-of-range arguments never hold.
    CHECK(!Ambience_BeatHolds(gWayfarerAmbienceBeatCount, brock, &ctx, 0));
    CHECK(!Ambience_BeatHolds(AMBIENCE_BEAT_NONE, brock, &ctx, 0));
}

static void TestApproach(void)
{
    u8 brock = Slot(NOTABLE_TRAINER_BROCK), sabrina = Slot(NOTABLE_TRAINER_SABRINA);
    struct AmbienceContext ctx = Ctx(AMBIENCE_FACT_WALKING);
    struct AmbienceWalker walker;
    u8 latches = 0;

    Ambience_InitWalker(&walker);
    ctx.playerDistance = 2;
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), BEAT_NOTICE_PLAYER);
    CHECK_EQ(walker.other, AMBIENCE_ACTOR_NONE);
    CHECK(latches & AMBIENCE_LATCH_APPROACH);
    Ambience_EndBeat(&walker);
    memset(walker.cooldown, 0, sizeof(walker.cooldown));
    // Still within range: once per approach.
    Ambience_UpdateLatches(&latches, brock, &ctx);
    CHECK(latches & AMBIENCE_LATCH_APPROACH);
    CHECK(!Ambience_BeatHolds(BEAT_NOTICE_PLAYER, brock, &ctx, latches));
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), AMBIENCE_BEAT_NONE);
    // The player steps out of range (4 tiles), or the walker out of view.
    ctx.playerDistance = 4;
    Ambience_UpdateLatches(&latches, brock, &ctx);
    CHECK_EQ(latches & AMBIENCE_LATCH_APPROACH, 0);
    ctx.playerDistance = 3;
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), BEAT_NOTICE_PLAYER);
    Ambience_EndBeat(&walker);
    memset(walker.cooldown, 0, sizeof(walker.cooldown));
    ctx.playerDistance = AMBIENCE_DISTANCE_NONE;
    Ambience_UpdateLatches(&latches, brock, &ctx);
    CHECK_EQ(latches & AMBIENCE_LATCH_APPROACH, 0);

    // The stoic stare follows the same limit.
    latches = 0;
    Ambience_InitWalker(&walker);
    ctx.playerDistance = 1;
    CHECK_EQ(Ambience_Select(&walker, &latches, sabrina, &ctx, AMBIENCE_DECIDE_REACT), BEAT_STARE_DOWN);
    Ambience_EndBeat(&walker);
    memset(walker.cooldown, 0, sizeof(walker.cooldown));
    CHECK_EQ(Ambience_Select(&walker, &latches, sabrina, &ctx, AMBIENCE_DECIDE_REACT), AMBIENCE_BEAT_NONE);
}

static void TestGreeting(void)
{
    u8 brock = Slot(NOTABLE_TRAINER_BROCK), misty = Slot(NOTABLE_TRAINER_MISTY);
    u8 lance = Slot(NOTABLE_TRAINER_LANCE), clair = Slot(NOTABLE_TRAINER_CLAIR);
    struct AmbienceContext ctx = Ctx(AMBIENCE_FACT_WALKING);
    struct AmbienceWalker walker;
    u8 latches = 0;

    CHECK_EQ(gWayfarerAmbienceRelations[brock][misty], AMBIENCE_RELATION_FRIEND);
    CHECK_EQ(gWayfarerAmbienceRelations[lance][clair], AMBIENCE_RELATION_FAMILY);

    // Misty (actor 1) is Brock's nearest friend, 2 tiles away.
    ctx.notableDistance[AMBIENCE_RELATION_NONE] = 2;
    ctx.notableActor[AMBIENCE_RELATION_NONE] = 1;
    ctx.notableDistance[AMBIENCE_RELATION_FRIEND] = 2;
    ctx.notableActor[AMBIENCE_RELATION_FRIEND] = 1;
    Ambience_InitWalker(&walker);
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), BEAT_GREET_FRIEND);
    CHECK_EQ(walker.other, 1);
    CHECK_EQ(latches, 1 << (AMBIENCE_LATCH_GREETED_SHIFT + 1));
    Ambience_EndBeat(&walker);
    CHECK_EQ(walker.other, AMBIENCE_ACTOR_NONE);
    memset(walker.cooldown, 0, sizeof(walker.cooldown));
    // Once per pair: no second greeting for actor 1 ...
    Ambience_UpdateLatches(&latches, brock, &ctx);
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), AMBIENCE_BEAT_NONE);
    // ... but another friend (actor 3) is greeted.
    ctx.notableActor[AMBIENCE_RELATION_FRIEND] = 3;
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), BEAT_GREET_FRIEND);
    CHECK_EQ(walker.other, 3);
    CHECK_EQ(latches, (1 << (AMBIENCE_LATCH_GREETED_SHIFT + 1)) | (1 << (AMBIENCE_LATCH_GREETED_SHIFT + 3)));
    // Out of range (4 tiles) or with no partner actor: nothing.
    ctx.notableDistance[AMBIENCE_RELATION_FRIEND] = 4;
    CHECK(!Ambience_BeatHolds(BEAT_GREET_FRIEND, brock, &ctx, 0));
    ctx.notableDistance[AMBIENCE_RELATION_FRIEND] = 3;
    ctx.notableActor[AMBIENCE_RELATION_FRIEND] = AMBIENCE_ACTOR_NONE;
    CHECK(!Ambience_BeatHolds(BEAT_GREET_FRIEND, brock, &ctx, 0));

    // Lance greets Clair (family) with greet_family, recording her actor.
    ctx = Ctx(AMBIENCE_FACT_DWELLING);
    ctx.notableDistance[AMBIENCE_RELATION_FAMILY] = 1;
    ctx.notableActor[AMBIENCE_RELATION_FAMILY] = 0;
    latches = 0;
    Ambience_InitWalker(&walker);
    CHECK_EQ(Ambience_Select(&walker, &latches, lance, &ctx, AMBIENCE_DECIDE_REACT), BEAT_GREET_FAMILY);
    CHECK_EQ(walker.other, 0);
    CHECK_EQ(latches, 1 << AMBIENCE_LATCH_GREETED_SHIFT);
}

static void TestLingers(void)
{
    u8 brock = Slot(NOTABLE_TRAINER_BROCK);
    struct AmbienceContext ctx = Dwelling(WORLD_SPOT_SQUARE, 0);
    struct AmbienceWalker walker;
    u8 latches = AMBIENCE_LATCH_APPROACH;   // the notice already fired this approach

    Ambience_InitWalker(&walker);
    ctx.playerDistance = 1;
    ctx.adjacentTicks = 11;
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), AMBIENCE_BEAT_NONE);
    ctx.adjacentTicks = 12;
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), BEAT_PLAYER_LINGERS);
    CHECK(latches & AMBIENCE_LATCH_EPISODE);
    Ambience_EndBeat(&walker);
    memset(walker.cooldown, 0, sizeof(walker.cooldown));
    // Once per standing-still episode.
    ctx.adjacentTicks = 40;
    Ambience_UpdateLatches(&latches, brock, &ctx);
    CHECK(latches & AMBIENCE_LATCH_EPISODE);
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), AMBIENCE_BEAT_NONE);
    // The player moves off: a new episode can start.
    ctx.adjacentTicks = 0;
    Ambience_UpdateLatches(&latches, brock, &ctx);
    CHECK_EQ(latches & AMBIENCE_LATCH_EPISODE, 0);
    CHECK(latches & AMBIENCE_LATCH_APPROACH);   // still within range
    ctx.adjacentTicks = 12;
    CHECK_EQ(Ambience_Select(&walker, &latches, brock, &ctx, AMBIENCE_DECIDE_REACT), BEAT_PLAYER_LINGERS);
}

// A scripted run: a few walkers through walking, arriving, dwelling and
// leaving, the player passing by, beats ending after a few ticks.
static void Script(u8 *out, u32 count)
{
    static const u16 trainers[] = {NOTABLE_TRAINER_BROCK, NOTABLE_TRAINER_MISTY, NOTABLE_TRAINER_SABRINA, NOTABLE_TRAINER_BLUE};
    struct AmbienceWalker walkers[4];
    u8 latches[4] = {0}, running[4] = {0};
    u32 frame, w;

    for (w = 0; w < 4; w++)
        Ambience_InitWalker(&walkers[w]);
    for (frame = 0; frame < count; frame++)
    {
        for (w = 0; w < 4; w++)
        {
            u8 slot = Slot(trainers[w]);
            u32 phase = (frame + 37 * w) % 200;
            u8 decision;
            struct AmbienceContext ctx;
            if (phase < 80)
            {
                ctx = Ctx(AMBIENCE_FACT_WALKING | AMBIENCE_FACT_OUTDOORS);
                decision = AMBIENCE_DECIDE_STEP;
            }
            else if (phase == 80)
            {
                ctx = Dwelling(WORLD_SPOT_WATER_EDGE, AMBIENCE_FACT_ARRIVING | AMBIENCE_FACT_OUTDOORS);
                decision = AMBIENCE_DECIDE_ARRIVE;
            }
            else if (phase < 199)
            {
                ctx = Dwelling(WORLD_SPOT_WATER_EDGE, AMBIENCE_FACT_FACING_WATER | AMBIENCE_FACT_OUTDOORS);
                ctx.activity = WORLD_ACTIVITY_FISH;
                decision = AMBIENCE_DECIDE_DWELL;
            }
            else
            {
                ctx = Dwelling(WORLD_SPOT_WATER_EDGE, AMBIENCE_FACT_LEAVING | AMBIENCE_FACT_OUTDOORS);
                decision = AMBIENCE_DECIDE_LEAVE;
            }
            ctx.playerDistance = (frame % 150) < 20 ? (frame % 150) / 4 : AMBIENCE_DISTANCE_NONE;
            Ambience_Tick(&walkers[w]);
            Ambience_UpdateLatches(&latches[w], slot, &ctx);
            if (walkers[w].beat != AMBIENCE_BEAT_NONE)
            {
                if (++running[w] >= 6)
                {
                    Ambience_EndBeat(&walkers[w]);
                    running[w] = 0;
                }
                out[frame * 4 + w] = 0xFE;
                continue;
            }
            out[frame * 4 + w] = Ambience_Select(&walkers[w], &latches[w], slot, &ctx, decision);
        }
    }
}

static void TestDeterminism(void)
{
    static u8 a[1000 * 4], b[1000 * 4];
    u32 i, beats = 0;
    Script(a, 1000);
    Script(b, 1000);
    CHECK(memcmp(a, b, sizeof(a)) == 0);
    for (i = 0; i < sizeof(a); i++)
        beats += a[i] < AMBIENCE_MAX_BEATS;
    CHECK(beats > 20);
}

int main(void)
{
    TestInit();
    TestGate();
    TestCounters();
    TestClassPriority();
    TestIdleOdds();
    TestPick();
    TestCooldown();
    TestContext();
    TestApproach();
    TestGreeting();
    TestLingers();
    TestDeterminism();
    if (sFailures)
    {
        printf("%d failures\n", sFailures);
        return 1;
    }
    printf("all ok\n");
    return 0;
}
