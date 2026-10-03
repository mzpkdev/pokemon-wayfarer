# Overworld walkers (stage 3: the local actor)

> **Snapshot.** The results and costs below are stage 3's (eight scenarios,
> content hash `0xACD5`). Later rounds re-ran the verifier into the same
> evidence folder, so its JSON files and screenshots now hold the latest run.
> [Current state](#current-state-critic-loop-cycle-2) summarises that run.

Related spec: [Notable world simulation](../specs/notable-world-simulation.md)
and [Notable spots](../specs/notable-spots.md)

Builds on: [Notable trainer travel proof of concept](notable-trainer-travel-poc.md)

## Question

Does the generic local actor (`game/src/wayfarer_walkers.c`) do what the
specs ask on the real ROM? That means spawning from the saved records,
walking with the engine's collision rules, handing off both ways at edges
and doors, keeping one actor across a seam, leaving off-screen trainers to
the heartbeat, playing the spots' behaviour templates, and surviving save
and Continue. It also means doing all that inside the CPU, heap and object
budget.

## Method

`game/tools/wayfarer_walkers/verify.py` is ported from the proof of
concept's verifier. It drives the E2E ROM (`make -C game e2e`) in headless
SkyEmu through Xvfb, with a fresh ROM copy and no save for each scenario.

- **Setup.** Each scenario starts a clean game through the E2E arrange
  mailbox (checkpoint `new-bark-after-intro`, plus a map override).
- **Records.** It writes the world records it needs straight into the saved
  `WayfarerWorldState` in RAM. The ROM publishes that struct's address in
  `gWayfarerWalkersDebug.worldState`.
- **Map loads.** It triggers them with E2E warps. A warp to the same map
  reloads the objects without a heartbeat. Real controller input does the
  following (the edge seam, the Center door, walking in the Gym) and the
  Bag round trip.
- **Evidence.** Symbol reads of the records, `gObjectEvents`, the
  local actor block and `gWayfarerWalkersDebug`, plus screenshots.

```sh
make -C game -j16 CXX=g++ e2e
python3 game/tools/wayfarer_walkers/verify.py \
  --rom game/pokemon-wayfarer-e2e.gba --symbols game/pokemon-wayfarer-e2e.sym \
  --output .product/research/overworld-walkers   # --scenario NAME for one
```

The run below used the E2E ROM built from this branch's working tree on
2026-10-02, with content hash `0xACD5`. Blue (slot 8), Lorelei (9), Lance
(10) and Brock (0) are the test trainers.

## Results

All eight scenarios pass ([summary](overworld-walkers/summary.json)).

| Scenario | What it proves | Evidence |
| --- | --- | --- |
| **spot** | Blue, travelling with a door arrival, spawns at the first free tile beside Viridian's Center door, (30, 37). He walks 407 search nodes' worth of path to the water's edge spot (13, 39), arrives (record Dwelling, `arrivals` 1), faces north, and plays Stand and face with its "!" emote. Mid-walk, the player opens the Bag and closes it. The heap reset (`heapResets` +1) drops the search workspace, and he re-plans (`searches` 2) and still arrives. | [json](overworld-walkers/spot.json), [arrived](overworld-walkers/spot-arrived.png), [template "!"](overworld-walkers/spot-template.png), [after the Bag](overworld-walkers/spot-after-bag.png) |
| **edge** (acceptance 2) | Blue leaves Viridian's north edge at (26, 0). The record commits as the exit step starts: Route 2 south node 88, arrival south, crossing **10 = 6 + (26 − 22)**, the lane offset. He keeps walking in the connection strip while the player is still in Viridian. The player then walks north across the seam. Over 34 two-frame samples, exactly **one** Blue object exists and his track never jumps more than one tile in Route 2 coordinates. He is rebased (`rebases` 1, no respawn) and walks on (y 78 to 74 in 90 frames). | [json](overworld-walkers/edge.json), [in the strip before crossing](overworld-walkers/edge-strip-before-cross.png), [after](overworld-walkers/edge-route2-after-cross.png), [walking on](overworld-walkers/edge-route2-moving.png) |
| **door** (acceptance 3) | Blue walks into the Viridian Center door; `warpExits` +1, then the actor is removed. The player follows with real input. Inside, Blue enters at (6, 8), beside the landing mat (the player is on (7, 8)), in the walking phase, and walks to the counter spot (6, 4). | [json](overworld-walkers/door.json), [outside](overworld-walkers/door-outside-after-entry.png), [walking in](overworld-walkers/door-inside-walking.png), [at the counter](overworld-walkers/door-inside-at-counter.png) |
| **linger** (acceptance 4) | Four E2E warps between the Center and the Mart give heartbeats 1 to 5. Off-screen Lorelei hops once per heartbeat: Route 1 (83), Viridian (67), Route 2 (88), where she arrives. Her dwell then runs 3 to 2 to 1. Blue, dwelling in the Mart with dwell 1, keeps identical record bytes at the heartbeat that brings the player into the Mart. At the next one, with the player gone, his routine advances; later, off-screen again, he hops to Viridian. | [json](overworld-walkers/linger.json), [Mart](overworld-walkers/linger-mart-blue-stays.png) |
| **save** (acceptance 10) | Blue dwells at the water's edge and the E2E save command writes flash. Then a GBA reset chord, the normal boot, and Continue with real input. The 200 record bytes and the header are identical (Blue `43d0d01000a00600` before and after). The local actor block held `{slot 8, (13, 39), north}`, and Blue is back at (13, 39), adopted from the restored objects (`restores` 1). The fresh session shows **0 heartbeats**, so Continue doesn't advance the world. | [json](overworld-walkers/save.json), [before](overworld-walkers/save-before.png), [after Continue](overworld-walkers/save-after-continue.png) |
| **gym** | With the Boulder Badge held, Brock is dwelling at home in Pewter Gym and his own object (graphics 464, at (6, 5)) is shown. With Brock out in Pewter, his object is hidden on entry, and stays hidden while the player walks the Gym, so scrolling doesn't respawn it. Blue, a visitor at the Gym spot, spawns on the nearest approach tile (6, 13), off the player's tile. He walks out through the exit warp, and his record moves on to Pewter with a new destination. He doesn't come back during that entry. | [json](overworld-walkers/gym.json), [leader home](overworld-walkers/gym-leader-home.png), [visitor leaving](overworld-walkers/gym-visitor-just-leaving.png), [leader out](overworld-walkers/gym-leader-out.png) |
| **budget** (acceptance 8) | Three actors on Viridian at once: Lorelei wanders a square, Lance walks to the water, Blue heads out north. Never more than 3 actors, and never more than **one** search running in any sampled frame. Lorelei's Wander made 5 template moves over 18 area tiles. | [json](overworld-walkers/budget.json), [three actors](overworld-walkers/budget-three-actors.png) |
| **browse** | Blue browses the Viridian Mart: shelves 2250, then 2265, then 2254, every 8 dwell ticks by the deterministic `(c + 7k) mod n` choice. Local dwell runs 2 heartbeats while watched (`localDwellBeats` 2). At 0 his routine advances while watched, and he walks out of the door on foot (record in Viridian, door arrival). | [json](overworld-walkers/browse.json), [next shelf](overworld-walkers/browse-next-shelf.png) |

## Costs

| Measure | Value |
| --- | --- |
| Worst search slice | **168 scanlines** (0.74 of a frame), door scenario; 137-147 on outdoor maps. VCOUNT-based, interrupts included. |
| Expansions per slice | 8 |
| Largest search | 999 nodes in 149 frames (Viridian to the north lane); 957 nodes in 181 frames with three actors queued |
| Heap workspace | **5,564 bytes**: one shared workspace, dropped on heap resets. A 7,420-byte core workspace is also allocated and freed within one call (`WorldSim_NextEdge`, spawn priority). |
| Static RAM | **368 bytes of EWRAM** (`wayfarer_walkers.o` `.sbss`: the 128-byte debug block, four 40-byte actors, search state, Continue restore, recent entries), **0 IWRAM** |
| ROM code | 14,428 bytes (`.text`) |
| Saved state | None added; the existing 9-byte local actor block |

## Limits

- **Not shown in the emulator:** the following-Pokémon rule (no party in
  the fixtures, and no map crowded enough to need it), the strip actor
  walking *into* the player's map when the player overtakes it, a Browse
  floor change (Viridian Mart has one floor), transit edges, the "…" emote,
  story-scene suppression, Continue through a continue-game warp, and
  actors keeping still while a talk or script locks the field controls.
  Each is implemented; the Bag round trip shows the AI and dwell stay
  suspended while the controls are locked.
- **Records written directly.** Scenarios arrange records directly in RAM.
  They exercise the walkers against valid records, not the routine choices
  that would produce them.
- **Reads can race.** Some reads land mid-frame. The edge samples take
  position and map from the walker debug block, which the ROM publishes
  together once per field frame. Object coordinates shift later in the same
  frame, and the grid search probes coordinates on the real object mid-frame.
- **Not tested:** physical hardware and a cold emulator restart. Like the
  E2E harness, the save check uses the in-game reset.

## Current state (critic loop cycle 2)

The evidence folder holds the run of 2026-10-03 on the E2E ROM built from
commit `2abf8fd183`, content hash `0x9F76` (the verifier now refuses a ROM
whose hash differs from the worktree's tables). All **22** scenarios pass
([summary](overworld-walkers/summary.json)); `perf`, `seam`, `recross` and
`longtrip` fail past ceilings of 200 scanlines in a frame and 44 frames per
warp.

| Measure | Value |
| --- | --- |
| Grid search workspace | 11,308 bytes of heap (per-tile moves and elevations, 64 bridge states, queue, paths) |
| Trip search workspace | 9,204 bytes of heap, only while a watched trainer's first trip search runs |
| Heartbeat workspace | 9,204 bytes of heap while a heartbeat is pending |
| Static RAM | 1,649 bytes of EWRAM (simulation 964, walkers 492, world 193), 0 IWRAM |
| Worst frame of simulation | 188 scanlines on warps, 144 on seams, 150 re-crossing a seam (660 finished in one seam frame with the queue full) |
| Frames per warp | 40.2 mean, 42 max (main: 40) |
| Saved state | 216 bytes; Blue's record in `save` is `5dd0261100a00600` |
