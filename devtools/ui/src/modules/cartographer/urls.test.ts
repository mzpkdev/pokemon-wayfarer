import { describe, expect, it } from "vitest"

import { metatileAssetUrl } from "../metatiles/catalog.js"

import { catalogUrl, mapImageUrl } from "./urls.js"

describe("catalog URLs", () => {
  it("serves map and metatile assets from the direct generated catalog", () => {
    expect(catalogUrl("/")).toBe("/catalog.json")
    expect(mapImageUrl("maps/Route101.png", "/")).toBe("/maps/Route101.png")
    expect(metatileAssetUrl("contexts/example/primary.png", "/")).toBe(
      "/metatiles/contexts/example/primary.png",
    )
  })
})
