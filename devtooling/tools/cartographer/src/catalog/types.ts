export type MapConnection = {
  map: string
  offset: number
  direction: "up" | "down" | "left" | "right" | "dive" | "emerge"
}

export type WarpEvent = {
  x: number
  y: number
  elevation: number
  dest_map: string
  dest_warp_id: string
}

export type ObjectEvent = {
  graphics_id: string
  x: number
  y: number
  elevation: number
  movement_type: string
  movement_range_x: number
  movement_range_y: number
  trainer_type: string
  trainer_sight_or_berry_tree_id: string
  script: string
  flag: string
}

export type CatalogObjectKind = {
  id: string
  label: string
  evidence: "trainer-type" | "graphics" | "script" | "fallback"
  action: string | null
}

export type CatalogSourcePointer = {
  path: string
  pointer: string
}

export type CatalogEncounterSlot = {
  slotIndex: number
  slotRate: number
  slotRateSource: CatalogSourcePointer
  groups: Array<{
    id: string
    source: CatalogSourcePointer
  }>
  minLevel: number
  maxLevel: number
  speciesId: string
  speciesLabel: string
  sprite: CatalogEncounterSprite | null
  source: CatalogSourcePointer
}

export type CatalogEncounterSprite = {
  path: string
  sha256: string
  widthPixels: number
  heightPixels: number
  source: string
}

export type CatalogEncounterMethod = {
  type: "land_mons" | "water_mons" | "rock_smash_mons" | "fishing_mons"
  encounterRate: number
  source: CatalogSourcePointer
  slots: CatalogEncounterSlot[]
}

export type CatalogEncounterSet = {
  mapId: string
  mapName: string
  baseLabel: string
  header: {
    groupLabel: string
    groupIndex: number
    headerIndex: number
  }
  source: CatalogSourcePointer
  methods: CatalogEncounterMethod[]
}

export type CatalogEncounterRuntimeTime = {
  timeOfDay: "morning" | "day" | "evening" | "night"
  methods: Array<{
    type: CatalogEncounterMethod["type"]
    resolution: "direct" | "fallback" | "unavailable"
    sets: Array<{
      baseLabel: string
      source: CatalogSourcePointer
    }>
  }>
}

export type CatalogWildEncounters = {
  sets: CatalogEncounterSet[]
  runtimeTimes: CatalogEncounterRuntimeTime[]
  diagnostics: CatalogEncounterDiagnostic[]
}

export type CatalogEncounterHabitatRectangle = {
  xMetatiles: number
  yMetatiles: number
  widthMetatiles: number
  heightMetatiles: number
}

export type CatalogEncounterHabitat = {
  land: CatalogEncounterHabitatRectangle[]
  water: CatalogEncounterHabitatRectangle[]
}

export type CatalogEncounterDiagnostic =
  | {
      code: "excluded_source_slot"
      reason: "species_none" | "zero_slot_rate"
      setBaseLabel: string
      methodType: CatalogEncounterMethod["type"]
      slotIndex: number
      speciesId: string
      slotRate: number
      source: CatalogSourcePointer
    }
  | {
      code: "unaddressable_source_slot"
      reason: "outside_method_slot_table"
      setBaseLabel: string
      methodType: CatalogEncounterMethod["type"]
      slotIndex: number
      speciesId: string
      minLevel: number
      maxLevel: number
      source: CatalogSourcePointer
    }
  | {
      code: "invalid_source_slot"
      reason: "invalid_level_range"
      setBaseLabel: string
      methodType: CatalogEncounterMethod["type"]
      slotIndex: number
      speciesId: string
      minLevel: number
      maxLevel: number
      source: CatalogSourcePointer
    }

export type SourceMap = {
  id: string
  layout: string
  music?: string
  region_map_section?: string
  requires_flash?: boolean
  weather?: string
  map_type: string
  show_map_name?: boolean
  connections?: MapConnection[]
  warp_events?: WarpEvent[]
  object_events?: ObjectEvent[]
}

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

export type MapGroups = {
  group_order: string[]
  [group: string]: string[]
}

export type CatalogRegion = {
  id: string
  label: string
}

export type CatalogPlacement = {
  x: number
  y: number
  width: number
  height: number
}

export type TopologySourceHeader = {
  map: string
  path: string
  pointer: string
}

export type TopologyConnectionRecord = {
  source: {
    map: string
    mapId: string
    header: TopologySourceHeader
  }
  destination: {
    map: string
    mapId: string
  }
  direction: "up" | "down" | "left" | "right"
  offsetMetatiles: number
}

export type TopologyDirectConnectionMismatch = {
  code: "direct_connection_mismatch"
  explanation: string
  connection: TopologyConnectionRecord
  reverseConnection: TopologyConnectionRecord
  expectedReverse: {
    direction: "up" | "down" | "left" | "right"
    offsetMetatiles: number
  }
  forwardPlacement: CatalogPlacement
  reversePlacement: CatalogPlacement
}

export type TopologyMissingReverseConnection = {
  code: "missing_reverse_connection"
  explanation: string
  connection: TopologyConnectionRecord
  expectedReverse: {
    direction: "up" | "down" | "left" | "right"
    offsetMetatiles: number
  }
}

export type TopologyDiagnostic = TopologyDirectConnectionMismatch | TopologyMissingReverseConnection

export type CatalogMap = {
  name: string
  id: string
  region: string
  category: string
  sourceGroup: string
  sourceRegion: null
  mapType: string
  mapSection: string | null
  image: {
    path: string
    sha256: string
    widthPixels: number
    heightPixels: number
    overview: {
      path: string
      sha256: string
      widthPixels: number
      heightPixels: number
    }
  }
  layout: {
    id: string
    format: string
    widthMetatiles: number
    heightMetatiles: number
    primaryTileset: string
    secondaryTileset: string
  }
  world: {
    layer: "surface" | "underwater"
    defaultVisible: boolean
    variantGroup: null
    variant: null
  }
  presentation: {
    music: string | null
    weather: string | null
    showMapName: boolean | null
    requiresFlash: boolean | null
  }
  connections: Array<{
    direction: MapConnection["direction"]
    offsetMetatiles: number
    destinationMapId: string
    destinationMap: string | null
  }>
  warps: Array<{
    warpId: string
    xMetatiles: number
    yMetatiles: number
    elevation: number
    destinationWarpId: string
    destinationMapId: string
    destinationMap: string | null
  }>
  objects: Array<{
    objectId: string
    kind: CatalogObjectKind
    graphicsId: string
    isShiny: boolean
    xMetatiles: number
    yMetatiles: number
    elevation: number
    movementType: string
    movementRange: { x: number; y: number }
    trainerType: string
    trainerSightOrBerryTreeId: string
    script: string
    flag: string
    sprite: {
      path: string
      sha256: string
      widthPixels: number
      heightPixels: number
      anchor: { xPixels: number; yPixels: number }
      source: string
    } | null
    diagnostic: { code: string; message: string } | null
  }>
  wildEncounters: CatalogWildEncounters
  encounterHabitat: CatalogEncounterHabitat
}

export type MapCatalog = {
  schemaVersion: 7
  format: "pokemon-wayfarer-exterior-map-catalog"
  pixelsPerMetatile: 16
  source: {
    revision: string
    workingTreeDirty: boolean
  }
  diagnostics: Array<{
    map: string
    objectId: string
    graphicsId: string
    code: string
    message: string
  }>
  topology: {
    conflicts: TopologyDiagnostic[]
  }
  regions: Array<CatalogRegion & { mapCount: number; maps: string[] }>
  maps: CatalogMap[]
}

export type RenderCatalogResult = {
  mapCount: number
  output: string
}
