import * as path from "node:path"

import { describe, expect, it } from "vitest"

import { sourceLayouts } from "../catalog/source"
import { encounterHabitat } from "./habitats"

const sourceRoot = path.resolve(import.meta.dirname, "../../../../..", "game")

describe("encounter habitat", () => {
  it("derives Route 32 land and water tiles from runtime behavior attributes", () => {
    const layout = sourceLayouts(sourceRoot).get("LAYOUT_ROUTE32_HNS")
    expect(layout).toBeDefined()

    const habitat = encounterHabitat(sourceRoot, layout!)

    expect(habitat.land).not.toEqual([])
    expect(habitat.water).not.toEqual([])
    expect(
      [...habitat.land, ...habitat.water].every(
        (rectangle) =>
          rectangle.xMetatiles >= 0 &&
          rectangle.yMetatiles >= 0 &&
          rectangle.widthMetatiles > 0 &&
          rectangle.heightMetatiles > 0,
      ),
    ).toBe(true)
  })
})
