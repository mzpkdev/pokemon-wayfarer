import { cartographerUrl } from "../cartographer/urls.js"

export type MetatileAtlas = {
  path: string
  sha256: string
  widthPixels: number
  heightPixels: number
  columns: number
  cellPixels: number
  unresolvedTileReferences: number
}

export type MetatileTile = {
  layer: 0 | 1
  quadrant: 0 | 1 | 2 | 3
  tileId: number
  source: "primary" | "secondary"
  sourceTileId: number
  paletteId: number
  horizontalFlip: boolean
  verticalFlip: boolean
}

export type CatalogMetatile = {
  id: number
  sourceId: string
  displayName: string
  tiles: MetatileTile[]
  attributes: {
    raw: number
    behavior: number
    behaviorName: string | null
    layerType: number
  }
  usedBy: Array<{
    mapName: string
    mapId: string
    layoutId: string
    count: number
    placements: Array<{
      collision: number
      elevation: number
      count: number
    }>
  }>
}

export type MetatileTileset = {
  tilesetId: string
  kind: "primary" | "secondary"
  metatileIdOffset: number
  atlas: MetatileAtlas
  metatiles: CatalogMetatile[]
}

export type MetatileRenderContext = {
  id: string
  format: string
  primaryTileset: string
  secondaryTileset: string
  maps: Array<{
    name: string
    id: string
    layoutId: string
  }>
  primary: MetatileTileset
  secondary: MetatileTileset
}

export type MetatileCatalogContext = {
  id: string
  format: string
  primaryTileset: string
  secondaryTileset: string
  mapCount: number
  usedMetatileCount: number
  placementCount: number
  path: string
}

export type MetatileCatalog = {
  schemaVersion: 1
  format: "pokemon-wayfarer-metatile-catalog"
  pixelsPerMetatile: number
  source: Record<string, unknown>
  contexts: MetatileCatalogContext[]
}

export type MetatileContextCatalog = {
  schemaVersion: 1
  format: "pokemon-wayfarer-metatile-context"
  pixelsPerMetatile: number
  source: Record<string, unknown>
  context: MetatileRenderContext
}

export class MetatileCatalogValidationError extends Error {
  constructor(
    readonly details: readonly string[],
    summary: string,
  ) {
    super(`${summary} ${details.join(" ")}`)
  }
}

const asRecord = (value: unknown): Record<string, unknown> | null => {
  return typeof value === "object" && value !== null && !Array.isArray(value)
    ? (value as Record<string, unknown>)
    : null
}

const hasString = (value: unknown): value is string => typeof value === "string"

const hasInteger = (value: unknown): value is number => {
  return typeof value === "number" && Number.isSafeInteger(value)
}

const hasPositiveInteger = (value: unknown): value is number => hasInteger(value) && value > 0

const hasAtlas = (value: unknown): value is MetatileAtlas => {
  const atlas = asRecord(value)
  return (
    !!atlas &&
    hasString(atlas.path) &&
    hasString(atlas.sha256) &&
    hasPositiveInteger(atlas.widthPixels) &&
    hasPositiveInteger(atlas.heightPixels) &&
    hasPositiveInteger(atlas.columns) &&
    hasPositiveInteger(atlas.cellPixels) &&
    hasInteger(atlas.unresolvedTileReferences) &&
    atlas.unresolvedTileReferences >= 0
  )
}

const hasMetatileTile = (value: unknown): value is MetatileTile => {
  const tile = asRecord(value)
  return (
    !!tile &&
    (tile.layer === 0 || tile.layer === 1) &&
    (tile.quadrant === 0 || tile.quadrant === 1 || tile.quadrant === 2 || tile.quadrant === 3) &&
    hasInteger(tile.tileId) &&
    (tile.source === "primary" || tile.source === "secondary") &&
    hasInteger(tile.sourceTileId) &&
    hasInteger(tile.paletteId) &&
    typeof tile.horizontalFlip === "boolean" &&
    typeof tile.verticalFlip === "boolean"
  )
}

const hasMetatile = (value: unknown): value is CatalogMetatile => {
  const metatile = asRecord(value)
  const attributes = asRecord(metatile?.attributes)
  return (
    !!metatile &&
    hasInteger(metatile.id) &&
    metatile.id >= 0 &&
    hasString(metatile.sourceId) &&
    hasString(metatile.displayName) &&
    Array.isArray(metatile.tiles) &&
    metatile.tiles.every(hasMetatileTile) &&
    !!attributes &&
    hasInteger(attributes.raw) &&
    hasInteger(attributes.behavior) &&
    (attributes.behaviorName === null || hasString(attributes.behaviorName)) &&
    hasInteger(attributes.layerType) &&
    Array.isArray(metatile.usedBy) &&
    metatile.usedBy.every((usage) => {
      const entry = asRecord(usage)
      return (
        !!entry &&
        hasString(entry.mapName) &&
        hasString(entry.mapId) &&
        hasString(entry.layoutId) &&
        hasPositiveInteger(entry.count) &&
        Array.isArray(entry.placements) &&
        entry.placements.every((placement) => {
          const record = asRecord(placement)
          return (
            !!record &&
            hasInteger(record.collision) &&
            hasInteger(record.elevation) &&
            hasPositiveInteger(record.count)
          )
        })
      )
    })
  )
}

const hasTileset = (value: unknown, kind: MetatileTileset["kind"]): value is MetatileTileset => {
  const tileset = asRecord(value)
  return (
    !!tileset &&
    hasString(tileset.tilesetId) &&
    tileset.kind === kind &&
    hasInteger(tileset.metatileIdOffset) &&
    tileset.metatileIdOffset >= 0 &&
    hasAtlas(tileset.atlas) &&
    Array.isArray(tileset.metatiles) &&
    tileset.metatiles.every(hasMetatile)
  )
}

const contextIssue = (value: unknown): string | null => {
  const context = asRecord(value)
  if (
    !context ||
    !hasString(context.id) ||
    !hasString(context.format) ||
    !hasString(context.primaryTileset) ||
    !hasString(context.secondaryTileset) ||
    !Array.isArray(context.maps) ||
    !hasTileset(context.primary, "primary") ||
    !hasTileset(context.secondary, "secondary")
  ) {
    return "must include a format, source tilesets, maps, and primary/secondary tilesets."
  }
  if (
    !context.maps.every((map) => {
      const entry = asRecord(map)
      return !!entry && hasString(entry.name) && hasString(entry.id) && hasString(entry.layoutId)
    })
  ) {
    return "contains an invalid source map reference."
  }
  if (
    context.primary.tilesetId !== context.primaryTileset ||
    context.secondary.tilesetId !== context.secondaryTileset
  ) {
    return "does not agree with its primary or secondary source tileset IDs."
  }
  if (context.primary.metatileIdOffset !== 0) {
    return "must begin primary metatile IDs at offset 0."
  }
  if (context.secondary.metatileIdOffset < context.primary.metatiles.length) {
    return "has a secondary metatile offset that overlaps the primary catalog."
  }
  for (const tileset of [context.primary, context.secondary]) {
    const atlasColumnsWidth = tileset.atlas.columns * tileset.atlas.cellPixels
    if (atlasColumnsWidth > tileset.atlas.widthPixels) {
      return `${tileset.kind} atlas columns exceed its image width.`
    }
    const requiredRows = Math.ceil(tileset.metatiles.length / tileset.atlas.columns)
    if (requiredRows * tileset.atlas.cellPixels > tileset.atlas.heightPixels) {
      return `${tileset.kind} atlas cannot contain all of its metatiles.`
    }
    const ids = new Set<number>()
    const sourceIds = new Set<string>()
    for (const metatile of tileset.metatiles) {
      if (ids.has(metatile.id)) return `${tileset.kind} tileset repeats local ID ${metatile.id}.`
      ids.add(metatile.id)
      if (sourceIds.has(metatile.sourceId)) {
        return `${tileset.kind} tileset repeats scoped ID ${JSON.stringify(metatile.sourceId)}.`
      }
      sourceIds.add(metatile.sourceId)
      const expectedSourceId = `${tileset.tilesetId}:0x${metatile.id
        .toString(16)
        .toUpperCase()
        .padStart(3, "0")}`
      if (metatile.sourceId !== expectedSourceId || metatile.displayName !== expectedSourceId) {
        return `${tileset.kind} metatile ${metatile.id} has an invalid scoped source ID.`
      }
      if (metatile.tiles.length !== 8) {
        return `${tileset.kind} metatile ${metatile.id} must contain eight source tiles.`
      }
      const positions = new Set(metatile.tiles.map((tile) => `${tile.layer}:${tile.quadrant}`))
      if (positions.size !== 8) {
        return `${tileset.kind} metatile ${metatile.id} repeats a layer/quadrant tile.`
      }
    }
  }
  return null
}

const indexContextIssue = (value: unknown): string | null => {
  const context = asRecord(value)
  if (
    !context ||
    !hasString(context.id) ||
    !hasString(context.format) ||
    !hasString(context.primaryTileset) ||
    !hasString(context.secondaryTileset) ||
    !hasInteger(context.mapCount) ||
    context.mapCount < 0 ||
    !hasInteger(context.usedMetatileCount) ||
    context.usedMetatileCount < 0 ||
    !hasInteger(context.placementCount) ||
    context.placementCount < 0 ||
    !hasString(context.path)
  ) {
    return "must include a format, source tilesets, usage counts, and context manifest path."
  }
  if (context.path.startsWith("/") || context.path.split("/").includes("..")) {
    return "uses an unsafe context manifest path."
  }
  return null
}

/** Validate generated source data before the metatile browser renders it. */
export const validateMetatileCatalog = (value: unknown): MetatileCatalog => {
  const root = asRecord(value)
  const details: string[] = []
  if (!root) {
    throw new MetatileCatalogValidationError(
      ["catalog must be an object."],
      "The metatile catalog is invalid.",
    )
  }
  if (root.schemaVersion !== 1) {
    details.push("schemaVersion must be 1. Regenerate with pnpm run metatiles:catalog.")
  }
  if (root.format !== "pokemon-wayfarer-metatile-catalog") {
    details.push("format must be pokemon-wayfarer-metatile-catalog.")
  }
  if (!hasPositiveInteger(root.pixelsPerMetatile)) {
    details.push("pixelsPerMetatile must be a positive integer.")
  }
  if (!asRecord(root.source)) {
    details.push("source must be an object.")
  }
  if (!Array.isArray(root.contexts)) {
    details.push("contexts must be an array.")
  }
  if (details.length > 0) {
    throw new MetatileCatalogValidationError(details, "The metatile catalog is invalid.")
  }

  const catalog = root as unknown as MetatileCatalog
  const contextIds = new Set<string>()
  const contextPaths = new Set<string>()
  for (const [index, context] of catalog.contexts.entries()) {
    if (contextIds.has(context.id)) {
      details.push(`contexts[${index}] repeats ID ${JSON.stringify(context.id)}.`)
    }
    if (contextPaths.has(context.path)) {
      details.push(`contexts[${index}] repeats manifest path ${JSON.stringify(context.path)}.`)
    }
    contextIds.add(context.id)
    contextPaths.add(context.path)
    const issue = indexContextIssue(context)
    if (issue) details.push(`contexts[${index}] ${issue}`)
  }
  if (details.length > 0) {
    throw new MetatileCatalogValidationError(details, "The metatile catalog is inconsistent.")
  }
  return catalog
}

/** Validate a lazily fetched rendering context against the selected catalog index entry. */
export const validateMetatileContextCatalog = (
  value: unknown,
  expected: MetatileCatalogContext,
): MetatileRenderContext => {
  const root = asRecord(value)
  const details: string[] = []
  if (!root) {
    throw new MetatileCatalogValidationError(
      ["context manifest must be an object."],
      "The metatile context is invalid.",
    )
  }
  if (root.schemaVersion !== 1) details.push("schemaVersion must be 1.")
  if (root.format !== "pokemon-wayfarer-metatile-context") {
    details.push("format must be pokemon-wayfarer-metatile-context.")
  }
  if (!hasPositiveInteger(root.pixelsPerMetatile)) {
    details.push("pixelsPerMetatile must be a positive integer.")
  }
  if (!asRecord(root.source)) details.push("source must be an object.")
  const issue = contextIssue(root.context)
  if (issue) details.push(`context ${issue}`)
  if (details.length > 0) {
    throw new MetatileCatalogValidationError(details, "The metatile context is invalid.")
  }

  const context = root.context as MetatileRenderContext
  if (
    context.id !== expected.id ||
    context.format !== expected.format ||
    context.primaryTileset !== expected.primaryTileset ||
    context.secondaryTileset !== expected.secondaryTileset
  ) {
    throw new MetatileCatalogValidationError(
      ["context identity does not match the selected catalog index entry."],
      "The metatile context is inconsistent.",
    )
  }
  return context
}

export const metatileCatalogUrl = (baseUrl?: string): string => {
  return cartographerUrl("metatiles/catalog.json", baseUrl)
}

export const metatileAssetUrl = (path: string, baseUrl?: string): string => {
  return cartographerUrl(`metatiles/${path.replace(/^\/+/, "")}`, baseUrl)
}

export const loadMetatileCatalog = async (signal?: AbortSignal): Promise<MetatileCatalog> => {
  const response = await fetch(metatileCatalogUrl(), { cache: "no-store", signal })
  if (!response.ok) {
    throw new Error(
      `Could not load the metatile catalog (${response.status} ${response.statusText}). Run pnpm run metatiles:catalog first.`,
    )
  }
  return validateMetatileCatalog(await response.json())
}

export const loadMetatileContext = async (
  entry: MetatileCatalogContext,
  signal?: AbortSignal,
): Promise<MetatileRenderContext> => {
  const response = await fetch(metatileAssetUrl(entry.path), { cache: "no-store", signal })
  if (!response.ok) {
    throw new Error(
      `Could not load the ${entry.id} context (${response.status} ${response.statusText}). Regenerate with pnpm run metatiles:catalog.`,
    )
  }
  return validateMetatileContextCatalog(await response.json(), entry)
}

export const metatileScopedLabel = (metatile: CatalogMetatile): string => {
  return metatile.sourceId
}

/** Atlas order follows the source metatile ID, never the UI's filtered or usage-ranked list. */
export const metatileAtlasPosition = (
  metatile: CatalogMetatile,
  tileset: MetatileTileset,
): { column: number; row: number } => ({
  column: metatile.id % tileset.atlas.columns,
  row: Math.floor(metatile.id / tileset.atlas.columns),
})
