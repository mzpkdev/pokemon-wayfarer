# Notable trainer routines (review draft)

Starting **activity cycles** and **favourite spots** for the 37 placeable
notable trainers in `devtools/ui/src/modules/trainer-balance/catalog.json`
(every entry but Tate & Liza, who are never placed). The
[world simulation](../specs/notable-world-simulation.md#activity-cycle)
runs each cycle from the trainer's home base, and
[spot choice](../specs/notable-spots.md#choosing-a-spot) uses the
favourites first.

**25 are simulated in v0.** Twelve trainers have only a face-only overworld
sprite, with no walk cycle, so they aren't simulated for now
([walking sprites](../specs/notable-world-simulation.md#known-limitation-walking-sprites)):
Agatha, Bruno, Koga, Roxanne, Brawly, Wattson, Flannery, Winona, Sidney,
Phoebe, Glacia, and Drake. Their routines stay authored here for later, each
marked **Not simulated in v0 (sprite limitation)**. The check still
validates them, so they are ready once they can walk, but leaves them out
of the shared favourites, since they never take a spot.

Status: review draft. Nothing here is in the catalog yet. Evidence:
[notable-trainer-routines.py](notable-trainer-routines.py) reads this file
and checks every routine ([check](#check)).

## Rules

- **Home base.** The spots spec's
  [home base](../specs/notable-spots.md#home-base): a Gym Leader's Gym
  city, otherwise the authored home map.
- **Cycle.** Three or four steps, each one activity: train, care, study,
  home, relax, shop, gamble, sightsee, fish, visit, or lie low. The cycle
  repeats. A **home** step is the leader's own Gym, or for anyone else a
  place on the home map
  ([home places](../specs/notable-world-simulation.md#home-places)).
- **Favourites.** Up to three per trainer. Each is a
  [named spot](notable-named-spots.md) (by its label and map) or a
  detected spot (by map and kind: Pokémon Center, store, Game Corner,
  Gym, tall grass, water's edge, town square, or NPC chat), and serves one
  step of the cycle, an activity that spot offers. A favourite ignores the
  radius, so it is how a trainer reaches a signature place outside their
  home radius. It is used when it has room; otherwise the derived pick
  runs ([favourites](../specs/notable-spots.md#favourites)). When two
  favourites serve the same step, the first listed wins.
- **Aloof.** Sabrina, Agatha, Lance, Clair, Karen, Glacia, Wallace, and
  Steven never use a public spot (Centers, stores, Game Corners, Gyms,
  town squares, NPC chats, and named spots on town or city maps or in the
  interiors entered from them), so their favourites are remote: tall
  grass, water's edge, or a named spot away from towns.
- **Region.** Travellers may favour places in other regions; everyone
  else keeps favourites in their home region (the home base's region, or
  the catalog's home region: Sevii or Kanto for Lorelei, Kanto or Johto
  for Koga).
- **Capacity.** A named spot holds 1 or 2 trainers and an interior map
  holds 1 notable, so a favourite two trainers share is often taken; the
  second then takes the derived pick. The [check](#check) lists shared
  favourites.
- **Gym Leaders.** An unbeaten Gym Leader is
  [home-locked](../specs/notable-world-simulation.md#who-is-simulated) in
  their Gym, so their cycle and favourites apply only after the player
  holds their badge.

### Sources

Hooks come from the same places as the
[voice bits](notable-trainer-voices.md#source-priority): anime first, then
later games (FRLG, HGSS, ORAS, Emerald, Let's Go), then manga. Where there
is no canon hook, the step fits the trainer's type or role. `(verify)`
marks a hook whose source needs checking.

---

## Kanto

### Brock

- **Home base:** `PewterCity_hns` (his Gym city). Traveller.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | Rock-type training on the routes round Pewter. |
| 2 | care | The would-be breeder who looks after everyone's Pokémon (anime). |
| 3 | study | Breeding and fossils; Pewter's museum is next door. |
| 4 | home | Back to his Gym and his family. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Route 34 Day Care (`Route34_DayCare_hns`) | named | care | Breeding is his dream (anime); a traveller, he can go to Johto. |
| 2 | Oak's Lab (`PalletTown_Lab_hns`) | named | study | Kanto's research hub, 5 hops away; he studies under Professor Ivy in the anime (verify). |

The Pewter Museum is a [haunt](../specs/notable-haunts.md), not a spot, so
it can't be a favourite.

### Misty

- **Home base:** `CeruleanCity_hns` (her Gym city). Traveller.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | relax | Lazing by the water at Cerulean Cape (anime). |
| 2 | train | Water training on Routes 24 and 25. |
| 3 | fish | A Water Pokémon master in the making, rod in hand (anime). |
| 4 | home | Back to the Gym she runs with her sisters. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Nugget Bridge (`Route24_hns`) | water's edge | fish | The river under Nugget Bridge, north of her Gym. |
| 2 | Route 25 (`Route25_hns`) | water's edge | relax | The cape road, Cerulean's date spot (FRLG, HGSS). |

### Lt. Surge

- **Home base:** `VermilionCity_hns` (his Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | The Gym and its trash-can locks. |
| 2 | visit | A soldier among sailors: the port and the S.S. Anne. |
| 3 | gamble | The "Lightning American" with a taste for risk. |
| 4 | train | Electric training on Routes 6 and 11. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Vermilion harbour (`VermilionCity_PortOutside_hns`) | named | visit | The S.S. Anne pier, one hop from his Gym. |
| 2 | Celadon Game Corner (`CeladonCity_GameCorner_hns`) | Game Corner | gamble | Kanto's only Game Corner, 5 hops away. |

### Erika

- **Home base:** `CeladonCity_hns` (her Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | Her flower Gym, where she dozes off (anime, HGSS). |
| 2 | shop | Perfume and gifts in Celadon (her perfume shop, anime). |
| 3 | relax | A quiet spell by the Celadon pond. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Celadon Dept. Store 4F (`CeladonCity_DepartmentStore_4F_hns`) | store | shop | No perfume floor exists; 4F, "Wiseman Gifts", is the gift floor. |
| 2 | Celadon pond (`CeladonCity_hns`) | water's edge | relax | The water beside her Gym. |

### Janine

- **Home base:** `FuchsiaCity_hns` (her Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | Ninja training in the grass east of Fuchsia. |
| 2 | sightsee | The Safari Zone at Fuchsia's north edge. |
| 3 | care | Her poison Pokémon need tending. |
| 4 | home | Her invisible-wall Gym (HGSS). |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Safari Zone gate (`FuchsiaCity_SafariZoneEntrance_hns`) | named | sightsee | One hop from her Gym. |

### Sabrina

- **Home base:** `SaffronCity_hns` (her Gym city). Aloof.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | Her teleport-pad Gym. |
| 2 | study | Psychic research since childhood (anime). |
| 3 | train | Honing her powers alone. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Bill's Sea Cottage (`Route25_BillsHouse_hns`) | named | study | Bill's teleporter interests a teleporting psychic; off a route, so not public. |

### Blaine

- **Home base:** `CinnabarIsland_Frlg` (his Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | His quiz Gym on Cinnabar. |
| 2 | study | The researcher behind the Pokémon Mansion's notes (anime, FRLG). |
| 3 | relax | After the volcano, his Gym moves to the Seafoam Islands (HGSS). |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Cinnabar Lab (`CinnabarIsland_PokemonLab_ExperimentRoom_Frlg`) | named | study | The fossil lab on his island. |
| 2 | Seafoam Islands B3F (`SeafoamIslands_B3F_Frlg`) | water's edge | relax | His HGSS refuge, beyond Cinnabar's few hops. |

### Giovanni

- **Home base:** `ViridianCity_hns` (his Gym city). Traveller.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | gamble | Team Rocket's front is the Celadon Game Corner (FRLG). |
| 2 | lie low | Hiding after Team Rocket falls (HGSS). |
| 3 | home | The Viridian Gym, his public face. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Celadon Game Corner (`CeladonCity_GameCorner_hns`) | Game Corner | gamble | His hideout's front, 10 hops from Viridian. |
| 2 | Burned Tower (`BurnedTower_1F_hns`) | named | lie low | The only lie-low spot; Team Rocket's remnants call for him from Johto (HGSS). |

### Blue

- **Home base:** `PalletTown_hns` (his hometown). Traveller.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | Always a step ahead of the player. |
| 2 | visit | He becomes the Viridian Gym Leader (HGSS). |
| 3 | study | His grandfather's lab. |
| 4 | home | Pallet Town and his sister Daisy. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Viridian Gym (`ViridianCity_Gym_Frlg`) | Gym | visit | The Gym he takes over in HGSS; it is Giovanni's here, so a visit. |
| 2 | Oak's Lab (`PalletTown_Lab_hns`) | named | study | Grandpa Oak's lab, one hop from home. |

### Lorelei

- **Home base:** `FourIsland_Frlg` (her home in FRLG).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | Her house full of dolls on Four Island (FRLG). |
| 2 | relax | The ice cave where she saves the Lapras (FRLG). |
| 3 | care | Looking after the island's Pokémon. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Icefall Cave (`FourIsland_IcefallCave_Entrance_Frlg`) | water's edge | relax | Her Lapras rescue (FRLG), one hop from home. |
| 2 | Four Island Day Care (`FourIsland_PokemonDayCare_Frlg`) | named | care | Next door on her island. |

### Bruno

- **Home base:** `OneIsland_Frlg` (where he appears in FRLG). Traveller.
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | Endless training, in the mountains if he can (anime, FRLG). |
| 2 | relax | Soaking sore muscles. |
| 3 | care | Tending Pokémon after a hard session. |
| 4 | home | One Island's town. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Mt. Ember (`MtEmber_Exterior_Frlg`) | tall grass | train | He trains on Mt. Ember's slopes in FRLG (verify). |
| 2 | Ember Spa (`OneIsland_KindleRoad_EmberSpa_Frlg`) | named | relax | One Island's hot spring, two hops from home. |

### Agatha

- **Home base:** `LavenderTown_hns` (the ghost town). Aloof.
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | Lavender, by the Pokémon Tower. |
| 2 | train | A veteran who still keeps her Ghosts sharp. |
| 3 | relax | A quiet walk by the water east of town. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Route 10 (`Route10_hns`) | water's edge | relax | The river by the Power Plant road, away from people. |

The Pokémon Tower floors give only NPC chats, a public kind, so they can't
be her favourite.

### Koga

- **Home base:** `FuchsiaCity_hns` (his old Gym city).
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | Ninja discipline never stops. |
| 2 | visit | His old Gym, now his daughter Janine's (HGSS). |
| 3 | sightsee | The Safari Zone at Fuchsia's edge. |
| 4 | home | Fuchsia City. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Fuchsia Gym (`FuchsiaCity_Gym_hns`) | Gym | visit | Checking on Janine; it is her Gym, not his. |
| 2 | Safari Zone gate (`FuchsiaCity_SafariZoneEntrance_hns`) | named | sightsee | One hop from his home map. |

### Lance

- **Home base:** `BlackthornCity_hns` (his dragon-clan home). Traveller,
  aloof.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | The Dragon master, training for the clan. |
| 2 | relax | The Dragon's Den, his clan's sacred place (HGSS). |
| 3 | home | Blackthorn City. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Dragon's Den (`DragonsDen_Cavern_hns`) | water's edge | relax | The clan's den, a remote cave. |

The Lake of Rage, where he chases the red Gyarados (HGSS, anime), is on a
city-typed map, so it is public and closed to him.

## Johto

### Falkner

- **Home base:** `VioletCity_hns` (his Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | His Gym, high above Violet. |
| 2 | train | The Sprout Tower, Violet's training tower. |
| 3 | sightsee | The Ruins of Alph, south of Violet. |
| 4 | study | Bird lore at Earl's Academy. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Sprout Tower (`SproutTower_3F_hns`) | named | train | Violet's tower of training, near Elder Li. |
| 2 | Ruins of Alph (`RuinsOfAlph_Outside_hns`) | named | sightsee | Two hops from his city. |

### Bugsy

- **Home base:** `AzaleaTown_hns` (his Gym city). Traveller.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | study | The Bug-type researcher (HGSS). |
| 2 | train | Catching and raising bugs. |
| 3 | relax | The Bug-Catching Contest at the National Park (HGSS). |
| 4 | home | His Gym in Azalea. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Ruins research center (`RuinsOfAlph_Lab_hns`) | named | study | A researcher among researchers. |
| 2 | National Park (`NationalPark_Normal_hns`) | named | relax | The contest's lawn. |

### Whitney

- **Home base:** `GoldenrodCity_hns` (her Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | shop | A Goldenrod girl at the department store. |
| 2 | visit | The Radio Tower, Goldenrod's landmark. |
| 3 | relax | A day out in the National Park. |
| 4 | home | Her Gym and her Miltank. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Goldenrod Dept. Store 2F (`GoldenrodCity_DepartmentStore_2F_hns`) | store | shop | The store next to her Gym. |
| 2 | Radio Tower (`GoldenrodCity_RadioTower_1F_hns`) | named | visit | The lobby, one hop from home. |
| 3 | National Park (`NationalPark_Normal_hns`) | named | relax | Three hops north. |

### Morty

- **Home base:** `EcruteakCity_hns` (his Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | His Gym of ghosts. |
| 2 | sightsee | Waiting for Ho-Oh at the Bell Tower (HGSS, anime). |
| 3 | train | Training his Ghosts to see the legend. |
| 4 | relax | The Kimono Girls at the Dance Theater. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Bell Tower (`TinTower_1F_hns`) | named | sightsee | Ho-Oh's tower, two hops away. |
| 2 | Dance Theater (`EcruteakCity_Theater_hns`) | named | relax | Ecruteak's theatre. |

### Chuck

- **Home base:** `CianwoodCity_hns` (his Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | Training under the waterfall with Poliwrath (HGSS). |
| 2 | care | A strong man with a soft spot for his Pokémon. |
| 3 | home | His Gym, with his wife keeping him in line (anime). |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Route 40 battle courtyard (`TrainerHill_Courtyard_hns`) | named | train | The battle courtyard across the sea. |
| 2 | Cianwood pharmacy (`CianwoodShop_hns`) | named | care | Medicine, one hop from his Gym. |

### Jasmine

- **Home base:** `OlivineCity_hns` (her Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | sightsee | The lighthouse, where Amphy shines (HGSS). |
| 2 | care | Fetching medicine for the sick Amphy (HGSS). |
| 3 | home | Her Steel Gym. |
| 4 | relax | A quiet table among sailors. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Olivine Lighthouse (`OlivineCity_Lighthouse_hns`) | named | sightsee | The top room, near Amphy. |
| 2 | Cianwood pharmacy (`CianwoodShop_hns`) | named | care | The Secret Potion for Amphy (HGSS), 4 hops away. |
| 3 | Olivine Café (`OlivineCity_Cafe_hns`) | named | relax | One hop from home. |

### Pryce

- **Home base:** `Mahoganytown_hns` (his Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | Fifty years with Ice Pokémon. |
| 2 | sightsee | The Lake of Rage, north of Mahogany. |
| 3 | relax | The frozen Ice Path. |
| 4 | home | His Ice Gym. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Lake of Rage (`LakeOfRage_hns`) | named | sightsee | The south shore, two hops away. |
| 2 | Ice Path (`IcePath_1F_hns`) | water's edge | relax | The ice cave between Mahogany and Blackthorn. |

### Clair

- **Home base:** `BlackthornCity_hns` (her Gym city). Aloof.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | Her Dragon Gym. |
| 2 | train | Proud of her dragons; trains alone. |
| 3 | relax | The Dragon's Den behind her Gym (HGSS). |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Dragon's Den (`DragonsDen_Cavern_hns`) | water's edge | relax | The clan's den, shared with her cousin Lance. |

### Will

- **Home base:** `IndigoPlateau_hns` (his post at the League). Traveller.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | study | A Psychic master, curious about the Unown. |
| 2 | train | He trained all over the world to join the Elite Four (HGSS). |
| 3 | relax | A pause by the water before the League. |
| 4 | home | The Indigo Plateau. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Ruins of Alph (`RuinsOfAlph_Outside_hns`) | named | study | Psychic mysteries of the Unown. |
| 2 | Battle Tower (`BattleFrontier_BattleTowerLobby`) | named | train | "Trained around the world": Hoenn's Battle Tower. |

### Karen

- **Home base:** `IndigoPlateau_hns` (her post at the League). Aloof.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | "Strong Pokémon, weak Pokémon": training her favourites. |
| 2 | relax | Somewhere dark and quiet. |
| 3 | home | The Indigo Plateau. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Route 28 (`Route28_hns`) | tall grass | train | The wild road to Mt. Silver. |
| 2 | Victory Road (`VictoryRoadKanto_B1F_hns`) | water's edge | relax | The cave below the Plateau. |

## Hoenn

### Roxanne

- **Home base:** `RustboroCity` (her Gym city).
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | study | The Trainer's School graduate who still studies (RSE, anime). |
| 2 | train | Rock training on Route 116. |
| 3 | home | Her Gym. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Rustboro Trainer's School (`RustboroCity_PokemonSchool`) | named | study | Her old school, one hop away. |

### Brawly

- **Home base:** `DewfordTown` (his Gym city). Traveller.
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | Big waves and big muscles. |
| 2 | relax | A surfer at the beach. |
| 3 | fish | Dewford is a fishing town. |
| 4 | home | His dark Gym. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Seashore House (`Route109_SeashoreHouse`) | named | relax | The beach house on Slateport's sand. |
| 2 | Dewford shore (`DewfordTown`) | water's edge | fish | Dewford's own shore. |

### Wattson

- **Home base:** `MauvilleCity` (his Gym city).
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | His Gym in the city he built. |
| 2 | gamble | A jolly old man at the Mauville Game Corner. |
| 3 | sightsee | The Cycling Road he had built. |
| 4 | shop | Rydel's bike shop. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Mauville Game Corner (`MauvilleCity_GameCorner`) | Game Corner | gamble | One hop from his Gym. |
| 2 | Seaside Cycling Road (`Route110`) | named | sightsee | At the north gate. |
| 3 | Mauville Bike Shop (`MauvilleCity_BikeShop`) | named | shop | Talking bikes with Rydel. |

### Flannery

- **Home base:** `LavaridgeTown` (her Gym city).
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | relax | Lavaridge's hot springs. |
| 2 | train | New to the job and trying hard (anime). |
| 3 | home | Her Gym, inherited from her grandfather. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Lavaridge hot spring (`LavaridgeTown`) | named | relax | Beside the two old ladies. |

### Norman

- **Home base:** `PetalburgCity` (his Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | The strict Gym Leader and the player's father. |
| 2 | home | His Gym. |
| 3 | study | Visiting his friend Professor Birch (RSE). |
| 4 | relax | Petalburg's pond. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Birch's Lab (`LittlerootTown_ProfessorBirchsLab`) | named | study | Littleroot, where his family lives. |

### Winona

- **Home base:** `FortreeCity` (her Gym city).
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | Her Gym among the treetops. |
| 2 | train | Flying with her bird Pokémon. |
| 3 | study | Reading the weather, as a flier must. |
| 4 | sightsee | The Safari Zone beyond Route 121. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Weather Institute (`Route119_WeatherInstitute_1F`) | named | study | Two hops from Fortree. |

### Juan

- **Home base:** `SootopolisCity` (his Gym city).

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | The Gym he took back from Wallace. |
| 2 | relax | The crater lake. |
| 3 | sightsee | An artist admiring his city. |
| 4 | visit | Wallace's old mentor, fond of fans and contests. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Sootopolis (`SootopolisCity`) | named | sightsee | The crater lake's east shore. |
| 2 | Lilycove Trainer Fan Club (`LilycoveCity_PokemonTrainerFanClub`) | named | visit | Where fans talk trainers, by the Contest Hall. |

Sootopolis has no walking link to the rest of Hoenn in the graph, so
everything beyond the city comes from favourites.

### Sidney

- **Home base:** `EverGrandeCity` (his post at the League).
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | Ever Grande City. |
| 2 | gamble | A cocky rebel who loves a gamble. |
| 3 | visit | Chatting up other trainers. |
| 4 | relax | The shore below the League. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Mauville Game Corner (`MauvilleCity_GameCorner`) | Game Corner | gamble | Hoenn's only Game Corner. |

### Phoebe

- **Home base:** `MtPyre_Summit` (where her grandmother keeps the orbs).
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | home | Mt. Pyre's summit. |
| 2 | sightsee | The shrine, with her grandmother (RSE). |
| 3 | train | She trained on Mt. Pyre to bond with Ghosts (RSE). |
| 4 | relax | The water beyond the mountain. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Mt. Pyre summit (`MtPyre_Summit`) | named | sightsee | The orbs' shrine. |
| 2 | Mt. Pyre summit (`MtPyre_Summit`) | tall grass | train | The grass round the summit. |

### Glacia

- **Home base:** `EverGrandeCity` (her post at the League). Traveller,
  aloof.
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | train | She came from afar to train in Hoenn's warm climate (RSE). |
| 2 | relax | The cold she left behind. |
| 3 | home | Ever Grande City. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Icefall Cave (`FourIsland_IcefallCave_Entrance_Frlg`) | water's edge | relax | The coldest cave a traveller can reach. |

### Drake

- **Home base:** `LilycoveCity` (a sailor by the sea). Traveller.
- **Not simulated in v0 (sprite limitation).** Authored for later.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | sightsee | An old sailor watching the ships. |
| 2 | fish | Life on the sea. |
| 3 | train | The Dragon master's discipline. |
| 4 | home | Lilycove. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Lilycove harbour (`LilycoveCity_Harbor`) | named | sightsee | The dock, one hop away. |
| 2 | Lilycove shore (`LilycoveCity`) | water's edge | fish | Lilycove's own shore. |

### Wallace

- **Home base:** `SootopolisCity` (his home city). Traveller, aloof.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | relax | The crater lake of his city. |
| 2 | fish | The Feebas he raised into Milotic (RSE). |
| 3 | home | Sootopolis. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Sootopolis lake (`SootopolisCity`) | water's edge | relax | The lake shore; the Sootopolis named spot is on a city map, so public. |
| 2 | Route 119 (`Route119`) | water's edge | fish | Where Feebas lives, the road he points the player to. |

### Steven

- **Home base:** `MossdeepCity` (his house). Traveller, aloof.

| Step | Activity | Why |
| ---: | --- | --- |
| 1 | study | Rare stones and ancient places. |
| 2 | relax | The caves he explores (RSE). |
| 3 | sightsee | Ruins far from Hoenn. |
| 4 | home | His house in Mossdeep. |

| # | Favourite | Kind | Serves | Why |
| ---: | --- | --- | --- | --- |
| 1 | Ruins of Alph (`RuinsOfAlph_Outside_hns`) | named | study | Ancient ruins in Johto. |
| 2 | Meteor Falls (`MeteorFalls_1F_1R`) | water's edge | relax | His cave at Meteor Falls (Emerald). |
| 3 | Tanoby Ruins (`SevenIsland_TanobyRuins_Frlg`) | named | sightsee | The Sevii chambers, far from anyone. |

---

## Check

`python3 .product/research/notable-trainer-routines.py` reads this file,
the catalog, the [named spots](notable-named-spots.md), and the spots
inventory's `per-map.csv`, and checks every routine:

- every simulated trainer has a home base in scope, 3-4 known activities,
  and at most 3 favourites;
- every favourite resolves to a named spot, or a detected spot of its kind
  on its map, in Wayfarer's scope; its activity is one that spot offers
  and a step of the cycle; it isn't public for an aloof trainer, or
  outside the home region for a non-traveller, or the trainer's own Gym;
- every cycle step has a candidate: a favourite, or a named or detected
  spot of a matching kind within the placeholder radius (3 hops, 8 for a
  traveller) after the aloof filter. A home step always has the leader's
  Gym, or for anyone else a spot on the home map.

**Hops** are approximated on a map graph built from the inventory's scope:
maps linked by map connections and by warps whose destination warps back
(HNS keeps a placeholder warp from Fuchsia City to New Bark Town that
nothing pairs with). It counts map hops, not walker-graph nodes, applies no
walking filters (a Surf-only connection counts), and has no transit edges,
so Sevii's islands and Sootopolis reach only what their own maps link to.

**Result:** 37 routines (25 simulated in v0, 12 kept for later), 134
steps and 67 favourites. Every step has a candidate, and every favourite
passes. Favourites shared by v0 trainers, each a second choice when taken:

- the Celadon Game Corner (an interior, capacity 1): Lt. Surge and
  Giovanni;
- the Dragon's Den cavern (outdoor, 3): Lance and Clair;
- Oak's Lab (1): Brock and Blue;
- the National Park (2): Bugsy and Whitney;
- the Cianwood pharmacy (1): Chuck and Jasmine;
- the Ruins of Alph (2): Falkner, Will, and Steven.

Once the face-only trainers walk, Wattson and Sidney share the Mauville
Game Corner, Lorelei and Glacia share Icefall Cave, and Janine and Koga
share the Safari Zone gate in Fuchsia (1).
