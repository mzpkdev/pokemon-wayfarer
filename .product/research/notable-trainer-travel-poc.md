# Notable trainer travel proof of concept

Related PRD: [Notable haunts](../prds/notable-haunts.md)

Related spec: [Notable haunts](../specs/notable-haunts.md)

Superseded by spec: [Notable world simulation](../specs/notable-world-simulation.md)

Evidence: closed draft PR
[#146](https://github.com/mzpkdev/pokemon-wayfarer/pull/146), branch
`task/viridian-walker-poc`. The branch's
`.product/research/viridian-walker-poc.md` holds the full report,
telemetry, and screenshots.

## Question

Can a notable trainer live in the overworld on this GBA engine? That breaks
down into:

1. Can they pick goals and walk a single map with real pathfinding and a
   small AI?
2. Can they actually travel between maps, so that following them or waiting
   for them behaves consistently?

Notable haunts, and the routines being designed on top of them, depend on
both answers.

## Method

A disposable proof of concept on a separate branch, behind the
`VIRIDIAN_WALKER_POC` switch. A new game starts the player in Viridian City,
beside Giovanni. Everything was built and validated by driving the real ROM
in headless SkyEmu with controller input and symbol reads.

- **Phase 1: one map.** Giovanni tours Viridian City: a green strip, the Poké
  Mart, the Pokémon Center, and the Route 1, Route 2 and Route 22 exits.
  - Goals come from tile behaviours, warp events and map connections.
  - A bounded breadth-first search runs over the map grid, using the engine's
    own collision and elevation checks (`GetCollisionAtCoords`), at eight
    expansions per field update.
  - He walks tile by tile with normal movement and re-plans when blocked.
  - Talking to him names his current goal.
- **Phase 2: travel.** A saved world record per trainer (12 bytes in
  SaveBlock3) holds:
  - current and destination map;
  - how he arrived (edge or door) and the crossing coordinate;
  - local position, state, goal, and dwell.

  Other pieces:
  - **Heartbeat:** runs on map loads from camera transitions and warps. A
    trainer not on the player's map hops one edge of a small hand-written
    graph: Route 1 ↔ Viridian ↔ Route 2 south ↔ Viridian Forest gate.
  - **Local actor:** the phase 1 walker spawns him from the record and
    reports exits back to it.

## Findings

**Viable.** Both questions answer yes, for one trainer.

- **Local AI and pathfinding.** All six Viridian goals were reached. When the
  player really did block him, the blocked-step and re-plan counters rose and
  he recovered. He also carried on after the player went in and out of the
  Center and through menus.
- **Following.** Giovanni left Viridian's north edge at (25, 0) and appeared
  on Route 2 at the matching tile (9, 79), computed from connection offsets.
  The player followed, saw exactly one Giovanni, and saw him step on within
  30 frames. He then walked on to the Forest gate and went inside. The
  handoff commits when the exit step starts, so a player crossing at once
  cannot outrun it.
- **Lingering.** Four Center door transitions advanced him off-screen: two
  waits at the gate, back down Route 2, then re-entry into Viridian at the
  north edge he left by. This is shown by saved state and object telemetry,
  not seen on camera.
- **Save and reload.** His 12 bytes were identical before saving, after a
  reset, and after Continue. Continue does not advance world time.

## Costs

| Measure | Value |
| --- | --- |
| Saved state per trainer | 12 bytes (SaveBlock3) |
| Static RAM added | about 40 bytes EWRAM, no IWRAM |
| Search workspace | 11,200 bytes of the EWRAM heap, one search at a time |
| Search cost | Mart 176 nodes in 27 frames; Route 2 401 nodes in 63 frames (elapsed, split into slices) |
| Worst observed slice | about 0.62 of a GBA frame (141 scanlines) |

These are observed costs, not worst-case bounds.

## Constraints discovered

1. **A map is not one place for walkers.** Route 2 is cut in two by solid
   rows 41–45, and its middle gate needs Cut, so the south end cannot reach
   Pewter on foot. A travel graph needs **walkable regions within maps**,
   found by checking reachability with NPC collision, elevation and ability
   rules. Which maps connect is not enough.
2. **Gatehouses, Cut trees and unwalkable connection borders** are real
   edges and blockers. Route 2's north border has wall tiles, for example.
3. **Heartbeat time is map-change time.** If the player stands still, the
   world does not move. A trainer inside a building cannot come out on their
   own while the player waits outside. That needs a gameplay timer, which
   ties back to the planned in-game clock.
4. **The engine heap resets on menus and warps.** Search workspaces must be
   dropped and rebuilt; the actor then re-plans from its saved position.
5. **Object slots.** The pool is 16. A walker keeps one slot even while
   hidden or away, and invisible objects still collide. The Pokémon that
   walks behind the player was disabled for the proof of concept.
6. **Doors** are blocked for NPCs by default and need one explicit
   exception. Door approaches were assumed south-facing.
7. **CPU budget.** Slices near 0.6 of a frame mean several walkers on one
   map need a shared search schedule, not each running their own.
8. **Walking only.** There is no planning for ledge jumps, Surf, or
   interiors beyond a door.

## Implications for the design

- The **two-layer model** holds: a saved world simulation with a heartbeat,
  plus a local actor on the player's map, with a handoff on exits and
  spawns.
- The **per-place cap** of two or three notable trainers fits the object
  pool and the CPU budget.
- **Haunts serve as waypoints and destinations** for routing; arriving means
  being at a haunt.
- **Next problems,** in order:
  1. a generated, region-aware walker graph;
  2. a shared search scheduler for two or three actors;
  3. a rule for the walking Pokémon behind the player;
  4. a gameplay timer (or accepting map-change time) for dwelling and
     interiors.

## Limits of this evidence

- One trainer, three maps, and a hand-written graph.
- No physical GBA hardware run.
- No cold emulator-process restart. The emulator's host save flush is not
  atomic, so the reload test used the in-game reset.
- Placement scores, routines, and quests were out of scope.
