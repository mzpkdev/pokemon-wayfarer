# Well-known trainer rating

Implemented: No. The current ROM keeps its existing Gym and League scaling
until this design is adopted; the balance explorer is provisional tooling.
Design status: v0 accepted. Each trainer's rating and team, and the exact
growth steps, are provisional content under review.

## Intent

Make familiar trainers feel like people with a place in the world. Gym
Leaders, Elite Four members, Champions, and Blue each have a strength of their
own, measured the same way as the player's. Whenever you meet one of them,
anywhere, you face the same trainer at the same strength, and you can tell at
a glance roughly how you compare.

## Design

### Their own rating, on your scale

You have a Trainer Rating (TR); so does every well-known trainer. Neither is
worked out from the other: your training, badges, or party never make a
trainer stronger or weaker. Their TR uses the same scale as yours, so a leader
at TR 40 brings Pokémon around the level your own cap reaches at TR 40.

A trainer's TR decides every battle with them: their Gym, a meeting on the
road, a story battle, a rematch, or a league. Story battles include Blue's
rival fights, Giovanni's Rocket battles, and the Saffron Dojo. There are no
special cases. In this first version each trainer's TR is set by hand and does
not change, so their team is the same every time and in every save. That
includes Blue: his early rival fights and his late ones bring the same team.

### Who is well-known

In this first version the well-known trainers are 37 people: the 23 singles
Gym Leaders, the Kanto, Johto, and Hoenn Elite Four, Lance, Wallace, Steven,
and Blue. Everyone else, including Red and Tate and Liza, keeps their current
rules.

### No ceiling

TR has no ceiling. Today's trainers span roughly the same range as the player,
but stronger future content can go higher.

### Bigger and stronger teams at higher TR

A higher TR means both higher levels and a bigger team. A low-rated trainer
brings two Pokémon; a top-rated one brings a full six, around level 100. Each
Pokémon has a small, hand-set level difference, so a team feels shaped rather
than uniform.

### The signature Pokémon comes last

Every trainer has one hand-picked list of six Pokémon, the same in every
battle; Blue brings the same six whichever starter you chose. A small team is
the start of that list, and each step up adds the next one. The first Pokémon
on the list is the one the trainer is known for: it is on every team they
field, and you always face it last.

FRLG, Emerald, and HNS parties are references for recognizable content, not
required teams. Challenge options such as trainer items, trainer IVs and EVs,
and the level cap apply on top, as they do today.

## Encounters

A battle's team is set when it starts and kept for the whole fight. Retrying
brings the same team at the same levels. League opponents also fight with
their own rating and team; the [Trainer Circuit](seeded-trainer-circuit.md)
decides who is in each field, which is set when you enter and kept until you
win.

## Boundaries

- Your own TR, level cap, experience, obedience, wild and static encounters,
  shops, ordinary trainers, and Gym members keep their current rules, which
  follow your TR.
- Tate and Liza keep their existing double Gym battle.
- The same rating decides every battle with a well-known trainer; how each
  battle is built stays with
  [Gym Leader scaling](../specs/gym-leader-scaling.md).
- Standalone builds are unchanged.

## Balance

Each trainer's TR and team are content under review. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) predicts
species, team size, and levels only; moves, items, AI, and combat difficulty
belong to playtesting.

## Later

- More well-known trainers, such as Red or Tate and Liza.
- A meaning for the "next Gym's highest or lowest level" cap options in an
  open world.
- Trainers whose TR grows through their own battles, journeys, or time.
- Growth arcs and other per-save variety in how trainers develop.
- Aces and supporting Pokémon that vary from save to save.
- Player influence: nudges, gifts, and trades
  ([Trainer roster influence](trainer-roster-influence.md)).
- Pokémon that evolve along a trainer's own line.
- Better items, moves, and AI once teams reach level 100.
- Tighter level spreads at the top.

## References

- [Well-known trainer rating specification](../specs/trainer-world-progression.md)
- [Gym Leader scaling](gym-leader-scaling.md)
- [Trainer Circuit](seeded-trainer-circuit.md)
- [Player progression](../specs/trainer-rating-party-progression.md)
