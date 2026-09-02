import { describe, expect, it } from "vitest"

import {
  effectiveRosterFor,
  fishingProfiles,
  fishingRarityBandIds,
  rarityBandLabel,
  resolveEncounterPopulation,
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
      groups: [{ id: "old_rod", source }],
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
    { profileKey: "super", fishingRod: "SUPER_ROD", levelOffset: 0 },
  ],
}

describe("encounter presentation", () => {
  it("retains fishing slots even when legacy rates are zero or the species is NONE", () => {
    expect(visibleEncounterSlots(fishing).map((slot) => slot.speciesLabel)).toEqual([
      "Magikarp",
      "None",
      "Goldeen",
    ])
  })

  it("uses source groups only as rarity-band labels", () => {
    expect(fishingRarityBandIds(fishing)).toEqual(["old_rod", "good_rod"])
    expect(fishing.slots.map(rarityBandLabel)).toEqual(["Common", "Common", "Less common"])
    expect(fishingProfiles(fishing).map((profile) => rodLabel(profile.fishingRod))).toEqual([
      "Old Rod",
      "Good Rod",
      "Super Rod",
    ])
  })

  const identity = Array.from({ length: 100 }, (_, index) => index + 1)
  const lowRating = Array.from({ length: 100 }, (_, index) =>
    Math.max(1, Math.ceil((index + 1) / 2)),
  )
  const projection: CatalogWildEncounterProjection = {
    schemaVersion: 2,
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
    profiles: [
      {
        profileKey: "old",
        product: "hns",
        map: "MAP_ALPHA",
        baseLabel: "alpha",
        header: "headers",
        headerId: 0,
        runtimeTime: "TIME_DAY",
        method: "fishing_mons",
        runtimeArea: "WILD_AREA_FISHING",
        fishingRod: "OLD_ROD",
        runtimeFishingRod: "WILD_ENCOUNTER_FISHING_ROD_OLD",
        levelOffset: 0,
        encounterRate: 20,
        authoredSlotCount: 10,
        runtimeSlotCount: 10,
        weights: [38, 22, 10, 8, 8, 4, 3, 3, 2, 2],
      },
      {
        profileKey: "good",
        product: "hns",
        map: "MAP_ALPHA",
        baseLabel: "alpha",
        header: "headers",
        headerId: 0,
        runtimeTime: "TIME_DAY",
        method: "fishing_mons",
        runtimeArea: "WILD_AREA_FISHING",
        fishingRod: "GOOD_ROD",
        runtimeFishingRod: "WILD_ENCOUNTER_FISHING_ROD_GOOD",
        levelOffset: 0,
        encounterRate: 20,
        authoredSlotCount: 10,
        runtimeSlotCount: 10,
        weights: [25, 18, 12, 10, 9, 7, 6, 5, 4, 4],
      },
      {
        profileKey: "super",
        product: "hns",
        map: "MAP_ALPHA",
        baseLabel: "alpha",
        header: "headers",
        headerId: 0,
        runtimeTime: "TIME_DAY",
        method: "fishing_mons",
        runtimeArea: "WILD_AREA_FISHING",
        fishingRod: "SUPER_ROD",
        runtimeFishingRod: "WILD_ENCOUNTER_FISHING_ROD_SUPER",
        levelOffset: 0,
        encounterRate: 20,
        authoredSlotCount: 10,
        runtimeSlotCount: 10,
        weights: [12, 10, 11, 10, 10, 10, 10, 9, 9, 9],
      },
    ],
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

  it("renormalizes a quality profile after excluding SPECIES_NONE", () => {
    const slots = resolveMethodSlots(projection, fishing, 10, "GOOD_ROD")
    expect(slots).toHaveLength(3)
    expect(slots[0]).toMatchObject({ rawWeight: 25, selectionWeight: 25 / 37 })
    expect(slots[1]).toMatchObject({ eligible: false, rawWeight: 18, selectionWeight: null })
    expect(slots[2]).toMatchObject({ rawWeight: 12, selectionWeight: 12 / 37 })
  })

  it("keeps all ten slots in every quality and includes every rarity band", () => {
    const tenSlots: CatalogWildEncounterMethod = {
      ...fishing,
      slots: Array.from({ length: 10 }, (_, slotIndex) => ({
        slotIndex,
        slotRate: slotIndex === 0 ? 100 : 0,
        slotRateSource: source,
        groups: [
          {
            id: slotIndex < 2 ? "old_rod" : slotIndex < 5 ? "good_rod" : "super_rod",
            source,
          },
        ],
        minLevel: 5,
        maxLevel: 5,
        runtimeMinLevel: 5,
        runtimeMaxLevel: 5,
        speciesId: "SPECIES_MAGIKARP",
        speciesLabel: "Magikarp",
        sprite: null,
        source,
      })),
    }

    expect(
      fishingProfiles(tenSlots).map((profile) =>
        resolveMethodSlots(projection, tenSlots, 10, profile.fishingRod).map(
          (slot) => slot.selectionWeight,
        ),
      ),
    ).toEqual([
      [0.38, 0.22, 0.1, 0.08, 0.08, 0.04, 0.03, 0.03, 0.02, 0.02],
      [0.25, 0.18, 0.12, 0.1, 0.09, 0.07, 0.06, 0.05, 0.04, 0.04],
      [0.12, 0.1, 0.11, 0.1, 0.1, 0.1, 0.1, 0.09, 0.09, 0.09],
    ])
    expect(new Set(tenSlots.slots.map(rarityBandLabel))).toEqual(
      new Set(["Common", "Less common", "Rare"]),
    )
  })

  it("renormalizes generated weights across Trainer Rating eligible slots", () => {
    const filtered: CatalogWildEncounterMethod = {
      ...fishing,
      slots: Array.from({ length: 10 }, (_, slotIndex) => ({
        slotIndex,
        slotRate: 0,
        slotRateSource: source,
        groups: [
          {
            id: slotIndex < 2 ? "old_rod" : slotIndex < 5 ? "good_rod" : "super_rod",
            source,
          },
        ],
        minLevel: 5,
        maxLevel: 5,
        runtimeMinLevel: 5,
        runtimeMaxLevel: 5,
        speciesId: slotIndex === 0 ? "SPECIES_SKARMORY" : "SPECIES_MAGIKARP",
        speciesLabel: slotIndex === 0 ? "Skarmory" : "Magikarp",
        sprite: null,
        source,
      })),
    }

    const slots = resolveMethodSlots(projection, filtered, 10, "OLD_ROD")
    expect(slots[0]).toMatchObject({ eligible: false, rawWeight: 38, selectionWeight: null })
    expect(slots[1]?.selectionWeight).toBeCloseTo(22 / 62)
    expect(slots[9]?.selectionWeight).toBeCloseTo(2 / 62)
  })

  it("returns a safe zero-data view when every fishing entry is SPECIES_NONE", () => {
    const zeroData: CatalogWildEncounterMethod = {
      ...fishing,
      slots: Array.from({ length: 10 }, (_, slotIndex) => ({
        slotIndex,
        slotRate: 0,
        slotRateSource: source,
        groups: [
          {
            id: slotIndex < 2 ? "old_rod" : slotIndex < 5 ? "good_rod" : "super_rod",
            source,
          },
        ],
        minLevel: 1,
        maxLevel: 1,
        runtimeMinLevel: 1,
        runtimeMaxLevel: 1,
        speciesId: "SPECIES_NONE",
        speciesLabel: "None",
        sprite: null,
        source,
      })),
    }

    const slots = resolveMethodSlots(projection, zeroData, 10, "SUPER_ROD")
    expect(slots).toHaveLength(10)
    expect(slots.every((slot) => !slot.eligible && slot.selectionWeight === null)).toBe(true)
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

  it("groups the complete resolved roster by source set and runtime time use", () => {
    const slot = (
      slotIndex: number,
      slotRate: number,
      runtimeLevel: number,
      speciesId: string,
      speciesLabel: string,
    ) => ({
      slotIndex,
      slotRate,
      slotRateSource: source,
      groups: [],
      minLevel: runtimeLevel,
      maxLevel: runtimeLevel,
      runtimeMinLevel: runtimeLevel,
      runtimeMaxLevel: runtimeLevel,
      speciesId,
      speciesLabel,
      sprite: null,
      source,
    })
    const method = (
      type: "land_mons" | "water_mons",
      slots: CatalogWildEncounterMethod["slots"],
    ): CatalogWildEncounterMethod => ({
      type,
      encounterRate: 20,
      source,
      profiles: [{ profileKey: `${type}/scaled`, fishingRod: "NONE", levelOffset: 1 }],
      slots,
    })
    const dayLand = method("land_mons", [
      slot(0, 50, 20, "SPECIES_GYARADOS", "Gyarados"),
      slot(1, 25, 8, "SPECIES_RATTATA", "Rattata"),
      slot(2, 25, 14, "SPECIES_SKARMORY", "Skarmory"),
    ])
    const nightLand = method("land_mons", [slot(0, 100, 12, "SPECIES_RATTATA", "Rattata")])
    const unusedWater = method("water_mons", [slot(0, 100, 20, "SPECIES_GYARADOS", "Gyarados")])
    const daySource = { ...source, pointer: "/wild_encounter_groups/0/encounters/0" }
    const nightSource = { ...source, pointer: "/wild_encounter_groups/0/encounters/1" }
    const encounters: ResolvedMapEncounters = {
      availableProducts: [{ id: "POKEMON_HNS", displayName: "HNS" }],
      product: "POKEMON_HNS",
      sets: [
        {
          mapId: "MAP_ROUTE_32_HNS",
          mapName: "Route32_hns",
          baseLabel: "gRoute32_hns_Day",
          product: "POKEMON_HNS",
          runtimeTime: "day",
          header: { groupLabel: "gWildMonHeaders", groupIndex: 0, headerIndex: 0 },
          source: daySource,
          methods: [dayLand, unusedWater],
        },
        {
          mapId: "MAP_ROUTE_32_HNS",
          mapName: "Route32_hns",
          baseLabel: "gRoute32_hns_Night",
          product: "POKEMON_HNS",
          runtimeTime: "night",
          header: { groupLabel: "gWildMonHeaders", groupIndex: 0, headerIndex: 1 },
          source: nightSource,
          methods: [nightLand],
        },
      ],
      runtimeTimes: [
        {
          product: "POKEMON_HNS",
          timeOfDay: "morning",
          methods: [
            {
              type: "land_mons",
              resolution: "fallback",
              sets: [{ baseLabel: "gRoute32_hns_Day", source: daySource }],
            },
          ],
        },
        {
          product: "POKEMON_HNS",
          timeOfDay: "day",
          methods: [
            {
              type: "land_mons",
              resolution: "direct",
              sets: [{ baseLabel: "gRoute32_hns_Day", source: daySource }],
            },
          ],
        },
        {
          product: "POKEMON_HNS",
          timeOfDay: "evening",
          methods: [
            {
              type: "land_mons",
              resolution: "fallback",
              sets: [{ baseLabel: "gRoute32_hns_Day", source: daySource }],
            },
          ],
        },
        {
          product: "POKEMON_HNS",
          timeOfDay: "night",
          methods: [
            {
              type: "land_mons",
              resolution: "direct",
              sets: [{ baseLabel: "gRoute32_hns_Night", source: nightSource }],
            },
          ],
        },
      ],
    }

    const population = resolveEncounterPopulation(projection, encounters, "land_mons", 10)
    const groups = population.sources

    expect(population.method).toBe("land_mons")
    expect(population.unavailableTimes).toEqual([])
    expect(groups).toHaveLength(2)
    expect(groups[0]).toMatchObject({
      set: { baseLabel: "gRoute32_hns_Day", runtimeTime: "day" },
      activations: [
        { timeOfDay: "morning", resolution: "fallback" },
        { timeOfDay: "day", resolution: "direct" },
        { timeOfDay: "evening", resolution: "fallback" },
      ],
      lockedSlotCount: 1,
    })
    expect(groups[0]?.slots).toHaveLength(3)
    expect(groups[0]?.effectiveSlots).toMatchObject([
      {
        source: { speciesId: "SPECIES_GYARADOS" },
        outcomes: [{ speciesId: "SPECIES_MAGIKARP", projectedMinimumLevel: 10 }],
        selectionWeight: 2 / 3,
      },
      {
        source: { speciesId: "SPECIES_RATTATA" },
        outcomes: [{ speciesId: "SPECIES_RATTATA", projectedMinimumLevel: 4 }],
        selectionWeight: 1 / 3,
      },
    ])
    expect(
      groups[0]?.effectiveSlots.some(
        (candidate) => candidate.source.speciesId === "SPECIES_SKARMORY",
      ),
    ).toBe(false)
    expect(groups[1]).toMatchObject({
      set: { baseLabel: "gRoute32_hns_Night", runtimeTime: "night" },
      activations: [{ timeOfDay: "night", resolution: "direct" }],
      lockedSlotCount: 0,
      effectiveSlots: [
        {
          source: { speciesId: "SPECIES_RATTATA" },
          outcomes: [{ projectedMinimumLevel: 6 }],
          selectionWeight: 1,
        },
      ],
    })
  })

  it("keeps source time provenance when runtime resolution metadata is absent", () => {
    const water: CatalogWildEncounterMethod = {
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
      availableProducts: [{ id: "EMERALD", displayName: "Emerald" }],
      product: "EMERALD",
      runtimeTimes: [],
      sets: [
        {
          mapId: "MAP_ALTERING_CAVE",
          mapName: "AlteringCave",
          baseLabel: "gAlteringCave1",
          product: "EMERALD",
          runtimeTime: "day",
          header: { groupLabel: "gWildMonHeaders", groupIndex: 0, headerIndex: 0 },
          source,
          methods: [water],
        },
      ],
    }

    expect(resolveEncounterPopulation(projection, encounters, "water_mons", 10)).toMatchObject({
      method: "water_mons",
      unavailableTimes: [],
      sources: [
        {
          set: { baseLabel: "gAlteringCave1", runtimeTime: "day" },
          activations: [],
          lockedSlotCount: 0,
          effectiveSlots: [
            {
              outcomes: [{ speciesId: "SPECIES_MAGIKARP", projectedMinimumLevel: 10 }],
              selectionWeight: 1,
            },
          ],
        },
      ],
    })
  })

  it("retains a fully locked source and method-wide unavailable runtime times", () => {
    const land: CatalogWildEncounterMethod = {
      type: "land_mons",
      encounterRate: 20,
      source,
      profiles: [{ profileKey: "land", fishingRod: "NONE", levelOffset: 1 }],
      slots: [
        {
          slotIndex: 0,
          slotRate: 100,
          slotRateSource: source,
          groups: [],
          minLevel: 14,
          maxLevel: 14,
          runtimeMinLevel: 14,
          runtimeMaxLevel: 14,
          speciesId: "SPECIES_SKARMORY",
          speciesLabel: "Skarmory",
          sprite: null,
          source,
        },
      ],
    }
    const encounters: ResolvedMapEncounters = {
      availableProducts: [{ id: "POKEMON_HNS", displayName: "HNS" }],
      product: "POKEMON_HNS",
      sets: [
        {
          mapId: "MAP_ROUTE_45_HNS",
          mapName: "Route45_hns",
          baseLabel: "gRoute45_hns_Day",
          product: "POKEMON_HNS",
          runtimeTime: "day",
          header: { groupLabel: "gWildMonHeaders", groupIndex: 0, headerIndex: 0 },
          source,
          methods: [land],
        },
      ],
      runtimeTimes: [
        {
          product: "POKEMON_HNS",
          timeOfDay: "morning",
          methods: [{ type: "land_mons", resolution: "unavailable", sets: [] }],
        },
      ],
    }

    const population = resolveEncounterPopulation(projection, encounters, "land_mons", 10)

    expect(population.unavailableTimes).toEqual(["morning"])
    expect(population.sources).toMatchObject([
      {
        set: { baseLabel: "gRoute45_hns_Day" },
        activations: [],
        slots: [{ eligible: false, selectionWeight: null }],
        effectiveSlots: [],
        lockedSlotCount: 1,
      },
    ])
  })
})
