# Notable trainers

PRD: [Notable trainers](../prds/notable-trainers.md)
Implemented: No. Today, the ROM keeps its existing Gym and league scaling
until adoption; the browser explorer is placeholder tooling.
Design status: v0 contract. The model is accepted. Rosters are approved
content (draft v1: species, order, and aces); their battle content (move
pools, and items where a slot doesn't match its source party) is still
placeholder. Growth
values (start TR, archetype, peak TR), home regions, travel styles, the aloof
trait, and every anchor marked placeholder are catalog content under review.

**Catalog as design reference.** "Catalog" in this spec means the
[balance explorer](../../devtools/ui/README.md#trainer-balance-explorer)'s
data: the design reference for rosters, growth values, and scalers. It is not
the ROM's data source. At implementation the same content is written directly
in game code, and the validation rules here apply to that code.

## Ownership and scope

This specification is the single owner of the v0 notable trainer model: the
notable trainer inventory, home regions, travel styles, willingness, and the
aloof trait, the
rule that routes every battle with a notable character to their Trainer Rating
(TR) and roster, trainer TR and its growth with world progress, the
archetypes, the v0 trainer scalers, rosters, move pools, team resolution, the
battle snapshot, and their validation. Consumers link here
rather than restating it.

- [Gym Leader scaling](gym-leader-scaling.md) owns badge-encounter coverage and
  is the single authority for battle construction (source-member identity,
  writing the resolved moves, rewards and AI, randomizer precedence). Those
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

## Home region and travel

Each entry authors a **home region** (`kanto`, `johto`, or `hoenn`) and a
**travel style**: `homebody` or `traveller`. Assignments follow lore and are
reviewable content:

| Home region | Travellers | Homebodies |
| --- | --- | --- |
| Kanto | Brock, Misty, Giovanni, Blue, Bruno, Lance | Lt. Surge, Erika, Janine, Sabrina, Blaine, Lorelei, Agatha |
| Johto | Bugsy, Will, Koga, Karen | Falkner, Whitney, Morty, Chuck, Jasmine, Pryce, Clair |
| Hoenn | Brawly, Glacia, Drake, Wallace, Steven | Roxanne, Wattson, Flannery, Norman, Winona, Tate & Liza, Juan, Sidney, Phoebe |

A **location** is anywhere a notable trainer can appear; in v0 the only
locations are the leagues. Each location has a **location region** (one or
more regions), or is a **neutral location**, home to everyone. A trainer is
**at home** when the location region includes their home region or the
location is neutral, and **away** otherwise. For any trainer and location:

```text
travelCost  = 0 at home; away: homebody 80, traveller 10
willingness = max(5, 100 - travelCost - fatigue)
```

**Fatigue** is a location-specific penalty, 0 unless the location defines one;
in v0 only leagues do. [Leagues](leagues.md#selection-and-order) owns the
league location regions, fatigue, and the league score that reads willingness.

## Aloof

Each entry authors an **aloof** trait (`true` or `false`), independent of
archetype and travel style: an aloof trainer won't join a league whose field
is well below their level. The trait only marks the trainer; in v0 only the
[league lineup rule](leagues.md#selection-and-order) reads it, comparing the
trainer's team level with the league's field level. Assignments follow lore
(each a strong trainer with a proud or distant persona) and are reviewable
content:

| Aloof trainer | Reason |
| --- | --- |
| Lance, Wallace, Steven | Champions only grace elite fields. |
| Agatha | Oak's proud old rival. |
| Glacia | Came to Hoenn seeking worthy opponents. |
| Clair | A proud dragon tamer. |
| Sabrina | Cold and distant. |
| Karen | "Strong Pokémon, weak Pokémon": disdains weak fields. |

Everyone else is not aloof. The Tate & Liza duo is never aloof, since leagues
are singles only.

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
  against the [balance targets](#balance-targets). Rosters are approved
  draft v1 content; only their battle content remains placeholder.

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
| Archetype | `steady`, `prodigy`, `sleeper`, `veteran`, `rival`, `legend`, `star`, `comeback`, or `burst`. |
| Peak TR | Integer ≥ start TR (equal to start TR for a Legend): the most the trainer can ever reach. Distinct from a scaler's ceiling TR. |

The nine archetypes are global scalers over world progress
([scaler definition](player-trainer-rating.md#scalers)), giving a growth
fraction in percent (0–100). Prose names them as proper nouns (Brock is a
Steady, a Sleeper finishes strong); content stores the lowercase identifier.
All are interpolated except Burst, a
[step](player-trainer-rating.md#scalers) scaler. Archetype scalers are
monotonic non-decreasing, so no trainer's TR ever drops:

| Archetype | World progress 0 / 40 / 80 / 120 / 160 → growth % | Feel |
| --- | --- | --- |
| Steady | 0 / 25 / 50 / 75 / 100 | Keeps a fixed fraction of the player's pace |
| Prodigy | 0 / 50 / 80 / 95 / 100 | Brilliant early, then evens out: fast early, then slows, a mid-game wall |
| Sleeper | 0 / 10 / 25 / 55 / 100 | Underestimated, strong at the end: slow start, strong finish, a late challenge |
| Veteran | 0 / 60 / 100 / 100 / 100 | Peaked already, you overtake them: reaches the peak early and stops |
| Rival | 0 / 29 / 53 / 76 / 100, plus 15 at world progress 20 | Level with the player at the start, then about 10 TR ahead (with start 0, peak 170) |
| Legend | 0 / 0 / 0 / 0 / 0 (anchors at 0 and 160 only) | Never changes, waits at the top: TR is start TR throughout, so peak TR equals start TR |
| Star | 0 / 10 / 50 / 90 / 100 | Explodes mid-journey: an S-curve with a slow start, then levels off |
| Comeback | 0 / 45 / 50 / 55 / 100 | Stalls, then returns stronger: fast early, stalls mid-journey, surges late |
| Burst (step) | 0 / 25 / 50 / 75 / 100 | Trains in jumps at milestones: Steady on average, but holds each value until the next anchor, so it jumps at 4, 8, 16, and 24 badges |

The growth rule is the same for every notable trainer, whatever the
archetype. With `wp` the world progress, `fraction` the archetype scaler's
value, and floor division (halves round up):

```text
trainerTR = start + floor(((peak - start) * fraction(wp) + 50) / 100)
```

Past the last anchor the fraction stays at its last value (100, or 0 for a
Legend), so the trainer stays at peak TR. Archetypes are authored globally for
v0, and any trainer may use any of them, except that no Gym Leader is a
Legend. Which archetype each trainer uses is catalog content: the lore-based
assignments are approved, and the growth numbers are placeholders. Blue is
the Rival, with start TR 0 and peak TR 170
(placeholder): TR 0 in Pallet Town, about 25 at world progress 20 (Cerulean),
about 50 at world progress 40, then about 10 ahead of the player until 170.

The Champions follow lore too: **Lance** is a Legend who never changes, at
TR 200 throughout (start TR = peak TR = 200); **Steven** is a Burst (start
TR 50, peak TR 195); and **Wallace** is a Star (start TR 48, peak TR 190),
all placeholder values. Their strength early in the journey is kept in check by
the [aloof](#aloof) trait, not by their archetype.

Examples (placeholder values):

| Trainer | wp 0 | wp 20 | wp 40 | wp 80 | wp 81 | wp 90 | wp 160 | wp 300 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Steady, start 20, peak 100 | 20 | 30 | 40 | 60 | 61 | 65 | 100 | 100 |
| Veteran, start 50, peak 90 | 50 | 62 | 74 | 90 | 90 | 90 | 90 | 90 |
| Rival (Blue), start 0, peak 170 | 0 | 26 | 49 | 90 | 92 | 100 | 170 | 170 |

From world progress 80, a +10 gain moves the Steady 5 TR and a +1
gain moves them 1 TR; the Veteran, already at peak, does not move.

## Trainer scalers

Team level and team size are scalers as defined in
[Player Trainer Rating](player-trainer-rating.md#scalers).

| Scaler | Form | Range | Ceiling | Ceiling TR |
| --- | --- | --- | --- | --- |
| Team level (notable trainers) | interpolated | Lv 5 → 100 | Lv 100 | TR 160 (placeholder) |
| Team size (notable trainers) | step | 1 → 6 | 6 | TR 71 (placeholder) |

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
44–56 → 4, 57–70 → 5, 71+ → 6.

| TR | 0 | 10 | 11 | 28 | 29 | 43 | 44 | 56 | 57 | 70 | 71 | 120 | 160 | 300 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| teamLevel | 5 | 10 | 10 | 20 | 20 | 30 | 30 | 37 | 37 | 45 | 45 | 75 | 100 | 100 |
| teamSize | 1 | 1 | 2 | 2 | 3 | 3 | 4 | 4 | 5 | 5 | 6 | 6 | 6 | 6 |

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
| Battle content | Held item, ability, nature, and IVs/EVs per the existing construction rules; unset fields use constructor defaults. |

Roster slots carry no moves: each member's moves come from the trainer's
[move pool](#move-pools).

Resolution for a trainer with TR `tr`:

```text
N           = teamSize(tr)
team        = entries[1..N]                          // the first N roster slots
memberLevel = clamp(teamLevel(tr) + entry.levelOffset, 1, 100)
species     = stepDown(entry.species, memberLevel)   // the downward rule
moves       = resolveMovePool(team, pool)            // after every member's species and level
battleOrder = fillers(team) in reverse slot order,
              then aces(team) in reverse slot order    // slot 1 last
```

**Join order is list order.** The team is always the first N roster slots, so
a slot unlocks when the team grows to its position, and the author controls
the rhythm: aces and filler slots can alternate, or an ace can wait in slot 6
as a late reveal. Roster slot 1 is the **signature Pokémon**: an ace, present
from TR 0, and always fought last. Offsets stay per slot, whatever the flag.

**Battle order** sends filler slots first, in reverse list order, then aces,
in reverse list order. Example (Brock's draft v1 roster, species at their
authored final stage): Steelix (ace), Golem, Crobat, Kabutops, Omastar,
Aerodactyl (ace).

| Team size | Slot that joins | Battle order |
| ---: | --- | --- |
| 1 | Steelix (ace) | Steelix |
| 2 | Golem | Golem, Steelix |
| 3 | Crobat | Crobat, Golem, Steelix |
| 4 | Kabutops | Kabutops, Crobat, Golem, Steelix |
| 5 | Omastar | Omastar, Kabutops, Crobat, Golem, Steelix |
| 6 | Aerodactyl (ace) | Omastar, Kabutops, Crobat, Golem, Aerodactyl, Steelix |

Members step down by level through the
[downward rule](player-trainer-rating.md#evolution-stages) and never evolve
forward. Moves are resolved from the [move pool](#move-pools) against the
member's current species, so a stepped-down member needs no special case. Brock's slot 1 Steelix (offset 0) appears as
Onix until his team level reaches 35; his slot 2 Golem (offset −2) is Geodude
below Lv 25, Graveler from 25, and Golem from 38; his slot 6 Aerodactyl ace has
no earlier stage and joins as Aerodactyl.

Early examples (placeholder content):

| Battle | Trainer TR | Team |
| --- | ---: | --- |
| Blue in Pallet Town (world progress 0) | 0 | Eevee Lv 5 |
| Blue at Cerulean (world progress about 20) | about 25 | two Pokémon at about Lv 18 |
| Brock at world progress 0 (start TR 25) | 25 | Onix Lv 18 (slot 1 Steelix, offset 0), Geodude Lv 16 (slot 2 Golem, offset −2) |

Source FRLG, Emerald,
and HNS parties are provenance and balance references; their levels never
override this resolver. As world progress rises, a trainer's team gains
levels and later roster slots; it never loses them.

## Move pools

Each notable trainer authors **one ordered move pool**: a per-trainer list,
separate from the roster, of the moves they like. An entry is a move plus an
optional **from level**. Without one, the entry follows the natural schedule:
it waits for the member's natural learn level. With one, it is a special
move that any learner, TM/tutor and egg moves included, gets from that level.
There is no shared flag: an entry goes to at most one member, so a move
listed twice can go to two members.

Resolution runs at battle start, after every member's species and level are
resolved (stepping down included):

1. Give each member its default level-up moveset: the existing constructor's
   last four level-up moves learned by its level.
2. Visit members **aces first, then fillers, each in list order**. For each
   member, walk the pool top to bottom and take every entry that is
   unassigned and eligible for the member at its level. Stop at four pool
   moves.
   - **Claim:** if the member already knows the entry's move from its
     level-up moveset, it claims the entry: the entry counts toward its four
     pool moves, and that move is protected from replacement.
   - **Otherwise** the member takes the entry as a new pool move.
3. Pool moves fill empty move slots, then **replace the oldest unprotected
   level-up moves first**, so claimed moves and the newest natural moves stay.

**Eligibility.** A member can learn a move when its **current species or any
earlier form in its evolution line** learns it by level-up, when its current
species learns it by TM/tutor, or when it is an egg move of the line's base
species. The **natural learn level** is the lowest level-up level across the
current species and its earlier forms; an evolution move counts as level 0.

- **No from level:** eligible only by level-up (own or earlier forms), once
  the member's level ≥ the natural learn level. A member that could get the
  move only by TM/tutor or as an egg move can't take the entry.
- **With a from level:** eligible by any of the ways above, once the member's
  level ≥ the from level.

Pool order is identity: top entries reach the aces first. It also routes
moves, so authors place the moves the ace can use above moves meant for a
later member, and the ace fills up first. TM, tutor, and egg moves have no
learn level, so they need a from level, and it alone decides their timing.

- **Dormant entries.** An entry no current member takes is dormant: no member
  can learn it; it has no from level and the members learn it only by
  TM/tutor or as an egg move (these need a from level); every eligible member is
  below its from level or, without one, its learn level; or every eligible
  member already knows it or has four pool moves. It wakes when a member that
  can use it joins, evolves, or reaches the level (Later, also when a traded
  Pokémon joins). Nothing is saved; resolution is a pure function of the team
  and the pool.
- **Stepping down.** No special case: learnability is always checked against
  the current species and its earlier forms, never a later form.
- **Randomizers.** When a species or learnset randomizer option applies, the
  pool is skipped and members keep the plain level-up moveset, as today;
  [Gym Leader scaling](gym-leader-scaling.md#overrides-and-enablement) owns
  that precedence.
- **Per-slot content.** Held items, abilities, natures, and IVs/EVs stay per
  roster slot. Traded Pokémon (Later) keep their own record moves and don't
  draw from the pool ([roster influence](trainer-roster-influence.md#trades)).

**Frustration rule.** When authoring a pool, draw on at most one
frustration category per trainer, and keep the trainer's most iconic one:

- sleep (Hypnosis, Sleep Powder, Spore, Lovely Kiss, Yawn, and the like);
- evasion (Double Team, Minimize, and accuracy drops such as Smokescreen or
  Sand Attack);
- OHKO (Sheer Cold, Fissure, Horn Drill, Guillotine);
- trapping (Bind- and Wrap-style moves, Mean Look, Block, and the like);
- Perish Song;
- Destiny Bond;
- infatuation and confusion (Attract, Swagger, Confuse Ray, Sweet Kiss,
  Dynamic Punch, and the like).

Paralysis, burns, hazards, and Toxic are not categories, but a pool with
evasion never also lists Toxic or Toxic Spikes. The catalog script holds the
move-to-category map and rejects a pool that breaks the rule. The rule covers
pool entries; a member's natural level-up moves are not checked.

Example (illustrative only; learnsets not checked): Brock's pool is Stone
Edge (from Lv 40), Earthquake, Stealth Rock, Iron Defense, Rock Slide,
Earthquake, Rock Slide. His slot 1 ace picks first and takes the entries it is
eligible for, up to four; Golem then takes from what is left. Two Earthquake
entries let two members carry it. The Earthquake entries have no from level,
so each waits until a member whose species learns Earthquake by level-up
reaches that learn level (an earlier form's level-up counts); a member that
could learn it only by TM never takes them. If Golem already knows Rock Slide
from its level-up moveset, it claims a Rock Slide entry and keeps the move.
Stone Edge has a from level, so it waits until some member that can learn it
at all, TM or egg move included, reaches Lv 40.

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
uses the higher world progress. Entering a league computes every eligible trainer's TR
at that moment and captures the selected trainers' battle snapshots in the
lineup, which stays locked until the league is won;
[Leagues](leagues.md#locked-lineup) owns that lifecycle. Invalid content or a
failed resolution fails preparation; never substitute player TR for a trainer's
TR, another trainer, or a random team.

## Validation

- Scalers: both pass the
  [scaler checks](player-trainer-rating.md#validation); team-level anchors
  equal the v0 level cap anchors from TR 40 up; team-size steps at 10/11,
  28/29, 43/44, 56/57, and 70/71.
- Rosters: exactly six roster slots per trainer; offsets in −6..0; slot 1 at
  offset 0 (`teamSize(0)` is 1); slot 1 is an ace and each roster has one to
  three aces; valid species/forms; a warning for each slot
  not at a final stage; every member has at least one usable move at every
  reachable level and stage, stepped-down stages included.
- Move pools: one ordered pool per trainer; every entry is a valid move; from
  levels are in 1–100; eligibility is checked against the member's current
  species and its earlier forms (without a from level, their level-up
  learnsets at the natural learn level, evolution moves at level 0; with one,
  also the current species' TM/tutor list and the base species' egg moves);
  a move the member already knows claims its entry and is never replaced. A
  warning, not a failure, for each entry no stage on the trainer's roster
  lines can learn, and for each entry without a from level that every learner
  on those lines gets only by TM/tutor or as an egg move.
- Archetypes: each archetype passes the scaler checks, with anchors at world
  progress 0/40/80/120/160 (the Rival adds 20; the Legend has only 0 and 160), 0%
  at the first, and 100% at the last (Legend: 0% at both); values are
  non-decreasing; Burst is a step scaler and every other archetype is
  interpolated.
- Catalog: the inventory holds exactly the 38 v0 entries (37 characters and
  the Tate & Liza duo); each has one roster, one home region, one travel
  style, one aloof trait, and valid growth values
  (non-negative integers, start TR ≤ peak TR, and peak TR = start TR for a
  Legend); no Gym Leader is a Legend; every enrolled encounter ID maps to exactly one `characterId`.
- Aloof: the aloof trait is `true` exactly for the trainers in the
  [aloof table](#aloof), each league-eligible.
- Travel: home regions and travel styles match the assignments table; for
  every trainer, willingness is 100 at home, 90 (traveller) or 20 (homebody)
  away, and never below 5 with fatigue added.
- Growth, for every trainer at world progress 0–200 and a very large value:
  TR is non-decreasing in world progress and never exceeds peak TR; a
  trainer equals start TR at world progress 0 and peak TR from world progress
  160; every archetype, the Rival included, uses the one growth rule; results
  match the examples above, including the +10 and +1 steps and the early Blue
  and Brock fights (Brock's stepped-down Onix and Geodude).
- Tate & Liza: their double battle draws both trainers' Pokémon from the shared
  roster in order, at the table's team size; they never enter a league lineup.
- Resolution report per trainer at world progress 0, 40, 80, 120, and 160: TR,
  size, member slots, species after stepping down, levels, moves, dormant
  move-pool entries, and battle order, each level in 1–100 and the battle order derived as above (filler
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

- **Leagues (informational).** League lineups depend on willingness and the
  entry sequence, so the catalog report shows league scores and lineups per
  league ([Leagues](leagues.md#balance-report)) instead of asserting a strength
  target; league balance comes later. At 160 the team-level and level-cap
  scalers both reach their Lv 100 ceiling; challenge beyond that needs a
  quality scaler (Later).
- **Start TR means how established a trainer is** when the journey begins.
  Blue (who leaves Pallet with the player) starts at TR 0. Gym Leaders are
  established: placeholder starts sit in TR 18–40 (opening team about Lv
  13–28, never below Lv 12), spread by archetype rather than original Gym
  order: Sleepers and Stars at TR 18–26, Prodigies at 22–30, Steadies and
  Bursts at 24–34, Veterans and Comebacks at 30–40. No Gym Leader is a
  Legend. The Elite Four and Champions start higher.
- **Gym ladder.** At every world progress point, some Gym Leaders sit below
  the player's TR (accessible), some near it, and some clearly above
  (challenges). The hardest leaders at 24 badges are Sleepers or
  high-peak Steadies.
- **Rival.** Blue starts level with the player, pulls about 10 TR ahead by 4
  badges, and stays there until his peak TR of 170.
- **Early fights.** At world progress 0 notable fights are small and near the
  player's level (Blue brings one Pokémon at Lv 5).
- **Full teams.** Every notable trainer reaches a full team of six at their
  peak TR: every peak TR is at least 71.

The [explorer](../../devtools/ui/README.md#trainer-balance-explorer) is
placeholder evidence and predicts species, sizes, and levels only. Playtesting
owns combat balance (moves, items, AI). Finalize catalog content and playtest
evidence before enabling this policy; the existing Gym and league
implementations stay active until then.

## Open questions

- Each trainer's growth values on the new scale, the archetype anchors, and
  the placeholder team-level low end and team-size steps (content review).
- Battle content: author each trainer's move pool; items and abilities per
  roster slot.

## Later

- Notable trainer status for more characters, such as Red.
- Overworld locations for notable trainers, read through willingness:
  homebodies stay in their home region and travellers roam.
- An overworld use of the aloof trait, such as aloof trainers keeping away
  from weak areas.
- Difficulty signposting: in-world hints about who is too strong (Gym guides,
  NPC gossip, a Trainer Card line), since TR is hidden.
- Testable feel targets for playtesting (first Gym winnable with a lightly
  trained team, first league at 8 badges near the level cap, late routes
  harmless to a capped team).
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
