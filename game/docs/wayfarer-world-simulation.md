# Wayfarer world simulation

Notable trainers live in the overworld: they follow routines, travel between
maps, and spend time at spots. The design is
[notable world simulation](../../.product/specs/notable-world-simulation.md)
and [notable spots](../../.product/specs/notable-spots.md); this page is the
implementation map. Only `IS_WAYFARER` builds compile it.

## Layers

| Layer | Code | Owns |
| --- | --- | --- |
| Build-time tables | `tools/wayfarer_world/` | Walker graph (nodes, edges), spot table, routines, candidate lists, validation, content hash. Writes `src/data/wayfarer_world/tables.h` (generated, gitignored). |
| Table types | `include/wayfarer_world_data.h`, `include/constants/wayfarer_world.h` | Record and save layout, ROM table structs, enums. Engine-free. |
| Simulation core | `src/wayfarer_world_sim.c`, `include/wayfarer_world_sim.h` | Routines, life events, spot choice, travel, capacity, priority, New Game seating, load checks. Engine-free and deterministic. |
| Engine seam | `src/wayfarer_world.c`, `include/wayfarer_world.h` | Builds the context from league state and badges, runs the heartbeat on map loads, New Game, load validation, the league resolution hook. |
| Local actor | `src/wayfarer_walkers.c`, `include/wayfarer_walkers.h` | The on-screen walkers on the player's map: spawning from records, the shared grid search, handoffs both ways, connection strips, behaviour templates, local dwell, the Gym leader object, the follower rule, the local actor block. |
| Offline report | `tools/wayfarer_world_report/` | Runs the core on the host for N heartbeats and reports itineraries, crowding, coverage and cost. |

## Saved state

`struct WayfarerWorldState` (216 bytes) is appended to `struct PokemonStorage`,
which holds 13 boxes in Wayfarer builds. It is a 4-byte header (schema,
content hash), 25 eight-byte records in catalog order, and a 9-byte local
actor block. `save.c` asserts the sizes.

25 trainers are simulated: the spec's 27 less Bruno and Koga, whose
overworld sheets are face-only (48x32) in both HNS and FireRed.

## The seam

The core never reads engine state. `WayfarerWorld_BuildContext` fills
`struct WayfarerWorldContext`:

| Input | Source today |
| --- | --- |
| Away: league | `GetAcceptedLeagueEventMember` for the accepted event's lineup. |
| Away: partner | None: the Masters partner isn't implemented. |
| Pinned at a haunt | None: haunt placement isn't implemented. |
| Home-locked | A Gym Leader whose badge `GetBadgeStateForRegion` reports missing. |
| Provisional lineup | `SelectLeagueLineup` while the invitation is `LEAGUE_INVITATION_INVITED` (nothing sets that state yet). |
| Rising momentum | None: momentum isn't implemented. |
| World progress | `GetTrainerRating()`. |
| Frozen | `gWayfarerWorldFrozenMask`, set by the local actor layer. |

`ResolveLeagueEvent` calls `WayfarerWorld_OnLeagueResolved` with the lineup in
battle order, so celebrating, brooding and recovering are set in the same
transaction.

## Heartbeat

`WayfarerWorld_OnMapLoad` runs from `LoadMapFromWarp` and
`LoadMapFromCameraTransition`, beside the roamer hooks. It skips Continue
(`WayfarerWorld_OnContinue`), New Game's first warp, and reloads of the same
map. It allocates the search workspace (5 bytes per node) from the heap for
the heartbeat only.

## Local actor

`WayfarerWalkers_Update` runs once per field frame from `OverworldBasic`.
Engine hooks, all `IS_WAYFARER` only:

| Hook | Where | Why |
| --- | --- | --- |
| `WayfarerWalkers_Update` | `OverworldBasic` | The walkers' per-frame update. |
| `WayfarerWalkers_OnWarp` / `OnCameraTransition` | `LoadMapFromWarp` / `LoadMapFromCameraTransition`, before the heartbeat | Drop actors on warps; keep them across a seam; set the frozen mask the heartbeat reads. |
| `WayfarerWalkers_OnContinue`, `WayfarerWalkers_Reset` | `WayfarerWorld_OnContinue`, `WayfarerWorld_InitNewGame` | Read the local actor block once on Continue; reset on New Game. |
| `WayfarerWalkers_OnHeapReset` | `InitHeap(gHeap)` | The search workspace is dropped; actors re-plan from their tiles. |
| `WayfarerWalkers_IsActorObject` | `RemoveObjectEventIfOutsideView` | Actors aren't culled off view while the walker layer owns them. |
| `WayfarerWalkers_HideTemplate` | `TrySpawnObjectEvents` | A Gym Leader's own object stays hidden (also on scroll) while the leader is out. |
| null template guard | `GetObjectEventScriptPointerByLocalIdAndMap` | Runtime actors have no template; talking to one does nothing until stage 4. |

Actors are runtime objects (`SpawnSpecialObjectEventParameterized`) with the
dynamic local ids `0xF5`-`0xF8`, looked up without a map, so a camera seam
only retags their map. Changing `event_object_movement.c` re-pins its digest
in `src/data/wayfarer_sevii_maps.json` (the Sevii content audit pins that
file); `UpdateFollowingPokemon` itself is untouched: the follower is hidden
with `FLAG_TEMP_HIDE_FOLLOWER` while fewer than 2 object slots would stay free.

One grid search runs at a time across all actors, 8 node expansions per field
update, in a 5,564-byte heap workspace (1-bit visited and 2-bit back pointers
for up to 7,320 tiles, a 1,024-entry ring queue, 192-step paths per actor).
`gWayfarerWalkersDebug` exports counters, the worst slice in scanlines and each
actor's state for `tools/wayfarer_walkers/verify.py`, the SkyEmu verifier
(results in `.product/research/overworld-walkers.md`).

## Checks

- `make BUILD=wayfarer check TESTS=test/wayfarer_world_sim.c` runs the core's
  mechanics tests against the generated tables; `test/wayfarer_walkers.c`
  covers the local actor's pure helpers.
- `python3 tools/wayfarer_walkers/verify.py --rom pokemon-wayfarer-e2e.gba
  --symbols pokemon-wayfarer-e2e.sym` drives the E2E ROM in headless SkyEmu.
- `make BUILD=wayfarer wayfarer-world-test` runs the generator's unit tests, and
  `wayfarer-world-report-test` runs the offline report's scenarios. Both are
  part of the Wayfarer `check`.
- `python3 tools/wayfarer_world_report/report.py --heartbeats 200 --all-badges`
  runs the offline report; it exits non-zero if any record turns invalid or a
  map goes over its cap.
