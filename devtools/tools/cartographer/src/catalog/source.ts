import * as childProcess from "node:child_process"
import * as crypto from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"

import type { Layout, LayoutDocument, MapCatalog, MapGroups, SourceMap } from "./types"

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
