# Leagues

PRD: [Leagues](../prds/leagues.md)
Implemented: No
Design status: v0 approved: leagues as locations, ten contenders (the
strongest eligible trainers by Trainer Rating (TR) when the player enters), a
seeded lineup draw of five weighted by willingness (travel cost and fatigue),
ascending battle order, a lineup drawn on first entry and locked until the
league is won, and a win that commits the league result. Balance is
informational for now. Signup and league order stay as Today's
[interregional circuit](wayfarer-interregional-league-circuit.md) until
designed.

## Scope

Own, for each `IS_WAYFARER` league: the league registry, eligibility,
location regions, contenders, fatigue, the lineup draw and battle order, the
locked lineup, battle construction, entering a league, active runs and dispatch, the win commit,
saved state, load validation, presentation, and regional integration.

- [Notable trainers](notable-trainers.md) owns notable trainers, their
  TR and its growth with world progress, home regions, travel styles, travel
  cost and willingness, the team-level and team-size scalers, rosters, and
  team composition. This spec reads a trainer's TR, willingness, and composed
  team; it never restates how they are computed.
- The [playthrough seed framework](playthrough-seed-framework.md) owns the
  root seed, keyed derivation, and the weighted-choice helper. The lineup draw
  is its only v0 consumer.
- [Player Trainer Rating](player-trainer-rating.md) owns the player's TR. In
  v0 a league win adds no player TR; Today's +8 per first league win stays
  documented in the circuit spec until adoption.
- Today's [interregional circuit spec](wayfarer-interregional-league-circuit.md)
  keeps admission, league order, replays, first-league-win facts, and
  ceremonies. This spec changes only who is in the lineup and what happens
  after a loss.

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

## Selection and order

Every eligible trainer can be invited to any league; location regions only
weight the draw. The first time the player enters a league, and on each replay
(see [Entering a league](#entering-a-league)):

1. **Contenders.** Compute each eligible trainer's TR at the current world
   progress ([Notable trainers](notable-trainers.md#growth-with-world-progress))
   and take the ten highest (all of them if fewer than ten are eligible). Equal
   TRs at the boundary break by ascending `characterId`.
2. **Willingness.** Score each contender with the
   [travel rule](notable-trainers.md#home-region-and-travel):
   `max(5, 100 - travelCost - fatigue)`. **Fatigue** is league-specific: 50 if
   the trainer is in the previous lineup, else 0. At home a contender scores
   100 (50 fatigued); away, a traveller 90 (40) and a homebody 20 (5).
3. **Lineup draw.** Draw five without replacement: five weighted choices by
   willingness over the contenders in ascending `characterId` order, each
   removing its pick ([seeded key](#seeded-key)).
4. **Battle order.** Order the five by ascending TR, so the strongest fights
   last. Equal TRs take whatever order the sort produces.

The **previous lineup** is the lineup of the last league the player entered
before this one, by entry sequence (re-entries of a locked lineup and replays
count), not league order. Every entry saves its lineup as the new previous
lineup, after this entry's draw has read the old one.

Title, party, and history play no part; player TR enters only as world
progress, through each trainer's TR. Other leagues are never resolved early.

### Seeded key

The lineup draw is one keyed decision of the
[playthrough seed framework](playthrough-seed-framework.md#decision-identities):

| Key field | Value |
| --- | --- |
| `domainId` / `decisionId` | Leagues (1) / lineup draw (1) |
| `decisionVersion` | 1; bump when contender, willingness, or draw rules change |
| `entityLo` | League identity (1-3); `entityHi` 0 |
| `occurrenceId` | That league's saved draw counter: 0 for its first draw |
| `drawId` | Pick 1-5 |

Inputs become authoritative at the moment of entry: world progress, the
previous lineup, and content. The result is committed before reveal, together
with the counter advance, so reloading before the commit re-derives the same
five and nothing after it re-rolls. The draw never touches the Pokémon RNG.

## Locked lineup

The drawn five are the locked lineup. The draw captures, for each of the five
in battle order:
`characterId`, their TR, and their composed team, plus the registry and roster
content versions. Per member, the team holds the roster slot index and every
resolved battle value the battle snapshot uses: species/form, level, moves,
item, ability, nature, IVs/EVs, and battle order. It is saved atomically
before reveal.

The lineup stays locked until the league is won; a loss or leaving keeps it.
Every retry at that league, including after reload, reconstructs battles from
the saved lineup and never reselects or recomposes, even after the player's TR
rises. If the content version changes while a lineup is locked,
the lock is dropped and the next time the player enters, a new lineup is
drawn with the league's next occurrence (prerelease policy; the save stays
valid).

## Saved state

Keep Today's saved circuit facts (league wins, pending ceremony phase). Add:

- at most one **locked lineup**: the league (`venueId`), the content versions,
  the occurrence it was drawn with, and the five drawn matches;
- a **draw counter** per league (`u16`, saturating): the occurrence its next
  draw uses;
- the **previous lineup**: the five `characterId`s of the last league entered,
  with its content versions, read for fatigue; empty on a new game; and
- active-run progress: the league and the defeated prefix of the five matches.

The root seed lives in the framework's shared record. There is no edition,
rotation history, or lineup history beyond the previous lineup. Only one lineup
can be locked at a time because admission already requires the previous league
to be won. Save an explicit schema discriminator for this layout; prerelease
saves need no migration.

## Lifecycle

### Entering a league

Admission follows Today's circuit rules. At a league that is not yet won:

1. If a locked lineup exists for this league, load it and save it as the
   previous lineup. Otherwise validate the absence of any other locked lineup
   or pending transaction, draw the five
   ([Selection and order](#selection-and-order)), and atomically save the
   locked lineup, the advanced draw counter, and the new previous lineup.
2. Start a fresh run and reveal the lineup.

A failure leaves the prior state intact; a crash exposes either the old state
or the complete locked lineup. Denied or cancelled requests change nothing and
reveal nothing.

A replay of a won league draws a fresh lineup with the league's next
occurrence. The drawn five, the counter advance, and the new previous lineup
commit atomically with the existing replay run record before reveal; a replay
never locks the league, and the next replay draws the following occurrence.
Whether a replay is available while another league is locked follows Today's
circuit rules.

### Loss

A loss blacks the player out as usual. A blackout or voluntary exit ends the
run, not the lineup: run progress resets and the locked lineup stays unchanged.
The next attempt starts at match 1 against the same five with the same teams and
levels. Record no result or reward. The player may retry at once or leave, earn
badges, and return any number of times; nothing earned meanwhile touches the
locked lineup. Save and reload preserve it. In v0 only a content version change
can alter a team (see [Load validation](#load-validation)).

### Win

After five victories, one transaction atomically:

- records the league win and, if it is the first league win, Today's
  first-league-win effects other than player TR, which a win never changes;
- queues Today's ceremony; and
- releases the locked lineup and run.

Stale or duplicate callbacks are rejected. Individual victories, losses, and
Red add no TR.

## Active run and dispatch

Progress belongs to the locked lineup's matches (or, in a replay, the
replay run's drawn lineup's); global Trainer defeat flags cannot skip a match.

1. Validate the locked lineup, admission, run, and destination before locking
   an entrance or changing room state.
2. Each battle validates league, room, and expected match. The stored team and
   versioned references supply party, class, sprite, portrait, name,
   introduction, defeat text, music, and AI. Fixed room-owner IDs never pick
   opponents.
3. A victory advances one match exactly once. A loss follows Loss.
4. Room and ceremony transitions keep the league identity until the win
   commits.

Live badges, league wins, player TR, party, and XP never mutate a locked
lineup. Debug and other battles cannot create runs, advance matches, grant
league wins, or create league records.

## Battle construction

Each match fights with the trainer's own team at their own TR, as
[Notable trainers](notable-trainers.md) composes it for any battle: there is
no league-specific level offset, role adjustment, or six-member competitive
profile. The old `[-4,-3,-2,-1,+1]` room offsets are removed.

Construct from the saved lineup. Resolve the selected trainer and roster owner
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

Validate the schema, saved league wins, and pending transactions before any
dispatch. No locked lineup is normal; never generate content on load.

A locked lineup must name a league that is not yet won and is the current
admissible league, hold five distinct eligible characters in non-decreasing TR
order, and resolve every reference. Stored teams must be valid for their
roster (known roster slots, legal forms, levels, and moves); they are never
recomposed on load. If the saved content versions differ from the build's,
drop the lock and run progress; the next time the player enters, a new lineup
is drawn with the next occurrence. A previous lineup from other content
versions is cleared, so the next draw has no fatigue. This is the prerelease
policy, not an invalid save.

The previous lineup is empty or holds five distinct known characters, and a
locked lineup's occurrence is below its league's draw counter. The
playthrough root must be valid
([seed framework](playthrough-seed-framework.md#root-seed-lifecycle)); a
missing root is an invalid save, never a reroll.

A valid locked lineup with damaged run progress recovers to its own lobby with
progress reset and the lineup kept. A missing, corrupt, or unsupported locked
lineup follows standard invalid-save handling: never regenerate a lineup on
load or synthesize results.

## Presentation

Before the player enters, lineups are unavailable and inspection generates
nothing. After entering, show the five names and battle order; moves and items
are hidden by default. After a loss, staff invite the player to retry the same
lineup whenever ready.

Graphics, portraits, dialogue, battle metadata, and names follow the selected
character even in historical rooms; a displaced fixed resident must not remain
in dialogue. The last opponent is this lineup's finalist, whatever their title.
Dialogue cannot assume Blue occupies Indigo or Lance ends Masters. Masters
never calls its winner a regional Champion.

## Records, ceremonies, and integration

### Ceremonies and records

Ceremonies, records, and unlocks stay as Today's
[interregional circuit](wayfarer-interregional-league-circuit.md) implements
them: Indigo's shared Kanto/Johto recognition, Masters Gallery without regional
awards, Hoenn's own recognition, cleanup, and credits, and replays granting
nothing further. Ceremony handling stays idempotent across callbacks, reloads,
and interrupted saves, and must finish before the player enters again.

### Story dependencies and travel

Red stays outside the pool. His Mt. Silver admission reads all three saved
league wins; his encounter and rewards are unchanged. Blue's Saffron Dojo
battle keeps its current unlock on the first committed Indigo win, whether or
not Blue was in the lineup.

Preserve current travel: the S.S. Aqua maiden voyage and Ticket,
Olivine–Vermilion–Slateport service, and numbered Sevii service from Vermilion
stay independent of badges, league wins, Rainbow Pass, Bill/Celio, National
Pokédex, and Sevii quests. Audit back warps, exits, healing and blackout
targets, Dig, Escape Rope, and ceremony returns; a blackout after a league loss
must not clear the locked lineup, and Masters never routes to the HNS Indigo
lobby.

Audit checks that equate a room with a fixed opponent, or read the old
fixed-lineup run record, before changing them. Preserve Giovanni's Earth Badge
and local finale, optional quests, deferred rewards, voyage state, initial Gym
access, and other regions' unfinished stories.

### Integration surfaces

Existing code to review, not new APIs:

- [Save ownership and initialization](../../game/src/wayfarer_persistence.c)
  and [run/save structures](../../game/include/global.h).
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
[explorer](../../devtools/ui/README.md#trainer-balance-explorer) reports, for
each league at sample world progress values and entry sequences, every
contender's appearance odds and the likely lineups. It asserts no fixed
lineup, finalist, or strength target. The Gym ladder and team targets
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
   where growth reorders the contenders: the ten highest over distinct TRs and
   ties at the tenth-place boundary; willingness for at-home, away homebody,
   away traveller, fatigued, and floored cases at each league, Masters as a
   neutral location; fatigue read from the last league entered by entry
   sequence, re-entries and replays included, not league order; five distinct
   picks from the contenders; the battle order non-decreasing in TR with the
   highest last; excluded and disabled trainers never appear; aliases never
   appear twice.
3. **Draw.** Golden lineups for fixed roots, leagues, and occurrences, matched
   between host tooling and game C; the same key and inputs give the same five,
   a different root or occurrence can differ, and the Pokémon RNG state is
   unchanged. Over many roots, observed pick rates follow willingness.
4. **Fresh state.** A new game has a root seed, zero draw counters, no
   previous lineup, and no locked lineup; load, display, and denied or
   cancelled attempts to enter generate nothing.
5. **Entering.** Entering a league the first time saves the drawn five in
   ascending TR order, the counter advance, and the previous lineup
   atomically; reloading before the commit re-derives the same five; inject
   failures before, during, and at commit.
6. **Loss.** Lose at each match: the locked lineup is unchanged, progress
   resets, nothing is recorded, and the player blacks out to the usual target.
   Earn a badge and level up, then retry: identical five trainers, teams, and
   levels, although every trainer's current TR has risen. Repeat with reloads
   and voluntary exits.
7. **Win.** Exactly one league win and ceremony; only the first league win
   grants first-league-win effects; no win changes player TR; the locked
   lineup is released. Interrupt and repeat ceremony commits.
8. **Replay.** Each replay of a won league draws with the next occurrence,
   commits it with the replay run record, updates the previous lineup, and
   never locks the league.
9. **Construction and presentation.** Build every eligible trainer's team,
   preserve member identity and metadata, and reconstruct identically from the
   saved lineup. Names, sprites, portraits, text, music, AI, money, XP, and
   parties match the lineup's matches, including a Gym Leader in match 5.
10. **Load validation.** Corrupt locked lineups, previous lineups, counters,
    roots, schema, or callbacks are rejected without regenerating, advancing,
    or rewarding. A content version change drops the lock, and the next time
    the player enters, a new lineup is drawn with the next occurrence.
11. **Standalone.** FRLG, HNS, and Emerald League behavior, travel, and
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

- Signup thresholds and qualification designed for these leagues.
- Balancing tools: tune the contender count, travel costs, fatigue, and the
  floor against appearance-odds reports, and set league balance targets.
- Seeded league order.
- Recurring editions with rollover, rotation weights, and rotation history.
- Role windows and standing-based matches, with a nearest-standing fallback for
  empty windows.
- A Trainer Card itinerary view for the three leagues.
- Winning-team records per edition and edition completion presentation.

## References

- [Leagues PRD](../prds/leagues.md)
- [Notable trainers](notable-trainers.md)
- [Player Trainer Rating](player-trainer-rating.md)
- [Playthrough seed framework](playthrough-seed-framework.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Existing League scaling contract](league-scaling.md)
- [Party construction](../../game/src/battle_main.c)
