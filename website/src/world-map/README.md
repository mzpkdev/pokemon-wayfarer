# World map

The whole overworld in the game's own metatiles: Johto, Kanto, Hoenn, Sinnoh,
Sinjoh, the Sevii Islands and Alola on one sea. It has two parts, a builder
that turns the game's maps into data and a client that draws that data.

The rest of the site uses it only through `index.ts`:

- `WorldTour` is the landing page's world section. As its section scrolls by,
  the camera pulls back from LITTLEROOT TOWN to all of Hoenn and then the whole
  world, with captions beside the map. Its EXPLORE THE MAP button opens the
  explorer full screen.
- `WorldMap` is the explorer. Drag, pinch, wheel, double-click, the arrow keys
  and +/- all pan and zoom it.

Outside this folder, the module uses the site's pixel CSS utilities
(`pixel-panel`, `pixel-label`, …), `useReducedMotion` from `~/lib/hooks`, and the
PNG/palette helpers in `scripts/lib/`.

## Data

`pnpm world-map` (run from `website/`) builds the data into
`public/world-map/`. It takes about 5 seconds and needs the game tree at
`../game`. Its output is committed, like the rest of the game art, so building
the site doesn't need the game tree. Rebuild it after maps, tilesets or the
builder change.

| File           | What it holds                                                                          |
| -------------- | -------------------------------------------------------------------------------------- |
| `atlas.png`    | Every metatile the world uses, 16×16 each, 64 to a row                                 |
| `grid.bin.gz`  | The world as gzipped little-endian u16 atlas indices, row by row; `0xffff` is open sea |
| `overview.jpg` | The far-out view, one pixel per metatile, painted like a map                           |
| `meta.json`    | Grid size, atlas columns, towns, region labels with bounds, and placed maps            |

The builder writes `overview.jpg` with ffmpeg. Without ffmpeg it writes
`overview.png` instead, and the client falls back to that.

## Builder (`build/`)

Plain Node scripts. Each stage gets a module:

- `index.mjs`: the entry point, which writes the outputs.
- `game.mjs`: reads the game's maps, layouts, tilesets and Town Map cells, and
  draws metatiles.
- `layout.mjs`: places the maps. Maps are joined along their connections. Where
  a loop of connections doesn't add up, the error is spread around the loop.
  Pieces no connection reaches are placed by their Town Map cells, overlaps are
  pushed apart, and the regions are arranged on the sea.
- `land.mjs`: fills in land between the maps. Coasts are grown with noise, the
  insides of loops are filled, straits keep regions apart, and each filled cell
  takes the border blocks of the nearest map.
- `overview.mjs`: paints the far-out view in one style. Sea depth, a smoothed
  coast, and one palette per kind of ground make every game's tiles match.
- `source/`: Sinnoh's Town Map cells, borrowed from the project its maps came
  from. Its README gives the credit.

The builder only reads the maps the wayfarer build compiles, and connects them
the way `tools/mapjson` does.

## Client

- `data.ts`: loads and caches the data, shared by the tour and the explorer.
- `camera.ts`: the camera (a world point and CSS pixels per metatile), fitting
  bounds into a view, and the eased flight between two cameras.
- `render.ts`: draws a view. Far out it draws the overview. Close in it draws
  32×32-metatile chunks, cross-fading between the two. Chunks are drawn a few
  per frame, and the overview shows through until they're done. It also draws
  the region and town names.
- `WorldTour.tsx` and `WorldMap.tsx`: the two components.
