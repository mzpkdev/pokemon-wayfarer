# Notable spots inventory (draft)

Related PRD: [Notable spots](../prds/notable-spots.md)

Related spec: [Notable spots](../specs/notable-spots.md)

Evidence: [inventory.py](notable-spots-inventory/inventory.py), a
read-only Python script (standard library only), and its outputs
[per-map.csv](notable-spots-inventory/per-map.csv) (one row per scanned
map, counts per kind) and
[inventory.json](notable-spots-inventory/inventory.json) (scope, totals,
findings, and the chosen NPCs per map). Every number below comes from one
run of the script against `game/` at the time of writing:

    python3 .product/research/notable-spots-inventory/inventory.py

## Question

The spec finds spots automatically from map data. Before anyone writes the
real build-time tool: what pool do the spec's detection rules actually
produce on Wayfarer's maps, and where do the rules miss, misfire, or need
changing?

## Method

The script applies the spec's
[spot kinds](../specs/notable-spots.md#spot-kinds) and
[detection sources](../specs/notable-spots.md#detection-sources) as
written, then reports where they disagree with the content.

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

Per kind, as the spec states it:

| Kind | Rule as run |
| --- | --- |
| Pokémon Center | An outdoor map's warp (town, city, route, ocean route) into a `MAP_TYPE_INDOOR` map with a nurse sprite. Spots: free tiles facing `MB_COUNTER`, minus the tile in front of the nurse. |
| Game Corner | Same door rule; slot-machine signs (`_EventScript_SlotMachineN`). Spots: the tile behind each sign along its facing. |
| Mart and store | Same door rule; shop-shelf tiles and a clerk sprite. Floors: interiors reached by interior warps that have shelves. Spots: free tiles facing a shelf. |
| Gym | Same door rule; Gym music on the destination. One spot per Gym. |
| Tall grass | 4-connected patches of `MetatileBehavior_IsTallGrass` tiles, at least 6 tiles, touching reachable land. |
| Water's edge | Free reachable land tiles with a 4-neighbour that is surfable and fishable water. A **stretch** is an 8-connected run of them. |
| Town square | On town and city maps: centres of a 3×3 block of reachable, non-grass land, at least 3 tiles (Chebyshev) from any warp, sign, or object. A **square** is an 8-connected group of such centres. |
| NPC chat | See the filter below. Spots: each free tile 4-adjacent to the NPC. |

Classification order is the spec's: Center, Game Corner, Mart, Gym.

### NPC chat filter

An object event is a chat partner when all of these hold, checked in
order (the rejection counts are over all 1,012 maps):

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

Per kind per region. Indented rows break a kind down; the first row of each
kind is the number of places.

| Kind | Kanto | Johto | Hoenn | Sevii | Total |
| --- | ---: | ---: | ---: | ---: | ---: |
| Maps scanned | 217 | 259 | 401 | 135 | 1,012 |
| Pokémon Centers | 11 | 13 | 18 | 8 | 50 |
| › counter tiles | 47 | 66 | 3 | 64 | 180 |
| Marts and stores | 7 | 7 | 12 | 0 | 26 |
| › floors | 7 | 7 | 12 | 0 | 26 |
| › shelf tiles | 127 | 179 | 227 | 0 | 533 |
| Game Corners | 1 | 1 | 1 | 0 | 3 |
| › slot tiles | 16 | 16 | 12 | 0 | 44 |
| Gyms | 8 | 8 | 8 | 0 | 24 |
| Tall-grass maps | 23 | 40 | 25 | 15 | 103 |
| › patches (6+ tiles) | 53 | 129 | 104 | 65 | 351 |
| › tiles | 2,518 | 5,668 | 2,801 | 2,192 | 13,179 |
| Water's-edge maps | 28 | 63 | 50 | 17 | 158 |
| › stretches | 88 | 262 | 187 | 63 | 600 |
| › tiles | 1,034 | 2,147 | 1,467 | 691 | 5,339 |
| Town and city maps | 11 | 15 | 16 | 7 | 49 |
| › with a square | 11 | 15 | 15 | 7 | 48 |
| › squares | 119 | 87 | 107 | 26 | 339 |
| › square tiles | 1,874 | 1,360 | 1,596 | 221 | 5,051 |
| NPC-chat maps | 103 | 107 | 187 | 40 | 437 |
| › NPCs | 183 | 198 | 402 | 60 | 843 |
| › adjacent tiles | 510 | 589 | 1,076 | 168 | 2,343 |

The full per-map list is
[per-map.csv](notable-spots-inventory/per-map.csv): 1,012 rows with the
region, map type, layout family, and every count above, plus the
tall-grass patch sizes and, for store floors, the store they belong to.
The chosen NPCs per map (tile and sprite) and the rejection reasons are in
[inventory.json](notable-spots-inventory/inventory.json) under `maps`.

Largest per map:

- **Tall grass:** `SixIsland_PatternBush_Frlg` 737 tiles,
  `ViridianForest_hns` 558, `Route34_hns` 446.
- **Water's edge:** `Route12_hns` 290 tiles,
  `SevenIsland_TanobyRuins_Frlg` 183, `WhirlIslands_B1F_hns` 141.
- **Squares:** `LilycoveCity` 397 tiles, `CeruleanCity_hns` 383,
  `ViridianCity_hns` 379.
- **NPC chats:** `BattleFrontier_OutsideEast` 16 NPCs, then the three Game
  Corners (8 or 9 each).

The spec's own examples check out: `Route1_hns` has 206 tall-grass tiles,
`Route25_hns` 41 tall-grass and 286 fishable water tiles (244 ocean plus
42 pond), `CeladonCity_hns` 18 pond tiles, and the Celadon HNS Game Corner
16 slot-machine signs.

## Findings

### Misses

- **Viridian City's Center.** `ViridianCity_PokemonCenter_hns` is
  `MAP_TYPE_NONE`, so the "door into a `MAP_TYPE_INDOOR` map" rule drops
  it. Kanto's 11 Centers hide this: they include a false hit (below).
  Other `MAP_TYPE_NONE` interiors in scope: `CeladonCity_House1_hns`,
  `CeladonCity_House2_hns`, `NewBarkTown_Lab_hns`, `Route12_House_hns`.
- **Emerald Center counters.** In all 15 Hoenn town Centers and the
  Battle Frontier Center, the only `MB_COUNTER` tile is the one in front
  of the nurse; the rest of the counter is plain collision. So the counter
  rule yields **0 counter spots** in those 16 Centers (Hoenn's 3 come from
  `TrainerHill_Entrance` and `EverGrandeCity_PokemonLeague_1F`). HNS and
  FRLG Centers mark the whole counter row (`CeruleanCity_PokemonCenter_hns`
  gives 5 spots after the nurse's tile).
- **Every FRLG Mart.** FRLG Mart shelves read as
  `MB_POKEMON_CENTER_BOOKSHELF` (`0xE2`) after the engine's FRLG
  normalisation, not as a shop shelf (12 such tiles in each of
  `CinnabarIsland_Mart_Frlg`, `ThreeIsland_Mart_Frlg`,
  `FourIsland_Mart_Frlg`, `SixIsland_Mart_Frlg`,
  `SevenIsland_Mart_Frlg`). No FRLG map in scope has a shop-shelf tile, so
  **Sevii has no Marts** and Kanto loses Cinnabar's.
- **Department stores.** The rule wants the door's own interior to have
  shelves and a clerk. The ground floors of
  `CeladonCity_DepartmentStore_1F_hns`,
  `GoldenrodCity_DepartmentStore_1F_hns`, and
  `LilycoveCity_DepartmentStore_1F` have neither, so none of the three
  stores is found, and the Celadon and Goldenrod upper floors (2F-4F,
  which do have shelves and clerks) show up as orphan content.
  Lilycove's sales staff aren't clerk sprites at all (`WOMAN_3`, `COOK`),
  and `CeladonCity_DepartmentStore_5F_hns` and
  `GoldenrodCity_DepartmentStore_6F_hns` have shelves but no clerk.
- **Other shops.** `LavaridgeTown_HerbShop` has shelves and a shop script
  but no clerk sprite; `MahoganyTown_Shop_hns` sells only through the
  granny's script and has no shelf tiles.
- **Two Island Joyful Game Corner** has Game Corner music and no slot
  machines, as the spec already says, so it yields nothing.
- **Unreached grass.** 62 patches of 6 or more tiles on 35 maps touch no
  reachable land, for example 5 on `Route23_hns` and 4 each on
  `Route34_hns` and `Route47_hns`. Some are truly behind ledges, cut trees,
  or water; some are artefacts of ignoring elevation. The real tool needs
  the engine's collision rules here.

### False or doubtful hits

- **Saffron Fighting Dojo.** `SaffronCity_FightingDojo_hns` has a nurse
  (the rematch hub), so it's classified a Center before its Gym music is
  checked. Kanto's real Center count is 10 found plus Viridian missed.
- **Goldenrod Bike Shop.** `GoldenrodCity_BikeShop_hns` has shelves and a
  clerk, so it counts as a Mart. Arguably fine, but it isn't somewhere you
  stock up.
- **Rooftops typed as cities.** `CeladonCity_Apartments_RoofDay_hns` and
  `GoldenrodCity_DepartmentStore_7F_hns` are `MAP_TYPE_CITY`, so they get
  squares, and a rooftop warp counts as an "outdoor door" into the top
  floor. `SafariZoneGate_hns` (town), `LakeOfRage_hns`,
  `MtSilver_Outside_hns` (cities) and `IndigoPlateau_hns` (town) also get
  squares.
- **Indoor water.** Water's edge isn't limited to outdoor maps:
  `CeruleanCity_Gym_hns` gives 70 edge tiles around its pool,
  `BattleFrontier_BattlePalaceCorridor` 23, `AquaHideout_1F` 18. 24 maps
  with edge tiles have no fishing encounters at all.
- **Grass without wild Pokémon.** `AzaleaTown_hns` has tall-grass patches
  but no land encounters.
- **Squares are far too many.** 339 squares, 5,051 centre tiles: Cerulean
  alone has 22 squares. The 3×3 rule finds every wide path. It needs a cap
  per map, or it needs to rank by openness.
- **Optional Hoenn systems.** The Battle Frontier is reachable and
  contributes a Center, a Mart, and the busiest NPC map
  (`BattleFrontier_OutsideEast`, 16). `TrainerHill_Entrance` and
  `TrainerTower_Lobby_Frlg` count as Centers through their nurses.

### Ambiguous

- **`MOVEMENT_TYPE_NONE` people.** HNS uses it for lights and items, but
  also for some people (kimono girls, officers). The spec's movement rule
  drops all of them; after the other filters, 90 remain that may be
  genuine standing NPCs.
- **Scripted actors.** The `local_id` check catches 190, but HNS objects
  often have no `local_id`, so HNS story actors with flag `0` can slip
  through. A rejected-or-kept list per map is in `inventory.json` for
  review.
- **Store identity.** A door from a rooftop finds the same department
  store from the top floor down. The tool must key a store by its floor
  set, not by its door.

### Maps that need the authored list

- **Benches and lookouts.** No behaviour marks them, so every bench and
  lookout is authored. `PacifidlogTown` has no square at all (all bridges
  and water); `IndigoPlateau_hns` and `FiveIsland_Frlg` have 1, and
  `CinnabarIsland_Frlg`, `SevenIsland_Frlg`, `SixIsland_Frlg`,
  `LavaridgeTown`, `OldaleTown`, and `VerdanturfTown` 2 each. Those want
  authored benches or lookouts.
- **Drops.** The rooftop squares above, and squares on
  `SafariZoneGate_hns` and `IndigoPlateau_hns`, are candidates for the
  drop list or a map-type fix.
- **Center seats.** The wall rule wasn't run; the counter numbers show
  that Hoenn Centers will lean entirely on it (or on authored seats).

### HNS, FRLG, and Emerald differences

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
- **Map types.** HNS uses `MAP_TYPE_CITY` for rooftops and
  `MAP_TYPE_NONE` for some interiors; FRLG Sevii uses only
  `MAP_TYPE_TOWN`.

### Where the spec is wrong

- "73 maps carry" a nurse: true across all of `game/data/maps`, but only
  52 are in Wayfarer's scope. Likewise "51 town and 37 city maps": 49 are
  in scope.
- `MAP_CIANWOOD_SHOP_HNS` is named as a Mart whose name breaks the
  pattern. It's the Cianwood pharmacy: shelves, no clerk, no shop, and its
  script gives the Secret Potion. Content detection rightly skips it.
- "A door is a spot source only when it leads ... into an interior
  (`map_type` `MAP_TYPE_INDOOR`)" misses Viridian's Center.
- The store rule as written finds no department store.
- The Center counter rule finds no counter spots in Emerald Centers.

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
   without a door, music without content, rooftops typed as cities) as a
   build-time report beside the per-map counts.

Proposed spec changes (for the spec owner; the spec isn't edited here):

1. **Interior test.** Treat a destination as an interior when its
   `map_type` is `MAP_TYPE_INDOOR` or `MAP_TYPE_NONE`, or fix Viridian's
   Center to `MAP_TYPE_INDOOR`.
2. **Store rule.** A store is the door's interior plus every interior
   reached by interior warps; the floors are the ones with shelves; and
   the store needs a **vendor** somewhere: a clerk sprite or an object
   whose script opens a shop (`pokemart`). Key the store by its floor set.
   Run as a check, this rule finds Kanto 9 stores on 12 floors, Johto 8 on
   11, Hoenn 14 on 17, and Sevii 4 on 4: the three department stores, the
   FRLG Marts, and the Lavaridge Herb Shop. It loses none of the 26 the
   spec's rule finds.
3. **FRLG shelves.** Count `MB_POKEMON_CENTER_BOOKSHELF` as a shelf on
   FRLG layouts in a map with a vendor.
4. **Center counter.** Where only one `MB_COUNTER` tile exists, take the
   counter row as the impassable tiles in line with the nurse's counter
   tile, and stand on the free tiles facing it.
5. **Classification order.** Check Gym music before the nurse, or skip
   maps with Gym music in the Center test, so the Fighting Dojo isn't a
   Center.
6. **Water's edge.** Limit it to outdoor and cave maps (not
   `MAP_TYPE_INDOOR`), or require fishing encounters on the map.
7. **Squares.** Exclude rooftop maps and keep at most a placeholder 2-3
   squares per map, choosing the largest and most open.
8. **NPC chats.** Add the villain-team and Pokémon-sprite exclusions and
   the scripted-actor check to the filter, and decide whether standing
   `MOVEMENT_TYPE_NONE` people count.
9. **Counts.** Drop the whole-repo counts (73 nurses, 51 towns, 37 cities)
   or restate them for Wayfarer's scope.

## Limits

- The Wayfarer selection and event filter are a simplified port of
  `mapjson.cpp`; the coast warp remapping (`resolve_wayfarer_coast_warp`)
  isn't ported.
- Reachability ignores elevation and one-sided walls; scripted entries
  outside `warp*` commands (specials, dynamic warps, dive) aren't followed.
- Center side seats, Gym exit warps, and haunt overlaps weren't computed.
- Patch and clearance sizes are the spec's placeholders (6 tiles and 3
  tiles).

## References

- [Notable spots PRD](../prds/notable-spots.md)
- [Notable spots specification](../specs/notable-spots.md)
- [Notable trainer travel proof of concept](notable-trainer-travel-poc.md)
