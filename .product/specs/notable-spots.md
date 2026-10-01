# Notable spots

PRD: [Notable spots](../prds/notable-spots.md)
Implemented: No
Design status: the everyday layer of the future **routine and travel**
design. Spots switch on together with routines and travel; v0 haunt
placement is unchanged, and nothing in this spec runs before then. Spots
are found automatically from map data, plus a short authored list of
[named spots](#named-spots), in one pool for every region, and carry no
quests. Home bases are decided, and a first talk at a spot counts
for friendship. Capacity values, the search radius, and the activity lines
are placeholders.

## Scope

Own, in `IS_WAYFARER`: the spot kinds and how each is detected from map
data, the build-time spot extraction and its authored overrides, the named
spots and their validation, spot capacity, the activity-to-kind mapping
and the two new activities (**fish** and **visit**), choosing a spot, home
bases and favourites as trainer values, the talk flow at spots and its
activity lines, the walker behaviours each kind needs, the Gym exit-spawn
rule, and the saved-state stance.

- [Notable haunts](notable-haunts.md) owns the haunts, their placement and
  quests, the [activity list](notable-haunts.md#activities) that spots
  extend, the [talk flow](notable-haunts.md#talk-flow) whose greeting and
  follow-up steps spots reuse, and the
  [travel engine note](notable-haunts.md#travel-and-on-map-walking).
- The routine design (future work) owns activity cycles: when a trainer
  does which activity, travel between maps, the world record,
  and the heartbeat. This spec owns only what happens once an activity is
  chosen. The
  [travel proof of concept](../research/notable-trainer-travel-poc.md) is
  its evidence so far.
- [Notable trainers](notable-trainers.md) owns friendship and its events,
  and holds home bases, and favourites once they are authored, in the
  catalog.
- [Gym Leader scaling](gym-leader-scaling.md) owns badge-encounter
  coverage, which names each Gym's leader.

## Behavior

### Spots

A **spot** is one standing tile (or, for tall grass, a patch of tiles) on
one map, with a facing and a **kind**. The eight detected kinds are never
authored one by one: the [spot extraction](#spot-extraction) finds them in
map data at build time, across every playable map of Kanto, Johto, Hoenn,
and Sevii, as one pool. The ninth kind, [named spots](#named-spots), is a
short authored list of one-off places that the extraction adds to the same
pool. A spot names no trainer and offers no quest. It holds one trainer at
a time, except a named spot with a capacity of 2.

**Haunts** stay the authored special places with quests. A haunt's standing
tile is never also a spot; the extraction drops any spot that lands on one.
In the same way, a detected spot on a named spot's tile is dropped.
Trainers spend most of their time at spots, and haunts are the highlights.

### Spot kinds

Each kind lists its detection source and what the trainer does there. A
trainer only does what the walker can do: walk to a tile, face a
direction, show an emote, and pass through a door. "Occasional" means a
placeholder 1 in 8 dwell ticks.

| # | Kind | Detection | What the trainer does |
| ---: | --- | --- | --- |
| 1 | Pokémon Center | A door whose destination has a nurse and a counter. | Stands facing the counter, or stands still at the side of the room ("sitting"). |
| 2 | Poké Mart and department store | A door into a store: its interior plus the floors reached from it, with shelves somewhere and a vendor somewhere. | Faces a shelf; now and then takes the stairs or escalator to another floor. |
| 3 | Game Corner | A door whose destination is a Game Corner. | Stands at a slot machine, facing it. |
| 4 | Other Gyms | A door whose destination is a Gym. | "Just leaving" ([below](#just-leaving)). |
| 5 | Tall grass | Tall-grass metatile behaviour. | Roams the patch, with an occasional "!". |
| 6 | Water's edge | Land tiles beside fishable water. | Faces the water in a "fishing" pose, with an occasional "!". |
| 7 | Town squares and benches | Open areas of town and city maps, plus an authored list of bench and lookout tiles. | Idles and looks around. |
| 8 | Chatting with an NPC | An existing NPC object on the map. | Stands adjacent, facing them, with an occasional "…". |
| 9 | Named spot | An authored row ([named spots](#named-spots)). | Stands on the authored tile, facing what's beside it. |

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
interior:

- the source's `map_type` is `MAP_TYPE_TOWN`, `MAP_TYPE_CITY`,
  `MAP_TYPE_ROUTE`, or `MAP_TYPE_OCEAN_ROUTE`, and the source isn't a
  **rooftop**: an outdoor-typed map with no map connections whose every
  warp leads to an interior that has no warp to an outdoor-typed map
  (`CeladonCity_Apartments_RoofDay_hns`,
  `CeladonCity_DepartmentStore_RoofDay_hns`, and
  `GoldenrodCity_DepartmentStore_7F_hns`);
- the destination's `map_type` is `MAP_TYPE_INDOOR` or `MAP_TYPE_NONE`.
  HNS types some interiors `MAP_TYPE_NONE`, among them
  `ViridianCity_PokemonCenter_hns`.

The warp's destination is the signal, not the door tile: most door tiles
carry the collision bit, and many outdoor-to-interior warps sit on arrow
or plain tiles rather than a door behaviour
([inventory](../research/notable-spots-inventory.md#hns-frlg-and-emerald-differences)).

**Classifying the interior.** Map names are not reliable:
`MAP_CIANWOOD_POKECENTER_HNS` breaks the usual `_POKEMON_CENTER` pattern,
`MAP_CIANWOOD_SHOP_HNS` is not a Mart at all but the Cianwood pharmacy
(shelves, no vendor, and a script that gives the Secret Potion), and HNS,
FRLG, and Emerald copies of the same building coexist
(`CeladonCity_GameCorner_hns` and `CeladonCity_GameCorner_Frlg`). The
extraction classifies a destination by its content, whatever its map type,
first match wins:

| Kind | Signal in the destination's `map.json` and layout |
| --- | --- |
| Pokémon Center | An object whose `graphics_id` is a nurse (`OBJ_EVENT_GFX_NURSE`, `_FRLG`, `_HNS`, or `_CHANSEY_HNS`), as at [CeruleanCity_PokemonCenter_hns/map.json](../../game/data/maps/CeruleanCity_PokemonCenter_hns/map.json) line 19, **and** at least one `MB_COUNTER` tile, on a map without Gym music. 52 maps in Wayfarer's scope carry a nurse; the Fighting Dojo's rematch nurse stands in a room with Gym music and no counter, so it isn't a Center. |
| Game Corner | `bg_events` signs whose script ends in `_EventScript_SlotMachineN`, as at [CeladonCity_GameCorner_hns/map.json](../../game/data/maps/CeladonCity_GameCorner_hns/map.json) lines 186-193 (16 machines). The music (`MUS_GAME_CORNER`, `MUS_HG_GAME_CORNER`, or `MUS_RG_GAME_CORNER`) confirms. |
| Poké Mart or store | The [store rule](#stores) below: shelves somewhere in the store and a vendor somewhere. `CeruleanCity_Mart_hns` has 12 `MB_SHOP_SHELF` tiles and a clerk. |
| Gym | Gym music (`MUS_GYM`, `MUS_HG_GYM`, or `MUS_RG_GYM`) on the map the town door leads into. |

The Game Corner check comes before the store check, since the HNS Game
Corner also uses department-shelf tiles, and the store check skips any map
that is a Center, a Game Corner, or a Gym. The
[authored drop list](#authored-overrides) removes two detected places:
`GoldenrodCity_BikeShop_hns` (shelves and a clerk, but not somewhere to
stock up; it's a named spot) and `SaffronCity_FightingDojo_hns` (Gym music
but no Gym Leader; it's a haunt and a named spot).

#### Stores

A **store** is the door's interior plus every interior reached from it by
interior warps (stairs, escalators), followed map to map, never through an
outdoor map or a Center, Game Corner, or Gym, into maps that have shelves
or a vendor. It is a store when:

- **Shelves anywhere:** at least one of its maps has shelf tiles. A shelf
  is a tile for which `MetatileBehavior_IsShopShelf`
  ([metatile_behavior.c](../../game/src/metatile_behavior.c) line 1386:
  `MB_SHOP_SHELF`, `MB_SHOP_SHELF_DEPARTMENT`,
  `MB_SHOP_SHELF_DEPARTMENT_FORWARD`) is true, or, on an FRLG layout in a
  map with a vendor, `MB_POKEMON_CENTER_BOOKSHELF`: the engine's FRLG
  normalisation
  ([fieldmap.c](../../game/src/fieldmap.c), `NormalizeFrlgMetatileBehavior`)
  turns FRLG Mart shelves into that behaviour, so without this no FRLG Mart
  is found (`CinnabarIsland_Mart_Frlg` and the Sevii Marts have 12 each).
- **A vendor somewhere:** one of its maps has a clerk object
  (`OBJ_EVENT_GFX_MART_EMPLOYEE`, `_HNS`, or `OBJ_EVENT_GFX_CLERK`) or an
  object whose script opens a shop (a `pokemart` command), as Lilycove's
  sales staff (`WOMAN_3`, `COOK`) and the Lavaridge Herb Shop do.

The store's **floors** are its maps with shelves; only they hold spots.
The ground floors of the Celadon, Goldenrod, and Lilycove department
stores have no shelves, so they are part of the store but not floors.
Elevator cars have neither shelves nor a vendor, so they aren't part of
the store; every floor in scope is also reached by stairs or escalators. A
store is keyed by its set of floors, not its door, so a second door into
the same floors finds the same store. `MB_SLOT_MACHINE` exists
([metatile_behaviors.h](../../game/include/constants/metatile_behaviors.h),
line 142) but the Celadon HNS Game Corner's tiles don't use it, so slot
machines are found through their sign events.

**Standing tiles inside.** Each indoor kind finds its tiles as follows. A
tile must be walkable, free of objects and warps, and reachable from the
entrance with the walker's collision rules.

- **Center counter:** a tile facing an `MB_COUNTER` tile
  (`MetatileBehavior_IsCounter`, line 499), other than the tile in front
  of the nurse, which stays free for the player.
  `CeruleanCity_PokemonCenter_hns` has six counter tiles. Emerald Centers
  mark only the tile in front of the nurse as `MB_COUNTER` and the rest of
  the counter as plain collision, so where a map has exactly one
  `MB_COUNTER` tile, the **counter row** is that tile plus the unbroken run
  of tiles with the collision bit set on either side of it in the same
  row, and the standing tiles are the free tiles directly beyond that row
  on the side away from the nurse (minus the tile in front of the nurse).
  `RustboroCity_PokemonCenter_1F` has its counter tile at (7, 3) and a row
  from (4, 3) to (9, 3), which gives five tiles on row 4.
- **Center side:** a tile next to a side wall, facing into the room. No
  seat or bench behaviour exists in
  [metatile_behaviors.h](../../game/include/constants/metatile_behaviors.h),
  so these come from the [authored list](#authored-overrides) where a
  Center has visible seats, and from the wall rule otherwise.
- **Mart and store:** a tile on a store floor facing a shelf tile, as the
  [store rule](#stores) counts shelves. Each floor is its own interior for
  [capacity](#capacity).
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
([fishing.c](../../game/src/fishing.c), line 583), so a trainer stands
where the player could cast a line. The trainer faces that water tile.
`Route25_hns` has 244 ocean and 42 pond tiles, and `CeladonCity_hns` 18
pond tiles. Ledges and warp tiles never qualify. The map must also:

- not be an interior (`MAP_TYPE_INDOOR` or `MAP_TYPE_NONE`), which drops
  pools such as the Cerulean Gym's and the Battle Palace's; and
- not have Gym music.

The map doesn't need fishing encounters. Standing by a pond is a fine
place to relax or fish even where no rod gets a bite, so decorative ponds
such as those on `Route38_hns` and `MtMoon_Outside_hns` count. There the
"fishing" pose is flavour: the trainer only faces the water and stands
still, so nothing claims a bite is possible. Caves keep their edges too.

**Town squares and benches.** Maps whose `map_type` is `MAP_TYPE_TOWN` or
`MAP_TYPE_CITY` (as at
[CeruleanCity_hns/map.json](../../game/data/maps/CeruleanCity_hns/map.json)
line 9), other than [rooftops](#detection-sources): 49 maps in Wayfarer's
scope, 47 without the two rooftops HNS types `MAP_TYPE_CITY`. A **centre**
is a walkable tile at the middle of a 3×3 block of walkable, non-grass,
non-water tiles, at least a placeholder 3 tiles (Chebyshev distance) from
any warp, sign, or object. A **square** is an 8-connected group of
centres. Each map keeps at most a placeholder **3 squares**: the largest by
centre count, ties broken by the group's top-left tile (smallest y, then
x). A square's standing tile is its centre nearest the group's mean
position, ties to the smaller y, then x. Without the cap, the rule finds
every wide path (Cerulean alone has 22 squares). This is still the most
ambiguous kind: the rule can pick an alley or a path that only looks open,
and benches and lookouts have no metatile behaviour of their own. Benches
and lookouts are therefore an [authored list](#authored-overrides), and
the same list can drop a bad square.

**Chatting with an NPC.** An `object_events` entry in the map's
`map.json` that is a person standing still:

- it is a plain object, not a clone;
- `trainer_type` is `TRAINER_TYPE_NONE`;
- `flag` is `"0"`, so they are always present; flagged objects come and go
  with story state and are left out;
- they are not a nurse or clerk;
- they are not a **notable cameo**, a sprite named after a catalog trainer
  (such as `OBJ_EVENT_GFX_MISTY_HNS`);
- they are not a **villain team** member (Rocket, Aqua, and Magma grunts
  and admins), who stand as story blockers even with no flag;
- they are not a **Pokémon**: a `MON_BASE+SPECIES_*` graphic, or a sprite
  named after a species;
- they are not an **item or prop**: item and Poké Balls, berry trees, cut
  trees, rocks, boulders, lights, `VAR_` sprites, vehicles, and dolls;
- `movement_type` keeps them in place: `MOVEMENT_TYPE_FACE_*` or
  `MOVEMENT_TYPE_LOOK_AROUND`, as the gentleman at
  [CeruleanCity_PokemonCenter_hns/map.json](../../game/data/maps/CeruleanCity_PokemonCenter_hns/map.json)
  lines 32-36. `MOVEMENT_TYPE_NONE` is left out too: HNS uses it mostly
  for lights and items, and the people among them (such as kimono girls)
  can go in the authored list if a review wants them;
- they are not a **scripted actor**: their `local_id` isn't the target of
  `applymovement`, `removeobject`, `addobject`, `setobjectxy`, and the
  like in the map's scripts; and
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
  that blocks a scripted scene, or a duplicate; or a whole detected place
  by map and kind, which drops every spot that place would give.

The drop list starts with two places: `GoldenrodCity_BikeShop_hns` as a
store and `SaffronCity_FightingDojo_hns` as a Gym (see
[classifying the interior](#detection-sources)). Otherwise the list starts
empty apart from benches and lookouts, and grows only when a playtest or a
review finds a bad spot. [Named spots](#named-spots) are a separate list.

### Named spots

A **named spot** (kind 9) is a one-off place with character that no
detection rule can see: a lab, a tower, a café, a Day Care, a harbour. They
are authored as one small list next to the eight detected kinds, and join
the same pool. The starting list, 59 rows across the four regions, is
[notable named spots](../research/notable-named-spots.md); its Haunt column
marks the rows whose map also hosts a haunt.

Each row authors:

| Field | Values | Meaning |
| --- | --- | --- |
| Spot | a short label | Names the row for review, such as "Oak's Lab"; never shown in game. |
| Map | a map name | The variant Wayfarer plays: HNS for Kanto and Johto, FireRed and LeafGreen for Sevii and the Kanto ports, Emerald for Hoenn. |
| Tile | `(x, y)` | Where the trainer stands. |
| Activities | one or two from the [activity list](notable-haunts.md#activities), with the spots' fish and visit | What a trainer does there: train, care, study, home, relax, shop, gamble, sightsee, fish, visit, or lie low. |
| Capacity | 1, or 2 outdoors | How many trainers it holds at once. |
| Note | a short phrase | Why the place is there; for review only, never shown in game. |

The row has no facing of its own: the trainer faces the first 4-neighbour
of the tile (up, left, right, down) that holds an object, a sign, a
counter, a shelf, or fishable water, and faces down otherwise. A second
trainer at a capacity-2 spot stands on the first free 4-neighbour of the
tile, in the same order.

**Validation.** The extraction checks every row at build time and fails
the build on any of these:

- the map doesn't exist, isn't in Wayfarer's scope (the
  [inventory's scoping rule](../research/notable-spots-inventory.md#scope):
  selected by mapjson for Wayfarer, in one of the four regions, and
  reachable), or is in a different region from the row's list;
- the tile is outside the map, not walkable land, not reachable from the
  map's warps and edges with the walker's collision rules, or holds an
  object or a warp;
- the tile is a haunt's standing tile, or another named spot's tile;
- there are no activities, more than two, or one not on the list;
- the capacity is above 1 on an interior (`MAP_TYPE_INDOOR` or
  `MAP_TYPE_NONE`) or above 2 elsewhere, or a capacity-2 spot has no free
  4-neighbour for its second trainer.

A detected spot on a named spot's tile is dropped, as on a haunt's tile.
A map can hold both a haunt and named spots, as long as their tiles differ;
haunt authors check the named-spot list in turn.

### Capacity

Capacity caps notable trainers per **map**, the per-place cap from the
routine design:

| Map | Notables at once |
| --- | ---: |
| Interior (`map_type` `MAP_TYPE_INDOOR` or `MAP_TYPE_NONE`) | 1 |
| Outdoor (every other map type) | 3 (placeholder; 2 if the object or search budget needs it) |

The cap counts every notable whose current destination is on that map,
whether at a spot, a named spot, or a haunt. Each spot also holds at most
one trainer, except a named spot with capacity 2, and a haunt keeps its own
[capacity](notable-haunts.md#capacity). A department store floor and a Gym
are interiors, so each holds one. So does an interior that hosts both a
haunt and a named spot, such as the Fighting Dojo: while a trainer is at
the haunt, the named spot is full. The cap is enforced when choosing a
spot: a full spot or full map is never chosen.

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
| relax | town squares and benches, water's edge (plus waterside haunts tagged relax) |
| fish | water's edge |
| visit | other Gyms, chatting with an NPC |

Every activity also takes the [named spots](#named-spots) that list it.
Study, home, lie low, and sightsee have no detected kind, so named spots
(and haunts) are their only spots; a step with home and no named spot is
up to the routine design. Whether a step lands on a spot or a haunt is the
routine's choice; this spec only lists both as candidates.

### Choosing a spot

When a routine step gives a trainer an activity, the spot is a pure
function of:

- the trainer and their activity;
- their [home base](#home-base) and the search radius;
- the current occupancy: every other notable's destination, from the
  travel records and the haunt placement;
- world progress (`GetTrainerRating()`); and
- content (the spot table, the authored overrides, and favourites).

Steps:

1. **Kinds.** Take the activity's kinds from the table above, and the
   named spots that list the activity.
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

### Home base

A trainer's **home base** is the one map that spot choice measures from. It
is a trainer value in the notable-trainer catalog, alongside buddy and
reward pool
([haunt and routine values](notable-trainers.md#haunt-and-routine-values)),
and this spec owns what it means.

- **Gym Leaders:** their Gym city, the town or city map whose door leads
  into their Gym ([Gym Leader scaling](gym-leader-scaling.md) names each
  Gym's leader). Giovanni is a Gym Leader in the catalog, so his is
  Viridian City. Tate & Liza's is Mossdeep City, though they are never
  placed.
- **Everyone else:** one authored home map, grounded in canon:

| Trainer | Home map | Why |
| --- | --- | --- |
| Blue | `PalletTown_hns` | His hometown. |
| Lorelei | `FourIsland_Frlg` | Her home in FireRed and LeafGreen. |
| Bruno | `OneIsland_Frlg` | Where he appears in FireRed and LeafGreen. |
| Agatha | `LavenderTown_hns` | The ghost town, home of the Pokémon Tower. |
| Koga | `FuchsiaCity_hns` | His old Gym city, now his daughter Janine's. |
| Lance | `BlackthornCity_hns` | His dragon-clan home. |
| Will | `IndigoPlateau_hns` | His post at the League. |
| Karen | `IndigoPlateau_hns` | Her post at the League. |
| Sidney | `EverGrandeCity` | His post at the Hoenn League. |
| Glacia | `EverGrandeCity` | Her post at the Hoenn League. |
| Phoebe | `MtPyre_Summit` | Where her grandmother keeps the orbs. |
| Drake | `LilycoveCity` | A sailor by the sea. |
| Wallace | `SootopolisCity` | His home city and former Gym. |
| Steven | `MossdeepCity` | His house (`MossdeepCity_StevensHouse`). |

Each map is the variant Wayfarer plays: HNS for Kanto and Johto, FireRed
and LeafGreen for Sevii, Emerald for Hoenn. Steven's home map is the city
that holds his house. The 3-hop and 8-hop radii in
[step 3](#choosing-a-spot) stay placeholders.

### Favourites

A trainer value, alongside buddy and reward pool: up to **3 favourite spot
references**, each a map and a kind, and for a tile-precise habit, a tile.
A favourite can name a named spot by its map and tile.
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
   `AGAIN`, `HELLO`, `CLOSE`). A first talk at a spot is the +1 "first
   talk" [friendship](notable-trainers.md#friendship) event, as at a haunt:
   friendship lives on the trainer, so meeting them anywhere counts.
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

A named spot says the line of the activity the trainer is doing there:
the matching kind's line above for care, shop, gamble, train, relax, fish,
and visit, and for the rest:

| Activity | Activity line |
| --- | --- |
| study | "Just reading up on a few things." |
| sightsee | "Just taking in the view." |
| lie low | "Keeping my head down for a while." |
| home | "Just spending some time at home." |

### Courier recipients at spots

Once routines run, a [Courier recipient](notable-haunts.md#the-recipient)
can be a trainer out at a spot, not only one at a haunt. `{PLACE}` then
says the spot map's own location name: the name of its region-map section,
such as "CERULEAN CITY" or "ROUTE 2". Spots have no authored name of their
own, unlike a haunt's Name; a named spot's Spot column is a review label.

The source is each map's `region_map_section`
([CeruleanCity_hns/map.json](../../game/data/maps/CeruleanCity_hns/map.json),
line 6: `MAPSEC_CERULEAN_CITY`), which mapjson writes into the map header
([mapjson.cpp](../../game/tools/mapjson/mapjson.cpp), line 1185) as
`regionMapSectionId`. `GetMapName`
([region_map.c](../../game/src/region_map.c), line 2652) turns a section
into its name, such as "CERULEAN CITY"
([region_map_sections.json](../../game/src/data/region_map/region_map_sections.json),
lines 728-729). The debug warp menu reads another map's name the same way,
through `Overworld_GetMapHeaderByGroupAndId`
([debug.c](../../game/src/debug.c), line 1543). One catch: `GetMapName`
looks names up in `GetActiveRegionMapEntries` (line 306), which picks the
Hoenn, Sevii, or Kanto and Johto table from the *player's* current map.
Naming a recipient on a map from another region needs the table for that
map's region, not the player's.

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
blockdata, and metatile attributes, applies the detection rules, the
authored overrides, and the named spots (after
[validating](#named-spots) them), and writes one spot table **per map**:
for each spot its kind, tile (or patch), facing, and kind details (store,
Gym exit warp, NPC object, or a named spot's activities and capacity).
The output order is fixed (kind, then y, then x), so spot tables are
stable between builds. It runs beside the existing per-map
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
  formats differ by family. The
  [inventory](../research/notable-spots-inventory.md#remaining-issues)
  lists what the revised rules still get wrong.

## Later

- Authoring favourites, with routines.
- More detected kinds, such as libraries, if the named-spot list grows
  past what is comfortable to author.
- Per-trainer activity lines, if the shared ones get stale.

## References

- [Notable spots PRD](../prds/notable-spots.md)
- [Notable haunts](notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Notable trainer travel proof of concept](../research/notable-trainer-travel-poc.md)
- [Notable spots inventory (draft)](../research/notable-spots-inventory.md)
- [Notable named spots (starting list)](../research/notable-named-spots.md)
