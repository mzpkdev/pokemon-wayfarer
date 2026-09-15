# Wayfarer title-background pipeline

`game/tools/convert_wayfarer_title.py` converts a reviewed Pixel Art Fixer image
into the 8bpp text-background assets used by the Wayfarer title screen. It keeps
the full-colour input as `source.png`; generated files live beside it in
`game/graphics/title_screen/wayfarer/background/`.

## Dependencies

The converter requires Pillow 12.3.0, pinned in
`game/tools/requirements-wayfarer-title.txt`. Create an isolated environment:

```bash
python3 -m venv /tmp/wayfarer-title-venv
/tmp/wayfarer-title-venv/bin/pip install -r game/tools/requirements-wayfarer-title.txt
```

An existing Pixel Art Fixer environment with the same Pillow version can also
run the converter. Commands below run from the repository root.

## Regenerate

Put the opaque, Pixel Art Fixer-reviewed source image at:

```text
game/graphics/title_screen/wayfarer/background/source.png
```

Then generate all title-background sources and inspect `preview.png` at its
native 240x160 size:

```bash
make -C game TITLE_ART_PYTHON=/tmp/wayfarer-title-venv/bin/python wayfarer-title-assets
```

The direct form is useful for a temporary source or output directory:

```bash
/tmp/wayfarer-title-venv/bin/python game/tools/convert_wayfarer_title.py \
  --input /absolute/path/to/source.png \
  --output /absolute/path/to/background \
  --logo-source game/graphics/title_screen/hns/pokemon_logo.png
```

The checked-in generated inputs are `scene.png`, `scene.bin`, `logo.png`, and
`shared.pal`. Normal Make rules create the ignored `.8bpp` and `.gbapal` files
from them. `preview.png` is reconstructed from exported scene tiles, map, and
the shared palette; it is the review image for the game background.

## Asset contract

- The source must be a fully opaque image. It is stretched with nearest-neighbour
  sampling to 240x160, restoring the title screen's 3:2 composition without a
  crop or dithering.
- Palette index 0 is transparent. Used nonzero HNS Pokemon-logo colours are
  first compacted and deduplicated after RGB555 conversion; an opaque black logo
  colour remains nonzero. The scene uses only following palette entries.
- `shared.pal` contains all 256 palette entries and `shared.gbapal` is its exact
  512-byte RGB555 form. `logo.png` preserves the 256x64 logo atlas layout.
- The scene is packed as exact 8x8 8bpp tiles. Tile 0 is blank, repeated tiles
  share an ID, and `scene.bin` is a 32x32 little-endian text map with no palette
  bank or flip bits. The 240x160 composition occupies the upper-left 30x20
  tiles; unused map cells point at tile 0.
- Scene tile data may use at most `0xB000` bytes for charbase 1 before the logo
  map at `0xF000`; the logo asset may use at most `0x4000` bytes. The manifest
  records the actual counts, source and artifact checksums, dimensions, and
  converter version.

Run the converter tests after changing the pipeline:

```bash
/tmp/wayfarer-title-venv/bin/python game/tools/test_convert_wayfarer_title.py
```

## Version banner

The `WAYFARER` banner uses white pixel lettering with a heavy black outline,
modeled on the original GBA version banners. Its glyphs live in
`game/tools/generate_wayfarer_banner.py`; regenerate it separately from the scene:

```bash
make -C game TITLE_ART_PYTHON=/tmp/wayfarer-title-venv/bin/python wayfarer-title-banner
```

The generator writes `wayfarer_version.png` and `wayfarer_version.pal` in
`game/graphics/title_screen/wayfarer/`. It packs the left and right 64x32 sprite
halves into a 64x64 sheet, reserves index 0 for transparency, and uses only the
first 16 sprite-palette entries. Regeneration does not change the background.

## Build and inspect the real screen

Build the ROM after regeneration; this also compresses the exported assets:

```bash
make -C game -j8 BUILD=wayfarer release
```

With the repository's dependencies installed, capture the release ROM through
the bundled SkyEmu HTTP server (requires Node, Xvfb, and `arm-none-eabi-nm`):

```bash
node game/tools/capture_wayfarer_title.mjs \
  --emulator node_modules/.pnpm/skyemu-static@0.0.2/node_modules/skyemu-static/vendor/SkyEmu \
  --rom game/pokewayfarer-release.gba \
  --elf game/pokewayfarer-release.elf \
  --output /tmp/wayfarer-title-review
```

The capture uses an isolated ROM copy and records its SHA256. It waits for the
actual title tasks, captures the natural intro and settled title across the
prompt blink, checks that Start leaves the title, and checks skipping the intro.
Review `title.png`, `title-blink.png`, `title-skipped.png`, and `after-start.png`.
`capture.json` records the ROM identity and display registers. Compare the
uncovered background with `preview.png`; only hardware RGB555 display expansion
should affect its colors. The logo and prompts use their existing overlays.
