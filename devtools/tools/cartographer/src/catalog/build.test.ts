import { describe, expect, it } from "vitest"

import { compactJson } from "./build"

describe("cartographer catalog output", () => {
  it("serializes compact JSON with a trailing newline", () => {
    expect(compactJson({ maps: [{ id: "MAP_ROUTE101", name: "Route101" }] })).toBe(
      '{"maps":[{"id":"MAP_ROUTE101","name":"Route101"}]}\n',
    )
  })
})
