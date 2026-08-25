import { describe, expect, it } from "vitest"

import { speciesGraphicsFor } from "./species-graphics"

describe("species graphics expressions", () => {
  it("supports the legacy arithmetic form", () => {
    expect(speciesGraphicsFor("OBJ_EVENT_GFX_MON_BASE+SPECIES_PIKACHU+SPECIES_SHINY_TAG")).toEqual({
      speciesId: "SPECIES_PIKACHU",
      isShiny: true,
    })
    expect(speciesGraphicsFor("OBJ_EVENT_MON+SPECIES_MAGIKARP+OBJ_EVENT_MON_SHINY")).toEqual({
      speciesId: "SPECIES_MAGIKARP",
      isShiny: true,
    })
  })

  it("supports Wayfarer's species helper macros", () => {
    expect(speciesGraphicsFor("OBJ_EVENT_GFX_SPECIES(BUTTERFREE)")).toEqual({
      speciesId: "SPECIES_BUTTERFREE",
      isShiny: false,
    })
    expect(speciesGraphicsFor("OBJ_EVENT_GFX_SPECIES_SHINY_FEMALE(SPECIES_PIKACHU)")).toEqual({
      speciesId: "SPECIES_PIKACHU",
      isShiny: true,
    })
  })
})
