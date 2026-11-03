// Builds the world map data for the pan/zoom explorer: every region's outdoor
// maps in their real metatiles, laid out by their connections, loose pieces
// placed by their Town Map cells, and the regions arranged on one sea. Writes
// the metatile atlas, the grid of atlas indices, the far-out overview and
// meta.json.
// usage: pnpm world-map [outDir]
import { spawnSync } from "node:child_process"
import fs from "node:fs"
import path from "node:path"
import { fileURLToPath } from "node:url"
import { gzipSync } from "node:zlib"
import { writePng } from "../../../scripts/lib/png.mjs"
import { writeEncounters } from "./encounters.mjs"
import { createAtlas, openGame } from "./game.mjs"
import { buildLand } from "./land.mjs"
import { buildLandmarks } from "./landmarks.mjs"
import { placeWorld } from "./layout.mjs"
import { writeLiving } from "./living.mjs"
import { paintOverview } from "./overview.mjs"

const here = path.dirname(fileURLToPath(import.meta.url))
const [outDir = path.resolve(here, "../../../public/world-map")] = process.argv.slice(2)

const game = openGame()
const metatiles = createAtlas(game)
const world = placeWorld(game)
const { all, minX, minY, W, H } = world
const land = buildLand(game, world, metatiles)
const overview = paintOverview(world, land)
const { grid } = land
const atlasTiles = metatiles.tiles

// The atlas: 64 metatiles a row.
const COLUMNS = 64
const rows = Math.ceil(atlasTiles.length / COLUMNS)
const atlas = new Uint8Array(COLUMNS * 16 * rows * 16 * 4)
atlasTiles.forEach((tile, k) => {
  const ax = (k % COLUMNS) * 16,
    ay = Math.floor(k / COLUMNS) * 16
  for (let y = 0; y < 16; y++)
    atlas.set(tile.subarray(y * 64, y * 64 + 64), ((ay + y) * COLUMNS * 16 + ax) * 4)
})
fs.mkdirSync(outDir, { recursive: true })
writePng(`${outDir}/atlas.png`, COLUMNS * 16, rows * 16, atlas)
fs.writeFileSync(`${outDir}/grid.bin.gz`, gzipSync(Buffer.from(grid.buffer), { level: 9 }))
fs.rmSync(`${outDir}/grid.bin`, { force: true })
// The far-out view is the first thing to load; a JPEG of it is a third the size
// and looks the same. Made when ffmpeg is around; without it the PNG stays.
{
  const png = `${outDir}/overview.png`,
    jpg = `${outDir}/overview.jpg`
  fs.rmSync(jpg, { force: true })
  writePng(png, W, H, overview)
  const available = spawnSync("ffmpeg", ["-version"], { stdio: "ignore" }).status === 0
  const made =
    available &&
    spawnSync("ffmpeg", ["-loglevel", "error", "-y", "-i", png, "-q:v", "2", jpg], {
      stdio: "inherit",
    }).status === 0
  if (made) {
    fs.rmSync(png)
    console.log("overview.jpg (ffmpeg)")
  } else {
    fs.rmSync(jpg, { force: true })
    console.log("overview.png (no ffmpeg, kept the PNG)")
  }
}
const towns = all
  .filter((p) => ["MAP_TYPE_TOWN", "MAP_TYPE_CITY"].includes(p.m.map_type))
  .map((p) => ({ name: p.m.name, x: p.x - minX + p.l.width / 2, y: p.y - minY + p.l.height / 2 }))
// Region names, centered on their towns. Johto and Kanto share one piece, so
// they are told apart by their towns.
const JOHTO =
  /^(NewBark|Cherrygrove|Violet|Azalea|Goldenrod|Ecruteak|Olivine|Cianwood|Mahogany|Blackthorn)/
const KANTO =
  /^(Pallet|Viridian|Pewter|Cerulean|Vermilion|Lavender|Celadon|Saffron|Fuchsia|Cinnabar)/
const labelFor = (p) => {
  if (p.region !== "Johto and Kanto")
    return p.region === "Sevii Islands" ? "SEVII ISLANDS" : p.region.toUpperCase()
  if (JOHTO.test(p.m.name)) return "JOHTO"
  if (KANTO.test(p.m.name)) return "KANTO"
  return null
}
const regionLabels = new Map()
for (const p of all) {
  const label = labelFor(p)
  if (!label) continue
  const entry = regionLabels.get(label) ?? {
    name: label,
    x: 0,
    y: 0,
    n: 0,
    bounds: { left: Infinity, top: Infinity, right: -Infinity, bottom: -Infinity },
  }
  // The label sits on the region's towns; its bounds hold all its maps.
  const { bounds } = entry
  bounds.left = Math.min(bounds.left, p.x - minX)
  bounds.top = Math.min(bounds.top, p.y - minY)
  bounds.right = Math.max(bounds.right, p.x - minX + p.l.width)
  bounds.bottom = Math.max(bounds.bottom, p.y - minY + p.l.height)
  regionLabels.set(label, entry)
  if (p.region === "Johto and Kanto" && !["MAP_TYPE_TOWN", "MAP_TYPE_CITY"].includes(p.m.map_type))
    continue
  entry.x += p.x - minX + p.l.width / 2
  entry.y += p.y - minY + p.l.height / 2
  entry.n++
}
const regions = [...regionLabels.values()].map(({ name, x, y, n, bounds }) => ({
  name,
  x: x / n,
  y: y / n,
  bounds,
}))
fs.writeFileSync(
  `${outDir}/meta.json`,
  JSON.stringify({
    width: W,
    height: H,
    columns: COLUMNS,
    count: atlasTiles.length,
    towns,
    regions,
    maps: all.map((p) => ({
      name: p.m.name,
      x: p.x - minX,
      y: p.y - minY,
      width: p.l.width,
      height: p.l.height,
      region: p.region,
    })),
  }),
)
// The tour's gym pins and league circuit.
fs.writeFileSync(`${outDir}/landmarks.json`, JSON.stringify(buildLandmarks(game, world, labelFor)))
// The ferries, their sea lanes and the times of day.
writeLiving(
  {
    grid,
    isWater: land.isWater,
    width: W,
    height: H,
    maps: all.map((p) => ({
      name: p.m.name,
      x: p.x - minX,
      y: p.y - minY,
      width: p.l.width,
      height: p.l.height,
    })),
  },
  outDir,
)
console.log(
  "atlas",
  atlasTiles.length,
  "metatiles,",
  COLUMNS * 16,
  "x",
  rows * 16,
  "; grid",
  W,
  "x",
  H,
)
const places = writeEncounters(
  game,
  all.map((p) => p.m.name),
  outDir,
)
console.log(
  "encounters",
  places.withEncounters,
  "of",
  places.maps,
  "maps,",
  places.species,
  "species",
)
