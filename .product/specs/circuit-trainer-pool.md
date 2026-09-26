# Circuit trainer pool, generation, and strength

PRD: [Seeded Trainer Circuit](../prds/seeded-trainer-circuit.md)
Implemented: No
Design status: Draft; trainer-owned world progression with a single-competition
entry lock and loss ending that event (D1), recurring championships, and
title-agnostic finals (D2) are approved directions. Content/bands (D3), common
qualification (D4), rotation tuning/acceptance (D5), and competition
availability/waiting (D6) remain under review.

The confirmed competition-entry snapshot follows [Trainer world progression](trainer-world-progression.md).
Numeric personal growth, role-band endpoints, and qualification remain draft contracts;
the [balance explorer](../../devtools/ui/README.md#trainer-balance-explorer)
provides provisional numeric experiments rather than approved ROM content.

## Scope

Own the trainer registry, character uniqueness, team profiles, seeded destination
and participant selection, regional pool selection, and NPC-TR-based party strength
for each `IS_WAYFARER` league competition. The [runtime specification](seeded-league-circuit.md) owns
competition availability/entry, snapshot persistence, circuit traversal, qualification,
room flow, completion, rewards, and UI.
The [playthrough seed framework](playthrough-seed-framework.md) owns root
initialization/persistence, keyed derivation, and unbiased bounded draws. This
specification is its first consumer and owns only circuit decision rules.

This proposes replacement of the fixed trainer allowlist and player-entry-TR
level policy in [League scaling](league-scaling.md). The shared trainer progression
proposal also covers Gym appearances; this document
owns only circuit enrollment and its frozen battle plans. Adopt the draft before
treating these rules as the production contract.

## Behavior

### Registry and identity

Author a versioned, machine-readable registry with these fields:

| Field | Contract |
| --- | --- |
| `characterId` | Stable identity for one person, independent of encounter IDs, title, party, or region. |
| `displayName` | Existing localized name or an explicitly authored circuit name. |
| `specialty` | Authored public type or battle-style label, including mixed-team styles; never inferred from trainer class. |
| `homeLeagues` | Explicit set of Indigo, Sevii Masters, and Hoenn home leagues, with authored rationale per trainer; multiple homes are allowed. |
| `baselineTR` | Integer 0–80; authored starting personal TR, equal to the zero-badge checkpoint. |
| `badgeTRCheckpoints` | Four authored, nondecreasing integer personal TR values at 0/8/16/24 global badges. |
| `leagueGrowth` | Authored bounded nonnegative integer per-trainer growth for each lifetime venue first clear. |
| `teamStages` | Versioned ordered TR thresholds and validated stage/profile references, beginning at TR 0. |
| `profileId` | Stable identity of the reviewed singles profile family; each stage resolves an exact profile. |
| `presentationId` | Audited overworld/battle graphics, introduction, defeat, and after-battle text. |
| `enabled` | Build-time content inclusion, with an authored exclusion reason when disabled. |

Maintain an alias inventory linking existing Trainer IDs and source roster owners
to canonical characters. Gym, rival, Champion, rematch, and regional versions of
the same person share `characterId`. Bruno and Lance each remain one character.
No duplicate entry, costume, or team variant can increase selection weight or
bypass uniqueness within a lineup. The same canonical person may be selected
in multiple leagues; each checks TR/content eligibility, while completed
appearances reduce selection weight as specified below. Two people with similar
names remain distinct.

For world point `(B,C)`, interpolate authored badge checkpoints with the
existing nearest-integer, halves-up rule to obtain `badgeTR(B)`. Each canonical
trainer also has one save-specific growth percentage, shared with their Gym
appearances and derived from the playthrough root:

```text
growthPercent = 90 + Uniform(21)  // integers 90 through 110 inclusive
authoredGrowth = (badgeTR(B) - baselineTR) + leagueGrowth * C
scaledGrowth = floor((authoredGrowth * growthPercent + 50) / 100)
effectiveTR = min(80, baselineTR + scaledGrowth)
B = number of global badges, 0–24
C = count of distinct lifetime venue first clears, 0–3
```

Require `badgeTRCheckpoints[0] = baselineTR` and nonnegative authored growth.
Scale the combined badge and first-clear growth, then round once; never scale the
baseline or round each growth component separately. At B = C = 0, every save
retains the exact authored baseline. At 100%, the previous authored formula is
reproduced exactly. Use wide integer intermediates; saturation at 80 is explicit.
The accepted initial modifier range is 90–110%; production curves, stages and
its balance impact still require measurement.

The [seed framework](playthrough-seed-framework.md#trainer-growth-consumer)
registers TRAINER_GROWTH domain 2, GROWTH_RATE decision 1/version 1,
`entityLo = characterId`, `entityHi = 0`, `occurrenceId = drawId = 0`.
Canonical character IDs fit `u32`; aliases share that identity and modifier.
The save pins the growth-policy version at new game before any Gym can resolve
strength. Derivation may be lazy and pure, but it cannot reroll per badge, visit,
competition or edition. Catalog array positions, venue/history and live milestones
are not modifier-key inputs. Store the percentage and policy version (or an
unambiguous reference) with captured event inputs; validate it against the root.

Badge and first-clear milestones intentionally affect world progression; the
fixed percentage changes their rate between saves. Player TR, party, XP,
historical title, edition/event counts and elapsed time add no strength.
Entering a later event captures actual milestones, not a new modifier. The
explorer's current curves remain provisional and its unmodified output is the
neutral 100% experiment; modifier support is not implemented there.

Home leagues describe sensible regional competitive participation, rather than
current map location or blanket geographic origin. D3 reviews each authored
rationale; neither appearing on a map nor needing more candidates establishes a
home league. A character can be home at multiple venues. Home status governs
pool classification, never TR eligibility or within-venue uniqueness. Eligible
characters are visitors at any venue absent from their `homeLeagues`.

Red is disabled for circuit selection and remains the separate mastery encounter.
The initial singles pool retains the existing 37 canonical trainers. Their
experimental numbers and source teams still require production review. The
initial pool excludes paired/double-battle profiles. Tate and Liza remain
excluded by confirmed content choice for this singles-only circuit. The exact
membership, home leagues, individual ratings, and profiles require the D3 content
inventory; this spec does not assign unreviewed ratings to named characters.

### Competitive profiles

Each enabled character has a reviewed singles profile family with authored
team stages. Resolve the stage from effective TR before eligibility; an early
Gym party and a mature championship party can belong to the same person.
Require increasing unique stage thresholds, an initial stage at TR 0,
nondecreasing party sizes, and no threshold above 80. The existing proposed
circuit default requires a dedicated six-member
competitive profile, pending D3. Earlier two-to-five-member world/Gym stages
are not automatically circuit-eligible. Resolve the stage first, then apply
versioned circuit-specific profile/content validation. The explorer's unmodified
five-member Elite Four source comparisons remain unreviewed evidence, not
approved competitive profiles.

A circuit profile must author its sixth member and complete movesets explicitly;
do not pad with duplicates or reuse incomplete comparison parties.

A resolved battle profile contains exact species/forms, four-position movesets,
held items, abilities, IVs, EVs, natures, gender/ball metadata, battle order,
trainer inventory, AI, and signed nonpositive support offsets with at least one
ace at offset 0. Record runtime Trainer ID, source roster owner, original member
indices, stage identity, and profile/content version explicitly. Preserve source
identity through ordering and gimmick remapping. Authored content is immutable
at runtime. Do not change shared Gym/story parties, pad missing content, invent
moves, or borrow random teams to make a profile eligible. Review each stage and
its progression together, including stable-stage and transition level behavior.

### Common battle-role bands and eligibility

Every venue and edition uses the same five slots across three battle roles.
Authored role bands are indexed by world point and are shared across all venues
at that same world point; seeded position alone does not select a tier. These are TR categories, never historical title requirements:

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
passes, and their effective TR at the captured competition-entry world point lies in that role's authored
band for that world point. Resolve both TR and stage/profile before eligibility,
then classify home/visitor pools. No home character bypasses a band. Badges and
lifetime first clears are declared growth and band inputs; eligibility does not
inspect player TR, starter, origin, story flags, party, XP, or difficulty options. Ratings outside every role band are excluded, with
an inventory reason; never adjust a rating merely to fill a vacant slot.

At a given `(B,C)`, the same authored bands apply to Indigo, Sevii Masters, and
Hoenn. Endpoints may change only through the reviewed world point table. Actual
badges and first clears earned between competitions can place later entries at
different world points. There are no venue-specific tiers, edition multipliers,
runtime band widening, or adaptation to the player's party/TR. The seeded order
determines travel, not a projected difficulty schedule. The runtime owner
defines qualification under D4 and competition availability/waiting under D6.

Before enabling the catalog, prove at least two distinct TR/content-qualified
home contenders, two home elites, and one home headliner for each venue at every
supported competition-entry world point, including C 0–3, every supported heterogeneous assignment of integer
90–110% growth modifiers, stage transitions and TR saturation. Checking only
90%/110% endpoints or giving every trainer the same percentage is insufficient.
Use exhaustive reasoning or a conservative proven bound over the whole range;
seed simulations alone are not feasibility proof. Disjoint
role bands and within-venue uniqueness make these counts sufficient: earlier
slots cannot exhaust the home pool needed for any later slot, even if every
selection uses home candidates. Positive rotation weights never remove a
candidate. Prove variety across roots and editions separately. Minimum feasible
headcounts do not establish turnover or broad participation.

The initial catalog is not assumed to satisfy this requirement. D3 must review
concrete homes/rationales, ratings, and teams before enabling it. A missing home
pool is invalid content, never a fallback to visitors. Saturation that collapses
required home roles is a content/design failure, not a runtime repair case. A
shortfall cannot widen
bands, invent home leagues, or retry draws. Revise reviewed content/design if
credible per-venue role coverage cannot be authored.

### TR-first home/visitor selection and participation rotation

For the entered venue at its captured live world point and role, first form the TR/content-eligible candidate list and
partition it by `homeLeagues`: candidates with that venue in their set are home;
all others are visitors. At each slot, remove characters already selected in
this competition before choosing a pool. There is no per-visitor lore exclusion draw.

When both eligible pools are nonempty, use an independent seeded `POOL_KIND`
draw `Uniform(100)`: values 0–84 select home, and 85–99 select visitor. This
85%/15% category split is the initial tuning. Its probability is independent of
the number of candidates or their weights in either pool. When the visitor pool
is empty, use home without consuming a POOL_KIND draw. A missing home pool is a
catalog/content failure, whether or not visitors are available; valid home-only
role counts guarantee this cannot happen during allocation.

After choosing the category, select a person using strictly positive repeat and
history weights only within that category. Home status avoids the regional
visitor disadvantage but does not exempt a person from those penalties. A
visitor uses the same TR eligibility, snapshotted rating, profile, and strength
as a home participant at the same world point; travel does not downgrade them.

There is no visitor quota/cap, reserved guest slot, forced exact local count,
additional home weight multiplier, title preference, mandatory Champion, or
guaranteed named character. Several visitors, including the headliner, may
appear at one venue. The 15% figure is a per-slot category probability when both
pools are available, not an individual appearance probability or an exact lineup
proportion.

All eligible candidates in the chosen pool start with flat `baseWeight = 1`.
Proposed D5 rotation tuning is:

```text
currentCircuitFactor[earlierVenueAppearances] = [16, 4, 1]  // counts 0, 1, 2
priorVenueFactor = 1 if in this venue's immediately previous completed competition
                   2 otherwise
selectionWeight = baseWeight * currentCircuitFactor * priorVenueFactor
```

`earlierVenueAppearances` counts distinct other venues in this circuit edition
where the character appeared in any completed competition, including losses.
It does not count individual battles or future assignments. Ignore the current
venue for this factor: its immediately previous completed event is represented
by `priorVenueFactor`. Earlier-venue participation is accumulated as bounded
canonical-character sets per venue; a person counts once per earlier venue even
if they appeared in several losing competitions there. The count is therefore
0–2, retaining positive weights without an unbounded event archive. At a new
edition, clear these current-circuit sets and retain latest prior-venue fields.

The runtime supplies the latest completed competition's five distinct canonical
IDs and event identity per stable venue, or an explicit never-competed marker.
Completion means WIN or LOSS. History starts empty at new game; it is not
required to come from `editionId - 1`. If this venue had no completed event, every
candidate receives prior-venue factor 2. A returner's penalty expires when they
are absent from that venue's next completed field. A prior appearance at another
venue affects the current-circuit factor only, never `priorVenueFactor`.

The entry transaction captures immutable history inputs before selection. On
WIN or LOSS, atomically update this venue's latest field and current-circuit
participation set while retiring the active event. A loss leaves the current
circuit venue unchanged and requires a later available competition; it does not
permit replaying the retired lineup. Reloading the same active event or querying
menus changes neither history nor weights. History committed after the event
cannot be substituted for its saved pre-entry inputs during verification.

Roughly two returning and three changed opponents per venue is a soft tuning
target where the catalog supports it. Identical consecutive fields and repeated
entrants remain valid. Never redraw, force replacements, weaken TR limits, or
advance an event identity to manufacture novelty. Category availability, role
pool size, soft weights, and completed participation determine final inclusion.

### Foundation consumer protocol and deterministic generation

At new game, establish the shared root and save edition 1's complete venue ORDER
only. Do not generate participants or projected strength for any future venue.
A circuit edition traverses three venues; each venue may host multiple separate
competitions before the player wins there. Only the currently entered competition
owns a five-trainer snapshot. A loss retires it and returns the player to waiting
for the next competition at that same venue. A win advances circuit traversal.
The exact availability/wait mechanism and entry gates remain undecided; possible
first entries near 8/16/24 badges are guidance, not approved thresholds.

Edition identity is a `u32` beginning at 1. Each `(editionId, venueId)` has a
1-based `u32` competition ordinal allocated under the runtime's persisted
availability rule. Registration calls, reopening/cancelling menus, previews,
and reloads do not allocate ordinals or make another event available. Loss ends
an event; only the adopted availability transition can create the next identity.
Reject overflow rather than wrapping. No cadence or timer is invented here.
The shared foundation stores one 64-bit root plus seed metadata. The circuit
stores no copied seed or persistent random cursor.

Use framework `KeyedU32` and unbiased `Uniform(n)` with domain `CIRCUIT = 1`.
The active numeric decision IDs remain ORDER = 1, ROSTER = 2, POOL_KIND = 5;
VISITOR_POLICY = 3 and LORE_FILTER = 4 remain retired. Pin the following keys:

| Decision | Rules version | Entity words | Occurrence | Semantic draw |
| --- | ---: | --- | --- | --- |
| ORDER | 1 | `entityLo = 0`, `entityHi = 0` | `editionId` | Fisher–Yates index 2, then 1 |
| POOL_KIND | 2 | `entityLo = venueId`, `entityHi = editionId` | `competitionOrdinal` | Allocation slot 0–4 |
| ROSTER | 3 | `entityLo = venueId`, `entityHi = editionId` | `competitionOrdinal` | Allocation slot 0–4 |

Stable venue IDs remain Indigo = 1, Masters = 2, Hoenn = 3. Packing the edition
in the entity's high word and venue in its low word mixes the whole event identity
without changing the framework's fixed key encoding. Revised POOL_KIND/ROSTER
versions reflect their new event scope; ORDER remains unchanged. Save an explicit
new competition-state schema discriminator incompatible with old whole-edition
snapshots. No prerelease migration or old-algorithm registry is required.

Each slot's category and person draws have independent keys. Home-only slots
consume no POOL_KIND draw. Rejection increments only the framework's internal
rejection index, never the event ordinal or another semantic draw. Eligibility,
classification, history, and feasibility consume no circuit draws. Trainer growth
uses its independently keyed, fixed TRAINER_GROWTH decision; it cannot advance
category or roster draws. Ordinary Pokémon RNG is neither consumed nor reseeded, including at root initialization.
Future features opt in under separate domains; no private circuit PRNG is added.

Use this fixed generation procedure:

1. Validate the foundation root, saved edition/order/current venue, available
   competition identity, catalog, shared role bands, content, and entry eligibility.
   Capture actual live `B_entry`, lifetime clear mask `L_entry`, immutable rotation
   history inputs, growth-policy version and all progression/profile/band/resolver
   versions. Derive candidate modifiers purely against that root/policy. This captures
   entry into one competition, not signup for the whole circuit. Reuse the saved
   venue order. Later editions derive ORDER once at their runtime-owned rollover
   using the same Fisher–Yates indices; do not generate a future roster there.
2. Set `B = B_entry` and `C = popcount(L_entry)`. Resolve every candidate's
   effective TR using its fixed percentage and stage/profile at this actual world
   point before role bands and content checks. Do not project clears or badges for this or later venues.
   Build fixed home/visitor lists for only the current venue.
3. Allocate slots in priority `[4, 2, 3, 0, 1]`: headliner, elites, contenders.
   Keep one selected-character set for this event. History factors remain fixed
   throughout allocation; do not count selected people as completed appearances.
4. At each allocation slot, remove people already selected in this event from
   both role pools. Reject an empty home pool. When visitors remain, draw
   POOL_KIND `Uniform(100)` with the event key above: 0–84 selects home, 85–99
   visitor. Otherwise use home with no category draw.
5. Enumerate the chosen pool by ascending stable `characterId`, compute positive
   weights from captured completed-event history, and use checked sums. Draw
   ROSTER `Uniform(totalWeight)` for that slot and select the matching cumulative
   half-open interval. Retain their resolved TR/stage/profile and mark selected.
6. Sort the contender pair and elite pair by ascending saved effective TR then
   ascending canonical ID. Keep the headliner in slot 4. Draw IDs refer to
   allocation slots before this presentation sort; consume no additional draws.
7. Return the full event identity, five ordered plans, actual captured world
   point, fixed growth percentages/policy version, pre-entry history, and all
   rules/content/schema versions. The runtime
   atomically saves this one active event before revealing participants or battle
   dispatch. No future venue snapshot is returned. Failure preserves availability,
   ordinal, history, and prior committed state without a partial active field.

POOL_KIND's 85/15 probability describes allocation slots before sorting, not
room position afterward. Read-only verification reproduces canonical allocation
and sorting from saved pre-entry inputs, compares the full active snapshot, and
never replaces it. Do not validate a room occupant against that room number's
raw category draw. Corruption uses invalid-save handling, never generation.

Different editions or competition ordinals provide new keys, not guaranteed
novel lineups. For the same root, event identity, versions, entry milestones and
history, loading before entry reproduces the same result; loading an active event
reads its saved plan. A changed badge/clear milestone before a later entry can
legitimately change eligibility, strength, and participants. Future venue fields
remain unresolved until their own entry. Do not expose preview/skip/cancel paths
that allocate identities or generate a new uncommitted field.

Catalog source order, unrelated feature calls, menus, asset loading, and normal
RNG cannot alter these decisions. Relevant completed-event history and candidate
changes can affect later competition selections; this is declared input coupling,
not a shared draw cursor. Canonical allocation within a single event is fixed.
Raw ORDER depends only on root, edition and its rules; raw category words depend
on root, full event identity, slot and POOL_KIND rules. Catalog/history/growth-modifier input changes
cannot perturb those raw words, though they can affect whether a category draw
is needed or which weighted person is selected. Version only affected decisions.

Invalid home coverage or profiles fail content validation and entry. Never ignore
TR, duplicate people, invent home leagues, substitute a fixed field, force visitors,
or reroll a key to repair content. Prove home-only feasibility for every supported
entry `(B,C)` once gates are adopted, including intermediate badge counts and
saturation; checking 24 badges or projected circuit points alone is insufficient.

### NPC TR to battle strength

Use the selected trainer's saved competition-entry effective TR and stage/profile as
the inputs to circuit strength, throughout that event and save/reload. Do not reevaluate
from live badges or clears after event entry. The experimental NPC curve is:

```text
(0,12), (4,16), (8,18), (16,23), (30,30),
(40,42), (55,60), (65,80), (80,100)
```

These numeric anchors are provisional and independent of the player's existing
soft-cap curve, which retains `(0,15)`. Store the resolver/content version with
the active event so its party plans remain reproducible.

For adjacent anchors `(r0,l0)` and `(r1,l1)`, clamp input to 0–80, then compute:

```text
width = r1 - r0
rise = (rating - r0) * (l1 - l0)
baselineLevel = l0 + floor((2 * rise + width) / (2 * width))
memberLevel = clamp(baselineLevel + member.levelOffset, 1, 100)
```

Consume the shared versioned NPC resolver owned by
[trainer world progression](trainer-world-progression.md#stages-and-levels).
The formula above restates that contract; it is not a separately tuned circuit
curve. Enrolled world Gyms use the same resolver with their own snapshots.
Keep it independent of player caps and the existing player-TR Gym scaler.
Use wide intermediates and signed offsets, rounding halves upward. Integer
rounding can give adjacent TR values equal levels.

Remove the old encounter-position offsets `[-4,-3,-2,-1,+1]` from this policy.
The ordered role bands and ascending TR within pairs supply progression through
a field. Room position, historical title, current player TR, player party, and source absolute levels add
no extra level adjustment. A trainer with the same stage/profile, resolver version, and effective TR has
the same party in every eligible circuit appearance. Different actual entry
world points can give the same person different saved plans within one edition.

Examples: TR 40 gives an ace level of 42; TR 56 gives 62; TR 72 gives 89.
Support offsets are applied afterward. Final field level ranges depend on D3's
approved eligibility endpoints. These examples are formula expectations,
not claims that the resulting encounters are balanced.

Construct and reconstruct through the same saved active-event profile plan. Resolve
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
canonical aliases, home leagues and rationale, specialties, starting TR, personal
checkpoints/first-clear growth, world point role bands, eligibility/exclusion
reasons, exact stage/profile content, graphics/text coverage,
and provenance. It must also
record the catalog, growth-policy and ORDER/POOL_KIND/ROSTER rules versions. D3 approval covers
concrete ratings and teams; titles or canonical reputation alone do not
establish balanced values.

Required automated evidence:

- Reject duplicate characters/aliases, unresolved source IDs, missing assets,
  invalid ratings/offsets, incomplete stage profiles, inconsistent team
  metadata, double-battle flags, overlapping/nonascending role bands, and
  out-of-band entries presented as eligible.
- Prove home-only feasibility of two contenders, two elites, and one headliner
  per venue at every supported competition-entry world point, including C 0–3,
  every supported heterogeneous 90–110% modifier assignment, stage eligibility
  and saturation. Prove coverage over the entire integer range; endpoint-only
  checks and root sampling are insufficient. Check TR-first exclusion, multiple valid homes, and within-venue
  uniqueness. Insufficient home role catalogs must fail without partial
  persistence, visitor substitution, or retries; visitors cannot hide missing
  home coverage. Prove home candidates remain available after all valid earlier
  same-venue selections.
- Test POOL_KIND boundaries 0, 84, 85, and 99, plus unbiased rejection behavior.
  With both pools present, category choice is independent of pool sizes and
  repeat/history weights. Empty visitors select home without a category draw;
  empty homes fail even when visitors exist. No visitor quota limits multiple
  guest selections or a visitor headliner. Verify visitors keep their full rating
  and strength, and home candidates still receive repeat/history penalties.
- Cover a home/visitor pair whose TR sort reverses its allocation order. Load
  verification must accept its saved sorted event field while rejecting a changed
  entrant; it must neither compare category keys to displayed slots nor replace
  committed content.
- Pin root vectors including all-zero (`lo = hi = 0`) and all-one
  (`lo = hi = 0xFFFFFFFF`) roots. Cover decision/entity/occurrence keys, versions,
  semantic draw IDs, rejection boundaries, weighted half-open intervals, checked
  weight sums, canonical traversal, within-pair presentation sorting, persisted
  catalog/reference versions, and the framework golden vectors. Verify root
  initialization and circuit resolution do not mutate Pokémon RNG.
- Pin editions 1/2 and boundary `u32` identities, and competition ordinals
  1/2 and overflow. Assert entity low/high words and occurrence mix the whole
  event identity in both category and roster vectors; ORDER retains edition-only
  keys. Test current-circuit factors 0/1/2, previous-event returner factors,
  WIN/LOSS history updates, empty never-competed history, and bounded per-venue
  unions. Repeated losses cannot overflow or permanently exclude candidates.
- Resolve the same available competition twice from identical inputs, including
  reload before entry: allocation, sorting, stages, actual world point and saved
  TR match. Queries/cancelled entry cannot allocate another identity. A loss retires
  the event; no immediate retry or replay is offered. Later availability creates
  a new ordinal at the same venue without guaranteeing a different field. Failure
  preserves identity, availability and history. No future lineup is generated.
- Resolve identical inputs around unrelated feature calls, menu opens, fights,
  queries and save/load. Canonical source reordering preserves results. Relevant
  completed-event history can change a later field, while raw ORDER/category keys
  remain unchanged for their same identities. Changing badges/clears between
  events legitimately changes TR and eligibility; an active event remains fixed.
- Add eligible visitors or vary home assignments/history in controlled fixtures.
  Assert raw category draws remain unchanged; effective category availability,
  selected bucket, and roster may change with inputs. Do not confuse 85%/15%
  category odds with per-person inclusion or exact field composition. Use exact
  fixtures for flat base weights and positive current-circuit/prior-competition factors,
  applied only after selecting the bucket; frequency thresholds cannot replace
  those deterministic tests.
- Sweep at least 10,000 documented roots over at least ten consecutive editions
  using production content, every supported entry world point, and valid completed-event
  history, with both wins and multiple losses at a venue. Report order counts,
  per-venue returning/changed opponents, identical consecutive fields, shared
  opponents within each circuit, home/visitor share, home-only fallbacks, category
  outcomes when both pools exist, role/field strength, per-character inclusion relative to eligible opportunities, and absence over
  documented bounded edition windows. Assert deterministic replay and structural
  invariants. D5 sets quantitative acceptance after measuring these distributions;
  there is no all-current-elites or 80%-per-character attendance target within a
  single circuit. Feasibility counts do not establish sufficient variety.
- Pin growth-key vectors for all percentage outcomes, canonical aliases, both
  root words and policy versions. Verify B=C=0 preserves baseline, 100% reproduces
  the old formula, combined growth rounds once, and modifier derivation never
  changes raw ORDER/POOL_KIND/ROSTER values for unchanged circuit keys. Gyms and
  competitions use the same canonical percentage; badges, visits and losses
  cannot redraw it. Validate captured percentages against root/policy on reload.
- Exhaust integer TR 0–80 for interpolation, saturation, signed offsets, and
  monotonicity. Construct every enabled profile, preserve source identity and
  non-level metadata, and verify identical party reconstruction.
- For identical world point, stage/profile, resolver version, and TR, prove strength
  identical across editions and venues. Band tables are shared at the same
  world point and never widened at runtime. Cover actual badge/first-clear growth between
  events and C = 3 saturation, with no edition/ordinal inflation.
- Vary live player TR, party, badges, clears, location history, and gameplay RNG around a saved
  active event; human participants and ordinary circuit strength remain unchanged.
  Cover species-randomizer bypass separately and verify lifecycle.
- Verify selected-trainer money, XP, AI, healing items, graphics, and dialogue
  follow the selected profile, including a former fixed room occupant's absence.

Run the relevant trainer/scaling mechanics and circuit E2E suites; build Wayfarer
and affected standalone configurations. Measure ROM and RAM against the current
reserve policy. Report balance playtesting separately from structural validation.

## Open questions

The parent PRD and [trainer progression proposal](trainer-world-progression.md)
own the remaining decisions. D1 confirms freezing only the entered competition
and retiring it on loss. D3 must approve concrete progression content, world point
band endpoints, profiles and balance evidence. D4 owns signup/entry gates and
progression balance; first entries near 8/16/24 badges are tentative guidance, not
adopted requirements. D5 owns rotation factors and variety acceptance. D6 owns
competition availability/waiting; no real-time/calendar cadence is approved. Title-agnostic finals and
recurring championships remain approved. No generation can ship before home-only
coverage and variety are established at all supported points. More regions or
doubles require an explicit revision.

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
