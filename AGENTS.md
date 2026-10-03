# Repository guide

## Layout

- `game/` is the GBA ROM project: engine code, data, maps, generators, mechanics tests, and Makefile build targets. Avoid concurrent builds for different map versions because they share generated map files.
- `e2e/` is the pnpm/Turborepo TypeScript suite that drives a prebuilt E2E-enabled HNS ROM through headless SkyEmu. It requires explicit `SKYEMU_ROM` and `SKYEMU_SYMS` paths and does not build or scan `game/`.
- `.product/` holds durable product documentation: product requirements in `prds/`, implementable behavior in `specs/`, and evidence-based investigations in `research/`.

## Map editing

Never hex-edit or directly byte-patch `game/data/layouts/**/map.bin`. Make static tile, collision, or elevation changes in Porymap configured for the target layout version; consult the layout-version guidance in `game/include/fieldmap.h`. Manual event edits in `map.json` remain allowed when they are schema-valid.

## Save compatibility

The ROM hack is not yet publicly released. Backward compatibility with save data from prerelease builds is not required. Do not add migrations or preserve obsolete save layouts solely for prerelease saves, because doing so creates technical debt before the first release. Revisit this policy only when an explicit product decision or public release establishes a save-compatibility baseline.

## RAM rules

The GBA has 256 KB of EWRAM and 32 KB of IWRAM, and the system stack lives at the top of IWRAM with no overflow check.

- Declare new buffers and state `EWRAM_DATA`. A plain `static` or global lands in IWRAM and shrinks the stack, which grows down into it; an IWRAM rollback snapshot once overflowed the stack into the heap (#136).
- Use IWRAM only on purpose, for tiny, hot data or code.
- Put temporary memory on the heap with `Alloc`/`Free` for the feature's lifetime only. Starting a battle and reloading the field (warps, returning from menus) reset the heap, so code must not hold heap pointers across them and must survive the reset.
- Keep constant data in ROM tables (`const`), never in RAM caches built at runtime.
- Pack saved and RAM structures: bitfields, the smallest fitting integer types, no padding.
- The mechanics-test build (`make check`) is the tightest: its EWRAM is almost full and its stack has the least room. Check new RAM against it, not only the release ROM.
- `game/tools/ram_report/ram_budget.json` holds per-build ceilings that every release, `make e2e`, and `make check` build enforces. Raising one is a deliberate, reviewed change in the PR that needs it. The release build's 4 KB stack and 4 KB EWRAM free floors are fixed in `ram_report.py`.

## Merge policy and upstream updates

Normal pull requests use squash merges only, with linear history required on `main`.

For `game/` upstream imports, follow [the upstream integration guide](docs/upstream-integration.md). Temporarily enable merge commits in the repository settings and the `Protect main` ruleset, and disable the linear-history requirement. Land the update with a merge commit, never squash or rebase, so upstream ancestry survives. Verify the imported upstream commit is an ancestor of `main`, then immediately restore squash-only merging and the linear-history requirement. Keep required checks and all other protections intact throughout.

## Agent workflow

A root agent, meaning an agent not spawned by another agent, acts as an orchestrator. Delegate bounded discovery, implementation, research, and review to native Codex subagents so the root context stays focused on decisions and synthesis. The root agent must inspect the resulting work, reconcile overlaps, and run relevant verification before reporting completion.

Available native roles are:

- `runner`: read-only repository reconnaissance.
- `explorer`: fast answers to specific codebase questions.
- `fixer`: open-ended read/write implementation.
- `worker`: implementation with explicit file or module ownership.
- `critic`: read-only adversarial review.
- `scanner`: read-only external research.
- `default`: general-purpose delegated work.

Subagents should stay within their assigned scope, avoid reverting concurrent work, and report changed files, findings, and exact validation results to the orchestrator.
