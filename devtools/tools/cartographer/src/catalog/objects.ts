import * as crypto from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"

import { cropSpriteFrame, replaceSpritePalette, writeRgbaPng } from "../renderer/png"
import { readPalette } from "../renderer/tilesets"
import { classifyObject, type ObjectKind } from "./object-kind"
import { speciesGraphicsFor } from "./species-graphics"
import { posixRelative, sha256 } from "./source"
import type { ObjectEvent } from "./types"

type SourceFrame = {
  graphicsId: string
  source: string
  widthPixels: number
  heightPixels: number
  frameIndex: number
}

export type ObjectDiagnostic = {
  code: string
  message: string
}

export type CatalogObject = {
  objectId: string
  kind: ObjectKind
  graphicsId: string
  isShiny: boolean
  xMetatiles: number
  yMetatiles: number
  elevation: number
  movementType: string
  movementRange: { x: number; y: number }
  trainerType: string
  trainerSightOrBerryTreeId: string
  script: string
  flag: string
  sprite: {
    path: string
    sha256: string
    widthPixels: number
    heightPixels: number
    anchor: { xPixels: number; yPixels: number }
    source: string
  } | null
  diagnostic: ObjectDiagnostic | null
}

export type ObjectSourceTables = {
  graphicsInfoById: Map<string, string>
  frameByGraphicsInfo: Map<string, SourceFrame>
  frameBySpecies: Map<string, SourceFrame>
  berryPreviewByName: Map<string, SourceFrame>
  berryPreviewByTreeId: Map<string, SourceFrame>
  berryPreviewFallback: SourceFrame | null
}

const sourceFile = (root: string, relative: string): string => {
  return path.join(root, "src/data/object_events", relative)
}

const readSource = (root: string, relative: string): string => {
  return fs
    .readFileSync(sourceFile(root, relative), "utf8")
    .replaceAll(/\/\*[\s\S]*?\*\/|\/\/[^\n]*/g, "")
}

const readRootSource = (root: string, relative: string): string => {
  return fs
    .readFileSync(path.join(root, relative), "utf8")
    .replaceAll(/\/\*[\s\S]*?\*\/|\/\/[^\n]*/g, "")
}

const picTables = (root: string): Map<string, SourceFrame> => {
  const result = new Map<string, SourceFrame>()
  const followersPath = sourceFile(root, "object_event_pic_tables_followers.h")
  const text = [
    readSource(root, "object_event_pic_tables.h"),
    fs.existsSync(followersPath) ? readSource(root, "object_event_pic_tables_followers.h") : "",
    readRootSource(root, "src/data/field_effects/field_effect_objects.h"),
  ].join("\n")
  const tablePattern =
    /(?:static\s+)?const struct SpriteFrameImage\s+(\w+)\[\]\s*=\s*\{([\s\S]*?)\};/g
  for (const match of text.matchAll(tablePattern)) {
    const [, tableName, body] = match
    const frame = body?.match(
      /overworld_frame\((g(?:ObjectEvent|FieldEffectObject)Pic_\w+),\s*(\d+),\s*(\d+),\s*(\d+)\)|obj_frame_tiles\((g(?:ObjectEvent|FieldEffectObject)Pic_\w+)\)/,
    )
    const ascending = body?.match(
      /overworld_ascending_frames\((g(?:ObjectEvent|FieldEffectObject)Pic_\w+),\s*(\d+),\s*(\d+)\)/,
    )
    if (!tableName || (!frame && !ascending)) {
      continue
    }
    const [, overworldPicture, widthTiles, heightTiles, frameIndex, staticPicture] = frame ?? []
    const [, ascendingPicture, ascendingWidthTiles, ascendingHeightTiles] = ascending ?? []
    const picture = overworldPicture ?? staticPicture ?? ascendingPicture
    if (!picture) {
      continue
    }
    result.set(tableName, {
      graphicsId: picture,
      source: "",
      widthPixels: Number(widthTiles ?? ascendingWidthTiles ?? 0) * 8,
      heightPixels: Number(heightTiles ?? ascendingHeightTiles ?? 0) * 8,
      frameIndex: Number(frameIndex ?? 0),
    })
  }
  return result
}

const pictureSources = (root: string): Map<string, string> => {
  const result = new Map<string, string>()
  const text = [
    readSource(root, "object_event_graphics.h"),
    readRootSource(root, "src/data/graphics/pokemon.h"),
  ].join("\n")
  const sourcePattern =
    /const u32\s+(g(?:ObjectEvent|FieldEffectObject)Pic_\w+)\[\]\s*=\s*INCBIN(?:_\w+)?\("([^"]+)"\)/g
  for (const match of text.matchAll(sourcePattern)) {
    const [, picture, asset] = match
    if (picture && asset) {
      result.set(picture, path.join(root, asset.replace(/\.4bpp(?:\.lz)?$/, ".png")))
    }
  }
  return result
}

/**
 * A berry tree's live stage and berry are stored in save data, so the catalog cannot render
 * its runtime state. It shows the flowering frame from its new-game source assignment instead.
 */
const berryTreePreviewFrames = (
  root: string,
  sources: ReadonlyMap<string, string>,
): { byName: Map<string, SourceFrame>; fallback: SourceFrame | null } => {
  const text = readSource(root, "berry_tree_graphics_tables.h")
  const tables = new Map<string, string>()
  for (const match of text.matchAll(
    /static const struct SpriteFrameImage\s+(sPicTable_\w+BerryTree)\[\]\s*=\s*\{([\s\S]*?)\};/g,
  )) {
    const [, tableName, body] = match
    if (tableName && body) tables.set(tableName, body)
  }
  const frameFor = (tableName: string, frameIndex: number): SourceFrame | null => {
    const body = tables.get(tableName)
    if (!body) return null
    const frames = [
      ...body.matchAll(/overworld_frame\((gObjectEventPic_\w+),\s*(\d+),\s*(\d+),\s*(\d+)\)/g),
    ]
    const frame = frames[frameIndex]
    if (!frame) return null
    const [, graphicsId, widthTiles, heightTiles, sourceFrameIndex] = frame
    if (!graphicsId || !widthTiles || !heightTiles || !sourceFrameIndex) return null
    const source = sources.get(graphicsId)
    if (!source) return null
    return {
      graphicsId,
      source,
      widthPixels: Number(widthTiles) * 8,
      heightPixels: Number(heightTiles) * 8,
      frameIndex: Number(sourceFrameIndex),
    }
  }
  const byName = new Map<string, SourceFrame>()
  for (const match of text.matchAll(
    /\[ITEM_(\w+)_BERRY\s*-\s*FIRST_BERRY_INDEX\]\s*=\s*(sPicTable_\w+BerryTree),/g,
  )) {
    const [, berryName, tableName] = match
    const frame = berryName && tableName ? frameFor(tableName, 3) : null
    if (berryName && frame) byName.set(berryName, frame)
  }
  return { byName, fallback: frameFor("sPicTable_PechaBerryTree", 0) }
}

const berryTreeAssignments = (
  root: string,
  framesByName: ReadonlyMap<string, SourceFrame>,
): Map<string, SourceFrame> => {
  const assignments = new Map<string, SourceFrame>()
  const source = readRootSource(root, "data/scripts/new_game.inc")
  for (const match of source.matchAll(
    /\bsetberrytree\s+(BERRY_TREE_\w+)\s*,\s*ITEM_TO_BERRY\(ITEM_(\w+)_BERRY\)/g,
  )) {
    const [, treeId, berryName] = match
    const frame = berryName ? framesByName.get(berryName) : null
    if (treeId && frame) assignments.set(treeId, frame)
  }
  return assignments
}

export const objectSourceTables = (root: string): ObjectSourceTables => {
  const tables = picTables(root)
  const sources = pictureSources(root)
  const berryPreviews = berryTreePreviewFrames(root, sources)
  const berryAssignments = berryTreeAssignments(root, berryPreviews.byName)
  const resolveFrame = (
    tableName: string,
    graphicsId: string,
    dimensions?: { widthPixels: number; heightPixels: number },
  ): SourceFrame | undefined => {
    const frame = tables.get(tableName)
    if (!frame) {
      return undefined
    }
    const source = sources.get(frame.graphicsId)
    if (!source) {
      return undefined
    }
    return {
      ...frame,
      graphicsId,
      source,
      widthPixels: frame.widthPixels || dimensions?.widthPixels || 0,
      heightPixels: frame.heightPixels || dimensions?.heightPixels || 0,
    }
  }
  const dimensionsFor = (
    body: string,
  ): { widthPixels: number; heightPixels: number } | undefined => {
    const namedDimensions = body.match(/\.width\s*=\s*(\d+),[\s\S]*?\.height\s*=\s*(\d+),/)
    if (namedDimensions?.[1] && namedDimensions[2]) {
      return { widthPixels: Number(namedDimensions[1]), heightPixels: Number(namedDimensions[2]) }
    }
    const dimensions = body.match(/^(?:[^,]*,){4}\s*(\d+),\s*(\d+)/)
    if (!dimensions?.[1] || !dimensions[2]) {
      return undefined
    }
    return { widthPixels: Number(dimensions[1]), heightPixels: Number(dimensions[2]) }
  }

  const graphicsInfoById = new Map<string, string>()
  const pointers = readSource(root, "object_event_graphics_info_pointers.h")
  for (const match of pointers.matchAll(
    /\[\s*(OBJ_EVENT_GFX_\w+)\s*\]\s*=\s*&?(gObjectEventGraphicsInfo_\w+)/g,
  )) {
    const [, graphicsId, info] = match
    if (graphicsId && info) {
      graphicsInfoById.set(graphicsId, info)
    }
  }

  const aliases = new Map<string, string>()
  const constants = readRootSource(root, "include/constants/event_objects.h")
  for (const match of constants.matchAll(
    /^#define\s+(OBJ_EVENT_GFX_\w+)\s+(OBJ_EVENT_GFX_\w+)\s*$/gm,
  )) {
    const [, alias, target] = match
    if (alias && target) {
      aliases.set(alias, target)
    }
  }
  const canonicalGraphicsId = (graphicsId: string): string => {
    const visited = new Set<string>()
    let current = graphicsId
    while (aliases.has(current) && !visited.has(current)) {
      visited.add(current)
      current = aliases.get(current)!
    }
    return current
  }
  for (const [graphicsId, graphicsInfo] of graphicsInfoById) {
    const canonical = canonicalGraphicsId(graphicsId)
    if (!graphicsInfoById.has(canonical)) {
      graphicsInfoById.set(canonical, graphicsInfo)
    }
  }

  const frameByGraphicsInfo = new Map<string, SourceFrame>()
  const info = readSource(root, "object_event_graphics_info.h")
  for (const match of info.matchAll(
    /const struct ObjectEventGraphicsInfo\s+(gObjectEventGraphicsInfo_\w+)\s*=\s*\{([^}]*)\};/g,
  )) {
    const [, graphicsInfo, body] = match
    const tableName = body?.match(/\b(?:sPicTable_|gFieldEffectObjectPicTable_)\w+\b/)?.[0]
    if (graphicsInfo && tableName) {
      const frame = resolveFrame(tableName, graphicsInfo, dimensionsFor(body ?? ""))
      if (frame) {
        frameByGraphicsInfo.set(graphicsInfo, frame)
      }
    }
  }

  const frameBySpecies = new Map<string, SourceFrame>()
  const legacyFollowersPath = sourceFile(root, "object_event_graphics_info_followers.h")
  if (fs.existsSync(legacyFollowersPath)) {
    const followers = readSource(root, "object_event_graphics_info_followers.h")
    for (const match of followers.matchAll(/\[\s*(SPECIES_\w+)\s*\]\s*=\s*\{([^}]*)\}/g)) {
      const [, species, body] = match
      const tableName = body?.match(/\b(sPicTable_\w+)\b/)?.[1]
      if (species && tableName) {
        const frame = resolveFrame(tableName, species, dimensionsFor(body ?? ""))
        if (frame) frameBySpecies.set(species, frame)
      }
    }
  }

  const speciesInfoDirectory = path.join(root, "src/data/pokemon/species_info")
  const speciesInfo = [
    readRootSource(root, "src/data/pokemon/species_info.h"),
    ...fs
      .readdirSync(speciesInfoDirectory)
      .filter((file) => file.endsWith(".h"))
      .sort()
      .map((file) => readRootSource(root, path.join("src/data/pokemon/species_info", file))),
  ].join("\n")
  for (const match of speciesInfo.matchAll(
    /^\s*\[\s*(SPECIES_\w+)\s*\]\s*=\s*\{([\s\S]*?)(?=^\s*\[\s*SPECIES_\w+\s*\]\s*=|(?![\s\S]))/gm,
  )) {
    const [, species, body] = match
    const tableName = body?.match(/\bOVERWORLD\(\s*(sPicTable_\w+)/)?.[1]
    if (species && tableName) {
      const frame = resolveFrame(tableName, species)
      if (frame) frameBySpecies.set(species, frame)
    }
  }
  return {
    graphicsInfoById,
    frameByGraphicsInfo,
    frameBySpecies,
    berryPreviewByName: berryPreviews.byName,
    berryPreviewByTreeId: berryAssignments,
    berryPreviewFallback: berryPreviews.fallback,
  }
}

const resolveFrame = (
  tables: ObjectSourceTables,
  event: ObjectEvent,
): SourceFrame | ObjectDiagnostic => {
  const { graphics_id: graphicsId } = event
  const species = speciesGraphicsFor(graphicsId)?.speciesId
  if (species) {
    return (
      tables.frameBySpecies.get(species) ?? {
        code: "unknown_species_graphics",
        message: `${graphicsId}: ${species} has no source follower graphic`,
      }
    )
  }
  if (graphicsId === "OBJ_EVENT_GFX_BERRY_TREE") {
    const assignedFrame = tables.berryPreviewByTreeId.get(event.trainer_sight_or_berry_tree_id)
    if (assignedFrame) return assignedFrame
    const berryNames = [...tables.berryPreviewByName.keys()]
      .filter((berryName) =>
        new RegExp(`(?:^|_)${berryName}(?:_|$)`).test(event.trainer_sight_or_berry_tree_id),
      )
      .sort((left, right) => right.length - left.length)
    const frame = berryNames.length > 0 ? tables.berryPreviewByName.get(berryNames[0]!) : null
    if (frame ?? tables.berryPreviewFallback) return frame ?? tables.berryPreviewFallback!
    return {
      code: "unresolved_berry_tree_graphics",
      message: `${graphicsId}: berry tree source preview is unavailable`,
    }
  }
  if (!/^OBJ_EVENT_GFX_\w+$/.test(graphicsId)) {
    return {
      code: "unsupported_graphics_expression",
      message: `${graphicsId}: expected an object graphics ID or supported species graphics expression`,
    }
  }
  const graphicsInfo = tables.graphicsInfoById.get(graphicsId)
  if (!graphicsInfo) {
    return { code: "unknown_graphics_id", message: `${graphicsId}: no graphics-info pointer` }
  }
  return (
    tables.frameByGraphicsInfo.get(graphicsInfo) ?? {
      code: "unresolved_graphics_info",
      message: `${graphicsId}: ${graphicsInfo} has no resolvable source frame`,
    }
  )
}

const isDiagnostic = (value: SourceFrame | ObjectDiagnostic): value is ObjectDiagnostic => {
  return "code" in value
}

const shinyPaletteFor = (
  root: string,
  frame: SourceFrame,
): { normal: string; shiny: string } | null => {
  const pokemonRoot = path.join(root, "graphics", "pokemon")
  const relative = path.relative(pokemonRoot, frame.source)
  const isPokemonSprite =
    relative !== "" && !relative.startsWith(`..${path.sep}`) && !path.isAbsolute(relative)
  const directory = isPokemonSprite
    ? path.join(pokemonRoot, relative.split(path.sep)[0]!)
    : path.join(pokemonRoot, path.basename(frame.source, ".png"))
  const normal = path.join(directory, isPokemonSprite ? "overworld_normal.pal" : "normal.pal")
  const shiny = path.join(directory, isPokemonSprite ? "overworld_shiny.pal" : "shiny.pal")
  return fs.existsSync(normal) && fs.existsSync(shiny) ? { normal, shiny } : null
}

const writeFrame = (
  output: string,
  frame: SourceFrame,
  shinyPalette: { normal: string; shiny: string } | null = null,
): CatalogObject["sprite"] => {
  if (!fs.existsSync(frame.source)) {
    return null
  }
  let cropped = cropSpriteFrame(frame.source, {
    index: frame.frameIndex,
    width: frame.widthPixels,
    height: frame.heightPixels,
  })
  if (shinyPalette) {
    cropped = replaceSpritePalette(
      cropped,
      readPalette(shinyPalette.normal),
      readPalette(shinyPalette.shiny),
    )
  }
  const digest = crypto
    .createHash("sha256")
    .update(`${cropped.width}x${cropped.height}:`)
    .update(cropped.pixels)
    .digest("hex")
  const filePath = path.join(output, "sprites", `${digest}.png`)
  if (!fs.existsSync(filePath)) {
    writeRgbaPng(filePath, cropped.width, cropped.height, cropped.pixels)
  }
  return {
    path: posixRelative(output, filePath),
    sha256: sha256(filePath),
    widthPixels: cropped.width,
    heightPixels: cropped.height,
    anchor: { xPixels: cropped.width / 2, yPixels: cropped.height },
    source: path.relative(output, frame.source).replaceAll("\\", "/"),
  }
}

export const catalogObjects = (
  root: string,
  output: string,
  events: ObjectEvent[],
  tables = objectSourceTables(root),
  scripts = new Map<string, string>(),
): CatalogObject[] => {
  return events.map((event, index) => {
    const objectId = String(index)
    const resolved = resolveFrame(tables, event)
    const isShiny = speciesGraphicsFor(event.graphics_id)?.isShiny ?? false
    let sprite: CatalogObject["sprite"] = null
    let diagnostic: ObjectDiagnostic | null = null
    if (isDiagnostic(resolved)) {
      diagnostic = resolved
    } else {
      try {
        sprite = writeFrame(output, resolved, isShiny ? shinyPaletteFor(root, resolved) : null)
        if (!sprite) {
          diagnostic = {
            code: "missing_source_sprite",
            message: `${event.graphics_id}: source file ${resolved.source} is missing`,
          }
        }
      } catch (error) {
        diagnostic = {
          code: "invalid_source_sprite",
          message: `${event.graphics_id}: ${error instanceof Error ? error.message : String(error)}`,
        }
      }
    }
    return {
      objectId,
      kind: classifyObject(event, sprite?.source ?? null, scripts.get(event.script)),
      graphicsId: event.graphics_id,
      isShiny,
      xMetatiles: event.x,
      yMetatiles: event.y,
      elevation: event.elevation,
      movementType: event.movement_type,
      movementRange: { x: event.movement_range_x, y: event.movement_range_y },
      trainerType: event.trainer_type,
      trainerSightOrBerryTreeId: event.trainer_sight_or_berry_tree_id,
      script: event.script,
      flag: event.flag,
      sprite,
      diagnostic,
    }
  })
}
