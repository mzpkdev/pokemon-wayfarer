# Daily world slots

Implemented: No

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
slot that changes from day to day. You meet the original trainer or item first.
After that, each day the slot shows a different trainer of the same kind from
the same region, a different item, or nothing at all. Routes keep feeling
lived in, revisiting pays off, and travel stays light because some spots are
empty.

## Terms

- **Slot:** a regular trainer spot or an item spot on a map, keeping its
  position, facing and sight range.
- **Authored occupant:** the trainer or item placed in the slot today.
- **Rotating slot:** a slot whose authored occupant is used up: the trainer is
  defeated or the item is picked up.
- **Draw:** what a rotating slot holds on a given day: one candidate from its
  pool, or nothing.
- **Pool:** the candidates a rotating slot draws from.
- **Class family:** a trainer class with its per-source copies merged, for
  example Hiker, Hiker (HNS) and Hiker (FRLG).

## Design

### Days

A day is the game clock's local calendar day, the same clock berries and other
daily events use. A new day only reaches a map the next time that map loads.

A slot's draw is fixed for the whole day. It depends on the save, the day, the
map and the slot. Reloading the map, saving and resetting, or leaving and
coming back on the same day gives the same draw. Changing the game clock
changes the day, as it does for every other daily event.

### Authored first

Every slot starts with its authored occupant and keeps it until it is used up.
Unique items, such as TMs, evolution stones, held items and key items, are
therefore never lost to a draw: each one is found exactly once, where it was
placed.

After that, the slot becomes a rotating slot from the next day.

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
- Trainers who give an item or register a phone number.
- Battle facility trainers, including the Battle Pyramid, Trainer Hill and
  Trainer Tower.

The classification reuses the regular-trainer classification that
[trainer party scaling](trainer-party-scaling.md#coverage-and-exclusions)
requires, and it must be inspectable in the same report.

A rotating trainer slot's pool is every regular authored trainer of the same
class family from the same region. Each candidate brings its own name, sprite,
party and dialogue. The slot keeps its own position, facing and sight range.

- **Pairs rotate as pairs.** Two-trainer slots, such as Twins or a Young
  Couple, draw only pairs from their class family. Single slots draw only
  single trainers.
- **No doubles on one map.** The same trainer never appears twice on one map
  on the same day. On another map, on the same day, they may.
- **Strength and rewards** follow the existing regular trainer rules: the
  [regular trainer scaling](trainer-party-scaling.md) from the player's
  Trainer Rating, normal prize money, and normal experience. Beating a
  rotating trainer adds no Trainer Rating.

### Item slots

Item balls and hidden items both rotate. A rotating item slot draws from its
reach's consumable pool:

- **Road:** everyday supplies, such as basic healing, status cures and Poké
  Balls.
- **Wilds:** a step up, such as stronger healing, Revives and Great Balls.
- **Outlands and deep dungeon floors:** the best consumables, such as Max
  Revives, Full Restores and Ultra Balls, with a small chance of rarer finds.

A dungeon's entrance floor uses the Wilds pool. Its deeper floors use the
Outlands pool. A map with no reach of its own, such as a building, uses its
surroundings: the dungeon it belongs to, or Road otherwise.

Rotating pools never contain unique items. A hidden item stays hidden.
The Itemfinder finds rotating hidden items the same way it finds authored ones.

## Boundaries

- Story content, Gyms, notable trainers and Leagues don't rotate. The
  repeatable challenge in the late game stays with
  [notable trainers](notable-trainers.md) and [Leagues](leagues.md).
- Authored placement, parties, dialogue and items stay as they are. This
  feature only adds what happens after the authored occupant is used up.
- No new trainer spots or item spots are added, and none are moved.
- Berry trees, gift NPCs, Pickup and other existing daily item sources are
  unchanged.
- Trainer levels and party building are unchanged. A separate design may
  later move trainer strength onto reaches.

## Balance

- **Placeholders:** the empty chance (25%) and the pool contents and weights.
- **Experience and money are renewable on purpose.** Waiting a day is the
  limit. The level cap keeps renewable experience in check. Prize money needs
  a check against Poké Mart prices once pools are tuned.
- **Rare Candy, vitamins and PP Ups** appear only in the Outlands and deep
  dungeon pools, at low weight. Each is a renewable source of permanent power.
- **Region size shapes variety.** With pools by class family and region, the
  median rotating trainer slot has about 7 candidates, and about 14 slots have
  none other than their authored trainer (2026-10-07 inventory).

## Content

The 2026-10-07 inventory of the Wayfarer build, with the Battle Pyramid's
generated objects excluded:

- **Trainer slots:** 1,182 in total, of which about 719 look regular and 463
  look like story trainers. Regular slots by region: Hoenn 377, Johto 141,
  Kanto 97, Sevii 85, Alola 19. The classification report settles the final
  split.
- **Item slots:** 762 in total: 485 item balls and 277 hidden items. 167 hold
  unique items: 58 TMs, 49 held items, 26 key items, 25 evolution stones,
  8 held evolution items and 1 HM.
- **Dialogue:** a trainer's intro and post-battle lines move with them. Lines
  that name their original place need an audit. They either get neutral
  wording or the trainer leaves the pool.
- **Rematches:** about 200 regular trainers have rematch scripts. Rotating
  slots replace rematches for those trainers.

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

- Should item pools also improve with Trainer Rating, the way Poké Mart stock
  does?
- Sevii has no item slots. The [exploration port](sevii-exploration-port.md)
  left items out, and the later Sevii work restored trainers and stories but
  not items. Should Sevii's FRLG items be restored before or with this
  feature?
- Should trainer rematch scripts be removed from rotating trainers or just
  left unused?

## References

- [Trainer party scaling](trainer-party-scaling.md)
- [Wild encounters v2](wild-encounters-v2.md)
- [Global TR Poké Marts](global-tr-pokemarts.md)
- [Notable world simulation](notable-world-simulation.md)
- [Playthrough-seeded variation](playthrough-seeded-variation.md)
