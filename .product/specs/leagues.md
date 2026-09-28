# Leagues

PRD: [Leagues](../prds/leagues.md)
Implemented: No
Design status: v0 approved: leagues as locations whose **league events**
reach the player as **invitations** by phone. From player TR 80 a league
calls every 7 in-game days (restarted when each invitation resolves, stopped
while an accepted event waits); only a league that knows the player calls
(Indigo with a Kanto or Johto badge, Hoenn with a Hoenn badge, the Sevii
Masters after any league win), round-robin by a call counter: the eligible
league that called least recently, ties to the most badges, then Indigo.
Accepting freezes a lineup that waits for the player, with one attempt;
declining runs the event without them. A league score per eligible trainer (Trainer Rating (TR) scaled by
willingness, from travel cost and fatigue), aloof trainers joining only a
base lineup near their level, the top five by league score with no
randomness, ascending battle order, a **reigning champion** per league, and
first-win one-time effects. Balance is informational for now. Today's
[interregional circuit](wayfarer-interregional-league-circuit.md) stays the
record of Today's admission, fixed league order, and replays.

## Scope

Own, for each `IS_WAYFARER` league: the league registry, eligibility,
location regions, invitations (the qualification gate, the countdown, which
league calls, accepting and declining), fatigue, the league score, the base
lineup and its level, the aloof rule, lineup selection and battle order, the
event lineup, the reigning champion, battle construction, entering an
accepted event, active runs and dispatch, the win commit, first and repeat
wins, saved state, load validation, presentation, and regional integration.

- [Notable trainers](notable-trainers.md) owns notable trainers, their
  TR and its growth with world progress, home regions, the traits (traveller
  and aloof), travel cost and willingness, the team-level scaler and the
  team-size step scaler, rosters, and team composition. This spec reads a
  trainer's TR, team level, willingness, aloof trait, and composed team; it
  never restates how they are computed.
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

## Invitations

The player takes part in league events only by invitation. An **invitation**
is a league's phone call inviting the player to its next event; the player
**accepts** or **declines** it. At most one invitation or accepted event
exists at a time.

These rules stand with the round-robin choice of caller: the player must
answer a call, with no "later" ([Answering](#answering)); leaving an accepted
event midway counts as a loss ([Loss](#loss)); after a loss the reigning
champion is the strongest of the frozen lineup; and a content version change
turns an accepted event back into an unanswered invitation from the same
league ([Load validation](#load-validation)).

Days come from the game's existing in-game day counter (`VAR_DAYS`, which
the daily update in [clock.c](../../game/src/clock.c) keeps); leagues add no
clock of their own. **Days never affect anyone's strength**: every TR stays a
function of world progress, and days only schedule invitations. The order of
calls never reads days: it comes from a monotonic
[call counter](#which-league-calls), so a day counter that goes backwards
cannot reorder the leagues.

### Qualification

The player **qualifies** once their TR is at least **80** (a placeholder;
today exactly 8 badges, and future TR sources may lower the badges needed).
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
day counter's value when the countdown last looked). On each check (the daily
update, and after a load):

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
- the **Sevii Masters** is eligible after any lifetime league win.

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
   league with the **most badges**; the Masters, with no badge count, loses
   a badge tie to a regional league.
3. A remaining tie (Indigo and Hoenn with equal badges) goes to **Indigo**.

So the very first call comes from the eligible league with the most badges,
and each eligible league then calls in turn. When only one league is
eligible, it calls every time, repeating as often as needed; a league that
becomes eligible later (a first badge in its regions, or the Masters after a
first win) has never called, so it calls next. Badges from any region count
toward the gate; which regions they come from decides who may call and breaks
ties.

**No eligible league.** If the player qualifies while no league knows them
(possible only with a future TR source that needs no badge), no call is made:
the countdown stays due, and each daily update checks again, until a league
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
base lineup is well below their level. When the player accepts or declines an
invitation:

1. **TR.** Compute each eligible trainer's TR at the current world progress
   ([Notable trainers](notable-trainers.md#growth-with-world-progress)).
2. **Willingness.** Score each eligible trainer's willingness for this league
   with the [travel rule](notable-trainers.md#home-region-and-travel), which
   reads the league's location region and this fatigue. **Fatigue** is
   league-specific: 50 if the trainer is in the most recent resolved lineup,
   else 0.
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

The **most recent resolved lineup** is the lineup of the most recent resolved
event at any league: an accepted event that ended (won, lost, or left), or a
declined one. Each resolution saves its lineup as the new most recent
resolved lineup, after its own selection has read the old one. Accepting reads
it but never changes it, because the event it reads is still the last one
resolved.

Selection consumes no randomness and reads no seed: the same world progress,
most recent resolved lineup, and content always give the same five. The day,
title, reigning champion, party, badges, and history play no part; player TR
enters only as world progress, through each trainer's TR. When the player
answers, the inputs become authoritative at that moment, and the result is
committed before reveal, so nothing after it reselects.

## Event lineup

The five selected when the player accepts are the event's lineup, frozen for
that event however long it waits. Accepting captures, for each of the five in
battle order: `characterId`, their TR, and their composed team, plus the
registry and roster content versions. Per member, the team holds the roster
slot index and every resolved battle value the battle snapshot uses:
species/form, level, moves, item, ability, nature, IVs/EVs, and battle order.
It is saved atomically with the accepted event before reveal, and every
battle, including after a reload, is reconstructed from it; it never
reselects or recomposes, even after the player's TR rises. The event's end
releases it, keeping only the five `characterId`s as the most recent
resolved lineup.

A declined event's lineup is only computed for its result: it saves the five
`characterId`s and composes no teams.

## Reigning champion

Each league has at most one **reigning champion**, who holds the title from
the resolution of one of its events until the resolution of its next:

- If the player wins the event, the player is the reigning champion, from the
  win.
- If the player loses or leaves an accepted event, the reigning champion is
  its frozen lineup's strongest member (the last in battle order).
- If the player declines, the reigning champion is the strongest member of the
  lineup computed at decline.

A new game has no reigning champion; each league gains one when its first
event resolves. The title is recognition only: it does not change selection,
battles, or rewards.

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
- the **most recent resolved lineup**: the five `characterId`s of the most
  recent resolved event, with its content versions, read for fatigue; empty
  on a new game;
- per league, the **reigning champion** (none, the player, or a
  `characterId`); and
- the **active run**, only while the player is fighting their accepted event:
  the defeated prefix.

There is no seed, rotation history, or lineup history beyond the most recent
resolved lineup, and no stored day other than the countdown's last counted
day. At most one accepted event and one active run exist. New Game saves the
not-qualified state, a call counter of 0, and no last call number, reigning
champion, or most recent resolved lineup. Save an explicit schema
discriminator for this layout; prerelease saves need no migration.

## Lifecycle

### Qualifying and the call

After any player TR change, and on the daily update, a not-qualified player
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

- **Accept.** Select the five ([Selection and order](#selection-and-order))
  and atomically save the accepted event with its event lineup, then reveal
  the league and the five names.
- **Decline.** Select the five and, in one transaction, set the league's
  reigning champion to their strongest, save them as the most recent resolved
  lineup, and restart the countdown (7 days remaining from today).

A failure leaves the invited state intact, so the call rings again; a crash
exposes either the invited state or the complete result. Answering creates no
battle, reward, or record.

### Entering the accepted event

1. Validate admission: the player has an accepted event at this league, and
   no pending ceremony, active run, or transaction. A refusal states only the
   immediate reason (no invitation yet and when the next call is due, an
   unanswered call, or an event accepted at another league) and changes
   nothing.
2. Save the active run and start at match 1, with the five and their teams
   from the event lineup.

### Loss

A loss blacks the player out as usual. A blackout or voluntary exit ends the
event, in one transaction: the frozen lineup's strongest becomes the league's
reigning champion, its five become the most recent resolved lineup, the run
and the accepted event are released, the countdown restarts (7 days
remaining from today), and nothing is recorded or rewarded.

### Win

After five victories, one transaction atomically:

- makes the player the league's reigning champion;
- if this is the player's first-ever win at this league, records the lifetime
  win and Today's first-league-win effects other than player TR, which a win
  never changes, and queues Today's ceremony; a repeat win instead gives its
  [repeat-win reward](#first-and-repeat-wins);
- saves the five as the most recent resolved lineup, releases the run and the
  accepted event, and restarts the countdown (7 days remaining from today).

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

Live badges, league wins, player TR, party, XP, and the day counter never
mutate an event lineup. Debug and other battles cannot create invitations,
accepted events, or runs, advance matches, grant league wins or titles, or
create league records.

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
before any dispatch, call, or answer. Not qualified, no accepted event, no
active run, and an empty most recent resolved lineup are normal; validation
never generates a lineup or places a call.

- The invitation state is exactly one of its four kinds. Counting down holds
  days remaining (0 to 7) and a last counted day; invited and accepted name a
  league whose last call number equals the call counter (the league that
  called most recently; the Masters only with a lifetime win); badges are not
  rechecked, since badges never decrease.
- A stored day is never a reason to reject a save. A last counted day ahead
  of the day counter (a clock turned back) is clamped to the day counter,
  which neither advances nor resets the countdown.
- Each league's last call number is none or a number from 1 to the call
  counter; no two leagues share a number, and when the counter is above 0 one
  league holds it. All are none only when the counter is 0, which means the
  player has never been called (not qualified, or counting down to the first
  call). The Masters has a number only with a lifetime win.
- An accepted event's lineup holds five distinct eligible characters in
  non-decreasing TR order and resolves every reference. Stored teams must be
  valid for their roster (known roster slots, legal forms, levels, and
  moves); they are never recomposed on load.
- The most recent resolved lineup is empty or holds five distinct known
  characters; a reigning champion is none, the player, or a known character.
- An active run exists only with an accepted event and names a match within
  its lineup.

If the saved content versions differ from the build's, turn an accepted event
back into an unanswered invitation from the same league (dropping its lineup
and any run), so the call rings again; clear the most recent resolved lineup,
so the next selection has no fatigue; and clear a reigning champion who is no
longer an eligible character. This is the prerelease policy, not an invalid
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
Accepting names the five and their battle order (moves and items hidden by
default) and says the event waits for the player; declining says the event
goes ahead without them and names its reigning champion. A win or a loss is
followed by the league's call or lobby word on the result and the title.
Before an answer, lineups are unavailable and inspection generates nothing.

Each lobby names the league's reigning champion (or none yet). Without an
accepted event there, staff turn the player away with the immediate reason
([Entering the accepted event](#entering-the-accepted-event)).

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
interrupted saves, and must finish before the next call rings.

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
- [The day counter's daily update](../../game/src/clock.c) and
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

## Balance report

League balance is informational in v0; tuning comes later. The
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) simulates a
chosen number of invitations at a chosen player TR (world progress) and badge
split (Kanto, Johto, Hoenn), with the player's answer to each (accept and
win, accept and lose, or decline); accepted events resolve the day they are
accepted. For each invitation it reports the day it arrives, which league
calls and why (the only eligible league, the least recently called, or a
tie going to the most badges or to Indigo), the frozen
lineup with league scores, the event whose lineup it fatigues, the result, and
the reigning champion. For a selected event it reports every eligible
trainer's TR, team level, willingness, league score, and rank, the base lineup
level, each aloof trainer's check (team level against base lineup level + 10,
joins or skips), and the resulting lineup. It asserts no fixed lineup,
finalist, or strength target. The report is informational; it also confirms
that no aloof trainer in a lineup is more than 10 levels above the base lineup
level. The Gym ladder and team targets stay in
[Notable trainers](notable-trainers.md#balance-targets).

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
   `characterId`; fatigue read from the most recent resolved event at any
   league, accepted or declined; the battle
   order non-decreasing in TR with the highest last; excluded and disabled
   trainers never appear; aliases never appear twice.
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
5. **Which league calls.** Known-there eligibility: with no badge in a
   league's regions that league never calls (Indigo counting Kanto and
   Johto), and the Masters never calls before a lifetime league win. Among the
   eligible leagues the least recently called calls, a never-called league
   first: with three eligible leagues they call in a fixed rotation. Ties go
   to the most badges, the Masters losing to a regional league, then to
   Indigo; the very first call comes from the most-badges eligible league. A
   single eligible league calls repeatedly; a league that becomes eligible
   later calls next. With no eligible league, a due call waits and is
   checked again each day until a league knows the player. The call counter
   only increases, and a day counter turned back never changes the order.
   The same badges, wins, and last call numbers always give the same caller.
6. **Accept and decline.** Accepting saves the selected five in ascending TR
   order with the accepted event atomically, and the event waits across many
   days and reloads with the same five; reloading before the commit selects
   the same five; inject failures before, during, and at commit. Declining
   crowns the strongest of the lineup computed at decline, saves it as the
   most recent resolved lineup, and restarts the countdown. Entering a league
   without an accepted event there is refused and changes nothing.
7. **Determinism.** Golden lineups and call sequences for fixed world
   progress values, badge splits, answers, and most recent resolved lineups,
   matched between host tooling and game C; the same inputs always give the
   same five and the same caller, and the Pokémon RNG state is unchanged.
8. **Fresh state.** A new game is not qualified, has a call counter of 0, and
   has no last call number, reigning champion, accepted event, active run, or
   most recent resolved lineup; load, display, and denied or cancelled requests generate nothing.
9. **Loss.** Lose at each match, and leave voluntarily: the event ends once,
   nothing is recorded, the player blacks out to the usual target, the frozen
   lineup's strongest reigns, that lineup becomes the most recent resolved
   lineup, and the countdown restarts, also across a reload.
10. **Win.** A first win records exactly one lifetime win and its one-time
    effects and ceremony, and makes the Masters eligible to call; a repeat win
    gives only prize money and the title; the player reigns until that
    league's next resolved event; no win changes player TR; the run and the
    accepted event are released. Interrupt and repeat win commits, calls, and
    answers.
11. **Construction and presentation.** Build every eligible trainer's team,
    preserve member identity and metadata, and reconstruct identically from
    the saved event lineup. Names, sprites, portraits, text, music, AI, money,
    XP, and parties match the lineup's matches, including a Gym Leader in
    match 5. The phone rings only when the player can take a call, and rings
    again after a reload until answered; lobbies name the reigning champion.
12. **Load validation.** Corrupt invitation state, call numbers, accepted events,
    runs, lineups, most recent resolved lineups, schema, or callbacks are
    rejected without regenerating, calling, advancing, or rewarding. A
    content version change turns an accepted event back into an unanswered
    invitation and clears the most recent resolved lineup.
13. **Standalone.** FRLG, HNS, and Emerald League behavior, travel, recovery,
    and phone calls are unchanged.

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
- Seeded lineups, varying which willing trainers come per save.
- Rotation weights and rotation history.
- Role windows and standing-based matches, with a nearest-standing fallback for
  empty windows.
- A Trainer Card view of the three leagues and their reigning champions.
- Winning-team records per league event, and presentation of a player's
  record across a run of events.

## References

- [Leagues PRD](../prds/leagues.md)
- [Notable trainers](notable-trainers.md)
- [Trainer AI](trainer-ai.md)
- [Player Trainer Rating](player-trainer-rating.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Existing League scaling contract](league-scaling.md)
- [Party construction](../../game/src/battle_main.c)
