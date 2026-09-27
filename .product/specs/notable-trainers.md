# Notable trainers

PRD: [Notable trainers](../prds/notable-trainers.md)
Implemented: No. Today, the ROM keeps its existing Gym and league scaling
until adoption; the browser explorer is placeholder tooling.
Design status: v0 contract. The model is accepted; each trainer's growth
values (start TR, archetype, peak TR) and roster, and every anchor marked
placeholder, are catalog content under review.

## Ownership and scope

This specification is the single owner of the v0 notable trainer model: the
notable trainer inventory, the rule that routes every battle with a notable
character to their Trainer Rating (TR) and roster, trainer TR and its growth
with world progress, the archetypes, the v0 trainer scalers, rosters, team
resolution, the battle snapshot, and their validation. Consumers link here
rather than restating it.

- [Gym Leader scaling](gym-leader-scaling.md) owns badge-encounter coverage and
  is the single authority for battle construction (source-member identity,
  `AUTHORED`/`LEVEL_UP` moves, rewards and AI, randomizer precedence). Those
  rules apply to every notable trainer battle, not only Gym battles.
- [Leagues](leagues.md) owns league lineups and their lifecycle.
- [Player Trainer Rating](player-trainer-rating.md) owns the player's TR,
  the scaler definition, and the downward rule and shared evolution-level table
  ([evolution stages](player-trainer-rating.md#evolution-stages));
  [party progression](trainer-rating-party-progression.md) owns the level cap
  curve.
- [Trainer roster influence](trainer-roster-influence.md) is parked and not
  part of v0; it records the extension path from filler slots to filler pools
  and trades.

v0 supersedes every earlier NPC growth model in full; the only growth is
[growth with world progress](#growth-with-world-progress). There is no world
cap, `levelBase` or headroom, progress index, standing or bias, growth arcs or
arc seeds, dynamic filler picks, filler weights or scores, trades or
gifts, and none of the older `baselineTR`, badge checkpoints,
`effectiveTR`, or TR role bands. Player TR, the level cap, experience,
obedience, wild and static encounters, marts, regular trainers, and Gym
members read player TR under their own policies and v0 curves
([Player Trainer Rating](player-trainer-rating.md#player-tr-scalers-v0)).
Standalone builds are unchanged.

## Notable trainer inventory

The v0 inventory is 38 entries in the explorer catalog: 37 characters (the 23
singles Gym Leaders, the Kanto, Johto, and Hoenn Elite Four, Lance, Wallace,
Steven, and Blue) plus the Tate & Liza duo
([badge coverage](gym-leader-scaling.md#coverage-and-identity)). Red and every
other character are not notable trainers in v0 and keep their current policies.

**Tate & Liza** are one notable entry with one `characterId`, one set of growth
values (start TR, archetype, peak TR), and one ordered six-slot roster. They
are fought as their existing double battle: team size comes from the same
table, and both trainers' Pokémon are drawn from the shared roster in roster
order. Every notable rule applies to them (world progress, battle snapshot,
construction); they are league-ineligible because leagues are singles only
([Leagues](leagues.md#registry-and-eligibility)).

Mapping each encounter ID of these characters (Gym, overworld, rematch,
league, and story battles) to its `characterId` is an implementation inventory
task owned here. Exactly one battle policy owns an encounter; a mapped
encounter always uses this model.

## Trainer rating

- **Own TR, read from world progress.** The player has one TR and each notable
  trainer has their own. Player TR is never computed from a trainer's TR. A
  trainer's TR is computed from **world progress** (the player's TR, below)
  through that trainer's own growth values; resolving a trainer reads
  `GetTrainerRating()` for that and never reads party levels, badges, league
  wins, or another trainer.
- **Source of truth for every battle.** Gym, overworld, rematch, league, and
  story battles with a trainer all build from that trainer's TR and roster.
  Story battles include Blue's rival fights, Giovanni's Rocket battles, and the
  Saffron Dojo. There is no battle-specific adjustment, role bonus, or
  exception. Every encounter ID of a character resolves to one canonical
  `characterId`, one TR, and one roster.
- **Grows with world progress.** A trainer's TR rises as the player
  progresses, in that trainer's own shape, up to their peak TR
  ([growth](#growth-with-world-progress)). Rosters are the same in every save.
- **Uncapped.** Trainer TR uses the player's v0 scale and units: 24
  badges put the player at TR 160, and nothing caps it
  ([range](player-trainer-rating.md#range)).
- **Placeholders.** Every trainer's growth values are placeholders authored
  on the new scale, tuned in the explorer, and re-authored with playtesting
  against the [balance targets](#balance-targets).

## Growth with world progress

**World progress** is the player's TR as the world sees it: the value
`GetTrainerRating()` returns, with every source counted. A trainer's TR is a
pure function of world progress and catalog content. There are no ticks and no
saved trainer state: gains are proportional to the player's (a +10 badge moves
a trainer far, a +1 source barely), idling moves nobody, and a reload changes
nothing.

Each trainer authors:

| Field | Contract |
| --- | --- |
| Start TR | Non-negative integer: the trainer's TR at world progress 0. |
| Archetype | `steady`, `early bloomer`, `late bloomer`, `plateau`, or `rival`. |
| Peak TR | Integer ≥ start TR: the most the trainer can ever reach. Distinct from a scaler's ceiling TR. |

The five archetypes are global scalers over world progress
([scaler definition](player-trainer-rating.md#scalers)), interpolated, giving
a growth fraction in percent (0–100):

| Archetype | World progress 0 / 40 / 80 / 120 / 160 → growth % | Feel |
| --- | --- | --- |
| Steady | 0 / 25 / 50 / 75 / 100 | Keeps a fixed fraction of the player's pace |
| Early bloomer | 0 / 50 / 80 / 95 / 100 | Fast early, then slows: a mid-game wall |
| Late bloomer | 0 / 10 / 25 / 55 / 100 | Slow start, strong finish: a late challenge |
| Plateau | 0 / 60 / 100 / 100 / 100 | Reaches the peak early and stops: a veteran the player overtakes |
| Rival | 0 / 29 / 53 / 76 / 100, plus 15 at world progress 20 | Level with the player at the start, then about 10 TR ahead (with start 0, peak 170) |

The growth rule is the same for every notable trainer, whatever the
archetype. With `wp` the world progress, `fraction` the archetype scaler's
value, and floor division (halves round up):

```text
trainerTR = start + floor(((peak - start) * fraction(wp) + 50) / 100)
```

Past the last anchor the fraction stays at 100, so the trainer stays at peak
TR. Archetypes are authored globally for v0, and any trainer may use any of
them. Blue uses the rival archetype with start TR 0 and peak TR 170
(placeholder): TR 0 in Pallet Town, about 25 at world progress 20 (Cerulean),
about 50 at world progress 40, then about 10 ahead of the player until 170.

Examples (placeholder values):

| Trainer | wp 0 | wp 20 | wp 40 | wp 80 | wp 81 | wp 90 | wp 160 | wp 300 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Steady, start 20, peak 100 | 20 | 30 | 40 | 60 | 61 | 65 | 100 | 100 |
| Plateau, start 50, peak 90 | 50 | 62 | 74 | 90 | 90 | 90 | 90 | 90 |
| Rival (Blue), start 0, peak 170 | 0 | 26 | 49 | 90 | 92 | 100 | 170 | 170 |

From world progress 80, a +10 gain moves the steady trainer 5 TR and a +1
gain moves them 1 TR; the plateau trainer, already at peak, does not move.

## Trainer scalers

Team level and team size are scalers as defined in
[Player Trainer Rating](player-trainer-rating.md#scalers).

| Scaler | Form | Range | Ceiling | Ceiling TR |
| --- | --- | --- | --- | --- |
| Team level (notable trainers) | interpolated | Lv 5 → 100 | Lv 100 | TR 160 (placeholder) |
| Team size (notable trainers) | step | 1 → 6 | 6 | TR 96 (placeholder) |

**Team level** has its own anchors (placeholder). Below TR 40 it is no longer a
copy of the level cap curve: it starts lower, so early notable fights are fair.
From TR 40 up it equals the
[v0 level cap curve](trainer-rating-party-progression.md#v0-level-cap-curve),
so a trainer at TR 80 and a player capped at TR 80 (8 badges) mean the same
level. The level cap itself is unchanged.

```text
(0,5) (20,14) (40,28) (80,50) (120,75) (160,100)
```

**Team size** (step, placeholder): TR 0–10 → 1, 11–28 → 2, 29–43 → 3,
44–70 → 4, 71–95 → 5, 96+ → 6.

| TR | 0 | 10 | 11 | 20 | 25 | 29 | 50 | 71 | 85 | 95 | 120 | 160 | 300 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| teamLevel | 5 | 10 | 10 | 14 | 18 | 20 | 34 | 45 | 53 | 59 | 75 | 100 | 100 |
| teamSize | 1 | 1 | 2 | 2 | 2 | 3 | 4 | 5 | 5 | 5 | 6 | 6 | 6 |

## Rosters

Every notable trainer authors **exactly one ordered list of six roster
slots**.
Rosters do not differ by battle, starter, or difficulty: Blue has one fixed
six whatever the player's starter. The game has no per-difficulty trainer data
([difficulty constants](../../game/include/constants/difficulty.h));
challenge options (trainer items, trainer IVs/EVs including badge
scaling, level cap, experience, and so on) layer on top as they do today.

| Field | Contract |
| --- | --- |
| Species/form | Authored per roster slot, recommended at its final stage (validation warns otherwise; any stage is allowed, such as Blue's Eevee). Runtime never substitutes another line; below the stage's evolution level the member steps down ([evolution stages](player-trainer-rating.md#evolution-stages)). |
| `levelOffset` | Integer −6..0, default −2. |
| `isAce` | One bit. Slot 1 is always an ace; one to three aces per roster. Every other slot is a **filler slot**; there is no separate filler flag. |
| Moves policy | `AUTHORED` (exact moves for the authored stage) or `LEVEL_UP` (latest four level-up moves of the species at its level, per the existing constructor). A stepped-down member always uses `LEVEL_UP` for its stage. |
| Battle content | Held item, ability, nature, and IVs/EVs per the existing construction rules; unset fields use constructor defaults. |

Resolution for a trainer with TR `tr`:

```text
N           = teamSize(tr)
team        = entries[1..N]                          // the first N roster slots
memberLevel = clamp(teamLevel(tr) + entry.levelOffset, 1, 100)
species     = stepDown(entry.species, memberLevel)   // the downward rule
movesPolicy = species == entry.species ? entry.movesPolicy : LEVEL_UP
battleOrder = fillers(team) in reverse slot order,
              then aces(team) in reverse slot order    // slot 1 last
```

**Join order is list order.** The team is always the first N roster slots, so
a slot unlocks when the team grows to its position, and the author controls
the rhythm: aces and filler slots can alternate, or an ace can wait in slot 6
as a late reveal. Roster slot 1 is the **signature Pokémon**: an ace, present
from TR 0, and always fought last. Offsets stay per slot, whatever the flag.

**Battle order** sends filler slots first, in reverse list order, then aces,
in reverse list order. Example (Brock's placeholder roster, species at their
authored final stage): Steelix (ace), Rhyperior, Aerodactyl (ace), Kabutops,
Omastar, Golem (ace).

| Team size | Slot that joins | Battle order |
| ---: | --- | --- |
| 1 | Steelix (ace) | Steelix |
| 2 | Rhyperior | Rhyperior, Steelix |
| 3 | Aerodactyl (ace) | Rhyperior, Aerodactyl, Steelix |
| 4 | Kabutops | Kabutops, Rhyperior, Aerodactyl, Steelix |
| 5 | Omastar | Omastar, Kabutops, Rhyperior, Aerodactyl, Steelix |
| 6 | Golem (ace) | Omastar, Kabutops, Rhyperior, Golem, Aerodactyl, Steelix |

Members step down by level through the
[downward rule](player-trainer-rating.md#evolution-stages) and never evolve
forward. Authored moves belong to the authored stage: at or above it the member
uses them, and a stepped-down member uses its stage's `LEVEL_UP` set at its
level, as regular trainers do. Brock's slot 1 Steelix (offset 0) appears as
Onix until his team level reaches 35; his slot 2 Rhyperior (offset −2) is
Rhyhorn until Lv 42 and Rhydon from 42; his slot 6 Golem ace is Geodude below
Lv 25, Graveler from 25, and Golem from 38.

Early examples (placeholder content):

| Battle | Trainer TR | Team |
| --- | ---: | --- |
| Blue in Pallet Town (world progress 0) | 0 | Eevee Lv 5 |
| Blue at Cerulean (world progress about 20) | about 25 | two Pokémon at about Lv 18 |
| Brock at world progress 0 (start TR 20) | 20 | Onix Lv 14 (slot 1 Steelix, offset 0), Rhyhorn Lv 12 (slot 2 Rhyperior, offset −2) |

Source FRLG, Emerald,
and HNS parties are provenance and balance references; their levels never
override this resolver. As world progress rises, a trainer's team gains
levels and later roster slots; it never loses them.

## Battle snapshot

At battle setup, after resolving encounter identity, compute the trainer's TR at
the current world progress and capture the `characterId`, that TR, the world
progress, scaler, archetype, roster, and evolution-level table content
versions, and the resolved team
before constructing the opponent: per member, the roster slot index and every
resolved battle value the snapshot uses (species/form, level, moves, item,
ability, nature, IVs/EVs, and battle order). Reconstruction within the battle
reuses that snapshot; teardown clears it, and world progress gained during the
battle never changes it. A retry at the same world progress produces an
identical team (battle RNG may still differ); a retry after the player gained TR
uses the higher world progress. Entering a league computes every candidate's TR
at that moment and captures the selected trainers' battle snapshots in the
lineup, which stays locked until the league is won;
[Leagues](leagues.md#locked-lineup) owns that lifecycle. Invalid content or a
failed resolution fails preparation; never substitute player TR for a trainer's
TR, another trainer, or a random team.

## Validation

- Scalers: both pass the
  [scaler checks](player-trainer-rating.md#validation); team-level anchors
  equal the v0 level cap anchors from TR 40 up; team-size steps at 10/11,
  28/29, 43/44, 70/71, and 95/96.
- Rosters: exactly six roster slots per trainer; offsets in −6..0; slot 1 at
  offset 0 (`teamSize(0)` is 1); slot 1 is an ace and each roster has one to
  three aces; valid species/forms; a warning for each slot
  not at a final stage; `AUTHORED` slots have one to four legal moves for the
  authored stage; `LEVEL_UP` yields a usable move at every reachable level and
  stage, stepped-down stages included.
- Archetypes: each archetype passes the scaler checks, with anchors at world
  progress 0/40/80/120/160 (the rival adds 20), 0% at the first, and 100% at
  the last.
- Catalog: the inventory holds exactly the 38 v0 entries (37 characters and
  the Tate & Liza duo); each has one roster and valid growth values
  (non-negative integers, start TR ≤ peak TR); every enrolled encounter ID maps to exactly one `characterId`.
- Growth, for every trainer at world progress 0–200 and a very large value:
  TR is non-decreasing in world progress and never exceeds peak TR; a
  trainer equals start TR at world progress 0 and peak TR from world progress
  160; every archetype, the rival included, uses the one growth rule; results
  match the examples above, including the +10 and +1 steps and the early Blue
  and Brock fights (Brock's stepped-down Onix and Rhyhorn).
- Tate & Liza: their double battle draws both trainers' Pokémon from the shared
  roster in order, at the table's team size; they never enter a league lineup.
- Resolution report per trainer at world progress 0, 40, 80, 120, and 160: TR,
  size, member slots, species after stepping down, levels, moves, and battle
  order, each level in 1–100 and the battle order derived as above (filler
  slots, then aces, each in reverse list order; slot 1 last), matching the
  Brock example at every team size.
- Determinism: trainer TR and resolution are pure functions of world progress
  and content, independent of party, badges or league wins beyond their effect
  on player TR, save seed, query order, call history, reloads, and battle RNG.
- Snapshot: repeated construction within one battle and retries at the same
  world progress reproduce the same battle snapshot; TR gained mid-battle
  changes nothing; invalid content fails preparation without a fallback.

## Balance targets

Content constraints checked by the catalog report and playtesting, not runtime
rules (placeholders tuned in the explorer):

- **First league.** At world progress 80 (8 badges, level cap Lv 50), the
  league top five sit around TR 85–95 (team level about 53–59), so the first
  league is beatable at 8 badges.
- **Later leagues.** At world progress 120 the top five sit a little above
  the player's level cap (about cap +2 to +8): a real fight, not a wall. At
  160 the team-level and level-cap scalers both reach their Lv 100 ceiling, so
  the lineup meets the cap. Challenge beyond that needs a quality scaler
  (Later).
- **Start TR means how established a trainer is** when the journey begins.
  Blue (who leaves Pallet with the player) starts at TR 0. Gym Leaders are
  established: placeholder starts sit in TR 18–40 (opening team about Lv
  13–28, never below Lv 12), spread by archetype rather than original Gym
  order. The Elite Four and Champions start higher.
- **Gym ladder.** At every world progress point, some Gym Leaders sit below
  the player's TR (accessible), some near it, and some clearly above
  (challenges). The hardest leaders at 24 badges are late bloomers or
  high-peak steadies.
- **Rival.** Blue starts level with the player, pulls about 10 TR ahead by 4
  badges, and stays there until his peak TR of 170.
- **Early fights.** At world progress 0 notable fights are small and near the
  player's level (Blue brings one Pokémon at Lv 5).

The [explorer](../../devtools/ui/README.md#trainer-balance-explorer) is
placeholder evidence and predicts species, sizes, and levels only. Playtesting
owns combat balance (moves, items, AI). Finalize catalog content and playtest
evidence before enabling this policy; the existing Gym and league
implementations stay active until then.

## Open questions

- Each trainer's growth values and roster on the new scale, the archetype
  anchors, and the placeholder team-level low end and team-size steps (content
  review).

## Later

- Notable trainer status for more characters, such as Red.
- A definition of the "next Gym's highest/lowest level" cap in an open world.
- Archetypes rolled per save (seeded), and other per-save seeded variation of
  trainers.
- Per-trainer speed multipliers and custom growth curves.
- Weighting TR sources differently for world progress.
- Notable trainers also growing from their own battles.
- Filler pools (weighted picks with seeded variation and gameplay-flag
  modifiers) and trades that fill filler slots; aces stay fixed
  ([roster influence](trainer-roster-influence.md)).
- Quality scalers beyond Lv 100 (items, IVs/EVs, movesets, AI) with a higher
  ceiling TR, so extra TR stays meaningful.
- Offsets that shrink as TR rises.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Leagues](leagues.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Player progression](trainer-rating-party-progression.md)
- [Trainer roster influence](trainer-roster-influence.md)
