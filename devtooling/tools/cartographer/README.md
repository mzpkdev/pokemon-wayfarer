# Pokémon Wayfarer Cartographer

`wcartographer` renders static terrain PNGs from the Pokémon Wayfarer source tree. It does not build or run the ROM.

Run these commands from `devtooling/`:

```sh
pnpm run cartographer Route101
pnpm run cartographer --all-exteriors --output ../build/cartographer/all-exterior-maps
pnpm run cartographer:catalog # recreates the shared map and metatile catalogs
```

Map names match directories under `data/maps/`. Use `--repo` to render another compatible source tree and `--output` to choose an output directory.

`--all-exteriors` renders towns, cities, routes, ocean routes, and underwater maps. The
`cartographer:catalog` command recreates the UI's terrain images and `catalog.json` under
`build/cartographer/map-catalog/`. The `cartographer` wrapper writes map renders under
the repository-level `build/cartographer/` directory, never under `game/build/`. Renders
show terrain only.
