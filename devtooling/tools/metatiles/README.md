# Pokémon Wayfarer Metatiles

`wmetatiles` builds source-driven metatile catalogs and palette-correct atlases. It does not build or run the ROM.

Run this from `devtooling/`:

```sh
pnpm run metatiles:catalog # recreates the shared map and metatile catalogs
```

The refresh writes the catalog under `build/cartographer/map-catalog/metatiles/`. Each
context combines the source tree's real primary and secondary tilesets, so palette slots
are resolved for the pairing that maps use.

Use `--repo` to read another compatible source tree and `--output` to choose an output directory.
