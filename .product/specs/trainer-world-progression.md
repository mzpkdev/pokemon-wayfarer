# Trainer world progression

PRD: [Trainer world progression](../prds/trainer-world-progression.md)
Implemented: No. The current ROM keeps its existing Gym and League scaling
until adoption; the browser explorer is provisional tooling.
Design status: Target contract. The standing/arc model is accepted; every
numeric default below is provisional catalog content (D3) unless stated.

## Ownership and scope

This specification is the single owner of NPC growth for enrolled Wayfarer
encounters: the world cap, the progress index, level headroom, per-trainer
standing, seeded growth arcs, team stages, level resolution, and the
Gym-encounter snapshot. Consumers link here
rather than restating it.

- [Gym Leader scaling](gym-leader-scaling.md) owns badge-encounter coverage and
  battle construction.
- [Circuit trainer pool](circuit-trainer-pool.md) owns league eligibility, role
  windows, home/visitor selection, rotation, and competitive profiles.
- [Seeded league circuit](seeded-league-circuit.md) owns signup, competition
  identity, entry and result transactions, and retries.
- [Player progression](trainer-rating-party-progression.md) owns the player's
  rating and soft cap, which this model reads only as a pure function.

The earlier NPC trainer-rating model is superseded in full: no `baselineTR`,
badge TR checkpoints, `leagueGrowth`, NPC TR-to-level anchors, `effectiveTR`,
90–110% growth modifier, or TR role bands. Player TR, soft cap, experience,
obedience, wild, mart, ordinary-trainer, and Gym-member policies are unchanged.
Standalone builds keep their existing contracts.

## World point and world cap

`B` is the count of distinct global badges (0–24). `L` is the lifetime
first-clear mask over Indigo, Sevii Masters, and Hoenn; `C = popcount(L)`
(0–3). Both come from the shared runtime's canonical facts, never from local
clear flags, titles, current-edition results, or encounter counts. A badge or
first clear commits through its existing reward transaction and affects only
later encounters. Replays, losses, reloads, and later edition clears add
nothing to `B` or `C`; completed editions advance only the progress index.

```text
milestoneTR(B, C) = clamp(the circuit producer's candidate rating for B
                          badges and C first clears, 0, 80)
worldCap(B, C)    = softCap(milestoneTR(B, C))
```

`softCap` is the player cap table and half-up interpolation in
[player progression](trainer-rating-party-progression.md#soft-level-cap-curve);
the candidate rating includes +8 per first clear. The resolver is pure in
`(B, C)`. It never calls `GetTrainerRating()`, reads or updates the saved
high-water rating, or looks at party levels, training, or play time.

| (B, C) | (0,0) | (4,0) | (8,0) | (8,1) | (16,1) | (16,2) | (24,2) | (24,3) |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| worldCap | 15 | 23 | 42 | 52 | 62 | 78 | 89 | 100 |

**NPCs follow the world, not the player's party.** The world cap is the level a
player would be capped at from milestones alone. Because it ignores the
player's actual rating and party, grinding or under-levelling cannot move any
NPC, while the same milestone still raises the player's cap and every trainer
together.

### Progress index

Arcs read a progress index `p` rather than `B`, so rivals keep developing
through the first few circuit editions:

```text
E = min(completedEditions, 3)
p = B + 8 * E                 // 0..48
```

`completedEditions` is the circuit runtime's committed count of fully
completed editions (all three venues won and ceremonies finished). Completing
edition 1 requires 24 badges, so edition 1 spans p = 0–24, edition 2 entries
use p = 32, edition 3 p = 40, and edition 4 onward p = 48, after which
standings stop changing (rotation still varies fields). Gyms read the same
`p`; in practice every Gym is beaten before edition 2, and rematches are not
enrolled.

### Level headroom

```text
HEADROOM        = 4                                 // provisional
levelBase(B, C) = min(worldCap(B, C), 100 - HEADROOM)
```

Every edition-1 field (entered with at most two first clears) has
`worldCap <= 96`, where `levelBase = worldCap` and nothing changes. At (24,3), where worldCap is 100,
the base is 96, so post-game fields stay ramped just below level 100:
contenders at or below 94, elites 95–97, and the headliner 98–100. Standing,
roles, and windows are unchanged because they read standing, not levels.

## Standing

Each canonical trainer has an integer standing in levels relative to the cap:

```text
standing(t, p)       = t.bias + arcDelta(t.arc, p)
aceLevel(t, B, C, p) = clamp(levelBase(B, C) + standing(t, p), 1, 100)
```

`C` enters only through `levelBase`; there is no per-trainer first-clear
growth. `bias` is authored per trainer in -6..+6. Role defaults:

| Role | Default bias | Notes |
| --- | ---: | --- |
| Gym Leader | -2 | Giovanni, Sabrina, Clair, Morty, Norman, Winona, and Juan default to -1 |
| Elite Four | 0 | |
| Champion | +2 | |
| Blue | +1 | With fast-only arcs, "always a step ahead" |

The -1 leaders give each region a Gym Leader who can headline in some saves,
while the Gym mean stays near -2. Role and title choose only the authored
default; runtime never derives bias from title, Gym order, home league, or
source Trainer ID.

## Growth arcs

An arc is an authored integer `arcDelta` tuple at progress checkpoints
p = 0/8/16/24/32/40/48. Arc IDs are stable and never reused:

| ID | Arc | 0 | 8 | 16 | 24 | 32 | 40 | 48 | Flavour |
| ---: | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | --- |
| 1 | steady | 0 | 0 | 0 | 0 | 0 | 0 | 0 | Keeps pace with the world |
| 2 | early | 0 | +3 | +2 | +1 | 0 | 0 | 0 | Rises fast, then settles at the world's pace |
| 3 | late | 0 | -2 | 0 | +3 | +3 | +2 | +2 | Slow start, strong finish |
| 4 | plateau | 0 | +1 | -1 | -3 | -3 | -2 | -1 | Veteran who stops improving |
| 5 | rival | 0 | +2 | +3 | +3 | +3 | +3 | +3 | Relentless climb |

Every arc is 0 at p=0, so each trainer's first encounter is identical in every
save. For neighboring checkpoints `(p0, a0)` and `(p1, a1)` around `p`, with
floor division (halves round toward positive infinity):

```text
d = p1 - p0
arcDelta = a0 + floor((2 * (p - p0) * (a1 - a0) + d) / (2 * d))
```

At p=48 use the final checkpoint. Examples: Brock (bias -2) at (0,0) has ace
13. Clair (bias -1) at (8,0) has ace 44 on early, 41 on steady. Blue on rival at
(16,1) has ace 62 + 1 + 3 = 66, and at (24,3) in edition 2 (p=32) has
96 + 1 + 3 = 100.

Each trainer authors `allowedArcs`: two or three lore-fitting arcs. Blue allows
only {rival, early}. Veterans (for example Bruno, Agatha, Lorelei, Pryce, Chuck,
Wattson) include plateau; rising stars (for example Whitney, Falkner, Clair,
Winona) include early. Build validation rejects empty, duplicated, or unknown
entries and sorts the list by arc ID, so source reordering changes nothing.

### Arc seed

The arc is one immutable choice per canonical trainer per save, derived from
the [shared seed framework](playthrough-seed-framework.md):

```text
arc = allowedArcs[Uniform(len(allowedArcs), growthArcKey)]
```

| Key field | Value |
| --- | --- |
| `domainId` | TRAINER_GROWTH = 2 |
| `decisionId` | GROWTH_ARC = 1 (renames GROWTH_RATE) |
| `decisionVersion` | 2 (draw layout) |
| `entityLo`, `entityHi` | canonical `characterId`, 0 |
| `occurrenceId`, `drawId` | 0, 0 |

The key's `decisionVersion` describes only the draw layout. It is separate
from the **growth-policy version**, which covers arc tuples, allowed-arc
lists, bias, and headroom. Pin the growth-policy version at new game, before
any enrolled encounter.
Derive lazily and without side effects; asking again returns the same arc.
Aliases, Gym profiles, and league appearances of one character share it. It
never rerolls on a badge, clear, loss, reload, region change, or edition. The
key contains no badge count, venue, edition, history, catalog position, party,
or player rating, so adding trainers cannot perturb existing arcs. Ordinary
Pokémon RNG is never used. Tuple, bias, or headroom changes bump only the
growth-policy version. Allowed-list changes bump the policy version too; the
draw key stays unchanged, so a changed list changes the outcome by design. A
save pinned to an unsupported policy follows prerelease save rejection rather
than migration.

### Arc hints

Arcs are presentation-visible only through hint lines keyed by
`(characterId, arc, phase)`, where `phase` is the arc segment containing `p`:
0–7, 8–15, 16–23, 24–31, 32–39, 40–47, or 48. Gym dialogue, NPC gossip, and league lineup previews select
from these lines. No UI shows arc names, bias, standing, or the cap gap. Hint
text is D3 content; a missing line falls back to the encounter's ordinary text.

## Stages and levels

Stages are selected by global badges: use the stage with the greatest authored
`minB <= B`. Thresholds are unique, increasing, within 0–24, and include 0.
Each stage references a complete authored profile with 2–6 members, explicit
species/forms, ace designation, battle order, level offsets, move policy,
items, abilities, stats, and AI/inventory. Proposed Gym defaults:

| Minimum B | 0 | 3 | 6 | 10 | 16 |
| --- | ---: | ---: | ---: | ---: | ---: |
| Party size | 2 | 3 | 4 | 5 | 6 |

League slots always use the trainer's reviewed six-member competitive profile,
whatever their Gym stage.

```text
memberLevel = clamp(aceLevel + member.levelOffset, 1, 100)
```

Offsets are integers in -30..0 with at least one ace at 0. Review source level
gaps rather than applying a universal rule. Author species and member changes
between stages explicitly: never infer evolution, pad a team, sample members,
or filter one six-member roster. Source parties are provenance and balance
references; their absolute levels never override this resolver.

For every arc and `C`, as `B` increases, party size and the lowest member level
must not drop. Validate authored content against this; never repair it at
runtime. Once `levelBase` stops rising at the headroom ceiling (post-game),
levels follow the arc alone and may fall between editions by design: a fading
early bloomer or a veteran past their peak is part of the living-rivals story.
Party size still never shrinks.

## Snapshot boundaries

### Gym encounters

After resolving encounter identity and variant, capture `(B, L)`, `p`, the
pinned growth-policy version, the derived arc, standing, worldCap and
levelBase, stage/profile IDs,
content versions, and the member level plan before constructing the opponent.
Reconstruction within the battle reuses that plan; teardown clears it. A retry
at unchanged milestones produces identical membership and levels (battle RNG
may still differ). A badge or first clear earned before the retry legitimately
advances the plan. The badge is awarded after the battle. Invalid content or a
failed derivation fails preparation; never substitute player TR, a default arc,
or another trainer.

### League competitions

Entry into a competition freezes, for that `(edition, venue)`, the world point,
`p_event`, worldCap and levelBase, every candidate's arc and standing, and the
resulting levels. A trainer fights at `levelBase_event + standing`, with no
per-role adjustment. A
loss retries the same frozen field; only a win advances. The
[seeded league circuit](seeded-league-circuit.md) and
[circuit trainer pool](circuit-trainer-pool.md) own that lifecycle, the role
windows on standing, and selection.

## Validation

- For every trainer × p 0–48 × C 0–3 × each allowed arc: arc interpolation
  and rounding at every checkpoint and adjacent value, clamps, headroom at
  (24,3), non-dropping party size and minimum level, profile completeness, and
  identical p=0 encounters across arcs.
- worldCap equals `softCap(milestoneTR)` at all 100 world points, including the
  reference table, and is independent of saved player TR and party.
- Arc derivation: key vectors, `Uniform` boundaries, canonical list ordering,
  stability across reloads, aliases, and editions, and independence from
  lookup order, catalog additions, and ordinary battle RNG.
- Gym/cap report: the ace-minus-cap distribution per B across all arcs. Target a
  Gym mean near -2 with spread within about ±3, no widening as `C` grows, and
  Blue always above 0. Where worldCap exceeds 96 (only (24,3)), the Blue and
  headliner checks compare against `levelBase` instead of worldCap, including
  the explorer's (24,3) Blue check.
- League role feasibility, fallback rate, and variety are owned by the
  [pool specification](circuit-trainer-pool.md), using standings from here.

The [explorer](../../devtools/ui/README.md#trainer-balance-explorer) is being
reworked to model arcs, the cap gap, and venue feasibility; until then it is
provisional evidence. It predicts species, sizes, and levels only. Playtesting
owns combat balance (moves, items, AI). Finalize catalog content, hint lines,
and playtest evidence before enabling this policy; the existing Gym and League
implementations remain active until then and are not alternative targets.
