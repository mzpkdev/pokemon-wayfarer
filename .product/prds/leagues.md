# Leagues

Implemented: No
Design status: v0 approved: leagues are seasons in the world, each holding
an event every three in-game days on a staggered calendar; Indigo and Hoenn
open at 8 badges in any order and Sevii Masters at 16 after a regional win;
one attempt per event, free skipping, and a reigning champion between events;
each lineup is the five strongest trainers who are willing to come
(favouring those at home and fresh, with aloof trainers joining only an elite
enough field), with no randomness, fought in ascending order. Balance is
informational for now. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Leagues should feel like seasons in a living world rather than a fixed
ladder to climb once. Each league holds its tournaments on its own rhythm
whether or not the player turns up; the player can enter, lose, skip, and come
back, and between tournaments someone always holds the title. Lineups are made
of the strongest notable trainers in the world rather than fixed room
occupants, and who turns up depends on where the league is and who has just
played. Each
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
rooms, and ceremonies. Each league event is a five-match singles lineup.

**Calendar.** Each league holds an event every three in-game days, and the
three are staggered so that somewhere there is an event every day: Indigo,
then Hoenn, then Sevii Masters, then Indigo again. The days come from the
game's existing day counter. Days only schedule events: nobody grows stronger
or weaker with time, only with the world's progress.

**Entry.** Indigo and Hoenn open once the player has 8 badges, from any
regions, and can be tried in either order. Sevii Masters is the invitational:
it opens at 16 badges for a player who has won Indigo or Hoenn at least once.

**Pool.** Every notable singles trainer can be invited to any league. Tate &
Liza are notable but fight only as a double battle, so they are left out; Red
(separate mastery encounter) is not notable.

**Home and away.** Every notable trainer has a home region, and some have
the traveller trait ([Notable trainers](notable-trainers.md)). A
league is a location: Indigo belongs to Kanto and Johto, and Hoenn to Hoenn,
so trainers from there are at home. Sevii Masters is neutral ground, where
everyone is at home.

**Lineup.** The strongest trainers who are willing to come make the lineup.
When the player enters a league, every trainer gets a league score: their
current TR, scaled down by how unwilling they are to come. Trainers prefer to
play at home: most rarely make the trip to a league away from home, while a
traveller roams almost as readily as they play at home. Someone who played
in the most recent event anywhere, whether or not the player was there, is
tired and less keen on this one.

Some proud trainers are aloof: they won't bother with a league whose base
lineup is far below them, but they join once it is elite. The base lineup is
the five best trainers by league score who are not aloof, and the base lineup
level is the highest team level among them. An aloof trainer joins only if
their own team level is no more than 10 above the base lineup level; aloof
trainers only measure themselves against the base lineup, never against each
other, and with no base lineup at all they skip. The Champions are aloof, so
early events go ahead without them, and they turn up once the base lineup is
strong enough to deserve them.

Everyone still eligible is then ranked by league score, and the five highest
make the lineup. Apart from the aloof, nobody is ruled out: a strong enough
trainer shows up even far from home or tired.
There is no dice roll, so the same world progress and the same last event
always give the same lineup. It is worked out when the player enters and
stays the same for that event.

**Battle order.** Ascending TR: the weakest of the five fights first and the
strongest last.

**Strength.** Each opponent uses their own TR, team, and levels, exactly as in
any other battle with them. There is no league-specific adjustment.
[Notable trainers](notable-trainers.md) owns trainer TR and how it grows,
the scalers that turn TR into team level and size, and rosters. The player's
TR enters only through how far each trainer has grown.

**One attempt.** Each event gives the player one attempt. A loss blacks the
player out as usual and ends that event for them; leaving midway does too.
They can't re-enter it, but the league's next event is only three days away,
and by then they may have trained, earned badges, and face a different five.

**Skipping.** Missing an event costs nothing. The world doesn't wait: the
event goes ahead without the player.

**Reigning champion.** Every event crowns someone. If the player wins, they
are the league's reigning champion. If they skip or lose, the strongest
trainer of that event's lineup takes the title. Either way the champion reigns
until the league's next event, when the title is on the line again.

**Winning.** The player's first win at each league keeps today's records,
ceremonies, and unlocks, all once only. Winning again brings prize money and
the title back. No win gives the player TR: a league is a test, not a source of
power ([Player Trainer Rating](player-trainer-rating.md)). Today, the ROM
still adds TR for a first league win until that design is adopted.

**Balance.** Informational for now: the balance explorer simulates the
calendar day by day, showing who fields each event, whether the player could
enter, and who reigns, and for any event each trainer's league score, the
base lineup level, and which aloof trainers join or skip; tuning comes later. The lineup comes from the strongest trainers of the moment, so it tends to sit a little above the player; with all 24 badges both
sides reach level 100, and tougher endgame teams (better items, stats,
movesets) are Later.

## Sample playthrough

1. With 8 badges, the player reaches Indigo on a Hoenn day. The lobby names
   Indigo's reigning champion, whoever topped its last event, and says the
   next event is in two days. They could fly to Hoenn, whose event is today,
   but they go training instead.
2. They return on Indigo's day and enter. The lineup is the five strongest who
   are willing to come: Kanto and Johto trainers at home, unless a traveller
   from Hoenn is strong enough to beat them even after the trip. Anyone who
   played Hoenn's event yesterday is tired. They fight weakest first, each with
   the team they would bring anywhere else.
3. They win matches 1-4 and lose match 5. They black out, and this event is
   over for them. When the day ends, that finalist is Indigo's reigning
   champion.
4. They skip the next few events while they win badges. The events go ahead
   without them, and titles change hands.
5. Three days or more later, they enter Indigo again and win. It is their
   first Indigo win: the Hall of Fame, the Kanto/Johto Champion recognition,
   and Blue's Dojo battle follow, and they reign until Indigo's next event.
6. With 16 badges and a regional win, Sevii Masters opens. Anyone from
   anywhere is at home there. An aloof Champion who skipped earlier events
   may still find the base lineup too far below them, and turn up only once
   the world's best are strong enough.
7. Later, they enter Indigo again and win: prize money and the title, with no
   second Hall of Fame.

## Records and recognition

The player's first win at each league keeps Today's records, as the
[interregional League circuit](wayfarer-interregional-league-circuit.md)
implements them; repeat wins give prize money and the title only.

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
- Each lobby names the reigning champion and when the next event is.

## Boundaries

In: the three leagues, the calendar and league events, entry rules, one
attempt per event, skipping, reigning champions, first and repeat wins,
notable Kanto, Johto, and Hoenn singles trainers, home and away, the league
score, the traveller and aloof traits and the base lineup level, the lineup,
and battle order.

Out of scope for v0: rotation, special events, off-screen results news, and
balance targets for leagues. Today's circuit, with its fixed order and
replays, stays documented as Today until this design is adopted; its record,
ceremony, and unlock rules carry over to first wins. Player TR
follows [Player Trainer Rating](player-trainer-rating.md), where league wins
add nothing. Wild encounters, shops, regular trainers, and standalone builds
follow their own contracts; other battles with notable trainers follow their
own TR.

## Specifications

- [Leagues specification](../specs/leagues.md): registry, eligibility, the
  calendar, entry rules, fatigue, the league score, lineup selection,
  ordering, the event lineup, reigning champions, battle construction, the
  win commit, saved state, and load validation.
- [Notable trainers specification](../specs/notable-trainers.md): TR, home
  regions, the traveller and aloof traits, willingness, scalers, and
  rosters.
- [Player Trainer Rating specification](../specs/player-trainer-rating.md):
  the player's TR, which league wins do not raise in v0.

## Later

- Off-screen results as signposting: NPC gossip, TV, and Match Call news about
  who won an event and who reigns.
- Tuning event rewards, the three-day cadence, and entry thresholds; special
  events.
- Balancing tools: tuning travel cost, fatigue, and the aloof margin against
  the lineup reports, and league balance targets.
- Seeded lineups or calendars
  ([Playthrough-seeded variation](playthrough-seeded-variation.md)) and
  rotation.
- Role windows (such as elite and headliner) and standing-based selection.
- Matchmaking lineups around the player's TR.
- NPC badge records.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Notable trainers](notable-trainers.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [League scaling](league-scaling.md)
- [Player progression](../specs/trainer-rating-party-progression.md)
