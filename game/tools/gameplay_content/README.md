# Gameplay content compiler

This compiler reads the selected map catalog through `mapjson inventory` and
rosters through the production preprocessor and `trainerproc`. Its host inventory
keeps source namespace, physical region, raw events, effective events, and resolved
roster ownership separate. It does not authorize encounters or store generated
indexes in save data.

Run from `game/`:

```sh
make BUILD=wayfarer gameplay-content-generate
make BUILD=wayfarer gameplay-content-check
make BUILD=wayfarer gameplay-content-test
make BUILD=wayfarer gameplay-content-report
```

Normal builds generate before dependency scanning. Inputs and the effective
preprocessor configuration select an immutable directory under
`build/gameplay-content/<product>/<digest>/`. Only a successful generation updates
`current`. `check` compares without rewriting files. Reports are ignored build
artifacts; their size is not ROM usage.

Build products and feature configurations sequentially in a worktree. For isolated
host checks use the CLI's `--output-root /tmp/<unique-directory>` so a test cannot
change the `current` pointer used by a running build. `--domains inventory` selects
the host-only adapter, independently of runtime adoptions.

The map export follows the current compiler, including shared-event ownership and
connection filtering. Existing raw warps are retained because the current ROM
compiler retains them. Synthetic overlay tests preserve the distinction between
raw provenance and effective objects; they do not introduce the proposed Sevii
overlay implementation. Some HNS physical regions require runtime section or
saved-context resolution, which the report identifies explicitly.

For linked resource evidence, use `../gameplay_content_resources/report.py` with
the matching ELF/map, full source commit, exact build command, and captured
configuration file. It checks ROM boundaries against both artifacts, uses the
existing ROM reporter, and records static RAM sections and artifact hashes.
`--base` rejects mismatched product/configuration/toolchain metadata and computes
storage deltas. CPU, stack, and gameplay acceptance need separate evidence.

Encounter policy adoption remains gated on the integrated trainer-only baseline
from PR #85. Discovery alone grants no new battle outcome behavior.

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
