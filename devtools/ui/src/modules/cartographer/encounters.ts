import type {
  CatalogEncounterSprite,
  CatalogMap,
  CatalogWildEncounterMethod,
  CatalogWildEncounterProjection,
  CatalogWildEncounterRuntimeTime,
  CatalogWildEncounterSet,
  CatalogWildEncounterSlot,
} from "./catalog.js"

export type ResolvedMapEncounters = {
  availableProducts: Array<{ id: string; displayName: string }>
  product: string | null
  sets: CatalogWildEncounterSet[]
  runtimeTimes: CatalogWildEncounterRuntimeTime[]
}

/** One species a slot can roll, over a run of consecutive levels. */
export type ProjectedEncounterOutcome = {
  projectedMinimumLevel: number
  projectedMaximumLevel: number
  speciesId: string
  speciesLabel: string
  sprite: CatalogEncounterSprite | null
  /** Chance within the slot, from 0 (exclusive) through 1. */
  chance: number
}

export type ResolvedEncounterSlot = {
  source: CatalogWildEncounterSlot
  outcomes: ProjectedEncounterOutcome[]
  rawWeight: number
  fishingRod: string | null
  /** Chance that the method picks this slot. */
  selectionWeight: number
}

export type EncounterRosterMethodType = Extract<
  CatalogWildEncounterMethod["type"],
  "land_mons" | "water_mons"
>

export type EncounterRosterActivation = {
  timeOfDay: CatalogWildEncounterRuntimeTime["timeOfDay"]
  resolution: Extract<CatalogWildEncounterRuntimeTime["methods"][number]["resolution"], "direct">
}

export type EncounterRosterSource = {
  key: string
  set: CatalogWildEncounterSet
  method: CatalogWildEncounterMethod
  activations: EncounterRosterActivation[]
  placeLevel: number
  slots: ResolvedEncounterSlot[]
}

export type ResolvedEncounterPopulation = {
  method: EncounterRosterMethodType
  sources: EncounterRosterSource[]
  unavailableTimes: CatalogWildEncounterRuntimeTime["timeOfDay"][]
}

type ProjectionIndex = {
  speciesByIndex: CatalogWildEncounterProjection["species"]
  distributions: Map<string, CatalogWildEncounterProjection["distributions"][number]>
}

const projectionIndexes = new WeakMap<CatalogWildEncounterProjection, ProjectionIndex>()

const projectionIndex = (projection: CatalogWildEncounterProjection): ProjectionIndex => {
  const existing = projectionIndexes.get(projection)
  if (existing) return existing
  const index = {
    speciesByIndex: projection.species,
    distributions: new Map(
      projection.distributions.map((entry) => [`${entry.speciesId}/${entry.regionClass}`, entry]),
    ),
  }
  projectionIndexes.set(projection, index)
  return index
}

/** The place level of a set at a Trainer Rating; ratings past the table use its last entry. */
export const placeLevelAt = (
  projection: CatalogWildEncounterProjection,
  set: CatalogWildEncounterSet,
  rating: number,
): number => {
  const levels = set.place.placeLevels
  const index = Math.min(
    levels.length - 1,
    Math.max(0, Math.round(rating) - projection.trainerRating.minimum),
  )
  return levels[index]!
}

/** "Road", "Wilds", "Outlands" or the dungeon's intent and floor, as a short label. */
export const placeLabel = (place: CatalogWildEncounterSet["place"]): string => {
  if (place.reach !== "Dungeon") return place.reach
  const floor =
    place.floors && place.floors > 1 ? ` · floor ${(place.floor ?? 0) + 1} of ${place.floors}` : ""
  return `Dungeon (${place.dungeon?.intent.toLowerCase() ?? "unknown"}${place.dungeon?.flat ? ", flat" : ""})${floor}`
}

export const rodLabel = (groupId: string): string => {
  return groupId
    .toLowerCase()
    .split("_")
    .map((word) => `${word.slice(0, 1).toUpperCase()}${word.slice(1)}`)
    .join(" ")
}

export const fishingProfiles = (
  method: CatalogWildEncounterMethod,
): CatalogWildEncounterMethod["profiles"] => {
  if (method.type !== "fishing_mons") return []
  return method.profiles.filter((profile) => profile.fishingRod !== "NONE")
}

export const resolveMapEncounters = (
  map: CatalogMap,
  preferredProduct: string | null,
  products: readonly { id: string; displayName: string }[],
): ResolvedMapEncounters => {
  const availableIds = [...new Set(map.wildEncounters.sets.map((set) => set.product))]
  const availableProducts = availableIds.map(
    (id) => products.find((product) => product.id === id) ?? { id, displayName: id },
  )
  const product =
    (preferredProduct && availableIds.includes(preferredProduct) ? preferredProduct : null) ??
    availableIds[0] ??
    null
  return {
    availableProducts,
    product,
    sets: product ? map.wildEncounters.sets.filter((set) => set.product === product) : [],
    runtimeTimes: product
      ? map.wildEncounters.runtimeTimes.filter((time) => time.product === product)
      : [],
  }
}

const profileFor = (
  method: CatalogWildEncounterMethod,
  fishingRod: string | null,
): CatalogWildEncounterMethod["profiles"][number] | null => {
  if (method.type !== "fishing_mons") return method.profiles[0] ?? null
  return method.profiles.find((profile) => profile.fishingRod === fishingRod) ?? null
}

const slotOutcomes = (
  projection: CatalogWildEncounterProjection,
  set: CatalogWildEncounterSet,
  slot: CatalogWildEncounterSlot,
  rating: number,
): ProjectedEncounterOutcome[] => {
  const { speciesByIndex, distributions } = projectionIndex(projection)
  const rolled = distributions.get(`${slot.speciesId}/${set.place.regionClass}`)?.byPlaceLevel[
    String(placeLevelAt(projection, set, rating))
  ]
  if (!rolled) return []

  const bySpecies = new Map<number, Map<number, number>>()
  for (const [species, level, weight] of rolled) {
    const levels = bySpecies.get(species) ?? new Map<number, number>()
    levels.set(level, (levels.get(level) ?? 0) + weight)
    bySpecies.set(species, levels)
  }
  const outcomes: ProjectedEncounterOutcome[] = []
  for (const [species, levels] of bySpecies) {
    const metadata = speciesByIndex[species]
    if (!metadata) continue
    let previous: ProjectedEncounterOutcome | undefined
    for (const [level, weight] of [...levels].sort(([left], [right]) => left - right)) {
      const chance = weight / projection.outcomeDenominator
      if (previous && previous.projectedMaximumLevel + 1 === level) {
        previous.projectedMaximumLevel = level
        previous.chance += chance
      } else {
        previous = {
          projectedMinimumLevel: level,
          projectedMaximumLevel: level,
          speciesId: metadata.speciesId,
          speciesLabel: metadata.speciesLabel,
          sprite: metadata.sprite,
          chance,
        }
        outcomes.push(previous)
      }
    }
  }
  return outcomes
}

export const resolveMethodSlots = (
  projection: CatalogWildEncounterProjection,
  set: CatalogWildEncounterSet,
  method: CatalogWildEncounterMethod,
  rating: number,
  fishingRod: string | null = null,
): ResolvedEncounterSlot[] => {
  const profile = profileFor(method, fishingRod)
  const projected = method.slots.map((source) => ({
    source,
    outcomes: slotOutcomes(projection, set, source, rating),
    rawWeight: profile?.weights[source.slotIndex] ?? 0,
    fishingRod: method.type === "fishing_mons" ? fishingRod : null,
  }))
  const denominator = projected.reduce((sum, slot) => sum + slot.rawWeight, 0)
  return projected.map((slot) => ({
    ...slot,
    selectionWeight: denominator > 0 ? slot.rawWeight / denominator : 0,
  }))
}

export const effectiveRosterFor = (
  projection: CatalogWildEncounterProjection,
  encounterSet: ResolvedMapEncounters,
  methodType: CatalogWildEncounterMethod["type"],
  rating: number,
): ProjectedEncounterOutcome[] => {
  const outcomes = encounterSet.sets.flatMap((set) =>
    set.methods
      .filter((method) => method.type === methodType)
      .flatMap((method) => {
        const profileRods =
          method.type === "fishing_mons"
            ? fishingProfiles(method).map((profile) => profile.fishingRod)
            : [null]
        return profileRods.flatMap((fishingRod) =>
          resolveMethodSlots(projection, set, method, rating, fishingRod).flatMap(
            (slot) => slot.outcomes,
          ),
        )
      }),
  )
  return [...new Map(outcomes.map((outcome) => [outcome.speciesId, outcome])).values()]
}

const sameSource = (
  left: CatalogWildEncounterSet["source"],
  right: CatalogWildEncounterSet["source"],
): boolean => left.path === right.path && left.pointer === right.pointer

const activationsFor = (
  encounterSet: ResolvedMapEncounters,
  sourceSet: CatalogWildEncounterSet,
  methodType: EncounterRosterMethodType,
): EncounterRosterActivation[] => {
  const uses = encounterSet.runtimeTimes.flatMap((runtimeTime) =>
    runtimeTime.methods
      .filter((method) => method.type === methodType && method.resolution !== "unavailable")
      .filter((method) =>
        method.sets.some(
          (set) =>
            set.baseLabel === sourceSet.baseLabel && sameSource(set.source, sourceSet.source),
        ),
      )
      .map((method) => ({
        timeOfDay: runtimeTime.timeOfDay,
        resolution: method.resolution as EncounterRosterActivation["resolution"],
      })),
  )
  return [...new Map(uses.map((use) => [`${use.timeOfDay}/${use.resolution}`, use])).values()]
}

export const resolveEncounterPopulation = (
  projection: CatalogWildEncounterProjection,
  encounterSet: ResolvedMapEncounters,
  methodType: EncounterRosterMethodType,
  rating: number,
): ResolvedEncounterPopulation => {
  const sources = encounterSet.sets.flatMap((sourceSet) =>
    sourceSet.methods.flatMap((method, methodIndex) => {
      if (method.type !== methodType) return []
      const slots = resolveMethodSlots(projection, sourceSet, method, rating)
      return [
        {
          key: `${sourceSet.product}/${sourceSet.baseLabel}/${sourceSet.source.path}${sourceSet.source.pointer}/${method.type}/${methodIndex}`,
          set: sourceSet,
          method,
          activations: activationsFor(encounterSet, sourceSet, methodType),
          placeLevel: placeLevelAt(projection, sourceSet, rating),
          slots,
        },
      ]
    }),
  )
  const unavailableTimes = encounterSet.runtimeTimes.flatMap((runtimeTime) =>
    runtimeTime.methods.some(
      (method) => method.type === methodType && method.resolution === "unavailable",
    )
      ? [runtimeTime.timeOfDay]
      : [],
  )
  return {
    method: methodType,
    sources,
    unavailableTimes: [...new Set(unavailableTimes)],
  }
}
