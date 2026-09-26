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
The circuit's initial record holds only edition 1's order, not a generated
field; its runtime owns entry. Do not expose an initialized circuit under an
uninitialized root.

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
| `drawId` | Semantic 32-bit subdecision, such as a fixed shuffle step or the battle slot index being allocated. |
| `rejectionIndex` | Internal 32-bit index used only to obtain another word for the same bounded draw. |

Maintain a small checked-in registry of domain/decision constants and their
descriptive names. Numeric IDs are explicit; adding an entry cannot renumber
existing entries. Reject accidental duplicate definitions and never repurpose
a retired ID for unrelated behavior. Reserve circuit domain ID 1, with active
ORDER decision ID 1, ROSTER decision ID 2, and POOL_KIND decision ID 5. Pin each
consumer rules version explicitly: ORDER remains version 1, POOL_KIND is version 3
and ROSTER version 4 for one competition per (edition, venue). IDs 3
(VISITOR_POLICY) and 4 (LORE_FILTER) are retired and never reused. Reserve
TRAINER_GROWTH domain ID 2 with GROWTH_ARC decision ID 1, decision version 2
(separate from the pinned growth-policy version), independently of the circuit
decisions. This does not require support for unshipped old algorithms.

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
Pin the growth-policy version at new game before any Gym or trainer strength
can resolve. Each canonical trainer chooses one growth arc from its authored
`allowedArcs` with this independent key:

| Key field | Value |
| --- | --- |
| `domainId` | TRAINER_GROWTH = 2 |
| `decisionId` | GROWTH_ARC = 1 |
| `decisionVersion` | 2 (draw layout; not the growth-policy version) |
| `entityLo`, `entityHi` | Canonical `characterId` (u32), 0 |
| `occurrenceId`, `drawId` | 0, 0 |

```text
arc = allowedArcs[Uniform(len(allowedArcs), growthArcKey)]
```

Sort `allowedArcs` by stable arc ID before indexing, so source order cannot
change the result. This is one immutable choice per canonical trainer per save,
not a badge or event occurrence. Gym, rival, and regional aliases share their
canonical identity. Pure lazy derivation is allowed: requesting the arc again
produces the same result, and no per-trainer array is eagerly saved. The key
contains no circuit edition, venue, badge count, history, catalog position,
time, party, or player TR. New canonical trainers derive arcs under the same
saved policy without disturbing any existing key.

Decision ID 1 keeps its number under the new name; decision version 2 replaces
the retired 90–110% percentage interpretation of version 1, which is never
reused. The decision version changes only with the draw layout. The separately
saved growth-policy version covers arc tuples, allowed-arc lists, bias, and
headroom: tuple or bias changes bump only the policy version, and an
allowed-list change bumps it too while leaving the key unchanged, so the
changed list changes the outcome by design.
The [progression spec](trainer-world-progression.md) owns arc tuples, standing,
and level arithmetic. A committed battle plan records the arc and policy version,
or validated references that reproduce them. Verify them against root and policy;
corruption or an unsupported policy cannot trigger an arc reroll. Adding this
consumer changes neither root derivation nor any circuit key. Arcs can
legitimately change which trainers fit a league role window under the same raw
circuit draws.

### Circuit as the first consumer

The circuit uses domain 1 and three independently keyed decisions. `editionId`
is an unsigned 32-bit value beginning at 1. An edition is a three-venue
traversal, and each (edition, venue) pair has exactly one competition:

| Decision | Rules version | Entity words | Occurrence | Draw identity |
| --- | ---: | --- | --- | --- |
| ORDER, ID 1 | 1 | `entityLo = entityHi = 0` | `editionId` | Fisher–Yates step index 2, then 1 |
| POOL_KIND, ID 5 | 3 | `entityLo = venueId`, `entityHi = editionId` | 0 | Battle slot index being allocated (0–4, before final ordering) |
| ROSTER, ID 2 | 4 | `entityLo = venueId`, `entityHi = editionId` | 0 | Battle slot index being allocated (0–4, before final ordering) |

Stable venue IDs remain Indigo = 1, Masters = 2, Hoenn = 3. POOL_KIND and ROSTER
drop the retired competition ordinal; their version bumps record the changed
identity and inputs. At Indigo and Hoenn, POOL_KIND runs `Uniform(100)` only
when both home and visitor pools are nonempty: 0–84 selects home, 85–99 visitor.
Masters is an open invitational with one pool and never evaluates POOL_KIND.
A slot resolved by the deterministic nearest-standing fallback consumes no
POOL_KIND or ROSTER draw. The draw ID is the zero-based battle slot index being
allocated, before the five are put in final ascending-standing order. Save a competition-state schema discriminator;
older ordinal-based records are invalid under prerelease save policy.

At new game, resolve the root and edition 1's entire ORDER and save a state with
no active competition and no participants. Inspecting the itinerary never
generates a field. Later editions derive ORDER at rollover.

Entry is idempotent per (edition, venue). It captures the world point and
progress index, each candidate's arc and standing, versions, and the participation history the
[pool spec](circuit-trainer-pool.md) needs, then resolves all five slots and
commits them atomically before reveal. A loss leaves that competition active and
unchanged, records no participation, and retries the same frozen field. Only a
win commits history and first-clear accounting and advances the venue. Loading
before entry reproduces the same field from identical inputs; loading afterward
reads the saved plan. Missing or corrupt active data is an invalid save, never a
reason to regenerate. The root is stored once, not copied into event records.

Raw ORDER and POOL_KIND words are isolated from catalogs, arcs, and history;
selected outcomes are deliberately coupled to declared eligibility and won-event
participation. There is no shared draw cursor. The
[circuit spec](seeded-league-circuit.md) and pool spec own signup, role windows,
selection, rotation weights, fallback, and saved schema.

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
- Pin GROWTH_ARC key vectors for both root words, canonical aliases, every
  allowed-arc list length, `Uniform` boundaries and rejection, and canonical
  arc ordering under reordered source lists. Verify policy is pinned before
  Gyms, lazy repeated derivation is stable, new trainer keys do not alter
  existing arcs or any raw circuit key, and unsupported policy or corrupt
  references cannot reroll.
- For the circuit, pin all six orders and full POOL_KIND/ROSTER keys including
  both entity words, editions 1/2, venues 1–3, slots 0–4, and boundary values.
  Test category boundaries 84/85, no POOL_KIND call at Masters, with an empty
  pool, or on fallback, and that catalog, arc, or history changes cannot change
  raw ORDER/POOL_KIND words for unchanged keys. Cover cross-feature calls,
  canonical source reordering, UI queries, and save/load.
- Exercise new-game order-only state, idempotent entry per (edition, venue),
  atomic commit before reveal, and identical reconstruction during an active
  competition. Verify a loss keeps the same field across retries, reloads, and
  leaving and returning, and records no participation; a win advances the venue
  once, even under duplicate callbacks. Test edition overflow and corruption
  rejecting regeneration.
- Reject missing roots and unsupported versions without a reroll. Verify valid
  all-zero roots, initialized-record reuse, copied saves, and genuine New Game.
- Measure root/consumer save size, transient RAM/stack, ROM cost, and runtime
  generation cost on the target. Avoid large static buffers and per-frame
  derivation. Build Wayfarer and verify unaffected standalone configurations.

The exact primitive selection and its published vectors are required before
enabling this framework. Structural determinism is not proof of good content
variety or gameplay balance; each consumer retains its own generation audit
and playtesting requirements.
