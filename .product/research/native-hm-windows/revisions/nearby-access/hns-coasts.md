# HNS coastal Surf acquisition without Surf

This audit traces checked-in map connections, warps, scripts, collision words and metatile behaviors. It does not run the game. Whirl Islands are out of scope. Encounter probabilities and TR coverage belong to the accompanying distribution simulation; a reachable bank alone does not establish a suitable catch.

## Reachable encounter sets

Use only these methods for the first Surf user. Water encounters require Surf and do not count.

| Starting settlement | Map ID | Method | Detour |
|---|---|---|---|
| Cianwood | `MAP_CIANWOOD_CITY_HNS` | `fishing_mons` | Local shore |
| Cianwood | `MAP_CLIFF_EDGE_CAVE_HNS` | `land_mons` | Cliff Edge Gate, Route 47, middle cave floor |
| Cianwood | `MAP_ROUTE47_HNS` | `fishing_mons` | Continue down the cave stairs to its southern beach |
| Olivine | `MAP_OLIVINE_CITY_HNS` | `fishing_mons` | Local shore |
| Olivine | `MAP_OLIVINE_CITY_PORT_OUTSIDE_HNS` | `fishing_mons` | South through the city connection |
| Olivine | `MAP_ROUTE40_HNS` | `fishing_mons` | Walk west to the beach |
| Cinnabar | `MAP_CINNABAR_ISLAND_HNS` | `fishing_mons` | Local southern shore |
| Vermilion | `MAP_VERMILION_CITY_HNS` | `fishing_mons` | Local shore |
| Vermilion | `MAP_VERMILION_CITY_PORT_OUTSIDE_HNS` | `fishing_mons` | South through the city connection |
| Vermilion | `MAP_ROUTE6_HNS` | `fishing_mons` | Walk north to the pond |

These are bounded nearby options, not an exhaustive regional flood fill. Check the applicable day/night profile rather than assuming a profile alias when the runtime manifest supplies none.

## Cianwood and Route 47

[Cianwood's warp 1](../../../../../game/data/maps/CianwoodCity_hns/map.json) at `(14,36)` leads to Cliff Edge Gate warp 0 at `(26,10)`. [The gate](../../../../../game/data/maps/CliffEdgeGate_hns/map.json) exits at `(6,26)` to Route 47 warp 0 `(106,32)`.

The gate has two engineers at `(16,13)` and `(16,14)`, hidden by `FLAG_AMPHAROS_HEALED`. Their presence deserves checking rather than assuming the original game's progression gate. In this layout, the static walk search reaches both exits even with every object coordinate blocked, including both engineers. [Their dialogue script](../../../../../game/data/maps/CliffEdgeGate_hns/scripts.inc) does not move the player or check progression. Sideways stairs are present in the gate, so its traversal has less assurance than the ordinary ground and bank checks below: the inspection helper does not reproduce sideways-stair movement frame by frame.

From the Route 47 entrance, the same dry component reaches warp 2 `(85,33)`, entering [Cliff Edge Cave](../../../../../game/data/maps/CliffEdgeCave_hns/map.json) warp 1 `(34,11)`. The middle floor reaches stair warp 5 `(36,8)`, which goes to warp 6 `(25,25)` on the bottom floor. That floor reaches exit warp 2 `(24,28)`, landing at [Route 47](../../../../../game/data/maps/Route47_hns/map.json) warp 3 `(85,47)`. The beach has a bank at `(84,48)` facing fishable water `(83,48)`. The same ground paths and paired warps provide the return route.

The cave's ordinary land table contains Krabby, Kingler, Wooper and Quagsire, among other species. Its lower rock encounters are not needed for this route, and the inspection blocks every breakable-rock object. The route does not enter the Safari Zone. Route 47's upper grass, Route 48 and the Safari Zone are not part of the certified encounter set. They add complexity without being necessary to test the useful cave and beach options.

Cliff Edge Gate itself has water encounters but no fishing table. Its visible pond is therefore not a first-Surf acquisition source.

## Olivine

[Olivine's south connection](../../../../../game/data/maps/OlivineCity_hns/map.json) reaches [PortOutside](../../../../../game/data/maps/OlivineCity_PortOutside_hns/map.json) directly. The city ground component reaches south edge `(20,71)`, which connects to port `(15,0)`. The port ground component reaches bank `(10,5)` facing water `(9,5)` with all authored NPC positions blocked.

The ticket checks live in [PortInside's ship dialogue](../../../../../game/data/maps/OlivineCity_PortInside_hns/scripts.inc), not in the outside approach. PortOutside has no coordinate-event gate, and [its script](../../../../../game/data/maps/OlivineCity_PortOutside_hns/scripts.inc) does not restrict fishing-bank access.

Alternatively, city west edge `(0,46)` connects to [Route 40](../../../../../game/data/maps/Route40_hns/map.json) `(33,14)`. A dry path reaches bank `(10,18)` facing water `(10,19)`. Route 40 has no coordinate triggers on this path. Both routes are reversible on foot.

Route 39 and Route 38 provide a walking route toward Ecruteak, but neither land table includes a Surf carrier in the approved roster. They are unnecessary additions unless the nearby fishing simulation leaves a gap.

## Cinnabar

[Cinnabar](../../../../../game/data/maps/CinnabarIsland_hns/map.json) connects north to Route 21 and east to Route 20. Both connections are across ocean, not a walk to the next settlement. The dry component from outside the Pokémon Center `(45,30)` has no map-edge exit. It does reach bank `(38,34)` facing water `(38,35)`, with all authored object positions blocked.

The town's ordinary land table contains Murkrow, Slugma and Tangela, none of which has Surf in the approved distribution. Do not count ocean encounters or a return journey to Pallet as first-Surf sources. Local fishing remains the meaningful ordinary acquisition mechanism. The extra map warp at `(41,1)` does not occur in the inspected inhabited ground component and is not treated as a travel option.

## Vermilion

[Vermilion's south connection](../../../../../game/data/maps/VermilionCity_hns/map.json) reaches [PortOutside](../../../../../game/data/maps/VermilionCity_PortOutside_hns/map.json). The city component reaches `(38,54)`, connecting to `(15,0)` in the port; bank `(10,5)` faces water `(9,5)`. [The outside script](../../../../../game/data/maps/VermilionCity_PortOutside_hns/scripts.inc) has no gate. S.S. Ticket and ship-state checks are in [PortInside's travel dialogue](../../../../../game/data/maps/VermilionCity_PortInside_hns/scripts.inc), beyond the fishing approach.

The city also reaches north edge `(32,0)`, which connects to [Route 6](../../../../../game/data/maps/Route6_hns/map.json) `(40,35)`. Route 6's ground component reaches bank `(28,20)` facing water `(28,21)` without entering its northern gates. Its pond fishing table includes Psyduck, Slowpoke, Poliwag, Poliwhirl, Qwilfish and Gyarados, alongside non-carriers. No coordinate-event gate lies on that route. It returns by the same walking connection.

## Method and limits

[hns_tiles.py](hns_tiles.py) reads the map binaries without modifying them. It decodes HNS's 640-primary-metatile boundary, collision and elevation fields, and metatile behavior attributes. It blocks water, tile collisions and all authored object coordinates, and checks source/destination directional barriers. It searches only within one map; the warp and connection chains above are joined manually from their JSON definitions.

The shore test follows [CanFish](../../../../../game/src/item_use.c), [IsPlayerFacingSurfableFishableWater](../../../../../game/src/field_player_avatar.c), and [fishable water behaviors](../../../../../game/src/metatile_behavior.c): default-elevation ground faces unobstructed fishable water at another elevation. Movement collision ordering and directional barriers come from [event_object_movement.c](../../../../../game/src/event_object_movement.c).

Run a check from the repository root, for example:

```sh
python3 .product/research/native-hm-windows/revisions/nearby-access/hns_tiles.py Route6_hns 40 35
```

This helper is evidence for ordinary ground components, not a complete movement emulator. It does not model forced movement, dynamic NPC positions, script execution, persistent elevation through transition tiles, sideways-stair diagonals, or player camera bounds. Paired cave stairs are resolved using their authored warps. All-object blocking makes ordinary NPC occupancy conservative, but does not prove that every script state has been exercised. No E2E play, ROM build or encounter edits were performed.
