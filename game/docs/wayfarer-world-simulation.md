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
holds the decision points and stops beats. The quiet gap holds back idle
beats only: react and transition beats (arrive_look, leave_turn) ignore it,
and a freshly spawned walker starts with it already passed.

**Decision points.** Only local actors run beats (not a Gym visitor just
leaving, a walk-off or a strip actor), and only between steps:

| Decision | Where |
| --- | --- |
| Step | `WalkStep`, before each path step; a beat that stops the walk takes the frame (hum walks on) |
| Blocked | `WalkStep`'s blocked path when an object, not the player or a wall, is in the way (react beats only) |
| Arrive | `OnGoalReached` when the stay starts, after `StartTemplate`; the beat's start facing is the spot's facing the walker is turning to |
| Leave | `LocalDwellTick` when the dwell runs out, after `Replan`: the plan waits for the beat. If a beat is still running then (`leavePending`), the leaving decision comes once it ends, before the plan |
| Dwell | each dwell tick in the template phase, before `TemplateTick` (a beat takes the template's turn) |
| React | `AmbienceFrame`, react beats only, the frame a react input changes (below) |

No beat starts during a back-off or the wait after it (`backingOff`, set by
`StartBackOff` and cleared by the next `ChooseGoal`). A beat that ends normally
at a stand, sit or browse spot facing elsewhere (`meditate`, `study_ground`...)
turns the walker back to the spot's facing.

Context: walking (a path, between steps), dwelling (at the spot, template
phase), outdoors, what the faced tile is (water, grass, counter) or holds (an
NPC: not the player, the follower, a walker or the companion), the spot's kind
and named activities, the record's activity, the player's distance while the
walker is in view, the ticks the player has stood adjacent and facing it
without pushing, and the nearest other local walker of each relation (the
nearest one this walker hasn't greeted yet, when there is one: a greeting is
once per pair, so the next walker of that relation gets one). The context is
gathered lazily: `needFacts` (worked out from the pool when the block is
allocated) holds the facts each class's rows read, and what the walker faces
(metatile and object lookups) is only read when a candidate row needs it.
`companion_room` is worked out only while dwelling, at an arrive or dwell
decision where an idle beat can win (`Ambience_IdleCanWin`: the gate and the
idle odds, as `Ambience_Select` is about to apply them; the companion beats
are idle), so at most about once every four dwell ticks; never every frame.

**Frame cost.** No beat may add a lag frame. The field frame reaches the
walkers' update 110 to 190 scanlines after VBlank in a busy outdoor map
(Viridian), and the rest of the frame after it takes 16 to 56, so most frames
have 20 to 40 lines to spare. The rules:

- The react check (latches and the react rows) runs only when its inputs
  changed: `ReactKey` folds the player's, the walker's and the other walkers'
  tiles, the camera, the latches, the adjacency ticks, walking or dwelling,
  the activity and whether a beat can start into one word. Each walker makes
  it once a spawn period, in its own odd frame (never a busy one), so a react
  beat fires within 8 frames (133 ms) of its condition first holding. A
  react row whose cooldown runs out at the coming tick also forces a check.
  At most one walker checks per frame; another's check waits for its next
  turn. A check also needs 12 lines left before the frame's end margin; it
  waits up to 8 samples (32 frames) for such a frame.
- The busy frames (`IsBusyFrame`: spawns at the spawn period's first frame,
  story scenes and culls at its third, the follower rule at its fifth, so no
  frame carries two of them; the engine's periodic time-of-day update and the
  frame after it, which re-blend the palettes) run no react check, no costly
  decision and start no beat step. A beat's own sprites (an icon, a ripple,
  the companion) still cost every frame they are on screen, the busy frames
  included: keeping each walker frame small leaves room for them.
- An idle walker's frame is just a tick count unless it is its react
  sample frame or its ambience tick.
- A decision where an idle beat can win (`Ambience_IdleCanWin`: the whole
  pool, maybe `companion_room`) is the costly kind: it waits for a frame with
  at least 24 lines left before a 64-line end margin (the rest of the field
  frame takes 16 to 56; the heartbeat keeps 40 for its own slices), one per
  frame (`WayfarerWalkers_ClaimFrame`), for at most 60 frames (the dwell tick
  or the step simply comes a few frames later). The other decisions are
  cheap and never wait. `companion_room` itself is only worked out when a
  companion beat could win (its cooldown over, its when, who and limits
  holding), its tiles only as far as a candidate needs them, and the follower
  rule's spawn-candidate count is reused from the last count less than a
  spawn period old.
- Costly beat starts take a claimed frame too: icons (30 lines), field effects
  (24), `companion in` (20) and the companion's spawn (30: the room check and
  the spawn, about 20), each waiting up to 30 frames (the spawn 120: in its
  second half it takes the first frame with as much room as the best one its
  first half saw) and then taking the next frame the walkers allow. The waits
  count every frame, the busy ones included. A start's new sprite also costs
  the frame after it (its first animation frame, a palette blend), so these
  starts never take a frame just before a busy one unless the wait ran out.

The one exception is the companion's sprite sheet (below).

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

**Companion.** The ace companion (the "Companion" section of
`src/wayfarer_walkers.c`) is an object with the dynamic local id `0xF9`
(`WALKER_COMPANION_LOCALID`, one past the walkers' ids) showing the species'
overworld follower sprite (`OBJ_EVENT_MON + species`, never shiny or
female), spawned with `MOVEMENT_TYPE_NONE` like a walker. One per map: the
ambience block holds its owner (the actor whose beat brought it out), its
object and its graphics. `companion_room` holds only when all of these do:

- the walker is dwelling (at its spot, template phase) and the spot is not a
  store, the Game Corner or a Center (counter or side);
- no companion is out or reserved by another walker (a companion beat
  reserves it as it starts, so a second walker's decision finds it busy and
  never frees the first one's preloaded sheet);
- at least `SPAWN_FREE_SLOTS + 1` (4) object slots are free, so the walker
  rule's 3 remain after it spawns;
- the follower rule, with the companion counted, still keeps the following
  Pokémon out: its free slots (`FollowerFreeSlots`, which counts spawn
  candidates not yet spawned and ignores the companion) minus 1 are at least
  2;
- two sprite palette slots are free: its dynamic palette (a full palette
  table isn't handled by the engine: an out-of-bounds write) and one for the
  beat's icon. This is stricter than the spec's one;
- a tile beside the walker the walker itself could step to (free, standable,
  elevation included), not the player's tile (current or previous) nor the
  tile straight ahead of the player, and inside the object view the engine
  keeps (`RemoveObjectEventIfOutsideView`; stricter than the spec, so it isn't
  culled at once). Tiles are tried at the walker's sides first, then behind,
  then ahead.

The species is the first of `gWayfarerAmbienceTrainers[slot].companions` that
fits. A big one (`AMBIENCE_COMPANION_BIG`, a 64×64 follower sprite: centred on
its tile, bottom-aligned, overhanging the tiles beside it and three rows
above) comes out only outdoors (the engine hides 64×64 followers indoors) and
needs a free 2×2 area: it stands only west or east of the walker (above or
below it would cover the walker), and its tile, the tile beyond it away from
the walker and the two tiles north of those must all be standable, without an
object and not the player's. Otherwise the next candidate is tried; with none
the room doesn't hold (no candidate is big in today's data: every resolved
companion has a 32×32 sprite).

`companion out` takes two frames: the planned species' sprite sheet is
decompressed first, alone, then the room is checked again and the object
spawns on the chosen tile facing the walker; without room the beat ends
there, cleanly. A preloaded sheet nothing used is freed.

The decompression is the one place a beat may cost a lag frame (a spec
deviation the user accepted). It is the engine's smol decompression of the
follower sheet (`LoadSheetGraphicsInfo`, the engine's own cost whenever any
follower sprite appears): one call that can't be resumed. In SkyEmu it took
61 to 142 scanlines by species in the greet scene (Brock's and Misty's aces)
and 75 for Blue's Umbreon (the companion scene), while the walkers' update starts
110 to 190 lines into the frame there. So no frame has room for it, and it waits for
the best frame it can get: for the first 60 frames only a frame with 150
lines left (rare outdoors), then the first frame with at least as much room
as the best one seen, and at 120 frames it goes ahead. That frame is often
one lag frame per companion out (none when the wait finds a roomy frame).
The companion's own spawn frame after it waits for room like any other start
and adds none.
A prepare lag shifts the emulated time against the field frames by one, so
engine work driven by emulated time (the time-of-day palette work) can then
land on a different field frame than it would have; the lag harness saw one
such knock-on frame in its 6,000-frame companion window. `companion_do face` turns it to the walker, `companion_do jump`
jumps it in place (each waits for its movement), and `face companion` turns
the walker to it. It is removed (`RemoveObjectEvent`) by `companion in`, at
its beat's end and on any interruption.

It is the lowest-priority object. `CompanionFrame` runs every walker frame,
controls locked or not: it removes any object with id `0xF9` that isn't the
live companion (a save captured it, a reset left it), and puts the live one
away at once, interrupting its beat, when its object vanished (culled out of
view, a script) or fewer than `SPAWN_FREE_SLOTS` slots are free (a map object
or a walker needs one). `UpdateFollower` (every 8 frames, when the follower
rule decides) puts it away before the rule could hide the follower because of
it, and `CullForSlots` removes it before any walker. A warp, a heap reset and a
Continue switch its object off with plain field writes (`active = FALSE`):
`RemoveObjectEvent` isn't safe there (the field's sprites may already be a
menu's or a battle's), and nothing respawns an inactive object; the reload
frees its sprite and palette. A seam interrupts its beat (`OnSeam`) and
removes it. The walkers' id checks (`WayfarerWalkers_IsActorObject`,
`AdoptRestoredObjects`, story suppression, `FacingFacts`, the NPC beats) never
take it for a walker or an NPC.

Debug counters (in the ambience block, after the log): companions out, put
away by their beat, put away early by reason (slots, follower, vanished,
interrupted), `companion_room` denied by its first failing condition (place,
busy, slots, follower, palette, tile), `companion out` failures, strays
removed and sheets decompressed (`companionPrepares`: the frames the `lag`
scenario allows).

**State and RAM.** All beat state is one heap block (`struct WalkerAmbience`,
680 bytes: a debug block, then per actor the selection state, the runner
state, the tick and adjacency counters, then the companion's owner, object,
graphics, preloaded sheet and per-actor plan, then the frame-cost state:
`needFacts`, the react keys, the follower rule's cached candidate count and
the decision waits), allocated while walkers exist (a failed allocation
retries next frame; beats just don't run) and held by the EWRAM pointer
`sAmbience`. It costs no EWRAM: the actors' bools became bitfields and the
template emote byte went, which pays for the pointer and the latches byte
(`ambienceLatches`: notice-player, lingers and the greeted actors survive a
menu). Three more bits in that bitfield byte (`beatActive`, `backingOff`,
`leavePending`) keep what a heap reset must not lose.

`InitHeap` calls `WayfarerWalkers_OnHeapReset`, but on the field's heap
resets (`MoveSaveBlocks_ResetHeap`: battles, map loads, the return from a
menu) only after the save blocks were copied over the heap, so the hook never
reads the block. It drops the pointer (never freed; counters and cooldowns
start again in the next block, like a fresh spawn's: counters at 0, the quiet
gap already passed), switches the companion's object off, ends any nurse's
bow and clears the walkers' facing locks, all with plain field writes. Every
path to a heap reset stops the beats first: the field-controls lock (menus,
scripts and every battle, wild ones included: `DoStandardWildBattle` and the
battle scripts lock the controls frames before `CB2_InitBattle`) and warps
(`WayfarerWalkers_OnWarp` forgets the actors); Continue starts with no block.
A beat still marked running anyway (`beatActive`) sends its walker back to
its spot, which turns it to the spot's facing again.

**Interruptions.** A running beat stops at once, unlocks and restores the
walker's facing, stops its effects and puts its companion away when the player presses into the walker
(at once; the back-off still needs 40 frames of pressing), at a back-off,
yield, walk-off or a Gym visitor's leaving, while the field controls are
locked, on a story object (via the yield), on a heap reset and on a map change
(`OnSeam`, warps and removals). A walk or a jump can't be cancelled: the
walker's `actionPending` waits for it, then it turns back. A walker the beat
left a tile off its spot walks back. Under the field-controls lock the stop
issues no held movement (`ObjectEventSetHeldMovement` would unfreeze the
walker under the script or menu): a face or icon action of the beat is
cleared and the facing set with a plain `ObjectEventTurn`; a walk or jump
under way turns back once the lock is gone. A nurse bowing for the beat is
left to the script and released when the lock is gone (or at a heap reset).

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
context holds; `water_bite` steps back off the water and ahead again and
leaves Blue on (13, 39) facing north; the screenshot shows its "!"),
`arrive` (Blue comes out of the Mart door 3 tiles from a square spot and
plays `arrive_look` the moment he reaches it), `leave` (Lance, stoic, with
one heartbeat of dwell left: `leave_turn` starts in the frame his local
dwell runs out, inside his quiet gap since `arrive_look`, so no idle beat
could have started then), `notice` (once per approach, over a 1,600-frame
stay in range, longer than its 1,200-frame cooldown; `stare_down` for a
stoic trainer), `greet` (Brock and Misty greet once; the map-name banner is
switched off, `FLAG_HIDE_MAP_NAME_POPUP`, so the screenshot shows them),
`beatpush` (pressing stops a beat within frames and restores the facing,
then the back-off), `beatlock` (Lance at the pond: the Start menu mid-
`stare_down` interrupts it within a few frames, and under the menu his
object stays frozen on its tile facing its start facing, north, as after;
the Start menu and the Bag (a heap reset) mid-`water_bite`, a tile off his
spot: no 0xF9 object, he is back on (13, 39) facing north and beats start
again in the new block; a map object given his sprite in RAM, standing in
for a story object, stops his next beat within a spawn period and he walks
off) and `determinism` (two runs from boot give the same beat log on the
same frames; the second run starts 60 frames later and has another RNG
state from the script's start, so a beat that read the RNG would come out
differently). Pace is gated on tile changes read from the walker's object
(a read caught partway through a lag frame is dropped): `spot` (the walk to
the pond after the Bag: 30 to 36 frames a tile, the median of the steps,
none faster; it measures 31) and `walkoff` (14 to 20; it measures 16).
Not covered on screen: an already-out companion put away because a map
object scrolled into view (free slots below 3; `companionslots` covers only
the room check before it comes out), and a real story scene (the RAM
stand-in exercises the same check). The companion
scenarios (results in `.product/research/ambience/`): `companion` (Blue at
Viridian's pond runs `ace_play`; Umbreon stands beside him facing him, not on
or ahead of the player, and is gone at the beat's end; the player's follower
stays out), `companionslots` (the crowded Safari Zone Gate leaves exactly 3
slots free: no companion, the room fails on slots), `companionfollower` (the
gate with a follower and two trainers travelling in from its solid west side,
spawn candidates that can't spawn: the follower rule has 2 free, none to
spare, so no companion and the follower stays out; with the travellers gone
the companion comes out) and `companionpush` (pressing into Blue removes his
companion and interrupts `ace_play` within a frame). `lag` (about 45
minutes, so not part of `--scenario all`: run `--scenario lag` after a change
to the walkers' frame costs) runs four beat
scenes at Viridian frame by frame (Blue's water beats, the player passing
Blue, Brock and Misty greeting, Blue's `ace_play` beside a following
Pikachu), each against a baseline in the same place with the same input and
the same walkers but no beats (the verifier holds every beat's cooldown up
in the ambience block each frame: the selection runs, nothing fires), and
for the record a run with every trainer parked in the Mart. A lag frame is
an emulated frame in which the walkers' frame counter didn't move with the
controls free and no map loading; each one is listed with the beat events
within 3 frames and the walker update of the frame that overran. It passes
when the lag frames other than a companion sheet's decompression (within 3
frames of a `companionPrepares` increment), its knock-on (one idle frame
later in the scene, see "Frame cost") and frames with no beat near them (no
beat running in the 4 frames before, no beat event, icon or effect in the
120 frames before or the 3 after: the beat layer only counted ticks) are no
more than the baseline's. The baseline runs later in the same emulator
session, so engine work driven by emulated time (the time-of-day re-blend)
lines up differently there; frames like these can lag in either run. The
walkers' own presence can lag a busy map frame now and then (a spawn or
follower-rule frame); the baseline counts those too. A manual comparison
with a ROM built with `EnsureAmbience` returning `FALSE` (the beats
compiled out) gave the same lag frames in every scene but the companion
sheets' decompressions.

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
