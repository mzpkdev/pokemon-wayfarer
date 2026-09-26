# Circuit trainer pool, generation, and strength

PRD: [Seeded Trainer Circuit](../prds/seeded-trainer-circuit.md)
Implemented: No
Design status: Draft; recurring core and fixed strength (D1) are approved;
D2–D3 and supporting defaults remain under review.

## Scope

Own the trainer registry, character uniqueness, team profiles, seeded destination
and participant selection, lore filtering, and NPC-TR-based party strength
for each `IS_WAYFARER` circuit edition. The [runtime specification](seeded-league-circuit.md) owns
edition registration, schedule persistence, qualification, room flow, completion, rewards, and UI.
The [playthrough seed framework](playthrough-seed-framework.md) owns root
initialization/persistence, keyed derivation, and unbiased bounded draws. This
specification is its first consumer and owns only circuit decision rules.

This proposes replacement of the fixed trainer allowlist and player-entry-TR
level policy in [League scaling](league-scaling.md). It does not modify ordinary
trainer or Gym scaling. Adopt the parent draft before treating these rules as
the production contract.

## Behavior

### Registry and identity

Author a versioned, machine-readable registry with these fields:

| Field | Contract |
| --- | --- |
| `characterId` | Stable identity for one person, independent of encounter IDs, title, party, or region. |
| `displayName` | Existing localized name or an explicitly authored circuit name. |
| `specialty` | Authored public type or battle-style label, including mixed-team styles; never inferred from trainer class. |
| `leagueAffiliations` | Explicit set of Indigo, Sevii Masters, and Hoenn memberships, with lore rationale per trainer; multiple memberships are allowed. |
| `baselineTR` | Integer 0–80; authored competitive strength, unrelated to the player's TR. |
| `profileId` | One reviewed competitive singles profile for this initial version. |
| `presentationId` | Audited overworld/battle graphics, introduction, defeat, and after-battle text. |
| `enabled` | Build-time content inclusion, with an authored exclusion reason when disabled. |

Maintain an alias inventory linking existing Trainer IDs and source roster owners
to canonical characters. Gym, rival, Champion, rematch, and regional versions of
the same person share `characterId`. Bruno and Lance each remain one character.
No duplicate entry, costume, or team variant can increase selection weight or
bypass uniqueness within a lineup. The same canonical person may be selected
in multiple leagues; each checks TR and lore independently, without global
exclusion or an anti-repeat penalty. Two people with similar names remain distinct.

The registry is static content, not a mutable world ranking. No player encounter
changes baseline TR. New editions do not inflate NPC TR or effective strength.
NPCs have no player badge contribution or high-water state.
League affiliations describe sensible competitive participation, rather than
current map location or blanket geographic origin. Review their lore rationale
under D3; neither appearing on a map nor needing more candidates establishes
membership. Multiple affiliations affect each venue's filter independently.
They never override TR eligibility or permit duplicate slots within one league.

Red is disabled for circuit selection and remains the separate mastery encounter.
The initial pool excludes paired/double-battle profiles. Do not split Tate and
Liza into invented solo entrants to satisfy pool size. The exact membership,
affiliations, individual ratings, and profiles require the D3 content inventory;
this spec does not assign unreviewed ratings to named characters.

### Competitive profiles

Each enabled character has one dedicated six-member circuit profile. Reuse
reviewed source content where appropriate, but do not repurpose a shared Gym or
story encounter in a way that changes its external behavior. Record resolved
runtime Trainer ID, source roster owner, and profile identity explicitly.

A profile contains exact species/forms, four-position movesets, held items,
abilities, IVs, EVs, natures, gender/ball metadata, battle order, trainer inventory,
and AI. Per-member metadata names the original source index and signed level
offset; ace members use 0, support members use -1 or -2. Require at least one
ace and six valid authored movesets. Preserve source identity through any party
ordering and gimmick remapping. Team source data remains immutable at runtime.

Existing five-member Elite Four teams need an authored sixth member before
enrollment. Do not pad with duplicates, borrow a random team, or silently change
species or moves to make an incomplete profile pass validation. A profile's
content and baseline rating must be reviewed together.

### Position bands and eligibility

Position targets remain proposed balance anchors for every edition. D3 must approve the inclusive
TR eligibility endpoints for each position alongside the catalog and teams:

| Circuit position | Target NPC TR | Eligible baseline TR | Player TR at earliest edition-1 entry |
| --- | ---: | ---: | ---: |
| 1 | 40 | Inclusive range pending D3 | 40 |
| 2 | 56 | Inclusive range pending D3 | 56 |
| 3 | 72 | Inclusive range pending D3 | 72 |

Bands belong to position, never to destination. A character is eligible when
enabled, presentation/profile validation passes, and their baseline is in that
position's approved inclusive range. Apply this TR/content eligibility before
league-affiliation filtering; no affiliated low-TR character can bypass the band.
Eligibility does not inspect current or future player TR, badges,
starter, origin, story flags, party, or difficulty options.

Ranges may overlap so a trainer can qualify for more than one position. Each
appearance must pass that position's real TR requirements; do not widen ranges
or upscale a weak trainer merely to force repetition. A character retains the
same profile, fixed TR, and strength across qualifying appearances, subject to
explicit challenge overrides. Ratings outside all approved ranges cannot appear;
the inventory must record them as excluded. The earlier disjoint numeric bands
are superseded and are not implementation defaults.
Later editions retain the same position targets and approved ranges. Do not
substitute stronger endgame bands, apply an edition multiplier, or adapt to the
player's lifetime TR; later-edition challenge and variety require playtesting.

Before enabling the catalog, prove at least five distinct TR-qualified affiliated
characters for every venue/position pairing across all six orders. This covers
the worst case where all unaffiliated candidates drop. Each league selects
independently without replacement. Require variation across roots separately;
feasible headcounts alone do not establish variety.

The initial catalog is not assumed to satisfy this requirement. D3 must review
concrete affiliations/rationales, ratings, and teams before enabling it. A
generation shortfall cannot restore filtered-out candidates, widen TR bands,
invent affiliations, or retry the draw. A reviewed design/content revision is
required if the per-venue feasibility requirement cannot be authored credibly.

### TR-first lore filter and rating weights

For each scheduled venue, first form its TR/content-eligible candidate list.
An eligible character whose `leagueAffiliations` contains that venue passes the
lore filter. Every other eligible character has exactly a 50% seeded chance of
being dropped from that venue's list: framework `Uniform(2) = 0` drops them,
and `Uniform(2) = 1` retains them. Resolve this once per canonical character,
venue, and edition; a candidate lookup, retry, or reload cannot draw again.

The gate is independent of circuit position, other candidates, and pool size.
Adding characters does not change existing character/venue/edition filter outcomes.
Multiple sensible league affiliations are allowed, supported by explicit lore
rationale; a character passes each affiliated venue's filter without receiving
extra weight or bypassing TR suitability.

There is no visitor quota, reserved guest slot, forced local count, or
affiliation-based weight. Multiple retained unaffiliated trainers may appear
in one field, including its final slot under D2. Within the retained feasible
candidate set, use:

```text
ratingWeight = 1 + max(0, 6 - abs(baselineTR - targetTR[p]))
selectionWeight = ratingWeight
```

The rating weight favors proximity to the position's target; the hard band
limits the full difficulty range. Each retained feasible candidate
has positive weight. There is no additional home multiplier, title preference,
mandatory Champion, or guaranteed named character. A 50% retention chance is
not a 50% final-selection chance. Relative pool size, ratings, weights, and
previous selections within that lineup determine the final share. A large global pool can
crowd out affiliated trainers even after the filter; generation audits must
report field composition and character inclusion for balance review without
reinterpreting or weakening the confirmed gate.

### Foundation consumer protocol and deterministic generation

Generate edition 1 during new-game initialization after the framework establishes
a valid playthrough root. Generate a later edition only for an authorized
registration transaction after the current edition is complete. Edition identity
is a `u32` beginning at 1; it is not a retry count, timestamp, or mutable random
cursor. The proposed foundation stores one 64-bit root as two
`u32` words plus seed-format/derivation-version metadata in shared Wayfarer
persistence. The circuit stores no copied root, independent seed, or persistent
random cursor. Tests/debug tooling may inject a root through the foundation;
no player-facing seed-entry UI is required in version 1.

Use the framework's pure keyed `KeyedU32` semantics and unbiased `Uniform(n)`
operation, not a private circuit PRNG. Each draw is keyed by root, stable numeric
domain ID, decision rules version, decision ID, stable `entityId` (`u64`),
`occurrenceId` (`u32`), semantic `drawId` (`u32`), and the framework's internal
rejection index (`u32`). The foundation pins fixed little-endian encoding and
the exact hash/mixer with golden vectors before this feature is enabled. These
names describe the required interface, not APIs already implemented in the ROM.

Use framework domain `CIRCUIT = 1`, with `ORDER = 1`, `ROSTER = 2`, and
`LORE_FILTER = 4`, initially at rules version 1 each. `VISITOR_POLICY = 3` is
retired; never reuse its ID. ORDER and ROSTER use campaign `entityId = 0`;
LORE_FILTER uses the canonical character ID as its `u64` entity. All use
`occurrenceId = editionId`; retry/replay/abandonment never creates another
occurrence. All three rules versions remain 1 because this draft is unimplemented;
no prerelease migration is required. Resolve keys independently so no feature-wide or global random
sequence is advanced. Ordinary Pokémon RNG is neither consumed nor reseeded,
including by root initialization. Future features opt in under their own domains.

Use semantic draw IDs directly. `ORDER` uses Fisher–Yates index 2 then index 1
as its draw IDs. LORE_FILTER uses venue draw IDs Indigo = 1, Masters = 2,
Hoenn = 3 for each character entity.
`ROSTER` uses `(position << 8) | temporarySlot`, with position
3 through 1 and temporary slots 0 through 4. A rejection required for unbiased
sampling increments only the internal rejection index of that draw; it does
not consume a different semantic draw or perturb later decisions. Eligibility,
weight lookup, and feasibility checks consume no draws. Existing
`LocalRandomSeed`/`LocalRandom32` may be used only inside one atomic decision
when explicitly pinned by the framework; this circuit protocol does not use
them or introduce another stateful generator.

Use this fixed generation procedure:

1. Start with `[Indigo, Masters, Hoenn]`. Fisher–Yates shuffle indices 2 then 1
   using framework `Uniform(i + 1)` under the corresponding ORDER key.
   All six venue orders must be reachable without weighting by player origin
   or future badges.
2. Build the TR/content-eligible candidate list for each scheduled venue,
   then resolve each eligible unaffiliated character's LORE_FILTER key. Hold
   the retained lists fixed throughout allocation; affiliated characters pass
   without a filter draw.
3. Allocate five slots for each league independently, using position order
   3, 2, 1 and temporary slot order 0 through 4 as the canonical traversal.
   A league's selected-character set starts empty and is private to its lineup.
   Traversing another league first cannot deplete its list or consume its draws.
4. Enumerate retained eligible characters not yet selected in that lineup,
   in ascending stable `characterId` order. At least five retained candidates
   must be available before selecting its first slot. Selections in other
   leagues do not exclude or penalize a candidate.
5. Sum retained candidates' weights, draw framework `Uniform(totalWeight)` under
   that position/temporary-slot ROSTER key, and choose the candidate whose
   cumulative half-open interval contains the draw. Select its
   single reviewed profile and mark its canonical character selected only
   within this league.
6. Sort each completed lineup by ascending baseline TR, breaking ties by
   ascending `characterId`. The last trainer is the finalist. Do not add a
   second random draw or a title-based reorder.
7. Return edition ID, ORDER, LORE_FILTER, and ROSTER rules versions, catalog version, shuffled
   venue list, and ordered character/profile references with their baseline
   ratings to the runtime owner. Persist the resolved whole schedule alongside
   the valid foundation root before a save or UI can consume it. The runtime
   atomically stages and commits the full new edition; generation must not
   increment or consume an edition ID itself.

A different edition supplies different keys, not a guarantee of a different
permutation or roster. Repeated orders/entrants are valid outcomes; never redraw
or advance the occurrence to force novelty. Loading a pre-registration save and
registering again with the same root/next edition/inputs gives the same outcome.
Do not expose an uncommitted next schedule through preview/skip/cancel paths.

Catalog enumeration/source order, asset loading, maps visited, menu opens,
fights, queries, and unrelated opted-in feature calls or content must not affect
these decisions. With the same root, edition, decision versions, and semantic inputs,
an unresolved draw is repeatable even before its first saved resolution. After
resolution, the saved schedule is authoritative; loading never redraws it.

Catalog versions validate persisted references and reproduce generation inputs;
do not put a global or catalog version into every random key. A changed roster's
membership, eligibility, or weights may legitimately change a new schedule's
ROSTER outcomes at each affected league. Candidate changes relevant to both
leagues may affect both; candidates eligible only for another venue cannot alter
this league's list or draw. Unrelated table changes and source ordering cannot. ORDER and
LORE_FILTER draws are independent of roster catalog size: the same root and
respective rules versions and edition give the same venue order and raw character/venue
filter outcomes across added candidates. A changed affiliation changes whether
that character needs the gate; it does not redraw its key. Increment only the affected
decision rules version when its draw protocol or selection rules change. Root
derivation version remains foundation-owned; the schedule stores its own schema,
decision rules versions, and catalog version. Follow prerelease save policy
rather than silently rebuilding or reinterpreting saved schedules.

Fewer than five retained distinct candidates or an invalid profile is a
generation failure, never a reason to allow within-lineup duplicates, ignore rating limits, restore dropped unaffiliated
candidates, invent affiliations, or substitute a fixed lineup. Never redraw
ORDER or LORE_FILTER to repair the catalog. Reject
incomplete content in build validation. If a runtime failure still occurs, do not
commit a playable new save with a partial schedule; report initialization failure.
Per-venue content validation must guarantee five retained distinct candidates
even when every unaffiliated candidate drops. Sampling many seeds alone is not
proof of that property.

### NPC TR to battle strength

Under resolved D1, use the selected trainer's saved baseline TR as the only rating input
to the circuit level resolver. Proposed independent anchor data:

```text
(0,15), (4,16), (8,18), (16,23), (30,30),
(40,42), (55,60), (65,80), (80,100)
```

For adjacent anchors `(r0,l0)` and `(r1,l1)`, clamp input to 0–80, then compute:

```text
width = r1 - r0
rise = (rating - r0) * (l1 - l0)
baselineLevel = l0 + floor((2 * rise + width) / (2 * width))
memberLevel = clamp(baselineLevel + member.levelOffset, 1, 100)
```

Use wide intermediates and signed offsets. This retains nearest-integer rounding
with halves upward. It is independently authored circuit data, not a call to the
player cap or Gym resolver. The formula is monotonic; integer rounding can give
adjacent TR values equal levels.

Remove the old encounter-position offsets `[-4,-3,-2,-1,+1]` from this policy.
Ascending NPC TR already supplies the progression through a field. Room position,
historical title, current player TR, player party, and source absolute levels add
no extra level adjustment. A trainer with the same profile and TR has the same
effective party in every eligible circuit appearance.

Examples: TR 40 gives an ace level of 42; TR 56 gives 62; TR 72 gives 89.
Support offsets are applied afterward. Final field level ranges depend on D3's
approved eligibility endpoints. These examples are formula expectations,
not claims that the resulting encounters are balanced.

Construct and reconstruct through the same schedule-bound profile plan. Resolve
the actual selected trainer and roster owner before applying the circuit policy;
do not identify enrollment from trainer class, map, or a shared party pointer.
Ordinary/debug battles outside a valid run retain their existing behavior and
cannot create a circuit record. Real circuit callers reject inconsistent identity
before battle rather than quietly fighting a hardcoded room occupant.

Preserve authored species/forms and all non-level profile fields. Do not apply
ordinary evolution reversal or automatic move replacement. Challenge settings
retain their documented overrides. Existing trainer species-randomizer mode may
bypass the authored party/level construction as it does today; it must still use
the saved human participants, order, qualification, and clear accounting. Clearly
limit authored-team/strength guarantees to configurations without that bypass.
Other explicit move/stat randomizers retain their existing precedence.

Battle XP uses actual species and effective levels. Prize money uses the chosen
profile's explicitly inventoried source reward basis and trainer class, not the
old room occupant or player TR. Adding a sixth member must not accidentally alter
the agreed money basis through a source-party last-slot lookup. Circuit and badge
rewards belong to the runtime/progression owner.

### Authoring and validation

The implementation deliverable includes a checked-in content inventory with
canonical aliases, league affiliations and lore rationale, specialties, baseline TR, eligibility and
exclusion reasons, exact profile content, graphics/text coverage, and provenance. It must also
record the catalog and ORDER/LORE_FILTER/ROSTER rules versions. D3 approval covers
concrete ratings and teams; titles or canonical reputation alone do not
establish balanced values.

Required automated evidence:

- Reject duplicate characters/aliases, unresolved source IDs, missing assets,
  invalid ratings/offsets, incomplete six-member profiles, inconsistent team
  metadata, double-battle flags, and out-of-band entries presented as eligible.
- Prove five distinct qualified affiliated candidates per venue/position across
  all six venue orders, covering
  the worst case where every unaffiliated candidate drops. Check TR-first
  exclusion of unsuitable affiliated characters, multiple valid affiliations,
  exact gate outcomes 0/1, and no quota when multiple outsiders survive.
  Never restore dropped candidates or widen TR limits. Exercise overlapping
  eligibility ranges where the same fixed-profile/TR character is selected in
  more than one league and aliases cannot occupy two slots within a league.
  An insufficient venue catalog must fail cleanly.
- Pin root vectors including all-zero (`lo = hi = 0`) and all-one
  (`lo = hi = 0xFFFFFFFF`) 64-bit roots. Cover ORDER/LORE_FILTER/ROSTER keys and rules
  versions, semantic draw IDs, rejection boundaries, exact weighted intervals,
  tie order, catalog/reference versions, and the framework's golden vectors.
  Verify root initialization and circuit resolution do not mutate Pokémon RNG.
- Pin editions 1, 2, and boundary `u32` identities through all three occurrence
  keys. Changing edition must change the derivation input, but need not produce
  a distinct order or field; preserve legitimate consecutive repeats without
  novelty retries. Registration generation cannot commit/consume an edition ID.
  Save/reload before registration and resolve the same next edition twice:
  schedule/profile/TR outcomes must match exactly.
- Resolve identical keys/inputs before and after unrelated seeded feature calls,
  menu opens, fights, queries, content-table changes, and save/load. Assert
  identical venue order and rosters without relying on prior persistence.
  Change roster membership/weights in controlled fixtures and prove ORDER and
  LORE_FILTER outcomes for existing character/venue pairs stay identical with
  the same root, edition, and respective versions, while ROSTER follows changed inputs.
  Add many eligible unaffiliated candidates without changing any existing
  pair's gate; report the resulting final field share rather than imposing a cap.
  Generate leagues in different traversal orders and vary candidates relevant
  only to another venue; neither may change this league's roster. Do not carry
  a selected-character set or draw cursor across venues.
  Reorder registry/source records without semantic changes and assert all stay
  identical. Prove no eligibility/feasibility lookup consumes a semantic draw.
- Sweep at least 10,000 documented playthrough roots for the production catalog.
  Report venue order counts, per-character inclusion, affiliated/unaffiliated share by venue and position,
  and field strength. Assert deterministic replay and all structural invariants;
  use exact controlled fixtures, not a flaky frequency threshold, to test weights.
- Exhaust all integer TR values 0–80 for interpolation, saturation, offsets, and
  monotonicity. Construct every enabled profile, preserve source identity and
  non-level metadata, and verify reconstruction produces the same effective party.
- For the same profile/TR, prove effective strength identical across later
  editions and all qualifying venues. Reuse position targets/approved eligibility
  ranges without edition inflation, player adaptation, or endgame substitution.
- Vary player TR, party, badges, location history, and gameplay RNG around the
  same persisted schedule; human participants and ordinary circuit strength must
  stay unchanged. Cover species-randomizer bypass separately and verify lifecycle.
- Verify selected-trainer money, XP, AI, healing items, graphics, and dialogue
  follow the selected profile, including a former fixed room occupant's absence.

Run the relevant trainer/scaling mechanics and circuit E2E suites; build Wayfarer
and affected standalone configurations. Measure ROM and RAM against the current
reserve policy. Report balance playtesting separately from structural validation.

## Open questions

D1–D3 and supporting defaults are centralized in the parent PRD.
Fixed NPC strength under D1 is resolved; D2/D3 remain open. The exact
production catalog and balance acceptance remain prerequisites for enabling
generation. Reversing fixed NPC strength, reserving Champion-only finals,
adding more regions, or supporting doubles requires revising this specification rather
than adding an undocumented selection exception.

## References

- [Playthrough-seeded variation](../prds/playthrough-seeded-variation.md)
- [Shared playthrough seed framework](playthrough-seed-framework.md)
- [Seeded circuit runtime](seeded-league-circuit.md)
- [Existing League scaling contract](league-scaling.md)
- [Existing Gym scaling contract](gym-leader-scaling.md)
- [Party construction](../../game/src/battle_main.c)
- [Scaling policy and roster validation](../../game/src/trainer_party_scaling.c)
- [Current fixed League metadata](../../game/src/data/trainer_scaling/league.h)
- [Player TR runtime](../../game/src/trainer_rating.c)
