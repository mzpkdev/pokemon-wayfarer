import { describe, expect, it } from "vitest"

import { topologyConflicts } from "./topology"
import type { CatalogMap } from "./types"

type TestConnection = {
  direction: "up" | "down" | "left" | "right"
  destinationMap: string
  offsetMetatiles: number
}

const map = (name: string, connections: TestConnection[]): CatalogMap => {
  return {
    name,
    id: `MAP_${name.toUpperCase()}`,
    layout: { widthMetatiles: 10, heightMetatiles: 10 },
    connections: connections.map((connection) => ({
      ...connection,
      destinationMapId: `MAP_${connection.destinationMap.toUpperCase()}`,
    })),
  } as CatalogMap
}

describe("topologyConflicts", () => {
  it("identifies the precise reciprocal records when a direct connection disagrees", () => {
    const diagnostics = topologyConflicts([
      map("Alpha", [{ direction: "right", destinationMap: "Bravo", offsetMetatiles: 0 }]),
      map("Bravo", [{ direction: "left", destinationMap: "Alpha", offsetMetatiles: 1 }]),
    ])

    expect(diagnostics).toMatchObject([
      {
        code: "direct_connection_mismatch",
        connection: {
          source: {
            map: "Alpha",
            header: { path: "data/maps/Alpha/map.json", pointer: "/connections/0" },
          },
          destination: { map: "Bravo" },
          direction: "right",
          offsetMetatiles: 0,
        },
        reverseConnection: {
          source: {
            map: "Bravo",
            header: { path: "data/maps/Bravo/map.json", pointer: "/connections/0" },
          },
          direction: "left",
          offsetMetatiles: 1,
        },
        expectedReverse: { direction: "left", offsetMetatiles: 0 },
        forwardPlacement: { x: 10, y: 0 },
        reversePlacement: { x: 10, y: -1 },
      },
    ])
    expect(diagnostics[0]?.explanation).toContain("reverse record")
  })

  it("reports a missing reciprocal record as a direct diagnostic", () => {
    const diagnostics = topologyConflicts([
      map("Alpha", [{ direction: "right", destinationMap: "Bravo", offsetMetatiles: 3 }]),
      map("Bravo", []),
    ])

    expect(diagnostics).toMatchObject([
      {
        code: "missing_reverse_connection",
        connection: {
          source: { map: "Alpha", header: { pointer: "/connections/0" } },
          destination: { map: "Bravo" },
        },
        expectedReverse: { direction: "left", offsetMetatiles: -3 },
      },
    ])
  })

  it("does not reuse a reverse record when duplicate connections need separate evidence", () => {
    const diagnostics = topologyConflicts([
      map("Alpha", [
        { direction: "right", destinationMap: "Bravo", offsetMetatiles: 0 },
        { direction: "right", destinationMap: "Bravo", offsetMetatiles: 0 },
      ]),
      map("Bravo", [{ direction: "left", destinationMap: "Alpha", offsetMetatiles: 0 }]),
    ])

    expect(diagnostics).toMatchObject([
      {
        code: "missing_reverse_connection",
        connection: { source: { map: "Alpha", header: { pointer: "/connections/1" } } },
      },
    ])
  })

  it("does not diagnose a mutually consistent shortcut cycle", () => {
    const diagnostics = topologyConflicts([
      map("Route32", [
        { direction: "up", destinationMap: "VioletCity", offsetMetatiles: 4 },
        { direction: "left", destinationMap: "RuinsOfAlph_Outside", offsetMetatiles: -24 },
      ]),
      map("RuinsOfAlph_Outside", [
        { direction: "up", destinationMap: "Route36", offsetMetatiles: -36 },
        { direction: "right", destinationMap: "Route32", offsetMetatiles: 24 },
      ]),
      map("VioletCity", [
        { direction: "down", destinationMap: "Route32", offsetMetatiles: -4 },
        { direction: "left", destinationMap: "Route36", offsetMetatiles: -2 },
      ]),
      map("Route36", [
        { direction: "right", destinationMap: "VioletCity", offsetMetatiles: 2 },
        { direction: "down", destinationMap: "RuinsOfAlph_Outside", offsetMetatiles: 36 },
      ]),
    ])

    expect(diagnostics).toEqual([])
  })
})
