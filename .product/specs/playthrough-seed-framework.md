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
them. The circuit's initial record is root/order-only pre-registration state,
not a generated roster; its runtime owns first eligible registration. Do not
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
| `entityLo`, `entityHi` | Stable 64-bit content identity; zero for a declared decision without a specific entity, including a whole-circuit draw. |
| `occurrenceId` | Explicit 32-bit occurrence; zero for a one-time decision. |
| `drawId` | Semantic 32-bit subdecision, such as a fixed shuffle step or allocation slot. |
| `rejectionIndex` | Internal 32-bit index used only to obtain another word for the same bounded draw. |

Maintain a small checked-in registry of domain/decision constants and their
descriptive names. Numeric IDs are explicit; adding an entry cannot renumber
existing entries. Reject accidental duplicate definitions and never repurpose
a retired ID for unrelated behavior. Reserve circuit domain ID 1, with active
ORDER decision ID 1, ROSTER decision ID 2, and POOL_KIND decision ID 5. Pin each
consumer rules version explicitly: ORDER/POOL_KIND remain version 1 and ROSTER
is version 2 for projected progression-aware inputs. IDs 3 (VISITOR_POLICY)
and 4 (LORE_FILTER) are retired and never reused. This does not require support
for unshipped old algorithms.

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
| POOL_KIND, ID 5 | Stable venue ID: Indigo = 1, Masters = 2, Hoenn = 3; occurrence = editionId | Battle slot 0–4; when both eligible buckets exist, `Uniform(100)` chooses home on 0–84, visitor on 85–99 |
| ROSTER, ID 2 | Stable venue ID: Indigo = 1, Masters = 2, Hoenn = 3; occurrence = editionId | Battle slot 0–4 |

The [pool spec](circuit-trainer-pool.md) owns allocation order, TR eligibility,
authored `homeLeagues`, rotation weights, feasibility, and final sorting.
Use active ORDER, POOL_KIND, and ROSTER rules versions. Registration captures
`B_reg` (global badges), `L_reg` (lifetime venue-clear mask), predecessor history,
and content/progression/band/resolver versions. For stop i, use `B_i = B_reg`
and `C_i = popcount(L_reg union earlier scheduled venue IDs)`. Resolve each
candidate's effective personal TR and stage/profile before world point role-band
eligibility, content validity, and within-venue canonical uniqueness; home membership never
admits an unsuitable trainer. Partition the eligible candidates into home and
visitor buckets using multi-home membership with explicit rationale. With both
buckets nonempty, POOL_KIND selects one using the initial conditional 85/15
tuning above. No eligible visitor means home without a POOL_KIND draw. Missing
eligible home is an invalid catalog; content must prove home-only two-contender,
two-elite, one-headliner feasibility for every venue at every supported point,
including C 0–3 and TR saturation. Prove variety separately.

Apply the positive repeat/history-weighted ROSTER draw only inside the chosen
bucket. Home candidates keep those penalties, and visiting does not change TR
or battle strength at the same projected world point. There is no visitor quota/cap or per-outsider exclusion
draw. The 85/15 chance applies to bucket choice when both exist, not an individual
trainer or guaranteed field share. Pure raw POOL_KIND calls remain defined even
when fallback means generation does not need to evaluate that key.

All venues at the same world point use the same authored ordered disjoint role
TR bands: contender slots 0–1, elite slots 2–3, headliner slot 4. Bands are
indexed by badges and lifetime clear count; endpoints remain unresolved D3
tuning. Projected first clears can change successive stop points. Travel
position and edition identity alone do not choose difficulty. The pool spec allocates battle-slot
priority `[4,2,3,0,1]`, and for each slot visits venues in saved travel order.
Sort only within contender/elite pairs by TR then canonical ID. Headliner
appointment is title-agnostic. Preserve existing numeric domain/decision/entity
IDs. ORDER/POOL_KIND rules remain version 1; ROSTER rules are version 2. Record
an explicit registration/schedule schema version for progression-aware inputs;
do not reinterpret a saved version.
Both POOL_KIND and ROSTER draw IDs refer to allocation slots before this sort;
85/15 is not a per-room probability after sorting. Saved-schedule verification
reproduces canonical allocation and sorting from the same inputs for comparison
only, never to replace a committed result. A room index must not be compared
directly with a POOL_KIND draw to validate its occupant's category.

Feasibility checks are pure and consume no keyed draws. Raw ORDER and POOL_KIND
values do not depend on catalog entries, their versions, or history. Changing
eligibility or earlier allocations can change which buckets exist and whether
fallback applies. ROSTER uses only the chosen eligible bucket,
canonical scheduled-occurrence counts within this edition, and
immediately prior completed-edition IDs at the same venue. Proposed D5 weight
is base 1 times within-edition factor `[16,4,1]` for scheduled counts 0/1/2,
times prior-venue factor 1 for a returner or 2 otherwise. Empty edition-1 history
creates no returning penalty. Approximately two returners and three new trainers
is a soft goal, not a quota, exclusion, or reroll condition. Choose five
distinct canonical characters per lineup; a character selected there remains
eligible for every other league whose role TR/content checks they pass. There is
no global used-character exclusion, but rotation weights softly discourage
repeat appointments. ROSTER decisions are jointly dependent through scheduled
counts/history: changing relevant candidates at one venue can affect later
choices elsewhere. Semantic keyed calls remain pure; there is no shared RNG
cursor, and unrelated features cannot perturb those inputs or draws.
Neither category selection nor order can be redrawn to repair a roster shortage.

At new game, resolve and persist edition 1 ORDER with the shared root and a
valid pre-registration discriminator. There is no roster, rating snapshot, active
run, clear result, or completed count; history is empty. The saved order may be
shown, but admission and roster preview are unavailable. First eligible
registration snapshots progression inputs and generates the whole field,
including every identity, projected point, effective TR, and stage/profile with
versions. The 24-badge first-registration threshold remains D4's proposed default.
These constructor/registration boundaries leave the keyed RNG contract unchanged.

After all three current venues complete and no run or ceremony is pending,
registration stages edition `editionId + 1`, captures current milestone inputs,
and snapshots the outgoing roster IDs by venue as history. Derive the next
complete schedule from those fixed inputs, then atomically commit history,
identity, order, field, snapshots, versions, and fresh edition progress.
Failures preserve the previous state and consume no edition number. Reject
counter overflow without wrapping or replacing the completed edition.

Loading a save from before registration must reproduce the same next edition
under the same root, keys, rules/content versions, registration badge/clear
snapshot, and staged prior-venue history.
Losses, exits, retries, replays,
time, and menu queries cannot advance the edition. Do not expose a separately
generated next-edition preview or allow skipping an unfinished edition. Starting
a new edition retains lifetime progression, unlocks, and first-clear accounting;
the runtime spec owns those transactions and their recovery.

The root lives once in shared persistence; the circuit stores neither another
seed nor a draw cursor. Loading a valid schedule reads it instead of regenerating
it. Unrelated features and ordinary gameplay RNG cannot change it. A registered
state saves the current edition's complete schedule, the immediately previous
completed edition's five character IDs per venue, registration snapshots, and
progression/profile/band/resolver versions alongside runtime metadata. History is empty
for edition 1; thereafter `history.editionId = editionId - 1`. It preserves
authoritative generation inputs, not full old schedules/parties or an unbounded
archive. Validate its schema, canonical references, per-venue uniqueness,
venue mapping, and edition relation; corrupt history cannot silently clear or
redraw. Gameplay, replay, and room defeats do not mutate history. Resolved
current slots use saved projected effective TR and stage/profile versions.
Read-only verification reproduces projection and allocation against registration
inputs, never live world progress. A valid pre-registration state is explicit;
an absent registered roster is corruption, never an initialization opportunity.

Different edition identities may produce the same order or participants. Never
reroll to force novelty or infer strength growth from an occurrence number.
Keep raw ORDER and POOL_KIND semantics unchanged unless their own rules change.
Progression-aware ROSTER version 2 and saved schemas carry their revised
meaning explicitly. Follow prerelease save policy; no historical compatibility
or reroll migration is required. Badge/first-clear growth is deterministic and
uses no keyed draws; neither player party/TR/XP nor edition number adds strength.

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
  acceptance. Changing catalog/history cannot alter raw ORDER or POOL_KIND
  values for the same root, edition, and respective rules versions. Test role
  TR/content/uniqueness checks before home/visitor partitioning, authored
  multi-home membership, and category boundary draws 84/85. With no eligible
  visitors, select home without a POOL_KIND call; with no eligible home, reject
  content instead of substituting visitors or widening TR bands. Prove home-only
  role feasibility at every supported world point including C 0–3/saturation,
  and permit multiple visitors without quotas or caps. Test projected milestone
  inputs and stage/TR eligibility before classification.
  Distinguish input-dependent bucket fallback and joint roster outcomes from
  raw-key isolation. Verify weighted choice cannot select outside its bucket.
  Permit repeated characters across qualifying leagues and reject duplicates
  within a lineup. Test positive repeat/history weights: earlier assignments
  may change later weights but cannot remove eligible people at another venue.
  Pin the canonical joint traversal and role counts; do not assert cross-venue
  roster independence.
- Exercise the circuit's confirmed recurring lifecycle: root/edition 1 ORDER at
  new game, first roster registration at the adopted gate, registration for
  edition 2 after completion, and reloads both before and after
  registration. Pin vectors that include edition identity; verify it is mixed
  into ORDER, POOL_KIND, and ROSTER without requiring every output to differ.
  Failed generation, duplicate callbacks, unfinished editions, and overflow
  cannot consume or skip an edition. Verify atomic prior-venue history updates,
  empty edition-1 history, predecessor identity, and failure preserving both
  old schedule and old history. Verify unchanged root/global RNG state and
  preservation of lifetime rewards and unlocks across rollover. Verify explicit
  valid pre-registration state, snapshot-only reproduction, and frozen complete
  editions through live badge/clear changes, retries, and replays. Corruption
  cannot invoke generation or replace snapshot inputs.
- Reject missing roots and unsupported versions without a reroll. Verify valid
  all-zero roots, initialized-record reuse, copied saves, and genuine New Game.
- Measure root/consumer save size, transient RAM/stack, ROM cost, and runtime
  generation cost on the target. Avoid large static buffers and per-frame
  derivation. Build Wayfarer and verify unaffected standalone configurations.

The exact primitive selection and its published vectors are required before
enabling this framework. Structural determinism is not proof of good content
variety or gameplay balance; each consumer retains its own generation audit
and playtesting requirements.
