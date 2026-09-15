# Sevii content foundation

This is milestone 1 of the [content overlay specification](../../.product/specs/sevii-content-overlay.md).
The 135-map exploration catalog remains enabled. Ordinary Trainers, story scenes,
rewards, statics, and Trainer Tower challenges have no production entries yet.

## Ownership

`game/src/data/wayfarer_sevii_maps.json` owns all event selection. Add a reviewed
source identity to that manifest and an implementation to its owner module;
do not add another event injector or edit the FRLG source maps.

| Stream | Implementation directory | Runtime work supplied by that stream |
| --- | --- | --- |
| Ordinary Trainers | `game/data/scripts/wayfarer_sevii/trainers/` | Selected roster integration, scaling enrollment, persistent defeat routing, complete rematch registry, sight/talk and paired battles |
| Story | `game/data/scripts/wayfarer_sevii/story/` | Objective scripts, state storage/accessors, semantic mainland predicates, battle continuations, recoverable transactions and static encounters |
| Trainer Tower | `game/data/scripts/wayfarer_sevii/trainer_tower/` | Focused capability/asset selection, facility runtime, transient party snapshot, persistent records/prize claim, facility loss routing |

The shared generator owns every imported map-script table. Modules export their
handlers, text, movements and data. `common.inc` remains exploration-owned.

## Host interfaces

All paths below are relative to `game/`.

- `tools/wayfarer_sevii_content/schema.py`: `validate_manifest(root, manifest)`
  validates the schema-v2 envelope and exact source records;
  `selected_records(manifest, domains)` returns the selected domain records.
- `tools/wayfarer_sevii_content/closure.py`: `build_script_closure(root, manifest)`
  checks module exports, references and source dependencies. Owner modules cannot
  redefine generated map-script tables or bypass reference review with raw
  binary includes or label expressions. Exclusions pin source script files and
  label bodies where a whole-map boundary would be too broad.
- `tools/wayfarer_sevii_content/contracts.py`: `validate_contracts(root, manifest)`
  checks the shared allocation and state declarations. `selected_trainer_render`
  compiles the current selected party source in a temporary directory and returns
  selected records with source and compiler provenance. See the
  [contract reference](../../game/tools/wayfarer_sevii_content/README-contracts.md).
- `tools/wayfarer_sevii_scripts/generate.py`: emits the single linkage include.
  `--check` rejects a stale artifact; `--dependencies` lists reviewed recursive
  prerequisites for Make.
- `tools/wayfarer_sevii_content/audit.py`: combines projection, closure, contracts
  and protected baseline evidence. `--rom-report` attaches a measured production
  size report; a host-only invocation explicitly reports ROM usage as unmeasured.

Run the focused checks from the repository root:

```sh
make -C game wayfarer-sevii-content-audit wayfarer-sevii-port-audit
python3 game/tools/wayfarer_sevii_scripts/generate.py --root game --check
```

The checked-in future domains have `enabled: false` and empty inventories.
Development domain selection must keep exploration present. A delivered feature
changes its reviewed manifest inventory and enables its domain in production.
The fixture tests exercise nonempty declarations without adding production actors.

## Save and battle integration

The host contracts reserve identities and validate declarations. This milestone
adds no save fields, runtime accessors, Trainer roster rows, battle dispatch,
scaling entries, or feature capability guards. Existing exploration flags stay
where they are until story introduces the dedicated saved bank.

The first consuming milestone must implement the normal flag/variable API routing,
new-game initialization, save-version update and mechanics tests together with
its payload. Route the generated Sevii Trainer ID range to its dedicated defeat
bitset before the existing appended-HNS remap. Do not reserve the entire partner
ID space in a runtime table or copy the complete FRLG roster.

Source FRLG Trainer IDs remain provenance. A generated ID needs exactly one
party owner and scaling classification. Ordinary wrappers use ordinary defeat
and blackout behavior; objective guards commit only on victory; Tower opponents
use facility outcomes and transient floor state.

Retain the compile-time `SaveBlock3 <= 1624` assertion in `game/src/save.c`.
Storage additions must fit the bound and require no migration for prerelease saves.

## Scope clarification

The story specification previously said defeating the bikers “opens” the path
to Bond Bridge and Berry Forest. Its primary overlay contract preserves the
already-open exploration routes. The corrected wording advances the investigation
without introducing a travel or collision gate.

The overlay's one-way transition rule applies to objective progress and one-time
receipts. Tower's pending prize and Selphy's repeatable request payloads need
explicit bounded reusable transitions, including clearing after a successful
claim. The specification now distinguishes those payloads from completion state.

The starting commit also failed to build standalone HNS: its generated constants
omitted Sevii zero aliases still used by the region-map code. The small template
repair restores those aliases only for standalone HNS. Wayfarer's appended
map-section IDs retain their values.

## Build selection

Use an explicit product with multi-goal builds, for example:

```sh
make -C game BUILD=wayfarer release
```

The existing Makefile's goal-based selection interprets `make wayfarer release`
as the default Emerald product. All recorded foundation builds use `BUILD=`.
Run products serially because map includes are shared within each build tree.
Use GNU Make 4.4 or newer: the product stamp rules use `.NOTINTERMEDIATE`.
Make 4.3 can reuse the previous product's generated maps after a product switch.
