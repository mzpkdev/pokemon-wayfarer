import { describe, expect, it } from "vitest"

import config, { generatedCatalogContentType } from "./webanvil.config.js"

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

  it("uses browser MIME types when a staged static site shares the generated catalog", () => {
    expect(generatedCatalogContentType("/tmp/catalog/assets/index.js")).toBe(
      "text/javascript; charset=utf-8",
    )
    expect(generatedCatalogContentType("/tmp/catalog/assets/index.css")).toBe(
      "text/css; charset=utf-8",
    )
    expect(generatedCatalogContentType("/tmp/catalog/catalog.json")).toBe(
      "application/json; charset=utf-8",
    )
    expect(generatedCatalogContentType("/tmp/catalog/maps/Route101.png")).toBe("image/png")
  })
})
