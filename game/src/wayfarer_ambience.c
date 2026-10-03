// Notable ambience: deterministic beat selection for the on-screen walkers.
// Spec: .product/specs/notable-ambience.md ("Context", "Selection").
//
// Engine-free, like wayfarer_world_sim.c: the ROM, the mechanics tests and
// the host test (tools/wayfarer_world_report/ambience) all compile it.
// Nothing here reads the random number generator; every choice comes from
// the walker's counters, the context and the generated tables.
//
// Counter order: at a decision point the counters advance first, then the
// selection runs, so the idle odds see the decision being made. The first
// dwell decision after spawning tests (c + 1) mod 4 and the first step
// (c + 1) mod 10. The counters advance whether or not a beat is chosen.
// s is kept modulo AMBIENCE_IDLE_STEPS (only its residue is used), so the
// every-10th-step rhythm survives the u8 wrapping; t wraps at 256, a
// multiple of AMBIENCE_IDLE_DECISIONS.

#include "global.h"
#include "wayfarer_ambience.h"
#include "wayfarer_world_data.h"

#if IS_WAYFARER

// The parameterless facts: the walker layer reports these in ctx->facts.
#define FACTS_PLAIN     ((AMBIENCE_FACT_FACING_COUNTER << 1) - 1)
#define FACTS_PARAMETRIC (AMBIENCE_FACT_SPOT | AMBIENCE_FACT_NOT_SPOT | AMBIENCE_FACT_ACTIVITY \
                        | AMBIENCE_FACT_PLAYER_WITHIN | AMBIENCE_FACT_PLAYER_ADJACENT | AMBIENCE_FACT_NOTABLE)
#define NIBBLE_NONE     15
#define GREETED_ACTORS  4   // bits 4..7 of the latches

static u8 CatalogIndex(u8 slot)
{
    return gWayfarerWorldTrainers[slot].characterId - 1;
}

// The greeted bit for an actor, or 0 when it has none (NONE, or no room).
static u8 GreetedBit(u8 actor)
{
    if (actor >= GREETED_ACTORS)
        return 0;
    return 1 << (AMBIENCE_LATCH_GREETED_SHIFT + actor);
}

static bool8 HasBit16(u16 mask, u8 index)
{
    return index < 16 && ((mask >> index) & 1);
}

static bool8 PlayerWithin(const struct WayfarerAmbienceBeat *beat, const struct AmbienceContext *ctx)
{
    return ctx->playerDistance != AMBIENCE_DISTANCE_NONE && ctx->playerDistance <= beat->playerWithin;
}

// The beat's parametric facts that hold now (only those it names).
static u32 ParametricFacts(const struct WayfarerAmbienceBeat *beat, const struct AmbienceContext *ctx)
{
    u32 wanted = (beat->all | beat->any) & FACTS_PARAMETRIC;
    u32 have = 0;
    bool8 dwelling = (ctx->facts & AMBIENCE_FACT_DWELLING) != 0;

    if (wanted == 0)
        return 0;
    if ((wanted & AMBIENCE_FACT_SPOT) && dwelling)
    {
        if (HasBit16(beat->spotKinds, ctx->spotKind))
            have |= AMBIENCE_FACT_SPOT;
        else if (ctx->spotKind == WORLD_SPOT_NAMED)
        {
            u8 low = ctx->spotActivities & 0xF, high = ctx->spotActivities >> 4;
            if ((low != NIBBLE_NONE && HasBit16(beat->spotNamed, low))
             || (high != NIBBLE_NONE && HasBit16(beat->spotNamed, high)))
                have |= AMBIENCE_FACT_SPOT;
        }
    }
    if ((wanted & AMBIENCE_FACT_NOT_SPOT) && dwelling && !HasBit16(beat->notSpotKinds, ctx->spotKind))
        have |= AMBIENCE_FACT_NOT_SPOT;
    if ((wanted & AMBIENCE_FACT_ACTIVITY) && HasBit16(beat->activities, ctx->activity))
        have |= AMBIENCE_FACT_ACTIVITY;
    if ((wanted & AMBIENCE_FACT_PLAYER_WITHIN) && PlayerWithin(beat, ctx))
        have |= AMBIENCE_FACT_PLAYER_WITHIN;
    if ((wanted & AMBIENCE_FACT_PLAYER_ADJACENT) && ctx->adjacentTicks != 0 && ctx->adjacentTicks >= beat->adjacentTicks)
        have |= AMBIENCE_FACT_PLAYER_ADJACENT;
    if ((wanted & AMBIENCE_FACT_NOTABLE) && beat->relation <= AMBIENCE_RELATION_COLLEAGUE
     && ctx->notableDistance[beat->relation] <= beat->notableWithin
     && ctx->notableDistance[beat->relation] != AMBIENCE_DISTANCE_NONE)
        have |= AMBIENCE_FACT_NOTABLE;
    return have;
}

static bool8 WhoHolds(const struct WayfarerAmbienceBeat *beat, u8 slot)
{
    const struct WayfarerAmbienceTrainer *trainer = &gWayfarerAmbienceTrainers[slot];
    if (beat->tagsAny != 0 && (trainer->tags & beat->tagsAny) == 0)
        return FALSE;
    if (trainer->tags & beat->tagsNone)
        return FALSE;
    if ((beat->flags & AMBIENCE_FLAG_NEEDS_COMPANION) && trainer->companionCount == 0)
        return FALSE;
    return TRUE;
}

static bool8 WhenHolds(const struct WayfarerAmbienceBeat *beat, const struct AmbienceContext *ctx)
{
    u32 have = (ctx->facts & FACTS_PLAIN) | ParametricFacts(beat, ctx);
    if ((beat->all & ~have) != 0)
        return FALSE;
    return beat->any == 0 || (beat->any & have) != 0;
}

// The actor a beat reacts to (its greeting partner), or AMBIENCE_ACTOR_NONE.
static u8 Partner(const struct WayfarerAmbienceBeat *beat, const struct AmbienceContext *ctx)
{
    if (((beat->all | beat->any) & AMBIENCE_FACT_NOTABLE) && beat->relation <= AMBIENCE_RELATION_COLLEAGUE)
        return ctx->notableActor[beat->relation];
    return AMBIENCE_ACTOR_NONE;
}

static bool8 LimitAllows(const struct WayfarerAmbienceBeat *beat, const struct AmbienceContext *ctx, u8 latches)
{
    if ((beat->flags & AMBIENCE_FLAG_ONCE_PER_APPROACH) && (latches & AMBIENCE_LATCH_APPROACH))
        return FALSE;
    if ((beat->flags & AMBIENCE_FLAG_ONCE_PER_EPISODE) && (latches & AMBIENCE_LATCH_EPISODE))
        return FALSE;
    if (beat->flags & AMBIENCE_FLAG_ONCE_PER_PAIR)
    {
        u8 bit = GreetedBit(Partner(beat, ctx));
        if (bit == 0 || (latches & bit))
            return FALSE;
    }
    return TRUE;
}

void Ambience_InitWalker(struct AmbienceWalker *walker)
{
    u32 i;
    u8 *bytes = (u8 *)walker;
    for (i = 0; i < sizeof(*walker); i++)
        bytes[i] = 0;
    walker->beat = AMBIENCE_BEAT_NONE;
    walker->other = AMBIENCE_ACTOR_NONE;
}

bool8 Ambience_BeatHolds(u8 beat, u8 slot, const struct AmbienceContext *ctx, u8 latches)
{
    const struct WayfarerAmbienceBeat *row;
    if (beat >= gWayfarerAmbienceBeatCount || slot >= WORLD_SIM_TRAINER_COUNT)
        return FALSE;
    row = &gWayfarerAmbienceBeats[beat];
    return WhoHolds(row, slot) && WhenHolds(row, ctx) && LimitAllows(row, ctx, latches);
}

u8 Ambience_QuietGap(u8 slot)
{
    u8 gap = AMBIENCE_GAP_BASE + AMBIENCE_GAP_STEP * (CatalogIndex(slot) % 3);
    if (gWayfarerAmbienceTrainers[slot].tags & AMBIENCE_TAG_STOIC)
        gap *= AMBIENCE_STOIC_GAP_FACTOR;
    return gap;
}

u8 Ambience_Select(struct AmbienceWalker *walker, u8 *latches, u8 slot,
                   const struct AmbienceContext *ctx, u8 decision)
{
    // Room for every beat plus the preferred beat's second entry.
    u8 list[AMBIENCE_MAX_BEATS + 1];
    u8 n = 0, best = AMBIENCE_CLASS_IDLE, minClass, i, beat, c, preferred;
    const struct WayfarerAmbienceBeat *row;

    if (slot >= WORLD_SIM_TRAINER_COUNT)
        return AMBIENCE_BEAT_NONE;
    c = CatalogIndex(slot);

    // Decision counters (none at a react check).
    if (decision == AMBIENCE_DECIDE_STEP)
    {
        if (++walker->stepCount >= AMBIENCE_IDLE_STEPS)
            walker->stepCount = 0;
        if (walker->stepsSinceBeat != 0xFF)
            walker->stepsSinceBeat++;
    }
    else if (decision != AMBIENCE_DECIDE_REACT)
    {
        walker->decisionCounter++;
    }

    // Gate and idle odds decide up front which classes can win, so rows
    // that could not are never evaluated. Transition beats obey the gate;
    // only react beats ignore it, and only they run at a react check.
    if (decision == AMBIENCE_DECIDE_REACT || walker->gapTicks < Ambience_QuietGap(slot))
        minClass = AMBIENCE_CLASS_REACT;
    else if (decision == AMBIENCE_DECIDE_STEP)
        minClass = ((c + walker->stepCount) % AMBIENCE_IDLE_STEPS == 0
                    && walker->stepsSinceBeat >= AMBIENCE_IDLE_MIN_STEPS)
                   ? AMBIENCE_CLASS_IDLE : AMBIENCE_CLASS_TRANSITION;
    else
        minClass = ((c + walker->decisionCounter) % AMBIENCE_IDLE_DECISIONS == 0)
                   ? AMBIENCE_CLASS_IDLE : AMBIENCE_CLASS_TRANSITION;

    // Filter and class: keep only the rows of the highest class present,
    // in pool order, the preferred beat twice.
    preferred = gWayfarerAmbienceTrainers[slot].preferred;
    for (beat = 0; beat < gWayfarerAmbienceBeatCount; beat++)
    {
        row = &gWayfarerAmbienceBeats[beat];
        if (row->cls < minClass || row->cls < best || walker->cooldown[beat] != 0)
            continue;
        if (!WhoHolds(row, slot) || !WhenHolds(row, ctx) || !LimitAllows(row, ctx, *latches))
            continue;
        if (row->cls > best)
        {
            best = row->cls;
            n = 0;
        }
        list[n++] = beat;
        if (beat == preferred)
            list[n++] = beat;
    }
    if (n == 0)
        return AMBIENCE_BEAT_NONE;

    // Pick: entry (c + 7k) mod n.
    i = (u32)(c + AMBIENCE_PICK_STRIDE * walker->beatCounter) % n;
    beat = list[i];
    row = &gWayfarerAmbienceBeats[beat];

    walker->beat = beat;
    walker->other = Partner(row, ctx);
    walker->beatCounter++;
    if (row->flags & AMBIENCE_FLAG_ONCE_PER_APPROACH)
        *latches |= AMBIENCE_LATCH_APPROACH;
    if (row->flags & AMBIENCE_FLAG_ONCE_PER_EPISODE)
        *latches |= AMBIENCE_LATCH_EPISODE;
    if (row->flags & AMBIENCE_FLAG_ONCE_PER_PAIR)
        *latches |= GreetedBit(walker->other);
    return beat;
}

void Ambience_EndBeat(struct AmbienceWalker *walker)
{
    if (walker->beat < gWayfarerAmbienceBeatCount)
        walker->cooldown[walker->beat] = gWayfarerAmbienceBeats[walker->beat].cooldown;
    walker->gapTicks = 0;
    walker->stepsSinceBeat = 0;
    walker->beat = AMBIENCE_BEAT_NONE;
    walker->other = AMBIENCE_ACTOR_NONE;
}

void Ambience_Tick(struct AmbienceWalker *walker)
{
    u8 beat;
    for (beat = 0; beat < gWayfarerAmbienceBeatCount; beat++)
    {
        if (walker->cooldown[beat] != 0)
            walker->cooldown[beat]--;
    }
    if (walker->gapTicks != 0xFF)
        walker->gapTicks++;
}

void Ambience_UpdateLatches(u8 *latches, u8 slot, const struct AmbienceContext *ctx)
{
    if (slot >= WORLD_SIM_TRAINER_COUNT)
        return;
    if (*latches & AMBIENCE_LATCH_APPROACH)
    {
        u8 beat;
        bool8 inRange = FALSE;
        for (beat = 0; beat < gWayfarerAmbienceBeatCount && !inRange; beat++)
        {
            const struct WayfarerAmbienceBeat *row = &gWayfarerAmbienceBeats[beat];
            if ((row->flags & AMBIENCE_FLAG_ONCE_PER_APPROACH)
             && ((row->all | row->any) & AMBIENCE_FACT_PLAYER_WITHIN)
             && WhoHolds(row, slot) && PlayerWithin(row, ctx))
                inRange = TRUE;
        }
        if (!inRange)
            *latches &= ~AMBIENCE_LATCH_APPROACH;
    }
    if (ctx->adjacentTicks == 0)
        *latches &= ~AMBIENCE_LATCH_EPISODE;
}

#endif // IS_WAYFARER
