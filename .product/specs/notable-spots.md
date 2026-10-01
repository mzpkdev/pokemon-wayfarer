# Notable spots

PRD: [Notable spots](../prds/notable-spots.md)
Implemented: No
Design status: the everyday layer of the future **routine and travel**
design. Spots switch on together with routines and travel; v0 haunt
placement is unchanged, and nothing in this spec runs before then. Spots
are found automatically from map data, in one pool for every region, and
carry no quests. Capacity values, the search radius, and the activity
lines are placeholders.

## Scope

Own, in `IS_WAYFARER`: the spot kinds and how each is detected from map
data, the build-time spot extraction and its authored overrides, spot
capacity, the activity-to-kind mapping and the two new activities
(**fish** and **visit**), choosing a spot, favourites as a trainer value,
the talk flow at spots and its activity lines, the walker behaviours each
kind needs, the Gym exit-spawn rule, and the saved-state stance.

- [Notable haunts](notable-haunts.md) owns the haunts, their placement and
  quests, the [activity list](notable-haunts.md#activities) that spots
  extend, the [talk flow](notable-haunts.md#talk-flow) whose greeting and
  follow-up steps spots reuse, and the
  [travel engine note](notable-haunts.md#travel-and-on-map-walking).
- The routine design (future work) owns activity cycles: when a trainer
  does which activity, home bases, travel between maps, the world record,
  and the heartbeat. This spec owns only what happens once an activity is
  chosen. The
  [travel proof of concept](../research/notable-trainer-travel-poc.md) is
  its evidence so far.
- [Notable trainers](notable-trainers.md) owns friendship and its events,
  and holds favourites in the catalog once they are authored.
- [Gym Leader scaling](gym-leader-scaling.md) owns badge-encounter
  coverage, which names each Gym's leader.

## Behavior

### Spots

A **spot** is one standing tile (or, for tall grass, a patch of tiles) on
one map, with a facing and a **kind**. Spots are never authored one by one:
the [spot extraction](#spot-extraction) finds them in map data at build
time, across every playable map of Kanto, Johto, Hoenn, and Sevii, as one
pool. A spot names no trainer, offers no quest, and holds one trainer at a
time.

**Haunts** stay the authored special places with quests. A haunt's standing
tile is never also a spot; the extraction drops any spot that lands on one.
Trainers spend most of their time at spots, and haunts are the highlights.

### Spot kinds

Each kind lists its detection source and what the trainer does there. A
trainer only does what the walker can do: walk to a tile, face a
direction, show an emote, and pass through a door. "Occasional" means a
placeholder 1 in 8 dwell ticks.

| # | Kind | Detection | What the trainer does |
| ---: | --- | --- | --- |
| 1 | Pokémon Center | A door whose destination is a Center interior. | Stands facing the counter, or stands still at the side of the room ("sitting"). |
| 2 | Poké Mart and department store | A door whose destination is a Mart or store, plus the store's other floors. | Faces a shelf; now and then takes the stairs or escalator to another floor. |
| 3 | Game Corner | A door whose destination is a Game Corner. | Stands at a slot machine, facing it. |
| 4 | Other Gyms | A door whose destination is a Gym. | "Just leaving" ([below](#just-leaving)). |
| 5 | Tall grass | Tall-grass metatile behaviour. | Roams the patch, with an occasional "!". |
| 6 | Water's edge | Land tiles beside fishable water. | Faces the water in a "fishing" pose, with an occasional "!". |
| 7 | Town squares and benches | Open areas of town and city maps, plus an authored list of bench and lookout tiles. | Idles and looks around. |
| 8 | Chatting with an NPC | An existing NPC object on the map. | Stands adjacent, facing them, with an occasional "…". |

The "fishing" pose is facing the water and standing still; there is no rod
sprite. "Sitting" at a Center is the same: no sitting frame exists, so the
trainer stands still facing into the room.

### Detection sources

Verified against `game/` at the time of writing. Line numbers are for
orientation and may drift.

**Doors.** Every `warp_events` entry in a map's `map.json` names its
`dest_map`: Cerulean City's warps at
[CeruleanCity_hns/map.json](../../game/data/maps/CeruleanCity_hns/map.json)
(line 431) lead to `MAP_CERULEAN_CITY_POKEMON_CENTER_HNS` (line 464),
`MAP_CERULEAN_CITY_GYM_HNS` (line 471), and `MAP_CERULEAN_CITY_MART_HNS`
(line 485). Door tiles carry `MB_ANIMATED_DOOR` or `MB_NON_ANIMATED_DOOR`
([metatile_behaviors.h](../../game/include/constants/metatile_behaviors.h),
lines 101 and 110), read by `MetatileBehavior_IsDoor`
([metatile_behavior.c](../../game/src/metatile_behavior.c), line 246).
A door is a spot source only when it leads from an outdoor map into an
interior (`map_type` `MAP_TYPE_INDOOR`).

**Classifying the interior.** Map names are not reliable:
`MAP_CIANWOOD_POKECENTER_HNS` and `MAP_CIANWOOD_SHOP_HNS` break the usual
`_POKEMON_CENTER` and `_MART` patterns, and HNS, FRLG, and Emerald copies
of the same building coexist (`CeladonCity_GameCorner_hns` and
`CeladonCity_GameCorner_Frlg`). The extraction classifies a destination by
its content, first match wins:

| Kind | Signal in the destination's `map.json` and layout |
| --- | --- |
| Pokémon Center | An object whose `graphics_id` is a nurse (`OBJ_EVENT_GFX_NURSE`, `_FRLG`, `_HNS`, or `_CHANSEY_HNS`), as at [CeruleanCity_PokemonCenter_hns/map.json](../../game/data/maps/CeruleanCity_PokemonCenter_hns/map.json) line 19; 73 maps carry one. |
| Game Corner | `bg_events` signs whose script ends in `_EventScript_SlotMachineN`, as at [CeladonCity_GameCorner_hns/map.json](../../game/data/maps/CeladonCity_GameCorner_hns/map.json) lines 186-193 (16 machines). The music (`MUS_GAME_CORNER`, `MUS_HG_GAME_CORNER`, or `MUS_RG_GAME_CORNER`) confirms. |
| Poké Mart or store | Shop-shelf tiles (`MetatileBehavior_IsShopShelf`, [metatile_behavior.c](../../game/src/metatile_behavior.c) line 1386: `MB_SHOP_SHELF`, `MB_SHOP_SHELF_DEPARTMENT`, `MB_SHOP_SHELF_DEPARTMENT_FORWARD`) and a clerk object (`OBJ_EVENT_GFX_MART_EMPLOYEE`, `_HNS`, or `OBJ_EVENT_GFX_CLERK`). `CeruleanCity_Mart_hns` has 12 `MB_SHOP_SHELF` tiles. |
| Gym | Gym music (`MUS_GYM`, `MUS_HG_GYM`, or `MUS_RG_GYM`) on the map the town door leads into. |

The Game Corner check comes before the Mart check, since the HNS Game
Corner also uses department-shelf tiles. `MB_SLOT_MACHINE` exists
([metatile_behaviors.h](../../game/include/constants/metatile_behaviors.h),
line 142) but the Celadon HNS Game Corner's tiles don't use it, so slot
machines are found through their sign events.

**Standing tiles inside.** Each indoor kind finds its tiles as follows. A
tile must be walkable, free of objects and warps, and reachable from the
entrance with the walker's collision rules.

- **Center counter:** a tile facing an `MB_COUNTER` tile
  (`MetatileBehavior_IsCounter`, line 499), other than the tile in front
  of the nurse, which stays free for the player.
  `CeruleanCity_PokemonCenter_hns` has six counter tiles.
- **Center side:** a tile next to a side wall, facing into the room. No
  seat or bench behaviour exists in
  [metatile_behaviors.h](../../game/include/constants/metatile_behaviors.h),
  so these come from the [authored list](#authored-overrides) where a
  Center has visible seats, and from the wall rule otherwise.
- **Mart and store:** a tile facing a shop-shelf tile. A **store** is the
  door's interior plus every interior reachable from it by interior warps
  (stairs, escalators, elevators) that also has shop-shelf tiles, such as
  the Celadon, Goldenrod, and Lilycove department stores. Each floor is its own
  interior for [capacity](#capacity).
- **Game Corner:** for each slot-machine sign, the tile the player stands
  on to use it: one step back from the sign along its `player_facing_dir`,
  facing the sign. The sign at (5, 6) with `BG_EVENT_PLAYER_FACING_EAST`
  gives the standing tile (4, 6), facing east.
- **Gym:** no standing tile; a Gym spot is the Gym's exit warp, used by the
  [exit-spawn rule](#just-leaving).

**Tall grass.** `MetatileBehavior_IsTallGrass`
([metatile_behavior.c](../../game/src/metatile_behavior.c), line 763) is
true for `MB_TALL_GRASS`
([metatile_behaviors.h](../../game/include/constants/metatile_behaviors.h),
line 7), `MB_CYCLING_ROAD_PULL_DOWN_GRASS`, and
`MB_TALL_GRASS_IMPASSABLE_NORTH`. A spot is one **patch**: a 4-connected
group of tall-grass tiles, at least a placeholder 6 tiles, that a walker
can reach. `Route1_hns` has 206 tall-grass tiles, and `Route25_hns` 41.
Long grass (`MB_LONG_GRASS`, line 8) is left out.

**Water's edge.** A walkable land tile with a 4-neighbour for which
`MetatileBehavior_IsSurfableFishableWater`
([metatile_behavior.c](../../game/src/metatile_behavior.c), line 1198) is
true: `MB_POND_WATER`, `MB_OCEAN_WATER`, `MB_DEEP_WATER`,
`MB_INTERIOR_DEEP_WATER`, `MB_SOOTOPOLIS_DEEP_WATER`, and the four
currents (the water behaviours start at line 21 of
[metatile_behaviors.h](../../game/include/constants/metatile_behaviors.h)).
It is the same check the rods use
([fishing.c](../../game/src/fishing.c), line 583), so a trainer only
"fishes" where the player could. The trainer faces that water tile.
`Route25_hns` has 244 ocean and 42 pond tiles, and `CeladonCity_hns` 18
pond tiles. Ledges and warp tiles never qualify.

**Town squares and benches.** Maps whose `map_type` is `MAP_TYPE_TOWN` or
`MAP_TYPE_CITY` (as at
[CeruleanCity_hns/map.json](../../game/data/maps/CeruleanCity_hns/map.json)
line 9; 51 town and 37 city maps): a walkable tile at the centre of a 3×3
block of walkable, non-grass, non-water tiles, at least a placeholder 3
tiles from any warp, sign, or object. This is the most ambiguous kind:
the rule can pick an alley or a path that only looks open, and benches and
lookouts have no metatile behaviour of their own. Benches and lookouts are
therefore an [authored list](#authored-overrides), and the same list can
drop a bad square.

**Chatting with an NPC.** An `object_events` entry in the map's
`map.json` that is a person standing still:

- `trainer_type` is `TRAINER_TYPE_NONE`;
- `movement_type` keeps them in place (`MOVEMENT_TYPE_FACE_*` or
  `MOVEMENT_TYPE_LOOK_AROUND`, as the gentleman at
  [CeruleanCity_PokemonCenter_hns/map.json](../../game/data/maps/CeruleanCity_PokemonCenter_hns/map.json)
  lines 32-36);
- `flag` is `"0"`, so they are always present; flagged objects come and go
  with story state and are left out;
- they are not a nurse, clerk, item ball, berry tree, or Pokémon; and
- a free walkable tile is 4-adjacent to them.

The spot is that adjacent tile, facing the NPC. The NPC itself is never
moved or changed.

**Tile attributes.** Behaviour bytes come from each layout's blockdata and
its tilesets' `metatile_attributes.bin`. The formats differ between
tileset families: HNS layouts use 640 primary metatiles
([fieldmap.h](../../game/include/fieldmap.h), lines 24-25), and FRLG
attributes are masked differently
([fieldmap.c](../../game/src/fieldmap.c), line 144). The extraction reads
each family as the engine does;
[e2e_collision/export.py](../../game/tools/e2e_collision/export.py) is a
working reader for HNS layouts.

### Authored overrides

One small authored file per region adds what detection can't see and
removes what it gets wrong:

- **Add:** bench and lookout tiles (map, tile, facing), and Center side
  seats, all as kind 7 or kind 1;
- **Drop:** any detected spot by map and tile, for a bad square, a tile
  that blocks a scripted scene, or a duplicate.

The list starts empty apart from benches and lookouts, and grows only when
a playtest or a review finds a bad spot.

### Capacity

Capacity caps notable trainers per **map**, the per-place cap from the
routine design:

| Map | Notables at once |
| --- | ---: |
| Interior (`map_type` `MAP_TYPE_INDOOR`) | 1 |
| Outdoor (every other map type) | 3 (placeholder; 2 if the object or search budget needs it) |

The cap counts every notable whose current destination is on that map,
whether at a spot or a haunt. Each spot also holds at most one trainer, and
a haunt keeps its own [capacity](notable-haunts.md#capacity). A department
store floor and a Gym are interiors, so each holds one. The cap is
enforced when choosing a spot: a full spot or full map is never chosen.

### Activities and kinds

The [activity list](notable-haunts.md#activities) gains two activities for
spots: **fish** and **visit**. v0 haunt tags are unchanged. Each activity
picks the spot kinds it can use:

| Activity | Kinds |
| --- | --- |
| care | Pokémon Center |
| shop | Poké Mart and department store |
| gamble | Game Corner |
| train | tall grass (plus haunts tagged train) |
| relax | town squares and benches (plus waterside haunts tagged relax) |
| fish | water's edge |
| visit | other Gyms, chatting with an NPC |

Study, home, lie low, and sightsee have no spot kind; a routine step with
one of them goes to a haunt, or, for home, is up to the routine design.
Whether a train or relax step lands on a spot or a haunt is the routine's
choice; this spec only lists both as candidates.

### Choosing a spot

When a routine step gives a trainer an activity, the spot is a pure
function of:

- the trainer and their activity;
- their **home base** and search radius (from the routine design);
- the current occupancy: every other notable's destination, from the
  travel records and the haunt placement;
- world progress (`GetTrainerRating()`); and
- content (the spot table, the authored overrides, and favourites).

Steps:

1. **Kinds.** Take the activity's kinds from the table above.
2. **Favourites.** If the trainer has a [favourite](#favourites) of one of
   those kinds with room, take the first in authored order and stop.
3. **Candidates.** Every spot of those kinds within the radius of the home
   base, counted in walker-graph map hops: a placeholder 3 hops, or 8 for
   a traveller. Drop any spot that is taken, or on a map at its
   [capacity](#capacity). For visit, drop the Gym whose leader is this
   trainer (their badge encounter's Gym), and an NPC chat on a map where
   another notable is already chatting.
4. **Pick.** The nearest candidate wins. Ties go in spot-table order,
   starting at entry `(c + wp) mod n` and wrapping, where `c` is the
   trainer's position in the trainer catalog order, `wp` is world
   progress, and `n` the number of tied spots. So two trainers with the
   same home base spread out, and the choice shifts as the journey goes on.
5. **None.** With no candidate, the routine moves on to its next step.

Nothing is random and nothing new is saved: the same inputs give the same
spot.

### Favourites

A trainer value, alongside buddy and reward pool: up to **3 favourite spot
references**, each a map and a kind, and for a tile-precise habit, a tile.
A favourite is an optional override for a signature habit:

- Lt. Surge: the Celadon Game Corner (`CeladonCity_GameCorner_hns`,
  gamble);
- Erika: the Celadon department store's perfume floor (shop).

A favourite is used whenever it fits the activity and has room; otherwise
the derived pick runs. Favourites are **authored later, with routines**;
none exist now.

### Just leaving

A Gym visit plays as an exit, never as a trainer standing in the Gym:

1. When the player enters a Gym whose spot holds a visitor, the visitor
   spawns on a free tile within a placeholder 3 tiles of the Gym's exit
   warp (for Cerulean, the warp at (7, 18),
   [CeruleanCity_Gym_hns/map.json](../../game/data/maps/CeruleanCity_Gym_hns/map.json)
   lines 150-155), off the player's arrival tile.
2. Once the exit tile is free, the visitor walks onto it and leaves
   through the door, and the travel record moves them on.
3. Talking to them on the way runs the [talk flow](#talk-flow); afterwards
   they carry on out.

Rules:

- A Gym's own leader is never a visitor there
  ([choosing a spot](#choosing-a-spot), step 3).
- Visiting a Gym whose leader the player hasn't beaten is allowed. The
  visitor never stands between the player and the leader or a Gym
  trainer, never blocks the badge battle, and never triggers a battle.
- A visitor appears at most once per entry: leaving and re-entering finds
  the Gym empty until the routine sends someone again.

### Talk flow

Talking to a trainer at a spot runs three steps, with no menu:

1. **Greeting**, by friendship stage, exactly as the haunt
   [talk flow](notable-haunts.md#talk-flow) picks it (`MEET` or `HEARD`,
   `AGAIN`, `HELLO`, `CLOSE`).
2. **Follow-up or activity line.** The follow-ups play at spots as at
   haunts, in the same order, at most one per talk:
   1. [Courier delivery](notable-haunts.md#delivery), while the trainer is
      the active parcel's recipient;
   2. the [Egg sitting follow-up](notable-haunts.md#the-follow-up), while
      the trainer has an outstanding egg; and
   3. the [Courier trail](notable-haunts.md#refreshing-the-trail), while
      the trainer is the active parcel's sender.

   Otherwise, the spot kind's activity line.
3. **Farewell:** `BYE`.

There are no quests, battles, rewards, or `QUIRK`s at spots, except a
follow-up's own reward. A **meeting** at a spot is one stay: from the
trainer's arrival there until they leave. The Egg sitting follow-up and
the Courier trail play once per stay, as they play once per placement at a
haunt.

**Activity lines.** One shared line per kind, neutral, describing only the
activity, never a trainer's personality or a favour, so every trainer can
say it; personality stays in the greeting's voice bit. Placeholders for
review:

| Kind | Activity line |
| --- | --- |
| Pokémon Center | "Just letting my team rest up." |
| Poké Mart and department store | "Just stocking up on supplies." |
| Game Corner | "Trying my luck for a while." |
| Other Gyms | "Just dropped by to see the Gym." |
| Tall grass | "Seeing what turns up in this grass." |
| Water's edge | "Waiting for a bite." |
| Town squares and benches | "Taking a breather." |
| Chatting with an NPC | "Just catching up with someone here." |

### Saved state

Spots add little or none, beyond what routines and travel will save. The
spot table, overrides, and favourites are ROM content. Spot choice is
derived from the inputs above, so a trainer's spot follows from their
travel record and is never stored on its own. The once-per-stay follow-up
and trail bits belong in that record, and clear when the trainer arrives
somewhere new.

## Engine notes

What building spots needs, on top of the routine and travel work the
[travel proof of concept](../research/notable-trainer-travel-poc.md)
validated ([travel engine note](notable-haunts.md#travel-and-on-map-walking)).

### Spot extraction

A build-time tool reads every playable map's `map.json` (`warp_events`,
`object_events`, `bg_events`, `map_type`, `music`), `layouts.json`,
blockdata, and metatile attributes, applies the detection rules and the
authored overrides, and writes one spot table **per map**: for each spot
its kind, tile (or patch), facing, and kind details (store, Gym exit warp,
NPC object). The output order is fixed (kind, then y, then x), so spot
tables are stable between builds. It runs beside the existing per-map
generation
([map_data_rules.mk](../../game/map_data_rules.mk), lines 93-94).

Only maps the player can reach in Wayfarer are scanned, starting from the
regions' playable maps and following connections and warps, so unused
donor copies of a building never yield spots. The tool reports per-kind
counts per map, so detection gaps show up in review.

### Walker behaviours

The proof of concept's walker walks to a tile and re-plans when blocked.
Spots add:

- **Face and dwell:** stand on the spot tile facing its direction.
- **Roam a patch:** walk to another tile of the same tall-grass patch after
  each dwell, chosen by the same deterministic rotation as the
  [pick](#choosing-a-spot).
- **Emotes:** "!" with `MOVEMENT_ACTION_EMOTE_EXCLAMATION_MARK`
  ([event_object_movement.h](../../game/include/constants/event_object_movement.h),
  line 177). "…" needs an emote frame: `FLDEFF_EMOTE`
  ([field_effects.h](../../game/include/constants/field_effects.h),
  line 38) shows the follower emotes, and whether one reads as "…" is
  unchecked; the question mark is the fallback.
- **Floors:** take an interior warp (stairs, escalator) between store
  floors. The proof of concept only handled south-facing doors, so these
  warps need their own collision exception and approach facing.

### Interiors

A trainer at an indoor spot is **inside** in their travel record: the
record holds how they arrived (by door), as in the proof of concept's
phase 2, and the local actor spawns them when the player enters that
interior. Heartbeat time only moves on map changes, so a trainer inside
can't come out while the player waits at the door; that needs the
gameplay timer the proof of concept named.

### Risks

- **Object slots.** The pool is 16 (`OBJECT_EVENTS_COUNT`,
  [global.h](../../game/include/constants/global.h), line 139). Centers
  and Marts already spend slots on the nurse, clerks, and visitors, and a
  walker keeps its slot while hidden. The interior cap of 1 is there for
  this.
- **Following Pokémon.** `OW_FOLLOWERS_ENABLED` is `TRUE`
  ([overworld.h](../../game/include/config/overworld.h), line 61), and the
  proof of concept switched the following Pokémon off to free slots. A
  rule for it is needed before spots run
  ([open risk](notable-haunts.md#open-risk-overworld-followers)).
- **Detection gaps.** Benches and lookouts need the authored list; open
  squares can misfire; multi-room Gyms and unusual names need the content
  signals, not names; the Two Island Joyful Game Corner has Game Corner
  music but no slot machines, so it yields no spot; and tileset attribute
  formats differ by family.

## Open questions

- Whether a first talk at a spot counts as the +1 "first talk" friendship
  event, as a haunt's does
  ([friendship](notable-trainers.md#friendship)). It would let a Courier
  delivery at a spot greet a Stranger exactly as at a haunt.
- Once routines run, whether a Courier recipient can be anyone out at a
  spot, not only at a haunt, and what `{PLACE}` says for a spot: the map's
  location name from `region_map_section` (such as `MAPSEC_CERULEAN_CITY`)
  is the likely source.
- What a home base is for a trainer who isn't a Gym Leader, and the real
  radius values; both belong to the routine design.

## Later

- Authoring favourites, with routines.
- More spot kinds, such as libraries, harbours, or Day Care fences.
- Per-trainer activity lines, if the shared ones get stale.

## References

- [Notable spots PRD](../prds/notable-spots.md)
- [Notable haunts](notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Notable trainer travel proof of concept](../research/notable-trainer-travel-poc.md)
