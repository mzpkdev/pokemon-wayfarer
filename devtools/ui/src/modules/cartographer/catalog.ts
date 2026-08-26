import { catalogUrl } from "./urls.js"

export type CatalogConnection = {
  direction: "up" | "down" | "left" | "right" | "dive" | "emerge"
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

export type CatalogTopologyHeader = {
  map: string
  path: string
  pointer: string
}

export type CatalogTopologyConnection = {
  source: {
    map: string
    mapId: string
    header: CatalogTopologyHeader
  }
  destination: {
    map: string
    mapId: string
  }
  direction: "up" | "down" | "left" | "right"
  offsetMetatiles: number
}

export type CatalogDirectTopologyMismatch = {
  code: "direct_connection_mismatch"
  explanation: string
  connection: CatalogTopologyConnection
  reverseConnection: CatalogTopologyConnection
  expectedReverse: {
    direction: "up" | "down" | "left" | "right"
    offsetMetatiles: number
  }
  forwardPlacement: CatalogPlacement
  reversePlacement: CatalogPlacement
}

export type CatalogMissingReverseConnection = {
  code: "missing_reverse_connection"
  explanation: string
  connection: CatalogTopologyConnection
  expectedReverse: {
    direction: "up" | "down" | "left" | "right"
    offsetMetatiles: number
  }
}

export type CatalogTopologyDiagnostic =
  | CatalogDirectTopologyMismatch
  | CatalogMissingReverseConnection

export type CatalogWarp = {
  warpId: string
  xMetatiles: number
  yMetatiles: number
  elevation: number
  destinationWarpId: string
  destinationMapId: string
  destinationMap: string | null
}

export type CatalogObjectSprite = {
  path: string
  sha256: string
  widthPixels: number
  heightPixels: number
  anchor: {
    xPixels: number
    yPixels: number
  }
  source: string
}

export type CatalogObject = {
  objectId: string
  kind: {
    id: string
    label: string
    evidence: "trainer-type" | "graphics" | "script" | "fallback"
    action: string | null
  }
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
  sprite: CatalogObjectSprite | null
  diagnostic: { code: string; message: string } | null
}

export type CatalogSourcePointer = {
  path: string
  pointer: string
}

export type CatalogWildEncounterSlot = {
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
  speciesLabel?: string
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

export type CatalogWildEncounterMethod = {
  type: "land_mons" | "water_mons" | "rock_smash_mons" | "fishing_mons"
  encounterRate: number
  source: CatalogSourcePointer
  slots: CatalogWildEncounterSlot[]
}

export type CatalogWildEncounterSet = {
  mapId: string
  mapName: string
  baseLabel: string
  header: {
    groupLabel: string
    groupIndex: number
    headerIndex: number
  }
  source: CatalogSourcePointer
  methods: CatalogWildEncounterMethod[]
}

export type CatalogWildEncounterRuntimeTime = {
  timeOfDay: "morning" | "day" | "evening" | "night"
  methods: Array<{
    type: CatalogWildEncounterMethod["type"]
    resolution: "direct" | "fallback" | "unavailable"
    sets: Array<{
      baseLabel: string
      source: CatalogSourcePointer
    }>
  }>
}

export type CatalogWildEncounters = {
  sets: CatalogWildEncounterSet[]
  runtimeTimes: CatalogWildEncounterRuntimeTime[]
  diagnostics: Array<
    | {
        code: "excluded_source_slot"
        reason: "species_none" | "zero_slot_rate"
        setBaseLabel: string
        methodType: CatalogWildEncounterMethod["type"]
        slotIndex: number
        speciesId: string
        slotRate: number
        source: CatalogSourcePointer
      }
    | {
        code: "unaddressable_source_slot"
        reason: "outside_method_slot_table"
        setBaseLabel: string
        methodType: CatalogWildEncounterMethod["type"]
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
        methodType: CatalogWildEncounterMethod["type"]
        slotIndex: number
        speciesId: string
        minLevel: number
        maxLevel: number
        source: CatalogSourcePointer
      }
  >
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

export type CatalogMap = {
  name: string
  id: string
  region: string
  category: string
  sourceGroup: string
  sourceRegion: string | null
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
    layer: "surface" | "underwater" | "generated"
    defaultVisible: boolean
    variantGroup: string | null
    variant: string | null
  }
  presentation: {
    music: string | null
    weather: string | null
    showMapName: boolean | null
    requiresFlash: boolean | null
  }
  connections: CatalogConnection[]
  warps: CatalogWarp[]
  objects: CatalogObject[]
  wildEncounters: CatalogWildEncounters
  encounterHabitat: CatalogEncounterHabitat
}

export type MapCatalog = {
  $schema: string
  schemaVersion: number
  format: string
  pixelsPerMetatile: number
  source: {
    revision: string
    workingTreeDirty: boolean
  }
  topology: {
    conflicts: CatalogTopologyDiagnostic[]
  }
  regions: Array<{
    id: string
    label: string
    mapCount: number
    maps: string[]
  }>
  maps: CatalogMap[]
}

export class CatalogValidationError extends Error {
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

const hasString = (value: unknown): value is string => {
  return typeof value === "string"
}

const hasNumber = (value: unknown): value is number => {
  return typeof value === "number" && Number.isFinite(value)
}

const hasInteger = (value: unknown): value is number => {
  return hasNumber(value) && Number.isInteger(value)
}

const hasSourcePointer = (value: unknown): value is CatalogSourcePointer => {
  const pointer = asRecord(value)
  return !!pointer && hasString(pointer.path) && hasString(pointer.pointer)
}

const wildEncounterTypes = ["land_mons", "water_mons", "rock_smash_mons", "fishing_mons"] as const

const hasWildEncounterType = (value: unknown): value is CatalogWildEncounterMethod["type"] => {
  return (
    typeof value === "string" &&
    wildEncounterTypes.includes(value as CatalogWildEncounterMethod["type"])
  )
}

const hasWildEncounterSlot = (value: unknown): value is CatalogWildEncounterSlot => {
  const slot = asRecord(value)
  return (
    !!slot &&
    hasInteger(slot.slotIndex) &&
    hasNumber(slot.slotRate) &&
    hasSourcePointer(slot.slotRateSource) &&
    Array.isArray(slot.groups) &&
    slot.groups.every((group) => {
      const record = asRecord(group)
      return !!record && hasString(record.id) && hasSourcePointer(record.source)
    }) &&
    hasInteger(slot.minLevel) &&
    hasInteger(slot.maxLevel) &&
    slot.minLevel <= slot.maxLevel &&
    hasString(slot.speciesId) &&
    (slot.speciesLabel === undefined || hasString(slot.speciesLabel)) &&
    (slot.sprite === null || hasEncounterSprite(slot.sprite)) &&
    hasSourcePointer(slot.source)
  )
}

const hasEncounterSprite = (value: unknown): value is CatalogEncounterSprite => {
  const sprite = asRecord(value)
  return (
    !!sprite &&
    hasString(sprite.path) &&
    hasString(sprite.sha256) &&
    hasInteger(sprite.widthPixels) &&
    hasInteger(sprite.heightPixels) &&
    hasString(sprite.source)
  )
}

const hasWildEncounterMethod = (value: unknown): value is CatalogWildEncounterMethod => {
  const method = asRecord(value)
  return (
    !!method &&
    hasWildEncounterType(method.type) &&
    hasNumber(method.encounterRate) &&
    hasSourcePointer(method.source) &&
    Array.isArray(method.slots) &&
    method.slots.every(hasWildEncounterSlot)
  )
}

const hasWildEncounterSet = (value: unknown): value is CatalogWildEncounterSet => {
  const set = asRecord(value)
  return (
    !!set &&
    hasString(set.mapId) &&
    hasString(set.mapName) &&
    hasString(set.baseLabel) &&
    !!asRecord(set.header) &&
    hasString(asRecord(set.header)?.groupLabel) &&
    hasInteger(asRecord(set.header)?.groupIndex) &&
    hasInteger(asRecord(set.header)?.headerIndex) &&
    hasSourcePointer(set.source) &&
    Array.isArray(set.methods) &&
    set.methods.every(hasWildEncounterMethod)
  )
}

const wildEncounterTimeIds = ["morning", "day", "evening", "night"] as const

const hasWildEncounterRuntimeTime = (value: unknown): value is CatalogWildEncounterRuntimeTime => {
  const time = asRecord(value)
  return (
    !!time &&
    typeof time.timeOfDay === "string" &&
    wildEncounterTimeIds.includes(time.timeOfDay as CatalogWildEncounterRuntimeTime["timeOfDay"]) &&
    Array.isArray(time.methods) &&
    time.methods.every((method) => {
      const record = asRecord(method)
      return (
        !!record &&
        hasWildEncounterType(record.type) &&
        (record.resolution === "direct" ||
          record.resolution === "fallback" ||
          record.resolution === "unavailable") &&
        Array.isArray(record.sets) &&
        record.sets.every((set) => {
          const setRecord = asRecord(set)
          return !!setRecord && hasString(setRecord.baseLabel) && hasSourcePointer(setRecord.source)
        }) &&
        (record.resolution === "unavailable" ? record.sets.length === 0 : record.sets.length > 0)
      )
    })
  )
}

const hasWildEncounterDiagnostics = (value: unknown): boolean => {
  return (
    Array.isArray(value) &&
    value.every((diagnostic) => {
      const record = asRecord(diagnostic)
      const hasCommonSourceSlotFields =
        !!record &&
        hasString(record.setBaseLabel) &&
        hasWildEncounterType(record.methodType) &&
        hasInteger(record.slotIndex) &&
        hasString(record.speciesId) &&
        hasSourcePointer(record.source)
      return (
        (record?.code === "excluded_source_slot" &&
          (record.reason === "species_none" || record.reason === "zero_slot_rate") &&
          hasNumber(record.slotRate) &&
          hasCommonSourceSlotFields) ||
        ((record?.code === "unaddressable_source_slot" || record?.code === "invalid_source_slot") &&
          (record.reason === "outside_method_slot_table" ||
            record.reason === "invalid_level_range") &&
          hasInteger(record.minLevel) &&
          hasInteger(record.maxLevel) &&
          hasCommonSourceSlotFields)
      )
    })
  )
}

const hasWildEncounters = (value: unknown): value is CatalogWildEncounters => {
  const encounters = asRecord(value)
  return (
    !!encounters &&
    Array.isArray(encounters.sets) &&
    encounters.sets.every(hasWildEncounterSet) &&
    Array.isArray(encounters.runtimeTimes) &&
    encounters.runtimeTimes.every(hasWildEncounterRuntimeTime) &&
    hasWildEncounterDiagnostics(encounters.diagnostics)
  )
}

const hasEncounterHabitatRectangle = (
  value: unknown,
): value is CatalogEncounterHabitatRectangle => {
  const rectangle = asRecord(value)
  return (
    !!rectangle &&
    hasInteger(rectangle.xMetatiles) &&
    hasInteger(rectangle.yMetatiles) &&
    hasInteger(rectangle.widthMetatiles) &&
    hasInteger(rectangle.heightMetatiles) &&
    rectangle.widthMetatiles > 0 &&
    rectangle.heightMetatiles > 0
  )
}

const hasEncounterHabitat = (value: unknown): value is CatalogEncounterHabitat => {
  const habitat = asRecord(value)
  return (
    !!habitat &&
    Array.isArray(habitat.land) &&
    habitat.land.every(hasEncounterHabitatRectangle) &&
    Array.isArray(habitat.water) &&
    habitat.water.every(hasEncounterHabitatRectangle)
  )
}

const hasCatalogObject = (value: unknown): value is CatalogObject => {
  const object = asRecord(value)
  return !!object && typeof object.isShiny === "boolean"
}

const hasCardinalDirection = (value: unknown): boolean => {
  return value === "up" || value === "down" || value === "left" || value === "right"
}

const hasTopologyHeader = (value: unknown): boolean => {
  const header = asRecord(value)
  return !!header && hasString(header.map) && hasString(header.path) && hasString(header.pointer)
}

const hasTopologyConnection = (value: unknown): boolean => {
  const connection = asRecord(value)
  const source = asRecord(connection?.source)
  const destination = asRecord(connection?.destination)
  return (
    !!connection &&
    !!source &&
    !!destination &&
    hasString(source.map) &&
    hasString(source.mapId) &&
    hasTopologyHeader(source.header) &&
    hasString(destination.map) &&
    hasString(destination.mapId) &&
    hasCardinalDirection(connection.direction) &&
    hasNumber(connection.offsetMetatiles)
  )
}

const hasPlacement = (value: unknown): boolean => {
  const placement = asRecord(value)
  return (
    !!placement &&
    hasNumber(placement.x) &&
    hasNumber(placement.y) &&
    hasNumber(placement.width) &&
    hasNumber(placement.height)
  )
}

const hasExpectedReverse = (value: unknown): boolean => {
  const expected = asRecord(value)
  return (
    !!expected && hasCardinalDirection(expected.direction) && hasNumber(expected.offsetMetatiles)
  )
}

const topologyDiagnosticIssue = (value: unknown): string | null => {
  const diagnostic = asRecord(value)
  if (!diagnostic || !hasString(diagnostic.code) || !hasString(diagnostic.explanation)) {
    return "must include a supported code and explanation."
  }
  if (diagnostic.code === "direct_connection_mismatch") {
    return hasTopologyConnection(diagnostic.connection) &&
      hasTopologyConnection(diagnostic.reverseConnection) &&
      hasExpectedReverse(diagnostic.expectedReverse) &&
      hasPlacement(diagnostic.forwardPlacement) &&
      hasPlacement(diagnostic.reversePlacement)
      ? null
      : "has an invalid direct reciprocal mismatch payload."
  }
  if (diagnostic.code === "missing_reverse_connection") {
    return hasTopologyConnection(diagnostic.connection) &&
      hasExpectedReverse(diagnostic.expectedReverse)
      ? null
      : "has an invalid missing reverse connection payload."
  }
  return `uses unsupported code ${JSON.stringify(diagnostic.code)}.`
}

/** Check the catalog fields the cartographer relies upon before rendering any map data. */
export const validateCatalog = (value: unknown): MapCatalog => {
  const root = asRecord(value)
  const details: string[] = []
  if (!root) {
    throw new CatalogValidationError(["catalog must be an object."], "The map catalog is invalid.")
  }
  if (root.schemaVersion !== 7) {
    details.push(
      "schemaVersion must be 7. Regenerate the catalog with pnpm run cartographer:catalog.",
    )
  }
  if (!Array.isArray(root.maps)) {
    details.push("maps must be an array.")
  }
  if (!Array.isArray(root.regions)) {
    details.push("regions must be an array.")
  }
  if (!asRecord(root.topology) || !Array.isArray(asRecord(root.topology)?.conflicts)) {
    details.push("topology.conflicts must be an array.")
  }
  if (typeof root.pixelsPerMetatile !== "number" || root.pixelsPerMetatile < 1) {
    details.push("pixelsPerMetatile must be a positive number.")
  }
  if (details.length > 0) {
    throw new CatalogValidationError(details, "The map catalog is invalid.")
  }

  const catalog = root as unknown as MapCatalog
  for (const [index, diagnostic] of catalog.topology.conflicts.entries()) {
    const issue = topologyDiagnosticIssue(diagnostic)
    if (issue) details.push(`topology.conflicts[${index}] ${issue}`)
  }
  const mapNames = new Set<string>()
  const mapIds = new Set<string>()
  const regions = new Set(catalog.regions.map((region) => region.id))
  for (const map of catalog.maps) {
    if (!hasString(map.name) || !hasString(map.id) || !hasString(map.region)) {
      details.push("every map needs a name, id, and region.")
      continue
    }
    if (mapNames.has(map.name)) {
      details.push(`duplicate map name ${JSON.stringify(map.name)}.`)
    }
    if (mapIds.has(map.id)) {
      details.push(`duplicate map id ${JSON.stringify(map.id)}.`)
    }
    if (!regions.has(map.region)) {
      details.push(`${map.name} refers to undeclared region ${JSON.stringify(map.region)}.`)
    }
    if (!hasWildEncounters(map.wildEncounters)) {
      details.push(`${map.name} wildEncounters must contain valid source encounter data.`)
    } else {
      for (const [setIndex, set] of map.wildEncounters.sets.entries()) {
        if (!hasWildEncounterSet(set)) {
          details.push(
            `${map.name} wildEncounters[${setIndex}] has an invalid source encounter set.`,
          )
          continue
        }
        if (set.mapId !== map.id || set.mapName !== map.name) {
          details.push(`${map.name} wildEncounters[${setIndex}] belongs to a different map.`)
        }
      }
    }
    if (!hasEncounterHabitat(map.encounterHabitat)) {
      details.push(`${map.name} encounterHabitat must contain valid source tile geometry.`)
    }
    if (!Array.isArray(map.objects) || !map.objects.every(hasCatalogObject)) {
      details.push(`${map.name} objects must contain an explicit shiny-state flag.`)
    }
    if (map.image.widthPixels !== map.layout.widthMetatiles * catalog.pixelsPerMetatile) {
      details.push(`${map.name} has an inconsistent image width.`)
    }
    if (map.image.heightPixels !== map.layout.heightMetatiles * catalog.pixelsPerMetatile) {
      details.push(`${map.name} has an inconsistent image height.`)
    }
    mapNames.add(map.name)
    mapIds.add(map.id)
  }
  if (details.length > 0) {
    throw new CatalogValidationError(details, "The map catalog is inconsistent.")
  }
  return catalog
}

export const loadCatalog = async (signal?: AbortSignal): Promise<MapCatalog> => {
  const response = await fetch(catalogUrl(), { cache: "no-store", signal })
  if (!response.ok) {
    throw new Error(
      `Could not load the map catalog (${response.status} ${response.statusText}). Run pnpm run cartographer:catalog first.`,
    )
  }
  return validateCatalog(await response.json())
}
