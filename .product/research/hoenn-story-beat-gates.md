# Hoenn story beat gates

Audited `main` at `e00a436914` on 2026-11-06 for the
[Hoenn independent story beats](../prds/hoenn-independent-story-beats.md)
design. Scope: Emerald's Hoenn maps in the Wayfarer build, which are the maps
under `game/data/maps/` without a `_hns` or `_Frlg` suffix.

Method: the scripts, `map.json` events, and the C code they call were read. One
finding was also run: the rival freeze on Routes 110 and 119, reproduced on the
E2E ROM. Everything else is from reading. Line numbers marked `~` are
approximate. Paths are relative to `game/data/` unless they start with `src/`
or `include/`.

This document records what the game does today. The
[specification](../specs/hoenn-independent-story-beats.md) says what it should
do.

## Verdict

Most of Hoenn's local adventures already start on their own state and need no
badge. The real ordering problems are few:

- Roxanne's badge is the only thing that starts the Rustboro theft.
- Norman needs four Hoenn badges, and Wattson's errand needs Norman.
- Stern and Steven accept a delivery from a player who holds no item.
- The Mossdeep Gym win starts the Space Center invasion and also removes the
  whole Aqua Hideout cast, so winning that Gym first strands the Aqua branch.
- Seafloor Cavern opens on Steven's Dive scene alone.
- Mt. Pyre's theft is live from the first visit, and Mt. Chimney's scenes
  cannot be told after it.
- Terra Cave and Marine Cave open on a Kanto or Johto Hall of Fame.
- A visitor with no Hoenn starter freezes in five rival scenes.

## Rustboro theft and Peeko

`VAR_RUSTBORO_CITY_STATE` carries the theft, the president's errand, the
PokéNav tutorial, and the rival:

| Value | Set by | Arms |
| --- | --- | --- |
| 0 | Default. | Nothing. |
| 1 | Roxanne's defeat, `maps/RustboroCity_Gym/scripts.inc:27`, unconditionally. | Five triggers at x=23, y=20–24 (`maps/RustboroCity/map.json` ~338–380) that run `RustboroCity_EventScript_StolenGoodsScene` (`maps/RustboroCity/scripts.inc` ~270–312). |
| 2 | The street scene, `maps/RustboroCity/scripts.inc:304`. It also sets `FLAG_DEVON_GOODS_STOLEN` and shows Briney, Peeko, and the tunnel grunt. | The employee's request triggers around (30,9–12). |
| 3 | The employee's request, `:504`. | Nothing reads it. |
| 4 | The Rusturf Tunnel grunt fight, `maps/RusturfTunnel/scripts.inc:337`. The grunt hands over `ITEM_DEVON_PARTS` (`~:301–306`); `FLAG_RECOVERED_DEVON_GOODS` is set. | The return triggers. |
| 5 | Returning the Goods, `maps/RustboroCity/scripts.inc:602`. Great Ball, `FLAG_RETURNED_DEVON_GOODS`, warp to Devon Corp 3F. | — |
| 6 | Mr. Stone, `maps/RustboroCity_DevonCorp_3F/scripts.inc:25–77`. He gives `ITEM_LETTER` and the PokéNav and shows Briney at his house (`:65`). | Read at `maps/RustboroCity/scripts.inc:9,11,28` and `maps/RustboroCity_DevonCorp_2F/scripts.inc:261`. |
| 7 | The PokéNav tutorial, `maps/RustboroCity/scripts.inc:99`. | The rival triggers on y=53. |
| 8 | The rival scene (`:841,909`), the Route 104 rival fallback (`maps/Route104/scripts.inc:147`), and Briney's sail (`maps/Route104_MrBrineysHouse/scripts.inc:78`). | — |

- The visitor baseline hides the employee, the grunt, Peeko, Briney, and the
  museum grunts (`scripts/wayfarer_hoenn_entry.inc:21–32`, `~105–112`). It
  never writes `VAR_RUSTBORO_CITY_STATE`, so a player who never beats Roxanne
  never sees the theft.
- Nothing reads state 0. Deleting the Gym's write leaves no reader of state 1.
  The two writers that jump to 8 would disarm a theft keyed on state 0.
- The employee's and grunt's street lines do not mention Roxanne or a badge.
  `RustboroCity_Text_HelpMeIWasRobbed` (`maps/RustboroCity/scripts.inc` ~1100)
  recalls help "in PETALBURG WOODS". The Woods flags were not traced.
- Rusturf Tunnel uses `VAR_RUSTURF_TUNNEL_STATE`, and Route 116 uses
  `VAR_ROUTE116_STATE` and the stolen and recovered flags. Neither reads the
  Rustboro state.

## Deliveries, Stern, and Briney

- Steven (`maps/GraniteCave_StevensRoom/scripts.inc:4–14`) sets
  `FLAG_DELIVERED_STEVEN_LETTER`, gives TM Steel Wing, and registers himself.
  Stern (`maps/SlateportCity_OceanicMuseum_2F/scripts.inc:4–86`) runs the grunt
  and Archie confrontation, takes the Parts, sets `FLAG_DELIVERED_DEVON_GOODS`
  and `VAR_SLATEPORT_OUTSIDE_MUSEUM_STATE` to 1, and clears the Route 110 Aqua
  barrier.
- **Neither checks for the item.** `ITEM_DEVON_PARTS` and `ITEM_LETTER` appear
  only where they are given and handed over.
  `Common_EventScript_PlayerHandedOverTheItem` (`event_scripts.s:1565`) plays
  the handover and runs `removeitem`, which does nothing for a missing item.
  Both men are visible from the start, and the museum lets a player in without
  the fee (`maps/SlateportCity_OceanicMuseum_1F/scripts.inc:46–59`). A player
  can complete either delivery and take its rewards with no item.
- Dock (`maps/SlateportCity_SternsShipyard_1F/scripts.inc:4–20`) sends the
  player to Stern and sets `FLAG_DOCK_REJECTED_DEVON_GOODS` without checking
  the item.
- Stern is three objects with separate hide flags: museum 2F (visible from the
  start, removed at delivery), the Slateport street (shown by
  `maps/MagmaHideout_4F/scripts.inc:60`), and the harbor (shown by
  `maps/SlateportCity/scripts.inc:592`). Nothing hides the museum Stern when
  the harbor scene runs, so a late delivery stays possible and the museum
  confrontation replays after the submarine theft. The harbor line about "the
  same thugs" at the museum (`maps/SlateportCity_Harbor/scripts.inc:337`, text
  ~488) and Stern's "now we can prepare for the expedition"
  (`maps/SlateportCity_OceanicMuseum_2F/scripts.inc:271–277`) then read wrong.
- Briney sails from his Route 104 house once he is shown there
  (`maps/Route104_MrBrineysHouse/scripts.inc:27–28`). From Dewford he offers
  Slateport only after the Letter is delivered (`maps/DewfordTown/scripts.inc:46`).
  On Route 109 he offers only Dewford until the Goods are delivered
  (`maps/Route109/scripts.inc:298`). By land or ferry the deliveries already
  work in either order.
- Norman's win calls `EventScript_HideMrBriney`
  (`maps/PetalburgCity_Gym/scripts.inc` ~585, defined at `event_scripts.s:1436`),
  which hides Briney everywhere.
- The Route 110 and Route 119 Aqua travel barriers are already open in
  Wayfarer (`scripts/wayfarer_hoenn_entry.inc:35,89`; the
  [traversal specification](../specs/emerald-open-world-region-traversal.md)).

## Meteor Falls and Mt. Chimney

- Meteor Falls starts from one trigger at (14,18) while
  `VAR_METEOR_FALLS_STATE` is 0 (`maps/MeteorFalls_1F_1R/map.json:204`), with
  no badge or flag. The scene sets `FLAG_MET_ARCHIE_METEOR_FALLS`, shows the
  teams on Mt. Chimney, and sets the state to 1
  (`maps/MeteorFalls_1F_1R/scripts.inc:86–91`).
- Mt. Chimney shows the teams only after that flag
  (`maps/MtChimney/scripts.inc:11–23`). Maxie's defeat (`:84–87`) sets
  `FLAG_DEFEATED_EVIL_TEAM_MT_CHIMNEY`, which also drives Lavaridge, Fallarbor,
  Cozmo's house, the meteorite machine (`:467`), and two match calls
  (`src/pokenav_match_call_data.c:380`, `src/match_call.c:1456`). The cable
  car and Jagged Pass do not read it.
- The scenes assume Groudon still sleeps and Maxie holds no orb: the machine
  line (`maps/MtChimney/scripts.inc:559–566`), Maxie's intro (`:568–594`) and
  exit (`:600–607`), the grunts and Tabitha (`:609–678`), and Archie
  (`:683–710`). The later scenes contradict them: Magma Hideout
  (`maps/MagmaHideout_4F/scripts.inc:212–224`, "there is no longer any need for
  this blasted volcano"), the Space Center
  (`maps/MossdeepCity_SpaceCenter_2F/scripts.inc:435–444,463–482`), and
  Sootopolis (`maps/SootopolisCity/scripts.inc:1588–1616`).
- At Meteor Falls, Archie recalls the museum
  (`maps/MeteorFalls_1F_1R/scripts.inc:263–276`).
- The Route 112 Magma grunts are hidden only by the Meteor Falls scene
  (`:86`). Whether they still block the cable car was not checked.

## Weather Institute

- The occupation is present while `VAR_WEATHER_INSTITUTE_STATE` is 0
  (`maps/Route119_WeatherInstitute_1F/scripts.inc:5–7`, `2F:5–7`), with no
  badge or story check, and nothing hides it later. Beating Shelly sets the
  state to 1 (`maps/Route119_WeatherInstitute_2F/scripts.inc:50–90`).
- A grunt and Shelly announce Mt. Pyre (`2F:271–282`). Their other lines hold
  in any order.
- Castform falls back to the PC, and a full party and box leaves it claimable
  from the scientist (`2F:112–128,204–209`).

## Steven and Kecleon, and the desert survey

- Route 120 gates only on `FLAG_RECEIVED_DEVON_SCOPE`
  (`maps/Route120/scripts.inc:28,55`). Steven's greeting and reward line switch
  on `FLAG_DELIVERED_STEVEN_LETTER` (`:159,259`), so the Letter is not a
  prerequisite. Full-Bag and loss paths restore Kecleon (`:176–182,262–270`).
- The desert survey is in `maps/Route111/scripts.inc:167–246`, with no badge
  or story gate. It and the Lavaridge handoff
  (`maps/LavaridgeTown/scripts.inc:22–32,82–131`) both check the Go-Goggles
  item and `FLAG_RECEIVED_GO_GOGGLES`, so the reward is given once.

## Wattson and New Mauville

- Wattson is two objects: the Gym's (`maps/MauvilleCity_Gym/map.json:19–29`)
  and the street's (`maps/MauvilleCity/map.json:132–143`), hidden at the start
  (`scripts/new_game.inc:276`, `scripts/wayfarer_hoenn_entry.inc:123`).
- `MauvilleCity_Gym_EventScript_TryRelocateWattson`
  (`maps/MauvilleCity_Gym/scripts.inc:101–110`) needs both
  `FLAG_DEFEATED_PETALBURG_GYM` and `FLAG_BADGE03_GET`. It hides the Gym
  Wattson and shows the street one. Both Gyms call it
  (`maps/MauvilleCity_Gym/scripts.inc:97–98`,
  `maps/PetalburgCity_Gym/scripts.inc:570–571`).
- The street Wattson gives the Basement Key and, once
  `VAR_NEW_MAUVILLE_STATE` is 2, TM Thunderbolt
  (`maps/MauvilleCity/scripts.inc:413–444`). New Mauville's door checks only
  the key (`maps/NewMauville_Entrance/scripts.inc:30–41`), and the generator
  button sets state 2 (`maps/NewMauville_Inside/scripts.inc:130–148`).
- `MauvilleCity_OnTransition` moves him back to the Gym after the TM and makes
  his rematch available (`maps/MauvilleCity/scripts.inc:12`).
- Wattson has no world-simulation routine; he is a fixed, face-only notable.

## Norman and Wally

- Talking to Norman while `VAR_PETALBURG_GYM_STATE` is under 2 starts Wally's
  catching tutorial (`maps/PetalburgCity_Gym/scripts.inc:127,147–269`,
  `maps/PetalburgCity/scripts.inc:28–56`). The west-entrance triggers redirect
  to the Gym until it is done.
- After it, `WayfarerGetHoennBadgeCountForScript` sends zero to three badges
  to advice lines and four or more to the battle (`:130–136`, `:528`).
- Lines that count badges: `NormanIntro` and `VisitorNormanIntro` ("So, you
  did get four GYM BADGES", `:1490`, `:1885`; the visitor one adds "as I
  promised"), and the advice lines.
- Visitor and native variants are chosen by `WayfarerUsesNativeHoennHousehold`
  (`src/wayfarer_origin.c:292`), in the Gym only.
- Wally in Mauville is a talk NPC, visible from the start, with a re-askable
  decline (`maps/MauvilleCity/scripts.inc:67–110,154–170`). His lines recall
  meeting the player. Verdanturf's Wally appears after the Mauville win
  (`maps/VerdanturfTown_WandasHouse/scripts.inc:4–35`). Victory Road's
  triggers fire at `VAR_VICTORY_ROAD_1F_STATE` 0 and recall "losing to you
  that time" without checking the Mauville win
  (`maps/VictoryRoad_1F/scripts.inc:20–47,121–131`).

## Brendan and May

`VAR_HOENN_STARTER_CHOICE` stays `HOENN_STARTER_CHOICE_NONE` until Birch's lab
commits a choice (`maps/LittlerootTown_ProfessorBirchsLab/scripts.inc:126–141`).

| Chapter | Starts on | Needs |
| --- | --- | --- |
| Route 103 (`maps/Route103/scripts.inc:20–28`) | Talk. Guarded at `:22`. | The starter. |
| Oldale (`maps/OldaleTown/scripts.inc:212–274`) | `VAR_OLDALE_RIVAL_STATE` 1. | The Route 103 win. |
| Rustboro (`maps/RustboroCity/scripts.inc:673–892`) | Eight triggers on y=53 at Rustboro state 7. Optional battle. | The starter and the PokéNav. |
| Route 104 fallback (`maps/Route104/scripts.inc:137–166`) | `VAR_ROUTE104_STATE` 1. | The starter. |
| Route 110 (`maps/Route110/scripts.inc:356–480`) | Triggers at (33–34,56) while `VAR_ROUTE110_STATE` is 0. Forced. Gives the Itemfinder. | The starter. |
| Route 119 (`maps/Route119/scripts.inc:34–150`) | Trigger at (25,31) while `VAR_ROUTE119_STATE` is 0. Forced; the rival is added by script. Gives HM Fly. | The starter. |
| Lavaridge (`maps/LavaridgeTown/scripts.inc:61–141`) | `VAR_LAVARIDGE_TOWN_STATE` 1, set by Flannery's defeat. Gives the Go-Goggles. | Nothing. |
| Lilycove (`maps/LilycoveCity/scripts.inc:209–371`) | Talk; visible from the start. | The starter. |

Route 110, Route 119, Rustboro, Route 104, and Lilycove each end in a
`switch VAR_HOENN_STARTER_CHOICE` with no default case. On the E2E ROM, a
visitor with no starter who steps on the Route 110 or Route 119 trigger sees
the rival's challenge line and then cannot walk: the text box stays up, the
script has ended, and the trigger stays armed. The Mauville Game Corner doll
girl has the same hole.

## The central villain story

| Step | Starts on today | Sets |
| --- | --- | --- |
| Mt. Pyre theft | Triggers at (22–24,7) while `VAR_MT_PYRE_STATE` is 0 (`maps/MtPyre_Summit/map.json` ~150–175). Archie and the grunts are visible from the start. No other gate. | State 1, the Magma Emblem, `FLAG_RECEIVED_RED_OR_BLUE_ORB`, the Jagged Pass guard hidden (`maps/MtPyre_Summit/scripts.inc` ~24–62). |
| Magma Hideout | The Emblem in the Bag opens the Jagged Pass wall (`maps/JaggedPass/scripts.inc:9–13`). On 4F, talk to Maxie. | `FLAG_GROUDON_AWAKENED_MAGMA_HIDEOUT`, Slateport city and harbor states 1, the street Stern shown (`maps/MagmaHideout_4F/scripts.inc:60–65`). |
| Submarine theft | The street Stern (`maps/SlateportCity/scripts.inc:545–598`), then the harbor triggers at harbor state 1 (`maps/SlateportCity_Harbor/scripts.inc:51–94`). | `FLAG_MET_TEAM_AQUA_HARBOR`; the two Aqua Hideout entrance guards hidden (`:75–76`). |
| Aqua Hideout | Open after the harbor scene. Matt's battle (`maps/AquaHideout_B2F/scripts.inc` ~22–52). | `FLAG_TEAM_AQUA_ESCAPED_IN_SUBMARINE`. |
| Space Center | Tate & Liza's defeat alone (`maps/MossdeepCity_Gym/scripts.inc:72–91`): city and Space Center states 1 and the Magma objects shown. | On completion: `FLAG_DEFEATED_MAGMA_SPACE_CENTER`, `VAR_STEVENS_HOUSE_STATE` 1 (`maps/MossdeepCity_SpaceCenter_2F/scripts.inc:270–299`). |
| Steven's Dive | Steven's house at that state (`maps/MossdeepCity_StevensHouse/scripts.inc:35,56–108`). | HM Dive and Hoenn's Dive authorization; `FLAG_HIDE_SEAFLOOR_CAVERN_ENTRANCE_AQUA_GRUNT`. |
| Seafloor Cavern | An Aqua grunt at (10,2) stands in front of the cavern door until that flag is set (`maps/SeafloorCavern_Entrance/scripts.inc:12–35`). Room 9's trigger fires at `VAR_SEAFLOOR_CAVERN_STATE` 0. | The weather crisis (`maps/SeafloorCavern_Room9/scripts.inc:118–140`). |
| Sootopolis and Sky Pillar | Sootopolis states 1 to 5, the Cave of Origin, the Sky Pillar awakening, and the return (`maps/SootopolisCity/scripts.inc:166–171,460–570`). | `VAR_SKY_PILLAR_STATE` 2 or 3; the weather cleared. |
| Juan | The Gym door opens once Archie and Maxie leave (`FLAG_SOOTOPOLIS_ARCHIE_MAXIE_LEAVE`, `:10`). | The Rain Badge. |

- The Mossdeep Gym win also sets `FLAG_HIDE_AQUA_HIDEOUT_GRUNTS` (`~:79`),
  which hides every Aqua Hideout grunt and Matt. Winning that Gym before
  Matt's battle makes the Aqua branch impossible to finish.
- The Space Center checks neither hideout. It can run before Groudon wakes,
  where Maxie's motive ("With GROUDON gone…") is untrue.
- Nothing at Seafloor Cavern checks the Aqua Hideout. Whether the entrance
  grunt physically blocks the door was inferred from his position.
- `FLAG_DEFEATED_MAGMA_SPACE_CENTER` is cleared by a later rival call
  (`maps/MossdeepCity_SpaceCenter_2F/scripts.inc:330`), so it cannot mark the
  Space Center as done.
- The harbor Stern's dialogue checks `FLAG_BADGE07_GET` before the escape flag
  (`maps/SlateportCity_Harbor/scripts.inc:335`).
- Juan's script has a branch on `FLAG_BADGE06_GET`
  (`maps/SootopolisCity_Gym_1F/scripts.inc:87`). What it selects was not read.
- Whether Steven's `AuthorizeDive` label sets the persistence-layer Dive
  authorization was not confirmed.

## Legendary captures

- Rayquaza waits at the top of Sky Pillar when `VAR_SKY_PILLAR_STATE` is at
  least 2 and `FLAG_DEFEATED_RAYQUAZA` is unset
  (`maps/SkyPillar_Top/scripts.inc:17–35,44–65`). There is no Hall of Fame
  check.
- Terra Cave and Marine Cave open only through abnormal weather, and the only
  activator is the Weather Institute scientist
  (`maps/Route119_WeatherInstitute_2F/scripts.inc:132–165`), who requires
  `FLAG_SYS_GAME_CLEAR`. The 1F hint does too (`1F:36`).
- In Wayfarer, `SetGameClearStateForRegion` sets `FLAG_SYS_GAME_CLEAR` for
  Johto and Kanto and not for Hoenn (`src/wayfarer_persistence.c:888–917`).
  Three more setters in `src/post_battle_event_funcs.c` were not traced.
- The TR 55 gate exists as `WAYFARER_LEGENDARY_TR`
  (`include/wayfarer_sevii_story.h:12–16`), compared with
  `GetTrainerRating()`. Zapdos and Articuno use eligibility specials
  (`src/wayfarer_persistence.c:758–770`, `specials.inc:302–303`), and the
  under-threshold line is "ZAPDOS watches you warily. You need more experience
  before you can challenge it." (`scripts/wayfarer_power_plant.inc:42–43,108–111`).

## Scott

`VAR_SCOTT_STATE` is raised with `addvar` at twelve sites and read only by
`maps/BattleFrontier_ScottsHouse/scripts.inc:153–155` for a Battle Points
reward. Petalburg uses `setvar VAR_SCOTT_STATE, 1`
(`maps/PetalburgCity/scripts.inc:543`), which resets the count if that meeting
comes after others.

## Not covered

- The Petalburg Woods flags behind the employee's recognition line.
- Magma Hideout below 4F, Seafloor Cavern rooms 1 to 8, the Cave of Origin
  interiors, and the Route 128 scene's lines.
- The Lilycove rival's battle texts and the PokéNav match-call data beyond the
  cited lines.
- Team Aqua and Team Magma grunt lines outside the scenes named above.
- Field-move needs on each adventure's approach.
