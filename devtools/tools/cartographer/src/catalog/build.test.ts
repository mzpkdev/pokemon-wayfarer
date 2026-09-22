import { describe, expect, it } from "vitest"

import { compactJson, isPlayerFacingCatalogSource } from "./build"

describe("cartographer catalog output", () => {
  it("serializes compact JSON with a trailing newline", () => {
    expect(compactJson({ maps: [{ id: "MAP_ROUTE101", name: "Route101" }] })).toBe(
      '{"maps":[{"id":"MAP_ROUTE101","name":"Route101"}]}\n',
    )
  })

  it("keeps inert Sinnoh source maps out of the player-facing catalog", () => {
    expect(isPlayerFacingCatalogSource({ game_version: "sinnoh" })).toBe(false)
    expect(isPlayerFacingCatalogSource({ game_version: "emerald" })).toBe(true)
    expect(isPlayerFacingCatalogSource({})).toBe(true)
  })
})
