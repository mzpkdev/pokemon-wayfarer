import * as childProcess from "node:child_process"
import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

import { afterAll, beforeAll, describe, expect, it } from "vitest"

import { catalogEncounterSprites } from "./encounter-sprites"
import { catalogWildEncounters, sourceWildEncounterCatalog } from "./encounters"

const sourceRoot = path.resolve(import.meta.dirname, "../../../../..", "game")

const mapNamesById = new Map([
  ["MAP_ROUTE1_HNS", "Route1_hns"],
  ["MAP_ROUTE102", "Route102"],
  ["MAP_MT_SILVER_SNOW_HNS", "MtSilverSnow_hns"],
  ["MAP_NATIONAL_PARK_BUG_CONTEST_HNS", "NationalParkBugContest_hns"],
])

describe("source encounter icons", () => {
  it("leaves a source-derived placeholder for encounter species without an icon", () => {
    const spriteForSpecies = catalogEncounterSprites(sourceRoot, "/tmp/cartographer-icons")

    expect(spriteForSpecies("SPECIES_NOT_A_POKEMON")).toBeNull()
  })
})

describe("generated v2 encounter projection", () => {
  let temporaryDirectory = ""
  let projectionPath = ""
  let catalog: ReturnType<typeof sourceWildEncounterCatalog>

  beforeAll(() => {
    temporaryDirectory = fs.mkdtempSync(path.join(os.tmpdir(), "wayfarer-cartographer-projection-"))
    projectionPath = path.join(temporaryDirectory, "projection.json")
    childProcess.execFileSync(
      "python3",
      [
        path.join(sourceRoot, "tools/wild_encounters/v2/cartographer_projection.py"),
        projectionPath,
      ],
      { cwd: sourceRoot },
    )
    catalog = sourceWildEncounterCatalog(
      sourceRoot,
      mapNamesById,
      temporaryDirectory,
      projectionPath,
      () => null,
    )
  }, 120_000)

  afterAll(() => {
    fs.rmSync(temporaryDirectory, { force: true, recursive: true })
  })

  it("covers Trainer Rating 0 through 160 for the Wayfarer product only", () => {
    expect(catalog.projection.schemaVersion).toBe(3)
    expect(catalog.projection.trainerRating).toEqual({ minimum: 0, maximum: 160 })
    expect(catalog.projection.products).toEqual([
      { id: "POKEMON_WAYFARER", displayName: "Pokémon Wayfarer" },
    ])
  })

  it("gives a map a day set and a night set that carry its place and per-method tables", () => {
    const route = catalog.encountersByMap.get("Route1_hns")

    expect(route?.sets.map((set) => [set.baseLabel, set.runtimeTime])).toEqual([
      ["gWildV2_Route1_Hns_Day", "day"],
      ["gWildV2_Route1_Hns_Night", "night"],
    ])
    const day = route!.sets[0]!
    expect(day.place).toMatchObject({
      name: "Route 1",
      region: "Kanto",
      reach: "Road",
      dungeon: null,
      floor: null,
      floors: null,
    })
    expect(day.place.placeLevels).toHaveLength(161)
    expect(day.place.placeLevels[0]).toBe(5)
    expect(day.place.placeLevels[160]).toBe(74)
    expect(day.source).toEqual({
      path: "src/data/wild_encounters_v2/kanto.json",
      pointer: "/MAP_ROUTE1_HNS",
    })
    expect(day.methods.map((method) => method.type)).toEqual(["land_mons"])
    const land = day.methods[0]!
    expect(land.encounterRate).toBe(12) // Route 1 is a Road: 60% of its table rate of 20
    expect(land.slots).toHaveLength(12)
    expect(land.slots[0]).toMatchObject({ slotIndex: 0, speciesId: "SPECIES_PIDGEOT" })
    expect(land.profiles).toEqual([
      {
        profileKey: "gWildV2_Route1_Hns/day/land_mons/NONE",
        fishingRod: "NONE",
        weights: [20, 20, 10, 10, 10, 10, 5, 5, 4, 4, 1, 1],
      },
    ])
  })

  it("resolves Morning and Day to the day tables and Evening and Night to the night tables", () => {
    const route = catalog.encountersByMap.get("Route1_hns")

    expect(
      route?.runtimeTimes.map((time) => [
        time.timeOfDay,
        time.methods.find((method) => method.type === "land_mons")?.resolution,
        time.methods.find((method) => method.type === "land_mons")?.sets[0]?.baseLabel,
      ]),
    ).toEqual([
      ["morning", "direct", "gWildV2_Route1_Hns_Day"],
      ["day", "direct", "gWildV2_Route1_Hns_Day"],
      ["evening", "direct", "gWildV2_Route1_Hns_Night"],
      ["night", "direct", "gWildV2_Route1_Hns_Night"],
    ])
    expect(
      route?.runtimeTimes[0]?.methods.find((method) => method.type === "fishing_mons")?.resolution,
    ).toBe("unavailable")
  })

  it("weights fishing slots per rod and keeps all ten slots", () => {
    const fishing = catalog.encountersByMap
      .get("Route102")
      ?.sets[0]?.methods.find((method) => method.type === "fishing_mons")

    expect(fishing?.slots).toHaveLength(10)
    expect(fishing?.profiles.map((profile) => profile.fishingRod)).toEqual([
      "OLD_ROD",
      "GOOD_ROD",
      "SUPER_ROD",
    ])
    for (const profile of fishing?.profiles ?? []) {
      expect(profile.weights).toHaveLength(10)
      expect(profile.weights.reduce((sum, weight) => sum + weight, 0)).toBe(100)
    }
  })

  it("records the dungeon intent and floor of a dungeon place", () => {
    const silver = catalog.encountersByMap.get("MtSilverSnow_hns")?.sets[0]

    expect(silver?.place).toMatchObject({
      reach: "Dungeon",
      dungeon: { intent: "Brutal", flat: false },
      floor: 4,
      floors: 6,
    })
    expect(silver?.place.placeLevels[0]).toBe(50) // Brutal floors start at 50
  })

  it("keeps the Bug Contest weekdays as separate tables without a time-of-day resolution", () => {
    const contest = catalog.encountersByMap.get("NationalParkBugContest_hns")

    expect(contest?.sets.map((set) => set.variant)).toEqual([
      "TUESDAY",
      "TUESDAY",
      "THURSDAY",
      "THURSDAY",
      "SATURDAY",
      "SATURDAY",
    ])
    expect(contest?.sets[0]?.place.regionClass).toBe("Safari")
    expect(contest?.runtimeTimes).toEqual([])
  })

  it("lists every slot's exact outcomes at the place levels it can reach", () => {
    const { projection } = catalog
    const speciesIdAt = (index: number): string | undefined => projection.species[index]?.speciesId
    const igglybuff = projection.distributions.find(
      (entry) => entry.speciesId === "SPECIES_IGGLYBUFF" && entry.regionClass === "Other",
    )

    // Route 1 is at place level 20 at Trainer Rating 40; a baby never rolls above level 10.
    const outcomes = igglybuff?.byPlaceLevel["20"] ?? []
    expect(outcomes.length).toBeGreaterThan(0)
    expect(outcomes.map(([species]) => speciesIdAt(species))).toEqual(
      outcomes.map(() => "SPECIES_IGGLYBUFF"),
    )
    expect(Math.max(...outcomes.map(([, level]) => level))).toBe(10)
    expect(outcomes.reduce((sum, [, , weight]) => sum + weight, 0)).toBe(
      projection.outcomeDenominator,
    )
  })

  it("fails generation when a species has no name source entry", () => {
    const document = JSON.parse(fs.readFileSync(projectionPath, "utf8"))

    expect(() => catalogWildEncounters(document, mapNamesById, new Map(), () => null)).toThrow(
      "has no species label source entry",
    )
  })

  it("rejects a projection with an unexpected schema version", () => {
    const document = JSON.parse(fs.readFileSync(projectionPath, "utf8")) as {
      schemaVersion: number
    }
    document.schemaVersion = 2

    expect(() => catalogWildEncounters(document, mapNamesById, new Map())).toThrow(
      "schemaVersion: expected 3",
    )
  })
})
