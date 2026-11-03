// The game side of the world map build: reads the romhack's maps, layouts,
// tilesets and Town Map cells from the game tree, and draws metatiles in
// their real colors into an atlas.
import fs from "node:fs"
import path from "node:path"
import { fileURLToPath } from "node:url"
import { readJascPalette } from "../../../scripts/lib/gba.mjs"
import { readPng } from "../../../scripts/lib/png.mjs"

const here = path.dirname(fileURLToPath(import.meta.url))
export const gameRoot = path.resolve(here, "../../../../game")

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
const OUTDOOR = new Set([
  "MAP_TYPE_TOWN",
  "MAP_TYPE_CITY",
  "MAP_TYPE_ROUTE",
  "MAP_TYPE_OCEAN_ROUTE",
])
// How a map's connections lead on: the target map, after the rewiring.
export const connections = (m) => (Array.isArray(m.connections) ? m.connections : [])
export const targetOf = (m, c) => REWIRED[`${m.name}|${c.map}`] ?? c.map

/**
 * Reads the game tree at `root`: the compiled outdoor `maps` (by id) and their
 * `layouts`, `byName`, the `tileset` reader, `drawMetatile`, and `read` for
 * any file under the root.
 */
export const openGame = (root = gameRoot) => {
  const read = (file) => fs.readFileSync(`${root}/${file}`)
  // Tileset symbol -> files, read from the C sources that INCBIN them.
  const src = [
    "headers.h",
    "sinnoh_headers.h",
    "graphics.h",
    "sinnoh_graphics.h",
    "metatiles.h",
    "sinnoh_metatiles.h",
  ]
    .map((f) => fs.readFileSync(`${root}/src/data/tilesets/${f}`, "utf8"))
    .concat(fs.readFileSync(`${root}/src/graphics.c`, "utf8"))
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
    const png = readPng(`${root}/${tilesPath}`)
    const palettes = paletteFiles(field(name, "palettes")).map((f) => {
      const jasc = `${root}/${f.replace(/\.gbapal$/, ".pal")}`
      return fs.existsSync(jasc) ? readJascPalette(jasc) : null
    })
    const metatiles = fs.readFileSync(`${root}/${incbin(field(name, "metatiles"))}`)
    const attributesFile = incbin(field(name, "metatileAttributes"))
    const t = {
      png,
      palettes,
      metatiles,
      /** Each metatile's attributes, u16 (Emerald, HNS) or u32 (FRLG) apiece; the behavior is in the low bits. */
      attributes: attributesFile ? fs.readFileSync(`${root}/${attributesFile}`) : undefined,
      count: metatiles.length / 16,
      tilesPerRow: png.width / 8,
    }
    tilesetCache.set(name, t)
    return t
  }

  // Maps and layouts.
  const layouts = Object.fromEntries(
    JSON.parse(fs.readFileSync(`${root}/data/layouts/layouts.json`))
      .layouts.filter((l) => l.id)
      .map((l) => [l.id, l]),
  )
  const sevii = new Set(
    JSON.parse(read("src/data/wayfarer_sevii_maps.json")).maps.map((entry) => entry.source_map),
  )
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
  for (const dir of fs.readdirSync(`${root}/data/maps`)) {
    const f = `${root}/data/maps/${dir}/map.json`
    if (!fs.existsSync(f)) continue
    const m = JSON.parse(fs.readFileSync(f))
    if (compiled(m)) maps[m.id] = m
  }
  const byName = Object.fromEntries(Object.values(maps).map((m) => [m.name, m]))

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
  return { root, read, layouts, maps, byName, tileset, drawMetatile }
}

// The grid of atlas indices: every distinct metatile drawn once, in the order
// first asked for.
export const createAtlas = (game) => {
  const keys = new Map()
  const tiles = []
  const index = (primary, secondary, id) => {
    const key = `${primary}|${secondary}|${id}`
    if (!keys.has(key)) {
      keys.set(key, tiles.length)
      tiles.push(game.drawMetatile(primary, secondary, id))
    }
    return keys.get(key)
  }
  return { tiles, index }
}

// Town Map cells, to place the pieces no connection reaches: the game's own
// sections, and Sinnoh's from the project its maps came from.
export const readTownMapCells = (game) => {
  const cells = new Map(
    JSON.parse(game.read("src/data/region_map/region_map_sections.json"))
      .map_sections.filter((section) => section.x !== undefined)
      .map((section) => [section.id, section]),
  )
  for (const section of JSON.parse(
    fs.readFileSync(new URL("./source/sinnoh-sections.json", import.meta.url), "utf8"),
  ).map_sections)
    cells.set(section.map_section.replace("MAPSEC_", "MAPSEC_SINNOH_"), section)
  return cells
}
