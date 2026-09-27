# Leagues

Implemented: No
Design status: v0 approved: top-five-by-TR fields drawn from one global pool,
ascending battle order, and a venue locked to its field until won. Signup,
venue order, and everything seeded are out of scope for v0.

## Intent

League fields should be made of the strongest well-known trainers in the
world rather than fixed room occupants. Each opponent fights as the same
person the player meets elsewhere, at their own Trainer Rating (TR) and with
their own team, so a league battle and a Gym battle with Clair are the same
Clair.

On adoption, Leagues replaces today's fixed lineups and their levels scaled
from the player's entry TR
([League scaling](league-scaling.md),
[interregional League circuit](wayfarer-interregional-league-circuit.md)),
which remain the record of the current ROM.

## Design

**Venues.** Indigo, Sevii Masters, and Hoenn keep their public entrances,
rooms, and ceremonies. Each holds a five-battle singles field.

**Pool.** One global pool serves all three venues: every well-known singles
trainer. Red (separate mastery encounter) and Tate & Liza (double battle) are
excluded. Region, title, and home venue play no part.

**Field.** Each venue fields the five pool trainers with the highest TR. Ties
fall in whatever order iteration returns; there is no tie-break rule. Because
the pool is shared, every venue may field the same five people, and v0 accepts
that.

**Battle order.** Ascending TR: the weakest of the five fights first and the
strongest last.

**Strength.** Each opponent uses their own TR, team, and levels, exactly as in
any other battle with them. There is no league-specific adjustment.
[Well-known trainers](well-known-trainers.md) owns trainer TR, the scalers
that turn TR into team level and size, and rosters. The player's TR never
enters it.

**Losing.** The field is set when the player enters and stays locked to the
same five trainers with the same teams until the player beats the venue. A
loss blacks the player out as usual, and neither a loss nor leaving changes
the field. They can retry at once or leave, train, and come back as often as
they like; saving and reloading keep the field.

**Winning.** A win commits the venue result. Its first-ever win keeps the
current first-clear effects, including the player's +8 TR.

## Sample playthrough

1. The player qualifies for their first venue under the current circuit rules
   and enters. The five highest-TR well-known trainers are fielded, weakest
   first, each with the team they would bring anywhere else.
2. They beat four and lose to the fifth. They black out, and the venue now
   holds that same field.
3. They leave, win another badge, level up, and return to the same five with
   the same teams. This time they win, and the venue is committed.
4. At the next venue the field may be the same five people. In v0 that is
   expected.

## Records and recognition

Current circuit behaviour stays until designed; see the
[interregional League circuit](wayfarer-interregional-league-circuit.md).

- Indigo's first win grants the shared Kanto/Johto Champion recognition with
  one Hall of Fame registration and Champion Ribbon flow.
- Masters records its result in the Masters Gallery and never grants a
  regional Champion title, Hall of Fame registration, or Ribbon.
- Hoenn keeps its own Champion recognition, Hall of Fame, and regional
  cleanup, and its first clear runs the full completion credits.
- Red unlocks after all three venues have been won. Blue's Saffron Dojo battle
  unlocks after the first Indigo win.
- Whoever fights last is presented as the final opponent without inventing
  Champion history. Dialogue never assumes a fixed person in any room.

## Boundaries

In: the three venues, well-known Kanto, Johto, and Hoenn singles trainers,
field selection, battle order, and the field lock.

Out of scope for v0: signup and qualification, venue order, seeding, rotation,
repeat editions, and home crowds. The current circuit's signup, order, replay,
record, ceremony, and unlock rules stay as they are until designed. Player TR
keeps its current formula. Wild encounters, shops, ordinary trainers, and
standalone builds keep their current contracts; other battles with well-known
trainers follow their own rating.

## Specifications

- [Leagues specification](../specs/leagues.md): registry, eligibility,
  selection, ordering, the frozen field, battle construction, the win commit,
  saved state, and load validation.
- [Well-known trainers specification](../specs/well-known-trainers.md): TR,
  scalers, and rosters.
- [Player Trainer Rating specification](../specs/player-trainer-rating.md):
  the player's TR, including its +8 first-clear contributions.

## Later

- Signup and qualification gates designed for this circuit.
- Seeded venue order, per-save variation, rotation, and recurring editions.
- Home crowds with an 85/15 home/visitor draw, and Masters as an open
  invitational.
- Role windows (contender, elite, headliner) and standing-based selection.
- Matchmaking fields around the player's TR.
- Off-screen NPC leagues, TV and Match Call news, and NPC badge records.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Well-known trainers](well-known-trainers.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [League scaling](league-scaling.md)
- [Playthrough-seeded variation](playthrough-seeded-variation.md) (parked)
- [Player progression](../specs/trainer-rating-party-progression.md)
