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
| Engine seam | `src/wayfarer_world.c`, `include/wayfarer_world.h` | Builds the context from league state and badges, starts the heartbeat on map loads and runs it over the next field frames, New Game, load validation, the league resolution hook. |
| Local actor | `src/wayfarer_walkers.c`, `include/wayfarer_walkers.h` | The on-screen walkers on the player's map: spawning from records, the shared grid search, handoffs both ways, connection strips, behaviour templates, local dwell, the Gym leader object, the follower rule, the local actor block. |
| Offline report | `tools/wayfarer_world_report/` | Runs the core on the host for N heartbeats and reports itineraries, crowding, coverage and cost. |

## Saved state

`struct WayfarerWorldState` (216 bytes) is appended to `struct PokemonStorage`,
which holds 13 boxes in Wayfarer builds. It is a 4-byte header (schema,
content hash), 25 eight-byte records in catalog order, a 9-byte local
actor block, and one byte of walker flags (whether the follower's hide flag
is the walkers' own, so a Continue without a warp can clear it again).
`save.c` asserts the sizes.

On load, a local actor block that no longer fits the saved location (its
trainer isn't on that map, or the tile is outside it, after a load-time
repair such as the league run's recovery to the lobby) is cleared; only
malformed bytes make the save corrupt (`WorldSim_CheckLocalActors`).

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
map. Otherwise it only starts the heartbeat: `WorldSim_HeartbeatBegin` applies
the derived states and fixes the priority order, the frozen mask and the
player's map as they are at the load.

`WayfarerWorld_Update` runs from `OverworldBasic` right after the walkers'
update and calls `WorldSim_HeartbeatStep` one work unit at a time (a node a
search expands, a search's start, five trainers of a reroute's full-map check,
a dwell tick, a spot choice or a hop):

- **Budget.** At most 88 scanlines with the walkers' update. Steps also stop
  40 lines before the next VBlank, because the rest of the field frame after
  the hook needs 16 to 32 lines (rarely 56) and an overrun is a lag frame.
- **Fades.** Nothing runs during a palette fade, such as a warp's fade-in.
- **Spot choices.** A spot choice (about 20 lines on average, up to about
  120 measured) only ever starts a frame's slice.
- **Starvation.** A frame with no time left skips its turn, but never more
  than 8 frames in a row (60 during a fade): then one unit runs anyway.

The search workspace (5 bytes per node, plus 136 bytes for a sliced search's
state and the maps a reroute avoids) is allocated on the first step and held
until the heartbeat ends. A warp resets the heap right after
`LoadMapFromWarp`, so it can't be allocated at the load.

Begin also brings back a trainer whose Away state ended, unless their home
map is full (they stay away until there is room). Their next step's spot
choices run first in the sliced phase, not inside Begin.

`WorldSim_Heartbeat` (Begin, then Step until done) is the same heartbeat at
once. The offline report and the tests use it. A trainer's first edge is found
just before it acts. That matches finding them all first, because it depends
only on the trainer's own record and path-cache entry, and acting changes
only those.

Edge cases:

| Case | Handling |
| --- | --- |
| A warp while one is pending | `OnMapLoad` finishes it at once first, behind the fade (`pendingAtLoad`). |
| A seam crossed again while one is pending | The new heartbeat queues (`deferredBegins`, up to three) with the context built at its load, and begins when the pending one completes: the same result as finishing it at the load, without the hitch in the seam's frame. A full queue finishes at once. |
| A save | `CopyPartyAndObjectsToSave`, the first step of every save path, finishes it (`forcedFinishes`). |
| League resolution | `WayfarerWorld_OnLeagueResolved` finishes it before setting life events. |
| A Gym Leader's own object | `WayfarerWalkers_HideTemplate` finishes it if that leader hasn't acted yet, so the leader's object matches the finished heartbeat. Gyms are entered by warp, so this runs behind the fade. |
| A Gym visitor leaving (`FinishLeaving`, reachable from story-scene and slot checks) | Leaves the record alone while one is pending (the visit's one-heartbeat dwell ends it next heartbeat); no forced finish. |
| Walkers | Their AI, spawns, world jobs and world writes pause while it runs, as with locked controls. A walker released to the heartbeat at a seam is dropped at once. |
| `InitHeap(gHeap)` | `WayfarerWorld_OnHeapReset` forgets the workspace (never frees it). The search under way restarts on the next frame with a new one; trainers that already acted stay done. |
| New Game, Continue, load | Drop any pending heartbeat. |
| No heap for the workspace | The heartbeat waits and retries next frame (`workspaceWaits`); only a forced finish with no heap skips the trainers still to act. |

## Local actor

`WayfarerWalkers_Update` runs once per field frame from `OverworldBasic`.
Engine hooks, all `IS_WAYFARER` only:

| Hook | Where | Why |
| --- | --- | --- |
| `WayfarerWalkers_Update`, then `WayfarerWorld_Update` | `OverworldBasic` | The walkers' per-frame update, then the pending heartbeat's steps. |
| `WayfarerWalkers_OnWarp` / `OnCameraTransition` | `LoadMapFromWarp` / `LoadMapFromCameraTransition`, before the heartbeat | Drop actors on warps; keep them across a seam; set the frozen mask the heartbeat reads. |
| `WayfarerWalkers_OnContinue`, `WayfarerWalkers_Reset` | `WayfarerWorld_OnContinue`, `WayfarerWorld_InitNewGame` | Read the local actor block once on Continue; reset on New Game. |
| `WayfarerWalkers_OnHeapReset`, `WayfarerWorld_OnHeapReset` | `InitHeap(gHeap)` | The search workspaces are dropped; actors re-plan from their tiles; the heartbeat restarts its current search. |
| `WayfarerWorld_FinishHeartbeatEarly` | `CopyPartyAndObjectsToSave` | A save never holds half a heartbeat or half a walker's routine advance (it completes the walkers' world jobs first). |
| `WayfarerWalkers_IsActorObject` | `RemoveObjectEventIfOutsideView` | Actors aren't culled off view while the walker layer owns them. |
| `WayfarerWalkers_HideTemplate` | `TrySpawnObjectEvents` | A Gym Leader's own object stays hidden (also on scroll) while the leader is out. |
| null template guard | `GetObjectEventScriptPointerByLocalIdAndMap` | Runtime actors have no template; talking to one does nothing until stage 4. |

Actors are runtime objects (`SpawnSpecialObjectEventParameterized`) with the
dynamic local ids `0xF5`-`0xF8`, looked up without a map, so a camera seam
only retags their map. Changing `event_object_movement.c` re-pins its digest
in `src/data/wayfarer_sevii_maps.json` (the Sevii content audit pins that
file); `UpdateFollowingPokemon` itself is untouched: the follower is hidden
with `FLAG_TEMP_HIDE_FOLLOWER` while fewer than 2 object slots would stay free.

One grid search runs at a time across all actors, up to 8 node expansions per
field update and stopping once a slice has run 48 scanlines, in an 11,308-byte
heap workspace (a byte per tile for up to 7,320 tiles holding the move in and
the settled elevation, 64 second-elevation states for bridge tiles with their
from-layer bits, a 1,024-entry ring queue, 192-step paths per actor).

World work the walkers do runs as world jobs, a slice per frame like the
heartbeat: a watched trainer's routine advance (one spot choice per frame,
`WorldSim_AdvanceRoutineStep`) and a trip's first search
(`WorldSim_NextEdgeBegin`/`Run`, at most about 32 scanlines a frame, in its
own 9 KB heap workspace while it runs; the cached path answers at once). A
frame that runs a spot choice skips the grid search. A map load, a save and
the league hook complete the advances first (`WayfarerWalkers_FlushWorldJobs`).

Exits commit a hop like the heartbeat's: a walker checks `WorldSim_HopAllowed`
before stepping out through a map side or a door, and while the next map is
full it waits once, then hands off. A strip actor that can't walk straight on
(a 1-wide lane into a wall) is removed after 30 frames.

`gWayfarerWalkersDebug` exports counters, the worst slice in scanlines and each
actor's state for `tools/wayfarer_walkers/verify.py`, the SkyEmu verifier
(results in `.product/research/overworld-walkers/`, `summary.json`). The
verifier refuses a ROM built from other tables (content hash), and `perf`,
`seam`, `recross` and `longtrip` fail past their ceilings (200 scanlines in a
frame, 44 frames per warp).

## Checks

- `make BUILD=wayfarer check TESTS=test/wayfarer_world_sim.c` runs the core's
  mechanics tests against the generated tables, including a sliced heartbeat
  (tiny steps, lost workspaces) against one run at once; `test/wayfarer_walkers.c`
  covers the local actor's pure helpers.
- `python3 tools/wayfarer_walkers/verify.py --rom pokemon-wayfarer-e2e.gba
  --symbols pokemon-wayfarer-e2e.sym` drives the E2E ROM in headless SkyEmu.
- `make BUILD=wayfarer wayfarer-world-test` runs the generator's unit tests, and
  `wayfarer-world-report-test` runs the offline report's scenarios. Both are
  part of the Wayfarer `check`.
- `python3 tools/wayfarer_world_report/report.py --heartbeats 200 --all-badges`
  runs the offline report; it exits non-zero if any record turns invalid or a
  map goes over its cap.
