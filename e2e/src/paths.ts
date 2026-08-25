import { skyEmuBinary as skyEmuStaticBinary } from "skyemu-static"

export const skyEmuBinary = process.env.SKYEMU_BIN ?? skyEmuStaticBinary

export const requireRomPath = (): string => {
  const romPath = process.env.SKYEMU_ROM
  if (!romPath) {
    throw new Error(
      "SKYEMU_ROM is required. Point it at a ROM file, for example: SKYEMU_ROM=/path/to/game.gba pnpm test",
    )
  }
  return romPath
}
