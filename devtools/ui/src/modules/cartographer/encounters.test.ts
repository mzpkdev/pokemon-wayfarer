import { describe, expect, it } from "vitest"

import { fishingGroupIds, rodLabel, visibleEncounterSlots } from "./encounters.js"
import type { CatalogWildEncounterMethod } from "./catalog.js"

const source = { path: "src/data/wild_encounters.json", pointer: "/wild_encounter_groups/0" }

const fishing: CatalogWildEncounterMethod = {
  type: "fishing_mons",
  encounterRate: 20,
  source,
  slots: [
    {
      slotIndex: 0,
      slotRate: 70,
      slotRateSource: source,
      groups: [{ id: "old_rod", source }],
      minLevel: 5,
      maxLevel: 5,
      speciesId: "SPECIES_MAGIKARP",
      speciesLabel: "Magikarp",
      sprite: null,
      source,
    },
    {
      slotIndex: 1,
      slotRate: 0,
      slotRateSource: source,
      groups: [{ id: "good_rod", source }],
      minLevel: 10,
      maxLevel: 10,
      speciesId: "SPECIES_NONE",
      speciesLabel: "None",
      sprite: null,
      source,
    },
    {
      slotIndex: 2,
      slotRate: 30,
      slotRateSource: source,
      groups: [{ id: "good_rod", source }],
      minLevel: 15,
      maxLevel: 20,
      speciesId: "SPECIES_GOLDEEN",
      speciesLabel: "Goldeen",
      sprite: null,
      source,
    },
  ],
}

describe("encounter presentation", () => {
  it("omits sentinel and zero-weight slots before rendering", () => {
    expect(visibleEncounterSlots(fishing).map((slot) => slot.speciesLabel)).toEqual([
      "Magikarp",
      "Goldeen",
    ])
  })

  it("keeps fishing visibly grouped by the source rod labels", () => {
    expect(fishingGroupIds(fishing)).toEqual(["old_rod", "good_rod"])
    expect(rodLabel("super_rod")).toBe("Super Rod")
  })
})
