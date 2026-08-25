import { describe, expect, it } from "vitest"

import type { CatalogMetatile, MetatileTileset } from "./catalog.js"

import {
  MetatileCatalogValidationError,
  metatileAtlasPosition,
  metatileScopedLabel,
  validateMetatileCatalog,
  validateMetatileContextCatalog,
} from "./catalog.js"

const tiles = (): Record<string, unknown>[] => {
  return [0, 1].flatMap((layer) =>
    [0, 1, 2, 3].map((quadrant) => ({
      layer,
      quadrant,
      tileId: quadrant,
      source: "primary",
      sourceTileId: quadrant,
      paletteId: 0,
      horizontalFlip: false,
      verticalFlip: false,
    })),
  )
}

const metatile = (tilesetId: string, id = 0): Record<string, unknown> => {
  const sourceId = `${tilesetId}:0x${id.toString(16).toUpperCase().padStart(3, "0")}`
  return {
    id,
    sourceId,
    displayName: sourceId,
    tiles: tiles(),
    attributes: { raw: 0, behavior: 0, behaviorName: null, layerType: 0 },
    usedBy: [
      {
        mapName: "Route101",
        mapId: "MAP_ROUTE101",
        layoutId: "LAYOUT_ROUTE101",
        count: 1,
        placements: [{ collision: 0, elevation: 0, count: 1 }],
      },
    ],
  }
}

const tileset = (
  id: string,
  kind: "primary" | "secondary",
  offset: number,
): Record<string, unknown> => {
  return {
    tilesetId: id,
    kind,
    metatileIdOffset: offset,
    atlas: {
      path: `contexts/example/${kind}.png`,
      sha256: "0123456789abcdef",
      widthPixels: 512,
      heightPixels: 16,
      columns: 32,
      cellPixels: 16,
      unresolvedTileReferences: 0,
    },
    metatiles: [metatile(id)],
  }
}

const catalog = (overrides: Record<string, unknown> = {}): Record<string, unknown> => {
  return {
    schemaVersion: 1,
    format: "pokemon-wayfarer-metatile-catalog",
    pixelsPerMetatile: 16,
    source: { revision: "example" },
    contexts: [
      {
        id: "example",
        format: "4x4",
        primaryTileset: "gTileset_General",
        secondaryTileset: "gTileset_Petalburg",
        mapCount: 1,
        usedMetatileCount: 2,
        placementCount: 2,
        path: "contexts/example/catalog.json",
      },
    ],
    ...overrides,
  }
}

const contextCatalog = (overrides: Record<string, unknown> = {}): Record<string, unknown> => {
  return {
    schemaVersion: 1,
    format: "pokemon-wayfarer-metatile-context",
    pixelsPerMetatile: 16,
    source: { revision: "example" },
    context: {
      id: "example",
      format: "4x4",
      primaryTileset: "gTileset_General",
      secondaryTileset: "gTileset_Petalburg",
      maps: [{ name: "Route101", id: "MAP_ROUTE101", layoutId: "LAYOUT_ROUTE101" }],
      primary: tileset("gTileset_General", "primary", 0),
      secondary: tileset("gTileset_Petalburg", "secondary", 1),
    },
    ...overrides,
  }
}

describe("validateMetatileCatalog", () => {
  it("keeps usage-ranked metatile previews at their source atlas locations", () => {
    const displayOrder = [metatile("gTileset_General", 368), metatile("gTileset_General", 0)]
    const atlas = tileset("gTileset_General", "primary", 0) as unknown as MetatileTileset

    expect(
      displayOrder.map((entry) => metatileAtlasPosition(entry as CatalogMetatile, atlas)),
    ).toEqual([
      { column: 16, row: 11 },
      { column: 0, row: 0 },
    ])
  })

  it("accepts context-rendered metatiles with a scoped source identity", () => {
    const index = validateMetatileCatalog(catalog())
    const entry = index.contexts[0]
    if (!entry) throw new Error("test fixture did not create a context index entry")
    const context = validateMetatileContextCatalog(contextCatalog(), entry)
    const metatile = context.primary.metatiles[0]

    expect(metatile && metatileScopedLabel(metatile)).toBe("gTileset_General:0x000")
  })

  it("accepts a render context that is defined but not used by a map", () => {
    const value = catalog()
    const contexts = value.contexts as Array<Record<string, unknown>>
    const first = contexts[0]
    if (!first) throw new Error("test fixture did not create a context index entry")
    first.mapCount = 0

    expect(validateMetatileCatalog(value).contexts[0]?.mapCount).toBe(0)
  })

  it("rejects an atlas that cannot contain its generated metatiles", () => {
    const value = contextCatalog()
    const context = value.context as Record<string, unknown>
    const primary = context.primary as Record<string, unknown>
    const atlas = primary.atlas as Record<string, unknown>
    atlas.heightPixels = 1
    const entry = validateMetatileCatalog(catalog()).contexts[0]
    if (!entry) throw new Error("test fixture did not create a context index entry")

    expect(() => validateMetatileContextCatalog(value, entry)).toThrow(
      MetatileCatalogValidationError,
    )
    expect(() => validateMetatileContextCatalog(value, entry)).toThrow("atlas cannot contain")
  })

  it("rejects an unscoped display name even when the local numeric ID is valid", () => {
    const value = contextCatalog()
    const context = value.context as Record<string, unknown>
    const primary = context.primary as Record<string, unknown>
    const metatiles = primary.metatiles as Array<Record<string, unknown>>
    const first = metatiles[0]
    if (!first) throw new Error("test fixture did not create a metatile")
    first.displayName = "0"
    const entry = validateMetatileCatalog(catalog()).contexts[0]
    if (!entry) throw new Error("test fixture did not create a context index entry")

    expect(() => validateMetatileContextCatalog(value, entry)).toThrow("invalid scoped source ID")
  })
})
