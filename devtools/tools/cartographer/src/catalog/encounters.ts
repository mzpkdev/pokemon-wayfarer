import * as fs from "node:fs"
import * as path from "node:path"

import { catalogEncounterSprites } from "./encounter-sprites"
import type {
  CatalogEncounterFishingRod,
  CatalogEncounterProduct,
  CatalogEncounterProjectionProfile,
  CatalogEncounterSprite,
  CatalogEncounterSet,
  CatalogEncounterTimeOfDay,
  CatalogEncounterRuntimeTime,
  CatalogSourcePointer,
  CatalogWildEncounterProjection,
  CatalogWildEncounters,
} from "./types"
import { profileIndex, profileLookupKey, readWildEncounterProjection } from "./projection"

const wildEncounterPath = "src/data/wild_encounters.json"
const speciesInfoPath = "src/data/pokemon/species_info.h"
const speciesInfoDirectory = "src/data/pokemon/species_info"
const wildEncounterRuntimePath = "src/wild_encounter.c"
const overworldConfigPath = "include/config/overworld.h"
const rtcConstantsPath = "include/constants/rtc.h"
const encounterTypes = ["land_mons", "water_mons", "rock_smash_mons", "fishing_mons"] as const
const runtimeTimeIds = ["morning", "day", "evening", "night"] as const
const profileRods: CatalogEncounterFishingRod[] = ["OLD_ROD", "GOOD_ROD", "SUPER_ROD"]

type EncounterType = (typeof encounterTypes)[number]
type RuntimeTimeId = (typeof runtimeTimeIds)[number]

type EncounterRuntimeConfig = {
  enabled: boolean
  disableFallback: boolean
  fallbackTime: RuntimeTimeId
  labels: ReadonlyMap<RuntimeTimeId, string>
}

const defaultRuntimeConfig: EncounterRuntimeConfig = {
  enabled: true,
  disableFallback: false,
  fallbackTime: "day",
  labels: new Map([
    ["morning", "Morning"],
    ["day", "Day"],
    ["evening", "Evening"],
    ["night", "Night"],
  ]),
}

type SourceField = {
  type: EncounterType
  encounter_rates: number[]
  groups?: Record<string, number[]>
}

type SourceSlot = {
  min_level: number
  max_level: number
  species: string
}

type SourceMethod = {
  encounter_rate: number
  mons: SourceSlot[]
}

type SourceEncounter = {
  map: string
  base_label: string
  [method: string]: unknown
}

type SourceEncounterGroup = {
  label: string
  for_maps?: boolean
  fields: SourceField[]
  encounters: SourceEncounter[]
}

type SourceEncounterDocument = {
  wild_encounter_groups: SourceEncounterGroup[]
}

type FieldMetadata = {
  field: SourceField
  groupIndex: number
  fieldIndex: number
}

const sourcePointer = (pointer: string): CatalogSourcePointer => {
  return { path: wildEncounterPath, pointer }
}

const sourceError = (pointer: string, message: string): Error => {
  return new Error(`${wildEncounterPath}${pointer}: ${message}`)
}

const isRecord = (value: unknown): value is Record<string, unknown> => {
  return typeof value === "object" && value !== null && !Array.isArray(value)
}

const isEncounterType = (value: unknown): value is EncounterType => {
  return typeof value === "string" && encounterTypes.includes(value as EncounterType)
}

const requireString = (value: unknown, pointer: string): string => {
  if (typeof value !== "string" || value.length === 0) {
    throw sourceError(pointer, "expected a non-empty string")
  }
  return value
}

const requireRate = (value: unknown, pointer: string): number => {
  if (typeof value !== "number" || !Number.isFinite(value) || value < 0) {
    throw sourceError(pointer, "expected a non-negative finite number")
  }
  return value
}

const requireIndex = (value: unknown, pointer: string, slotCount: number): number => {
  if (!Number.isInteger(value) || (value as number) < 0 || (value as number) >= slotCount) {
    throw sourceError(pointer, `expected a slot index from 0 through ${slotCount - 1}`)
  }
  return value as number
}

const fieldsFor = (document: unknown): FieldMetadata[][] => {
  if (!isRecord(document) || !Array.isArray(document.wild_encounter_groups)) {
    throw sourceError("", "expected wild_encounter_groups to be an array")
  }
  return document.wild_encounter_groups.map((group, groupIndex) => {
    const groupPointer = `/wild_encounter_groups/${groupIndex}`
    if (!isRecord(group)) throw sourceError(groupPointer, "expected an object")
    requireString(group.label, `${groupPointer}/label`)
    if (group.for_maps !== true) return []
    if (!Array.isArray(group.fields)) {
      throw sourceError(`${groupPointer}/fields`, "expected an array")
    }
    if (!Array.isArray(group.encounters)) {
      throw sourceError(`${groupPointer}/encounters`, "expected an array")
    }
    return group.fields.map((field, fieldIndex) => {
      const fieldPointer = `${groupPointer}/fields/${fieldIndex}`
      if (!isRecord(field) || !isEncounterType(field.type)) {
        throw sourceError(fieldPointer, "expected a supported encounter method type")
      }
      if (!Array.isArray(field.encounter_rates) || field.encounter_rates.length === 0) {
        throw sourceError(`${fieldPointer}/encounter_rates`, "expected a non-empty array")
      }
      for (const [rateIndex, rate] of field.encounter_rates.entries()) {
        requireRate(rate, `${fieldPointer}/encounter_rates/${rateIndex}`)
      }
      if (field.groups !== undefined) {
        if (!isRecord(field.groups))
          throw sourceError(`${fieldPointer}/groups`, "expected an object")
        for (const [groupId, indices] of Object.entries(field.groups)) {
          requireString(groupId, `${fieldPointer}/groups`)
          if (!Array.isArray(indices)) {
            throw sourceError(`${fieldPointer}/groups/${groupId}`, "expected an array")
          }
          for (const [index, slotIndex] of indices.entries()) {
            requireIndex(
              slotIndex,
              `${fieldPointer}/groups/${groupId}/${index}`,
              field.encounter_rates.length,
            )
          }
        }
      }
      return { field: field as SourceField, groupIndex, fieldIndex }
    })
  })
}

const methodFor = (value: unknown, pointer: string): SourceMethod => {
  if (!isRecord(value)) throw sourceError(pointer, "expected an object")
  const encounterRate = requireRate(value.encounter_rate, `${pointer}/encounter_rate`)
  if (!Array.isArray(value.mons) || value.mons.length === 0) {
    throw sourceError(`${pointer}/mons`, "expected at least one source slot")
  }
  for (const [slotIndex, slot] of value.mons.entries()) {
    const slotPointer = `${pointer}/mons/${slotIndex}`
    if (!isRecord(slot)) throw sourceError(slotPointer, "expected an object")
    const minLevel = requireRate(slot.min_level, `${slotPointer}/min_level`)
    const maxLevel = requireRate(slot.max_level, `${slotPointer}/max_level`)
    if (!Number.isInteger(minLevel) || !Number.isInteger(maxLevel)) {
      throw sourceError(slotPointer, "expected integral levels")
    }
    requireString(slot.species, `${slotPointer}/species`)
  }
  return { encounter_rate: encounterRate, mons: value.mons as SourceSlot[] }
}

const groupsForSlot = (
  metadata: FieldMetadata,
  slotIndex: number,
): CatalogEncounterSet["methods"][number]["slots"][number]["groups"] => {
  const groupPointer = `/wild_encounter_groups/${metadata.groupIndex}/fields/${metadata.fieldIndex}/groups`
  return Object.entries(metadata.field.groups ?? [])
    .filter(([, slots]) => slots.includes(slotIndex))
    .map(([id]) => ({ id, source: sourcePointer(`${groupPointer}/${id}`) }))
}

const sourceTimeForBaseLabel = (
  baseLabel: string,
  runtimeConfig: EncounterRuntimeConfig,
): RuntimeTimeId => {
  for (const timeOfDay of runtimeTimeIds) {
    const label = runtimeConfig.labels.get(timeOfDay)
    if (label && baseLabel.includes(`_${label}`)) return timeOfDay
  }
  return runtimeConfig.fallbackTime
}

export const sourceProductForBaseLabel = (baseLabel: string): CatalogEncounterProduct => {
  const markedProducts: CatalogEncounterProduct[] = []
  if (baseLabel.includes("FireRed")) markedProducts.push("FIRERED")
  if (baseLabel.includes("LeafGreen")) markedProducts.push("LEAFGREEN")
  if (baseLabel.includes("_Hns") || baseLabel.includes("_hns")) {
    markedProducts.push("POKEMON_HNS")
  }
  if (markedProducts.length > 1) {
    throw new Error(`${baseLabel}: source label has ambiguous product markers`)
  }
  return markedProducts[0] ?? "EMERALD"
}

const runtimeTimesFor = (
  sets: readonly CatalogEncounterSet[],
  runtimeConfig: EncounterRuntimeConfig,
): CatalogEncounterRuntimeTime[] => {
  if (!runtimeConfig.enabled) return []
  const presentProducts = [...new Set(sets.map((set) => set.product))]
  return presentProducts.flatMap((product) =>
    runtimeTimeIds.map((timeOfDay) => ({
      product,
      timeOfDay,
      methods: encounterTypes.map((type) => {
        const direct = sets.filter(
          (set) =>
            set.product === product &&
            set.runtimeTime === timeOfDay &&
            set.methods.some((method) => method.type === type),
        )
        const fallback = runtimeConfig.disableFallback
          ? []
          : sets.filter(
              (set) =>
                set.product === product &&
                set.runtimeTime === runtimeConfig.fallbackTime &&
                set.methods.some((method) => method.type === type),
            )
        const resolvedSets = direct.length > 0 ? direct : fallback
        return {
          type,
          resolution:
            direct.length > 0 ? "direct" : fallback.length > 0 ? "fallback" : "unavailable",
          sets: resolvedSets.map((set) => ({ baseLabel: set.baseLabel, source: set.source })),
        }
      }),
    })),
  )
}

const timeOfDayForProfile = (
  runtimeTime: CatalogEncounterProjectionProfile["runtimeTime"],
): CatalogEncounterTimeOfDay =>
  runtimeTime.replace(/^TIME_/, "").toLowerCase() as CatalogEncounterTimeOfDay

const joinedProfiles = (
  product: CatalogEncounterProduct,
  baseLabel: string,
  mapId: string,
  metadata: FieldMetadata,
  method: SourceMethod,
  profiles: ReadonlyMap<string, CatalogEncounterProjectionProfile> | undefined,
  consumedProfiles: Set<string>,
): CatalogEncounterProjectionProfile[] => {
  if (!profiles) return []
  const rodsForMethod: CatalogEncounterFishingRod[] =
    metadata.field.type === "fishing_mons" ? profileRods : ["NONE"]
  return rodsForMethod.map((rod) => {
    const key = profileLookupKey(product, baseLabel, metadata.field.type, rod)
    const profile = profiles.get(key)
    if (!profile) throw sourceError("", `projection has no exact profile join for ${key}`)
    const runtimeSlotCount =
      rod === "NONE"
        ? metadata.field.encounter_rates.length
        : (metadata.field.groups?.[rod.toLowerCase()]?.length ?? 0)
    const mismatches = [
      profile.map === mapId ? null : `map ${profile.map}`,
      profile.encounterRate === method.encounter_rate
        ? null
        : `encounter rate ${profile.encounterRate}`,
      profile.authoredSlotCount === method.mons.length
        ? null
        : `authored slot count ${profile.authoredSlotCount}`,
      profile.runtimeSlotCount === runtimeSlotCount
        ? null
        : `runtime slot count ${profile.runtimeSlotCount}`,
    ].filter((value): value is string => value !== null)
    if (mismatches.length > 0) {
      throw sourceError(
        "",
        `projection profile ${profile.profileKey} disagrees on ${mismatches.join(", ")}`,
      )
    }
    if (consumedProfiles.has(profile.profileKey)) {
      throw sourceError("", `projection profile joined more than once: ${profile.profileKey}`)
    }
    consumedProfiles.add(profile.profileKey)
    return profile
  })
}

/**
 * Preserve source encounter sets and resolve their time-of-day tables as the source generator
 * does: labels such as gRoute32_hns_Day and _Night share one runtime map header. Each runtime
 * area resolves independently, falling back only when its direct table is absent.
 */
export const catalogWildEncounters = (
  document: unknown,
  mapNamesById: ReadonlyMap<string, string>,
  speciesLabelsById: ReadonlyMap<string, string>,
  selectorMapIds: ReadonlySet<string> = new Set(),
  spriteForSpecies: (speciesId: string) => CatalogEncounterSprite | null = () => null,
  runtimeConfig: EncounterRuntimeConfig = defaultRuntimeConfig,
  projection?: CatalogWildEncounterProjection,
): Map<string, CatalogWildEncounters> => {
  const groupFields = fieldsFor(document)
  const groups = (document as SourceEncounterDocument).wild_encounter_groups
  const profiles = projection ? profileIndex(projection) : undefined
  const consumedProfiles = new Set<string>()
  const byMap = new Map<string, CatalogWildEncounters>()
  for (const [groupIndex, group] of groups.entries()) {
    const groupPointer = `/wild_encounter_groups/${groupIndex}`
    if (group.for_maps !== true) continue
    const setsByMapId = new Map<string, CatalogEncounterSet[]>()
    const diagnosticsByHeaderIndex = new Map<number, CatalogWildEncounters["diagnostics"]>()
    for (const [headerIndex, encounter] of group.encounters.entries()) {
      const encounterPointer = `${groupPointer}/encounters/${headerIndex}`
      if (!isRecord(encounter)) throw sourceError(encounterPointer, "expected an object")
      const mapId = requireString(encounter.map, `${encounterPointer}/map`)
      const baseLabel = requireString(encounter.base_label, `${encounterPointer}/base_label`)
      const sourceProduct = sourceProductForBaseLabel(baseLabel)
      const diagnostics: CatalogWildEncounters["diagnostics"] = []
      const methodProfiles: CatalogEncounterProjectionProfile[][] = []
      const methods = groupFields[groupIndex]!.filter(
        (metadata) => encounter[metadata.field.type] !== null && metadata.field.type in encounter,
      ).map((metadata) => {
        const methodType = metadata.field.type
        const methodPointer = `${encounterPointer}/${methodType}`
        const method = methodFor(encounter[methodType], methodPointer)
        const joined = joinedProfiles(
          sourceProduct,
          baseLabel,
          mapId,
          metadata,
          method,
          profiles,
          consumedProfiles,
        )
        methodProfiles.push(joined)
        return {
          type: methodType,
          encounterRate: method.encounter_rate,
          source: sourcePointer(methodPointer),
          slots: method.mons.flatMap((slot, slotIndex) => {
            const source = sourcePointer(`${methodPointer}/mons/${slotIndex}`)
            if (slot.min_level > slot.max_level) {
              diagnostics.push({
                code: "invalid_source_slot",
                reason: "invalid_level_range",
                setBaseLabel: baseLabel,
                methodType,
                slotIndex,
                speciesId: slot.species,
                minLevel: slot.min_level,
                maxLevel: slot.max_level,
                source,
              })
            }
            if (slotIndex >= metadata.field.encounter_rates.length) {
              diagnostics.push({
                code: "unaddressable_source_slot",
                reason: "outside_method_slot_table",
                setBaseLabel: baseLabel,
                methodType,
                slotIndex,
                speciesId: slot.species,
                minLevel: slot.min_level,
                maxLevel: slot.max_level,
                source,
              })
              return []
            }
            const slotRate = metadata.field.encounter_rates[slotIndex]!
            const reason =
              slot.species === "SPECIES_NONE"
                ? "species_none"
                : slotRate === 0
                  ? "zero_slot_rate"
                  : null
            if (reason) {
              diagnostics.push({
                code: "excluded_source_slot",
                reason,
                setBaseLabel: baseLabel,
                methodType,
                slotIndex,
                speciesId: slot.species,
                slotRate,
                source,
              })
              return []
            }
            const speciesLabel = speciesLabelsById.get(slot.species)
            if (!speciesLabel) {
              throw sourceError(
                `${methodPointer}/mons/${slotIndex}/species`,
                "has no Pokémon Wayfarer species label source entry",
              )
            }
            return [
              {
                slotIndex,
                slotRate,
                slotRateSource: sourcePointer(
                  `/wild_encounter_groups/${metadata.groupIndex}/fields/${metadata.fieldIndex}/encounter_rates/${slotIndex}`,
                ),
                groups: groupsForSlot(metadata, slotIndex),
                minLevel: slot.min_level,
                maxLevel: slot.max_level,
                runtimeMinLevel: Math.min(slot.min_level, slot.max_level),
                runtimeMaxLevel: Math.max(slot.min_level, slot.max_level),
                speciesId: slot.species,
                speciesLabel,
                sprite: spriteForSpecies(slot.species),
                source,
              },
            ]
          }),
          profiles: joined.map((profile) => ({
            profileKey: profile.profileKey,
            fishingRod: profile.fishingRod,
            levelOffset: profile.levelOffset,
          })),
        }
      })
      const joined = methodProfiles.flat()
      const products = new Set(joined.map((profile) => profile.product))
      const runtimeTimes = new Set(joined.map((profile) => profile.runtimeTime))
      if (profiles && (products.size !== 1 || runtimeTimes.size !== 1)) {
        throw sourceError(
          encounterPointer,
          "projection method profiles must resolve to one product and runtime time",
        )
      }
      if (profiles && joined.some((profile) => profile.product !== sourceProduct)) {
        throw sourceError(encounterPointer, "projection product does not match the source label")
      }
      const product = joined[0]?.product ?? sourceProduct
      const runtimeTime = joined[0]
        ? timeOfDayForProfile(joined[0].runtimeTime)
        : sourceTimeForBaseLabel(baseLabel, runtimeConfig)
      const mapName = mapNamesById.get(mapId)
      if (!mapName) continue
      const set: CatalogEncounterSet = {
        mapId,
        mapName,
        baseLabel,
        product,
        runtimeTime,
        header: { groupLabel: group.label, groupIndex, headerIndex },
        source: sourcePointer(encounterPointer),
        methods,
      }
      const mapSets = setsByMapId.get(mapId) ?? []
      mapSets.push(set)
      setsByMapId.set(mapId, mapSets)
      diagnosticsByHeaderIndex.set(headerIndex, diagnostics)
    }
    for (const [mapId, sets] of setsByMapId) {
      const mapName = mapNamesById.get(mapId)!
      const current = byMap.get(mapName) ?? { sets: [], runtimeTimes: [], diagnostics: [] }
      current.sets.push(...sets)
      current.diagnostics.push(
        ...sets.flatMap((set) => diagnosticsByHeaderIndex.get(set.header.headerIndex) ?? []),
      )
      if (group.label === "gWildMonHeaders" && !selectorMapIds.has(mapId)) {
        current.runtimeTimes = runtimeTimesFor(sets, runtimeConfig)
      }
      byMap.set(mapName, current)
    }
  }
  if (projection && consumedProfiles.size !== projection.profiles.length) {
    const missing = projection.profiles.find((profile) => !consumedProfiles.has(profile.profileKey))
    throw sourceError("", `projection profile was not joined: ${missing?.profileKey ?? "unknown"}`)
  }
  return byMap
}

export const sourceWildEncounters = (
  root: string,
  mapNamesById: ReadonlyMap<string, string>,
  spriteForSpecies?: (speciesId: string) => CatalogEncounterSprite | null,
): Map<string, CatalogWildEncounters> => {
  const filePath = path.join(root, wildEncounterPath)
  return catalogWildEncounters(
    JSON.parse(fs.readFileSync(filePath, "utf8")),
    mapNamesById,
    sourceSpeciesLabels(root),
    runtimeSelectorMapIds(root),
    spriteForSpecies,
    sourceEncounterRuntimeConfig(root),
  )
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
  const speciesLabels = sourceSpeciesLabels(root)
  const projection = readWildEncounterProjection(projectionPath, speciesLabels, spriteForSpecies)
  const encountersByMap = catalogWildEncounters(
    JSON.parse(fs.readFileSync(path.join(root, wildEncounterPath), "utf8")),
    mapNamesById,
    speciesLabels,
    runtimeSelectorMapIds(root),
    spriteForSpecies,
    sourceEncounterRuntimeConfig(root),
    projection,
  )
  return { encountersByMap, projection }
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
      /^\s*\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*\{[\s\S]*?\.speciesName\s*=\s*_\("([^"]*)"\)/gm,
    )
    for (const [, speciesId, speciesLabel] of entries) {
      speciesLabels.set(speciesId!, speciesLabel!)
    }
  }
  const macroLabels = new Map<string, string>()
  for (const source of sourceTexts) {
    for (const [, macro, body] of source.matchAll(
      /^\s*#define\s+(\w+)\([^)]*\)([\s\S]*?)(?=^\s*(?:#(?:define|if|endif)|\[\s*SPECIES_)|(?![\s\S]))/gm,
    )) {
      const label = body?.match(/\.speciesName\s*=\s*_\("([^"]*)"\)/)?.[1]
      if (macro && label) macroLabels.set(macro, label)
    }
  }
  for (const source of sourceTexts) {
    for (const [, speciesId, macro] of source.matchAll(
      /^\s*\[\s*(SPECIES_[A-Z0-9_]+)\s*\]\s*=\s*(\w+)\s*\(/gm,
    )) {
      const label = macro ? macroLabels.get(macro) : undefined
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

/**
 * Some maps select encounter headers at runtime after the normal map lookup. Keep their source
 * rows visible without presenting them as a time-of-day resolution.
 */
const runtimeSelectorMapIds = (root: string): ReadonlySet<string> => {
  const runtimeSource = fs.readFileSync(path.join(root, wildEncounterRuntimePath), "utf8")
  const selectorMapIds = new Set<string>()
  for (const match of runtimeSource.matchAll(
    /MAP_GROUP\(\s*(MAP_\w+)\s*\)[\s\S]{0,500}?\bi\s*\+=/g,
  )) {
    if (match[1]) selectorMapIds.add(match[1])
  }
  return selectorMapIds
}

const sourceEncounterRuntimeConfig = (root: string): EncounterRuntimeConfig => {
  const rtcConstants = fs.readFileSync(path.join(root, rtcConstantsPath), "utf8")
  const overworldConfig = fs.readFileSync(path.join(root, overworldConfigPath), "utf8")
  const enumBody = /enum\s+TimeOfDay\s*\{([\s\S]*?)\};/.exec(rtcConstants)?.[1] ?? ""
  const labels = new Map<RuntimeTimeId, string>()
  for (const [, constant] of enumBody.matchAll(/\bTIME_(MORNING|DAY|EVENING|NIGHT)\b/g)) {
    const id = constant!.toLowerCase() as RuntimeTimeId
    labels.set(id, constant![0] + constant!.slice(1).toLowerCase())
  }
  const fallback = /#define\s+OW_TIME_OF_DAY_FALLBACK\s+TIME_(MORNING|DAY|EVENING|NIGHT)\b/.exec(
    overworldConfig,
  )?.[1]
  const enabled = /#define\s+OW_TIME_OF_DAY_ENCOUNTERS\s+TRUE\b/.test(overworldConfig)
  const disableFallback = /#define\s+OW_TIME_OF_DAY_DISABLE_FALLBACK\s+TRUE\b/.test(overworldConfig)
  return {
    enabled,
    disableFallback,
    fallbackTime: (fallback?.toLowerCase() as RuntimeTimeId | undefined) ?? "day",
    labels: labels.size > 0 ? labels : defaultRuntimeConfig.labels,
  }
}
