# Viridian walker proof of concept

The sections before **Phase 2** record the original six-stop experiment. The
current branch runs the Phase 2 itinerary; use its validation commands below.

This experiment tests one autonomous notable trainer on the 56×50 HNS Viridian
map. It is disposable branch-only code, controlled by `VIRIDIAN_WALKER_POC` in
`game/include/config/viridian_walker_poc.h`. Giovanni completed all six goals in
headless SkyEmu, including a real player obstruction and dialogue. The approach is viable for one local roaming trainer.

## Build and run

`game/README.md` was empty in the starting revision. The Makefile and
`e2e/README.md` establish the Wayfarer E2E target, which is a playable ROM:

```sh
make -C game -j16 CXX=g++ e2e
pnpm install --frozen-lockfile --ignore-scripts --store-dir /tmp/viridian-pnpm-store
python3 game/tools/viridian_walker_poc/verify.py \
  --rom game/pokemon-wayfarer-e2e.gba \
  --symbols game/pokemon-wayfarer-e2e.sym \
  --output /tmp/viridian-walker-evidence --min-goals 6 --lifecycle
```

Build map variants serially because they share generated files. `CXX=g++` works
around this machine's unavailable `g++-11` default. The verifier uses the pinned
`skyemu-static` binary through Xvfb, an isolated ROM copy, and no existing save.
It needs the Linux libraries listed in `e2e/README.md` and waits for SkyEmu’s
HTTP-ready message. Simulation observations advance emulated frames.
The normal product target is `make -C game -j16 CXX=g++ wayfarer`, producing
`game/pokewayfarer.gba`. Both targets build successfully.

In an emulator, select **NEW GAME**. The POC skips the speech, appearance picker,
and origin story, creates GOLD with a New Bark origin, and places the player at
Viridian `(30,37)`. Standard new-game initialization still runs. The existing
New Bark opening callback is a black-screen fade; Viridian has no New Bark
origin event. Giovanni starts on the sidewalk at `(32,37)`.

## Scope and map findings

- `game/src/main_menu.c` bypasses the new-game speech under the POC switch.
- `game/src/new_game.c` overrides the first warp and recovery location and sets
  the existing saved follower preference to disabled. A temporary event flag
  would be cleared on map load. The player can still re-enable the follower in
  the normal menu; leave it disabled for this experiment.
- `game/src/viridian_walker_poc.c` owns the trainer, goal selection, and search.
  The public header exports bounded telemetry for the verifier.
- `game/src/overworld.c` calls the POC once per field update;
  `game/src/event_object_movement.c` exempts this actor from off-camera removal.
  The map's `map.json` and `scripts.inc` add Giovanni and his goal-specific text.
- `game/src/malloc.c` invalidates the POC workspace when the engine resets its
  heap. The next field update allocates again and restarts the tour. Menus and
  warps reset this engine's heap; retaining the pointer would corrupt memory.
  `game/src/data/wayfarer_sevii_maps.json` updates the existing audit fingerprint
  for the changed movement source; the audit remains enabled.
- `game/tools/viridian_walker_poc/verify.py` drives the actual ROM with controller
  input and reads its symbols; it does not change the shared E2E mailbox ABI.

A read-only decode of all 2,800 map entries found **zero native tall, long, or
short grass tiles**. The pale green strips have normal behavior. The HNS primary
tileset does contain real tall-grass metatiles, but the user chose to preserve
the map and use a decorative green strip for the pause and exclamation activity.
The runtime scan reports zero grass and explicitly marks this fallback. No
static or runtime metatile patch is applied; no `map.bin` is edited.
Mart and Center destinations are identifiable from warp events; Route 1, Route
2, and Route 22 are explicit map connections.

## Movement and limits

The deterministic rotation is green strip → Mart → Center → Route 1 → Route 2
→ Route 22. A bounded BFS expands at most eight nodes per field update across the
2,800-tile grid. Each edge uses `GetCollisionAtCoords` with a scratch object at
the hypothetical location and elevation. Every real step checks collision again
before using normal held walking movement. A blocked step starts another search;
four blocked steps abandon the goal, as does an exhausted search with no path.
This is walking-only; it does not jump ledges, surf, or navigate interiors.

Door warp tiles are collision-blocked for NPCs. Search targets the tile directly
south of the discovered door; a final scripted walking step enters that known
door while retaining the object-occupancy check. Visits last about three seconds.
An invisible object still collides in this engine, so the POC parks it outside
the playable grid during visits. It retains one object slot, including while
hidden. Route visits walk one tile beyond the map boundary and later animate
back in through another connected edge. A connection alone does not guarantee a
walkable border: the north edge includes Route 2 wall tiles. Exit selection and
reentry check the actual neighboring tiles with the engine’s collision rules.
Interiors and travel between maps are simulated; the player never changes map
on Giovanni's behalf.

The green-strip activity pauses for two seconds and shows the normal “!” emote.
Talking freezes the actor through the engine's normal script lock, displays his
current destination, then releases him. Setting the POC define to `0` restores
the normal new-game flow and hides the extra event on map load. Use a fresh save
when comparing builds: the POC's chosen origin and follower preference are saved.

## Measurements and validation

The linked E2E ROM uses 255,428 bytes EWRAM and 25,652 bytes IWRAM; the normal
Wayfarer ROM uses 249,520 and 25,644 bytes respectively. The POC adds **68 bytes
of static EWRAM and no static IWRAM** versus the startup-only E2E baseline
(255,360 EWRAM / 25,652 IWRAM).
Its BFS workspace uses **11,200 bytes of the existing 115,968-byte EWRAM heap**
(9.7%), plus a 16-byte allocator header. The fixed queue and predecessor arrays
bound searches to 2,800 visited tiles; eight expansions per update means at most
350 search slices. Temporary C stack use is additional, not a static IWRAM array.

The full tour used eight expansions per slice. Completed routes took 1–364
expanded nodes and 2–56 elapsed VBlank frames per search; the initial 27-frame
search includes the field fade. Example searches were Mart 176 nodes/27 frames,
Center 312/43, and Route 1 300/42. Route 2 and Route 22 needed one node each after
reentering at their edge. These are observed routes, not a worst-case bound.
The largest measured slice was 127 scanlines, about **9.33 ms / 0.56 GBA frame**.
That is a substantial share of one frame, so several independent walkers cannot
safely use this budget simultaneously without further measurement.
Timing uses VCOUNT plus the VBlank counter and includes interrupt time; it is
scanline-granular, not a cycle profile. The earlier 48-node slice crossed two
VBlanks, so the final budget was reduced to eight.

The fresh-save SkyEmu run passed in 3,412 simulated frames after startup (about
57 seconds), then passed map/menu recovery. It observed all six destinations,
three travel reentries, a real player obstruction increasing both blocked-step
and replan counters from 0 to 1, and the Center dialogue. After that obstruction,
Giovanni skipped the temporarily unreachable Center and completed it on the
next cycle. The player entered and left the actual Center, then opened the Bag,
closed it and the Start menu, and Giovanni continued moving in both cases.
The extra recovery searches included fade/menu waits (64 and 78 elapsed frames);
those are not CPU search costs.

[Results](viridian-walker-poc/results.json) and
[build/source hashes](viridian-walker-poc/validation.json) accompany the report.
Screenshots show [fresh Viridian spawn](viridian-walker-poc/viridian-start.png),
[goal dialogue](viridian-walker-poc/walker-interaction.png), and
[the exclamation after Bag recovery](viridian-walker-poc/lifecycle-walker-after-menu.png).
Raw captures/logs are in `/tmp/viridian-walker-complete`; build logs are
`/tmp/viridian-e2e-build.log` and `/tmp/viridian-wayfarer-build.log`.
Both builds exit successfully; existing assembler warnings remain. The disabled
POC module also compiled to zero code/data, but a full disabled-ROM startup and
physical GBA hardware were not exercised.

## Verdict and remaining risks

**Viable for one trainer’s local haunts and simulated travel.** The normal ROM
builds, the actor uses normal walking animation, and collision-based searches
handle the player dynamically. This experiment does not establish a budget for
many simultaneous trainers or persistent travel across unloaded maps.

The largest risks are search latency, the engine’s global heap resets, and the
16-slot object pool. Giovanni reserves one slot even off-camera or hidden; the
POC disables the follower to reduce pressure. No slot exhaustion occurred in the
validated route, but crowded maps and re-enabling the follower need separate
work. Menus and map transitions intentionally restart Giovanni’s tour.

Doors require one explicit collision exception and assume a south-facing
approach. Exit selection must inspect connected-map border collision, elevation,
and occupancy. Walking respects the engine’s ledge and water restrictions but
has no jump or surf planner; no exhaustive elevation/ledge test suite was added.
For a larger haunts system, share a search workspace and scheduling budget,
measure several active actors, and model persistent travel separately.

## Phase 2: saved travel between maps

Phase 2 replaces the six-stop local demonstration above with a saved itinerary:
Viridian’s green strip → Route 2’s southern section → the southern Forest gate,
then back through Viridian toward Route 1. The gate is the destination;
Giovanni does not traverse the Forest in this slice.

The world layer (`viridian_walker_world.c/.h`) adds a 12-byte record to
SaveBlock3: current and destination map IDs, arrival kind and crossing coordinate,
local x/y, state, goal, and dwell count. Its states are travelling, at a spot,
and inside a building. POC builds use save version 11; the disabled POC retains
version 10. Start a fresh Phase 2 game; there is no prerelease save migration.

Heartbeats run from `LoadMapFromCameraTransition` and `LoadMapFromWarp` after
the player’s new map is known. One eligible tick advances an off-map traveller
one graph edge. The gate adds two heartbeat ticks of dwell. A trainer on the
player’s newly loaded map stays there, allowing the player to follow. Continue
initializes the map observation without advancing the simulation; opening the
Bag is also not a travel heartbeat. This is transition-driven world time, not
continuous movement or real-time travel on unloaded maps. If the player leaves
while Giovanni is walking to a local activity, the off-map abstraction can skip
the remaining local walk and advance one graph edge at the next heartbeat.

The graph is `Route 1 ↔ Viridian ↔ Route 2 south ↔ Forest south gate`.
A read-only collision/elevation flood found Route 2’s north and south components
separated by solid rows 41–45. Although its northern boundary connects to
Pewter, Giovanni cannot walk there from the southern entrance. The middle-gate
bypass requires removing a Cut tree at `(15,69)`. The selected Forest gate is
reachable without Cut; the static flood found a 43-step path from `(9,79)` to
`(6,51)`. This flood approximates movement; local runtime paths use the engine’s
full collision checks.

Connection crossings use source-map offsets: Viridian north `(x,0)` becomes
Route 2 `(x−16,79)`, and the return becomes Viridian `(x+16,0)`. The walkable
lanes are Viridian x=22–27 / Route 2 x=6–11. Viridian south uses Route 1’s offset
of 2. Route 2 warps `(5,51)` and `(6,51)` enter gate warp 0 at `(7,9)`; the reverse
warp returns to `(5,51)`, with the outdoor actor resuming at `(5,52)`.

Future graph generation should extract connections and warp pairs, then check
reachability between their entrances using NPC collision, elevation, and ability
rules. Map-level adjacency alone misses disconnected regions, Cut trees, and
interior gatehouses. A graph node may need to represent a walkable region within
a map rather than an entire map.

The local actor still uses one 11,200-byte heap workspace, now indexed by the
current map dimensions: Viridian 56×50, Route 2 30×80, and Route 1 50×40. It
reports each completed step to the saved record. Heap resets discard the search
workspace; menus and map reloads restore the actor from saved position and plan
again. They no longer restart the itinerary. Connection handoffs commit when
the held exit step starts, so a player crossing immediately cannot outrun the
saved handoff. Cleanup identifies the actor by map and local ID, including the
old-map object retained during a scrolling connection transition.

The integration touches `new_game.c`, `overworld.c`, `global.h`, and `save.h`,
plus the POC actor and new world module. Route 1 and Route 2 receive Giovanni
object events and goal dialogue in `map.json`/`scripts.inc`; template events
stay hidden until the simulation places Giovanni there. The follower remains
disabled, and only one Giovanni object is active. No map tile binaries changed.

Build and exercise the current itinerary with:

```sh
make -C game -j16 CXX=g++ e2e
python3 game/tools/viridian_walker_poc/verify.py \
  --rom game/pokemon-wayfarer-e2e.gba --symbols game/pokemon-wayfarer-e2e.sym \
  --output /tmp/viridian-phase2-follow --phase2 follow
python3 game/tools/viridian_walker_poc/verify.py \
  --rom game/pokemon-wayfarer-e2e.gba --symbols game/pokemon-wayfarer-e2e.sym \
  --output /tmp/viridian-phase2-linger --phase2 linger
python3 game/tools/viridian_walker_poc/verify.py \
  --rom game/pokemon-wayfarer-e2e.gba --symbols game/pokemon-wayfarer-e2e.sym \
  --output /tmp/viridian-phase2-save --phase2 save
make -C game -j16 CXX=g++ wayfarer
```

Both targets build successfully. The E2E image uses 255,468 bytes EWRAM and
25,652 bytes IWRAM; the normal Wayfarer image uses 249,560 and 25,644 bytes.
Against Phase 1, the E2E build adds **40 static EWRAM bytes**, including the
**12-byte saved record**, and no static IWRAM. The BFS heap workspace remains
11,200 bytes. The measured Route 2 search expanded **401 nodes in 63 elapsed
VBlank frames**, including transition scheduling, with at most eight expansions
per update. The largest observed slice in the follow run was **141 scanlines**,
about **10.35 ms / 0.62 GBA frame**. These are observed costs, not a worst-case
bound; the Phase 1 scheduling caveat still applies.

The final follow run crossed Giovanni from Viridian `(25,0)` to Route 2 `(9,79)`.
The player followed at `(10,79)`, saw exactly one Giovanni at `(9,79)`, and saw him
step to `(9,78)` within 30 frames. He then reached the Forest gate after another
690 frames. The local-actor exit counter increased from one to two while the
heartbeat count stayed at one and abstract hops stayed at zero. No duplicate
Giovanni appeared, and the actor was removed after entering the gate.
[Follow telemetry](viridian-walker-poc-phase2/follow-results.json),
[arrival screenshot](viridian-walker-poc-phase2/follow-route2-giovanni.png), and
[moving screenshot](viridian-walker-poc-phase2/follow-route2-moving.png) capture
this handoff. The gate entry is verified by actor/world telemetry; the player
remained near Route 2's southern edge during that part of the test.

The linger run used four actual Center transitions: gate with two dwell ticks,
gate with one, Route 2 at `(5,52)`, then Viridian at `(24,0)` with north arrival.
It observed an active Viridian Giovanni object at that matching north edge.
The player stayed near the Center, so the re-entry evidence is
[object and saved-state telemetry](viridian-walker-poc-phase2/linger-results.json),
not an on-camera sighting at the north boundary.

The first save/reload attempt exposed a SkyEmu harness issue: killing the
process immediately after accelerated stepping left its host `.sav` with only
four complete sectors and part of the fifth, despite the in-game success text.
The existing E2E harness documents this non-atomic host flush. The POC verifier
therefore uses the same GBA reset chord and normal boot/Continue path to read
serialized emulated flash. This does not use a savestate or write game memory.
A cold host-process restart is not claimed as validated.

The save test passed: the reset counter dropped from 2,643 to zero, normal boot
reported a valid flash save after 240 frames, and Continue restored the player
to the Center. Giovanni's exact 12 saved bytes were
`021729000502060709040200` before save, after boot, and after Continue: inside
the Forest gate, destination Route 1, door arrival, two dwell ticks remaining.
Continue did not advance world time.
[Save/reload telemetry](viridian-walker-poc-phase2/save-results.json),
[save success](viridian-walker-poc-phase2/save-success.png), and
[restored Center](viridian-walker-poc-phase2/reload-center-after-continue.png)
record the check. The screenshots show the player's save/restore; Giovanni is
inside another building and is verified through the saved record.

[Build/source hashes](viridian-walker-poc-phase2/validation.json) identify the
tested binaries. Raw scenario captures remain in `/tmp/viridian-phase2-*-final`;
build logs are `/tmp/viridian-phase2-e2e-build.log` and
`/tmp/viridian-phase2-wayfarer-build.log`. The failed cold-restart attempt's
[sector audit](viridian-walker-poc-phase2/host-flush-diagnostic.txt) records the
host-file flush limitation separately.

**Updated verdict:** the two-layer design is viable for this slice. A saved
12-byte world record can drive abstract travel and restore one visible actor
at the correct connection or door. The main scaling risks are the local BFS
frame budget, limited object slots, and building a graph of walkable regions
rather than merely connected map names. Door orientation and gatehouse routing
need explicit semantics. This POC does not establish continuous off-screen
travel, full-Forest traversal, many simultaneous trainers, or physical-hardware
performance. Route 1 is implemented in the itinerary but its complete local
round trip was not part of the three required emulator scenarios.
