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

export type ProjectedEncounterOutcome = {
  authoredMinimumLevel: number
  authoredMaximumLevel: number
  projectedMinimumLevel: number
  projectedMaximumLevel: number
  speciesId: string
  speciesLabel: string
  sprite: CatalogEncounterSprite | null
  eligible: boolean
  minimumOrdinaryWildLevel: number
}

export type ResolvedEncounterSlot = {
  source: CatalogWildEncounterSlot
  outcomes: ProjectedEncounterOutcome[]
  eligible: boolean
  selectionWeight: number | null
}

type ProjectionIndex = {
  speciesById: Map<string, CatalogWildEncounterProjection["species"][number]>
  levelsByOffsetAndRating: Map<string, readonly number[]>
}

const projectionIndexes = new WeakMap<CatalogWildEncounterProjection, ProjectionIndex>()

const projectionIndex = (projection: CatalogWildEncounterProjection): ProjectionIndex => {
  const existing = projectionIndexes.get(projection)
  if (existing) return existing
  const index = {
    speciesById: new Map(projection.species.map((species) => [species.authoredSpecies, species])),
    levelsByOffsetAndRating: new Map(
      projection.levelProjections.flatMap((table) =>
        table.ratings.map((row) => [`${table.levelOffset}/${row.rating}`, row.projectedLevels]),
      ),
    ),
  }
  projectionIndexes.set(projection, index)
  return index
}

export const visibleEncounterSlots = (
  method: CatalogWildEncounterMethod,
): CatalogWildEncounterSlot[] => {
  return method.slots.filter((slot) => slot.speciesId !== "SPECIES_NONE" && slot.slotRate > 0)
}

export const fishingGroupIds = (method: CatalogWildEncounterMethod): string[] => {
  return [
    ...new Set(
      visibleEncounterSlots(method).flatMap((slot) => slot.groups.map((group) => group.id)),
    ),
  ]
}

export const rodLabel = (groupId: string): string => {
  return groupId
    .split("_")
    .map((word) => `${word.slice(0, 1).toUpperCase()}${word.slice(1)}`)
    .join(" ")
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

const projectionFor = (
  projection: CatalogWildEncounterProjection,
  levelOffset: number,
  rating: number,
  authoredLevel: number,
): number | null => {
  const levels = projectionIndex(projection).levelsByOffsetAndRating.get(`${levelOffset}/${rating}`)
  return levels?.[authoredLevel - projection.authoredLevel.minimum] ?? null
}

const profileFor = (
  method: CatalogWildEncounterMethod,
  groupId: string | null,
): CatalogWildEncounterMethod["profiles"][number] | null => {
  if (method.type !== "fishing_mons") return method.profiles[0] ?? null
  return method.profiles.find((profile) => profile.fishingRod === groupId?.toUpperCase()) ?? null
}

const slotOutcomes = (
  projection: CatalogWildEncounterProjection,
  method: CatalogWildEncounterMethod,
  slot: CatalogWildEncounterSlot,
  rating: number,
  groupId: string | null,
): ProjectedEncounterOutcome[] => {
  const profile = profileFor(method, groupId)
  const speciesById = projectionIndex(projection).speciesById
  const authoredSpecies = speciesById.get(slot.speciesId)
  if (!profile || !authoredSpecies) return []

  const rows: Array<ProjectedEncounterOutcome & { authoredLevel: number }> = []
  for (
    let authoredLevel = slot.runtimeMinLevel;
    authoredLevel <= slot.runtimeMaxLevel;
    authoredLevel += 1
  ) {
    const projectedLevel = projectionFor(projection, profile.levelOffset, rating, authoredLevel)
    if (projectedLevel === null) continue
    const outcome = authoredSpecies.outcomesByProjectedLevel.find(
      (candidate) =>
        projectedLevel >= candidate.minimumProjectedLevel &&
        projectedLevel <= candidate.maximumProjectedLevel,
    )
    if (!outcome) continue
    const effectiveSpecies = speciesById.get(outcome.effectiveSpecies)
    rows.push({
      authoredLevel,
      authoredMinimumLevel: authoredLevel,
      authoredMaximumLevel: authoredLevel,
      projectedMinimumLevel: projectedLevel,
      projectedMaximumLevel: projectedLevel,
      speciesId: outcome.effectiveSpecies,
      speciesLabel: effectiveSpecies?.speciesLabel ?? outcome.effectiveSpecies,
      sprite: effectiveSpecies?.sprite ?? null,
      eligible: outcome.eligible,
      minimumOrdinaryWildLevel: outcome.minimumOrdinaryWildLevel,
    })
  }

  const ranges: ProjectedEncounterOutcome[] = []
  for (const row of rows) {
    const previous = ranges.at(-1)
    if (
      previous &&
      previous.speciesId === row.speciesId &&
      previous.eligible === row.eligible &&
      previous.minimumOrdinaryWildLevel === row.minimumOrdinaryWildLevel &&
      previous.authoredMaximumLevel + 1 === row.authoredLevel &&
      row.projectedMinimumLevel >= previous.projectedMaximumLevel
    ) {
      previous.authoredMaximumLevel = row.authoredLevel
      previous.projectedMinimumLevel = Math.min(
        previous.projectedMinimumLevel,
        row.projectedMinimumLevel,
      )
      previous.projectedMaximumLevel = Math.max(
        previous.projectedMaximumLevel,
        row.projectedMaximumLevel,
      )
    } else {
      const { authoredLevel: _authoredLevel, ...range } = row
      ranges.push(range)
    }
  }
  return ranges
}

export const resolveMethodSlots = (
  projection: CatalogWildEncounterProjection,
  method: CatalogWildEncounterMethod,
  rating: number,
  groupId: string | null = null,
): ResolvedEncounterSlot[] => {
  const slots = visibleEncounterSlots(method).filter(
    (slot) =>
      method.type !== "fishing_mons" ||
      (groupId === null
        ? slot.groups.length === 0
        : slot.groups.some((group) => group.id === groupId)),
  )
  const projected = slots.map((source) => {
    const outcomes = slotOutcomes(projection, method, source, rating, groupId)
    return {
      source,
      outcomes,
      eligible: outcomes.length > 0 && outcomes.every((row) => row.eligible),
    }
  })
  const denominator = projected
    .filter((slot) => slot.eligible)
    .reduce((sum, slot) => sum + slot.source.slotRate, 0)
  return projected.map((slot) => ({
    ...slot,
    selectionWeight: slot.eligible && denominator > 0 ? slot.source.slotRate / denominator : null,
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
        const groupIds = method.type === "fishing_mons" ? fishingGroupIds(method) : [null]
        return groupIds.flatMap((groupId) =>
          resolveMethodSlots(projection, method, rating, groupId).flatMap((slot) =>
            slot.eligible ? slot.outcomes : [],
          ),
        )
      }),
  )
  return [...new Map(outcomes.map((outcome) => [outcome.speciesId, outcome])).values()]
}
