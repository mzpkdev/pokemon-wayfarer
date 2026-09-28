# Playthrough seed framework

PRD: [Playthrough-seeded variation](../prds/playthrough-seeded-variation.md)
Implemented: No
Design status: Parked: no v0 consumer. Kept as the implementation contract for
the first approved seeded feature; nothing is built until then.

## Scope

Own root-seed initialization and persistence, stable decision identities,
portable derivation, deterministic selection helpers, isolation from existing
RNG, and the adoption contract for Wayfarer features. The API names below
describe required behavior, not functions present in the game. This does not
enroll existing random mechanics.

## Root seed lifecycle

One shared Wayfarer root record:

| Field | Meaning |
| --- | --- |
| `seedLo`, `seedHi` | 64-bit root as two explicit unsigned 32-bit words. |
| `seedFormatVersion` | Layout and initialization contract for the record. |
| `derivationVersion` | Pinned primitive and byte-encoding contract. |
| Initialized marker | Distinguishes a valid all-zero seed from an absent record. |

Use normal Wayfarer shared save ownership. Initialize once at new game, before
any seeded content resolves, from new-game entropy without consuming,
reseeding, or replacing the Pokémon RNGs. The bootstrap documents its entropy
inputs, works without an RTC, and does not rely on the starter or Trainer ID
alone. Tests and debug tooling may supply both words; all-zero and all-one
roots are valid.

Repeated initialization never replaces the seed. New Game creates a new root;
Continue, region changes, defeat, and save copying preserve it. A continuing
save without a valid root uses invalid-save handling and is never repaired
with a fresh seed. Prerelease migration is not required.

## Decision identities

A random word is a pure function of the root and this key:

| Key field | Contract |
| --- | --- |
| `domainId` | Stable numeric namespace for a feature. |
| `decisionId` | Stable named decision within that feature. |
| `decisionVersion` | Rules version for this decision only. |
| `entityLo`, `entityHi` | Stable 64-bit authored identity; zero when there is no entity. |
| `occurrenceId` | Explicit 32-bit occurrence; zero for a one-time decision. |
| `drawId` | Semantic 32-bit subdecision, such as a fixed shuffle step. |
| `rejectionIndex` | Internal index used only to retry the same bounded draw. |

Keep a checked-in registry of explicit numeric IDs. Adding an entry never
renumbers others, and a retired ID is never reused. No IDs are reserved in v0.
Entity identity comes from stable authored IDs, never pointers, table
positions, text, or visit order.

Queries have no side effects: no persistent cursor, query counter, or hidden
attempt counter.

## Portable derivation

`KeyedU32(root, key)` returns an unsigned 32-bit word from these 48 canonical
bytes:

```text
ASCII bytes "WFSD"
u32le(derivationVersion)
u32le(seedLo), u32le(seedHi)
u32le(domainId), u32le(decisionId), u32le(decisionVersion)
u32le(entityLo), u32le(entityHi)
u32le(occurrenceId), u32le(drawId), u32le(rejectionIndex)
```

Encode each word explicitly; never hash a padded struct, address, or
host-endian memory. Before enabling any seeded build, select and document a
portable, reviewed mixing primitive suitable for the GBA, with constants,
output extraction, and cross-platform golden vectors. It must mix the whole
key and both root words. Never derive features from one sequential PRNG or mix
in a build timestamp, repository hash, clock, player RNG state, or global
catalog version.

## Selection helpers

- Raw word for a named draw.
- `Uniform(n)` for `n` in 1 through `UINT32_MAX`: with
  `limit = 2^32 - (2^32 mod n)` computed wide, draw `x`; while `x >= limit`,
  increment only `rejectionIndex`; return `x mod n`. Index overflow is an
  explicit failure.
- Weighted choice over positive weights in canonical (stable ID) order, total
  at most `UINT32_MAX`, overflow detected, via the matching half-open interval.
- Shuffle with consumer-declared semantic draw IDs.

Consumers define empty-pool handling; the framework never substitutes global
randomness. A complex decision may opt into a transient decision-local stream
seeded from a derived word, confined to one resolution.

## Consumer contract

Every adopting feature documents its keys and versions; whether each decision
is playthrough-wide, per-entity, or recurring; candidates, canonical order,
weights, and eligibility inputs; when inputs become authoritative; whether
results are recomputed or stored; reveal timing and occurrence advancement;
and invalid-input handling.

Results with lasting consequences are committed to the feature's own fixed
save fields before reveal, together with any reward accounting and occurrence
advance, so duplicate callbacks cannot duplicate or skip anything. Loading from
before the commit re-derives the same result from unchanged inputs; caching a
fresh global roll after first interaction does not satisfy this. Failed
attempts, inspection, map re-entry, loads, and waiting never create
occurrences. A world-fixed decision freezes its inputs at new game or a named
milestone. No unbounded outcome database.

## Isolation and versioning

Adding a decision leaves existing keys unchanged. Changing a decision's
interpretation or draw layout bumps only its version. Committed results store
the rules and content versions they need, so a new build cannot reinterpret
them. Primitive or encoding changes bump `derivationVersion`; record layout
changes bump `seedFormatVersion`. Unsupported versions take the incompatible
save path and never regenerate.

## Existing Pokémon RNG

Framework evaluation and root acquisition never read, advance, reseed, or
replace `gRngValue`, `gRng2Value`, their draw functions, or randomizer state;
audit helper implementations and interrupts rather than trusting names.
Battle, capture, encounter, generation, item, and randomizer rolls stay on
their current paths.

## Validation

Review [new-game initialization](../../game/src/new_game.c),
[shared persistence](../../game/src/wayfarer_persistence.c),
[save structures](../../game/include/global.h), the
[runtime foundation](wayfarer-runtime-foundation.md), and the existing
[RNG interfaces](../../game/include/random.h) and
[implementations](../../game/src/random.c). Keep the module Wayfarer-only.

When un-parked, required evidence:

- Golden vectors for zero and all-one roots, high-word-only changes, every key
  field, endian-sensitive patterns, interval boundaries, rejection retries, and
  invalid bounds and weights, matched byte for byte between host tooling and
  game C.
- Repeated, reordered, and newly added unrelated queries leave existing values
  unchanged; source reordering does not change canonical order.
- Both global RNG states are unchanged around root creation and queries.
- A test-only one-time and recurring consumer prove reload before and after
  commit, duplicate callbacks, invalid input, and occurrence transitions.
- Missing roots and unsupported versions are rejected without a reroll.
- Save size, RAM, stack, ROM, and runtime cost are measured on target.

## Later

- League consumer: seeded league choice for invitations or special events (an earlier draft
  seeded league order per edition: domain 1, ORDER; v0 leagues have no order).
- League consumer: seeded lineups, varying the deterministic
  [league score](leagues.md#selection-and-order) lineup per save, with
  rotation (earlier draft: domain 1, ROSTER and POOL_KIND).
- Trainer growth consumer: one growth arc per trainer per save (earlier draft:
  domain 2, GROWTH_ARC).
- Roster consumer: per-save filler weight variation
  ([roster influence](trainer-roster-influence.md#weighted-pools); earlier
  draft: domain 3, FILLER_JITTER).
