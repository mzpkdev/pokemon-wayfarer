import * as childProcess from "node:child_process"
import * as fs from "node:fs"
import * as path from "node:path"

import type { SourceLayout, SourceMap, SourceMapReference, SourceState } from "./types"

type LayoutDocument = {
  layouts: SourceLayout[]
}

const readJson = <T>(filePath: string): T => {
  return JSON.parse(fs.readFileSync(filePath, "utf8")) as T
}

const git = (root: string, args: string[]): string | null => {
  try {
    return childProcess
      .execFileSync("git", args, {
        cwd: root,
        encoding: "utf8",
        stdio: ["ignore", "pipe", "ignore"],
      })
      .trim()
  } catch {
    return null
  }
}

export const findRepositoryRoot = (start: string): string => {
  let current = path.resolve(start)
  while (true) {
    if (
      fs.existsSync(path.join(current, "data/maps")) &&
      fs.existsSync(path.join(current, "data/layouts/layouts.json"))
    ) {
      return current
    }
    const parent = path.dirname(current)
    if (parent === current) {
      throw new Error("cannot find a Pokémon Wayfarer source tree; pass --repo <path>")
    }
    current = parent
  }
}

export const sourceState = (root: string): SourceState => {
  return {
    revision: git(root, ["rev-parse", "HEAD"]) ?? "unknown",
    workingTreeDirty: Boolean(git(root, ["status", "--porcelain"])),
  }
}

export const sourceLayouts = (root: string): SourceLayout[] => {
  return readJson<LayoutDocument>(path.join(root, "data/layouts/layouts.json")).layouts
}

export const sourceMaps = (root: string): SourceMapReference[] => {
  const mapsRoot = path.join(root, "data/maps")
  return fs
    .readdirSync(mapsRoot)
    .sort()
    .flatMap((name) => {
      const mapPath = path.join(mapsRoot, name, "map.json")
      if (!fs.existsSync(mapPath)) return []
      const map = readJson<SourceMap>(mapPath)
      if (!map.id || !map.layout) {
        throw new Error(`${mapPath}: map is missing an id or layout`)
      }
      return [{ name, id: map.id, layoutId: map.layout }]
    })
}

export const sourceBehaviorNames = (root: string): Map<number, string> => {
  const source = fs.readFileSync(path.join(root, "include/constants/metatile_behaviors.h"), "utf8")
  const behaviors = new Map<number, string>()
  for (const match of source.matchAll(/^\s*#define\s+(MB_[A-Z0-9_]+)\s+(0x[\dA-F]+|\d+)\b/gm)) {
    const name = match[1]
    const rawValue = match[2]
    if (!name || !rawValue || name === "MB_INVALID") continue
    behaviors.set(Number.parseInt(rawValue, 0), name)
  }
  const behaviorEnum = source.match(/enum\s*\{([\s\S]*?)\};/)?.[1]
  if (behaviorEnum) {
    let value = 0
    for (const [, name, explicitValue] of behaviorEnum.matchAll(
      /^\s*(MB_[A-Z0-9_]+)(?:\s*=\s*(0x[\dA-F]+|\d+))?,/gm,
    )) {
      if (explicitValue) value = Number.parseInt(explicitValue, 0)
      if (name !== "MB_INVALID") behaviors.set(value, name!)
      value += 1
    }
  }
  return behaviors
}

export const readDefine = (filePath: string, name: string): number => {
  const source = fs.readFileSync(filePath, "utf8")
  const match = new RegExp(`^\\s*#define\\s+${name}\\s+(\\d+)\\s*$`, "m").exec(source)
  if (!match?.[1]) {
    throw new Error(`cannot resolve ${name} from ${filePath}`)
  }
  return Number(match[1])
}

export const posixRelative = (root: string, filePath: string): string => {
  return path.relative(root, filePath).replaceAll("\\", "/")
}
