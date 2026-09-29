# Leagues

PRD: [Leagues](../prds/leagues.md)
Implemented: No
Design status: v0 approved: leagues as locations whose **league events** reach
the player as **invitations** by phone. From player TR 80 a league calls every 7
in-game days (restarted when each invitation resolves, stopped while an accepted
event waits); only a league that knows the player calls (Indigo with a Kanto or
Johto badge, Hoenn with a Hoenn badge, the Sevii Masters once the player is a
**Master**, with lifetime wins at both Indigo and Hoenn), round-robin by a call
counter: the eligible league that called least recently, ties to the most
badges, then Indigo. Accepting freezes a lineup that waits for the player, with
one attempt; declining runs the event without them. A league score per eligible
trainer (Trainer Rating (TR) scaled by willingness, from travel cost and
fatigue), aloof trainers joining only a base lineup near their level (never at
the Masters), guaranteed Masters seats for notable trainers who have reigned at
both Indigo and Hoenn, the top five by league score with no randomness,
ascending battle order, match N in **hall** N (each themed Elite Four room a
named hall with one fixed **hall condition** for both sides; Champion rooms
neutral). The Masters is a tag-battle league: eight opponents (never the
**partner**, the contact the player last asked by phone, Lorelei by default)
paired in ascending order, four **tag matches** (the player and partner against
a pair, three Pokémon each, the player picking three at each door with damage
carrying over), then a full-team singles final against the partner after a full
heal. A **reigning champion** per league, reign records, and first-win one-time
effects. Balance is informational for now. Today's [interregional
circuit](wayfarer-interregional-league-circuit.md) stays the record of Today's
admission, fixed league order, and replays.

## Scope

Own, for each `IS_WAYFARER` league: the league registry, eligibility,
location regions, invitations (the qualification gate, the countdown, which
league calls, accepting and declining), fatigue, the league score, the base
lineup and its level, the aloof rule, reign records and Masters, lineup
selection (with the Masters' guaranteed seats) and battle order, the
Masters' partner and pairs, the halls and their conditions, the event
lineup, the reigning champion, the Masters Gallery, battle construction
(with the hall condition hook, the Masters' tag matches, and its final),
entering an accepted event, active runs and dispatch (with the Masters'
party selection, restore, and heal), the win commit, first and repeat wins,
saved state, load validation, presentation, and regional integration.

- [Notable trainers](notable-trainers.md) owns notable trainers, their
  TR and its growth with world progress, home regions, the traits (traveller
  and aloof), travel cost and willingness, the team-level scaler and the
  team-size step scaler, rosters, and team composition. This spec reads a
  trainer's TR, team level, willingness, aloof trait, composed team, and
  [phone contact](notable-trainers.md#phone-contacts) bit; it never
  restates how they are computed.
- [Player Trainer Rating](player-trainer-rating.md) owns the player's TR,
  which this spec reads only for the qualification gate and as world
  progress. In v0 a league win adds no player TR; Today's +8 per first league
  win stays documented in the circuit spec until adoption.
- Today's [interregional circuit spec](wayfarer-interregional-league-circuit.md)
  stays the record of Today's admission, fixed league order, and replays, and
  keeps owning the first-league-win facts, ceremonies,
  and unlocks this spec reuses. Upon adoption, this spec replaces its
  admission, order, replay, and loss rules with invitations and one attempt
  per accepted event.

Upon adoption this replaces the fixed lineups and the level policy based on the
player's TR when entering a league in [League scaling](league-scaling.md),
which stays the record of Today.

## League identity

League identity is stable and distinct from region: Indigo = 1, Masters = 2,
Hoenn = 3. The leagues keep their public entrances and rooms: FRLG Indigo, the
Seven Island Masters House leading into HNS rooms, and Emerald Hoenn.

Masters resolves to Sevii/Kanto for other regional systems. Its reused HNS
rooms need a Wayfarer-only map-context override; standalone HNS keeps its
identity. The final HNS ceremony room is the Masters Gallery.

The Masters is presented as an off-the-record club, not an official league:
a hidden basement under Seven Island's battle house (the Masters House
entrance), where champions find out who is actually best. An old caretaker,
the retired trainer who runs the house, keeps the door and makes its calls.
A **Master** is anyone, the player or a notable trainer, who has been
champion at both Indigo and Hoenn at some point; a Masters title never counts
([Reign records](#reign-records)). Geography, room
names, and titles never substitute for saved league or selected-character
identity.

Each league is a [location](notable-trainers.md#home-region-and-travel) with
a location region, which decides who is at home there:

| League | Location region |
| --- | --- |
| Indigo | Kanto, Johto |
| Masters | Neutral location: home to everyone |
| Hoenn | Hoenn |

### Halls

Each league has an ordered list of five **halls**, one per match, authored in
the league registry (a location property, never a trainer's). A hall has:

| Field | Contract |
| --- | --- |
| `room` | The map that hosts the match: the league's room chain at the same position. |
| `name` | The hall's displayed name, or none for a room that honours no one. |
| `honours` | The Elite Four member the hall is named for (the league's own; at the Masters, Johto's old Elite Four whose rooms it reuses), or none. |
| `condition` | One hall condition, or neutral. |
| `weather` | The room map's own weather (`map.json`): the condition's weather for a snow or sandstorm hall, otherwise none. |

A **hall condition** starts the battle already in effect for both sides. It
is chosen from the room's look (its floor, pillars, and decor), never from
who fought there before, and no two halls share a condition. Karen's Hall
is the one exception to the look rule: its room shows no theme, and its
Wonder Room comes from Karen's own "strong or weak is only perception".

What each room shows, and the condition it gives:

| Hall | Look | Condition |
| --- | --- | --- |
| Lorelei's | blue floor, ice-block pillars | snow |
| Bruno's (Indigo) | sand floor, stacked stone pillars | sandstorm |
| Agatha's | violet floor, glowing light pillars | Trick Room |
| Lance's | teal floor, dragon horn and dragon statues | Tailwind |
| Sidney's | wooden floor, cabinets stocked with items | Magic Room (the items stay shelved) |
| Phoebe's | crumbling, overgrown ruins | Sticky Web (cobwebbed ruins) |
| Glacia's | pale frosty floor, ice-crystal cases | Misty Terrain (freezing mist) |
| Drake's | glowing orange floor, jagged rock rows | Sea of Fire |
| Will's | violet floor, glowing light pillars | Psychic Terrain |
| Koga's | grass floor, potted plants | Grassy Terrain |
| Bruno's (Masters) | sand floor, stacked stone pillars | Stealth Rock |
| Karen's | teal floor, plain stone pillars | Wonder Room (the exception) |

Some Masters rooms share art with Indigo's (Will's with Agatha's, the
Masters' Bruno's with Indigo's); each takes its condition from a different
part of the same look.

The conditions:

| Condition | Battle start | Lasts |
| --- | --- | --- |
| Snow | Snow weather, also falling in the room | Until replaced, like overworld weather |
| Sandstorm | Sandstorm weather, also blowing in the room | Until replaced, like overworld weather |
| Trick Room | Trick Room | 5 turns |
| Magic Room | Magic Room | 5 turns |
| Wonder Room | Wonder Room | 5 turns |
| Psychic Terrain | Psychic Terrain | 5 turns |
| Grassy Terrain | Grassy Terrain | 5 turns |
| Misty Terrain | Misty Terrain | 5 turns |
| Tailwind (both sides) | Tailwind on the player's side and on the opponent's side | The engine's temporary Tailwind (4 turns) on each side |
| Sea of Fire (both sides) | Sea of Fire on the player's side and on the opponent's side | The engine's temporary Sea of Fire (4 turns) on each side |
| Sticky Web (both sides) | Sticky Web on each side | Until cleared, like any hazard |
| Stealth Rock (both sides) | Stealth Rock on each side | Until cleared, like any hazard |

A side condition covers every battler on its side: in a [tag
match](#tag-matches) the player and partner share one side, and the pair the
other. Starting hazards also hit every lead, two in singles and four in a tag
match: the first-turn switch-in events run the hazard block for every battler,
and the engine then handles any lead that fainted from them
(`FIRST_TURN_FAINTED_BATTLERS` in
[battle_main.c](../../game/src/battle_main.c)). Sticky Web lowers the Speed of
each grounded lead and of every grounded Pokémon that switches in, as usual.

The v0 halls, by match:

| Match | Indigo | Hoenn | Masters |
| --- | --- | --- | --- |
| 1 | Lorelei's Hall (`PokemonLeague_LoreleisRoom_Frlg`): snow | Sidney's Hall (`EverGrandeCity_SidneysRoom`): Magic Room | Will's Hall (`PokemonLeague_WillsRoom_hns`): Psychic Terrain |
| 2 | Bruno's Hall (`PokemonLeague_BrunosRoom_Frlg`): sandstorm | Phoebe's Hall (`EverGrandeCity_PhoebesRoom`): Sticky Web (both sides) | Koga's Hall (`PokemonLeague_KogasRoom_hns`): Grassy Terrain |
| 3 | Agatha's Hall (`PokemonLeague_AgathasRoom_Frlg`): Trick Room | Glacia's Hall (`EverGrandeCity_GlaciasRoom`): Misty Terrain | Bruno's Hall (`PokemonLeague_BrunosRoom_hns`): Stealth Rock (both sides) |
| 4 | Lance's Hall (`PokemonLeague_LancesRoom_Frlg`): Tailwind (both sides) | Drake's Hall (`EverGrandeCity_DrakesRoom`): Sea of Fire (both sides) | Karen's Hall (`PokemonLeague_KarensRoom_hns`): Wonder Room |
| 5 | Champion's Room (`PokemonLeague_ChampionsRoom_Frlg`): neutral | Champion's Room (`EverGrandeCity_ChampionsRoom`): neutral | `PokemonLeague_ChampionsRoom_hns`: neutral |

Each chain is Today's room chain in
[league_circuit.c](../../game/src/league_circuit.c) (`sIndigoRooms`,
`sHoennRooms`, `sMastersRooms`), where the room at position N already hosts
match N. Indigo's and Hoenn's four Elite Four rooms map one to one onto
matches 1-4, and their Champion's Room hosts match 5, honouring no one. The
Masters reuses the HNS rooms of Johto's old Elite Four, and a room becomes a
named hall only where its art is themed to that member's type: Will's room
(violet floor, glowing pillars) reads as Psychic, Koga's (grass floor,
potted plants) as Grass, and Bruno's (sand-coloured floor, stacked boulder
pillars) as Rock and Fighting. Karen's room (teal floor, plain stone
pillars) shows no Dark theme but is still Karen's Hall, the look rule's one
exception, and the HNS Champion's Room is neutral. The Emerald corridors
`EverGrandeCity_Hall1` to
`EverGrandeCity_Hall5` are passages between rooms, not halls: no match is
fought there. A room keeps its look (map, tiles, music), plus the hall's
weather where it has one, and a hall's name never implies who fights there.

## Registry and eligibility

Author a versioned, machine-readable registry:

| Field | Contract |
| --- | --- |
| `characterId` | Stable `u32` identity for one person, shared with the notable trainer catalog and independent of Trainer IDs, title, party, or region. |
| `displayName` | Existing localized name or an authored league name. |
| `presentationId` | Audited graphics, introduction, defeat, and after-battle text. |
| `enabled` | Build-time inclusion, with an authored reason when disabled. |

Maintain an alias inventory linking existing Trainer IDs and source roster
owners to canonical characters. Gym, rival, Champion, rematch, and regional
encounters of one person share one `characterId` and one roster, so aliases
cannot appear in a lineup twice. People with similar names remain distinct.

A trainer is **eligible** when they are a notable trainer with valid growth
values and a valid roster, battle alone (not as a duo), are enabled, and have
validated presentation, including a back pic for battling as a
[partner](#partner). Tate & Liza are a notable duo who fight only together, so
they are league-ineligible: never an opponent and never a partner, in singles or
in a tag match. Red is not a notable trainer in v0, so he is league-ineligible
and keeps his separate mastery encounter. Region, title, and story availability
neither add nor remove a trainer. League eligibility does not change story
battles, which follow the [every-battle
rule](notable-trainers.md#trainer-rating).

## Invitations

The player takes part in league events only by invitation. An **invitation**
is a league's phone call inviting the player to its next event; the player
**accepts** or **declines** it. At most one invitation or accepted event
exists at a time.

Days come from the game's in-game day clock; leagues add no clock of their own.
**Dependency:** Today the day counter (`VAR_DAYS`, kept by the daily update in
[clock.c](../../game/src/clock.c)) follows the real-time clock (RTC). The game
is moving to an in-game clock with no RTC, and this design depends on it:
adopting Leagues requires that clock, and every day below is an in-game day from
it. All Leagues needs from that clock is a **day count** that advances with
play, which Leagues reads only through the forward-counting
[countdown](#countdown); the length of a day is owned by the in-game clock
design, which is not yet written. **Days never affect anyone's strength**: every TR stays a function of world
progress, and days only schedule invitations. The order of calls never reads
days: it comes from a monotonic [call counter](#which-league-calls), so a day
counter that goes backwards cannot reorder the leagues.

### Qualification

The player **qualifies** once their TR is at least **80** (a placeholder;
in v0 exactly 8 badges, and future TR sources may lower the badges needed).
Player TR never decreases, so qualifying happens once. Below 80 no league
calls, and every league's rooms stay closed to the player.

### Countdown

An invitation arrives **7 in-game days** (a placeholder) after the countdown
starts. The countdown starts when the player first qualifies and restarts
when each invitation resolves: when the player declines, or when their
accepted event ends in a win, a loss, or leaving. No countdown runs while an
invitation is waiting to be answered or an accepted event is pending, however
long the player takes.

The countdown counts only **forward day advances**. It saves the **days
remaining** (7 when it starts or restarts) and the **last counted day** (the
day counter's value when the countdown last looked). On each check (whenever
the day count advances, and after a load):

```text
today = day counter
if today > lastCountedDay: daysRemaining = max(0, daysRemaining - (today - lastCountedDay))
lastCountedDay = today
```

The call is due once the days remaining reach 0. A day counter that goes
backwards (a clock turned back) neither advances nor resets the countdown: it
only moves the last counted day down, and counting resumes from there. A
counter far ahead brings one call, never a backlog: missed days leave no
trace.

### Which league calls

The league that calls is chosen when the call arrives, from the player's
badges and lifetime wins then. A league calls only where the player is known
(**known-there eligibility**):

- **Indigo** is eligible once the player holds at least one Kanto or Johto
  badge;
- **Hoenn** is eligible once the player holds at least one Hoenn badge; and
- the **Sevii Masters** is eligible once the player is a Master: lifetime wins
  at both Indigo and Hoenn (`indigoCleared` and `hoennCleared`), in either
  order. A lifetime win at only one of them, or a Masters win, never makes it
  eligible.

The TR 80 gate still applies to every call. A league's **badges** are the
player's badges from its regions: Indigo counts Kanto and Johto badges, Hoenn
counts Hoenn badges, and the Masters has no badge count.

The saved **call counter** is monotonic: it starts at 0, and each call takes
the next number (counter + 1), which becomes the calling league's **last call
number**. Among the eligible leagues, the last call numbers decide,
**round-robin**:

1. The eligible league whose last call has the **lowest number** calls. A
   league that has never called has no number and counts as lowest.
2. A tie (only possible between leagues that have never called) goes to the
   league with the **most badges**. The Masters is never in a tie: it
   becomes eligible only after lifetime wins at Indigo and Hoenn, so both
   have called by then.
3. A remaining tie (Indigo and Hoenn with equal badges) goes to **Indigo**.

So the very first call comes from the eligible league with the most badges,
and each eligible league then calls in turn. When only one league is
eligible, it calls every time, repeating as often as needed; a league that
becomes eligible later (a first badge in its regions, or the Masters after
the second of the two regional first wins) has never called, so it calls
next. Badges from any region count
toward the gate; which regions they come from decides who may call and breaks
ties.

**No eligible league.** If the player qualifies while no league knows them
(possible only with a future TR source that needs no badge), no call is made:
the countdown stays due, and each day advance checks again, until a league
knows them and calls.

### Accept

Accepting locks the event for the player. At that moment the lineup is
selected ([Selection and order](#selection-and-order)) and frozen as the
[event lineup](#event-lineup), and the event **waits forever** for the player
to arrive at that league. The player has **one attempt**: a loss, or leaving
midway, ends the event. The player cannot enter a league that has no
accepted event of theirs, and the countdown stays stopped until the event
ends.

### Decline

Declining runs the event without the player at once. Its lineup is selected
at that moment with the same rule, its strongest member (the last in battle
order) becomes that league's reigning champion, and the lineup becomes the
most recent resolved lineup. The invitation resolves and the countdown
restarts.

## Selection and order

Every eligible trainer can be invited to any league; location regions only
scale how willing they are to come, and an aloof trainer skips a league whose
base lineup is well below their level. The **lineup size** is five at
Indigo and Hoenn and eight at the Masters. The Masters also differs in three
more ways: the aloof rule is off there, notable trainers who are Masters
have guaranteed seats, and the player's partner is never in its lineup.
When the player accepts or declines an invitation:

1. **Partner** (the Masters only). Resolve the [partner](#partner) and
   remove them from the eligible trainers for this event.
2. **TR.** Compute each eligible trainer's TR at the current world progress
   ([Notable trainers](notable-trainers.md#growth-with-world-progress)).
3. **Willingness.** Score each eligible trainer's willingness for this league
   with the [travel rule](notable-trainers.md#home-region-and-travel), which
   reads the league's location region and this fatigue. **Fatigue** is
   league-specific: 50 if the trainer is in the most recent resolved lineup,
   else 0.
4. **League score.** `floor(TR × willingness / 100)`, in integer arithmetic.
5. **Base lineup** (Indigo and Hoenn only). Rank the eligible trainers who
   are not [aloof](notable-trainers.md#aloof) by league score, ties by
   ascending `characterId`, and take the top five (all of them if fewer than
   five): the **base lineup**. The **base lineup level** is the strongest
   [team level](notable-trainers.md#trainer-scalers) in it.
6. **Aloof** (Indigo and Hoenn only). An aloof trainer is eligible for this
   league only if their team level is at most base lineup level + 10 (a
   placeholder margin, in levels, not TR); otherwise they skip it. Aloof
   trainers are judged against the base lineup only, never against each
   other, so one aloof trainer joining never lets another in. With no base
   lineup (no eligible trainer who is not aloof), there is no base lineup
   level and every aloof trainer skips. Fatigue still applies: it shapes the
   base lineup through the league scores, and an aloof trainer who joins
   keeps their fatigued league score. At the Masters there is no base lineup
   and no aloof check: every eligible trainer stays eligible.
7. **Master seats** (the Masters only). Rank the eligible notable trainers
   who are [Masters](#reign-records) by league score, ties by ascending
   `characterId`, and seat the top eight (all of them if fewer than eight).
   Fatigue lowers a Master's league score but never removes the guarantee;
   a Master who is the partner was removed in step 1 and has no seat.
8. **Lineup.** Rank every trainer still eligible and not already seated (at
   Indigo and Hoenn: the non-aloof and the aloof who join) by league score,
   and fill the remaining seats, the lineup size minus the Master seats,
   with the highest. Equal league scores break by ascending `characterId`.
9. **Battle order.** Order the lineup by ascending TR, so the strongest
   fights last. Equal TRs break by ascending `characterId`.
10. **Pairs** (the Masters only). Pair the eight in battle order: the first
    and second are pair 1, the third and fourth pair 2, and so on, so the
    strongest two are pair 4. In each pair the first is **opponent A** and
    the second **opponent B**.
11. **Halls.** Match N is fought in the league's [hall](#halls) N, so the
    battle order decides who lands in which hall. At the Masters, match N is
    pair N's [tag match](#tag-matches) for N from 1 to 4, and match 5 is the
    [final](#the-final) against the partner in the Champion's Room. Hall
    conditions play no part in selection, order, or pairing.

The **most recent resolved lineup** is the lineup of the most recent resolved
event at any league: an accepted event that ended (won, lost, or left), or a
declined one. For an accepted Masters event it also holds the partner, who
played in it; a declined Masters event holds only its eight. Each resolution
saves its lineup as the new most recent resolved lineup, after its own
selection has read the old one. Accepting reads it but never changes it,
because the event it reads is still the last one resolved.

Selection consumes no randomness and reads no seed: the same world progress,
most recent resolved lineup, reign records, partner, and content always give the
same lineup. The day, title, reigning champion, party, badges, and contacts play
no part; history enters only through the most recent resolved lineup and, at the
Masters, the reign records and the partner. Player TR enters only as world
progress, through each trainer's TR. When the player answers, the inputs become
authoritative at that moment, and the result is committed before reveal, so
nothing after it reselects.

## Partner

At the Masters the player fights beside a **partner**: the notable trainer
they last asked. The saved **partner choice** is none or one `characterId`.

- **Asking.** From the phone's contact list, the player can ask any
  [contact](notable-trainers.md#phone-contacts) to be their partner once
  they are a Master (the Masters knows them), whenever they can make a call
  (in the overworld, with no script, battle, or ceremony running). The ask
  always succeeds: it saves that contact as the partner choice, replacing
  any earlier one, and does nothing else. Only a league-eligible contact
  can be asked (Tate & Liza give no number), and no trait, TR, reign, or
  friendship check applies.
- **Default.** With no partner choice (the player has never asked anyone),
  the partner is **Lorelei**, who spoke of the player to the caretaker. She
  needs no contact bit.
- **Resolution.** Selection step 1 resolves the partner when the player
  accepts or declines a Masters invitation: the partner choice, or Lorelei
  without one. An accepted event records that partner in its
  [event lineup](#event-lineup), so asking someone else while it waits
  changes only later Masters events. A declined event reads the partner only
  to keep them out of its lineup; they do not play in it.

The partner is never in the lineup, is a notable trainer like any other
(their own TR, team, and AI), and plays no part in Indigo or Hoenn events.

## Event lineup

The lineup selected when the player accepts is the event's lineup, frozen for
that event however long it waits. Accepting captures, for each member in battle
order: `characterId`, their TR, and their composed team. At the Masters it holds
the eight in battle order, whose positions give the
[pairs](#selection-and-order), and also captures the partner the same way:
`characterId`, TR, and composed team. The lineup stores no content versions of
its own: the league state's [content versions](#saved-state) cover it, and they
match the build whenever an event is accepted. Per member, the team holds the
roster slot index and every resolved battle value the battle snapshot uses:
species/form, level, moves, item, ability, nature, IVs/EVs, and battle order. It
is saved atomically with the accepted event before reveal, and every battle,
including after a reload, is reconstructed from it; it never reselects or
recomposes, even after the player's TR rises. The composed team is each
trainer's whole resolved team; a tag match takes its three from it at
construction ([Tag matches](#tag-matches)), so nothing else is stored. The
event's end releases it, keeping only the `characterId`s (the five, or at the
Masters the eight and the partner) as the most recent resolved lineup.

A declined event's lineup is only computed for its result: it saves the
five or eight `characterId`s and composes no teams.

## Reigning champion

Each league has at most one **reigning champion**, who holds the title from
the resolution of one of its events until the resolution of its next:

- If the player wins the event, the player is the reigning champion, from the
  win.
- If the player loses or leaves an accepted event, the reigning champion is
  its frozen lineup's strongest member (the last in battle order). At the
  Masters this covers a loss in any hall (matches 1-4) and leaving before
  the final.
- If the player loses the Masters final, the reigning champion is the
  partner, who beat them.
- If the player declines, the reigning champion is the strongest member of the
  lineup computed at decline.

A new game has no reigning champion; each league gains one when its first
event resolves. Holding a title changes nothing by itself: it does not change
selection, battles, or rewards. What it leaves behind, the reign records,
decides who is a Master.

### Reign records

Each notable trainer has two lifetime **reign flags**: **has reigned at
Indigo** and **has reigned at Hoenn**. A flag is set, in the same transaction,
whenever that trainer becomes the league's reigning champion (after a loss,
leaving, or a decline), and it is never cleared by play. A Masters title sets
neither flag. A notable trainer with both flags is a **Master**, for good,
whether or not they reign anywhere now.

The player is a Master once they have lifetime wins at both Indigo and Hoenn;
the saved first-league-win facts already record that, so the player has no
reign flags. Being a Master changes only the Masters: its calls for the
player, and its guaranteed seats for notable trainers. Winning the Masters
adds a Gallery win and gives no new status.

### Masters Gallery

The Masters Gallery, the final ceremony room on the basement wall, tallies each
Masters winner's wins, adding one for the winner of every resolved Masters
event: the player after a win, otherwise the reigning champion that event
crowned, so trainers who won events the player declined appear too. It keeps a
saved win count per winner (one per notable trainer and one for the player),
shown on the wall; a count stops at its maximum rather than wrapping.

## Saved state

Keep Today's saved first-league-win facts (`indigoCleared`, `mastersCleared`,
`hoennCleared`, which serve as the lifetime wins) and pending ceremony phase.
Add:

- the **invitation state**, exactly one of: not qualified; counting down, with
  the **days remaining** and the **last counted day**; **invited** by a
  league, waiting for the player's answer; or an **accepted event**, holding
  its league and its [event lineup](#event-lineup);
- the **call counter**, and per league its **last call number** (none, or a
  call number), read for [which league calls](#which-league-calls);
- the league state's **content versions**, one set for all of the state
  below: the league registry, the rosters, world progress and the scalers, the
  growth archetypes, the evolution-level table, the move pools and learnsets,
  and the play styles and AI tiers, written at New Game and rewritten after
  each [content-change cleanup](#load-validation);
- the **most recent resolved lineup**: the `characterId`s of the most
  recent resolved event (five, eight for a declined Masters event, or eight
  and the partner for an accepted one), read for fatigue; empty on a new
  game;
- the [partner choice](#partner): none or one `characterId`;
- per league, the **reigning champion** (none, the player, or a
  `characterId`);
- per notable trainer, the two [reign flags](#reign-records) (has reigned at
  Indigo, has reigned at Hoenn);
- the [Masters Gallery](#masters-gallery) win counts, one per notable trainer
  and one for the player; and
- the **active run**, only while the player is fighting their accepted event:
  the defeated prefix, and at the Masters the **tag selection**, the party
  slots the player chose at the current door, present only between that
  choice and the end of its [party restore](#masters-party-and-heal).

The reign flags are unchanged by the tag format. Contact bits are
[Notable trainers](notable-trainers.md#phone-contacts)' saved state; this
spec only reads them.

There is no seed, rotation history, or lineup history beyond the most recent
resolved lineup, the reign flags, and the Gallery counts, and no stored day
other than the countdown's last counted day. At most one accepted event and
one active run exist. New Game saves the not-qualified state, a call counter
of 0, no last call number, reigning champion, or most recent resolved lineup,
every reign flag clear, every Gallery count 0, and the build's content
versions, with no partner choice. Save an explicit schema
discriminator for this layout; prerelease saves need no migration.

## Lifecycle

### Qualifying and the call

After any player TR change, and on each day advance, a not-qualified player
whose TR is at least 80 starts counting down: 7 days remaining, with the
current day as the last counted day.

When the countdown is due, choose the calling league
([Which league calls](#which-league-calls)); with no eligible league, nothing
happens until a later check. Otherwise, in one transaction, save the invited
state with that league, advance the call counter, and give that league the
new number as its last call number. The phone rings at the next moment the
player can take a call (in the overworld, with no script, battle, or ceremony
running); an invitation not yet answered rings again after a reload.

### Answering

The call asks the player to accept or decline; there is no "later".

- **Accept.** Select the lineup
  ([Selection and order](#selection-and-order)) and atomically save the
  accepted event with its event lineup, then reveal the league and the
  names (at the Masters, the four pairs and the partner).
- **Decline.** Select the lineup and, in one transaction, set the league's
  reigning champion to their strongest, set that trainer's reign flag for the
  league (Indigo or Hoenn) or, at the Masters, add their Gallery win, save
  the lineup as the most recent resolved lineup, and restart the countdown
  (7 days remaining from today).

A failure leaves the invited state intact, so the call rings again; a crash
exposes either the invited state or the complete result. Answering creates no
battle, reward, or record.

### Entering the accepted event

1. Validate admission: the player has an accepted event at this league, and
   no pending ceremony, active run, or transaction. A refusal states only the
   immediate reason (no invitation yet and when the next call is due, an
   unanswered call, or an event accepted at another league) and changes
   nothing.
2. Save the active run and start at match 1, with the lineup (and at the
   Masters the partner) and their teams from the event lineup.

### Loss

A loss blacks the player out as usual. A blackout or voluntary exit ends the
event, in one transaction: the frozen lineup's strongest becomes the league's
reigning champion (with their reign flag at Indigo or Hoenn, or their Gallery
win at the Masters), except that a loss in the Masters final crowns the
partner (with their Gallery win); the event's lineup (at the Masters, with
the partner) becomes the most recent resolved lineup, the run and the
accepted event are released, the countdown restarts (7 days remaining from
today), and nothing is recorded or rewarded. A tag match loss first
[restores the party](#masters-party-and-heal), then blacks out.

### Win

After five victories, one transaction atomically:

- makes the player the league's reigning champion (at the Masters, also
  adding the player's Gallery win);
- if this is the player's first-ever win at this league, records the lifetime
  win and Today's first-league-win effects other than player TR, which a win
  never changes, and queues Today's ceremony; a repeat win instead gives its
  [repeat-win reward](#first-and-repeat-wins);
- saves the event's lineup (at the Masters, with the partner) as the most
  recent resolved lineup, releases the run and the accepted event, and
  restarts the countdown (7 days remaining from today).

Stale or duplicate callbacks are rejected. Individual victories, losses, and
Red add no TR.

## Active run and dispatch

Progress belongs to the accepted event's lineup; global Trainer defeat flags
cannot skip a match.

1. Validate the accepted event, the active run, and the destination before
   locking an entrance or changing room state.
2. Each battle validates league, room, and expected match. The stored team and
   versioned references supply party, class, sprite, portrait, name,
   introduction, defeat text, music, and AI. Fixed room-owner IDs never pick
   opponents.
3. A victory advances one match exactly once. A loss follows Loss.
4. Room and ceremony transitions keep the league identity until the win
   commits.

Live badges, league wins, player TR, party, XP, the day counter, and a
later partner choice never mutate an event lineup. Debug and other battles
cannot create invitations, accepted events, or runs, advance matches, grant
league wins or titles, or create league records.

### Masters party and heal

In a tag match each trainer brings three Pokémon, the engine's fixed
per-trainer share of a multi battle. The player's three come from their
current party, and their state carries from hall to hall:

1. **Choose three.** At each hall's door (matches 1-4), after the pair is
   introduced, the player picks three party members on the engine's
   existing choose-3 screen (`ChooseHalfPartyForBattle` in
   [script_pokemon_util.c](../../game/src/script_pokemon_util.c)). The
   screen's own rules apply: it refuses fainted Pokémon, and it accepts one
   or two when the player chooses fewer, so a party with fewer than three
   able Pokémon can still fight. Cancelling returns to the door with nothing
   started, as Mossdeep's Steven battle does.
2. **Save and reduce.** Save the whole party as the backup
   (`SavePlayerParty`), record the chosen slots as the run's tag selection,
   then reduce the battle party to the three in the chosen order
   (`ReducePlayerPartyToSelectedMons`, which the `multi_2_vs_2` macro's
   `multi_do` runs); the partner's three fill `gPlayerParty[3..5]`.
3. **Restore.** When the battle ends, won or lost, write each of the three
   back to its original slot in the backup, then reload the whole party, as
   `CB2_EndDebugBattle` in
   [battle_setup.c](../../game/src/battle_setup.c) does with
   `frontier.selectedPartyMons` and `LoadPlayerParty`, and clear the tag
   selection. The three keep their HP, PP, status, fainting, experience,
   levels, and consumed items; the rest of the party is untouched; the
   partner's Pokémon leave with the battle. The multi macro's own
   end-of-battle copy stays skipped (`MULTI_BATTLE_CHOOSE_MONS`), so this is
   the only restore.
4. **No heal between halls.** Damage carries over: nothing heals the party
   between matches 1-4 beyond what the player does themselves, as between
   any two league matches. The pair and the partner are built fresh from the
   event lineup for each match.
5. **Full heal before the final.** When match 4 is won and the party is
   restored, fully heal the player's whole party (HP, PP, and status), and
   the partner's team is built fresh for the final: both sides start it
   whole.

No save point exists between the choice and the end of the restore: the
choose-3 screen, the battle, and the restore run in one script, so a reset
at any moment of a tag match reloads the last save, where the party is whole
and no tag selection is recorded. That is neither a win nor a loss: the run
resumes wherever that save was made, and the match is fought again, as after
a reset in any league match. A save routine that ever runs in that window
(such as a future autosave) must write the backup as the party, never the
reduced battle party; [load validation](#load-validation) recovers such a
save.

## Battle construction

Each match fights with the trainer's own team at their own TR, as
[Notable trainers](notable-trainers.md) composes it for any battle: there is
no league-specific level offset, role adjustment, or six-member competitive
profile. Its AI flags come from [Trainer AI](trainer-ai.md) at the
trainer's TR, as in any notable battle. The old `[-4,-3,-2,-1,+1]` room offsets are removed.

Construct from the saved event lineup. Resolve the selected trainer and roster owner
before applying league policy; never identify enrollment from class, map, or
a shared party pointer. Record runtime Trainer ID, source roster owner, and
content version, and preserve member identity through ordering and gimmick
remapping. Content is immutable at runtime; do not edit shared Gym or story
parties.

### Hall condition

The hall condition comes from the [location registry](#halls), never from
the trainer: construction ignores the selected trainer's authored
`struct Trainer.startingStatus`, and no authored trainer data supplies a
hall condition. Resolve it at battle start, alongside the battle snapshot,
from the validated league, room, and match, and write it once per battle
through a per-battle override, as
[Trainer AI](trainer-ai.md#runtime-and-the-override-point) writes its flags:

- **Rooms, terrains, Tailwind, Sea of Fire, and hazards** use the engine's
  starting statuses (`STARTING_STATUS_*` in
  [constants/battle.h](../../game/include/constants/battle.h)). Battle start
  ORs the trainer's `startingStatus` into `gStartingStatuses`
  ([battle_main.c](../../game/src/battle_main.c),
  `UNPACK_STARTING_STATUS_TO_BATTLE`); directly after that, a league match
  replaces `gStartingStatuses` with the hall's: `trickRoomTemporary`,
  `magicRoomTemporary`, `wonderRoomTemporary`, `psychicTerrainTemporary`,
  `grassyTerrainTemporary`, or `mistyTerrainTemporary` (5 turns each); both
  `tailwindPlayerTemporary` and `tailwindOpponentTemporary` (the engine's
  `B_TAILWIND_TURNS` duration on each side); both
  `seaOfFirePlayerTemporary` and `seaOfFireOpponentTemporary` (4 turns on
  each side); both `stickyWebPlayer` and `stickyWebOpponent`; or both
  `stealthRockPlayer` and
  `stealthRockOpponent`. The first-turn starting-status step
  (`FIRST_TURN_EVENTS_STARTING_STATUS`, through
  `TryFieldEffects(FIELD_EFFECT_TRAINER_STATUSES)` in
  [battle_util.c](../../game/src/battle_util.c)) applies them with their
  usual messages and animations, one after another, and clears each flag as
  it applies it.
- **Weather** has no starting status; it comes from the room itself. A snow
  or sandstorm hall's room map carries that weather in its `map.json`
  (`WEATHER_SNOW` for `PokemonLeague_LoreleisRoom_Frlg`, `WEATHER_SANDSTORM` for
  `PokemonLeague_BrunosRoom_Frlg`), map loading saves and starts it
  (`SetSavedWeatherFromCurrMapHeader()` and `DoCurrentWeather()` in
  [overworld.c](../../game/src/overworld.c)), and the
  battle's first-turn weather step (`FIELD_EFFECT_OVERWORLD_WEATHER` in
  battle_util.c) turns it into battle weather as it does outdoors: it reads
  `GetCurrentWeather()` and sets `gBattleWeather` with no duration, so it
  lasts until replaced, and `B_OVERWORLD_SNOW` makes overworld snow battle
  snow. No battle-side weather override or Wayfarer-only map patch is
  needed. These maps are shared with the standalone FRLG build, so its rooms
  show the weather there too, and a battle in them starts in it; that is
  accepted, since only Wayfarer is a product target. No hall room
  script sets or resets weather today, and none may.

A neutral hall writes no starting status and keeps the room's own weather
(none in every league room today), and its empty `gStartingStatuses` also
discards anything the trainer's authored data contributed. The starting
statuses belong to the battle: the engine consumes them as it applies them,
reconstruction of the same match writes the same condition, and every other
battle, including debug battles and Gym and story battles, reads the
engine's usual sources unchanged.

Challenge options keep their overrides. Party randomizers keep their
precedence but cannot reroll participants: the trainer species randomizer may
bypass authored parties as it does today but still uses the saved people and
order. Its legacy party for a selected trainer is that trainer's own authored
source party: their league source party where they have one, otherwise their
Gym party (Lorelei's League party for Lorelei, Brock's Gym party for Brock in
match 1 at Indigo), as today's constructor randomizes the party of the trainer
it builds; another room occupant's old fixed party never stands in. XP uses actual species and levels. Prize money uses the trainer's
inventoried source reward basis and class, not the old room occupant, and team
size must not shift it.

### Tag matches

Masters matches 1-4 are tag matches: the player and the partner against
pair N, in the engine's existing two-versus-two partner battle. Mossdeep's
Steven battle
([scripts](../../game/data/maps/MossdeepCity_SpaceCenter_2F/scripts.inc))
is the reference flow: `SavePlayerParty`, then `ChooseHalfPartyForBattle`,
then the `multi_2_vs_2` macro
([battle_tower.inc](../../game/asm/macros/battle_frontier/battle_tower.inc)),
which calls `SetMultiTrainerBattle`
([battle_setup.c](../../game/src/battle_setup.c)) and starts
`SPECIAL_BATTLE_MULTI` in
[battle_special.c](../../game/src/battle_special.c); that sets
`BATTLE_TYPE_TRAINER | DOUBLE | TWO_OPPONENTS | MULTI | INGAME_PARTNER` and
calls `FillPartnerParty`, and the script then reads `VAR_RESULT` and calls
`SetCB2WhiteOut` on a loss.

- **Three each.** The engine fixes three Pokémon per trainer: the player's
  in `gPlayerParty[0..2]`, the partner's in `gPlayerParty[3..5]`
  ([battle_partner.c](../../game/src/battle_partner.c) caps them at 3), and
  each opponent's half of `gEnemyParty`, capped at `PARTY_SIZE / 2` in a
  two-opponent battle (`CreateNPCTrainerPartyInternal` in
  [battle_main.c](../../game/src/battle_main.c)). Opponent A fills the first
  half and opponent B the second.
- **Best three, aces first.** Each notable trainer in a tag match brings
  the last three members of their stored team's
  [battle order](notable-trainers.md#rosters) (all of it with fewer than
  three), in that order. Battle order sends fillers first and aces last,
  and a roster has one to three aces, so this takes every ace, then the
  filler slots closest to them in battle order (the lowest-numbered filler
  slots in the team); the signature Pokémon still comes out last. Every
  member keeps the species, level, moves, item, ability, nature, and IVs/EVs
  the stored team resolved for it (its moves resolved against the whole
  team), so a Pokémon is the same in a tag match as anywhere else. Brock's
  full team (Omastar, Kabutops, Crobat, Golem, Aerodactyl, Steelix) brings
  Golem, Aerodactyl, Steelix.
- **Notable team path.** Today's scaling skips these battles: the scaling
  context excludes `BATTLE_TYPE_INGAME_PARTNER`
  (`IsTrainerScalingBattleContext` in
  [trainer_party_scaling.c](../../game/src/trainer_party_scaling.c)), and
  league rosters are skipped under `BATTLE_TYPE_TWO_OPPONENTS` in
  battle_main.c. A Masters tag match must build both opponents and the
  partner from the event lineup's stored teams through a notable team path
  that those exclusions do not stop; every other partner or two-opponent
  battle keeps today's construction.
- **Runtime partner slot.** Partners are the static `gBattlePartners`
  ([battle_partners.party](../../game/src/data/battle_partners.party)).
  Reserve one runtime partner slot whose trainer struct (name, class,
  front pic, back pic, AI flags) and three-member party are filled for each
  battle from the partner's stored snapshot and presentation, and point
  `gPartnerTrainerId` at it; `FillPartnerParty` builds `gPlayerParty[3..5]`
  from that party. No other partner entry changes.
- **AI.** `BattleAI_SetupFlags()` in
  [battle_ai_main.c](../../game/src/battle_ai_main.c) sets flags per
  battler: opponent A, opponent B, and the partner. The per-battle override
  writes all three from each trainer's own snapshot
  ([Trainer AI](trainer-ai.md#options-and-double-battles)).
- **Money.** `Cmd_getmoneyreward` in
  [battle_script_commands.c](../../game/src/battle_script_commands.c) adds
  both opponents' rewards, each from their authored party's last level. A
  tag match uses the snapshot level basis instead: each opponent's reward
  reads the level of the last member they bring (their signature Pokémon)
  with their class's rate, and the two are summed as the engine sums them.
- **Experience and whiteout.** The player's Pokémon gain experience as
  usual; the partner's gain none (the engine skips partner slots under
  `BATTLE_TYPE_INGAME_PARTNER`). With `B_MULTI_BATTLE_WHITEOUT` at
  `GEN_LATEST` ([config](../../game/include/config/battle.h)), the match is
  lost only when the player's and the partner's Pokémon have all fainted.
- **Halls.** The [hall condition](#hall-condition) is written the same way;
  each side's starting status covers both battlers on that side, and
  starting hazards hit all four leads, two per side, as Bruno's Hall's
  Stealth Rock does; a Sticky Web start would lower the Speed of each
  grounded lead the same way.
- **Rooms.** The Masters
  [room scripts](../../game/data/scripts/wayfarer_masters_league.inc)
  (`wayfarer_masters_league.inc`) start one singles battle per room, and
  each HNS hall room map holds one opponent object. Each of the four tag
  halls needs a second opponent object for opponent B, and its script
  starts the tag match through the flow above.

### The final

Match 5 at the Masters is a singles battle in the neutral Champion's Room
against the partner, after the [full heal](#masters-party-and-heal): the
player's whole party against the partner's whole stored team, in its battle
order. It is built like any league match: the partner's snapshot, AI flags
from [Trainer AI](trainer-ai.md) at their TR, and prize money as in any
league singles match. Winning it wins the event; losing it crowns the partner
([Reigning champion](#reigning-champion)).

## Load validation

Validate the schema, lifetime wins, pending transactions, and league state
before any dispatch, call, or answer. Not qualified, no accepted event, no
active run, and an empty most recent resolved lineup are normal; validation
never generates a lineup or places a call.

- The invitation state is exactly one of its four kinds. Counting down holds
  days remaining (0 to 7) and a last counted day; invited and accepted name a
  league whose last call number equals the call counter (the league that
  called most recently; the Masters only with lifetime wins at both Indigo
  and Hoenn); badges are not rechecked, since badges never decrease.
- A stored day is never a reason to reject a save. A last counted day ahead
  of the day counter (a clock turned back) is clamped to the day counter,
  which neither advances nor resets the countdown.
- Each league's last call number is none or a number from 1 to the call
  counter; no two leagues share a number, and when the counter is above 0 one
  league holds it. All are none only when the counter is 0, which means the
  player has never been called (not qualified, or counting down to the first
  call). The Masters has a number only with lifetime wins at both Indigo and
  Hoenn.
- An accepted event's lineup holds five distinct eligible characters (eight
  at the Masters) in non-decreasing TR order and resolves every reference; a
  Masters event also holds an eligible partner who is none of the eight.
  Stored teams must be valid for their roster (known roster slots, legal
  forms, levels, and moves); they are never recomposed on load.
- The most recent resolved lineup is empty or holds five, eight, or nine
  distinct known characters; a reigning champion is none, the player, or a
  known character.
- The partner choice is none or a known eligible character whose contact bit
  is set.
- Reign flags and Gallery counts exist only for known notable trainers (and
  the player's Gallery count). A trainer reigning at Indigo or Hoenn has that
  league's reign flag; a trainer reigning at the Masters has a Gallery count
  of at least 1; the player reigning at a league has that league's lifetime
  win (at the Masters, also a Gallery count of at least 1). Flags and counts
  are otherwise never rechecked against history, which the save does not
  keep.
- An active run exists only with an accepted event and names a match within
  its lineup. A tag selection exists only in a Masters run at matches 1-4
  and names one to three distinct party slots.

On every load, first recover a mid-tag save: a Masters run holding a tag
selection was saved between a door's choice and its restore, which
[normal play never does](#masters-party-and-heal). Its saved party is the
whole backup from the door, so keep that party as it is (each chosen member
as it was at the door), clear the tag selection, and resume the run at that
match's door with the match unfought; the reset is neither a win nor a
loss. A tag selection that names slots outside the saved party is corrupt.

Then, before the checks above, drop the reign flags and Gallery counts of
characters no longer in the registry, keeping the rest, clear a reigning
champion who is no longer in the registry, and clear a partner choice naming one
(so Lorelei steps in again); this runs whatever the versions say, so a save that
crossed several content builds still loads. If any saved league content version
differs from the build's, also, still before those checks, turn an accepted
event back into an unanswered invitation from the same league (dropping its
lineup and any run), so the call rings again; clear the most recent resolved
lineup, so the next selection has no fatigue; clear a reigning champion or
partner choice who is no longer an eligible character; then rewrite the saved
content versions as the build's. This is the prerelease policy, not an invalid
save.

A valid active run with damaged run progress recovers to its own lobby with
progress reset and the event lineup kept, so the attempt restarts at match 1.
A missing, corrupt, or unsupported invitation state, lineup, run, or league
state follows standard invalid-save handling: never regenerate a lineup on
load or synthesize results.

## Presentation

Invitations and results arrive by **phone call**, on the Pokégear/PokéNav
phone in the HNS/Emerald style; the exact phone UI is implementation. The
invitation call names the league and asks the player to accept or decline.
Accepting names the lineup and its battle order (moves and items hidden by
default) and says the event waits for the player; declining says the event
goes ahead without them and names its reigning champion. A win or a loss is
followed by the league's call or lobby word on the result and the title.
Before an answer, lineups are unavailable and inspection generates nothing.

The Masters' call comes from the caretaker, not from an official league.
Her first call says that Lorelei, retired to Four Island, spoke of the player;
beyond being the default [partner](#partner), Lorelei stays an ordinary
notable trainer with no guaranteed seat. Accepting names the four pairs in
order, which of the eight hold a Master's seat, and the partner who will
join the player. When the run starts, the caretaker introduces the pairs in
the lobby, and each hall's door names its pair along with the hall and its
condition. The Masters Gallery shows every recorded winner
([Masters Gallery](#masters-gallery)).

The player asks a partner with an outgoing phone call to a contact
([phone calls](../../game/src/match_call.c)); the contact agrees, and the
call says they will join the player at the next Masters event. The phone's
contact list shows who can be asked and who is the current partner. In the
tag matches the partner appears beside the player with their back pic, and
in the final they are the opponent, with their own lines.

Each lobby names the league's reigning champion (or none yet). Without an
accepted event there, staff turn the player away with the immediate reason
([Entering the accepted event](#entering-the-accepted-event)).

Graphics, portraits, dialogue, battle metadata, and names follow the selected
character even in historical rooms; a displaced fixed resident must not remain
in dialogue. The last opponent is this lineup's finalist, whatever their title;
at the Masters it is the partner.
Dialogue cannot assume Blue occupies Indigo or Lance ends Masters. Masters
never calls its winner a regional Champion.

The lobby, or a sign at each hall's door, names the hall and its condition
("Lorelei's Hall: snow") before the match; a neutral room shows no condition.
A snow or sandstorm hall shows its weather in the room.
The hall's name honours its Elite Four member and never stands for the
occupant: dialogue and signs never say or suggest that the honoured member
fights there. The condition starts with the engine's usual start messages and
animations for that weather, room, terrain, Tailwind, Sea of Fire, or hazard.

## Records, ceremonies, and integration

### First and repeat wins

The player's first-ever win at each league keeps the one-time effects as Today's
[interregional circuit](wayfarer-interregional-league-circuit.md) implements
them: Indigo's shared Kanto/Johto Champion recognition with one Hall of Fame
registration, Hoenn's own recognition, Hall of Fame, regional cleanup, and full
completion credits, Red's unlock once all three leagues have a lifetime win, and
Blue's Saffron Dojo battle after the first Indigo win. The first Masters win
brings no regional awards or new status. A repeat win gives the prize money of
its battles and the reigning-champion title (plus the Gallery win at the
Masters) and nothing else: the one-time effects never repeat, and whether a
repeat win shows a short ceremony follows Today's replay presentation until
designed. Wins grant no TR. Ceremony handling stays idempotent across callbacks,
reloads, and interrupted saves, and must finish before the next call rings.

### Story dependencies and travel

Red stays outside the pool. His Mt. Silver admission reads all three saved
lifetime wins; his encounter and rewards are unchanged. Blue's Saffron Dojo
battle keeps its current unlock on the first committed Indigo win, whether or
not Blue was in the lineup.

Preserve current travel: the S.S. Aqua maiden voyage and Ticket,
Olivine–Vermilion–Slateport service, and numbered Sevii service from Vermilion
stay independent of badges, league wins, Rainbow Pass, Bill/Celio, National
Pokédex, and Sevii quests. Any league may call a player who qualified in any
region, so audit that every league is reachable from any region at player
TR 80 and after the first league win. Audit back warps, exits, healing and
blackout targets, Dig, Escape Rope, and ceremony returns; a blackout after a
league loss must end the event exactly once, and Masters never routes to the
HNS Indigo lobby.

Audit checks that equate a room with a fixed opponent, read the old
fixed-lineup run record, or assume the fixed Indigo → Masters → Hoenn order,
before changing them. Preserve Giovanni's Earth Badge
and local finale, optional quests, deferred rewards, voyage state, initial Gym
access, and other regions' unfinished stories.

### Integration surfaces

Existing code to review, not new APIs:

- [Save ownership and initialization](../../game/src/wayfarer_persistence.c)
  and [run/save structures](../../game/include/global.h).
- The planned in-game clock's day count (no design yet; Today's RTC day
  counter and daily update live in [clock.c](../../game/src/clock.c)) and
  [phone calls](../../game/src/match_call.c).
- [Circuit admission and lifecycle](../../game/src/league_circuit.c),
  [script wrappers](../../game/src/league_circuit_scripts.c),
  [script entry points](../../game/data/scripts/league_circuit.inc), and
  [warp/blackout handling](../../game/src/overworld.c).
- [Battle construction](../../game/src/battle_main.c),
  [scaling policy and roster validation](../../game/src/trainer_party_scaling.c),
  [current fixed League metadata](../../game/src/data/trainer_scaling/league.h),
  [player TR producer](../../game/src/trainer_rating.c), and
  [post-battle ceremonies](../../game/src/post_battle_event_funcs.c).
- [Trainer Card](../../game/src/trainer_card.c),
  [circuit status](../../game/src/league_circuit_status.c),
  [Sevii content manifest](../../game/src/data/wayfarer_sevii_maps.json), and
  [Blue Dojo scripts](../../game/data/maps/SaffronCity_FightingDojoVIP_hns/scripts.inc).
- The tag match path listed in [Tag matches](#tag-matches):
  [partner battle setup](../../game/src/battle_special.c),
  [partner parties](../../game/src/battle_partner.c),
  [AI flag setup](../../game/src/battle_ai_main.c),
  [party choice](../../game/src/script_pokemon_util.c),
  [money and experience](../../game/src/battle_script_commands.c), and
  [Masters room scripts](../../game/data/scripts/wayfarer_masters_league.inc).

## Balance report

League balance is informational in v0; tuning comes later. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) simulates a
chosen number of invitations at a chosen player TR (world progress) and badge
split (Kanto, Johto, Hoenn), with the player's answer to each (accept and
win, accept and lose, or decline); accepted events resolve the day they are
accepted. For each invitation it reports the day it arrives, which league
calls and why (the only eligible league, the least recently called, or a
tie going to the most badges or to Indigo; whether the Masters knows the
player yet), the frozen lineup with league scores and Master seats, the event
whose lineup it fatigues, the result, and the reigning champion, and after
the run who has reigned at Indigo and at Hoenn, the notable Masters, and the
Masters Gallery. For a selected event it reports every eligible trainer's TR,
team level, willingness, league score, rank, and reign flags, the base lineup
level, each aloof trainer's check (team level against base lineup level + 10,
joins or skips; at the Masters, the aloof rule off), the Master seats, and
the resulting lineup. It doesn't show each match's hall and condition yet,
or the Masters' tag format: its Masters lineups still take five seats, with
no partner, pairs, or three-member tag teams ([Later](#later)). It asserts
no fixed lineup,
finalist, or strength target. The report is informational; it also confirms
that no aloof trainer in an Indigo or Hoenn lineup is more than 10 levels
above the base lineup level. The Gym ladder and team targets stay in
[Notable trainers](notable-trainers.md#balance-targets).

## Acceptance

Check in an inventory: aliases, roster references, presentation coverage,
exclusion reasons, provenance, and content versions. Required implementation
evidence (not yet run):

1. **Registry.** Reject duplicate characters or aliases, unresolved source
   IDs, missing assets, double-battle flags, and trainers without valid
   growth values or a valid roster. The build must hold at least five eligible
   trainers who are not aloof and at least nine eligible trainers (the
   Masters' eight and a partner); Lorelei is eligible; every eligible
   trainer has a back pic for partnering. Each league has exactly five
   halls whose rooms match its room chain in order; Indigo and Hoenn halls
   1-4 each honour a
   distinct member of that league's own Elite Four; the Masters' halls 1-4
   honour Will, Koga, Bruno, and Karen, and each Champion's Room is neutral
   and honours no one; every condition is one of neutral, snow, sandstorm,
   Trick Room, Magic Room, Wonder Room, Psychic Terrain, Grassy Terrain,
   Misty Terrain, Sea of Fire (both sides), Tailwind (both
   sides), Sticky Web (both sides), or Stealth Rock (both sides); no two
   halls share a condition other than neutral; a hall has weather exactly
   when its condition is snow or sandstorm.
2. **Selection.** Fixtures at several world progress values, including one
   where growth reorders the lineup: willingness for at-home, away
   non-traveller, away traveller, fatigued, and floored cases at each league,
   Masters as a neutral location; the league score floored; the base lineup
   level taken from the top five non-aloof trainers only; an aloof trainer
   joining at exactly base lineup level + 10 and skipping at one level more,
   never judged against another aloof trainer, and skipping when there is no
   base lineup; fatigue applied to the base lineup and to an aloof trainer's
   score; at the Masters no base lineup and every aloof trainer eligible
   however far above the other trainers; a Master seated over a higher-scoring
   trainer who is not one, more than eight Masters seated by league score
   with ties by `characterId`, a fatigued Master keeping the seat, a trainer who
   reigned at only one of Indigo and Hoenn not a Master, and Master status
   ignored at Indigo and Hoenn; the five highest scores (eight at the
   Masters) over distinct scores and ties at the last-seat boundary broken
   by ascending `characterId`; fatigue read from the most recent resolved
   event at any
   league, accepted or declined, including the partner after an accepted
   Masters event; the battle
   order non-decreasing in TR with the highest last; excluded and disabled
   trainers never appear; aliases never appear twice. At the Masters: eight
   seats, up to eight Master seats, the partner never seated (a Master who
   is the partner gives up the seat and the next score moves in), the same
   eight whether the player accepts or declines with the same partner, and
   pairs taken two by two in battle order.
3. **Qualification.** No call at player TR 79; at 80 the countdown starts on
   the day of qualifying and the first call comes 7 days later.
4. **Countdown.** Each resolution (a decline, a win, a loss, leaving) restarts
   the countdown at 7 days; no call arrives while an invitation is unanswered
   or an accepted event waits, however many days pass; a day counter turned
   back neither advances nor resets the countdown, which then counts forward
   from the lower day; a save whose last counted day is ahead of the day
   counter loads, clamped, and is never rejected; a counter jump of many days
   brings one call and no backlog; strength is identical on every day at one
   world progress.
5. **Which league calls.** Known-there eligibility: with no badge in a league's
   regions that league never calls (Indigo counting Kanto and Johto), and the
   Masters never calls before lifetime wins at both Indigo and Hoenn: not after
   wins at only one, in either order. Among the eligible leagues the least
   recently called calls, a never-called league first: with three eligible
   leagues they call in a fixed rotation. Ties go to the most badges, then to
   Indigo; the very first call comes from the most-badges eligible league. A
   single eligible league calls repeatedly; a league that becomes eligible later
   calls next. With no eligible league, a due call waits and is checked again
   each day until a league knows the player. The call counter only increases,
   and a day counter turned back never changes the order. The same badges, wins,
   and last call numbers always give the same caller.
6. **Accept and decline.** Accepting saves the selected lineup (at the
   Masters, with the partner) in ascending TR order with the accepted event
   atomically, and the event waits across many days and reloads with the
   same lineup; reloading before the commit selects the same lineup;
   inject failures before, during, and at commit. Declining
   crowns the strongest of the lineup computed at decline, saves it as the
   most recent resolved lineup, and restarts the countdown. Entering a league
   without an accepted event there is refused and changes nothing. A decline
   sets the new champion's reign flag at Indigo or Hoenn, or adds their
   Gallery win at the Masters, and a Masters title never sets a reign flag.
7. **Determinism.** Golden lineups and call sequences for fixed world
   progress values, badge splits, answers, most recent resolved lineups,
   reign records, and partners, matched between host tooling and game C;
   the same inputs always give the same lineup and the same caller, and the
   Pokémon RNG state is unchanged.
8. **Fresh state.** A new game is not qualified, has a call counter of 0, and
   has no last call number, reigning champion, accepted event, active run,
   most recent resolved lineup, reign flag, or Gallery win; load, display,
   and denied or cancelled requests generate nothing.
9. **Loss.** Lose at each match, and leave voluntarily: the event ends once,
   nothing is recorded, the player blacks out to the usual target, the frozen
   lineup's strongest reigns and gains the reign flag (or Gallery win), that
   lineup becomes the most recent resolved lineup, and the countdown
   restarts, also across a reload. At the Masters, a loss in any hall or
   leaving before the final crowns the lineup's strongest, a loss in the
   final crowns the partner with a Gallery win, and a tag match loss
   restores the party before the blackout.
10. **Win.** A first win records exactly one lifetime win and its one-time
    effects and ceremony; the second of the Indigo and Hoenn first wins makes
    the player a Master and the Masters eligible to call; a Masters win adds
    the player's Gallery win and no status, on every Masters win; a repeat
    win gives only prize money and the title (plus the Gallery win at the
    Masters); the player reigns until that
    league's next resolved event; no win changes player TR; the run and the
    accepted event are released. Interrupt and repeat win commits, calls, and
    answers.
11. **Construction and presentation.** Build every eligible trainer's team,
    preserve member identity and metadata, and reconstruct identically from
    the saved event lineup. Names, sprites, portraits, text, music, AI, money,
    XP, and parties match the lineup's matches, including a Gym Leader in
    match 5. The phone rings only when the player can take a call, and rings
    again after a reload until answered; lobbies name the reigning champion.
    The Masters' calls come from the caretaker, the first naming Lorelei;
    the Gallery shows the winners of declined Masters events too.
12. **Halls.** Every match is fought in its hall and starts with that hall's
    condition on both sides: snow and sandstorm last until a move or ability
    replaces them; rooms and terrains end after 5 turns; Tailwind and Sea
    of Fire are up on both sides and end on each after the engine's
    temporary duration; Sticky Web and Stealth Rock are down on both sides,
    hit every lead (all four in a tag match), and stay until cleared. The
    same trainer gets each hall's
    condition in whichever match they fight, a trainer with an authored
    `startingStatus` gets only the hall's, and neutral rooms start clear. A
    snow or sandstorm hall shows its weather in the room on entry, after a
    reload, and after returning from the battle, and the room is otherwise
    unchanged; the lobby or door sign names the hall and its condition; the
    condition is the same when the match is reconstructed after a reload;
    and no other Wayfarer battle or map (Gym, story, or debug) gains a hall
    condition or weather. In standalone FRLG, Lorelei's and Bruno's rooms
    show their weather and their battles start
    in it; that change is accepted.
13. **Load validation.** Corrupt invitation state, call numbers, accepted
    events, runs, lineups, most recent resolved lineups, partner choices,
    tag selections, reign flags, Gallery counts, schema, or callbacks are
    rejected without regenerating, calling,
    advancing, or rewarding; a Masters invitation without both lifetime wins is
    rejected, and so are a trainer reigning at the Masters with a Gallery count
    of 0 and the player reigning at a league without its lifetime win. A content
    version change turns an accepted event back into an unanswered invitation,
    clears the most recent resolved lineup, drops the reign flags and Gallery
    counts of removed characters, and rewrites the saved content versions. A
    double update loads too: a save that went through one content change and
    was saved again before the next event resolved, then loaded under a build
    that removes a character who reigns at Hoenn or has a Gallery count, is
    pruned, not rejected.
14. **Standalone.** FRLG, HNS, and Emerald League behavior, travel, recovery,
    and phone calls are unchanged, apart from the accepted hall-room weather
    (item 12); Mossdeep's Steven battle and every other partner or
    two-opponent battle are built as today.
15. **Partner.** With no ask, the partner is Lorelei, with or without her
    number; asking any contact always succeeds and sets the partner, the
    latest ask winning; a non-contact and Tate & Liza cannot be asked, and
    no one can be asked before the player is a Master; an ask while an
    accepted Masters event waits leaves that event's partner and lineup
    unchanged and applies to the next.
16. **Tag matches.** Each tag match is a two-versus-two partner battle
    against its pair, opponent A the weaker, under its hall's condition,
    with starting hazards on all four leads. Each notable brings the last
    three of their stored battle order, with fixtures for teams of one,
    two, three, and six members and one, two, and three aces, each member
    matching its stored species, level, moves, and item. Both opponents
    and the partner come from the event lineup despite today's scaling
    exclusions; the runtime partner slot shows the partner's name, class,
    pics, and AI, and no static partner changes. The override writes the AI
    of battlers 1, 2, and 3. Prize money sums both opponents at their
    snapshot levels; the partner's Pokémon gain no experience; the match
    goes on while the partner can still fight. Reconstruction after a
    reload builds the same match.
17. **Party and heal.** At each door the player picks one to three able
    Pokémon, and cancelling starts nothing; after a win or a loss the
    fought Pokémon return to their own slots with their damage, experience,
    and evolutions, and the rest of the party is unchanged; damage carries
    from hall to hall; after hall 4 the whole party is fully healed, and the
    final uses the player's whole party against the partner's whole team.
    A reset at any moment of a tag match reloads a save with the whole
    party and no tag selection, and the match is fought again; a save
    carrying a tag selection loads with the party whole, resuming at that
    match's door.

Run the trainer/scaling mechanics suites and extend
[mechanics coverage](../../game/test/league_circuit.c),
[script coverage](../../game/test/league_circuit_scripts.c),
[status coverage](../../game/test/league_circuit_status.c), and
[the League E2E journey](../../e2e/src/journeys/wayfarer-league-circuit.e2e.ts).
Build the production Wayfarer configuration, relevant test ROMs, and affected
standalone configurations when shared code changes, and measure ROM/RAM
against the reserve policy. E2E needs explicit prebuilt ROM and symbol paths.
Report balance playtesting separately from structural checks.

## Later

- Seeded or reputation-based league choice, as candidate consumers of the
  [playthrough seed framework](playthrough-seed-framework.md) or a future
  reputation system.
- Invitation news: who won the events the player declined, as NPC gossip, TV,
  or phone news.
- Special invitational events.
- Event rewards tuning, interval tuning, and qualification threshold tuning.
- Balancing tools: tune travel costs, fatigue, the floor, and the aloof margin
  against the lineup reports, and set league balance targets.
- The caretaker's later calls and lines, and what the Masters Gallery shows
  beyond each winner's count.
- Seeded lineups, varying which willing trainers come per save.
- Rotation weights and rotation history.
- Role windows and standing-based matches, with a nearest-standing fallback for
  empty windows.
- A Trainer Card view of the three leagues and their reigning champions.
- Winning-team records per league event, and presentation of a player's
  record across a run of events.
- Gym arenas with their own field conditions, like the halls
  ([Notable trainers](notable-trainers.md#later)).
- Each match's hall and condition in the balance explorer.
- Explorer support for the Masters' tag format: the partner, eight seats,
  the pairs, and each notable's three for a tag match.
- A friendship score per contact, raised by partnering and other shared
  play; picky partners who can refuse (by friendship or by trait, such as
  aloof); asking a partner in person in the overworld; and gifts and trades
  with contacts ([Notable trainers](notable-trainers.md#later)).

## References

- [Leagues PRD](../prds/leagues.md)
- [Notable trainers](notable-trainers.md)
- [Trainer AI](trainer-ai.md)
- [Player Trainer Rating](player-trainer-rating.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Existing League scaling contract](league-scaling.md)
- [Party construction](../../game/src/battle_main.c)
