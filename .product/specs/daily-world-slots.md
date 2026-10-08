# Daily world slots

PRD: [Daily world slots](../prds/daily-world-slots.md)
Implemented: No

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
  [World items](world-items.md#reach-of-an-item-spot).
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
with a reason. A rotating candidate must also pass the
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

One Wayfarer block in SaveBlock3:

```text
u16 stampDay                       // day the bits below belong to
bitset clearedToday[slotCount]     // every trainer slot and item spot
bitset homeBeatenToday[trainerSlotCount]
```

On map load, if `stampDay != day`, clear both bitsets and set `stampDay`.
Budget: about 720 trainer slots and 860 item spots, Sevii included, which is
about 290 bytes. Follow the RAM rules in `AGENTS.md`; the per-map resolved
occupants live in EWRAM for the map's lifetime only.

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
6. **Ready rematches.** A wanderer whose rematch is ready (see
   [Phone rematches](phone-rematches.md)) is always placed. If it was assigned
   to an empty slot, it swaps with the first placed wanderer that isn't
   ready. If none can swap, the first empty slot in group order becomes
   occupied for it.

Only the current map's groups need resolving, but the result is the same
wherever it's computed. A lookup `WhereIsTrainerToday(trainerId)` resolves the
trainer's group and returns the map and slot, or none. Phone calls use it to
name today's place.

### The map-load hook

After the map's object templates are copied to
`gSaveBlock1Ptr->objectEventTemplates` and before objects spawn, rewrite each
rotating slot's template:

- **Occupied:** set the graphics to the occupant's trainer sprite, the script
  to the shared rotating-trainer script, and the trainer type and sight from
  the slot. The occupant is remembered per local id for the map's lifetime.
- **Empty:** hide the template.

The [notable walkers' hook](../../game/src/event_object_movement.c) uses the
same seam. Walkers never spawn on a trainer slot's tile, and slot rewriting
runs before walker placement. The Battle Pyramid rewrites templates the same
way and is the precedent.

### Battles

The shared rotating-trainer script battles the slot's occupant with that
trainer's own intro, defeat and post-battle text:

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

## Items

### Static prizes

A prize spot shows its prize while the prize's flag is unset. Picking it up
sets that flag and the spot's `clearedToday` bit. From the next day the spot
draws like a dynamic spot.

### Dynamic spots

A dynamic spot, or a prize spot whose prize is taken, resolves on map load
unless its `clearedToday` bit is set:

1. Empty when `roll(ITEM_EMPTY, spotIndex, 0) % 100 < 25`.
2. Otherwise roll the tier from the spot's reach tier with
   `roll(ITEM_TIER, spotIndex, 0)`, then the item by weight with
   `roll(ITEM_PICK, spotIndex, tier)`.
3. Item balls: rewrite the template's script to the shared dynamic-item
   script for the resolved item, or hide it when empty or cleared. Hidden
   items: the hidden-item lookup returns the resolved item, or nothing.
4. Picking it up sets the spot's `clearedToday` bit. Dynamic items use no
   permanent flags.

The Itemfinder sees resolved hidden items only.

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
- the home-beaten-today freeze, and the wanderer starting the next day;
- one place per trainer per day, pairs, and ready-rematch placement;
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
