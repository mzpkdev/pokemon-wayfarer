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
  runtimeMinLevel: number
  runtimeMaxLevel: number
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
  profiles: Array<{
    profileKey: string
    fishingRod: string
    levelOffset: number
  }>
}

export type CatalogWildEncounterSet = {
  mapId: string
  mapName: string
  baseLabel: string
  product: string
  runtimeTime: string
  header: {
    groupLabel: string
    groupIndex: number
    headerIndex: number
  }
  source: CatalogSourcePointer
  methods: CatalogWildEncounterMethod[]
}

export type CatalogWildEncounterRuntimeTime = {
  product: string
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

export type CatalogWildEncounterProjection = {
  schemaVersion: 2
  trainerRating: { minimum: number; maximum: number }
  authoredLevel: { minimum: number; maximum: number }
  products: Array<{ id: string; displayName: string }>
  levelProjections: Array<{
    levelOffset: number
    ratings: Array<{ rating: number; projectedLevels: number[] }>
  }>
  species: Array<{
    authoredSpecies: string
    authoredSpeciesId: number
    speciesLabel: string
    sprite: CatalogEncounterSprite | null
    outcomesByProjectedLevel: Array<{
      minimumProjectedLevel: number
      maximumProjectedLevel: number
      effectiveSpecies: string
      eligible: boolean
      minimumOrdinaryWildLevel: number
    }>
  }>
  profiles: Array<{
    profileKey: string
    product: string
    map: string
    baseLabel: string
    header: string
    headerId: number
    runtimeTime: string
    method: CatalogWildEncounterMethod["type"]
    runtimeArea: string
    fishingRod: string
    runtimeFishingRod: string
    levelOffset: number
    encounterRate: number
    authoredSlotCount: number
    runtimeSlotCount: number
    weights?: number[]
  }>
  headerCounts: Record<string, number>
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
  wildEncounterProjection: CatalogWildEncounterProjection
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
const fishingRods = ["OLD_ROD", "GOOD_ROD", "SUPER_ROD"] as const

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
    hasInteger(slot.runtimeMinLevel) &&
    hasInteger(slot.runtimeMaxLevel) &&
    slot.runtimeMinLevel === Math.min(slot.minLevel as number, slot.maxLevel as number) &&
    slot.runtimeMaxLevel === Math.max(slot.minLevel as number, slot.maxLevel as number) &&
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
    method.slots.every(hasWildEncounterSlot) &&
    Array.isArray(method.profiles) &&
    method.profiles.every((profile) => {
      const record = asRecord(profile)
      return (
        !!record &&
        hasString(record.profileKey) &&
        hasString(record.fishingRod) &&
        hasInteger(record.levelOffset)
      )
    })
  )
}

const hasWildEncounterSet = (value: unknown): value is CatalogWildEncounterSet => {
  const set = asRecord(value)
  return (
    !!set &&
    hasString(set.mapId) &&
    hasString(set.mapName) &&
    hasString(set.baseLabel) &&
    hasString(set.product) &&
    hasString(set.runtimeTime) &&
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
    hasString(time.product) &&
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

const wildEncounterProjectionIssue = (value: unknown): string | null => {
  const projection = asRecord(value)
  if (!projection || projection.schemaVersion !== 2) return "must use projection schemaVersion 2"
  const trainerRating = asRecord(projection?.trainerRating)
  const authoredLevel = asRecord(projection?.authoredLevel)
  if (trainerRating?.minimum !== 10 || trainerRating.maximum !== 80) {
    return "trainerRating must cover 10 through 80"
  }
  if (authoredLevel?.minimum !== 1 || authoredLevel.maximum !== 100) {
    return "authoredLevel must cover 1 through 100"
  }
  if (!Array.isArray(projection.products) || projection.products.length === 0) {
    return "products must be a non-empty array"
  }
  const productIds = new Set<string>()
  for (const product of projection.products) {
    const record = asRecord(product)
    if (!record || !hasString(record.id) || !hasString(record.displayName)) {
      return "products must contain IDs and display names"
    }
    if (productIds.has(record.id)) return `contains duplicate product ${record.id}`
    productIds.add(record.id)
  }

  if (!Array.isArray(projection.levelProjections) || projection.levelProjections.length === 0) {
    return "levelProjections must be a non-empty array"
  }
  const offsets = new Set<number>()
  for (const tableValue of projection.levelProjections) {
    const table = asRecord(tableValue)
    if (!table || !hasInteger(table.levelOffset) || !Array.isArray(table.ratings)) {
      return "levelProjections contains an invalid offset table"
    }
    if (offsets.has(table.levelOffset))
      return `contains duplicate level offset ${table.levelOffset}`
    offsets.add(table.levelOffset)
    if (table.ratings.length !== 71)
      return `level offset ${table.levelOffset} must contain 71 ratings`
    for (const [ratingIndex, ratingValue] of table.ratings.entries()) {
      const row = asRecord(ratingValue)
      const expectedRating = 10 + ratingIndex
      if (
        !row ||
        row.rating !== expectedRating ||
        !Array.isArray(row.projectedLevels) ||
        row.projectedLevels.length !== 100 ||
        !row.projectedLevels.every(
          (level) => hasInteger(level) && (level as number) >= 1 && (level as number) <= 100,
        )
      ) {
        return `level offset ${table.levelOffset} has an invalid rating ${expectedRating} row`
      }
    }
  }
  if (!offsets.has(0)) return "levelProjections must contain offset 0"

  if (!Array.isArray(projection.species) || projection.species.length === 0) {
    return "species must be a non-empty array"
  }
  const speciesIds = new Set<string>()
  for (const speciesValue of projection.species) {
    const species = asRecord(speciesValue)
    if (
      !species ||
      !hasString(species.authoredSpecies) ||
      !hasInteger(species.authoredSpeciesId) ||
      !hasString(species.speciesLabel) ||
      (species.sprite !== null && !hasEncounterSprite(species.sprite)) ||
      !Array.isArray(species.outcomesByProjectedLevel) ||
      species.outcomesByProjectedLevel.length === 0
    ) {
      return "species contains an invalid metadata row"
    }
    if (speciesIds.has(species.authoredSpecies)) {
      return `contains duplicate species ${species.authoredSpecies}`
    }
    speciesIds.add(species.authoredSpecies)
  }
  for (const speciesValue of projection.species) {
    const species = asRecord(speciesValue)!
    let expectedMinimum = 1
    for (const outcomeValue of species.outcomesByProjectedLevel as unknown[]) {
      const outcome = asRecord(outcomeValue)
      if (
        !outcome ||
        outcome.minimumProjectedLevel !== expectedMinimum ||
        !hasInteger(outcome.maximumProjectedLevel) ||
        outcome.maximumProjectedLevel < expectedMinimum ||
        outcome.maximumProjectedLevel > 100 ||
        !hasString(outcome.effectiveSpecies) ||
        !speciesIds.has(outcome.effectiveSpecies) ||
        typeof outcome.eligible !== "boolean" ||
        !hasInteger(outcome.minimumOrdinaryWildLevel) ||
        outcome.minimumOrdinaryWildLevel < 1 ||
        outcome.minimumOrdinaryWildLevel > 100
      ) {
        return `${String(species.authoredSpecies)} has invalid or incomplete outcome intervals`
      }
      expectedMinimum = outcome.maximumProjectedLevel + 1
    }
    if (expectedMinimum !== 101) {
      return `${String(species.authoredSpecies)} outcomes must cover levels 1 through 100`
    }
  }

  if (!Array.isArray(projection.profiles) || projection.profiles.length === 0) {
    return "profiles must be a non-empty array"
  }
  const profileKeys = new Set<string>()
  for (const profileValue of projection.profiles) {
    const profile = asRecord(profileValue)
    if (
      !profile ||
      !hasString(profile.profileKey) ||
      !hasString(profile.product) ||
      !productIds.has(profile.product) ||
      !hasString(profile.map) ||
      !hasString(profile.baseLabel) ||
      !hasString(profile.header) ||
      !hasInteger(profile.headerId) ||
      !hasString(profile.runtimeTime) ||
      !hasWildEncounterType(profile.method) ||
      !hasString(profile.runtimeArea) ||
      !hasString(profile.fishingRod) ||
      !hasString(profile.runtimeFishingRod) ||
      !hasInteger(profile.levelOffset) ||
      !offsets.has(profile.levelOffset) ||
      !hasInteger(profile.encounterRate) ||
      !hasInteger(profile.authoredSlotCount) ||
      !hasInteger(profile.runtimeSlotCount) ||
      profile.profileKey !==
        `${profile.product}/${profile.baseLabel}/${profile.method}/${profile.fishingRod}`
    ) {
      return "profiles contains an invalid runtime profile"
    }
    if (profile.method === "fishing_mons") {
      if (
        !fishingRods.includes(profile.fishingRod as (typeof fishingRods)[number]) ||
        profile.runtimeSlotCount !== 10 ||
        !Array.isArray(profile.weights) ||
        profile.weights.length !== 10 ||
        !profile.weights.every((weight) => hasInteger(weight) && weight > 0) ||
        profile.weights.reduce((sum, weight) => sum + (weight as number), 0) !== 100
      ) {
        return `${profile.profileKey} must contain 10 positive integer weights totaling 100`
      }
    } else if (profile.fishingRod !== "NONE" || "weights" in profile) {
      return `${profile.profileKey} must use NONE and must not contain fishing weights`
    }
    if (profileKeys.has(profile.profileKey))
      return `contains duplicate profile ${profile.profileKey}`
    profileKeys.add(profile.profileKey)
  }
  const headerCounts = asRecord(projection.headerCounts)
  if (!headerCounts || [...productIds].some((product) => !hasInteger(headerCounts[product]))) {
    return "headerCounts must cover every product"
  }
  return null
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
  if (root.schemaVersion !== 8) {
    details.push(
      "schemaVersion must be 8. Regenerate the catalog with pnpm run cartographer:catalog.",
    )
  }
  const projectionIssue = wildEncounterProjectionIssue(root.wildEncounterProjection)
  if (projectionIssue) {
    details.push(`wildEncounterProjection ${projectionIssue}.`)
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
  const projectionProducts = new Set(
    catalog.wildEncounterProjection.products.map((product) => product.id),
  )
  const projectionSpecies = new Set(
    catalog.wildEncounterProjection.species.map((species) => species.authoredSpecies),
  )
  const projectionProfiles = new Map(
    catalog.wildEncounterProjection.profiles.map((profile) => [profile.profileKey, profile]),
  )
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
        if (!projectionProducts.has(set.product)) {
          details.push(
            `${map.name} wildEncounters[${setIndex}] uses unknown product ${set.product}.`,
          )
        }
        for (const method of set.methods) {
          if (method.profiles.length === 0) {
            details.push(`${set.baseLabel} ${method.type} has no projection profiles.`)
          }
          if (method.type === "fishing_mons") {
            const rods = method.profiles.map((profile) => profile.fishingRod).sort()
            const expectedRods = ["GOOD_ROD", "OLD_ROD", "SUPER_ROD"]
            if (
              rods.length !== expectedRods.length ||
              rods.some((rod, index) => rod !== expectedRods[index])
            ) {
              details.push(
                `${set.baseLabel} fishing_mons must reference Old, Good, and Super Rod profiles.`,
              )
            }
            const slotIndices = method.slots.map((slot) => slot.slotIndex).sort((a, b) => a - b)
            if (
              slotIndices.length !== 10 ||
              slotIndices.some((slotIndex, index) => slotIndex !== index)
            ) {
              details.push(`${set.baseLabel} fishing_mons must retain slots 0 through 9.`)
            }
          }
          for (const reference of method.profiles) {
            const profile = projectionProfiles.get(reference.profileKey)
            const expectedRuntimeTime = `TIME_${set.runtimeTime.toUpperCase()}`
            if (
              !profile ||
              profile.product !== set.product ||
              profile.map !== set.mapId ||
              profile.baseLabel !== set.baseLabel ||
              profile.runtimeTime !== expectedRuntimeTime ||
              profile.method !== method.type ||
              profile.fishingRod !== reference.fishingRod ||
              profile.levelOffset !== reference.levelOffset
            ) {
              details.push(
                `${set.baseLabel} ${method.type} has invalid projection profile ${reference.profileKey}.`,
              )
            }
          }
          for (const slot of method.slots) {
            if (!projectionSpecies.has(slot.speciesId)) {
              details.push(
                `${set.baseLabel} ${method.type} slot ${slot.slotIndex} has no species projection for ${slot.speciesId}.`,
              )
            }
          }
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
