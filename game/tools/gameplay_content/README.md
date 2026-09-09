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
