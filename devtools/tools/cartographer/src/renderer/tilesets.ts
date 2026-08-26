import * as fs from "node:fs"
import * as path from "node:path"

import { readIndexedPng } from "./png"
import type { IndexedPng, Layout, LayoutFormat, RenderAssets, Rgb, TilesetAssets } from "./types"

const renderAssets = new Map<string, RenderAssets>()

export const readPalette = (filePath: string): Rgb[] => {
  const lines = fs.readFileSync(filePath, "utf8").split(/\r?\n/)
  const colorCount = Number(lines[2])
  const colors = lines
    .slice(3, 3 + colorCount)
    .map((line) => line.split(/\s+/).map(Number) as [number, number, number])
  if (
    !Number.isInteger(colorCount) ||
    colorCount < 1 ||
    colorCount > 16 ||
    colors.length !== colorCount ||
    colors.some((color) => color.some(Number.isNaN))
  ) {
    throw new Error(`invalid palette: ${filePath}`)
  }
  return colors
}

const splitTiles = (image: IndexedPng): Uint8Array[] => {
  const tiles: Uint8Array[] = []
  for (let tileY = 0; tileY < image.height; tileY += 8) {
    for (let tileX = 0; tileX < image.width; tileX += 8) {
      const tile = new Uint8Array(64)
      for (let y = 0; y < 8; y += 1) {
        tile.set(image.rows[tileY + y]!.subarray(tileX, tileX + 8), y * 8)
      }
      tiles.push(tile)
    }
  }
  return tiles
}

const tilesetGraphics = (root: string): string => {
  return [
    fs.readFileSync(path.join(root, "src/data/tilesets/graphics.h"), "utf8"),
    fs.existsSync(path.join(root, "src/graphics.c"))
      ? fs.readFileSync(path.join(root, "src/graphics.c"), "utf8")
      : "",
  ].join("\n")
}

const resolveTilesetDirectory = (root: string, symbol: string): string => {
  const graphics = tilesetGraphics(root)
  const stem = symbol.replace(/^gTileset_/, "")
  const expression = new RegExp(
    `gTilesetTiles_${stem}\\[\\].*?"([^"]+)/tiles(?:\\.png|\\.4bpp(?:\\.\\w+)?)"`,
  )
  const match = expression.exec(graphics)
  if (match?.[1]) {
    return path.join(root, match[1])
  }
  const snakeName = stem.replace(/(?!^)([A-Z])/g, "_$1").toLowerCase()
  for (const kind of ["primary", "secondary"]) {
    const candidate = path.join(root, "data/tilesets", kind, snakeName)
    if (fs.existsSync(path.join(candidate, "tiles.png"))) {
      return candidate
    }
  }
  throw new Error(`cannot resolve ${symbol}`)
}

export const resolveTilesetAssets = (root: string, symbol: string): TilesetAssets => {
  const headers = fs.readFileSync(path.join(root, "src/data/tilesets/headers.h"), "utf8")
  const graphics = tilesetGraphics(root)
  const metatiles = fs.readFileSync(path.join(root, "src/data/tilesets/metatiles.h"), "utf8")
  const header = new RegExp(`const struct Tileset ${symbol}\\s*=\\s*\\{([\\s\\S]*?)\\};`).exec(
    headers,
  )
  if (header?.[1]) {
    const fields = new Map(
      [...header[1].matchAll(/\.(tiles|palettes|metatiles|metatileAttributes)\s*=\s*(\w+)/g)].map(
        (match) => [match[1]!, match[2]!],
      ),
    )
    const files = new Map<string, string>()
    const patterns: Record<string, [string, RegExp]> = {
      tiles: [graphics, /\[\].*?"([^"]+\/tiles(?:\.png|\.4bpp(?:\.\w+)?))"/s],
      palettes: [graphics, /.*?\{.*?"([^"]+\/palettes\/\d+\.(?:pal|gbapal))"/s],
      metatiles: [metatiles, /\[\].*?"([^"]+\/metatiles\.bin)"/s],
      metatileAttributes: [metatiles, /\[\].*?"([^"]+\/metatile_attributes\.bin)"/s],
    }
    for (const [field, [source, pattern]] of Object.entries(patterns)) {
      const resource = fields.get(field)
      const match = resource
        ? new RegExp(`${resource}\\b${pattern.source}`, pattern.flags).exec(source)
        : null
      if (match?.[1]) {
        files.set(field, path.join(root, match[1]))
      }
    }
    const tiles = files.get("tiles")
    const palettes = files.get("palettes")
    const metatilePath = files.get("metatiles")
    const metatileAttributes = files.get("metatileAttributes")
    if (tiles && palettes && metatilePath && metatileAttributes) {
      const pngTiles = tiles.replace(/\/tiles(?:\.png|\.4bpp(?:\.\w+)?)$/, "/tiles.png")
      return {
        tiles: fs.existsSync(pngTiles) ? pngTiles : tiles,
        palettes: path.dirname(palettes),
        metatiles: metatilePath,
        metatileAttributes,
      }
    }
  }
  const directory = resolveTilesetDirectory(root, symbol)
  return {
    tiles: path.join(directory, "tiles.png"),
    palettes: path.join(directory, "palettes"),
    metatiles: path.join(directory, "metatiles.bin"),
    metatileAttributes: path.join(directory, "metatile_attributes.bin"),
  }
}

export const normalizeLayoutFormat = (
  layout: Pick<Layout, "format" | "game_version" | "layout_version">,
): LayoutFormat => {
  const format = layout.layout_version ?? layout.format ?? layout.game_version ?? "emerald"
  if (format === "emerald" || format === "frlg" || format === "hns") return format
  throw new Error(`unsupported Pokémon Wayfarer layout format: ${format}`)
}

/** Match Pokémon Wayfarer's runtime layout-version table, not conditional C header defines. */
export const readLayoutFormatCounts = (layoutFormat: LayoutFormat): [number, number, number] => {
  if (layoutFormat === "emerald") return [512, 512, 6]
  return [640, 640, 7]
}

export const metatileAttributeSize = (attributes: Buffer, metatileCount: number): 2 | 4 => {
  if (!Number.isInteger(metatileCount) || metatileCount < 1) {
    throw new Error(`cannot determine metatile attribute width for ${metatileCount} metatile(s)`)
  }
  const width = attributes.length / metatileCount
  if (width !== 2 && width !== 4) {
    throw new Error(
      `metatile attributes must contain 2 or 4 bytes per metatile, found ${attributes.length} bytes for ${metatileCount} metatile(s)`,
    )
  }
  return width
}

export const readMetatileAttribute = (
  attributes: Buffer,
  id: number,
  attributeSize: 2 | 4,
): number => {
  const offset = id * attributeSize
  if (offset + attributeSize > attributes.length) {
    throw new Error(`metatile attributes are missing entry 0x${id.toString(16).padStart(3, "0")}`)
  }
  return attributeSize === 4 ? attributes.readUInt32LE(offset) : attributes.readUInt16LE(offset)
}

const choosePalettePath = (primary: string, secondary: string, index: number): string => {
  const name = `${String(index).padStart(2, "0")}.pal`
  const preferred = path.join(primary, name)
  return fs.existsSync(preferred) ? preferred : path.join(secondary, name)
}

export const loadRenderAssets = (
  root: string,
  layout: Pick<Layout, "primary_tileset" | "secondary_tileset">,
  primaryTileCount: number,
  primaryPaletteCount: number,
  paletteCount: number,
): RenderAssets => {
  const cacheKey = [
    root,
    layout.primary_tileset,
    layout.secondary_tileset,
    primaryTileCount,
    primaryPaletteCount,
    paletteCount,
  ].join("\u0000")
  const cached = renderAssets.get(cacheKey)
  if (cached) {
    return cached
  }
  const primary = resolveTilesetAssets(root, layout.primary_tileset)
  const secondary = resolveTilesetAssets(root, layout.secondary_tileset)
  const assets = {
    primaryTiles: splitTiles(readIndexedPng(primary.tiles)),
    secondaryTiles: splitTiles(readIndexedPng(secondary.tiles)),
    primaryMetatiles: fs.readFileSync(primary.metatiles),
    secondaryMetatiles: fs.readFileSync(secondary.metatiles),
    primaryMetatileAttributes: fs.readFileSync(primary.metatileAttributes),
    secondaryMetatileAttributes: fs.readFileSync(secondary.metatileAttributes),
    palettes: Array.from({ length: paletteCount }, (_, index) =>
      readPalette(
        choosePalettePath(
          index < primaryPaletteCount ? primary.palettes : secondary.palettes,
          index < primaryPaletteCount ? secondary.palettes : primary.palettes,
          index,
        ),
      ),
    ),
  }
  renderAssets.set(cacheKey, assets)
  return assets
}
