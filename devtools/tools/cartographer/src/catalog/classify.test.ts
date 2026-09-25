import { describe, expect, it } from "vitest"

import {
  buildsForSourceVersion,
  catalogRegions,
  catalogRegionsFor,
  regionFor,
  sourceRegionFor,
} from "./classify"

describe("Wayfarer map regions", () => {
  it("retains each source map family's native region", () => {
    expect(regionFor("Route101", "gMapGroup_TownsAndRoutes").id).toBe("hoenn")
    expect(regionFor("PalletTown_Frlg", "gMapGroup_TownsAndRoutes_Frlg").id).toBe("kanto")
    expect(
      regionFor(
        "OneIsland_Frlg",
        "gMapGroup_OutdoorOneIsland_Frlg",
        "MAPSEC_ONE_ISLAND",
        "frlg",
        true,
      ).id,
    ).toBe("sevii")
    expect(regionFor("Route5_hns", "gMapGroup_TownsAndRoutes_Hns").id).toBe("kanto")
    expect(regionFor("Route30_hns", "gMapGroup_TownsAndRoutes_Hns").id).toBe("johto")
    expect(regionFor("Akala_Forest_hns", "gMapGrouop_OutdoorAlola_Hns").id).toBe("alola")
    expect(
      regionFor(
        "BattleFrontier_OutsideEast_hns",
        "gMapGroup_SpecialArea_Hns",
        "MAPSEC_BATTLE_FRONTIER",
      ).id,
    ).toBe("hoenn")
    expect(
      regionFor("Route201", "gMapGroup_TownsAndRoutes", "MAPSEC_LITTLEROOT_TOWN", "sinnoh").id,
    ).toBe("sinnoh")
    expect(sourceRegionFor("sinnoh")).toBe("sinnoh")
    expect(sourceRegionFor("hns")).toBeNull()
    expect(
      regionFor("TrainerHill_Courtyard_hns", "gMapGroup_SpecialArea_Hns", "MAPSEC_TRAINER_HILL").id,
    ).toBe("hoenn")
    expect(
      regionFor(
        "SouthernIsland_Exterior_hns",
        "gMapGroup_SpecialArea_Hns",
        "MAPSEC_SOUTHERN_ISLAND",
      ).id,
    ).toBe("hoenn")
  })

  it("publishes the regions represented by Wayfarer maps", () => {
    expect(catalogRegions.map((region) => region.id)).toEqual([
      "johto",
      "kanto",
      "sevii",
      "hoenn",
      "alola",
    ])
    expect(catalogRegionsFor(["sinnoh"]).map((region) => region.id)).toEqual([
      "johto",
      "kanto",
      "sevii",
      "hoenn",
      "alola",
      "sinnoh",
    ])
  })
})

describe("Cartographer map build membership", () => {
  it("includes native sources and explicit Wayfarer imports in the correct builds", () => {
    expect(buildsForSourceVersion(undefined)).toEqual(["emerald", "wayfarer"])
    expect(buildsForSourceVersion("frlg")).toEqual(["firered", "leafgreen"])
    expect(buildsForSourceVersion("frlg", true)).toEqual(["firered", "leafgreen", "wayfarer"])
    expect(buildsForSourceVersion("hns")).toEqual(["hns", "wayfarer"])
    expect(buildsForSourceVersion("sinnoh")).toEqual(["wayfarer"])
  })

  it("rejects unknown source versions instead of inferring a build from a map name", () => {
    expect(() => buildsForSourceVersion("crystal")).toThrow(
      'Unsupported map source version "crystal"',
    )
  })
})
