# Notable trainers

Implemented: No. Today, the ROM keeps its existing Gym and league scaling
until this design is adopted; the balance explorer is placeholder tooling.
Design status: v0 accepted. Each trainer's TR, growth, and team, and the
exact team-size steps, are placeholder content under review. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Make familiar trainers feel like people with a place in the world. Gym
Leaders, Elite Four members, Champions, and Blue each have a strength of their
own, measured the same way as the player's, and they keep pace with your
journey in their own way. Whenever you meet one of them, anywhere, you face the
same trainer at their current strength, and you can tell at a glance roughly
how you compare.

## Design

### Their own rating, on your scale

You have a Trainer Rating (TR); so does every notable trainer. Yours is never
worked out from theirs. Their TR uses the same scale as yours, so a leader
rated like a player with eight badges brings Pokémon around the level your own
cap reaches with eight badges.

A trainer's TR decides every battle with them: their Gym, a meeting on the
road, a story battle, a rematch, or a league. Story battles include Blue's
rival fights, Giovanni's Rocket battles, and the Saffron Dojo. There are no
special cases.

### They keep pace with your journey

Notable trainers grow as your journey goes on, each in their own way:

- **The steady Gym Leader** keeps a fixed share of your pace, from their
  starting strength to their best.
- **The early bloomer** grows fast early and then slows: a wall in the middle
  of your journey.
- **The late bloomer** starts slow and finishes strong: a challenge waiting
  late in the game.
- **The veteran** reaches their best early and stops there; eventually you
  overtake them.
- **Blue** starts level with you and pulls a step ahead over your first
  badges, then stays there until he reaches his best.

Everyone grows by the same rule: only the shape, the starting strength, and
the best differ. They only move when you do. A big step, like one of your first badges, moves
them a lot; a small one barely moves them; wandering around moves nobody, and
saving and reloading changes nothing. Every trainer has a best they never go
past. Their growth is the same in every save.

### Who is notable

In this first version there are 38 notable trainers: 37 people (the 23
singles Gym Leaders, the Kanto, Johto, and Hoenn Elite Four, Lance, Wallace,
Steven, and Blue) plus Tate and Liza as one duo. Tate and Liza share one
strength and one list of six Pokémon, grow like everyone else, and still fight
you together in their double battle. Leagues are singles only, so they never
appear in one. Everyone else, including Red, keeps their current rules.

### No ceiling

Like yours, a trainer's TR has no ceiling
([Player Trainer Rating](player-trainer-rating.md#no-ceiling)). Today's
trainers span roughly the same range as the player, but stronger future
content can go higher.

Every trainer's starting strength, growth, and best are placeholders for now,
set on the new badge scale
([Player Trainer Rating](player-trainer-rating.md)) and re-set with
playtesting.

### Bigger and stronger teams at higher TR

A higher TR means both higher levels and a bigger team. Early fights are fair:
the lowest-rated trainers bring a single Pokémon, so Blue's first fight is one
on one at your level (level 5), and he pulls ahead over your first badges.
For trainers rated like a player with four badges or more, levels follow the
same curve as your level cap, and a trainer rated like a player with all 24
badges brings a full six at level 100. When you have eight badges, the
strongest trainers are meant to sit a little above you, so you can win your
first league; later leagues stay a real fight a little above your level cap,
and at the very end both sides meet at level 100 ([Leagues](leagues.md)). Each Pokémon has a small, hand-set
level difference, so a team feels shaped rather than uniform.

### The signature Pokémon comes last

Every trainer has one hand-picked list of six Pokémon, the same in every battle
and every save; Blue brings the same six whichever starter you chose. A small
team is the start of that list, and each step up adds the next one. The first
Pokémon on the list is their signature Pokémon: it is on every team they bring,
and you always face it last.

Lists name each Pokémon at its final form, such as Brock's Steelix and Golem.
Stronger forms appear only once they've reached the right level: until then a
Pokémon comes as an earlier form, so Brock opens with Onix and Geodude and
brings Steelix once his team reaches level 35. Pokémon never evolve past what
the list names, and a trainer can name an earlier form on purpose, like Blue's
Eevee. Hand-picked moves belong to the named form; an earlier form uses its
usual moves for its level
([evolution stages](../specs/player-trainer-rating.md#evolution-stages)).

FRLG, Emerald, and HNS parties are references for recognizable content, not
required teams. Challenge options such as trainer items, trainer IVs and EVs,
and the level cap apply on top, as they do today.

## Encounters

A battle's team is set from the trainer's strength when it starts and kept for
the whole fight. Retrying brings the same team at the same levels, unless you
have grown in the meantime. League opponents also fight with
their own TR and team; [Leagues](leagues.md) decides who is in each lineup,
which is set when you enter a league and kept until you win.

## Gym battles

Players can take the Gyms in any order and always find a fair, recognizable
fight. At any point in your journey some leaders sit below you, some near
you, and some clearly above: good first challenges, fair fights, and clear
goals to build towards.

A Gym battle uses the leader's own TR at that point in your journey, with their
team and levels. The Gym adds nothing on top: the same leader met anywhere else
at that point is the same trainer at the same strength. The badge is awarded
after the battle. Until adoption, the ROM keeps today's behavior: the earlier
player-TR Gym scaler exists in code but is disabled by default, and Giovanni's
Wayfarer finale has its own path.

Wayfarer has 24 badge encounters, and this design covers all of them: Brock, Misty, Lt. Surge, Erika, Janine, Sabrina, Blaine, Giovanni
(his Viridian finale); Falkner, Bugsy, Whitney, Morty, Chuck, Jasmine, Pryce,
Clair; Roxanne, Brawly, Wattson, Flannery, Norman, Winona, Tate and Liza, and
Juan.

Tate and Liza stay a double battle, with both of them sending out Pokémon from
their shared list in order. Blue has no badge encounter in Wayfarer.

Each leader's hand-written team supplies species, moves, items, and
abilities, which stay attached to the right Pokémon when a team is reordered.
Rewards, prize money, badge scripts, and AI are preserved unless a team
deliberately changes them. Trainer-species randomization keeps its existing
path; other randomizer and challenge options keep their precedence.

## Boundaries

- Your own TR, level cap, experience, obedience, wild and static encounters,
  shops, regular trainers, and Gym members follow your TR under their own
  rules ([Player Trainer Rating](player-trainer-rating.md)).
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

- More notable trainers, such as Red.
- A meaning for the "next Gym's highest or lowest level" cap options in an
  open world.
- Growth shapes that differ from save to save, and other per-save variety.
- Faster or slower growers, and hand-made growth shapes for single trainers.
- Some of your progress counting more than other progress for how trainers
  grow.
- Trainers who also grow from their own battles.
- Signature and supporting Pokémon that vary from save to save.
- Player influence: nudges, gifts, and trades
  ([Trainer roster influence](trainer-roster-influence.md)).
- Better items, moves, and AI once teams reach level 100.
- Tighter level spreads at the top.

## Specifications

- [Notable trainers specification](../specs/notable-trainers.md):
  inventory, TR and its growth, scalers, rosters, and the battle snapshot.
- [Gym Leader scaling](../specs/gym-leader-scaling.md): badge-encounter
  coverage and battle construction.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Leagues](leagues.md)
- [Regular trainer and Gym member scaling](trainer-party-scaling.md)
- [Player progression](../specs/trainer-rating-party-progression.md)
