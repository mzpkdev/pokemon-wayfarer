import * as childProcess from "node:child_process"
import * as crypto from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"

import type { Layout, LayoutDocument, MapCatalog, MapGroups, SourceMap } from "./types"

const readJson = <T>(filePath: string): T => {
  return JSON.parse(fs.readFileSync(filePath, "utf8")) as T
}

export type WayfarerSeviiSelection = {
  maps: ReadonlySet<string>
  releaseMaps: ReadonlySet<string>
}

export const sourceWayfarerSeviiSelection = (root: string): WayfarerSeviiSelection => {
  const relativePath = "src/data/wayfarer_sevii_maps.json"
  const manifest = readJson<unknown>(path.join(root, relativePath))
  if (!manifest || typeof manifest !== "object" || Array.isArray(manifest)) {
    throw new Error(`${relativePath}: expected an object`)
  }
  const rootRecord = manifest as Record<string, unknown>
  if (typeof rootRecord.release_link_enabled !== "boolean") {
    throw new Error(`${relativePath}: release_link_enabled must be a boolean`)
  }
  if (!Array.isArray(rootRecord.maps)) {
    throw new Error(`${relativePath}: maps must be an array`)
  }
  const maps = new Set<string>()
  const releaseMaps = new Set<string>()
  for (const [index, value] of rootRecord.maps.entries()) {
    if (!value || typeof value !== "object" || Array.isArray(value)) {
      throw new Error(`${relativePath}: maps/${index} must be an object`)
    }
    const record = value as Record<string, unknown>
    const sourceMap = record.source_map
    if (typeof sourceMap !== "string" || sourceMap.length === 0) {
      throw new Error(`${relativePath}: maps/${index}/source_map must be a non-empty string`)
    }
    if (record.enabled !== undefined && typeof record.enabled !== "boolean") {
      throw new Error(`${relativePath}: maps/${index}/enabled must be a boolean`)
    }
    if (maps.has(sourceMap)) {
      throw new Error(`${relativePath}: duplicate source map ${JSON.stringify(sourceMap)}`)
    }
    maps.add(sourceMap)
    if (rootRecord.release_link_enabled && (record.enabled ?? true)) {
      releaseMaps.add(sourceMap)
    }
  }
  return { maps, releaseMaps }
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

export const sourceState = (root: string): MapCatalog["source"] => {
  return {
    revision: git(root, ["rev-parse", "HEAD"]) ?? "unknown",
    workingTreeDirty: Boolean(git(root, ["status", "--porcelain"])),
  }
}

export const sourceGroups = (root: string): Map<string, string> => {
  const groups = readJson<MapGroups>(path.join(root, "data/maps/map_groups.json"))
  const index = new Map<string, string>()
  for (const group of groups.group_order) {
    for (const name of groups[group] ?? []) {
      index.set(name, group)
    }
  }
  return index
}

export const sourceLayouts = (root: string): Map<string, Layout> => {
  return new Map(
    readJson<LayoutDocument>(path.join(root, "data/layouts/layouts.json")).layouts.map((layout) => [
      layout.id,
      layout,
    ]),
  )
}

export const sourceMaps = (root: string, names: string[]): Map<string, SourceMap> => {
  return new Map(
    names.map((name) => [
      name,
      readJson<SourceMap>(path.join(root, "data/maps", name, "map.json")),
    ]),
  )
}

export const sha256 = (filePath: string): string => {
  return crypto.createHash("sha256").update(fs.readFileSync(filePath)).digest("hex")
}

export const posixRelative = (root: string, filePath: string): string => {
  return path.relative(root, filePath).replaceAll("\\", "/")
}
