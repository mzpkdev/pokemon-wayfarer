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

### Seam visibility follow-up

Player testing exposed a gap in the initial Phase 2 proof: Viridian's connected
Route 2 tiles remain visible before the player changes maps. The POC removed
Giovanni as soon as he finished his exit step, then spawned him again when the
player crossed. A valid saved destination did not guarantee visual continuity.
The new regression observes individual frames on both sides of that boundary,
including the interval while the player is still in Viridian.

The baseline regression reproduced an actor removal at sampled frame 94 with
sprite bounds still at screen `(144,40)`; screenshots show Giovanni present at
frame 95 and absent at frame 96. The delayed-follow case also found that his
saved Route 2 position did not advance while the player waited. These captures
are retained in `/tmp/viridian-seam-baseline/verifier-delayed`.

The fix retains the existing object and sprite in the loaded connection strip,
continues its normal walking animation with engine collision checks, and saves
its position in the neighboring map's coordinates. When the player crosses,
the engine rebases the object's coordinates and the POC changes its map/local
identity without replacing its sprite. It finishes any held step before
starting the destination map's BFS. The same mechanism handles the player
crossing ahead of an approaching trainer: his actual goal determines whether
he walks toward or away from the player's map.

Camera transitions still count as heartbeats, but a visible connected actor
cannot advance through the abstract graph. Ordinary interior transitions keep
the Phase 2 heartbeat behavior. Offscreen actors retire after their animation;
recovery can reconstruct a visible border actor from saved coordinates. The
save record and save version are unchanged. The fix adds **four static EWRAM
bytes**, no static IWRAM, and no additional search workspace. Normal/E2E EWRAM
usage is now 249,564/255,472 bytes respectively; IWRAM remains 25,644/25,652.

The regression captures every emulated frame around the transition, including
object identity, sprite bounds, saved coordinates, and screenshots. It covers
following immediately, waiting 60 frames, waiting 160 frames until Giovanni
leaves the viewport, bouncing across the boundary, and entering Viridian ahead
of him on his return trip. The return test first follows him into the Forest
gate, then uses two actual boundary transitions to release his gate dwell;
there is no test warp or memory arrangement.

```sh
python3 game/tools/viridian_walker_poc/verify.py \
  --rom game/pokemon-wayfarer-e2e.gba --symbols game/pokemon-wayfarer-e2e.sym \
  --output /tmp/viridian-seam-prompt --phase2 seam --seam-follow prompt --seam-bounce
# Repeat with --seam-follow delayed or late, omitting --seam-bounce.
# Use --phase2 seam-return for the incoming/overtake case.
```

This remains a bounded POC: the border actor walks through the loaded connection
strip and waits if collision blocks it. Full goal pathfinding resumes when the
player enters its map; it does not search an entire unloaded neighbor. The
runtime seam checks cover Viridian/Route 2. Route 1 shares the implementation
but was not separately exercised at its border.

All six final checks passed against the rebuilt ROM: delayed, prompt with
bounce, late, return/overtake (including the full outbound gate walk), linger,
and save/reset/Continue. The four seam recordings contain 190, 200, 290, and
114 individually sampled frames respectively, with no reported visibility gap,
duplicate, or position jump. The delayed and bounce runs retain the same
object slot and sprite throughout. See [results and per-frame tables](viridian-walker-poc-seam/results.json),
[build/source hashes](viridian-walker-poc-seam/validation.json),
[visible before player crossing](viridian-walker-poc-seam/fixed-visible-before-player-cross.png),
[during crossing](viridian-walker-poc-seam/fixed-during-player-cross.png), and
[Giovanni returning into Viridian](viridian-walker-poc-seam/fixed-return-cross.png).
Raw full-frame PNG sequences are under `/tmp/viridian-seam-final-*`; the rebuilt
normal ROM remains `game/pokewayfarer.gba`.

### Followable indoor stops

The Forest south gate is now a visible stop. Giovanni walks to `(7,5)`, pauses
for about five seconds, turns south, and walks back through the Route 2 door.
His return itinerary visits Viridian's Pokémon Center and Poké Mart in the same
way, at `(6,5)` and `(5,5)`, before continuing to Route 1 and back north. The
initial green-strip/Route 2 walk is retained. He does not traverse the Forest,
heal Pokémon, or buy items in this POC.

The changes are in `viridian_walker_poc.c/.h`, `viridian_walker_world.c/.h`,
the three interiors' `map.json` and `scripts.inc`, and the SkyEmu verifier.
Each interior adds one hidden Giovanni object template and goal dialogue.
The existing POC switch still controls actor/world code; templates remain
hidden when disabled. No map tile binaries changed. The follower stays disabled.

Destination map now encodes the loop's next stop: gate → Center → Mart →
Route 1 → gate. Position, goal, state, and remaining pause fit in the existing
**12-byte saved record**, with no save-version change. A pause unit represents
two active field frames. The local actor restores its indoor approach, pause,
or exit goal after a reload. Off-screen progress still uses map-transition
heartbeats, consuming 75 pause units per eligible transition. Leaving a building
ahead of Giovanni can therefore abstract away his remaining unseen walk;
there is no continuous off-screen clock.

Door transitions needed two specific fixes. Fresh spawning waits until player
controls unlock, so the engine's forced door step can finish. Outdoor return
positions are one tile east of the player's landing: Route 2 `(6,52)`, Center
`(31,37)`, and Mart `(41,28)`. Recovery checks occupancy before placement.
The earlier gate-return coordinate in this report is superseded by `(6,52)`.
Indoor searches retry the same goal with a 30-frame delay when blocked, rather
than silently skipping the visit or despawning at an occupied exit.

Testing also exposed a real dialogue bug: `faceplayer` unfreezes its object as
part of a held movement, so testing only the object's `frozen` bit let the pause
timer continue. The POC now suspends its AI while player field controls are
locked; engine sprite callbacks still finish held animations. All three final
dialogue checks retain identical saved world bytes over 120 locked frames.

Both builds pass. Static memory remains **249,564 EWRAM / 25,644 IWRAM bytes**
for the playable ROM and **255,472 / 25,652** for E2E: no increase from the seam
fix. The single **11,200-byte BFS heap workspace** is reused indoors. Searches
remain capped at eight expansions per update. Across the final recorded runs,
the largest observed slice was **144 scanlines**, approximately **10.57 ms /
0.63 GBA frame**; the outbound Route 2 search recorded 413 nodes and 64 elapsed
VBlank frames. These are observations, not worst-case guarantees. The normal
ROM uses 32,096,024 bytes before padding.

Final SkyEmu validation passed on frozen source and matching ROM/symbols:

- Gate: follow inside, visible approach/pause/dialogue/turn/exit, follow outside,
  and observe Giovanni moving again on Route 2.
- Center and Mart: follow both visits and both exits, with outdoor movement
  continuing toward the next destination. During a **360-frame** Mart doorway
  blockage, Giovanni stayed visible inside and searches increased from 8 to 17;
  clearing the doorway let him leave normally.
- Save indoors: use the real Save menu, GBA reset, and Continue. Flash reload
  restores the exact record `000c010c05011e06050e9200`; Giovanni reappears at the
  Center stop with the remaining pause, which resumes ticking.
- Seam regressions: immediate following with a return crossing, plus Giovanni's
  southward return while the player crosses first. The respective 201- and
  118-frame recordings report no visibility gap, duplicate, or position jump.

Reproduce with the build commands above and `verify.py --phase2 interior-gate`,
`--phase2 interior-city`, and `--phase2 interior-save`, each with explicit
`--rom`, `--symbols`, and a separate `--output` directory. The existing
`--phase2 seam --seam-follow prompt --seam-bounce` and `--phase2 seam-return`
cover the borders. These use fresh games and real controller input, without
test warps or memory writes. The gate approach avoids Route 2 grass because
wild encounters can otherwise interrupt the test player's walk. Cold emulator
process restart remains outside the save claim, as explained above.
The indoor save check covers the Center pause; Bag return and saves during the
approach or exit were reviewed in code, not separately exercised in SkyEmu.

[Results and compact traces](viridian-walker-poc-interiors/results.json),
[source/build hashes](viridian-walker-poc-interiors/validation.json),
[gate turn](viridian-walker-poc-interiors/gate-turn.png),
[Center dialogue](viridian-walker-poc-interiors/center-dialogue.png),
[Mart blockage](viridian-walker-poc-interiors/mart-blocked.png), and
[restored indoor actor](viridian-walker-poc-interiors/saved-center.png)
retain the evidence. Full raw runs are in `/tmp/viridian-interiors/final-*`.
The playable build is `game/pokewayfarer.gba`.

**Verdict:** followable indoor haunts are viable with the same small actor and
search budget. The main remaining scaling risks are authoring valid indoor
spots and door approaches, arbitrating cramped entrances for several actors,
and replacing the transition-driven off-screen timing with the intended world
schedule. The current stops and return offsets are deliberately limited to
these three buildings.
