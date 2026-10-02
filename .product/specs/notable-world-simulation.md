# Notable world simulation

PRD: [Notable world simulation](../prds/notable-world-simulation.md)
Implemented: No
Design status: draft for review. The model is decided: a saved, abstract
**world simulation** advanced by a map-change **heartbeat**, plus a
**local actor** on the player's map, with a handoff both ways, as the
[travel proof of concept](../research/notable-trainer-travel-poc.md)
showed. Destinations are [spots](notable-spots.md) only. Routines, life
events, the walker graph, the saved record, and the local actor budget are
specified here. Every number marked placeholder is waiting for tuning. The
[save layout](#where-it-lives) is decided: one PC box fewer.

## Scope

Own, in `IS_WAYFARER`: who is simulated and their states, the world
simulation and the local actor and the handoff between them, the walker
graph and its generator, the heartbeat, routines (home base use, activity
cycles, life events, dwell), travel and capacity on the move, the local
actor budget, the saved world record and its layout, load validation,
determinism, the debug and balance report, and acceptance.

- [Notable spots](notable-spots.md) owns the spots: their kinds and
  detection, capacity per map, the activity-to-kind table, choosing a spot
  for an activity, home bases, favourites, behaviour templates, the Gym
  "just leaving" rule, and the talk flow at spots. This spec decides
  *when* a trainer does which activity and moves them there; spots decide
  *where*.
- [Notable haunts](notable-haunts.md) owns haunts, v0 haunt placement, and
  the [talk flow](notable-haunts.md#talk-flow) follow-ups. Haunts are not
  destinations here yet ([haunts, later](#haunts-later)).
- [Notable trainers](notable-trainers.md) owns the catalog, TR and growth,
  the traveller and aloof traits, and friendship. This spec reads them.
- [Leagues](leagues.md) owns lineups, event resolution, fatigue, and
  reigning champions; [Sevii Masters](sevii-masters.md) owns the partner.
  This spec reads their state and adds one hook at event resolution
  ([life events](#life-events)).
- The planned in-game clock (named in [Leagues](leagues.md#invitations))
  is not designed yet; this spec runs on map changes until it exists.

## Behavior

### Who is simulated

A notable trainer in the
[inventory](notable-trainers.md#notable-trainer-inventory) is simulated
when they have a walking overworld sprite. That is **25 trainers**, each
with one saved [world record](#the-world-record): Brock, Misty, Lt. Surge,
Erika, Janine, Sabrina, Blaine, Giovanni, Blue, Lorelei, Lance, Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine, Pryce, Clair, Will,
Karen, Norman, Juan, Wallace, and Steven.

The other **13 entries** are not simulated in v0: Tate & Liza, who are
never placed, and twelve trainers whose sprite can only face, not walk
([walking sprites](#known-limitation-walking-sprites)). They have no
record and no routine, and behave as today: their fixed map objects and
v0 haunt placement are unchanged.

At any moment a simulated trainer is in exactly one **state**. The first
row that applies wins:

| State | Applies when | In the world |
| --- | --- | --- |
| Away: league | They are in the accepted event's [event lineup](leagues.md#event-lineup), from acceptance until the event ends (won, lost, or left). | Not shown anywhere. They are at the league. |
| Away: partner | The player is a [Master](sevii-masters.md#master) and they are the resolved [partner](sevii-masters.md#partner): the partner choice, or Lorelei without one. | Not shown. They travel with the player. |
| Pinned at a haunt | v0 [haunt placement](notable-haunts.md#placement) has placed them at a haunt. | Shown at the haunt, exactly as v0 does today. |
| Home-locked | A Gym Leader whose badge the player doesn't hold yet. | In their Gym, as the Gym's own leader object. No local actor. |
| Simulated | Everyone else. | Travelling or dwelling in the world below. |

The first four are **derived**: recomputed from league state, the partner,
haunt placement, and badges whenever those change and on load. The record
still saves which one held last, so a change is noticed and handled once.

A **Simulated** trainer is in one of two **activity states**:

- **Travelling:** heading for their destination spot, one graph hop per
  heartbeat.
- **Dwelling:** at their destination spot, for a number of heartbeats.

Either one is **inside** when the trainer's current node is an interior
(`map_type` `MAP_TYPE_INDOOR` or `MAP_TYPE_NONE`), as in the proof of
concept's "inside a building" state.

**Leaving a derived state.** When a derived state stops applying:

- **From a league:** they reappear at their home base and pick their next
  step at once, as if a dwell had just run out. Their
  [life event](#life-events) starts with it.
- **From the partner role** (another contact was asked): the same, with
  their routine at its first step.
- **From a haunt:** they start dwelling on the haunt's map with dwell 0, so
  the next heartbeat moves them on.
- **From home-locked** (the player wins the badge): they start dwelling at
  home ([home places](#home-places)) with their routine at its first step.

These changes take effect at the next heartbeat, never under the player's
eyes, except that winning a badge changes nothing visible in the Gym: the
leader object stays where it is.

**Gym Leaders' own Gym object.** A Gym's leader object is the existing map
object the badge battle uses. It is shown while the leader is
**Home-locked** (always, so the badge battle is never blocked) or while
their record is dwelling at home in their Gym, and hidden otherwise. A leader
at home in their Gym takes no local actor and doesn't count towards the
Gym's [capacity](notable-spots.md#capacity). While a leader is pinned at
a haunt or away and still unbeaten, their Gym object stays shown, as v0
already shows both today.

### Known limitation: walking sprites

A local actor walks, so only a trainer with a **Full** overworld sprite
can be simulated: the standard 9-frame sheet, facing and walking in all
four directions, animated by `sAnimTable_Standard`
([object_event_anims.h](../../game/src/data/object_events/object_event_anims.h),
line 1223). A read-only sprite audit found:

- **Full (25, simulated):** the HNS `*_HNS` sprites for the Kanto and
  Johto trainers; FireRed and LeafGreen's `LORELEI`, compiled through the
  Sevii block, for Lorelei; and Emerald's `NORMAN`, `JUAN`, `WALLACE`, and
  `STEVEN`.
- **Face-only (13, not simulated):** a 3-frame sheet that faces each way
  but has no walk frames. Roxanne, Brawly, Wattson, Flannery, Winona,
  Tate & Liza, Sidney, Phoebe, Glacia, Drake, Agatha, Bruno, and Koga.

Evidence: the graphics pointer table
([object_event_graphics_info_pointers.h](../../game/src/data/object_events/object_event_graphics_info_pointers.h):
the HNS block near lines 1068-1108, the Emerald entries near 678-691 and
775, the Sevii block near 953-1016), and the sheet sizes in
`graphics/object_events/pics/people/` (144×32 for a Full sheet, 48×32 for
a face-only one). Frames were counted from sheet widths and the pic and
animation tables, not checked visually. An earlier count said 27 Full
sprites; Bruno's and Koga's sheets (HNS and FireRed) are 48×32, face-only,
so the confirmed count is 25.

Hoenn is hit hardest: only Norman, Juan, Wallace, and Steven walk there.
The face-only trainers' routines stay authored, marked not simulated in
v0, in the [routines research file](../research/notable-trainer-routines.md),
so they can join once they walk. The fix is new 9-frame sheets for the
13; a generic stand-in sprite would break their identity, so it is
rejected for v0 ([later](#later)).

### Two layers

The design has two layers, as the proof of concept validated.

1. **World simulation.** Off-screen, abstract, and saved. Each trainer is
   a [world record](#the-world-record): a node of the
   [walker graph](#walker-graph), a destination spot, a state, a routine
   position, and a dwell. A [heartbeat](#heartbeat) advances every
   trainer who isn't on the player's map. No tiles, no objects, no
   pathfinding over the grid.
2. **Local actor.** On the player's map only. The proof of concept's
   walker: an object event that walks tile by tile with the engine's
   collision checks, re-plans when blocked, goes through doors, and plays
   the spot's [behaviour template](notable-spots.md#behaviour-templates)
   once it arrives. It reports every exit and arrival back to the record.

A trainer is never in both layers at once: while a local actor exists for
them, the record follows the actor, and the heartbeat leaves them alone.

### Handoff: world to local

After each map load and its heartbeat, for every Simulated trainer whose
current node is on the player's new map, in
[priority order](#priority), spawn a local actor, if there is
[room](#local-actor-budget):

| Record | Spawns at |
| --- | --- |
| Travelling, arrived by an edge | The entered edge, at the crossing tile in this map's coordinates; if taken, the nearest free tile of the same lane. |
| Travelling, arrived by a door or a transit edge | The first free 4-neighbour of the door's landing tile, never the player's own landing tile. |
| Dwelling | The spot's anchor tile, starting the spot's template. A Gym visit spawns by the ["just leaving"](notable-spots.md#just-leaving) rule instead. |
| On the player's map at the last save | The tile saved in the [local actor block](#the-world-record). |

Spawning waits until the player's field controls unlock, so the engine's
forced door step can finish first (a proof-of-concept finding). A
spawned travelling actor walks on towards its next hop, or to its spot if
the spot is on this map.

**Interiors.** A trainer inside a building is spawned when the player
enters that interior, exactly as the proof of concept's followable indoor
stops: the player can follow a trainer through a door and find them
inside, walking to their spot.

### Handoff: local to world

A local actor hands back to the record when it:

- **Leaves by an edge.** The handoff commits when the exit step starts, so
  a player crossing at once can't outrun it. The record moves to the node
  on the other side, arrival by that edge, with the crossing converted by
  the connection's offset. For Viridian City's north edge
  (Route 2 `up`, offset 16), Viridian (25, 0) becomes Route 2 (9, 79);
  the return adds 16. The actor stays visible in the loaded connection
  strip until it walks out of view, keeps its object and sprite if the
  player crosses after it, and is rebased into the new map's coordinates
  without a respawn (the proof of concept's seam follow-up). A visible
  actor in the strip never advances through the graph.
- **Leaves by a door, stairs, or a transit warp.** The record moves to the
  node behind the warp, arrival by door; the actor is removed after its
  step into the warp.
- **Arrives at its spot.** The record becomes Dwelling, and the template
  starts.
- **Finishes dwelling while watched.** On the player's map, dwell also
  runs down in local time: one heartbeat of dwell per placeholder
  **10 dwell ticks** (the spots spec's 60-frame tick), counted only while
  the player's field controls are unlocked. When it runs out, the routine
  advances and the actor walks to its next spot, so a trainer the player
  watches eventually leaves on foot and can be followed.

If the player leaves the map while an actor is still walking, the actor
is removed, and the rest of its walk is abstracted: at the next heartbeat
it is treated as a record on its current node, travelling as usual.

### Walker graph

The world simulation travels over a **walker graph** generated at build
time. Map adjacency is not enough: the proof of concept found
`Route2_hns` cut in two by solid rows 41-45, with its middle gate behind a
Cut tree at (15, 69), so its south end can't reach Pewter on foot, and
Route 2's north border includes wall tiles
([constraints](../research/notable-trainer-travel-poc.md#constraints-discovered)).

**Nodes** are **walkable regions within maps**: each 4-connected component
of tiles a walking NPC can reach, flood-filled with NPC collision and
elevation rules on every playable map in scope (the
[spot extraction's](notable-spots.md#spot-extraction) reachable maps).
Route 2 gives two nodes, not one. A component with no edge, no spot, and
no warp is dropped.

**Edges** connect nodes:

- **Connections:** a map connection where at least one pair of facing
  border tiles is walkable on both sides (the **lane**). Each lane run is
  stored with the connection's offset.
- **Warps:** a warp tile reachable from the node, to the region holding
  its destination warp's landing tile. Doors, stairs, gatehouses, cave
  entrances, and store floors are all warps.
- **Transit:** an authored list of scripted links, open to
  [travellers](notable-trainers.md#traveller) only, one hop each:
  placeholders are the S.S. Aqua (Olivine Port to Vermilion Port) and the
  Magnet Train (Goldenrod to Saffron), plus Hoenn's and Sevii's ferries.
  Transit never reads the player's tickets or story flags.

**Filters (v0):** walking only. No Surf, Cut, Strength, Rock Smash,
Waterfall, Dive, or ledge jumps; Cut trees, boulders, and smashable rocks
are solid. **One-way edges** are kept one-way: a ledge drop splits a
region into two nodes with a one-way edge, and holes and one-way warps
point one way. Story-gated objects with a flag are treated as solid when
they are present at New Game (placeholder; see
[open questions](#open-questions)).

**Generator outputs**, one ROM table each, in a fixed order (map order,
then component top-left tile) so ids are stable between builds of the
same content:

1. the **node table**: map, a representative tile, interior or outdoor,
   and its region (Kanto, Johto, Hoenn, or Sevii);
2. the **edge table**: per node, its outgoing edges in fixed order (kind,
   target node, lane or warp tiles, offset);
3. the **spot-to-node index**: each spot of the
   [spot table](notable-spots.md#spot-extraction) and the node its
   standing tile (or patch) belongs to;
4. **home-base candidate lists**: for each home base, every spot within
   the traveller radius, sorted by hop distance from the home base, then
   spot-table order, so a step's pick is a scan, not a search; and
5. a **content hash** of the node, edge, and spot tables, saved in the
   [record header](#the-world-record).

**Validation** fails the build when:

- a spot's standing tile isn't in any node, or a home base map has no
  node;
- a home base's node can't reach a spot on its own home map;
- a trainer could get stuck: a node holding a spot has no path back to
  its home base's node (one-way edges included);
- a map is wider or taller than 255 tiles (the record stores a u8
  crossing; the largest layout today is 170×160,
  `LAYOUT_OLIVINE_CITY_LIGHTHOUSE_HNS`);
- there are more than 4,095 nodes or 16,383 spots (the record's field
  widths); or
- an edge's target doesn't exist. A connection with no walkable lane
  simply gives no edge, as on Route 2's north border.

The generator also reports per-map node counts, split maps, and
unreachable spots, so a split like Route 2's shows up in review.

### Heartbeat

The world moves on a roamer-style **heartbeat**: one tick on each map load
from a camera transition or a warp, as the proof of concept did. The
engine's roamers move at the same points: `LoadMapFromCameraTransition`
calls `MoveAllRoamers`, and `LoadMapFromWarp` calls
`MoveAllRoamersToOtherLocationSets` ([engine notes](#engine-notes)). The
heartbeat runs after the player's new map is known.

**Not a heartbeat:** Continue (it restores the map without advancing the
world; proof of concept), opening the Bag or any menu, battles, and
scripts that reload the same map without a transition.

**One heartbeat**, in [priority order](#priority):

1. Apply any pending derived-state changes ([who is
   simulated](#who-is-simulated)).
2. For each Simulated trainer **not** on the player's new map, and not
   visible in a connection strip:
   - **Dwelling:** dwell − 1. At 0, the [routine advances](#routine-advance).
   - **Travelling:** take one hop along the
     [shortest path](#travel) to the destination's node. Reaching it makes
     them Dwelling with the step's dwell.
3. Trainers on the player's new map **stay put**: no hop, no dwell. So
   the player can follow a trainer, and a trainer the player walks in on
   is still there.
4. Spawn local actors ([world to local](#handoff-world-to-local)).

**Default: map-change time.** Dwell is counted in heartbeats. This is
recommended for v0 because it is proven, cheap, saved, and deterministic.
Its known limits:

- standing still freezes the off-screen world; and
- a trainer inside a building can't leave on their own while the player
  waits outside, since nothing ticks.

The local dwell on the player's map ([local to
world](#handoff-local-to-world)) softens the first limit for the trainers
the player can see. A gameplay timer, or the planned in-game clock, can
later replace or add to the heartbeat without changing the record: a
clock tick would run the same step 2.

### Priority

Whenever trainers compete in one heartbeat (for a spot, for room on a
map, or for an object slot), they are handled in **priority order**:

1. closer to home first: fewer graph hops from their current node to
   their home base's node; then
2. catalog order (the
   [inventory's](notable-trainers.md#notable-trainer-inventory) order).

### Routines

A **routine** is how a trainer spends their time: a **home base**, an
**activity cycle**, and **life events** that override the cycle.

#### Home base

Each trainer's [home base](notable-spots.md#home-base) is the map that
spot choice measures from. The spots spec lists them: a Gym Leader's Gym
city, otherwise one authored home map. Its node is the region of the home
map holding most of the home map's spots (ties to the lower node id).

#### Activity cycle

Each trainer authors an **activity cycle**: an ordered list of **3-4
steps**, each one activity from the
[activity list](notable-spots.md#activities-and-kinds) (care, shop,
gamble, train, relax, fish, visit, study, home, sightsee, lie low). The
cycle repeats. Cycles and [favourites](notable-spots.md#favourites) are
authored together, per trainer, in the
[routines research file](../research/notable-trainer-routines.md), which
then moves into the catalog. A trainer without a cycle would use the
placeholder default **train → care → relax → home**; every simulated
trainer has one.

A favourite is how a trainer reaches a signature place outside their
home radius. Lt. Surge's cycle is home → visit → gamble → train: his visit
favourite is the Vermilion harbour named spot, which now lists visit, and
his gamble favourite is the Celadon Game Corner, 5 map hops from Vermilion.
Giovanni's is gamble → lie low → home: the Celadon Game Corner and the
Burned Tower (the only lie-low named spot, in Johto) are both beyond his
8-hop radius from Viridian, so his favourites reach them. The routines
file's check finds a candidate for every step of every cycle; the
[itinerary report](#debug-and-balance-report) shows skips per trainer in
play.

#### Choosing the step's spot

Each step picks one spot by the
[spots rules](notable-spots.md#choosing-a-spot): its kinds, favourites
first (used when free, else the derived pick), then candidates within the
home-base radius (placeholder 3 hops, or 8 for a traveller), excluding
taken spots and full maps, nearest first, with the spots spec's rotating
tie-break. The routine adds these filters before the pick:

- **Aloof** trainers never pick a **public** spot: kinds Pokémon Center,
  Poké Mart and department store, Game Corner, other Gyms, town squares
  and benches, and chatting with an NPC, plus named spots on town or city
  maps and the interiors entered from them. Their **remote** spots are
  tall grass, water's edge, and the other named spots. A home step is
  never filtered.
- **Travellers** use the wider radius and may cross regions through
  connections and [transit edges](#walker-graph); everyone else uses the
  narrow radius.

#### Home places

**Home** has no detected spot kind, so a home step resolves to a **home
place**:

- **Gym Leaders:** their own Gym, shown by the Gym's leader object, not a
  local actor ([who is simulated](#who-is-simulated)). It is always
  available.
- **Everyone else:** on the home map only, the first of: a named spot
  listing home (none in the starting list), the home map's first town
  square, the home map's first spot of any kind, in spot-table order.
  Without any, the home step is skipped. Placeholder; a short list of home
  named spots (such as Steven's house) is an
  [open question](#open-questions).

#### Dwell

When a step's spot is chosen, its **dwell** is set by activity, in
heartbeats (placeholders):

| Activity | Dwell |
| --- | ---: |
| visit | 1 |
| care, shop, relax, sightsee | 2 |
| gamble, train, fish, study | 3 |
| home, lie low | 4 |

Dwell counts down only once the trainer has arrived. A Gym visit ends
when the visitor walks out ([just leaving](notable-spots.md#just-leaving)),
or after its dwell if the player never comes in.

#### Life events

A **life event** overrides the cycle for a while. There are four, defined
only from saved league state and TR:

| Life event | Who | Steps |
| --- | --- | --- |
| Celebrating | The trainer who became a league's reigning champion at that event's resolution: the strongest of a declined event or of an accepted event the player lost or left, or the Masters partner crowned by a lost final ([reigning champion](leagues.md#reigning-champion)). | relax, then visit, limited to public kinds and the home map first (then the narrow radius). |
| Brooding | The trainer who lost the final: in an accepted event the player won, the last member in battle order (fought in match 5; at the Masters, the partner). | lie low, then train, limited to remote spots and picking the **farthest** candidate in radius instead of the nearest. |
| Recovering | Every other member of the resolved event's lineup, accepted or declined. Fatigue (50) applies to the same trainers. | home, then relax. |
| Preparing | Derived, not stored: either the trainer is in the **provisional lineup** while an invitation waits for an answer, or their [momentum](notable-haunts.md#momentum) is rising. | Provisional lineup: every non-home step becomes train. Rising momentum: a relax or sightsee step becomes train. |

**Setting and lasting.** Celebrating, brooding, and recovering are set by
one hook at event resolution, in the same transaction
([lifecycle](leagues.md#lifecycle)): each lineup member's record gets the
life event and **2 steps** (placeholder). Each routine advance uses one
step's override and counts down; at 0 the cycle resumes where it left off.
A trainer who is away when set starts the event on their return. A newer
resolution replaces an older life event.

**Provisional lineup.** While the invitation state is **invited**, the
provisional lineup is what [selection](leagues.md#selection-and-order)
would pick if the player accepted now. It is recomputed when that state
starts and on load, never saved, and committed nowhere. Accepting makes
those trainers Away: league.

**Precedence** when more than one applies: celebrating, brooding,
recovering, preparing by provisional lineup, preparing by momentum, then
the cycle. Aloof filters still apply: an aloof trainer celebrates at remote
spots near home.

#### Routine advance

When a dwell runs out (in a heartbeat, or locally while watched):

1. If a life event has steps left, take its next activity and count it
   down. Otherwise move the cycle to its next step (wrapping) and take its
   activity, adjusted by preparing.
2. Pick the step's spot ([choosing](#choosing-the-steps-spot)).
3. **Skipped:** with no candidate, move to the following step, at most
   one full cycle in one advance. If nothing fits, go home (or, for a
   trainer with no home place, stay where they are with dwell 1).
4. Save the destination and dwell; the trainer becomes Travelling, or
   Dwelling at once if the spot is on their current node.

### Travel

- **Path.** Shortest path in hops over the walker graph from the current
  node to the destination's node. Ties between paths go to the lower
  edge order at each node, so the path is fixed. The path is recomputed
  every heartbeat from the record (never saved), by a search bounded to
  twice the trainer's radius.
- **One hop per heartbeat.** Arrival at the destination's node makes the
  trainer Dwelling. Arrival through an edge or a warp records the
  arrival kind and crossing for the [spawn](#handoff-world-to-local).
- **Capacity on the move.** A map's **occupancy** counts every notable
  on the map now or with their destination on it, each once; haunt-pinned
  trainers count on their haunt's map. A hop into a map other than the
  trainer's destination map is allowed only while occupancy is under the
  [capacity](notable-spots.md#capacity) (interior 1, outdoor 3 as a
  placeholder, 2 if the budget needs it). A hop into the destination map
  is always allowed, since spot choice already reserved room. So no map
  ever holds more notables than its cap.
- **Blocked.** A blocked traveller **waits** one heartbeat and sets the
  record's waited bit. Blocked again, they **reroute**: the shortest path
  that avoids the full map, if one exists within the search bound;
  otherwise they wait again. The bit clears on any hop.
- **Order.** Hops are taken in [priority order](#priority), so the
  trainer closer to home gets the last room on a map.

### Local actor budget

- **Actors per map:** at most the map's capacity: 1 indoors, 2-3 outdoors.
  The cap that limits notables per map also limits actors.
- **Object slots.** The engine has 16 object events
  (`OBJECT_EVENTS_COUNT`), shared with the map's own objects, the
  following Pokémon, and haunt objects. An actor is spawned at runtime
  (not from a map template), so no map needs an object template per
  trainer. With no free slot, the actor isn't spawned: the trainer stays
  in their record, unseen, and is spawned when a slot frees up. An actor
  keeps its slot while visible; it is freed when the actor leaves. Hidden
  objects still collide, so the walker never hides in place (proof of
  concept).
- **Following Pokémon.** The proof of concept switched the player's
  following Pokémon off to free slots. Recommended rule, as an
  [open question](#open-questions): notable actors take priority, and the
  follower is hidden while fewer than 2 slots (placeholder) would remain
  free with every actor spawned, and comes back when the actors leave.
- **Shared search scheduler.** Only one grid search runs at a time,
  across all actors on the map, in one shared 11,200-byte heap workspace
  at eight node expansions per field update (the proof of concept
  measured slices of about 0.56-0.63 of a frame). Requests queue in
  priority order; an actor waiting for a search stands still or keeps
  playing its template. A blocked step re-queues a search, and four
  blocked steps drop the goal and re-plan.
- **Heap resets.** Menus and warps reset the engine heap. The workspace
  is dropped and rebuilt, and each actor re-plans from its current tile
  (proof of concept, constraint 4). Template counters live in RAM and
  restart at the spot's anchor, as the spots spec allows.
- **Talking.** An actor's AI is suspended while the player's field
  controls are locked, so a talk never lets the dwell or a step run on
  (a proof-of-concept fix: `faceplayer` alone was not enough).
- **Story scenes win.** An actor is never spawned on a map whose story
  scene currently shows an object for the same character (Blue's rival
  battles, Giovanni's Rocket scenes); the record waits there unseen.
  Placeholder; see [open questions](#open-questions).

### Interactions

- **Talk flow.** Talking to a local actor at a spot runs the
  [spots talk flow](notable-spots.md#talk-flow): greeting by friendship,
  a follow-up or the activity line, farewell. Talking to a travelling
  actor runs the same flow with the line of the activity they are heading
  for. A first talk anywhere is the "first talk" friendship event.
- **Follow-ups go where the trainer is.** The
  [Courier delivery](notable-haunts.md#delivery), the
  [Egg sitting follow-up](notable-haunts.md#the-follow-up), and the
  [Courier trail](notable-haunts.md#refreshing-the-trail) play wherever
  the player finds the trainer, in the
  [haunt talk flow's](notable-haunts.md#talk-flow) order. A **stay** is
  one Dwelling period at one spot; the once-per-stay bits live in the
  record and clear when the trainer arrives at a new spot.
- **Courier `{PLACE}`.** The sender names the recipient's current map:
  the location name of the region-map section of their record's node's
  map, as
  [Courier recipients at spots](notable-spots.md#courier-recipients-at-spots)
  describes ("CERULEAN CITY", "ROUTE 2"). An away or home-locked
  recipient is named by their home base's map, and a pinned one as the
  haunts spec names a haunt.
- **Gyms.** A visitor walks out as the player walks in, and never blocks
  a badge battle ([just leaving](notable-spots.md#just-leaving)).
- **Leagues.** The simulation never changes selection, lineups, or
  battles; it only reads league state and adds the resolution hook.

### The world record

Each simulated trainer has one record. The proof of concept's 12 bytes
held a current and destination map, arrival kind and crossing, local x
and y, state, goal, and dwell. Spots change it:

- the **current map** becomes a **node** (a region within a map);
- the **destination map** becomes a **destination spot**;
- the **goal** becomes the **routine position** and **activity**;
- **local x and y** move out to a shared **local actor block**, since only
  trainers on the player's map need a tile; everyone else spawns at a
  spot, an edge, or a door; and
- it gains the life event, the once-per-stay bits, and the waited bit.

**Record, 8 bytes (64 bits):**

| Field | Bits | Holds |
| --- | ---: | --- |
| `node` | 12 | The current walker-graph node (0-4,095). |
| `destKind` | 2 | None, spot, or home place; the fourth value is reserved for [haunts](#haunts-later). |
| `destId` | 14 | The spot id (0-16,383) in the generated spot table. |
| `state` | 3 | Travelling, Dwelling, Away: league, Away: partner, Pinned, or Home-locked. |
| `arrival` | 3 | None, north, south, east, west, door, or transit. |
| `crossing` | 8 | The crossing coordinate along the entered edge. |
| `step` | 2 | The position in the activity cycle (0-3). |
| `activity` | 4 | The current step's activity (11 values). |
| `dwell` | 6 | Heartbeats left (0-63). |
| `lifeEvent` | 2 | None, recovering, celebrating, or brooding. |
| `lifeSteps` | 2 | Override steps left (0-3). |
| `stayBits` | 2 | The once-per-stay Egg sitting follow-up and Courier trail bits. |
| `waited` | 1 | Blocked last heartbeat. |
| reserved | 3 | Zero. |

**Local actor block, 9 bytes:** 3 entries, one per actor on the player's
map at save time: the trainer (6 bits), x and y (8 bits each), and facing
(2 bits), 24 bits each. An unused entry holds trainer value 63 (none)
and zeros elsewhere.

**Header, 4 bytes:** a schema version (1 byte), a reserved byte, and the
walker graph's 16-bit content hash.

**Total:** 25 × 8 = 200 bytes of records, plus 9 and 4, is 213 bytes,
**216 bytes** padded to a word. Adding the twelve placeable face-only
trainers later ([walking sprites](#known-limitation-walking-sprites)) takes
it to 37 records, 312 bytes.

#### Save budget

Measured with `sizeof` against this branch's headers (cross-compiled for
`POKEMON_WAYFARER`), and against open PR
[#139](https://github.com/mzpkdev/pokemon-wayfarer/pull/139)
(`task/v0-trainer-runtime`, saved leagues) at commit `6160294f26`:

| Block | Capacity | Used (main) | Free (main) | Free with PR #139 |
| --- | ---: | ---: | ---: | ---: |
| SaveBlock1 | 15,872 | 15,760 | 112 | 112 |
| SaveBlock2 | 3,968 | 3,892 | 76 | 76 |
| SaveBlock3 | 1,624 | 1,160 | 464 | 276 |
| `PokemonStorage` | 35,712 | 34,144 | 1,568 | 112 |

PR #139 adds `LeagueEventState` (180 bytes) to SaveBlock3 and the five
saved league teams (`LeagueSavedTeams`, 1,456 bytes, 288 per team) to
`PokemonStorage`.

**What else competes,** all specified but not yet in code:

| State | Owner | Size | Intended home |
| --- | --- | ---: | --- |
| Friendship scores, first-win and repeat-win bits | [Notable trainers](notable-trainers.md#friendship) | 48 | SaveBlock3 (haunt and notable state) |
| Reward counters | [Haunts](notable-haunts.md#saved-state) | 19 | SaveBlock3 |
| Egg sitting outstanding bits and counts | Haunts | 24 | SaveBlock3 |
| Handicap bits | Haunts | 10 | SaveBlock3 |
| Courier state | Haunts | 2 | SaveBlock3 |
| Per-haunt claim, placement, follow-up, and quest bits (22 Kanto haunts) | Haunts | about 28 | SaveBlock3 |
| Quest in progress | Haunts | about 2 | SaveBlock3 |
| Masters partner choice and tag selection | [Sevii Masters](sevii-masters.md#saved-state) | about 2 | league state |
| Traded-slot records (16 × 40) | Haunts | 640 | `PokemonStorage` |
| Masters lineup: 8 opponents and the partner, at PR #139's 288 bytes a team | Sevii Masters | about 1,150 more than PR #139's five | `PokemonStorage` |
| Follower NPC state, if enabled for walks | Haunts | 24 | SaveBlock3 |

**Where it fits.**

- **SaveBlock3** is where the proof of concept put its record, next to
  the haunt state. With PR #139 it has 276 bytes free; the haunt and
  friendship state above take about 133 of them, and the follower NPC 24,
  leaving about **120**. The world's 216 bytes **do not fit**. Today, on
  main, they would fit (464 free), but only by taking the space the
  haunts and leagues already count on.
- **`PokemonStorage`** has 112 bytes free with PR #139. That already
  can't hold the haunts' 640-byte traded-slot pool or the Masters'
  larger lineup, let alone the world. With 14 boxes it **does not fit**
  either; with 13 it does ([where it lives](#where-it-lives)).
- **SaveBlock1** (112 free) and **SaveBlock2** (76 free) are too small.
- The **special sectors** (Hall of Fame, Trainer Hill, recorded battle,
  sectors 28-31) sit outside the two alternating save slots, so they
  can't be written atomically with the rest of the save. Not an option.

**Risk, plainly:** once saved leagues land, no save block has room for
the world simulation, and `PokemonStorage` is already short for the
haunts and the Masters. The [decision below](#where-it-lives) frees a PC
box to make that room.

#### Where it lives

**Decision: one PC box fewer (14 to 13).** This is a **cross-cutting
save-layout decision**: it frees the room in `PokemonStorage` that the
world state, the haunts' trade pool, and the Masters' extra teams all need,
and each of those specs points here.

- **Freed.** A box is 30 slots of the 80-byte `BoxPokemon`, a 9-byte name,
  and a wallpaper byte, 2,410 bytes. The struct then pads the fusion
  Pokémon that follow to a word, so `sizeof(struct PokemonStorage)` drops
  by **2,408 bytes**: from 34,144 to 31,736 on main, and from 35,600 to
  33,192 with PR #139 (`sizeof` compiled with `arm-none-eabi-gcc
  -mabi=apcs-gnu`, as the build does, against this branch and PR #139's
  headers). The nine storage sectors hold 35,712 bytes.
- **What it hosts.** With PR #139, `PokemonStorage` has **2,520 bytes**
  free after the change. It takes the `WayfarerWorldState` (216 for the
  25 simulated trainers), the haunts'
  [traded-slot pool](notable-haunts.md#saved-state) (640), and the
  [Masters lineup](sevii-masters.md#saved-state) growth: 8 opponents and the
  partner at 288 bytes a team, 2,592 plus PR #139's 16-byte header, against
  PR #139's 1,456, so about 1,152 more. That is 2,008 bytes, leaving a
  margin of about **512 bytes**. Adding the twelve face-only trainers later
  (96 bytes, 312 in all) leaves about **416**. Without PR #139 (main
  today) 3,976 bytes would be free.
- **Depends on PR #139.** Saved leagues
  ([#139](https://github.com/mzpkdev/pokemon-wayfarer/pull/139)) add
  `LeagueSavedTeams` to `PokemonStorage`, so the margin above is counted
  with it merged. If Leagues later saves its teams more compactly, the
  margin grows; if it grows them, the margin shrinks first.
- **Game-wide.** `TOTAL_BOXES_COUNT`
  ([pokemon_storage_system.h](../../game/include/pokemon_storage_system.h),
  line 4) is read across the PC code, so the PC shows **13 boxes** in every
  Wayfarer game, not only for the simulation. Prerelease saves don't carry
  over and need no migration, per the repository's save policy
  ([AGENTS.md](../../AGENTS.md#save-compatibility)).

**Layout:**

1. Keep the record at **8 bytes** and the whole world state as **one
   contiguous struct** (`WayfarerWorldState`, 216 bytes) with a
   `STATIC_ASSERT` on its size, never in SaveBlock1.
2. Append it to **`PokemonStorage`**, after the 13 boxes, beside the trade
   pool and the league teams.
3. Keep the build-time size check: `PokemonStorageFreeSpace`
   ([save.c](../../game/src/save.c), line 123) fails the build rather than
   overflowing the storage sectors.

Shrinking the record further (about 6 bytes, by deriving the crossing from
the edge's lane and narrowing dwell) would save about 75 bytes; it isn't
needed with the margin above.

### New Game

New Game writes the header (schema, content hash), an empty local actor
block, and one record per simulated trainer: derived states as they hold
at New Game (Gym Leaders Home-locked); everyone else Dwelling at their
first cycle step's spot, picked in [priority order](#priority), on that
spot's node, with that step's dwell. No heartbeat runs. Trainers whose spot
is on the player's starting map spawn there.

### Load validation

On every load, before the overworld runs:

1. **Content change.** If the saved content hash differs from the build's
   (the graph or spot table changed), re-seat every record as New Game
   does, keeping each trainer's cycle step and life event. It is never an
   invalid save.
2. **Checks.** Each record's node exists; a spot destination exists and
   its node is reachable; `destKind` is never the reserved value; arrival
   is a known value and is none while Dwelling; activity is a known value;
   `lifeSteps` is 0 when `lifeEvent` is none; the reserved bits are zero.
   Each local actor block entry names a simulated trainer at most once, on
   the saved map's node, with a tile inside the map. A failed check is an
   invalid save.
3. **Recompute** the derived states and apply them as a heartbeat would,
   without advancing anyone. The local actor block is used once, for the
   restored map, and then cleared.

### Determinism

Nothing here reads the random number generator: a cosmetic walker must not
shift encounter or battle rolls. Every choice comes from saved state,
world progress, league state, and content, through fixed rules: spot
choice (the spots spec's rotation), [priority order](#priority), path
ties by edge order, and the life-event hook. The same save and the same
sequence of map loads always give the same world. Continue doesn't
advance it; a save and reload restores identical record bytes (proof of
concept).

### Debug and balance report

A build-time tool runs the simulation offline on the generated tables
and catalog, over **N heartbeats** (default 200), for a given world
progress, badge set, league state, and a **player path** (a list of map
loads, or "stays on one map"). It reports:

- **Itineraries:** per trainer and heartbeat: node and map, state,
  activity, destination spot, dwell, life event, and every skipped step
  with its reason (no candidate, aloof filter, radius).
- **Crowding:** per map, the peak and mean occupancy, cap hits, waits,
  and reroutes; the busiest maps; how often two or three notables share
  an outdoor map.
- **Coverage:** per trainer, the share of heartbeats per activity and
  per region; spots never used; trainers who never leave home.
- **Cost:** search nodes visited per heartbeat (worst and mean), to check
  the heartbeat fits inside a map transition.

In game, a debug menu shows a trainer's record, warps the player to a
trainer, and forces a heartbeat. Balance is informational.

## Engine notes

Verified against `game/` at the time of writing; line numbers may drift.

- **Roamer heartbeat points.** `LoadMapFromCameraTransition`
  ([overworld.c](../../game/src/overworld.c), line 976) calls
  `UpdateLocationHistoryForRoamer` and `MoveAllRoamers` (lines 1036-1037).
  `LoadMapFromWarp` (line 1056) calls `MoveAllRoamersToOtherLocationSets`
  (line 1124). `MoveAllRoamers` is in
  [roamer.c](../../game/src/roamer.c) (line 280).
- **Connections.** `struct MapConnection` holds a direction, an `s32`
  offset, and the target map
  ([global.fieldmap.h](../../game/include/global.fieldmap.h), line 220).
  `ViridianCity_hns` connects `MAP_ROUTE2_HNS` up at offset 16 and
  `MAP_ROUTE1_HNS` down at offset 2
  ([map.json](../../game/data/maps/ViridianCity_hns/map.json)), the
  offsets the proof of concept used. The largest connection offset in
  `data/maps` is 120.
- **Collision.** `GetCollisionAtCoords`
  ([event_object_movement.h](../../game/include/event_object_movement.h),
  line 191) is the check the walker and the graph flood use.
- **Objects.** `OBJECT_EVENTS_COUNT` is 16 and
  `OBJECT_EVENT_TEMPLATES_COUNT` 64
  ([constants/global.h](../../game/include/constants/global.h), lines 139
  and 144). Runtime spawning exists:
  `SpawnSpecialObjectEventParameterized`
  ([event_object_movement.c](../../game/src/event_object_movement.c),
  line 2097). The following Pokémon uses local id 254
  (`LOCALID_FOLLOWING_POKEMON`,
  [event_objects.h](../../game/include/constants/event_objects.h), line
  671), and `OW_FOLLOWERS_ENABLED` is `TRUE`
  ([overworld.h](../../game/include/config/overworld.h), line 61).
- **Heap.** `HEAP_SIZE` is `0x1C500` (115,968 bytes,
  [malloc.h](../../game/include/malloc.h), line 44).
  `MoveSaveBlocks_ResetHeap` re-runs `InitHeap`
  ([load_save.c](../../game/src/load_save.c), lines 93 and 131); the
  overworld calls it from `ResetMirageTowerAndSaveBlockPtrs`
  ([overworld.c](../../game/src/overworld.c), line 2734).
- **Save blocks.** SaveBlock3 is spread over 14 sectors of 116 bytes
  ([save.h](../../game/include/save.h), lines 8 and 31) and capped at
  1,624 bytes (`WayfarerSaveBlock3SectorAllocation`,
  [save.c](../../game/src/save.c), line 93). `PokemonStorageFreeSpace`
  asserts the storage size (line 123). Sectors 28-31 are the special
  sectors (save.h, lines 33-36). `struct PokemonStorage` holds 14 boxes
  today
  ([pokemon_storage_system.h](../../game/include/pokemon_storage_system.h),
  line 4); the [save layout](#where-it-lives) takes it to 13.
- **Map sizes.** The largest layout is 170×160
  (`LAYOUT_OLIVINE_CITY_LIGHTHOUSE_HNS`,
  [layouts.json](../../game/data/layouts/layouts.json)), so u8 tiles and
  crossings fit.

## Haunts (later)

Haunts will plug in as a second destination kind (the record's reserved
`destKind` value): a routine step could pick a haunt as well as a spot,
and haunt placement would become the simulation's choice. Until then,
v0 haunt placement stays exactly as
[specced](notable-haunts.md#placement), and a placed trainer is Pinned at
their haunt.

## Acceptance

1. **Scope.** The 25 trainers with a walking sprite have a record; Tate &
   Liza and the twelve face-only trainers have none and keep their fixed map
   objects and haunt placement. Each derived
   state applies exactly when its table row says, with the given
   precedence.
2. **Follow across an edge.** A trainer leaving the player's map by an
   edge appears on the neighbour map at the crossing converted by the
   connection offset; the player following sees exactly one actor, with
   no gap or duplicate at the seam.
3. **Follow through a door.** The player follows a trainer into an
   interior and finds them walking to their spot.
4. **Linger.** A trainer off the player's map moves one hop per heartbeat
   and dwells for their step's dwell; a trainer on the player's new map
   doesn't move.
5. **Routine.** A trainer's spots follow their cycle; a step with no
   candidate is skipped; aloof trainers never stand at a public spot; a
   Gym Leader stays in their Gym until the player holds their badge.
6. **Life events.** After a resolution, the champion celebrates, the
   final's loser broods (when the player won), the others recover, each
   for 2 steps; a waiting invitation's provisional lineup trains.
7. **Capacity.** No map ever holds more notables than its cap; a blocked
   traveller waits, then reroutes.
8. **Budget.** At most 3 actors on a map; one grid search at a time;
   no slice over one frame in the measured scenarios; actors recover after
   menus and warps.
9. **Talk.** The spots talk flow and follow-ups play at any spot; Courier
   `{PLACE}` names the recipient's current map.
10. **Save.** Save, reset, and Continue restore identical records and the
    actors on the player's map at their saved tiles; Continue doesn't
    advance the world; a content-hash change re-seats records without an
    invalid save.
11. **Determinism.** Two runs with the same save and map loads produce
    identical itinerary traces.
12. **Report.** The itinerary and crowding report runs on the generated
    tables and flags skipped steps and unreachable spots.

## Open questions

- **Following Pokémon:** accept the recommended rule (actors first, the
  follower hidden while slots are tight), or keep followers and cap
  actors at 2 outdoors.
- **Preparing by momentum:** does turning relax and sightsee into train
  make rising trainers train too often mid-game, when many are rising?
- **Provisional lineup:** is training while the invitation waits the
  right reading of "upcoming lineup", given that accepted lineups are
  away?
- **Home named spots:** author one home place for each non-leader (such
  as Steven's house and Lorelei's house), or keep the town-square
  fallback.
- **Story scenes:** which story objects suppress an actor (Blue, Giovanni,
  league hosts), and whether the record should move on instead of
  waiting.
- **Story-gated walls:** whether the walker graph reads flagged blockers
  at New Game only (placeholder) or rebuilds edges as story flags change.
- **Own-Gym talk:** whether talking to a beaten leader at home in their
  Gym keeps the Gym's script or runs the spots talk flow.

## Later

- **In-game clock:** a gameplay timer or the planned in-game clock driving
  ticks between map changes, so dwelling and interiors move while the
  player stands still.
- **Haunts as destinations** ([above](#haunts-later)).
- **Walking sprites for the face-only trainers:** new 9-frame sheets for
  the 13 ([walking sprites](#known-limitation-walking-sprites)), so they
  can be simulated with the routines already authored.
- **Surf- and Cut-aware walkers:** water and Cut edges in the graph, for
  trainers whose team can use them.
- **Routines into the catalog:** the
  [routines research file's](../research/notable-trainer-routines.md)
  cycles and favourites become catalog values.
- Play style or momentum shaping dwell lengths.

## References

- [Notable world simulation PRD](../prds/notable-world-simulation.md)
- [Notable spots](notable-spots.md)
- [Notable haunts](notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Leagues](leagues.md)
- [Sevii Masters](sevii-masters.md)
- [Notable trainer travel proof of concept](../research/notable-trainer-travel-poc.md)
- [Notable spots inventory (draft)](../research/notable-spots-inventory.md)
- [Notable named spots (starting list)](../research/notable-named-spots.md)
- [Notable trainer routines](../research/notable-trainer-routines.md)
- [Kanto and Johto inter-region travel](../research/kanto-johto-inter-region-travel.md)
