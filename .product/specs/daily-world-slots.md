# Daily world slots

PRD: [Daily world slots](../prds/daily-world-slots.md)
Implemented: Yes, apart from the phone itself. The item half: the day, deterministic
draws, the saved daily state, the item spot generator and tables, the map-load and
Continue hooks for item balls, the spawn check, hidden items and the Itemfinder, and the
debug and E2E day and seed override. The trainer half: the trainer slot and group
generator (`game/tools/wayfarer_trainer_slots`), the dialogue audit with its reviewed
resolutions, who stands where today, the map-load and Continue hook, the battle and sight
paths, the day-start phone snapshots and `WhereIsTrainerToday`. Calls, phone numbers,
readiness and the rematch table growth belong to [Phone rematches](phone-rematches.md)
and are not implemented; [what the trainer half leaves for it](#what-the-trainer-half-leaves-for-the-phone)
lists the seams.

Design status: draft. Numbers are placeholders for playtesting. Item pools and
static prizes are in [World items](world-items.md); trainer levels are in
[trainer party scaling](trainer-party-scaling.md#v0-levels); phone numbers,
calls and rematch readiness are in [Phone rematches](phone-rematches.md).

## Scope

This spec covers the runtime behind daily world slots: the slot data, the day,
the daily draws for trainers and items, the save state, the map-load hook, the
battle and pickup paths, and the generated reports. It applies to
`IS_WAYFARER` only.

## Slot data

A build-time generator reads Wayfarer's compiled map events, the trainer
classification and [World items](world-items.md), and emits constant ROM
tables. Nothing is built at runtime.

- **Trainer slots:** one row per daily regular trainer: map, local id, home
  trainer id, range (a group id, or the home marker), flags (pair, home range,
  absence level) and, per object, the sprite and the authored script. A pair is
  one row and two objects (two local ids and two sprites, one double-battle
  trainer). The slot keeps its own position, facing, movement and sight range.
  The generator emits only daily slots; an `ORDINARY` trainer that is a rematch
  contact but not daily gets a row in a small table of homes, which is all
  `WhereIsTrainerToday` needs.
- **Absence levels:** a small generated table of percentages (`sWorldTrainerAbsence[]`,
  at most 8 distinct values). A slot's flags hold the index of its level, so a
  slot costs no extra byte.
- **Trainer groups:** one row per class family and region, split into a
  singles group and a pairs group, and one row per place type and region (and
  singles or pairs) for the trainers whose lines need a kind of place: a group
  of that type, mixing classes. A group lists its slots in a fixed order. A home
  range has no group row: its slot carries the reserved group id `0xFF` and the
  home flag, so the 255-group cap counts only class and place groups.
- **Item spots:** one row per item ball (map, local id) and hidden item (map,
  bg event index): kind (static prize or dynamic), the prize item and its
  existing flag for prizes, and the spot's reach tier from
  [World items](world-items.md#reach-of-an-item-spot). A Sevii spot is
  identified by its exploration row id (`exploration.<map>.object.N` or
  `exploration.<map>.bg.N`, N being the 0-based FRLG source index) or by its
  coordinates. The generator maps it to the compiled local id after the
  [Sevii restoration](sevii-exploration-restoration.md), and fails on a spot it
  can't find.
- **Pools:** the dynamic tier odds and weighted item tables from
  [World items](world-items.md#dynamic-pools).

Every slot has a dense slot index, which the save state uses.

### Which trainers are daily

One rule covers every daily trainer: **a beaten trainer is placed somewhere in
its range each day, and is battleable once there; each free slot is empty with
its absence chance.** A trainer's range is the group of slots it can stand in.
There are three kinds and no modes:

- **class:** the slots of its class family in its region (singles or pairs);
- **place:** the slots of one place type in its region, mixing classes, for a
  trainer whose lines need a kind of place;
- **home:** its own slot alone, for a trainer whose lines are about its own
  spot. Each day it is at home or away, never anywhere else.

Each slot has an **absence chance**, 25% by default. At 0% someone is always
there; the trainer is still battleable once a day after the first win.

A trainer is daily when it is `ORDINARY` in the
[classification manifest](trainer-party-scaling.md#classification-contract)
and isn't kept out by the PRD's list: Gym members, rivals, bosses, notable
trainers, facility trainers, story trainers, path blockers, and trainers whose
script does more than battle. Phone registration and one-time gifts don't keep
a trainer out; they move with it. Every other trainer is **not daily**: it
stays where it was authored, and the report gives the reason.

The generator proposes each slot's status from script analysis, and authors
review it in the report. A slot can be made not daily by a reviewed entry with
a reason (`reviewed.json`, by map prefix, trainer class or slot), and a slot's
absence chance can be set per trainer with a reason (`absence`). A not-daily
contact stays at home and still gets Battle and FoundItem calls. A daily
candidate must also pass the [dialogue audit](#dialogue-audit).

Script analysis marks a trainer not daily when its object is hidden by a flag,
is a buried trainer, has non-static graphics, or its script (followed through
gotos, calls and fall-through) does anything beyond battling, rematching,
registering a phone number, handing over a gift and speaking. The reviewed
lists add the S.S. Anne, S.S. Aqua and S.S. Tidal trainers (one-time ship
content), Pokemon Tower, the Trick House puzzle rooms, Silph Co., the Rocket
Hideouts, the Game Corner hideout entrance, the villain grunt classes, and the
two Route 26 trainers authored on two maps. Of 901 `ORDINARY` trainer slots
(1,051 trainer objects in all, pairs counting once per trainer), 659 are daily
(632 singles and 27 pairs, 686 objects): 525 with a class range, 20 with a
place range and 114 with a home range; the report lists why each of the others
is not daily.

**Place ranges.** A trainer whose lines need a kind of place ("this cave", "in
the desert", "this ship") has the range of that place type in its region,
mixing classes, so it can only stand in spots of that type. `reviewed.json`
assigns the type per trainer with the phrases that need it, and the generator
checks every slot against the type's rule in `place_types.json` (an underground
map, a map-name prefix such as the Abandoned Ship or the Pokemon Mansion, or a
metatile behaviour near the slot's object: deep sand for a desert, surfable
water within three tiles for a coast) and fails on a slot that is not that
kind of place. The types are cave, tower, mountain, ship, mansion, desert and
coast. A trainer has exactly one range. A place group of one slot only keeps
the trainer's own spot in the daily bookkeeping.

**Home ranges.** A trainer whose dialogue is bound to its own place (it names a
town, a route, a building or a landmark) has its own slot as its range. After
its first win it is at home or away each day (the slot's absence roll),
battleable once a day while there, and placed every day when it is ready or
holds a gift as the day starts. Its lines are never out of place. A home range
needs no group: the slot row carries the home marker (group `0xFF` and the home
flag), resolved as a range of one slot. The only daily-adjacent trainers left
out are those whose lines cannot survive absence at all: the Route 25
six-trainer challenge, whose lines number its members, is not daily.

## The day

`day = RtcGetLocalDayCount()`, the same real-time calendar day berry trees use.
A later change moves Wayfarer to an in-game clock; this spec then reads that
clock's day instead, with no other change.

A new day takes effect on the next map load. The current map keeps its
occupants until then.

## Deterministic draws

Every draw comes from a hash, never from the game's random number generator:

```text
roll(kind, a, b) = hash32(saveSeed, day, kind, a, b)
saveSeed = the player's 32-bit trainer ID
```

`hash32` is a fixed integer mix with no state. The same save, day and inputs
always give the same result, so reloading, saving and resetting, or leaving
and coming back on the same day changes nothing.

## Save state

The daily state is one struct in `PokemonStorage`, beside `wayfarerWorld`
(`IS_WAYFARER` block of `pokemon_storage_system.h`), in the room freed by the
13-box decision in
[Notable world simulation](notable-world-simulation.md#where-it-lives).
SaveBlock3 gets nothing: its free bytes are already claimed by the haunt,
friendship and follower state.

```text
u16 stampDay                          // day the bits below belong to
bitset clearedToday[slotCount]        // every item spot, then every trainer slot
bitset homeBeatenToday[trainerSlotCount]
bitset readyAtDayStart[contactCount]  // contacts ready for a rematch when the day began
bitset giftAtDayStart[contactCount]   // contacts holding a gift when the day began
```

SaveBlock1 only grows by [`trainerRematches`](phone-rematches.md#rematch-table-and-save-state),
100 to 118 bytes. The trainer half does not grow it: that belongs to the phone.

**New Game.** New Game doesn't clear `PokemonStorage` beyond the boxes, so the
struct clears itself like its neighbours do: a new-game init, called from
`NewGameInitData` beside `WayfarerWorld_InitNewGame` (`src/new_game.c`),
zeroes it and sets `stampDay` to `0xFFFF`, a sentinel no real day equals. The
first map load then starts a fresh day.

On map load, if `stampDay != (u16)day` (the day count is `u32`; compare its low 16 bits), clear `clearedToday` and `homeBeatenToday`,
take the two snapshots and set `stampDay`. The snapshots are the
[phone contacts](phone-rematches.md) that are ready for a rematch, and those
that hold a gift, at that moment (the contact index covers the rematch table
plus the Sevii and coast families, at most 160 bits each). They are fixed for
the day: a trainer who becomes ready, or whose gift flag is set, later in the
day is placed and calls from the next day. Battle text uses only
`readyAtDayStart` and FoundItem text only `giftAtDayStart`; a contact in
either set is placed. The phone reads the stamped day, never the clock, so a
call before the next map load uses the previous day's arrangement.

Budget (659 trainer slots and 838 item spots, Sevii included; the generated
counts are in `include/constants/world_trainer_slots.h` and `world_item_spots.h`):

| Part | Bytes |
| --- | ---: |
| `stampDay` | 2 |
| `clearedToday` (1,497 bits) | 188 |
| `homeBeatenToday` (659 bits) | 83 |
| `readyAtDayStart`, `giftAtDayStart` (160 bits each) | 40 |
| Total in `PokemonStorage` (a word-aligned struct) | 316 |

| Block | Free before | Added | Left after |
| --- | ---: | --- | ---: |
| `PokemonStorage` (13 boxes, PR #139 merged, after the 2,104 bytes other specs plan) | 416 | 316 | 100 |
| SaveBlock1 | 112 | 18 (`trainerRematches` 100 to 118) | 94 |

The `PokemonStorage` figure is Notable world simulation's: of its 2,520
bytes, the world state, the haunts' traded slots and the Masters growth plan
2,008 (about 416 left with the 96 bytes of face-only trainers), and the daily
state takes 316 of that margin (the budget assert allows 330). Capacity figures come from the built ELF and
the limits in `src/save.c`: nine storage sectors hold 35,712 bytes, and four
SaveBlock1 sectors of 3,968 bytes hold 15,872 against 15,760 used (`gSaveblock1`
is 0x3E10, less the 128-byte `SAVEBLOCK_MOVE_RANGE` pad in `load_save.h`).
`save.c` asserts the daily struct's size and its 330-byte budget, and keeps
`PokemonStorageFreeSpace`. The layout change bumped `SAVE_VERSION` to 14. Follow the RAM rules in `AGENTS.md`; the
per-map resolved occupants live in EWRAM for the map's lifetime only.

## Trainers

### Who stands where today

For each range, on map load, resolve all of the range's slots at once (a group
for a class or place range, the slot alone for a home range, seeded by the slot):

1. **Home slots.** A slot whose home trainer is undefeated shows the home
   trainer. A slot whose `homeBeatenToday` bit is set also shows the home
   trainer, defeated. Neither is a free slot today.
2. **Wanderers.** The group's wandering trainers are its defeated trainers,
   minus those whose `homeBeatenToday` bit is set. A trainer beaten for the
   first time today starts wandering tomorrow, so the day's shuffle never
   changes after a win.
3. **Free slots** are the remaining slots: those whose home trainer is
   defeated and not beaten today. There are exactly as many as wanderers.
4. **Shuffle.** Order the wanderers by a hash permutation keyed by
   `roll(GROUP, groupId, 0)` and assign them to the free slots in group
   order.
5. **Empty spots.** Each free slot is empty when
   `roll(EMPTY, slotIndex, 0) % 100 < absence(slot)`, the slot's own absence
   chance (25% unless the slot's level says otherwise; a slot at 0% is never
   empty). A wanderer assigned to an empty slot is away today.
6. **Guaranteed places.** A wanderer in `readyAtDayStart` or `giftAtDayStart`
   (ready for a rematch or holding a gift when the day started; see
   [Phone rematches](phone-rematches.md)) is always placed. If it was assigned
   to an empty slot, it swaps with the first placed wanderer that is in
   neither set. If none can swap, the first empty slot in group order
   becomes occupied for it. Readiness itself is never read here, so a win that
   clears readiness, or a trainer who becomes ready mid-day, changes nothing
   until the next day.

Only the current map's groups need resolving, but the result is the same
wherever it's computed. A lookup `WhereIsTrainerToday(trainerId)` resolves the
trainer's group and returns the map and slot, or none. For a contact that is
not daily it returns the home map and slot (it has no
daily battle, so `clearedToday` doesn't apply), and such a contact is placed at
home for ready and gift calls. Its home script still checks live readiness
and the live gift flag, so its Battle and FoundItem calls also need those to
hold (see [Calls](phone-rematches.md#calls)). It returns none
for a trainer whose slot is cleared today, so a call never names a trainer who
has already had today's battle. Phone calls use it to name today's place.

### The map-load hook

After the map's object templates are copied to
`gSaveBlock1Ptr->objectEventTemplates` and before objects spawn, rewrite each
free slot's template:

- **Occupied:** set the graphics to the occupant's trainer sprite and the script
  to the occupant's own authored script (the generated table keeps both for
  every slot), and keep the trainer type, movement and sight from the slot. The
  occupant is remembered per local id for the map's lifetime. There is no
  separate shared script: the trainer sight check runs the object's script until
  its `trainerbattle`, so only the occupant's own script gives the sight path the
  occupant's texts, and the home scripts already carry the gift hand-off.
- **Empty:** nothing is rewritten; the [spawn check](#flags-and-visibility)
  keeps the template from spawning.

**On Continue.** `CB2_ContinueSavedGame` (`overworld.c`) loads the templates
from the save, then `LoadSaveblockObjEventScripts` resets every template's
script to the authored one, and the EWRAM occupant table is empty. So the hook
also runs on Continue, right after `LoadSaveblockObjEventScripts` and before
`RunOnLoadMapScript` and `WayfarerWorld_OnContinue` (which restores the
walkers), the way the Battle Pyramid's `LoadBattlePyramidFloorObjectEventScripts`
does. Hidden items re-resolve with the map. It re-resolves the
current map from the stamped day and rewrites scripts, graphics, `flagId`s and
occupants again, which also rebuilds the per-map occupant table the
[sight branch](#battles) reads. Continue applies no day change; a new day is
still applied only by the map-load rule in [The day](#the-day).

The [notable walkers' hook](../../game/src/event_object_movement.c) uses the
same seam. Walkers never spawn on a trainer slot's tile, and slot rewriting
runs before walker placement. The Battle Pyramid rewrites templates the same
way and is the precedent.

### Battles

The occupant's own authored script battles it with that trainer's own intro,
defeat and post-battle text. The script's flag check is the only thing that
changes: `GetTrainerFlag` and the trainer sight check read the slot, not the
trainer's permanent flag:

- **Gift first.** If the occupant holds a phone gift (see
  [Phone rematches](phone-rematches.md#gifts)), the flag check reads done, so
  the script falls through to its own gift hand-off before any battle, as the
  trainer's home map script does today. A trainer holding a gift does not
  approach by sight, as at home today; the player talks to it.
- **Eligible** when the slot's `clearedToday` bit is unset. The trainer's
  permanent defeat flag is already set, so it doesn't gate this battle.
- **After a win**, set the slot's `clearedToday` bit. The occupant stays and
  speaks its post-battle line for the rest of the day. The one-time script the
  home battle runs after the win (registering a phone number) is skipped for a
  wanderer; its post-battle conversation still offers what it offered before.
- **A loss** clears nothing.
- **Party, levels and rewards** come from the occupant's trainer id, including
  its latest rematch team, through
  [regular trainer scaling](trainer-party-scaling.md). The level uses the
  battle map's reach.

A home trainer's first battle uses its authored script. On that win the game
sets the trainer's defeat flag as today, plus the slot's `homeBeatenToday` and
`clearedToday` bits.

Pairs battle as one unit and share one slot pair: both objects take the
occupant pair's sprites and scripts.

The game notes the slot of each opponent when a trainer battle is configured,
and only when the opponent is the slot's home trainer or its wanderer; a win
(`SetBattledTrainersFlags`) then clears the notes' slots. Rematch battles
(`ShouldTryRematchBattle`) are not offered at a free slot: a ready
contact is placed, but its rematch is the phone's.

**Sight.** The trainer sight check (`trainer_see.c`) treats a defeated trainer
as done, so a wanderer would never approach. Like the Battle Pyramid and
Trainer Hill overrides, it gets a free-slot branch: for an object the hook
rewrote, it reads the slot's `clearedToday` bit instead of the trainer's defeat
flag, so a wanderer approaches by sight until it is cleared for the day.

## Items

### Static prizes

A prize spot shows its prize while the prize's flag is unset. Picking it up
sets that flag and the spot's `clearedToday` bit. From the next day the spot
draws like a dynamic spot.

The generator sets each prize spot's item at build time, because a prize often
replaces or moves into a spot whose authored script or hidden item gives
another item. An item ball runs a generated prize script that gives the prize
item and keeps the spot's permanent flag. A hidden prize spot gets the prize
item in its generated hidden-item data. The authored item is never given.

### Flags and visibility

The game gates items by flag in three places: the object spawn skips a template
whose `flagId` is set, `finditem`'s `removeobject` sets that flag, and hidden
items are gated by `GetHiddenItemFlagId`. A permanent flag would therefore
hide a dynamic spot for good. Clearing the `flagId` instead would not work on
its own: `TrySpawnObjectEvents` (`event_object_movement.c`) runs on every
camera step through `UpdateObjectEventsForCameraUpdate`, and a template with
no flag is respawned at once after `removeobject`, because flag 0 is never
set. So the hook handles flags per spot kind and adds a spawn check:

- **Untaken prize:** keeps its authored template and permanent flag. Its own
  pickup path runs and sets the flag and `clearedToday`.
- **Dynamic spot and taken prize:** the hook substitutes the template's
  `flagId` with none. The dynamic pickup script sets only `clearedToday`,
  then runs `removeobject`.
- **Spawn check.** In `TrySpawnObjectEvents`, next to the existing
  `WayfarerWalkers_HideTemplate` call, a template that belongs to a daily trainer slot or to a dynamic or taken-prize item spot spawns only if the slot
  is occupied today. An item spot must also have its `clearedToday` bit unset,
  so a picked-up ball stays gone on every camera step and reload until the next
  day. A cleared trainer slot still spawns: its occupant stays and speaks. The
  same check hides the template of an empty slot, so the hook needs no separate
  hide.
- **Hidden items:** for these spots the facing-tile check, the pickup script and
  the Itemfinder read the resolved item and `clearedToday` instead of
  `GetHiddenItemFlagId`. An untaken prize keeps its flag.

### Dynamic spots

A dynamic spot, or a prize spot whose prize is taken, resolves on map load
unless its `clearedToday` bit is set:

1. Empty when `roll(ITEM_EMPTY, spotIndex, 0) % 100 < 25`.
2. Otherwise roll the tier from the spot's reach tier with
   `roll(ITEM_TIER, spotIndex, 0)`, then the item by weight with
   `roll(ITEM_PICK, spotIndex, tier)`.
3. Item balls: rewrite the template's script to the shared dynamic-item
   script for the resolved item and clear its `flagId`; the
   [spawn check](#flags-and-visibility) hides it when empty or cleared. Hidden items: the hidden-item lookup returns the resolved item, or
   nothing.
4. Picking it up sets the spot's `clearedToday` bit. Dynamic items use no
   permanent flags.

The Itemfinder sees resolved hidden items only.

### Implementation notes

- **One ball script.** Instead of one generated prize script per prize, the hook
  points every item ball spot at one shared script
  (`DailyItems_EventScript_Ball`). It asks the generated tables for the ball's
  item: the prize while the prize's flag is unset, otherwise today's find. The
  behavior is the one above; the authored item is never given.
- **Stateless resolution.** A spot's find is a function of the save seed, the
  stamped day and the slot index, so the per-map state in EWRAM is only the
  pending hidden-item pickup. The spawn check, the facing check and the
  Itemfinder (including connected maps) all recompute it.
- **Slot index.** Item spots are the first rows of the shared `clearedToday`
  bitset, in the order of `src/data/item_slots/spots.h` (map, ball before
  hidden, id). Trainer slots follow them, in the order of
  `src/data/trainer_slots/slots.h` (map, then the first local id).
- **Prize flags.** The generator writes each prize spot's permanent flag into the
  spot table as the map assembles it, and the runtime reads a prize's flag from
  there, for balls and hidden items alike. For a hidden prize it is the same flag
  the bg event decodes to (Hoenn rows carry the Hoenn hidden-item marker); a
  mechanics test checks this for every Hoenn hidden prize. A hidden prize on an underfoot row would be unreachable, so the generator
  fails on one and excludes underfoot dynamic rows; the two prize rows that were
  underfoot (Pokemon Tower 7F and Cape Brink) have the flag cleared for Wayfarer, and
  Cape Brink's row also gets elevation 0 so it can be faced from the water around it.
- **Spawn check.** It runs in `TrySpawnObjectEvents` only. No script `addobject`s an
  item ball, so the one-off spawn path needs no check.
- **Debug override.** `gDailySlotsDebugFlags`, `gDailySlotsDebugDay` and
  `gDailySlotsDebugSeed` pin the day and seed in the mechanics-test and E2E
  builds. `DAILY_DEBUG_NO_EMPTY` also keeps every free trainer slot occupied,
  `DAILY_DEBUG_ABSENCE` replaces every slot's absence chance with
  `gDailySlotsDebugAbsence`,
  and `gDailyTrainersDebugOccupants` lets a journey read the map's occupants.

## Dialogue audit

The generator writes an inventory of every daily candidate's intro, defeat
and post-battle lines (`src/data/trainer_slots/dialogue.md`). It flags lines
that name a map, landmark, building or local event, using the map name table
(and the stems of its town, city and island names) and the reviewed word lists
in `tools/wayfarer_trainer_slots/landmarks.json`: landmark nouns after a
determiner, terrain nouns after a demonstrative ("this beach", not "the sea"),
place phrases ("around here"), local-event words written in capitals, and
phrases that number a trainer among a challenge's opponents ("No. 3", "last in
line"). Each flagged trainer is resolved in `reviewed.json` with a reason, by one of:
a reviewed edit to neutral wording (`range: class` with the old line in `was`,
which the audit then confirms by finding nothing); a place range
(`range: place:<type>`, with the phrases that need the kind of place); a home
range (`range: home`: the lines are about the spot itself); or `not_daily`,
when absence would break its lines. A resolution may also set `absence` for the
trainer's slot. Generation fails while a flagged line is unresolved, a reword
still names a place, or a resolution is stale. A false positive of the word
lists is fixed in the lists, not waived. Text of generated Sevii and coast
scripts is not edited by hand: those trainers get a home or place range, never a
reword.

Of 665 candidates, 164 named a place at some point: 114 have a home range, 6 are
not daily (the Route 25 sequence), 16 were reworded, 20 have a place range and
8 were cleared by narrow reviewed
phrases (a goal or plan that reads fine anywhere: "aim for the POKéMON
LEAGUE", "going to the GYM?"). The report lists every resolution with its reason
and every allowed phrase with why it is fine anywhere.

### What the trainer half leaves for the phone

[Phone rematches](phone-rematches.md) owns calls, numbers, readiness and rematch
teams. This spec ships the seams it reads:

- `readyAtDayStart` and `giftAtDayStart` are saved and filled at day start from
  the readiness and gift state the game has today (the HNS rematch table's
  `trainerRematches`, the HNS gift flags, the Sevii and coast registries'
  pending bits). A contact is a bit in the index space the phone spec lays out:
  table rows 0-117, then the 32 Sevii families that carry a stronger team, then
  the nine coast families. Hoenn and the table's growth to 118 rows join by
  appending rows; `DailyContacts_IndexOf` finds them with no change.
- `WhereIsTrainerToday` and `DailyContacts_ReadyAtDayStart`/`GiftAtDayStart` are
  the lookups the calls use. A not-daily contact's home is a small generated table.
- Ready contacts are placed but their next-team rematch battle is not offered at
  a free slot; a gift holder hands its gift over wherever it stands (HNS gifts
  only, the one gift source that exists).
- A wanderer fights with its latest beaten rematch team, not its next one.

## Interactions

- **Vs. Seeker:** retired. The Vermilion Pokémon Center giver isn't compiled
  into Wayfarer, and no daily path reads Vs. Seeker readiness.
- **Randomizer settings:** a draw resolves the occupant or item first; the
  trainer and item randomizers then apply as they do to authored content.
- **Challenge settings** such as Nuzlocke and mono-type apply to daily
  battles as they do to authored ones.
- **Defeat flags and trainer rematch flags** keep their existing meaning.

## Debug and testing

Debug and E2E builds can override `day` and `saveSeed`, so any draw can be
reproduced. Tests cover:

- the same draw across reload, save and reset, and a new draw on a new day;
- Continue: save on a map with a daily occupant and a dynamic ball, reset
  and Continue, then talk to the trainer (the occupant's battle, not the
  authored one), pick up the ball, and take camera steps (it stays gone);
- New Game on the same day as an existing save: no item spot or trainer slot
  stays cleared from the old save (every non-empty dynamic spot spawns), and no
  snapshot carries over;
- the home-beaten-today freeze, and the wanderer starting the next day;
- one place per trainer per day, pairs, and placement of ready and gift trainers, including readiness or a gift flag changing mid-day, and Battle text only for a ready-at-day-start contact and FoundItem text only for a gift-at-day-start one;
- a not-daily contact receiving Battle and FoundItem calls that name its home map, and only General calls after that day's rematch or gift;
- wanderers approaching by sight until cleared, and not after (the mechanics test
  reads the flag checks the sight path and the battle script make; the E2E journey
  walks into the sight line);
- dynamic and taken-prize spots staying visible after a reload on the same day, and refilling the next day, with no permanent flag set;
- a picked-up ball (dynamic or taken prize) not respawning on later camera steps, a reload or leaving and re-entering the map the same day, and an empty slot's template never spawning;
- a prize spot giving the prize item, never its authored item, for both balls and hidden spots;
- empty odds and tier odds over many days within tolerance;
- prize pickup becoming dynamic the next day;
- the template hook changing nothing but sprite and script on every map with a
  slot, and holding every free slot of the busiest map;
- the generated reports: slot status, exclusions, the dialogue audit and pool
  validation.

## Generation and reports

The generator fails on: a slot that names a map Wayfarer doesn't compile, an
unknown item or trainer, a daily trainer with a class or place range but no group, a prize spot with
no flag, a group whose wanderers can't all fit its slots, or an unresolved
dialogue flag. Its report lists slot status by region with reasons, group
sizes, the dialogue audit, and pool weights per reach.

The trainer generator (`make world-trainers-generate`, audited and tested by
`make check`) also fails on a trainer in two slots, a script that is not a global
label, a map whose local ids are not dense, and a stale reviewed entry. It writes
`src/data/trainer_slots/{slots.h,report.md,dialogue.md}` and
`include/constants/world_trainer_slots.h`; the report has slot status by region
with the reason each `ORDINARY` trainer stays at home, the group sizes and the
audit's resolutions; its columns are range (class, place, home), the count
of slots at 0% absence, and the reason each not-daily trainer is not daily. A group's slots are its wanderers' places by construction,
so "wanderers that cannot fit" reduces to a trainer listed twice in one group.
