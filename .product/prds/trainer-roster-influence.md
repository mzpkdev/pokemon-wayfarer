# Trainer roster influence

Implemented: No
Design status: Parked: not part of v0.

This is a design note kept for later. In v0 every notable trainer has one
fixed list of six Pokémon: their aces, the stars they're known for, and the
supporting Pokémon in between ([Notable trainers](notable-trainers.md)). These
ideas let the supporting places change; the aces never do.

## Ideas

- **Supporting Pokémon that vary.** Each trainer has a wider group of
  supporting Pokémon to pick from. Their favourites fill the supporting places
  in their list, and the favourites can differ a little from save to save. As
  a trainer grows, new supporting Pokémon join and the ones already there
  stay. Some strong choices wait until the trainer is strong enough.
- **Nudges.** What you do can make a trainer keener on a supporting Pokémon.
  Telling Misty where Lapras lives could put Lapras on her team the next time
  you meet. Nudges are permanent for the save.
- **Trades.** A trainer asks for a species and gives one of their supporting
  Pokémon in return, never an ace. The Pokémon you give keeps who it is:
  nickname, shininess, gender, nature, ability, ball, and original trainer. It
  becomes a favourite on their team, never appears weaker than when you gave it
  away, and evolves as the trainer grows. It keeps its moves and learns new ones
  only by levelling up; its new trainer never teaches it anything.
- **Evolution gifts.** A trainer accepts an item such as a Water Stone, and a
  supporting Pokémon appears evolved from then on.

## Open questions

- Which groups, nudges, and trades to author, and for which trainers.
- The total trade budget in the save.

## References

- [Trainer roster influence specification](../specs/trainer-roster-influence.md)
- [Notable trainers](notable-trainers.md)
