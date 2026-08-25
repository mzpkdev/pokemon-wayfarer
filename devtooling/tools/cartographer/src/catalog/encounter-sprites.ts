import * as crypto from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"

import { cropSpriteFrame, writeRgbaPng } from "../renderer/png"
import { posixRelative, sha256 } from "./source"
import type { CatalogEncounterSprite } from "./types"

const iconDirectory = "graphics/pokemon"
const iconFrame = { index: 0, width: 32, height: 32 }

const iconCandidates = (speciesId: string): string[] => {
  const species = speciesId.replace(/^SPECIES_/, "").toLowerCase()
  return species === "unown" ? ["unown/a"] : [species]
}

const sourceIcon = (root: string, speciesId: string): string | null => {
  for (const candidate of iconCandidates(speciesId)) {
    const source = path.join(root, iconDirectory, candidate, "icon.png")
    if (fs.existsSync(source)) return source
  }
  return null
}

const writeIcon = (root: string, output: string, source: string): CatalogEncounterSprite => {
  const icon = cropSpriteFrame(source, iconFrame)
  const digest = crypto
    .createHash("sha256")
    .update(`${icon.width}x${icon.height}:`)
    .update(icon.pixels)
    .digest("hex")
  const filePath = path.join(output, "pokemon-icons", `${digest}.png`)
  if (!fs.existsSync(filePath)) {
    writeRgbaPng(filePath, icon.width, icon.height, icon.pixels)
  }
  return {
    path: posixRelative(output, filePath),
    sha256: sha256(filePath),
    widthPixels: icon.width,
    heightPixels: icon.height,
    source: path.relative(root, source).replaceAll("\\", "/"),
  }
}

/** Generate compact encounter icons from the source graphics, without a maintained species map. */
export const catalogEncounterSprites = (
  root: string,
  output: string,
): ((speciesId: string) => CatalogEncounterSprite | null) => {
  const sprites = new Map<string, CatalogEncounterSprite | null>()
  return (speciesId: string): CatalogEncounterSprite | null => {
    if (sprites.has(speciesId)) return sprites.get(speciesId)!
    const source = sourceIcon(root, speciesId)
    if (!source) {
      sprites.set(speciesId, null)
      return null
    }
    try {
      const sprite = writeIcon(root, output, source)
      sprites.set(speciesId, sprite)
      return sprite
    } catch {
      sprites.set(speciesId, null)
      return null
    }
  }
}
