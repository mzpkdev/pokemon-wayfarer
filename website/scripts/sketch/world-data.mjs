// Sketch: the whole overworld as a metatile atlas and index grid for the
// world explorer: every region's outdoor maps in their real metatiles, laid
// out by their connections, loose pieces placed by their Town Map cells, and
// the regions arranged on one sea.
// usage: pnpm sketch:world
import fs from "node:fs"
const game = "/home/mzpk/Workspace/pokemon-wayfarer-v2/tasks/website-landing/pokemon-wayfarer/game"
const lib =
  "/home/mzpk/Workspace/pokemon-wayfarer-v2/tasks/website-landing/pokemon-wayfarer/website/scripts/lib/"
const { readPng, writePng } = await import(lib + "png.mjs")
const { readJascPalette } = await import(lib + "gba.mjs")
const [outDir] = process.argv.slice(2)

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
  .concat(fs.readFileSync(`${game}/src/graphics.c`, "utf8"))
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
  const tilesSymbol = field(name, "tiles")
  if (!incbin(tilesSymbol)) throw new Error(`no tiles for ${name}: ${tilesSymbol}`)
  const tilesPath = incbin(tilesSymbol).replace(/\.4bpp.*$/, ".png")
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
const sevii = new Set(
  JSON.parse(fs.readFileSync(`${game}/src/data/wayfarer_sevii_maps.json`, "utf8")).maps.map(
    (entry) => entry.source_map,
  ),
)
const OUTDOOR = new Set([
  "MAP_TYPE_TOWN",
  "MAP_TYPE_CITY",
  "MAP_TYPE_ROUTE",
  "MAP_TYPE_OCEAN_ROUTE",
])
const compiled = (m) => {
  if (!OUTDOOR.has(m.map_type)) return false
  if (REPLACED.has(m.id) || m.id.endsWith("_POC") || m.name.endsWith("_Port")) return false
  // Another state of a map already drawn (the Lake of Rage at low tide), and
  // Trainer Hill, a special area hung off Olivine, are not part of the land.
  if (m.name.includes("LowTide") || m.name.startsWith("TrainerHill_")) return false
  const layout = layouts[m.layout]
  if (layout?.game_version === "frlg")
    return COAST.has(m.name) || sevii.has(m.name) || Boolean(m.wayfarer_include)
  return true
}
const maps = {}
for (const dir of fs.readdirSync(`${game}/data/maps`)) {
  const f = `${game}/data/maps/${dir}/map.json`
  if (!fs.existsSync(f)) continue
  const m = JSON.parse(fs.readFileSync(f))
  if (compiled(m)) maps[m.id] = m
}
const byName = Object.fromEntries(Object.values(maps).map((m) => [m.name, m]))
const connections = (m) => (Array.isArray(m.connections) ? m.connections : [])
const targetOf = (m, c) => REWIRED[`${m.name}|${c.map}`] ?? c.map

// Connections only promise that each pair of neighbors lines up; around a
// loop of maps the offsets need not add up, so one link per loop must give.
// Place a piece's maps along a spanning tree of links, and choose which links
// to leave out so the fewest tiles end up drawn over each other: a leftover
// mismatch then shows as a gap, filled like any other, instead of one map cut
// through another.
const edgeKey = (a, b) => [a, b].toSorted().join(" ~ ")
const walk = (root, forbidden) => {
  const placed = new Map()
  const queue = [[root, 0, 0]]
  while (queue.length > 0) {
    const [id, x, y] = queue.shift()
    if (placed.has(id)) continue
    const m = maps[id],
      l = layouts[m.layout]
    placed.set(id, { x, y, m, l })
    for (const c of connections(m)) {
      const target = targetOf(m, c)
      const t = maps[target]
      if (!t || placed.has(target) || forbidden.has(edgeKey(id, target))) continue
      const tl = layouts[t.layout],
        o = c.offset
      const at = {
        up: [x + o, y - tl.height],
        down: [x + o, y + l.height],
        left: [x - tl.width, y + o],
        right: [x + l.width, y + o],
      }[c.direction]
      if (at) queue.push([target, ...at])
    }
  }
  return placed
}
const overlap = (list) => {
  let area = 0
  for (let i = 0; i < list.length; i++)
    for (let j = i + 1; j < list.length; j++) {
      const a = list[i],
        b = list[j]
      const w = Math.min(a.x + a.l.width, b.x + b.l.width) - Math.max(a.x, b.x)
      const h = Math.min(a.y + a.l.height, b.y + b.l.height) - Math.max(a.y, b.y)
      if (w > 0 && h > 0) area += w * h
    }
  return area
}
const layOut = (root) => {
  const forbidden = new Set()
  let placed = walk(root, forbidden)
  const total = placed.size
  const links = new Set()
  for (const id of placed.keys())
    for (const c of connections(maps[id]))
      if (placed.has(targetOf(maps[id], c))) links.add(edgeKey(id, targetOf(maps[id], c)))
  let best = overlap([...placed.values()])
  for (let round = 0; round < 12 && best > 0; round++) {
    let pick = null
    for (const link of links) {
      if (forbidden.has(link)) continue
      forbidden.add(link)
      const trial = walk(root, forbidden)
      forbidden.delete(link)
      if (trial.size < total) continue
      const area = overlap([...trial.values()])
      if (area < best) {
        best = area
        pick = link
      }
    }
    if (!pick) break
    forbidden.add(pick)
  }
  placed = walk(root, forbidden)
  if (forbidden.size > 0) console.log(`  ${maps[root].name}: left out ${[...forbidden].join(", ")}`)
  return [...placed.values()]
}

// Town Map cells, to place the pieces no connection reaches: the game's own
// sections, and Sinnoh's from the project its maps came from.
const cells = new Map(
  JSON.parse(fs.readFileSync(`${game}/src/data/region_map/region_map_sections.json`, "utf8"))
    .map_sections.filter((section) => section.x !== undefined)
    .map((section) => [section.id, section]),
)
for (const section of JSON.parse(
  fs.readFileSync(new URL("../source/sinnoh/sections.json", import.meta.url), "utf8"),
).map_sections)
  cells.set(section.map_section.replace("MAPSEC_", "MAPSEC_SINNOH_"), section)
const cellOf = (m) => {
  const section = cells.get(m.region_map_section)
  return section && { x: section.x + section.width / 2, y: section.y + section.height / 2 }
}

/**
 * The regions, each a set of pieces in one Town Map's cell space. The first
 * root's piece is laid out by its connections; the cell scale (metatiles per
 * cell) and the region's origin then place every other piece by one of its
 * map's cells. `at` puts the region in the world, in metatiles.
 */
const REGIONS = [
  { name: "Johto and Kanto", roots: ["MAP_NEW_BARK_TOWN_HNS"], at: [0, 0] },
  {
    name: "Hoenn",
    roots: ["MAP_LITTLEROOT_TOWN", "MAP_SOOTOPOLIS_CITY"],
    at: [-260, 560],
  },
  { name: "Sinjoh", roots: ["MAP_ROUTE49_HNS"], at: [0, 0], beside: "Sinjoh" },
  {
    name: "Sinnoh",
    roots: [
      "MAP_TWINLEAF_TOWN",
      "MAP_HEARTHOME_CITY",
      "MAP_FIGHT_AREA",
      "MAP_SNOWPOINT_CITY",
      "MAP_ROUTE224",
    ],
    at: [0, 0],
    beside: "Sinnoh",
  },
  {
    name: "Sevii Islands",
    roots: [
      "MAP_ONE_ISLAND",
      "MAP_TWO_ISLAND",
      "MAP_THREE_ISLAND",
      "MAP_THREE_ISLAND_BERRY_FOREST",
      "MAP_MT_EMBER_EXTERIOR",
      "MAP_FOUR_ISLAND",
      "MAP_FIVE_ISLAND",
      "MAP_SIX_ISLAND",
      "MAP_SIX_ISLAND_PATTERN_BUSH",
      "MAP_SEVEN_ISLAND",
    ],
    at: [760, 640],
  },
  { name: "Alola", roots: ["MAP_MELEMELE_ISLE_HNS"], at: [1500, 760] },
]
// Each Sevii Town Map is its own cell space; set them out like the islands lie.
const SEVII_MAPS = [
  {
    islands: [
      "ONE",
      "TWO",
      "THREE",
      "KINDLE",
      "TREASURE",
      "CAPE",
      "BOND",
      "BERRY",
      "MT_EMBER",
      "THREE_ISLE",
    ],
    at: [0, 0],
  },
  { islands: ["FOUR", "FIVE", "RESORT", "WATER_LABYRINTH", "FIVE_ISLE", "MEMORIAL"], at: [6, 14] },
  {
    islands: [
      "SIX",
      "SEVEN",
      "TRAINER_TOWER",
      "CANYON",
      "SEVAULT",
      "TANOBY",
      "WATER_PATH",
      "GREEN_PATH",
      "RUIN_VALLEY",
      "OUTCAST",
      "PATTERN",
    ],
    at: [-6, 30],
  },
]
const seviiCell = (m) => {
  const cell = cellOf(m)
  if (!cell) return null
  const name = m.region_map_section.replace("MAPSEC_", "")
  const group = SEVII_MAPS.find(({ islands }) => islands.some((island) => name.startsWith(island)))
  return group ? { x: cell.x + group.at[0], y: cell.y + group.at[1] } : cell
}

// Metatiles per Town Map cell, measured on Hoenn, whose maps all connect.
const hoennPiece = layOut("MAP_LITTLEROOT_TOWN")
const center = (p) => ({ x: p.x + p.l.width / 2, y: p.y + p.l.height / 2 })
const CELL = (() => {
  const towns = hoennPiece.filter((p) => cellOf(p.m))
  let num = 0,
    den = 0
  for (const a of towns)
    for (const b of towns) {
      const ca = cellOf(a.m),
        cb = cellOf(b.m),
        pa = center(a),
        pb = center(b)
      num += (pa.x - pb.x) * (ca.x - cb.x) + (pa.y - pb.y) * (ca.y - cb.y)
      den += (ca.x - cb.x) ** 2 + (ca.y - cb.y) ** 2
    }
  return num / den
})()
console.log("metatiles per Town Map cell:", CELL.toFixed(1))

const all = []
const regionBounds = {}
for (const region of REGIONS) {
  const cellFor = region.name === "Sevii Islands" ? seviiCell : cellOf
  // The Sevii Town Maps are drawn much closer in than Hoenn's.
  const scale = region.name === "Sevii Islands" ? CELL * 0.4 : CELL
  let origin = null
  const pieces = []
  for (const root of region.roots) {
    if (!maps[root]) {
      console.log(`  missing root ${root}`)
      continue
    }
    if (pieces.some((piece) => piece.some((p) => p.m.id === root))) continue
    const piece = root === "MAP_LITTLEROOT_TOWN" ? hoennPiece : layOut(root)
    // Line the piece up on the region's cells, by the average of its maps' cells.
    const anchored = piece.filter((p) => cellFor(p.m))
    if (origin === null) {
      origin = anchored.length
        ? {
            x:
              anchored.reduce((sum, p) => sum + (p.x + p.l.width / 2 - cellFor(p.m).x * scale), 0) /
              anchored.length,
            y:
              anchored.reduce(
                (sum, p) => sum + (p.y + p.l.height / 2 - cellFor(p.m).y * scale),
                0,
              ) / anchored.length,
          }
        : { x: 0, y: 0 }
    } else if (anchored.length) {
      const dx =
        anchored.reduce(
          (sum, p) => sum + (cellFor(p.m).x * scale + origin.x - (p.x + p.l.width / 2)),
          0,
        ) / anchored.length
      const dy =
        anchored.reduce(
          (sum, p) => sum + (cellFor(p.m).y * scale + origin.y - (p.y + p.l.height / 2)),
          0,
        ) / anchored.length
      for (const p of piece) {
        p.x += Math.round(dx)
        p.y += Math.round(dy)
      }
    }
    pieces.push(piece)
  }
  const placed = pieces.flat()
  const left = Math.min(...placed.map((p) => p.x)),
    top = Math.min(...placed.map((p) => p.y))
  for (const p of placed) {
    p.x += region.at[0] - left
    p.y += region.at[1] - top
    p.region = region.name
  }
  regionBounds[region.name] = {
    width: Math.max(...placed.map((p) => p.x + p.l.width)) - region.at[0],
    height: Math.max(...placed.map((p) => p.y + p.l.height)) - region.at[1],
  }
  console.log(
    `${region.name}: ${placed.length} maps in ${pieces.length} pieces, ${regionBounds[region.name].width}x${regionBounds[region.name].height}`,
  )
  all.push(...placed)
}
// Sinjoh lies past MT. SILVER, north of Route 28, and Sinnoh beyond it, with
// MT. CORONET's south side facing Sinjoh, close enough that the fill between
// them reads as one stretch of land.
const shift = (region, dx, dy) => {
  for (const p of all)
    if (p.region === region) {
      p.x += dx
      p.y += dy
    }
}
const bounds = (region) => {
  const list = all.filter((p) => p.region === region)
  return {
    left: Math.min(...list.map((p) => p.x)),
    top: Math.min(...list.map((p) => p.y)),
    right: Math.max(...list.map((p) => p.x + p.l.width)),
    bottom: Math.max(...list.map((p) => p.y + p.l.height)),
  }
}
const route28 = all.find((p) => p.m.name === "Route28_hns")
{
  const sinjoh = bounds("Sinjoh")
  shift("Sinjoh", Math.round(route28.x + route28.l.width / 2 - (sinjoh.left + sinjoh.right) / 2), 0)
  // Seat it on whichever Johto and Kanto maps lie below it.
  const placed = bounds("Sinjoh")
  const below = all.filter(
    (p) => p.region === "Johto and Kanto" && p.x < placed.right && p.x + p.l.width > placed.left,
  )
  shift("Sinjoh", 0, Math.min(...below.map((p) => p.y)) - 6 - placed.bottom)
  const sinjohNow = bounds("Sinjoh")
  const sinnohBelow = all.filter(
    (p) => p.region === "Sinnoh" && p.x < sinjohNow.right && p.x + p.l.width > sinjohNow.left,
  )
  const coronet = all.find((p) => p.m.name === "Route207")
  const dx = Math.round((sinjohNow.left + sinjohNow.right) / 2 - (coronet.x + coronet.l.width / 2))
  const sinnohBottom = Math.max(
    ...all
      .filter(
        (p) =>
          p.region === "Sinnoh" &&
          p.x + dx < sinjohNow.right &&
          p.x + p.l.width + dx > sinjohNow.left,
      )
      .map((p) => p.y + p.l.height),
  )
  void sinnohBelow
  shift("Sinnoh", dx, sinjohNow.top - 6 - sinnohBottom)
}
for (const p of all.filter((q) => q.region === "Sinjoh"))
  console.log("  ", p.m.name, p.x, p.y, p.l.width, p.l.height)
// Open sea all round.
const MARGIN = 48
const minX = Math.min(...all.map((p) => p.x)) - MARGIN,
  minY = Math.min(...all.map((p) => p.y)) - MARGIN
const W = Math.max(...all.map((p) => p.x + p.l.width)) - minX + MARGIN,
  H = Math.max(...all.map((p) => p.y + p.l.height)) - minY + MARGIN
console.log("world", all.length, "maps,", W, "x", H, "metatiles")

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
// Sinjoh's maps are bordered by a cave's black void; give them the snow past
// Route 217's edge instead, so Sinjoh joins the land on either side.
const route217 = layouts[byName.Route217.layout]
const borders = all.map((p) => {
  if (p.region === "Sinjoh")
    p = { ...p, l: { ...route217, primary_tileset: route217.primary_tileset } }
  const bin = fs.readFileSync(`${game}/${p.l.border_filepath}`)
  // A few layouts declare a bigger border than their file holds; fall back to 2x2.
  const declared = (p.l.border_width ?? 2) * (p.l.border_height ?? 2)
  const fits = bin.length / 2 >= declared
  const bw = fits ? (p.l.border_width ?? 2) : 2,
    bh = fits ? (p.l.border_height ?? 2) : 2
  return {
    bw,
    bh,
    ids: Array.from({ length: bw * bh }, (_, k) =>
      atlasIndex(p.l.primary_tileset, p.l.secondary_tileset, bin.readUInt16LE(k * 2) & 0x3ff),
    ),
  }
})
// Only near the maps, though: a border spread over open ocean would paint
// whole seas in one map's trees. Maps whose border is the black void past a
// cave's edge don't spread at all. Past the fill, it's open sea.
const FILL = 20
const average = (k) => {
  const tile = atlasTiles[k]
  let sum = 0
  for (let i = 0; i < 256; i++) sum += tile[i * 4] + tile[i * 4 + 1] + tile[i * 4 + 2]
  return sum / 256
}
const spreads = borders.map((b) => b.ids.every((k) => average(k) > 60))
let frontier = []
for (let i = 0; i < W * H; i++) if (owner[i] >= 0 && spreads[owner[i]]) frontier.push(i)
for (let step = 0; step < FILL && frontier.length; step++) {
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
const ocean = borders[all.findIndex((p) => p.m.name === "Route105")]
for (let i = 0; i < W * H; i++) {
  if (grid[i] !== EMPTY) continue
  const x = i % W,
    y = (i - x) / W
  grid[i] = ocean.ids[(y % ocean.bh) * ocean.bw + (x % ocean.bw)]
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
  JSON.stringify({
    width: W,
    height: H,
    columns: COLUMNS,
    count: atlasTiles.length,
    towns,
    maps: all.map((p) => ({
      name: p.m.name,
      x: p.x - minX,
      y: p.y - minY,
      width: p.l.width,
      height: p.l.height,
    })),
  }),
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
