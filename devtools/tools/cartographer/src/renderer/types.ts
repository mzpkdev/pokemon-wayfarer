export type LayoutFormat = "emerald" | "frlg" | "hns"

export type Layout = {
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

export type LayoutDocument = {
  layouts: Layout[]
}

export type MapData = {
  layout: string
  map_type: string
}

export type Rgb = readonly [number, number, number]

export type TilesetAssets = {
  tiles: string
  palettes: string
  metatiles: string
  metatileAttributes: string
}

export type IndexedPng = {
  width: number
  height: number
  rows: Uint8Array[]
}

export type RenderAssets = {
  primaryTiles: Uint8Array[]
  secondaryTiles: Uint8Array[]
  primaryMetatiles: Buffer
  secondaryMetatiles: Buffer
  primaryMetatileAttributes: Buffer
  secondaryMetatileAttributes: Buffer
  palettes: Rgb[][]
}

export type EncounterHabitatRectangle = {
  xMetatiles: number
  yMetatiles: number
  widthMetatiles: number
  heightMetatiles: number
}

export type EncounterHabitat = {
  land: EncounterHabitatRectangle[]
  water: EncounterHabitatRectangle[]
}
