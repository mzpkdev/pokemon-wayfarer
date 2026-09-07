# Blackthorn and Dragon's Den nearby-route audit

This code and map-data audit supports a nearby-detour certificate, subject to a cleared Ice Path. It does not use emulator play. Whirl Islands are out of scope.

The useful catch area extends west through Ice Path to Route 44, Mahogany, Route 43 and Route 42's eastern bank. It is a substantial cave backtrack, not a direct road from Blackthorn to Route 44. The nominal connection between those two maps has no traversable boundary tiles.

## Accepted catch areas

Coordinates below are authored map coordinates, without the engine's seven-tile border offset. Fishing examples satisfy the relevant geometry: standing elevation 3, adjacent fishable water at elevation 1, and no static collision on the water. The encounter header applies to the whole map, so the eastern bank of Route 42 uses the Route 42 fishing table without requiring the player to cross its lakes.

| Map ID | First Surf search | Whirlpool search after obtaining Surf | Usable location |
| --- | --- | --- | --- |
| `MAP_BLACKTHORN_CITY_HNS` | Fishing | Fishing or water encounters | Stand `(18,26)`, fish north into `(18,25)` in the pool south of the Den. The southern decorative water is not a valid fishing source. |
| `MAP_ROUTE44_HNS` | Fishing | Fishing or water encounters | From Ice Path exit `(68,12)`, reach `(48,13)` and fish west into `(47,13)` without Surf. |
| `MAP_ROUTE43_HNS` | Land and fishing | Land, fishing or water encounters | The western grass path reaches `(10,31)`, facing west into `(9,31)`, without the Rocket toll gate or Surf. |
| `MAP_ROUTE42_HNS` | Fishing from eastern bank | Fishing or water encounters | From Mahogany enter near `(93,12)`, then stand `(69,12)`, facing west into `(68,12)`. No crossing of the lake is needed. |
| `MAP_DRAGONS_DEN_CAVERN_HNS` | Excluded: getting here requires Surf | Fishing or water encounters before the whirlpool | From northern ladder `(31,3)`, reach `(30,19)`, facing south into `(30,20)`. This bank remains reachable even if every authored NPC position is treated as occupied. |

Use each map's actual DAY/NIGHT encounter profiles rather than assuming identical species at night. Mahogany is a transit stop, not a useful fishing source. Ice Path's cave encounters may be inspected separately, but they are unnecessary in the minimal candidate-map certificate.

Map sources: [Blackthorn](../../../../../game/data/maps/BlackthornCity_hns/map.json), [Route 44](../../../../../game/data/maps/Route44_hns/map.json), [Route 43](../../../../../game/data/maps/Route43_hns/map.json), [Route 42](../../../../../game/data/maps/Route42_hns/map.json), [Den cavern](../../../../../game/data/maps/DragonsDen_Cavern_hns/map.json). Fishing eligibility is implemented in `IsPlayerFacingSurfableFishableWater`, [field_player_avatar.c](../../../../../game/src/field_player_avatar.c).

## Directed itinerary and the Ice Path condition

The route from Blackthorn to Route 44 is:

`Blackthorn → Ice Path 1F east → B1F south → B3F → B4F → B2F → B1F north → 1F west → Route 44`.

The exact paired warp chain, using zero-based warp IDs, is:

```text
Blackthorn 0 → 1F 3
1F 2 → B1F 3
B1F 1 → B3F 1
B3F 0 → B4F 1
B4F 0 → B2F 1
B2F 0 → B1F 2
B1F 0 → 1F 1
1F 0 → Route44 0
```

The terrain probe finds all of these internal links in both directions when B2F's four fallen boulders are present. Ice sliding is included, and fixed breakable rocks remain obstacles, so the witness does not require Rock Smash. The selected B1F path avoids the cracked-ice puzzle and its skip button.

Fresh-game flags hide all four B2F boulders. Without them, the B2F central staircase can reach the exterior ladder when going west out of Blackthorn, but the reverse route fails in the probe. Pushing the boulders down from B1F clears their B2F hide flags. Consequently, a certificate must state that the player already cleared this puzzle on the normal approach, or can clear it with Strength before returning. Arrival by some other route must not silently inherit that condition. Merely allowing the player to leave Blackthorn westward is insufficient proof of a round trip.

Sources: [Ice Path floor warps and boulders](../../../../../game/data/maps/IcePath_B1F_hns/map.json), [B2F objects](../../../../../game/data/maps/IcePath_B2F_hns/map.json), [fresh-game hide flags](../../../../../game/data/scripts/new_game.inc), and `HandleBoulderFallThroughHole` in [field_control_avatar.c](../../../../../game/src/field_control_avatar.c). The optional B1F skip script only warps within B1F; it is not evidence that the B2F boulder condition is waived.

After Route 44, its western connection reaches Mahogany. Mahogany's northern gate reaches Route 43; its western connection reaches Route 42's eastern bank. These route and gate connections are bidirectional.

## Story prerequisites and return travel

Blackthorn's gym-boy checks require badges 5, 6 and 7. Pryce's victory script awards badge 7 and sets `VAR_MAHOGANY_TOWN_STATE` to 15. At state 15, the candy merchant still triggers at `(30,14)`, but not at `(30,12)` or `(30,13)`. Crossing via row 12 therefore avoids him in either direction. Do not describe the merchant as wholly removed at this state. See [Blackthorn scripts](../../../../../game/data/maps/BlackthornCity_hns/scripts.inc), [Pryce victory](../../../../../game/data/maps/MahoganyTown_Gym_hns/scripts.inc), and [Mahogany coordinate triggers](../../../../../game/data/maps/Mahoganytown_hns/map.json).

Route 43's western grass path bypasses its toll gate. This avoids dependence on the Rocket shakedown script or available money. Reaching Route 42's eastern fishing shore does not require Mt. Mortar, Strength or Surf.

Clair's victory sets Blackthorn state 2, allowing the Den challenge; the shrine awards badge 8. The Den entrance contains two internally linked sections, then leads to the cavern's northern ladder. These links are reversible. See [Clair victory](../../../../../game/data/maps/BlackthornCity_Gym_hns/scripts.inc), [Den entrance](../../../../../game/data/maps/DragonsDen_Entrance_hns/map.json), and [shrine reward](../../../../../game/data/maps/DragonsDen_Shrine_hns/scripts.inc).

## Whirlpool really blocks the shrine route

The gate includes invisible objects with `OBJ_EVENT_GFX_ARCHER_HNS` and `EventScript_Whirlpool`, not only the visible Whirlpool sprites. Blocking only objects whose graphics name contains `WHIRLPOOL` produces a false bypass.

The southwest footprint occupies `(15,38)`, `(16,38)`, `(15,39)` and `(16,39)`. With Surf available and all scripted Whirlpool objects blocked, the terrain probe reaches 819 cells from the entry ladder but cannot reach the shrine doorstep `(31,47)`. Removing those obstacles reaches 1,212 cells, including the doorstep. The Whirlpool script supports movement in all four directions, so crossing the gate does not create a one-way return trap once the player has the move.

Sources: [cavern object events](../../../../../game/data/maps/DragonsDen_Cavern_hns/map.json), `GetObjectObjectCollidesWith` in [event_object_movement.c](../../../../../game/src/event_object_movement.c), and [Whirlpool movement scripts](../../../../../game/data/scripts/field_move_scripts_hns.inc).

## Areas deliberately not credited

- Route 45 descends south through ledges. Do not count all of its encounters as a returnable nearby catch area merely because it has a connection to Blackthorn. The lower-route return via Route 46, Dark Cave or a larger regional loop is outside this minimal certificate.
- Lake of Rage is reachable beyond Route 43, but its script selects high- or low-tide layouts depending on Mahogany state and tide. The high-tide southern fishing shore is visible in the probe. It is omitted from the minimal distribution certificate until both layouts have equivalent bank witnesses; it is not needed to establish the smaller catch area above.
- The Rocket Hideout Whirlpool HM is an alternative, not a prerequisite credited by this native-carrier audit.

## Reproduction and limits

Run `python3 .product/research/native-hm-windows/revisions/nearby-access/blackthorn_routes.py` from the repository worktree. It reads layout binaries, tileset attributes, map events and movement constants, prints terrain probes, and asserts the Den obstacle and Ice Path boulder-state differences. It does not alter game data.

The probe is intentionally narrower than an emulator: it models static collision, basic elevation changes, ordinary ice sliding and fixed utility obstacles. It does not emulate roaming NPC timing, all directional stairs, every script, ice-floor timing or battle outcomes. Door activation is not represented by ordinary collision-free stepping; for the shrine, the tested target is the doorstep rather than the door tile. Script prerequisites above were reviewed separately. The route evidence establishes code-level witnesses under named conditions, not a universal proof for every possible save state.
