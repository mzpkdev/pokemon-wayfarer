# Shared gameplay content compilation and services

PRD: [Shared gameplay content framework](../prds/gameplay-content-framework.md)
Implemented: No

## Scope

Specify the shared host inventory, declaration formats, runtime boundaries, and
three required adoption phases: progression, NPC services, and trainer inventory
with scaling preservation.
This is a proposed implementation contract; paths and APIs identified as new do
not exist yet. The [research record](../research/gameplay-content-framework.md)
distinguishes existing behavior from recommendations.

Research baseline: `a5dd5178f098cc3ae80fbca73a59e04da0f6f835` on `main`.
Historical encounter evidence additionally used PR #85 at
`6799eb7d9f625fefe0c0983104d38e3b13e7d4d5`. It is research context only: do not
import its trainer-story behavior or treat it as the Phase D baseline. Phase D
preserves current native trainer routing and the separately gated wild-only
trainer-only boundary. Reconcile source changes and record exact commits at the
start of every phase.

## Behavior

### 1. Ownership and identity

| Fact | Authoritative source | Framework responsibility |
| --- | --- | --- |
| Map ID, source selection, physical region, event locations | Existing map JSON and selected map catalog | Read actual catalog output and preserve source/region distinctions |
| Species, evolution, moves, items | Existing domain definitions and generators | Reference IDs; no copied universal species or item catalog in v1 |
| Trainer IDs and compiled roster variants | Existing opponents and trainerproc outputs | Inventory canonical owner, alias, difficulty, and pool relationships |
| Progression curve points | New `game/data/gameplay/progression.json` | Emit shared runtime points and host views |
| NPC service binding | New optional `game/data/maps/<map>/gameplay.json` | Validate live script/event binding; emit membership and lookup views |
| Mart stock and local specialties | Existing `game/src/data/wayfarer_marts.h` | Preserve stock authority; replace only duplicated binding ownership |
| Encounter semantic declaration | Same map sidecar, or new `game/data/gameplay/shared.json` for genuinely shared script callers | Compile typed policy and validate legacy evidence |
| Player state and mutations | Existing Rating, rod, origin, League, battle, and persistence APIs | Pass explicit inputs and call established operations |

Do not store these facts in one runtime struct. Host records may be rich and
joined; runtime representations are owned by each consuming domain.

Use qualified host keys: `(product, sourceNamespace, kind, authoredId)`. Map
records reference canonical map IDs; object bindings use map ID and local ID;
encounters use the map plus an explicit label immediately before the battle
command. Shared scripts declare every applicable map binding or one explicitly
context-independent binding. A command label must identify exactly one command.

Trainer ID identifies a roster actor, not an encounter. A shared trainer may have
multiple encounter records. Base/rematch relationships must be explicit or derived
from the actual rematch source; preserve existing stable dialogue keys.

Keep source namespace, physical region, campaign namespace, starting origin, and
active challenge identity separate. Resolve Hoenn-banked flags and variables
through existing source-constant and persistence machinery. Presentation map
sections and physical location cannot select a state bank.

Generated dense indexes are build-local and never saved. For new index types use
`u16`, reserve `0xFFFF` for invalid, and reject more than 65,535 entries. Existing
stable flags, trainer IDs, map IDs, and scene IDs retain their established numeric
contracts. Sort qualified keys deterministically before assigning new indexes;
scripts reference generated symbolic constants, never numeric literals. A rename
that changes an authored semantic key requires an explicit source update, not an
automatic alias guessed from similar names.

### 2. Host compiler and build integration

Add a Python package at `game/tools/gameplay_content/` with separate adapters for
map catalogs, trainerproc, progression, services, and encounters. Reuse the real
map compiler, trainer compiler, and domain parsers. Do not implement another
partial C or assembler preprocessor with guessed feature flags.

Pipeline:

1. Obtain the selected product, source namespaces, feature defines, and active
   sources from the same build configuration used for the ROM.
2. Compile/read source inventories, then load declarations and validate schemas.
3. Resolve IDs, live event bindings, shared callers, references, and aliases.
4. Expand named domain profiles; reject conflicts and unsupported combinations.
5. Produce consumer projections and a host report with source provenance.
6. Emit temporary files, compare with existing outputs, and replace only changed
   outputs atomically. Failure must not leave a partially updated output set.

The map adapter inventories effective selected content after product-specific
overlays, event retention/removal, script replacement, and path filtering. Raw
source events are provenance, not automatically live bindings. Reuse the map
compiler's effective projection or consume a machine-readable export of it;
do not reimplement overlay rules separately in each audit. Retain both the
original source reference and effective event/handler identity in host output.
An event removed by an overlay must not register a service or trainer encounter.

Imported nurse, PC, daycare, ferry, and environmental bindings may be reported as
opaque authored handlers without a v1 service declaration. Discovery must not
require registering every handler as a supported service or imply that the
framework validates its gameplay semantics. For supported declarations, resolve
bindings against effective events. A script replacement invalidates a binding
to the old handler even when its source event still exists.

Use an output directory keyed by product and a digest of the effective defines,
adapter/schema versions, and relevant inputs under `game/build/gameplay-content/`.
Generated includes must participate in Make dependencies before consuming C or
assembler compilation. Bootstrap ordering is map/source IDs and compiled roster
inventory, then framework outputs, then consuming objects; adapters must not
depend on a framework-consuming final object or ELF. Final symbol checks are a
separate post-link step.

Progression loading is a leaf module that depends only on its JSON schema and
source file. Emit/read its curve view before invoking a wild or trainer audit
that consumes it. Those audits are downstream validation, never prerequisites of
curve generation. This prevents an inventory-to-wild-generator dependency cycle.

Add proposed Make targets `gameplay-content-generate`, `gameplay-content-check`,
`gameplay-content-test`, and `gameplay-content-report`. Integrate generation and
validation into affected normal builds, including `release`; standalone and
feature-disabled builds receive only their selected records. A clean build must
generate missing outputs. `check` generates into a temporary directory and reports
missing/stale outputs without rewriting them. Host reports and temporary output
remain ignored build artifacts. Existing checked-in generated files keep their
current convention until their owner is migrated.

JSON schema version is `1`. Reject unknown versions, unknown keys, duplicate JSON
keys, duplicate qualified IDs, invalid enums, missing references, unsupported
predicates, overflow, and declarations pointing to inactive content within an
active product. Each record has an explicit `products` list; skip other products
before resolving their references. An empty selected consumer inventory is valid
only when its expected scope is empty or disabled.

Product selection is followed by domain-owned feature selection using the actual
preprocessed numeric values, with zero meaning disabled. Declarations do not
carry arbitrary feature expressions or another copy of configuration defaults.

| Projection | Selection rule |
| --- | --- |
| Rod contributions | Existing Standard Rod product path; no new feature gate |
| Mart bindings | `IS_WAYFARER && WAYFARER_TR_MARTS_ENABLED` |
| Ordinary/Gym-member scaling activation | `IS_WAYFARER && B_TRAINER_PARTY_SCALING` |
| Gym Leader scaling activation | `IS_WAYFARER && B_GYM_LEADER_SCALING` |
| League scaling activation | `IS_WAYFARER && B_LEAGUE_SCALING` |
| Future authored trainer-outcome records | A separately approved, dedicated feature gate; the current trainer-only mechanic has no trainer outcome records |
| Progression curve payloads | Union of curves consumed by selected gameplay paths, including existing standalone consumers |

Schema validation applies to every loaded declaration. Binding resolution and
runtime emission apply only to selected domain projections. Report disabled
declarations separately; they neither fail on an inactive converted script path
nor emit runtime membership. Keep their existing legacy script branch unchanged.
Validate bindings in a separate enabled configuration to detect stale disabled
content. Shared host trainer inventory remains complete, while activation and
emission follow each domain's gate; disabling scaling cannot remove a record
selected by an independently enabled future outcome domain. Do not erase
dedicated legacy roster data needed by disabled or randomizer paths.

Diagnostics contain a stable category, source path, record key, and offending
reference. Categories include `SCHEMA`, `DUPLICATE`, `UNRESOLVED`, `INACTIVE`,
`CONFLICT`, `UNSUPPORTED_CONTEXT`, `STALE_REVIEW`, and `CAPACITY`. Reports include
the configuration digest, input hashes, selected counts, unresolved coverage,
legacy coverage, and provenance for each emitted value. Do not emit provenance
strings or full host inventories into the ROM.

### 3. Declaration format and conflict rules

Map sidecars have `schemaVersion`, `services`, and `encounters`; both arrays may
be empty. The containing map supplies map identity and source namespace. Global
`shared.json` has the same version and arrays, plus `legacyTrainers`; each binding
explicitly names its map, or declares context independence after validation.
No executable Python,
arbitrary expressions, or user-defined runtime predicate language is accepted.

A service declaration has `id`, `products`, `kind`, `binding`, and kind-specific
parameters. `binding` requires `script` and may include `localId`. Without
`localId`, exactly one object event in the containing map must reference that
script; derive its runtime ID from the map compiler's object ordering. This
supports existing HNS events that have no authored `local_id`. If several events
share a script, give each selected event an explicit symbolic `local_id` through
the existing map JSON/compiler facility and require that symbol in the binding.
Validate both script and object identity. Never choose the first matching event.

Resolve coordinates and visibility from the actual map event rather than
repeating them. Reordering objects regenerates runtime IDs without changing the
service's authored key or persistent contribution flag. Changing the bound script
requires updating the declaration; losing uniqueness fails validation. A caller
reached indirectly must provide its explicit call-site label and validate that
path; an ambiguous binding is an error.

Required identity values are nonempty strings. When present, `localId` is an
existing symbolic constant declared by that map's object event; raw numeric
object positions are not accepted as authored identity.
`products` is a nonempty unique subset of `wayfarer`, `hns`, `emerald`, `firered`,
and `leafgreen`. A declaration cannot select a product its source does not enter.
`rod_contribution` requires `contribution` and forbids `profile`; `mart` requires
`profile` and forbids `contribution`. Contribution namespaces are existing
persistence domains resolved by the adapter, not arbitrary author-created banks.

Example schema shape for a rod interaction (symbol values are illustrative and
must resolve in the chosen map):

```json
{
  "schemaVersion": 1,
  "services": [{
    "id": "fisherman_rod",
    "products": ["wayfarer"],
    "kind": "rod_contribution",
    "binding": {"script": "Map_EventScript_Fisherman", "localId": "LOCALID_FISHERMAN"},
    "contribution": {"namespace": "global", "flag": "FLAG_STANDARD_ROD_DEWFORD_CONTRIBUTED"}
  }],
  "encounters": []
}
```

For `mart`, replace `contribution` with `profile`, an existing symbolic mart
profile ID. Several bindings may reference one stock profile. Multiple counters
in the same map remain distinct bindings.

Profiles are flat, typed defaults implemented by domain compilers. Only
documented fields may be overridden; arbitrary inheritance and last-writer-wins
merging are forbidden. Duplicate declarations for the same binding and domain
fail even if their values match. Distinct domains may share an interaction only
through an explicitly supported combination. V1 supports rod and mart as separate
interactions, not a new combined reward/shop flow.

### 4. Progression adoption

`progression.json` defines named curves with `id`, `points` (Rating/value pairs),
and `interpolation: "nearest_half_up"`. V1 accepts monotonically nondecreasing
integer values in 0..100, strictly increasing integer Ratings in 0..80, and
endpoints at 0 and 80. No runtime floating point is used.

Migrate these named baseline curves:

| Rating | 0 | 4 | 8 | 16 | 30 | 40 | 55 | 65 | 80 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `wild_baseline` | 5 | 6 | 8 | 13 | 20 | 32 | 50 | 70 | 90 |
| `ordinary_trainer_baseline` | 7 | 8 | 10 | 15 | 22 | 34 | 52 | 72 | 92 |
| `soft_cap` | 15 | 16 | 18 | 23 | 30 | 42 | 60 | 80 | 100 |
| `gym_baseline` | 15 | 16 | 18 | 23 | 30 | 42 | 60 | 80 | 100 |
| `league_baseline` | 15 | 16 | 18 | 23 | 30 | 42 | 60 | 80 | 100 |

Keep named curves logically independent even when their payloads are identical.
Intern exact identical payloads at generation time. Do not enforce a permanent
wild+2 or wild+10 balance relationship. The wild generator reads its baseline
from this source; zone retention, source levels, minimum species levels, and
profile offsets remain in their existing domains. Remove the old writable
`levelAnchors` copy when migrating its readers; retain the other wild JSON fields.

For input `r`, clamp to 0..80. Between `(r0,v0)` and `(r1,v1)`, evaluate
`v0 + (2 * (r-r0) * (v1-v0) + (r1-r0)) / (2 * (r1-r0))` using integer division
and unsigned 32-bit intermediates. Exact endpoints return their exact values.
Existing public functions retain their signatures and delegate only baseline
evaluation to a new private/shared evaluator taking `(curveId, rating, out)`.
Invalid IDs return failure without changing `out`; a consumer follows its current
invalid/disabled path and never substitutes an unrelated curve.

Preserve authored-level retention, Gym-member additions, slot offsets, party-size
thresholds, clamping order, and product-specific Rating getter behavior. Do not
move Rating production or challenge policy into the evaluator. The Rating input
is explicit: marts sample on open, ordinary/Gym parties at existing setup, and
Leagues use their persisted admission value. Clear/retry/load behavior remains
owned by the current lifecycle code. Host Gym/League audits consume the same
points; keep independent golden expected results for equivalence tests.

### 5. Service adoption

Generate one canonical selected contributor set, referenced by both membership
validation and counting in `game/src/item.c`. Validate one contribution identity
per distinct giver; an intentionally shared contribution requires an explicit
alias and counts once. Maintain existing product selections: Wayfarer/HNS six
contributors and each standalone product's existing three. Validate selected
scripts against the set so a live giver cannot silently call an unsupported flag.

Use existing flag values and bank-aware access. The framework does not assign
new persistent indexes or infer flag storage from the giver's region. Preserve
the current award transaction, distinct-contributor counting, upgrade cap, failure
rollback, registered item handling, and giver-specific follow-up dialogue. This
phase changes binding/membership ownership, not reward semantics.

For marts, migrate all existing converted Wayfarer bindings into declarations.
Keep item arrays and stock profiles authoritative in `wayfarer_marts.h`, including
category masks, local signatures, retained items, PP supplements, order, and
deduplication. The existing catalog resolver and 64-item buffer remain. A script
may invoke a generated binding constant that resolves to its stock profile; it
must not require a second hand-authored binding row. Existing shared clerk lookup
derives from these declarations with exact map/caller context.

Change the mart audit to consume the shared inventory for bindings and its
existing production parser for stock. Generate NPC coordinates, visibility, and
profile coverage evidence. Remove superseded manual binding metadata after
equivalence, rather than keeping it as another editable manifest. Leave specialist
shops and unconverted counters outside this service until explicitly adopted.

### 6. Encounter adoption

Phase D reconciles current trainer scaling with the current wild-only trainer-only
mechanic. The current baseline has no trainer caller registry, refusal, retreat,
deferred-rival, or non-victory outcome module. Inventory every compiled trainer
record and every discovered battle call site, including excluded and unmigrated
ones. Unknown discovery must be visible in reports but must not gain framework
authorization.

An encounter declaration includes `id`, `products`, exact `caller`, optional
`baseEncounter`, and a `profile`. The compiler resolves actual trainer arguments
and command mode from compiled source. Dynamic trainer selection must enumerate
its validated possibilities or remain an explicit legacy adapter. Never infer a
single trainer from the label's spelling.

V1 inventory records use `legacy` native outcome routing and a separate
`scalingPolicy`; no trainer-only outcome policy is adopted. A later, separately
approved author-owned trainer-outcome capability may introduce `ordinary`,
`objective_guard`, and `deferred_rival` profiles with an independent
`outcomePolicy` domain. That future capability must supply its policies explicitly
and prove its command/lifecycle contract; a role name alone cannot enable it.

Preserve existing trainer-level scaling inventory as a generated projection.
In v1, all callers of a trainer ID must agree with its established scaling policy;
conflicts fail generation and remain legacy until deliberately resolved. This
avoids silently changing the current shared-ID semantics. Any future outcome policy
must stay caller-specific. A trainer ID, class, or Gym location cannot authorize
loss return.

`legacyTrainers` covers roster IDs not fully described by migrated callers,
including unplaced, dynamic, facility, and excluded records. Each row requires
`id`, `products`, `sourceNamespace`, `trainer`, `scalingPolicy` (one of `ORDINARY`,
`GYM_MEMBER`, `GYM_LEADER`, `EXCLUDED`), and `review`. `review` includes source references and a
reason for exclusions or mixed-role decisions. Import the current reviewed
classification into these rows; derive structural roster facts and region evidence
from inventory instead of copying them. When all callers of an actor migrate,
derive its policy from their agreed contracts and remove its fallback row.
While a fallback remains, migrated callers must agree with it. Remove the old
writable classification inventory once its ownership moves; consumers may retain
a generated compatibility view. A newly discovered roster with neither complete
declarations nor a reviewed fallback fails coverage validation.

`GYM_LEADER` delegates to the existing dedicated Gym roster compiler, retention
and ordering rules, and `B_GYM_LEADER_SCALING` gate. It never falls through to
ordinary scaling when that gate is disabled. Migrating its classification does
not require converting every leader's outcome script. League, rival, and facility
IDs classified `EXCLUDED` retain their existing separately owned special handling;
excluded here means excluded from the ordinary/Gym policy projection, not disabled
in every gameplay system. Validate against the complete selected baseline policy
enum rather than silently mapping new upstream categories to an existing value.

The `legacy` adapter stores reviewed policy and command provenance in the host
inventory. Import current classification evidence and reviewed caller fingerprints;
changed relevant command blocks invalidate review. Retain needed runtime adapters
for unconverted scripts. A reviewed source fingerprint is an audit mechanism,
not a runtime field or proof of arbitrary script semantics.

If a later author-owned trainer-outcome capability adopts an `ordinary` template,
it must identify entry handling, supported battle mode, success continuation, and
non-victory continuation. Preserve completed-trainer aftertext and rematch
identity. A future `objective_guard` additionally supplies its pending/completed
state predicate and retreat script. A future `deferred_rival` also binds the scene
ID, trigger, object lifecycle, rearm region, and eligibility predicate. Use
existing named C predicates or script continuations through a closed domain
adapter, not a generic predicate interpreter. Derive object coordinates from map
events where equivalent; explicitly retain authored activation rectangles that
differ from those coordinates.

Current trainer callers retain native routing; this framework must not infer party
eligibility, refusal, retreat, deferral, or recovery behavior from trainer-only.
If a later author-owned outcome capability is approved, its order must be existing
product/challenge/battle-mode exclusions, exact caller contract resolution, battle
setup with captured context, then outcome dispatch. Only the existing victory path
may perform victory effects. Its non-victory path must use its declared continuation
and clear transient context on every finish, cancellation, new encounter, map
reload, and recovery path. Restoration of a transient rival must check that its
chapter is still pending.

Map-local lookup must include map context whenever one script has different
contracts at different sites. Context-independent callers may use one shared
entry. Never bind runtime identity to a trainer ID alone or to an unverified label
address: post-link validation proves the label points at the intended battle
command and uses the correct argument offset for that command encoding.

Required Phase D coverage is all trainer scaling classifications into shared host
inventory/generated projections. Current trainer callers remain native legacy
routing. A future author-owned outcome capability must separately cover every
adopted caller, including at least one objective guard and one deferred rival when
those profiles are introduced. It must not broaden the set of battles allowed to
return after loss. Existing challenge, partner, chained, scripted-wild, unaudited
Gym, randomizer, and disabled-feature paths retain their current routing.

### 7. Runtime representation and future extensions

Prefer existing data references, small profile IDs, shared exact payloads, and
sparse exceptions. Ordinary entries must not carry unused rival trigger fields.
Store scene-only fields in scene records. Select linear, sorted, or dense lookup
per domain from measured total cost; do not add a global runtime hash table.
Never persist pointers, build-local indexes, or host keys in save data.

Generate compact membership once when several consumers need the same fact.
Separate projections remain valid when their scopes differ and sharing a union
would cost more. Require size attribution for both the proposed shared payload
and its indexes/code. Removing roster copies or runtime validation is a separate
measured optimization: retain safety checks needed for dynamic aliases, difficulty,
pools, randomizers, and invalid runtime IDs.

The host inventory exposes extension adapters rather than preallocating runtime
fields for hypothetical features. Future species queries must reuse canonical
evolution/learnset facts while preserving encounter-specific floors. Future
acquisition checks must distinguish obtainable, currently catchable, and reachable
before a capability gate. Future travel profiles must preserve authored route
direction and story voyages. No reachability proof, travel migration, or universal
species index is delivered by v1.

#### Sevii import and tooling candidate

[PR #92](https://github.com/mzpkdev/pokemon-wayfarer/pull/92), examined separately
at open head `85748ebb6731a562b110cfef18034f664518f006`, adds a concrete candidate
for later adapters. It is not part of the research main baseline or a prerequisite
for phases A-D. If it lands before implementation, the selected-content inventory
must observe its effective overlay; this does not require migrating its gameplay
services or encounter-data generation in v1.

| Candidate surface | Proposed refactor | Behavior or ownership to retain |
| --- | --- | --- |
| `wayfarer_sevii_maps.json`, map compiler, port audit | Derive structural IDs/layouts and effective event/path views once | Explicit map scope, retained/removed events, replacement handlers, protected event-island exclusions |
| Sevii script-table generator | Reuse inventory, include resolution, and deterministic emission infrastructure | Reviewed Wayfarer-owned handlers and empty tables; no import of FRLG story bodies |
| Python wild generator and Cartographer encounter join | Publish one resolved FRLG source-pair view consumed by both tools | Species choice, slot order, source levels, weights, rates, omissions, and day/night alias policy |
| Fixed-stock marts and other NPC services | Add domain adapters only when separately specified | Exact stock and existing healing, PC, daycare, and follow-up behavior |
| Ferry menus and routes | Later travel profiles with explicit destination identity | Direction, cancellation, ticket/origin conditions, and protected Birth Island/Navel Rock routes |

Source event snapshots/indexes in the port manifest are deliberate drift guards.
A later compiler adapter may generate normalized review snapshots or fingerprints,
but must still reject relevant source changes until reviewed. An automatically
refreshed fingerprint cannot silently approve a changed retained event. Import
scope and retain/remove decisions remain authored even when structural fields
move to generated reports.

The Python/TypeScript encounter consolidation is a future data-adapter task,
separate from v1 trainer battle contracts. Preserve independent golden examples
and source comparison tests when both consumers share a generated view; agreement
between two consumers of the same incorrect output is not proof of correctness.
The regional port is trainer-free and adds no required trainer migration coverage.
Expected benefits are less repeated authoring and tooling drift; savings in map,
layout, or graphics ROM are not implied by host inventory consolidation.

### 8. Resource acceptance

Before each runtime phase, create paired clean release builds with identical
toolchain, product, feature switches, optimization, and authored content. Record
base/head commits, config digest, commands, ELF/map hashes, and used ROM from the
existing ROM reporter. Use linked used bytes, not padded `.gba` file length or
source-file size. Report code, read-only data, alignment, generated index costs,
and retained legacy paths without double-counting symbols.

Host-only phase A must introduce zero runtime bytes. Each runtime phase must have
`headUsedRom <= baseUsedRom`, no static EWRAM/IWRAM increase, no new save fields or
heap allocation, and must pass existing capacity/reserve checks. Across completed
phases B-D the sum of equivalent before/after ROM deltas must be negative. If
other content changes intervene, use per-phase paired deltas and also produce a
final same-content comparison with all framework adoptions reverted. Do not
compare main without trainer-only gameplay to a head that includes it.

Existing caller-owned buffers and snapshots are reused. For new or changed hot
queries, compare worst-case cycles and stack high-water under the same emulator
or hardware setup. Allow at most 256 additional CPU cycles per migrated query,
at most 4,096 additional cycles across all framework queries in any frame, and
at most 64 additional bytes of peak stack depth against the equivalent baseline.
These proposed ceilings permit bounded call/lookup overhead when it saves ROM;
they are acceptance budgets, not claims about current execution cost. New
per-frame callbacks or whole-world scans remain prohibited. Record maximum
observed input sizes and the measurement method. Code inspection must establish
loop/call-frequency bounds and stack depth beyond sampled timing. Host-only
processing has no runtime budget. A phase that misses a gate stays
unaccepted; improve its representation rather than removing content or weakening
the protected reserve. Numbers in this section are acceptance limits, not
measured savings claims.

### 9. Validation and delivery

| Phase | Deliverable | Exit evidence |
| --- | --- | --- |
| A | Build adapters, schema validation, inventory, deterministic outputs, baseline reports | Repeated clean generation is byte-identical; wrong-product and stale inputs rejected; zero runtime delta |
| B | Shared progression source and migrated gameplay/host consumers | Exact outputs at every Rating 0..80, clamped inputs, curve boundaries, existing modifiers, and unchanged snapshot lifecycle |
| C | Rod and mart declarations, generated membership/bindings, updated audits | All selected contributors/counters covered; transaction and catalog equivalence; authoring exercises pass |
| D | Trainer inventory/adapter and required scaling migration coverage | Scaling equivalence, caller validation, native-routing preservation, and final resource gates |

Phases B and C may proceed independently after A. D consumes the inventory and
preserves the current wild-only trainer-only boundary; it requires no trainer-story
or trainer-outcome prerequisite. It does not block earlier phases. Do not mark this
spec implemented until all four exit gates pass.

Tests must include independent pre-refactor expected results, not only two outputs
from the new generator. Retain meaningful mechanics and emulator journeys. Add
negative fixtures for duplicate IDs/JSON keys, inactive references, wrong source
bank, conflicting shared callers, invalid curve points, oversized indexes, stale
review, and unsupported battle modes. Test enabled and disabled feature paths.

Phase A includes a synthetic overlay fixture with a removed trainer, a replaced
service handler, a retained opaque nurse handler, and a filtered warp. Assert that
only effective bindings/paths enter selected inventory, that raw provenance stays
available, and that an old-handler declaration fails resolution. A source event
change must invalidate its reviewed fingerprint. This tests the import contract
without depending on PR #92 or importing its content into this task.

For progression, compare all Rating values, trainer authored levels 1..100 for
each existing scaling policy, existing Gym/League offset ranges, and wild effective
population regressions. Exercise League admission, Rating changes during the run,
save/load, defeat, and retry. For rods, exercise every distinct three-giver order
in Wayfarer (120), duplicates, completed progression, transaction failure, shortcut
preservation, and mixed-region save/load. For marts, compare all profiles across
0..80 and relevant challenge settings including exact item ordering and terminator.

For current encounter inventory, compare all generated scaling assignments against
the native caller baseline and prove that newly discovered callers cannot gain
outcome authorization. A future author-owned outcome capability must separately
cover its adopted actual-win/loss paths, completed aftertext, rematches, refusal,
objective retreat, deferred-rival restoration, and challenge exclusions. Forced
outcomes may cover rare lifecycle boundaries but cannot replace real input win/loss
journeys for that future capability.

Authoring fixtures add/remove a temporary rod service, add a mart binding using an
existing profile, and add/remove a native-routed trainer caller in the host
inventory. Require generation and consumer tests to reflect each change without
consumer table/code edits. The new fixture's expected player behavior must still
be asserted explicitly; generated inventory coverage is not an independent
gameplay oracle. A future encounter-outcome profile needs its own approved fixture.

At runtime-phase release acceptance, build Wayfarer, HNS, Emerald, FireRed, and
LeafGreen sequentially per worktree, preserving existing feature defaults. Never
run different map-version builds concurrently in the same tree. Run affected
mechanics, generator/content audits, and scoped emulator journeys with explicitly
matched ROM/symbol artifacts. Link map checks prove excluded product records and
host evidence are absent from the release.

Keep each phase revertible by commit without a permanent duplicate runtime path.
On rollback restore the prior generator/source ownership and regenerate outputs
for the same content/configuration. Existing gameplay feature switches retain
their meaning; do not add one global switch that silently disables unrelated
services. Preserve upstream adapters and test standalone behavior when updating
the imported game subtree.

### 10. Expected code surfaces

New work belongs in `game/tools/gameplay_content/`, `game/data/gameplay/`, map
sidecars, generated includes under the configuration-specific build directory,
and narrow gameplay helpers under `game/include/` and `game/src/` as needed.
Integrate dependencies in `game/Makefile` and `game/map_data_rules.mk`.

Primary migrated consumers are `trainer_rating.c`, `trainer_party_scaling.c`,
the wild generator, trainer scaling host tools, `item.c`, `wayfarer_marts.c`,
and mart binding/audit tooling. Phase D may add trainer inventory and scaling
adapters, but no trainer-story or trainer-outcome runtime module. The current
wild-only trainer-only controller remains separately gated. Extend existing tests
in their owning domains. No Devtools authoring UI is required; machine-readable
inventory can support one later.

## References

- [Research and historical PR evidence](../research/gameplay-content-framework.md)
- [Runtime foundation](wayfarer-runtime-foundation.md)
- [Trainer party scaling](trainer-party-scaling.md)
- [Gym scaling](gym-leader-scaling.md)
- [League scaling](league-scaling.md)
- [Standard Rod](standard-rod-fishing.md)
- [Mart behavior](global-tr-pokemarts.md)
- [Regional starts](wayfarer-regional-start-choice.md)
- [Trainer-only wild mechanic and no-story boundary](trainer-only-story-encounters.md)
