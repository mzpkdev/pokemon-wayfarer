# Hoenn nearby Surf acquisition audit

This is a code and map-data audit of reachable encounter sources before acquiring Surf. It does not certify the whole sea journey or run the game. Whirl Islands are outside scope.

## Accepted encounter sources

Each source below has a reachable grass tile or fishable bank on the correct side of the obstacle. Assume a Rod, catching supplies, and the ability to fight ordinary trainers. Do not assume Cut, another field move, a bicycle, or completion of unrelated story events.

| Origin | Accepted maps and methods | Exclusions or conditions |
|---|---|---|
| Lilycove town | `MAP_LILYCOVE_CITY`: `fishing_mons`; `MAP_ROUTE121`: `land_mons`, `fishing_mons` | Route 121 eastern section only. Route 120 is conditional on the Aqua grunts already being absent or separately acquiring Cut. |
| Route 118 west bank | `MAP_ROUTE118`: `fishing_mons`; `MAP_ROUTE117`: `land_mons`, `fishing_mons` via Mauville | Route 118's grass is east of the water. Do not count it from the west bank. |
| Route 118 east bank | `MAP_ROUTE118`: `land_mons`, `fishing_mons`; `MAP_ROUTE119`: southern `land_mons`, `fishing_mons` | Route 123's reachable western pocket has no grass or bank in the conservative graph. Do not count its full table. |
| Mossdeep town | `MAP_MOSSDEEP_CITY`: `fishing_mons` | No other encounter map can be entered on foot from this island. |
| Pacifidlog town | `MAP_PACIFIDLOG_TOWN`: `fishing_mons` | No other encounter map can be entered on foot from its connected platforms. |

These Hoenn encounter profiles use the day table as their alias outside daytime. The encounter simulator should apply its existing alias rules rather than require an absent night profile. A map-level method table is usable once at least one reachable tile supports that method; its inaccessible water and grass areas are not independently credited.

## Lilycove and Route 121

[Lilycove's connections](../../../../../game/data/maps/LilycoveCity/map.json) lead west to Route 121 and east to the sea on Route 124. From the Pokémon Center exit at `(24,15)`, the layout graph reaches the west boundary `(0,16)` and a local fishing position `(69,22)`, facing water `(70,22)`. Getting to the beach uses ordinary ledge jumps, including `(53,8) -> (53,10) -> (53,12)`. The reverse search reaches the Center again, so catching at that bank does not rely on already having Surf to leave the beach.

From Route 121's Lilycove entrance `(79,6)`, the conservative graph reaches 89 grass tiles and the Mt. Pyre pier fishing position `(31,18)`, facing `(31,19)`. The grass table contains Poochyena, Mightyena, Shuppet, Oddish, Gloom, Wingull and Kecleon. None has Surf in the approved distribution. The pier's fishing table adds no new early Surf carrier relative to the existing Tentacool/Wailmer constraints. See [Route 121 events](../../../../../game/data/maps/Route121/map.json) and [encounter data](../../../../../game/src/data/wild_encounters.json).

Route 120 should not be counted unconditionally. Three Aqua grunts occupy `(30,7)`, `(31,7)` and `(30,8)` on Route 121. The automatic event that removes them triggers at `x=25`, `y=5..8`, on their western side. The alternate openings contain Cut trees at `(32,5)` and `(26,12)`. With all objects present, the east-side graph cannot reach Route 120. Removing those three grunts makes the ordinary path passable. [The removal script](../../../../../game/data/maps/Route121/scripts.inc) requires no item or battle once its trigger is reachable, but that is not the same as being reachable from Lilycove first.

If the grunts are already gone, the detour continues into Route 120's southern section. It has reachable Marill grass and pond fishing without Surf. The tested entry `(39,86)` reaches long grass `(11,72)` and a pond bank `(9,81)` facing `(8,81)`. This does not require the Devon Scope bridge at `(12..13,15..17)` in the north. [Route 120 scripts](../../../../../game/data/maps/Route120/scripts.inc) distinguish those northern bridge changes from the southern area.

Design implication: nearby routes do not, by themselves, eliminate Lilycove's previously reported early modern Surf gap in every story state. Options include the existing encounter proposal, a Surf role on Wingull after window validation, or an explicit Cut-acquisition chain. The extracted compatibility baseline lists Surf for Wingull, which already occurs in the reachable Route 121 grass. Do not silently assume the grunts have cleared.

## Route 118's two banks

[Route 118](../../../../../game/data/maps/Route118/map.json) has one encounter table but two disconnected land components. From `(0,10)` on the Mauville side, the graph reaches 11 fishing positions and no grass. One bank is `(17,9)` facing `(18,9)`. It cannot reach the eastern boundary.

The walking detour west is `Route118 -> MauvilleCity -> Route117`. The [Mauville connections](../../../../../game/data/maps/MauvilleCity/map.json) and [Route 117 connections](../../../../../game/data/maps/Route117/map.json) are reciprocal, with zero vertical offset. The graph crosses Mauville from `(39,10)` to `(0,8)`. Route 117's eastern entry `(59,8)` reaches grass, including `(17,14)`, and a pond bank `(30,8)` facing `(30,7)`. This detour does not use Wally's Gym entrance or require his battle.

On the eastern side, `(79,10)` reaches 110 grass tiles, 22 bank positions, and the northern exit `(57,0)`. One bank is `(44,15)` facing `(44,16)`. The northern connection enters Route 119 at `(17,139)` after applying the connection offset. Its southern section contains accessible long grass and river banks, including `(17,111)` facing `(17,110)`. The ordinary [Steven event on Route 118](../../../../../game/data/maps/Route118/scripts.inc) is dialogue and movement, not an HM prerequisite. The southern Route 119 sample does not pass the Weather Institute bridge or its northern rival sequence. See [Route 119 scripts](../../../../../game/data/maps/Route119/scripts.inc).

Route 123 deserves a warning: its connection exists, but starting at `(0,10)` reaches neither grass nor a fishable bank in this graph. Giving the east-bank search all of Route 123's encounters would overstate access. No Route 123 source is needed in the accepted set.

## Mossdeep and Pacifidlog

Mossdeep's Pokémon Center exit `(28,17)` reaches the western beach and a fishable position `(26,30)` facing `(26,31)`. This route stays west of the Space Center scene and does not require the Gym or Dive. [Mossdeep scripts](../../../../../game/data/maps/MossdeepCity/scripts.inc) set visited state and run the Space Center sequence separately.

Some Mossdeep land touches the map's western border, but the destination tiles in Route 124 are water. In particular, Route 124 `x=79`, `y=59..66` is collision-zero ocean water at elevation 1. These are the destinations opposite the reachable Mossdeep boundary `x=0`, `y=19..26` with the map connection offset applied. The northern and southern town boundaries have no reachable land exit. Do not count Route 124, Route 125 or Route 127 encounters as pre-Surf sources from Mossdeep.

Pacifidlog's Center exit `(8,16)` reaches the connected platforms and the bank `(9,17)` facing `(9,18)`. The [town script](../../../../../game/data/maps/PacifidlogTown/scripts.inc) installs the bridge step callback; it does not impose a story gate on fishing. The walking tiles use `MB_NO_RUNNING` and the Pacifidlog log behaviors at elevation 3.

The western platforms touch `x=0`, `y=11..14`, but the opposite Route 132 tiles are ocean water at elevation 1. The eastern platforms touch `x=19`, `y=20..22`, but the opposite Route 131 tiles are also water. This makes the town fishing table the only ordinary pre-Surf catch source in the reviewed area. A walk to the next settlement is not available here.

## Reproduction and limits

Run from the repository root:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 .product/research/native-hm-windows/revisions/nearby-access/hoenn_routes.py --report
```

[hoenn_routes.py](hoenn_routes.py) reads the Emerald layouts and their tileset attributes without changing game data. [hoenn_routes.json](hoenn_routes.json) preserves the resulting paths, source coordinates, component summaries and assertions for 13 cases. Run the script with map names instead of `--report` to print the decoded layout grid.

The decoder uses the Emerald 512-entry primary tileset boundary, little-endian 16-bit map entries, two collision bits, four elevation bits and the low-byte behavior. See [fieldmap.h](../../../../../game/include/fieldmap.h), [fieldmap.c](../../../../../game/src/fieldmap.c), and the [layout catalog](../../../../../game/data/layouts/layouts.json).

Movement checks use collision-zero land, compatible elevations including transition elevation zero, and optional directed cardinal ledge jumps. All authored object coordinates are treated as occupied unless the case explicitly removes the Route 121 grunts. It excludes multi-level bridges, bikes, boulder movement, Cut and Surf. This is a conservative graph for the reviewed ordinary paths, not an exhaustive implementation of collision callbacks, moving NPCs or script execution. The graph's script-sensitive boundaries were inspected separately above.

The bank test requires a reachable elevation-3 tile next to unblocked, fishable elevation-1 water. This follows [IsPlayerFacingSurfableFishableWater](../../../../../game/src/field_player_avatar.c), [CanFish](../../../../../game/src/item_use.c), and [the fishable-water behavior list](../../../../../game/src/metatile_behavior.c). Cardinal ledge behavior follows `GetLedgeJumpDirection` in [event_object_movement.c](../../../../../game/src/event_object_movement.c) and `ShouldJumpLedge` in the player-avatar code. The paths avoid sideways stairs and multi-level bridge state.

No encounter probabilities, utility learning levels, production encounters, or movement rules were changed by this audit. The orchestrator's catch-window simulation owns TR and learnset coverage for these accepted source sets.
