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

export type CatalogWildEncounterPlace = {
  name: string
  region: string
  regionClass: "Other" | "Safari" | "Sinjoh"
  reach: "Road" | "Wilds" | "Outlands" | "Dungeon"
  dungeon: { intent: string; flat: boolean } | null
  floor: number | null
  floors: number | null
  /** Place level at each Trainer Rating, from the projection's minimum rating upward. */
  placeLevels: number[]
}

export type CatalogWildEncounterMethod = {
  type: "land_mons" | "water_mons" | "rock_smash_mons" | "fishing_mons"
  encounterRate: number
  source: CatalogSourcePointer
  slots: CatalogWildEncounterSlot[]
  /** Slot weights: one profile for the table methods (rod NONE), one per rod for fishing. */
  profiles: Array<{
    profileKey: string
    fishingRod: string
    weights: number[]
  }>
}

export type CatalogWildEncounterSet = {
  mapId: string
  mapName: string
  baseLabel: string
  product: string
  /** The v2 table this set holds: Morning and Day use "day", Evening and Night use "night". */
  runtimeTime: "day" | "night"
  /** The Bug Contest weekday of a contest table, otherwise null. */
  variant: string | null
  place: CatalogWildEncounterPlace
  source: CatalogSourcePointer
  methods: CatalogWildEncounterMethod[]
}

export type CatalogWildEncounterRuntimeTime = {
  product: string
  timeOfDay: "morning" | "day" | "evening" | "night"
  methods: Array<{
    type: CatalogWildEncounterMethod["type"]
    resolution: "direct" | "unavailable"
    sets: Array<{
      baseLabel: string
      source: CatalogSourcePointer
    }>
  }>
}

/** One rolled outcome of a slot: index into the projection's species list, level, weight out of outcomeDenominator. */
export type CatalogWildEncounterOutcome = [species: number, level: number, weight: number]

export type CatalogWildEncounterProjection = {
  schemaVersion: 3
  trainerRating: { minimum: number; maximum: number }
  outcomeDenominator: number
  products: Array<{ id: string; displayName: string }>
  species: Array<{
    speciesId: string
    speciesLabel: string
    sprite: CatalogEncounterSprite | null
  }>
  /** Exact outcomes of a slot species at each place level it can reach. */
  distributions: Array<{
    speciesId: string
    regionClass: CatalogWildEncounterPlace["regionClass"]
    byPlaceLevel: Record<string, CatalogWildEncounterOutcome[]>
  }>
}

export type CatalogWildEncounters = {
  sets: CatalogWildEncounterSet[]
  runtimeTimes: CatalogWildEncounterRuntimeTime[]
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
  builds: string[]
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
  connectionOverrides?: Record<string, CatalogConnection[]>
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
  builds: Array<{
    id: string
    label: string
    mapCount: number
    maps: string[]
  }>
  regions: Array<{
    id: string
    label: string
    mapCount: number
    maps: string[]
  }>
  maps: CatalogMap[]
}

export const mapsForBuild = (catalog: MapCatalog, buildId: string): CatalogMap[] =>
  catalog.maps
    .filter((map) => map.builds.includes(buildId))
    .map((map) => {
      const connections = map.connectionOverrides?.[buildId]
      return connections ? { ...map, connections } : map
    })

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

const connectionDirections = new Set(["up", "down", "left", "right", "dive", "emerge"])

const hasCatalogConnection = (value: unknown): value is CatalogConnection => {
  const connection = asRecord(value)
  return (
    !!connection &&
    hasString(connection.direction) &&
    connectionDirections.has(connection.direction) &&
    hasNumber(connection.offsetMetatiles) &&
    hasString(connection.destinationMapId) &&
    (connection.destinationMap === null || hasString(connection.destinationMap))
  )
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

const hasWildEncounterSlot = (value: unknown): value is CatalogWildEncounterSlot => {
  const slot = asRecord(value)
  return (
    !!slot &&
    hasInteger(slot.slotIndex) &&
    hasString(slot.speciesId) &&
    hasString(slot.speciesLabel) &&
    (slot.sprite === null || hasEncounterSprite(slot.sprite)) &&
    hasSourcePointer(slot.source)
  )
}

const wildEncounterReaches = ["Road", "Wilds", "Outlands", "Dungeon"]
const wildEncounterRegionClasses = ["Other", "Safari", "Sinjoh"]

const hasWildEncounterPlace = (value: unknown): value is CatalogWildEncounterPlace => {
  const place = asRecord(value)
  const dungeon = asRecord(place?.dungeon)
  return (
    !!place &&
    hasString(place.name) &&
    hasString(place.region) &&
    wildEncounterRegionClasses.includes(place.regionClass as string) &&
    wildEncounterReaches.includes(place.reach as string) &&
    (place.reach === "Dungeon"
      ? !!dungeon && hasString(dungeon.intent) && typeof dungeon.flat === "boolean"
      : place.dungeon === null) &&
    (place.floor === null || hasInteger(place.floor)) &&
    (place.floors === null || hasInteger(place.floors)) &&
    Array.isArray(place.placeLevels) &&
    place.placeLevels.every((level) => hasInteger(level) && level >= 1 && level <= 100)
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
        Array.isArray(record.weights) &&
        record.weights.every((weight) => hasInteger(weight) && weight > 0)
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
    (set.runtimeTime === "day" || set.runtimeTime === "night") &&
    (set.variant === null || hasString(set.variant)) &&
    hasWildEncounterPlace(set.place) &&
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
        (record.resolution === "direct" || record.resolution === "unavailable") &&
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
  if (!projection || projection.schemaVersion !== 3) return "must use projection schemaVersion 3"
  const trainerRating = asRecord(projection.trainerRating)
  if (
    !trainerRating ||
    !hasInteger(trainerRating.minimum) ||
    !hasInteger(trainerRating.maximum) ||
    trainerRating.minimum !== 0 ||
    trainerRating.maximum < trainerRating.minimum
  ) {
    return "trainerRating must start at 0"
  }
  if (!hasInteger(projection.outcomeDenominator) || projection.outcomeDenominator < 1) {
    return "outcomeDenominator must be a positive integer"
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

  if (!Array.isArray(projection.species) || projection.species.length === 0) {
    return "species must be a non-empty array"
  }
  const speciesIds = new Set<string>()
  for (const speciesValue of projection.species) {
    const species = asRecord(speciesValue)
    if (
      !species ||
      !hasString(species.speciesId) ||
      !hasString(species.speciesLabel) ||
      (species.sprite !== null && !hasEncounterSprite(species.sprite))
    ) {
      return "species contains an invalid metadata row"
    }
    if (speciesIds.has(species.speciesId)) return `contains duplicate species ${species.speciesId}`
    speciesIds.add(species.speciesId)
  }

  if (!Array.isArray(projection.distributions) || projection.distributions.length === 0) {
    return "distributions must be a non-empty array"
  }
  const distributionKeys = new Set<string>()
  for (const distributionValue of projection.distributions) {
    const distribution = asRecord(distributionValue)
    const byPlaceLevel = asRecord(distribution?.byPlaceLevel)
    if (
      !distribution ||
      !byPlaceLevel ||
      !hasString(distribution.speciesId) ||
      !speciesIds.has(distribution.speciesId) ||
      !wildEncounterRegionClasses.includes(distribution.regionClass as string)
    ) {
      return "distributions contains an invalid row"
    }
    const key = `${distribution.speciesId}/${distribution.regionClass}`
    if (distributionKeys.has(key)) return `contains duplicate distribution ${key}`
    distributionKeys.add(key)
    for (const [level, outcomes] of Object.entries(byPlaceLevel)) {
      const weight = Array.isArray(outcomes)
        ? outcomes.reduce<number>(
            (sum, outcome) =>
              Array.isArray(outcome) &&
              hasInteger(outcome[0]) &&
              outcome[0] >= 0 &&
              outcome[0] < (projection.species as unknown[]).length &&
              hasInteger(outcome[1]) &&
              outcome[1] >= 1 &&
              outcome[1] <= 100 &&
              hasInteger(outcome[2]) &&
              outcome[2] > 0
                ? sum + outcome[2]
                : Number.NaN,
            0,
          )
        : Number.NaN
      if (!/^\d+$/.test(level) || weight !== projection.outcomeDenominator) {
        return `${key} has an invalid outcome list at place level ${level}`
      }
    }
  }
  return null
}

const hasWildEncounters = (value: unknown): value is CatalogWildEncounters => {
  const encounters = asRecord(value)
  return (
    !!encounters &&
    Array.isArray(encounters.sets) &&
    encounters.sets.every(hasWildEncounterSet) &&
    Array.isArray(encounters.runtimeTimes) &&
    encounters.runtimeTimes.every(hasWildEncounterRuntimeTime)
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
  if (root.schemaVersion !== 11) {
    details.push(
      "schemaVersion must be 11. Regenerate the catalog with pnpm run cartographer:catalog.",
    )
  }
  const projectionIssue = wildEncounterProjectionIssue(root.wildEncounterProjection)
  if (projectionIssue) {
    details.push(`wildEncounterProjection ${projectionIssue}.`)
  }
  if (!Array.isArray(root.maps)) {
    details.push("maps must be an array.")
  }
  if (!Array.isArray(root.builds)) {
    details.push("builds must be an array.")
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
  const builds = new Set<string>()
  const validBuilds: Array<{ id: string; mapCount: number; maps: string[] }> = []
  for (const [index, build] of catalog.builds.entries()) {
    const record = asRecord(build)
    if (
      !record ||
      !hasString(record.id) ||
      !hasString(record.label) ||
      !hasInteger(record.mapCount) ||
      !Array.isArray(record.maps) ||
      !record.maps.every(hasString)
    ) {
      details.push(`builds[${index}] must include an ID, label, map count, and map names.`)
      continue
    }
    if (builds.has(record.id)) details.push(`duplicate build ${JSON.stringify(record.id)}.`)
    builds.add(record.id)
    validBuilds.push({ id: record.id, mapCount: record.mapCount, maps: record.maps })
  }
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
  const projectionDistributions = new Map(
    catalog.wildEncounterProjection.distributions.map((distribution) => [
      `${distribution.speciesId}/${distribution.regionClass}`,
      distribution,
    ]),
  )
  const ratingCount =
    catalog.wildEncounterProjection.trainerRating.maximum -
    catalog.wildEncounterProjection.trainerRating.minimum +
    1
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
    if (!Array.isArray(map.builds) || map.builds.length === 0 || !map.builds.every(hasString)) {
      details.push(`${map.name} must belong to one or more builds.`)
    } else {
      const mapBuilds = new Set(map.builds)
      if (mapBuilds.size !== map.builds.length) {
        details.push(`${map.name} has duplicate build membership.`)
      }
      for (const build of mapBuilds) {
        if (!builds.has(build)) {
          details.push(`${map.name} refers to undeclared build ${JSON.stringify(build)}.`)
        }
      }
    }
    const connectionOverrides = asRecord(map.connectionOverrides)
    if (map.connectionOverrides !== undefined && !connectionOverrides) {
      details.push(`${map.name} connectionOverrides must be an object.`)
    } else if (connectionOverrides) {
      for (const [build, connections] of Object.entries(connectionOverrides)) {
        if (!builds.has(build) || !Array.isArray(map.builds) || !map.builds.includes(build)) {
          details.push(`${map.name} has a connection override for unavailable build ${build}.`)
        }
        if (!Array.isArray(connections) || !connections.every(hasCatalogConnection)) {
          details.push(`${map.name} connectionOverrides.${build} must contain valid connections.`)
        }
      }
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
        if (set.place.placeLevels.length !== ratingCount) {
          details.push(`${set.baseLabel} place levels must cover the Trainer Rating range.`)
        }
        for (const method of set.methods) {
          const expectedRods =
            method.type === "fishing_mons" ? ["GOOD_ROD", "OLD_ROD", "SUPER_ROD"] : ["NONE"]
          const rods = method.profiles.map((profile) => profile.fishingRod).sort()
          if (
            rods.length !== expectedRods.length ||
            rods.some((rod, index) => rod !== expectedRods[index])
          ) {
            details.push(
              `${set.baseLabel} ${method.type} must have ${method.type === "fishing_mons" ? "Old, Good, and Super Rod weights" : "one weight list"}.`,
            )
          }
          for (const profile of method.profiles) {
            if (method.slots.some((slot) => slot.slotIndex >= profile.weights.length)) {
              details.push(`${set.baseLabel} ${method.type} has a slot without a weight.`)
            }
          }
          for (const slot of method.slots) {
            const distribution = projectionDistributions.get(
              `${slot.speciesId}/${set.place.regionClass}`,
            )
            if (!distribution) {
              details.push(
                `${set.baseLabel} ${method.type} slot ${slot.slotIndex} has no outcome distribution for ${slot.speciesId}.`,
              )
            } else if (
              set.place.placeLevels.some((level) => !(String(level) in distribution.byPlaceLevel))
            ) {
              details.push(
                `${set.baseLabel} ${method.type} slot ${slot.slotIndex} lacks outcomes at a place level of ${slot.speciesId}.`,
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
  for (const build of validBuilds) {
    const expectedMaps = catalog.maps
      .filter((map) => Array.isArray(map.builds) && map.builds.includes(build.id))
      .map((map) => map.name)
    if (build.mapCount !== expectedMaps.length) {
      details.push(`${build.id} has an incorrect map count.`)
    }
    if (
      build.maps.length !== expectedMaps.length ||
      build.maps.some((map, index) => map !== expectedMaps[index])
    ) {
      details.push(`${build.id} has an incorrect map membership list.`)
    }
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
