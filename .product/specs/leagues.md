# Leagues

PRD: [Leagues](../prds/leagues.md)
Implemented: No
Design status: v0 approved: one global pool, top five by TR, ascending battle
order, a field captured at entry and locked until the venue is won, and a win
that commits the venue. Signup and venue order stay as the current
[interregional circuit](wayfarer-interregional-league-circuit.md) until
designed.

## Scope

Own, for each `IS_WAYFARER` league venue: the circuit registry, eligibility,
field selection and battle order, the frozen field, battle construction, entry
and the field lock, active runs and dispatch, the win commit, saved state,
load validation, presentation, and regional integration.

- [Well-known trainers](well-known-trainers.md) owns well-known trainers, their
  TR, the team-level and team-size scalers, rosters, and team composition. This
  spec reads a trainer's TR and composed team; it never restates how they are
  computed.
- [Player Trainer Rating](player-trainer-rating.md) owns the player's TR. In
  the target a league win adds no player TR; the current ROM's +8 per first
  venue clear stays documented in the circuit spec until adoption.
- The current [interregional circuit spec](wayfarer-interregional-league-circuit.md)
  keeps admission, venue order, replays, first-clear facts, and ceremonies.
  This spec changes only who is fielded and what happens after a loss.

Upon adoption this replaces the fixed lineups and player-entry-TR level policy
in [League scaling](league-scaling.md), which stays the record of current ROM
behavior.

## Venues

Venue identity is stable and distinct from region: Indigo = 1, Masters = 2,
Hoenn = 3. The venues keep their public entrances and rooms: FRLG Indigo, the
Seven Island Masters House leading into HNS rooms, and Emerald Hoenn.

Masters resolves to Sevii/Kanto for ordinary regional systems. Its reused HNS
rooms need a Wayfarer-only map-context override; standalone HNS keeps its
identity. The final HNS ceremony room is the Masters Gallery. Geography, room
names, and titles never substitute for saved venue or selected-character
identity.

## Registry and eligibility

Author a versioned, machine-readable registry:

| Field | Contract |
| --- | --- |
| `characterId` | Stable `u32` identity for one person, shared with the well-known catalog and independent of Trainer IDs, title, party, or region. |
| `displayName` | Existing localized name or an authored circuit name. |
| `presentationId` | Audited graphics, introduction, defeat, and after-battle text. |
| `enabled` | Build-time inclusion, with an authored reason when disabled. |

Maintain an alias inventory linking existing Trainer IDs and source roster
owners to canonical characters. Gym, rival, Champion, rematch, and regional
encounters of one person share one `characterId` and one roster, so aliases
cannot enter a field twice. People with similar names remain distinct.

A trainer is **eligible** when they are a well-known trainer with an authored
TR and a valid roster, fight in singles, are enabled, and have validated
presentation. Red and Tate & Liza are not well-known in v0, so they are
league-ineligible and keep their current policies: Red his separate mastery
encounter, Tate & Liza their double battle. Region, title, and story
availability neither add nor remove a trainer. Circuit eligibility does not
change story battles, which follow the
[every-battle rule](well-known-trainers.md#trainer-rating).

## Selection and order

One global pool of eligible trainers serves every venue. At entry:

1. Sort the pool by TR, highest first.
2. Take the first five. Equal TRs keep whatever order the registry iteration
   and sort produce; there is no tie-break rule.
3. Order the five by ascending TR for battle, so the highest TR fights last.
   Equal TRs again take whatever order the sort produces.

Venue, region, home membership, title, player TR, party, and history play no
part. Every venue may therefore field the same five, and v0 accepts that. The
procedure consumes no randomness and reads no seed. Other venues are never
resolved early.

Because ties have no rule, a changed registry order or sort can reorder tied
trainers in a field not yet entered. The frozen field keeps any entered field
stable.

## Frozen field

Entry captures, for each of the five in battle order: `characterId`, their TR,
and their composed team, plus the registry and roster content versions. Per
member, the team holds the roster entry index and every resolved battle field
the plan uses: species/form, level, moves, item, ability, nature, IVs/EVs, and
battle order. It is saved atomically before reveal.

The field stays locked until the venue is won; a loss or leaving keeps it.
Every retry at that venue, including after reload, reconstructs battles from
the saved field and never reselects or recomposes. With fixed TRs this equals
locking after a loss. If the content version changes while a field is locked,
the lock is dropped and the next entry captures a new field (prerelease
policy; the save stays valid).

## Saved state

Keep the current circuit's saved facts (lifetime clears, pending ceremony
phase). Add:

- at most one **locked field**: `venueId`, the content versions, and the five
  slots of the frozen field; and
- active-run progress: the venue and the defeated prefix of the five slots.

There is no seed, edition, rotation history, or field history. Only one field
can be locked at a time because admission already requires the previous venue
to be won. Save an explicit schema discriminator for this layout; prerelease
saves need no migration.

## Lifecycle

### Entry

Admission follows the current circuit rules. At a venue that is not yet won:

1. If a locked field exists for this venue, load it. Otherwise validate the
   absence of any other locked field or pending transaction, select the five,
   and atomically save the locked field.
2. Start a fresh run and reveal the lineup.

A failure leaves the prior state intact; a crash exposes either the old state
or the complete locked field. Denied or cancelled requests change nothing and
reveal nothing.

Replays of a won venue reselect the field deterministically; fixed TRs make it
identical to the won field. A replay stores nothing beyond the existing replay
run record and never locks the venue. Whether a replay is available while
another venue is locked follows the current circuit rules.

### Loss

A loss blacks the player out as usual. A blackout or voluntary exit ends the
run, not the field: run progress resets and the locked field stays unchanged.
The next attempt starts at slot 1 against the same five with the same teams
and levels. Record no result or reward. The player may retry at once or leave,
earn badges, and return any number of times; nothing earned meanwhile touches
the locked field. Save and reload preserve it. In v0 only a content version
change can alter a team (see [Load validation](#load-validation)).

### Win

After five victories, one transaction atomically:

- records the venue clear and, if it is the first, its current first-clear
  effects other than player TR, which a win never changes;
- queues the current ceremony; and
- releases the locked field and run.

Stale or duplicate callbacks are rejected. Individual victories, losses, and
Red add no circuit TR.

## Active run and dispatch

Progress belongs to the locked field's slots (or, in a replay, the reselected
field's); global Trainer defeat flags cannot skip a slot.

1. Validate the locked field, admission, run, and destination before locking
   an entrance or changing room state.
2. Each battle validates venue, room, and expected slot. The stored team and
   versioned references supply party, class, sprite, portrait, name,
   introduction, defeat text, music, and AI. Fixed room-owner IDs never pick
   opponents.
3. A victory advances one slot exactly once. A loss follows Loss.
4. Room and ceremony transitions keep the venue identity until the win
   commits.

Live badges, clears, player TR, party, and XP never mutate a locked field.
Debug and ordinary battles cannot create runs, advance slots, grant clears, or
create circuit records.

## Battle construction

Each slot fights with the trainer's own team at their own TR, as
[Well-known trainers](well-known-trainers.md) composes it for any battle: there
is no league-specific level offset, role adjustment, or six-member competitive
profile. The old `[-4,-3,-2,-1,+1]` room offsets are removed.

Construct from the saved field. Resolve the selected trainer and roster owner
before applying circuit policy; never identify enrollment from class, map, or
a shared party pointer. Record runtime Trainer ID, source roster owner, and
content version, and preserve member identity through ordering and gimmick
remapping. Content is immutable at runtime; do not edit shared Gym or story
parties.

Challenge settings keep their overrides. Party randomizers keep their
precedence but cannot reroll participants: the trainer species randomizer may
bypass authored parties as it does today but still uses the saved people and
order. XP uses actual species and levels. Prize money uses the trainer's
inventoried source reward basis and class, not the old room occupant, and team
size must not shift it.

## Load validation

Validate the schema, lifetime clears, and pending transactions before any
dispatch. No locked field is normal; never generate content on load.

A locked field must name a venue that is not yet won and is the current
admissible venue, hold five distinct eligible characters in non-decreasing TR
order, and resolve every reference. Stored teams must be valid for their
roster (known entries, legal forms, levels, and moves); they are never
recomposed on load. If the saved content versions differ from the build's,
drop the lock and run progress; the next entry captures a new field. This is
the prerelease policy, not an invalid save.

A valid locked field with damaged run progress recovers to its own lobby with
progress reset and the field kept. A missing, corrupt, or unsupported locked
field follows standard invalid-save handling: never regenerate a field on
load or synthesize results.

## Presentation

Before entry, lineups are unavailable and inspection generates nothing. After
entry, show the five names and battle order; moves and items are hidden by
default. After a loss, staff invite the player to retry the same field
whenever ready.

Graphics, portraits, dialogue, battle metadata, and names follow the selected
character even in historical rooms; a displaced fixed resident must not remain
in dialogue. The last opponent is this field's finalist, whatever their title.
Dialogue cannot assume Blue occupies Indigo or Lance ends Masters. Masters
never calls its winner a regional Champion.

## Records, ceremonies, and integration

### Ceremonies and records

Ceremonies, records, and unlocks stay as the current
[interregional circuit](wayfarer-interregional-league-circuit.md) implements
them: Indigo's shared Kanto/Johto recognition, Masters Gallery without regional
awards, Hoenn's own recognition, cleanup, and credits, and replays granting
nothing further. Ceremony handling stays idempotent across callbacks, reloads,
and interrupted saves, and must finish before another entry.

### Story dependencies and travel

Red stays outside the pool. His Mt. Silver admission reads all three lifetime
clears; his encounter and rewards are unchanged. Blue's Saffron Dojo battle
keeps its current unlock on the first committed Indigo win, whether or not
Blue was fielded.

Preserve current travel: the S.S. Aqua maiden voyage and Ticket,
Olivine–Vermilion–Slateport service, and numbered Sevii service from Vermilion
stay independent of badges, clears, Rainbow Pass, Bill/Celio, National Pokédex,
and Sevii quests. Audit back warps, exits, healing and blackout targets, Dig,
Escape Rope, and ceremony returns; a blackout after a league loss must not
clear the locked field, and Masters never routes to the HNS Indigo lobby.

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

## Balance target

The first league must be beatable after 8 badges. An 8-badge player sits at
TR 80 with a soft cap of Lv 50
([Player Trainer Rating](player-trainer-rating.md#player-tr-scalers-v0)), so the
provisional catalog puts the top five around TR 85–95, a team level of about
53–59 ([Well-known trainers](well-known-trainers.md#trainer-scalers)). Every
well-known TR is a placeholder, so this is a content constraint checked by the
catalog report and playtesting, not a runtime rule. League wins add no player
TR, and every venue may field the same five, so later leagues are easy for a
player with more badges; v0 accepts that.

## Acceptance

Check in an inventory: aliases, roster references, presentation coverage,
exclusion reasons, provenance, and content versions. Required implementation
evidence (not yet run):

1. **Registry.** Reject duplicate characters or aliases, unresolved source
   IDs, missing assets, double-battle flags, and trainers without an authored
   TR or valid roster. The build must hold at least five eligible trainers.
2. **Selection.** Fixtures for the top five over distinct TRs, ties at the
   fifth-place boundary, and ties inside the field; the battle order is
   non-decreasing in TR with the highest last; excluded and disabled trainers
   never appear; aliases never appear twice.
3. **Fresh state.** A new game has no locked field; load, display, and denied
   or cancelled entry generate nothing.
4. **Entry.** First entry saves five slots atomically in ascending TR order;
   inject failures before, during, and at commit.
5. **Loss.** Lose at each slot: the locked field is unchanged, progress
   resets, nothing is recorded, and the player blacks out to the usual target.
   Earn a badge and level up, then retry: identical five trainers, teams, and
   levels. Repeat with reloads and voluntary exits.
6. **Win.** Exactly one clear and ceremony; only the first win grants
   first-clear effects; no win changes player TR; the locked field is
   released. Interrupt and repeat ceremony commits.
7. **Replay.** Replays of a won venue reselect an identical field, store
   nothing beyond the existing replay run record, and never lock it.
8. **Construction and presentation.** Build every eligible trainer's team,
   preserve member identity and metadata, and reconstruct identically from the
   saved field. Names, sprites, portraits, text, music, AI, money, XP, and
   parties match the fielded slots, including a Gym Leader fielded last.
9. **Load validation.** Corrupt locked fields, schema, or callbacks are
   rejected without regenerating, advancing, or rewarding. A content version
   change drops the lock, and the next entry captures a new field.
10. **Standalone.** FRLG, HNS, and Emerald League behavior, travel, and
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

- Signup thresholds and qualification designed for this circuit.
- Seeded keys and draws for selection, and seeded venue order.
- Recurring editions with rollover, rotation weights, and rotation history.
- Role windows and standing-based slots, with a nearest-standing fallback for
  empty windows.
- Home leagues and the 85/15 home/visitor draw.
- A Trainer Card itinerary view for the new circuit.
- Winning-team records per edition and edition completion presentation.

## References

- [Leagues PRD](../prds/leagues.md)
- [Well-known trainers](well-known-trainers.md)
- [Player Trainer Rating](player-trainer-rating.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
- [Existing League scaling contract](league-scaling.md)
- [Party construction](../../game/src/battle_main.c)
