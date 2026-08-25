import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

import { afterAll, describe, expect, it } from "vitest"

import { buildMetatileCatalog, describeMetatileTiles } from "./build"

const sourceRoot = path.resolve(import.meta.dirname, "../../../../..", "game")
const output = fs.mkdtempSync(path.join(os.tmpdir(), "wayfarer-metatiles-test-"))

afterAll(() => fs.rmSync(output, { recursive: true, force: true }))

describe("metatile catalog decoding", () => {
  it("retains layer, palette, flips, and the primary-tile reference inside secondary data", () => {
    const metatiles = Buffer.alloc(16)
    metatiles.writeUInt16LE(0xa294, 0)
    metatiles.writeUInt16LE(0x0f21, 8)

    const tiles = describeMetatileTiles(metatiles, 0, 640)

    expect(tiles[0]).toMatchObject({
      layer: 0,
      quadrant: 0,
      tileId: 660,
      source: "secondary",
      sourceTileId: 20,
      paletteId: 10,
      horizontalFlip: false,
      verticalFlip: false,
    })
    expect(tiles[4]).toMatchObject({
      layer: 1,
      quadrant: 0,
      tileId: 801,
      source: "secondary",
      sourceTileId: 161,
      paletteId: 0,
      horizontalFlip: true,
      verticalFlip: true,
    })
  })

  it("builds every source-backed context without resolving the unused sentinel layout", () => {
    const result = buildMetatileCatalog(sourceRoot, output)
    const catalogFile = fs.readFileSync(path.join(output, "catalog.json"), "utf8")
    const catalog = JSON.parse(catalogFile) as {
      contexts: Array<{ path: string; secondaryTileset: string }>
    }

    expect(result.contextCount).toBeGreaterThan(0)
    expect(result.metatileCount).toBeGreaterThan(0)
    expect(catalog).not.toHaveProperty("$schema")
    expect(catalogFile).toBe(`${JSON.stringify(catalog)}\n`)
    expect(catalog.contexts.some((context) => context.secondaryTileset === "0")).toBe(false)

    const firstContext = catalog.contexts[0]
    if (!firstContext) throw new Error("Expected at least one generated metatile context")
    const contextCatalogFile = fs.readFileSync(path.join(output, firstContext.path), "utf8")
    const contextCatalog = JSON.parse(contextCatalogFile)
    expect(contextCatalog).not.toHaveProperty("$schema")
    expect(contextCatalogFile).toBe(`${JSON.stringify(contextCatalog)}\n`)
  }, 30_000)
})
