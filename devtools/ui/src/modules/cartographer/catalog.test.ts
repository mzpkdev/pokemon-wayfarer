import { describe, expect, it } from "vitest"

import { CatalogValidationError, mapsForBuild, validateCatalog } from "./catalog.js"

const projection = (): Record<string, unknown> => ({
  schemaVersion: 3,
  trainerRating: { minimum: 0, maximum: 2 },
  outcomeDenominator: 50,
  products: [{ id: "emerald", displayName: "Emerald" }],
  species: [{ speciesId: "SPECIES_ESPEON", speciesLabel: "Espeon", sprite: null }],
  distributions: [
    {
      speciesId: "SPECIES_ESPEON",
      regionClass: "Other",
      byPlaceLevel: {
        "5": [[0, 5, 50]],
        "6": [
          [0, 5, 25],
          [0, 6, 25],
        ],
      },
    },
  ],
})

const catalog = (overrides: Record<string, unknown> = {}): Record<string, unknown> => {
  const maps = Array.isArray(overrides.maps) ? overrides.maps : []
  const builds = Object.hasOwn(overrides, "builds")
    ? overrides.builds
    : [
        {
          id: "emerald",
          label: "Emerald",
          mapCount: maps.filter(
            (map) =>
              typeof map === "object" &&
              map !== null &&
              Array.isArray((map as Record<string, unknown>).builds) &&
              (map as Record<string, unknown>).builds.includes("emerald"),
          ).length,
          maps: maps.flatMap((map) => {
            if (
              typeof map !== "object" ||
              map === null ||
              !Array.isArray((map as Record<string, unknown>).builds) ||
              !(map as Record<string, unknown>).builds.includes("emerald") ||
              typeof (map as Record<string, unknown>).name !== "string"
            )
              return []
            return [(map as Record<string, unknown>).name]
          }),
        },
      ]
  return {
    schemaVersion: 11,
    pixelsPerMetatile: 16,
    wildEncounterProjection: projection(),
    builds,
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
    builds: ["emerald"],
    image: { widthPixels: 16, heightPixels: 16 },
    layout: { widthMetatiles: 1, heightMetatiles: 1 },
    objects: [],
    wildEncounters,
    encounterHabitat: { land: [], water: [] },
  }
}

const wildEncounters = (): Record<string, unknown> => {
  const source = { path: "src/data/wild_encounters_v2/hoenn.json", pointer: "/MAP_ROUTE101" }
  return {
    sets: [
      {
        mapId: "MAP_ROUTE101",
        mapName: "Route101",
        baseLabel: "gWildV2_Route101_Day",
        product: "emerald",
        runtimeTime: "day",
        variant: null,
        place: {
          name: "Route 101",
          region: "Hoenn",
          regionClass: "Other",
          reach: "Road",
          dungeon: null,
          floor: null,
          floors: null,
          placeLevels: [5, 5, 6],
        },
        source,
        methods: [
          {
            type: "land_mons",
            encounterRate: 20,
            source: { ...source, pointer: "/MAP_ROUTE101/land/day" },
            slots: [
              {
                slotIndex: 0,
                speciesId: "SPECIES_ESPEON",
                speciesLabel: "Espeon",
                sprite: null,
                source: { ...source, pointer: "/MAP_ROUTE101/land/day/0" },
              },
            ],
            profiles: [
              {
                profileKey: "gWildV2_Route101/day/land_mons/NONE",
                fishingRod: "NONE",
                weights: [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1],
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
            sets: [{ baseLabel: "gWildV2_Route101_Day", source }],
          },
        ],
      },
      {
        product: "emerald",
        timeOfDay: "night",
        methods: [{ type: "land_mons", resolution: "unavailable", sets: [] }],
      },
    ],
  }
}

const mapSet = (encounters: Record<string, unknown>): Record<string, unknown> =>
  (encounters.sets as Array<Record<string, unknown>>)[0]!

const catalogWith = (encounters: Record<string, unknown>, overrides = {}) =>
  catalog({
    regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
    maps: [mapWithWildEncounters(encounters)],
    ...overrides,
  })

describe("validateCatalog", () => {
  it("rejects stale catalog schemas before the viewport can interpret their topology", () => {
    expect(() => validateCatalog(catalog({ schemaVersion: 1 }))).toThrow(CatalogValidationError)
    expect(() => validateCatalog(catalog({ schemaVersion: 1 }))).toThrow("schemaVersion must be 11")
  })

  it("rejects an empty or stale level projection", () => {
    expect(() =>
      validateCatalog(catalog({ wildEncounterProjection: { ...projection(), distributions: [] } })),
    ).toThrow("distributions must be a non-empty array")

    const model = projection()
    model.schemaVersion = 2
    expect(() => validateCatalog(catalog({ wildEncounterProjection: model }))).toThrow(
      "must use projection schemaVersion 3",
    )
  })

  it("rejects outcome lists that do not add up to one roll", () => {
    const model = projection()
    const distributions = model.distributions as Array<Record<string, unknown>>
    ;(distributions[0]!.byPlaceLevel as Record<string, unknown>)["5"] = [[0, 5, 49]]

    expect(() => validateCatalog(catalog({ wildEncounterProjection: model }))).toThrow(
      "has an invalid outcome list at place level 5",
    )
  })

  it("rejects outcomes that name an unknown species", () => {
    const model = projection()
    const distributions = model.distributions as Array<Record<string, unknown>>
    ;(distributions[0]!.byPlaceLevel as Record<string, unknown>)["5"] = [[3, 5, 50]]

    expect(() => validateCatalog(catalog({ wildEncounterProjection: model }))).toThrow(
      "has an invalid outcome list",
    )
  })

  it("requires every fishing method to weigh its slots for each rod", () => {
    const encounters = wildEncounters()
    const method = (mapSet(encounters).methods as Array<Record<string, unknown>>)[0]!
    method.type = "fishing_mons"

    expect(() => validateCatalog(catalogWith(encounters))).toThrow(
      "fishing_mons must have Old, Good, and Super Rod weights",
    )

    method.profiles = ["OLD_ROD", "GOOD_ROD", "SUPER_ROD"].map((fishingRod) => ({
      profileKey: fishingRod,
      fishingRod,
      weights: [100],
    }))
    expect(validateCatalog(catalogWith(encounters))).toBeDefined()
  })

  it("rejects slots without an outcome distribution at every place level", () => {
    const encounters = wildEncounters()
    const method = (mapSet(encounters).methods as Array<Record<string, unknown>>)[0]!
    const slots = method.slots as Array<Record<string, unknown>>
    slots[0]!.speciesId = "SPECIES_MISSING"
    expect(() => validateCatalog(catalogWith(encounters))).toThrow(
      "has no outcome distribution for SPECIES_MISSING",
    )

    slots[0]!.speciesId = "SPECIES_ESPEON"
    ;(mapSet(encounters).place as Record<string, unknown>).placeLevels = [5, 5, 7]
    expect(() => validateCatalog(catalogWith(encounters))).toThrow(
      "lacks outcomes at a place level",
    )
  })

  it("rejects places that do not cover the Trainer Rating range", () => {
    const encounters = wildEncounters()
    ;(mapSet(encounters).place as Record<string, unknown>).placeLevels = [5, 5]

    expect(() => validateCatalog(catalogWith(encounters))).toThrow(
      "place levels must cover the Trainer Rating range",
    )
  })

  it("requires an intent for dungeon places only", () => {
    const encounters = wildEncounters()
    ;(mapSet(encounters).place as Record<string, unknown>).reach = "Dungeon"

    expect(() => validateCatalog(catalogWith(encounters))).toThrow(
      "wildEncounters must contain valid source encounter data",
    )

    Object.assign(mapSet(encounters).place as Record<string, unknown>, {
      dungeon: { intent: "Mild", flat: false },
      floor: 0,
      floors: 2,
    })
    expect(validateCatalog(catalogWith(encounters))).toBeDefined()
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

  it("accepts the current catalog schema", () => {
    expect(validateCatalog(catalog()).schemaVersion).toBe(11)
  })

  it("rejects malformed or inconsistent build membership metadata", () => {
    expect(() => validateCatalog(catalog({ builds: [null] }))).toThrow(
      "builds[0] must include an ID, label, map count, and map names",
    )

    const value = catalog({
      builds: [{ id: "hns", label: "HNS", mapCount: 0, maps: [] }],
      regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
      maps: [mapWithWildEncounters(wildEncounters())],
    })

    expect(() => validateCatalog(value)).toThrow('Route101 refers to undeclared build "emerald"')

    const incompleteSummary = catalog({
      builds: [{ id: "emerald", label: "Emerald", mapCount: 0, maps: [] }],
      regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
      maps: [mapWithWildEncounters(wildEncounters())],
    })

    expect(() => validateCatalog(incompleteSummary)).toThrow("emerald has an incorrect map count")
    expect(() => validateCatalog(incompleteSummary)).toThrow(
      "emerald has an incorrect map membership list",
    )
  })

  it("accepts wild encounter sets and runtime time-of-day resolution", () => {
    const value = catalogWith(wildEncounters())

    expect(
      validateCatalog(value).maps[0]?.wildEncounters.runtimeTimes[1]?.methods[0]?.resolution,
    ).toBe("unavailable")
  })

  it("rejects incomplete source encounter data", () => {
    const encounters = wildEncounters()
    encounters.runtimeTimes = [
      { product: "emerald", timeOfDay: "day", methods: [{ type: "land_mons" }] },
    ]

    expect(() => validateCatalog(catalogWith(encounters))).toThrow(
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

  it("projects build-specific connection overrides without changing native builds", () => {
    const map = {
      ...mapWithWildEncounters(wildEncounters()),
      builds: ["hns", "wayfarer"],
      connections: [
        {
          direction: "down",
          offsetMetatiles: 0,
          destinationMapId: "MAP_NATIVE",
          destinationMap: "Native",
        },
      ],
      connectionOverrides: {
        wayfarer: [
          {
            direction: "down",
            offsetMetatiles: 0,
            destinationMapId: "MAP_WAYFARER",
            destinationMap: "Wayfarer",
          },
        ],
      },
    }
    const value = validateCatalog(
      catalog({
        builds: [
          { id: "hns", label: "HNS", mapCount: 1, maps: ["Route101"] },
          { id: "wayfarer", label: "Wayfarer", mapCount: 1, maps: ["Route101"] },
        ],
        regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
        maps: [map],
      }),
    )

    expect(mapsForBuild(value, "hns")[0]?.connections[0]?.destinationMap).toBe("Native")
    expect(mapsForBuild(value, "wayfarer")[0]?.connections[0]?.destinationMap).toBe("Wayfarer")
  })

  it("reports malformed build membership alongside connection overrides", () => {
    const map = {
      ...mapWithWildEncounters(wildEncounters()),
      builds: null,
      connectionOverrides: { wayfarer: [] },
    }

    expect(() =>
      validateCatalog(
        catalog({
          regions: [{ id: "routes", label: "Routes", mapCount: 1, maps: ["Route101"] }],
          maps: [map],
        }),
      ),
    ).toThrow(CatalogValidationError)
  })
})
