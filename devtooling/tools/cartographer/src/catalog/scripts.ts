import * as fs from "node:fs"
import * as path from "node:path"

const labelPattern = /^([A-Za-z]\w*)::?\s*$/gm

/** Read top-level scripts for one map without following calls or interpreting control flow. */
export const mapScriptBodies = (root: string, mapName: string): Map<string, string> => {
  const scriptPath = path.join(root, "data", "maps", mapName, "scripts.inc")
  if (!fs.existsSync(scriptPath)) return new Map()
  const source = fs.readFileSync(scriptPath, "utf8").replaceAll(/\/\*[\s\S]*?\*\/|\/\/[^\n]*/g, "")
  const labels = [...source.matchAll(labelPattern)]
  const result = new Map<string, string>()
  for (const [index, label] of labels.entries()) {
    const name = label[1]
    if (!name || label.index === undefined) continue
    const start = label.index + label[0].length
    const end = labels[index + 1]?.index ?? source.length
    result.set(name, source.slice(start, end))
  }
  return result
}
