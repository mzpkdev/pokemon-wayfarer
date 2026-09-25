import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

import { afterEach, describe, expect, it } from "vitest"

import { sourceWayfarerSeviiSelection } from "./source"

const temporaryDirectories: string[] = []

const writeManifest = (manifest: unknown): string => {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), "cartographer-sevii-"))
  temporaryDirectories.push(root)
  const data = path.join(root, "src/data")
  fs.mkdirSync(data, { recursive: true })
  fs.writeFileSync(path.join(data, "wayfarer_sevii_maps.json"), JSON.stringify(manifest))
  return root
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
