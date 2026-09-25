import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

import { afterEach, describe, expect, it } from "vitest"

import {
  sourceWayfarerConnections,
  sourceWayfarerMembership,
  sourceWayfarerSeviiSelection,
} from "./source"
import type { SourceMap } from "./types"

const temporaryDirectories: string[] = []
const gameRoot = path.resolve(import.meta.dirname, "../../../../..", "game")

const writeManifest = (manifest: unknown): string => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "cartographer-sevii-"))
  temporaryDirectories.push(root)
  const data = path.join(root, "src/data")
  fs.mkdirSync(data, { recursive: true })
  fs.writeFileSync(path.join(data, "wayfarer_sevii_maps.json"), JSON.stringify(manifest))
  return root
}

const sourceMap = (
  id: string,
  game_version: SourceMap["game_version"],
  wayfarer_include?: boolean,
): SourceMap => ({
  id,
  game_version,
  wayfarer_include,
  layout: "LAYOUT_TEST",
  map_type: "MAP_TYPE_ROUTE",
})

const mapjsonStringSet = (name: string): string[] => {
  const source = fs.readFileSync(path.join(gameRoot, "tools/mapjson/mapjson.cpp"), "utf8")
  const declaration = new RegExp(`(?:static\\s+)?const set<string>\\s+${name}\\s*=\\s*\\{`).exec(
    source,
  )
  if (!declaration) throw new Error(`mapjson set ${name} was not found`)
  const bodyStart = declaration.index + declaration[0].length
  const bodyEnd = source.indexOf("};", bodyStart)
  if (bodyEnd < 0) throw new Error(`mapjson set ${name} is unterminated`)
  return [...source.slice(bodyStart, bodyEnd).matchAll(/"([^"]+)"/g)].map((match) => match[1]!)
}

const sourceMapByName = (name: string): SourceMap =>
  JSON.parse(
    fs.readFileSync(path.join(gameRoot, "data/maps", name, "map.json"), "utf8"),
  ) as SourceMap

const sourceMapById = (id: string): { name: string; source: SourceMap } => {
  for (const entry of fs.readdirSync(path.join(gameRoot, "data/maps"))) {
    const file = path.join(gameRoot, "data/maps", entry, "map.json")
    if (!fs.existsSync(file)) continue
    const source = JSON.parse(fs.readFileSync(file, "utf8")) as SourceMap
    if (source.id === id) return { name: entry, source }
  }
  throw new Error(`source map ${id} was not found`)
}

afterEach(() => {
  for (const directory of temporaryDirectories.splice(0)) {
    fs.rmSync(directory, { force: true, recursive: true })
  }
})

describe("Wayfarer Sevii map selection", () => {
  it("loads release membership from the authored manifest", () => {
    const selection = sourceWayfarerSeviiSelection(
      writeManifest({
        release_link_enabled: true,
        maps: [{ source_map: "OneIsland_Frlg" }, { source_map: "TwoIsland_Frlg" }],
      }),
    )

    expect([...selection.maps]).toEqual(["OneIsland_Frlg", "TwoIsland_Frlg"])
    expect([...selection.releaseMaps]).toEqual(["OneIsland_Frlg", "TwoIsland_Frlg"])
  })

  it("keeps disabled maps in the region without adding them to the Wayfarer release", () => {
    const selection = sourceWayfarerSeviiSelection(
      writeManifest({
        release_link_enabled: true,
        maps: [{ source_map: "OneIsland_Frlg" }, { source_map: "TwoIsland_Frlg", enabled: false }],
      }),
    )

    expect([...selection.maps]).toEqual(["OneIsland_Frlg", "TwoIsland_Frlg"])
    expect([...selection.releaseMaps]).toEqual(["OneIsland_Frlg"])
  })

  it("rejects duplicate source maps", () => {
    const root = writeManifest({
      release_link_enabled: true,
      maps: [{ source_map: "OneIsland_Frlg" }, { source_map: "OneIsland_Frlg" }],
    })

    expect(() => sourceWayfarerSeviiSelection(root)).toThrow(
      'duplicate source map "OneIsland_Frlg"',
    )
  })
})

describe("Wayfarer source-map membership", () => {
  const sevii = {
    maps: new Set(["OneIsland_Frlg", "TwoIsland_Frlg"]),
    releaseMaps: new Set(["OneIsland_Frlg"]),
  }

  it("includes selected FRLG coast and enabled Sevii maps", () => {
    expect(sourceWayfarerMembership("Route20_Frlg", sourceMap("MAP_ROUTE20", "frlg"), sevii)).toBe(
      "include",
    )
    expect(
      sourceWayfarerMembership("OneIsland_Frlg", sourceMap("MAP_ONE_ISLAND", "frlg"), sevii),
    ).toBe("include")
  })

  it("keeps unselected or disabled FRLG maps out of Wayfarer", () => {
    expect(
      sourceWayfarerMembership("PalletTown_Frlg", sourceMap("MAP_PALLET_TOWN", "frlg"), sevii),
    ).toBe("default")
    expect(
      sourceWayfarerMembership("TwoIsland_Frlg", sourceMap("MAP_TWO_ISLAND", "frlg"), sevii),
    ).toBe("default")
  })

  it("honors explicit FRLG imports", () => {
    expect(
      sourceWayfarerMembership(
        "SilphCo_2F_Frlg",
        sourceMap("MAP_SILPH_CO_2F", "frlg", true),
        sevii,
      ),
    ).toBe("include")
  })

  it("removes replaced HNS coast maps and retired previews from Wayfarer", () => {
    expect(
      sourceWayfarerMembership(
        "CinnabarIsland_hns",
        sourceMap("MAP_CINNABAR_ISLAND_HNS", "hns"),
        sevii,
      ),
    ).toBe("exclude")
    expect(
      sourceWayfarerMembership(
        "Route20_CoastPoc",
        sourceMap("MAP_ROUTE20_COAST_POC", "hns"),
        sevii,
      ),
    ).toBe("exclude")
  })

  it("retains unrelated HNS maps in Wayfarer", () => {
    expect(
      sourceWayfarerMembership("NewBarkTown_hns", sourceMap("MAP_NEW_BARK_TOWN_HNS", "hns"), sevii),
    ).toBe("default")
  })

  it("stays aligned with mapjson's complete mainland coast policy", () => {
    for (const name of mapjsonStringSet("wayfarer_coast_map_names")) {
      expect(sourceWayfarerMembership(name, sourceMapByName(name), sevii), name).toBe("include")
    }

    for (const id of mapjsonStringSet("replaced_hns_ids").filter((value) =>
      value.startsWith("MAP_"),
    )) {
      const { name, source } = sourceMapById(id)
      expect(sourceWayfarerMembership(name, source, sevii), id).toBe("exclude")
    }
  })
})

describe("Wayfarer coast connections", () => {
  it.each([
    ["FuchsiaCity_hns", "MAP_ROUTE19_COAST_POC", "MAP_ROUTE19"],
    ["PalletTown_hns", "MAP_ROUTE21_NORTH_COAST_POC", "MAP_ROUTE21_NORTH"],
    ["Route19_Frlg", "MAP_FUCHSIA_CITY", "MAP_FUCHSIA_CITY_HNS"],
    ["Route21_North_Frlg", "MAP_PALLET_TOWN", "MAP_PALLET_TOWN_HNS"],
  ])("rewires %s from %s to %s", (name, source, destination) => {
    expect(sourceWayfarerConnections(name, [{ map: source, direction: "up", offset: 0 }])).toEqual([
      { map: destination, direction: "up", offset: 0 },
    ])
  })

  it("preserves native connections for builds without an override", () => {
    const connections = [{ map: "MAP_ROUTE18_HNS", direction: "left" as const, offset: 10 }]
    expect(sourceWayfarerConnections("FuchsiaCity_hns", connections)).toBe(connections)
  })
})
