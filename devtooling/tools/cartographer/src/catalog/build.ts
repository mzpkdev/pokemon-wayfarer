import * as fs from "node:fs"
import * as path from "node:path"

import {
  discoverExteriorMaps,
  encounterHabitat,
  exteriorMapTypes,
  normalizeLayoutFormat,
  renderMap,
  writeNearestNeighborOverview,
} from "../renderer"
import { catalogRegions, categoryFor, mapOutputPaths, regionFor } from "./classify"
import { catalogObjects, objectSourceTables } from "./objects"
import { catalogEncounterSprites } from "./encounter-sprites"
import type { ObjectSourceTables } from "./objects"
import { mapScriptBodies } from "./scripts"
import { sourceWildEncounters } from "./encounters"
import { topologyConflicts } from "./topology"
import {
  posixRelative,
  sha256,
  sourceGroups,
  sourceLayouts,
  sourceMaps,
  sourceState,
} from "./source"
import type {
  CatalogMap,
  CatalogWildEncounters,
  Layout,
  MapCatalog,
  RenderCatalogResult,
  SourceMap,
} from "./types"

export const compactJson = (value: unknown): string => `${JSON.stringify(value)}\n`

const createCatalogMap = (
  root: string,
  output: string,
  name: string,
  source: SourceMap,
  layout: Layout,
  group: string,
  namesById: Map<string, string>,
  objectTables: ObjectSourceTables,
  wildEncounters: CatalogWildEncounters,
): CatalogMap => {
  const region = regionFor(name, group, source.region_map_section)
  const category = categoryFor(source.map_type)
  const paths = mapOutputPaths(output, region.id, category, name)
  fs.mkdirSync(path.dirname(paths.native), { recursive: true })
  fs.mkdirSync(path.dirname(paths.overview), { recursive: true })
  renderMap(root, name, paths.native)
  writeNearestNeighborOverview(paths.native, paths.overview)
  const widthPixels = layout.width * 16
  const heightPixels = layout.height * 16

  const objects = catalogObjects(
    root,
    output,
    Array.isArray(source.object_events) ? source.object_events : [],
    objectTables,
    mapScriptBodies(root, name),
  )
  return {
    name,
    id: source.id,
    region: region.id,
    category,
    sourceGroup: group,
    sourceRegion: null,
    mapType: source.map_type,
    mapSection: source.region_map_section ?? null,
    image: {
      path: posixRelative(output, paths.native),
      sha256: sha256(paths.native),
      widthPixels,
      heightPixels,
      overview: {
        path: posixRelative(output, paths.overview),
        sha256: sha256(paths.overview),
        widthPixels: widthPixels / 4,
        heightPixels: heightPixels / 4,
      },
    },
    layout: {
      id: layout.id,
      format: normalizeLayoutFormat(layout),
      widthMetatiles: layout.width,
      heightMetatiles: layout.height,
      primaryTileset: layout.primary_tileset,
      secondaryTileset: layout.secondary_tileset,
    },
    world: {
      layer: source.map_type === "MAP_TYPE_UNDERWATER" ? "underwater" : "surface",
      defaultVisible: source.map_type !== "MAP_TYPE_UNDERWATER",
      variantGroup: null,
      variant: null,
    },
    presentation: {
      music: source.music ?? null,
      weather: source.weather ?? null,
      showMapName: source.show_map_name ?? null,
      requiresFlash: source.requires_flash ?? null,
    },
    connections: (Array.isArray(source.connections) ? source.connections : []).map(
      (connection) => ({
        direction: connection.direction,
        offsetMetatiles: connection.offset,
        destinationMapId: connection.map,
        destinationMap: namesById.get(connection.map) ?? null,
      }),
    ),
    warps: (Array.isArray(source.warp_events) ? source.warp_events : []).map((warp, index) => ({
      warpId: String(index),
      xMetatiles: warp.x,
      yMetatiles: warp.y,
      elevation: warp.elevation,
      destinationWarpId: warp.dest_warp_id,
      destinationMapId: warp.dest_map,
      destinationMap: namesById.get(warp.dest_map) ?? null,
    })),
    objects,
    wildEncounters,
    encounterHabitat: encounterHabitat(root, layout),
  }
}

/** Render every exterior map plus the metadata needed by the static cartographer. */
export const renderCatalog = (root: string, output: string): RenderCatalogResult => {
  const layouts = sourceLayouts(root)
  const groups = sourceGroups(root)
  const exteriorMaps = discoverExteriorMaps(root)
  const mapsByName = sourceMaps(root, exteriorMaps)
  const namesById = new Map([...mapsByName].map(([name, map]) => [map.id, name]))
  const objectTables = objectSourceTables(root)
  const wildEncountersByMap = sourceWildEncounters(
    root,
    namesById,
    catalogEncounterSprites(root, output),
  )
  const maps: CatalogMap[] = []

  for (const name of exteriorMaps) {
    const source = mapsByName.get(name)!
    if (!exteriorMapTypes.has(source.map_type)) {
      continue
    }
    const layout = layouts.get(source.layout)
    if (!layout) {
      throw new Error(`${name}: unknown layout ${source.layout}`)
    }
    maps.push(
      createCatalogMap(
        root,
        output,
        name,
        source,
        layout,
        groups.get(name) ?? "gMapGroup_Unassigned",
        namesById,
        objectTables,
        wildEncountersByMap.get(name) ?? { sets: [], runtimeTimes: [], diagnostics: [] },
      ),
    )
  }

  const catalog: MapCatalog = {
    schemaVersion: 7,
    format: "pokemon-wayfarer-exterior-map-catalog",
    pixelsPerMetatile: 16,
    source: sourceState(root),
    diagnostics: maps.flatMap((map) =>
      map.objects.flatMap((object) =>
        object.diagnostic
          ? [
              {
                map: map.name,
                objectId: object.objectId,
                graphicsId: object.graphicsId,
                ...object.diagnostic,
              },
            ]
          : [],
      ),
    ),
    topology: {
      conflicts: topologyConflicts(maps),
    },
    regions: catalogRegions.map((region) => {
      const names = maps.filter((map) => map.region === region.id).map((map) => map.name)
      return { ...region, mapCount: names.length, maps: names }
    }),
    maps,
  }
  fs.mkdirSync(output, { recursive: true })
  fs.writeFileSync(path.join(output, "catalog.json"), compactJson(catalog))
  return { mapCount: maps.length, output }
}
