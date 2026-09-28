# Leagues

Implemented: No
Design status: v0 approved: once the player is known (TR 80), a league
phones with an invitation every seven in-game days; the league where they
hold the most badges calls, never the same one twice in a row, and Sevii
Masters joins in after any league win. Accept and the event waits for them,
with one attempt; decline and the season goes on without them. Someone always
holds each league's title. Each lineup is the five strongest trainers who are
willing to come (favouring those at home and fresh, with aloof trainers
joining only an elite enough field), with no randomness, fought in ascending
order. Balance is informational for now. Terms follow the
[glossary](player-trainer-rating.md#glossary).

## Intent

Leagues should feel like seasons in a living world rather than a fixed
ladder to climb once. Once the player is known, the leagues come to them: a
league phones to invite them to its next event. Accept, and the event waits
until they are ready; decline, and it goes ahead without them, and someone
else takes the title. Lineups are made of the strongest notable trainers in
the world rather than fixed room occupants, and who turns up depends on where
the league is and who played last. Each opponent fights as the same person the
player meets elsewhere, at their own Trainer Rating (TR) and with their own
team, so a league battle and a Gym battle with Clair are the same Clair.

On adoption, Leagues replaces today's fixed lineups and their levels scaled
from the player's TR when entering a league
([League scaling](league-scaling.md),
[interregional League circuit](wayfarer-interregional-league-circuit.md)),
which remain the record of Today.

## Design

**Leagues.** Indigo, Sevii Masters, and Hoenn keep their public entrances,
rooms, and ceremonies. Each league event is a five-match singles lineup.

**Getting known.** Leagues notice the player once their TR reaches 80, which
today means 8 badges from any regions; later sources of TR may get them known
sooner. Until then, no league calls and the league doors stay shut.

**The phone rings.** Seven in-game days after the player gets known, a league
phones with an invitation, and the next one comes seven days after the player
answers, or, if they accepted, seven days after that event is over. While an
invitation is waiting for an answer, or an accepted event is waiting for the
player, no other league calls. Days come from the game's existing day
counter and only schedule calls: nobody grows stronger or weaker with time,
only with the world's progress.

**Who calls.** The league where the player has earned the most badges calls:
Indigo counts Kanto and Johto badges, Hoenn counts Hoenn badges. The same
league never calls twice in a row, so the other one takes the next turn. When
the counts are level, the league that did not call last calls, and the very
first time, Indigo. Sevii Masters is the invitational: after the player's
first league win anywhere, it takes the turn whenever the leading league has
just called or the counts are level.

**Accept.** Accepting books the player in. The lineup is worked out then and
kept, and the event waits for them as long as they like: they can train, earn
badges, and come back. When they arrive, they get one attempt: a loss, or
leaving midway, ends the event.

**Decline.** Declining costs nothing. The event goes ahead without the player
straight away, and its strongest trainer takes the title.

**Pool.** Every notable singles trainer can be invited to any league. Tate &
Liza are notable but fight only as a double battle, so they are left out; Red
(separate mastery encounter) is not notable.

**Home and away.** Every notable trainer has a home region, and some have
the traveller trait ([Notable trainers](notable-trainers.md)). A
league is a location: Indigo belongs to Kanto and Johto, and Hoenn to Hoenn,
so trainers from there are at home. Sevii Masters is neutral ground, where
everyone is at home.

**Lineup.** The strongest trainers who are willing to come make the lineup.
When the player accepts or declines, every trainer gets a league score: their
current TR, scaled down by how unwilling they are to come. Trainers prefer to
play at home: most rarely make the trip to a league away from home, while a
traveller roams almost as readily as they play at home. Someone who played
in the most recent event, whether the player took part or declined it, is
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
always give the same lineup.

**Battle order.** Ascending TR: the weakest of the five fights first and the
strongest last.

**Strength.** Each opponent uses their own TR, team, and levels, exactly as in
any other battle with them. There is no league-specific adjustment.
[Notable trainers](notable-trainers.md) owns trainer TR and how it grows,
the scalers that turn TR into team level and size, and rosters. The player's
TR enters only through how far each trainer has grown, and through getting
known.

**Reigning champion.** Every event crowns someone. If the player wins, they
are the league's reigning champion. If they lose or decline, the strongest
trainer of that event's lineup takes the title. Either way the champion reigns
until the league's next event.

**Winning.** The player's first win at each league keeps today's records,
ceremonies, and unlocks, all once only, and opens the Sevii Masters' turns.
Winning again brings prize money and the title back. No win gives the player
TR: a league is a test, not a source of power
([Player Trainer Rating](player-trainer-rating.md)). Today, the ROM still adds
TR for a first league win until that design is adopted.

**Presentation.** Invitations and results come by phone call, on a
Pokégear/PokéNav-style phone as in HeartGold/SoulSilver and Emerald; the exact
phone screens are an implementation detail.

**Balance.** Informational for now: the balance explorer simulates a run of
invitations for a chosen TR and badge split and the player's answer to each,
showing who calls and why, each event's lineup, who is tired, and who reigns,
and for any event each trainer's league score, the base lineup level, and which
aloof trainers join or skip; tuning comes later. The lineup comes from the
strongest trainers of the moment, so it tends to sit a little above the
player; with all 24 badges both sides reach level 100, and tougher endgame
teams (better items, stats, movesets) are Later.

## Sample playthrough

1. A player starts in Hoenn and earns all eight Hoenn badges: TR 80, and the
   leagues know their name. A week of in-game days later the phone rings:
   Hoenn, where they have the most badges, invites them. They feel unready and
   decline. The event runs without them, and Norman, its strongest trainer,
   is Hoenn's reigning champion.
2. Seven days later Indigo calls, since Hoenn called last. They accept. The
   five are fixed on the spot, mostly Kanto and Johto trainers at home, with
   anyone who played Hoenn's event tired. The event waits while they sail
   north and train.
3. They arrive and fight, weakest first, each opponent with the team they
   would bring anywhere else. They win: the Hall of Fame, the Kanto/Johto
   Champion recognition, and Blue's Dojo battle follow, and they reign at
   Indigo.
4. A week after that win, Hoenn calls again. They accept, and later lose at
   match 5. They black out, the event is over, and that finalist reigns in
   Hoenn.
5. The next call is from Sevii Masters: after a league win it takes the turn
   Hoenn would repeat. Anyone from anywhere is at home there. An aloof
   Champion may still find the base lineup too far below them, and turn up
   only once the world's best are strong enough.
6. Hoenn and the Masters now take turns. When the player accepts Hoenn again
   and wins, it is their first Hoenn win, with the full completion credits;
   later wins bring prize money and the title, with no second Hall of Fame.

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
- Each lobby names the reigning champion.

## Boundaries

In: the three leagues, getting known, invitations by phone and which league
calls, accepting and declining, one attempt per accepted event, reigning
champions, first and repeat wins, notable Kanto, Johto, and Hoenn singles
trainers, home and away, the league score, the traveller and aloof traits and
the base lineup level, the lineup, and battle order.

Out of scope for v0: seeded or reputation-based league choice, news of who won
the events the player declined, special invitational events, and balance
targets for leagues. Today's circuit, with its fixed order and replays, stays
documented as Today until this design is adopted; its record, ceremony, and
unlock rules carry over to first wins. Player TR follows
[Player Trainer Rating](player-trainer-rating.md), where league wins add
nothing. Wild encounters, shops, regular trainers, and standalone builds
follow their own contracts; other battles with notable trainers follow their
own TR.

## Specifications

- [Leagues specification](../specs/leagues.md): registry, eligibility,
  invitations (qualification, the countdown, which league calls, accept and
  decline), fatigue, the league score, lineup selection, ordering, the event
  lineup, reigning champions, battle construction, the win commit, saved
  state, load validation, and presentation.
- [Notable trainers specification](../specs/notable-trainers.md): TR, home
  regions, the traveller and aloof traits, willingness, scalers, and
  rosters.
- [Player Trainer Rating specification](../specs/player-trainer-rating.md):
  the player's TR, which league wins do not raise in v0.

## Later

- Seeded or reputation-based league choice
  ([Playthrough-seeded variation](playthrough-seeded-variation.md)).
- Invitation news: who won the events the player declined, as NPC gossip, TV,
  or phone news.
- Special invitational events.
- Tuning event rewards, the seven-day interval, and the TR that gets the
  player known.
- Balancing tools: tuning travel cost, fatigue, and the aloof margin against
  the lineup reports, and league balance targets.
- Seeded lineups and rotation.
- Role windows (such as elite and headliner) and standing-based selection.
- Matchmaking lineups around the player's TR.
- NPC badge records.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Notable trainers](notable-trainers.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [League scaling](league-scaling.md)
- [Player progression](../specs/trainer-rating-party-progression.md)
