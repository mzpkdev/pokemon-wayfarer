import * as fs from "node:fs"
import * as path from "node:path"

import { readDefine } from "./source"
import {
  loadRenderAssets,
  metatileAttributeSize,
  normalizeLayoutFormat,
  readLayoutFormatCounts,
  readMetatileAttribute,
} from "./tilesets"
import type { EncounterHabitat, EncounterHabitatRectangle, Layout } from "./types"

type BehaviorFlags = {
  hasEncounters: boolean
  surfable: boolean
}

type HabitatKind = keyof EncounterHabitat

type BehaviorTables = {
  flagsByBehavior: ReadonlyMap<number, BehaviorFlags>
  bridgeBehaviors: ReadonlySet<number>
}

const behaviorTables = new Map<string, BehaviorTables>()

const behaviorTablesFor = (root: string): BehaviorTables => {
  const cached = behaviorTables.get(root)
  if (cached) return cached
  const constants = fs.readFileSync(
    path.join(root, "include/constants/metatile_behaviors.h"),
    "utf8",
  )
  const values = new Map<string, number>()
  for (const [, name, value] of constants.matchAll(/^#define\s+(MB_\w+)\s+(0x[\dA-F]+|\d+)/gm)) {
    values.set(name!, Number(value))
  }
  const behaviorEnum = constants.match(/enum\s*\{([\s\S]*?)\};/)?.[1]
  if (behaviorEnum) {
    let value = 0
    for (const [, name, explicitValue] of behaviorEnum.matchAll(
      /^\s*(MB_\w+)(?:\s*=\s*(0x[\dA-F]+|\d+))?,/gm,
    )) {
      if (explicitValue) value = Number(explicitValue)
      if (name) values.set(name, value)
      value += 1
    }
  }

  const behaviors = fs.readFileSync(path.join(root, "src/metatile_behavior.c"), "utf8")
  const flags = new Map<number, BehaviorFlags>()
  for (const [, name, expression] of behaviors.matchAll(/^\s*\[(MB_\w+)\]\s*=\s*([^,\n]+),/gm)) {
    const value = values.get(name!)
    if (value === undefined) continue
    flags.set(value, {
      hasEncounters: expression!.includes("TILE_FLAG_HAS_ENCOUNTERS"),
      surfable: expression!.includes("TILE_FLAG_SURFABLE"),
    })
  }
  const bridgeFunction =
    /bool8 MetatileBehavior_IsBridgeOverWater\(u8 metatileBehavior\)\s*\{([\s\S]*?)\n\}/.exec(
      behaviors,
    )?.[1]
  const bridgeBehaviors = new Set(
    [...(bridgeFunction?.matchAll(/\b(MB_\w+)\b/g) ?? [])]
      .map((match) => values.get(match[1]!))
      .filter((value): value is number => value !== undefined),
  )
  const tables = { flagsByBehavior: flags, bridgeBehaviors }
  behaviorTables.set(root, tables)
  return tables
}

const mergeRectangles = (
  cells: ReadonlyMap<HabitatKind, Set<string>>,
  layout: Layout,
): EncounterHabitat => {
  const result: EncounterHabitat = { land: [], water: [] }
  for (const kind of ["land", "water"] as const) {
    const active = new Map<string, EncounterHabitatRectangle>()
    for (let y = 0; y < layout.height; y += 1) {
      const current = new Map<string, EncounterHabitatRectangle>()
      let x = 0
      while (x < layout.width) {
        if (!cells.get(kind)?.has(`${x}:${y}`)) {
          x += 1
          continue
        }
        const start = x
        while (x < layout.width && cells.get(kind)?.has(`${x}:${y}`)) x += 1
        const width = x - start
        const key = `${start}:${width}`
        const previous = active.get(key)
        const rectangle =
          previous && previous.yMetatiles + previous.heightMetatiles === y
            ? { ...previous, heightMetatiles: previous.heightMetatiles + 1 }
            : { xMetatiles: start, yMetatiles: y, widthMetatiles: width, heightMetatiles: 1 }
        current.set(key, rectangle)
      }
      for (const [key, rectangle] of active) {
        if (!current.has(key)) result[kind].push(rectangle)
      }
      active.clear()
      for (const [key, rectangle] of current) active.set(key, rectangle)
    }
    result[kind].push(...active.values())
  }
  return result
}

/** Reproduce the runtime's land and water encounter-tile classification from source assets. */
export const encounterHabitat = (root: string, layout: Layout): EncounterHabitat => {
  const layoutFormat = normalizeLayoutFormat(layout)
  const [primaryTileCount, primaryMetatileCount, primaryPaletteCount] =
    readLayoutFormatCounts(layoutFormat)
  const paletteCount = readDefine(path.join(root, "include/fieldmap.h"), "NUM_PALS_TOTAL")
  const assets = loadRenderAssets(root, layout, primaryTileCount, primaryPaletteCount, paletteCount)
  const primaryAttributeSize = metatileAttributeSize(
    assets.primaryMetatileAttributes,
    assets.primaryMetatiles.length / 16,
  )
  const secondaryAttributeSize = metatileAttributeSize(
    assets.secondaryMetatileAttributes,
    assets.secondaryMetatiles.length / 16,
  )
  const blockdata = fs.readFileSync(path.join(root, layout.blockdata_filepath))
  const cells = new Map<HabitatKind, Set<string>>([
    ["land", new Set()],
    ["water", new Set()],
  ])
  const { flagsByBehavior, bridgeBehaviors } = behaviorTablesFor(root)
  for (let y = 0; y < layout.height; y += 1) {
    for (let x = 0; x < layout.width; x += 1) {
      const offset = (y * layout.width + x) * 2
      if (offset + 2 > blockdata.length) continue
      const metatileId = blockdata.readUInt16LE(offset) & 0x3ff
      const secondary = metatileId >= primaryMetatileCount
      const index = secondary ? metatileId - primaryMetatileCount : metatileId
      const attributes = secondary
        ? assets.secondaryMetatileAttributes
        : assets.primaryMetatileAttributes
      const attributeSize = secondary ? secondaryAttributeSize : primaryAttributeSize
      let rawAttributes: number
      try {
        rawAttributes = readMetatileAttribute(attributes, index, attributeSize)
      } catch {
        continue
      }
      const behavior = rawAttributes & (attributeSize === 4 ? 0x1ff : 0xff)
      const flags = flagsByBehavior.get(behavior)
      const kind =
        flags?.hasEncounters && flags.surfable
          ? "water"
          : bridgeBehaviors.has(behavior)
            ? "water"
            : flags?.hasEncounters
              ? "land"
              : null
      if (kind) cells.get(kind)?.add(`${x}:${y}`)
    }
  }
  return mergeRectangles(cells, layout)
}
