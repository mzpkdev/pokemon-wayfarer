# Player Trainer Rating

PRD: [Player Trainer Rating](../prds/player-trainer-rating.md)
Implemented: Partial. The formula, its high-water save, and the consumers
below exist; uncapped TR and the scaler framing are the v0 target.
Design status: v0 accepted. The formula is unchanged in v0; changing it is
Later.

## Scope and ownership

This specification owns the player's TR as the foundation other systems read:
what it is, how it is earned, its range, the **scaler** concept every
TR-driven property uses, and the list of what player TR drives. Consumers keep
their own current behavior and link here for the shared model.

- The [interregional League circuit](wayfarer-interregional-league-circuit.md#trainer-rating)
  implements the formula's badge and first-clear contributions.
- [Wild encounter scaling](trainer-rating-wild-encounter-scaling.md#trainer-rating-lifecycle)
  implements the getter, its persistence, and the standalone builds' own
  progression.
- [Well-known trainers](well-known-trainers.md) own trainer TR and the
  team-level and team-size scalers; [Leagues](leagues.md) own league fields.

## Player TR

- **Hidden.** TR is never shown to the player as a number; the Trainer Card
  and dialogue never display it.
- **Independent.** Player TR is never computed from a well-known trainer's TR,
  and no well-known trainer's TR is computed from it.
- **High-water.** `GetTrainerRating()` derives a candidate from current facts,
  compares it with the saved value, and keeps the higher. A new Wayfarer game
  starts at 0; nothing lowers the saved value.

### Formula (current, kept in v0)

Wayfarer TR is the global badge contribution plus +8 for each canonical first
venue clear (Indigo, Masters, Hoenn). The exact badge contribution and
milestones are in the
[circuit contract](wayfarer-interregional-league-circuit.md#trainer-rating).
Losses, replays, individual League victories, repeated ceremonies, and Red add
nothing. v0 keeps this formula unchanged; how TR changes is Later.

### Range

| | Current ROM | v0 target |
| --- | --- | --- |
| Value | Clamped to 0–80 on every read | Non-negative integer, no upper limit |
| Consumers | Clamp to 0–80, then look up their table | Look up their scaler, flat past its last anchor |
| Units | TR points | Unchanged (no ×10) |

Nothing in the target hardcodes 80; today's content happens to span about 0–80.
Because every scaler stays flat past its last anchor, the target gives the
same value as today for every TR the current formula can reach. The soft cap,
for example, stays at Lv 100 past TR 80.

## Scalers

Every TR-driven property is a **scaler**: an authored table of anchors
`(tr, value)` with the first at TR 0, strictly increasing TRs, and
non-decreasing values. Between adjacent anchors `(t0, v0)` and `(t1, v1)`, with
floor division (halves round up):

```text
d     = t1 - t0
value = v0 + floor((2 * (tr - t0) * (v1 - v0) + d) / (2 * d))
```

At or above the last anchor the value **stays flat**. Each scaler therefore has
its own natural cap (the last value), reached at an authored saturation TR (the
last anchor's TR). A **step** scaler instead holds each anchor's value until
the next anchor. Scalers are versioned catalog content; runtime never repairs
or extrapolates them.

### Player-TR scalers (v0)

| Scaler | Form | Range | Natural cap | Saturates at |
| --- | --- | --- | --- | --- |
| Player soft cap | interpolated | Lv 15 → 100 | Lv 100 | TR 80 (existing curve) |
| Wild, static, ordinary trainers | existing policies | existing | existing | existing |

The soft cap's anchors are the
[soft-cap curve](trainer-rating-party-progression.md#soft-level-cap-curve).
Well-known trainers' team level reuses them; their own v0 scalers are in
[Well-known trainers](well-known-trainers.md#trainer-scalers).

## What player TR drives

Each consumer reads `GetTrainerRating()` under its current policy and never
reads a well-known trainer's TR:

| Consumer | Owner |
| --- | --- |
| Soft level cap, experience reduction, obedience | [Party progression](trainer-rating-party-progression.md) ([PRD](../prds/trainer-rating-wild-encounter-scaling.md)) |
| Wild and static encounter levels | [Wild encounter scaling](trainer-rating-wild-encounter-scaling.md) |
| Ordinary trainers and Gym members (battle-start snapshot) | [Trainer party scaling](trainer-party-scaling.md) |
| Poké Mart stock (counter-open snapshot, unchanged thresholds) | [Global TR Poké Marts](global-tr-pokemarts.md) |

Well-known trainers use the soft-cap anchors for team level but have their own
authored, fixed TR; nothing they do changes the getter, the saved high-water
value, or any consumer above. Standalone builds are unchanged.

## Validation

- Scalers: first anchor at TR 0, strictly increasing TRs, non-decreasing
  values; interpolation and half-up rounding at every anchor, midpoint, and
  adjacent TR; step scalers hold between anchors; flat above the last anchor,
  including very large TRs.
- Target range: with the 80 clamps removed, every consumer returns today's
  value for TR 0–80 and its last-anchor value above 80.
- Formula and consumer checks stay with their owners listed above.

## Later

- How player TR changes: a reworked badge curve, renown, exploration or
  catching paths, and leagues as tests.
- Scalers that saturate beyond today's range (team quality, AI, Mart tiers).
  Guideline: content that pushes TR higher comes with a scaler that saturates
  later, so extra TR stays meaningful.
- The treadmill concern (world scaling vs player growth) and the widening
  world gap.

## References

- [Well-known trainers](well-known-trainers.md)
- [Leagues](leagues.md)
- [Party progression](trainer-rating-party-progression.md)
- [Wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Trainer party scaling](trainer-party-scaling.md)
- [Global TR Poké Marts](global-tr-pokemarts.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Player TR getter](../../game/src/trainer_rating.c)
