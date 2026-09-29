# Trainer AI

Implemented: No. Today, every trainer battles the way its party data sets;
the balance explorer shows the new behaviour as placeholder tooling.
Design status: v0 accepted. Each trainer's play style is approved and open
to review; how quickly trainers get smarter is placeholder, tuned by
playtesting. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Make famous trainers battle like themselves. A Gym Leader should not only
bring the right Pokémon but use them the way their story suggests: Brock
builds a wall of rocks, Whitney's Miltank will not go down, Wattson happily
blows his own Pokémon up. As trainers grow stronger they should also play
smarter, so a late rematch feels different from an early one, and the very
top should feel like facing someone who has seen it all.

## Design

### Every trainer has a play style

Every [notable trainer](notable-trainers.md) has one **play style**, their
battle identity. It never changes, whatever their strength:

| Play style | How they play | Trainers |
| --- | --- | --- |
| Gambler | Swings for big damage and takes risks. | Blaine, Lt. Surge, Flannery, Winona |
| Bomber | A gambler who also blows up their own Pokémon. | Wattson |
| Sweeper | Sets up on the first turn, then sweeps. | Norman, Lorelei, Clair, Drake, Sidney, Bugsy |
| Field marshal | Controls the field first: weather, hazards, screens. | Brock, Steven, Misty, Will, Tate & Liza, Falkner |
| Hexer | Wears you down with status moves. | Erika, Morty, Sabrina, Juan, Janine, Koga, Agatha, Karen, Phoebe |
| Turtle | Plays safe and lasts. | Whitney, Pryce, Wallace, Jasmine, Glacia, Roxanne |
| Brawler | Hits as hard as possible. | Bruno, Chuck, Brawly, Giovanni |
| Tactician | No gimmick: picks the right move for the moment. | Lance, Blue |

Everyone keeps the sensible basics: nobody wastes turns on moves that cannot
work, and everyone goes for the knockout when it is there. A play style
shapes what they prefer on top of that; it never scripts their moves.

### Stronger trainers play smarter

A trainer's **AI skill** follows their own strength, not yours. Early on,
they simply play their style. As they grow they send in better Pokémon after
a knockout and read which moves your Pokémon are likely to have; stronger
still, they switch out of bad matchups and guess your ability; at the top
they predict your switches and your next move. Meeting a leader again later
in your journey feels different because they have grown.

### Aces are held back

A trainer's aces, the stars of their team, are kept back until the end: the
trainer will not send them out early to patch a bad matchup. With one ace
that is their signature Pokémon; with two or more, the last two.

### A few bosses know everything

A rare **boss** knows your whole party: every move, ability, and held item.
In this first version the only boss is Lance; Red will join him once he is a
notable trainer.

## Boundaries

- Regular trainers, Gym members, wild Pokémon, and facilities keep their
  current behaviour.
- Randomizer and challenge options keep working as they do today.
- Tate & Liza keep their double battle and share one play style.
- Standalone builds are unchanged.

## Balance

Play styles are about feel, not raw difficulty; strength still comes from a
trainer's rating and team. How fast trainers get smarter is placeholder and
tuned by playtesting.

## Later

- Regular trainers growing smarter with your rating.
- Trainers using items more cleverly as they grow.
- More play styles for new trainers.
- Holding back a third ace.

## Specifications

- [Trainer AI specification](../specs/trainer-ai.md): play styles,
  AI skill, ace protection, the boss flag, and how a battle applies them.

## References

- [Notable trainers](notable-trainers.md)
- [Leagues](leagues.md)
- [Player Trainer Rating](player-trainer-rating.md)
