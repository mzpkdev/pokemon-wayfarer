# Reach assignments

PRD: [Wild encounters v2](../prds/wild-encounters-v2.md)
Implemented: Yes (Wayfarer)

Design status: v0 approved. It covers every Wayfarer map with wild encounters:
Kanto, Johto, the Sevii Islands, Alola, Sinjoh, Hoenn and two remote islands.

## Scope

This spec assigns a reach to every Wayfarer map with wild encounters, and
groups dungeon maps into named dungeons with their floors in order.

It doesn't cover:

- reach and dungeon levels, which belong to
  [Wild level scaling](wild-level-scaling.md);
- which species live on each map, or which prowlers appear where;
- maps without wild encounters, except where trainers or item spots need a
  reach, in [Places without wild encounters](#places-without-wild-encounters).

## Behavior

### Assignment rules

1. **Settlement rule:** a town or city map is **Road**. This includes its own
   surfing, fishing and Rock Smash spots. A lone Pokémon Center doesn't make a
   map a settlement.
2. **Road** is the way between places: routes, sea lanes, and forests you pass
   through to get somewhere. A route that leads to a single destination is Road,
   as the way there.
3. **Caves are never Road.** A cave you only pass through is **Wilds**. A cave
   with floors beyond the path through it, or one you go into to explore, is a
   **dungeon**. Victory Road is always a dungeon: it is the gauntlet before the
   League.
4. **Wilds** is outdoors and off the road: a preserve, a site, or the
   destination at the end of a branch. It also covers caves you pass through.
5. **Outlands** is outdoors and far from home: the approaches to a region's
   edge, remote islands and the open sea.
6. **Dungeon** is a place you go into to explore. Every map of a dungeon belongs
   to it, listed from the entrance to the deepest or highest floor. A dungeon
   includes its outdoor parts where they belong to the same place.
7. **One safe road:** every town and city is reachable by at least one path of
   Road maps, except in Sinjoh, which lies beyond Mt. Silver, and in Hoenn's
   Pacifidlog, a floating town in the far sea, and Sootopolis, a crater entered
   only by diving (Underwater Route 126). Boats and trains
   count as part of that path, and so do Surf crossings. A route that isn't
   needed for that path takes the reach that fits its character: long, rugged
   or roundabout routes become Wilds, and open sea far from shore or the
   approach to a region's edge becomes Outlands.
8. A map belongs to exactly one reach.

### Dungeon intents

Each dungeon's notes give its intent, one of Mild, Mild to moderate,
Moderate, Moderate to hard, Hard or Brutal. The intent sets the levels of its
first and deepest floors, and Brutal adds a level-50 floor; the
[wild level scaling spec](wild-level-scaling.md#dungeon-levels) gives the
values. A single-floor dungeon, or one whose notes call it flat, uses the
middle of its range on every map.

### Dungeon floors

A dungeon's notes give its floor order: its steps from the entrance to the
deepest or highest floor, written with arrows. Every map of the dungeon takes
one step, and maps named together at one step share a floor, such as Mt.
Silver's mountainside, item room and Moltres room. A dungeon with no arrows
is a single floor, and one whose notes call it flat is one floor on every map.
The generator needs each map's step, the step count and whether the dungeon is
flat, so *flat* is per-dungeon data, not something it works out.

**Deeper floors** are every step after the first. A single-floor or flat
dungeon has no entrance floor, so it counts as deeper on every map. Item,
trade, friendship and other non-level evolution stages appear in Outlands and
on deeper floors only.

### Kanto

Kanto has 67 maps with wild encounters in Wayfarer, in 46 rows. The safe
roads were checked against the walkable map graph: every town keeps a path of
Road maps. Retired
proof-of-concept maps are excluded. Places with several maps, such as a
dungeon's floors, share one row. Dungeon floors are listed from the entrance to
the deepest floor. That order is authored, not derived from warps.

| Place | Reach | Maps | Notes |
| --- | --- | --- | --- |
| Pallet Town | Road | `MAP_PALLET_TOWN_HNS` | Settlement |
| Viridian City | Road | `MAP_VIRIDIAN_CITY_HNS` | Settlement |
| Pewter City | Road | `MAP_PEWTER_CITY_HNS` | Settlement |
| Cerulean City | Road | `MAP_CERULEAN_CITY_HNS` | Settlement |
| Vermilion City | Road | `MAP_VERMILION_CITY_HNS`, `MAP_VERMILION_CITY_PORT_OUTSIDE_HNS` | Settlement, including the port |
| Lavender Town | Road | `MAP_LAVENDER_TOWN_HNS` | Settlement |
| Celadon City | Road | `MAP_CELADON_CITY_HNS` | Settlement |
| Saffron City | Road | `MAP_SAFFRON_CITY_HNS` | Settlement |
| Fuchsia City | Road | `MAP_FUCHSIA_CITY_HNS` | Settlement |
| Cinnabar Island | Road | `MAP_CINNABAR_ISLAND` | Settlement |
| Route 1 | Road | `MAP_ROUTE1_HNS` | |
| Route 2 | Road | `MAP_ROUTE2_HNS` | |
| Route 3 | Road | `MAP_ROUTE3_HNS` | |
| Route 4 | Road | `MAP_ROUTE4_HNS` | |
| Route 5 | Road | `MAP_ROUTE5_HNS` | |
| Route 6 | Road | `MAP_ROUTE6_HNS` | |
| Route 7 | Road | `MAP_ROUTE7_HNS` | |
| Route 8 | Road | `MAP_ROUTE8_HNS` | |
| Route 11 | Road | `MAP_ROUTE11_HNS` | |
| Route 16 | Road | `MAP_ROUTE16_HNS` | Cycling Road: the safe way to Fuchsia |
| Route 17 | Road | `MAP_ROUTE17_HNS` | Cycling Road: the safe way to Fuchsia |
| Route 18 | Road | `MAP_ROUTE18_HNS` | Cycling Road: the safe way to Fuchsia |
| Route 21 | Road | `MAP_ROUTE21_NORTH`, `MAP_ROUTE21_SOUTH` | Sea lane: the safe way from Pallet to Cinnabar |
| Route 22 | Road | `MAP_ROUTE22_HNS` | |
| Route 24 | Road | `MAP_ROUTE24_HNS` | Nugget Bridge |
| Viridian Forest | Road | `MAP_VIRIDIAN_FOREST_HNS` | Passage |
| Route 12 | Road | `MAP_ROUTE12_HNS` | The safe way from Lavender to the Alola boat on Route 13 |
| Route 13 | Road | `MAP_ROUTE13_HNS` | The Alola boat leaves from here |
| Route 9 | Wilds | `MAP_ROUTE9_HNS` | A rugged path to Rock Tunnel |
| Route 10 | Wilds | `MAP_ROUTE10_HNS` | River country by the Power Plant |
| Route 14 | Wilds | `MAP_ROUTE14_HNS` | The wild way from Route 13 to Fuchsia |
| Route 15 | Wilds | `MAP_ROUTE15_HNS` | The wild way from Route 13 to Fuchsia |
| Route 19 | Wilds | `MAP_ROUTE19` | Coastal water off Fuchsia |
| Mt. Moon | Wilds | `MAP_MT_MOON_CAVE_HNS` | A cave you pass through |
| Rock Tunnel | Wilds | `MAP_ROCK_TUNNEL_1F_HNS`, `MAP_ROCK_TUNNEL_B1F_HNS` | A cave you pass through |
| Diglett's Cave | Wilds | `MAP_DIGLETTS_CAVE_TUNNEL_HNS` | A cave you pass through |
| Safari Zone | Wilds | `MAP_FUCHSIA_CITY_SAFARI_ZONE_BEACH_HNS`, `MAP_FUCHSIA_CITY_SAFARI_ZONE_BRUSH_HNS`, `MAP_FUCHSIA_CITY_SAFARI_ZONE_CAVE_HNS`, `MAP_FUCHSIA_CITY_SAFARI_ZONE_MOUNTAIN_HNS` | Preserve: Beach, Brush, Cave, Mountain |
| Route 25 | Wilds | `MAP_ROUTE25_HNS` | Cerulean Cape, a branch ending at Bill's cottage |
| Route 20 | Outlands | `MAP_ROUTE20` | Open sea around Seafoam, far from shore |
| Route 23 | Outlands | `MAP_ROUTE23_HNS` | The approach to Victory Road at Kanto's edge |
| Pokémon Tower | Dungeon | `MAP_POKEMON_TOWER_3F`, `MAP_POKEMON_TOWER_4F`, `MAP_POKEMON_TOWER_5F`, `MAP_POKEMON_TOWER_6F`, `MAP_POKEMON_TOWER_7F` | Floors 3F → 7F. Mild: a tower in a town |
| Pokémon Mansion | Dungeon | `MAP_POKEMON_MANSION_1F`, `MAP_POKEMON_MANSION_2F`, `MAP_POKEMON_MANSION_3F`, `MAP_POKEMON_MANSION_B1F` | Floors 1F → 2F → 3F → B1F. Mild to moderate |
| Power Plant | Dungeon | `MAP_POWER_PLANT` | Single floor. Moderate |
| Seafoam Islands | Dungeon | `MAP_SEAFOAM_ISLANDS_1F`, `MAP_SEAFOAM_ISLANDS_B1F`, `MAP_SEAFOAM_ISLANDS_B2F`, `MAP_SEAFOAM_ISLANDS_B3F`, `MAP_SEAFOAM_ISLANDS_B4F` | Floors 1F → B4F. Moderate to hard |
| Victory Road | Dungeon | `MAP_VICTORY_ROAD_KANTO_1F_HNS`, `MAP_VICTORY_ROAD_KANTO_B1F_HNS`, `MAP_VICTORY_ROAD_KANTO_B2F_HNS` | Floors 1F → B1F → B2F. Hard. A path, but a dungeon as the gauntlet before the League |
| Cerulean Cave | Dungeon | `MAP_CERULEAN_CAVE_1F_HNS`, `MAP_CERULEAN_CAVE_B1F_HNS`, `MAP_CERULEAN_CAVE_B2F_HNS` | Floors 1F → B1F → B2F. Brutal, with a floor |

### Johto

Johto has 93 maps with wild encounters in Wayfarer, in 53 rows. The same
conventions as Kanto apply. Kanto and Johto connect by Road through the Magnet
Train and the S.S. Aqua. The walking link, Routes 26 and 27, is the wild way.

| Place | Reach | Maps | Notes |
| --- | --- | --- | --- |
| New Bark Town | Road | `MAP_NEW_BARK_TOWN_HNS` | Settlement |
| Cherrygrove City | Road | `MAP_CHERRYGROVE_CITY_HNS` | Settlement |
| Violet City | Road | `MAP_VIOLET_CITY_HNS` | Settlement |
| Azalea Town | Road | `MAP_AZALEA_TOWN_HNS` | Settlement |
| Goldenrod City | Road | `MAP_GOLDENROD_CITY_HNS` | Settlement |
| Ecruteak City | Road | `MAP_ECRUTEAK_CITY_HNS` | Settlement |
| Olivine City | Road | `MAP_OLIVINE_CITY_HNS`, `MAP_OLIVINE_CITY_PORT_OUTSIDE_HNS` | Settlement, including the port |
| Cianwood City | Road | `MAP_CIANWOOD_CITY_HNS` | Settlement |
| Mahogany Town | Road | `MAP_MAHOGANYTOWN_HNS` | Settlement |
| Blackthorn City | Road | `MAP_BLACKTHORN_CITY_HNS` | Settlement |
| Safari Zone Gate | Road | `MAP_SAFARI_ZONE_GATE_HNS` | The rest stop before the Safari Zone, on the way from Route 48 |
| Route 29 | Road | `MAP_ROUTE29_HNS` | |
| Route 30 | Road | `MAP_ROUTE30_HNS` | |
| Route 31 | Road | `MAP_ROUTE31_HNS` | |
| Route 32 | Road | `MAP_ROUTE32_HNS` | |
| Route 33 | Road | `MAP_ROUTE33_HNS` | |
| Route 34 | Road | `MAP_ROUTE34_HNS` | Azalea's safe way out, with Ilex Forest |
| Route 35 | Road | `MAP_ROUTE35_HNS` | |
| Route 36 | Road | `MAP_ROUTE36_HNS` | |
| Route 37 | Road | `MAP_ROUTE37_HNS` | |
| Route 38 | Road | `MAP_ROUTE38_HNS` | |
| Route 39 | Road | `MAP_ROUTE39_HNS` | |
| Route 40 | Road | `MAP_ROUTE40_HNS` | Sea lane: the safe way to Cianwood |
| Route 41 | Road | `MAP_ROUTE41_HNS` | Sea lane: the safe way to Cianwood |
| Route 42 | Road | `MAP_ROUTE42_HNS` | |
| Route 43 | Road | `MAP_ROUTE43_HNS` | The way to Lake of Rage |
| Route 44 | Road | `MAP_ROUTE44_HNS` | |
| Route 45 | Road | `MAP_ROUTE45_HNS` | Blackthorn's safe way out, with Routes 46 and 29 |
| Route 46 | Road | `MAP_ROUTE46_HNS` | |
| Route 48 | Road | `MAP_ROUTE48_HNS` | The way to the Safari Zone |
| Ilex Forest | Road | `MAP_ILEX_FOREST_HNS` | Passage |
| Route 27 | Wilds | `MAP_ROUTE27_HNS` | A wild coast with Tohjo Falls |
| Route 47 | Wilds | `MAP_ROUTE47_HNS` | Cianwood's cliffs |
| Dark Cave | Wilds | `MAP_DARK_CAVE_SOUTH_SIDE_HNS`, `MAP_DARK_CAVE_NORTH_SIDE_HNS` | A cave you pass through, between Routes 31, 45 and 46 |
| Cliff Edge Cave | Wilds | `MAP_CLIFF_EDGE_GATE_HNS`, `MAP_CLIFF_EDGE_CAVE_HNS` | A cave you pass through, between Cianwood and Route 47 |
| Tohjo Falls | Wilds | `MAP_TOHJO_FALLS_CAVERN_HNS` | A cave you pass through, on Route 27 |
| Ruins of Alph | Wilds | `MAP_RUINS_OF_ALPH_OUTSIDE_HNS`, `MAP_RUINS_OF_ALPH_B1F_HNS` | An ancient site off the road, with its chambers |
| National Park | Wilds | `MAP_NATIONAL_PARK_NORMAL_HNS`, `MAP_NATIONAL_PARK_BUG_CONTEST_HNS` | A park off the road, including the Bug-Catching Contest version |
| Lake of Rage | Wilds | `MAP_LAKE_OF_RAGE_HNS` | A lake off the road, at the end of Route 43 |
| Safari Zone | Wilds | `MAP_SAFARI_ZONE_LOW_LEFT_HNS`, `MAP_SAFARI_ZONE_LOW_MID_HNS`, `MAP_SAFARI_ZONE_LOW_RIGHT_HNS`, `MAP_SAFARI_ZONE_TOP_LEFT_HNS`, `MAP_SAFARI_ZONE_TOP_MID_HNS`, `MAP_SAFARI_ZONE_TOP_RIGHT_HNS` | Preserve |
| Route 26 | Outlands | `MAP_ROUTE26_HNS` | The approach to Victory Road and the League |
| Route 28 | Outlands | `MAP_ROUTE28_HNS` | The approach to Mt. Silver at Johto's edge |
| Union Cave | Dungeon | `MAP_UNION_CAVE_1F_HNS`, `MAP_UNION_CAVE_B1F_HNS`, `MAP_UNION_CAVE_B2F_HNS` | Floors 1F → B1F → B2F. Mild. A path between Routes 32 and 33, with B2F as a dead end below |
| Ice Path | Dungeon | `MAP_ICE_PATH_1F_HNS`, `MAP_ICE_PATH_B1F_HNS`, `MAP_ICE_PATH_B2F_HNS`, `MAP_ICE_PATH_B3F_HNS`, `MAP_ICE_PATH_B4F_HNS` | Floors 1F → B1F → B2F → B3F → B4F. Moderate. A path to Blackthorn whose lower floors loop |
| Sprout Tower | Dungeon | `MAP_SPROUT_TOWER_2F_HNS`, `MAP_SPROUT_TOWER_3F_HNS` | Floors 2F → 3F. Mild: a tower in a town |
| Slowpoke Well | Dungeon | `MAP_SLOWPOKE_WELL_B1F_HNS`, `MAP_SLOWPOKE_WELL_B2F_HNS` | Floors B1F → B2F. Mild |
| Burned Tower | Dungeon | `MAP_BURNED_TOWER_1F_HNS`, `MAP_BURNED_TOWER_B1F_HNS` | Floors 1F → B1F. Mild |
| Rocket Hideout | Dungeon | `MAP_ROCKET_HIDEOUT_B1F_HNS` | Single floor. Mild |
| Mt. Mortar | Dungeon | `MAP_MT_MORTAR_1F_SOUTH_HNS`, `MAP_MT_MORTAR_1F_NORTH_HNS`, `MAP_MT_MORTAR_2F_HNS`, `MAP_MT_MORTAR_B1F_HNS` | Floors 1F south → 1F north → 2F → B1F. Moderate |
| Tin Tower | Dungeon | `MAP_TIN_TOWER_3F_HNS`, `MAP_TIN_TOWER_4F_HNS`, `MAP_TIN_TOWER_5F_HNS`, `MAP_TIN_TOWER_6F_HNS`, `MAP_TIN_TOWER_7F_HNS`, `MAP_TIN_TOWER_8F_HNS`, `MAP_TIN_TOWER_9F_HNS` | Floors 3F → 9F. Moderate to hard, climbing to Ho-Oh |
| Dragon's Den | Dungeon | `MAP_DRAGONS_DEN_CAVERN_HNS` | Single floor. Hard |
| Whirl Islands | Dungeon | `MAP_WHIRL_ISLANDS_1F_HNS`, `MAP_WHIRL_ISLANDS_B1F_HNS`, `MAP_WHIRL_ISLANDS_B1F_INNER_HNS`, `MAP_WHIRL_ISLANDS_B2F_HNS`, `MAP_WHIRL_ISLANDS_B3F_HNS`, `MAP_WHIRL_ISLANDS_DESCENT_HNS` | Floors 1F → B1F → B1F inner → B2F → B3F → Descent. Hard, descending to Lugia |
| Mt. Silver | Dungeon | `MAP_MT_SILVER_OUTSIDE_HNS`, `MAP_MT_SILVER_1F_WATERFALL_ROOM_HNS`, `MAP_MT_SILVER_MOUNTAIN_SIDE_HNS`, `MAP_MT_SILVER_1F_ITEM_ROOM_HNS`, `MAP_MT_SILVER_1F_MOLTRES_ROOM_HNS`, `MAP_MT_SILVER_2F_HNS`, `MAP_MT_SILVER_SNOW_HNS`, `MAP_MT_SILVER_3F_HNS` | Floors: outside → 1F waterfall room → mountainside, item room, Moltres room → 2F → snow → 3F. Brutal, with a floor. Includes its outdoor parts |

### Sevii Islands

The Sevii Islands have 58 maps with wild encounters in Wayfarer, in 27 rows.
The same conventions apply.

- **Islands 1–3** (One, Two and Three Island) are near home. Their towns and
  connecting paths are Road, and their side areas are Wilds.
- **Islands 4–7** are the outer islands, far from home. Their towns are Road.
  Outdoor maps directly next to a town are Wilds, to soften the first step out
  of town. Every other outdoor map is Outlands.
- Every island town is reachable by Road through the Seagallop ferry, so the
  one-safe-road rule holds without any Sevii route. Two Island, Three Island,
  Six Island and Seven Island have no wild encounters in town.

| Place | Reach | Maps | Notes |
| --- | --- | --- | --- |
| One Island | Road | `MAP_ONE_ISLAND` | Settlement |
| Four Island | Road | `MAP_FOUR_ISLAND` | Settlement |
| Five Island | Road | `MAP_FIVE_ISLAND` | Settlement |
| Three Isle Port | Road | `MAP_THREE_ISLAND_PORT` | Three Island's harbor side |
| Kindle Road | Road | `MAP_ONE_ISLAND_KINDLE_ROAD` | The way from One Island to Mt. Ember |
| Bond Bridge | Road | `MAP_THREE_ISLAND_BOND_BRIDGE` | The way from Three Island to Berry Forest |
| Treasure Beach | Wilds | `MAP_ONE_ISLAND_TREASURE_BEACH` | A beach off the road, south of One Island |
| Cape Brink | Wilds | `MAP_TWO_ISLAND_CAPE_BRINK` | A cape at the end of Two Island |
| Berry Forest | Wilds | `MAP_THREE_ISLAND_BERRY_FOREST` | A forest at the end of Bond Bridge |
| Five Isle Meadow | Wilds | `MAP_FIVE_ISLAND_MEADOW` | Outer islands, next to Five Island |
| Resort Gorgeous | Wilds | `MAP_FIVE_ISLAND_RESORT_GORGEOUS` | Outer islands, across the water from Five Island |
| Water Path | Wilds | `MAP_SIX_ISLAND_WATER_PATH` | Outer islands, next to Six Island |
| Sevault Canyon Entrance | Wilds | `MAP_SEVEN_ISLAND_SEVAULT_CANYON_ENTRANCE` | Outer islands, next to Seven Island |
| Trainer Tower grounds | Wilds | `MAP_SEVEN_ISLAND_TRAINER_TOWER` | Outer islands: the waters next to Seven Island, around Trainer Tower |
| Memorial Pillar | Outlands | `MAP_FIVE_ISLAND_MEMORIAL_PILLAR` | Outer islands |
| Water Labyrinth | Outlands | `MAP_FIVE_ISLAND_WATER_LABYRINTH` | Outer islands |
| Ruin Valley | Outlands | `MAP_SIX_ISLAND_RUIN_VALLEY` | Outer islands |
| Green Path | Outlands | `MAP_SIX_ISLAND_GREEN_PATH` | Outer islands |
| Pattern Bush | Outlands | `MAP_SIX_ISLAND_PATTERN_BUSH` | Outer islands |
| Outcast Island | Outlands | `MAP_SIX_ISLAND_OUTCAST_ISLAND` | Outer islands |
| Sevault Canyon | Outlands | `MAP_SEVEN_ISLAND_SEVAULT_CANYON` | Outer islands: the canyon at the chain's far end |
| Tanoby Ruins | Outlands | `MAP_SEVEN_ISLAND_TANOBY_RUINS` | Outer islands: the waters around the ruins |
| Icefall Cave | Dungeon | `MAP_FOUR_ISLAND_ICEFALL_CAVE_ENTRANCE`, `MAP_FOUR_ISLAND_ICEFALL_CAVE_1F`, `MAP_FOUR_ISLAND_ICEFALL_CAVE_B1F`, `MAP_FOUR_ISLAND_ICEFALL_CAVE_BACK` | Floors entrance → 1F → B1F → back. Moderate |
| Altering Cave | Dungeon | `MAP_SIX_ISLAND_ALTERING_CAVE` | Single floor. Moderate |
| Tanoby Chambers | Dungeon | `MAP_SEVEN_ISLAND_TANOBY_RUINS_MONEAN_CHAMBER`, `MAP_SEVEN_ISLAND_TANOBY_RUINS_LIPTOO_CHAMBER`, `MAP_SEVEN_ISLAND_TANOBY_RUINS_WEEPTH_CHAMBER`, `MAP_SEVEN_ISLAND_TANOBY_RUINS_DILFORD_CHAMBER`, `MAP_SEVEN_ISLAND_TANOBY_RUINS_SCUFIB_CHAMBER`, `MAP_SEVEN_ISLAND_TANOBY_RUINS_RIXY_CHAMBER`, `MAP_SEVEN_ISLAND_TANOBY_RUINS_VIAPOIS_CHAMBER` | Seven small chambers off the ruins, each one floor. Flat and moderate |
| Mt. Ember | Dungeon | `MAP_MT_EMBER_EXTERIOR`, `MAP_MT_EMBER_SUMMIT_PATH_1F`, `MAP_MT_EMBER_SUMMIT_PATH_2F`, `MAP_MT_EMBER_SUMMIT_PATH_3F`, `MAP_MT_EMBER_RUBY_PATH_1F`, `MAP_MT_EMBER_RUBY_PATH_B1F`, `MAP_MT_EMBER_RUBY_PATH_B1F_STAIRS`, `MAP_MT_EMBER_RUBY_PATH_B2F`, `MAP_MT_EMBER_RUBY_PATH_B2F_STAIRS`, `MAP_MT_EMBER_RUBY_PATH_B3F` | Floors: exterior → summit path 1F → 2F → 3F → ruby path 1F → B1F, with its stairs map → B2F, with its stairs map → B3F. Moderate to hard. Includes its outdoor slopes |
| Lost Cave | Dungeon | `MAP_FIVE_ISLAND_LOST_CAVE_ROOM1`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM2`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM3`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM4`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM5`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM6`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM7`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM8`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM9`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM10`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM11`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM12`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM13`, `MAP_FIVE_ISLAND_LOST_CAVE_ROOM14` | Rooms 1 → 14, a maze. Hard |

### Alola

Alola has 10 maps with wild encounters in Wayfarer, in 10 rows. Each island is
a single map that holds its wild ground, so the whole island takes one reach.
Melemele also holds the arrival village; the other islands have at most a lone
house, which doesn't make a settlement.

- **Melemele Isle** is the only settlement. Its arrival village makes it Road,
  and its safe road is the boat from Route 13 in Kanto.
- Outdoor maps directly next to Melemele are Wilds, as on Sevii's outer
  islands. Ula'ula Isle, beyond Akala, is Outlands.
- **Exception:** Poni Isle touches Melemele, but it is Outlands. It stays
  Alola's wildest island, as in the original games.
- Travel between the islands needs Surf. Ula'ula Forest has no wild encounters.

| Place | Reach | Maps | Notes |
| --- | --- | --- | --- |
| Melemele Isle | Road | `MAP_MELEMELE_ISLE_HNS` | Settlement: the arrival village, reached by boat from Route 13 |
| Akala Isle | Wilds | `MAP_AKALA_ISLE_HNS` | Next to Melemele |
| Alola sea | Wilds | `MAP_ALOLA_WATER_HNS` | Next to Melemele: the sea between the islands |
| Akala Forest | Wilds | `MAP_AKALA_FOREST_HNS` | A forest off Akala Isle |
| Ula'ula Isle | Outlands | `MAP_ULAULA_ISLE_HNS` | Beyond Akala, the far end of the chain |
| Poni Isle | Outlands | `MAP_PONI_ISLE_HNS` | Exception: next to Melemele, but kept as Alola's wildest island |
| Akala Cave | Dungeon | `MAP_AKALA_CAVE_HNS` | Single floor. Moderate |
| Ula'ula Cave | Dungeon | `MAP_ULA_ULA_CAVE_HNS` | Single floor. Hard |
| Ula'ula Cave 2 | Dungeon | `MAP_ULA_ULA_CAVE_2_HNS` | Single floor. Hard |
| Poni Cave | Dungeon | `MAP_PONI_CAVE_HNS` | Single floor. Hard |

### Sinjoh and Sinnoh

Sinjoh has 8 maps with wild encounters in Wayfarer, in 6 rows. New Sinjoh,
its only town, has no wild encounters of its own. The Sinnoh exploration maps
have no wild encounters at all, so they need no reach.

- **Sinjoh lies beyond Mt. Silver.** The only way in is through Mt. Silver's
  first floor and Snowswept Cavern, so Sinjoh is exempt from the one-safe-road
  rule. Meara's Azure Flute warp also leads into Mt. Silver, not past it.
- Snowswept Cavern is a cave you pass through, so it is Wilds. Route 49, next
  to New Sinjoh, is Wilds. Route 50 is the approach to the Ruins at the region's
  edge, so it is Outlands.

| Place | Reach | Maps | Notes |
| --- | --- | --- | --- |
| Snowswept Cavern | Wilds | `MAP_SNOWSWEPT_CAVERN_HNS` | A cave you pass through, from Mt. Silver's first floor to Route 49 |
| Route 49 | Wilds | `MAP_ROUTE49_HNS` | The way in, next to New Sinjoh |
| Sinjoh Ruins | Wilds | `MAP_SINJOH_RUINS_HNS` | An ancient site, not a settlement, despite its house |
| Route 50 | Outlands | `MAP_ROUTE50_HNS` | The snowy approach to the Ruins at Sinjoh's edge |
| New Sinjoh Hot Springs | Dungeon | `MAP_NEWSINJOH_HOTSPRINGS_HNS` | Single floor. Moderate. A hot-spring cave beside New Sinjoh |
| Sinjoh Ruins chambers | Dungeon | `MAP_SINJOH_RUINS_TEMPLE_HNS`, `MAP_SINJOH_RUINS_REGICE_ROOM_HNS`, `MAP_SINJOH_RUINS_REGIROCK_ROOM_HNS` | Separate one-room chambers off the Ruins: the temple and two Regi rooms. Flat and moderate. The Regi rooms open only after collecting plates, so nothing found only there may be placed in them |

### Hoenn

Hoenn has 116 maps with wild encounters in Wayfarer, in 64 rows. Littleroot,
Oldale, Rustboro, Mauville, Verdanturf, Fallarbor, Lavaridge and Fortree have
no wild encounters in town. The safe roads were checked against the walkable
map graph: every Hoenn town keeps a path of Road maps except Pacifidlog and
Sootopolis, which are exempt from the one-safe-road rule. Pacifidlog is a
floating town in the far sea, and Sootopolis a crater entered only by diving
from Underwater Route 126.

- The western sea lanes to Dewford and the eastern lanes through Mossdeep to
  Ever Grande are Road. Lilycove also has its ferry.
- The far sea beyond those lanes is Outlands. Far-sea maps directly next to a
  town are Wilds, as on Sevii's outer islands.
- Route 111 is one map that holds the desert, so the whole route is Road.

| Place | Reach | Maps | Notes |
| --- | --- | --- | --- |
| Petalburg City | Road | `MAP_PETALBURG_CITY` | Settlement |
| Dewford Town | Road | `MAP_DEWFORD_TOWN` | Settlement |
| Slateport City | Road | `MAP_SLATEPORT_CITY` | Settlement |
| Lilycove City | Road | `MAP_LILYCOVE_CITY` | Settlement |
| Mossdeep City | Road | `MAP_MOSSDEEP_CITY` | Settlement |
| Sootopolis City | Road | `MAP_SOOTOPOLIS_CITY` | Settlement |
| Pacifidlog Town | Road | `MAP_PACIFIDLOG_TOWN` | Settlement |
| Ever Grande City | Road | `MAP_EVER_GRANDE_CITY` | Settlement |
| Route 101 | Road | `MAP_ROUTE101` | |
| Route 102 | Road | `MAP_ROUTE102` | |
| Route 103 | Road | `MAP_ROUTE103` | |
| Route 104 | Road | `MAP_ROUTE104` | |
| Route 105 | Road | `MAP_ROUTE105` | Sea lane: the safe way to Dewford |
| Route 106 | Road | `MAP_ROUTE106` | Sea lane: the safe way to Dewford |
| Route 109 | Road | `MAP_ROUTE109` | Slateport's beach and sea lane |
| Route 110 | Road | `MAP_ROUTE110` | |
| Route 111 | Road | `MAP_ROUTE111` | The desert lies inside this map |
| Route 112 | Road | `MAP_ROUTE112` | |
| Route 113 | Road | `MAP_ROUTE113` | Fallarbor's safe way |
| Route 115 | Road | `MAP_ROUTE115` | The way to Meteor Falls |
| Route 116 | Road | `MAP_ROUTE116` | |
| Route 117 | Road | `MAP_ROUTE117` | |
| Route 118 | Road | `MAP_ROUTE118` | |
| Route 119 | Road | `MAP_ROUTE119` | Fortree's safe way |
| Route 121 | Road | `MAP_ROUTE121` | |
| Route 122 | Road | `MAP_ROUTE122` | The way to Mt. Pyre |
| Route 123 | Road | `MAP_ROUTE123` | |
| Route 124 | Road | `MAP_ROUTE124` | Sea lane: the safe way to Mossdeep |
| Route 127 | Road | `MAP_ROUTE127` | Sea lane: Mossdeep to Route 128 and Ever Grande |
| Route 128 | Road | `MAP_ROUTE128` | Sea lane: the safe way to Ever Grande |
| Petalburg Woods | Road | `MAP_PETALBURG_WOODS` | Passage |
| Route 107 | Wilds | `MAP_ROUTE107` | Open sea between Dewford and Slateport |
| Route 108 | Wilds | `MAP_ROUTE108` | Open sea around the Abandoned Ship |
| Route 114 | Wilds | `MAP_ROUTE114` | A rugged route to Meteor Falls |
| Route 120 | Wilds | `MAP_ROUTE120` | Jungle: Fortree's wild way to Lilycove |
| Route 125 | Wilds | `MAP_ROUTE125` | Far sea, next to Mossdeep |
| Route 132 | Wilds | `MAP_ROUTE132` | Far sea, next to Pacifidlog |
| Route 134 | Wilds | `MAP_ROUTE134` | Far sea, next to Slateport |
| Jagged Pass | Wilds | `MAP_JAGGED_PASS` | A rugged mountain trail |
| Rusturf Tunnel | Wilds | `MAP_RUSTURF_TUNNEL` | A cave you pass through |
| Fiery Path | Wilds | `MAP_FIERY_PATH` | A cave you pass through |
| Safari Zone | Wilds | `MAP_SAFARI_ZONE_NORTH`, `MAP_SAFARI_ZONE_NORTHEAST`, `MAP_SAFARI_ZONE_NORTHWEST`, `MAP_SAFARI_ZONE_SOUTH`, `MAP_SAFARI_ZONE_SOUTHEAST`, `MAP_SAFARI_ZONE_SOUTHWEST` | Preserve |
| Route 126 | Outlands | `MAP_ROUTE126` | Far sea around Sootopolis |
| Route 129 | Outlands | `MAP_ROUTE129` | Far sea |
| Route 130 | Outlands | `MAP_ROUTE130` | Far sea, with Mirage Island |
| Route 131 | Outlands | `MAP_ROUTE131` | Far sea, by Sky Pillar |
| Route 133 | Outlands | `MAP_ROUTE133` | Far sea, with strong currents |
| Underwater Route 124 | Outlands | `MAP_UNDERWATER_ROUTE124` | Deep water |
| Underwater Route 126 | Outlands | `MAP_UNDERWATER_ROUTE126` | Deep water |
| Granite Cave | Dungeon | `MAP_GRANITE_CAVE_1F`, `MAP_GRANITE_CAVE_B1F`, `MAP_GRANITE_CAVE_B2F`, `MAP_GRANITE_CAVE_STEVENS_ROOM` | Floors 1F → B1F → B2F → Steven's room. Mild |
| Altering Cave | Dungeon | `MAP_ALTERING_CAVE` | Single floor. Mild |
| Abandoned Ship | Dungeon | `MAP_ABANDONED_SHIP_ROOMS_B1F`, `MAP_ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS` | Floors B1F rooms → hidden floor. Mild to moderate |
| New Mauville | Dungeon | `MAP_NEW_MAUVILLE_ENTRANCE`, `MAP_NEW_MAUVILLE_INSIDE` | Floors entrance → inside. Moderate |
| Desert Underpass | Dungeon | `MAP_DESERT_UNDERPASS` | Single floor. Moderate |
| Mirage Tower | Dungeon | `MAP_MIRAGE_TOWER_1F`, `MAP_MIRAGE_TOWER_2F`, `MAP_MIRAGE_TOWER_3F`, `MAP_MIRAGE_TOWER_4F` | Floors 1F → 4F. Moderate |
| Meteor Falls | Dungeon | `MAP_METEOR_FALLS_1F_1R`, `MAP_METEOR_FALLS_1F_2R`, `MAP_METEOR_FALLS_B1F_1R`, `MAP_METEOR_FALLS_B1F_2R`, `MAP_METEOR_FALLS_STEVENS_CAVE` | Floors 1F room 1 → 1F room 2 → B1F room 1 → B1F room 2 → Steven's cave, one step each. Moderate to hard. A path between Routes 114 and 115, with deep floors beyond it |
| Magma Hideout | Dungeon | `MAP_MAGMA_HIDEOUT_1F`, `MAP_MAGMA_HIDEOUT_2F_1R`, `MAP_MAGMA_HIDEOUT_2F_2R`, `MAP_MAGMA_HIDEOUT_2F_3R`, `MAP_MAGMA_HIDEOUT_3F_1R`, `MAP_MAGMA_HIDEOUT_3F_2R`, `MAP_MAGMA_HIDEOUT_3F_3R`, `MAP_MAGMA_HIDEOUT_4F` | Floors 1F → 2F rooms 1–3 → 3F rooms 1–3 → 4F, one step per map. Moderate to hard |
| Mt. Pyre | Dungeon | `MAP_MT_PYRE_1F`, `MAP_MT_PYRE_2F`, `MAP_MT_PYRE_3F`, `MAP_MT_PYRE_4F`, `MAP_MT_PYRE_5F`, `MAP_MT_PYRE_6F`, `MAP_MT_PYRE_EXTERIOR`, `MAP_MT_PYRE_SUMMIT` | Floors 1F → 6F → exterior → summit. Moderate to hard. Includes its outdoor slopes |
| Shoal Cave | Dungeon | `MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM`, `MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM`, `MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM`, `MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM`, `MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM` | Floors entrance → inner → stairs → lower → ice room. Hard |
| Seafloor Cavern | Dungeon | `MAP_SEAFLOOR_CAVERN_ENTRANCE`, `MAP_SEAFLOOR_CAVERN_ROOM1`, `MAP_SEAFLOOR_CAVERN_ROOM2`, `MAP_SEAFLOOR_CAVERN_ROOM3`, `MAP_SEAFLOOR_CAVERN_ROOM4`, `MAP_SEAFLOOR_CAVERN_ROOM5`, `MAP_SEAFLOOR_CAVERN_ROOM6`, `MAP_SEAFLOOR_CAVERN_ROOM7`, `MAP_SEAFLOOR_CAVERN_ROOM8` | Floors entrance → rooms 1–8. Hard |
| Artisan Cave | Dungeon | `MAP_ARTISAN_CAVE_1F`, `MAP_ARTISAN_CAVE_B1F` | Floors 1F → B1F. Hard. On the Battle Frontier island |
| Sky Pillar | Dungeon | `MAP_SKY_PILLAR_1F`, `MAP_SKY_PILLAR_3F`, `MAP_SKY_PILLAR_5F` | Floors 1F → 3F → 5F. Hard, climbing to Rayquaza |
| Cave of Origin | Dungeon | `MAP_CAVE_OF_ORIGIN_ENTRANCE`, `MAP_CAVE_OF_ORIGIN_1F`, `MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP1`, `MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP2`, `MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP3` | Floors entrance → 1F, with the three unused Ruby/Sapphire maps sharing 1F's step. Hard. They are probably unreachable |
| Victory Road | Dungeon | `MAP_VICTORY_ROAD_1F`, `MAP_VICTORY_ROAD_B1F`, `MAP_VICTORY_ROAD_B2F` | Floors 1F → B1F → B2F. Hard. The gauntlet before the League |

### Remote islands

Two HNS islands are reached by boat from the Olivine and Vermilion ports, with
an event item. Their wild encounters are ordinary, and each is a remote island,
so they are Outlands. Each is also a legendary's lair. Their tables are in the
[Johto encounter tables](johto-encounter-tables.md).

| Place | Reach | Maps | Notes |
| --- | --- | --- | --- |
| Faraway Island | Outlands | `MAP_FARAWAY_ISLAND_ENTRANCE_HNS` | Reached with the Old Sea Map. Mew's lair |
| Southern Island | Outlands | `MAP_SOUTHERN_ISLAND_EXTERIOR_HNS` | Reached with the Eon Ticket. Latias and Latios's lair |

### Places without wild encounters

Trainers and item spots also stand on maps with no wild encounters. Their
level and item odds still need a reach, so every such map that hosts a covered
Trainer or an item spot resolves one with these rules. Their dungeon levels
only drive trainers and items, since nothing spawns there.

1. **Gyms and battle facilities have no reach.** Gym members use their own
   curve, and facility trainers and items belong to the facility. An item
   spot inside a Gym counts as Road.
2. **Ferries are Road.** The S.S. Aqua and the S.S. Tidal are part of the safe
   path, like the routes they connect.
3. **Interiors take the map they open onto.** Labs, houses, shops, contest
   halls, gatehouses, the Trick House and the Seashore House use the reach of
   their outside map, which is Road in a town.
4. **Extra floors join their dungeon** at their place in the floor order, as
   listed below.
5. **Story sites become dungeons.** A hideout, office tower or ship you go
   into to explore is a dungeon with its own intent, as listed below.

#### Joining an existing place

| Map(s) | Joins | Notes |
| --- | --- | --- |
| `MAP_SPROUT_TOWER_1F_HNS` | Sprout Tower | New first floor: 1F → 2F → 3F |
| `MAP_ROCKET_HIDEOUT_B2F_HNS`, `MAP_ROCKET_HIDEOUT_B3F_HNS` | Rocket Hideout (Johto) | Floors B1F → B2F → B3F. Its intent rises to Mild to moderate |
| `MAP_ABANDONED_SHIP_CORRIDORS_1F`, `MAP_ABANDONED_SHIP_ROOMS_1F`, `MAP_ABANDONED_SHIP_ROOMS2_1F`, `MAP_ABANDONED_SHIP_CAPTAINS_OFFICE` | Abandoned Ship | New first step, before the B1F rooms |
| `MAP_ABANDONED_SHIP_CORRIDORS_B1F`, `MAP_ABANDONED_SHIP_ROOM_B1F`, `MAP_ABANDONED_SHIP_ROOMS2_B1F` | Abandoned Ship | Share the B1F step |
| `MAP_ABANDONED_SHIP_HIDDEN_FLOOR_ROOMS` | Abandoned Ship | Shares the hidden-floor step |
| `MAP_SEAFLOOR_CAVERN_ROOM9` | Seafloor Cavern | New last step after room 8 |
| `MAP_RUINS_OF_ALPH_PUZZLE_AND_REWARD_CHAMBERS_HNS` | Ruins of Alph | Wilds |
| `MAP_MT_MOON_OUTSIDE_HNS` | Mt. Moon | Wilds |
| `MAP_ROUTE26NORTH_HNS` | Route 26 | Outlands |
| `MAP_ULA_ULA_FOREST_HNS` | Ula'ula Isle | Outlands |
| `MAP_BELLCHIME_TRAIL_HNS` | Ecruteak City | Road |
| `MAP_MT_CHIMNEY` | Jagged Pass | Wilds: the summit above the trail |
| `MAP_UNDERWATER_ROUTE127`, `MAP_UNDERWATER_ROUTE128` | Underwater Route 126 | Outlands: deep water |
| `MAP_SOUTHERN_ISLAND_INTERIOR_HNS` | Southern Island | Outlands |
| `MAP_NAVEL_ROCK_TOP` | Remote islands | Outlands |

#### New dungeons

| Place | Region | Maps | Floors and intent |
| --- | --- | --- | --- |
| Rocket Hideout (Celadon) | Kanto | `MAP_ROCKET_HIDEOUT_B1F`, `MAP_ROCKET_HIDEOUT_B2F`, `MAP_ROCKET_HIDEOUT_B3F`, `MAP_ROCKET_HIDEOUT_B4F` | B1F → B4F. Moderate |
| Silph Co. | Kanto | `MAP_SILPH_CO_2F` to `MAP_SILPH_CO_11F` | 2F → 11F, one step per floor. Moderate to hard |
| S.S. Anne | Kanto | Deck and 1F rooms; 2F rooms; B1F corridor, B1F rooms and kitchen | Deck and 1F → 2F → B1F. Mild |
| Radio Tower | Johto | `MAP_GOLDENROD_CITY_RADIO_TOWER_2F_HNS` to `MAP_GOLDENROD_CITY_RADIO_TOWER_5F_HNS` | 2F → 5F. Moderate to hard |
| Goldenrod Underground | Johto | Underground tunnel, Department Store basement, underground storage, underground switches | Tunnel → basement → storage → switches. Mild to moderate |
| Olivine Lighthouse | Johto | `MAP_OLIVINE_CITY_LIGHTHOUSE_HNS` | Single floor. Mild |
| Rocket Warehouse | Sevii | `MAP_FIVE_ISLAND_ROCKET_WAREHOUSE` | Single floor. Moderate to hard |
| Aqua Hideout | Hoenn | `MAP_AQUA_HIDEOUT_1F`, `MAP_AQUA_HIDEOUT_B1F`, `MAP_AQUA_HIDEOUT_B2F` | 1F → B1F → B2F. Moderate to hard |
| Weather Institute | Hoenn | `MAP_ROUTE119_WEATHER_INSTITUTE_1F`, `MAP_ROUTE119_WEATHER_INSTITUTE_2F` | 1F → 2F. Mild |
| Space Center | Hoenn | `MAP_MOSSDEEP_CITY_SPACE_CENTER_1F`, `MAP_MOSSDEEP_CITY_SPACE_CENTER_2F` | 1F → 2F. Moderate to hard |
| Scorched Slab | Hoenn | `MAP_SCORCHED_SLAB` | Single floor. Moderate |

Map constants are written as they appear in the generated map table; the
generator confirms each one and fails on a name it can't find.

## Open questions

- **Alola's single-map islands:** each island takes one reach for both its
  village and its wild ground. Giving Alola real Road, Wilds and Outlands areas
  needs the islands split into smaller maps, which waits for Porymap work.
