import { describe, expect, it } from "vitest"

import { CatalogValidationError, validateCatalog } from "./catalog.js"

const projection = (): Record<string, unknown> => ({
  schemaVersion: 1,
  trainerRating: { minimum: 10, maximum: 80 },
  authoredLevel: { minimum: 1, maximum: 100 },
  products: [{ id: "emerald", displayName: "Emerald" }],
  levelProjections: [
    {
      levelOffset: 0,
      ratings: Array.from({ length: 71 }, (_, ratingIndex) => ({
        rating: ratingIndex + 10,
        projectedLevels: Array.from({ length: 100 }, (_, levelIndex) => levelIndex + 1),
      })),
    },
  ],
  species: [
    {
      authoredSpecies: "SPECIES_ESPEON",
      authoredSpeciesId: 196,
      speciesLabel: "Espeon",
      sprite: null,
      outcomesByProjectedLevel: [
        {
          minimumProjectedLevel: 1,
          maximumProjectedLevel: 100,
          effectiveSpecies: "SPECIES_ESPEON",
          eligible: true,
          minimumOrdinaryWildLevel: 1,
        },
      ],
    },
  ],
  profiles: [
    {
      profileKey: "emerald/gRoute101/land_mons/NONE",
      product: "emerald",
      map: "MAP_ROUTE101",
      baseLabel: "gRoute101",
      header: "gWildMonHeaders",
      headerId: 0,
      runtimeTime: "TIME_DAY",
      method: "land_mons",
      runtimeArea: "WILD_AREA_LAND",
      fishingRod: "NONE",
      runtimeFishingRod: "WILD_ENCOUNTER_FISHING_ROD_NONE",
      levelOffset: 0,
      encounterRate: 20,
      authoredSlotCount: 1,
      runtimeSlotCount: 1,
    },
  ],
  headerCounts: { emerald: 1 },
})

const catalog = (overrides: Record<string, unknown> = {}): Record<string, unknown> => {
  return {
    schemaVersion: 8,
    pixelsPerMetatile: 16,
    wildEncounterProjection: projection(),
    regions: [],
    maps: [],
    topology: { conflicts: [] },
    ...overrides,
  }
}

const mapWithWildEncounters = (
  wildEncounters: Record<string, unknown>,
): Record<string, unknown> => {
  return {
    name: "Route101",
    id: "MAP_ROUTE101",
    region: "routes",
    image: { widthPixels: 16, heightPixels: 16 },
    layout: { widthMetatiles: 1, heightMetatiles: 1 },
    objects: [],
    wildEncounters,
    encounterHabitat: { land: [], water: [] },
  }
}

const wildEncounters = (): Record<string, unknown> => {
  return {
    sets: [
      {
        mapId: "MAP_ROUTE101",
        mapName: "Route101",
        baseLabel: "gRoute101",
        product: "emerald",
        runtimeTime: "day",
        header: { groupLabel: "gWildMonHeaders", groupIndex: 0, headerIndex: 0 },
        source: {
          path: "src/data/wild_encounters.json",
          pointer: "/wild_encounter_groups/0/encounters/0",
        },
        methods: [
          {
            type: "land_mons",
            encounterRate: 20,
            source: {
              path: "src/data/wild_encounters.json",
              pointer: "/wild_encounter_groups/0/encounters/0/land_mons",
            },
            slots: [
              {
                slotIndex: 0,
                slotRate: 20,
                slotRateSource: {
                  path: "src/data/wild_encounters.json",
                  pointer: "/wild_encounter_groups/0/fields/0/encounter_rates/0",
                },
                groups: [],
                minLevel: 2,
                maxLevel: 3,
                runtimeMinLevel: 2,
                runtimeMaxLevel: 3,
                speciesId: "SPECIES_ESPEON",
                speciesLabel: "Espeon",
                sprite: null,
                source: {
                  path: "src/data/wild_encounters.json",
                  pointer: "/wild_encounter_groups/0/encounters/0/land_mons/mons/0",
                },
              },
            ],
            profiles: [
              {
                profileKey: "emerald/gRoute101/land_mons/NONE",
                fishingRod: "NONE",
                levelOffset: 0,
              },
            ],
          },
        ],
      },
    ],
    runtimeTimes: [
      {
        product: "emerald",
        timeOfDay: "day",
        methods: [
          {
            type: "land_mons",
            resolution: "direct",
            sets: [
              {
                baseLabel: "gRoute101",
                source: {
                  path: "src/data/wild_encounters.json",
                  pointer: "/wild_encounter_groups/0/encounters/0",
                },
              },
            ],
          },
        ],
      },
      {
        product: "emerald",
        timeOfDay: "night",
        methods: [
          {
            type: "land_mons",
            resolution: "fallback",
            sets: [
              {
                baseLabel: "gRoute101",
                source: {
                  path: "src/data/wild_encounters.json",
                  pointer: "/wild_encounter_groups/0/encounters/0",
                },
              },
            ],
          },
        ],
      },
    ],
    diagnostics: [
      {
        code: "unaddressable_source_slot",
        reason: "outside_method_slot_table",
        setBaseLabel: "gRoute101",
        methodType: "fishing_mons",
        slotIndex: 10,
        speciesId: "SPECIES_MAGIKARP",
        minLevel: 3,
        maxLevel: 5,
        source: {
          path: "src/data/wild_encounters.json",
          pointer: "/wild_encounter_groups/0/encounters/0/fishing_mons/mons/10",
        },
      },
    ],
  }
}

describe("validateCatalog", () => {
  it("rejects stale catalog schemas before the viewport can interpret their topology", () => {
    expect(() => validateCatalog(catalog({ schemaVersion: 1 }))).toThrow(CatalogValidationError)
    expect(() => validateCatalog(catalog({ schemaVersion: 1 }))).toThrow("schemaVersion must be 8")
  })

  it("rejects empty and dimensionally incomplete projection lookup tables", () => {
    expect(() =>
      validateCatalog(
        catalog({ wildEncounterProjection: { ...projection(), levelProjections: [] } }),
      ),
    ).toThrow("levelProjections must be a non-empty array")

    const model = projection()
    const tables = model.levelProjections as Array<Record<string, unknown>>
    tables[0]!.ratings = (tables[0]!.ratings as unknown[]).slice(0, 70)
    expect(() => validateCatalog(catalog({ wildEncounterProjection: model }))).toThrow(
      "must contain 71 ratings",
    )
  })

  it("rejects projection species intervals with missing coverage or references", () => {
    const model = projection()
    const species = model.species as Array<Record<string, unknown>>
    species[0]!.outcomesByProjectedLevel = [
      {
        minimumProjectedLevel: 1,
        maximumProjectedLevel: 99,
        effectiveSpecies: "SPECIES_MISSING",
        eligible: true,
        minimumOrdinaryWildLevel: 1,
      },
    ]

    expect(() => validateCatalog(catalog({ wildEncounterProjection: model }))).toThrow(
      "invalid or incomplete outcome intervals",
    )
  })

  it("rejects source slots and methods without exact projection lookups", () => {
    const encounters = wildEncounters()
    const sets = encounters.sets as Array<Record<string, unknown>>
    const methods = sets[0]!.methods as Array<Record<string, unknown>>
    methods[0]!.profiles = [{ profileKey: "missing", fishingRod: "NONE", levelOffset: 0 }]
    const slots = methods[0]!.slots as Array<Record<string, unknown>>
    slots[0]!.speciesId = "SPECIES_MISSING"
    const value = catalog({
      regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
      maps: [mapWithWildEncounters(encounters)],
    })

    expect(() => validateCatalog(value)).toThrow("invalid projection profile missing")
    expect(() => validateCatalog(value)).toThrow("has no species projection for SPECIES_MISSING")
  })

  it("rejects unsupported topology diagnostic codes", () => {
    expect(() =>
      validateCatalog(
        catalog({
          topology: {
            conflicts: [{ code: "connection_placement_mismatch", explanation: "old contract" }],
          },
        }),
      ),
    ).toThrow("unsupported code")
  })

  it("accepts the current empty diagnostic contract", () => {
    expect(validateCatalog(catalog()).schemaVersion).toBe(8)
  })

  it("accepts source-backed wild encounter sets and runtime time-of-day resolution", () => {
    const value = catalog({
      regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
      maps: [mapWithWildEncounters(wildEncounters())],
    })

    expect(
      validateCatalog(value).maps[0]?.wildEncounters.runtimeTimes[1]?.methods[0]?.resolution,
    ).toBe("fallback")
  })

  it("accepts invalid source-slot diagnostics retained by the generated catalog", () => {
    const encounters = wildEncounters()
    encounters.diagnostics = [
      {
        code: "invalid_source_slot",
        reason: "invalid_level_range",
        setBaseLabel: "gRoute101",
        methodType: "land_mons",
        slotIndex: 0,
        speciesId: "SPECIES_ESPEON",
        minLevel: 5,
        maxLevel: 3,
        source: {
          path: "src/data/wild_encounters.json",
          pointer: "/wild_encounter_groups/0/encounters/0/land_mons/mons/0",
        },
      },
    ]
    const value = catalog({
      regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
      maps: [mapWithWildEncounters(encounters)],
    })

    expect(validateCatalog(value).maps[0]?.wildEncounters.diagnostics[0]).toMatchObject({
      code: "invalid_source_slot",
      reason: "invalid_level_range",
    })
  })

  it("rejects incomplete source encounter data", () => {
    const encounters = wildEncounters()
    encounters.runtimeTimes = [
      { product: "emerald", timeOfDay: "day", methods: [{ type: "land_mons" }] },
    ]
    const value = catalog({
      regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
      maps: [mapWithWildEncounters(encounters)],
    })

    expect(() => validateCatalog(value)).toThrow(
      "wildEncounters must contain valid source encounter data",
    )
  })

  it("rejects maps without generated runtime encounter-tile geometry", () => {
    const map = mapWithWildEncounters(wildEncounters())
    delete map.encounterHabitat
    const value = catalog({
      regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
      maps: [map],
    })

    expect(() => validateCatalog(value)).toThrow(
      "encounterHabitat must contain valid source tile geometry",
    )
  })

  it("rejects map objects without an explicit shiny-state flag", () => {
    const value = catalog({
      regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
      maps: [{ ...mapWithWildEncounters(wildEncounters()), objects: [{ objectId: "0" }] }],
    })

    expect(() => validateCatalog(value)).toThrow(
      "objects must contain an explicit shiny-state flag",
    )
  })
})
