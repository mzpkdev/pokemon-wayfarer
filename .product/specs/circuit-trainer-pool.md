# Circuit trainer pool, generation, and strength

PRD: [Seeded Trainer Circuit](../prds/seeded-trainer-circuit.md)
Implemented: No
Design status: Draft; recurring regional championships, stable initial strength (D1),
and title-agnostic finals (D2) are approved. Content/bands (D3), common
qualification (D4), and rotation tuning/acceptance (D5) remain under review.

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
in multiple leagues; each checks TR and lore eligibility, while scheduled
appearances reduce selection weight as specified below. Two people with similar
names remain distinct.

The initial version uses static authored baseline TR. No player encounter
changes it, and new editions do not inflate NPC TR or effective strength. NPCs
have no player badge contribution or high-water state. Future modest dynamic TR
is outside this initial contract; it must provide an edition-start rating snapshot
for eligibility and strength, never mutate a registered edition in place. Selection
variety comes from participation rotation rather than dramatic rating changes.
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

### Common battle-role bands and eligibility

Every venue and edition uses the same five slots across three battle roles,
regardless of seeded travel position. These are TR categories, never historical title requirements:

| Battle slots (zero-based) | Role | Required count | Inclusive TR band |
| --- | --- | ---: | --- |
| 0–1 | Contender | 2 | Endpoints pending D3 |
| 2–3 | Elite | 2 | Endpoints pending D3 |
| 4 | Headliner | 1 | Endpoints pending D3 |

The proposed concrete default is three disjoint inclusive bands with strictly
ascending boundaries: the highest contender rating is below the lowest elite
rating, and the highest elite rating is below the lowest headliner rating. D3 must
approve numeric endpoints alongside the catalog and teams. A sufficiently rated
Gym Leader can occupy an elite or headliner slot; an Elite Four or Champion title
does not guarantee enrollment or a particular role. The headliner is the strongest
selected trainer by TR because of the ordered bands.

A character is eligible for a role when enabled, presentation/profile validation
passes, and their edition-start TR lies in that role's approved band. In the
initial version this snapshot equals baseline TR. Apply this eligibility before
league-affiliation filtering. No affiliated low-TR character bypasses a band.
Eligibility does not inspect player TR, badges, starter, origin, story flags,
party, or difficulty options. Ratings outside every role band are excluded, with
an inventory reason; never adjust a rating merely to fill a vacant slot.

The same bands apply to Indigo, Sevii Masters, and Hoenn in every edition. There
are no whole-league position targets, edition multipliers, stronger late-edition
bands, or player-relative adaptation. The seeded venue order changes the journey
and allocation traversal, not a venue's eligibility or championship tier. The
runtime owner defines common qualification under D4.

Before enabling the catalog, prove at least two distinct TR-qualified affiliated
contenders, two elites, and one headliner for each venue. Disjoint role bands and
within-venue uniqueness make those counts sufficient even when every outsider
drops; positive rotation weights never remove a candidate. Prove variety across
roots and editions separately. Minimum feasible headcounts do not establish
turnover or broad participation.

The initial catalog is not assumed to satisfy this requirement. D3 must review
concrete affiliations/rationales, ratings, and teams before enabling it. A
shortfall cannot restore filtered-out candidates, widen bands, invent
memberships, or retry draws. Revise reviewed content/design if credible
per-venue role coverage cannot be authored.

### TR-first lore filter and participation rotation

For each venue, first form its TR/content-eligible role lists. An eligible
character whose `leagueAffiliations` contains that venue passes the lore filter.
Every other eligible character has exactly a 50% seeded chance of being dropped
from that venue's lists: framework `Uniform(2) = 0` drops them, and
`Uniform(2) = 1` retains them. Resolve this once per canonical character, venue,
and edition; a lookup, retry, or reload cannot draw again.

The gate is independent of travel position, allocation traversal, other
candidates, and pool size. Adding characters does not change existing raw
character/venue/edition filter outcomes. Multiple sensible affiliations are
allowed, supported by explicit lore rationale; membership bypasses that venue's
gate without extra weight or overriding TR suitability.

There is no visitor quota, reserved guest slot, forced local count, home weight
multiplier, title preference, mandatory Champion, or guaranteed named character.
Multiple retained outsiders may appear, including as headliner. All retained
eligible candidates start with flat `baseWeight = 1`. Proposed D5 tuning is:

```text
withinEditionFactor[scheduledAppearances] = [16, 4, 1]  // counts 0, 1, 2
priorVenueFactor = 1 if selected at this venue in the prior completed edition
                   2 otherwise
selectionWeight = baseWeight * withinEditionFactor * priorVenueFactor
```

`scheduledAppearances` counts assignments already made anywhere in this new
edition, not battles entered, fought, or won. At most two earlier appearances
are possible when selecting at the third venue because a person cannot repeat
inside a venue. Every weight remains positive: across-venue repeats are legal
but less likely, and no person is exhausted for future editions.

The runtime supplies the immediately prior completed edition's five canonical
character IDs per stable venue, tagged with the predecessor edition ID. Edition
1 uses empty history. For a later edition, require that predecessor to equal
`editionId - 1`, with exactly five distinct valid canonical IDs per stable venue;
reject missing or inconsistent input. A prior appearance at
another venue does not trigger `priorVenueFactor`. History has no cumulative or
lifetime count. A trainer absent from this venue's previous edition gets the
full prior-venue factor even if they appeared there in older editions.

On successful next registration, the old edition becomes this history atomically
with committing the next schedule. Defeats, retries, replays, and venue clears do
not alter history or weights. The runtime owns history persistence and the
transaction; generation reads an immutable input snapshot.

Roughly two returning and three changed opponents per venue is a soft tuning
target where the catalog supports it. Identical consecutive fields and repeated
entrants remain valid. Never redraw, force replacements, restore dropped
trainers, weaken TR limits, or advance edition identity to manufacture novelty.
A 50% outsider retention chance is not a 50% appearance chance; role pool size,
soft weights, history, and prior allocations determine final inclusion.

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
retired; never reuse its ID. ORDER uses campaign `entityId = 0`; ROSTER uses
the stable venue ID (Indigo = 1, Masters = 2, Hoenn = 3) as its `u64` entity.
LORE_FILTER uses the canonical character ID as its `u64` entity. All use
`occurrenceId = editionId`; retry/replay/abandonment never creates another
occurrence. All three rules versions remain 1 because this draft is unimplemented;
no prerelease migration is required. Resolve keys independently so no
feature-wide or global random sequence is advanced. Ordinary Pokémon RNG is neither consumed nor reseeded,
including by root initialization. Future features opt in under their own domains.

Use semantic draw IDs directly. `ORDER` uses Fisher–Yates index 2 then index 1
as its draw IDs. LORE_FILTER uses venue draw IDs Indigo = 1, Masters = 2,
Hoenn = 3 for each character entity.
`ROSTER` uses the allocated battle slot (0 through 4) as its semantic draw ID.
Its venue entity keeps keys stable when travel position changes. A rejection
required for unbiased sampling increments only the internal rejection index of
that draw; it does
not consume a different semantic draw or perturb later decisions. Eligibility,
weight lookup, and feasibility checks consume no draws. Existing
`LocalRandomSeed`/`LocalRandom32` may be used only inside one atomic decision
when explicitly pinned by the framework; this circuit protocol does not use
them or introduce another stateful generator.

Use this fixed generation procedure:

1. Validate the immutable catalog, common role bands, edition-start rating
   snapshots, and previous-edition venue history. Start with
   `[Indigo, Masters, Hoenn]`. Fisher–Yates shuffle indices 2 then 1 using framework
   `Uniform(i + 1)` under the corresponding ORDER key. All six venue orders must
   be reachable without weighting by player origin or future badges.
2. Build each venue's TR/content-eligible role lists, then resolve eligible
   outsiders' LORE_FILTER keys. Hold the retained lists fixed throughout
   allocation; affiliated characters pass without a filter draw.
3. Generate the whole circuit together in battle-slot priority `[4, 2, 3, 0, 1]`.
   For each slot, visit the three venues in saved seeded order (positions 1, 2,
   then 3). Initialize one local appearance counter per canonical person to zero
   and one selected-character set per venue. Traversal is part of the contract:
   headliners are allocated first, then elites, then contenders.
4. At each slot/venue, enumerate retained candidates eligible for that slot's
   role and not selected at that venue, in ascending stable `characterId` order.
   Compute each candidate's positive weight using their local scheduled count
   and this venue's prior-edition history. A selection at another venue affects
   the count, but cannot exclude the candidate or change their lore outcome.
5. Sum weights using checked arithmetic, draw framework `Uniform(totalWeight)`
   under ROSTER with `entityId = venueId`, `occurrenceId = editionId`, and
   `drawId = battleSlot`. Choose the candidate whose cumulative half-open interval
   contains the draw. Select their single reviewed profile, mark the person in
   this venue's selected set, and increment their local scheduled count.
6. After allocation, sort the contender pair (slots 0–1) and elite pair
   (slots 2–3) by ascending snapshotted TR, breaking ties by ascending
   `characterId`. Keep the headliner in slot 4. The allocation draw IDs remain
   those used before this presentation sort; no extra draw or title-based reorder
   occurs.
7. Return edition ID, ORDER/LORE_FILTER/ROSTER rules versions, catalog version,
   shuffled venues, and each ordered battle's role, character/profile references,
   and snapshotted rating to the runtime owner. Persist the resolved whole schedule
   alongside a valid foundation root before a save or UI can consume it. Runtime
   atomically commits the new edition and its predecessor history; generation
   does not increment or consume an edition ID or mutate persistent history.

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
membership, eligibility, ratings, weights, or prior-edition history can legitimately
change allocation outcomes. Because shared scheduled counts are updated in the
canonical traversal, relevant changes at an earlier allocation can affect later
venues, including venues whose own candidate lists are unchanged. There is no
cross-venue roster independence guarantee and no freedom to reorder allocation.
Unrelated features, source enumeration order, and content outside the circuit's
semantic inputs cannot affect its outcome.

ORDER and raw LORE_FILTER draws remain independent of roster catalog size and
history. The same root, respective rule versions, and edition yield the same
venue order and raw character/venue filter outcomes across added candidates.
A changed affiliation changes whether a character needs the gate, not its raw key.
Increment only the affected decision rules version when its protocol or selection
rules change. Root derivation version remains foundation-owned; schedule schema,
decision versions, and catalog version remain explicit. Follow prerelease save
policy rather than rebuilding or reinterpreting registered schedules.

A missing role candidate or invalid profile is a generation failure. Never allow
within-venue duplicates, ignore rating limits, restore dropped outsiders, invent
affiliations, substitute fixed lineups, or redraw ORDER/LORE_FILTER to repair
content. Reject incomplete content in build validation. A runtime failure cannot
commit a playable new save with a partial schedule; report initialization or
registration failure while preserving existing committed state. Per-venue content
validation must prove the affiliated-only role counts; seed sampling alone does
not establish that property.

### NPC TR to battle strength

Under resolved D1, use the selected trainer's saved edition-start TR snapshot as
the only rating input to the circuit level resolver. It equals baseline TR in
the initial version. Proposed independent anchor data:

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
The ordered role bands and ascending TR within pairs supply progression through
a field. Room position, historical title, current player TR, player party, and source absolute levels add
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
  metadata, double-battle flags, overlapping/nonascending role bands, and
  out-of-band entries presented as eligible.
- Prove affiliated-only feasibility of two contenders, two elites, and one
  headliner per venue. Check TR-first exclusion, multiple valid affiliations,
  exact gate outcomes 0/1, and no outsider quota. Every selected person must fit
  the slot role; aliases cannot fill multiple slots in one venue. Insufficient
  role catalogs must fail without partial persistence or draw retries.
- Pin root vectors including all-zero (`lo = hi = 0`) and all-one
  (`lo = hi = 0xFFFFFFFF`) roots. Cover decision/entity/occurrence keys, versions,
  semantic draw IDs, rejection boundaries, weighted half-open intervals, checked
  weight sums, canonical traversal, within-pair presentation sorting, persisted
  catalog/reference versions, and the framework golden vectors. Verify root initialization and circuit resolution do not mutate Pokémon RNG.
- Pin editions 1, 2, and boundary `u32` identities. Edition 1 has empty history;
  later editions require exactly the predecessor's completed venue lineups.
  Check same-venue history factor, scheduled-count factors 0/1/2, absence resetting
  the prior-venue penalty, and positive weights permitting repeats. History must
  remain unchanged by defeat, replay, queries, or individual venue clears.
- Resolve the same next edition twice from identical root/catalog/rating/history
  inputs, including save/reload before registration: order, assignments, sorting,
  profiles, and TR snapshots must match. Generation cannot consume an edition ID
  or update persistent history. Preserve legal repeated orders and full fields
  without novelty retries.
- Resolve identical inputs around unrelated feature calls, menu opens, fights,
  queries, and save/load. Reorder registry/source records without changing semantic
  inputs and assert identical results. Verify lookups consume no semantic draws.
  Change an earlier allocation's relevant candidates/history in controlled
  fixtures and allow later rosters to change through scheduled counts. Assert
  ORDER and existing raw LORE_FILTER keys remain identical. Do not assert that
  different allocation traversals or changes at another venue preserve rosters.
- Add eligible outsiders without changing existing raw gate outcomes. Report final
  guest share and regional composition without imposing a quota or home multiplier.
  Use exact controlled fixtures to test flat base weights and the proposed
  within-edition/history factors; frequency thresholds cannot substitute for those
  deterministic tests.
- Sweep at least 10,000 documented roots over at least ten consecutive editions
  using production content and valid predecessor history. Report order counts,
  per-venue returning/changed opponents, identical consecutive fields, shared
  opponents within each circuit, affiliated/guest share, role/field strength,
  per-character inclusion relative to eligible opportunities, and absence over
  documented bounded edition windows. Assert deterministic replay and structural
  invariants. D5 sets quantitative acceptance after measuring these distributions;
  there is no all-current-elites or 80%-per-character attendance target within a
  single circuit. Feasibility counts do not establish sufficient variety.
- Exhaust integer TR 0–80 for interpolation, saturation, signed offsets, and
  monotonicity. Construct every enabled profile, preserve source identity and
  non-level metadata, and verify identical party reconstruction.
- For the same profile/TR snapshot, prove strength identical across editions and
  qualifying venues. Common role bands remain unchanged by travel position,
  edition number, or player strength; no automatic rating/level inflation occurs.
- Vary player TR, party, badges, location history, and gameplay RNG around a saved
  schedule; human participants and ordinary circuit strength remain unchanged.
  Cover species-randomizer bypass separately and verify lifecycle.
- Verify selected-trainer money, XP, AI, healing items, graphics, and dialogue
  follow the selected profile, including a former fixed room occupant's absence.

Run the relevant trainer/scaling mechanics and circuit E2E suites; build Wayfarer
and affected standalone configurations. Measure ROM and RAM against the current
reserve policy. Report balance playtesting separately from structural validation.

## Open questions

D1–D5 and supporting defaults are centralized in the parent PRD. Stable initial
NPC strength (D1), title-agnostic finals (D2), and recurring regional championship
structure are approved. D3 owns exact content, role bands, and battle balance;
D4 owns common qualification/progression; D5 owns rotation factors and quantitative
variety acceptance. These remain prerequisites for enabling generation. Future
dynamic TR, more regions, Champion-only finals, or doubles require an explicit
revision rather than undocumented selection exceptions.

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
