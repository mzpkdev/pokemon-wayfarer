# Kanto and Johto inter-region travel

## Scope

This document maps the player-facing ways to travel between Johto and Kanto in
the HNS implementation. It covers the first regional transition, repeatable
transport, the overland connection, and cross-region Fly. Event-island boats,
debug warps, blackout recovery, Teleport, and Dig are excluded because they are
not independent Kanto to Johto routes.

## At a glance

| Method | Johto endpoint | Kanto endpoint | Normal-play unlock | Direction notes |
| --- | --- | --- | --- | --- |
| S.S. Aqua | Olivine Port | Vermilion Port | First Pokemon League clear and S.S. Ticket | First trip is Johto to Kanto. Later Johto departures need the ticket; Kanto departures also require the Machine Part to be returned. |
| Fly | Any visited Fly destination | Any visited Fly destination | Fly HM access, Badge 5, a valid party user, and at least one visited destination in the target region | Becomes the earliest practical return to Johto after the player exits the first ferry in Vermilion. |
| Overland | New Bark, Route 27, Tohjo Falls, Route 26, Reception Gate | Route 22, then Viridian City | Surf, Waterfall, and clearance of the Vermilion Snorlax after the Power Plant and radio chain | Bidirectional after the Kanto-side Reception Gate guard is removed. |
| Magnet Train | Goldenrod Station | Saffron Station | Return the Machine Part, complete Copycat's lost doll quest, and obtain the Pass | Repeatable and direct in both directions. Goldenrod has a latent prerequisite-check asymmetry. |

## Unlock order

In normal play, these routes do not become available together:

1. The first League clear enables Elm's S.S. Ticket handoff.
2. The S.S. Aqua maiden voyage creates the intended first Kanto arrival and
   initializes Kanto story state.
3. Once Vermilion has been visited, Fly can return the player to any previously
   visited Johto destination.
4. Returning the Machine Part enables the Vermilion ferry departure and starts
   the Copycat quest that awards the Magnet Train Pass.
5. The radio upgrade from the same story chain allows the player to clear the
   Vermilion Snorlax. That removes Reception Gate's Kanto-side guard and opens
   the overland route.

The S.S. Aqua is therefore the only normal first-entry route into Kanto. Every
other Kanto to Johto option depends either on having visited a Kanto Fly point
or on objectives that begin after the first Kanto arrival.

## S.S. Aqua

### First trip from Johto to Kanto

- The first League clear sets New Bark Town to the post-League state. Elm then
  calls the player back and gives the permanent S.S. Ticket in his lab.
- The Olivine sailor checks the ticket and sends the player into the S.S. Aqua
  maiden voyage at `VAR_SSAQUA_STATE = 1`.
- The player cannot disembark immediately. The onboard grandfather,
  granddaughter, and sleeping-sailor sequence advances the ship to state 5.
- Leaving the ship initializes the Kanto story flags, sets the ship to state 7,
  and warps the player to Vermilion Port.

Evidence: `game/data/maps/PokemonLeague_HallOfFame_hns/scripts.inc:57-67`,
`game/data/maps/NewBarkTown_hns/scripts.inc:7-19`,
`game/data/maps/NewBarkTown_Lab_hns/scripts.inc:479-487`,
`game/data/maps/OlivineCity_PortInside_hns/scripts.inc:107-118,191-203`,
`game/data/maps/SSAqua_1F_hns/scripts.inc:5-38,154-157,168-192`,
`game/data/maps/SSAqua_RoomNW_hns/scripts.inc:14-27`,
`game/data/maps/SSAqua_CaptainsRoom_hns/scripts.inc:13-33`,
`game/data/maps/SSAqua_RoomSSE_hns/scripts.inc:19-41`.

### Repeat trips

- Olivine to Vermilion checks `VAR_SSAQUA_STATE >= 7` and the S.S. Ticket, then
  warps directly to Vermilion Port. It does not require the Power Plant repair.
- Vermilion to Olivine first checks `FLAG_RETURNED_MACHINE_PART`, then checks
  the S.S. Ticket. This makes the return ferry unavailable until the Power Plant
  story is complete.
- Repeat crossings do not revisit the ship-interior quest.
- Port dialogue says the ship sails every day, and the transport scripts contain
  no weekday or time check.

Evidence: `game/data/maps/OlivineCity_PortInside_hns/scripts.inc:107-143`,
`game/data/maps/VermilionCity_PortInside_hns/scripts.inc:11-63`.

## Fly

Fly is a true cross-region transport method in HNS. Once
`FLAG_VISITED_KANTO` is set, the Fly screen uses one combined Johto and Kanto
map. Its destination list contains Fly points from both regions, and each point
is selectable only after its own visited flag is set.

The combined map flag is set during the player's first Route 27 border scene,
before the first Kanto arrival. That does not permit an early Kanto flight
because no Kanto destination has been visited yet. After the first S.S. Aqua
arrival and the Vermilion visit flag, the player can Fly from either region to
visited points in the other.

Normal field use requires Badge 5, the Fly HM access obtained in Cianwood, a
compatible party setup, and a map type that allows Fly. Those requirements are
already satisfied or obtainable before the first League clear, so Fly is the
earliest practical way back to Johto after the maiden voyage.

Evidence: `game/data/maps/Route27_hns/scripts.inc:22-48`,
`game/src/region_map.c:1498-1503,2794-2822,2862-2890,2980-3027`,
`game/src/field_move.c:57-64`,
`game/data/maps/CianwoodCity_hns/scripts.inc:33-47,65-82`,
`game/data/maps/VermilionCity_hns/scripts.inc:1-10`.

## Overland route through Reception Gate

The physical route is:

`New Bark Town -> Route 27 west -> Tohjo Falls -> Route 27 east -> Route 26 -> Route 26 North -> Reception Gate -> Route 22 -> Viridian City`

Route 27 contains both exterior cave entrances, but impassable terrain separates
its western and eastern sections. The normal path enters Tohjo Falls at warp 0,
crosses the cavern, and returns to Route 27 through warp 1. The cavern has two
waterfalls between the entrance pools and upper waterway, so the crossing needs
both Surf and Waterfall under the current field-move rules.

Reception Gate's east exit leads to Route 22. A Black Belt occupies that branch
under `FLAG_INDIGOJUNCTION_HIDE_KANTO_GUARD`. Entering Route 27 for the first
time explicitly makes the guard visible. Clearing the Vermilion Snorlax later
sets the hide flag, removing the guard and making the route bidirectional.

The Snorlax requires the Power Plant theft chain, return of the Machine Part,
the Lavender radio upgrade, and a successful Snorlax encounter. Running from
Snorlax does not open the gate. The Reception Gate officer's Badge 8 and
legendary-story checks apply to the northbound Victory Road exit, not to the
east-west regional crossing after its guard is gone.

Evidence: `game/data/maps/NewBarkTown_hns/map.json:15-25`,
`game/data/maps/Route27_hns/map.json:15-25,290-304`,
`game/data/maps/TohjoFalls_Cavern_hns/map.json:96-109`,
`game/data/maps/Route26_hns/map.json:15-25`,
`game/data/maps/Route26North_hns/map.json:15-30,125-143`,
`game/data/maps/ReceptionGate_hns/map.json:44-56,72-107`,
`game/data/maps/Route22_hns/map.json:15-25,174-184`,
`game/data/maps/Route27_hns/scripts.inc:22-48`,
`game/data/maps/VermilionCity_hns/scripts.inc:179-215`,
`game/data/maps/ReceptionGate_hns/scripts.inc:31-60`,
`game/data/scripts/field_move_scripts_hns.inc:386-405`.

## Magnet Train

The Magnet Train links Goldenrod Station and Saffron Station with direct,
repeatable warps.

In normal play, the player must:

1. Return the stolen Machine Part to restore Power Plant service.
2. Speak to Copycat after the repair to start the lost doll quest.
3. Recover the Lost Item from Vermilion and return it to receive the Pass.

Saffron Station explicitly checks both the returned Machine Part flag and the
Pass. Goldenrod Station checks the Pass first and boards immediately when it is
present, without rechecking the power flag. This has no effect in the intended
story sequence because Copycat does not offer the Pass quest until the Machine
Part is returned, but it is a real asymmetry if state or inventory is modified.

Evidence: `game/data/maps/SaffronCity_CopyCatsHouse_2F_hns/scripts.inc:28-36,143-155`,
`game/data/maps/SaffronCity_TrainStation_hns/scripts.inc:23-45,47-84`,
`game/data/maps/GoldenrodCity_TrainStation_hns/scripts.inc:9-24,27-64`.

## Product implications

- Removing the maiden-voyage gate would require moving the Kanto initialization
  currently performed when the player leaves the S.S. Aqua.
- Removing badge locks from field moves does not create an early Kanto Fly
  bypass. Kanto destinations still require their visited flags.
- Opening Reception Gate early would create a pre-ferry Kanto entrance and
  bypass the ship's initialization unless that state setup is moved elsewhere.
- The Vermilion ferry, Saffron train, and Reception Gate land route all depend
  on the Power Plant branch, but at different points in that branch.
- The Goldenrod train check should be made symmetrical if transport
  prerequisites are later normalized or granted out of story order.
