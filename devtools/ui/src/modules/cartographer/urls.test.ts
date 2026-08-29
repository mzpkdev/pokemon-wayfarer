import { describe, expect, it } from "vitest"

import { metatileAssetUrl } from "../metatiles/catalog.js"

import {
  catalogUrl,
  cartographerUrlWithState,
  mapImageUrl,
  parseCartographerUrlState,
} from "./urls.js"

describe("catalog URLs", () => {
  it("serves map and metatile assets from the direct generated catalog", () => {
    expect(catalogUrl("/")).toBe("/catalog.json")
    expect(mapImageUrl("maps/Route101.png", "/")).toBe("/maps/Route101.png")
    expect(metatileAssetUrl("contexts/example/primary.png", "/")).toBe(
      "/metatiles/contexts/example/primary.png",
    )
  })
})

describe("Cartographer URL state", () => {
  it("defaults and clamps the trainer rating", () => {
    expect(parseCartographerUrlState("https://example.test/?rating=4").trainerRating).toBe(10)
    expect(parseCartographerUrlState("https://example.test/?rating=91").trainerRating).toBe(80)
    expect(parseCartographerUrlState("https://example.test/?rating=31.6").trainerRating).toBe(32)
    expect(parseCartographerUrlState("https://example.test/").trainerRating).toBe(10)
  })

  it("round-trips rating and product while preserving unrelated parameters", () => {
    const href = cartographerUrlWithState("https://example.test/?tool=cartographer", {
      region: "kanto",
      selectedMap: "Route1_frlg",
      view: null,
      trainerRating: 44,
      product: "firered",
    })

    expect(href).toBe("/?tool=cartographer&region=kanto&map=Route1_frlg&rating=44&product=firered")
    expect(parseCartographerUrlState(`https://example.test${href}`)).toMatchObject({
      trainerRating: 44,
      product: "firered",
    })
  })
})
