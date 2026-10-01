# Notable spots inventory (draft)

Related PRD: [Notable spots](../prds/notable-spots.md)

Related spec: [Notable spots](../specs/notable-spots.md)

Related list: [Notable named spots](notable-named-spots.md)

Evidence: [inventory.py](notable-spots-inventory/inventory.py), a
read-only Python script (standard library only), and its outputs
[per-map.csv](notable-spots-inventory/per-map.csv) (one row per scanned
map, counts per kind under the revised rules) and
[inventory.json](notable-spots-inventory/inventory.json) (scope, totals
under both rule sets, findings, named-spot results, and the chosen NPCs
per map). Every number below comes from one run of the script against
`game/` at the time of writing:

    python3 .product/research/notable-spots-inventory/inventory.py

## Question

The spec finds spots automatically from map data. Before anyone writes the
real build-time tool: what pool do the spec's detection rules actually
produce on Wayfarer's maps, and where do the rules miss, misfire, or need
changing?

The first run answered that for the rules as first drafted and proposed
nine changes. The spec now carries those changes, plus an authored ninth
kind, [named spots](../specs/notable-spots.md#named-spots). This revision
re-runs the inventory under both rule sets and checks every named-spot row.

## Method

The script applies the spec's
[spot kinds](../specs/notable-spots.md#spot-kinds) and
[detection sources](../specs/notable-spots.md#detection-sources) twice:
the **draft** rules (the spec before this revision, as the first run read
it) and the **revised** rules (the spec now). It reports the counts under
both, and where the revised rules still disagree with the content.

### Scope

A map is scanned when all of these hold:

1. **Listed** in `game/data/maps/map_groups.json`, so mapjson builds it.
2. **Selected for Wayfarer.** A simplified port of
   `data_matches_version()` in `game/tools/mapjson/mapjson.cpp`: HNS and
   Emerald maps are in, minus the retired HNS coast cluster and Cinnabar
   `_Port` copies; FRLG maps are in only when they carry
   `wayfarer_include`, belong to the closed coast, S.S. Anne, Pokémon Tower
   or Celadon hideout sets, or are enabled in
   `game/src/data/wayfarer_sevii_maps.json`. The script reads those sets
   out of `mapjson.cpp` itself. This matches `global.h` lines 87-98:
   Wayfarer is HNS plus the reviewed Sevii closure plus Emerald, and
   general FRLG content stays out.
3. **In one of the four spot regions.** Sinnoh (133 maps), and HNS maps
   outside Kanto and Johto, are out (133: Alola, Sinjoh, and the HNS
   copies of Hoenn's Frontier, contest, event, and link maps).
4. **Not dropped by the Hoenn policy**
   (`game/tools/wayfarer_hoenn_content/classification.json`): its
   `excluded` maps (unused and link maps, 11), plus secret bases (24) and
   ticket-only event islands (28), which aren't everyday places.
5. **Reachable.** A search from every heal location and respawn map,
   plus the destinations of warps in shared scripts
   (`game/data/scripts/*.inc`), following warp events, connections, and
   `warp*` commands in each map's own `scripts.inc`. This drops 69 more
   maps, such as the unused Ruby maps, Battle Pyramid squares, contest
   halls, dive-only maps (Seafloor Cavern, Marine Cave), the night copies
   of HNS rooftops, `ViridianCity_Gym_hns` (replaced by the FRLG Gym), and
   two debug `TestMap` maps that New Bark Town and Ecruteak City warp into.

Events are taken as mapjson emits them for Wayfarer: `wayfarer_exclude`
events dropped, `wayfarer_*` field overrides applied, shared event maps
followed, and Sevii maps reduced to their manifest's retained events and
enabled warps. The coast's flag remaps aren't ported; they don't change
which objects are always present.

**Result: 1,012 maps** (Kanto 217, Johto 259, Hoenn 401, Sevii 135).

### Region tagging

- Emerald maps are **Hoenn**.
- FRLG maps are **Sevii** when their `region_map_section` falls in the
  range `IsWayfarerSeviiMapSecId` uses (`MAPSEC_ONE_ISLAND` to
  `MAPSEC_EMBER_SPA`, `game/src/region_map.c` line 129), and **Kanto**
  otherwise (the coast, Silph Co., the League rooms).
- HNS maps use their own `region` field (`REGION_KANTO` or
  `REGION_JOHTO`); where it's missing, the HNS map group decides
  (`gMapGroup_IndoorCerulean_Hns` is Kanto, and so on).

HNS tags Indigo Plateau and its Center as Johto; the counts keep that.

### Detection

Behaviour bytes are read as the engine reads them (`game/src/fieldmap.c`):

- **HNS:** 16-bit attributes, 640 primary metatiles, behaviour `& 0xFF`.
- **Emerald:** 16-bit attributes, 512 primary metatiles.
- **FRLG:** 32-bit attributes, 640 primary metatiles, behaviour
  `& 0x1FF`, then `NormalizeFrlgMetatileBehavior`.

A tile is **walkable land** when its collision bits are clear and it isn't
fishable water, a waterfall, or a ledge. Reachability is a 4-way search
from the map's warp tiles and connection edges that treats always-present
objects as walls and lets ledges be jumped one way. Elevation and the
one-sided `MB_IMPASSABLE_*` behaviours are ignored, so it is approximate.

Per kind, the draft rules (the rows the first run used):

| Kind | Draft rule as run |
| --- | --- |
| Pokémon Center | An outdoor map's warp (town, city, route, ocean route) into a `MAP_TYPE_INDOOR` map with a nurse sprite. Spots: free tiles facing `MB_COUNTER`, minus the tile in front of the nurse. |
| Game Corner | Same door rule; slot-machine signs (`_EventScript_SlotMachineN`). Spots: the tile behind each sign along its facing. |
| Mart and store | Same door rule; shop-shelf tiles and a clerk sprite. Floors: interiors reached by interior warps that have shelves. Spots: free tiles facing a shelf. |
| Gym | Same door rule; Gym music on the destination. One spot per Gym. |
| Tall grass | 4-connected patches of `MetatileBehavior_IsTallGrass` tiles, at least 6 tiles, touching reachable land. |
| Water's edge | Free reachable land tiles with a 4-neighbour that is surfable and fishable water. A **stretch** is an 8-connected run of them. |
| Town square | On town and city maps: centres of a 3×3 block of reachable, non-grass land, at least 3 tiles (Chebyshev) from any warp, sign, or object. A **square** is an 8-connected group of such centres. |
| NPC chat | See the filter below. Spots: each free tile 4-adjacent to the NPC. |

Classification order was the draft spec's: Center, Game Corner, Mart, Gym.

The revised rules change these (the spec's
[detection sources](../specs/notable-spots.md#detection-sources) hold the
exact wording):

| Kind | Revised rule as run |
| --- | --- |
| Doors | From an outdoor-typed map that isn't a rooftop (outdoor type, no connections, every warp into an interior with no warp to an outdoor map) into a `MAP_TYPE_INDOOR` or `MAP_TYPE_NONE` map. |
| Pokémon Center | A nurse and at least one `MB_COUNTER` tile, no Gym music, whatever the map type. Where the map has one `MB_COUNTER` tile, the counter row extends along the collision tiles beside it, and spots are the free tiles beyond it, away from the nurse. |
| Mart and store | The door's interior plus interiors reached by interior warps (not through a Center, Game Corner, or Gym) that have shelves or a vendor. A store needs shelves on some map and a vendor (clerk sprite or `pokemart` script) on some map; floors are the maps with shelves. On FRLG layouts in a map with a vendor, `MB_POKEMON_CENTER_BOOKSHELF` is a shelf. Keyed by floor set. |
| Gym | Gym music, checked after Center, Game Corner, and store. |
| Authored drops | `GoldenrodCity_BikeShop_hns` as a store; `SaffronCity_FightingDojo_hns` as a Gym. |
| Water's edge | As before, on maps that aren't interiors, have no Gym music, and have fishing encounters. |
| Town square | Not on rooftops. At most 3 squares per map, the largest by centre count; each stands on the centre nearest its group's mean. |
| NPC chat | Unchanged: the filter below already ran in the first inventory, and the spec now states it. |

### NPC chat filter

An object event is a chat partner when all of these hold, checked in
order (the rejection counts are over all 1,012 maps). The spec now states
this filter, with `MOVEMENT_TYPE_NONE` left out:

1. It's a plain object, not a clone.
2. `trainer_type` is `TRAINER_TYPE_NONE` (rejects 1,046 trainers).
3. `flag` is `0`, so it's always present (rejects 1,865).
4. It isn't a nurse or clerk (129).
5. It isn't a **notable cameo**: a sprite named after a catalog trainer,
   such as `OBJ_EVENT_GFX_MISTY_HNS` (40 always present).
6. It isn't a **villain team** sprite (Rocket, Aqua, Magma grunts and
   admins), which stand as story blockers even with no flag (38).
7. It isn't a **Pokémon**: `MON_BASE+SPECIES_*` or a sprite named after a
   species (959, mostly HNS overworld wild Pokémon).
8. It isn't an **item or prop**: item and Poké Balls, berry trees, cut
   trees, rocks, boulders, lights, `VAR_` sprites, vehicles, dolls (569).
9. Its `movement_type` is `MOVEMENT_TYPE_FACE_*` or `LOOK_AROUND`. Moving
   NPCs are out (402), and so are `MOVEMENT_TYPE_NONE` (90) and
   walk-in-place (4).
10. It isn't a **scripted actor**: its `local_id` isn't the target of
    `applymovement`, `removeobject`, `setobjectxy` and the like in the
    map's scripts (190).
11. A free walkable tile is 4-adjacent to it (rejects 129).

## Counts

Per kind per region, under the revised rules. Where the draft rules gave a
different number, the cell reads "draft → revised". Indented rows break a
kind down; the first row of each kind is the number of places.

| Kind | Kanto | Johto | Hoenn | Sevii | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Maps scanned | 217 | 259 | 401 | 135 | 1,012 |
| Pokémon Centers | 11 | 13 | 18 | 8 | 50 |
| › counter tiles | 47 → 51 | 66 | 3 → 79 | 64 | 180 → 260 |
| Marts and stores | 7 → 9 | 7 | 12 → 14 | 0 → 4 | 26 → 34 |
| › floors | 7 → 12 | 7 → 10 | 12 → 17 | 0 → 4 | 26 → 43 |
| › shelf tiles | 127 → 280 | 179 → 247 | 227 → 314 | 0 → 54 | 533 → 895 |
| Game Corners | 1 | 1 | 1 | 0 | 3 |
| › slot tiles | 16 | 16 | 12 | 0 | 44 |
| Gyms | 8 | 8 | 8 | 0 | 24 |
| Tall-grass maps | 23 | 40 | 25 | 15 | 103 |
| › patches (6+ tiles) | 53 | 129 | 104 | 65 | 351 |
| › tiles | 2,518 | 5,668 | 2,801 | 2,192 | 13,179 |
| Water's-edge maps | 28 → 26 | 63 → 49 | 50 → 42 | 17 | 158 → 134 |
| › stretches | 88 → 85 | 262 → 218 | 187 → 169 | 63 | 600 → 535 |
| › tiles | 1,034 → 944 | 2,147 → 1,762 | 1,467 → 1,384 | 691 | 5,339 → 4,781 |
| Town and city maps | 11 → 10 | 15 → 14 | 16 | 7 | 49 → 47 |
| › with a square | 11 → 10 | 15 → 14 | 15 | 7 | 48 → 46 |
| › squares | 119 → 29 | 87 → 40 | 107 → 42 | 26 → 17 | 339 → 128 |
| › square tiles | 1,874 → 1,277 | 1,360 → 1,181 | 1,596 → 1,224 | 221 → 192 | 5,051 → 3,874 |
| NPC-chat maps | 103 | 107 | 187 | 40 | 437 |
| › NPCs | 183 | 198 | 402 | 60 | 843 |
| › adjacent tiles | 510 | 589 | 1,076 | 168 | 2,343 |
| Named spots | 10 | 22 | 21 | 6 | 59 |

Equal totals can hide swaps. Kanto's 11 Centers gain Viridian's and lose
the Fighting Dojo. Johto's 7 stores gain the Goldenrod department store and
lose the Goldenrod Bike Shop. Kanto's 8 Gyms would be 9 without the
authored drop, since the Fighting Dojo has Gym music. "Square tiles" now
counts only the centres in the squares kept.

The full per-map list is
[per-map.csv](notable-spots-inventory/per-map.csv): 1,012 rows with the
region, map type, layout family, and every revised count above, plus the
tall-grass patch sizes, the store a floor belongs to, why a map's water's
edge was dropped, whether it's a rooftop, and each kept square's standing
tile. The chosen NPCs per map (tile and sprite) and the rejection reasons
are in [inventory.json](notable-spots-inventory/inventory.json) under
`maps`; the draft totals and findings are under `draft_region_totals` and
`draft_findings`.

Largest per map, revised:

- **Tall grass:** `SixIsland_PatternBush_Frlg` 737 tiles,
  `ViridianForest_hns` 558, `Route34_hns` 446.
- **Water's edge:** `Route12_hns` 290 tiles,
  `SevenIsland_TanobyRuins_Frlg` 183, `WhirlIslands_B1F_hns` 141.
- **Squares:** `LilycoveCity` 314 centres in its 3 squares,
  `ViridianCity_hns` 286, `EcruteakCity_hns` 240.
- **NPC chats:** `BattleFrontier_OutsideEast` 16 NPCs, then the three Game
  Corners (8 or 9 each).

The spec's own examples check out: `Route1_hns` has 206 tall-grass tiles,
`Route25_hns` 41 tall-grass and 286 fishable water tiles (244 ocean plus
42 pond), `CeladonCity_hns` 18 pond tiles, the Celadon HNS Game Corner 16
slot-machine signs, and `RustboroCity_PokemonCenter_1F` 5 counter tiles
under the Emerald counter rule.

## Changes applied

The nine proposals from the first run, as the spec now states them. Where
a proposal offered a choice or left a value open, the spec makes it
precise, as noted.

1. **Interior test.** A door's destination is an interior when its
   `map_type` is `MAP_TYPE_INDOOR` or `MAP_TYPE_NONE`. The Center test reads
   content only, so it no longer depends on the map type. Made precise:
   the door's source must also not be a rooftop (defined by warps and
   connections, not names), so a rooftop warp isn't an outdoor door.
2. **Store rule.** The door's interior plus every interior reached by
   interior warps, shelves somewhere, a vendor somewhere (a clerk sprite or
   a `pokemart` script), keyed by floor set. Made precise: the walk passes
   only through maps with shelves or a vendor, never through a Center, Game
   Corner, or Gym, so elevator cars aren't part of a store; every floor in
   scope is reached by stairs or escalators anyway.
3. **FRLG shelves.** `MB_POKEMON_CENTER_BOOKSHELF` is a shelf on FRLG
   layouts in a map with a vendor.
4. **Center counter.** Where a map has one `MB_COUNTER` tile, the counter
   row is that tile plus the unbroken run of collision tiles beside it in
   the same row; spots are the free tiles beyond it, away from the nurse.
   Made precise: "in line with" is that row, and "facing it" is the side
   away from the nurse.
5. **Classification order.** A Center needs a nurse, at least one
   `MB_COUNTER` tile, and no Gym music; both conditions keep the Fighting
   Dojo out. Made precise: the Dojo then classifies as a Gym through its
   music, so the authored drop list removes it as a Gym.
6. **Water's edge.** The first run offered two alternatives; the spec takes
   both: no interiors (which also covers every Gym in scope), no Gym music,
   and fishing encounters on the map.
7. **Squares.** Rooftops excluded; at most 3 squares per map. Made precise:
   ranked by centre count, ties to the top-left tile, standing on the
   centre nearest the group's mean.
8. **NPC chats.** The villain-team, Pokémon-sprite, notable-cameo, prop,
   and scripted-actor exclusions are in the spec. Made precise: standing
   `MOVEMENT_TYPE_NONE` people stay out; the authored list can add one.
9. **Counts.** The spec now says 52 in-scope maps carry a nurse, and 49
   town and city maps are in scope (47 without the two `MAP_TYPE_CITY`
   rooftops), instead of 73, 51, and 37 across the whole repo.

Two corrections ride along: the **Cianwood pharmacy**
(`MAP_CIANWOOD_SHOP_HNS`) is no longer named as a Mart; the spec says it's
the pharmacy, which content detection rightly skips, and it's a named spot.
The **Goldenrod Bike Shop** leaves the store kind through the authored drop
list and is a named spot instead.

## Resolved findings

- **Viridian City's Center** (`MAP_TYPE_NONE`) is found.
- **Emerald Center counters.** All 16 Emerald Centers have counter spots
  (4 or 5 each); Hoenn goes from 3 counter tiles to 79. No Center in scope
  is left without counter spots.
- **FRLG Marts.** `CinnabarIsland_Mart_Frlg` and the Three, Four, Six, and
  Seven Island Marts are found: Sevii has 4 stores, Kanto 9.
- **Department stores.** Celadon (2F-5F), Goldenrod (2F-4F and 6F), and
  Lilycove (2F-5F) are found, each as one store keyed by its floors, and
  the rooftop door no longer finds a second copy. Goldenrod 5F (TM clerk,
  no shelves) is part of its store but not a floor.
- **Lavaridge Herb Shop** is found as a store through its shop script, so
  it isn't a named spot.
- **Saffron Fighting Dojo** is neither a Center nor a Gym.
- **Rooftops.** `CeladonCity_Apartments_RoofDay_hns`,
  `CeladonCity_DepartmentStore_RoofDay_hns`, and
  `GoldenrodCity_DepartmentStore_7F_hns` give no squares and aren't
  outdoor doors.
- **Indoor and fishless water.** 24 maps lose their water's edge (558
  tiles): 5 interiors (`CeruleanCity_Gym_hns` 70, the two Battle Palace
  rooms, `AquaHideout_1F`, `LilycoveCity_Harbor`) and 19 maps with no
  fishing encounters.
- **Too many squares.** 339 squares become 128; 33 maps hit the cap of 3.
- **Wrong counts** in the spec are restated for Wayfarer's scope.
- **Benches and lookouts.** Named spots now carry the one-off places the
  first run said needed an authored list (labs, towers, harbours, Day
  Cares); benches and lookouts stay in the authored overrides.

## Remaining issues

- **Fishless ponds.** Requiring fishing encounters drops the ponds on
  `Route33_hns` (46 tiles), `Route38_hns` (116), `Route39_hns` (89),
  `Route48_hns` (27), `LakeOfRageLowTide_hns` (34), `MtMoon_Outside_hns`
  (20), `BellchimeTrail_hns` (36), `WhirlIslands_LugiaChamber_hns` (19),
  and the Battle Frontier's two outdoor maps. That follows "fish where the
  player could", but if these ponds should have fishing tables, that's a
  wild-encounter question, and the edges come back once they do.
- **Squares on non-town maps.** `SafariZoneGate_hns`, `LakeOfRage_hns`, and
  `MtSilver_Outside_hns` are typed town or city and keep 3 squares each;
  `IndigoPlateau_hns` keeps 1. They are candidates for the drop list.
  `PacifidlogTown` still has no square; `IndigoPlateau_hns` and
  `FiveIsland_Frlg` have 1, and `CinnabarIsland_Frlg`, `LavaridgeTown`,
  `OldaleTown`, `VerdanturfTown`, `SevenIsland_Frlg`, and `SixIsland_Frlg`
  2 each. Those want authored benches or lookouts.
- **Centers in optional buildings.** `EverGrandeCity_PokemonLeague_1F`,
  `TrainerHill_Entrance`, and `TrainerTower_Lobby_Frlg` pass the Center
  test (nurse and counter). They are real healing counters, so they're
  kept; Trainer Hill and the Trainer Tower also have named spots, on tiles
  away from the counter.
- **Shelves outside stores.** `CeladonCity_Apartments_1F_hns` and
  `TinTower_RoofDay_hns` have shop-shelf behaviours with no vendor
  (probably decor tiles); the pharmacy and the Goldenrod Bike Shop are
  there by design. None yields a spot.
- **The Dojo as a Gym.** The drop list handles it. A rule would also work
  (a Gym needs a leader in
  [Gym Leader scaling's](../specs/gym-leader-scaling.md) coverage), but
  that ties detection to the catalog; it's left as a later option.
- **Unreached grass.** 62 patches of 6 or more tiles on 35 maps still touch
  no reachable land; some are artefacts of ignoring elevation.
  `AzaleaTown_hns` still has tall grass and no land encounters.
- **Scripted actors.** HNS objects often have no `local_id`, so HNS story
  actors with flag `0` can still slip through the NPC filter.
- **HNS Battle Tent copies.** `TrainerHill_Courtyard_hns` (Route 40) warps
  into HNS copies of the Slateport Battle Tent, which the region tagging
  counts as Johto. They add a few NPC chats; the real tool should decide
  whether they belong.
- **Two Island Joyful Game Corner** still has Game Corner music and no
  slot machines, so it yields nothing, as the spec says.
- **Approximate collision.** Elevation and one-sided walls are still
  ignored, so reachability and the named-spot walkability check are
  approximate (Sootopolis's raised paths, for example).

## Named-spot validation

The script reads the four tables in
[notable named spots](notable-named-spots.md) and checks each row the way
the spec's [validation](../specs/notable-spots.md#named-spots) says: the
map exists, is in scope (this note's [scope](#scope)), and is in the
row's region; the tile is inside, walkable, reachable, and free of objects
and warps; it isn't a worked-out haunt standing tile or a duplicate; the
activities are one or two from the list; the capacity fits (1 on an
interior, up to 2 elsewhere, with a free neighbour for a second trainer).

**Result: 59 rows (Kanto 10, Johto 22, Hoenn 21, Sevii 6), all pass.**
Each row's derived facing, notes, and problems are in `inventory.json`
under `findings.named_spots`. Notes:

- Three Kanto rows share a map with a v0 haunt: Vermilion harbour, the
  Fighting Dojo, and the Safari Zone gate. Only four haunt standing tiles
  are worked out (Celadon Game Corner, Cerulean Cape, Pewter Museum,
  Viridian Forest), so for the rest, the haunt author must avoid the named
  spot's tile.
- Four rows sit on detected water's-edge tiles (Vermilion harbour, Lake of
  Rage, Olivine harbour, Berry Forest), which the extraction then drops.
- The list drops the Johto Battle Tower (not in Wayfarer; HNS's Route 40
  leads to `TrainerHill_Courtyard_hns`, listed instead) and the Lavaridge
  Herb Shop (now a detected store).

## HNS, FRLG, and Emerald differences

- **Attribute format.** FRLG attributes are 32-bit with a 9-bit behaviour;
  HNS and Emerald are 16-bit. HNS and FRLG layouts have 640 primary
  metatiles, Emerald 512. A reader that assumes the HNS format (as
  `e2e_collision/export.py` does) misreads Emerald and FRLG layouts.
- **Shop shelves.** FRLG Marts use `MB_POKEMON_CENTER_BOOKSHELF`;
  HNS and Emerald use the three shop-shelf behaviours.
- **Counters.** Emerald Centers mark one counter tile; HNS and FRLG mark
  the whole row.
- **Slot machines.** Only `MauvilleCity_GameCorner` uses
  `MB_SLOT_MACHINE` (20 tiles for 12 signs). Both HNS Game Corners don't,
  while `NewBarkTown_PlayersHouse_1F_hns` and
  `PalletTown_RedsHouse_1F_hns` have two each (likely the console props).
- **Doors.** Most door tiles carry the collision bit in every family
  (HNS 152 of 264 outdoor-to-interior warps, Emerald 133 of 170, FRLG 38
  of 47), and only 180 of HNS's 264 such warps sit on a door behaviour;
  the rest include arrow warps into gates and plain tiles. So the door
  behaviour is a poor filter; the warp's destination is the real signal.
- **Map types.** HNS types rooftops `MAP_TYPE_CITY` or `MAP_TYPE_ROUTE`,
  and some interiors `MAP_TYPE_NONE`; FRLG Sevii types every island town
  `MAP_TYPE_TOWN`, never `MAP_TYPE_CITY`.

## Recommendations

For the real tool:

1. **Use mapjson's own selection.** Port or call
   `data_matches_version()` and the Sevii event filter rather than a copy;
   this script's port is approximate. Then search reachability from the
   heal locations, as here, and drop debug maps by name.
2. **Read attributes per family.** Follow `ExtractMetatileAttribute` and
   `NormalizeFrlgMetatileBehavior`, with the primary metatile count per
   layout version.
3. **Reuse the engine's collision.** Model elevation and one-sided
   impassable behaviours, or reachability will both over- and under-count.
4. **Report, don't hide.** Emit the findings lists in this note (content
   without a door, music without content, rooftops, dropped edges) as a
   build-time report beside the per-map counts.
5. **Validate named spots** as the script does, and fail the build on a
   bad row.

The nine spec changes this note first proposed are now in the spec
([changes applied](#changes-applied)).

## Limits

- The Wayfarer selection and event filter are a simplified port of
  `mapjson.cpp`; the coast warp remapping (`resolve_wayfarer_coast_warp`)
  isn't ported.
- Reachability ignores elevation and one-sided walls; scripted entries
  outside `warp*` commands (specials, dynamic warps, dive) aren't followed.
- Center side seats and Gym exit warps weren't computed. Haunt overlaps
  are checked only against the four worked-out haunt standing tiles.
- Patch, clearance, and square-cap sizes are the spec's placeholders (6
  tiles, 3 tiles, and 3 squares).
- The draft column re-runs the first inventory's rules in the same script,
  so it matches the first run's numbers exactly.

## References

- [Notable spots PRD](../prds/notable-spots.md)
- [Notable spots specification](../specs/notable-spots.md)
- [Notable named spots](notable-named-spots.md)
- [Notable trainer travel proof of concept](notable-trainer-travel-poc.md)
