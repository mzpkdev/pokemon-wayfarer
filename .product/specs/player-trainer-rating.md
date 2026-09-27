# Player Trainer Rating

PRD: [Player Trainer Rating](../prds/player-trainer-rating.md)
Implemented: Partial. The current formula, its high-water save, and the
consumers below exist; the rescaled formula, uncapped TR, and the player-TR
scalers are the v0 target.
Design status: v0 accepted. The target formula is approved; every scaler
anchor is provisional and tuned by playtesting.

## Scope and ownership

This specification owns the player's TR as the foundation other systems read:
what it is, how it is earned, its scale, the **scaler** concept every
TR-driven property uses, the v0 player-TR scalers, and the list of what player
TR drives. Consumers keep their own current behavior and their own curves, and
link here for the shared model.

- The [interregional League circuit](wayfarer-interregional-league-circuit.md#trainer-rating)
  implements the current formula's badge and first-clear contributions.
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

### Formula (v0 target)

TR is a non-negative integer earned from global badges, whichever region
awarded them:

| Badges | TR per badge | TR after the last badge in the band |
| --- | ---: | ---: |
| 1–8 | +10 | 80 |
| 9–24 | +5 | 160 |

League wins give **0 TR**: leagues test the player and grant no power.
Losses, replays, repeated ceremonies, and Red add nothing. TR stays uncapped,
so future sources can add more above 160.

### Formula (current ROM)

Wayfarer TR is the global badge contribution (4 per badge for badges 1–4, 6
for 5–8, 1 for 9–24) plus +8 for each canonical first venue clear (Indigo,
Masters, Hoenn), clamped to 0–80. The exact contract and milestones are in the
[circuit contract](wayfarer-interregional-league-circuit.md#trainer-rating).
It stays in the ROM until the target is adopted.

### Range

| | Current ROM | v0 target |
| --- | --- | --- |
| Formula | Badges 4/6/1, +8 per first venue clear | Badges 1–8 +10, 9–24 +5, league wins 0 |
| All 24 badges, no clears | 56 | 160 |
| Value | Clamped to 0–80 on every read | Non-negative integer, no upper limit |
| Consumers | Clamp to 0–80, then look up their table | Look up their scaler, flat past its last anchor |
| Units | TR points | TR points (integers, no ×10) |

Nothing in the target hardcodes a maximum. Today's content happens to span
0–160; each scaler saturates at its own last anchor.

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

Anchors are provisional and sit on badge milestones (0, 4, 8, 16, and 24
badges):

| Scaler | Form | Anchors (TR → Lv) | Owner |
| --- | --- | --- | --- |
| Player soft cap | interpolated | (0,15) (40,28) (80,50) (120,75) (160,100) | [Party progression](trainer-rating-party-progression.md#v0-target-curve) |
| Wild encounter level target | interpolated | (0,6) (40,24) (80,40) (120,58) (160,78) | [Wild encounter scaling](trainer-rating-wild-encounter-scaling.md#v0-target-curve) |
| Ordinary (route) trainer baseline | interpolated | (0,9) (40,27) (80,44) (120,62) (160,82) | [Trainer party scaling](trainer-party-scaling.md#v0-target-baseline) |
| Poké Mart essentials tier | step | tiers 0–5 at TR 0, 10, 40, 70, 80, 120 | [Global TR Poké Marts](global-tr-pokemarts.md#v0-target-thresholds) |

Intent:

- **Early danger.** Around 4 badges the world presses up against the cap: wild
  Pokémon sit about 4 levels below it and route trainers about 1 below.
- **Late comfort.** At 24 badges routes are no threat: wild Pokémon sit about
  22 levels below the cap and route trainers about 18 below.
- **Challenge from well-known trainers.** The late challenge comes from Gyms,
  leagues, and overworld meetings with
  [well-known trainers](well-known-trainers.md#trainer-scalers), whose team
  level reuses the soft-cap anchors.

The Mart tiers keep today's badge milestones: each current threshold maps to
the target TR at the same badge count. Obedience, the half-experience soft
cap, Exp. Candy and Rare Candy rules, and species floors are unchanged,
because they are relative to the cap or authored.

## What player TR drives

Each consumer reads `GetTrainerRating()` under its own policy and never
reads a well-known trainer's TR:

| Consumer | Owner |
| --- | --- |
| Soft level cap, experience reduction, obedience | [Party progression](trainer-rating-party-progression.md) ([PRD](../prds/trainer-rating-wild-encounter-scaling.md)) |
| Wild and static encounter levels | [Wild encounter scaling](trainer-rating-wild-encounter-scaling.md) |
| Ordinary trainers and Gym members (battle-start snapshot) | [Trainer party scaling](trainer-party-scaling.md) |
| Poké Mart stock (counter-open snapshot) | [Global TR Poké Marts](global-tr-pokemarts.md) |

Well-known trainers use the soft-cap anchors for team level but have their own
authored, fixed TR; nothing they do changes the getter, the saved high-water
value, or any consumer above. Standalone builds are unchanged.

## Validation

- Scalers: first anchor at TR 0, strictly increasing TRs, non-decreasing
  values; interpolation and half-up rounding at every anchor, midpoint, and
  adjacent TR; step scalers hold between anchors; flat above the last anchor,
  including very large TRs.
- Target formula: every badge count 0–24 gives the tabled TR (10 per badge
  through 8, then 5 per badge to 160); league wins, losses, replays, and Red
  add nothing; the high-water value never drops.
- Target range: with the 80 clamps removed, every consumer reads its v0 scaler
  for TR 0–160 and its last-anchor value above 160.
- Consumer checks stay with their owners listed above.

## Later

- Further ways to earn TR: renown, and exploration or catching paths.
- Scalers that saturate beyond today's range (team quality, AI, Mart tiers).
  Guideline: content that pushes TR higher comes with a scaler that saturates
  later, so extra TR stays meaningful.
- Further tuning of the treadmill (world scaling vs player growth) and the
  widening world gap; the v0 curves already let routes fall behind late.

## References

- [Well-known trainers](well-known-trainers.md)
- [Leagues](leagues.md)
- [Party progression](trainer-rating-party-progression.md)
- [Wild encounter scaling](trainer-rating-wild-encounter-scaling.md)
- [Trainer party scaling](trainer-party-scaling.md)
- [Global TR Poké Marts](global-tr-pokemarts.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Player TR getter](../../game/src/trainer_rating.c)
