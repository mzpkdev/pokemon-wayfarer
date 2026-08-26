export type SpeciesGraphics = {
  speciesId: string
  isShiny: boolean
}

/**
 * Map JSON preserves both legacy arithmetic object graphic IDs and the newer helper macros.
 * Keep the two spellings together so rendering and object classification agree.
 */
export const speciesGraphicsFor = (graphicsId: string): SpeciesGraphics | null => {
  const legacy = graphicsId.match(
    /^(?:OBJ_EVENT_GFX_MON_BASE|OBJ_EVENT_MON)\s*\+\s*(SPECIES_\w+)(?:\s*\+\s*(?:SPECIES_SHINY_TAG|OBJ_EVENT_MON_SHINY))?$/,
  )
  if (legacy?.[1]) {
    return {
      speciesId: legacy[1],
      isShiny: /\+\s*(?:SPECIES_SHINY_TAG|OBJ_EVENT_MON_SHINY)$/.test(graphicsId),
    }
  }

  const macro = graphicsId.match(
    /^OBJ_EVENT_GFX_SPECIES(?<modifier>_SHINY(?:_FEMALE)?|_FEMALE)?\(\s*(?<species>(?:SPECIES_)?\w+)\s*\)$/,
  )
  if (!macro?.groups?.species) return null
  return {
    speciesId: macro.groups.species.startsWith("SPECIES_")
      ? macro.groups.species
      : `SPECIES_${macro.groups.species}`,
    isShiny: macro.groups.modifier?.startsWith("_SHINY") ?? false,
  }
}
