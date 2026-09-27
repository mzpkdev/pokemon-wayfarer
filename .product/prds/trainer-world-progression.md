# Trainer world progression

Implemented: No. The current ROM keeps its existing Gym and League scaling
until this design is adopted; the balance explorer is provisional tooling.
Design status: "Living rivals" is the accepted direction. Per-trainer strength,
growth arcs, rosters, and hint content are provisional content under review.

## Intent

Make familiar trainers feel alive. Gym Leaders, Elite Four members, Champions,
and Blue grow alongside the player's journey, each with a personality of their
own, so that two saves tell different stories: "Clair got scary this save,"
while Bruno has settled into a comfortable veteran's pace. Players can take
Gyms in any order without meeting a late-game team too early, and a postponed
leader is still a real fight.

## Design

### NPCs follow the world, not the player's party

Every trainer's strength is measured against the world's level cap: the soft
cap a player would have from badges and first league clears alone. Earning a
badge or clearing a league for the first time moves the world forward, and
every trainer moves with it. Grinding, training, swapping party members, or
waiting never does. Players cannot make opponents harder by over-levelling,
and cannot make them easier by under-levelling.

### Standing and growth arcs

Each trainer sits at a personal standing relative to the cap. Gym Leaders sit
a little under it on average, Elite Four members at it, Champions above it, and
Blue a step above the cap at every point in the game. Blue grows this way only
in his league appearances; his rival and Dojo battles keep their own teams.

On top of that standing, each trainer follows a growth arc chosen once per
save from a short list that fits who they are:

- **Steady**: keeps pace with the world.
- **Early**: rises fast, then others catch up.
- **Late**: slow start, strong finish.
- **Plateau**: a veteran who stops improving.
- **Rival**: a relentless climb.

Veterans such as Bruno or Pryce can plateau; rising stars such as Whitney or
Clair can take off early; Blue only ever climbs fast. The arc never changes
within a save, and it follows the trainer everywhere: their Gym, their league
appearances, and any other enrolled encounter. Every arc starts level, so a
trainer's very first encounter is the same in every save.

Arcs keep running for the first few circuit editions after the journey ends.
A fast starter may have peaked and drifted back, a slow starter may finally be
at their best, and a veteran may find a second wind. After about three
completed editions everyone settles, and only rotation keeps the fields
changing.

### Arcs are hinted, never labelled

Players discover arcs in the world. Gym dialogue, gossip from townsfolk, and
league lineup previews carry lines such as "Clair has been training nonstop."
No menu shows a trainer's arc or numbers.

### True potential: aces and fillers

Every trainer has a true potential: a hand-written roster of one to three
signature aces plus a wider pool of supporting Pokémon, the fillers. How much of
it you face grows with the trainer's own strength. A leader met early brings two
Pokémon, and a full team of six appears once they are strong enough. Because
strength follows each trainer's arc, a fast riser fields a bigger team sooner.
League trainers follow the same rule, so a mid-game contender may bring four or
five while the headliner brings six. A team never shrinks as the world advances.
Only in the post-game, once levels reach their ceiling, can a rival whose form
is fading drop a level or two between editions.

Aces come first: the top ace is always there, and a trainer with more aces
brings them out as the team grows. The ace you face last is the one the
trainer is known for. Every Pokémon evolves along a hand-written path, so
Brock's Onix can become Steelix once it is strong enough.

Fillers vary from save to save. Each trainer has favourites, but which of
them make the team differs between playthroughs, and what you do in the world
can nudge it. For example, telling a trainer where to find a Pokémon they have
been looking for could bring it onto their team. Once a filler has joined, it
stays as the team grows; only something you do can swap it out.

In future content, trainers may accept gifts that help a Pokémon evolve or offer
trades: your traded Pokémon keeps its name and identity, joins their team, and
grows with them. [Trainer roster influence](trainer-roster-influence.md) owns
these hooks. A trainer never trades away an ace.

FRLG, Emerald, and HNS parties are references for recognizable content and
strength, not mandatory opening teams. Brock can open with a small Geodude and
Onix team that a fresh player can handle.

## Encounters

A Gym battle takes its opponent from the world as it stands when the battle
starts and keeps that team for the whole fight. Retrying without earning
anything new gives the same team at the same levels; earning a badge or first
league clear elsewhere first means the leader has grown too. The badge is
awarded after the battle, so it never strengthens the leader mid-fight.

A league competition freezes its five trainers, their teams, and their strength
when the player enters; nothing done between attempts changes them. Losing lets
the player retry the same field as often as they like; badges earned in between
make the retry easier. The [Seeded Trainer Circuit](seeded-trainer-circuit.md)
owns league fields.

## Boundaries

- The player's own rating, soft cap, experience, obedience, wild encounters,
  shops, ordinary trainers, and Gym members are unchanged.
- Only explicitly enrolled encounters use this model. Story, rival, Dojo,
  rematch, and facility battles keep their policies; Red stays separate.
- Tate and Liza keep their existing double Gym battle.
- Standalone builds are unchanged.

## Balance

Gym Leaders stay near the cap on average: fast arcs a little above, slow arcs a
little below, and the gap does not widen as clears pile up. Blue is always a
few levels ahead and never a wall. Once the player's cap reaches level 100,
trainers are held a few levels lower so that league fields still ramp just
below 100, and Blue and the headliners stay a step above that base rather than
above level 100. All numbers remain provisional until content
review and playtesting.

The [explorer](../../devtools/ui/README.md#trainer-balance-explorer) is being
reworked to model arcs, each trainer's gap to the cap, and whether each league
venue can fill its field. It predicts species, team size, and levels only;
moves, items, AI, and combat difficulty belong to playtesting.

## Open questions

- Each trainer's standing, allowed arcs, arc shapes, roster (aces, supporting
  pool, and their favourites), and hint lines are content under review.

## References

- [Trainer world progression specification](../specs/trainer-world-progression.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Trainer roster influence](trainer-roster-influence.md)
- [Seeded Trainer Circuit](seeded-trainer-circuit.md)
- [Player progression](../specs/trainer-rating-party-progression.md)
