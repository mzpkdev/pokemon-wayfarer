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
them. Do not expose an initialized circuit under an uninitialized root.

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
| `entityLo`, `entityHi` | Stable 64-bit content identity; zero for a declared decision without a specific entity, including a whole-circuit draw. |
| `occurrenceId` | Explicit 32-bit occurrence; zero for a one-time decision. |
| `drawId` | Semantic 32-bit subdecision, such as a fixed shuffle step or allocation slot. |
| `rejectionIndex` | Internal 32-bit index used only to obtain another word for the same bounded draw. |

Maintain a small checked-in registry of domain/decision constants and their
descriptive names. Numeric IDs are explicit; adding an entry cannot renumber
existing entries. Reject accidental duplicate definitions and never repurpose
a retired ID for unrelated behavior. Reserve circuit domain ID 1, with ORDER
decision ID 1, ROSTER decision ID 2, and LORE_FILTER decision ID 4, initially
at version 1 each. Decision ID 3 is retired from the earlier visitor-quota draft
and is not reused. This does not require support for an unshipped old algorithm.

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

### Circuit as the first consumer

The circuit uses domain 1 and three independently keyed decisions. Its saved
`editionId` is an unsigned 32-bit value starting at 1 and is the `occurrenceId`
for all three decisions:

| Decision | Entity / occurrence | Draw identity |
| --- | --- | --- |
| ORDER, ID 1 | Entity zero; occurrence = editionId | Fisher–Yates step index, 2 then 1 |
| LORE_FILTER, ID 4 | Canonical character identity; occurrence = editionId | Venue ID: Indigo = 1, Masters = 2, Hoenn = 3; after TR eligibility, an unaffiliated candidate is dropped when `Uniform(2) == 0` |
| ROSTER, ID 2 | Entity zero; occurrence = editionId | `(position << 8) \| temporarySlot`, position 3–1 and slot 0–4 |

The [pool spec](circuit-trainer-pool.md) owns allocation order, TR eligibility,
authored league affiliations, rating weights, feasibility, and final sorting.
Use explicit ORDER, LORE_FILTER, and ROSTER rules versions. Evaluate TR first;
a lore affiliation cannot admit a trainer whose rating is outside the band.
Eligible affiliated trainers pass the lore step without a draw. Other eligible
trainers face one 50% drop decision per character/venue/edition; a removed trainer
may still survive for another venue. The gate is not a 50% appearance chance,
a visitor quota, or a decision to remove a character globally.

Feasibility checks are pure and consume no keyed draws. ORDER and a given pair's
raw LORE_FILTER value do not depend on unrelated catalog entries or their
versions. Changing that pair's TR eligibility or authored affiliation can
legitimately change whether the lore draw applies. ROSTER uses the retained
candidates and rating weights through the consumer algorithm. Choose five
distinct canonical characters per lineup; a character selected there remains
eligible for every other league whose TR and lore checks they pass. There is
no global used-character set or cross-league repeat penalty. Choices within
one lineup affect its remaining slots, without depleting another venue's pool.
Neither lore gates nor order can be redrawn to repair a roster shortage.

Persist edition 1 with its resolved order and fifteen character/profile/TR
selections at new game through the [runtime spec](seeded-league-circuit.md).
After all three leagues are completed and no run or ceremony is pending, the
player can register for edition `editionId + 1`. Derive its complete schedule
under that occurrence, then commit the new edition identity, schedule, and fresh
edition progress together. Do not consume an edition number if generation or
registration fails. Reject counter overflow without wrapping or replacing the
previous completed edition.

Loading a save from before registration must reproduce the same next edition
under the same root, keys, rules, and content. Losses, exits, retries, replays,
time, and menu queries cannot advance the edition. Do not expose a separately
generated next-edition preview or allow skipping an unfinished edition. Starting
a new edition retains lifetime progression, unlocks, and first-clear accounting;
the runtime spec owns those transactions and their recovery.

The root lives once in shared persistence; the circuit stores neither another
seed nor a draw cursor. Loading a valid schedule reads it instead of regenerating
it. Unrelated features and ordinary gameplay RNG cannot change it. Only the
current edition's complete schedule is required in the save; recurrence does
not justify an unbounded archive of prior schedules.

Different edition identities may produce the same order or participants. Never
reroll to force novelty or infer strength growth from an occurrence number.
The three decision protocols remain version 1 in this unimplemented draft;
no previous circuit-key version or prerelease-save migration is required.

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
- For the circuit, test cross-feature calls before/between/after generation,
  save/load and UI queries, plus all six venue orders and existing circuit
  acceptance. Changing unrelated roster content cannot alter ORDER or an
  unchanged character/venue pair's LORE_FILTER draw for the same root, edition,
  and respective rules versions. Test TR rejection before lore, affiliated bypass,
  the 50% drop boundary, and venue-local rather than global exclusion.
  Permit repeated characters across qualifying leagues, reject duplicates
  within a lineup, and prove generating one lineup cannot deplete another.
- Exercise the circuit's actual recurring lifecycle: edition 1 at new game,
  registration for edition 2 after completion, and reloads both before and after
  registration. Pin vectors that include edition identity; verify it is mixed
  into ORDER, LORE_FILTER, and ROSTER without requiring every output to differ.
  Failed generation, duplicate callbacks, unfinished editions, and overflow
  cannot consume or skip an edition. Verify unchanged root/global RNG state and
  preservation of lifetime rewards and unlocks across rollover.
- Reject missing roots and unsupported versions without a reroll. Verify valid
  all-zero roots, initialized-record reuse, copied saves, and genuine New Game.
- Measure root/consumer save size, transient RAM/stack, ROM cost, and runtime
  generation cost on the target. Avoid large static buffers and per-frame
  derivation. Build Wayfarer and verify unaffected standalone configurations.

The exact primitive selection and its published vectors are required before
enabling this framework. Structural determinism is not proof of good content
variety or gameplay balance; each consumer retains its own generation audit
and playtesting requirements.
