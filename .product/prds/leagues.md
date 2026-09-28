# Leagues

Implemented: No
Design status: v0 approved: each lineup is a seeded draw of five from the ten
strongest trainers, weighted by willingness (favouring those at home and
fresh); ascending
battle order; and a locked lineup kept until the league is won. Balance is
informational for now. Signup and league order are out of scope for v0.
Terms follow the [glossary](player-trainer-rating.md#glossary).

## Intent

League lineups should be made of the strongest notable trainers in the
world rather than fixed room occupants, and feel like a living circuit where
who turns up depends on where the league is and who has just played. Each
opponent fights as the same person the player meets elsewhere, at their own
Trainer Rating (TR) and with their own team, so a league battle and a Gym
battle with Clair are the same Clair.

On adoption, Leagues replaces today's fixed lineups and their levels scaled
from the player's TR when entering a league
([League scaling](league-scaling.md),
[interregional League circuit](wayfarer-interregional-league-circuit.md)),
which remain the record of Today.

## Design

**Leagues.** Indigo, Sevii Masters, and Hoenn keep their public entrances,
rooms, and ceremonies. Each holds a five-match singles lineup.

**Pool.** Every notable singles trainer can be invited to any league. Tate &
Liza are notable but fight only as a double battle, so they are left out; Red
(separate mastery encounter) is not notable.

**Home and away.** Every notable trainer has a home region and a travel
style, homebody or traveller ([Notable trainers](notable-trainers.md)). A
league is a location: Indigo belongs to Kanto and Johto, and Hoenn to Hoenn,
so trainers from there are at home. Sevii Masters is neutral ground, where
everyone is at home.

**Lineup.** When the player enters a league, the ten strongest trainers by
current TR are the contenders, and five of them are drawn. Trainers prefer to
play at home: a homebody rarely makes the trip to a league away from home,
while a traveller roams almost as readily as they play at home. Someone who
was in the lineup of the last league the player entered is tired and likely
to skip this one. Nobody is ruled out: even a tired homebody far from home has
a small chance. The draw comes from the playthrough seed, so it differs
between saves but never changes on reload
([Playthrough-seeded variation](playthrough-seeded-variation.md)). It happens
once, on the first entry, and the lineup is then locked until the league is
won. A replay after winning draws a fresh lineup.

**Battle order.** Ascending TR: the weakest of the five fights first and the
strongest last.

**Strength.** Each opponent uses their own TR, team, and levels, exactly as in
any other battle with them. There is no league-specific adjustment.
[Notable trainers](notable-trainers.md) owns trainer TR and how it grows,
the scalers that turn TR into team level and size, and rosters. The player's
TR enters only through how far each trainer has grown.

**Losing.** The lineup is set when the player enters the league and stays
locked to the same five trainers with the same teams until the player wins the
league. A loss blacks the player out as usual, and neither a loss nor leaving
changes the locked lineup. They can retry at once or leave, train, and come
back as often as they like; saving and reloading keep the lineup.

**Winning.** A win commits the league result. The first league win keeps
today's records, ceremonies, and unlocks, but gives the player no TR: a league
is a test, not a source of power
([Player Trainer Rating](player-trainer-rating.md)). Today, the ROM still adds
TR for a first league win until that design is adopted.

**Balance.** Informational for now: the balance explorer reports how likely
each trainer is to appear at each league and which lineups are likely, and
tuning comes later. The lineup is drawn from the strongest trainers of the
moment, so it tends to sit a little above the player; with all 24 badges both
sides reach level 100, and tougher endgame teams (better items, stats,
movesets) are Later.

## Sample playthrough

1. The player qualifies for Indigo under today's circuit rules and enters.
   Five of the ten strongest trainers are drawn: mostly Kanto and Johto
   trainers, perhaps with a travelling Steven. They fight weakest first, each
   with the team they would bring anywhere else.
2. They win matches 1-4 and lose match 5. They black out, and the league now
   holds that locked lineup.
3. They leave, win another badge, level up, and return to the same five with
   the same teams. This time they win, and the league result is committed.
4. At Sevii Masters, entered with more badges, new contenders are worked out
   from everyone's current TR. Sleepers may have climbed in and Veterans
   dropped out. Anyone from anywhere is at home here, but the five who just
   played Indigo are tired and mostly sit this one out.

## Records and recognition

Today's circuit behaviour stays until designed; see the
[interregional League circuit](wayfarer-interregional-league-circuit.md).

- Indigo's first win grants the shared Kanto/Johto Champion recognition with
  one Hall of Fame registration and Champion Ribbon flow.
- Masters records its result in the Masters Gallery and never grants a
  regional Champion title, Hall of Fame registration, or Ribbon.
- Hoenn keeps its own Champion recognition, Hall of Fame, and regional
  cleanup, and its first league win runs the full completion credits.
- Red unlocks after all three leagues have been won. Blue's Saffron Dojo battle
  unlocks after the first Indigo win.
- Whoever fights last is presented as the final opponent without inventing
  Champion history. Dialogue never assumes a fixed person in any room.

## Boundaries

In: the three leagues, notable Kanto, Johto, and Hoenn singles trainers,
home and away, the seeded lineup draw, battle order, and the locked lineup.

Out of scope for v0: signup and qualification, league order, rotation, repeat
editions, and balance targets for leagues. The current circuit's signup,
order, replay, record, ceremony, and unlock rules stay as they are until
designed. Player TR
follows [Player Trainer Rating](player-trainer-rating.md), where league wins
add nothing. Wild encounters, shops, regular trainers, and standalone builds
follow their own contracts; other battles with notable trainers follow their
own TR.

## Specifications

- [Leagues specification](../specs/leagues.md): registry, eligibility,
  contenders, fatigue, the lineup draw, ordering, the locked lineup, battle
  construction, the win commit, saved state, and load validation.
- [Notable trainers specification](../specs/notable-trainers.md): TR, home
  regions, travel styles and willingness, scalers, and rosters.
- [Player Trainer Rating specification](../specs/player-trainer-rating.md):
  the player's TR, which league wins do not raise in v0.
- [Playthrough seed framework](../specs/playthrough-seed-framework.md): the
  root seed and keyed draw behind lineups.

## Later

- Signup and qualification gates designed for these leagues.
- Balancing tools: tuning travel cost, fatigue, and the number of contenders
  against appearance odds, and league balance targets.
- Seeded league order, rotation, and recurring editions.
- Role windows (such as elite and headliner) and standing-based selection.
- Matchmaking lineups around the player's TR.
- Off-screen NPC leagues, TV and Match Call news, and NPC badge records.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Notable trainers](notable-trainers.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [League scaling](league-scaling.md)
- [Playthrough-seeded variation](playthrough-seeded-variation.md)
- [Player progression](../specs/trainer-rating-party-progression.md)
