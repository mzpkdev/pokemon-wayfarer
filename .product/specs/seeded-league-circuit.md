# Seeded League circuit runtime

PRD: [Seeded trainer circuit](../prds/seeded-trainer-circuit.md)
Implemented: No
Design status: Draft successor to the fixed-order
[interregional circuit](wayfarer-interregional-league-circuit.md). One frozen
competition per edition and venue, with unlimited retry, is approved. Signup
thresholds (D4) and supporting ceremony, record, and presentation defaults
remain proposed.

## Scope

Own edition and order persistence, signup, competition entry and capture,
retry, active runs and battle dispatch, result and lifetime-clear
transactions, ceremonies, load validation, presentation, and regional
integration. The PRD owns the player-facing lifecycle.

- [Circuit trainer pool](circuit-trainer-pool.md) owns selection, roles,
  rotation, keys, and battle construction.
- [Trainer world progression](trainer-world-progression.md) owns `worldCap`,
  arcs, standing, rosters, and levels.
- The [seed framework](playthrough-seed-framework.md) owns the root and draws.

## Venues and positions

Venue identity is stable and distinct from region and position: Indigo = 1,
Masters = 2, Hoenn = 3. The venues keep their public entrances and rooms: FRLG
Indigo, the Seven Island Masters House leading into HNS rooms, and Emerald
Hoenn. Each edition visits all three once in its saved order; positions 1–3
fix the required travel order. Only a win advances the position.

Masters resolves to Sevii/Kanto for ordinary regional systems. Its reused HNS
rooms need a Wayfarer-only map-context override; standalone HNS keeps its
identity. The final HNS ceremony room is the Masters Gallery. Geography,
room names, and titles never substitute for saved venue, position, or
selected-character identity.

## Saved state

New game initializes the shared root (one 64-bit root as two `u32` words with
format metadata, without touching Pokémon RNG), assigns edition 1, and saves
its ORDER. No lineup exists yet. The circuit saves:

- `editionId` (one-based `u32`), ORDER version, and three ordered venues;
- current-edition result mask (a contiguous win prefix), lifetime venue-clear
  mask, and a bounded completed-edition count (which also feeds the progress
  index `p` owned by trainer world progression);
- per venue, the latest won five-person field and its edition (rotation
  history; losses record nothing);
- at most one active competition snapshot; and
- active-run progress and any pending result/ceremony transaction phase.

The active snapshot holds `(editionId, venueId)`, `B_event`, `L_event`,
`p_event`, `worldCap_event`, the pinned growth-policy version,
catalog/roster/rules versions, and five ordered slots. Each slot stores role,
`characterId`, arc, standing, fallback flag, and the trainer's composed team:
per member, its reference (`aceId` or `fillerId`), species/form, level, and
moves, in battle order. Freezing the team means trades, gifts, or modifiers
earned during retries never change the field. Other member fields come from the
versioned roster or trade record. Candidates' arcs and standings are pure
functions of the root, pinned policy, and `p_event`, so verification rederives
them rather than storing all of them. Rotation reads the saved per-venue fields;
only a win changes them, and that win also releases the snapshot, so the
snapshot does not copy them.

Save an explicit schema discriminator for this layout. Prerelease saves
need no migration.

## Lifecycle

### Signup

| Position | Requirement (D4, provisional thresholds) |
| --- | --- |
| 1 | ≥ 8 global badges |
| 2 | ≥ 16 global badges and position 1 won this edition |
| 3 | ≥ 24 global badges and positions 1–2 won this edition |

Global badges come from the three regional sets, counted once and never
spent. Lifetime clears never satisfy a current-edition predecessor. Badge
origin, Champion flags, player origin, and game-clear flags cannot replace
these facts. All 24 badges stay obtainable without circuit clears, and later
editions use the same table (a fully badged player qualifies at once).

### Entry

Entry is idempotent per `(editionId, venueId)`. The first successful entry at
the current venue:

1. Validates root, order, current position, signup, and the absence of an
   active snapshot or pending transaction.
2. Captures live `B_event`, `L_event`, `C_event = popcount(L_event)`,
   `p_event` from `B_event` and the committed completed-edition count,
   `worldCap_event`, and versions, reads rotation history from the saved
   per-venue fields, and resolves five slots through the pool spec.
3. Atomically saves the snapshot and a fresh run, then reveals the lineup.

A failure at any step leaves the prior state intact; a crash exposes either
the old state or the complete snapshot. Later entries to the same competition,
including after reload, read the saved snapshot and never regenerate. Denied
or cancelled requests change nothing and reveal nothing. Future venues are
never resolved early.

### Retry

A loss, blackout, or voluntary exit ends the run, not the competition. Return
the player to the venue's own lobby with the snapshot unchanged and run progress
reset, so the next attempt starts at slot 1 against the same five with the same
teams and levels. Record no participation, result, or reward. The player may
retry at once or leave, earn badges, and return any number of times; badges,
flags, gifts, and trades earned meanwhile never touch the snapshot. Save and
reload preserve it. There is no waiting state or availability rule.

### Win

After five victories, one transaction atomically:

- records the current-edition result and this venue's won field in history;
- sets the lifetime clear and its first-ever effects if not already set,
  including the player's +8 TR (at most three contributions, ceiling and
  high-water unchanged), which raises `worldCap` for later events;
- queues the ceremony and releases the snapshot and run.

Stale or duplicate callbacks are rejected. Individual victories, losses,
later editions, and Red add no circuit TR.

### Edition rollover

After the third win and its ceremonies, the edition is complete and the
completed count increments once. Rollover is explicit and atomic: bind the
expected edition, reject overflow and any active or pending state, derive the
next ORDER, and commit the new ID, order, and cleared result mask together.
Lifetime facts, history, badges, player TR, titles, and unlocks persist.
Rollover generates no lineup. The edition number itself adds no NPC strength;
the incremented completed count advances the progress index `p` for later
entries until it reaches its cap.

## Active run and dispatch

Persist edition, venue, position, and the defeated prefix of the five slots.
Progress belongs to this competition's slots; global Trainer defeat flags
cannot skip a slot.

1. Validate root, order, snapshot, signup, run, and destination before
   locking an entrance or changing room state.
2. Each battle validates edition, venue, room, and expected slot. The stored
   team and versioned references supply party, class, sprite, portrait, name,
   introduction, defeat text, music, and AI. Fixed room-owner IDs never pick
   opponents.
3. A victory advances one slot exactly once. A loss follows Retry.
4. Room and ceremony transitions keep the competition identity until the win
   commits.

Live badges, clears, flags, trade records, player TR, party, and XP never mutate
an active snapshot. Party randomizers keep their precedence but cannot reroll
participants or identity. Debug battles cannot create runs, advance slots, or
grant clears.

## Ceremonies and records

| Venue, any position | Every edition win | First-ever lifetime win |
| --- | --- | --- |
| Indigo | One FRLG winning-team Hall of Fame and Champion Ribbon flow | Shared Kanto/Johto Champion and game-clear recognition; +8 player TR |
| Masters | Masters Gallery result; no Hall of Fame or Ribbon | Lifetime Masters clear; +8 player TR; no regional status or cleanup |
| Hoenn | One Emerald winning-team Hall of Fame and Champion Ribbon flow | Hoenn Champion and game-clear recognition, Hoenn cleanup; +8 player TR |

Regional cleanup happens once per lifetime. Existing Ribbon ownership rules
apply. Full credits follow only edition 1's third win, whichever venue it is;
Hoenn cleans up on its own lifetime win without credits; later editions use a
brief completion presentation. Ceremony handling is idempotent across
callbacks, reloads, and interrupted saves: pending markers name edition,
venue, position, and phase, and must finish before another entry or rollover.
Winning-team records never authorize exhibition battles against past fields.

## Load validation

Validate root format, schema, edition and order, contiguous result prefix,
lifetime mask, completed count, history, and pending transactions before any
dispatch. A state with no active snapshot is normal (before first entry or
between venues); never generate content on load.

An active snapshot must match the current edition and venue, have `B_event` in
0–24, `L_event` within valid lifetime bits, and `p_event` consistent with
`B_event` and the completed count, not exceed or contradict live monotone facts,
use the pinned growth policy, hold five distinct characters in ascending
standing order, and resolve every reference. Verify each slot's arc against the
root, and recompute `worldCap_event`, standings, selection, and final ordering
from the captured inputs read-only. Stored teams must be valid for their roster
and version (known members, legal forms, levels, and moves); they are never
recomposed from live flags or trades. Draw IDs are pre-sort battle slot indices,
so room indices cannot validate POOL_KIND directly. History fields must hold
five distinct known characters and valid editions, and a venue's current-edition
field must match a set result bit.

A valid snapshot with damaged transient run state recovers to its own lobby
with progress reset and the field kept. Missing or corrupt root, order,
snapshot, history, or versions follow standard invalid-save handling: never
regenerate a field, replace the root, clear history, or synthesize results.

## Presentation

Propose a Trainer Card circuit view from new game: global badges, edition,
saved order, current venue, signup requirement, and per-venue state (Locked,
Open, In progress, Won). Completed-edition totals and winning-team records
read committed facts.

Before entry, lineups are unavailable and inspection generates nothing. After
entry, show the five names, specialties, and battle order, with arc hint lines
keyed by (trainer, arc, progress phase) from the progression catalog. Arcs and
standings are never shown as labels; moves and items are hidden by default.
After a loss, staff invite the player to retry the same field whenever ready.

Graphics, portraits, dialogue, battle metadata, and names follow the selected
character even in historical rooms; a displaced fixed resident must not
remain in dialogue. The headliner is this event's finalist, whatever their
title. Dialogue cannot assume Blue occupies Indigo, Lance ends Masters, or
Hoenn ends the circuit. Masters never calls its winner a regional Champion.
The Masters caretaker supports every order position.

## Story dependencies and travel

Red stays outside the pool. His Mt. Silver admission reads all three lifetime
clears, independent of the current edition, order, or regional flags; his
encounter and rewards are unchanged.

Blue's Saffron Dojo battle (proposed) unlocks on the first committed lifetime
clear, whatever the venue and whether Blue was drawn. His party and Battle
Point rules stay; entry, a loss, a raw battle win, or an unfinished ceremony
does not unlock it. Blue and Red access persist through rollovers.

Every venue must be reachable at its earliest signup. Preserve the S.S. Aqua
maiden voyage and Ticket, Olivine–Vermilion–Slateport service, and numbered
Sevii service from Vermilion independent of badges, clears, Rainbow Pass,
Bill/Celio, National Pokédex, and Sevii quests. Requirements apply at the
venue door, not to travel. Audit back warps, exits, healing and blackout
targets, Dig, Escape Rope, and ceremony returns; never route Masters to the
HNS Indigo lobby.

Audit checks that equate Indigo, Masters, or Hoenn with a fixed position,
regional Champion flags with circuit order, or Hoenn game clear with full
credits. Classify each as position-dependent signup, venue-specific
recognition or cleanup, or unrelated regional story before changing it.
Hoenn's cleanup stays with Hoenn. Preserve Giovanni's Earth Badge and local
finale, optional quests, deferred rewards, voyage state, initial Gym access,
and other regions' unfinished stories in every completion order.

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

1. New-game state round-trips with order only; load, display, denied or
   cancelled entry generate nothing. Cover all six orders, mixed and duplicate
   badge origins, and the adopted signup boundaries.
2. Entry captures live milestones and saves five slots atomically. Earning
   badges before the next venue changes its world point; future venues are
   never resolved. Inject failures before, during, and at commit.
3. Lose at each slot: the snapshot is unchanged, progress resets, nothing is
   recorded, and the player returns to the right lobby. Earn a badge, set a
   roster modifier flag, or trade, then retry: identical five trainers, teams,
   and levels. Repeat many times, including reloads and voluntary exits.
4. Win: exactly one result, history update, and ceremony; only the first
   lifetime win grants +8 TR, recognition, and cleanup. Interrupt and repeat
   ceremony commits.
5. Complete each venue in every position: Indigo's shared recognition once,
   Hoenn's own cleanup, Masters without regional awards, credits only on
   edition 1's third win, and Blue/Red gates as specified.
6. Rollover: atomic, every order, overflow, duplicate or delayed requests,
   interrupted saves; no lineup generated and all lifetime state kept.
7. Keys: ORDER by edition; POOL_KIND and ROSTER by `(editionId, venueId)` and
   slot at their pinned versions; no Pokémon RNG use; zero and all-ones roots.
8. Names, sprites, portraits, text, music, AI, and parties match the active
   slots, including visitors, fallbacks, and a Gym Leader headliner fixture.
   The pool's seeded-roots variety report shows Gym Leader headliners from
   every region (Kanto, Johto, Hoenn) at a nonzero rate.
9. Corrupt or mismatched root, order, snapshot, history, schema, or callbacks
   are rejected without regenerating, advancing, or rewarding.
10. Standalone FRLG, HNS, and Emerald League behavior, travel, and recovery
    are unchanged.

Extend [mechanics coverage](../../game/test/league_circuit.c),
[script coverage](../../game/test/league_circuit_scripts.c),
[status coverage](../../game/test/league_circuit_status.c), and
[the League E2E journey](../../e2e/src/journeys/wayfarer-league-circuit.e2e.ts).
Build the production Wayfarer configuration and relevant test ROMs; compile
standalone configurations when shared code changes. E2E needs explicit
prebuilt ROM and symbol paths.

Release is blocked on D4 and the pool's content decisions, seed golden
vectors, feasibility and fallback reports, audited save transactions, and
travel and journey evidence. Emulator checks measure attrition against actual
player caps at the adopted thresholds.

## Open questions

The PRD owns D4 (signup thresholds). Blue's Dojo gate, ceremonies, credits,
Ribbons, records, and the itinerary view remain proposed defaults.
