import * as path from "node:path"

import type { CatalogRegion } from "./types"

const kantoNamedMaps = new Set([
  "CeladonCity",
  "CeladonCity_Apartments_RoofNight",
  "CeladonCity_DepartmentStore_RoofNight",
  "CeruleanCity",
  "CinnabarIsland",
  "FuchsiaCity",
  "FuchsiaCity_SafariZoneBeach",
  "FuchsiaCity_SafariZoneBrush",
  "FuchsiaCity_SafariZoneMountain",
  "IndigoPlateau",
  "LavenderTown",
  "MtMoon_Outside",
  "PalletTown",
  "PewterCity",
  "SaffronCity",
  "VermilionCity",
  "VermilionCity_PortOutside",
  "ViridianCity",
  "ViridianForest",
])

const johto: CatalogRegion = { id: "johto", label: "Johto" }
const kanto: CatalogRegion = { id: "kanto", label: "Kanto" }
const hoenn: CatalogRegion = { id: "hoenn", label: "Hoenn" }
const alola: CatalogRegion = { id: "alola", label: "Alola" }

export const catalogRegions: CatalogRegion[] = [johto, kanto, hoenn, alola]

const hoennHnsMapSections = new Set([
  "MAPSEC_BATTLE_FRONTIER",
  "MAPSEC_TRAINER_HILL",
  "MAPSEC_SOUTHERN_ISLAND",
])

export const regionFor = (name: string, group: string, mapSection?: string): CatalogRegion => {
  if (group.includes("Alola")) return alola
  if (group.endsWith("_Frlg")) return kanto
  if (!group.endsWith("_Hns")) {
    return hoenn
  }
  if (mapSection && hoennHnsMapSections.has(mapSection)) return hoenn
  const baseName = name.replace(/_hns$/, "")
  const route = /^Route(\d+)(?:North)?$/.exec(baseName)
  if ((route && Number(route[1]) <= 28) || kantoNamedMaps.has(baseName)) {
    return kanto
  }
  return johto
}

export const categoryFor = (mapType: string): string => {
  if (mapType === "MAP_TYPE_TOWN" || mapType === "MAP_TYPE_CITY") {
    return "towns"
  }
  if (mapType === "MAP_TYPE_UNDERWATER") {
    return "underwater"
  }
  return "routes"
}

export const mapOutputPaths = (
  output: string,
  region: string,
  category: string,
  mapName: string,
): { native: string; overview: string } => {
  return {
    native: path.join(output, "maps", region, category, `${mapName}.png`),
    overview: path.join(output, "overviews", region, category, `${mapName}.png`),
  }
}
