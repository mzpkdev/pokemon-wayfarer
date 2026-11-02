// Renders GBA graphics the way the hardware composes them: 8x8 tiles from a
// tile sheet, placed by a text-mode tilemap, colored by 16- or 256-color
// palettes, with color 0 transparent.
import * as fs from "node:fs"

import { readPng } from "./png.mjs"

/** Reads a JASC-PAL file into [r, g, b, a] entries. */
export const readJascPalette = (file) => {
  const lines = fs.readFileSync(file, "utf8").trim().split(/\r?\n/)
  if (lines[0] !== "JASC-PAL") throw new Error(`${file} is not a JASC palette`)
  return lines.slice(3).map((line) => [...line.trim().split(/\s+/).map(Number), 255])
}

/** Splits an indexed tile sheet into 8x8 tiles of palette indices, row-major. */
export const readTiles = (file) => {
  const png = readPng(file)
  if (png.colorType !== 3) throw new Error(`${file} is not an indexed PNG`)
  const tiles = []
  for (let ty = 0; ty < png.height / 8; ty++)
    for (let tx = 0; tx < png.width / 8; tx++) {
      const tile = new Uint8Array(64)
      for (let y = 0; y < 8; y++)
        for (let x = 0; x < 8; x++)
          tile[y * 8 + x] = png.samples[(ty * 8 + y) * png.width + tx * 8 + x]
      tiles.push(tile)
    }
  return { tiles, palette: png.palette }
}

/**
 * Draws a text-mode tilemap. Each 16-bit entry holds the tile number (bits
 * 0-9), horizontal and vertical flips (bits 10, 11) and, for 4bpp layers, the
 * palette bank (bits 12-15).
 */
export const renderTilemap = ({ tiles, palette, map, columns = 32, bpp }) => {
  const entries = new Uint16Array(map.buffer, map.byteOffset, map.length / 2)
  const rows = Math.ceil(entries.length / columns)
  const width = columns * 8
  const height = rows * 8
  const rgba = new Uint8Array(width * height * 4)

  entries.forEach((entry, i) => {
    const tile = tiles[entry & 0x3ff]
    if (!tile) return
    const hflip = entry & 0x400
    const vflip = entry & 0x800
    const bank = bpp === 4 ? entry >> 12 : 0
    const originX = (i % columns) * 8
    const originY = Math.floor(i / columns) * 8
    for (let y = 0; y < 8; y++)
      for (let x = 0; x < 8; x++) {
        const raw = tile[(vflip ? 7 - y : y) * 8 + (hflip ? 7 - x : x)]
        const index = bpp === 4 ? raw & 0xf : raw
        if (index === 0) continue
        const color = palette[bank * 16 + index] ?? [255, 0, 255, 255]
        rgba.set(color, ((originY + y) * width + originX + x) * 4)
      }
  })

  return { width, height, rgba }
}

/** Draws an indexed sprite sheet with a palette, treating index 0 as transparent. */
export const renderIndexed = (file, palette) => {
  const png = readPng(file)
  const colors = palette ?? png.palette
  const rgba = new Uint8Array(png.width * png.height * 4)
  png.samples.forEach((index, i) => {
    if (index !== 0) rgba.set(colors[index] ?? [255, 0, 255, 255], i * 4)
  })
  return { width: png.width, height: png.height, rgba }
}
