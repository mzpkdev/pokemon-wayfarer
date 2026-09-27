# Well-known trainers

PRD: [Well-known trainers](../prds/well-known-trainers.md)
Implemented: No. The current ROM keeps its existing Gym and League scaling
until adoption; the browser explorer is provisional tooling.
Design status: v0 target contract. The model is accepted; each trainer's TR
and roster, and every anchor marked provisional, are catalog content under
review.

## Ownership and scope

This specification is the single owner of the v0 well-known trainer model:
the well-known inventory, the rule that routes every battle with a well-known
character to their TR plan, trainer TR, the v0 trainer scalers, rosters, team
resolution, the battle snapshot, and their validation. Consumers link here
rather than restating it.

- [Gym Leader scaling](gym-leader-scaling.md) owns badge-encounter coverage and
  is the single authority for battle construction (source-member identity,
  `AUTHORED`/`LEVEL_UP` moves, rewards and AI, randomizer precedence). Those
  rules apply to every well-known battle, not only Gym battles.
- [Leagues](leagues.md) owns league fields and their lifecycle.
- [Player Trainer Rating](player-trainer-rating.md) owns the player's TR and
  the scaler definition;
  [party progression](trainer-rating-party-progression.md) owns the soft-cap
  curve.
- [Trainer roster influence](trainer-roster-influence.md) is parked and not
  part of v0.

v0 supersedes every earlier NPC growth model in full: no world cap,
`levelBase` or headroom, progress index, standing or bias, growth arcs or arc
seeds, aces and fillers, filler scores or jitter, evolution by line, trades or
gifts, and none of the older `baselineTR`, badge checkpoints, `effectiveTR`, or
TR role bands. Player TR, the soft cap, experience, obedience, wild and static
encounters, marts, ordinary trainers, and Gym members read player TR under
their own policies and v0 curves
([Player Trainer Rating](player-trainer-rating.md#player-tr-scalers-v0)).
Standalone builds are unchanged.

## Well-known inventory

The v0 inventory is the 37 characters in the explorer catalog: the 23 singles
Gym Leaders (per [Gym Leader scaling](gym-leader-scaling.md#coverage-and-identity)),
the Kanto, Johto, and Hoenn Elite Four, Lance, Wallace, Steven, and Blue. Red,
Tate & Liza, and every other character are not well-known in v0 and keep their
current policies.

Mapping each encounter ID of these characters (Gym, overworld, rematch,
league, and story battles) to its `characterId` is an implementation inventory
task owned here. Exactly one battle policy owns an encounter; a mapped
encounter always uses this model.

## Trainer rating

- **Independent.** The player has one TR and each well-known trainer has their
  own. Neither is ever computed from the other. Resolving a trainer never reads
  `GetTrainerRating()`, the saved high-water rating, party levels, badges, or
  clears.
- **Source of truth for every battle.** Gym, overworld, rematch, league, and
  story battles with a trainer all build from that trainer's TR and roster.
  Story battles include Blue's rival fights, Giovanni's Rocket battles, and the
  Saffron Dojo. There is no battle-specific adjustment, role bonus, or
  exception. Every encounter ID of a character resolves to one canonical
  `characterId`, one TR, and one roster.
- **Authored and fixed.** Each trainer's TR is a catalog value. v0 has no rule
  that changes it, so a trainer fields the same team in every battle and every
  save. Known v0 consequence: Blue's early rival fights and his late ones use
  the same team.
- **Uncapped.** Trainer TR uses the player's target scale and units: 24
  badges put the player at TR 160, and nothing caps it
  ([range](player-trainer-rating.md#range)).
- **Placeholders.** Every well-known trainer's TR is a provisional placeholder
  authored on the new scale and is re-authored with playtesting. The
  [league balance target](leagues.md#balance-target) constrains the top of the
  catalog.

## Trainer scalers

Team level and team size are scalers as defined in
[Player Trainer Rating](player-trainer-rating.md#scalers).

| Scaler | Form | Range | Natural cap | Saturates at |
| --- | --- | --- | --- | --- |
| Team level (well-known trainers) | interpolated | Lv 15 → 100 | Lv 100 | TR 160 (provisional) |
| Team size (well-known trainers) | step | 2 → 6 | 6 | TR 96 (provisional) |

**Team level** uses the same anchors as the
[v0 player soft-cap curve](trainer-rating-party-progression.md#v0-target-curve),
so a trainer at TR 80 and a player capped at TR 80 (8 badges) mean the same
level:

```text
(0,15) (40,28) (80,50) (120,75) (160,100)
```

**Team size** (step, provisional): TR 0–15 → 2, 16–43 → 3, 44–70 → 4,
71–95 → 5, 96+ → 6. The steps line up with roughly Lv 20/30/45/60 on the level
curve.

| TR | 0 | 20 | 50 | 71 | 85 | 95 | 120 | 160 | 300 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| teamLevel | 15 | 22 | 34 | 45 | 53 | 59 | 75 | 100 | 100 |
| teamSize | 2 | 3 | 4 | 5 | 5 | 5 | 6 | 6 | 6 |

## Rosters

Every well-known trainer authors **exactly one ordered list of six entries**.
Rosters do not differ by battle, starter, or difficulty: Blue has one fixed
six whatever the player's starter. The game has no per-difficulty trainer data
([difficulty constants](../../game/include/constants/difficulty.h));
challenge-menu options (trainer items, trainer IVs/EVs including badge
scaling, level cap, experience, and so on) layer on top as they do today.

| Field | Contract |
| --- | --- |
| Species/form | Authored per entry; runtime never evolves or substitutes it. |
| `levelOffset` | Integer −6..0, default −2. |
| Moves policy | `AUTHORED` (exact moves) or `LEVEL_UP` (latest four level-up moves of the species at its level, per the existing constructor). |
| Battle content | Held item, ability, nature, and IVs/EVs per the existing construction rules; unset fields use constructor defaults. |

Resolution for a trainer with rating `tr`:

```text
N           = teamSize(tr)
team        = entries[1..N]                          // the first N entries
memberLevel = clamp(teamLevel(tr) + entry.levelOffset, 1, 100)
battleOrder = team reversed                          // entry N first, entry 1 last
```

Entry 1 is the **signature Pokémon**: present from TR 0 and always fought last.
Example: at TR 50 with offsets `0, −2, −2, −4, −1, −3`, the team is entries
1–4 at Lv 34/32/32/30, sent out in order 4, 3, 2, 1. Source FRLG, Emerald, and
HNS parties are provenance and balance references; their levels never override
this resolver. Because TR is fixed in v0, each trainer's team is fixed.

## Battle snapshot

At battle setup, after resolving encounter identity, capture the
`characterId`, TR, scaler and roster content versions, and the resolved team
before constructing the opponent: per member, the roster entry index and every
resolved battle field the plan uses (species/form, level, moves, item,
ability, nature, IVs/EVs, and battle order). Reconstruction within the battle
reuses that plan; teardown clears it. A retry produces an identical team
(battle RNG may still differ). League entry captures the selected trainers'
plans in the field, which stays locked until the venue is won;
[Leagues](leagues.md#frozen-field) owns that lifecycle. Invalid content or a
failed resolution fails preparation; never substitute player TR, another
trainer, or a random team. With fixed TRs every snapshot of a trainer is equal
in v0; the snapshot is the boundary a future TR-change rule must respect.

## Validation

- Scalers: both pass the
  [scaler checks](player-trainer-rating.md#validation); team-level anchors
  equal the v0 player soft-cap anchors; team-size steps at 15/16, 43/44,
  70/71, and 95/96.
- Rosters: exactly six entries per trainer; offsets in −6..0; at least one
  offset-0 entry among the first `teamSize(0)` entries (entry 1 at offset 0 is
  the simplest satisfying rule); valid species/forms; `AUTHORED` entries have
  one to four legal moves; `LEVEL_UP` yields a usable move at every reachable
  level.
- Catalog: the inventory holds exactly the 37 v0 characters; each has one
  non-negative integer TR and one roster; every enrolled encounter ID maps to
  exactly one `characterId`.
- Resolution report per trainer: TR, size, member entries, levels, moves, and
  battle order, each level in 1–100 and the order reversed.
- Determinism: resolution is a pure function of TR and content, independent of
  player TR, party, badges, clears, save seed, query order, and battle RNG.
- Snapshot: repeated construction within one battle and retries reproduce the
  same plan; invalid content fails preparation without a fallback.

The [explorer](../../devtools/ui/README.md#trainer-balance-explorer) is
provisional evidence and predicts species, sizes, and levels only. Playtesting
owns combat balance (moves, items, AI). Finalize catalog content and playtest
evidence before enabling this policy; the existing Gym and League
implementations stay active until then.

## Open questions

- Each trainer's TR and roster on the new scale, and the provisional
  team-size steps (content review).

## Later

- Well-known status for more characters, such as Red or Tate & Liza.
- A definition of the "next Gym's highest/lowest level" cap in an open world.
- How NPC TR changes: own battles, journeys, a world clock.
- Growth arcs and other per-save seeded variation of trainers.
- Aces vs fillers with dynamic filler picks (an `ace` flag returns).
- Player influence: modifiers, gifts, and trades
  ([roster influence](trainer-roster-influence.md)).
- Evolution by line instead of per-entry species.
- Quality scalers beyond Lv 100 (items, IVs/EVs, movesets, AI) that saturate
  later, so extra TR stays meaningful.
- Offsets that shrink as TR rises.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Leagues](leagues.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Player progression](trainer-rating-party-progression.md)
- [Trainer roster influence](trainer-roster-influence.md)
