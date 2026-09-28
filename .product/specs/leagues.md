# Leagues

PRD: [Leagues](../prds/leagues.md)
Implemented: No
Design status: v0 approved: leagues as locations holding recurring
**league events** on a staggered calendar of in-game days, Indigo and Hoenn
open from 8 badges in any order and the Sevii Masters from 16 badges after a
regional win, one attempt per event, a league score per eligible trainer
(Trainer Rating (TR) scaled by willingness, from travel cost and fatigue),
aloof trainers joining only a base lineup near their level, the top five by
league score with no randomness, ascending battle order, a lineup computed
when the player enters and frozen for that event, free skipping, a
**reigning champion** per league between events, and first-win one-time
effects. Balance is informational for now. Today's
[interregional circuit](wayfarer-interregional-league-circuit.md) stays the
record of Today's admission, fixed league order, and replays.

## Scope

Own, for each `IS_WAYFARER` league: the league registry, eligibility,
location regions, the calendar and league events, entry rules, fatigue, the
league score, the base lineup and its level, the aloof rule, lineup selection
and battle order, the event lineup, the reigning champion, battle
construction, entering an event, active runs and dispatch, the win commit,
first and repeat wins, saved state, load validation, presentation, and
regional integration.

- [Notable trainers](notable-trainers.md) owns notable trainers, their
  TR and its growth with world progress, home regions, the traits (traveller
  and aloof), travel cost and willingness, the team-level and team-size
  scalers, rosters, and team composition. This spec reads a trainer's TR, team
  level, willingness, aloof trait, and composed team; it never restates how they are
  computed.
- [Player Trainer Rating](player-trainer-rating.md) owns the player's TR. In
  v0 a league win adds no player TR; Today's +8 per first league win stays
  documented in the circuit spec until adoption.
- Today's [interregional circuit spec](wayfarer-interregional-league-circuit.md)
  stays the record of Today's admission, fixed league order, and replays, and
  keeps owning the first-league-win facts, ceremonies,
  and unlocks this spec reuses. Upon adoption, this spec replaces its
  admission, order, replay, and loss rules with the calendar, entry rules,
  and one attempt per event.

Upon adoption this replaces the fixed lineups and the level policy based on the
player's TR when entering a league in [League scaling](league-scaling.md),
which stays the record of Today.

## League identity

League identity is stable and distinct from region: Indigo = 1, Masters = 2,
Hoenn = 3. The leagues keep their public entrances and rooms: FRLG Indigo, the
Seven Island Masters House leading into HNS rooms, and Emerald Hoenn.

Masters resolves to Sevii/Kanto for other regional systems. Its reused HNS
rooms need a Wayfarer-only map-context override; standalone HNS keeps its
identity. The final HNS ceremony room is the Masters Gallery. Geography, room
names, and titles never substitute for saved league or selected-character
identity.

Each league is a [location](notable-trainers.md#home-region-and-travel) with
a location region, which decides who is at home there:

| League | Location region |
| --- | --- |
| Indigo | Kanto, Johto |
| Masters | Neutral location: home to everyone |
| Hoenn | Hoenn |

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
values and a valid roster, fight in singles, are enabled, and have validated
presentation. Tate & Liza are a notable duo but fight only as a double battle,
so the singles-only rule makes them league-ineligible. Red is not a notable
trainer in v0, so he is league-ineligible and keeps his separate mastery
encounter. Region, title, and story
availability neither add nor remove a trainer. League eligibility does not
change story battles, which follow the
[every-battle rule](notable-trainers.md#trainer-rating).

## Calendar

Each league holds a **league event**, one tournament on a given in-game day,
every **3 in-game days** (a placeholder cadence). The **calendar** staggers
the leagues so that one league holds an event every day:

| League | Event days |
| --- | --- |
| Indigo | day mod 3 = 0 |
| Hoenn | day mod 3 = 1 |
| Masters | day mod 3 = 2 |

The day is the game's existing in-game day counter (`VAR_DAYS`, which the
clock's daily update in [clock.c](../../game/src/clock.c) keeps); leagues add
no clock of their own. **Days never affect anyone's strength**: every TR stays
a function of world progress, and days only schedule events.

An event is **current** on its day and **ends** when the day counter moves
past it. A league holds no event on a day at or before its last processed
event day, so turning the clock back never reopens an event or reruns one.

## Entry

| League | Opens at (placeholders) |
| --- | --- |
| Indigo | 8 global badges |
| Hoenn | 8 global badges |
| Masters | 16 global badges and at least one lifetime win at Indigo or Hoenn |

Indigo and Hoenn open in any order. Global badges follow Today's
[global badge count](wayfarer-interregional-league-circuit.md#global-badges).
A lifetime win is a saved first-league-win fact, so the Masters stays open
once it has opened.

The player may enter a league when its event is current, they meet its entry
rule, they have not attempted this event, and no ceremony is pending. There is
**one attempt per event**: a loss or leaving ends the event for the player, who
waits for that league's next event, and re-entering the same event is refused.
**Skipping is free**: missing an event costs nothing, and the next one comes
three days later.

## Selection and order

Every eligible trainer can be invited to any league; location regions only
scale how willing they are to come, and an aloof trainer skips a league whose
base lineup is well below their level. When the player enters an event, and
when an event ends (see [Event end](#event-end)):

1. **TR.** Compute each eligible trainer's TR at the current world progress
   ([Notable trainers](notable-trainers.md#growth-with-world-progress)).
2. **Willingness.** Score each eligible trainer with the
   [travel rule](notable-trainers.md#home-region-and-travel):
   `max(5, 100 - travelCost - fatigue)`. **Fatigue** is league-specific: 50 if
   the trainer is in the most recent event lineup, else 0. At home a trainer scores
   100 (50 fatigued); away, a [traveller](notable-trainers.md#traveller) 90
   (40) and anyone else 20 (5).
3. **League score.** `floor(TR × willingness / 100)`, in integer arithmetic.
4. **Base lineup.** Rank the eligible trainers who are not
   [aloof](notable-trainers.md#aloof) by league score, ties by ascending
   `characterId`, and take the top five (all of them if fewer than five):
   the **base lineup**. The **base lineup level** is the strongest
   [team level](notable-trainers.md#trainer-scalers) in it.
5. **Aloof.** An aloof trainer is eligible for this league only if their team
   level is at most base lineup level + 10 (a placeholder margin, in levels,
   not TR); otherwise they skip it. Aloof trainers are judged against the
   base lineup only, never against each other, so one aloof trainer joining
   never lets another in. With no base lineup (no eligible trainer who is not
   aloof), there is no base lineup level and every aloof trainer skips.
   Fatigue still applies: it shapes the base lineup through the league
   scores, and an aloof trainer who joins keeps their fatigued league score.
6. **Lineup.** Rank every trainer still eligible (the non-aloof and the aloof
   who join) by league score and take the five highest (all of them if fewer
   than five). Equal league scores break by ascending `characterId`.
7. **Battle order.** Order the five by ascending TR, so the strongest fights
   last. Equal TRs break by ascending `characterId`.

The **most recent event lineup** is the lineup of the most recent completed
event at any league, whether or not the player entered it. Each event's end
saves its lineup as the new most recent event lineup, after its own selection
has read the old one. Entering reads it but never changes it, because the
event it reads is still the last one completed.

Selection consumes no randomness and reads no seed: the same world progress,
most recent event lineup, and content always give the same five. The day,
title, reigning champion, party, and history play no part; player TR enters
only as world progress, through each trainer's TR. Other leagues' events are
never resolved early. When the player enters, the inputs become
authoritative at that moment, and the result is committed before reveal, so
nothing after it reselects.

## Event lineup

The five selected when the player enters are the event's lineup, frozen for
that event. Entering captures, for each of the five in battle order:
`characterId`, their TR, and their composed team, plus the registry and roster
content versions. Per member, the team holds the roster slot index and every
resolved battle value the battle snapshot uses: species/form, level, moves,
item, ability, nature, IVs/EVs, and battle order. It is saved atomically with
the active run before reveal, and the run reconstructs every battle from it,
including after a reload; it never reselects or recomposes, even after the
player's TR rises. The run releases it when it ends.

The event's result is taken at its end ([Event end](#event-end)), from the
lineup computed then with the same rule and inputs. Unless world progress rose
after the player entered, that is the same five they faced; either way it
never changes the lineup they fought.

## Reigning champion

Each league has at most one **reigning champion**, who holds the title from
the end of one of its events until the end of its next:

- If the player wins the event, the player is the reigning champion, from the
  win.
- Otherwise, at the event's end, the reigning champion is the strongest
  member of the lineup the event fields at that moment (the last in battle
  order), computed with the same rule, whether the player skipped, lost, or
  left.

A new game has no reigning champion; each league gains one when its first
event ends. The title is recognition only: it does not change selection,
battles, or rewards.

## Saved state

Keep Today's saved first-league-win facts (`indigoCleared`, `mastersCleared`,
`hoennCleared`, now the lifetime wins) and pending ceremony phase. Add:

- per league: the **last processed event day** (or none), the **reigning
  champion** (none, the player, or a `characterId`), and the player's
  **attempt** at its current event, if any (the event day, and whether they
  won);
- the **most recent event lineup**: the five `characterId`s of the most
  recent completed event, with its content versions, read for fatigue; empty
  on a new game; and
- the **active run**, only while the player is fighting: the league, the event
  day, the event lineup (the five matches), and the defeated prefix.

There is no seed, edition, rotation history, calendar copy, or lineup history
beyond the most recent event lineup. At most one active run exists, because
only one event is current on any day. New Game sets every league's last
processed event day to the day before the counter's current day, so no event
before the new game is processed, and saves no reigning champion, attempt, or
most recent event lineup. Save an explicit schema discriminator for this
layout; prerelease saves need no migration.

## Lifecycle

### Event end

When the daily update sees a new day, and before any entry, process the
events that have ended and are not yet processed, oldest first:

1. Each league's event the player attempted, once it has ended.
2. Then each league's latest ended event after its last processed event day.
   Older events missed while the counter jumped several days leave no trace.

Processing one event computes its lineup at the current world progress with
the [selection rule](#selection-and-order), reading the most recent event
lineup, and in one transaction: sets the reigning champion (the player if
their attempt won, otherwise that lineup's strongest), saves that lineup as
the new most recent event lineup, sets the league's last processed event day,
and clears its attempt. An event with an active run ends only after the run
ends; processing waits. Processing creates no battle, reward, or record, and
repeated or interrupted processing never processes an event twice.

### Entering an event

1. Process ended events ([Event end](#event-end)).
2. Validate admission: the league's event is current, the player meets the
   [entry rule](#entry), has not attempted this event, and has no pending
   ceremony, active run, or transaction. A refusal states only the immediate
   reason (no event today and the day of the next, badges, a regional win,
   or already competed) and changes nothing.
3. Select the five ([Selection and order](#selection-and-order)), then
   atomically save the attempt (entered, not won), the active run, and the
   [event lineup](#event-lineup).
4. Reveal the lineup and start at match 1.

A failure leaves the prior state intact; a crash exposes either the old state
or the complete attempt and run. Denied or cancelled requests change nothing
and reveal nothing.

### Loss

A loss blacks the player out as usual. A blackout or voluntary exit ends the
run and the player's attempt: the run and its lineup are released, nothing is
recorded or rewarded, and the attempt stays saved, so re-entering this event
is refused. The player waits for that league's next event, three days later,
and may earn badges meanwhile; that event's lineup is selected afresh when
they enter it. At this event's end, the reigning champion follows
[Reigning champion](#reigning-champion).

### Win

After five victories, one transaction atomically:

- marks the attempt won and makes the player the league's reigning champion;
- if this is the player's first-ever win at this league, records the lifetime
  win and Today's first-league-win effects other than player TR, which a win
  never changes, and queues Today's ceremony; a repeat win instead gives its
  [repeat-win reward](#first-and-repeat-wins); and
- releases the run and its lineup.

Stale or duplicate callbacks are rejected. Individual victories, losses, and
Red add no TR.

## Active run and dispatch

Progress belongs to the active run's event lineup; global Trainer defeat flags
cannot skip a match.

1. Validate the attempt, the active run and its lineup, and the destination
   before locking an entrance or changing room state.
2. Each battle validates league, room, and expected match. The stored team and
   versioned references supply party, class, sprite, portrait, name,
   introduction, defeat text, music, and AI. Fixed room-owner IDs never pick
   opponents.
3. A victory advances one match exactly once. A loss follows Loss.
4. Room and ceremony transitions keep the league identity until the win
   commits.

Live badges, league wins, player TR, party, XP, and the day counter never
mutate an event lineup. Debug and other battles cannot create runs or
attempts, advance matches, grant league wins or titles, or create league
records.

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

Challenge options keep their overrides. Party randomizers keep their
precedence but cannot reroll participants: the trainer species randomizer may
bypass authored parties as it does today but still uses the saved people and
order. XP uses actual species and levels. Prize money uses the trainer's
inventoried source reward basis and class, not the old room occupant, and team
size must not shift it.

## Load validation

Validate the schema, lifetime wins, pending transactions, and league state
before any dispatch or event processing. No attempt, no active run, and an
empty most recent event lineup are normal; validation never generates a
lineup, processes an event, or reads the day counter.

- Each league's last processed event day is none or one of its event days; a
  reigning champion is none, the player, or a known character; an attempt
  names one of the league's event days after its last processed event day,
  and a won attempt has the player as reigning champion.
- The most recent event lineup is empty or holds five distinct known
  characters.
- An active run names a league whose attempt is on the run's event day and
  not won, holds five distinct eligible characters in non-decreasing TR
  order, and resolves every reference. Stored teams must be valid for their
  roster (known roster slots, legal forms, levels, and moves); they are never
  recomposed on load.
- A day counter behind the saved event days is valid (a clock change): no
  event is held until it passes them.

If the saved content versions differ from the build's, drop the active run
and clear its attempt, so the player may enter that event afresh; clear the
most recent event lineup, so the next selection has no fatigue; and clear a
reigning champion who is no longer an eligible character. This is the
prerelease policy, not an invalid save.

A valid active run with damaged run progress recovers to its own lobby with
progress reset and the event lineup kept, so the attempt restarts at match 1.
A missing, corrupt, or unsupported run, lineup, or league state follows
standard invalid-save handling: never regenerate a lineup on load or
synthesize results.

## Presentation

Each lobby names the league's reigning champion (or none yet) and whether its
event is today or in how many days. Before the player enters, lineups are
unavailable and inspection generates nothing. After entering, show the five
names and battle order; moves and items are hidden by default. After a loss,
staff say the event is over for the player and when the next one is.

Graphics, portraits, dialogue, battle metadata, and names follow the selected
character even in historical rooms; a displaced fixed resident must not remain
in dialogue. The last opponent is this lineup's finalist, whatever their title.
Dialogue cannot assume Blue occupies Indigo or Lance ends Masters. Masters
never calls its winner a regional Champion.

## Records, ceremonies, and integration

### First and repeat wins

The player's first-ever win at each league keeps the one-time effects as
Today's [interregional circuit](wayfarer-interregional-league-circuit.md)
implements them: Indigo's shared Kanto/Johto Champion recognition with one
Hall of Fame registration, the Masters Gallery without regional awards,
Hoenn's own recognition, Hall of Fame, regional cleanup, and full completion
credits, Red's unlock once all three leagues have a lifetime win, and Blue's
Saffron Dojo battle after the first Indigo win. A repeat win gives the prize
money of its battles and the reigning-champion title and nothing else: the
one-time effects never repeat, and whether a repeat win shows a short
ceremony follows Today's replay presentation until designed. Wins grant no
TR. Ceremony handling stays idempotent across callbacks, reloads, and
interrupted saves, and must finish before the player enters again.

### Story dependencies and travel

Red stays outside the pool. His Mt. Silver admission reads all three saved
lifetime wins; his encounter and rewards are unchanged. Blue's Saffron Dojo
battle keeps its current unlock on the first committed Indigo win, whether or
not Blue was in the lineup.

Preserve current travel: the S.S. Aqua maiden voyage and Ticket,
Olivine–Vermilion–Slateport service, and numbered Sevii service from Vermilion
stay independent of badges, league wins, Rainbow Pass, Bill/Celio, National
Pokédex, and Sevii quests. With Hoenn opening at 8 global badges, audit that
every league is reachable at its entry threshold from any region's badges.
Audit back warps, exits, healing and blackout targets, Dig, Escape Rope, and
ceremony returns; a blackout after a league loss must keep the player's
attempt, and Masters never routes to the HNS Indigo lobby.

Audit checks that equate a room with a fixed opponent, read the old
fixed-lineup run record, or assume the fixed Indigo → Masters → Hoenn order,
before changing them. Preserve Giovanni's Earth Badge
and local finale, optional quests, deferred rewards, voyage state, initial Gym
access, and other regions' unfinished stories.

### Integration surfaces

Existing code to review, not new APIs:

- [Save ownership and initialization](../../game/src/wayfarer_persistence.c)
  and [run/save structures](../../game/include/global.h).
- [The day counter's daily update](../../game/src/clock.c).
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

## Balance report

League balance is informational in v0; tuning comes later. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) simulates the
calendar over a chosen number of in-game days at a chosen world progress and
badge count: for each day, the league holding an event, its lineup (fatigued
by the event the day before), whether the player may enter, and the reigning
champion, the player where marked as entering and winning, otherwise the
lineup's strongest. For a selected event it reports every eligible trainer's
TR, team level, willingness, league score, and rank, the base lineup level,
each aloof trainer's check (team level against base lineup level + 10, joins
or skips), and the resulting lineup. It asserts no fixed
lineup, finalist, or strength target. The report is informational; it also
confirms that no aloof trainer in a lineup is more than 10 levels above the
base lineup level. The Gym ladder and team targets
stay in [Notable trainers](notable-trainers.md#balance-targets).

## Acceptance

Check in an inventory: aliases, roster references, presentation coverage,
exclusion reasons, provenance, and content versions. Required implementation
evidence (not yet run):

1. **Registry.** Reject duplicate characters or aliases, unresolved source
   IDs, missing assets, double-battle flags, and trainers without valid
   growth values or a valid roster. The build must hold at least five eligible
   trainers.
2. **Selection.** Fixtures at several world progress values, including one
   where growth reorders the lineup: willingness for at-home, away
   non-traveller, away traveller, fatigued, and floored cases at each league,
   Masters as a neutral location; the league score floored; the base lineup
   level taken from the top five non-aloof trainers only; an aloof trainer
   joining at exactly base lineup level + 10 and skipping at one level more,
   never judged against another aloof trainer, and skipping when there is no
   base lineup; fatigue applied to the base lineup and to an aloof trainer's
   score; the five highest scores over
   distinct scores and ties at the fifth-place boundary broken by ascending
   `characterId`; fatigue read from the most recent completed event at any
   league, whether or not the player entered it; the battle
   order non-decreasing in TR with the highest last; excluded and disabled
   trainers never appear; aliases never appear twice.
3. **Calendar.** Each league holds events exactly on its days (Indigo day mod
   3 = 0, Hoenn 1, Masters 2), one league a day; strength is identical on
   every day at one world progress; a clock turned back reopens and reruns
   nothing; a counter jump of many days processes each league's attempted
   event and latest ended event, oldest first, and nothing older.
4. **Entry.** Indigo and Hoenn refuse below 8 badges and open at 8 in either
   order; the Masters refuses below 16 badges and, at 16 or more, until a
   lifetime win at Indigo or Hoenn (a Masters win never counts); a second
   entry to the same event, entry with no current event, and entry with a
   pending ceremony are refused and change nothing.
5. **Determinism.** Golden lineups for fixed world progress values, leagues,
   and most recent event lineups, matched between host tooling and game C;
   the same inputs always give the same five, and the Pokémon RNG state is
   unchanged.
6. **Fresh state.** A new game has no reigning champion, attempt, active run,
   or most recent event lineup, and processes no event from before it; load,
   display, and denied or cancelled attempts to enter generate nothing.
7. **Entering.** Entering saves the selected five in ascending TR order with
   the attempt and run atomically; reloading before the commit selects the
   same five; inject failures before, during, and at commit.
8. **Loss.** Lose at each match, and leave voluntarily: the run ends, nothing
   is recorded, the player blacks out to the usual target, and re-entering the
   event is refused, also after a reload. At the event's end the lineup's
   strongest becomes reigning champion and that lineup becomes the most
   recent event lineup. The next event of that league selects a new lineup.
9. **Skipping and reigning champions.** Skip events at each league: each
   event's end makes its lineup's strongest the reigning champion until that
   league's next event and fatigues the next event's lineup; nothing else
   changes.
10. **Win.** A first win records exactly one lifetime win and its one-time
    effects and ceremony; a repeat win gives only prize money and the title;
    the player reigns until the league's next event ends; no win changes
    player TR; the run is released. Interrupt and repeat win commits and
    event processing.
11. **Construction and presentation.** Build every eligible trainer's team,
    preserve member identity and metadata, and reconstruct identically from
    the saved event lineup. Names, sprites, portraits, text, music, AI, money,
    XP, and parties match the lineup's matches, including a Gym Leader in
    match 5. Lobbies name the reigning champion and the next event.
12. **Load validation.** Corrupt league state, attempts, runs, event
    lineups, most recent event lineups, schema, or callbacks are rejected
    without regenerating, advancing, or rewarding. A content version change
    drops the run and attempt and clears the most recent event lineup.
13. **Standalone.** FRLG, HNS, and Emerald League behavior, travel, and
    recovery are unchanged.

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

- A per-save calendar rotation from the playthrough seed (e.g. Hoenn, Masters,
  Indigo), shifting who tends to be rested where. v0 uses the fixed rotation.
- Off-screen event results as signposting: NPC gossip, TV, and Match Call
  news about who won and who reigns.
- Event rewards tuning, cadence tuning, entry threshold tuning, and special
  events.
- Balancing tools: tune travel costs, fatigue, the floor, and the aloof margin
  against the lineup reports, and set league balance targets.
- Seeded lineups or calendars, as candidate consumers of the
  [playthrough seed framework](playthrough-seed-framework.md).
- Rotation weights and rotation history.
- Role windows and standing-based matches, with a nearest-standing fallback for
  empty windows.
- A Trainer Card itinerary view for the three leagues.
- Winning-team records per edition and edition completion presentation.

## References

- [Leagues PRD](../prds/leagues.md)
- [Notable trainers](notable-trainers.md)
- [Trainer AI](trainer-ai.md)
- [Player Trainer Rating](player-trainer-rating.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Existing League scaling contract](league-scaling.md)
- [Party construction](../../game/src/battle_main.c)
