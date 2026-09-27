# Notable trainers

Implemented: No. Today, the ROM keeps its existing Gym and league scaling
until this design is adopted; the balance explorer is placeholder tooling.
Design status: v0 accepted. Each trainer's TR and team, and the exact growth
steps, are placeholder content under review. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Make familiar trainers feel like people with a place in the world. Gym
Leaders, Elite Four members, Champions, and Blue each have a strength of their
own, measured the same way as the player's. Whenever you meet one of them,
anywhere, you face the same trainer at the same strength, and you can tell at
a glance roughly how you compare.

## Design

### Their own rating, on your scale

You have a Trainer Rating (TR); so does every notable trainer. Neither is
worked out from the other: your training, badges, or party never make a
trainer stronger or weaker. Their TR uses the same scale as yours, so a leader
rated like a player with eight badges brings Pokémon around the level your own
cap reaches with eight badges.

A trainer's TR decides every battle with them: their Gym, a meeting on the
road, a story battle, a rematch, or a league. Story battles include Blue's
rival fights, Giovanni's Rocket battles, and the Saffron Dojo. There are no
special cases. In this first version each trainer's TR is set by hand and does
not change, so their team is the same every time and in every save. That
includes Blue: his early rival fights and his late ones bring the same team.

### Who is notable

In this first version the notable trainers are 37 people: the 23 singles
Gym Leaders, the Kanto, Johto, and Hoenn Elite Four, Lance, Wallace, Steven,
and Blue. Everyone else, including Red and Tate and Liza, keeps their current
rules.

### No ceiling

Like yours, a trainer's TR has no ceiling
([Player Trainer Rating](player-trainer-rating.md#no-ceiling)). Today's
trainers span roughly the same range as the player, but stronger future
content can go higher.

Every trainer's TR is a placeholder for now, set on the new badge scale
([Player Trainer Rating](player-trainer-rating.md)) and re-set with
playtesting.

### Bigger and stronger teams at higher TR

A higher TR means both higher levels and a bigger team, on the same curve as
your level cap. A low-rated trainer brings two Pokémon; a team grows by one at
roughly levels 20, 30, 45, and 60; a trainer rated like a player with all 24
badges brings a full six at level 100. The strongest trainers are meant to
sit a little above the eight-badge mark, so a player with eight badges can win
their first league ([Leagues](leagues.md)). Each Pokémon has a small, hand-set
level difference, so a team feels shaped rather than uniform.

### The signature Pokémon comes last

Every trainer has one hand-picked list of six Pokémon, the same in every
battle; Blue brings the same six whichever starter you chose. A small team is
the start of that list, and each step up adds the next one. The first Pokémon
on the list is their signature Pokémon: it is on every team they bring, and
you always face it last.

FRLG, Emerald, and HNS parties are references for recognizable content, not
required teams. Challenge options such as trainer items, trainer IVs and EVs,
and the level cap apply on top, as they do today.

## Encounters

A battle's team is set when it starts and kept for the whole fight. Retrying
brings the same team at the same levels. League opponents also fight with
their own TR and team; [Leagues](leagues.md) decides who is in each lineup,
which is set when you enter a league and kept until you win.

## Gym battles

Players can take the Gyms in any order and always find a fair, recognizable
fight. A low-rated leader is a good first challenge; a high-rated one is a
clear goal to build towards.

A Gym battle uses the leader's own TR, team, and levels. The Gym adds nothing
on top: the same leader met anywhere else is the same trainer at the same
strength. The badge is awarded after the battle. Until adoption, the ROM keeps
today's behavior: the earlier player-TR Gym scaler exists in code but is
disabled by default, and Giovanni's Wayfarer finale has its own path.

Wayfarer has 24 badge encounters. This design covers the 23 singles badge
opponents: Brock, Misty, Lt. Surge, Erika, Janine, Sabrina, Blaine, Giovanni
(his Viridian finale); Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine, Pryce,
Clair; Roxanne, Brawly, Wattson, Flannery, Norman, Winona, and Juan.

Tate and Liza's double battle keeps its existing policy and badge; it is not
converted to singles. Blue has no badge encounter in Wayfarer.

Each leader's hand-written team supplies exact species, moves, items, and
abilities, which stay attached to the right Pokémon when a team is reordered.
Rewards, prize money, badge scripts, and AI are preserved unless a team
deliberately changes them. Trainer-species randomization keeps its existing
path; other randomizer and challenge options keep their precedence.

## Boundaries

- Your own TR, level cap, experience, obedience, wild and static encounters,
  shops, regular trainers, and Gym members follow your TR under their own
  rules ([Player Trainer Rating](player-trainer-rating.md)).
- Tate and Liza keep their existing double Gym battle.
- The same TR decides every battle with a notable trainer; how each
  battle is built stays with the
  [Gym Leader scaling specification](../specs/gym-leader-scaling.md).
- Standalone builds are unchanged.

## Balance

Each trainer's TR and team are content under review. Leader ratings and teams
need ROM playtesting before enablement, including checks that Gym members do
not routinely outclass their leader. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) predicts
species, team size, and levels only; moves, items, AI, and combat difficulty
belong to playtesting.

## Later

- More notable trainers, such as Red or Tate and Liza.
- A meaning for the "next Gym's highest or lowest level" cap options in an
  open world.
- Trainers whose TR grows through their own battles, journeys, or time.
- Growth arcs and other per-save variety in how trainers develop.
- Signature and supporting Pokémon that vary from save to save.
- Player influence: nudges, gifts, and trades
  ([Trainer roster influence](trainer-roster-influence.md)).
- Pokémon that evolve along a trainer's own line.
- Better items, moves, and AI once teams reach level 100.
- Tighter level spreads at the top.

## Specifications

- [Notable trainers specification](../specs/notable-trainers.md):
  inventory, TR, scalers, rosters, and the battle snapshot.
- [Gym Leader scaling](../specs/gym-leader-scaling.md): badge-encounter
  coverage and battle construction.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Leagues](leagues.md)
- [Regular trainer and Gym member scaling](trainer-party-scaling.md)
- [Player progression](../specs/trainer-rating-party-progression.md)
