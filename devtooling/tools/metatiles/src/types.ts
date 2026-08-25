export type SourceState = {
  revision: string
  workingTreeDirty: boolean
}

export type SourceLayout = {
  id: string
  width: number
  height: number
  format?: string
  layout_version?: string
  game_version?: string
  primary_tileset: string
  secondary_tileset: string
  blockdata_filepath: string
}

export type SourceMap = {
  id: string
  layout: string
}

export type SourceMapReference = {
  name: string
  id: string
  layoutId: string
}

export type MetatilePlacement = {
  collision: number
  elevation: number
  count: number
}

export type MetatileUsage = {
  mapName: string
  mapId: string
  layoutId: string
  count: number
  placements: MetatilePlacement[]
}

export type MetatileUsageDiagnostic = SourceMapReference & {
  code: "missing_source_metatile"
  source: "primary" | "secondary"
  globalMetatileId: number
  localMetatileId: number
  availableMetatileCount: number
  count: number
  placements: MetatilePlacement[]
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

export type MetatileAttributes = {
  raw: number
  behavior: number
  behaviorName: string | null
  layerType: number
}

export type CatalogMetatile = {
  id: number
  sourceId: string
  displayName: string
  tiles: MetatileTile[]
  attributes: MetatileAttributes
  usedBy: MetatileUsage[]
}

export type MetatileAtlas = {
  path: string
  sha256: string
  widthPixels: number
  heightPixels: number
  columns: number
  cellPixels: number
  unresolvedTileReferences: number
}

export type TilesetSource = {
  tiles: string
  palettes: string
  metatiles: string
  metatileAttributes: string
}

export type ContextTileset = {
  tilesetId: string
  kind: "primary" | "secondary"
  metatileIdOffset: number
  source: TilesetSource
  atlas: MetatileAtlas
  metatiles: CatalogMetatile[]
}

export type MetatileContext = {
  id: string
  format: string
  primaryTileset: string
  secondaryTileset: string
  maps: SourceMapReference[]
  diagnostics: MetatileUsageDiagnostic[]
  primary: ContextTileset
  secondary: ContextTileset
}

export type MetatileCatalog = {
  schemaVersion: 1
  format: "pokemon-wayfarer-metatile-catalog"
  pixelsPerMetatile: 16
  source: SourceState
  contexts: MetatileContextIndex[]
}

export type MetatileContextIndex = {
  id: string
  format: string
  primaryTileset: string
  secondaryTileset: string
  mapCount: number
  usedMetatileCount: number
  placementCount: number
  path: string
}

export type MetatileContextCatalog = {
  schemaVersion: 1
  format: "pokemon-wayfarer-metatile-context"
  pixelsPerMetatile: 16
  source: SourceState
  context: MetatileContext
}

export type MetatileCatalogResult = {
  contextCount: number
  metatileCount: number
  output: string
}
