import * as childProcess from "node:child_process"
import * as crypto from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"

import type {
  CatalogWayfarerMembership,
  Layout,
  LayoutDocument,
  MapCatalog,
  MapConnection,
  MapGroups,
  SourceMap,
} from "./types"

const readJson = <T>(filePath: string): T => {
  return JSON.parse(fs.readFileSync(filePath, "utf8")) as T
}

export type WayfarerSeviiSelection = {
  maps: ReadonlySet<string>
  releaseMaps: ReadonlySet<string>
}

// Cartographer renders exterior maps only. These imports and replacements are
// the exterior-relevant portion of mapjson's Wayfarer catalog policy.
const wayfarerSelectedFrlgMaps = new Set([
  "Route19_Frlg",
  "Route20_Frlg",
  "Route21_North_Frlg",
  "Route21_South_Frlg",
  "CinnabarIsland_Frlg",
  "CinnabarIsland_Gym_Frlg",
  "CinnabarIsland_PokemonLab_Entrance_Frlg",
  "CinnabarIsland_PokemonLab_Lounge_Frlg",
  "CinnabarIsland_PokemonLab_ResearchRoom_Frlg",
  "CinnabarIsland_PokemonLab_ExperimentRoom_Frlg",
  "CinnabarIsland_PokemonCenter_1F_Frlg",
  "CinnabarIsland_PokemonCenter_2F_Frlg",
  "CinnabarIsland_Mart_Frlg",
  "PokemonMansion_1F_Frlg",
  "PokemonMansion_2F_Frlg",
  "PokemonMansion_3F_Frlg",
  "PokemonMansion_B1F_Frlg",
  "SeafoamIslands_1F_Frlg",
  "SeafoamIslands_B1F_Frlg",
  "SeafoamIslands_B2F_Frlg",
  "SeafoamIslands_B3F_Frlg",
  "SeafoamIslands_B4F_Frlg",
])

const wayfarerReplacedHnsMapIds = new Set([
  "MAP_CINNABAR_ISLAND_HNS",
  "MAP_CINNABAR_ISLAND_POKEMON_CENTER_HNS",
  "MAP_SEAFOAM_ISLANDS_1F_HNS",
  "MAP_SEAFOAM_ISLANDS_B1F_HNS",
  "MAP_SEAFOAM_ISLANDS_GYM_HNS",
  "MAP_SEAFOAM_ISLANDS_SECRET_CAVE_HNS",
  "MAP_ROUTE21_HNS",
  "MAP_ROUTE19_HNS",
  "MAP_ROUTE20_HNS",
  "MAP_ROUTE19_CAVE_HNS",
  "MAP_FUCHSIA_ROUTE19GATE_HNS",
  "MAP_CINNABAR_SEAM_POC",
  "MAP_ROUTE19_COAST_POC",
  "MAP_ROUTE20_COAST_POC",
  "MAP_ROUTE21_NORTH_COAST_POC",
  "MAP_ROUTE21_SOUTH_COAST_POC",
  "MAP_SEAFOAM_ISLANDS_1F_COAST_POC",
  "MAP_SEAFOAM_ISLANDS_B1F_COAST_POC",
  "MAP_SEAFOAM_ISLANDS_B2F_COAST_POC",
  "MAP_SEAFOAM_ISLANDS_B3F_COAST_POC",
  "MAP_SEAFOAM_ISLANDS_B4F_COAST_POC",
])

const isRetiredHnsPort = (name: string, id: string): boolean =>
  (id.endsWith("_PORT") &&
    (id.startsWith("MAP_CINNABAR_ISLAND_") || id.startsWith("MAP_POKEMON_MANSION_"))) ||
  ((name.startsWith("CinnabarIsland_") || name.startsWith("PokemonMansion_")) &&
    name.endsWith("_Port"))

export const sourceWayfarerMembership = (
  name: string,
  source: SourceMap,
  sevii: WayfarerSeviiSelection,
): CatalogWayfarerMembership => {
  const sourceVersion = source.game_version ?? "emerald"
  if (sourceVersion === "frlg") {
    return source.wayfarer_include ||
      wayfarerSelectedFrlgMaps.has(name) ||
      sevii.releaseMaps.has(name)
      ? "include"
      : "default"
  }
  if (
    sourceVersion === "hns" &&
    (wayfarerReplacedHnsMapIds.has(source.id) || isRetiredHnsPort(name, source.id))
  ) {
    return "exclude"
  }
  return "default"
}

const wayfarerCoastConnectionDestinations: Record<string, Record<string, string>> = {
  FuchsiaCity_hns: { MAP_ROUTE19_COAST_POC: "MAP_ROUTE19" },
  PalletTown_hns: { MAP_ROUTE21_NORTH_COAST_POC: "MAP_ROUTE21_NORTH" },
  Route19_Frlg: { MAP_FUCHSIA_CITY: "MAP_FUCHSIA_CITY_HNS" },
  Route21_North_Frlg: { MAP_PALLET_TOWN: "MAP_PALLET_TOWN_HNS" },
}

export const sourceWayfarerConnections = (
  name: string,
  connections: readonly MapConnection[],
): readonly MapConnection[] => {
  const replacements = wayfarerCoastConnectionDestinations[name]
  if (!replacements) return connections
  let changed = false
  const resolved = connections.map((connection) => {
    const destination = replacements[connection.map]
    if (!destination) return connection
    changed = true
    return { ...connection, map: destination }
  })
  return changed ? resolved : connections
}

export const sourceWayfarerSeviiSelection = (root: string): WayfarerSeviiSelection => {
  const relativePath = "src/data/wayfarer_sevii_maps.json"
  const manifest = readJson<unknown>(path.join(root, relativePath))
  if (!manifest || typeof manifest !== "object" || Array.isArray(manifest)) {
    throw new Error(`${relativePath}: expected an object`)
  }
  const rootRecord = manifest as Record<string, unknown>
  if (typeof rootRecord.release_link_enabled !== "boolean") {
    throw new Error(`${relativePath}: release_link_enabled must be a boolean`)
  }
  if (!Array.isArray(rootRecord.maps)) {
    throw new Error(`${relativePath}: maps must be an array`)
  }
  const maps = new Set<string>()
  const releaseMaps = new Set<string>()
  for (const [index, value] of rootRecord.maps.entries()) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
      throw new Error(`${relativePath}: maps/${index} must be an object`)
    }
    const record = value as Record<string, unknown>
    const sourceMap = record.source_map
    if (typeof sourceMap !== "string" || sourceMap.length === 0) {
      throw new Error(`${relativePath}: maps/${index}/source_map must be a non-empty string`)
    }
    if (record.enabled !== undefined && typeof record.enabled !== "boolean") {
      throw new Error(`${relativePath}: maps/${index}/enabled must be a boolean`)
    }
    if (maps.has(sourceMap)) {
      throw new Error(`${relativePath}: duplicate source map ${JSON.stringify(sourceMap)}`)
    }
    maps.add(sourceMap)
    if (rootRecord.release_link_enabled && (record.enabled ?? true)) {
      releaseMaps.add(sourceMap)
    }
  }
  return { maps, releaseMaps }
}

const git = (root: string, args: string[]): string | null => {
  try {
    return childProcess
      .execFileSync("git", args, {
        cwd: root,
        encoding: "utf8",
        stdio: ["ignore", "pipe", "ignore"],
      })
      .trim()
  } catch {
    return null
  }
}

export const sourceState = (root: string): MapCatalog["source"] => {
  return {
    revision: git(root, ["rev-parse", "HEAD"]) ?? "unknown",
    workingTreeDirty: Boolean(git(root, ["status", "--porcelain"])),
  }
}

export const sourceGroups = (root: string): Map<string, string> => {
  const groups = readJson<MapGroups>(path.join(root, "data/maps/map_groups.json"))
  const index = new Map<string, string>()
  for (const group of groups.group_order) {
    for (const name of groups[group] ?? []) {
      index.set(name, group)
    }
  }
  return index
}

export const sourceLayouts = (root: string): Map<string, Layout> => {
  return new Map(
    readJson<LayoutDocument>(path.join(root, "data/layouts/layouts.json")).layouts.map((layout) => [
      layout.id,
      layout,
    ]),
  )
}

export const sourceMaps = (root: string, names: string[]): Map<string, SourceMap> => {
  return new Map(
    names.map((name) => [
      name,
      readJson<SourceMap>(path.join(root, "data/maps", name, "map.json")),
    ]),
  )
}

export const sha256 = (filePath: string): string => {
  return crypto.createHash("sha256").update(fs.readFileSync(filePath)).digest("hex")
}

export const posixRelative = (root: string, filePath: string): string => {
  return path.relative(root, filePath).replaceAll("\\", "/")
}
