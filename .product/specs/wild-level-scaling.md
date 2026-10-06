# Wild level scaling

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: No

Design status: draft. It sets how a wild Pokémon's level and stage come from
your Trainer Rating (TR) and the place you meet it. Every number below is a
placeholder for playtesting, but each one is exact, so the implementation has
nothing to guess.

## Scope

This spec defines:

- the level of each reach, as [scalers](player-trainer-rating.md#scalers)
  over your TR;
- the level of each dungeon floor, from its intent;
- the level spread of a single encounter;
- the stage mix, and the level limit of slots capped at an early stage;
- prowler minimum levels, and how Repel treats prowlers.

It doesn't cover:

- which reach each map has, or each dungeon's intent and floor order, which
  belong to [Reach assignments](reach-assignments.md);
- which species each slot holds, which belongs to the encounter table specs;
- the shared evolution-level table and the downward rule, which belong to
  [Player Trainer Rating](player-trainer-rating.md#evolution-stages);
- the level cap, obedience and experience, which keep their current rules;
- scripted, static, gift and legendary encounters.

## Behavior

### Reach levels

A place's level follows three scalers over your TR. Road is a level curve.
Wilds and Outlands are bonuses on top of it that grow as your TR grows:

| Scaler | Anchors (TR → value) |
| --- | --- |
| Road level | (0, 5) (40, 20) (80, 38) (120, 56) (160, 74) |
| Wilds bonus | (0, 4) (80, 6) (160, 11) |
| Outlands bonus | (0, 8) (40, 8) (80, 12) (160, 24) |

- **Road** is the Road level.
- **Wilds** is the Road level plus the Wilds bonus.
- **Outlands** is the Road level plus the Outlands bonus.

Each scaler interpolates and rounds as the
[scalers section](player-trainer-rating.md#scalers) defines, and stays flat
past its last anchor. Each is worked out on its own, then added.

What this means at a glance:

| Badges | TR | Level cap | Road | Wilds | Outlands |
| ---: | ---: | ---: | ---: | ---: | ---: |
| 0 | 0 | 15 | 5 | 9 | 13 |
| 2 | 20 | 22 | 13 | 18 | 21 |
| 4 | 40 | 28 | 20 | 25 | 28 |
| 6 | 60 | 39 | 29 | 35 | 39 |
| 8 | 80 | 50 | 38 | 44 | 50 |
| 12 | 100 | 63 | 47 | 54 | 62 |
| 16 | 120 | 75 | 56 | 65 | 74 |
| 20 | 140 | 88 | 65 | 75 | 86 |
| 24 | 160 | 100 | 74 | 85 | 98 |

- **Roads fall behind.** A Road starts 10 levels under your level cap and ends
  26 under it, so travel gets easier the further you go.
- **Wilds stay a step up.** They sit between the Road and the cap all game.
- **Outlands meet you at the cap.** From about two badges on, an Outlands
  place sits within 3 levels of your level cap, and its top rolls go over it.
- **The danger ladder holds all game.** A place's danger compared with your
  team stays about the same from the first badge to the last; only Roads get
  easier.

### Dungeon levels

A dungeon's intent gives the level of its first and deepest floors, built from
the reach levels at your TR. "Halfway" means the two levels added and halved,
rounded down:

| Intent | First floor | Deepest floor | Floor |
| --- | --- | --- | --- |
| Mild | Halfway between Road and Wilds | Wilds | None |
| Mild to moderate | Halfway between Road and Wilds | Halfway between Wilds and Outlands | None |
| Moderate | Wilds | Halfway between Wilds and Outlands | None |
| Moderate to hard | Wilds | Outlands | None |
| Hard | Halfway between Wilds and Outlands | Outlands + 3 | None |
| Brutal | Outlands | Outlands + 6 | Level 50 |

- **Floors in between climb evenly.** On a dungeon with *n* floors, floor *i*
  (the entrance is 0) has the first floor's level plus
  (deepest − first) × *i* / (*n* − 1), with halves rounding up. The floors are
  the steps of the dungeon's floor order in Reach assignments; maps listed
  together at one step, such as Mt. Silver's mountainside, item room and
  Moltres room, share a floor.
- **A single-floor dungeon, or one whose notes call it flat,** uses the middle
  of its range, rounded down, on every map.
- **The floor** is a minimum level for every map of the dungeon, whatever your
  TR. Only Brutal dungeons have one.
- A dungeon's outdoor maps take the level of their place in its floor order,
  like any other floor.

At a glance, as first floor → deepest floor:

| Badges | Mild | Mild to moderate | Moderate | Moderate to hard | Hard | Brutal |
| ---: | --- | --- | --- | --- | --- | --- |
| 0 | 7 → 9 | 7 → 11 | 9 → 11 | 9 → 13 | 11 → 16 | 50 → 50 |
| 4 | 22 → 25 | 22 → 26 | 25 → 26 | 25 → 28 | 26 → 31 | 50 → 50 |
| 8 | 41 → 44 | 41 → 47 | 44 → 47 | 44 → 50 | 47 → 53 | 50 → 56 |
| 16 | 60 → 65 | 60 → 69 | 65 → 69 | 65 → 74 | 69 → 77 | 74 → 80 |
| 24 | 79 → 85 | 79 → 91 | 85 → 91 | 85 → 98 | 91 → 101 | 98 → 104 |

Levels above 100 become 100.

### Above the level cap

Remote places may go above your level cap: Outlands' top rolls, Hard and Brutal
dungeons, and prowlers above their surroundings. A Pokémon caught above the
cap keeps its level and follows the existing obedience rules, so it may
disobey until your cap catches up. That is the price of a strong catch from
somewhere dangerous. Roads and Wilds stay under the cap, apart from prowlers.

### Encounter level

A single encounter's level comes from its place's level:

1. **Spread.** Every method, fishing with any rod included, rolls the place's
   level −2 to +2, each equally likely. Rods differ only in which entries they
   favour.
2. **Prowler minimum.** A prowler takes the higher of the rolled level and its
   minimum level.
3. **Young levels.** A slot capped at an early stage keeps its level below the
   next evolution, as described under [Young levels](#young-levels).
4. **Limits.** The result is at least 1 and at most 100.

Abilities that favour higher wild levels, such as Pressure, Hustle and Vital
Spirit, treat the top of the spread as the top of a slot's level range, as the
engine already does. Safari Zones, the Bug-Catching Contest and Feebas's
fishing tiles use their place's level like any other table.

### Stage mix

The stage comes from the encounter level and the slot's stage cap:

1. The [downward rule](player-trainer-rating.md#evolution-stages) finds the
   highest stage, up to the cap, whose evolution level the encounter has
   reached. Non-level evolutions use the shared evolution-level table.
2. If the downward rule could step that stage down further, the encounter
   rolls once. The chance of keeping the stage is 10% at its evolution level
   and rises by 10% a level, so it is certain from 9 levels above. If the roll
   fails, the encounter is one stage lower.

So a Pokémon never appears below its stage's evolution level, and young and
grown Pokémon appear together for a while after each evolution level. For
example, a Pidgeotto-capped slot at level 20 is Pidgeotto 30% of the time and
Pidgey otherwise; from level 27 it is always Pidgeotto.

Babies never take part: the downward rule never steps into a baby, and a
baby slot is the baby.

### Young levels

A slot capped at a stage that can still evolve keeps young levels: its level
is at most one below that stage's lowest evolution level. A Caterpie slot is
at most level 6, and a Pikachu slot stays under Raichu's level in the shared
evolution-level table.

- **Babies** are at most level 10.
- **A prowler's minimum level wins** over this limit, so a Dratini slot can
  hold a level-30 Dratini.
- **A line that never evolves** has no limit.

### Prowler minimum levels

A prowler's minimum level follows its temperament and base stat total, as the
[prowlers spec](prowlers.md) lists them:

| Temperament | Minimum level |
| --- | --- |
| Harmless | None |
| Fierce, base stat total under 485 | 20 |
| Fierce, base stat total 485 or more | 25 |
| Dangerous | 30 |

A fierce line listed without a base stat total, such as Noibat's, takes 20.

Early in the game that puts fierce and dangerous prowlers well above their
surroundings. Wilds reach level 25 at four badges and Outlands pass 30 between
four and five, after which prowlers are simply rare finds. Kalos's rewards in
the Safari Zones and Sinjoh's residents at home have no minimum, as their
specs say.

### Running and Repel

- **Running follows the game's normal rules** everywhere, for prowlers too. A
  fast prowler early in the game can be hard to run from, which is part of
  its danger.
- **Repel treats prowlers like any wild Pokémon,** comparing its level with
  your lead's.

## Implementation notes

- **This replaces today's projection.** The v0 wild level curve, authored
  levels, the retention shape, the cumulative maximum, the per-table level
  offsets and the old species floors all go. A wild level is the place's
  level at your TR, then the steps above. Every scaler only rises, so a
  higher TR never lowers a place's level.
- **Pokémon Tower's ghost Marowak** uses today's projection, which goes. It
  takes its floor's level instead, with no spread. Other scripted and static
  encounters keep their own levels.
- **Each map needs its reach, and each dungeon map its floor index, floor
  count and intent,** generated from [Reach assignments](reach-assignments.md).
- **Prowler minimum levels are generated from the prowlers spec's lists.**
  A species' minimum applies wherever it appears as a prowler, at any stage
  of its line.
- **Randomized encounters** keep the place's level and spread but skip the
  stage mix, young levels and prowler minimums, as the randomizer does today.
- **DexNav, the Pokédex area screen and other readers of the wild population**
  use the same rules, so what they show matches what appears.
- **The shared evolution-level table must cover every non-level evolution of a
  species in the encounter tables.** Today it lacks Galarian Darumaka,
  Alolan Graveler, Floette's colours, Doublade, Phantump, Pumpkaboo's sizes,
  Sinistea and Milcery, so their young levels and stage mix have no level to
  use.

## Validation

- Every scaler: anchors, midpoints and adjacent TRs give the tabled values;
  every value stays flat above TR 160.
- Every reach and dungeon intent reproduces the tables above at their badge
  counts.
- Every dungeon floor's level lies between its first and deepest floors, and
  rises with depth.
- No encounter, at any TR, is below its stage's evolution level, above 100,
  or below a prowler's minimum.
- A young-level slot never appears at or above its next evolution level,
  apart from prowlers at their minimum.

