# Repository guide

## Layout

- `game/` is the GBA ROM project: engine code, data, maps, generators, mechanics tests, and Makefile build targets. Avoid concurrent builds for different map versions because they share generated map files.
- `e2e/` is the pnpm/Turborepo TypeScript suite that drives a prebuilt E2E-enabled HNS ROM through headless SkyEmu. It requires explicit `SKYEMU_ROM` and `SKYEMU_SYMS` paths and does not build or scan `game/`.
- `.product/` holds durable product documentation: product requirements in `prds/`, implementable behavior in `specs/`, and evidence-based investigations in `research/`.

## Map editing

Never hex-edit or directly byte-patch `game/data/layouts/**/map.bin`. Make static tile, collision, or elevation changes in Porymap configured for the target layout version; consult the layout-version guidance in `game/include/fieldmap.h`. Manual event edits in `map.json` remain allowed when they are schema-valid.

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
