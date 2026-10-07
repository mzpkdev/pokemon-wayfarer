import { describe, expect, it } from "vitest"

import type {
  CatalogMap,
  CatalogWildEncounterMethod,
  CatalogWildEncounterProjection,
  CatalogWildEncounterSet,
} from "./catalog.js"
import {
  effectiveRosterFor,
  fishingProfiles,
  placeLabel,
  placeLevelAt,
  resolveEncounterPopulation,
  resolveMapEncounters,
  resolveMethodSlots,
  rodLabel,
  type ResolvedMapEncounters,
} from "./encounters.js"

const source = { path: "src/data/wild_encounters_v2/kanto.json", pointer: "/MAP_ROUTE1_HNS" }

const slot = (slotIndex: number, speciesId: string, speciesLabel: string) => ({
  slotIndex,
  speciesId,
  speciesLabel,
  sprite: null,
  source: { ...source, pointer: `${source.pointer}/land/day/${slotIndex}` },
})

const land: CatalogWildEncounterMethod = {
  type: "land_mons",
  encounterRate: 20,
  source,
  slots: [slot(0, "SPECIES_PIDGEY", "Pidgey"), slot(1, "SPECIES_RATTATA", "Rattata")],
  profiles: [{ profileKey: "land", fishingRod: "NONE", weights: [60, 30, 10] }],
}

const fishing: CatalogWildEncounterMethod = {
  type: "fishing_mons",
  encounterRate: 30,
  source,
  slots: [slot(0, "SPECIES_MAGIKARP", "Magikarp"), slot(1, "SPECIES_GOLDEEN", "Goldeen")],
  profiles: [
    { profileKey: "old", fishingRod: "OLD_ROD", weights: [80, 20] },
    { profileKey: "good", fishingRod: "GOOD_ROD", weights: [50, 50] },
    { profileKey: "super", fishingRod: "SUPER_ROD", weights: [20, 80] },
  ],
}

const set = (
  baseLabel: string,
  runtimeTime: "day" | "night",
  methods: CatalogWildEncounterMethod[],
  overrides: Partial<CatalogWildEncounterSet["place"]> = {},
): CatalogWildEncounterSet => ({
  mapId: "MAP_ROUTE1_HNS",
  mapName: "Route1_hns",
  baseLabel,
  product: "POKEMON_WAYFARER",
  runtimeTime,
  variant: null,
  place: {
    name: "Route 1",
    region: "Kanto",
    regionClass: "Other",
    reach: "Road",
    dungeon: null,
    floor: null,
    floors: null,
    placeLevels: [5, 6, 20],
    ...overrides,
  },
  source,
  methods,
})

// Species: 0 Pidgey, 1 Pidgeotto, 2 Rattata, 3 Magikarp, 4 Goldeen.
const projection: CatalogWildEncounterProjection = {
  schemaVersion: 3,
  trainerRating: { minimum: 0, maximum: 2 },
  outcomeDenominator: 50,
  products: [{ id: "POKEMON_WAYFARER", displayName: "Pokémon Wayfarer" }],
  species: [
    { speciesId: "SPECIES_PIDGEY", speciesLabel: "Pidgey", sprite: null },
    { speciesId: "SPECIES_PIDGEOTTO", speciesLabel: "Pidgeotto", sprite: null },
    { speciesId: "SPECIES_RATTATA", speciesLabel: "Rattata", sprite: null },
    { speciesId: "SPECIES_MAGIKARP", speciesLabel: "Magikarp", sprite: null },
    { speciesId: "SPECIES_GOLDEEN", speciesLabel: "Goldeen", sprite: null },
  ],
  distributions: [
    {
      speciesId: "SPECIES_PIDGEY",
      regionClass: "Other",
      byPlaceLevel: {
        "5": [
          [0, 3, 10],
          [0, 4, 10],
          [0, 5, 10],
          [0, 6, 10],
          [0, 7, 10],
        ],
        "6": [[0, 4, 50]],
        // Past the evolution level the roll splits between the stages.
        "20": [
          [0, 17, 25],
          [1, 17, 25],
        ],
      },
    },
    {
      speciesId: "SPECIES_RATTATA",
      regionClass: "Other",
      byPlaceLevel: { "5": [[2, 5, 50]], "6": [[2, 6, 50]], "20": [[2, 20, 50]] },
    },
    {
      speciesId: "SPECIES_MAGIKARP",
      regionClass: "Other",
      byPlaceLevel: { "5": [[3, 5, 50]], "6": [[3, 6, 50]], "20": [[3, 20, 50]] },
    },
    {
      speciesId: "SPECIES_GOLDEEN",
      regionClass: "Other",
      byPlaceLevel: { "5": [[4, 5, 50]], "6": [[4, 6, 50]], "20": [[4, 20, 50]] },
    },
  ],
}

const day = set("gWildV2_Route1_Hns_Day", "day", [land, fishing])
const night = set("gWildV2_Route1_Hns_Night", "night", [land])

const resolved = (
  sets: CatalogWildEncounterSet[],
  runtimeTimes: ResolvedMapEncounters["runtimeTimes"] = [],
): ResolvedMapEncounters => ({
  availableProducts: projection.products,
  product: "POKEMON_WAYFARER",
  sets,
  runtimeTimes,
})

describe("place levels", () => {
  it("reads the place level at a Trainer Rating and clamps outside the table", () => {
    expect(placeLevelAt(projection, day, 0)).toBe(5)
    expect(placeLevelAt(projection, day, 2)).toBe(20)
    expect(placeLevelAt(projection, day, 99)).toBe(20)
    expect(placeLevelAt(projection, day, -3)).toBe(5)
  })

  it("labels roads and dungeons by reach, intent and floor", () => {
    expect(placeLabel(day.place)).toBe("Road")
    expect(
      placeLabel({
        ...day.place,
        reach: "Dungeon",
        dungeon: { intent: "Moderate to hard", flat: false },
        floor: 1,
        floors: 7,
      }),
    ).toBe("Dungeon (moderate to hard) · floor 2 of 7")
    expect(
      placeLabel({
        ...day.place,
        reach: "Dungeon",
        dungeon: { intent: "Hard", flat: true },
        floor: 0,
        floors: 1,
      }),
    ).toBe("Dungeon (hard, flat)")
  })
})

describe("encounter presentation", () => {
  it("lists the three rods of a fishing method and none for other methods", () => {
    expect(fishingProfiles(fishing).map((profile) => rodLabel(profile.fishingRod))).toEqual([
      "Old Rod",
      "Good Rod",
      "Super Rod",
    ])
    expect(fishingProfiles(land)).toEqual([])
  })

  it("picks the requested product and falls back to the first one present", () => {
    const map = { wildEncounters: { sets: [day], runtimeTimes: [] } } as unknown as CatalogMap

    expect(resolveMapEncounters(map, "POKEMON_WAYFARER", projection.products).product).toBe(
      "POKEMON_WAYFARER",
    )
    expect(resolveMapEncounters(map, "EMERALD", projection.products).product).toBe(
      "POKEMON_WAYFARER",
    )
  })

  it("merges a slot's consecutive levels into one range with its chance", () => {
    const [pidgey] = resolveMethodSlots(projection, day, land, 0)

    expect(pidgey?.outcomes).toEqual([
      expect.objectContaining({
        speciesLabel: "Pidgey",
        projectedMinimumLevel: 3,
        projectedMaximumLevel: 7,
        chance: 1,
      }),
    ])
  })

  it("shows each stage of a stage mix with its own level and chance", () => {
    const [pidgey] = resolveMethodSlots(projection, day, land, 2)

    expect(
      pidgey?.outcomes.map((outcome) => [
        outcome.speciesLabel,
        outcome.projectedMinimumLevel,
        outcome.chance,
      ]),
    ).toEqual([
      ["Pidgey", 17, 0.5],
      ["Pidgeotto", 17, 0.5],
    ])
  })

  it("weighs slots by their share of the method's table weights", () => {
    const slots = resolveMethodSlots(projection, day, land, 1)

    expect(slots.map((entry) => [entry.rawWeight, entry.selectionWeight])).toEqual([
      [60, 60 / 90],
      [30, 30 / 90],
    ])
  })

  it("weighs fishing slots per rod", () => {
    expect(
      resolveMethodSlots(projection, day, fishing, 0, "OLD_ROD").map((entry) => entry.rawWeight),
    ).toEqual([80, 20])
    expect(
      resolveMethodSlots(projection, day, fishing, 0, "SUPER_ROD").map((entry) => entry.rawWeight),
    ).toEqual([20, 80])
    expect(resolveMethodSlots(projection, day, fishing, 0, "SUPER_ROD")[0]?.fishingRod).toBe(
      "SUPER_ROD",
    )
  })

  it("returns no outcomes for a slot whose species is missing at the place level", () => {
    const missing = { ...land, slots: [slot(0, "SPECIES_UNKNOWN", "Unknown")] }

    expect(resolveMethodSlots(projection, day, missing, 0)[0]?.outcomes).toEqual([])
  })

  it("lists the species a map can roll across its methods and rods, once each", () => {
    expect(
      effectiveRosterFor(projection, resolved([day, night]), "land_mons", 0).map(
        (outcome) => outcome.speciesLabel,
      ),
    ).toEqual(["Pidgey", "Rattata"])
    expect(
      effectiveRosterFor(projection, resolved([day]), "fishing_mons", 0).map(
        (outcome) => outcome.speciesLabel,
      ),
    ).toEqual(["Magikarp", "Goldeen"])
    expect(
      effectiveRosterFor(projection, resolved([day]), "land_mons", 2).map(
        (outcome) => outcome.speciesLabel,
      ),
    ).toEqual(["Pidgey", "Pidgeotto", "Rattata"])
  })

  it("groups the population by table with its place level and runtime time use", () => {
    const encounters = resolved(
      [day, night],
      [
        {
          product: "POKEMON_WAYFARER",
          timeOfDay: "morning",
          methods: [
            {
              type: "land_mons",
              resolution: "direct",
              sets: [{ baseLabel: day.baseLabel, source: day.source }],
            },
          ],
        },
        {
          product: "POKEMON_WAYFARER",
          timeOfDay: "evening",
          methods: [
            {
              type: "land_mons",
              resolution: "direct",
              sets: [{ baseLabel: night.baseLabel, source: night.source }],
            },
          ],
        },
        {
          product: "POKEMON_WAYFARER",
          timeOfDay: "night",
          methods: [{ type: "water_mons", resolution: "unavailable", sets: [] }],
        },
      ],
    )

    const population = resolveEncounterPopulation(projection, encounters, "land_mons", 1)

    expect(
      population.sources.map((entry) => [
        entry.set.baseLabel,
        entry.placeLevel,
        entry.activations.map((use) => use.timeOfDay),
        entry.slots.length,
      ]),
    ).toEqual([
      ["gWildV2_Route1_Hns_Day", 6, ["morning"], 2],
      ["gWildV2_Route1_Hns_Night", 6, ["evening"], 2],
    ])
    expect(population.unavailableTimes).toEqual([])
    expect(
      resolveEncounterPopulation(projection, encounters, "water_mons", 1).unavailableTimes,
    ).toEqual(["night"])
  })
})
