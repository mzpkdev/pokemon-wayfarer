# Johto story traversal blockers

## Scope

This inventory covers the HNS Johto implementation. A blocker is included when
the player cannot cross a map connection, reach the next city or route, or use
the only practical path forward until story state changes. Forced encounters
and HM terrain are included when they sit on that path. Optional interiors,
item branches, and Gym access are excluded unless the resulting badge or story
state is required by a later world connection.

The evidence comes from `game/data/maps/*_hns`, the shared HNS field-move
scripts, and `game/src/field_move.c`. Coordinates and visibility flags were
checked in each map's `map.json`; prerequisites and state changes were traced
through `scripts.inc`.

## Blockers in progression order

### New Bark Town west exit

- **Blocks:** New Bark Town to Route 29.
- **Mechanism:** Three exit triggers at the west connection step the player
  back while `VAR_NEWBARK_TOWN_STATE` is 2 or 4.
- **Prerequisite:** State 2 clears when Elm assigns the Mr. Pokemon errand and
  advances the town to state 3, not merely when the starter is received. The
  later state 4 lock clears only after the player returns the Mystery Egg,
  completes the police and Elm sequence, and speaks to Mom. Mom sets
  `FLAG_MOM_VISITED` and advances the town to state 5.
- **Status:** Mandatory and not bypassable. The same exit is deliberately
  locked twice during the opening story.
- **Evidence:** `game/data/maps/NewBarkTown_hns/map.json:206-260`,
  `game/data/maps/NewBarkTown_hns/scripts.inc:25-38,102-131`,
  `game/data/maps/NewBarkTown_Lab_hns/scripts.inc:371-429`,
  `game/data/maps/NewBarkTown_PlayersHouse_1F_hns/scripts.inc:40-45`.

### Possible Route 30 staging obstruction

- **Blocks:** Cherrygrove and southern Route 30 to Route 31 and Violet City.
- **Mechanism:** Joey and the two battling Pokemon occupy consecutive tiles on
  the northbound branch. Their objects remain until Mom is visited, while Mikey
  is a separate normal trainer object.
- **Prerequisite:** Complete the Mr. Pokemon errand, return the Mystery Egg to
  Elm, and speak to Mom. Setting `FLAG_MOM_VISITED` hides the battling group;
  the normal trainer version of Joey is then shown separately.
- **Status:** Unverified as a hard blocker. There is no coordinate turn-back or
  explicit story check on Route 30, and the map connections are unconditional.
  Collision or runtime validation is needed to prove that the staged objects
  close every northbound lane. Until then, treat this as a likely physical
  obstruction rather than a confirmed mandatory gate.
- **Evidence:** `game/data/maps/Route30_hns/map.json:41-52,94-145`,
  `game/data/maps/NewBarkTown_PlayersHouse_1F_hns/scripts.inc:40-44`,
  `game/data/maps/Route30_MrPokemonsHouse_hns/scripts.inc:6-66`.

### Route 32 guard south of Violet

- **Blocks:** Violet City to southern Route 32, Union Cave, Route 33, and
  Azalea Town.
- **Mechanism:** A guard and three coordinate triggers turn the player back.
- **Prerequisite:** The script requires the Sprout Tower rival/event flag
  `FLAG_HIDE_SPROUT_TOWER_SILVER`, Violet Gym completion through
  `FLAG_DEFEATED_VIOLET_GYM`, and receipt of the Togepi Egg through
  `FLAG_RECEIVED_TOGEPI_EGG`.
- **Clears:** When all three checks pass, the guard gives the Miracle Seed and
  advances the local progression state to 5.
- **Status:** Mandatory and not bypassable.
- **Evidence:** `game/data/maps/Route32_hns/map.json:553-634`,
  `game/data/maps/Route32_hns/scripts.inc:263-296`,
  `game/data/maps/VioletCity_hns/scripts.inc:16-47`.

### Azalea to Ilex Forest and Route 34

- **Blocks:** Azalea Town to Ilex Forest, Route 34, and Goldenrod City.
- **Mechanism:** This is a chain of world and story locks:
  1. Bugsy sets `FLAG_BADGE02_GET` and `VAR_AZALEA_TOWN_STATE = 5`.
  2. Crossing toward the Ilex gate at state 5 forces the Silver battle, which
     advances the state to 6.
  3. The Farfetch'd sequence awards HM Cut.
  4. A Cut tree at `(32,40)` occupies the forest choke point. Direct Cut use
     currently requires the Hive Badge and a party member with Cut.
- **Status:** Mandatory. The rival fight and Cut tree sit on the forward path;
  there is no land bypass to Route 34.
- **Evidence:** `game/data/maps/AzaleaTown_Gym_hns/scripts.inc:525-548`,
  `game/data/maps/AzaleaTown_hns/map.json:376-394`,
  `game/data/maps/AzaleaTown_hns/scripts.inc:152-217`,
  `game/data/maps/IlexForest_hns/scripts.inc:159-205`,
  `game/data/maps/IlexForest_hns/map.json:65-74`,
  `game/data/scripts/field_move_scripts_hns.inc:1-27`.

### Route 36 Sudowoodo junction

- **Blocks:** The junction between Route 35 and National Park, Violet and the
  Ruins of Alph, and Route 37 and Ecruteak.
- **Mechanism:** Sudowoodo stands at `(39,19)` on the junction tile.
- **Prerequisite:** Interaction requires `ITEM_SQUIRT_BOTTLE`. The Goldenrod
  Flower Shop owner gives the item only after `FLAG_BADGE03_GET`, the Plain
  Badge.
- **Clears:** Catching or defeating Sudowoodo sets its hide flag. Running leaves
  the object and road block in place.
- **Status:** Mandatory for this land junction and not bypassable in the current
  map. This is the blocker targeted by Phase 2 of the traversal PRD.
- **Evidence:** `game/data/maps/Route36_hns/map.json:105-116`,
  `game/data/maps/Route36_hns/scripts.inc:88-137`,
  `game/data/maps/GoldenrodCity_FlowerShop_hns/scripts.inc:53-72`.

### Route 40 and Route 41 sea crossing

- **Blocks:** Olivine and Route 40 to Cianwood City.
- **Mechanism:** The only normal connection is open water across Routes 40 and
  41.
- **Prerequisite:** The player needs HM Surf, a compatible party member with
  Surf, and `FLAG_BADGE04_GET` from Morty under the current field-move rules.
  The Ecruteak Theater Surf NPC supplies the HM. If the theater is in state 1,
  the handoff is withheld until the player defeats the Theater Rocket and
  advances the theater state.
- **Status:** Mandatory for Cianwood and for the Chuck, medicine, Jasmine branch
  that later feeds the eight-badge requirement. This is a combined HM, party,
  and badge gate rather than an NPC roadblock.
- **Evidence:** `game/data/maps/EcruteakCity_Theater_hns/scripts.inc:256-295`,
  `game/data/maps/EcruteakCity_Gym_hns/scripts.inc:40-48`,
  `game/data/maps/Route40_hns/map.json:15-25`,
  `game/data/maps/Route41_hns/map.json:15-30`,
  `game/data/maps/CianwoodCity_hns/map.json:15-25`,
  `game/src/field_move.c:40-53`.

### Mahogany east road

- **Blocks:** Mahogany Town to Route 44, Ice Path, and Blackthorn City.
- **Mechanism:** The RageCandyBar merchant occupies `(30,11)`. Coordinate
  triggers cover the road immediately below him and push the player west after
  every purchase, refusal, bag-full result, or insufficient-money result.
- **Prerequisite:** Buying the item does not open the road. The merchant stops
  enforcing the block only at `VAR_MAHOGANY_TOWN_STATE = 17`, which is set
  after the Goldenrod Radio Tower Archer and Director sequence is completed.
- **Status:** Payment never clears it; the apparent shop interaction masks a
  story-state road gate. The trigger series covers states 1 through 16 and is
  absent at state 17. A rendered collision or runtime check should still
  confirm that no terrain lane bypasses the merchant.
- **Evidence:** `game/data/maps/Mahoganytown_hns/map.json:59-71,189-621`,
  `game/data/maps/Mahoganytown_hns/scripts.inc:92-187`,
  `game/data/maps/GoldenrodCity_RadioTower_5F_hns/scripts.inc:236-252`.

### Ice Path Kimono Girl

- **Blocks:** The Ice Path corridor from Route 44 to Blackthorn.
- **Mechanism:** A Kimono Girl occupies the progression corridor and pushes the
  player back when approached.
- **Prerequisite:** There is no earlier flag, badge, item, or battle check. The
  player must accept the prompt to push her free. Refusing loops the block.
- **Clears:** Acceptance sets `FLAG_HIDE_ICE_PATH_KIMONO` and
  `VAR_ICE_PATH_STATE = 1`.
- **Status:** Mandatory one-time scripted interaction, but not a dependency on
  earlier story progress.
- **Evidence:** `game/data/maps/IcePath_1F_hns/map.json:122-134`,
  `game/data/maps/IcePath_1F_hns/scripts.inc:20-67`.

### Blackthorn Gym and Dragon's Den

- **Blocks:** Badge 8, which is required by the League path.
- **Mechanism:** A boy is placed at the Gym entrance while any of Badges 5, 6,
  or 7 is missing. Defeating Clair is not enough: the Dragon's Den quiz awards
  `FLAG_BADGE08_GET` afterward.
- **Status:** This is an interior progression gate, not a city or route
  connection. It is included because Badge 8 is directly checked at Reception
  Gate and by Waterfall field use.
- **Evidence:** `game/data/maps/BlackthornCity_hns/scripts.inc:8-16,80-101`,
  `game/data/maps/DragonsDen_Shrine_hns/scripts.inc:95-175,225-281`.

### Ecruteak legendary storyline

- **Blocks:** Reception Gate and therefore Victory Road and the Pokemon League.
- **Mechanism:** Reception Gate requires `VAR_ECRUTEAK_CITY_THEATER >= 8` in
  addition to Badge 8. The theater reaches state 6 after the Kimono sequence,
  then sends the player down the selected Lugia or Ho-Oh branch. Winning or
  catching the selected legendary sets state 8. Running or teleporting does
  not.
- **Branch requirements:** The Lugia branch also needs Whirlpool access through
  Route 41. HM Whirlpool comes from the Mahogany Rocket Hideout electrode
  sequence, and field use currently requires Badge 7 and a capable party
  member. The Ho-Oh branch uses the Tin Tower and wing path instead.
- **Status:** Mandatory in this project even though the legendary encounter may
  appear optional to players familiar with other versions.
- **Evidence:** `game/data/maps/EcruteakCity_Theater_hns/scripts.inc:683-700,821-876`,
  `game/data/maps/WhirlIslands_LugiaChamber_hns/scripts.inc:172-200`,
  `game/data/maps/TinTower_RoofDay_hns/scripts.inc:169-197`,
  `game/data/maps/RocketHideout_B2F_hns/scripts.inc:478-528`,
  `game/data/scripts/field_move_scripts_hns.inc:467-537`.

### Route 27 and Tohjo Falls approach

- **Blocks:** New Bark Town and western Route 27 to eastern Route 27, Route 26,
  and Reception Gate.
- **Mechanism:** Route 27 contains both Tohjo Falls entrances, but terrain
  separates the two exterior sections. The cavern joins those entrances through
  an upper waterway reached by one waterfall and left by another.
- **Prerequisite:** The player needs Surf and Waterfall, compatible party
  members, and the badges currently checked for those moves. Waterfall requires
  Badge 8, which is already part of the Reception Gate progression chain.
- **Status:** Mandatory for the overland approach to Reception Gate. The cave is
  not an optional side branch, even though both of its exterior warps return to
  the same Route 27 map.
- **Evidence:** `game/data/maps/Route27_hns/map.json:290-304`,
  `game/data/maps/TohjoFalls_Cavern_hns/map.json:96-109`,
  `game/data/scripts/field_move_scripts_hns.inc:386-405`,
  `game/src/field_move.c:49-64,75-82`.

### Reception Gate

- **Blocks:** Route 26 and Route 27 to Victory Road and Indigo Plateau.
- **Mechanism:** The Reception Gate officer turns the player back unless both
  late-story checks pass.
- **Prerequisite:** `FLAG_BADGE08_GET` and
  `VAR_ECRUTEAK_CITY_THEATER >= 8`.
- **Clears:** Reception Gate advances `VAR_ROUTE27_STATE` to 2 once authorized.
- **Status:** Mandatory and not bypassable on the first League trip.
- **Evidence:** `game/data/maps/ReceptionGate_hns/scripts.inc:31-60`.

### Victory Road Silver battle

- **Blocks:** Victory Road 1F to the later floors and Indigo Plateau.
- **Mechanism:** Silver starts a forced rival battle on the progression path.
- **Clears:** Winning advances `VAR_ROUTE27_STATE` to 3 and removes the
  encounter as a blocker.
- **Status:** Confirmed state-gated encounter. Trigger tiles `(27-29,7)` are
  active at Route 27 state 2, but collision validation is still needed to prove
  that every onward lane crosses them. Strength and Rock Smash objects also
  need individual geometry validation before they are called critical-path
  gates.
- **Evidence:** `game/data/maps/VictoryRoadKanto_1F_hns/map.json:260-288`,
  `game/data/maps/VictoryRoadKanto_1F_hns/scripts.inc:14-39`.

## Apparent blockers that do not close a world connection

- Slowpoke Well removes Team Rocket and advances Azalea state, but the source
  does not require that state before fighting Bugsy or entering Ilex Forest.
  Treating the Well as part of the enforced traversal chain would overstate the
  implementation.
- Olivine has a state-1 Silver cutscene on three entrance tiles at `(15,28-30)`.
  It resolves immediately and advances the city to state 2 without a battle or
  earlier prerequisite. It is not classified as a progression lock unless
  collision review proves the trigger lane is unavoidable and the project
  chooses to count immediate cutscenes as blockers.
- Olivine's lighthouse medicine story closes Jasmine's Gym, not a city or route
  exit. It still matters indirectly because Blackthorn and Reception Gate need
  the resulting badge count.
- The Mahogany Rocket Hideout and Goldenrod Radio Tower are story dungeons. The
  overworld effect relevant here is the merchant road block and the legendary
  prerequisites, not the ability to enter Mahogany itself.
- Most Cut, Rock Smash, Strength, Surf, Whirlpool, and Waterfall objects lead to
  items or shortcuts. Their shared badge checks do not prove that every object
  is a mandatory route choke point.
- Route 42's Suicune scene is a forced cutscene, but it does not leave the route
  closed after the scene starts.
