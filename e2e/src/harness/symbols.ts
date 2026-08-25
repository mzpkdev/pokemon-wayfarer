import * as fs from "node:fs"

export type SkyEmuSymbols = {
  address: (name: string) => number
}

export const parseSkyEmuSymbols = (source: string): SkyEmuSymbols => {
  const addresses = new Map<string, number>()

  for (const line of source.split("\n")) {
    const [address, binding, , name] = line.trim().split(/\s+/, 4)
    if (!address || binding !== "g" || !name || !/^[0-9a-f]{8}$/i.test(address)) continue

    addresses.set(name, Number.parseInt(address, 16))
  }

  return {
    address: (name: string): number => {
      const address = addresses.get(name)
      if (address === undefined) throw new Error(`SkyEmu symbol file does not define ${name}`)
      return address
    },
  }
}

export const readSkyEmuSymbols = async (filePath: string): Promise<SkyEmuSymbols> =>
  parseSkyEmuSymbols(await fs.promises.readFile(filePath, "utf8"))
