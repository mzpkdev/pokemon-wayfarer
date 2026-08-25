import * as crypto from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"

import {
  loadRenderAssets,
  metatileAttributeSize,
  normalizeLayoutFormat,
  readLayoutFormatCounts,
  readMetatileAttribute,
  resolveTilesetAssets,
  writeRgbPng,
} from "@wayfarer/cartographer/renderer"
import type { RenderAssets, Rgb, TilesetAssets } from "@wayfarer/cartographer/renderer"

import {
  posixRelative,
  readDefine,
  sourceBehaviorNames,
  sourceLayouts,
  sourceMaps,
  sourceState,
} from "../source"
import type {
  CatalogMetatile,
  ContextTileset,
  MetatileAttributes,
  MetatileCatalog,
  MetatileCatalogResult,
  MetatileContext,
  MetatileContextCatalog,
  MetatilePlacement,
  MetatileTile,
  MetatileUsage,
  MetatileUsageDiagnostic,
  SourceLayout,
  SourceMapReference,
  TilesetSource,
} from "../types"

const atlasColumns = 32
const pixelsPerMetatile = 16

type ContextDefinition = {
  id: string
  format: "emerald" | "frlg" | "hns"
  primaryTileset: string
  secondaryTileset: string
  layouts: SourceLayout[]
}

type UsageAccumulator = {
  mapName: string
  mapId: string
  layoutId: string
  count: number
  placements: Map<string, MetatilePlacement>
}

type SideUsage = Map<number, Map<string, UsageAccumulator>>

type ContextUsage = {
  primary: SideUsage
  secondary: SideUsage
  diagnostics: Map<
    string,
    UsageAccumulator &
      Omit<MetatileUsageDiagnostic, keyof SourceMapReference | "count" | "placements">
  >
}

const sha256 = (filePath: string): string => {
  return crypto.createHash("sha256").update(fs.readFileSync(filePath)).digest("hex")
}

const slug = (value: string): string => {
  return value
    .replace(/^gTileset_/, "")
    .replace(/([a-z0-9])([A-Z])/g, "$1-$2")
    .replaceAll("_", "-")
    .toLowerCase()
}

const contextId = (
  layout: Pick<
    SourceLayout,
    "format" | "game_version" | "layout_version" | "primary_tileset" | "secondary_tileset"
  >,
): string => {
  return `${normalizeLayoutFormat(layout)}:${layout.primary_tileset}:${layout.secondary_tileset}`
}

const contextDirectory = (context: ContextDefinition): string => {
  return `${slug(context.format)}--${slug(context.primaryTileset)}--${slug(context.secondaryTileset)}`
}

const toSource = (root: string, assets: TilesetAssets): TilesetSource => {
  return {
    tiles: posixRelative(root, assets.tiles),
    palettes: posixRelative(root, assets.palettes),
    metatiles: posixRelative(root, assets.metatiles),
    metatileAttributes: posixRelative(root, assets.metatileAttributes),
  }
}

const attributeFor = (
  attributes: Buffer,
  id: number,
  attributeSize: 2 | 4,
  behaviors: ReadonlyMap<number, string>,
): MetatileAttributes => {
  const raw = readMetatileAttribute(attributes, id, attributeSize)
  const behavior = raw & (attributeSize === 4 ? 0x1ff : 0xff)
  return {
    raw,
    behavior,
    behaviorName: behaviors.get(behavior) ?? null,
    layerType: attributeSize === 4 ? (raw >> 29) & 0x03 : (raw >> 12) & 0x0f,
  }
}

const tileFor = (tileWord: number, primaryTileCount: number): MetatileTile => {
  const tileId = tileWord & 0x3ff
  const source = tileId >= primaryTileCount ? "secondary" : "primary"
  return {
    layer: 0,
    quadrant: 0,
    tileId,
    source,
    sourceTileId: source === "secondary" ? tileId - primaryTileCount : tileId,
    paletteId: (tileWord >> 12) & 0x0f,
    horizontalFlip: Boolean(tileWord & 0x400),
    verticalFlip: Boolean(tileWord & 0x800),
  }
}

export const describeMetatileTiles = (
  metatiles: Buffer,
  id: number,
  primaryTileCount: number,
): MetatileTile[] => {
  const offset = id * 16
  if (offset + 16 > metatiles.length) {
    throw new Error(`metatiles are missing entry 0x${id.toString(16).padStart(3, "0")}`)
  }
  const tiles: MetatileTile[] = []
  for (let layer = 0; layer < 2; layer += 1) {
    for (let quadrant = 0; quadrant < 4; quadrant += 1) {
      const tile = tileFor(
        metatiles.readUInt16LE(offset + layer * 8 + quadrant * 2),
        primaryTileCount,
      )
      tile.layer = layer as 0 | 1
      tile.quadrant = quadrant as 0 | 1 | 2 | 3
      tiles.push(tile)
    }
  }
  return tiles
}

const sourceId = (tilesetId: string, id: number): string => {
  return `${tilesetId}:0x${id.toString(16).toUpperCase().padStart(3, "0")}`
}

const assertMetatileTable = (filePath: string, data: Buffer): void => {
  if (data.length === 0 || data.length % 16 !== 0) {
    throw new Error(`${filePath}: metatiles must contain whole 16-byte entries`)
  }
}

const assertAttributes = (filePath: string, attributes: Buffer, metatileCount: number): 2 | 4 => {
  try {
    return metatileAttributeSize(attributes, metatileCount)
  } catch (error) {
    throw new Error(`${filePath}: ${error instanceof Error ? error.message : String(error)}`)
  }
}

const copyTile = (
  pixels: Uint8Array,
  outputWidth: number,
  destinationX: number,
  destinationY: number,
  tile: Uint8Array,
  palette: Rgb[],
  horizontalFlip: boolean,
  verticalFlip: boolean,
  transparent: boolean,
): void => {
  for (let pixelY = 0; pixelY < 8; pixelY += 1) {
    const sourceY = verticalFlip ? 7 - pixelY : pixelY
    for (let pixelX = 0; pixelX < 8; pixelX += 1) {
      const sourceX = horizontalFlip ? 7 - pixelX : pixelX
      const colorIndex = tile[sourceY * 8 + sourceX]
      if (colorIndex === undefined) {
        throw new Error("decoded tileset data is shorter than 8x8 pixels")
      }
      if (transparent && colorIndex === 0) continue
      const color = palette[colorIndex] ?? [184, 111, 124]
      const outputOffset = ((destinationY + pixelY) * outputWidth + destinationX + pixelX) * 3
      pixels.set(color, outputOffset)
    }
  }
}

const drawUnavailableTile = (
  pixels: Uint8Array,
  outputWidth: number,
  destinationX: number,
  destinationY: number,
): void => {
  for (let pixelY = 0; pixelY < 8; pixelY += 1) {
    for (let pixelX = 0; pixelX < 8; pixelX += 1) {
      const color = (pixelX + pixelY) % 2 === 0 ? [80, 23, 34] : [184, 111, 124]
      const offset = ((destinationY + pixelY) * outputWidth + destinationX + pixelX) * 3
      pixels.set(color, offset)
    }
  }
}

const renderAtlas = (
  output: string,
  context: ContextDefinition,
  side: "primary" | "secondary",
  metatiles: Buffer,
  assets: RenderAssets,
  primaryTileCount: number,
): { widthPixels: number; heightPixels: number; unresolvedTileReferences: number } => {
  const metatileCount = metatiles.length / 16
  const rows = Math.ceil(metatileCount / atlasColumns)
  const widthPixels = atlasColumns * pixelsPerMetatile
  const heightPixels = rows * pixelsPerMetatile
  const pixels = new Uint8Array(widthPixels * heightPixels * 3)
  let unresolvedTileReferences = 0
  for (let metatileId = 0; metatileId < metatileCount; metatileId += 1) {
    const atlasX = (metatileId % atlasColumns) * pixelsPerMetatile
    const atlasY = Math.floor(metatileId / atlasColumns) * pixelsPerMetatile
    for (const tile of describeMetatileTiles(metatiles, metatileId, primaryTileCount)) {
      const sourceTiles = tile.source === "primary" ? assets.primaryTiles : assets.secondaryTiles
      const sourceTile = sourceTiles[tile.sourceTileId]
      if (!sourceTile) {
        unresolvedTileReferences += 1
        drawUnavailableTile(
          pixels,
          widthPixels,
          atlasX + (tile.quadrant % 2) * 8,
          atlasY + Math.floor(tile.quadrant / 2) * 8,
        )
        continue
      }
      const palette = assets.palettes[tile.paletteId]
      if (!palette) {
        unresolvedTileReferences += 1
        drawUnavailableTile(
          pixels,
          widthPixels,
          atlasX + (tile.quadrant % 2) * 8,
          atlasY + Math.floor(tile.quadrant / 2) * 8,
        )
        continue
      }
      copyTile(
        pixels,
        widthPixels,
        atlasX + (tile.quadrant % 2) * 8,
        atlasY + Math.floor(tile.quadrant / 2) * 8,
        sourceTile,
        palette,
        tile.horizontalFlip,
        tile.verticalFlip,
        tile.layer === 1,
      )
    }
  }
  writeRgbPng(output, widthPixels, heightPixels, pixels)
  return { widthPixels, heightPixels, unresolvedTileReferences }
}

const emptyUsage = (): ContextUsage => {
  return { primary: new Map(), secondary: new Map(), diagnostics: new Map() }
}

const usageFor = (
  usage: SideUsage,
  metatileId: number,
  map: SourceMapReference,
  collision: number,
  elevation: number,
): void => {
  const mapKey = `${map.layoutId}\u0000${map.name}`
  const metatileUsage = usage.get(metatileId) ?? new Map<string, UsageAccumulator>()
  const entry = metatileUsage.get(mapKey) ?? {
    mapName: map.name,
    mapId: map.id,
    layoutId: map.layoutId,
    count: 0,
    placements: new Map<string, MetatilePlacement>(),
  }
  const placementKey = `${collision}:${elevation}`
  const placement = entry.placements.get(placementKey) ?? { collision, elevation, count: 0 }
  entry.count += 1
  placement.count += 1
  entry.placements.set(placementKey, placement)
  metatileUsage.set(mapKey, entry)
  usage.set(metatileId, metatileUsage)
}

const usageList = (usage: SideUsage, metatileId: number): MetatileUsage[] => {
  return [...(usage.get(metatileId)?.values() ?? [])]
    .map((entry) => ({
      mapName: entry.mapName,
      mapId: entry.mapId,
      layoutId: entry.layoutId,
      count: entry.count,
      placements: [...entry.placements.values()].sort(
        (left, right) => left.collision - right.collision || left.elevation - right.elevation,
      ),
    }))
    .sort((left, right) => left.mapName.localeCompare(right.mapName))
}

const diagnosticFor = (
  diagnostics: ContextUsage["diagnostics"],
  map: SourceMapReference,
  source: "primary" | "secondary",
  globalMetatileId: number,
  localMetatileId: number,
  availableMetatileCount: number,
  collision: number,
  elevation: number,
): void => {
  const key = `${map.layoutId}\u0000${map.name}\u0000${source}\u0000${localMetatileId}`
  const entry = diagnostics.get(key) ?? {
    mapName: map.name,
    mapId: map.id,
    layoutId: map.layoutId,
    code: "missing_source_metatile" as const,
    source,
    globalMetatileId,
    localMetatileId,
    availableMetatileCount,
    count: 0,
    placements: new Map<string, MetatilePlacement>(),
  }
  const placementKey = `${collision}:${elevation}`
  const placement = entry.placements.get(placementKey) ?? { collision, elevation, count: 0 }
  entry.count += 1
  placement.count += 1
  entry.placements.set(placementKey, placement)
  diagnostics.set(key, entry)
}

const diagnosticList = (diagnostics: ContextUsage["diagnostics"]): MetatileUsageDiagnostic[] => {
  return [...diagnostics.values()]
    .map((entry) => ({
      name: entry.mapName,
      id: entry.mapId,
      layoutId: entry.layoutId,
      code: entry.code,
      source: entry.source,
      globalMetatileId: entry.globalMetatileId,
      localMetatileId: entry.localMetatileId,
      availableMetatileCount: entry.availableMetatileCount,
      count: entry.count,
      placements: [...entry.placements.values()].sort(
        (left, right) => left.collision - right.collision || left.elevation - right.elevation,
      ),
    }))
    .sort(
      (left, right) =>
        left.name.localeCompare(right.name) || left.localMetatileId - right.localMetatileId,
    )
}

const usageForContext = (
  root: string,
  context: ContextDefinition,
  mapsByLayout: ReadonlyMap<string, SourceMapReference[]>,
  primaryMetatileCount: number,
  primaryAvailable: number,
  secondaryAvailable: number,
): ContextUsage => {
  const usage = emptyUsage()
  for (const layout of context.layouts) {
    const blockdataPath = path.join(root, layout.blockdata_filepath)
    const blockdata = fs.readFileSync(blockdataPath)
    const entryCount = layout.width * layout.height
    if (blockdata.length < entryCount * 2) {
      throw new Error(`${blockdataPath}: layout blockdata is shorter than ${entryCount} entries`)
    }
    for (const map of mapsByLayout.get(layout.id) ?? []) {
      for (let offset = 0; offset < entryCount * 2; offset += 2) {
        const word = blockdata.readUInt16LE(offset)
        const globalMetatileId = word & 0x3ff
        const collision = (word >> 10) & 0x03
        const elevation = (word >> 12) & 0x0f
        const side = globalMetatileId >= primaryMetatileCount ? "secondary" : "primary"
        const localMetatileId =
          side === "secondary" ? globalMetatileId - primaryMetatileCount : globalMetatileId
        const available = side === "secondary" ? secondaryAvailable : primaryAvailable
        if (localMetatileId >= available) {
          diagnosticFor(
            usage.diagnostics,
            map,
            side,
            globalMetatileId,
            localMetatileId,
            available,
            collision,
            elevation,
          )
          continue
        }
        usageFor(usage[side], localMetatileId, map, collision, elevation)
      }
    }
  }
  return usage
}

const contexts = (layouts: SourceLayout[]): ContextDefinition[] => {
  const grouped = new Map<string, ContextDefinition>()
  for (const layout of layouts) {
    const id = contextId(layout)
    const current = grouped.get(id) ?? {
      id,
      format: normalizeLayoutFormat(layout),
      primaryTileset: layout.primary_tileset,
      secondaryTileset: layout.secondary_tileset,
      layouts: [],
    }
    current.layouts.push(layout)
    grouped.set(id, current)
  }
  return [...grouped.values()].sort((left, right) => left.id.localeCompare(right.id))
}

const catalogMetatiles = (
  tilesetId: string,
  metatiles: Buffer,
  attributes: Buffer,
  side: "primary" | "secondary",
  primaryTileCount: number,
  attributeSize: 2 | 4,
  behaviors: ReadonlyMap<number, string>,
  usage: SideUsage,
): CatalogMetatile[] => {
  const count = metatiles.length / 16
  return Array.from({ length: count }, (_, id) => ({
    id,
    sourceId: sourceId(tilesetId, id),
    displayName: sourceId(tilesetId, id),
    tiles: describeMetatileTiles(metatiles, id, primaryTileCount),
    attributes: attributeFor(attributes, id, attributeSize, behaviors),
    usedBy: usageList(usage, id),
  }))
}

const contextMaps = (
  context: ContextDefinition,
  mapsByLayout: ReadonlyMap<string, SourceMapReference[]>,
): SourceMapReference[] => {
  return context.layouts
    .flatMap((layout) => mapsByLayout.get(layout.id) ?? [])
    .sort((left, right) => left.name.localeCompare(right.name))
}

const createContext = (
  root: string,
  output: string,
  context: ContextDefinition,
  mapsByLayout: ReadonlyMap<string, SourceMapReference[]>,
  behaviors: ReadonlyMap<number, string>,
  paletteCount: number,
): MetatileContext => {
  const [primaryTileCount, primaryMetatileCount, primaryPaletteCount] = readLayoutFormatCounts(
    context.format,
  )
  const assets = loadRenderAssets(
    root,
    { primary_tileset: context.primaryTileset, secondary_tileset: context.secondaryTileset },
    primaryTileCount,
    primaryPaletteCount,
    paletteCount,
  )
  const primarySource = resolveTilesetAssets(root, context.primaryTileset)
  const secondarySource = resolveTilesetAssets(root, context.secondaryTileset)
  assertMetatileTable(primarySource.metatiles, assets.primaryMetatiles)
  assertMetatileTable(secondarySource.metatiles, assets.secondaryMetatiles)
  const primaryCount = assets.primaryMetatiles.length / 16
  const secondaryCount = assets.secondaryMetatiles.length / 16
  const primaryAttributeSize = assertAttributes(
    primarySource.metatileAttributes,
    assets.primaryMetatileAttributes,
    primaryCount,
  )
  const secondaryAttributeSize = assertAttributes(
    secondarySource.metatileAttributes,
    assets.secondaryMetatileAttributes,
    secondaryCount,
  )
  const usage = usageForContext(
    root,
    context,
    mapsByLayout,
    primaryMetatileCount,
    primaryCount,
    secondaryCount,
  )
  const directory = path.join(output, "contexts", contextDirectory(context))
  const primaryAtlasPath = path.join(directory, "primary.png")
  const secondaryAtlasPath = path.join(directory, "secondary.png")
  const primaryDimensions = renderAtlas(
    primaryAtlasPath,
    context,
    "primary",
    assets.primaryMetatiles,
    assets,
    primaryTileCount,
  )
  const secondaryDimensions = renderAtlas(
    secondaryAtlasPath,
    context,
    "secondary",
    assets.secondaryMetatiles,
    assets,
    primaryTileCount,
  )
  const createTileset = (
    tilesetId: string,
    kind: "primary" | "secondary",
    source: TilesetAssets,
    metatiles: Buffer,
    attributes: Buffer,
    attributeSize: 2 | 4,
    atlasPath: string,
    dimensions: { widthPixels: number; heightPixels: number; unresolvedTileReferences: number },
    metatileIdOffset: number,
  ): ContextTileset => {
    return {
      tilesetId,
      kind,
      metatileIdOffset,
      source: toSource(root, source),
      atlas: {
        path: posixRelative(output, atlasPath),
        sha256: sha256(atlasPath),
        ...dimensions,
        columns: atlasColumns,
        cellPixels: pixelsPerMetatile,
      },
      metatiles: catalogMetatiles(
        tilesetId,
        metatiles,
        attributes,
        kind,
        primaryTileCount,
        attributeSize,
        behaviors,
        usage[kind],
      ),
    }
  }
  return {
    id: context.id,
    format: context.format,
    primaryTileset: context.primaryTileset,
    secondaryTileset: context.secondaryTileset,
    maps: contextMaps(context, mapsByLayout),
    diagnostics: diagnosticList(usage.diagnostics),
    primary: createTileset(
      context.primaryTileset,
      "primary",
      primarySource,
      assets.primaryMetatiles,
      assets.primaryMetatileAttributes,
      primaryAttributeSize,
      primaryAtlasPath,
      primaryDimensions,
      0,
    ),
    secondary: createTileset(
      context.secondaryTileset,
      "secondary",
      secondarySource,
      assets.secondaryMetatiles,
      assets.secondaryMetatileAttributes,
      secondaryAttributeSize,
      secondaryAtlasPath,
      secondaryDimensions,
      primaryMetatileCount,
    ),
  }
}

/** Generate every source-backed metatile in the palette context used by real layouts. */
export const buildMetatileCatalog = (root: string, output: string): MetatileCatalogResult => {
  const layouts = sourceLayouts(root)
  const mapsByLayout = new Map<string, SourceMapReference[]>()
  for (const map of sourceMaps(root)) {
    const maps = mapsByLayout.get(map.layoutId) ?? []
    maps.push(map)
    mapsByLayout.set(map.layoutId, maps)
  }
  const generatedContexts = contexts(layouts.filter((layout) => mapsByLayout.has(layout.id)))
  const catalog: MetatileCatalog = {
    schemaVersion: 1,
    format: "pokemon-wayfarer-metatile-catalog",
    pixelsPerMetatile,
    source: sourceState(root),
    contexts: [],
  }
  const behaviorNames = sourceBehaviorNames(root)
  const paletteCount = readDefine(path.join(root, "include/fieldmap.h"), "NUM_PALS_TOTAL")
  let metatileCount = 0
  for (const contextDefinition of generatedContexts) {
    const context = createContext(
      root,
      output,
      contextDefinition,
      mapsByLayout,
      behaviorNames,
      paletteCount,
    )
    const directory = path.join(output, "contexts", contextDirectory(contextDefinition))
    const contextCatalog: MetatileContextCatalog = {
      schemaVersion: 1,
      format: "pokemon-wayfarer-metatile-context",
      pixelsPerMetatile,
      source: catalog.source,
      context,
    }
    fs.writeFileSync(
      path.join(directory, "catalog.json"),
      `${JSON.stringify(contextCatalog, null, 2)}\n`,
    )
    catalog.contexts.push({
      id: context.id,
      format: context.format,
      primaryTileset: context.primaryTileset,
      secondaryTileset: context.secondaryTileset,
      mapCount: context.maps.length,
      usedMetatileCount: [...context.primary.metatiles, ...context.secondary.metatiles].filter(
        (metatile) => metatile.usedBy.length > 0,
      ).length,
      placementCount: [...context.primary.metatiles, ...context.secondary.metatiles].reduce(
        (total, metatile) =>
          total + metatile.usedBy.reduce((usageTotal, usage) => usageTotal + usage.count, 0),
        0,
      ),
      path: posixRelative(output, path.join(directory, "catalog.json")),
    })
    metatileCount += context.primary.metatiles.length + context.secondary.metatiles.length
  }
  fs.mkdirSync(output, { recursive: true })
  fs.writeFileSync(path.join(output, "catalog.json"), `${JSON.stringify(catalog, null, 2)}\n`)
  return {
    contextCount: catalog.contexts.length,
    metatileCount,
    output,
  }
}
