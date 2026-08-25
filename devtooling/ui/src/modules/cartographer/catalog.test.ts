import { describe, expect, it } from "vitest"

import { CatalogValidationError, validateCatalog } from "./catalog.js"

const catalog = (overrides: Record<string, unknown> = {}): Record<string, unknown> => {
  return {
    schemaVersion: 7,
    pixelsPerMetatile: 16,
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
                speciesId: "SPECIES_ESPEON",
                speciesLabel: "Espeon",
                sprite: null,
                source: {
                  path: "src/data/wild_encounters.json",
                  pointer: "/wild_encounter_groups/0/encounters/0/land_mons/mons/0",
                },
              },
            ],
          },
        ],
      },
    ],
    runtimeTimes: [
      {
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
    expect(() => validateCatalog(catalog({ schemaVersion: 1 }))).toThrow("schemaVersion must be 7")
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
    expect(validateCatalog(catalog()).schemaVersion).toBe(7)
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
    encounters.runtimeTimes = [{ timeOfDay: "day", methods: [{ type: "land_mons" }] }]
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
