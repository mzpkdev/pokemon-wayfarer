import * as fs from "node:fs"
import * as path from "node:path"

import { describe, expect, it } from "vitest"

import {
  metatileAttributeSize,
  normalizeLayoutFormat,
  readLayoutFormatCounts,
  readMetatileAttribute,
  resolveTilesetAssets,
} from "./tilesets"

const sourceRoot = path.resolve(import.meta.dirname, "../../../../..", "game")

describe("Wayfarer layout formats", () => {
  it("uses layout_version instead of a legacy conditional header define", () => {
    expect(normalizeLayoutFormat({ layout_version: "emerald" })).toBe("emerald")
    expect(normalizeLayoutFormat({ layout_version: "frlg" })).toBe("frlg")
    expect(normalizeLayoutFormat({ layout_version: "hns" })).toBe("hns")
    expect(readLayoutFormatCounts("emerald")).toEqual([512, 512, 6])
    expect(readLayoutFormatCounts("frlg")).toEqual([640, 640, 7])
    expect(readLayoutFormatCounts("hns")).toEqual([640, 640, 7])
  })

  it("decodes four-byte FRLG attributes separately from two-byte layouts", () => {
    const attributes = Buffer.alloc(4)
    attributes.writeUInt32LE(0x600001a5)

    expect(metatileAttributeSize(attributes, 1)).toBe(4)
    expect(readMetatileAttribute(attributes, 0, 4)).toBe(0x600001a5)
    expect(() => readMetatileAttribute(Buffer.from([0xa5, 0x01]), 0, 4)).toThrow("missing entry")
  })

  it("uses the actual attribute table width and graphics declaration for Wayfarer tilesets", () => {
    const lab = resolveTilesetAssets(sourceRoot, "gTileset_Lab_Frlg")
    const labMetatileCount = fs.readFileSync(lab.metatiles).length / 16
    expect(metatileAttributeSize(fs.readFileSync(lab.metatileAttributes), labMetatileCount)).toBe(2)

    const mtEmber = resolveTilesetAssets(sourceRoot, "gTileset_MtEmber_Hns")
    expect(mtEmber.tiles).toBe(
      path.join(sourceRoot, "data/tilesets/secondary/mt_ember_hns/tiles.png"),
    )
  })
})
