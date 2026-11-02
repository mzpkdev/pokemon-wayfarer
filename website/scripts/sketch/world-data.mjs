// Sketch: the whole overworld as a metatile atlas and index grid for the
// world explorer: every region's outdoor maps in their real metatiles, laid
// out by their connections, loose pieces placed by their Town Map cells, and
// the regions arranged on one sea.
// usage: pnpm sketch:world
import { spawnSync } from "node:child_process"
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
  return relax([...placed.values()])
}

// The links left out of the tree still say where their maps belong, and
// their whole mismatch sits on that one link. Spread it round the loop: move
// every map a little toward where all its links want it, then push apart any
// maps that touch, until it settles. A loop's error then shows as a few short
// gaps the fill turns into land, not one long one.
const linkErrors = (list) => {
  const at = new Map(list.map((p) => [p.m.id, p]))
  const links = []
  for (const a of list)
    for (const c of connections(a.m)) {
      const b = at.get(targetOf(a.m, c))
      if (!b || a.m.id > b.m.id) continue
      const o = c.offset
      const [dx, dy] = {
        up: [o, -b.l.height],
        down: [o, a.l.height],
        left: [-b.l.width, o],
        right: [a.l.width, o],
      }[c.direction] ?? [null, null]
      if (dx === null) continue
      links.push({ a, b, dx, dy })
    }
  return links
}
const relax = (list) => {
  const links = linkErrors(list)
  if (links.length < list.length) return list
  // The worst link is what shows: one long gap or skewed road.
  const error = () =>
    Math.max(
      ...links.map(({ a, b, dx, dy }) => Math.abs(b.x - a.x - dx) + Math.abs(b.y - a.y - dy)),
    )
  const before = error()
  if (before === 0) return list
  const start = list.map((p) => [p.x, p.y])
  const root = list[0]
  for (let round = 0; round < 400; round++) {
    const pull = new Map(list.map((p) => [p, { x: 0, y: 0, n: 0 }]))
    for (const { a, b, dx, dy } of links) {
      const pa = pull.get(a),
        pb = pull.get(b)
      pb.x += a.x + dx - b.x
      pb.y += a.y + dy - b.y
      pb.n++
      pa.x += b.x - dx - a.x
      pa.y += b.y - dy - a.y
      pa.n++
    }
    for (const p of list) {
      if (p === root) continue
      const { x, y, n } = pull.get(p)
      if (n === 0) continue
      p.x += Math.round((x / n) * 0.5)
      p.y += Math.round((y / n) * 0.5)
    }
    // Push touching maps apart along their shallower overlap.
    for (let i = 0; i < list.length; i++)
      for (let j = i + 1; j < list.length; j++) {
        const a = list[i],
          b = list[j]
        const w = Math.min(a.x + a.l.width, b.x + b.l.width) - Math.max(a.x, b.x)
        const h = Math.min(a.y + a.l.height, b.y + b.l.height) - Math.max(a.y, b.y)
        if (w <= 0 || h <= 0) continue
        const [mover, still] = b === root ? [a, b] : [b, a]
        if (w < h) mover.x += mover.x + mover.l.width / 2 >= still.x + still.l.width / 2 ? w : -w
        else mover.y += mover.y + mover.l.height / 2 >= still.y + still.l.height / 2 ? h : -h
      }
  }
  const after = error()
  const clash = overlap(list)
  if (clash > 0 || after >= before) {
    list.forEach((p, k) => {
      p.x = start[k][0]
      p.y = start[k][1]
    })
    console.log(
      `  ${root.m.name}: kept the tree (worst link ${after} vs ${before}, overlap ${clash})`,
    )
  } else {
    const worst = links.toSorted(
      (p, q) =>
        Math.abs(q.b.x - q.a.x - q.dx) +
        Math.abs(q.b.y - q.a.y - q.dy) -
        (Math.abs(p.b.x - p.a.x - p.dx) + Math.abs(p.b.y - p.a.y - p.dy)),
    )[0]
    console.log(
      `  ${root.m.name}: worst link ${before} -> ${after} metatiles (${worst.a.m.name} ~ ${worst.b.m.name})`,
    )
  }
  return list
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
    at: [-160, 600],
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
    at: [720, 560],
  },
  { name: "Alola", roots: ["MAP_MELEMELE_ISLE_HNS"], at: [1220, 700] },
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

// A Town Map cell is coarse, so pieces placed by their cells can land on one
// another. Nudge each piece but the first in shrinking steps, whichever way
// most reduces what it covers of the others, until nothing overlaps (with a
// few tiles of room). Sootopolis belongs inside Route 126's crater, so the
// two may overlap.
const ROOM = 3
const covered = (a, b) => {
  if (a.m.name === "SootopolisCity" || b.m.name === "SootopolisCity") return 0
  const w = Math.min(a.x + a.l.width, b.x + b.l.width) - Math.max(a.x, b.x) + ROOM
  const h = Math.min(a.y + a.l.height, b.y + b.l.height) - Math.max(a.y, b.y) + ROOM
  return w > 0 && h > 0 ? w * h : 0
}
const pieceOverlap = (piece, others) => {
  let area = 0
  for (const p of piece) for (const q of others) area += covered(p, q)
  return area
}
const separate = (pieces) => {
  for (let round = 0; round < 400; round++) {
    let moved = false
    for (let k = 1; k < pieces.length; k++) {
      const others = pieces.filter((_, j) => j !== k).flat()
      let area = pieceOverlap(pieces[k], others)
      if (area === 0) continue
      for (const step of [64, 32, 16, 8, 4, 2, 1]) {
        let best = null
        for (const [dx, dy] of [
          [step, 0],
          [-step, 0],
          [0, step],
          [0, -step],
        ]) {
          for (const p of pieces[k]) {
            p.x += dx
            p.y += dy
          }
          const trial = pieceOverlap(pieces[k], others)
          for (const p of pieces[k]) {
            p.x -= dx
            p.y -= dy
          }
          if (trial < area && (!best || trial < best.area)) best = { dx, dy, area: trial }
        }
        if (best) {
          for (const p of pieces[k]) {
            p.x += best.dx
            p.y += best.dy
          }
          area = best.area
          moved = true
          break
        }
      }
    }
    if (!moved) break
  }
}

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
  separate(pieces)
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
  // Away from Sinjoh, keep a strait between Sinnoh and Johto and Kanto wide
  // enough that their coasts don't meet, so the land runs only through Sinjoh.
  const STRAIT = 20
  let lift = sinjohNow.top - 6 - sinnohBottom
  for (const s of all.filter((p) => p.region === "Sinnoh")) {
    const left = s.x + dx,
      right = left + s.l.width
    if (left < sinjohNow.right + STRAIT / 2 && right > sinjohNow.left - STRAIT / 2) continue
    for (const j of all.filter((p) => p.region === "Johto and Kanto"))
      if (left < j.x + j.l.width + STRAIT && right > j.x - STRAIT)
        lift = Math.min(lift, j.y - STRAIT - (s.y + s.l.height))
  }
  shift("Sinnoh", dx, lift)
  console.log("  Sinnoh to Sinjoh gap:", sinjohNow.top - (sinnohBottom + lift))
}
for (const p of all.filter((q) => q.region === "Sinjoh"))
  console.log("  ", p.m.name, p.x, p.y, p.l.width, p.l.height)
// Open sea all round.
const MARGIN = 72
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
// The land between the maps. Map tiles say where the world is land and where
// it is water; land grows out from the maps' land to a coastline that wanders
// with fractal noise, sea stays sea, and stretches of water that no open sea
// reaches (the insides of route loops) become land too. Each filled cell
// then takes the border blocks of the nearest map with that kind of border,
// the way the game fills past a map's edge, tiled on the world grid.
const N = W * H
const tileColor = (() => {
  const cache = new Map()
  return (k) => {
    if (cache.has(k)) return cache.get(k)
    const tile = atlasTiles[k]
    let r = 0,
      g = 0,
      b = 0
    for (let i = 0; i < 256; i++) {
      r += tile[i * 4]
      g += tile[i * 4 + 1]
      b += tile[i * 4 + 2]
    }
    const color = [r / 256, g / 256, b / 256]
    cache.set(k, color)
    return color
  }
})()
// Water is a clear, saturated blue; grey-blue rock, snow and ice are land.
const isWater = (k) => {
  const [r, g, b] = tileColor(k)
  return b > 150 && b - r > 55 && b - g > 28
}
const isDark = (k) => {
  const [r, g, b] = tileColor(k)
  return r + g + b < 120
}

// Sinjoh's maps are bordered by a cave's black void; give them the snow past
// Route 217's edge instead, so Sinjoh joins the land on either side.
const route217 = layouts[byName.Route217.layout]
const borders = all.map((p) => {
  const layout = p.region === "Sinjoh" ? route217 : p.l
  const bin = fs.readFileSync(`${game}/${layout.border_filepath}`)
  // A few layouts declare a bigger border than their file holds; fall back to 2x2.
  const declared = (layout.border_width ?? 2) * (layout.border_height ?? 2)
  const fits = bin.length / 2 >= declared
  const bw = fits ? (layout.border_width ?? 2) : 2,
    bh = fits ? (layout.border_height ?? 2) : 2
  const ids = Array.from({ length: bw * bh }, (_, k) =>
    atlasIndex(layout.primary_tileset, layout.secondary_tileset, bin.readUInt16LE(k * 2) & 0x3ff),
  )
  const water = ids.filter(isWater).length
  const kind = ids.some(isDark) ? "none" : water * 2 > ids.length ? "water" : "land"
  return { bw, bh, ids, kind }
})
const pattern = (b, i) => {
  const x = i % W,
    y = (i - x) / W
  return b.ids[(y % b.bh) * b.bw + (x % b.bw)]
}

// Fractal value noise, fixed seed, in [-1, 1].
const hash = (x, y) => {
  let h = (x * 374761393 + y * 668265263) ^ 0x5bd1e995
  h = Math.imul(h ^ (h >>> 13), 1274126177)
  return ((h ^ (h >>> 16)) >>> 0) / 4294967296
}
const valueNoise = (x, y) => {
  const ix = Math.floor(x),
    iy = Math.floor(y)
  const fx = x - ix,
    fy = y - iy
  const sx = fx * fx * (3 - 2 * fx),
    sy = fy * fy * (3 - 2 * fy)
  const top = hash(ix, iy) + (hash(ix + 1, iy) - hash(ix, iy)) * sx
  const bottom = hash(ix, iy + 1) + (hash(ix + 1, iy + 1) - hash(ix, iy + 1)) * sx
  return top + (bottom - top) * sy
}
const fractal = (x, y, base = 1 / 48) => {
  let sum = 0,
    weight = 0,
    amplitude = 1,
    frequency = base
  for (let octave = 0; octave < 4; octave++) {
    sum += amplitude * valueNoise(x * frequency + octave * 17.3, y * frequency - octave * 9.1)
    weight += amplitude
    amplitude /= 2
    frequency *= 2
  }
  return (sum / weight) * 2 - 1
}
// Domain-warped: the noise sampled at a point pushed about by more noise,
// which bends coastlines into capes and inlets instead of ripples.
const coastNoise = (x, y) => {
  const wx = fractal(x + 101, y - 57, 1 / 90) * 28
  const wy = fractal(x - 33, y + 211, 1 / 90) * 28
  return fractal(x + wx, y + wy, 1 / 70)
}

// Distance (in metatiles, chamfer 3-4) from each cell to the nearest cell in `seed`.
const distanceFrom = (seed) => {
  const d = new Float32Array(N).fill(1e9)
  for (let i = 0; i < N; i++) if (seed[i]) d[i] = 0
  for (let y = 0; y < H; y++)
    for (let x = 0; x < W; x++) {
      const i = y * W + x
      let v = d[i]
      if (x > 0) v = Math.min(v, d[i - 1] + 1)
      if (y > 0) {
        v = Math.min(v, d[i - W] + 1)
        if (x > 0) v = Math.min(v, d[i - W - 1] + Math.SQRT2)
        if (x < W - 1) v = Math.min(v, d[i - W + 1] + Math.SQRT2)
      }
      d[i] = v
    }
  for (let y = H - 1; y >= 0; y--)
    for (let x = W - 1; x >= 0; x--) {
      const i = y * W + x
      let v = d[i]
      if (x < W - 1) v = Math.min(v, d[i + 1] + 1)
      if (y < H - 1) {
        v = Math.min(v, d[i + W] + 1)
        if (x < W - 1) v = Math.min(v, d[i + W + 1] + Math.SQRT2)
        if (x > 0) v = Math.min(v, d[i + W - 1] + Math.SQRT2)
      }
      d[i] = v
    }
  return d
}

// Land grows only from maps that are mostly land; a sea route's rocks and
// islets don't make a coast, and the water past a sea route stays sea.
const waterShare = new Float32Array(all.length)
const cellCount = new Float32Array(all.length)
for (let i = 0; i < N; i++)
  if (owner[i] >= 0) {
    cellCount[owner[i]]++
    if (isWater(grid[i])) waterShare[owner[i]]++
  }
for (let m = 0; m < all.length; m++) waterShare[m] /= cellCount[m]
if (process.env.DEBUG_MAPS)
  for (const name of process.env.DEBUG_MAPS.split(",")) {
    const m = all.findIndex((p) => p.m.name === name)
    const colors = new Map()
    for (let i = 0; i < N; i++)
      if (owner[i] === m && isWater(grid[i])) colors.set(grid[i], (colors.get(grid[i]) ?? 0) + 1)
    const top = [...colors].toSorted((a, b) => b[1] - a[1]).slice(0, 4)
    console.log(
      name,
      waterShare[m].toFixed(2),
      top.map(([k, n]) => `${tileColor(k).map(Math.round)}x${n}`).join(" "),
    )
  }
const mapLand = new Uint8Array(N)
const mapSea = new Uint8Array(N)
for (let i = 0; i < N; i++)
  if (owner[i] >= 0) {
    if (waterShare[owner[i]] > 0.55) mapSea[i] = 1
    else if (!isWater(grid[i])) mapLand[i] = 1
  }
const toLand = distanceFrom(mapLand)
const toSeaRoute = distanceFrom(mapSea)
// How far land reaches past the maps: about 18 metatiles, give or take 24.
const REACH = 18,
  WANDER = 24
let land = new Uint8Array(N)
for (let i = 0; i < N; i++) {
  if (owner[i] >= 0) land[i] = owner[i] >= 0 && !isWater(grid[i]) ? 1 : 0
  else {
    const x = i % W,
      y = (i - x) / W
    land[i] = toLand[i] < REACH + WANDER * coastNoise(x, y) && toLand[i] < toSeaRoute[i] ? 1 : 0
  }
}
// Smooth the filled coast: each filled cell takes the majority of its 5x5
// neighborhood, twice, which rounds off stray corners and specks.
for (let pass = 0; pass < 2; pass++) {
  const next = land.slice()
  for (let y = 2; y < H - 2; y++)
    for (let x = 2; x < W - 2; x++) {
      const i = y * W + x
      if (owner[i] >= 0) continue
      let count = 0
      for (let dy = -2; dy <= 2; dy++)
        for (let dx = -2; dx <= 2; dx++) count += land[i + dy * W + dx]
      next[i] = count > 12 ? 1 : 0
    }
  land = next
}
// Water that no open sea reaches is enclosed by land: the inside of a route
// loop, not a lake any map draws. Fill it in.
{
  const open = new Uint8Array(N)
  const queue = []
  for (let x = 0; x < W; x++) queue.push(x, (H - 1) * W + x)
  for (let y = 0; y < H; y++) queue.push(y * W, y * W + W - 1)
  for (const i of queue) open[i] = land[i] ? 0 : 1
  let head = 0
  const stack = queue.filter((i) => open[i])
  while (head < stack.length) {
    const i = stack[head++]
    const x = i % W,
      y = (i - x) / W
    for (const j of [
      x > 0 ? i - 1 : -1,
      x < W - 1 ? i + 1 : -1,
      y > 0 ? i - W : -1,
      y < H - 1 ? i + W : -1,
    ])
      if (j >= 0 && !open[j] && !land[j]) {
        open[j] = 1
        stack.push(j)
      }
  }
  for (let i = 0; i < N; i++) if (!land[i] && !open[i] && owner[i] < 0) land[i] = 1
}

// Regions stay apart: each cell belongs to its nearest region, and where two
// regions' cells meet, a strait of sea runs between them. Sinjoh is the
// exception; it is the land that joins Johto to Sinnoh.
{
  const regionOf = all.map((p) => p.region)
  const nearestRegion = new Int16Array(N).fill(-1)
  const names = [...new Set(regionOf)]
  const queue = []
  for (let i = 0; i < N; i++)
    if (owner[i] >= 0) {
      nearestRegion[i] = names.indexOf(regionOf[owner[i]])
      queue.push(i)
    }
  let head = 0
  while (head < queue.length) {
    const i = queue[head++]
    const x = i % W,
      y = (i - x) / W
    for (const j of [
      x > 0 ? i - 1 : -1,
      x < W - 1 ? i + 1 : -1,
      y > 0 ? i - W : -1,
      y < H - 1 ? i + W : -1,
    ])
      if (j >= 0 && nearestRegion[j] < 0) {
        nearestRegion[j] = nearestRegion[i]
        queue.push(j)
      }
  }
  const sinjoh = names.indexOf("Sinjoh")
  const HALF = 5
  for (let y = HALF; y < H - HALF; y++)
    for (let x = HALF; x < W - HALF; x++) {
      const i = y * W + x
      if (owner[i] >= 0 || !land[i]) continue
      const mine = nearestRegion[i]
      if (mine === sinjoh) continue
      for (const [dx, dy] of [
        [HALF, 0],
        [-HALF, 0],
        [0, HALF],
        [0, -HALF],
      ]) {
        const other = nearestRegion[i + dy * W + dx]
        if (other !== mine && other !== sinjoh) {
          land[i] = 0
          break
        }
      }
    }
}

// Which map each cell takes its border from: the nearest map of the right kind.
const nearest = (kind) => {
  const from = new Int32Array(N).fill(-1)
  const queue = []
  for (let i = 0; i < N; i++)
    if (owner[i] >= 0 && borders[owner[i]].kind === kind) {
      from[i] = owner[i]
      queue.push(i)
    }
  let head = 0
  while (head < queue.length) {
    const i = queue[head++]
    const x = i % W,
      y = (i - x) / W
    for (const j of [
      x > 0 ? i - 1 : -1,
      x < W - 1 ? i + 1 : -1,
      y > 0 ? i - W : -1,
      y < H - 1 ? i + W : -1,
    ])
      if (j >= 0 && from[j] < 0) {
        from[j] = from[i]
        queue.push(j)
      }
  }
  return from
}
const landFrom = nearest("land")
const waterFrom = nearest("water")
const ocean = borders[all.findIndex((p) => p.m.name === "Route105")]
// Look the nearest map up from a point pushed about by noise, so where one
// map's fill meets another's the boundary wanders instead of running straight.
const warped = (from, i) => {
  const x = i % W,
    y = (i - x) / W
  const wx = Math.round(x + fractal(x + 400, y, 1 / 40) * 22)
  const wy = Math.round(y + fractal(x, y + 400, 1 / 40) * 22)
  const j = Math.min(H - 1, Math.max(0, wy)) * W + Math.min(W - 1, Math.max(0, wx))
  return from[j] >= 0 ? from[j] : from[i]
}
const fillBorder = new Int32Array(N).fill(-1)
for (let i = 0; i < N; i++) {
  if (owner[i] >= 0) continue
  if (land[i]) {
    fillBorder[i] = warped(landFrom, i)
    grid[i] = pattern(borders[fillBorder[i]], i)
  } else
    grid[i] =
      waterFrom[i] >= 0 && toLand[i] < 40 ? pattern(borders[waterFrom[i]], i) : pattern(ocean, i)
}

// The far-out view, drawn like a map. Land is each cell's tile color, with
// filled land given a little texture and darkening inland, and the coast
// picked out a shade darker. All water is one sea, whatever game its tiles
// come from: light in the shallows, deepening away from the shore.
const borderColor = (() => {
  const cache = new Map()
  return (m) => {
    if (!cache.has(m)) {
      const colors = borders[m].ids.map(tileColor)
      cache.set(
        m,
        [0, 1, 2].map((c) => colors.reduce((sum, color) => sum + color[c], 0) / colors.length),
      )
    }
    return cache.get(m)
  }
})()
const borderSets = borders.map((b) => new Set(b.ids))
const SEA_SHALLOW = [150, 206, 240],
  SEA_DEEP = [52, 92, 178]
const overview = new Uint8Array(N * 4)
{
  const isLand = new Uint8Array(N)
  for (let i = 0; i < N; i++) isLand[i] = isWater(grid[i]) ? 0 : 1
  const toShore = distanceFrom(isLand)
  const sea = new Uint8Array(N)
  for (let i = 0; i < N; i++) sea[i] = 1 - isLand[i]
  const toSea = distanceFrom(sea)
  for (let i = 0; i < N; i++) {
    const x = i % W,
      y = (i - x) / W
    let r, g, b
    if (isLand[i]) {
      // Filled land takes its whole border pattern's color, so a pattern's
      // alternating blocks don't stripe the far-out view.
      // A map's own edge is mostly its border blocks too; color those the same
      // so maps blend into the land around them.
      const own = owner[i] >= 0 && borderSets[owner[i]].has(grid[i]) ? owner[i] : -1
      ;[r, g, b] =
        fillBorder[i] >= 0
          ? borderColor(fillBorder[i])
          : own >= 0
            ? borderColor(own)
            : tileColor(grid[i])
      if (owner[i] < 0) {
        // Relief: deep inland, denser forest, and highlands where the noise rises.
        const inland = Math.min(1, Math.max(0, (toLand[i] - 6) / 24))
        const forest = inland * 0.3 * (0.6 + 0.4 * fractal(x - 77, y + 13, 1 / 30))
        r += (44 - r) * forest
        g += (108 - g) * forest
        b += (52 - b) * forest
        const high = inland * Math.max(0, fractal(x + 900, y - 300, 1 / 60) - 0.15) * 1.4
        r += (176 - r) * Math.min(0.55, high)
        g += (148 - g) * Math.min(0.55, high)
        b += (112 - b) * Math.min(0.55, high)
        const shade = 1 + 0.08 * fractal(x, y, 1 / 20)
        r *= shade
        g *= shade
        b *= shade
      }
      if (toSea[i] <= 1.5) {
        r *= 0.6
        g *= 0.6
        b *= 0.6
      }
    } else {
      const t = Math.min(1, Math.max(0, (toShore[i] - 1) / 34)) ** 0.7
      const ripple = 1 + 0.03 * fractal(x, y, 1 / 18)
      r = (SEA_SHALLOW[0] + (SEA_DEEP[0] - SEA_SHALLOW[0]) * t) * ripple
      g = (SEA_SHALLOW[1] + (SEA_DEEP[1] - SEA_SHALLOW[1]) * t) * ripple
      b = (SEA_SHALLOW[2] + (SEA_DEEP[2] - SEA_SHALLOW[2]) * t) * ripple
      if (toShore[i] <= 1.5) {
        r = r * 0.6 + 255 * 0.4
        g = g * 0.6 + 255 * 0.4
        b = b * 0.6 + 255 * 0.4
      }
    }
    overview.set([r, g, b, 255], i * 4)
  }
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
writePng(`${outDir}/overview.png`, W, H, overview)
// The far-out view is the first thing to load; a JPEG of it is a third the size
// and looks the same. Made when ffmpeg is around; the explorer falls back to the PNG.
{
  const made = spawnSync(
    "ffmpeg",
    [
      "-loglevel",
      "error",
      "-y",
      "-i",
      `${outDir}/overview.png`,
      "-q:v",
      "2",
      `${outDir}/overview.jpg`,
    ],
    { stdio: "inherit" },
  )
  if (made.status !== 0) fs.rmSync(`${outDir}/overview.jpg`, { force: true })
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
  if (
    !label ||
    (p.region === "Johto and Kanto" && !["MAP_TYPE_TOWN", "MAP_TYPE_CITY"].includes(p.m.map_type))
  )
    continue
  const entry = regionLabels.get(label) ?? { name: label, x: 0, y: 0, n: 0 }
  entry.x += p.x - minX + p.l.width / 2
  entry.y += p.y - minY + p.l.height / 2
  entry.n++
  regionLabels.set(label, entry)
}
const regions = [...regionLabels.values()].map(({ name, x, y, n }) => ({
  name,
  x: x / n,
  y: y / n,
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
