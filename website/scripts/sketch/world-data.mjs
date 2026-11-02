// Sketch: render a region's outdoor maps with their real metatiles, laid out by connections.
// Sketch: the overworld as a metatile atlas and index grid for the world explorer.
// usage: node scripts/sketch/world-data.mjs public/sketch/world MAP_NEW_BARK_TOWN_HNS
import fs from "node:fs"
const game = "/home/mzpk/Workspace/pokemon-wayfarer-v2/tasks/website-landing/pokemon-wayfarer/game"
const lib =
  "/home/mzpk/Workspace/pokemon-wayfarer-v2/tasks/website-landing/pokemon-wayfarer/website/scripts/lib/"
const { readPng, writePng } = await import(lib + "png.mjs")
const { readJascPalette } = await import(lib + "gba.mjs")
const [outDir, root] = process.argv.slice(2)

// Tileset symbol -> files, read from the C sources that INCBIN them.
const src = [
  "headers.h",
  "sinnoh_headers.h",
  "graphics.h",
  "sinnoh_graphics.h",
  "metatiles.h",
  "sinnoh_metatiles.h",
]
  .map((f) => fs.readFileSync(`${game}/src/data/tilesets/${f}`, "utf8"))
  .join("\n")
const field = (name, key) =>
  src.match(
    new RegExp(`const struct Tileset ${name}\\s*=\\s*\\{[^}]*?\\.${key}\\s*=\\s*(\\w+)`),
  )?.[1]
const incbin = (symbol) =>
  src.match(new RegExp(`${symbol}\\[\\]\\s*=\\s*INCBIN_U(?:8|16|32)\\("([^"]+)"`))?.[1]
const paletteFiles = (symbol) => {
  const block =
    src.match(new RegExp(`${symbol}\\[\\]\\[16\\]\\s*=\\s*\\{([\\s\\S]*?)\\};`))?.[1] ?? ""
  return [...block.matchAll(/"([^"]+)"/g)].map((m) => m[1])
}
const tilesetCache = new Map()
const tileset = (name) => {
  if (tilesetCache.has(name)) return tilesetCache.get(name)
  const tilesPath = incbin(field(name, "tiles")).replace(/\.4bpp.*$/, ".png")
  const png = readPng(`${game}/${tilesPath}`)
  const palettes = paletteFiles(field(name, "palettes")).map((f) => {
    const jasc = `${game}/${f.replace(/\.gbapal$/, ".pal")}`
    return fs.existsSync(jasc) ? readJascPalette(jasc) : null
  })
  const metatiles = fs.readFileSync(`${game}/${incbin(field(name, "metatiles"))}`)
  const t = { png, palettes, metatiles, count: metatiles.length / 16, tilesPerRow: png.width / 8 }
  tilesetCache.set(name, t)
  return t
}

// Maps and layouts, placed by their connections from the root.
const layouts = Object.fromEntries(
  JSON.parse(fs.readFileSync(`${game}/data/layouts/layouts.json`))
    .layouts.filter((l) => l.id)
    .map((l) => [l.id, l]),
)
// Only the maps the wayfarer build compiles, connected the way it connects
// them (tools/mapjson/mapjson.cpp): the FRLG coast replaces the HNS Routes
// 19-21, Cinnabar and Seafoam, and the proof-of-concept coast maps, and
// Fuchsia and Pallet are rewired to it.
const REPLACED = new Set([
  "MAP_CINNABAR_ISLAND_HNS",
  "MAP_ROUTE19_HNS",
  "MAP_ROUTE20_HNS",
  "MAP_ROUTE21_HNS",
  "MAP_ROUTE19_CAVE_HNS",
  "MAP_FUCHSIA_ROUTE19GATE_HNS",
  "MAP_SEAFOAM_ISLANDS_1F_HNS",
])
const COAST = new Set([
  "Route19_Frlg",
  "Route20_Frlg",
  "Route21_North_Frlg",
  "Route21_South_Frlg",
  "CinnabarIsland_Frlg",
])
const REWIRED = {
  "FuchsiaCity_hns|MAP_ROUTE19_COAST_POC": "MAP_ROUTE19",
  "PalletTown_hns|MAP_ROUTE21_NORTH_COAST_POC": "MAP_ROUTE21_NORTH",
  "Route19_Frlg|MAP_FUCHSIA_CITY": "MAP_FUCHSIA_CITY_HNS",
  "Route21_North_Frlg|MAP_PALLET_TOWN": "MAP_PALLET_TOWN_HNS",
}
const compiled = (m) => {
  if (REPLACED.has(m.id) || m.id.endsWith("_POC") || m.name.endsWith("_Port")) return false
  const layout = layouts[m.layout]
  if (layout?.game_version === "frlg") return COAST.has(m.name) || Boolean(m.wayfarer_include)
  return true
}
const maps = {}
for (const dir of fs.readdirSync(`${game}/data/maps`)) {
  const f = `${game}/data/maps/${dir}/map.json`
  if (!fs.existsSync(f)) continue
  const m = JSON.parse(fs.readFileSync(f))
  if (compiled(m)) maps[m.id] = m
}
const placed = new Map()
const seams = []
const queue = [[root, 0, 0]]
while (queue.length) {
  const [id, x, y] = queue.shift()
  if (placed.has(id)) continue
  const m = maps[id],
    l = layouts[m.layout]
  placed.set(id, { x, y, m, l })
  for (const c of m.connections ?? []) {
    const target = REWIRED[`${m.name}|${c.map}`] ?? c.map
    const t = maps[target]
    if (!t) continue
    const tl = layouts[t.layout],
      o = c.offset
    const at = {
      up: [x + o, y - tl.height],
      down: [x + o, y + l.height],
      left: [x - tl.width, y + o],
      right: [x + l.width, y + o],
    }[c.direction]
    if (!at) continue
    if (placed.has(target)) {
      // Already placed from elsewhere: note where this connection disagrees.
      const other = placed.get(target)
      if (other.x !== at[0] || other.y !== at[1])
        seams.push(
          `${m.name} -> ${t.name}: placed at ${other.x},${other.y}, this connection says ${at[0]},${at[1]}`,
        )
      continue
    }
    queue.push([target, ...at])
  }
}
for (const seam of seams) console.log("disagrees:", seam)
const all = [...placed.values()]
const minX = Math.min(...all.map((p) => p.x)),
  minY = Math.min(...all.map((p) => p.y))
const W = Math.max(...all.map((p) => p.x + p.l.width)) - minX,
  H = Math.max(...all.map((p) => p.y + p.l.height)) - minY
console.log(root, all.length, "maps,", W, "x", H, "metatiles")

// One metatile's 16x16 RGBA.
const metatileCache = new Map()
const drawMetatile = (primaryName, secondaryName, id) => {
  const key = `${primaryName}|${secondaryName}|${id}`
  if (metatileCache.has(key)) return metatileCache.get(key)
  const primary = tileset(primaryName),
    secondary = tileset(secondaryName)
  const emerald = primary.count === 512
  const primaryTiles = emerald ? 512 : 640,
    primaryPals = emerald ? 6 : 7
  const source = id < primary.count ? primary : secondary
  const local = id < primary.count ? id : id - primary.count
  const out = new Uint8Array(16 * 16 * 4)
  const backdrop = primary.palettes[0]?.[0] ?? [0, 0, 0]
  for (let i = 0; i < 256; i++) out.set([...backdrop.slice(0, 3), 255], i * 4)
  for (let e = 0; e < 8; e++) {
    if ((local * 8 + e) * 2 + 2 > source.metatiles.length) break
    const entry = source.metatiles.readUInt16LE((local * 8 + e) * 2)
    const tileIndex = entry & 0x3ff,
      hflip = entry & 0x400,
      vflip = entry & 0x800,
      pal = entry >> 12
    const tiles = tileIndex < primaryTiles ? primary : secondary
    const t = tileIndex < primaryTiles ? tileIndex : tileIndex - primaryTiles
    const palette = (pal < primaryPals ? primary : secondary).palettes[pal]
    if (!palette) continue
    const tx = (t % tiles.tilesPerRow) * 8,
      ty = Math.floor(t / tiles.tilesPerRow) * 8
    if (ty >= tiles.png.height) continue
    const ox = (e % 2) * 8,
      oy = (Math.floor(e / 2) % 2) * 8
    for (let y = 0; y < 8; y++)
      for (let x = 0; x < 8; x++) {
        const sx = hflip ? 7 - x : x,
          sy = vflip ? 7 - y : y
        const index = tiles.png.samples[(ty + sy) * tiles.png.width + tx + sx] & 0xf
        if (index === 0) continue
        const c = palette[index]
        out.set([c[0] & 0xf8, c[1] & 0xf8, c[2] & 0xf8, 255], ((oy + y) * 16 + ox + x) * 4)
      }
  }
  metatileCache.set(key, out)
  return out
}

// The grid of atlas indices, every placed map's metatiles first.
const atlasKeys = new Map()
const atlasTiles = []
const atlasIndex = (primary, secondary, id) => {
  const key = `${primary}|${secondary}|${id}`
  if (!atlasKeys.has(key)) {
    atlasKeys.set(key, atlasTiles.length)
    atlasTiles.push(drawMetatile(primary, secondary, id))
  }
  return atlasKeys.get(key)
}
const EMPTY = 0xffff
const grid = new Uint16Array(W * H).fill(EMPTY)
const owner = new Int32Array(W * H).fill(-1)
all.forEach((p, mapIndex) => {
  const blocks = fs.readFileSync(`${game}/${p.l.blockdata_filepath}`)
  for (let y = 0; y < p.l.height; y++)
    for (let x = 0; x < p.l.width; x++) {
      const i = (p.y - minY + y) * W + (p.x - minX + x)
      grid[i] = atlasIndex(
        p.l.primary_tileset,
        p.l.secondary_tileset,
        blocks.readUInt16LE((y * p.l.width + x) * 2) & 0x3ff,
      )
      owner[i] = mapIndex
    }
})
// Fill the gaps the way the game fills past a map's edge: with the nearest
// map's border blocks, tiled on the world grid.
const borders = all.map((p) => {
  const bin = fs.readFileSync(`${game}/${p.l.border_filepath}`)
  const bw = p.l.border_width ?? 2,
    bh = p.l.border_height ?? 2
  return {
    bw,
    bh,
    ids: Array.from({ length: bw * bh }, (_, k) =>
      atlasIndex(p.l.primary_tileset, p.l.secondary_tileset, bin.readUInt16LE(k * 2) & 0x3ff),
    ),
  }
})
let frontier = []
for (let i = 0; i < W * H; i++) if (owner[i] >= 0) frontier.push(i)
while (frontier.length) {
  const next = []
  for (const i of frontier) {
    const x = i % W,
      y = (i - x) / W
    for (const [dx, dy] of [
      [1, 0],
      [-1, 0],
      [0, 1],
      [0, -1],
    ]) {
      const nx = x + dx,
        ny = y + dy
      if (nx < 0 || ny < 0 || nx >= W || ny >= H) continue
      const j = ny * W + nx
      if (owner[j] >= 0) continue
      owner[j] = owner[i]
      const b = borders[owner[i]]
      grid[j] = b.ids[(ny % b.bh) * b.bw + (nx % b.bw)]
      next.push(j)
    }
  }
  frontier = next
}
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
fs.writeFileSync(`${outDir}/grid.bin`, Buffer.from(grid.buffer))
const towns = all
  .filter((p) => ["MAP_TYPE_TOWN", "MAP_TYPE_CITY"].includes(p.m.map_type))
  .map((p) => ({ name: p.m.name, x: p.x - minX + p.l.width / 2, y: p.y - minY + p.l.height / 2 }))
fs.writeFileSync(
  `${outDir}/meta.json`,
  JSON.stringify({ width: W, height: H, columns: COLUMNS, count: atlasTiles.length, towns }),
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
