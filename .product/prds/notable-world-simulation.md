# Notable world simulation

Implemented: No

Specification: [Notable world simulation specification](../specs/notable-world-simulation.md)

Design status: draft for review. It is the **routine and travel** design
that [spots](notable-spots.md) were waiting for. Destinations are spots
only; haunts join later, and until then v0 haunt placement works exactly
as the [haunts PRD](notable-haunts.md) describes. Numbers are
placeholders. The save data's home is decided: the PC loses one box. Terms
follow the [glossary](player-trainer-rating.md#glossary).

## Intent

Notable trainers should feel like people with lives, not statues. Brock
trains in the grass outside Pewter, heals up at the Center, reads up at a
lab, and goes back to his Gym. Erika shops at the department store and
sits by the pond. After a league event, the new champion is out in town,
and the trainer who lost the final is off somewhere quiet. The player can
spot a trainer leaving a Mart, follow them down a route and through a
door, and find them again later where their day took them.

## Design

### Who lives in the world

Every notable trainer who can walk on screen is out in the world: 25 of
them in v0. Tate & Liza are never placed, and twelve more have no walking
sprite yet ([below](#known-limitation-walking-sprites)); those 13 stay as
they are today. A trainer is out of the world for a while when:

- they are in a league event the player has accepted, until it ends:
  they are at the league;
- they are the player's Sevii Masters **partner**: they travel with the
  player.

A trainer placed at a **haunt** stays there, as today. A **Gym Leader the
player hasn't beaten yet stays home** in their Gym, ready for the badge
battle; they start going out once the player has their badge.

### Known limitation: walking sprites

A trainer who walks around needs a sprite that walks. Only 25 notable
trainers have one: the Kanto and Johto trainers (HNS sprites, except Bruno
and Koga), Lorelei
(her FireRed and LeafGreen sprite), and Norman, Juan, Wallace, and Steven
(Emerald). The other 13 have a sprite that can only face each way:
Roxanne, Brawly, Wattson, Flannery, Winona, Tate & Liza, Sidney, Phoebe,
Glacia, Drake, Agatha, Bruno, and Koga.

For now those 13 are not in the world simulation. They stand where they
always stood, in their Gyms and story scenes, and turn up at haunts as
before. Their routines are written and wait for them. Hoenn feels this
most, since only four of its notables walk. New walking sprites can bring
them in later; a shared stand-in sprite would make them lose their look,
so it isn't used.

### Two layers

- **The world simulation** keeps track of everyone, everywhere, off
  screen: which place each trainer is in, where they are heading, and how
  long they stay. It is saved.
- **On the player's map,** the trainers there are real people on screen,
  walking to their spot and doing what spots do: browsing shelves,
  standing at the Center counter, wandering the grass.

When a trainer walks off the player's map by a road or a door, the world
simulation takes them over. When the player arrives somewhere a trainer
is, they appear: at the road they came in by, the door they came through,
or their spot. The player can follow a trainer from map to map, or into a
building, and they are there.

### Time

The world moves when the player changes maps, like roaming legendaries
already do. Each map change is one **heartbeat**: every trainer who isn't
on the player's new map takes one step along their way, or spends one
more beat at their spot. Trainers on the player's map stay put, so the
player can follow them.

The catch: if the player stands still, the off-screen world waits too,
and a trainer inside a building can't come out while the player waits at
the door. Trainers on the player's map still finish what they're doing
and walk on, so the ones the player can see don't freeze. A gameplay
timer or the planned in-game clock can take over later.

### Routines

Each trainer has a **home base** (their Gym city, or a home map from
canon) and an **activity cycle**: three or four everyday activities on
repeat, such as Brock's train, care, study, home. Each activity picks a
nearby spot by the spots rules, and the trainer walks there, stays a
while, and moves on to the next.

A trainer can also have up to three **favourite spots**, the places
they're known for, even far from home: Lt. Surge at the Vermilion harbour
and the Celadon Game Corner, Giovanni at the Game Corner and lying low in
Johto. A favourite wins whenever it's free. Every trainer's cycle and
favourites are written in the
[routines research file](../research/notable-trainer-routines.md).

Examples:

| Trainer | Cycle |
| --- | --- |
| Brock | train → care → study → home |
| Misty | relax → train → fish → home |
| Lt. Surge | home → visit (the harbour) → gamble → train |
| Erika | home → shop → relax |
| Giovanni | gamble → lie low → home |

**Life events** break the routine for a little while:

- **Preparing:** a trainer who is about to be in a league event, or who
  is on the rise, trains.
- **Recovering:** after playing a league event, a trainer goes home and
  rests.
- **Celebrating:** a new champion is out in public near home.
- **Brooding:** whoever lost the final goes somewhere remote.

Aloof trainers stay away from busy public places. Travellers roam much
further, into other regions. Nothing is random: the same save played the
same way gives the same world.

### Travel

Trainers walk from map to map along real paths that a person can walk:
no cutting trees, no jumping down ledges. On screen they never surf;
out of sight they may cross water, as if they came by boat, so island
towns and lakeside places are part of their world. They
take one step of their route each heartbeat. Places have room for only a
few notables at once, one inside a building and two or three outdoors; a
trainer who finds the way full waits a beat or goes around.

## Boundaries

In: who is in the world and when they're away, the two layers and the
handoff between them, the walker paths between maps, the heartbeat,
routines and life events, travel and crowding, the on-screen budget, and
the saved state.

Unchanged: where spots are and how a spot is chosen for an activity
([Notable spots](notable-spots.md)); v0 haunt placement, quests, and the
haunt talk flow; leagues, lineups, and every battle.

Out of scope: haunts as destinations, walkers who surf or cut in view, the
in-game clock, and walking sprites for the trainers who lack one, all for
later. Quests or battles out in
the world.

## Presentation

Trainers on screen walk, face, and use doors with what the overworld
already has, as at spots. They appear at the edge or door they came in
by, never out of thin air in the middle of a map, and a trainer leaving
by a road stays visible until they walk out of view.

## Interactions

- **Spots.** Spots are where trainers go; this design decides when.
- **Talking.** Talking to a trainer anywhere is the spots talk: a
  greeting, a follow-up or what they're doing, a goodbye.
- **Egg sitting and Courier.** Follow-ups happen wherever the player
  finds the trainer. A Courier sender names the town or route the
  recipient is in right now.
- **Gyms.** An unbeaten leader is always in their Gym. A visitor never
  blocks a badge battle.
- **Leagues.** Accepted lineups leave the world until the event ends;
  results set the life events. The world never changes who is picked.
- **Following Pokémon.** The Pokémon walking behind the player competes
  with notables for the few overworld slots; how they share is still
  open.

## Constraints

- The world's saved data is about **216 bytes** for 25 trainers (312 once
  all 37 walk). Once saved leagues land, no part of the save has that room
  free, so the PC drops from 14 boxes to 13 to make it, alongside the
  haunts' trade pool and the Masters teams
  ([where it lives](../specs/notable-world-simulation.md#where-it-lives)).
- Only two or three trainers can be on screen on one map, because the
  overworld holds 16 people and each walker costs processing time
  ([travel proof of concept](../research/notable-trainer-travel-poc.md)).
- Nothing may use the game's random numbers, so walkers never change
  encounters or battles.

## Playtesting

- Does finding a trainer mid-routine feel alive, or too often?
- Is following a trainer fun, and are they easy to lose?
- Do life events read (a champion in town, a loser out in the wilds)?
- Does the world freezing while the player stands still show?
- Is two or three notables on one map lively or crowded?

## Specifications

- [Notable world simulation specification](../specs/notable-world-simulation.md):
  states, the two layers and handoff, the walker graph, the heartbeat,
  routines and life events, travel, the local actor budget, the saved
  record and save budget, and acceptance.

## Later

- The in-game clock moving the world while the player stands still.
- Haunts as destinations.
- Trainers who surf and cut, when their team can.
- Walking sprites for the 13 trainers who only face, so they join the
  world.

## References

- [Notable spots](notable-spots.md)
- [Notable haunts](notable-haunts.md)
- [Notable trainers](notable-trainers.md)
- [Leagues](leagues.md)
- [Sevii Masters](sevii-masters.md)
- [Notable trainer travel proof of concept](../research/notable-trainer-travel-poc.md)
