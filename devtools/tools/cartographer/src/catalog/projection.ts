import * as fs from "node:fs"

import type {
  CatalogEncounterFishingRod,
  CatalogEncounterMethod,
  CatalogEncounterProduct,
  CatalogEncounterProjectionProfile,
  CatalogEncounterSprite,
  CatalogWildEncounterProjection,
} from "./types"

const products = ["EMERALD", "FIRERED", "LEAFGREEN", "POKEMON_HNS"] as const
const methods = ["land_mons", "water_mons", "rock_smash_mons", "fishing_mons"] as const
const rods = ["NONE", "OLD_ROD", "GOOD_ROD", "SUPER_ROD"] as const
const runtimeTimes = ["TIME_MORNING", "TIME_DAY", "TIME_EVENING", "TIME_NIGHT"] as const
const runtimeAreas = [
  "WILD_AREA_LAND",
  "WILD_AREA_WATER",
  "WILD_AREA_ROCKS",
  "WILD_AREA_FISHING",
] as const
const runtimeRods = [
  "WILD_ENCOUNTER_FISHING_ROD_NONE",
  "WILD_ENCOUNTER_FISHING_ROD_OLD",
  "WILD_ENCOUNTER_FISHING_ROD_GOOD",
  "WILD_ENCOUNTER_FISHING_ROD_SUPER",
] as const
const runtimeAreaForMethod = {
  land_mons: "WILD_AREA_LAND",
  water_mons: "WILD_AREA_WATER",
  rock_smash_mons: "WILD_AREA_ROCKS",
  fishing_mons: "WILD_AREA_FISHING",
} as const
const runtimeRodForRod = {
  NONE: "WILD_ENCOUNTER_FISHING_ROD_NONE",
  OLD_ROD: "WILD_ENCOUNTER_FISHING_ROD_OLD",
  GOOD_ROD: "WILD_ENCOUNTER_FISHING_ROD_GOOD",
  SUPER_ROD: "WILD_ENCOUNTER_FISHING_ROD_SUPER",
} as const

const isRecord = (value: unknown): value is Record<string, unknown> =>
  typeof value === "object" && value !== null && !Array.isArray(value)

const fail = (path: string, message: string): never => {
  throw new Error(`${path}: ${message}`)
}

const exactKeys = (value: Record<string, unknown>, expected: readonly string[], path: string) => {
  const actual = Object.keys(value).sort()
  const wanted = [...expected].sort()
  if (actual.length !== wanted.length || actual.some((key, index) => key !== wanted[index])) {
    fail(path, `expected exactly ${wanted.join(", ")}`)
  }
}

const record = (value: unknown, path: string): Record<string, unknown> =>
  isRecord(value) ? value : fail(path, "expected an object")

const array = (value: unknown, path: string): unknown[] =>
  Array.isArray(value) ? value : fail(path, "expected an array")

const string = (value: unknown, path: string): string =>
  typeof value === "string" && value.length > 0 ? value : fail(path, "expected a string")

const integer = (value: unknown, path: string, minimum: number, maximum: number): number =>
  Number.isInteger(value) && (value as number) >= minimum && (value as number) <= maximum
    ? (value as number)
    : fail(path, `expected an integer from ${minimum} through ${maximum}`)

const member = <T extends string>(value: unknown, values: readonly T[], path: string): T =>
  typeof value === "string" && values.includes(value as T)
    ? (value as T)
    : fail(path, `expected one of ${values.join(", ")}`)

const boolean = (value: unknown, path: string): boolean =>
  typeof value === "boolean" ? value : fail(path, "expected a boolean")

const boundedRange = (
  value: unknown,
  path: string,
  expectedMinimum: number,
  expectedMaximum: number,
): { minimum: number; maximum: number } => {
  const range = record(value, path)
  exactKeys(range, ["minimum", "maximum"], path)
  const minimum = integer(range.minimum, `${path}/minimum`, 0, expectedMaximum)
  const maximum = integer(range.maximum, `${path}/maximum`, minimum, expectedMaximum)
  if (minimum !== expectedMinimum || maximum !== expectedMaximum) {
    fail(path, `expected ${expectedMinimum} through ${expectedMaximum}`)
  }
  return { minimum, maximum }
}

const profile = (value: unknown, path: string): CatalogEncounterProjectionProfile => {
  const item = record(value, path)
  const method = member(item.method, methods, `${path}/method`)
  exactKeys(
    item,
    [
      "profileKey",
      "product",
      "map",
      "baseLabel",
      "header",
      "headerId",
      "runtimeTime",
      "method",
      "runtimeArea",
      "fishingRod",
      "runtimeFishingRod",
      "levelOffset",
      "encounterRate",
      "authoredSlotCount",
      "runtimeSlotCount",
      ...(method === "fishing_mons" ? ["weights"] : []),
    ],
    path,
  )
  const product = member(item.product, products, `${path}/product`)
  const baseLabel = string(item.baseLabel, `${path}/baseLabel`)
  const fishingRod = member(item.fishingRod, rods, `${path}/fishingRod`)
  const profileKey = string(item.profileKey, `${path}/profileKey`)
  if (profileKey !== `${product}/${baseLabel}/${method}/${fishingRod}`) {
    fail(`${path}/profileKey`, "does not match its product, label, method, and rod")
  }
  if ((method === "fishing_mons") !== (fishingRod !== "NONE")) {
    fail(path, "fishing profiles must have a rod and non-fishing profiles must use NONE")
  }
  const runtimeArea = member(item.runtimeArea, runtimeAreas, `${path}/runtimeArea`)
  if (runtimeArea !== runtimeAreaForMethod[method]) {
    fail(`${path}/runtimeArea`, `does not match ${method}`)
  }
  const runtimeFishingRod = member(item.runtimeFishingRod, runtimeRods, `${path}/runtimeFishingRod`)
  if (runtimeFishingRod !== runtimeRodForRod[fishingRod]) {
    fail(`${path}/runtimeFishingRod`, `does not match ${fishingRod}`)
  }
  const runtimeSlotCount = integer(item.runtimeSlotCount, `${path}/runtimeSlotCount`, 0, 255)
  let weights: number[] | undefined
  if (method === "fishing_mons") {
    if (runtimeSlotCount !== 10) fail(`${path}/runtimeSlotCount`, "expected 10 for fishing")
    weights = array(item.weights, `${path}/weights`).map((value, index) =>
      integer(value, `${path}/weights/${index}`, 1, 255),
    )
    if (weights.length !== 10) fail(`${path}/weights`, "expected exactly 10 weights")
    if (weights.reduce((sum, weight) => sum + weight, 0) !== 100) {
      fail(`${path}/weights`, "expected weights to total 100")
    }
  }
  return {
    profileKey,
    product,
    map: string(item.map, `${path}/map`),
    baseLabel,
    header: string(item.header, `${path}/header`),
    headerId: integer(item.headerId, `${path}/headerId`, 0, 65_535),
    runtimeTime: member(item.runtimeTime, runtimeTimes, `${path}/runtimeTime`),
    method,
    runtimeArea,
    fishingRod,
    runtimeFishingRod,
    levelOffset: integer(item.levelOffset, `${path}/levelOffset`, -5, 5),
    encounterRate: integer(item.encounterRate, `${path}/encounterRate`, 0, 255),
    authoredSlotCount: integer(item.authoredSlotCount, `${path}/authoredSlotCount`, 1, 255),
    runtimeSlotCount,
    ...(weights ? { weights } : {}),
  }
}

export const readWildEncounterProjection = (
  filePath: string,
  speciesLabelsById: ReadonlyMap<string, string>,
  spriteForSpecies: (speciesId: string) => CatalogEncounterSprite | null,
): CatalogWildEncounterProjection => {
  let parsed: unknown
  try {
    parsed = JSON.parse(fs.readFileSync(filePath, "utf8"))
  } catch (error) {
    fail(filePath, error instanceof Error ? error.message : String(error))
  }
  const root = record(parsed, filePath)
  exactKeys(
    root,
    [
      "schemaVersion",
      "trainerRating",
      "authoredLevel",
      "products",
      "levelProjections",
      "species",
      "profiles",
      "headerCounts",
    ],
    filePath,
  )
  if (root.schemaVersion !== 2) fail(`${filePath}/schemaVersion`, "expected 2")
  const trainerRating = boundedRange(root.trainerRating, `${filePath}/trainerRating`, 10, 80)
  const authoredLevel = boundedRange(root.authoredLevel, `${filePath}/authoredLevel`, 1, 100)

  const productRows = array(root.products, `${filePath}/products`).map((value, index) => {
    const path = `${filePath}/products/${index}`
    const item = record(value, path)
    exactKeys(item, ["id", "displayName"], path)
    return {
      id: member(item.id, products, `${path}/id`),
      displayName: string(item.displayName, `${path}/displayName`),
    }
  })
  if (
    productRows.length !== products.length ||
    products.some((product) => productRows.filter((row) => row.id === product).length !== 1)
  ) {
    fail(`${filePath}/products`, "expected each supported product exactly once")
  }

  const offsets = new Set<number>()
  const levelProjections = array(root.levelProjections, `${filePath}/levelProjections`).map(
    (value, offsetIndex) => {
      const path = `${filePath}/levelProjections/${offsetIndex}`
      const item = record(value, path)
      exactKeys(item, ["levelOffset", "ratings"], path)
      const levelOffset = integer(item.levelOffset, `${path}/levelOffset`, -5, 5)
      if (offsets.has(levelOffset)) fail(`${path}/levelOffset`, "duplicate offset")
      offsets.add(levelOffset)
      const ratings = array(item.ratings, `${path}/ratings`).map((value, ratingIndex) => {
        const ratingPath = `${path}/ratings/${ratingIndex}`
        const ratingRow = record(value, ratingPath)
        exactKeys(ratingRow, ["rating", "projectedLevels"], ratingPath)
        const expectedRating = trainerRating.minimum + ratingIndex
        const rating = integer(
          ratingRow.rating,
          `${ratingPath}/rating`,
          trainerRating.minimum,
          trainerRating.maximum,
        )
        if (rating !== expectedRating) fail(`${ratingPath}/rating`, `expected ${expectedRating}`)
        const projectedLevels = array(
          ratingRow.projectedLevels,
          `${ratingPath}/projectedLevels`,
        ).map((level, levelIndex) =>
          integer(level, `${ratingPath}/projectedLevels/${levelIndex}`, 1, 100),
        )
        if (projectedLevels.length !== 100) {
          fail(`${ratingPath}/projectedLevels`, "expected 100 authored-level projections")
        }
        return { rating, projectedLevels }
      })
      if (ratings.length !== trainerRating.maximum - trainerRating.minimum + 1) {
        fail(`${path}/ratings`, "does not cover the Trainer Rating range")
      }
      return { levelOffset, ratings }
    },
  )
  if (!offsets.has(0)) fail(`${filePath}/levelProjections`, "must include offset 0")

  const rawSpecies = array(root.species, `${filePath}/species`)
  const speciesIds = new Set<string>()
  for (const [index, value] of rawSpecies.entries()) {
    const item = record(value, `${filePath}/species/${index}`)
    const id = string(item.authoredSpecies, `${filePath}/species/${index}/authoredSpecies`)
    if (speciesIds.has(id))
      fail(`${filePath}/species/${index}/authoredSpecies`, "duplicate species")
    speciesIds.add(id)
  }
  const species = rawSpecies.map((value, speciesIndex) => {
    const path = `${filePath}/species/${speciesIndex}`
    const item = record(value, path)
    exactKeys(item, ["authoredSpecies", "authoredSpeciesId", "outcomesByProjectedLevel"], path)
    const authoredSpecies = string(item.authoredSpecies, `${path}/authoredSpecies`)
    const speciesLabel =
      speciesLabelsById.get(authoredSpecies) ??
      (authoredSpecies === "SPECIES_NONE" ? "NONE" : undefined)
    if (!speciesLabel) fail(`${path}/authoredSpecies`, "has no species label source entry")
    let expectedMinimum = 1
    const outcomesByProjectedLevel = array(
      item.outcomesByProjectedLevel,
      `${path}/outcomesByProjectedLevel`,
    ).map((value, outcomeIndex) => {
      const outcomePath = `${path}/outcomesByProjectedLevel/${outcomeIndex}`
      const outcome = record(value, outcomePath)
      exactKeys(
        outcome,
        [
          "minimumProjectedLevel",
          "maximumProjectedLevel",
          "effectiveSpecies",
          "eligible",
          "minimumOrdinaryWildLevel",
        ],
        outcomePath,
      )
      const minimumProjectedLevel = integer(
        outcome.minimumProjectedLevel,
        `${outcomePath}/minimumProjectedLevel`,
        1,
        100,
      )
      const maximumProjectedLevel = integer(
        outcome.maximumProjectedLevel,
        `${outcomePath}/maximumProjectedLevel`,
        minimumProjectedLevel,
        100,
      )
      if (minimumProjectedLevel !== expectedMinimum) {
        fail(outcomePath, `expected interval to begin at projected level ${expectedMinimum}`)
      }
      expectedMinimum = maximumProjectedLevel + 1
      const effectiveSpecies = string(outcome.effectiveSpecies, `${outcomePath}/effectiveSpecies`)
      if (!speciesIds.has(effectiveSpecies)) {
        fail(`${outcomePath}/effectiveSpecies`, "does not reference projection species metadata")
      }
      return {
        minimumProjectedLevel,
        maximumProjectedLevel,
        effectiveSpecies,
        eligible: boolean(outcome.eligible, `${outcomePath}/eligible`),
        minimumOrdinaryWildLevel: integer(
          outcome.minimumOrdinaryWildLevel,
          `${outcomePath}/minimumOrdinaryWildLevel`,
          1,
          100,
        ),
      }
    })
    if (expectedMinimum !== 101)
      fail(`${path}/outcomesByProjectedLevel`, "must cover levels 1 through 100")
    return {
      authoredSpecies,
      authoredSpeciesId: integer(item.authoredSpeciesId, `${path}/authoredSpeciesId`, 0, 65_535),
      speciesLabel: speciesLabel!,
      sprite: spriteForSpecies(authoredSpecies),
      outcomesByProjectedLevel,
    }
  })

  const profileKeys = new Set<string>()
  const profiles = array(root.profiles, `${filePath}/profiles`).map((value, index) => {
    const item = profile(value, `${filePath}/profiles/${index}`)
    if (profileKeys.has(item.profileKey))
      fail(`${filePath}/profiles/${index}/profileKey`, "duplicate key")
    if (!offsets.has(item.levelOffset))
      fail(`${filePath}/profiles/${index}/levelOffset`, "has no projection table")
    profileKeys.add(item.profileKey)
    return item
  })

  const headerCountsRecord = record(root.headerCounts, `${filePath}/headerCounts`)
  exactKeys(headerCountsRecord, products, `${filePath}/headerCounts`)
  const headerCounts = Object.fromEntries(
    products.map((product) => [
      product,
      integer(headerCountsRecord[product], `${filePath}/headerCounts/${product}`, 0, 65_535),
    ]),
  ) as Record<CatalogEncounterProduct, number>
  for (const item of profiles) {
    if (item.headerId >= headerCounts[item.product]) {
      fail(
        `${filePath}/profiles/${item.profileKey}/headerId`,
        `exceeds the ${item.product} header count`,
      )
    }
  }

  return {
    schemaVersion: 2,
    trainerRating,
    authoredLevel,
    products: productRows,
    levelProjections,
    species,
    profiles,
    headerCounts,
  }
}

export const profileIndex = (
  projection: CatalogWildEncounterProjection,
): ReadonlyMap<string, CatalogEncounterProjectionProfile> => {
  const indexed = new Map<string, CatalogEncounterProjectionProfile>()
  for (const item of projection.profiles) {
    const key = `${item.product}/${item.baseLabel}/${item.method}/${item.fishingRod}`
    if (indexed.has(key)) throw new Error(`wild encounter projection: duplicate join key ${key}`)
    indexed.set(key, item)
  }
  return indexed
}

export const profileLookupKey = (
  product: CatalogEncounterProduct,
  baseLabel: string,
  method: CatalogEncounterMethod["type"],
  fishingRod: CatalogEncounterFishingRod,
): string => `${product}/${baseLabel}/${method}/${fishingRod}`
