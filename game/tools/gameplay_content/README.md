# Gameplay content compiler

This compiler reads the selected map catalog through `mapjson inventory` and
rosters through the production preprocessor and `trainerproc`. Its host inventory
keeps source namespace, physical region, raw events, effective events, and resolved
roster ownership separate. Encounter declarations authorize only reviewed caller
contracts; discovery alone grants no outcome behavior. Generated indexes are never
stored in save data.

Run from `game/`:

```sh
make BUILD=wayfarer gameplay-content-generate
make BUILD=wayfarer gameplay-content-check
make BUILD=wayfarer gameplay-content-test
make BUILD=wayfarer gameplay-content-report
```

`gameplay-content-contract-test` is the permanent host-contract leaf and runs
through the existing Expansion Suite's `make check`. The local
`gameplay-content-test` aggregate also runs `gameplay-content-migration-test`.
That migration-equivalence target has its own temporary CI job and is retired
after the remaining v1 content is ported and equivalent domain coverage replaces
its legacy comparisons.

Normal builds generate before dependency scanning. Inputs and the effective
preprocessor configuration select an immutable directory under
`build/gameplay-content/<product>/<digest>/`. Only a successful generation updates
`current`. `check` compares without rewriting files. Reports are ignored build
artifacts; their size is not ROM usage.

Build products and feature configurations sequentially in a worktree. For isolated
host checks use the CLI's `--output-root /tmp/<unique-directory>` so a test cannot
change the `current` pointer used by a running build. `--domains inventory` selects
the host-only adapter, independently of runtime adoptions.

The map export follows the current compiler, including shared-event ownership,
connection filtering, and the selected Wayfarer Sevii manifest. Raw provenance
remains separate from the effective events and connections produced by the ROM
compiler. Manifest changes invalidate the generated inventory. Some HNS physical
regions require runtime section or saved-context resolution, which the report
identifies explicitly.

For linked resource evidence, use `../gameplay_content_resources/report.py` with
the matching ELF/map, full source commit, exact build command, and captured
configuration file. It checks ROM boundaries against both artifacts, uses the
existing ROM reporter, and records static RAM sections and artifact hashes.
`--base` rejects mismatched product/configuration/toolchain metadata and computes
storage deltas. See [RELEASE_PROFILING.md](RELEASE_PROFILING.md) for the paired
release comparison and limited release inspection, and
[VALIDATION.md](VALIDATION.md) for the behavioral checks. These are explicit
refactor evidence, not routine build or CI work.

Encounter policy adoption uses PR #85 rebased on merged framework main
`d99050250df47fe785736a7932d0dc33ef22031a`. The unrefactored comparison baseline is
`f4f602d0dd5e00edae4b49f1f4992d217ed02ed2`. Keep the refactor comparison separate
from PR #85's incremental cost above main. Phase-D validation is recorded in
[PHASE_D_VALIDATION.md](PHASE_D_VALIDATION.md).

Services are authored in map-local `gameplay.json` sidecars. Script-only bindings
require exactly one matching effective object; use an authored symbolic local ID
when that script is shared. Generated mart assembler aliases and shared-clerk rows
come from these declarations. Stock remains in `src/data/wayfarer_marts.h`.
An intentional shared rod contribution uses
`contribution.aliasOf: "MAP_CANONICAL/service_id"`; its flag and namespace must
match that selected declaration. Both interactions are validated, but the shared
contribution counts once. Alias cycles and missing targets fail generation.

Standalone HNS already accepts the same six rod contribution flags as Wayfarer,
although only three giver maps are selected there. Its domain adapter derives the
other contribution identities from validated Wayfarer declarations and reports
them as inactive legacy membership. They are not active HNS service bindings. This
preserves the existing membership without a second writable flag list or fictitious
map inventory. Emerald and FRLG keep their existing three contributors.

Baseline numerical helpers remain callable in standalone and feature-disabled
builds. Their curve union therefore includes ordinary, soft-cap, Gym, and League
baselines; gameplay policy still decides whether a battle uses the result. Exact
identical point arrays are interned, but curve names stay independent. Wild points
are consumed by the host generator and do not add a runtime curve.

Encounters use map-local sidecars or an explicit context-independent contribution
in `data/gameplay/shared.json`. Each declaration names a reviewed caller label,
its source and command fingerprint, and separate scaling/outcome policies. The
adapter runs the production script preprocessor, CPP, and ARM assembler to select
actual emitted calls. Every discovered call appears in the report; an unreviewed
call never enters the outcome registry. Post-link validation checks each declared
label's opcode, mode, trainer argument, and argument offset in the final ELF.

The ordinary template preserves completed aftertext, rematch identity, dialogue
keys, and the compact eight-byte runtime rows. Rusturf's Aqua objective and the
Route 110 rival use closed scene adapters with their existing predicates and
continuations. The rival declaration explicitly retains its activation rectangle,
which differs from the map object's coordinates. Other regional scenes remain
reviewed legacy adapters.

Shared `legacyTrainers` rows own fallback scaling classifications. When every
compiled caller for an actor has an agreed declaration, the fallback is removed.
A newly discovered undeclared caller then fails completeness validation. Numeric
or dynamic trainer operands without a supported closed adapter fail generation. This
keeps scaling policy independent from permission to return after losing a battle.
The old trainer-scaling CLI is a compatibility consumer of these declarations;
the frozen encounter snapshot under `migration_tests/` is test evidence only.
