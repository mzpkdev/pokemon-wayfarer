import * as path from "node:path"
import * as url from "node:url"

export const e2eRoot = url.fileURLToPath(new URL("..", import.meta.url))
export const skyEmuDirectory = process.env.SKYEMU_DIR ?? path.join(e2eRoot, ".skyemu")
export const skyEmuBinary = process.env.SKYEMU_BIN ?? path.join(skyEmuDirectory, "SkyEmu")

export const requireRomPath = (): string => {
  const romPath = process.env.SKYEMU_ROM
  if (!romPath) {
    throw new Error(
      "SKYEMU_ROM is required. Point it at a ROM file, for example: SKYEMU_ROM=/path/to/game.gba pnpm test",
    )
  }
  return romPath
}
