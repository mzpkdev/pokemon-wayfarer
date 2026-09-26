# Seeded League circuit runtime

PRD: [Seeded trainer circuit](../prds/seeded-trainer-circuit.md)

Implemented: No

Draft successor to the fixed-order [interregional circuit](wayfarer-interregional-league-circuit.md).
The confirmed direction keeps recurring circuits and a seeded venue order, but
locks opponents and strength only for the current individual competition.
A loss ends that event; the player must wait for the next competition at the same
venue. Entire-edition snapshots and immediate retries have been rejected.
Registration timing, the event-availability rule, and supporting presentation,
history, record, and ceremony defaults remain unresolved or proposed.

The [trainer world progression proposal](trainer-world-progression.md) defines
personal badge/first-clear growth. The experimental
[balance explorer](../../devtools/ui/README.md#trainer-balance-explorer) provides
provisional values rather than approved ROM content. The user's tentative
qualification direction is around 8, 16, and 24 global badges for the first
circuit's successive stops; those numbers are not approved admission gates.

## Scope

This specification owns circuit/order persistence, individual competition entry,
availability and lifecycle, active battle runs, result/lifetime-clear
transactions, recovery, presentation, regional integration, and acceptance.
The sibling [trainer pool specification](circuit-trainer-pool.md) owns canonical
identity, eligible profiles, home leagues, participant allocation, and rotation inputs.
[Trainer world progression](trainer-world-progression.md) owns personal growth
and the shared versioned NPC TR-to-level resolver. The
[shared playthrough seed framework](playthrough-seed-framework.md) owns root
initialization/persistence, pure keyed draws, and unbiased bounded sampling.
This runtime owns committed competition outcomes and availability facts.

The three venues retain their public entrances and selected rooms: FRLG Indigo,
the Seven Island Masters House leading into HNS rooms, and Emerald Hoenn.
Regional stories, badges, ordinary battles, and travel retain their existing
owners except for dependencies stated below. Historical winning-team records
may remain; playable exhibitions of completed lineups require separate design.

## Behavior

### Venue and position identity

Use stable venue identities distinct from region and order position. Each
circuit edition contains Indigo, Masters, and Hoenn once, in its saved seeded
order. Positions 1–3 determine mandatory travel order. Only a competition win
advances the current position; a loss leaves the same venue current.

Every competition has two contenders, two elite trainers, and one headliner.
Roles express TR suitability rather than historical titles. Ordered, disjoint
bands are authored by actual entry world point `(B,C)`, where B is global badges
and C is distinct lifetime venue clears. Bands are shared across venues at the
same point; endpoints remain D3. Venue, order position, competition ordinal, and
edition number do not themselves add strength. Actual milestones may change
between competitions and stops.

Masters resolves to Sevii/Kanto for ordinary regional systems. Its reused HNS
rooms need a Wayfarer-only map-context override independent of active-run
validity; standalone HNS retains its source identity. The final HNS ceremony
room is the Masters Gallery. Geography, room names, and historical titles cannot
substitute for saved venue, order position, or selected character identity.

### Saved schedule

At new game, initialize and validate the shared root, assign edition ID 1, and
save its complete ORDER. Save empty edition-result/lifetime-clear masks,
completed count 0, no active competition or run, and empty participant history.
The valid order-only state can be saved and displayed; it cannot dispatch battles
or expose a lineup. No future venue roster, effective TR, or party plan exists.

The proposed foundation stores one 64-bit root as two `u32` words with explicit
format/derivation metadata. Root/order construction cannot consume or reseed
Pokémon RNG. The circuit saves:

- one-based `editionId`, ORDER version, and three ordered stable venues;
- current-edition contiguous win prefix, distinct lifetime venue-clear mask,
  and proposed bounded completed-edition count;
- versioned per-venue competition identity/ordinal within the edition and lifecycle state,
  including authoritative availability facts under the adopted rule;
- bounded participant rotation history including concluded losses and wins;
- at most one current active competition snapshot, with its generation inputs,
  allocation/history inputs, content/rule versions, and five ordered slots; and
- active-run progress and pending result/closure transaction identity/phase.

The snapshot captures actual live `B_entry` and `L_entry` at successful individual
entry/registration. Use `C_entry = popcount(L_entry)` for every slot. Resolve
candidate personal TR through [trainer world progression](trainer-world-progression.md),
including the immutable root-derived growth percentage under the save's pinned
growth policy. Choose stage/profile before role eligibility. Do not project
future badges or clears or reroll personal growth per event.
Later venues resolve only when their own competition can be entered.

Each slot saves role, canonical character ID, effective NPC TR, `growthPercent`,
growth-policy version, stage/profile references, and versions sufficient to
reconstruct its exact team and levels. The root and pinned growth-policy version
exist at new game before world Gyms; aliases and later events share each person's
root-derived percentage.
Do not store full parties or derived levels when versioned references reproduce
them. Five canonical identities must be distinct within the competition; source
Trainer aliases do not create extra people. A person may recur in later events
or other venues, with strength resolved from that event's live entry milestones.
The active snapshot remains stable through reload and battle reconstruction.
Enrolled world Gyms keep using live world progression; story, Dojo, and rematch
encounters retain their separate enrollment and policies.

First apply the entry point's TR/content eligibility and within-event identity
uniqueness. Partition eligible candidates by versioned `homeLeagues`: home if
this venue is included, visitor otherwise. Multiple sensible homes are allowed;
map region does not establish membership. Home membership cannot override TR
requirements, and visiting does not alter strength at the same world point.

With both buckets nonempty, POOL_KIND `Uniform(100)` chooses home for 0–84 and
visitor for 85–99. With no eligible visitors, use home without a category draw.
An empty home bucket is invalid authored content; never force visitors or widen
bands. Draw only within the chosen bucket using strictly positive rotation
weights. The 85/15 split is a category chance, not an individual trainer chance
or lineup quota. There is no visitor cap or novelty redraw.

Rotation uses committed participation history from concluded competitions,
including losses. Save each venue's latest concluded five-character field and
its event identity/outcome, plus bounded per-venue character unions for the
current edition. Every conclusion updates that venue's latest field and union
exactly once. The proposed current-circuit factor `[16,4,1]` uses how many distinct
OTHER earlier venues' unions contain the character (0/1/2), excluding this venue.
Repeated failures at one venue therefore cannot inflate the factor without bound.
The proposed prior-venue factor is 1 for a character in this venue's latest
concluded field, 2 otherwise; a venue with no completed event uses factor 2.
Reset current-edition unions at rollover but retain each venue's latest field.
The active snapshot captures immutable pre-entry history inputs for read-only
verification. D5 weight values and concrete bounded storage remain provisional.
No unlimited old-party or playable-schedule archive is required.

ORDER uses edition identity. POOL_KIND and ROSTER use the tuple
`(editionId, stableVenueId, competitionOrdinal)`, with the ordinal one-based per
venue within that edition. Encode ORDER with entity words 0/0 and occurrence
`editionId`; POOL_KIND/ROSTER use entityLo = venueId, entityHi = editionId and
occurrenceId = competitionOrdinal. Their pre-sort battle-slot draw IDs are 0–4.
Preserve semantic IDs ORDER 1, POOL_KIND 5, and ROSTER 2; retired IDs 3 and 4
cannot be reused. ORDER remains rules version 1; proposed POOL_KIND version 2 and
ROSTER version 3 distinguish this event lifecycle. Save an explicit schema
version/discriminator for the new state. It cannot be silently interpreted as
the former whole-edition snapshot layout.

Pure keys and unchanged inputs produce the same unresolved lineup. Once entered,
the saved active snapshot is authoritative. Menus, previews, cancellation,
reloading, unrelated seeded features, and Pokémon RNG calls cannot advance event
identity or expose another candidate. Content updates cannot reinterpret saved
references under another version. Catalog/global versions must not be added to
every random key. Fresh competition identities can produce new lineups, but
participants and even the entire field may legitimately repeat.

Every current venue result requires its lifetime fact. Edition 1's committed
result and lifetime masks match; later editions start with all lifetime facts
true. Derive the next position from the contiguous current-result prefix.
Proposed completed count is `editionId - 1` while unfinished and `editionId` when
complete. Losses and new competition ordinals never increment that count.

### First registration and registering the next edition

Registration/entry here means the successful transition that creates one current
venue competition snapshot. The UI and exact time remain undecided; it is never
registration of all three lineups. Entry requires the current venue, the adopted
qualification and event-availability rules, no active competition/run, and no
pending closure or ceremony. A denied or cancelled request changes no identity,
availability, history, or roster state and presents no generated lineup.

1. Bind the request to expected edition, current venue/position, available event
   identity, and availability revision. Validate qualification and absence of a
   prior entry for that event. Reject exhausted bounded counters without changes.
2. Capture live `B_entry`, `L_entry`, progression/band/resolver/catalog/profile
   versions and committed participant history. Stage and validate five slots for
   this venue only, using the same stable event keys on transaction retry.
3. Revalidate request identity and eligibility, then atomically save the active
   snapshot, captured inputs, history inputs, consumed-entry/availability state,
   and matching run with empty room progress. Failure preserves the prior valid
   available/waiting state; no partial field or consumed event without its plan.
4. Show the committed lineup and enter its first room. Repeated/stale requests
   cannot overwrite it or generate another event. A crash exposes either the
   preceding valid state or this complete active event.

An event-availability transition is separately governed by unresolved D6.
After loss, that event is closed and cannot be re-entered. Only the adopted rule
can make the next competition available and allocate its next ordinal. Save its
availability facts and identity atomically, and reject stale/duplicate transition
requests. The first permitted competition at a venue uses ordinal 1; a successor
uses the next ordinal only when this transition commits. Reject overflow without
allocating a replacement identity. Reload, preview, cancel, or a failed entry
cannot authorize advancement.
Do not invent an RTC, day, step, battle-count, or real-time timer in this draft.
Loading a save before entry with unchanged identity, milestones, history, and
versions reproduces the same event field; loading before a loss does not provide
a different field or next-event identity.

After the third win, the edition is complete. Recurring circuits retain the
shared root and use the next edition's seeded order, with cleared current-result
mask and preserved lifetime progression/history. Edition rollover remains
explicit and atomic: bind the expected completed edition, reject overflow and
pending/active state, stage the next ORDER, and commit new ID/order/results as a
unit. It does not create lineups or guarantee that the next competition is
immediately available. The rollover interface and recurring-event cadence remain
undecided. Interrupted/duplicate requests cannot skip editions, change history,
or expose alternate orders. Prerelease save migration is not required.

### Qualification and travel

Global badges come from the three existing regional badge sets, counted once and
never spent or gated by circuit progress. Exact registration/admission thresholds
remain D4. Around 8/16/24 badges for successive first-circuit stops is tentative
user intent; there is no universal 24-badge registration requirement.

| Schedule position | Confirmed prerequisite |
| --- | --- |
| 1 | Current venue uncleared in this edition; adopted qualification and event available |
| 2 | Position 1 won; current venue uncleared; adopted qualification and event available |
| 3 | Positions 1–2 won; current venue uncleared; adopted qualification and event available |

Lifetime clears do not satisfy current-edition predecessors. Badge origin,
Champion flags, player origin, local quest state, and generic game-clear flags
cannot replace qualification facts. Later visits can explain requirements but
cannot register or snapshot future venues. A loss leaves this same position
current and unavailable until the next competition; it never skips a venue.
Later editions retain their new order and qualify under the eventual recurrence
rules. Qualification and timing must explicitly cover a fully badged player.

All 24 badges remain obtainable without circuit clears. NPC strength and role
bands use live entry badges/lifetime clears, never player party or player TR.
Only first-lifetime venue wins add the existing +8 player TR contribution.
Validate battle attrition against the player's actual soft cap at each supported
entry point; the former fixed 56/64/72/80 admission sequence is not assumed.

Every venue must be reachable at its earliest adopted eligibility. Preserve
S.S. Aqua maiden-voyage/Ticket and Olivine–Vermilion–Slateport service. Numbered
Sevii service from Vermilion remains independent of badges, clears, Rainbow Pass,
Bill/Celio, National Pokédex, and Sevii quests. Masters' caretaker/hidden door
read its current scheduled requirements. Entry requirements apply at the door,
not to island travel. Win, loss, refusal, departure, and recovery connect to the
ordinary travel network.

### Active-run record and battle dispatch

Persist active status, edition, competition identity/ordinal, venue, position,
and contiguous defeated prefix of the five slots. Progress belongs to this
event and actual slot; global character or shared Trainer defeat flags cannot
skip another appearance. A saved current competition may exist while the player
is outside its rooms. Departure preserves the committed lineup and event identity;
it cannot release them to generate a new field. The exact voluntary-exit/resume
interface and room-progress policy require design before implementation.

There is no strength input from player `ratingAtEntry`. Saved NPC TR and selected
stage/profile determine teams and levels through the shared resolver. Changes to
live badges, clears, player TR, party, or XP do not mutate an active event. World
Gym encounters continue to resolve their live growth independently.

1. Validate root/order, event snapshot, qualification, run, and destination
   before locking an entrance or changing room state.
2. Successful entry creates the event and matching run atomically; re-entry into
   that existing event cannot perform another allocation or ordinal transition.
3. Reload/resume and normal room transitions preserve the matching event and
   defeated prefix, subject to the eventual voluntary-departure policy.
4. Each battle validates edition/event, venue/position, room, and expected slot.
   Versioned references supply party, class, portrait/sprite, name, introduction,
   defeat text, music, and AI. Fixed room-owner IDs do not select opponents.
5. Victory advances one slot exactly once; loss advances none and closes the
   competition through the loss transaction below. Final victory requires all
   five victories before its result transaction.
6. Room/ceremony transitions retain event identity until conclusion commits.

Five singles use contender slots 0–1, elite slots 2–3, and title-agnostic headliner
slot 4. Allocate `[4,2,3,0,1]` within this venue only, then sort the contender and
elite pairs by TR and canonical ID. POOL_KIND/ROSTER use stable venue entities
Indigo 1, Masters 2, Hoenn 3 and pre-sort slot draw IDs. There is no generation
pass over future venues. Room entry cannot reorder or reroll selections.

Preserve explicit party-randomizer precedence: it may replace Pokémon under its
own contract but cannot reroll participants, qualification, or event identity.
Ordinary individual battle RNG remains unchanged. Debug battles cannot create
runs, advance competition slots, or grant clears.

### Edition-result and lifetime-clear transaction

Commit a venue win only for the valid current competition after all five
victories. Atomically record that event's win and participant-history update,
current-edition venue result, required once-only effects, and closure/release of
its active snapshot/run. If this venue's lifetime fact is false, commit it and
its first-ever effects in the same transaction. Reject stale event/edition
callbacks; a lifetime fact cannot replace the current-edition result.

The following ceremony/record defaults remain proposed:

| Venue, at any position | Once per edition venue result | First-ever lifetime effects |
| --- | --- | --- |
| Indigo | One FRLG winning-team Hall of Fame registration and Champion Ribbon flow | Shared Kanto/Johto Champion/game-clear recognition; +8 player TR |
| Masters | Gallery result/presentation; no Hall of Fame or Champion Ribbon | Lifetime Masters clear; +8 player TR; no regional status/cleanup |
| Hoenn | One Emerald winning-team Hall of Fame registration and Champion Ribbon flow | Hoenn Champion/game-clear recognition and Hoenn-owned cleanup; +8 player TR |

Masters never grants regional status, Hall of Fame, Ribbon, or cleanup. Later
Indigo/Hoenn wins may record a winning team without repeating lifetime effects.
Existing Ribbon ownership rules apply. Ordinary individual battle rewards retain
their existing behavior; new prizes/currencies/prestige require separate design.
Retained records do not authorize exhibition battles against a closed snapshot.

Only the first-ever win at each venue contributes +8 player TR, at most three
contributions. Preserve badge contribution, ceiling 80, and high-water behavior.
Later editions, losses, repeat callbacks, individual victories, and Red add no
circuit progression TR. Current-result resets cannot remove or re-award it.

The third result completes the edition and increments its aggregate count once.
Full credits follow only edition 1's third result, whichever venue it is. Hoenn
early performs its own lifetime cleanup without full credits; Masters final
runs Gallery/credits without regional awards. Later editions use a proposed
brief completion presentation. Event, result, history, and ceremony handling must
be idempotent across callbacks, loads, interrupted saves, and ceremony recovery.
Pending markers identify edition, event, venue, position, and phase. Finish the
same transaction before admitting another competition or rollover. Document the
actual save/ceremony commit boundary; do not claim a deterministic seed prevents
replaying a battle from an older save or changes existing battle RNG.

### Loss, exit, and replay

A loss/blackout concludes the current competition as a loss. Atomically record
its event identity, loss outcome and participant-history update; release its
active lineup/TR/profile snapshot and run progress; persist waiting/unavailable
state; and return to its own connected lobby. No current-edition win, lifetime
clear, victory record, circuit TR, regional title, cleanup, or clear reward is
granted. Earlier edition results, lifetime facts, titles, and player TR remain.

The same venue stays current. Immediate entry into the lost event is forbidden.
The player waits for the next competition under the eventual availability rule.
Its own entry uses actual then-current badges/clears, fresh stable event identity,
and updated participation history. It may draw returning contestants; novelty is
not guaranteed. A repeated loss callback cannot update history twice or allocate
another event. An interrupted closure cannot leave both an active field and
permission to enter its successor.

Audit back warps, voluntary exits, healing/blackout targets, Dig, Escape Rope,
and ceremony returns. Never route Masters to the HNS Indigo lobby. Voluntary
departure/re-entry and reload cannot create another field; the current event
snapshot stays committed pending an explicitly designed resume/forfeit policy.
A forfeit rule has not been approved and cannot be assumed equivalent to loss.

Loading a valid unfinished competition preserves that event and its saved
progress. Completed/lost lineups are not playable retries or frozen exhibitions.
Winning-team records and bounded participant history may survive closure, but
post-completion replay/rematch combat and its rewards require separate design.

### Load validation and damaged state

Validate root format/derivation, circuit schema, edition/order, contiguous result
prefix, lifetime facts, completed count, event identities/states/availability,
history, and pending transactions before dispatch. The initial order-only state
has edition 1, empty masks/history, zero completed count, and no event/run.
Ordinary waiting/available states after losses or earlier wins also legitimately
have no active snapshot; validate their saved lifecycle facts rather than treating
absence as deferred roster initialization. Never generate content merely on load.

An active event requires exactly one complete five-slot snapshot with matching
run/event ownership. Validate `B_entry` in 0–24, `L_entry` against valid lifetime
bits, captured input/version ranges, role layout, uniqueness, and references.
Verify every slot's growth-policy version against the save's pinned policy and
its `growthPercent` against root/canonical identity (integer 90–110). Recompute
TR/stage/profile eligibility from captured `(B_entry,popcount(L_entry))` through
the shared growth authority, including that percentage, then verify bucket
resolution. Never substitute a neutral percentage for invalid saved growth data.
Captured badges/clears may precede live progress; they cannot exceed or contradict
monotone live facts. Captured clears must reflect the actual preceding results at
entry, not projected future wins. There is no `B_entry = 24` invariant or
edition-1-empty-clear invariant: a later first-edition stop may capture earlier
wins. Later editions require all lifetime facts already true.

Purely reproduce canonical allocation and pair sorting from saved root, stable
venue/event key, rules/content versions, entry inputs, and immutable generation
history inputs. Compare against the saved active field without modifying inputs
or outputs. Draw IDs identify pre-sort allocation slots; displayed room indices
cannot validate POOL_KIND directly after pair sorting. Never recompute from live
badges/history or replace the saved field when verification fails.

Validate bounded history schema, canonical references, distinct IDs per event,
venue/event mapping, ordering and exactly-once conclusion relations. Losses must
be represented in rotation inputs even before edition 1 has any win. Do not
require old identities to fit current bands or treat them as defeat flags.
Validate union membership/ranges against the saved character catalog, latest-field
identity/outcome consistency, and immutable entry-history schemas. Current-edition
unions contain concluded participation only; an active event cannot count itself
in its captured rotation inputs. Latest fields can belong to an earlier edition
and therefore need not be members of the reset current-edition unions. Closed
event identity cannot also be active or available for entry. Invalid
counter, event-state, or history relations cannot authorize the next ordinal.

If the snapshot and lifecycle are valid but transient room/run state is damaged,
recover to its own lobby without rewards and retain the committed event field.
A run outside its expected rooms may become suspended; it cannot be silently
closed, advanced, or replaced. Recovery/resume of malformed progress remains an
implementation requirement, with its exact policy resolved alongside departure.

Missing/damaged/unsupported root, order, active snapshot, history, versions, or
authoritative event state follows standard invalid-save handling. Do not replace
roots, clear required history, regenerate fields, synthesize wins or availability,
or introduce prerelease compatibility solely to retain old saves. A valid root
alone does not authorize rebuilding outcomes under acquired progression.

### Itinerary, lineups, and dialogue

Propose a local Trainer Card circuit view from new game showing global badges,
edition number, saved complete order, current required venue, qualification, and
lifecycle states such as Locked, Waiting for competition, Available, In progress,
and Cleared. Edition complete appears only after all three results. Proposed
completed-edition totals and historical winning-team records read committed facts.

Before individual entry, lineups are unavailable; inspection cannot generate a
preview or advance an event. After entry, display only the active competition's
five names, authored specialties, and NPC TR in battle order. Later venues have
no frozen lineups to inspect. Closed-event history may show recorded identities,
but it cannot masquerade as a currently enterable field. Distinguish player TR
from NPC TR without requiring public moves/items/full parties.

Staff explain actual qualification, the next required venue, and saved event
availability. After loss, explain that the competition ended and the player must
wait for the next one; do not offer an immediate retry or promise a wait duration
before its rule is adopted. Masters' caretaker supports every order position.
Graphics, portraits, dialogue, battle metadata, and opponent names match the
selected character even where historical room assets remain. Labels may identify
a venue/slot rather than its displaced resident.

Historical titles describe background; the headliner is this event's finalist.
Dialogue cannot assume Blue always occupies Indigo, Lance concludes Masters,
or Hoenn ends the circuit. Masters never calls its winner a regional Champion.
No new automatic itinerary announcements are required on badge awards or other
unrelated interactions.

### Story dependencies and integration audit

Red remains outside the pool. His Mt. Silver admission reads all three lifetime
venue clears, independent of current edition results, order, or projected regional flags. Keep
his authored encounter and reward/repeat behavior unchanged.

The proposed Blue Dojo dependency is the first committed lifetime circuit clear,
regardless of venue or whether Blue was drawn anywhere. Keep its authored party
and existing Battle Point rules; circuit completion grants no extra Dojo reward.
Starting or losing a challenge, a raw battle victory, and an unfinished ceremony
do not unlock it. Blue and Red access persist through all rollovers. Audit dialogue that assumes defeating Blue or Indigo, while
preserving Giovanni's Earth Badge ownership and local finale.

Audit existing checks that equate Indigo with position 1, Masters with position
2, Hoenn with position 3, regional Champion flags with circuit order, or Hoenn
game clear with full credits. Classify each as position-dependent qualification,
venue-specific recognition/cleanup, or unrelated regional story behavior before
changing it. Hoenn's own completion cleanup stays attached to Hoenn; it cannot
be moved wholesale to the final venue. Optional quests, deferred rewards,
voyage state, initial Gym access, and other regions' unfinished stories must
survive every completion order.

Existing integration surfaces to review, not newly available APIs:

- [Save ownership and initialization](../../game/src/wayfarer_persistence.c)
  and [run/save structures](../../game/include/global.h).
- [Current circuit admission and lifecycle](../../game/src/league_circuit.c),
  [script wrappers](../../game/src/league_circuit_scripts.c),
  [script entry points](../../game/data/scripts/league_circuit.inc), and
  [warp/blackout handling](../../game/src/overworld.c).
- [Resolved battle construction](../../game/src/battle_main.c),
  [player TR producer](../../game/src/trainer_rating.c), and
  [post-battle ceremony handling](../../game/src/post_battle_event_funcs.c).
- [Trainer Card](../../game/src/trainer_card.c),
  [circuit status](../../game/src/league_circuit_status.c),
  [Sevii content manifest](../../game/src/data/wayfarer_sevii_maps.json), and
  [Blue Dojo scripts](../../game/data/maps/SaffronCity_FightingDojoVIP_hns/scripts.inc).

### Acceptance and release

Implementation must supply this evidence; these are required future checks,
not tests claimed to have run for this documentation revision:

1. Round-trip root/order-only new-game state without lineup generation on load,
   display, denied entry, preview, or cancellation. Cover all six venue orders,
   mixed badge origins, duplicate badges, and the eventual qualification
   boundaries. Do not hardcode 24-badge admission or adopt tentative 8/16/24
   values without resolving D4.
2. At successful current-venue entry, capture actual badges/clears, validate and
   atomically save only five slots. Earn badges before the next venue/event and
   prove it uses the new live world point. No future roster or projected clear
   may influence allocation or strength. Validate role bands/party stages before
   weighting, across supported B values, C 0–3, 90/100/110% personal growth,
   and TR saturation. Check unchanged baseline and alias-shared percentages;
   different roots may change effective TR at an identical entry world point.
   Reject corrupted percentage/policy snapshots rather than rerolling them.
3. Inject entry failures before generation, after staging, and at commit/save
   boundaries. Preserve either valid available state or the complete active
   event, never consumed availability without a field, partial slots, or another
   ordinal. Repeated/stale/cancelled entry reproduces the same field.
4. Preserve active participants, TR/profiles/levels and slot progress through
   reload, reconstruction, normal transitions, voluntary departure/resume, live
   world growth, Pokémon RNG use, and unrelated seeded feature calls. Verify
   aliases cannot duplicate a person in a field, while repeats across events or
   venues are allowed. Battle RNG retains its existing contract.
5. Lose at each of the five slots. Closure records participants and loss once,
   removes the active snapshot, resets run state, returns to the correct lobby,
   grants no victory effects, leaves this venue current, and blocks immediate
   entry. Inject closure/save interruptions and duplicate/stale callbacks.
   Reload/preview/cancel cannot turn the lost event into an available successor.
6. After the adopted wait/availability condition, atomically make a fresh stable
   event identity available. Enter with current milestones and loss-aware history.
   Participants may repeat; no redraw or forced novelty. Test qualification and
   recurrence for fully badged saves as well as first-circuit progression.
7. Finish all five battles and interrupt/repeat ceremony commits. Exactly one
   event win/history update/current result is recorded; only the first lifetime
   win supplies its +8 player TR, recognition, and cleanup. Individual battle
   wins/losses never count as venue clears. The next venue is unlocked by this
   result but remains subject to qualification and event availability.
8. Complete each venue in every position. Indigo grants shared Kanto/Johto
   recognition once, Hoenn performs its own cleanup, and Masters never grants
   regional status/Hall of Fame/Ribbon. Edition 1 third result alone produces
   full credits. Preserve deferred rewards, voyage/ferries, stories, Giovanni,
   Blue's proposed first-clear unlock, and Red's three-lifetime-clear gate.
9. Finish an edition and test atomic explicit rollover, every new order, counter
   overflow, duplicate/delayed requests, and interrupted saves. Rollover saves
   order/results only and preserves badges/lifetime facts/history/player TR,
   titles/unlocks and completed count. It generates no future lineups and cannot
   assume immediate competition availability. Edition number adds no NPC TR.
10. Verify ORDER depends on edition while event allocation uses stable venue and
    competition ordinal with correct versions. Catalog changes cannot perturb
    raw ORDER or unrelated keys; changed eligibility/history may affect new
    fields. Verify zero/all-one roots and no Pokémon RNG consumption by root,
    order, entry, or availability handling. No call-count random cursor is used.
11. Verify names, ratings, sprites, portraits, introductions, defeat text, music,
    AI, and parties agree with the active slots, including visitors/title-agnostic
    finalists. Future lineups cannot appear before individual entry, and closed
    events cannot offer retries/exhibition combat under this specification.
12. Prove home-only feasibility for every venue and supported entry world point:
    two distinct home contenders, two home elites, one home headliner. Cover
    multi-home membership, ineligible local TR, no visitors/no category draw,
    invalid empty-home content, category boundary 84/85, and multiple visitors.
    Verify positive loss-aware rotation weights, allocation `[4,2,3,0,1]`, pair
    sorting, and read-only verification using pre-sort draw IDs.
13. Inject missing/corrupt/unsupported root/order/event/schema/history/profile
    inputs, noncontiguous progress, wrong venue/edition/event callbacks, and
    inconsistent closed/available/active states. Reject without replacing a
    lineup, advancing availability, clearing history, or inventing rewards.
    Valid transient-run recovery preserves the committed field. Validate history
    includes losses and exactly-once participation across interruptions; retain
    immutable entry history inputs for active-field verification.
14. Audit standalone FRLG/HNS/Emerald League behavior, regional dispatch, travel,
    blackout/departure recovery, and all source fixed-order assumptions. Resolve
    event cadence, qualification, voluntary departure/resume and bounded history
    before implementing their scripts or acceptance assertions.

Extend relevant [mechanics coverage](../../game/test/league_circuit.c),
[script coverage](../../game/test/league_circuit_scripts.c),
[status coverage](../../game/test/league_circuit_status.c), and
[the League E2E journey](../../e2e/src/journeys/wayfarer-league-circuit.e2e.ts).
Build the production Wayfarer configuration and relevant test ROMs; compile
standalone configurations when shared code changes. E2E requires explicit
prebuilt ROM/symbol paths and does not build the game.

Release is blocked on unresolved parent decisions, foundation hash encoding and
golden vectors, feasible content at every supported live entry point, audited
save-sector/history/event transactions, and travel/presentation/journey evidence.
Emulator balance checks must measure attrition against actual player caps across
adopted badge thresholds and recurring competitions, plus rotation after wins
and losses. Structural feasibility alone does not establish combat balance.

## Open questions

The parent PRD owns D1's current-competition lifecycle, D2's title-agnostic
headliner, D3 catalog/bands, D4 qualification and entry timing, D5 rotation tuning,
and D6 competition availability/waiting.
Registration timing and D6 competition wait/availability are unresolved; 8/16/24
badges are tentative first-circuit intent. Resolve availability on recurrence,
voluntary departure/resume/forfeit, bounded loss-aware history, and edition
rollover interface before implementation. Supporting Blue access, ceremonies,
credits, winning-team records/Ribbons, and itinerary defaults remain proposed.
Frozen whole-edition retries and exhibition combat are not approved features.
Any revision must reconcile this runtime, world progression, pool, seed
framework, and save foundation together.
