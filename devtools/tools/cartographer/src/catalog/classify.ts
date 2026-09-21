import * as path from "node:path"

import type { CatalogBuild, CatalogBuildId, CatalogRegion } from "./types"

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
export const sinnoh: CatalogRegion = { id: "sinnoh", label: "Sinnoh" }

// Keep a no-Sinnoh source checkout byte-stable. Sinnoh joins generated region
// indexes only after a map with explicit Sinnoh provenance is actually present.
export const catalogRegions: CatalogRegion[] = [johto, kanto, hoenn, alola]
const allCatalogRegions: CatalogRegion[] = [...catalogRegions, sinnoh]

export const catalogRegionsFor = (regionIds: Iterable<string>): CatalogRegion[] => {
  const present = new Set(regionIds)
  return allCatalogRegions.filter((region) => catalogRegions.includes(region) || present.has(region.id))
}

/**
 * Build targets mirror mapjson's source_version_is_selected logic. FireRed and
 * LeafGreen share the FRLG map source; Wayfarer combines Emerald and HNS maps.
 */
export const catalogBuilds: CatalogBuild[] = [
  { id: "wayfarer", label: "Wayfarer" },
  { id: "emerald", label: "Emerald" },
  { id: "firered", label: "FireRed" },
  { id: "leafgreen", label: "LeafGreen" },
  { id: "hns", label: "HNS" },
]

const buildsBySourceVersion: Record<string, CatalogBuildId[]> = {
  emerald: ["emerald", "wayfarer"],
  frlg: ["firered", "leafgreen"],
  hns: ["hns", "wayfarer"],
  sinnoh: ["wayfarer"],
}

/** Resolve source metadata into the game builds that include a map. */
export const buildsForSourceVersion = (sourceVersion: string | undefined): CatalogBuildId[] => {
  const source = sourceVersion ?? "emerald"
  const builds = buildsBySourceVersion[source]
  if (!builds) throw new Error(`Unsupported map source version ${JSON.stringify(source)}`)
  return builds
}

/** Source provenance is reported independently from a map's physical region. */
export const sourceRegionFor = (sourceVersion: string | undefined): "sinnoh" | null =>
  sourceVersion === "sinnoh" ? "sinnoh" : null

const hoennHnsMapSections = new Set([
  "MAPSEC_BATTLE_FRONTIER",
  "MAPSEC_TRAINER_HILL",
  "MAPSEC_SOUTHERN_ISLAND",
])

export const regionFor = (
  name: string,
  group: string,
  mapSection?: string,
  sourceVersion?: string,
): CatalogRegion => {
  // Provenance, not a source group's name or a map-section range, owns the
  // physical region for imported Sinnoh maps.
  if (sourceVersion === "sinnoh") return sinnoh
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
