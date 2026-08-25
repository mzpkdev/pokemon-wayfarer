import * as path from "node:path"

import { describe, expect, it } from "vitest"

import { catalogEncounterSprites } from "./encounter-sprites"
import { catalogWildEncounters, sourceWildEncounters } from "./encounters"

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

  it("excludes Route5_hns's real NONE sentinel rows from player-facing methods", () => {
    const encounters = sourceWildEncounters(sourceRoot, mapNamesById).get("Route5_hns")
    const baseSet = encounters?.sets[0]
    const water = baseSet?.methods.find((method) => method.type === "water_mons")
    const fishing = baseSet?.methods.find((method) => method.type === "fishing_mons")

    expect(water?.slots).toEqual([])
    expect(fishing?.slots).toEqual([])
    expect(
      baseSet?.methods
        .flatMap((method) => method.slots)
        .some((slot) => slot.speciesId === "SPECIES_NONE"),
    ).toBe(false)
    expect(encounters?.diagnostics).toEqual(
      expect.arrayContaining([
        expect.objectContaining({
          code: "excluded_source_slot",
          reason: "species_none",
          methodType: "water_mons",
          speciesId: "SPECIES_NONE",
        }),
        expect.objectContaining({
          code: "excluded_source_slot",
          reason: "species_none",
          methodType: "fishing_mons",
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
