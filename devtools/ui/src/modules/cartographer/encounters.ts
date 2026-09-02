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
  rawWeight: number
  fishingRod: string | null
  selectionWeight: number | null
}

export type EncounterRosterMethodType = Extract<
  CatalogWildEncounterMethod["type"],
  "land_mons" | "water_mons"
>

export type EncounterRosterActivation = {
  timeOfDay: CatalogWildEncounterRuntimeTime["timeOfDay"]
  resolution: Extract<
    CatalogWildEncounterRuntimeTime["methods"][number]["resolution"],
    "direct" | "fallback"
  >
}

export type EncounterRosterSlot = ResolvedEncounterSlot & {
  eligible: true
  selectionWeight: number
}

export type EncounterRosterSource = {
  key: string
  set: CatalogWildEncounterSet
  method: CatalogWildEncounterMethod
  activations: EncounterRosterActivation[]
  slots: ResolvedEncounterSlot[]
  effectiveSlots: EncounterRosterSlot[]
  lockedSlotCount: number
}

export type ResolvedEncounterPopulation = {
  method: EncounterRosterMethodType
  sources: EncounterRosterSource[]
  unavailableTimes: CatalogWildEncounterRuntimeTime["timeOfDay"][]
}

type ProjectionIndex = {
  speciesById: Map<string, CatalogWildEncounterProjection["species"][number]>
  levelsByOffsetAndRating: Map<string, readonly number[]>
  profilesByKey: Map<string, CatalogWildEncounterProjection["profiles"][number]>
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
    profilesByKey: new Map(projection.profiles.map((profile) => [profile.profileKey, profile])),
  }
  projectionIndexes.set(projection, index)
  return index
}

export const visibleEncounterSlots = (
  method: CatalogWildEncounterMethod,
): CatalogWildEncounterSlot[] => {
  if (method.type === "fishing_mons") return method.slots
  return method.slots.filter((slot) => slot.speciesId !== "SPECIES_NONE" && slot.slotRate > 0)
}

export const fishingRarityBandIds = (method: CatalogWildEncounterMethod): string[] => {
  return [
    ...new Set(
      visibleEncounterSlots(method).flatMap((slot) => slot.groups.map((group) => group.id)),
    ),
  ]
}

export const rodLabel = (groupId: string): string => {
  return groupId
    .toLowerCase()
    .split("_")
    .map((word) => `${word.slice(0, 1).toUpperCase()}${word.slice(1)}`)
    .join(" ")
}

const rarityBandLabels: Readonly<Record<string, string>> = {
  old_rod: "Common",
  good_rod: "Less common",
  super_rod: "Rare",
}

export const rarityBandLabel = (slot: CatalogWildEncounterSlot): string => {
  const labels = slot.groups
    .map((group) => rarityBandLabels[group.id])
    .filter((label): label is string => label !== undefined)
  return [...new Set(labels)].join(", ") || "Unbanded"
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
  fishingRod: string | null,
): CatalogWildEncounterMethod["profiles"][number] | null => {
  if (method.type !== "fishing_mons") return method.profiles[0] ?? null
  return method.profiles.find((profile) => profile.fishingRod === fishingRod) ?? null
}

const slotOutcomes = (
  projection: CatalogWildEncounterProjection,
  method: CatalogWildEncounterMethod,
  slot: CatalogWildEncounterSlot,
  rating: number,
  fishingRod: string | null,
): ProjectedEncounterOutcome[] => {
  const profile = profileFor(method, fishingRod)
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
  fishingRod: string | null = null,
): ResolvedEncounterSlot[] => {
  const slots = visibleEncounterSlots(method)
  const profileReference = profileFor(method, fishingRod)
  const profile = profileReference
    ? projectionIndex(projection).profilesByKey.get(profileReference.profileKey)
    : undefined
  const projected = slots.map((source) => {
    const outcomes = slotOutcomes(projection, method, source, rating, fishingRod)
    const rawWeight =
      method.type === "fishing_mons" ? (profile?.weights?.[source.slotIndex] ?? 0) : source.slotRate
    return {
      source,
      outcomes,
      rawWeight,
      fishingRod: method.type === "fishing_mons" ? fishingRod : null,
      eligible:
        source.speciesId !== "SPECIES_NONE" &&
        rawWeight > 0 &&
        outcomes.length > 0 &&
        outcomes.every((row) => row.eligible),
    }
  })
  const denominator = projected
    .filter((slot) => slot.eligible)
    .reduce((sum, slot) => sum + slot.rawWeight, 0)
  return projected.map((slot) => ({
    ...slot,
    selectionWeight: slot.eligible && denominator > 0 ? slot.rawWeight / denominator : null,
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
          resolveMethodSlots(projection, method, rating, fishingRod).flatMap((slot) =>
            slot.eligible ? slot.outcomes : [],
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
      const slots = resolveMethodSlots(projection, method, rating)
      const effectiveSlots = slots.filter(
        (slot): slot is EncounterRosterSlot => slot.eligible && slot.selectionWeight !== null,
      )
      return [
        {
          key: `${sourceSet.product}/${sourceSet.baseLabel}/${sourceSet.source.path}${sourceSet.source.pointer}/${method.type}/${methodIndex}`,
          set: sourceSet,
          method,
          activations: activationsFor(encounterSet, sourceSet, methodType),
          slots,
          effectiveSlots,
          lockedSlotCount: slots.length - effectiveSlots.length,
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
