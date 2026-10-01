# Notable named spots (starting list)

Related spec: [Notable spots](../specs/notable-spots.md#named-spots)

Evidence: [inventory.py](notable-spots-inventory/inventory.py) reads the
tables below and checks every row; its results are in
[inventory.json](notable-spots-inventory/inventory.json) under
`findings.named_spots`, and summarised in the
[inventory](notable-spots-inventory.md#named-spot-validation).

Status: review draft. Tiles and notes are content for review; nothing here
is in the build yet.

## Rules for this list

- **Map.** The variant Wayfarer plays (HNS for Kanto and Johto, FireRed and
  LeafGreen for Sevii and the Cinnabar port, Emerald for Hoenn), in scope
  by the [inventory's scoping rule](notable-spots-inventory.md#scope), and
  in the region of its table.
- **Tile.** `(x, y)` on that map: walkable land, reachable from the map's
  warps and edges, and free of objects and warps, read with the
  inventory's map decoding. The trainer faces the first 4-neighbour (up,
  left, right, down) that holds an object, a sign, a counter, a shelf, or
  fishable water, and otherwise faces down.
- **Activities.** One or two from the shared list: train, care, study,
  home, relax, shop, gamble, sightsee, fish, visit, lie low.
- **Capacity.** 1 on an interior map (`MAP_TYPE_INDOOR` or
  `MAP_TYPE_NONE`), 1 or 2 elsewhere. A second trainer stands on the first
  free 4-neighbour of the tile, in the same order.
- **Haunt.** The haunt that shares the map, if any. A named spot never
  uses a haunt's standing tile; the two share the map's
  [capacity](../specs/notable-spots.md#capacity).

All 59 rows below pass the checks.

## Kanto

| # | Spot | Map | Tile | Activities | Capacity | Haunt | Why |
| ---: | --- | --- | --- | --- | ---: | --- | --- |
| 1 | Oak's Lab | `PalletTown_Lab_hns` | (18, 14) | study | 1 | – | Kanto's research hub, beside Oak's aides. |
| 2 | Cinnabar Lab | `CinnabarIsland_PokemonLab_ExperimentRoom_Frlg` | (11, 4) | study | 1 | – | The fossil-revival room, by the scientist. |
| 3 | Vermilion Fan Club | `VermilionCity_FanClub_hns` | (9, 7) | visit, relax | 1 | – | Trading stories with the chairman's members. |
| 4 | Vermilion harbour | `VermilionCity_PortOutside_hns` | (24, 5) | sightsee, fish | 2 | Vermilion harbour | The east end of the S.S. Anne pier. |
| 5 | Fighting Dojo | `SaffronCity_FightingDojo_hns` | (8, 11) | train | 1 | Saffron Fighting Dojo | The training floor, off the haunt's spot. |
| 6 | Cerulean Bike Shop | `CeruleanCity_BikeShop_hns` | (3, 5) | shop | 1 | – | Browsing the bikes. |
| 7 | Lavender Name Rater | `LavenderTown_House3_hns` | (8, 5) | visit | 1 | – | Waiting on a nickname. |
| 8 | Safari Zone gate | `FuchsiaCity_SafariZoneEntrance_hns` | (10, 6) | sightsee | 1 | Safari Zone | Planning a trip into the zone. |
| 9 | Bill's Sea Cottage | `Route25_BillsHouse_hns` | (11, 7) | visit, study | 1 | – | Calling on Bill and his teleporter. |
| 10 | Cycling Road | `Route17_hns` | (14, 7) | sightsee | 2 | – | The top of the slope, on flat ground before the ride down. |

## Johto

| # | Spot | Map | Tile | Activities | Capacity | Haunt | Why |
| ---: | --- | --- | --- | --- | ---: | --- | --- |
| 1 | Sprout Tower | `SproutTower_3F_hns` | (6, 4) | sightsee, train | 1 | – | The top floor, near Elder Li. |
| 2 | Bell Tower | `TinTower_1F_hns` | (12, 14) | sightsee | 1 | – | The tower's ground floor, among the sages. |
| 3 | Burned Tower | `BurnedTower_1F_hns` | (21, 8) | sightsee, lie low | 1 | – | Poking around the ruin. |
| 4 | Radio Tower | `GoldenrodCity_RadioTower_1F_hns` | (8, 6) | sightsee, visit | 1 | – | The lobby, by the reception. |
| 5 | Olivine Lighthouse | `OlivineCity_Lighthouse_hns` | (160, 10) | sightsee | 1 | – | The top room, near Amphy. |
| 6 | Ruins of Alph | `RuinsOfAlph_Outside_hns` | (33, 18) | sightsee, study | 2 | – | Between the ruins and the research center. |
| 7 | Ruins research center | `RuinsOfAlph_Lab_hns` | (4, 6) | study | 1 | – | Reading up on the Unown. |
| 8 | Lake of Rage | `LakeOfRage_hns` | (42, 34) | sightsee, fish | 2 | – | The south shore, looking over the lake. |
| 9 | Elm's Lab | `NewBarkTown_Lab_hns` | (2, 9) | study | 1 | – | At Elm's bookshelves. |
| 10 | Earl's Academy | `VioletCity_TrainerSchool_hns` | (8, 11) | study | 1 | – | At the back of the class. |
| 11 | Olivine Café | `OlivineCity_Cafe_hns` | (1, 6) | relax | 1 | – | A table among the sailors. |
| 12 | Olivine harbour | `OlivineCity_PortOutside_hns` | (18, 6) | sightsee | 2 | – | The pier, watching the ferry. |
| 13 | Route 34 Day Care | `Route34_DayCare_hns` | (5, 4) | care, visit | 1 | – | Checking on a Pokémon left with the couple. |
| 14 | Goldenrod Name Rater | `GoldenrodCity_House1_hns` | (8, 5) | visit | 1 | – | Waiting on a nickname. |
| 15 | Goldenrod flower shop | `GoldenrodCity_FlowerShop_hns` | (4, 5) | shop | 1 | – | Among the flowers. |
| 16 | Goldenrod Bike Shop | `GoldenrodCity_BikeShop_hns` | (6, 5) | shop | 1 | – | Browsing the bikes; dropped from the store kind. |
| 17 | Cianwood pharmacy | `CianwoodShop_hns` | (10, 9) | shop, care | 1 | – | In line at the counter for medicine. |
| 18 | Mahogany shop | `MahoganyTown_Shop_hns` | (5, 3) | shop | 1 | – | The granny's souvenir shop. |
| 19 | National Park | `NationalPark_Normal_hns` | (20, 11) | relax, sightsee | 2 | – | The central lawn. |
| 20 | Safari Zone gate | `SafariZoneGate_SafariZoneEntrance_hns` | (8, 8) | sightsee | 1 | – | Planning a trip into the zone. |
| 21 | Dance Theater | `EcruteakCity_Theater_hns` | (10, 14) | sightsee, relax | 1 | – | In the audience. |
| 22 | Route 40 battle courtyard | `TrainerHill_Courtyard_hns` | (24, 31) | train | 2 | – | HNS's stand-in for the Battle Tower, between the Battle Tent doors. |

## Hoenn

| # | Spot | Map | Tile | Activities | Capacity | Haunt | Why |
| ---: | --- | --- | --- | --- | ---: | --- | --- |
| 1 | Mt. Pyre summit | `MtPyre_Summit` | (20, 4) | sightsee | 2 | – | The shrine where the old couple keep the orbs. |
| 2 | Sootopolis | `SootopolisCity` | (44, 41) | sightsee, relax | 2 | – | The crater lake's east shore. |
| 3 | Seaside Cycling Road | `Route110` | (11, 17) | sightsee | 2 | – | At the north gate, by the Cycling Road sign. |
| 4 | Birch's Lab | `LittlerootTown_ProfessorBirchsLab` | (2, 8) | study | 1 | – | At Birch's bookshelves. |
| 5 | Weather Institute | `Route119_WeatherInstitute_1F` | (18, 6) | study | 1 | – | Watching the forecasts come in. |
| 6 | Mossdeep Space Center | `MossdeepCity_SpaceCenter_1F` | (4, 3) | study, sightsee | 1 | – | By the launch counter. |
| 7 | Rustboro Trainer's School | `RustboroCity_PokemonSchool` | (6, 8) | study | 1 | – | Between the desks. |
| 8 | Seashore House | `Route109_SeashoreHouse` | (4, 7) | relax | 1 | – | A table, with a Soda Pop. |
| 9 | Lavaridge hot spring | `LavaridgeTown` | (6, 3) | relax | 2 | – | In the hot sand beside the two old ladies. |
| 10 | Slateport Fan Club | `SlateportCity_PokemonFanClub` | (3, 3) | visit | 1 | – | Showing off a Pokémon to the chairman. |
| 11 | Lilycove Trainer Fan Club | `LilycoveCity_PokemonTrainerFanClub` | (9, 4) | visit | 1 | – | Where fans talk about trainers. |
| 12 | Slateport harbour | `SlateportCity_Harbor` | (1, 7) | sightsee | 1 | – | The dock, watching the S.S. Tidal. |
| 13 | Lilycove harbour | `LilycoveCity_Harbor` | (1, 7) | sightsee | 1 | – | The dock, waiting for a ferry. |
| 14 | Route 117 Day Care | `Route117_PokemonDayCare` | (8, 4) | care, visit | 1 | – | Checking on a Pokémon left with the couple. |
| 15 | Slateport Name Rater | `SlateportCity_NameRatersHouse` | (3, 4) | visit | 1 | – | Waiting on a nickname. |
| 16 | Trainer Hill | `TrainerHill_Entrance` | (12, 12) | train | 1 | – | The lobby, before a climb. |
| 17 | Battle Tower | `BattleFrontier_BattleTowerLobby` | (21, 7) | train | 1 | – | The lobby, before a challenge. |
| 18 | Mauville Bike Shop | `MauvilleCity_BikeShop` | (5, 5) | shop | 1 | – | Talking bikes with Rydel. |
| 19 | Pretty Petal flower shop | `Route104_PrettyPetalFlowerShop` | (4, 4) | shop | 1 | – | Among the flowers. |
| 20 | Route 123 berry rows | `Route123` | (13, 4) | relax | 2 | – | The Berry Master's tree rows. |
| 21 | Safari Zone gate | `Route121_SafariZoneEntrance` | (14, 4) | sightsee | 1 | – | Planning a trip into the zone. |

## Sevii

| # | Spot | Map | Tile | Activities | Capacity | Haunt | Why |
| ---: | --- | --- | --- | --- | ---: | --- | --- |
| 1 | Ember Spa | `OneIsland_KindleRoad_EmberSpa_Frlg` | (13, 12) | relax, care | 1 | – | Soaking in One Island's hot spring. |
| 2 | Trainer Tower | `TrainerTower_Lobby_Frlg` | (16, 13) | train | 1 | – | The lobby, before a climb. |
| 3 | Four Island Day Care | `FourIsland_PokemonDayCare_Frlg` | (8, 6) | care, visit | 1 | – | Checking on a Pokémon left with the couple. |
| 4 | Berry Forest | `ThreeIsland_BerryForest_Frlg` | (28, 11) | relax, sightsee | 2 | – | By the forest pond. |
| 5 | Tanoby Ruins | `SevenIsland_TanobyRuins_Frlg` | (118, 14) | sightsee, study | 2 | – | Outside the Monean Chamber. |
| 6 | Cape Brink | `TwoIsland_CapeBrink_Frlg` | (15, 17) | relax, sightsee | 2 | – | The cape, looking out to sea. |

## Notes

- **Shared maps.** Three Kanto rows share a map with a v0 haunt: Vermilion
  harbour, the Fighting Dojo, and the Safari Zone gate. The Dojo and the
  gate are interiors, so their cap of 1 means the named spot is only free
  while the haunt is empty; the harbour is outdoors and shares its cap of
  3. The haunts' standing tiles there aren't chosen yet, so the haunt
  author must avoid these tiles. The worked-out haunt tiles (Celadon Game
  Corner, Cerulean Cape, Pewter Museum, Viridian Forest) have no named
  spot on their maps: the Game Corner is already a detected kind, and the
  museum floor is the haunt itself.
- **Detected kinds on the same map.** Trainer Hill and the Trainer Tower
  lobby are also detected Pokémon Centers (each has a nurse and a
  counter); the rows stand away from the counter, so no detected tile is
  dropped. The Vermilion harbour, Lake of Rage, Olivine harbour, and
  Berry Forest rows sit on water's-edge tiles, which the extraction then
  drops as detected spots.
- **Berry Forest.** Its berry trees aren't among the retained Sevii events
  in `game/src/data/wayfarer_sevii_maps.json`, so the forest has no berry
  objects in Wayfarer; the row is kept as a nature spot.
- **Cycling Road.** Most of Route 17 is `MB_CYCLING_ROAD_PULL_DOWN`; the
  tile at (14, 7) is plain `MB_NORMAL`, so a standing trainer isn't on a
  forced-movement tile.

## Dropped from the brief

- **Johto Battle Tower.** Not in Wayfarer: HNS's Route 40 leads to
  `TrainerHill_Courtyard_hns` instead, and the HNS Battle Tower and Trainer
  Hill copies are outside Kanto and Johto. The courtyard is listed instead
  (Johto 22).
- **Lavaridge Herb Shop.** The revised store rule now detects it as a
  store (shelves and a shop script), so it's already a shop spot.
- **Bike shops** stay in the list: Cerulean and Mauville were never
  detected, and Goldenrod's is now dropped from the store kind.

Every other place in the brief is in scope and reachable, and has a row.
