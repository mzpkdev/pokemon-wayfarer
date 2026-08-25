import { describe, expect, it } from "vitest"

import config from "./webanvil.config.js"

describe("generated catalog assets", () => {
  it("serves generated files directly without copying them into builds", () => {
    expect(config.vite?.publicDir).toBe(false)
    expect(config.vite?.build?.copyPublicDir).toBe(false)
    const plugin = config.vite?.plugins?.find(
      (candidate) => candidate.name === "wayfarer-preview-generated-catalog",
    )
    expect(plugin).toBeDefined()
    expect(plugin?.configureServer).toBeTypeOf("function")
    expect(plugin?.configurePreviewServer).toBeTypeOf("function")
  })
})
