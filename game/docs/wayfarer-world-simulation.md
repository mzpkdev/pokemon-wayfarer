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
| Ambience | `src/wayfarer_ambience.c`, `include/wayfarer_ambience.h`; `src/wayfarer_walker_beats.c`, `include/wayfarer_walker_beats.h` | Beat selection (engine-free, deterministic); the beat runner (the primitives on a walker's object). The walker layer owns the decision points and interruptions. |
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
| A seam crossed again while one is pending | The new heartbeat queues (`deferredBegins`, up to two) with the context built at its load, and begins as the first unit of a later frame once the pending one completes: the same result as finishing it at the load, without the hitch in the seam's frame. With the queue full, only the oldest heartbeat finishes in the seam's frame, and the next begins sliced. A walker's routine advance under way stays queued across the seam. |
| A save | `CopyPartyAndObjectsToSave`, the first step of every save path, finishes it (`forcedFinishes`). |
| League resolution | `WayfarerWorld_OnLeagueResolved` finishes it before setting life events. |
| A Gym Leader's own object | `WayfarerWalkers_HideTemplate` finishes it if that leader hasn't acted yet, so the leader's object matches the finished heartbeat. Gyms are entered by warp, so this runs behind the fade. |
| A Gym visitor leaving (`FinishLeaving`, reachable from story-scene and slot checks) | Leaves the record alone while one is pending (the visit's one-heartbeat dwell ends it next heartbeat); no forced finish. |
| Walkers | Their AI, spawns, world jobs and world writes pause while it runs, as with locked controls. A walker released to the heartbeat at a seam is dropped at once. |
| `InitHeap(gHeap)` | `WayfarerWorld_OnHeapReset` forgets the workspace (never frees it). The search under way restarts on the next frame with a new one; trainers that already acted stay done. |
| New Game, Continue, load | Drop any pending heartbeat. |
| No heap for the workspace | The heartbeat waits and retries next frame (`workspaceWaits`), at most 60 frames (walkers pause meanwhile), then the rest of it skips. A forced finish with no heap skips the rest and any queued heartbeats. |

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
own 9 KB heap workspace while it runs; the cached path answers at once). The
trip search and the grid search share one 48-line slice a frame, and a frame
that runs a spot choice skips the grid search. A map load, a save and
the league hook complete the advances first (`WayfarerWalkers_FlushWorldJobs`).

Walkers stroll: every step is `MOVEMENT_ACTION_WALK_SLOW_*` (32 frames a
tile, [notable ambience](../../.product/specs/notable-ambience.md), "Pace"),
except a walk-off, a back-off from the player, a strip actor, and the step
out of (or into) the map through a side lane, which walk at normal speed
(`WalksNormalSpeed`). The walk-off timeout doubled to 1,200 frames.

Exits commit a hop like the heartbeat's: a walker checks `WorldSim_HopAllowed`
before stepping out through a map side or a door, and while the next map is
full it waits once, then hands off. A strip actor that can't walk straight on
(a 1-wide lane into a wall) is removed after 30 frames.

## Beats

[Notable ambience](../../.product/specs/notable-ambience.md) adds short
expressive beats to the walkers. `src/wayfarer_ambience.c` picks them (the
spec's gate, filter, class, idle odds and pick; it never reads the RNG);
`src/wayfarer_walker_beats.c` runs a beat's primitives on the object; the
"Notable ambience" section of `src/wayfarer_walkers.c` gathers the context,
holds the decision points and stops beats.

**Decision points.** Only local actors run beats (not a Gym visitor just
leaving, a walk-off or a strip actor), and only between steps:

| Decision | Where |
| --- | --- |
| Step | `WalkStep`, before each path step; a beat that stops the walk takes the frame (hum walks on) |
| Blocked | `WalkStep`'s blocked path when an object, not the player or a wall, is in the way (react beats only) |
| Arrive | `OnGoalReached` when the stay starts, after `StartTemplate` |
| Leave | `LocalDwellTick` when the dwell runs out, after `Replan`: the plan waits for the beat |
| Dwell | each dwell tick in the template phase, before `TemplateTick` (a beat takes the template's turn) |
| React | every frame (`AmbienceFrame`), react beats only |

Context: walking (a path, between steps), dwelling (at the spot, template
phase), outdoors, what the faced tile is (water, grass, counter) or holds (an
NPC: not the player, the follower, a walker or the companion), the spot's kind
and named activities, the record's activity, the player's distance while the
walker is in view, the ticks the player has stood adjacent and facing it
without pushing, and the nearest other local walker of each relation.
`companion_room` never holds until the companion lands.

While a beat holds the walker, `UpdateActor` still runs the yield checks and
the dwell clock, but no template move, step or plan; a search under way still
finishes. The templates' emotes are beats now (`grass_rustle`, `water_bite`
and `water_wait`, `chat_talk`); `WorldSim_TemplateEmote` is no longer called.

**Primitives.** Every held movement waits until the last one is cleared and
waits for its own end. Steps are slow walks to a free, standable tile
(`WayfarerWalkers_BeatCanStep`: the walker's collision rules); a blocked step
is skipped with its return, and a blocked return waits 60 frames, then the
beat ends and the walker walks back to its spot. `step back` locks the facing
for the step only. `spin` is four face turns (the engine's spin action slides
a tile). `bow nurse` bows the nurse (only the three nurse sprites with a bow
animation; never Chansey) within 3 tiles, and clears her held movement after.
`turn npc` turns the NPC on the faced tile with `ObjectEventTurn` (still and
not a trainer only). Icons and effects are skipped when no sprite palette slot
is free; an FLDEFF icon (or "?", which shares its effect id) that is busy
waits a tick, then is skipped. The shaking grass is stopped after 2 ticks, at
the beat's end or on a stop. `splash` is a ripple on the water tile with the
splash's sound: `FLDEFF_SPLASH` draws at an object's feet and reads past the
object table without one. Effects only play while the walker is in view.

**State and RAM.** All beat state is one heap block (`struct WalkerAmbience`,
568 bytes: a debug block, then per actor the selection state, the runner
state, the tick and adjacency counters), allocated while walkers exist (a
failed allocation retries next frame; beats just don't run) and held by the
EWRAM pointer `sAmbience`. It costs no EWRAM: the actors' bools became
bitfields and the template emote byte went, which pays for the pointer and
the latches byte (`ambienceLatches`: notice-player, lingers and the greeted
actors survive a menu). `InitHeap` calls `WayfarerWalkers_OnHeapReset`
before it rewrites the heap: running beats end with plain field writes
(facing, lock, a bowing nurse), and the pointer is dropped, never freed;
counters, cooldowns and quiet gaps start again in the next block.

**Interruptions.** A running beat stops at once, unlocks and restores the
walker's facing and stops its effects when the player presses into the walker
(at once; the back-off still needs 40 frames of pressing), at a back-off,
yield, walk-off or a Gym visitor's leaving, while the field controls are
locked, on a story object (via the yield), on a heap reset and on a map change
(`OnSeam`, warps and removals). A walk or a jump can't be cancelled: the
walker's `actionPending` waits for it, then it turns back. A walker the beat
left a tile off its spot walks back.

The yield rule's trigger is pressing into the walker (adjacent, facing it,
direction held), not just standing there facing it.

`gWayfarerWalkersDebug` exports counters, the worst slice in scanlines and each
actor's state for `tools/wayfarer_walkers/verify.py`, the SkyEmu verifier
(results in `.product/research/overworld-walkers/`, `summary.json`). The
verifier refuses a ROM built from other tables (content hash). `perf`,
`seam`, `recross` and `longtrip` fail past 200 scanlines in a frame; `perf`
also past 44 frames per warp, and `recross` past 1,000 scanlines finished
inside one seam frame. The beat scenarios read `sAmbience`'s debug block (beats
started by class, ended, interrupted, icons, effects, skips, the running beat
per actor, and a ring of the last 32 start/end/interrupt events with their
walker frame): `beatspot` (a water beat at Viridian's pond, only beats whose
context holds), `notice` (once per approach, `stare_down` for a stoic
trainer), `greet` (Brock and Misty greet once), `beatpush` (pressing stops a
beat within frames and restores the facing, then the back-off) and
`determinism` (two runs from boot give the same beat log).

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
