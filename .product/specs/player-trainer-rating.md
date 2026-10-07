# Player Trainer Rating

PRD: [Player Trainer Rating](../prds/player-trainer-rating.md)
Implemented: Partial. Today's formula, its saved high-water value, and every
consumer below except notable-trainer world progress and league invitations
(v0) exist; the rescaled formula, uncapped TR, and the player-TR
scalers are v0.
Design status: v0 accepted. The v0 formula is approved; every scaler anchor
is a placeholder tuned by playtesting.

## Scope and ownership

This specification owns the player's TR as the foundation other systems read:
what it is, how it is earned, its scale, the **scaler** concept every
TR-driven property uses, the v0 player-TR scalers, the shared
[evolution stages](#evolution-stages), and the list of what player TR drives. Consumers keep their own current behavior and their own curves, and
link here for the shared model.

- The [interregional League circuit](wayfarer-interregional-league-circuit.md#trainer-rating)
  implements today's formula's badge and first-league-win contributions.
- [Wild encounter scaling](trainer-rating-wild-encounter-scaling.md#trainer-rating-lifecycle)
  implements the getter, its persistence, and the standalone builds' own
  progression.
- [Notable trainers](notable-trainers.md) own trainer TR, the team-level
  scaler, and the team-size step scaler; [Leagues](leagues.md) own league
  lineups.

## Player TR

- **Hidden.** TR is never shown to the player as a number; the Trainer Card
  and dialogue never display it.
- **One direction.** Player TR is never computed from a notable trainer's
  TR. Notable trainers read player TR as **world progress** through their own
  archetype ([growth](notable-trainers.md#growth-with-world-progress)).
- **Never decreases.** `GetTrainerRating()` derives a candidate from current
  facts, compares it with the saved value, and keeps the higher. A new
  Wayfarer game starts at 0; nothing lowers the saved value.

### Formula (v0)

TR is a non-negative integer earned from global badges, whichever region
awarded them:

| Badges | TR per badge | TR after the last badge in the band |
| --- | ---: | ---: |
| 1–8 | +10 | 80 |
| 9–24 | +5 | 160 |

League wins give **0 TR**: leagues test the player and grant no power.
Losses, replays and repeat wins, repeated ceremonies, and Red add nothing. TR stays uncapped,
so future sources can add more above 160.

### Formula (Today)

Wayfarer TR is the global badge contribution (4 per badge for badges 1–4, 6
for 5–8, 1 for 9–24) plus +8 for each canonical first league win (Indigo,
Masters, Hoenn), clamped to 0–80. The exact contract and milestones are in the
[circuit contract](wayfarer-interregional-league-circuit.md#trainer-rating).
It stays in the ROM until v0 is adopted.

### Range

| | Today | v0 |
| --- | --- | --- |
| Formula | Badges 4/6/1, +8 per first league win | Badges 1–8 +10, 9–24 +5, league wins 0 |
| All 24 badges, no league wins | 56 | 160 |
| Value | Clamped to 0–80 on every read | Non-negative integer, no upper limit |
| Consumers | Clamp to 0–80, then look up their table | Look up their scaler, flat past its last anchor |
| Units | TR points | TR points (integers, no ×10) |

Nothing in v0 hardcodes a maximum. Today's content happens to span 0–160;
each scaler reaches its ceiling at its own last anchor.

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
its own ceiling (the last value), reached at an authored ceiling TR (the last
anchor's TR). A **step** scaler instead holds each anchor's value until
the next anchor. Scalers are versioned catalog content; runtime never repairs
or extrapolates them.

### Player-TR scalers (v0)

Anchors are placeholders and sit on badge milestones (0, 4, 8, 16, and 24
badges):

| Scaler | Form | Anchors (TR → Lv) | Owner |
| --- | --- | --- | --- |
| Level cap | interpolated | (0,15) (40,28) (80,50) (120,75) (160,100) | [Party progression](trainer-rating-party-progression.md#v0-level-cap-curve) |
| Road level | interpolated | (0,5) (40,20) (80,38) (120,56) (160,74) | [Wild level scaling](wild-level-scaling.md#reach-levels) |
| Wilds bonus | interpolated | (0,4) (80,6) (160,11) | [Wild level scaling](wild-level-scaling.md#reach-levels) |
| Outlands bonus | interpolated | (0,8) (40,8) (80,12) (160,24) | [Wild level scaling](wild-level-scaling.md#reach-levels) |
| Gym member level curve (regular trainers use reach levels) | interpolated | (0,9) (40,27) (80,44) (120,62) (160,82) | [Trainer party scaling](trainer-party-scaling.md#v0-levels) |
| Poké Mart essentials tier | step | tiers 0–5 at TR 0, 10, 40, 70, 80, 120 | [Global TR Poké Marts](global-tr-pokemarts.md#v0-thresholds) |

Intent:

- **Early danger.** Around 4 badges the world presses up against the level
  cap: Outlands meet it, Wilds sit about 3 levels below it, Roads about 8 and
  regular trainers about 1.
- **Late comfort.** At 24 badges routes are no threat: Roads sit 26 levels
  below the level cap and regular trainers about 18 below. Outlands still sit
  about 2 below, so remote places stay dangerous.
- **Challenge from notable trainers.** The late challenge comes from Gyms,
  leagues, and every other battle with
  [notable trainers](notable-trainers.md#trainer-scalers), whose team level
  matches the level cap from TR 40 up.

The Mart tiers keep today's badge milestones: each current threshold maps to
the v0 TR at the same badge count. Obedience, the half-experience rule that
makes the level cap soft, and Exp. Candy and Rare Candy rules are unchanged,
because they are relative to the level cap.

## Evolution stages

A world-scaling asset shared by every scaled population. This section owns
the downward rule and the shared evolution-level table; consumers link here.

**Downward rule.** A Pokémon whose level is below its stage's evolution level
steps down its predecessor chain, one stage at a time, until the level supports
the stage. There is never forward evolution: a low stage at a high level stays
as it is. The one exception is a POKéMON the player traded to a notable
trainer, which evolves forward by these levels and never steps down below its
traded stage ([traded slot](notable-haunts.md#the-trainers-team-after-a-trade)).

**No baby forms.** Stepping down never enters a baby form, whatever the
evolution method: the chain ends at the first stage above the baby, which
stays as it is at any level, like a base species. Pikachu
never becomes Pichu, Snorlax never becomes Munchlax, and Jynx never becomes
Smoochum. This holds for every consumer below. The baby forms are listed with
the shared evolution-level table as catalog content.

**Evolution level.** The table covers only evolutions without a level in the
game data; level evolutions use the game's own levels. A non-level evolution
(trade, stone, friendship, or other) uses its one global entry in the **shared
evolution-level table**, so the downward rule treats it like a level evolution.
Examples (placeholders, reviewed as content):

| Evolution | Method | Evolution level |
| --- | --- | ---: |
| Onix → Steelix | trade | 35 |
| Staryu → Starmie | stone | 30 |
| Growlithe → Arcanine | stone | 35 |

So a Steelix below Lv 35 appears as Onix. Graveler → Golem is a level
evolution in the game data (Lv 38), so it is not in the table: a Golem at Lv 30
appears as Graveler (below 38, at least 25), and at Lv 12 as Geodude.

**Format.** One row per non-level edge: predecessor, successor, evolution
level. The table is versioned catalog content like the scalers; runtime never
repairs or infers it.

**Validation.** Every non-level edge in the evolution data has exactly one
level, except an edge out of a baby form, which has none; a table row for an
edge that already has a level in the game data fails; levels increase along a
line (each edge above the edge into its predecessor); the predecessor graph has
no cycles; a species with more than one possible predecessor has its ancestry
resolved explicitly, and an unresolved one fails validation; and no step-down
result is a baby form.

| Consumer | Uses it for |
| --- | --- |
| [Notable trainers](notable-trainers.md#rosters) | Roster slots, authored at final stages, stepped down by member level |
| [Regular trainers and Gym members](trainer-party-scaling.md#species-moves-and-per-pokémon-fields) | Authored species at the effective level |
| [Wild encounters](wild-level-scaling.md#stage-mix) | Ordinary non-randomized encounters at their encounter level, with the stage mix |

Today, regular trainers and wild encounters step down numeric level evolutions
only, and non-level evolutions have no reverse; the table is v0.

## What player TR drives

Each consumer reads `GetTrainerRating()` under its own policy and never
reads a notable trainer's TR:

| Consumer | Owner |
| --- | --- |
| Level cap, experience reduction, obedience | [Party progression](trainer-rating-party-progression.md) ([PRD](../prds/trainer-rating-wild-encounter-scaling.md)) |
| Wild encounter levels | [Wild level scaling](wild-level-scaling.md) |
| Regular trainers and Gym members (battle snapshot) | [Trainer party scaling](trainer-party-scaling.md) |
| Poké Mart stock (counter-open snapshot) | [Global TR Poké Marts](global-tr-pokemarts.md) |
| World progress: each notable trainer's TR (battle snapshot, or the event lineup when accepting or declining a league invitation) | [Notable trainers](notable-trainers.md#growth-with-world-progress) |
| League invitations: the qualification gate (TR 80 or more, placeholder) | [Leagues](leagues.md#qualification) |

Notable trainers use their own team-level scaler, equal to the level cap from
TR 40 up, applied to their own TR; nothing they do changes the getter, the saved value, or any consumer
above. Standalone builds are unchanged.

## Validation

- Scalers: first anchor at TR 0, strictly increasing TRs, non-decreasing
  values; interpolation and half-up rounding at every anchor, midpoint, and
  adjacent TR; step scalers hold between anchors; flat above the last anchor,
  including very large TRs.
- v0 formula: every badge count 0–24 gives the tabled TR (10 per badge
  through 8, then 5 per badge to 160); league wins (first or repeat), losses, replays, and Red
  add nothing; the saved value never decreases.
- v0 range: with the 80 clamps removed, every consumer reads its v0 scaler
  for TR 0–160 and its last-anchor value above 160.
- Evolution stages: the table passes the checks in
  [Evolution stages](#evolution-stages).
- Consumer checks stay with their owners listed above.

## Later

- Further ways to earn TR: renown, and exploration or catching paths.
- Scalers whose ceilings lie beyond today's range (team quality, AI, Mart
  tiers). Guideline: content that pushes TR higher comes with a scaler whose
  ceiling TR is higher, so extra TR stays meaningful.
- Further tuning of the treadmill (world scaling vs player growth) and the
  widening world gap; the v0 curves already let routes fall behind late.

## References

- [Notable trainers](notable-trainers.md)
- [Leagues](leagues.md)
- [Party progression](trainer-rating-party-progression.md)
- [Wild level scaling](wild-level-scaling.md)
- [Trainer party scaling](trainer-party-scaling.md)
- [Global TR Poké Marts](global-tr-pokemarts.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Player TR getter](../../game/src/trainer_rating.c)
