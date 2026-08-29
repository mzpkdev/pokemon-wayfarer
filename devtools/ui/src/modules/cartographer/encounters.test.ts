import { describe, expect, it } from "vitest"

import {
  effectiveRosterFor,
  fishingGroupIds,
  resolveMethodSlots,
  rodLabel,
  type ResolvedMapEncounters,
  visibleEncounterSlots,
} from "./encounters.js"
import type { CatalogWildEncounterMethod, CatalogWildEncounterProjection } from "./catalog.js"

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
      runtimeMinLevel: 5,
      runtimeMaxLevel: 5,
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
      runtimeMinLevel: 10,
      runtimeMaxLevel: 10,
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
      runtimeMinLevel: 15,
      runtimeMaxLevel: 20,
      speciesId: "SPECIES_GOLDEEN",
      speciesLabel: "Goldeen",
      sprite: null,
      source,
    },
  ],
  profiles: [
    { profileKey: "old", fishingRod: "OLD_ROD", levelOffset: 0 },
    { profileKey: "good", fishingRod: "GOOD_ROD", levelOffset: 0 },
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

  const identity = Array.from({ length: 100 }, (_, index) => index + 1)
  const lowRating = Array.from({ length: 100 }, (_, index) =>
    Math.max(1, Math.ceil((index + 1) / 2)),
  )
  const projection: CatalogWildEncounterProjection = {
    schemaVersion: 1,
    trainerRating: { minimum: 10, maximum: 80 },
    authoredLevel: { minimum: 1, maximum: 100 },
    products: [{ id: "hns", displayName: "HeartGold and SoulSilver" }],
    levelProjections: [
      { levelOffset: 0, ratings: [{ rating: 10, projectedLevels: identity }] },
      {
        levelOffset: 1,
        ratings: [
          { rating: 10, projectedLevels: lowRating },
          { rating: 30, projectedLevels: identity },
        ],
      },
    ],
    species: [
      {
        authoredSpecies: "SPECIES_SKARMORY",
        authoredSpeciesId: 1,
        speciesLabel: "Skarmory",
        sprite: null,
        outcomesByProjectedLevel: [
          {
            minimumProjectedLevel: 1,
            maximumProjectedLevel: 14,
            effectiveSpecies: "SPECIES_SKARMORY",
            eligible: false,
            minimumOrdinaryWildLevel: 15,
          },
          {
            minimumProjectedLevel: 15,
            maximumProjectedLevel: 100,
            effectiveSpecies: "SPECIES_SKARMORY",
            eligible: true,
            minimumOrdinaryWildLevel: 15,
          },
        ],
      },
      {
        authoredSpecies: "SPECIES_RATTATA",
        authoredSpeciesId: 2,
        speciesLabel: "Rattata",
        sprite: null,
        outcomesByProjectedLevel: [
          {
            minimumProjectedLevel: 1,
            maximumProjectedLevel: 100,
            effectiveSpecies: "SPECIES_RATTATA",
            eligible: true,
            minimumOrdinaryWildLevel: 1,
          },
        ],
      },
      {
        authoredSpecies: "SPECIES_GOLDEEN",
        authoredSpeciesId: 3,
        speciesLabel: "Goldeen",
        sprite: null,
        outcomesByProjectedLevel: [
          {
            minimumProjectedLevel: 1,
            maximumProjectedLevel: 100,
            effectiveSpecies: "SPECIES_GOLDEEN",
            eligible: true,
            minimumOrdinaryWildLevel: 1,
          },
        ],
      },
      {
        authoredSpecies: "SPECIES_MAGIKARP",
        authoredSpeciesId: 4,
        speciesLabel: "Magikarp",
        sprite: null,
        outcomesByProjectedLevel: [
          {
            minimumProjectedLevel: 1,
            maximumProjectedLevel: 100,
            effectiveSpecies: "SPECIES_MAGIKARP",
            eligible: true,
            minimumOrdinaryWildLevel: 1,
          },
        ],
      },
      {
        authoredSpecies: "SPECIES_GYARADOS",
        authoredSpeciesId: 5,
        speciesLabel: "Gyarados",
        sprite: null,
        outcomesByProjectedLevel: [
          {
            minimumProjectedLevel: 1,
            maximumProjectedLevel: 19,
            effectiveSpecies: "SPECIES_MAGIKARP",
            eligible: true,
            minimumOrdinaryWildLevel: 1,
          },
          {
            minimumProjectedLevel: 20,
            maximumProjectedLevel: 100,
            effectiveSpecies: "SPECIES_GYARADOS",
            eligible: true,
            minimumOrdinaryWildLevel: 1,
          },
        ],
      },
    ],
    profiles: [],
    headerCounts: {},
  }

  it("locks a whole authored slot when any projected level is below its species floor", () => {
    const method: CatalogWildEncounterMethod = {
      type: "land_mons",
      encounterRate: 20,
      source,
      profiles: [{ profileKey: "land", fishingRod: "NONE", levelOffset: 0 }],
      slots: [
        {
          slotIndex: 0,
          slotRate: 20,
          slotRateSource: source,
          groups: [],
          minLevel: 15,
          maxLevel: 14,
          runtimeMinLevel: 14,
          runtimeMaxLevel: 15,
          speciesId: "SPECIES_SKARMORY",
          speciesLabel: "Skarmory",
          sprite: null,
          source,
        },
        {
          slotIndex: 1,
          slotRate: 80,
          slotRateSource: source,
          groups: [],
          minLevel: 5,
          maxLevel: 5,
          runtimeMinLevel: 5,
          runtimeMaxLevel: 5,
          speciesId: "SPECIES_RATTATA",
          speciesLabel: "Rattata",
          sprite: null,
          source,
        },
      ],
    }

    const slots = resolveMethodSlots(projection, method, 10)
    expect(slots[0]).toMatchObject({ eligible: false, selectionWeight: null })
    expect(slots[0]?.outcomes).toHaveLength(2)
    expect(slots[1]).toMatchObject({ eligible: true, selectionWeight: 1 })
  })

  it("renormalizes each fishing rod partition independently", () => {
    const slots = resolveMethodSlots(projection, fishing, 10, "good_rod")
    expect(slots).toHaveLength(1)
    expect(slots[0]?.selectionWeight).toBe(1)
  })

  it("changes the effective map roster across an evolution-reversal threshold", () => {
    const method: CatalogWildEncounterMethod = {
      type: "water_mons",
      encounterRate: 20,
      source,
      profiles: [{ profileKey: "water", fishingRod: "NONE", levelOffset: 1 }],
      slots: [
        {
          slotIndex: 0,
          slotRate: 100,
          slotRateSource: source,
          groups: [],
          minLevel: 20,
          maxLevel: 20,
          runtimeMinLevel: 20,
          runtimeMaxLevel: 20,
          speciesId: "SPECIES_GYARADOS",
          speciesLabel: "Gyarados",
          sprite: null,
          source,
        },
      ],
    }
    const encounters: ResolvedMapEncounters = {
      availableProducts: [{ id: "POKEMON_HNS", displayName: "HNS" }],
      product: "POKEMON_HNS",
      runtimeTimes: [],
      sets: [
        {
          mapId: "MAP_LAKE_OF_RAGE_HNS",
          mapName: "LakeOfRage_hns",
          baseLabel: "gLakeOfRage_hns_Day",
          product: "POKEMON_HNS",
          runtimeTime: "day",
          header: { groupLabel: "gWildMonHeaders", groupIndex: 0, headerIndex: 0 },
          source,
          methods: [method],
        },
      ],
    }

    expect(effectiveRosterFor(projection, encounters, "water_mons", 10)).toMatchObject([
      { speciesId: "SPECIES_MAGIKARP", projectedMinimumLevel: 10 },
    ])
    expect(effectiveRosterFor(projection, encounters, "water_mons", 30)).toMatchObject([
      { speciesId: "SPECIES_GYARADOS", projectedMinimumLevel: 20 },
    ])
  })
})
