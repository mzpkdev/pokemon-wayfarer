import * as fs from "node:fs"

export type SkyEmuSymbols = {
  address: (name: string) => number
}

export const parseSkyEmuSymbols = (source: string): SkyEmuSymbols => {
  const globals = new Map<string, number>()
  const locals = new Map<string, Set<number>>()

  for (const line of source.split("\n")) {
    const [address, binding, , name] = line.trim().split(/\s+/, 4)
    if (!address || !binding || !name || name.startsWith(".") || !/^[0-9a-f]{8}$/i.test(address))
      continue

    const value = Number.parseInt(address, 16)
    if (binding === "g") {
      const previous = globals.get(name)
      if (previous !== undefined && previous !== value) {
        throw new Error(`SkyEmu symbol file defines conflicting global symbol ${name}`)
      }
      globals.set(name, value)
      continue
    }

    const values = locals.get(name) ?? new Set<number>()
    values.add(value)
    locals.set(name, values)
  }

  return {
    address: (name: string): number => {
      const global = globals.get(name)
      if (global !== undefined) return global

      const local = locals.get(name)
      if (!local) throw new Error(`SkyEmu symbol file does not define ${name}`)
      if (local.size !== 1)
        throw new Error(`SkyEmu symbol file defines ambiguous local symbol ${name}`)
      return local.values().next().value as number
    },
  }
}

export const readSkyEmuSymbols = async (filePath: string): Promise<SkyEmuSymbols> =>
  parseSkyEmuSymbols(await fs.promises.readFile(filePath, "utf8"))
