import * as fs from "node:fs"
import * as path from "node:path"

import { catalogEncounterSprites } from "./encounter-sprites"
import type {
  CatalogEncounterFishingRod,
  CatalogEncounterMethod,
  CatalogEncounterOutcome,
  CatalogEncounterPlace,
  CatalogEncounterReach,
  CatalogEncounterRegionClass,
  CatalogEncounterRuntimeTime,
  CatalogEncounterSet,
  CatalogEncounterSprite,
  CatalogSourcePointer,
  CatalogWildEncounterProjection,
  CatalogWildEncounters,
} from "./types"

const speciesInfoPath = "src/data/pokemon/species_info.h"
const speciesInfoDirectory = "src/data/pokemon/species_info"
const wildEncounterDataDirectory = "src/data/wild_encounters_v2"
const product = "POKEMON_WAYFARER"
const productName = "Pokémon Wayfarer"
const schemaVersion = 3
const timesOfDay = ["morning", "day", "evening", "night"] as const
// Every v2 header fills all four time slots: Morning and Day alias the day table, Evening and
// Night the night table.
const tableForTime = { morning: "day", day: "day", evening: "night", night: "night" } as const
const methodForSource = {
  land: "land_mons",
  surf: "water_mons",
  rock: "rock_smash_mons",
  fish: "fishing_mons",
} as const
const rodForSource = { old: "OLD_ROD", good: "GOOD_ROD", super: "SUPER_ROD" } as const
const reaches: readonly CatalogEncounterReach[] = ["Road", "Wilds", "Outlands", "Dungeon"]
const regionClasses: readonly CatalogEncounterRegionClass[] = ["Other", "Safari", "Sinjoh"]

type SourceMethodKey = keyof typeof methodForSource
type SourceTable = {
  rate: { day: number; night: number }
  day: string[]
  night: string[]
}

const fail = (pointer: string, message: string): never => {
  throw new Error(`wild encounter projection${pointer}: ${message}`)
}

const isRecord = (value: unknown): value is Record<string, unknown> =>
  typeof value === "object" && value !== null && !Array.isArray(value)

const record = (value: unknown, pointer: string): Record<string, unknown> =>
  isRecord(value) ? value : fail(pointer, "expected an object")

const array = (value: unknown, pointer: string): unknown[] =>
  Array.isArray(value) ? value : fail(pointer, "expected an array")

const text = (value: unknown, pointer: string): string =>
  typeof value === "string" && value.length > 0 ? value : fail(pointer, "expected a string")

const integer = (value: unknown, pointer: string, minimum: number, maximum: number): number =>
  typeof value === "number" && Number.isInteger(value) && value >= minimum && value <= maximum
    ? value
    : fail(pointer, `expected an integer from ${minimum} through ${maximum}`)

const integers = (value: unknown, pointer: string, minimum: number, maximum: number): number[] =>
  array(value, pointer).map((item, index) => integer(item, `${pointer}/${index}`, minimum, maximum))

const member = <T extends string>(value: unknown, values: readonly T[], pointer: string): T =>
  typeof value === "string" && values.includes(value as T)
    ? (value as T)
    : fail(pointer, `expected one of ${values.join(", ")}`)

const sourcePointer = (file: string, pointer: string): CatalogSourcePointer => ({
  path: `${wildEncounterDataDirectory}/${file}`,
  pointer,
})

const speciesConstant = (name: string): string => `SPECIES_${name}`

const sourceTable = (
  value: unknown,
  slotWeights: readonly number[],
  pointer: string,
): SourceTable => {
  const table = record(value, pointer)
  const rate = record(table.rate, `${pointer}/rate`)
  const species = (time: "day" | "night"): string[] => {
    const names = array(table[time], `${pointer}/${time}`).map((name, index) =>
      text(name, `${pointer}/${time}/${index}`),
    )
    if (names.length !== slotWeights.length) {
      fail(`${pointer}/${time}`, `expected ${slotWeights.length} slots`)
    }
    return names
  }
  return {
    rate: {
      day: integer(rate.day, `${pointer}/rate/day`, 0, 255),
      night: integer(rate.night, `${pointer}/rate/night`, 0, 255),
    },
    day: species("day"),
    night: species("night"),
  }
}

const runtimeTimesFor = (sets: readonly CatalogEncounterSet[]): CatalogEncounterRuntimeTime[] =>
  timesOfDay.map((timeOfDay) => ({
    product,
    timeOfDay,
    methods: (Object.values(methodForSource) as CatalogEncounterMethod["type"][]).map((type) => {
      const direct = sets.filter(
        (set) =>
          set.runtimeTime === tableForTime[timeOfDay] &&
          set.methods.some((method) => method.type === type),
      )
      return {
        type,
        resolution: direct.length > 0 ? ("direct" as const) : ("unavailable" as const),
        sets: direct.map((set) => ({ baseLabel: set.baseLabel, source: set.source })),
      }
    }),
  }))

/**
 * Turn the generator's v2 projection (game/tools/wild_encounters/v2/cartographer_projection.py)
 * into Cartographer's per-map encounter sets and the level projection the UI evaluates.
 * Each header key yields a day set and a night set; Morning and Day use the day set, Evening and
 * Night the night set.
 */
export const catalogWildEncounters = (
  document: unknown,
  mapNamesById: ReadonlyMap<string, string>,
  speciesLabelsById: ReadonlyMap<string, string>,
  spriteForSpecies: (speciesId: string) => CatalogEncounterSprite | null = () => null,
): {
  encountersByMap: Map<string, CatalogWildEncounters>
  projection: CatalogWildEncounterProjection
} => {
  const root = record(document, "")
  if (root.schemaVersion !== schemaVersion) fail("/schemaVersion", `expected ${schemaVersion}`)
  const rating = record(root.trainerRating, "/trainerRating")
  const trainerRating = {
    minimum: integer(rating.minimum, "/trainerRating/minimum", 0, 1000),
    maximum: integer(rating.maximum, "/trainerRating/maximum", 0, 1000),
  }
  const ratingCount = trainerRating.maximum - trainerRating.minimum + 1
  const weights = record(root.weights, "/weights")
  const tableWeights = (name: "land" | "surf" | "rock") =>
    integers(weights[name], `/weights/${name}`, 1, 100)
  const slotWeights: Record<SourceMethodKey, number[]> = {
    land: tableWeights("land"),
    surf: tableWeights("surf"),
    rock: tableWeights("rock"),
    fish: [],
  }
  const rodWeights = record(weights.fish, "/weights/fish")
  const fishingWeights = (Object.keys(rodForSource) as (keyof typeof rodForSource)[]).map(
    (rod) => [rod, integers(rodWeights[rod], `/weights/fish/${rod}`, 1, 100)] as const,
  )
  slotWeights.fish = fishingWeights[0]![1]

  const speciesIndex = new Map<string, number>()
  const species: CatalogWildEncounterProjection["species"] = []
  const speciesIndexFor = (name: string, pointer: string): number => {
    const speciesId = speciesConstant(name)
    const known = speciesIndex.get(speciesId)
    if (known !== undefined) return known
    const speciesLabel = speciesLabelsById.get(speciesId)
    if (!speciesLabel) fail(pointer, `${speciesId} has no species label source entry`)
    speciesIndex.set(speciesId, species.length)
    species.push({ speciesId, speciesLabel: speciesLabel!, sprite: spriteForSpecies(speciesId) })
    return species.length - 1
  }

  const byMap = new Map<string, CatalogWildEncounters>()
  const setsByMap = new Map<string, CatalogEncounterSet[]>()
  const keys = new Set<string>()
  for (const [index, value] of array(root.sets, "/sets").entries()) {
    const pointer = `/sets/${index}`
    const item = record(value, pointer)
    const key = text(item.key, `${pointer}/key`)
    if (keys.has(key)) fail(`${pointer}/key`, "duplicate header key")
    keys.add(key)
    const mapId = text(item.map, `${pointer}/map`)
    const mapName = mapNamesById.get(mapId)
    const baseLabel = text(item.baseLabel, `${pointer}/baseLabel`)
    const file = text(item.sourceFile, `${pointer}/sourceFile`)
    const placeSource = record(item.place, `${pointer}/place`)
    const dungeon =
      placeSource.dungeon === null ? null : record(placeSource.dungeon, `${pointer}/place/dungeon`)
    const reach = member(placeSource.reach, reaches, `${pointer}/place/reach`)
    if ((reach === "Dungeon") !== (dungeon !== null)) {
      fail(`${pointer}/place/dungeon`, "expected an intent for dungeons only")
    }
    const placeLevels = integers(item.placeLevels, `${pointer}/placeLevels`, 1, 100)
    if (placeLevels.length !== ratingCount) {
      fail(`${pointer}/placeLevels`, "does not cover the Trainer Rating range")
    }
    const nullableInteger = (field: unknown, name: string): number | null =>
      field === null ? null : integer(field, `${pointer}/place/${name}`, 0, 255)
    const place: CatalogEncounterPlace = {
      name: text(placeSource.name, `${pointer}/place/name`),
      region: text(placeSource.region, `${pointer}/place/region`),
      regionClass: member(placeSource.regionClass, regionClasses, `${pointer}/place/regionClass`),
      reach,
      dungeon: dungeon
        ? {
            intent: text(dungeon.intent, `${pointer}/place/dungeon/intent`),
            flat: dungeon.flat === true,
          }
        : null,
      floor: nullableInteger(placeSource.floor, "floor"),
      floors: nullableInteger(placeSource.floors, "floors"),
      placeLevels,
    }
    const variant = item.variant === null ? null : text(item.variant, `${pointer}/variant`)
    const methodsSource = record(item.methods, `${pointer}/methods`)
    if (!mapName) continue

    const sets = (["day", "night"] as const).map((time) => {
      const methods: CatalogEncounterMethod[] = []
      for (const sourceMethod of Object.keys(methodForSource) as SourceMethodKey[]) {
        if (!(sourceMethod in methodsSource)) continue
        const methodPointer = `${pointer}/methods/${sourceMethod}`
        const table = sourceTable(
          methodsSource[sourceMethod],
          slotWeights[sourceMethod],
          methodPointer,
        )
        const type = methodForSource[sourceMethod]
        const source = sourcePointer(file, `/${key}/${sourceMethod}/${time}`)
        const names = table[time]
        const slots = names.flatMap((name, slotIndex) =>
          name === "NONE"
            ? []
            : [
                {
                  slotIndex,
                  speciesId: speciesConstant(name),
                  speciesLabel:
                    species[speciesIndexFor(name, `${methodPointer}/${time}/${slotIndex}`)]!
                      .speciesLabel,
                  sprite: spriteForSpecies(speciesConstant(name)),
                  source: sourcePointer(file, `/${key}/${sourceMethod}/${time}/${slotIndex}`),
                },
              ],
        )
        const profiles =
          sourceMethod === "fish"
            ? fishingWeights.map(([rod, rodSlotWeights]) => ({
                profileKey: `${baseLabel}/${time}/${type}/${rodForSource[rod]}`,
                fishingRod: rodForSource[rod] as CatalogEncounterFishingRod,
                weights: rodSlotWeights,
              }))
            : [
                {
                  profileKey: `${baseLabel}/${time}/${type}/NONE`,
                  fishingRod: "NONE" as const,
                  weights: slotWeights[sourceMethod],
                },
              ]
        methods.push({ type, encounterRate: table.rate[time], source, slots, profiles })
      }
      return {
        mapId,
        mapName,
        baseLabel: `${baseLabel}_${time === "day" ? "Day" : "Night"}`,
        product,
        runtimeTime: time,
        variant,
        place,
        source: sourcePointer(file, `/${key}`),
        methods,
      } satisfies CatalogEncounterSet
    })
    setsByMap.set(mapName, [...(setsByMap.get(mapName) ?? []), ...sets])
  }
  for (const [mapName, sets] of setsByMap) {
    // A map with several tables per time of day (the Bug Contest weekdays) is chosen by the
    // game, not by the clock, so it carries no time-of-day resolution.
    const selected = sets.some((set) => set.variant !== null)
    byMap.set(mapName, { sets, runtimeTimes: selected ? [] : runtimeTimesFor(sets) })
  }

  const distributions = array(root.outcomes, "/outcomes").map((value, index) => {
    const pointer = `/outcomes/${index}`
    const item = record(value, pointer)
    const speciesName = text(item.species, `${pointer}/species`)
    const byPlaceLevel: Record<string, CatalogEncounterOutcome[]> = {}
    for (const [level, outcomes] of Object.entries(
      record(item.byPlaceLevel, `${pointer}/byPlaceLevel`),
    )) {
      byPlaceLevel[level] = array(outcomes, `${pointer}/byPlaceLevel/${level}`).map(
        (outcome, outcomeIndex) => {
          const row = array(outcome, `${pointer}/byPlaceLevel/${level}/${outcomeIndex}`)
          const rowPointer = `${pointer}/byPlaceLevel/${level}/${outcomeIndex}`
          return [
            speciesIndexFor(text(row[0], rowPointer), rowPointer),
            integer(row[1], rowPointer, 1, 100),
            integer(row[2], rowPointer, 1, 1000),
          ]
        },
      )
    }
    speciesIndexFor(speciesName, `${pointer}/species`)
    return {
      speciesId: speciesConstant(speciesName),
      regionClass: member(item.regionClass, regionClasses, `${pointer}/regionClass`),
      byPlaceLevel,
    }
  })

  return {
    encountersByMap: byMap,
    projection: {
      schemaVersion,
      trainerRating,
      outcomeDenominator: integer(root.outcomeDenominator, "/outcomeDenominator", 1, 1000),
      products: [{ id: product, displayName: productName }],
      species,
      distributions,
    },
  }
}

export const sourceWildEncounterCatalog = (
  root: string,
  mapNamesById: ReadonlyMap<string, string>,
  output: string,
  projectionPath: string,
  spriteForSpecies: (speciesId: string) => CatalogEncounterSprite | null = catalogEncounterSprites(
    root,
    output,
  ),
): {
  encountersByMap: Map<string, CatalogWildEncounters>
  projection: CatalogWildEncounterProjection
} => {
  let document: unknown
  try {
    document = JSON.parse(fs.readFileSync(projectionPath, "utf8"))
  } catch (error) {
    throw new Error(
      `${projectionPath}: ${error instanceof Error ? error.message : String(error)}`,
      {
        cause: error,
      },
    )
  }
  return catalogWildEncounters(document, mapNamesById, sourceSpeciesLabels(root), spriteForSpecies)
}

export const sourceSpeciesLabels = (root: string): Map<string, string> => {
  const speciesLabels = new Map<string, string>()
  const directory = path.join(root, speciesInfoDirectory)
  const sources = [
    path.join(root, speciesInfoPath),
    ...fs
      .readdirSync(directory)
      .filter((file) => file.endsWith(".h"))
      .sort()
      .map((file) => path.join(directory, file)),
  ]
  const sourceTexts = sources.map((filePath) => fs.readFileSync(filePath, "utf8"))
  for (const source of sourceTexts) {
    const entries = source.matchAll(
      /^\s*\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*\{(?:(?!^\s*\[\s*SPECIES_)[\s\S])*?\.speciesName\s*=\s*_\("([^"]*)"\)/gm,
    )
    for (const [, speciesId, speciesLabel] of entries) {
      speciesLabels.set(speciesId!, speciesLabel!)
    }
  }
  // A macro names its species directly or through the macros its body invokes
  // (FLOETTE_NORMAL_INFO -> FLOETTE_MISC_INFO).
  const macroBodies = new Map<string, string>()
  for (const source of sourceTexts) {
    for (const [, macro, body] of source.matchAll(
      /^\s*#define\s+(\w+)\([^)]*\)([\s\S]*?)(?=^\s*(?:#(?:define|if|endif)|\[\s*SPECIES_)|(?![\s\S]))/gm,
    )) {
      if (macro && body) macroBodies.set(macro, body)
    }
  }
  const macroLabel = (macro: string, visited: Set<string> = new Set()): string | undefined => {
    const body = macroBodies.get(macro)
    if (body === undefined || visited.has(macro)) return undefined
    visited.add(macro)
    const direct = body.match(/\.speciesName\s*=\s*_\("([^"]*)"\)/)?.[1]
    if (direct) return direct
    for (const [, invoked] of body.matchAll(/\b(\w+)\s*\(/g)) {
      const label = invoked ? macroLabel(invoked, visited) : undefined
      if (label) return label
    }
    return undefined
  }
  for (const source of sourceTexts) {
    for (const [, speciesId, macro] of source.matchAll(
      /^\s*\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*(?:\{\s*)?(\w+)\s*\(/gm,
    )) {
      const label = macro ? macroLabel(macro) : undefined
      if (speciesId && label) speciesLabels.set(speciesId, label)
    }
  }
  const aliases = new Map<string, string>()
  const speciesConstants = fs.readFileSync(path.join(root, "include/constants/species.h"), "utf8")
  for (const [, alias, target] of speciesConstants.matchAll(
    /^\s*#define\s+(SPECIES_[A-Z0-9_]+)\s+(SPECIES_[A-Z0-9_]+)\s*$/gm,
  )) {
    if (alias && target) aliases.set(alias, target)
  }
  for (const alias of aliases.keys()) {
    const visited = new Set<string>()
    let target = alias
    while (aliases.has(target) && !visited.has(target)) {
      visited.add(target)
      target = aliases.get(target)!
    }
    const label = speciesLabels.get(target)
    if (label) speciesLabels.set(alias, label)
  }

  if (speciesLabels.size === 0) {
    throw new Error(`${speciesInfoPath}: expected Pokémon Wayfarer species label source entries`)
  }
  return speciesLabels
}
