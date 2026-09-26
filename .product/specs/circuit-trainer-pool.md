# Circuit trainer pool, selection, and rotation

PRD: [Seeded Trainer Circuit](../prds/seeded-trainer-circuit.md)
Implemented: No
Design status: Draft. Standing-based roles and title-agnostic headliners (D2)
are approved. Catalog content (D3), rotation tuning (D5), and role windows (D7)
remain provisional.

## Scope

Own the circuit registry, canonical identity, competitive profiles, home
leagues, role windows, per-slot selection, rotation weights, and battle
construction for each `IS_WAYFARER` league competition.

- [Trainer world progression](trainer-world-progression.md) owns `worldCap`,
  standing, bias, growth arcs, team stages, and the level formula. This spec
  consumes a candidate's standing and ace level; it never restates them.
- The [circuit runtime](seeded-league-circuit.md) owns signup, competition
  identity, entry capture, retry, results, persistence, and presentation.
- The [seed framework](playthrough-seed-framework.md) owns the root, keyed
  derivation, and unbiased `Uniform(n)`.

This replaces the fixed allowlist and player-entry-TR level policy in
[League scaling](league-scaling.md) upon adoption.

## Registry and identity

Author a versioned, machine-readable registry:

| Field | Contract |
| --- | --- |
| `characterId` | Stable `u32` identity for one person, independent of Trainer IDs, title, party, or region. |
| `displayName` | Existing localized name or an authored circuit name. |
| `specialty` | Authored public type or style label; never inferred from trainer class. |
| `homeLeagues` | Authored venue set with a lore rationale per trainer; may be empty or hold several. Only Indigo and Hoenn read it; Masters ignores membership. |
| `competitiveProfileId` | Reviewed six-member singles profile used for every league slot, independent of Gym stage. Absent means not league-eligible. |
| `presentationId` | Audited graphics, introduction, defeat, and after-battle text. |
| `enabled` | Build-time inclusion, with an authored reason when disabled. |

Growth fields (bias, allowed arcs, stages) live in the progression catalog
keyed by the same `characterId`.

Maintain an alias inventory linking existing Trainer IDs and source roster
owners to canonical characters. Gym, rival, Champion, rematch, and regional
versions of one person share `characterId` (Bruno and Lance each remain one
person). Aliases, costumes, and team variants cannot add selection weight or
bypass within-field uniqueness. People with similar names remain distinct.

Home leagues describe sensible regional competitive participation, not map
location. Neither appearing on a map nor a thin candidate pool establishes
home membership. Home status affects pool classification only, never
strength.

The initial pool is the existing 37 singles trainers, pending D3 review. Red
is disabled (separate mastery encounter). Tate and Liza are excluded
(double battle). Story availability neither removes a trainer from the pool
nor enrolls their story encounters.

## Competitive profiles

A league slot always uses the trainer's reviewed six-member competitive
profile, whatever their current Gym stage. Author the sixth member and full
movesets explicitly; never pad with duplicates or reuse incomplete source
parties.

A resolved profile contains exact species/forms, four-move sets, held items,
abilities, IVs, EVs, natures, gender/ball metadata, battle order, trainer
inventory, AI, and member level offsets (−30..0, at least one ace at 0).
Record runtime Trainer ID, source roster owner, original member indices, and
profile/content version. Preserve source identity through ordering and gimmick
remapping. Content is immutable at runtime; do not edit shared Gym/story
parties or invent moves to make a profile eligible.

## Roles

| Battle slots (zero-based) | Role | Count | Standing window (D7, provisional) |
| --- | --- | ---: | --- |
| 0–1 | Contender | 2 | ≤ −2 |
| 2–3 | Elite | 2 | −1..+1 |
| 4 | Headliner | 1 | ≥ +2 |

Windows are disjoint and cover every integer, so each candidate has exactly one
role at a given standing. They are shared by all venues and editions. Titles
never grant or deny a role. Standing is read at the competition's captured
progress index `p_event`; venue and position add nothing, and editions matter
only through `p`. After final ordering, a slot's role label follows its battle
position in this table.

A candidate is **selectable** when enabled, presentation and competitive
profile validate, and they are not already chosen in this field.

## Selection

Inputs, captured or read by the runtime at entry: event identity
`(editionId, venueId)`, `B_event`, `C_event`, `p_event`, `worldCap_event`,
every candidate's arc and standing at `p_event`, versions, and the saved
rotation history.
Nothing reads player TR, party, starter, story flags, or live milestones
after entry.

Allocate battle slots in priority `[4, 2, 3, 0, 1]` (headliner, elites,
contenders). For each slot being allocated:

1. **Window.** Take selectable candidates whose standing is in the slot's
   role window.
2. **Pool.** At Indigo and Hoenn, partition into home (venue in
   `homeLeagues`) and visitor. If both are non-empty, draw POOL_KIND
   `Uniform(100)`: 0–84 home, 85–99 visitor. If only one is non-empty, use it
   with no draw. Masters uses one pool and never draws POOL_KIND.
3. **Person.** Enumerate the chosen pool by ascending `characterId`, compute
   rotation weights with checked sums, draw ROSTER `Uniform(totalWeight)`, and
   take the matching half-open cumulative interval.
4. **Fallback.** If step 1 is empty, pick deterministically with no draw: the
   selectable candidate whose standing is nearest the window (smallest
   distance outside it), preferring home over visitor at equal distance at
   Indigo/Hoenn, then lowest `characterId`. Mark the slot as a fallback in
   the plan. Fallbacks are legal; validation reports their rate.

Then order the field: the final battle order is all five chosen trainers
sorted by ascending standing, with lower `characterId` breaking ties, and each
slot's role label follows its final position. Because windows are disjoint,
this equals role order whenever no fallback happened; with a fallback it still
guarantees a non-decreasing ramp. The draw ID is the zero-based battle slot
index being allocated, before final ordering; ordering consumes no draws.

The 85/15 split is a per-slot category chance, not an individual appearance
probability or a quota. Several visitors, including the headliner, may appear.
There is no visitor cap, reserved guest, home multiplier, title preference, or
guaranteed named trainer.

## Rotation

All candidates in the chosen pool start at base weight 1 (D5, provisional):

```text
crossVenue  = [16, 4, 1][venues won earlier this edition whose field included them]
priorField  = 1 if in this venue's field from the previous edition, else 2
weight      = crossVenue * priorField
```

Only won competitions record participation, so history is each venue's
latest won five-person field plus its edition. `crossVenue` counts other
venues whose latest field belongs to the current edition (0–2). `priorField`
reads this venue's field when it belongs to `editionId − 1`; in edition 1
every candidate gets 2. Weights stay positive; rotation never removes a
candidate or overrides the window or pool draw.

The target is roughly two returning and three new trainers at a venue
between editions. It is a tuning target, not a quota: identical consecutive
fields are valid. Never redraw or alter identity to force novelty.

## Keys and determinism

Use framework `KeyedU32` with domain CIRCUIT = 1. Semantic IDs ORDER = 1,
ROSTER = 2, and POOL_KIND = 5 remain; retired IDs 3 and 4 are never reused.

| Decision | Rules version | Entity words | Occurrence | Draw ID |
| --- | ---: | --- | --- | --- |
| ORDER | 1 | `0`, `0` | `editionId` | Fisher–Yates index 2, then 1 |
| POOL_KIND | 3 | `entityLo = venueId`, `entityHi = editionId` | 0 | Battle slot index being allocated (0–4, before final ordering) |
| ROSTER | 4 | `entityLo = venueId`, `entityHi = editionId` | 0 | Battle slot index being allocated (0–4, before final ordering) |

Venue IDs are Indigo = 1, Masters = 2, Hoenn = 3. The competition ordinal is
gone; each `(editionId, venueId)` has exactly one competition.

Category and person draws per slot have independent keys. Rejection sampling
advances only the framework's internal index. Eligibility, pools, weights,
fallbacks, and final ordering consume no draws. Arc derivation uses its own
TRAINER_GROWTH keys and cannot perturb circuit words. Ordinary Pokémon RNG is
neither consumed nor reseeded. Catalog source order, menus, unrelated seeded
features, and reloads cannot change a result. Changed inputs (milestones,
history, catalog) can legitimately change a not-yet-entered field but never
the raw ORDER or category words for the same identity.

The procedure returns the event identity, five ordered slot plans (role,
`characterId`, arc, standing, profile reference, fallback flag), the captured
inputs, and all rules/content versions. The runtime saves them atomically
before reveal. Read-only verification reproduces allocation and final ordering
from the saved inputs and compares; it never replaces a saved field.

## Battle strength

Each slot fights at its real strength, frozen for the event: the progression
spec's ace level from `levelBase_event` and standing, clamped and applied to
member offsets. There is no per-role fudge and no room-position offset (the
old `[-4,-3,-2,-1,+1]` offsets are removed). The field always ramps, because
the final order guarantees non-decreasing standing; after a fallback, levels
may sit closer together than the role windows suggest.

Construct and reconstruct from the saved slot plan. Resolve the selected
trainer and roster owner before applying circuit policy; never identify
enrollment from class, map, or a shared party pointer. Debug and ordinary
battles cannot create circuit records.

Preserve authored species/forms and all non-level fields; no evolution
reversal or automatic move replacement. Challenge settings keep their
overrides. The trainer species randomizer may bypass authored parties as it
does today but still uses the saved people, order, and results; authored-team
guarantees exclude that mode. XP uses actual species and levels. Prize money
uses the profile's inventoried source reward basis and class, not the old room
occupant, and the sixth member must not shift it.

## Validation

The deliverable includes a checked-in inventory: aliases, home leagues and
rationales, specialties, competitive profiles, presentation coverage,
exclusion reasons, provenance, and catalog and rules versions.

- Reject duplicate characters or aliases, unresolved source IDs, missing
  assets, invalid offsets, incomplete or non-six-member competitive profiles,
  and double-battle flags.
- **Feasibility.** For every venue × signup-reachable `(B, C, p)` × role,
  report in-window candidate counts under every allowed arc (guaranteed) and
  under some allowed arc (possible), plus the fallback rate over at least
  10,000 seeded roots. Fallbacks must be rare and documented.
- **Full rosters first (D3).** Feasibility depends on which trainers have
  reviewed six-member competitive profiles. The prototype catalog lacks many,
  so its current fallback risks are not tuning evidence. Author the full
  rosters, then rerun feasibility. Adjust arcs, biases, or windows only for
  gaps that remain after that.
- **Variety.** Over at least 10,000 roots × 10 editions with wins, report
  returning/new trainers per venue, cross-venue repetition, headliner
  distribution (including Gym Leader headliners from every region at a
  nonzero rate), and home/visitor share. D5 sets acceptance after
  measurement.
- **Selection.** Test POOL_KIND boundaries 0/84/85/99 and rejection; one-pool
  slots and Masters consume no category draw; category choice is independent of
  pool sizes and weights; fallback ordering and tie-breaks; a field whose final
  order differs from allocation order, including after a fallback, verifies
  correctly with role labels following final positions.
- **Rotation.** Exact fixtures for factors 16/4/1 and 1/2, edition 1 with no
  history, and losses recording nothing.
- **Keys.** Pin vectors for zero and all-ones roots, editions 1/2 and `u32`
  bounds, all three venues, rules versions, weighted intervals, and checked
  sums. Verify no Pokémon RNG use.
- **Construction.** Build every enabled profile at level bounds, preserve
  source identity and metadata, reconstruct identically, and verify money, XP,
  AI, graphics, and dialogue follow the selected trainer.

Run the trainer/scaling mechanics and circuit E2E suites, build Wayfarer and
affected standalone configurations, and measure ROM/RAM against the reserve
policy. Report balance playtesting separately from structural checks.

## Open questions

The PRD owns D3 (catalog and profiles), D5 (rotation and variety acceptance),
and D7 (role windows). More regions or doubles require a revision.

## References

- [Seeded Trainer Circuit PRD](../prds/seeded-trainer-circuit.md)
- [Trainer world progression](trainer-world-progression.md)
- [Seeded circuit runtime](seeded-league-circuit.md)
- [Shared playthrough seed framework](playthrough-seed-framework.md)
- [Existing League scaling contract](league-scaling.md)
- [Party construction](../../game/src/battle_main.c)
- [Scaling policy and roster validation](../../game/src/trainer_party_scaling.c)
- [Current fixed League metadata](../../game/src/data/trainer_scaling/league.h)
