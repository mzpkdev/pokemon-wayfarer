# Playthrough seed framework

PRD: [Playthrough-seeded variation](../prds/playthrough-seeded-variation.md)
Implemented: No
Design status: Draft implementation contract for the confirmed framework intent.

## Scope

Own root-seed initialization and persistence, stable decision identities,
portable derivation, deterministic selection helpers, isolation from existing
RNG, and the adoption contract for future Wayfarer features. The circuit is the
first consumer; its specs own participant constraints and the saved schedule.

The API names below describe required behavior, not functions already present
in the game. Choose concrete C interfaces during implementation. This does not
enroll other existing random mechanics or implement hypothetical consumers.

## Behavior

### Root seed lifecycle

Add one shared Wayfarer root record containing:

| Field | Meaning |
| --- | --- |
| `seedLo`, `seedHi` | Proposed 64-bit root represented by two explicit unsigned 32-bit words. |
| `seedFormatVersion` | Layout/initialization contract for the root record. |
| `derivationVersion` | The pinned primitive and byte-encoding contract used for derived results. |
| Initialized marker | Distinguishes a valid all-zero seed from an absent record. |

Use normal Wayfarer shared save ownership, not region-swapped event variables.
Initialize once before generating or displaying seeded content. Acquire a root
from new-game entropy without consuming, reseeding, or replacing the existing
Pokémon RNGs. The bootstrap adapter must document its entropy inputs, work
without a functioning RTC, and not use the player's starter or Trainer ID as
its sole source. Input timing or available timer samples may contribute at this
one boundary; future resolution cannot read them. Do not infer 64 bits of actual
entropy merely from the storage width.

Tests/debug tooling may supply both words directly. All-zero and all-one roots
are valid. There is no required player-facing seed-entry UI. Completing new-game
initialization commits the root and any required initial consumer records to
one valid logical save state before the normal save path or gameplay can use
them. Pin the trainer-growth policy version in that state before any Gym or
trainer strength can resolve; no per-trainer array must be eagerly generated.
The circuit's initial record is root/order-only waiting state,
not a generated roster; its runtime owns competition availability and entry. Do not
expose an initialized circuit under an uninitialized root.

Loading validates the marker and supported versions. Repeated initialization
calls on the same initialized game do not replace the seed. New Game creates a
new root; Continue, region changes, defeat, replay, starting a new circuit
edition, and save copying preserve it.
If a continuing save lacks a valid root, use ordinary invalid/incompatible-save
handling. Never repair it with a fresh seed. Prerelease save migration is not
required, and no seed inference from old fixed-roster saves is required.

### Registered decision identities

Resolve a random word through a pure function of the root and this key:

| Key field | Contract |
| --- | --- |
| `domainId` | Stable explicit numeric namespace for a feature, such as circuit. |
| `decisionId` | Stable named decision within that feature, such as venue order. |
| `decisionVersion` | Rules/interpretation version for this decision only. |
| `entityLo`, `entityHi` | Stable 64-bit authored or explicitly encoded composite identity; zero for a declared decision without a specific entity, including a whole-circuit draw. |
| `occurrenceId` | Explicit 32-bit occurrence; zero for a one-time decision. |
| `drawId` | Semantic 32-bit subdecision, such as a fixed shuffle step or allocation slot. |
| `rejectionIndex` | Internal 32-bit index used only to obtain another word for the same bounded draw. |

Maintain a small checked-in registry of domain/decision constants and their
descriptive names. Numeric IDs are explicit; adding an entry cannot renumber
existing entries. Reject accidental duplicate definitions and never repurpose
a retired ID for unrelated behavior. Reserve circuit domain ID 1, with active
ORDER decision ID 1, ROSTER decision ID 2, and POOL_KIND decision ID 5. Pin each
consumer rules version explicitly: ORDER remains version 1, POOL_KIND becomes version 2 and ROSTER
version 3 for single-competition identity and live entry inputs. IDs 3 (VISITOR_POLICY)
and 4 (LORE_FILTER) are retired and never reused. This does not require support
for unshipped old algorithms. Reserve TRAINER_GROWTH domain ID 2 with
GROWTH_RATE decision ID 1, version 1, independently of the circuit decisions.

Entity identity comes from stable authored content IDs, not pointers, table
positions, localized text, generated map numbering, or the order in which
objects were visited. A fixed algorithm step or authored slot index may be a
`drawId` when the consumer explicitly defines it as a semantic identity. The
same slot must not acquire a different key merely because another loop or UI
query executes first.

Queries have no side effects. There is no global or per-feature persistent
"next random number" cursor, no global query counter, and no hidden attempt
counter. A different key can coincidentally produce the same value; identity
isolation is not a promise that every output is unique.

### Portable derivation

Define `KeyedU32(root, key)` as a deterministic unsigned 32-bit result. Canonical
input bytes, in order, are:

```text
ASCII bytes "WFSD"
u32le(derivationVersion)
u32le(seedLo), u32le(seedHi)
u32le(domainId), u32le(decisionId), u32le(decisionVersion)
u32le(entityLo), u32le(entityHi)
u32le(occurrenceId), u32le(drawId), u32le(rejectionIndex)
```

This is 48 bytes. Encode each word explicitly; do not hash a C struct with
padding, an address, native-width `long`, or host-endian memory. Intermediate
arithmetic must use documented unsigned widths and overflow semantics. There
are no runtime strings or implicit context fields beyond the fixed prefix.

The exact hash/mixing primitive is an implementation choice, not another
player-facing design question. Before any seeded build is enabled, select a
portable, reviewed primitive suitable for these short inputs and the GBA;
document its complete algorithm, constants, output-word extraction, version,
and cross-platform golden vectors. It must mix the complete key and root,
including both seed words. A weak truncation or concatenation that ignores
part of the key is not acceptable. This selection is an explicit implementation
gate; this draft does not claim a primitive is already implemented or pinned.

Do not derive every feature from one sequential PRNG. Adding a decision,
repeating a query, or changing an unrelated caller cannot alter another key's
result. Do not include a build timestamp, whole-repository content hash, current
clock, player RNG state, or global catalog version in the canonical input.

### Selection helpers

Provide these deterministic operations over registered keys:

- A raw unsigned word for a named draw.
- Uniform choice from an explicit positive integer bound.
- Choice from positive integer weights in canonical candidate order.
- A shuffle using consumer-declared semantic draw IDs.

For a bound `n` in 1 through `UINT32_MAX`, start `rejectionIndex` at zero.
Compute `limit = 2^32 - (2^32 mod n)` with a wide intermediate. Resolve `x` from
`KeyedU32`; if `x >= limit`, increment only the internal rejection index and
try again. Return `x mod n` on acceptance. Detect index overflow as an explicit
failure. Rejections never advance an occurrence, change `drawId`, or consume
another decision's output. This avoids adding modulo bias to the primitive's
word distribution.

Weighted choice validates positive weights and a nonzero total no greater than
`UINT32_MAX`, then draws uniformly below that total and selects the matching
cumulative half-open interval. Exclude zero-weight entries explicitly before
selection. Detect overflow rather than wrapping. Canonically sort candidates
by stable content ID before cumulative selection; source declaration order
must not affect the result.

A consumer defines empty-pool handling: fail validation/initialization, decline
the action without committing an outcome, or use a specifically authored
deterministic fallback. The framework never invents a substitute or falls back
to global randomness. Weights, uniqueness constraints, and eligibility remain
consumer rules; the helper cannot make an infeasible roster feasible.

The first consumer uses keyed draws directly. A future complex decision may
explicitly opt into a decision-local stream initialized from a derived word,
using the existing local SFC32 helpers if their behavior is pinned for that
decision version. Such a stream is transient and confined to one resolution;
its cursor cannot escape into other decisions or become a save-wide sequence.
Extra draws may affect later values inside that decision, so independently
stable subchoices need separate `drawId` values instead. There is no requirement
to implement an additional stream abstraction for the initial circuit.

### Consumer input and commitment contract

Every adopting feature documents:

1. Registered keys, versions, entity identities, and semantic draws.
2. Whether the decision is playthrough-wide, per-entity, or recurring.
3. Candidate set, canonical ordering, weights, and every eligibility input.
4. When those inputs become authoritative and which can intentionally change.
5. Whether the result is recomputed purely or stored, and which fields own it.
6. Reveal/application timing, once-only rewards, and occurrence advancement.
7. Empty/invalid input handling and supported-version behavior.

The same root, key, relevant versions, and inputs reproduce an unresolved
decision even when loading a save from before its first evaluation. Caching a
fresh global random roll only after the first interaction does not meet this
contract: reloading an earlier save would still reroll it.

For outcomes with lasting consequences, commit the complete result to the
feature's own fixed save fields before revealing or applying it. Thereafter
read the committed result. Commit reward/accounting state and any occurrence
advance consistently with the outcome so repeated callbacks cannot duplicate
delivery, skip an event, or award a different result. This is a logical save
transaction, not a demand for a physical autosave after every interaction.
If the player reloads before that transaction was saved, re-derivation from
the unchanged earlier state must yield the same result.

Persist occurrence advancement only for a specified meaningful new event.
Inspecting, accepting/declining the same offer, failed attempts, map re-entry,
load count, wall-clock waiting, and presentation callbacks do not automatically
create a new occurrence. A feature can define a legitimate attempt-based or
calendar-based event only through its own explicit design, including why that
does not undermine its promised reload stability.

Pure, repeatable lookups with no irreversible effects need not allocate saved
results when their inputs are stable and reproducible. A world-fixed decision
must freeze inputs at new game or at a specified milestone. Sampling the same
key against a changing live pool is deterministic but does not freeze the
outcome. Do not hide TR, party, time, currently loaded maps, or query-order
dependencies in a supposedly fixed decision.

When a feature deliberately permits progression or story choices to change
eligibility before commitment, its outcome may change. That is a declared input
change, not a reload reroll. A changed eligible pool cannot be promised to keep
the same unresolved selection merely because the root seed is unchanged.

Use fixed consumer-owned save records. Do not introduce an unbounded key/value
outcome database or store every random word produced by this framework.

### Isolation and versioning

Adding a domain or decision must leave all existing keys unchanged. Changing a
decision's interpretation or draw layout requires its own version change;
unrelated decisions keep their versions. Actual candidate/weight changes can
change an unresolved result and require the owning content validation/version
policy, but do not get mixed into every other key.

Store a committed result's necessary rules/content versions with that consumer's
state so profile and item IDs retain meaning. A changed build cannot reinterpret
an existing result as a different trainer, item, or event. Merely adding an
unrelated table does not require rerolling or invalidating that result.

Changes to the core primitive or byte encoding require a new root
`derivationVersion`; changes to record layout require `seedFormatVersion`.
Unchanged behavior needs no version bump. Unsupported versions use the normal
incompatible-save path and cannot silently regenerate a seed or committed
consumer data. Apply the current prerelease policy rather than building a
general migration or historical-algorithm registry. A public compatibility
baseline would require a separately reviewed policy.

### Trainer growth consumer

Trainer progression uses the shared playthrough root, not a private trainer seed.
Pin growth-policy version 1 at new game before Gym progression is usable. Each
canonical trainer derives a stable percentage with the following independent key:

| Key field | Value |
| --- | --- |
| `domainId` | TRAINER_GROWTH = 2 |
| `decisionId` | GROWTH_RATE = 1 |
| `decisionVersion` | Saved growth-policy version; initial version 1 |
| `entityLo`, `entityHi` | Canonical `characterId` (u32), 0 |
| `occurrenceId`, `drawId` | 0, 0 |

Set `growthPercent = 90 + Uniform(21)` for a uniform integer 90–110 inclusive.
This is one immutable choice per canonical trainer per save, not a badge or event
occurrence. Gym/rival/regional aliases share their canonical identity. Pure lazy
derivation is allowed: requesting the value again produces the same result and
never reallocates it. The key contains no circuit edition, venue, badge count,
history, catalog position, time or player TR. New canonical trainers can derive
values from the same saved policy without disturbing any existing key.

Keep authored baseline TR unchanged. The shared progression authority computes
`badgeTR(B)` with its existing half-up interpolation, then:

```text
authoredGrowth = (badgeTR(B) - baselineTR) + leagueGrowth * C
scaledGrowth = floor((authoredGrowth * growthPercent + 50) / 100)
effectiveTR = min(80, baselineTR + scaledGrowth)
```

Scale combined badge/first-clear growth and round once. At B=C=0 the baseline is
unchanged; 100% exactly reproduces the previous formula. Use wide integer
intermediates. World Gyms and circuit entries consume the same percentage with
actual current milestones. An active competition retains its captured plans.
The accepted initial range still requires balance evidence alongside provisional
numeric curves, stage transitions and role bands.

Save the growth-policy version once with playthrough state. A committed battle
plan records its canonical percentage and policy version, or sufficient validated
references to reproduce them. Verify the percentage against root and policy;
corruption or an unsupported policy cannot trigger a modifier reroll. Growth
policy is independently versioned: adding this consumer changes neither root
encoding/derivation nor existing ORDER/POOL_KIND/ROSTER rules or raw values.
Changed seed-dependent ratings/stages can legitimately change new roster
eligibility under the same raw circuit draws. Every supported heterogeneous
90–110% assignment must preserve home-only feasibility; endpoint-only tests or
simulations do not establish coverage through intermediate stage transitions.

### Circuit as the first consumer

The circuit uses domain 1 and three independently keyed decisions. `editionId`
and each venue's competition ordinal are unsigned 32-bit values beginning at 1.
A circuit edition is a three-venue traversal; a competition is one event at the
current venue. These are distinct lifetimes and identities:

| Decision | Rules version | Entity words / occurrence | Draw identity |
| --- | ---: | --- | --- |
| ORDER, ID 1 | 1 | `entityLo = entityHi = 0`; occurrence = `editionId` | Fisher–Yates step index 2, then 1 |
| POOL_KIND, ID 5 | 2 | `entityLo = venueId`, `entityHi = editionId`; occurrence = `competitionOrdinal` | Allocation slot 0–4; `Uniform(100)` selects home on 0–84, visitor on 85–99 when both buckets exist |
| ROSTER, ID 2 | 3 | `entityLo = venueId`, `entityHi = editionId`; occurrence = `competitionOrdinal` | Allocation slot 0–4 |

Stable venue IDs remain Indigo = 1, Masters = 2, Hoenn = 3. The explicit low/high
word assignment includes edition, venue and event ordinal without extending the
48-byte derivation encoding. Preserve numeric decision/domain IDs. Save a new
explicit competition-state schema discriminator; old complete-edition snapshots
cannot be interpreted as single-event records. Follow prerelease save policy,
without migrations or unshipped historical algorithms.

At new game, resolve the root and edition 1's entire ORDER. Save a valid waiting
state with no active event, no participants for any venue, zero current-circuit
progress, empty completed-event history and no lifetime first clears. Show the
saved itinerary if desired; merely inspecting it never generates participants.
A later edition derives its order at the runtime-owned rollover. Neither initial
construction nor rollover creates future venue lineups.

The runtime must adopt an explicit persisted competition-availability rule before
shipping. That rule allocates each meaningful event identity once. Registering,
reopening menus, previewing, cancellation, duplicate callbacks and reloading
cannot allocate ordinals or skip an event. No calendar, wait duration or cadence
is chosen by this framework. Signup/entry gates remain undecided under D4; possible first
entries around 8/16/24 badges are tentative rather than required. D6 owns
competition availability/waiting; no real-time/calendar choice is approved.

On entering an available competition at the current venue, capture actual live
`B_entry`, lifetime clear mask `L_entry`, immutable pre-entry participation history
and fixed canonical growth percentages/policy plus all content/progression/
band/resolver versions. Validate modifiers against the shared root. Use world point
`(B_entry, popcount(L_entry))`; never project later badges or clears. Resolve each
trainer's effective TR and stage/profile before role/content eligibility. The
[pool spec](circuit-trainer-pool.md) owns exact allocation, home membership,
weights, feasibility and presentation sorting. At the same world point all venues
share the same ordered disjoint contender/elite/headliner bands; endpoints remain
pending D3. Travel position, edition identity and competition ordinal do not add
strength. Actual milestones earned between events intentionally affect new entries.

Partition TR/content-qualified candidates by authored multi-home membership.
When both buckets exist, use the conditional 85/15 category draw, independent of
bucket size and weights. No eligible visitor means home without evaluating a
POOL_KIND key; missing eligible home is invalid content, never permission to use
visitors or widen bands. Prove home-only two-contender/two-elite/one-headliner
feasibility at every supported entry `(B,C)` including intermediate badges, heterogeneous 90–110% modifiers, stage
transitions and saturation, not only projected or 24-badge points. Endpoint-only
modifier checks and sampled roots cannot prove full-range feasibility. Visiting does not reduce TR
or battle strength. No visitor quota, mandatory guest or lore exclusion is added.

Select within the chosen category using positive proposed D5 weights: base 1,
times current-circuit factor `[16,4,1]` for appearances at 0/1/2 distinct earlier
other venues, times prior-venue factor 1 for inclusion in this venue's immediately
previous completed competition or 2 otherwise. Completed competitions include
WIN and LOSS. Current-venue appearances are excluded from the circuit factor;
the latest event there owns its returner penalty. Bounded per-venue character
unions record earlier completed appearances, counting each trainer once per
venue even across repeated losses. No full historical party archive is required.
Unseen venue history gives every candidate factor 2. Reset circuit unions at
rollover and retain latest per-venue completed fields. These factors remain
provisional soft preferences, not exclusions or turnover quotas.

Allocate only this event's five slots in priority `[4,2,3,0,1]`, removing already
selected canonical people within the event. History factors stay immutable during
allocation. Sort contender/elite pairs by saved TR then canonical ID; leave the
headliner in slot 4. POOL_KIND/ROSTER draw IDs refer to allocation before sorting,
so a displayed room's category cannot be validated by that room number's draw.
Pure verification replays canonical allocation against captured pre-entry inputs,
compares all five plans, and never replaces committed content.

Atomically save one active competition: full identity, five participants, captured
actual world point, fixed growth percentages/policy, effective TR, stages/profiles
and versions, pre-entry history,
and runtime progress. Freeze participants/strength throughout this event and save
reconstruction, even if live milestones change. Future venues have no participant
snapshot. Loading an active event reads it; loading before entry reproduces it
from identical available identity and inputs. No global RNG or unrelated feature
can perturb those keys or inputs. Different milestone/history inputs can legitimately
change an unresolved later event, while its raw category word remains key-stable.

On loss, atomically finish the event, commit its completed-field history, retire
its active plan and enter waiting at the same current venue. There is no immediate
retry of that competition. Only the separately adopted availability transition
can allocate a new ordinal there. On win, commit equivalent history and lifetime
first-clear accounting, retire the event, and advance to the next venue. Circuit
completion permits a later edition according to the runtime contract; losses do
not skip venues or advance edition IDs. Reject overflow without wrapping.
Repeated callbacks cannot update history/rewards twice. Cancellation before entry
cannot manufacture a fresh identity. A new event can repeat all five trainers;
never reroll to force novelty.

Persist only bounded latest completed fields, current-circuit participation sets,
one active event with captured inputs, availability/ordinal state, edition order
and progression. Keep retired identity markers needed to prevent reuse; do not
retain old full teams. Initial order-only, available before entry, waiting and
completed states are valid without an active snapshot, provided their lifecycle
and schema invariants validate. Only an active state requires the full snapshot.
Missing/corrupt data in an active state is invalid save data, never permission to
regenerate participants or replace inputs. Histories
change only through completed-event transactions; loading/UI queries do not
mutate them. The root is stored once, not copied into event records.

Raw ORDER/category keys are isolated from catalogs/history; selected outcomes
are deliberately coupled to declared eligibility and completed participation.
There is no shared draw cursor or future-venue joint allocation. Badge/first-clear
growth uses deterministic milestone arithmetic and the fixed independently keyed
trainer percentage; milestones do not request a new percentage draw. Player
party/TR/XP, event ordinals and edition counts supply no strength bonus.

### Existing Pokémon RNG boundary

Framework evaluation cannot read, advance, reseed, or replace `gRngValue`,
`gRng2Value`, their global draw functions, or randomizer state. Global RNG helper
names alone do not prove locality; audit implementations and interrupts.
New-game root acquisition obeys the same no-consumption/no-reseed rule.

Keep existing battle rolls, capture rolls, ordinary encounters, Pokémon
generation, existing random item rolls, and existing randomizer behavior on
their current paths. A seeded circuit participant can enter a battle whose
damage and capture-related mechanics still use normal RNG. No whole-battle
deterministic replay guarantee is introduced.

### Integration and validation

Review [new-game initialization](../../game/src/new_game.c),
[shared persistence](../../game/src/wayfarer_persistence.c),
[save structures](../../game/include/global.h), and the
[runtime foundation](wayfarer-runtime-foundation.md). Existing
[local/global RNG interfaces](../../game/include/random.h) and
[implementations](../../game/src/random.c) are reference surfaces, not an
instruction to replace their RNG. Allocate a Wayfarer-owned module and records;
avoid introducing this seed into standalone products.

Required implementation evidence:

- Pin primitive, encoding, and selection vectors for zero/all-one roots,
  high-word-only changes, all key fields, endian-sensitive patterns, exact
  interval boundaries, rejection retries, and invalid bounds/weights. Compare
  host tooling and game C results byte for byte.
- Verify repeated/reordered queries and newly added unrelated keys return the
  same existing values, without relying on a promise of collision-free outputs.
  Reorder candidate source declarations and verify canonical-order stability.
- Snapshot both global RNG states around root creation and deterministic queries;
  prove no extra framework RNG advances, including realistic initialization and
  interrupt behavior. Existing RNG control tests must retain their behavior.
- Use a minimal test-only one-time consumer and recurring consumer to prove
  reload before first reveal, reload after commitment, duplicate callbacks,
  invalid input, and meaningful occurrence transitions. Do not ship placeholder
  reward or quest systems solely for framework testing.
- Demonstrate an intentionally changed eligibility input can affect an
  unresolved result, while a committed result stays unchanged. Demonstrate
  new noneligible/unrelated content does not perturb it.
- Pin TRAINER_GROWTH key/percentage vectors, `Uniform(21)` boundaries/rejection,
  all integer percentages, canonical aliases and both root words. Verify policy
  is pinned before Gyms, lazy repeated derivation stays stable and new trainer
  keys do not alter old values or any raw circuit key. Cover baseline preservation,
  neutral 100% equivalence, combined-growth rounding once, saturation, Gym/circuit
  agreement and root-validated snapshots. Test heterogeneous full-range modifiers
  and stage eligibility in content feasibility; simulations are balance evidence
  rather than proof. Unsupported policy or corrupt references cannot reroll.
- For the circuit, test cross-feature calls, canonical source reordering, UI
  queries and save/load. Pin all six orders and full event keys including both
  entity words, event ordinals 1/2, editions 1/2 and boundary values. Changing
  catalog/history cannot change raw ORDER/category words for unchanged keys.
  Verify TR/stage checks before home classification, multi-home membership,
  category boundaries 84/85, no category call with empty visitors and invalid
  content with missing homes. Prove home-only feasibility over all supported
  entry badge/clear points, including saturation. Category chance is not an
  individual inclusion probability, quota or per-room guarantee after sorting.
- Pin allocation `[4,2,3,0,1]`, canonical weighted intervals, sorting, no within-event
  duplicates and legal across-event/venue repeats. Capture previous completed
  field and current-circuit unions before entry. Test WIN/LOSS history, factors
  0/1/2, absence resetting returner penalties and many losses without archive
  growth. Declared earlier-event history can change later outcomes; it cannot
  alter raw category keys or exclude every candidate through rotation weights.
- Exercise new-game root/order-only state, actual entry snapshots, no future
  lineups and identical reconstruction during an active event. Reload before
  entry must reproduce the same available competition under unchanged inputs.
  Changing milestones before a later entry can legitimately change teams and
  eligibility; changing them during an active event cannot change saved plans.
- Verify loss commits history and retires the event, returns to waiting at the
  same venue and offers no immediate retry. Only the adopted availability rule
  creates a later ordinal. Menus, cancellation, previews, duplicate callbacks,
  failures and reloads cannot allocate identities. Win advances venue traversal;
  edition rollover resets circuit sets, retains latest venue fields and preserves
  lifetime rewards. Test atomic transactions, snapshot-only verification,
  ordinal/edition overflow, no identity reuse and corruption rejecting regeneration.
- Reject missing roots and unsupported versions without a reroll. Verify valid
  all-zero roots, initialized-record reuse, copied saves, and genuine New Game.
- Measure root/consumer save size, transient RAM/stack, ROM cost, and runtime
  generation cost on the target. Avoid large static buffers and per-frame
  derivation. Build Wayfarer and verify unaffected standalone configurations.

The exact primitive selection and its published vectors are required before
enabling this framework. Structural determinism is not proof of good content
variety or gameplay balance; each consumer retains its own generation audit
and playtesting requirements.
