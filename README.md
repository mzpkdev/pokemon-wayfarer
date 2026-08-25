# pokemon-wayfarer

## TypeScript E2E workspace

The repository uses pnpm and Turborepo for the TypeScript E2E workspace. The
E2E suite stays in `e2e/`; the `skyemu-static` dependency provides its pinned
SkyEmu binary.

```sh
pnpm install
pnpm run check
```

See [e2e/README.md](e2e/README.md) for the ROM-backed test command.

## Developer tooling

The source-driven Cartographer and Metatiles tools live in their own
[devtooling workspace](devtooling/README.md). Its commands read the `game/`
source tree and keep generated catalogs in the repository-level `build/`
directory:

```sh
pnpm install
pnpm --dir devtooling run check
```
