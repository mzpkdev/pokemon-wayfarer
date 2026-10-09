# Daily world slots

PRD: [Daily world slots](../prds/daily-world-slots.md)
Implemented: Partial. The item half is implemented: the day, deterministic draws, the
saved daily state with `clearedToday`, the item spot generator and tables, the
map-load and Continue hooks for item balls, the spawn check, hidden items and the
Itemfinder, and the debug and E2E day and seed override. Trainer slots, trainer
groups, the dialogue audit and the phone snapshots are not.

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

- **Trainer slots:** one row per regular trainer object: map, local id, home
  trainer id, trainer group and pair flag. The slot keeps its own position,
  facing, movement and sight range.
- **Trainer groups:** one row per class family and region, split into a
  singles group and a pairs group. A group lists its slots in a fixed order.
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

### Which trainers rotate

A trainer slot rotates only when its trainer is `ORDINARY` in the
[classification manifest](trainer-party-scaling.md#classification-contract)
and isn't excluded by the PRD's fixed list: Gym members, rivals, bosses,
notable trainers, facility trainers, story trainers, path blockers, and
trainers whose script does more than battle. Phone registration and one-time
gifts don't exclude a trainer; they move with it.

The generator proposes each slot's status from script analysis, and authors
review it in the report. A slot can be forced fixed by a reviewed exclusion
with a reason. A fixed contact stays at home and still gets Battle and
FoundItem calls. A rotating candidate must also pass the
[dialogue audit](#dialogue-audit).

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
bitset clearedToday[slotCount]        // every trainer slot and item spot
bitset homeBeatenToday[trainerSlotCount]
bitset readyAtDayStart[contactCount]  // contacts ready for a rematch when the day began
bitset giftAtDayStart[contactCount]   // contacts holding a gift when the day began
```

SaveBlock1 only grows by [`trainerRematches`](phone-rematches.md#rematch-table-and-save-state),
100 to 118 bytes.

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

Budget (about 720 trainer slots and 860 item spots, Sevii included):

| Part | Bytes |
| --- | ---: |
| `stampDay` | 2 |
| `clearedToday` (1,580 bits) | 198 |
| `homeBeatenToday` (720 bits) | 90 |
| `readyAtDayStart`, `giftAtDayStart` (160 bits each) | 40 |
| Total in `PokemonStorage` | 330 |

| Block | Free before | Added | Left after |
| --- | ---: | --- | ---: |
| `PokemonStorage` (13 boxes, PR #139 merged, after the 2,104 bytes other specs plan) | 416 | 330 | 86 |
| SaveBlock1 | 112 | 18 (`trainerRematches` 100 to 118) | 94 |

The `PokemonStorage` figure is Notable world simulation's: of its 2,520
bytes, the world state, the haunts' traded slots and the Masters growth plan
2,008 (about 416 left with the 96 bytes of face-only trainers), and the daily
state takes 330 of that margin. Capacity figures come from the built ELF and
the limits in `src/save.c`: nine storage sectors hold 35,712 bytes, and four
SaveBlock1 sectors of 3,968 bytes hold 15,872 against 15,760 used (`gSaveblock1`
is 0x3E10, less the 128-byte `SAVEBLOCK_MOVE_RANGE` pad in `load_save.h`).
Add static asserts on the size of the daily struct and on both blocks, and
keep `PokemonStorageFreeSpace`. Follow the RAM rules in `AGENTS.md`; the
per-map resolved occupants live in EWRAM for the map's lifetime only.

## Trainers

### Who stands where today

For each trainer group, on map load, resolve all of the group's slots at once:

1. **Home slots.** A slot whose home trainer is undefeated shows the home
   trainer. A slot whose `homeBeatenToday` bit is set also shows the home
   trainer, defeated. Neither is a rotating slot today.
2. **Wanderers.** The group's wandering trainers are its defeated trainers,
   minus those whose `homeBeatenToday` bit is set. A trainer beaten for the
   first time today starts wandering tomorrow, so the day's shuffle never
   changes after a win.
3. **Rotating slots** are the remaining slots: those whose home trainer is
   defeated and not beaten today. There are exactly as many as wanderers.
4. **Shuffle.** Order the wanderers by a hash permutation keyed by
   `roll(GROUP, groupId, 0)` and assign them to the rotating slots in group
   order.
5. **Empty spots.** Each rotating slot is empty when
   `roll(EMPTY, slotIndex, 0) % 100 < 25`. A wanderer assigned to an empty
   slot is away today.
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
trainer's group and returns the map and slot, or none. For a contact whose slot
is fixed (excluded from rotation) it returns the home map and slot (it has no
daily battle, so `clearedToday` doesn't apply), and such a contact is placed at
home for ready and gift calls. Its home script still checks live readiness
and the live gift flag, so its Battle and FoundItem calls also need those to
hold (see [Calls](phone-rematches.md#calls)). It returns none
for a trainer whose slot is cleared today, so a call never names a trainer who
has already had today's battle. Phone calls use it to name today's place.

### The map-load hook

After the map's object templates are copied to
`gSaveBlock1Ptr->objectEventTemplates` and before objects spawn, rewrite each
rotating slot's template:

- **Occupied:** set the graphics to the occupant's trainer sprite, the script
  to the shared rotating-trainer script, and the trainer type and sight from
  the slot. The occupant is remembered per local id for the map's lifetime.
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

The shared rotating-trainer script battles the slot's occupant with that
trainer's own intro, defeat and post-battle text:

- **Gift first.** If the occupant holds a phone gift (see
  [Phone rematches](phone-rematches.md#gifts)), the script runs the gift
  hand-off before anything else, as the trainer's home map script does today.
- **Eligible** when the slot's `clearedToday` bit is unset. The trainer's
  permanent defeat flag is already set, so it doesn't gate this battle.
- **After a win**, set the slot's `clearedToday` bit. The occupant stays and
  speaks its post-battle line for the rest of the day.
- **A loss** clears nothing.
- **Party, levels and rewards** come from the occupant's trainer id, including
  its latest rematch team, through
  [regular trainer scaling](trainer-party-scaling.md). The level uses the
  battle map's reach.

A home trainer's first battle uses its authored script. On that win the game
sets the trainer's defeat flag as today, plus the slot's `homeBeatenToday` and
`clearedToday` bits.

Pairs battle as one unit and share one slot pair.

**Sight.** The trainer sight check (`trainer_see.c`) treats a defeated trainer
as done, so a wanderer would never approach. Like the Battle Pyramid and
Trainer Hill overrides, it gets a rotating-slot branch: for an object the hook
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
  `WayfarerWalkers_HideTemplate` call, a template that belongs to a rotating
  trainer slot or to a dynamic or taken-prize item spot spawns only if the slot
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
  hidden, id). Trainer slots will follow them.
- **Prize flags.** The generator writes each prize spot's permanent flag into the
  spot table as the map assembles it, and the runtime uses that flag, never the
  one decoded from a hidden item's packed bg event (Hoenn's flags overflow that
  field). A hidden prize on an underfoot row would be unreachable, so the generator
  fails on one and excludes underfoot dynamic rows; the two prize rows that were
  underfoot (Pokemon Tower 7F and Cape Brink) have the flag cleared for Wayfarer, and
  Cape Brink's row also gets elevation 0 so it can be faced from the water around it.
- **Spawn check.** It runs in `TrySpawnObjectEvents` only. No script `addobject`s an
  item ball, so the one-off spawn path needs no check.
- **Debug override.** `gDailySlotsDebugFlags`, `gDailySlotsDebugDay` and
  `gDailySlotsDebugSeed` pin the day and seed in the mechanics-test and E2E
  builds.

## Dialogue audit

The generator writes an inventory of every rotating candidate's intro, defeat
and post-battle lines. It flags lines that name a map, landmark, building or
local event, using the map name table and a reviewed landmark word list. Each
flagged trainer is resolved by a reviewed edit to neutral wording, or by
excluding it from rotation with a reason. Generation fails while a flagged
line is unresolved.

## Interactions

- **Vs. Seeker:** retired. The Vermilion Pokémon Center giver isn't compiled
  into Wayfarer, and no rotating path reads Vs. Seeker readiness.
- **Randomizer settings:** a draw resolves the occupant or item first; the
  trainer and item randomizers then apply as they do to authored content.
- **Challenge settings** such as Nuzlocke and mono-type apply to rotating
  battles as they do to authored ones.
- **Defeat flags and trainer rematch flags** keep their existing meaning.

## Debug and testing

Debug and E2E builds can override `day` and `saveSeed`, so any draw can be
reproduced. Tests cover:

- the same draw across reload, save and reset, and a new draw on a new day;
- Continue: save on a map with a rotating occupant and a dynamic ball, reset
  and Continue, then talk to the trainer (the occupant's battle, not the
  authored one), pick up the ball, and take camera steps (it stays gone);
- New Game on the same day as an existing save: no item spot or trainer slot
  stays cleared from the old save (every non-empty dynamic spot spawns), and no
  snapshot carries over;
- the home-beaten-today freeze, and the wanderer starting the next day;
- one place per trainer per day, pairs, and placement of ready and gift trainers, including readiness or a gift flag changing mid-day, and Battle text only for a ready-at-day-start contact and FoundItem text only for a gift-at-day-start one;
- a fixed (non-rotating) contact receiving Battle and FoundItem calls that name its home map, and only General calls after that day's rematch or gift;
- wanderers approaching by sight until cleared, and not after;
- dynamic and taken-prize spots staying visible after a reload on the same day, and refilling the next day, with no permanent flag set;
- a picked-up ball (dynamic or taken prize) not respawning on later camera steps, a reload or leaving and re-entering the map the same day, and an empty slot's template never spawning;
- a prize spot giving the prize item, never its authored item, for both balls and hidden spots;
- empty odds and tier odds over many days within tolerance;
- prize pickup becoming dynamic the next day;
- the template hook with walkers present and the 16-sprite budget on the
  busiest maps;
- the generated reports: slot status, exclusions, the dialogue audit and pool
  validation.

## Generation and reports

The generator fails on: a slot that names a map Wayfarer doesn't compile, an
unknown item or trainer, a rotating trainer without a group, a prize spot with
no flag, a group whose wanderers can't all fit its slots, or an unresolved
dialogue flag. Its report lists slot status by region with reasons, group
sizes, the dialogue audit, and pool weights per reach.
