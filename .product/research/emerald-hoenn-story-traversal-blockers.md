# Emerald Hoenn story traversal blockers

## Scope

This inventory covers Emerald Hoenn maps without the `_hns` or `_Frlg` suffix.
A blocker is included when it closes a route, city, dungeon, ferry, or the only
practical path forward until story state changes. Forced story encounters are
listed separately from persistent NPC, object, metatile, and HM gates.

The evidence comes from `game/data/maps` scripts and map event data. Shared
field-move behavior is treated as a combined gate only where the campaign route
requires the move.

## Blockers in progression order

### Route 101 Birch rescue

- **Blocks:** Leaving the opening Route 101 rescue area and starting normal
  world traversal.
- **Mechanism:** Exit triggers turn the player back while Birch is under attack.
- **Prerequisite:** Choose a starter from Birch's bag and finish the rescue
  battle.
- **Clears:** The sequence sets `FLAG_SYS_POKEMON_GET`,
  `FLAG_RESCUED_BIRCH`, and advances `VAR_ROUTE101_STATE` before warping to the
  lab.
- **Status:** Mandatory tutorial gate.
- **Evidence:** `game/data/maps/Route101/scripts.inc:19-62,218-250`.

### Route 103 rival battle and Birch's Lab handoff

- **Blocks:** Completion of the opening loop and release of Oldale's west exit
  toward Route 102.
- **Mechanism:** The Route 103 rival encounter is mandatory and has no decline
  branch. Winning alone does not release Oldale; it sends the player back to
  Birch's Lab for the Pokedex handoff.
- **Clears:** The battle sets `FLAG_DEFEATED_RIVAL_ROUTE103` and advances
  `VAR_BIRCH_LAB_STATE` to 4. The Lab sequence then sets
  `FLAG_ADVENTURE_STARTED`, which is the flag checked by Oldale's roadblock.
- **Status:** Mandatory forced encounter and story handoff.
- **Evidence:** `game/data/maps/Route103/scripts.inc:20-149`,
  `game/data/maps/LittlerootTown_ProfessorBirchsLab/scripts.inc:506-524`,
  `game/data/maps/OldaleTown/scripts.inc:5-21,190-215`.

### Oldale west exit to Route 102

- **Blocks:** Oldale Town to Route 102 and Petalburg City.
- **Mechanism:** The footprints man is moved to `(1,11)` and blocks the west
  entrance while `FLAG_ADVENTURE_STARTED` is unset. His trigger steps the
  player back.
- **Prerequisite:** Defeat the rival on Route 103, return to Birch's lab, and
  complete the Pokedex and first-adventure handoff that sets
  `FLAG_ADVENTURE_STARTED`.
- **Status:** Mandatory and not bypassable during the opening loop.
- **Evidence:** `game/data/maps/OldaleTown/scripts.inc:5-21,190-215`,
  `game/data/maps/OldaleTown/map.json:16-31,61-73`.

### Petalburg westward progression

- **Blocks:** The first westward crossing of Petalburg toward Route 104.
- **Mechanism:** At city state 0, the Gym Boy is moved to the west approach.
  Four coordinate triggers span the crossing and redirect the player to
  Petalburg Gym.
- **Prerequisite:** Enter the Gym, speak with Norman, and complete Wally's
  catching tutorial. The sequence advances the city and Gym state so the
  redirection no longer repeats.
- **Status:** Mandatory opening story redirect on the city path.
- **Evidence:** `game/data/maps/PetalburgCity/scripts.inc:6-16,27-56,237-290`,
  `game/data/maps/PetalburgCity/map.json:122-134,195-230`.

### Petalburg Woods Aqua encounter

- **Blocks:** The northbound woods path toward northern Route 104 and Rustboro.
- **Mechanism:** The Devon employee and Aqua Grunt scene occupies the narrow
  progression path and starts an unavoidable battle.
- **Clears:** Defeat the grunt. The scene removes its actors and advances
  `VAR_PETALBURG_WOODS_STATE` to 1.
- **Status:** Mandatory forced story encounter rather than a persistent border
  object.
- **Evidence:** `game/data/maps/PetalburgWoods/scripts.inc:39-72,109-112`.

### Devon Goods, Peeko, and Briney's ferry

- **Blocks:** The intended early connection from Route 104 to Dewford and then
  Slateport.
- **Mechanism:** After Roxanne, the Devon theft sequence moves the Aqua Grunt
  and Peeko into Rusturf Tunnel. Briney's boat service is unavailable until the
  rescue chain is completed.
- **Prerequisite:** Trigger the theft, pursue the grunt through Route 116,
  defeat the grunt in Rusturf Tunnel, recover the Devon Goods, and rescue Peeko.
- **Status:** Mandatory for the intended Dewford and Slateport transport chain.
  The Rusturf rock wall to Verdanturf is a separate Rock Smash shortcut, not
  this story gate.
- **Evidence:** `game/data/maps/RustboroCity/scripts.inc:284-312`,
  `game/data/maps/RusturfTunnel/scripts.inc:11-18`,
  `game/data/maps/DewfordTown/scripts.inc:113-180`.

### Slateport to Route 110

- **Blocks:** Slateport City to Route 110 and Mauville.
- **Mechanism:** Five physical Team Aqua objects form an obstruction across the
  northbound Route 110 path. All use `FLAG_HIDE_ROUTE_110_TEAM_AQUA`; their
  placement supplies the collision barrier rather than a scripted turn-back.
- **Prerequisite:** Find Captain Stern in the Oceanic Museum, defeat the Aqua
  grunts, and deliver the Devon Goods.
- **Clears:** The museum sequence sets
  `FLAG_HIDE_ROUTE_110_TEAM_AQUA` and `FLAG_DELIVERED_DEVON_GOODS`.
- **Status:** Mandatory and not bypassable on the first northbound trip.
- **Evidence:** `game/data/maps/Route110/map.json:308-371`,
  `game/data/maps/SlateportCity_OceanicMuseum_2F/scripts.inc:73-84`.

### Route 110 rival battle

- **Blocks:** The lower and upper Route 110 corridor on the way to Mauville.
- **Mechanism:** One of three coordinate triggers starts the rival encounter on
  the narrow land route.
- **Clears:** Win the battle; the rival changes to the bicycle object, exits,
  and is removed.
- **Status:** Mandatory forced encounter on the normal path.
- **Evidence:** `game/data/maps/Route110/scripts.inc:356-477`.

### Route 112 cable car

- **Blocks:** The cable-car approach to Mt. Chimney and the Jagged Pass route to
  Lavaridge.
- **Mechanism:** Two Magma grunts physically hold the approach and refuse
  passage while their Meteor Falls team is active.
- **Prerequisite:** Reach Meteor Falls and trigger the Magma and Aqua scene.
  That scene sets `FLAG_HIDE_ROUTE_112_TEAM_MAGMA`.
- **Status:** Mandatory for the Lavaridge branch at this stage and not bypassable
  by the Fiery Path route alone.
- **Evidence:** `game/data/maps/Route112/scripts.inc:10-45`,
  `game/data/maps/MeteorFalls_1F_1R/scripts.inc:16-90`,
  `game/data/maps/MtChimney/scripts.inc:34-74`.

### Norman and the Route 118 Surf crossing

- **Blocks:** Mauville and western Hoenn to eastern Route 118, Route 119,
  Fortree, and the eastern campaign.
- **Mechanism:** Route 118 requires Surf. Norman refuses the Petalburg Gym battle
  until four earlier badges advance `VAR_PETALBURG_GYM_STATE` to 6. His Balance
  Badge authorizes field Surf under the current rules, and Wally's family gives
  HM Surf after the battle.
- **Prerequisite:** Earn four badges, defeat Norman, receive HM Surf, and bring a
  compatible party member.
- **Status:** Mandatory combined story, Gym, HM, party, and terrain gate.
- **Evidence:** `game/data/maps/PetalburgCity_Gym/scripts.inc:102-113,294-314,371-390`,
  `game/data/maps/PetalburgCity_WallysHouse/scripts.inc:17-33`.

### Weather Institute occupation

- **Blocks:** Upper Route 119 and Fortree City.
- **Mechanism:** Team Aqua's occupation controls the Route 119 blocking objects
  and the Institute sequence.
- **Prerequisite:** Clear the Institute and defeat Shelly.
- **Clears:** The scene advances `VAR_WEATHER_INSTITUTE_STATE` to 1 and hides
  the Route 119 Aqua blockers.
- **Status:** Mandatory for the direct northbound route.
- **Evidence:** `game/data/maps/Route119_WeatherInstitute_2F/scripts.inc:41-80`,
  `game/data/maps/Route119/map.json:327-340`.

### Route 119 rival battle

- **Blocks:** The route immediately beyond the Weather Institute toward
  Fortree.
- **Mechanism:** Coordinate triggers start a forced rival encounter on the
  progression path.
- **Clears:** Win the battle; the rival leaves by bicycle and the route resumes.
- **Status:** Mandatory one-time encounter, not a persistent flag-closed border.
- **Evidence:** `game/data/maps/Route119/scripts.inc:29-144`.

### Fortree Gym Kecleon

- **Blocks:** Fortree Gym and therefore Badge 6, which is required later by the
  League sequence.
- **Mechanism:** An invisible Kecleon occupies the Gym approach.
- **Prerequisite:** Meet Steven at the Route 120 bridge, resolve that Kecleon
  interaction, and receive the Devon Scope.
- **Clears:** Use the Scope on the Fortree Kecleon, setting
  `FLAG_KECLEON_FLED_FORTREE`.
- **Status:** An interior access gate rather than a city exit. It is included
  because the badge is part of the hard League path.
- **Evidence:** `game/data/maps/FortreeCity/scripts.inc:55-84`,
  `game/data/maps/Route120/scripts.inc:34-50,154-236`.

### Lilycove east sea outlet

- **Blocks:** Route 124, Mossdeep, and the southeast ocean network.
- **Mechanism:** Wailmer metatiles close the east water outlet while Team Aqua
  remains in Lilycove.
- **Prerequisite chain:** Complete the Mt. Pyre summit scene, obtain the Magma
  Emblem, clear Magma Hideout and Maxie, witness the submarine theft at
  Slateport, then clear the Aqua Hideout sequence through Matt.
- **Clears:** Aqua Hideout sets `FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE` and hides
  the Lilycove Aqua group. Lilycove no longer installs the blocking Wailmer
  layout.
- **Status:** Mandatory main-line sea gate.
- **Evidence:** `game/data/maps/LilycoveCity/scripts.inc:14-31`,
  `game/data/maps/MtPyre_Summit/scripts.inc:32-63`,
  `game/data/maps/JaggedPass/scripts.inc:7-63`,
  `game/data/maps/MagmaHideout_4F/scripts.inc:4-74`,
  `game/data/maps/SlateportCity_Harbor/scripts.inc:49-78`,
  `game/data/maps/AquaHideout_B2F/scripts.inc:25-50`.

### Dive and Seafloor Cavern

- **Blocks:** Route 128's underwater passage, Seafloor Cavern, and the
  Sootopolis crisis chain.
- **Mechanism:** Dive is not available until the Mossdeep Space Center story is
  cleared. Field use also requires the Mind Badge under current rules.
- **Prerequisite:** Defeat the Mossdeep Gym, complete the Space Center double
  battle with Steven, visit Steven's house for HM Dive, and bring a compatible
  party member.
- **Clears:** Steven's handoff sets the relevant state and removes the Aqua
  guard. The Seafloor Cavern outcome then starts Sootopolis's crisis state.
- **Status:** Mandatory combined story, badge, HM, party, and terrain gate.
- **Evidence:** `game/data/maps/MossdeepCity_SpaceCenter_2F/scripts.inc:248-304`,
  `game/data/maps/MossdeepCity_StevensHouse/scripts.inc:24-51`,
  `game/data/maps/SeafloorCavern_Room9/scripts.inc:118-143`.

### Sootopolis crisis and Sky Pillar

- **Blocks:** Sootopolis Gym, the Rain Badge, HM Waterfall, and final League
  progression.
- **Mechanism:** During the crisis, staged NPCs and city state control Cave of
  Origin and the Gym. Steven must guide the player; Wallace then sends the
  player to Sky Pillar. Rayquaza's scripted awakening advances the city state
  and resolves the crisis on return.
- **Prerequisite:** Complete Seafloor Cavern, follow Steven into Cave of Origin,
  speak to Wallace, reach Sky Pillar, and awaken Rayquaza.
- **Status:** Mandatory story chain. Rayquaza is not captured in this sequence.
- **Evidence:** `game/data/maps/SootopolisCity/scripts.inc:459-573,667-690,879-925,1282-1325`,
  `game/data/maps/SkyPillar_Outside/scripts.inc:29-87`,
  `game/data/maps/SkyPillar_Top/scripts.inc:88-130`.

### Ever Grande waterfall and League entry

- **Blocks:** Upper Ever Grande, Victory Road, and the Elite Four.
- **Mechanism:** Waterfall is required to ascend Ever Grande. The League guard
  then performs its badge check before opening the Elite Four path.
- **Prerequisite:** Resolve the Sootopolis crisis, defeat Wallace, obtain the
  Rain Badge and HM Waterfall, and satisfy the field-move party requirement.
- **Clears:** League authorization sets `FLAG_ENTERED_ELITE_FOUR`.
- **Status:** Mandatory combined story, Gym, HM, party, and terrain gate.
- **Evidence:** `game/data/maps/SootopolisCity/scripts.inc:1282-1316`,
  `game/data/maps/EverGrandeCity_PokemonLeague_1F/scripts.inc:46-88`.

### Victory Road Wally

- **Blocks:** The Victory Road progression path toward Ever Grande and the
  League.
- **Mechanism:** Entrance triggers spawn Wally and start a forced battle.
- **Clears:** Winning sets `FLAG_DEFEATED_WALLY_VICTORY_ROAD`; Wally remains as
  a nonblocking post-battle object until later state changes.
- **Status:** Mandatory forced encounter. Strength boulders in Victory Road are
  additional HM terrain and should be validated against the exact collision
  path before every boulder is called mandatory.
- **Evidence:** `game/data/maps/VictoryRoad_1F/scripts.inc:6-80`.

### Battle Frontier ferry

- **Blocks:** First access to the postgame Battle Frontier.
- **Mechanism:** The Lilycove ferry attendant checks `FLAG_SYS_GAME_CLEAR`.
  Before the Champion clear, the Frontier destination is unavailable; after
  the clear, the destination menu includes the Battle Frontier warp.
- **Status:** Mandatory for the new postgame area, not for the main campaign.
- **Evidence:** `game/data/maps/LilycoveCity_Harbor/scripts.inc:9-50,91-98`,
  `game/data/maps/SSTidalCorridor/scripts.inc:253-271`.

## Apparent blockers that are optional or local

- Rusturf Tunnel's breakable wall opens an early Verdanturf shortcut. Hoenn's
  main route reaches Verdanturf from Mauville, so the wall is not a campaign
  city gate.
- Flash in Granite Cave, the desert Go-Goggles check, bicycle slopes, and most
  Cut trees open side areas, item branches, or shortcuts.
- Lilycove can be entered from Route 121 while Team Aqua is present. The story
  block is the east sea outlet, not the city's west entrance.
- Rival scenes on Route 104 and at Lilycove were not found to leave a world
  connection closed after their immediate event.
- Gym doors are excluded unless their reward directly authorizes a later
  mandatory route, as with Norman, Mossdeep, and Sootopolis.
