import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

import { afterEach, describe, expect, it } from "vitest"

import { contextId } from "./catalog/build"
import { sourceMaps } from "./source"

const temporaryRoots: string[] = []

afterEach(() => {
  for (const root of temporaryRoots.splice(0)) fs.rmSync(root, { recursive: true, force: true })
})

const sourceRoot = (): string => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "wayfarer-metatile-source-"))
  temporaryRoots.push(root)
  return root
}

const writeMap = (root: string, name: string, map: object): void => {
  const directory = path.join(root, "data/maps", name)
  fs.mkdirSync(directory, { recursive: true })
  fs.writeFileSync(path.join(directory, "map.json"), JSON.stringify(map))
}

describe("metatile source provenance", () => {
  it("retains explicit Sinnoh source and physical region classification", () => {
    const root = sourceRoot()
    writeMap(root, "Route201", {
      id: "MAP_ROUTE201",
      layout: "LAYOUT_ROUTE201",
      game_version: "sinnoh",
    })

    expect(sourceMaps(root)).toEqual([
      {
        name: "Route201",
        id: "MAP_ROUTE201",
        layoutId: "LAYOUT_ROUTE201",
        sourceRegion: "sinnoh",
        region: "sinnoh",
      },
    ])
  })

  it("omits provenance fields for ordinary maps and preserves Emerald layout format", () => {
    const root = sourceRoot()
    writeMap(root, "Route101", {
      id: "MAP_ROUTE101",
      layout: "LAYOUT_ROUTE101",
      game_version: "emerald",
    })

    expect(sourceMaps(root)).toEqual([
      { name: "Route101", id: "MAP_ROUTE101", layoutId: "LAYOUT_ROUTE101" },
    ])
    expect(
      contextId({
        game_version: "sinnoh",
        layout_version: "emerald",
        primary_tileset: "gTileset_General",
        secondary_tileset: "gTileset_Petalburg",
      }),
    ).toBe("emerald:gTileset_General:gTileset_Petalburg")
  })
})
