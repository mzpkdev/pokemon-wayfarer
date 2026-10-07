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

export type CatalogEncounterProduct = "POKEMON_WAYFARER"

export type CatalogEncounterTimeOfDay = "morning" | "day" | "evening" | "night"

export type CatalogEncounterFishingRod = "NONE" | "OLD_ROD" | "GOOD_ROD" | "SUPER_ROD"

export type CatalogEncounterReach = "Road" | "Wilds" | "Outlands" | "Dungeon"

export type CatalogEncounterRegionClass = "Other" | "Safari" | "Sinjoh"

/** Where a wild place sits in the level model (specs/wild-level-scaling.md). */
export type CatalogEncounterPlace = {
  name: string
  region: string
  regionClass: CatalogEncounterRegionClass
  reach: CatalogEncounterReach
  dungeon: { intent: string; flat: boolean } | null
  floor: number | null
  floors: number | null
  /** Place level at each Trainer Rating, from the projection's minimum rating upward. */
  placeLevels: number[]
}

export type CatalogEncounterMethod = {
  type: "land_mons" | "water_mons" | "rock_smash_mons" | "fishing_mons"
  encounterRate: number
  source: CatalogSourcePointer
  slots: CatalogEncounterSlot[]
  /** Slot weights: one profile for the table methods (rod NONE), one per rod for fishing. */
  profiles: Array<{
    profileKey: string
    fishingRod: CatalogEncounterFishingRod
    weights: number[]
  }>
}

export type CatalogEncounterSet = {
  mapId: string
  mapName: string
  baseLabel: string
  product: CatalogEncounterProduct
  /** The v2 table this set holds: Morning and Day use "day", Evening and Night use "night". */
  runtimeTime: "day" | "night"
  /** The Bug Contest weekday of a contest table, otherwise null. */
  variant: string | null
  place: CatalogEncounterPlace
  source: CatalogSourcePointer
  methods: CatalogEncounterMethod[]
}

export type CatalogEncounterRuntimeTime = {
  product: CatalogEncounterProduct
  timeOfDay: CatalogEncounterTimeOfDay
  methods: Array<{
    type: CatalogEncounterMethod["type"]
    resolution: "direct" | "unavailable"
    sets: Array<{
      baseLabel: string
      source: CatalogSourcePointer
    }>
  }>
}

export type CatalogWildEncounters = {
  sets: CatalogEncounterSet[]
  runtimeTimes: CatalogEncounterRuntimeTime[]
}

/** One rolled outcome of a slot: index into the projection's species list, level, weight out of outcomeDenominator. */
export type CatalogEncounterOutcome = [species: number, level: number, weight: number]

export type CatalogWildEncounterProjection = {
  schemaVersion: 3
  trainerRating: { minimum: number; maximum: number }
  outcomeDenominator: number
  products: Array<{ id: CatalogEncounterProduct; displayName: string }>
  species: Array<{
    speciesId: string
    speciesLabel: string
    sprite: CatalogEncounterSprite | null
  }>
  /** Exact outcomes of a slot species at each place level it can reach (hm_model.slot_dist). */
  distributions: Array<{
    speciesId: string
    regionClass: CatalogEncounterRegionClass
    byPlaceLevel: Record<string, CatalogEncounterOutcome[]>
  }>
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

export type SourceMap = {
  id: string
  game_version?: MapSourceVersion
  wayfarer_include?: boolean
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

export type MapSourceVersion = "emerald" | "frlg" | "hns" | "sinnoh"

export type CatalogRegionId = "johto" | "kanto" | "sevii" | "hoenn" | "alola" | "sinnoh"

export type CatalogRegion = {
  id: CatalogRegionId
  label: string
}

export type CatalogBuildId = "emerald" | "firered" | "leafgreen" | "hns" | "wayfarer"

export type CatalogWayfarerMembership = "default" | "include" | "exclude"

export type CatalogBuild = {
  id: CatalogBuildId
  label: string
}

export type CatalogConnection = {
  direction: MapConnection["direction"]
  offsetMetatiles: number
  destinationMapId: string
  destinationMap: string | null
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
  region: CatalogRegionId
  builds: CatalogBuildId[]
  category: string
  sourceGroup: string
  sourceRegion: "sinnoh" | null
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
  connections: CatalogConnection[]
  connectionOverrides?: Partial<Record<CatalogBuildId, CatalogConnection[]>>
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
  schemaVersion: 11
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
  wildEncounterProjection: CatalogWildEncounterProjection
  builds: Array<CatalogBuild & { mapCount: number; maps: string[] }>
  regions: Array<CatalogRegion & { mapCount: number; maps: string[] }>
  maps: CatalogMap[]
}

export type RenderCatalogResult = {
  mapCount: number
  output: string
}
