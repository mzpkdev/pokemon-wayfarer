# pokemon-wayfarer

## TypeScript E2E workspace

The repository uses pnpm and Turborepo for the TypeScript E2E workspace. The
E2E suite stays in `e2e/`; the `static-skyemu` dependency provides its pinned
SkyEmu binary.

```sh
pnpm install
pnpm run check
```

See [e2e/README.md](e2e/README.md) for the ROM-backed test command.
