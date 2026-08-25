# Pokémon Wayfarer developer tooling

This is an isolated Node 24 workspace for tools that support Pokémon Wayfarer.
It does not participate in the ROM's Makefile build.

The ROM source tree is in the repository's `game/` directory. Workspace commands
select it automatically and write generated catalogs to the repository-level
`build/` directory, leaving the ROM build output untouched.

The workspace uses pnpm workspaces, Turborepo, WebAnvil, TypeScript 7, and Ark
UI in the Svelte app. WebAnvil owns builds, formatting, linting, test commands,
browser tests, and declaration generation.

## Layout

```text
tools/      CLI packages
ui/         Svelte cartographer
```

`tools/cartographer` is the source-driven map-render CLI. `tools/metatiles`
generates source-driven metatile metadata and palette-correct preview atlases.
Add further CLI tools under `tools/`. `ui` is the only browser package; add a
separate UI package only when it has a real independent consumer.

`ui` is the Svelte cartographer. It consumes the static catalog and terrain images
created by `tools/cartographer`; it does not read the ROM or source
tree in the browser.

`ui/src/modules/cartographer/ui-toolkit` owns styled local UI primitives. It wraps Ark
UI for stateful controls so cartographer components can stay focused on map behavior.

## Commands

Install dependencies once from the repository root with `pnpm install`. Then run
these commands from `devtooling/`:

```sh
pnpm run build
pnpm run check
pnpm run dev
pnpm run format
pnpm run lint
pnpm run test
pnpm run e2e
pnpm run clean
pnpm run cartographer Route101
pnpm run catalog
pnpm run cartographer:catalog
pnpm run metatiles:catalog
pnpm run cartographer:ui
```

`pnpm run check` runs formatting, linting, and TypeScript checks across the
workspace. Build output, dependency installs, Turborepo cache, and WebAnvil
metadata are ignored by Git.

`pnpm run dev` regenerates the Cartographer and metatile catalogs before
starting the Svelte app. Restart it after changing map source files so the UI
serves current map images.

`pnpm run e2e` and `pnpm run build` likewise regenerate a fresh catalog before
running their respective UI commands. `pnpm run build` then stages the compact
UI bundle beside that catalog, making `build/cartographer/map-catalog/` a
self-contained static site. It copies only the small UI bundle, not the catalog
itself. CI follows the same path from a clean catalog directory.

`pnpm run clean` removes package build output and the generated Cartographer and
metatile catalogs under the repository's `build/` directory. It does not touch
the ROM build output.

`pnpm run catalog` recreates both catalog types in
`build/cartographer/map-catalog/`. `pnpm run cartographer:catalog` and
`pnpm run metatiles:catalog` are aliases for that complete refresh.
Catalog JSON is compact in every command, including local development, to keep
the generated site practical to serve and publish.
`pnpm run cartographer:ui` then starts the Svelte cartographer.

The metatile refresh writes palette-correct context atlases with the map catalog.
A context is a real primary and secondary tileset pairing, because metatile palette
slots are resolved by the pairing.

`pnpm run cartographer` renders requested maps to
`build/cartographer/map-renders/`; pass a path below `../build/` to `--output`
when choosing another root-level destination. It builds Cartographer's compiled
`main` entry first. The direct linked binary needs the sibling source tree and
root build output explicitly, for example:

```sh
pnpm exec wcartographer --repo ../game --output ../build/cartographer/map-renders Route101
```
