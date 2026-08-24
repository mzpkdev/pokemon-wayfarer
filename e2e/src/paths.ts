import { skyEmuBinary as staticSkyEmuBinary } from "static-skyemu"

export const skyEmuBinary = process.env.SKYEMU_BIN ?? staticSkyEmuBinary

export const requireRomPath = (): string => {
  const romPath = process.env.SKYEMU_ROM
  if (!romPath) {
    throw new Error(
      "SKYEMU_ROM is required. Point it at a ROM file, for example: SKYEMU_ROM=/path/to/game.gba pnpm test",
    )
  }
  return romPath
}
