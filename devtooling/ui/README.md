# Pokémon Wayfarer Cartographer

Cartographer is a local developer tool for a Pokémon Wayfarer source checkout.
It reads the checkout's generated catalog and image assets, then places
default-visible exterior maps from their cardinal source connections.

Run these commands from `devtooling/`:

```sh
pnpm run dev
pnpm run e2e
```

`pnpm run dev` generates the Cartographer and metatile catalogs before starting
the app. Use `pnpm run catalog` when you need to refresh the generated data
without starting the development server.

`wa build` writes a compact client bundle to `ui/dist`. `pnpm run build` from
the parent `devtooling/` directory stages that bundle alongside the generated
catalog in `build/cartographer/map-catalog/`, which is the standalone
static-host deployment artifact. `ui/dist` remains a bundle only.

The cartographer provides region selection, map and map-section search, URL-persisted
map selection and camera state, native and overview image switching, map facts,
and warp navigation. Its generated input is ignored under
`build/cartographer/map-catalog/`.

`src/App.svelte` owns the page shell and module navigation. Each module lives under
`src/modules/`. Cartographer and its styled interface primitives live in
`src/modules/cartographer/`, including `ui-toolkit/`. The map search combobox and exits
checkbox wrap Ark UI; compose these local controls to keep the cartographer's visual and
accessibility contracts consistent.

The root `pnpm run e2e` command generates both catalogs before running the
browser test.
