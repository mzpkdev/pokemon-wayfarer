# League circuit runtime

PRD: [Trainer Circuit](../prds/seeded-trainer-circuit.md)
Implemented: No
Design status: v0 approved: entry captures the field and locks the venue to
it until won, and a win commits the venue. Signup and venue order stay as the current
[interregional circuit](wayfarer-interregional-league-circuit.md) until
designed.

## Scope

Own entry and field capture, the field lock, active runs and battle dispatch,
the win commit, saved state, load validation, presentation, and regional
integration for the v0 fields.

- [Circuit trainer pool](circuit-trainer-pool.md) owns eligibility, selection,
  ordering, the frozen field's contents, and battle construction.
- [Well-known trainer rating](trainer-world-progression.md) owns trainer TR,
  scalers, and rosters.
- The current [interregional circuit spec](wayfarer-interregional-league-circuit.md)
  keeps admission, venue order, replays, first-clear facts, player TR
  contributions, and ceremonies. This spec changes only who is fielded and
  what happens after a loss.

## Venues

Venue identity is stable and distinct from region: Indigo = 1, Masters = 2,
Hoenn = 3. The venues keep their public entrances and rooms: FRLG Indigo, the
Seven Island Masters House leading into HNS rooms, and Emerald Hoenn.

Masters resolves to Sevii/Kanto for ordinary regional systems. Its reused HNS
rooms need a Wayfarer-only map-context override; standalone HNS keeps its
identity. The final HNS ceremony room is the Masters Gallery. Geography, room
names, and titles never substitute for saved venue or selected-character
identity.

## Saved state

Keep the current circuit's saved facts (lifetime clears, pending ceremony
phase). Add:

- at most one **locked field**: `venueId`, the pool's content versions, and
  five slots in battle order, each holding `characterId`, TR, and the composed
  team (per member: the roster entry index and every resolved battle field the
  plan uses, namely species/form, level, moves, item, ability, nature,
  IVs/EVs, and battle order); and
- active-run progress: the venue and the defeated prefix of the five slots.

There is no seed, edition, rotation history, or field history. Only one field
can be locked at a time because admission already requires the previous venue
to be won. Save an explicit schema discriminator for this layout; prerelease
saves need no migration.

## Lifecycle

### Entry

Admission follows the current circuit rules. At a venue that is not yet won:

1. If a locked field exists for this venue, load it. Otherwise validate the
   absence of any other locked field or pending transaction, resolve the five
   through the pool spec, and atomically save the locked field.
2. Start a fresh run and reveal the lineup.

A failure leaves the prior state intact; a crash exposes either the old state
or the complete locked field. Denied or cancelled requests change nothing and
reveal nothing. Other venues are never resolved early.

Replays of a won venue reselect the field deterministically through the pool
spec; fixed TRs make it identical to the won field. A replay stores nothing
beyond the existing replay run record and never locks the venue. Whether a
replay is available while another venue is locked follows the current circuit
rules.

### Loss

A loss blacks the player out as usual. A blackout or voluntary exit ends the
run, not the field: run progress resets and the locked field stays unchanged.
The next attempt starts at slot 1 against the same five with the same teams
and levels. Record no result or reward. The player may retry at once or leave,
earn badges, and return any number of times; nothing earned meanwhile touches
the locked field. Save and reload preserve it. In v0 only a content version
change can alter a team (see Load validation).

### Win

After five victories, one transaction atomically:

- records the venue clear and, if it is the first, its current first-clear
  effects, including the player's +8 TR;
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

Live badges, clears, player TR, party, and XP never mutate a locked field. Party randomizers keep their precedence but cannot
reroll participants. Debug battles cannot create runs, advance slots, or
grant clears.

## Ceremonies and records

Ceremonies, records, and unlocks stay as the current
[interregional circuit](wayfarer-interregional-league-circuit.md) implements
them: Indigo's shared Kanto/Johto recognition, Masters Gallery without regional
awards, Hoenn's own recognition, cleanup, and credits, and replays granting
nothing further. Ceremony handling stays idempotent across callbacks, reloads,
and interrupted saves, and must finish before another entry.

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

## Story dependencies and travel

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

Integration surfaces to review (existing code, not new APIs):

- [Save ownership and initialization](../../game/src/wayfarer_persistence.c)
  and [run/save structures](../../game/include/global.h).
- [Circuit admission and lifecycle](../../game/src/league_circuit.c),
  [script wrappers](../../game/src/league_circuit_scripts.c),
  [script entry points](../../game/data/scripts/league_circuit.inc), and
  [warp/blackout handling](../../game/src/overworld.c).
- [Battle construction](../../game/src/battle_main.c),
  [player TR producer](../../game/src/trainer_rating.c), and
  [post-battle ceremonies](../../game/src/post_battle_event_funcs.c).
- [Trainer Card](../../game/src/trainer_card.c),
  [circuit status](../../game/src/league_circuit_status.c),
  [Sevii content manifest](../../game/src/data/wayfarer_sevii_maps.json), and
  [Blue Dojo scripts](../../game/data/maps/SaffronCity_FightingDojoVIP_hns/scripts.inc).

## Acceptance

Required implementation evidence (not yet run):

1. New game has no locked field; load, display, and denied or cancelled entry
   generate nothing.
2. First entry saves five slots atomically in ascending TR order; inject
   failures before, during, and at commit.
3. Lose at each slot: the locked field is unchanged, progress resets, nothing
   is recorded, and the player blacks out to the usual target. Earn a badge
   and level up, then retry: identical five trainers, teams, and levels.
   Repeat with reloads and voluntary exits.
4. Win: exactly one clear and ceremony; only the first win grants +8 TR and
   first-clear effects; the locked field is released. Interrupt and repeat
   ceremony commits.
5. Replays of a won venue reselect an identical field, store nothing beyond
   the existing replay run record, and never lock it.
6. Names, sprites, portraits, text, music, AI, and parties match the fielded
   slots, including a Gym Leader fielded last.
7. Corrupt locked fields, schema, or callbacks are rejected without
   regenerating, advancing, or rewarding. A content version change drops the
   lock, and the next entry captures a new field.
8. Standalone FRLG, HNS, and Emerald League behavior, travel, and recovery are
   unchanged.

Extend [mechanics coverage](../../game/test/league_circuit.c),
[script coverage](../../game/test/league_circuit_scripts.c),
[status coverage](../../game/test/league_circuit_status.c), and
[the League E2E journey](../../e2e/src/journeys/wayfarer-league-circuit.e2e.ts).
Build the production Wayfarer configuration and relevant test ROMs; compile
standalone configurations when shared code changes. E2E needs explicit
prebuilt ROM and symbol paths.

## Later

- Signup thresholds and qualification designed for this circuit.
- Seeded venue order and recurring editions with rollover.
- Rotation history across editions.
- A Trainer Card itinerary view for the new circuit.
- Winning-team records per edition and edition completion presentation.

## References

- [Trainer Circuit PRD](../prds/seeded-trainer-circuit.md)
- [Circuit trainer pool](circuit-trainer-pool.md)
- [Well-known trainer rating](trainer-world-progression.md)
- [Interregional League circuit](wayfarer-interregional-league-circuit.md)
