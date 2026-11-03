// Builds the world map data for the pan/zoom explorer: every region's outdoor
// maps in their real metatiles, laid out by their connections, loose pieces
// placed by their Town Map cells, and the regions arranged on one sea. Writes
// the metatile atlas, the grid of atlas indices, the far-out overview and
// meta.json.
// usage: pnpm world-map [outDir]
import fs from "node:fs"
import path from "node:path"
import { fileURLToPath } from "node:url"
import { gzipSync } from "node:zlib"
import { renderIndexed } from "../../../scripts/lib/gba.mjs"
import { writePng } from "../../../scripts/lib/png.mjs"
import { writeEncounters } from "./encounters.mjs"
import { createAtlas, openGame } from "./game.mjs"
import { buildLand } from "./land.mjs"
import { buildLandmarks } from "./landmarks.mjs"
import { placeWorld } from "./layout.mjs"
import { writeLiving } from "./living.mjs"
import { writeNotables } from "./notables.mjs"
import { overviewLevels, packLevels } from "./overview.mjs"

const here = path.dirname(fileURLToPath(import.meta.url))
const [outDir = path.resolve(here, "../../../public/world-map")] = process.argv.slice(2)

const game = openGame()
const metatiles = createAtlas(game)
const world = placeWorld(game)
const { all, minX, minY, W, H } = world
const land = buildLand(game, world, metatiles)
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
// The far-out view, a pixel a metatile and coarser, packed in one PNG. The
// open sea past the grid is the darkest ocean, in metatiles and in color.
const overview = packLevels(
  overviewLevels({ grid, W, H, tiles: atlasTiles, isWater: land.isWater }),
)
writePng(`${outDir}/overview.png`, overview.width, overview.height, overview.rgba)
fs.rmSync(`${outDir}/overview.jpg`, { force: true })
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
    overview: overview.places,
    ocean: land.ocean,
    sea: [...overview.rgba.subarray(0, 3)],
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
const landmarks = buildLandmarks(game, world, labelFor)
fs.writeFileSync(`${outDir}/landmarks.json`, JSON.stringify(landmarks))
// Each Gym Leader's and each league trainer's battle portrait, 64x64 in the
// palette its PNG carries, as the battle shows it.
// Stale portraits go file by file, keeping the folder, so a running dev
// server's watch on it holds.
const pics = new Set([
  ...landmarks.badges.map((gym) => gym.leader?.pic).filter(Boolean),
  ...landmarks.roster.trainers.map((trainer) => trainer.pic),
])
fs.mkdirSync(`${outDir}/leaders`, { recursive: true })
for (const file of fs.readdirSync(`${outDir}/leaders`))
  if (!pics.has(file.replace(/\.png$/, ""))) fs.rmSync(`${outDir}/leaders/${file}`)
for (const pic of pics) {
  const image = renderIndexed(path.join(game.root, `graphics/trainers/front_pics/${pic}.png`))
  writePng(`${outDir}/leaders/${pic}.png`, image.width, image.height, image.rgba)
}
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
// The Gym Leaders and Elite Four, their places and the routes between them.
console.log("notables", writeNotables(game, world, land, landmarks, outDir))
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
  "species; by generation I-VIII, forms:",
  places.counts.join(" "),
)
