# Trainer roster influence

Implemented: No
Design status: Mechanism defined; concrete interactions are future content.

## Intent

Let the player leave a mark on the trainers they meet. Every trainer has a
true potential, a roster of signature aces and supporting Pokémon described in
[trainer world progression](trainer-world-progression.md). This feature lets
things the player does in the world change which supporting Pokémon a trainer
brings, help one of their Pokémon evolve, or put one of the player's own
Pokémon on their team. Meeting Misty again and seeing the Lapras you told her
about should feel like a small story the player caused.

## Design

### Nudging a trainer's favourites

Some conversations, quests, or favours can make a trainer more or less keen on
a supporting Pokémon. Telling Misty where Lapras lives might make her go and
catch one, so Lapras is likely to join her team the next time she has room. A
nudge can also push a Pokémon down the list. Nudges never touch a trainer's
aces and never make a team bigger than the trainer's strength allows.

A nudge is permanent for the save and applies from the trainer's next battle.
A league field already entered keeps the teams it was frozen with until the
player wins it.

### Evolution gifts

A trainer may accept an evolution item for one of their Pokémon, such as a
Water Stone for a Staryu. From then on that Pokémon appears in its evolved
form, even if it is not yet strong enough to evolve on its own. A gift is a
one-off offer and permanent for the save.

### Trades

A trainer may offer a trade: they ask for any Pokémon of a species they want
and give one of their supporting Pokémon in return. A trainer never trades
away an ace.

The Pokémon the player gives keeps who it is: its nickname, shininess, gender,
nature, ability, ball, and original trainer. It becomes one of the trainer's
supporting Pokémon, and a strong favourite: they will almost always bring it
whenever there is room. It grows with its new trainer and evolves as its species
does; a species that evolves by trading evolves at the moment of the trade, as
in the main games. It keeps every move it knew, including taught moves, and
learns new moves only by levelling up. Its new trainer never teaches it
anything.

The Pokémon the trainer gave away no longer appears on their team.

## Boundaries

- Only enrolled trainers with an authored roster can be influenced.
- Aces are never tradable, and nothing here changes a trainer's strength,
  growth arc, or team size.
- The first version defines how influence works. Which conversations, gifts,
  and trades exist is future content.
- No trade-back, trade history screen, or link trading is included.

## Constraints

Each trade is stored as one small record in the save, so the number of trades
across the whole game is limited, about one per trainer at most. Nudges and
gifts are ordinary one-bit flags. Reloading never changes a trainer's team:
the result depends only on the save's seed and what the player has done.

## Playtesting

- Do players notice a nudged Pokémon joining a team and connect it to what
  they did?
- Does meeting a traded Pokémon later feel rewarding rather than punishing?
- Are evolution gifts worth giving when they make the trainer stronger?

## Open questions

- Which interactions, gifts, and trades to author, and for which trainers.
- The total trade budget in the save.

## References

- [Trainer roster influence specification](../specs/trainer-roster-influence.md)
- [Trainer world progression](trainer-world-progression.md)
- [Seeded Trainer Circuit](seeded-trainer-circuit.md)
