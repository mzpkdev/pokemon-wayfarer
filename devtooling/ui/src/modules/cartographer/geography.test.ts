import { describe, expect, it } from "vitest"

import type { CatalogMap } from "./catalog.js"
import { solveGeography } from "./geography.js"

const map = (
  name: string,
  connections: Array<{ direction: "up" | "down" | "left" | "right"; destinationMap: string }>,
): CatalogMap => {
  return {
    name,
    layout: { widthMetatiles: 10, heightMetatiles: 10 },
    connections: connections.map((connection) => ({
      ...connection,
      offsetMetatiles: 0,
      destinationMapId: connection.destinationMap,
    })),
  } as CatalogMap
}

describe("solveGeography", () => {
  it("keeps the established placement when source connections disagree", () => {
    const geography = solveGeography([
      map("Alpha", [{ direction: "right", destinationMap: "Bravo" }]),
      map("Bravo", [{ direction: "down", destinationMap: "Charlie" }]),
      map("Charlie", [{ direction: "left", destinationMap: "Alpha" }]),
    ])

    expect(geography.placements).toMatchObject({
      Alpha: { x: 0, y: 0 },
      Bravo: { x: 10, y: 0 },
      Charlie: { x: 10, y: 0 },
    })
    expect(geography.overlaps).toEqual([
      {
        maps: ["Bravo", "Charlie"],
        area: { x: 10, y: 0, width: 10, height: 10 },
      },
    ])
  })
})
