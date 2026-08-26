import type { CatalogWildEncounterMethod, CatalogWildEncounterSlot } from "./catalog.js"

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
