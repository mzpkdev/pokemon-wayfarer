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

## Pull request previews

Same-repository pull requests publish their generated devtooling site at
`https://mzpkdev.github.io/pokemon-wayfarer/preview/pr-<number>/`. The workflow
rebuilds every open same-repository pull request into one GitHub Pages artifact,
so closing a pull request removes its preview on the next deployment. Fork pull
requests are deliberately excluded because their workflow tokens cannot safely
publish Pages content.

Before the first deployment, set the repository's Pages source to **GitHub
Actions**. The preview workflow keeps the published artifact under 950 MiB and
fails before GitHub Pages' 1 GiB site limit.
