# HNS Kanto story traversal blockers

## Scope

This inventory covers the HNS Kanto maps and the shared HNS scripts they call.
FRLG-only maps are excluded. A blocker is included when it closes a connection,
transport link, or new world area until story state changes. Kanto Gyms are
listed only when their completion unlocks one of those connections.

The HNS Kanto overworld is open compared with Johto and Emerald Hoenn. Most of
its story progression changes Gym availability without closing the surrounding
route network.

## Blockers

### First Kanto arrival on the S.S. Aqua

- **Blocks:** Olivine Port to Vermilion City and the intended first Kanto
  arrival.
- **Mechanism:** The Olivine sailor requires `ITEM_SS_TICKET`. The maiden voyage
  sets `VAR_SSAQUA_STATE = 1`; the ship's exit sailor refuses disembarkation
  until the onboard sequence reaches state 5.
- **Prerequisite:** Complete the first Pokemon League run, take Elm's call, and
  collect the S.S. Ticket from his lab.
- **Onboard clearance:** Meet the grandfather, defeat the sleeping sailor who
  blocks the crew corridor, find the granddaughter in the captain's room, and
  trigger the reunion. Those steps advance states 2 through 5 and start the
  Vermilion arrival scene.
- **Status:** Mandatory for the intended ferry transition. Route 26 and Route 27
  form a land-side regional boundary, but they do not replace the scripted
  first-arrival initialization performed by the ship.
- **Evidence:** `game/data/maps/PokemonLeague_HallOfFame_hns/scripts.inc:57-67`,
  `game/data/maps/NewBarkTown_hns/scripts.inc:7-19`,
  `game/data/maps/NewBarkTown_Lab_hns/scripts.inc:67-75,479-487`,
  `game/data/maps/OlivineCity_PortInside_hns/scripts.inc:107-118,191-203`,
  `game/data/maps/SSAqua_1F_hns/scripts.inc:5-38,168-192`,
  `game/data/maps/SSAqua_RoomNW_hns/scripts.inc:14-27`,
  `game/data/maps/SSAqua_CaptainsRoom_hns/scripts.inc:13-33`,
  `game/data/maps/SSAqua_RoomSSE_hns/scripts.inc:19-41`.

### Power Plant theft, radio upgrade, and Vermilion Snorlax

- **Blocks:** The Snorlax connection on Vermilion's east side and the linked
  Kanto-side branch through Reception Gate. It is a local connection gate, not
  a lock on every Kanto city.
- **Mechanism:** Snorlax occupies `(61,13)` and is surrounded by four invisible
  collision objects. It wakes only when the Poke Flute radio condition passes.
- **Prerequisite chain:**
  1. Speak to the Power Plant manager to start
     `VAR_KANTO_ROCKET_STORY_STATE`.
  2. Trigger the Cerulean Gym Rocket scene and chase the grunt to Route 24.
  3. Defeat the grunt, recover `ITEM_MACHINE_PART` from Cerulean Gym, and return
     it to the manager. This sets `FLAG_RETURNED_MACHINE_PART`.
  4. Speak to the Lavender Radio director to set `FLAG_KANTO_RADIO_GOT`.
  5. Tune the Poke Flute station at Snorlax and win or catch the encounter.
- **Clears:** Winning or catching removes Snorlax and the collision objects and
  sets `FLAG_INDIGOJUNCTION_HIDE_KANTO_GUARD`. Running leaves the block intact.
- **Status:** Mandatory for this connection and for the newly enabled Reception
  Gate branch. Other Kanto routes can bypass it for general city access.
- **Evidence:** `game/data/maps/Route10_PowerPlantBackRoom_hns/scripts.inc:5-48`,
  `game/data/maps/Route10_PowerPlantEntrance_hns/scripts.inc:67-87`,
  `game/data/maps/CeruleanCity_Gym_hns/scripts.inc:5-74`,
  `game/data/maps/Route24_hns/scripts.inc:47-96`,
  `game/data/maps/LavenderTown_RadioStation_hns/scripts.inc:16-35`,
  `game/data/maps/VermilionCity_hns/map.json:32-45,126-175`,
  `game/data/maps/VermilionCity_hns/scripts.inc:179-215`,
  `game/data/maps/ReceptionGate_hns/map.json:44-56,95-106`.

### Vermilion return ferry

- **Blocks:** Vermilion Port to Olivine and the scripted return to Johto.
- **Mechanism:** The Vermilion sailor refuses to sail while
  `FLAG_RETURNED_MACHINE_PART` is unset, even if the player has the S.S. Ticket.
- **Prerequisite:** Complete the Power Plant theft chain through returning the
  Machine Part.
- **Status:** Mandatory for the ferry link. It may be bypassed as regional
  transport by Fly, the land route, or the Magnet Train once those are
  available, but those alternatives do not open the ferry itself.
- **Evidence:** `game/data/maps/VermilionCity_PortInside_hns/scripts.inc:19-63,169-172`.

### Magnet Train

- **Blocks:** The optional Saffron to Goldenrod transport shortcut.
- **Mechanism:** Saffron Station requires both `FLAG_RETURNED_MACHINE_PART` and
  `ITEM_PASS`.
- **Prerequisite:** Restore the Power Plant and finish Copycat's lost doll side
  quest to receive the Pass.
- **Status:** Optional transport gate. The ferry and field routes provide other
  regional connections.
- **Implementation note:** Goldenrod Station checks the Pass before its power
  flag check and can therefore behave asymmetrically if the player somehow has
  the Pass early.
- **Evidence:** `game/data/maps/SaffronCity_TrainStation_hns/scripts.inc:23-45`,
  `game/data/maps/SaffronCity_CopyCatsHouse_2F_hns/scripts.inc:28-36,150-155,220-251`,
  `game/data/maps/GoldenrodCity_TrainStation_hns/scripts.inc:9-24`.

### Route 19 Kingler closure

- **Blocks:** Fuchsia City to southern Route 19, Route 20, and the Seafoam
  Islands coastal connection.
- **Mechanism:** A cluster of Kingler objects occupies the route choke point.
  Nearby NPC dialogue also identifies the beach and road as closed.
- **Prerequisite:** Defeat Blaine in the Seafoam Islands Gym. The battle sets
  `FLAG_DEFEATED_CINNABAR_ISLAND_GYM`, which is the visibility flag assigned to
  every blocking Kingler.
- **Status:** Mandatory for this coastal connection, but bypassable for reaching
  Cinnabar itself because Route 21 connects Pallet Town to Cinnabar.
- **Evidence:** `game/data/maps/Route19_hns/scripts.inc:30-37,57-64,116-148`,
  `game/data/maps/Route19_hns/map.json:107-208`,
  `game/data/maps/SeafoamIslands_Gym_hns/scripts.inc:9-28`,
  `game/data/maps/Route21_hns/map.json:15-25`.

### Route 28 and Mt. Silver authorization

- **Blocks:** Reception Gate's west branch to Route 28 and Mt. Silver.
- **Mechanism:** A Black Belt occupies the branch leading to the Route 28 warp.
  Its hide flag is `FLAG_INDIGOJUNCTION_HIDE_SILVER_GUARD`.
- **Prerequisite:** Reach `VAR_NUM_BADGES >= 16` and then speak to Oak in Pallet
  Lab. Badge count alone does not move the guard.
- **Clears:** Oak sets the guard's hide flag and persists the lab state. Blue's
  Viridian Gym victory supplies Badge 16 and triggers Oak's call.
- **Status:** Mandatory for Mt. Silver, which is an optional late-game area.
- **Evidence:** `game/data/maps/ReceptionGate_hns/map.json:31-43,95-100`,
  `game/data/maps/PalletTown_Lab_hns/scripts.inc:4-16,23-43,87-114`,
  `game/data/maps/ViridianCity_Gym_hns/scripts.inc:4-29`,
  `game/data/maps/ViridianCity_hns/scripts.inc:10-31`.

### Cerulean Cave

- **Blocks:** Cerulean Cave, an optional postgame dungeon.
- **Mechanism:** The cave guard remains until the Kanto League clear sets
  `FLAG_HIDE_CERULEANCAVE_GUARD`.
- **Status:** A real story-gated world area, but not part of city-to-city Kanto
  progression.
- **Evidence:** `game/data/maps/PokemonLeague_HallOfFame_hns/scripts.inc:69-88`,
  `game/data/maps/CeruleanCity_hns/scripts.inc:22-36`.

## Johto-side prerequisite

Before the first League clear and the S.S. Aqua transition, Reception Gate
requires both `FLAG_BADGE08_GET` and `VAR_ECRUTEAK_CITY_THEATER >= 8`. That gate
is documented in the Johto inventory because it closes the Johto-to-League
route rather than a Kanto-internal connection.

Evidence: `game/data/maps/ReceptionGate_hns/scripts.inc:31-60`.

## Apparent blockers that do not close Kanto routes

- The four Saffron gatehouses have dialogue but no flag or item checks that deny
  passage.
- The Power Plant Rocket and Misty chain delays the Cerulean Gym battle, but the
  city and Routes 24 and 25 remain traversable.
- Blue remains away from Viridian Gym until the player has 15 badges. This
  delays Badge 16 and Mt. Silver authorization, not entry to Viridian or its
  surrounding routes.
- Kanto Gym rematches, dojo leaders, Safari content, and legendary encounters
  do not occupy connective paths in the reviewed scripts.
- Ordinary Cut, Rock Smash, Strength, Surf, and Waterfall terrain was not
  classified as Kanto story state unless it combined with one of the checks
  above.
