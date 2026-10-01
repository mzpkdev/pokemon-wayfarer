# Notable trainers

PRD: [Notable trainers](../prds/notable-trainers.md)
Implemented: No. Today, the ROM keeps its existing Gym and league scaling
until adoption; the browser explorer is placeholder tooling.
Design status: v0 contract. The model is accepted. Rosters are approved
content (draft v1: species, order, and aces); their battle content (move
pools, and items where a slot doesn't match its source party) is still
placeholder. Growth
values (start TR, archetype, peak TR), home regions, trait assignments, and
every anchor marked placeholder are catalog content under review.

**Catalog as design reference.** "Catalog" in this spec means the
[balance explorer](../../devtools/ui/README.md#trainer-balance-explorer)'s
data: the design reference for rosters, growth values, and scalers. It is not
the ROM's data source. At implementation the same content is written directly
in game code, and the validation rules here apply to that code.

## Ownership and scope

This specification is the single owner of the v0 notable trainer model: the
notable trainer inventory, home regions, willingness, the traits (traveller
and aloof), the
rule that routes every battle with a notable character to their Trainer Rating
(TR) and roster, trainer TR and its growth with world progress, the
archetypes, the v0 trainer scalers, rosters, move pools, team resolution, the
battle snapshot, and their validation. Consumers link here
rather than restating it.

- [Gym Leader scaling](gym-leader-scaling.md) owns badge-encounter coverage and
  is the single authority for battle construction (source-member identity,
  writing the resolved moves, rewards, randomizer precedence). Those
  rules apply to every notable trainer battle, not only Gym battles.
- [Trainer AI](trainer-ai.md) owns the AI flags of every notable trainer
  battle: the play style, AI skill, ace protection, and the boss flag.
- [Leagues](leagues.md) owns league lineups and their lifecycle.
- [Notable haunts](notable-haunts.md) places notable trainers in the
  overworld and owns the meaning of the buddy and reward pool
  values ([below](#haunt-values-buddy-reward-pool)).
- [Player Trainer Rating](player-trainer-rating.md) owns the player's TR,
  the scaler definition, and the downward rule and shared evolution-level table
  ([evolution stages](player-trainer-rating.md#evolution-stages));
  [party progression](trainer-rating-party-progression.md) owns the level cap
  curve.
- [Trainer roster influence](trainer-roster-influence.md) is parked and not
  part of v0; it records the extension path from filler slots to filler pools
  and trades.

The only growth is [growth with world progress](#growth-with-world-progress).
Player TR, the level cap, experience, obedience, wild and static encounters,
marts, regular trainers, and Gym members read player TR under their own policies
and v0 curves ([Player Trainer
Rating](player-trainer-rating.md#player-tr-scalers-v0)). Standalone builds are
unchanged.

## Notable trainer inventory

The v0 inventory is 38 entries in the explorer catalog: 37 characters (the 23
singles Gym Leaders, the Kanto, Johto, and Hoenn Elite Four, Lance, Wallace,
Steven, and Blue) plus the Tate & Liza duo
([badge coverage](gym-leader-scaling.md#coverage-and-identity)). Red and every
other character are not notable trainers in v0 and keep their current policies.

**Tate & Liza** are one notable entry with one `characterId`, one set of growth
values (start TR, archetype, peak TR), and one ordered six-slot roster. They
are fought as their existing double battle. The team size comes from the
duo's TR like any notable trainer's. Members come out in
[battle order](#rosters) (fillers first, aces last), as for every notable
trainer, and alternate between the two partners along that order, counting back from
the last member, who goes to Tate. So each partner's last Pokémon is an ace
where possible (always when the team has two aces or more). With the full team
the battle order is Grumpig, Xatu, Claydol, Gardevoir, Lunatone, Solrock: Tate
brings Xatu, Gardevoir, and Solrock, and Liza brings Grumpig, Claydol, and
Lunatone. At world progress 0 (TR 26, two Pokémon) Tate brings Solrock and
Liza Lunatone. Every notable rule applies to them (world progress, battle snapshot,
construction); they are league-ineligible, as opponents and as partners,
because leagues field trainers who battle alone
([Leagues](leagues.md#registry-and-eligibility)).

Mapping each encounter ID of these characters (Gym, rematch, league, and
story battles) to its `characterId` is an implementation inventory
task owned here. Exactly one battle policy owns an encounter; a mapped
encounter always uses this model.

## Home region and travel

Each entry authors a **home region** (`kanto`, `johto`, or `hoenn`).
Assignments follow lore and are reviewable content:

| Home region | Trainers |
| --- | --- |
| Kanto | Brock, Misty, Lt. Surge, Erika, Janine, Sabrina, Blaine, Giovanni, Lorelei, Bruno, Agatha, Lance, Blue |
| Johto | Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine, Pryce, Clair, Will, Koga, Karen |
| Hoenn | Roxanne, Brawly, Wattson, Flannery, Norman, Winona, Tate & Liza, Juan, Sidney, Phoebe, Glacia, Drake, Wallace, Steven |

A **location** is a place that picks which notable trainers turn up there, by
their willingness; in v0 the only locations are the leagues. Gym and story
battles are not locations: each always brings its own trainer. Each location
has a **location region** (one or
more regions), or is a **neutral location**, home to everyone. A trainer is
**at home** when the location region includes their home region or the
location is neutral, and **away** otherwise. For any trainer and location:

```text
travelCost  = 0 at home; away: 10 for a traveller, otherwise 80
willingness = max(5, 100 - travelCost - fatigue)
```

**Fatigue** is a location-specific penalty, 0 unless the location defines one;
in v0 only leagues do. [Leagues](leagues.md#selection-and-order) owns the
league location regions, league fatigue, and the league score that reads
willingness.

## Traits

A **trait** is an opt-in yes/no behaviour of a notable trainer, authored per
entry as a boolean that defaults to `false`. Traits are independent of each
other and of the archetype. v0 has two traits, **traveller** and **aloof**;
the [league lineup rule](leagues.md#selection-and-order) and
[haunt placement](notable-haunts.md#candidates) read them. Assignments
follow lore and are reviewable content.

### Traveller

`traveller: true` lowers the away travel cost from 80 to 10
([travel cost](#home-region-and-travel)). A trainer without the trait stays
close to home. The travellers:

| Home region | Travellers |
| --- | --- |
| Kanto | Brock, Misty, Giovanni, Blue, Bruno, Lance |
| Johto | Bugsy, Will |
| Hoenn | Brawly, Glacia, Drake, Wallace, Steven |

Everyone else is not a traveller.

### Aloof

`aloof: true` marks a trainer who won't join a league whose base lineup is
well below their level: the league lineup rule compares the trainer's team
level with the league's **base lineup level**. The aloof trainers (each a
strong trainer with a proud or distant persona):

| Aloof trainer | Reason |
| --- | --- |
| Lance, Wallace, Steven | Champions only grace elite leagues. |
| Agatha | Oak's proud old rival. |
| Glacia | Came to Hoenn seeking worthy opponents. |
| Clair | A proud dragon tamer. |
| Sabrina | Cold and distant. |
| Karen | "Strong Pokémon, weak Pokémon": disdains weak company. |

Everyone else is not aloof. The Tate & Liza duo is never aloof, since it is
league-ineligible. The aloof rule does not apply at the Sevii Masters
([Sevii Masters](sevii-masters.md#lineup)).

## Trainer AI

Each entry also authors exactly one **play style** (its battle identity) and
a boolean **boss flag** (`bossOmniscient`, Lance only in v0).
[Trainer AI](trainer-ai.md) owns both, the style assignments, and the AI
flags each battle resolves from them, the trainer's TR, and the resolved
team's aces.

## Haunt values: buddy, reward pool

Each entry also authors a **buddy** (one roster slot, 1-6). Notable trainers
also appear at haunts in the overworld, and
[Notable haunts](notable-haunts.md#trainer-values) owns what it means.
Each entry also authors a **reward pool**, an ordered list of items and
lessons with a from-TR gate each, which pays for finished haunt quests;
[Notable haunts](notable-haunts.md#rewards-and-claims) owns what it means
and its rules.

## Friendship

Each notable trainer, Tate & Liza included, has a **friendship score**: one
saved byte, 0-255, that starts at 0 and **never decreases**; additions stop
at 255. The **stage** is read from the score against fixed thresholds, and
nothing else moves a stage.

| Stage | Score at least | Notes |
| --- | ---: | --- |
| Stranger | 0 | The player has not talked to them. |
| Met | 1 | They know the player. |
| Friend | 20 | They give their phone number. |
| Close | 60 | The warmest stage. |

The thresholds are the same for every trainer in v0, and they are
placeholder data like the weights below.

Events only add weighted points (placeholders):

| Event | Points | Bounds |
| --- | ---: | --- |
| First talk | +1 | Once: when a [haunt](notable-haunts.md#talk-flow) talk starts with the trainer at Stranger. |
| Battle won | +20 | The first win over them, in any kind of battle. After that, a repeat win counts at most once per step of world progress: only if no repeat win over them has counted since world progress last rose. |
| Haunt quest completed | +10 | Each completed [quest](notable-haunts.md#quests), at most once per placement. |

Rules:

- **Points come only from explicit, bounded events.** Nothing passive adds
  any: repeat chats, time, world progress, and visits give nothing.
- **A first win always counts**, whatever the kind of battle (Gym, rematch,
  story, league, singles or a tag match, where both opponents count). A loss,
  a draw, fleeing, a declined league event, a battle beside the trainer as a
  partner, a One on one, Swap battle, or Handicap quest battle (each
  counts as the quest, win or lose), and debug
  battles add nothing. The first win alone reaches Friend.
- **Which battles count.** Haunts offer no battles of their own, so wins come
  from Gym, story, and league battles, and later from rematch spots. Only
  battles that can come again (league events, and future rematch spots) give
  repeat wins, and the step cap keeps them from being farmed: a repeat win
  counts only if world progress has risen since the last repeat win that
  counted.
- **Consumers read only the stage, never the events or the score.** They are the
  [haunts](notable-haunts.md#talk-flow) (greetings and quests) and [Sevii
  Masters](sevii-masters.md#phone-contacts): a trainer at Friend or above is a
  contact, and any contact can be the partner. The number is handed over when
  the score first crosses the Friend threshold, by any route (a win, or quests),
  with one system line that names the trainer. Tate & Liza keep a score, but
  they give no number and cannot be the partner, and they are not placed at
  haunts in v0.

Saved state: one score byte per notable trainer, one **first-win bit** per
notable trainer (the bit records that the first win was counted), and one
**repeat-win bit** per notable trainer (set when a repeat win counts, and
cleared for every trainer whenever world progress rises). New Game saves every
score at 0 and every bit clear. On load, drop the scores and bits of characters
no longer in the registry; a first-win bit set with a score under the Friend
threshold, a repeat-win bit set without the first-win bit, or a score for an
unknown character, is an invalid save.

Later sources of points (gifts, tag battles beside the trainer, trades,
partnering) are not in v0.

## Trainer rating

- **Own TR, read from world progress.** The player has one TR and each notable
  trainer has their own. Player TR is never computed from a trainer's TR. A
  trainer's TR is computed from **world progress** (the player's TR, below)
  through that trainer's own growth values; resolving a trainer reads
  `GetTrainerRating()` for that and never reads party levels, badges, league
  wins, or another trainer.
- **Source of truth for every battle.** Gym, rematch, league, and story
  battles with a trainer all build from that trainer's TR and roster.
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
  on the v0 scale, tuned in the explorer, and re-authored with playtesting
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
fraction in whole percent (0–100; the scaler rule rounds halves up). Prose names them as proper nouns (Brock is a
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
| Rival | 0 / 29 / 53 / 76 / 100, plus 15 at world progress 20 | Level with the player at the start, then about 10 TR ahead, 9–11 (with start 0, peak 170) |
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
(placeholder): TR 0 in Pallet Town, 26 at world progress 20 (Cerulean), 49 at
world progress 40, then 9–11 ahead of the player until 170 at world progress
160 (a whole percent of his 170-point range is 1.7 TR).

The Champions follow lore too: **Lance** is a Legend who never changes, at
TR 200 throughout (start TR = peak TR = 200); **Steven** is a Burst (start
TR 50, peak TR 195); and **Wallace** is a Star (start TR 48, peak TR 190),
all placeholder values. Their strength early in the journey is kept in check by
the [aloof](#aloof) trait, not by their archetype.

Examples (placeholder values):

| Trainer | wp 0 | wp 20 | wp 40 | wp 80 | wp 85 | wp 90 | wp 160 | wp 300 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Steady, start 20, peak 100 | 20 | 30 | 40 | 60 | 62 | 65 | 100 | 100 |
| Veteran, start 50, peak 90 | 50 | 62 | 74 | 90 | 90 | 90 | 90 | 90 |
| Rival (Blue), start 0, peak 170 | 0 | 26 | 49 | 90 | 95 | 100 | 170 | 170 |

The growth percent is whole, so the Steady at world progress 85 uses 53%
(53.125 rounded) and reaches 62, not the 63 an exact fraction would give.
From world progress 80, a +10 gain moves the Steady 5 TR (60 to 65) and a +1
gain moves them 0 or 1 TR (80 to 81 gives 61); the Veteran, already at peak,
does not move.

## Trainer scalers

Team level and team size are scalers as defined in
[Player Trainer Rating](player-trainer-rating.md#scalers).

| Scaler | Form | Range | Ceiling | Ceiling TR |
| --- | --- | --- | --- | --- |
| Team level (notable trainers) | interpolated | Lv 5 → 100 | Lv 100 | TR 160 (placeholder) |
| Team size (notable trainers) | step | 1 → 6 | 6 | TR 71 (placeholder) |

**Team level** has its own anchors (placeholder). Below TR 40 it sits below
the level cap curve, so early notable fights are fair.
From TR 40 up it equals the
[v0 level cap curve](trainer-rating-party-progression.md#v0-level-cap-curve),
so a trainer at TR 80 and a player capped at TR 80 (8 badges) mean the same
level. The level cap itself is unchanged.

```text
(0,5) (20,14) (40,28) (80,50) (120,75) (160,100)
```

**Team size** is a [step](player-trainer-rating.md#scalers) scaler
(placeholder): it holds each anchor's value until the next anchor. Anchors
`(0,1) (11,2) (29,3) (44,4) (57,5) (71,6)`: TR 0–10 → 1, 11–28 → 2,
29–43 → 3, 44–56 → 4, 57–70 → 5, 71+ → 6.

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
| Species/form | Authored per roster slot, recommended at its final stage (validation warns otherwise; any stage is allowed, such as Whitney's Ursaring, which appears as Teddiursa below its evolution level). Runtime never substitutes another line; below the stage's evolution level the member steps down ([evolution stages](player-trainer-rating.md#evolution-stages)). |
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
forward, except a POKéMON the player traded into a filler slot, which takes
the slot's species and evolves forward, never below its traded stage
([traded slot](notable-haunts.md#the-trainers-team-after-a-trade)).
Moves are resolved from the [move pool](#move-pools) against the
member's current species, so a stepped-down member needs no special case.
Brock (a Steady, start TR 25, peak TR 100) shows it: his slot 1 Steelix
(offset 0) appears as Onix until his team level reaches 35 (world progress
57, TR 52); his slot 2 Golem (offset −2) is Geodude below Lv 25, Graveler
from 25, and Golem from 38; his slot 6 Aerodactyl ace has no earlier stage
and joins as Aerodactyl at TR 71 (world progress 97). At world progress 0, 40,
80, 120, and 160 he is at TR 25, 44, 63, 81, and 100 with team level 18, 30,
41, 51, and 63, fielding Geodude, Onix; then Kabuto, Golbat, Graveler, Onix;
then Omanyte, Kabuto, Crobat, Golem, Steelix; then Omastar, Kabutops, Crobat,
Golem, Aerodactyl, Steelix at 120 and 160.

Early examples (placeholder content):

| Battle | Trainer TR | Team |
| --- | ---: | --- |
| Blue in Pallet Town (world progress 0) | 0 | Eevee Lv 5 |
| Blue at Cerulean (world progress 20) | 26 | Pidgey Lv 16 (slot 2 Pidgeot, offset −2), Eevee Lv 18 (slot 1 Umbreon, offset 0) |
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
  can use it joins, evolves, or reaches the level, a traded POKéMON
  included. Nothing is saved; resolution is a pure function of the team
  and the pool.
- **Stepping down.** No special case: learnability is always checked against
  the current species and its earlier forms, never a later form.
- **Randomizers.** When a species or learnset randomizer option applies, the
  pool is skipped and members keep the plain level-up moveset, as today;
  [Gym Leader scaling](gym-leader-scaling.md#overrides-and-enablement) owns
  that precedence.
- **Per-slot content.** Held items, abilities, natures, and IVs/EVs stay per
  roster slot. A POKéMON the player traded into a filler slot draws from the
  pool like any filler, but keeps its own ability, nature, and IVs
  ([traded slot](notable-haunts.md#the-trainers-team-after-a-trade)).

**Frustration rule.** When authoring a pool, draw on at most one
frustration category per trainer, and keep the trainer's most iconic one. A
move belongs to a category only when the frustration is its **main effect**,
including a damaging move whose effect always happens (Mud-Slap's accuracy
drop, Dynamic Punch's confusion, Thousand Waves' trap). A secondary chance
doesn't count: Hurricane's or Water Pulse's chance to confuse, or a damaging
move with a chance to lower accuracy (Muddy Water, Octazooka, Mirror Shot).

- sleep (Hypnosis, Sleep Powder, Spore, Lovely Kiss, Yawn, and the like);
- evasion (Double Team, Minimize, and accuracy drops such as Smokescreen,
  Sand Attack, or Mud-Slap);
- OHKO (Sheer Cold, Fissure, Horn Drill, Guillotine);
- trapping (Bind- and Wrap-style moves, Mean Look, Block, Fairy Lock,
  Thousand Waves, and the like);
- Perish Song;
- Destiny Bond;
- infatuation and confusion (Attract, Swagger, Confuse Ray, Sweet Kiss,
  Dynamic Punch, and the like).

Paralysis, burns, hazards, and Toxic are not categories, but a pool with
evasion never also lists Toxic or Toxic Spikes. The catalog script holds the
move-to-category map and rejects a pool that breaks the rule. The rule covers
pool entries; a member's natural level-up moves are not checked.

Example (Brock's draft pool, resolved against the game's learnsets): Bind,
Stealth Rock, Sandstorm (from Lv 20), Curse, Stone Edge, Earthquake, Rock
Slide, Heavy Slam, Rock Blast, Cross Poison, Explosion. Bind is his one
frustration category (trapping). At world progress 0 his slot 1 ace, Onix at
Lv 18, picks first: it takes Bind and Curse, claims Stealth Rock, which it
already knows from its level-up moveset, and keeps Rage; Geodude keeps its
level-up moves; the other eight entries are dormant. Sandstorm has a from
level and wakes for Onix at Lv 20 (world progress 6). Entries without a from
level wait for a member's natural learn level (an earlier form's level-up
counts): Rock Blast wakes for Graveler at world progress 46, Earthquake at 60,
and Explosion at 70; Heavy Slam wakes for Golem and Cross Poison for Crobat
at 76; Stone Edge wakes for Golem at 92. Rock Slide never wakes: its only
level-up learner on the roster, Onix and Steelix, already holds four pool
moves by the time it could learn it.

## Battle snapshot

At battle setup, after resolving encounter identity, compute the trainer's TR at
the current world progress and capture the `characterId`, that TR, the world
progress, scaler, archetype, roster, evolution-level table, move pool, and
learnset content versions, and the resolved team
before constructing the opponent: per member, the roster slot index and every
resolved battle value the snapshot uses (species/form, level, moves, item,
ability, nature, IVs/EVs, and battle order). Resolution reads the trainer's
traded slot, if any, whose rules
[Notable haunts](notable-haunts.md#the-trainers-team-after-a-trade) owns.
Reconstruction within the battle reuses that snapshot, and the battle's AI
flags are resolved alongside it
([Trainer AI](trainer-ai.md#runtime-and-the-override-point)); teardown clears it, and world progress gained during the
battle never changes it. A retry at the same world progress produces an
identical team (battle RNG may still differ); a retry after the player gained TR
uses the higher world progress. Accepting a league invitation computes every
eligible trainer's TR and league score at that moment and captures the
selected trainers' battle snapshots (and at the Sevii Masters the
[partner's](sevii-masters.md#event-lineup)) in the event lineup, frozen for
that event; there the league state's
[content versions](leagues.md#saved-state) stand in for each snapshot's own. Declining also computes the event lineup (and the league scores) at
that moment, but captures no teams.
[Leagues](leagues.md#event-lineup) owns that lifecycle. Invalid content or a
failed resolution fails preparation; never substitute player TR for a trainer's
TR, another trainer, or a random team.

## Phone contacts

A notable trainer's phone number comes when their [friendship](#friendship)
first reaches Friend, and a contact is a trainer at Friend or above. Sevii
Masters owns the phone side
([phone contacts](sevii-masters.md#phone-contacts)): it uses contacts to ask
a partner.

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
  the Tate & Liza duo); each has one roster, one home region, a boolean
  `traveller` and `aloof` trait, one play style and a boolean boss flag
  ([Trainer AI](trainer-ai.md#validation)), and valid growth values
  (non-negative integers, start TR ≤ peak TR, and peak TR = start TR for a
  Legend); no Gym Leader is a Legend; every enrolled encounter ID maps to exactly one `characterId`.
- Aloof: the aloof trait is `true` exactly for the trainers in the
  [aloof table](#aloof), each league-eligible.
- Traveller: home regions match the [home region table](#home-region-and-travel)
  and the traveller trait is `true` exactly for the trainers in the
  [traveller table](#traveller); for every trainer, willingness is 100 at
  home, 90 (traveller) or 20 (not a traveller) away, and never below 5 with
  fatigue added.
- Growth, for every trainer at world progress 0–200 and a very large value:
  TR is non-decreasing in world progress and never exceeds peak TR; a
  trainer equals start TR at world progress 0 and peak TR from world progress
  160; every archetype, the Rival included, uses the one growth rule; results
  match the examples above, including the +10 and +1 steps and the early Blue
  and Brock fights (Brock's stepped-down Onix and Geodude).
- Tate & Liza: their double battle takes the team size from the duo's TR,
  sends members out in battle order (fillers first, aces last), alternating
  between the two partners counting back from the last member (Tate's), so
  each partner's last Pokémon is an ace where possible, matching the example
  above; they never enter a league lineup.
- Resolution report per trainer at world progress 0, 40, 80, 120, and 160: TR,
  size, member slots, species after stepping down, levels, moves, dormant
  entries, and battle order, each level in 1–100 and the battle order derived as above (filler
  slots, then aces, each in reverse list order; slot 1 last), matching the
  Brock example at every team size.
- Friendship: scores start at 0 and never decrease, cap at 255, and the stage
  follows the thresholds (0, 1, 20, 60); a first win adds 20 once in every kind
  of battle (both opponents of a tag match), a repeat win adds 20 at most once
  per step of world progress and nothing more until world progress rises; a
  first talk adds 1 once; a quest adds 10; losses, declines, and partnering add
  nothing; the number is handed over once, at the Friend crossing; scores
  survive reloads and prune with removed characters.
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
  invitations (fatigue from the most recent resolved event), so the catalog
  report shows league scores and lineups per league event ([Leagues](leagues.md#balance-report)) instead of asserting a strength
  target; league balance comes later. At 160 the team-level and level-cap
  scalers both reach their Lv 100 ceiling; challenge beyond that needs a
  quality scaler (Later).
- **Start TR means how established a trainer is** when the journey begins.
  Blue (who leaves Pallet with the player) starts at TR 0. Gym Leaders are
  established: placeholder starts sit in TR 18–40 (opening team level about
  Lv 13–28, never below team level 12), spread by archetype rather than
  original Gym order: Sleepers and Stars at TR 18–26, Prodigies at 22–30,
  Steadies and Bursts at 24–34, Veterans and Comebacks at 30–40. No Gym
  Leader is a Legend. The floor is on team level: a filler may open lower
  (Winona's fillers open at Lv 11 on team level 13). The Elite Four and
  Champions start above the Gym band, at TR 41 or more (opening team level 29
  or more), except Blue, the Rival, who starts at 0: Will and Sidney 41,
  Lorelei 42, Phoebe 43, Koga and Glacia 44, Bruno and Drake 45, Karen 47,
  Wallace 48, Steven 50, Agatha 95, and Lance 200. Each region's lowest Elite
  Four start is Lorelei's (Kanto), Will's (Johto), and Sidney's (Hoenn), and
  the catalog script rejects an Elite Four or Champion start below 41, except
  the Rival's.
- **Gym ladder.** From the first badges on, some Gym Leaders sit near the
  player's TR and some clearly above (challenges), and from 8 badges some
  also sit below it (accessible). At the start every leader is above the
  player; at 4 badges none is below yet, but several are at or under the
  player's TR. The hardest leaders at 24 badges are Sleepers or
  high-peak Steadies.
- **Rival.** Blue starts level with the player, pulls about 10 TR ahead (9–11) by 4
  badges, and stays there until his peak TR of 170.
- **Early fights.** At world progress 0 notable fights are small and near the
  player's level (Blue brings one Pokémon at Lv 5).
- **Full teams.** Every notable trainer reaches a full team of six at their
  peak TR: every peak TR is at least 71.

The [explorer](../../devtools/ui/README.md#trainer-balance-explorer) is
placeholder evidence and predicts species, sizes, levels, moves, and the
resolved AI flags. Playtesting owns combat balance (moves, items, how the AI
plays). Finalize catalog content and playtest
evidence before enabling this policy; the existing Gym and league
implementations stay active until then.

## Open questions

- Each trainer's growth values on the v0 scale, the archetype anchors, and
  the placeholder team-level low end and team-size steps (content review).
- Battle content: author each trainer's move pool; items and abilities per
  roster slot.

## Later

- Notable trainer status for more characters, such as Red.
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
  modifiers) and authored trade offers that fill filler slots; aces stay
  fixed ([roster influence](trainer-roster-influence.md)). Haunt
  [trades](notable-haunts.md#trade) are the v0 slice.
- Quality scalers beyond Lv 100 (items, IVs/EVs, movesets) with a higher
  ceiling TR, so extra TR stays meaningful; AI skill tiers belong to
  [Trainer AI](trainer-ai.md#later).
- Offsets that shrink as TR rises.
- Gym arenas: a fixed field condition for both sides in a Gym battle, from
  the Gym as a place rather than the leader, like the leagues'
  [halls](leagues.md#halls).

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Leagues](leagues.md)
- [Sevii Masters](sevii-masters.md)
- [Notable haunts](notable-haunts.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Trainer AI](trainer-ai.md)
- [Player progression](trainer-rating-party-progression.md)
- [Trainer roster influence](trainer-roster-influence.md)
