import * as fs from "node:fs"

export const readJson = <T>(filePath: string): T => {
  return JSON.parse(fs.readFileSync(filePath, "utf8")) as T
}

export const readDefine = (filePath: string, name: string): number => {
  const source = fs.readFileSync(filePath, "utf8")
  const match = new RegExp(`^\\s*#define\\s+${name}\\s+(\\d+)\\s*$`, "m").exec(source)
  if (!match?.[1]) {
    throw new Error(`cannot resolve ${name} from ${filePath}`)
  }
  return Number(match[1])
}
