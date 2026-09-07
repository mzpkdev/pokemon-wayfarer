# Native HM catch windows

PRD: [Native HM catch windows](../prds/native-hm-catch-windows.md)
Status: Core implementation complete; full route acceptance pending.
Implemented: Core data and production tests; full route acceptance pending.
Implementation: [Production changes and acceptance evidence](../research/native-hm-windows/revisions/implementation/README.md)
Current revision: [Nearby-access design and evidence](../research/native-hm-windows/revisions/nearby-access/README.md)
Historical attachments: [Original distribution, evidence and reproduction tools](../research/native-hm-windows/attachments.md)

## Scope

Implement for `IS_WAYFARER`. The combined Wayfarer build uses HNS and Hoenn
content, with Kanto supplied by HNS. Do not apply this distribution to standalone
Emerald, FireRed, LeafGreen or HNS builds. Preserve their existing schedules.

For Wayfarer, this specification supersedes the roster, regional-isolation,
repeated-entry, permanent-anchor and blanket successor-reminder requirements
in [native-hm-learnsets.md](native-hm-learnsets.md). References elsewhere to
Chinchou or Wailmer retaining utilities throughout TR 0-80 become roster-level
coverage requirements. Existing route ownership, field-action authorization
and traversal-recovery boundaries remain in force.

The PRD defines product intent and scope. This specification defines behavior
and acceptance. The selected nearby-access proposal defines the species, moves,
levels and enumerated encounter replacements. Exploratory optimizer outputs
and historical alternative patches are evidence, not instructions to replace
the selected proposal automatically. Whirl Islands are out of scope.

## Approved data and provenance

The source baseline is commit `479b0c83aea4ad90feb0af649e83ccb1a5916770`.
The current authoring file is
[`revisions/nearby-access/proposal.json`](../research/native-hm-windows/revisions/nearby-access/proposal.json).
Use its assignment hash recorded in the revision's `results.json`, and verify
that the file and results agree. Preserve the original `proposal.json` and its
117-species, 148-role outputs as historical evidence. They are no longer the
implementation target.

Use these attachments as a consistent set:

- Original `baseline.json`: native learnsets with the old feature injections
  removed in the research model, plus compatibility and pointer provenance.
- Revision `proposal.json`: assigned utilities and candidate insertion levels. A scalar
  means both modes; an object selects `modern` and `legacy` separately.
- Revision `roster.json`: effective ordered learnsets, actual native/addition levels,
  and calculated catch windows after preserving existing native occurrences.
- Revision `roster.md` and `roster.csv`: human-readable and spreadsheet views of those
  effective assignments. Native levels take precedence over candidate levels.

Reject unexplained disagreement between these artifacts. Do not treat a
candidate level in `proposal.json` as permission to move an original native
occurrence. The selected effective inventory has 121 species and 154 roles;
take per-mode addition counts from the generated revision inventory.

Before implementing against a newer base, diff its learnsets, species
compatibility, evolution data and encounter rules against this snapshot.
Preserve intervening unrelated changes. Recompute affected windows and obtain
review for resulting assignment changes; do not overwrite newer native data
with the research baseline.

## Behavior

### Learnset transformation

Use the active normal and legacy sources:

- `game/src/data/pokemon/level_up_learnsets/gen_7.h` for modern mode;
- `game/src/data/pokemon/level_up_learnsets/gen_3.h` and its species selector for
  Generation III legacy mode.

For Wayfarer only, remove the old native-HM feature's anchor injections and
successor reminder additions before applying the new roster. Do not leave
both systems active. Restore native entries suppressed by that feature,
including Chinchou's modern Charge at level 50. Do not disable content flags
globally in production: the extraction script does that only to construct an
isolated research baseline.

For each selected species and mode:

1. Keep every original native entry at its original level and relative position.
2. If an assigned utility already occurs natively, keep its native occurrences
   and add none. Otherwise add exactly one entry at the assigned level, 1-100.
3. Place additions after all original entries at the same level. Multiple new
   entries at one level follow the `proposal.json` move-section order: Cut,
   Flash, Strength, Rock Smash, Surf, Waterfall, Whirlpool, Dive.
4. Retain ascending level order and the source's terminator. Validate both
   `MAX_LEVEL_UP_MOVES` and `MAX_RELEARNER_MOVES`, not only a research constant.
5. Validate the resulting ordered learnset against `roster.json` for this
   snapshot, or against a reviewed regenerated revision after source drift.

No new utility occurrence may duplicate another occurrence of that utility in
the same mode. Existing unrelated native duplicates are not removed to satisfy
this rule. The feature does not change a utility's move data or field PP rules.

Assignments are species-level learnset changes, not wild-only overrides.
Ordinary Trainer, egg, gift or other creation paths that already consume the
same level-up learnsets continue to do so. Audit affected ordinary Trainer
movesets for balance; do not modify explicit custom movesets as part of this
data transformation.

## Utility cap and compatibility

Count Cut, Flash, Surf, Strength, Rock Smash, Waterfall, Whirlpool and Dive.
For each selected species, the union of original native utility types across
both modes and all proposed assignments must have cardinality at most two.
The union along any evolution path touching the roster, including unselected
ancestors and successors, must also remain at most two. Test both mode data
sets; a mode switch must not conceal a third authored role.

This restriction does not cap manually taught, egg or copied moves at runtime.
Do not introduce a new rejection rule for player actions or rewrite an owned
Pokemon's moveset to enforce it.

Every assigned utility must be supported by the repository's species
compatibility inventory or by an original native occurrence. The research
inventory uses historical learnable methods, not only Generation III TMs.
Do not broaden compatibility to make an unsupported assignment pass. Verify
the generated teachable data wherever HM-item fallback is expected, and report
any mismatch with the research criterion for review.

In particular, Chinchou receives Surf/Whirlpool, not Flash. Wailmer retains
native Whirlpool/Dive and receives no Surf. Preserve the attached family
choices, including Psyduck/Golduck Surf/Dive and Barboach/Whiscash
Waterfall/Dive. Unlisted descendants receive no blanket reminder injection.

## Initial moves, evolution and reminders

Keep the production initial-moveset algorithm unchanged. Iterate ordered
entries up to the caught level, skip level-zero entries, ignore a move already
in the current four slots, and append each new distinct move while discarding
the oldest if necessary. A repeated native occurrence is only reconsidered if
its move has already left the current moveset.

A catch window is the inclusive set of levels 1-100 at which that algorithm
produces a moveset containing the utility. It is not a Trainer Rating range
and need not match the other learnset mode. Test presence and absence, including
the levels immediately outside each declared window.

Leveling an owned Pokemon follows normal move-learning choices. Its utility
does not disappear when it crosses the window's upper bound. Evolution keeps
known moves. The current species' ordinary learnset and reminder eligibility
determine relearning; no second level-one copy or special reminder exception
is added. Test an inherited utility absent from an unlisted descendant's
learnset as well as a listed descendant's own utility entry.

Examples from the modern attachment, useful as deterministic fixtures:

| Species | Entry | Expected fresh-catch window |
| --- | --- | --- |
| Geodude | Rock Smash 5; Strength 12 | Rock Smash 5-11; Strength 12-23 |
| Miltank | Strength 25; Rock Smash 45 | Strength 25-44; Rock Smash 45-100 |
| Psyduck | Surf 10; Dive 22 | Surf 10-21; Dive 22-33 |
| Tentacool | Surf 16; Whirlpool 45 | Surf 16-27; Whirlpool 45-100 |
| Staryu | Surf 42 | Surf 42-100 |

## Encounter coverage contract

Keep Trainer Rating projection, slot eligibility, devolution, profile offsets,
time aliases and Standard Rod weighting unchanged. Evaluate the effective
species at the projected caught level, not only the authored slot species.
Underlevel evolved slots may provide predecessors; a high-level base slot
does not automatically evolve. Do not approximate TR 80 as every catch being
exactly level 90.

Enumerate all integer TR values 0-80 in both modes. Maintain at least one
encounter-table witness per region and utility, with Johto, HNS Kanto and Hoenn
reported separately. This is 3,888 region/mode/utility/TR cells. Keep probability
and witness identity, map, method, time and rod in the report. Mark optional,
late, underwater and unclassified areas explicitly. The current research's
filtered regional check covers all cells but does not establish reachability.

For each practical acquisition claim, record its accessible map/bank, traversal
prerequisites, method, applicable times, rod and TR interval. A source requiring
the same utility cannot certify that utility's acquisition. Record a viable
return path too. Nearby routes and neighboring towns are acceptable detours;
the proof need not use the same settlement's encounters. Headbutt use of a
shared Rock Smash table must be verified as an independent interaction.
Kirlia, Huntail and Gorebyss provide no ordinary wild witness in the attached
inventory; Crawdaunt's special HNS placement is not a normal regional witness.

### Selected nearby-access scenarios

Use the revision's `scenarios.json` as the reviewed directional inventory. Its
11 scenarios cover:

| Crossing | Required source areas |
| --- | --- |
| Johto mainland / Cianwood | Reachable mainland/Olivine sources and Cianwood |
| Kanto mainland / Cinnabar | Reachable mainland sources, including Vermilion city/port, and Cinnabar |
| Route 118 | Independently reachable west-bank and east-bank sources |
| Lilycove / Mossdeep / Pacifidlog | Reachable shores in each settlement |
| Blackthorn / Dragon's Den entrance | Blackthorn shore and the documented nearby detour, without Surf |
| Dragon's Den entrance / shrine | Local Whirlpool acquisition before its obstacle, with Surf already available |

Require an accessible land or shore-fishing witness across TR 0-80 in each
mode, with applicable day/night profiles and each Standard Rod quality checked
separately. Do not substitute a daytime witness for a night gap or require the
player to wait. Hoenn's static encounter profile is evaluated in both clock
cases rather than inventing a separate night population.
Water encounters cannot certify acquisition before Surf. The inventory must
also verify rod and capture preparation assumptions; it does not promise
recovery after a player deliberately loses their last usable Surf carrier.

The selected Blackthorn Surf and Den Whirlpool assignments each provide a
qualifying local source across all tested TRs, modes and clock cases. Their
mandatory acquisition proof must not depend on traversing Ice Path.

For the optional Blackthorn westward detour, explicitly require the cleared Ice Path
boulder puzzle that permits the return journey. Route 44 is not directly
walk-connected to Blackthorn despite its nominal map connection. Record the
long cave backtrack as such. Do not credit Route 45's lower encounters merely
because a southbound ledge route reaches them.

The Den's local Whirlpool handover must meet the acquisition floor before the
shrine obstacle. Surf is an allowed prerequisite here; Whirlpool, badge 8 and
the Rocket Hideout HM are not. Preserve native field-use eligibility, avoiding
a badge-8 authorization cycle. Include invisible objects whose script is
`EventScript_Whirlpool` in the obstacle footprint, not only visible sprites.

The original `fixed_ports.json` is a historical same-shore test. The selected
revision's code-traced scenarios and `results.json` replace it as current
nearby-access evidence. Passing these static scenarios is not a universal
save-state proof or production/E2E acceptance.

### Acquisition probability

Report carrier probability conditional on a successful encounter, separately
from bite rate and per-cast probability. Account for all eligible slot weights
and level outcomes. The research uses exact fractions under a uniform-roll
model; runtime modulo bias and lure/lead modifiers are outside that estimate.
Document those assumptions in regenerated reports.

At each scenario/TR/time/mode, require at least one allowed reachable source
with an 8% chance of a catch actually knowing the required move. A source is
one map, encounter method and runtime time profile. It can be an ordinary land
encounter or a successful Old Rod fishing encounter. Sum eligible carrier
species/level outcomes within that source, then take the best reachable source;
do not add probabilities across locations, methods or times. This floor applies
to Surf and the required local Den Whirlpool handover.

An 8% Old Rod result means at least 2% per unmodified cast with its 25% bite
rate. Land probability is conditional on an encounter, not each step. Record
both the best source and any shorter qualifying detour, without presenting
the best-source maximum as a guarantee at every nearby spot. Report Good/Super
Rod availability and odds separately. No lure, lead modifier or waiting for a
different clock case is required to meet the floor.

The selected model has four Super Rod cells at 5%: modern TR 35 at Lilycove
and Route 118 east, in both clock cases. They retain nonzero coverage and
their Old Rod floor passes. The 8% threshold is an Old Rod or land-source
requirement, not a guarantee that every upgraded rod improves utility odds.

Update Wayfarer's `nativeSurfAccessibility` validation/report contract to use
this aggregate known-move criterion and the reviewed directional sources.
Replace obsolete expected Chinchou/Wailmer assertions; keep unrelated rod
profile/weight invariants and standalone-build accessibility tests unchanged.

Update the Johto `protectedAnchors` and Kanto native-HM certificate checks as
well. Replace permanent named-species utility-presence and named accessibility
share assertions with the reviewed roster-level coverage inventory. A retained
Chinchou encounter slot is not evidence that it knows Surf at every projected
level. Preserve unrelated ecology and slot-authoring checks; any actual
species/level changes are limited to the selected replacements below. Preserve
all global weight vectors.

The original modern Lilycove TR 0-1 gap, sub-8% intervals and three alternative
slot replacements remain in the historical evidence. They are not current
unresolved choices. The selected revision meets the modeled nearby-source
floor using its roster and explicit encounter edits, including Route 121 as a
Lilycove detour. Do not apply one of the old Lilycove alternatives on top of it.

### Selected encounter replacements

Apply only `encounter_replacements` in the current proposal, for Wayfarer.
These records are the limited exception to Standard Rod's no-map-specific
species/level-edit boundary. They do not change weights, ten-slot fishing
shape, bite rates, eligibility, projection or selection rules. Preserve all
unlisted entries and standalone-build populations.

Each record has this schema:

| Field | Meaning |
| --- | --- |
| `map` | Exact map constant; all matching authored time profiles are in scope. |
| `method` | Exact encounter method, such as `fishing_mons` or `land_mons`. |
| `slot` | Zero-based raw slot index within that method. |
| `expected_species` | Existing species constant, or an explicit list of allowed existing constants where day/night profiles differ. |
| `species` | Replacement authored species constant. |
| `min_level`, `max_level` | Optional replacement authored bounds; omitted values preserve the matching profile's current bound. |

Resolve and validate every matching profile before editing. Each slot must
exist, its existing species must equal the expected constant or belong to the
explicit allowed list, and its resulting level bounds must be valid and
ordered. Reject missing maps/methods/slots, no-match records, duplicate or
conflicting replacements, and any unexpected species. Stop on source drift
instead of guessing a replacement target. The encounter attachment must show
each old species by time where an allowed list is used.

A fishing replacement edits the authored ten-entry population once. Apply it
consistently to every derived Old/Good/Super Rod view and every matching time
profile; never patch only the Old Rod projection that supplied a witness.
Evaluate the replacement's effective species after normal underlevel
devolution, including any changed whole-slot eligibility caused by authored
level bounds. Regenerate the full scenario and regional reports after all
learnset and encounter edits are applied together.

## Field use and saved Pokemon

Retain known-move eligibility, HM-in-Bag compatibility fallback, fainted-user
eligibility, Egg exclusion, party selection order and existing field checks.
Dive remains subject to Steven's Hoenn grant and the existing HNS seventh-badge
authorization. Native move availability does not activate unsupported field
actions or bypass terrain, destination or story checks.

Do not mutate stored moves on existing Pokemon. New creation, level-up offers
and reminder queries use the revised learnsets normally. No persistent feature
state or save migration is introduced. Prerelease old catches can therefore
retain moves or three-role combinations no longer authored by this roster;
that does not justify a cleanup migration or break the new-data cap.

## Implementation and validation

1. Audit baseline drift and isolate the old feature injections. Keep standalone
   behavior intact with mutually exclusive build guards.
2. Apply the selected single-entry distribution to both modes and the exact
   enumerated encounter replacements. Keep research tooling out of the runtime;
   no moveset override or post-catch patch is needed.
3. Replace Wayfarer's permanent-anchor tests with production-path tests of all
   selected species at every level 1-100 in both modes. Compare complete ordered
   movesets where possible, not only one utility-presence bit.
4. Validate original native entries/order, exact added-entry counts, per-species
   and evolutionary caps, compatibility, and both configured table limits.
   Add same-level ordering and duplicate-suppression regression cases.
5. Enumerate authored levels and TR 0-80 through production projection,
   effective-species resolution and initial-moveset creation. Compare these
   results with regional and directional acquisition evidence. A separately
   implemented Python last-four model is not the production acceptance test.
6. Test evolution, owned-move retention after a window, forgetting and reminder
   behavior. Test field use with the HM absent/present, fainted users and Eggs;
   verify Dive still fails before authorization and succeeds afterward in valid
   contexts. Keep species/learnset randomizers disabled for coverage tests.
7. Build the affected Wayfarer objects and ROM in both modes as applicable, run
   deterministic tests, and compile/test standalone configurations for unchanged
   behavior. Avoid simultaneous builds that share generated map files.
8. Validate the 11 directional acquisition scenarios with the HM absent at
   handover boundaries and lowest-odds intervals. Verify their named banks,
   return paths, time assumptions and the Den Whirlpool obstacle. Meet the
   single-source 8% floor in production. Release playtesting remains separate
   from the code-only design audit; no E2E result is claimed by these attachments.

The documentation approval is complete independently of those implementation
checks. The static revision report passes its selected scenarios under the
documented prerequisites. Do not mark the feature implemented or release-ready
until the production acceptance checks pass.

## Attachment maintenance

Keep all original findings, distribution views, location inventories, source
baseline, probability reports and reproduction scripts linked through the
[attachment index](../research/native-hm-windows/attachments.md). They are
versioned repository attachments, not external or temporary files.

Preserve the approved snapshot when producing implementation evidence. Store
new results under a separately identified revision directory and record its
base commit, assignment hash, source changes and acceptance outcome. Update
PRD/spec links to the reviewed revision without silently replacing the original
research. Optimizer output never becomes authoritative merely because it
scores better.
