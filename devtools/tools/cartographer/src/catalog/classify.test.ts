import { describe, expect, it } from "vitest"

import { catalogRegions, regionFor } from "./classify"

describe("Wayfarer map regions", () => {
  it("retains each source map family's native region", () => {
    expect(regionFor("Route101", "gMapGroup_TownsAndRoutes").id).toBe("hoenn")
    expect(regionFor("PalletTown_Frlg", "gMapGroup_TownsAndRoutes_Frlg").id).toBe("kanto")
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
    expect(catalogRegions.map((region) => region.id)).toEqual(["johto", "kanto", "hoenn", "alola"])
  })
})
