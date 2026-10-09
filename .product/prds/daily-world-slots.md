# Daily world slots

Implemented: Partial. The item slots ([Item slots](#item-slots)) are implemented;
trainer slots, trainer groups, phone rematches and the dialogue audit are not.

Specifications: [Daily world slots](../specs/daily-world-slots.md),
[World items](../specs/world-items.md),
[Sevii exploration restoration](../specs/sevii-exploration-restoration.md),
[Phone rematches](../specs/phone-rematches.md)

Design status: draft. All numbers are placeholders for playtesting. Terms
follow the [glossary](player-trainer-rating.md#glossary) and the
[wild encounters v2 terms](wild-encounters-v2.md#terms), plus the terms below.

## Intent

Wayfarer's regular trainers and overworld items were placed for each original
game's story order. In an open world that order no longer exists. Every
trainer is a one-time fight and every item a one-time pickup, so a route you
have cleared is empty for good. Late in the game, routes are a row of fixed
fights you can't lose, and revisiting them gives you nothing.

Daily world slots turn each regular trainer spot and each item spot into a
slot that changes from day to day. Trainers and prize items, such as TMs,
are met first where they were placed. After that, and from the first day for
ordinary items, each day a spot holds a random find or nothing, or one of the
trainers of its kind you have already beaten in that region, or nobody.

Trainers stay people, not spawns. Each one is in at most one place a day, keeps
their phone number and rematches wherever they turn up, and tells you on the
phone where to find them. Routes keep feeling lived in, revisiting pays off,
and travel stays light because some spots are empty.

## Terms

- **Slot:** a regular trainer spot or an item spot on a map, keeping its
  position, facing and sight range.
- **Authored occupant:** the trainer or item placed in the slot today.
- **Rotating slot:** a dynamic spot, or a slot whose authored occupant is used
  up: the trainer is defeated or the prize is picked up.
- **Draw:** what a rotating slot holds on a given day: one candidate from its
  pool, or nothing.
- **Pool:** the candidates a rotating slot draws from.
- **Static prize:** an item spot holding a fixed, authored item that you find
  once.
- **Dynamic spot:** an item spot that draws a random find every day.
- **Class family:** a trainer class with its per-source copies merged, for
  example Hiker, Hiker (HNS) and Hiker (FRLG).
- **Trainer group:** the regular trainers of one class family in one region,
  together with their home slots.
- **Home slot:** the slot a trainer was authored in.
- **Wandering trainer:** a regular trainer you have beaten. Their home slot
  rotates, and they appear in their group's rotating slots.
- **Rematch team:** one of a trainer's stronger authored teams for later
  battles, up to five in total.

## Design

### Days

A day is the game clock's local calendar day, the same clock berries and other
daily events use. A new day only reaches a map the next time that map loads.

A slot's draw is fixed for the whole day. It depends on the save, the day, the
map and the slot. Reloading the map, saving and resetting, or leaving and
coming back on the same day gives the same draw. Changing the game clock
changes the day, as it does for every other daily event. Wayfarer will later move to an in-game clock; the day then follows that clock.

### Authored first

Every trainer slot and every static prize (see [Item slots](#item-slots)) starts
with its authored occupant and keeps it until it is used up. Prizes are
therefore never lost to a draw: each one is found exactly once, where it is
placed.

After that, the slot becomes a rotating slot from the next day. Dynamic spots
skip this step and rotate from the first day.

### Clearing a slot

A slot cleared today stays cleared until the next day. A trainer you beat stays
in place and says their post-battle line. An item spot you emptied stays
empty. Losing to a trainer clears nothing.

### Empty slots

Each day a rotating slot is empty with a flat 25% chance. Otherwise it draws
one candidate from its pool. The chance is the same in every region, reach and
Trainer Rating.

### Trainer slots

Only regular trainer slots rotate. These stay fixed under today's rules:

- Gym trainers, inside their Gyms.
- Rivals, villain bosses and admins, Elite Four members, Champions and
  notable trainers.
- Story trainers: villain grunts in story places, trainers gated by story
  flags, path blockers, and trainers whose script does more than battle.
  Offering a phone number or giving a one-time item doesn't count.
- Battle facility trainers, including the Battle Pyramid, Trainer Hill and
  Trainer Tower.

The classification reuses the regular-trainer classification that
[trainer party scaling](trainer-party-scaling.md#coverage-and-exclusions)
requires, and it must be inspectable in the same report. Phone numbers and
one-time gifts move with the trainer (see
[Phone numbers and rematches](#phone-numbers-and-rematches)).

#### Trainers are people

A trainer you haven't beaten waits at their home slot. Once you beat them,
they become a wandering trainer and their home slot starts rotating the next
day.

Each day, every trainer group shuffles its wandering trainers into its rotating
slots. As a result:

- **One place a day.** A trainer appears in at most one slot a day, anywhere in
  the world. Some days they appear nowhere.
- **Variety grows as you explore.** A group's rotating slots only draw from
  trainers you have beaten in that group. Early on a group may have one
  wandering trainer, who keeps returning to their own spot. Once you have
  cleared a region, every spot of that kind can hold any of its trainers.
- **Empty spots.** Each rotating slot is empty with the flat empty chance. A
  trainer left without a slot is away for the day.
- **Each candidate brings their own** name, sprite, party, dialogue, phone
  number and gifts. The slot keeps its own position, facing and sight range.
- **Pairs rotate as pairs.** Two-trainer slots, such as Twins or a Young
  Couple, form their own group and draw only pairs.

A wandering trainer in a rotating slot battles you again on any day you meet
them. Clearing them clears that slot for the day.

**Strength and rewards** follow the existing regular trainer rules: the
[regular trainer scaling](trainer-party-scaling.md) from the player's
Trainer Rating, normal prize money and normal experience. Beating a wandering
trainer adds no Trainer Rating.

#### Phone numbers and rematches

Phone numbers and rematches belong to the trainer, not the spot:

- **Asking for a number.** After you beat a trainer who has a phone number,
  they offer it. If you decline, they offer it again the next time you beat
  them, at home or wandering. Today HNS registers the number automatically
  after the first win, without asking.
- **Rematch teams.** A trainer with rematch teams always uses their latest
  unlocked one. A phone trainer unlocks their next team the way HNS phone
  trainers do today: they call you wanting a rematch, and you beat them. A
  trainer without a registered number (no number, or you declined) unlocks
  their next team as your Trainer Rating reaches a step: team 2 at 40, team 3
  at 80, team 4 at 120 and team 5 at 160 (placeholders; a trainer with fewer
  teams uses the lowest steps). A registered contact keeps the phone path.
- **Readiness travels with you.** Today an HNS phone trainer becomes ready for
  a rematch after you walk 255 steps on their home map. Under rotation they
  have no fixed map, so readiness builds up as you walk anywhere in their
  region.
- **Calls name today's place.** When a registered trainer calls, they say where
  they are today. HNS battle requests never name a place and its gift calls
  name the home route, so every battle-request call gains a line naming
  today's place, and every gift call names it instead of the home route. The set
  of ready trainers (and trainers holding a gift) is fixed when the day
  starts. Each of them is always given a slot that day, so a call never sends
  you to an empty spot. A trainer who becomes ready during the day is placed
  and calls from the next day, and beating a ready trainer clears their
  readiness without moving anyone that day. If their group has no rotating
  slot free that day, the call waits for the next day.
- **Gifts.** The nine HNS contacts who hand out items keep doing so from
  wherever they stand: their gift call names today's place, and the item is
  handed over at the rotating slot, not only in their home map.
- **Hoenn's trainers** join the phone through the separate
  [Hoenn phone rematches](hoenn-phone-rematches.md) design, and the FRLG
  trainers on Sevii and the Kanto coast through
  [Sevii and Kanto coast phone rematches](sevii-coast-phone-rematches.md).

#### Vs. Seeker

The Vs. Seeker retires. Wayfarer wired it up for the FRLG trainers it ported,
on Sevii (64 rematch families) and the Kanto coast, Routes 19–21 (9 families).
However, the item can't be obtained today: its only giver is FRLG's Vermilion
Pokémon Center, and Wayfarer uses HNS's Vermilion. Those rematches are
therefore unreachable, so retiring it takes nothing away from players.

Rotation already brings every beaten trainer back on later days, and rematch
teams unlock by phone or by Trainer Rating, so a Vs. Seeker would only add
same-day repeat battles, which the "wait a day" rule rules out. The Sevii and
coast families with stronger teams get phone numbers through the separate
[Sevii and Kanto coast phone rematches](sevii-coast-phone-rematches.md)
design, which reuses their Vs. Seeker teams.

### Item slots

Item spots come in two kinds with two different jobs:

- **Static prizes** give players something to remember and talk about. Each
  one is found exactly once, and together they should be worth wikis and
  forum threads arguing which trail to take first for a given goal.
- **Dynamic spots** make the world worth re-exploring. They respawn daily, are
  sometimes empty, and mix everyday supplies with better and special finds.

#### Static prizes

- **Earned, never handed out.** Every static prize sits in Wilds, Outlands or
  a dungeon, or is hidden somewhere non-obvious: a dead end, behind an
  obstacle, a puzzle corner. A visible ball on an open road is never a prize.
- **Danger sets the prize.** *Finds* (Wilds, dungeon entrances, hidden road
  spots) hold solid TMs, type-boosting and utility held items, and evolution
  items. *Treasures* (Outlands, deep dungeon floors, hard-to-reach Wilds) hold
  strong TMs and top held items such as Leftovers, Life Orb or Choice items.
  Each region has up to four *Legends*: signature rewards in its most remote
  or best-hidden places. A small region such as Alola may have none.
- **Trails.** Each region's prizes form a few named trails: geographic chains
  that grow more dangerous and build toward a playstyle, such as a special
  attacker, a bulky wall or a weather team. Trails start in different
  directions, so first journeys differ and players can debate the best one
  for their goal.
- **Worth the trip.** No filler. Weak or redundant authored items, such as
  duplicate stones or minor TMs, become dynamic spots unless a better prize
  replaces them.
- **Existing spots only.** A prize stays in its authored spot or moves to
  another existing item spot, ball or hidden, and the spot it leaves becomes
  dynamic. Key items, HMs and anything a script depends on never move.
- **After pickup**, a prize's spot becomes a dynamic spot from the next day.

The [world items spec](../specs/world-items.md) lists every region's prizes
and trails, Sevii included.

#### Dynamic spots

A dynamic spot draws from one broad pool from the first day. Its authored
consumable, such as a Potion on Route 29, is dropped. Each find rolls a tier,
then an item within the tier:

- **Regular:** everyday healing, Poké Balls, status cures, Repels, Escape
  Ropes.
- **Better:** stronger healing, Revives, Ultra Balls, PP restores, battle
  items, useful berries.
- **Special:** Rare Candy, PP Ups, vitamins, valuables such as Nuggets and
  Star Pieces, evolution stones, Heart Scales, Max Revives, and occasionally a
  TM.

Stones and TMs can therefore be both static prizes and rare dynamic finds.

The reach tilts the tier odds: Road leans Regular, Wilds a step up, and
Outlands and deep dungeon floors lean Better and Special. Special finds stay
possible everywhere. Finds don't improve with Trainer Rating: a Road spot on
day one and on day three hundred has the same odds. A region may lean its pool
toward local flavour, such as Apricorns in Johto or Shards in Hoenn.

A dungeon's entrance floor counts as Wilds and its deeper floors as Outlands.
A map with no reach of its own, such as a building, resolves one with the
[places without wild encounters](../specs/reach-assignments.md#places-without-wild-encounters)
rules. A hidden spot stays hidden, and the
Itemfinder finds hidden dynamic items the same way it finds authored ones.

## Boundaries

- Story content, Gyms, notable trainers and Leagues don't rotate. The
  repeatable challenge in the late game stays with
  [notable trainers](notable-trainers.md) and [Leagues](leagues.md).
- Authored trainer placement, parties and dialogue stay as they are. Items
  change only as [Item slots](#item-slots) describes: dynamic spots drop their
  consumables, and static prizes are curated and may move between existing
  spots.
- No new trainer spots or item spots are added, and none are moved.
- Berry trees, gift NPCs, Pickup and other existing daily item sources are
  unchanged.
- Trainer levels and party building belong to
  [regular trainer scaling](trainer-party-scaling.md), which sets them by
  reach.

## Balance

- **Placeholders:** the empty chance (25%) and the pool contents and weights.
- **Experience and money are renewable on purpose.** Waiting a day is the
  limit. The level cap keeps renewable experience in check. Prize money needs
  a check against Poké Mart prices once pools are tuned.
- **Rare Candy, vitamins and PP Ups** are rare in every reach, rarest on the
  Road. Each is a renewable source of permanent power, so their weights need
  the closest tuning.
- **Region size shapes variety.** Once a region is cleared, the median trainer
  group has about 7 trainers, and about 14 slots are alone in their group
  (2026-10-07 inventory). Before that, a group's variety is the number of its
  trainers you have beaten.

## Content

The 2026-10-07 inventory of the Wayfarer build, with the Battle Pyramid's
generated objects excluded:

- **Trainer slots:** 1,182 in total, of which about 719 look regular and 463
  look like story trainers. Regular slots by region: Hoenn 377, Johto 141,
  Kanto 97, Sevii 85, Alola 19. The classification report settles the final
  split.
- **Item slots:** 762 in total: 485 item balls and 277 hidden items. About
  155 hold non-consumable items (TMs, held items, key items, evolution stones
  and items, one HM), the raw material for the curated static prizes. Escape
  Ropes are consumables in Wayfarer and become dynamic. Hoenn and the FRLG
  Kanto ports keep every authored item.
- **Sevii items (prerequisite):** Sevii's 97 FRLG items (39 item balls and 58
  hidden items) were left out by the
  [exploration port](sevii-exploration-port.md) and never restored. The
  [Sevii exploration restoration](../specs/sevii-exploration-restoration.md)
  brings them back first, with the rest of what the port meant to keep, so
  Sevii's spots start with their authored items like every other region.
- **Dialogue audit (required):** a trainer's intro and post-battle lines move
  with them. Every candidate's lines are audited before it can rotate. Lines
  that name a place, landmark or local event get neutral wording, or the
  trainer leaves the pool.
- **Rematches:** 176 rematch families (a pair counts once) have rematch
  scripts, through three separate systems. This design puts all of them on the
  rules in [Phone numbers and rematches](#phone-numbers-and-rematches):
  - 39 Johto and Kanto trainers from HNS give a phone number and call for
    tiered rematches (Nicole is not compiled in Wayfarer).
  - 64 Hoenn trainers register for Emerald's Match Call. Wayfarer compiles the
    HNS rematch table, which has no Hoenn entries, so these rematches appear
    to be inert today.
  - 73 FRLG families, 64 on Sevii and 9 on Kanto's Routes 19–21, support
    Vs. Seeker rematches, but the Vs. Seeker can't be obtained in Wayfarer.

## Interactions

- **Trainer party scaling:** rotating trainers are regular trainers for
  scaling. This replaces that design's rule that adds no repeatable farming.
- **Wild encounters v2:** item pools follow the
  [reach assignments](../specs/reach-assignments.md).
- **Notable walkers:** walkers and slots are independent. A walker never
  stands on a slot's tile.
- **Randomizer settings:** the item and trainer randomizers apply to a draw the
  same way they apply to authored content.
- **Challenge settings** such as Nuzlocke and mono-type apply to rotating
  trainers as they apply to authored ones.

## Constraints

- **Wayfarer only.** Standalone builds are out of scope.
- **Fixed for the day.** A day's draw comes from the save, the day, the map and
  the slot, never from a fresh random roll on load.
- **Save:** the authored-first state reuses the existing trainer and item
  flags. The daily cleared state needs a few bits per slot, a few hundred bytes
  in total, reset when the day changes. It follows the RAM rules in
  `AGENTS.md`.
- **ROM:** pools are generated at build time into constant tables from the
  map events and trainer data, with a report of slots, pools and exclusions.
- **Sprites:** a slot takes its candidate's sprite when the map loads. No map
  shows more than 16 distinct regular trainer sprites today.
- **Testing:** debug and E2E builds can choose the day so a draw can be
  reproduced.

## Playtesting

- Do routes feel lived in rather than random? Do pools repeat too often in
  small regions such as Alola?
- Does 25% empty make cleared routes lighter without making them feel
  deserted?
- Is a daily trip to the Outlands for items worth the danger, or does it turn
  into a chore?
- Does renewable prize money upset the Poké Mart economy?

## Open questions

- Does a group with very few wandering trainers feel repetitive early on, and
  should the empty chance be higher in small groups?

## References

- [Trainer party scaling](trainer-party-scaling.md)
- [Wild encounters v2](wild-encounters-v2.md)
- [Global TR Poké Marts](global-tr-pokemarts.md)
- [Notable world simulation](notable-world-simulation.md)
- [Playthrough-seeded variation](playthrough-seeded-variation.md)
