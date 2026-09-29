# Leagues

Implemented: No
Design status: v0 approved: once the player qualifies (TR 80), a league
phones with an invitation every seven in-game days; only leagues that know
them call (where they hold a badge; the Sevii Masters, an off-the-record club
for champions, once they are a Master, with wins at both Indigo and Hoenn),
taking turns: whichever called longest ago calls next. Accept and the event
waits for them, with one attempt; decline and the season goes on without
them. Someone always holds each league's title. Each lineup is the five
strongest trainers who are willing to come (favouring those at home and
fresh, with aloof trainers joining only a strong enough base lineup), with no
randomness, fought in ascending order. At the Masters the aloof rule is off
and notable trainers who are Masters have guaranteed seats. Each themed
Elite Four room is a named hall with one fixed field condition for both
sides; the Champion rooms are neutral. Balance is informational for now.
Terms follow the [glossary](player-trainer-rating.md#glossary).

## Intent

Leagues should feel like seasons in a living world rather than a fixed
ladder to climb once. Once the player qualifies, the leagues come to them: a
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

**The Sevii Masters.** The Masters isn't an official league. It lives in a
hidden basement under Seven Island's battle house: an off-the-record club
where champions find out who is actually best. An old caretaker, the retired
trainer who runs the house, keeps the door. A **Master** is anyone, the player
or a notable trainer, who has been champion at both Indigo and Hoenn at some
point, whether or not they reign there now. A Masters title never counts
toward it.

**Qualifying.** The player qualifies for invitations once their TR reaches
80, which in v0 means 8 badges from any regions; later sources of TR may
qualify them sooner. Until then, no league calls and the league doors stay
shut.

**The phone rings.** Seven in-game days after the player qualifies, a league
phones with an invitation, and the next one comes seven days after the player
answers, or, if they accepted, seven days after that event is over. While an
invitation is waiting for an answer, or an accepted event is waiting for the
player, no other league calls. Days come from the game's in-game day clock
and only schedule calls: nobody grows stronger or weaker with time, only with
the world's progress. Only days going forward count: if the day counter ever
goes backwards, the countdown neither moves on nor starts over.

**Depends on the in-game clock.** Today the game's day counter follows the
cartridge's real-time clock (RTC). The game is moving to an in-game clock with
no RTC, and Leagues depends on it: league days are in-game days, and Leagues
can't be adopted before that clock is. Leagues needs only a day count from it
that moves forward as you play; how long a day lasts is up to the in-game
clock's design, which hasn't been written yet.

**Who calls.** Only a league that knows the player calls: Indigo once they
hold a Kanto or Johto badge, Hoenn once they hold a Hoenn badge, and the Sevii
Masters once they are a Master: they have won both Indigo and Hoenn at least
once, in either order. Until then the Masters never calls. The leagues
that know them take turns: the one that called longest ago calls next, and one
that has never called goes first. When that is a tie, the league where they
have earned the most badges calls (Indigo counts Kanto and Johto badges,
Hoenn counts Hoenn badges), and on equal counts, Indigo. The Masters is never
in such a tie: it knows the player only after Indigo and Hoenn have both
called. So the first call comes from where they have the most badges, and a
player known in only one region hears from that league every time until
another league knows them. If the player qualifies while no league knows
them (possible only with a future source of TR), no call comes: the call
stays due and is checked again each day until a league knows them.

**Accept.** The player must accept or decline each call; there is no
"later". Accepting books the player in. The lineup is worked out then and
kept, and the event waits for them as long as they like: they can train, earn
badges, and come back. When they arrive, they get one attempt: a loss, or
leaving midway, which counts as a loss, ends the event, and the strongest
trainer of the kept lineup takes the title. If the game's content changes
under a save, an accepted event goes back to an unanswered invitation from the
same league.

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
traveller makes the trip almost as readily as they play at home. Someone who played
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
There is no dice roll, so the same world progress, the same last event, and
the same Masters always give the same lineup.

**The Masters lineup.** The Masters is elite by definition, the company aloof
trainers seek, so the aloof rule doesn't apply there: there is no base lineup,
and everyone may come. Notable trainers who are Masters get guaranteed seats;
if more than five are Masters, the five with the highest league scores sit.
The remaining seats go to the highest league scores, as anywhere. Fatigue
still lowers everyone's league score, a Master's included, and everyone is at
home there. Being a Master gives no seat at Indigo or Hoenn.

**Battle order.** Ascending TR: the weakest of the five fights first and the
strongest last.

**Halls.** Each match is fought in a **hall**, one of the league's rooms. At
Indigo and Hoenn each Elite Four room is a named hall that honours one of
that league's own Elite Four, such as Lorelei's Hall. The Sevii Masters'
basement reuses the rooms of Johto's old Elite Four, and its themed rooms
honour them in the same way. A room keeps its look, plus the hall's weather
where it has one: snow falls in Lorelei's Hall. Its name honours the Elite
Four member without implying who fights there: in any event, whoever the
lineup puts in that match is the occupant. Each hall has one fixed **hall
condition** that applies to both sides from the start of the battle. It's
the hall's identity, not a balancing lever: it matches what the room looks
like, never who used to fight there, and every hall's condition is
different. Match 1 is fought in the first
hall, match 2 in the second, and so on, so the ascending battle order decides
who lands in which hall.

| League | Hall 1 | Hall 2 | Hall 3 | Hall 4 | Hall 5 |
| --- | --- | --- | --- | --- | --- |
| Indigo | Lorelei's Hall: snow | Bruno's Hall: sandstorm | Agatha's Hall: Trick Room (5 turns) | Lance's Hall: Tailwind for both sides (temporary) | Champion's Room: neutral |
| Hoenn | Sidney's Hall: Magic Room (5 turns) | Phoebe's Hall: Grassy Terrain (5 turns) | Glacia's Hall: Misty Terrain (5 turns) | Drake's Hall: Sea of Fire on both sides (4 turns) | Champion's Room: neutral |
| Sevii Masters | Will's Hall: Psychic Terrain (5 turns) | Koga's Hall: Toxic Spikes on both sides | Bruno's Hall: Stealth Rock on both sides | Karen's Hall: Wonder Room (5 turns) | Champion's Room: neutral |

Weather lasts until something replaces it, as overworld weather does when a
battle starts outdoors. Rooms, terrains, Tailwind, and Sea of Fire wear off
after a few turns. Toxic Spikes and Stealth Rock stay down until a
Pokémon clears them, and they greet both leads as the battle opens. The
Champion's Rooms are neutral everywhere: the last match is
just the best. Because each condition is known in advance and fixed, the
player can plan a team for each hall, and matchups emerge on their own: a
trainer who happens to land in the hall that suits their team is more
dangerous there than anywhere else.

**Strength.** Each opponent uses their own TR, team, and levels, exactly as in
any other battle with them. There is no league-specific adjustment.
[Notable trainers](notable-trainers.md) owns trainer TR and how it grows,
the scalers that turn TR into team level and team size (which grows in
steps), and rosters. The player's TR enters only through how far each
trainer has grown, and through qualifying.

**Reigning champion.** Every event crowns someone. If the player wins, they
are the league's reigning champion. If they lose or decline, the strongest
trainer of that event's lineup takes the title. Either way the champion reigns
until the league's next event. The game remembers, for each notable trainer,
whether they have ever reigned at Indigo and whether they have ever reigned at
Hoenn; once both are true, they are a Master for good.

**Winning.** The player's first win at each league keeps today's records,
ceremonies, and unlocks, all once only. Their first wins at both Indigo and
Hoenn make them a Master, which opens the Sevii Masters' turns. Winning the
Masters brings a Gallery win, prize money, and the title, and no new status.
Winning again brings prize money and the title back, plus another Gallery win
at the Masters. No win gives the player
TR: a league is a test, not a source of power
([Player Trainer Rating](player-trainer-rating.md)). Today, the ROM still adds
TR for a first league win until that design is adopted.

**Presentation.** Invitations and results come by phone call, on a
Pokégear/PokéNav-style phone as in HeartGold/SoulSilver and Emerald; the exact
phone screens are an implementation detail. The Masters' call comes from the
caretaker, not from an official league, and her first call says that Lorelei,
retired to Four Island, spoke of the player. That is story only: Lorelei stays
an ordinary notable trainer with no guaranteed seat. The **Masters Gallery**
on the basement wall tallies each Masters winner's wins, including trainers
who won the events the player declined. The lobby, or a sign at each hall's
door, names the hall and its condition, so the player knows what's ahead
before the match starts, and a snow or sandstorm hall shows its weather in
the room.

**Balance.** Informational for now: the balance explorer simulates a run of
invitations for a chosen TR and badge split and the player's answer to each,
showing who calls and why, each event's lineup, who is tired, and who reigns,
and for any event each trainer's league score, the base lineup level, and which
aloof trainers join or skip, and, at the Masters, who has reigned where, who
is a Master, and which seats are guaranteed; tuning comes later. The explorer
doesn't show each match's hall yet (Later). Hall conditions aren't tuned
against lineups: they're identity, and they apply to both sides. The lineup
comes from the strongest trainers of the moment, so it tends to sit a little
above the player; with all 24 badges both sides reach level 100, and tougher
endgame teams (better items, stats, movesets) are Later. With the aloof rule
off, the Champions come to the Masters at any world progress: Lance (TR 200,
level 100) attends from the Masters' first call.

## Sample playthrough

The player holds all eight Kanto badges and all eight Hoenn badges, TR 120.
To keep the example simple, their TR stays at 120 throughout, and they fight
each accepted event on the day of its call, as in the balance explorer.

1. Day 7: Indigo calls first (it ties with Hoenn on badges, so Indigo). The
   base lineup tops out at level 82, so the aloof Champions (levels 99-100)
   stay away. The player accepts and beats Koga, Karen, Blue, Giovanni, and
   Jasmine: their first Indigo win, with its Hall of Fame.
2. Day 14: Hoenn calls. The player accepts and beats Sidney, Winona, Juan,
   Phoebe, and Norman. With Indigo and Hoenn both won, the player is a Master.
3. Day 21: the phone rings, and it is the caretaker of Seven Island's battle
   house: Lorelei spoke of the player, and there is a room downstairs. The aloof
   rule is off at the Masters, so the Champions come; the lineup is Giovanni,
   Jasmine, Steven, Wallace, and Lance. The player declines. Lance takes the
   Masters title, and the Masters Gallery records his win. A Masters title
   doesn't make him a Master.
4. Days 28 to 49: the player declines everything. Blue takes Indigo's title,
   then Giovanni, a traveller, takes Hoenn's away from home; the Masters
   crowns Lance again; then Giovanni takes Indigo's title too. He has now
   reigned at both, so he is a Master for good.
5. Day 56: Norman takes Hoenn's title. Day 63: at the Masters, Giovanni's
   seat is guaranteed, though his league score would have put him in the top
   five anyway. Lance wins the declined event and adds a Gallery win.
6. Days 70 and 77: Blue reigns at Indigo again, Giovanni at Hoenn again.
   Day 84: the Masters calls. Giovanni fought at Hoenn a week earlier, so he
   is tired: league score 65, far below Blue's 129. His guaranteed seat still
   holds, and Blue sits out. The lineup is Giovanni, Jasmine, Steven, Wallace,
   and Lance. The player accepts and wins: a Gallery win for the player, with
   prize money and the Masters title, and no Hall of Fame, Ribbon, or regional
   title.

## Records and recognition

The player's first win at each league keeps Today's records, as the
[interregional League circuit](wayfarer-interregional-league-circuit.md)
implements them; repeat wins give prize money and the title only (plus the
Gallery win at the Masters).

- Indigo's first win grants the shared Kanto/Johto Champion recognition with
  one Hall of Fame registration and Champion Ribbon flow.
- The Masters Gallery tallies each Masters winner's wins, the player's or a
  notable trainer's. Every Masters win gives a Gallery win, prize money, and
  the title, and never a regional Champion title, Hall of Fame registration,
  Ribbon, or new status.
- First wins at both Indigo and Hoenn make the player a Master, which opens
  the Masters' calls.
- Hoenn keeps its own Champion recognition, Hall of Fame, and regional
  cleanup, and its first league win runs the full completion credits.
- Red unlocks after all three leagues have been won. Blue's Saffron Dojo battle
  unlocks after the first Indigo win.
- Whoever fights last is presented as the final opponent without inventing
  Champion history. Dialogue never assumes a fixed person in any room.
- Each lobby names the reigning champion.

## Boundaries

In: the three leagues, qualifying, which leagues know the player,
invitations by phone and which league calls, accepting and declining, one attempt per accepted event, reigning
champions, reign records and Masters, first and repeat wins, notable Kanto,
Johto, and Hoenn singles trainers, home and away, the league score, the
traveller and aloof traits and the base lineup level, the lineup, guaranteed
seats at the Masters, battle order, the halls and their conditions, and the
Masters' caretaker and Gallery.

Out of scope for v0: seeded or reputation-based league choice, news of who won
the events the player declined (beyond the Masters Gallery), special
invitational events, doubles at the Masters, any status or reward for winning
the Masters beyond its Gallery win, prize money, and title, balance
targets for leagues, and field conditions outside the leagues' halls (such
as Gym arenas). Red's unlock after all three leagues and Blue's Dojo stay
as they are. Today's circuit, with its fixed order and replays, stays
documented as Today until this design is adopted; its record, ceremony, and
unlock rules carry over to first wins. Player TR follows
[Player Trainer Rating](player-trainer-rating.md), where league wins add
nothing. Wild encounters, shops, regular trainers, and standalone builds
follow their own contracts, except that the snow and sandstorm halls' shared
room maps carry their weather into standalone FRLG too; other
battles with notable trainers follow their own TR.

## Specifications

- [Leagues specification](../specs/leagues.md): registry, eligibility,
  invitations (qualification, the countdown, which league calls, accept and
  decline), fatigue, the league score, lineup selection (with the Masters'
  guaranteed seats), ordering, the halls, the event lineup, reigning champions
  and reign records, battle construction (with the hall condition), the win
  commit, saved state, load validation, and presentation.
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
- Tuning event rewards, the seven-day interval, and the TR that qualifies
  the player.
- Balancing tools: tuning travel cost, fatigue, and the aloof margin against
  the lineup reports, and league balance targets.
- More of the Masters' story: the caretaker's later calls and lines, and what
  the Gallery shows beyond each winner's win count.
- Seeded lineups and rotation.
- Role windows (such as elite and headliner) and standing-based selection.
- Matchmaking lineups around the player's TR.
- NPC badge records.
- Gym arenas with their own field conditions, like the halls
  ([Notable trainers](notable-trainers.md#later)).
- Each match's hall and its condition in the balance explorer.

## References

- [Player Trainer Rating](player-trainer-rating.md)
- [Notable trainers](notable-trainers.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [League scaling](league-scaling.md)
- [Player progression](../specs/trainer-rating-party-progression.md)
