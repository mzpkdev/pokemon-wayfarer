import * as childProcess from "node:child_process"
import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

import { afterAll, beforeAll, describe, expect, it } from "vitest"

import { catalogEncounterSprites } from "./encounter-sprites"
import {
  catalogWildEncounters,
  sourceProductForBaseLabel,
  sourceWildEncounterCatalog,
  sourceWildEncounters,
} from "./encounters"
import { profileIndex, profileLookupKey } from "./projection"
import type { CatalogEncounterProjectionProfile, CatalogWildEncounterProjection } from "./types"

const sourceRoot = path.resolve(import.meta.dirname, "../../../../..", "game")

const mapNamesById = new Map([
  ["MAP_ROUTE101", "Route101"],
  ["MAP_ROUTE102", "Route102"],
  ["MAP_ROUTE30_HNS", "Route30_hns"],
  ["MAP_ROUTE32_HNS", "Route32_hns"],
  ["MAP_ROUTE5_HNS", "Route5_hns"],
  ["MAP_ALTERING_CAVE", "AlteringCave"],
  ["MAP_ALPHA", "Alpha"],
  ["MAP_BRAVO", "Bravo"],
])

const speciesLabelsById = new Map([
  ["SPECIES_EEVEE", "EEVEE"],
  ["SPECIES_UMBREON", "UMBREON"],
  ["SPECIES_PICHU", "PICHU"],
])

const slot = (species: string): Record<string, unknown> => {
  return { min_level: 3, max_level: 5, species }
}

const encounterDocument = (
  encounters: Record<string, unknown>[],
  encounterRates = [100],
): Record<string, unknown> => {
  return {
    wild_encounter_groups: [
      {
        label: "gWildMonHeaders",
        for_maps: true,
        fields: [{ type: "land_mons", encounter_rates: encounterRates }],
        encounters,
      },
    ],
  }
}

describe("source wild encounters", () => {
  it("leaves a source-derived placeholder for encounter species without an icon", () => {
    const spriteForSpecies = catalogEncounterSprites(sourceRoot, "/tmp/cartographer-icons")

    expect(spriteForSpecies("SPECIES_NOT_A_POKEMON")).toBeNull()
  })

  it("preserves Route102's raw sets, method slots, rates, species, and provenance", () => {
    const encounters = sourceWildEncounters(sourceRoot, mapNamesById).get("Route102")

    expect(encounters?.sets.map((set) => set.baseLabel)).toEqual(["gRoute102"])
    const fishing = encounters?.sets[0]?.methods.find((method) => method.type === "fishing_mons")
    expect(fishing).toMatchObject({
      encounterRate: 30,
      source: {
        path: "src/data/wild_encounters.json",
        pointer: "/wild_encounter_groups/0/encounters/1/fishing_mons",
      },
    })
    expect(fishing?.slots).toContainEqual(
      expect.objectContaining({
        slotIndex: 0,
        slotRate: 70,
        minLevel: 5,
        maxLevel: 10,
        speciesId: "SPECIES_MAGIKARP",
        speciesLabel: "MAGIKARP",
        groups: expect.arrayContaining([expect.objectContaining({ id: "old_rod" })]),
      }),
    )
    expect(fishing?.slots[0]?.slotRateSource.pointer).toBe(
      "/wild_encounter_groups/0/fields/3/encounter_rates/0",
    )
  })

  it("uses the Wayfarer fallback table when Route101 has no time-labelled source tables", () => {
    const encounters = sourceWildEncounters(sourceRoot, mapNamesById).get("Route101")

    expect(encounters?.sets.map((set) => set.baseLabel)).toEqual(["gRoute101"])
    expect(
      encounters?.runtimeTimes.map((time) => [
        time.timeOfDay,
        time.methods[0]?.resolution,
        time.methods[0]?.sets[0]?.baseLabel,
      ]),
    ).toEqual([
      ["morning", "fallback", "gRoute101"],
      ["day", "direct", "gRoute101"],
      ["evening", "fallback", "gRoute101"],
      ["night", "fallback", "gRoute101"],
    ])
  })

  it("keeps Route30_hns's runtime-unaddressable fishing rows as source diagnostics", () => {
    const encounters = sourceWildEncounters(sourceRoot, mapNamesById).get("Route30_hns")
    const fishing = encounters?.sets[0]?.methods.find((method) => method.type === "fishing_mons")

    expect(fishing?.slots).toHaveLength(10)
    expect(encounters?.diagnostics).toEqual(
      expect.arrayContaining([
        expect.objectContaining({
          code: "unaddressable_source_slot",
          reason: "outside_method_slot_table",
          methodType: "fishing_mons",
          slotIndex: 10,
          speciesId: "SPECIES_MAGIKARP",
        }),
        expect.objectContaining({
          code: "unaddressable_source_slot",
          reason: "outside_method_slot_table",
          methodType: "fishing_mons",
          slotIndex: 11,
          speciesId: "SPECIES_POLIWAG",
        }),
      ]),
    )
  })

  it("retains Route5_hns's ten fishing slots while excluding non-fishing NONE rows", () => {
    const encounters = sourceWildEncounters(sourceRoot, mapNamesById).get("Route5_hns")
    const baseSet = encounters?.sets[0]
    const water = baseSet?.methods.find((method) => method.type === "water_mons")
    const fishing = baseSet?.methods.find((method) => method.type === "fishing_mons")

    expect(water?.slots).toEqual([])
    expect(fishing?.slots).toHaveLength(10)
    expect(fishing?.slots.every((slot) => slot.speciesId === "SPECIES_NONE")).toBe(true)
    expect(encounters?.diagnostics).toEqual(
      expect.arrayContaining([
        expect.objectContaining({
          code: "excluded_source_slot",
          reason: "species_none",
          methodType: "water_mons",
          speciesId: "SPECIES_NONE",
        }),
      ]),
    )
  })

  it("groups Route32_hns's Day and Night rows as runtime time-of-day tables", () => {
    const encounters = sourceWildEncounters(sourceRoot, mapNamesById).get("Route32_hns")

    expect(
      encounters?.runtimeTimes.map((time) => [
        time.timeOfDay,
        time.methods[0]?.resolution,
        time.methods[0]?.sets[0]?.baseLabel,
      ]),
    ).toEqual([
      ["morning", "fallback", "gRoute32_hns_Day"],
      ["day", "direct", "gRoute32_hns_Day"],
      ["evening", "fallback", "gRoute32_hns_Day"],
      ["night", "direct", "gRoute32_hns_Night"],
    ])
  })

  it("preserves Altering Cave's source-order selector rows without time labels", () => {
    const encounters = sourceWildEncounters(sourceRoot, mapNamesById).get("AlteringCave")

    expect(encounters?.sets.map((set) => set.baseLabel)).toEqual([
      "gAlteringCave1",
      "gAlteringCave2",
      "gAlteringCave3",
      "gAlteringCave4",
      "gAlteringCave5",
      "gAlteringCave6",
      "gAlteringCave7",
      "gAlteringCave8",
      "gAlteringCave9",
    ])
    expect(encounters?.runtimeTimes).toEqual([])
  })

  it("uses time labels and the configured fallback instead of header position", () => {
    const encounters = catalogWildEncounters(
      encounterDocument([
        {
          map: "MAP_ALPHA",
          base_label: "gAlpha_Day",
          land_mons: { encounter_rate: 20, mons: [slot("SPECIES_EEVEE")] },
        },
        {
          map: "MAP_ALPHA",
          base_label: "gAlpha_Night",
          land_mons: { encounter_rate: 20, mons: [slot("SPECIES_UMBREON")] },
        },
        {
          map: "MAP_BRAVO",
          base_label: "next_map",
          land_mons: { encounter_rate: 20, mons: [slot("SPECIES_PICHU")] },
        },
      ]),
      mapNamesById,
      speciesLabelsById,
    ).get("Alpha")

    expect(
      encounters?.runtimeTimes.map((time) => [
        time.timeOfDay,
        time.methods[0]?.resolution,
        time.methods[0]?.sets[0]?.baseLabel,
      ]),
    ).toEqual([
      ["morning", "fallback", "gAlpha_Day"],
      ["day", "direct", "gAlpha_Day"],
      ["evening", "fallback", "gAlpha_Day"],
      ["night", "direct", "gAlpha_Night"],
    ])
  })

  it("hides NONE and zero-weight source slots while retaining their diagnostics", () => {
    const encounters = catalogWildEncounters(
      encounterDocument(
        [
          {
            map: "MAP_ALPHA",
            base_label: "with_excluded_slots",
            land_mons: {
              encounter_rate: 20,
              mons: [slot("SPECIES_NONE"), slot("SPECIES_EEVEE"), slot("SPECIES_UMBREON")],
            },
          },
        ],
        [25, 0, 50],
      ),
      mapNamesById,
      speciesLabelsById,
    ).get("Alpha")

    expect(encounters?.sets[0]?.methods[0]?.slots).toEqual([
      expect.objectContaining({
        slotIndex: 2,
        speciesId: "SPECIES_UMBREON",
        speciesLabel: "UMBREON",
        slotRate: 50,
      }),
    ])
    expect(encounters?.diagnostics).toEqual([
      expect.objectContaining({ reason: "species_none", speciesId: "SPECIES_NONE", slotRate: 25 }),
      expect.objectContaining({
        reason: "zero_slot_rate",
        speciesId: "SPECIES_EEVEE",
        slotRate: 0,
      }),
    ])
  })

  it("retains inverted authored ranges with their normalized runtime envelope", () => {
    const encounters = catalogWildEncounters(
      encounterDocument([
        {
          map: "MAP_ALPHA",
          base_label: "inverted",
          land_mons: {
            encounter_rate: 20,
            mons: [{ min_level: 43, max_level: 42, species: "SPECIES_EEVEE" }],
          },
        },
      ]),
      mapNamesById,
      speciesLabelsById,
    ).get("Alpha")

    expect(encounters?.sets[0]?.methods[0]?.slots[0]).toMatchObject({
      minLevel: 43,
      maxLevel: 42,
      runtimeMinLevel: 42,
      runtimeMaxLevel: 43,
    })
    expect(encounters?.diagnostics).toContainEqual(
      expect.objectContaining({
        code: "invalid_source_slot",
        reason: "invalid_level_range",
        minLevel: 43,
        maxLevel: 42,
      }),
    )
  })

  it("fails generation when method slots do not match the source field definition", () => {
    expect(() =>
      catalogWildEncounters(
        encounterDocument([
          {
            map: "MAP_ALPHA",
            base_label: "broken",
            land_mons: { encounter_rate: 20, mons: [] },
          },
        ]),
        mapNamesById,
        speciesLabelsById,
      ),
    ).toThrow("expected at least one source slot")
  })

  it("fails generation when a player-facing species has no name source entry", () => {
    expect(() =>
      catalogWildEncounters(
        encounterDocument([
          {
            map: "MAP_ALPHA",
            base_label: "unknown_species",
            land_mons: { encounter_rate: 20, mons: [slot("SPECIES_UNKNOWN")] },
          },
        ]),
        mapNamesById,
        speciesLabelsById,
      ),
    ).toThrow("has no Pokémon Wayfarer species label source entry")
  })
})

describe("generated Trainer Rating projection joins", () => {
  let temporaryDirectory = ""
  let projectionPath = ""
  let catalog: ReturnType<typeof sourceWildEncounterCatalog>

  beforeAll(() => {
    temporaryDirectory = fs.mkdtempSync(path.join(os.tmpdir(), "wayfarer-cartographer-projection-"))
    projectionPath = path.join(temporaryDirectory, "projection.json")
    childProcess.execFileSync(
      "python3",
      [
        path.join(sourceRoot, "tools/wild_encounters/wild_encounters_to_header.py"),
        "--cartographer-projection",
        projectionPath,
      ],
      { cwd: sourceRoot },
    )
    catalog = sourceWildEncounterCatalog(
      sourceRoot,
      new Map([
        ["MAP_ROUTE101", "Route101"],
        ["MAP_ROUTE102", "Route102"],
        ["MAP_MT_SILVER_SNOW_HNS", "MtSilverSnow_hns"],
        ["MAP_ROUTE18", "Route18"],
      ]),
      temporaryDirectory,
      projectionPath,
      () => null,
    )
  })

  afterAll(() => {
    fs.rmSync(temporaryDirectory, { force: true, recursive: true })
  })

  it("decorates projection species and joins exact product profile references", () => {
    expect(catalog.projection.schemaVersion).toBe(2)
    expect(catalog.projection.trainerRating).toEqual({ minimum: 10, maximum: 80 })
    expect(
      catalog.projection.species.find((species) => species.authoredSpecies === "SPECIES_MAGIKARP"),
    ).toMatchObject({ speciesLabel: "MAGIKARP", sprite: null })

    const route101 = catalog.encountersByMap.get("Route101")?.sets[0]
    expect(route101).toMatchObject({ product: "EMERALD", runtimeTime: "day" })
    expect(route101?.methods[0]?.profiles[0]).toEqual({
      profileKey: "EMERALD/gRoute101/land_mons/NONE",
      fishingRod: "NONE",
      levelOffset: 0,
    })
  })

  it("joins all three ten-slot fishing profiles with their exact generated weights", () => {
    const fishing = catalog.encountersByMap
      .get("Route102")
      ?.sets[0]?.methods.find((method) => method.type === "fishing_mons")

    expect(fishing?.profiles.map((profile) => profile.fishingRod)).toEqual([
      "OLD_ROD",
      "GOOD_ROD",
      "SUPER_ROD",
    ])
    expect(
      fishing?.profiles.map((reference) =>
        profileIndex(catalog.projection).get(reference.profileKey),
      ),
    ).toMatchObject([
      { runtimeSlotCount: 10, weights: [38, 22, 10, 8, 8, 4, 3, 3, 2, 2] },
      { runtimeSlotCount: 10, weights: [25, 18, 12, 10, 9, 7, 6, 5, 4, 4] },
      { runtimeSlotCount: 10, weights: [12, 10, 11, 10, 10, 10, 10, 9, 9, 9] },
    ])
  })

  it("rejects malformed fishing weights in projection schema 2", () => {
    const malformed = JSON.parse(fs.readFileSync(projectionPath, "utf8")) as {
      profiles: Array<{ method: string; weights?: number[] }>
    }
    const fishing = malformed.profiles.find((profile) => profile.method === "fishing_mons")
    expect(fishing).toBeDefined()
    fishing!.weights = [100]
    const malformedPath = path.join(temporaryDirectory, "malformed-projection.json")
    fs.writeFileSync(malformedPath, JSON.stringify(malformed))

    expect(() =>
      sourceWildEncounterCatalog(
        sourceRoot,
        new Map([["MAP_ROUTE101", "Route101"]]),
        temporaryDirectory,
        malformedPath,
        () => null,
      ),
    ).toThrow("expected exactly 10 weights")
  })

  it("rejects projection schema 1", () => {
    const stale = JSON.parse(fs.readFileSync(projectionPath, "utf8")) as {
      schemaVersion: number
    }
    stale.schemaVersion = 1
    const stalePath = path.join(temporaryDirectory, "stale-projection.json")
    fs.writeFileSync(stalePath, JSON.stringify(stale))

    expect(() =>
      sourceWildEncounterCatalog(
        sourceRoot,
        new Map([["MAP_ROUTE101", "Route101"]]),
        temporaryDirectory,
        stalePath,
        () => null,
      ),
    ).toThrow("schemaVersion: expected 2")
  })

  it("uses generator runtime identity and keeps version populations separate", () => {
    expect(
      catalog.encountersByMap
        .get("MtSilverSnow_hns")
        ?.sets.find((set) => set.baseLabel === "gMtSilver_SnowNight_hns_Day")?.runtimeTime,
    ).toBe("night")

    const route18 = catalog.encountersByMap.get("Route18")
    expect(new Set(route18?.sets.map((set) => set.product))).toEqual(
      new Set(["FIRERED", "LEAFGREEN"]),
    )
    expect(new Set(route18?.runtimeTimes.map((time) => time.product))).toEqual(
      new Set(["FIRERED", "LEAFGREEN"]),
    )
  })

  it("indexes an otherwise identical profile separately for every product", () => {
    const base: Omit<CatalogEncounterProjectionProfile, "product" | "profileKey"> = {
      map: "MAP_ALPHA",
      baseLabel: "shared_label",
      header: "shared_label",
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
    }
    const profiles: CatalogEncounterProjectionProfile[] = ["FIRERED", "LEAFGREEN"].map(
      (product) => ({
        ...base,
        product: product as "FIRERED" | "LEAFGREEN",
        profileKey: `${product}/shared_label/land_mons/NONE`,
      }),
    )
    const indexed = profileIndex({ profiles } as CatalogWildEncounterProjection)

    expect(indexed.get(profileLookupKey("FIRERED", "shared_label", "land_mons", "NONE"))).toBe(
      profiles[0],
    )
    expect(indexed.get(profileLookupKey("LEAFGREEN", "shared_label", "land_mons", "NONE"))).toBe(
      profiles[1],
    )
  })

  it("rejects an ambiguous source product label before profile lookup", () => {
    expect(() => sourceProductForBaseLabel("gSharedFireRed_LeafGreen")).toThrow(
      "source label has ambiguous product markers",
    )
  })
})
