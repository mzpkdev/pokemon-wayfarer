import * as fs from "node:fs"
import * as path from "node:path"

import { writeRgbPng } from "./png"
import { readDefine, readJson } from "./source"
import { loadRenderAssets, normalizeLayoutFormat, readLayoutFormatCounts } from "./tilesets"
import type { LayoutDocument, MapData } from "./types"

export const exteriorMapTypes = new Set([
  "MAP_TYPE_TOWN",
  "MAP_TYPE_CITY",
  "MAP_TYPE_ROUTE",
  "MAP_TYPE_OCEAN_ROUTE",
  "MAP_TYPE_UNDERWATER",
])

export const discoverExteriorMaps = (root: string): string[] => {
  const mapsRoot = path.join(root, "data/maps")
  return fs
    .readdirSync(mapsRoot)
    .sort()
    .filter((name) => fs.existsSync(path.join(mapsRoot, name, "map.json")))
    .filter((name) =>
      exteriorMapTypes.has(readJson<MapData>(path.join(mapsRoot, name, "map.json")).map_type),
    )
}

export const renderMap = (
  root: string,
  mapName: string,
  output: string,
): { width: number; height: number } => {
  const layouts = readJson<LayoutDocument>(path.join(root, "data/layouts/layouts.json")).layouts
  const mapData = readJson<MapData>(path.join(root, "data/maps", mapName, "map.json"))
  const layout = layouts.find((candidate) => candidate.id === mapData.layout)
  if (!layout) {
    throw new Error(`${mapName}: unknown layout ${mapData.layout}`)
  }
  const [primaryTileCount, primaryMetatileCount, primaryPaletteCount] = readLayoutFormatCounts(
    normalizeLayoutFormat(layout),
  )
  const paletteCount = readDefine(path.join(root, "include/fieldmap.h"), "NUM_PALS_TOTAL")
  const assets = loadRenderAssets(root, layout, primaryTileCount, primaryPaletteCount, paletteCount)

  const blockdata = fs.readFileSync(path.join(root, layout.blockdata_filepath))
  const mapWordCount = layout.width * layout.height
  if (blockdata.length < mapWordCount * 2) {
    throw new Error(`${mapName}: blockdata is shorter than its layout`)
  }
  const outputWidth = layout.width * 16
  const outputHeight = layout.height * 16
  const outputPixels = new Uint8Array(outputWidth * outputHeight * 3)

  const drawTile = (
    tileWord: number,
    destinationX: number,
    destinationY: number,
    transparent: boolean,
  ): void => {
    const tileId = tileWord & 0x3ff
    const horizontalFlip = Boolean(tileWord & 0x400)
    const verticalFlip = Boolean(tileWord & 0x800)
    const palette = assets.palettes[(tileWord >> 12) & 0xf] ?? assets.palettes[0]!
    const tile =
      tileId >= primaryTileCount
        ? (assets.secondaryTiles[tileId - primaryTileCount] ?? new Uint8Array(64))
        : (assets.primaryTiles[tileId] ?? new Uint8Array(64))
    for (let pixelY = 0; pixelY < 8; pixelY += 1) {
      const sourceY = verticalFlip ? 7 - pixelY : pixelY
      for (let pixelX = 0; pixelX < 8; pixelX += 1) {
        const sourceX = horizontalFlip ? 7 - pixelX : pixelX
        const colorIndex = tile[sourceY * 8 + sourceX]!
        if (transparent && colorIndex === 0) {
          continue
        }
        const color = palette[colorIndex] ?? [0, 0, 0]
        const offset = ((destinationY + pixelY) * outputWidth + destinationX + pixelX) * 3
        outputPixels.set(color, offset)
      }
    }
  }

  for (let mapY = 0; mapY < layout.height; mapY += 1) {
    for (let mapX = 0; mapX < layout.width; mapX += 1) {
      const mapWord = blockdata.readUInt16LE((mapY * layout.width + mapX) * 2)
      const metatileId = mapWord & 0x3ff
      const secondarySource = metatileId >= primaryMetatileCount
      const metatileBytes = secondarySource ? assets.secondaryMetatiles : assets.primaryMetatiles
      const metatileIndex = secondarySource ? metatileId - primaryMetatileCount : metatileId
      const start = metatileIndex * 16
      for (let layer = 0; layer < 2; layer += 1) {
        for (let quadrant = 0; quadrant < 4; quadrant += 1) {
          drawTile(
            start + layer * 8 + quadrant * 2 + 2 <= metatileBytes.length
              ? metatileBytes.readUInt16LE(start + layer * 8 + quadrant * 2)
              : 0,
            mapX * 16 + (quadrant % 2) * 8,
            mapY * 16 + Math.floor(quadrant / 2) * 8,
            layer === 1,
          )
        }
      }
    }
  }

  writeRgbPng(output, outputWidth, outputHeight, outputPixels)
  return { width: layout.width, height: layout.height }
}
