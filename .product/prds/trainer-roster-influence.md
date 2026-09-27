# Trainer roster influence

Implemented: No
Design status: Parked: not part of v0.

This is a design note kept for later. v0 trainers field a fixed team from one
hand-written list ([well-known trainer rating](trainer-world-progression.md)),
so there is nothing for the player to influence yet. These ideas depend on a
future rule that lets trainers pick some supporting Pokémon dynamically.

## Ideas

- **Nudges.** Conversations, quests, or favours make a trainer more or less
  keen on a supporting Pokémon. Telling Misty where Lapras lives could put
  Lapras on her team next time she has room. Nudges never touch signature
  Pokémon or team size, and are permanent for the save.
- **Evolution gifts.** A trainer accepts an item such as a Water Stone, and
  that Pokémon appears evolved from then on.
- **Trades.** A trainer asks for a species and gives a supporting Pokémon in
  return, never a signature one. The Pokémon you give keeps who it is:
  nickname, shininess, gender, nature, ability, ball, and original trainer. It
  becomes a strong favourite on their team and grows with them. It keeps every
  move it knew and learns new ones only by levelling up; its new trainer never
  teaches it anything.

## Open questions

- Which interactions, gifts, and trades to author, and for which trainers.
- The total trade budget in the save.

## References

- [Trainer roster influence specification](../specs/trainer-roster-influence.md)
- [Well-known trainer rating](trainer-world-progression.md)
