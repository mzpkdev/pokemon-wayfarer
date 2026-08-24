import { join } from "node:path"
import { fileURLToPath } from "node:url"

export const e2eRoot = fileURLToPath(new URL("..", import.meta.url))
export const skyEmuDirectory = process.env.SKYEMU_DIR ?? join(e2eRoot, ".skyemu")
export const skyEmuBinary =
  process.env.SKYEMU_BIN ?? join(skyEmuDirectory, "build", "bin", "SkyEmu")

export const requireRomPath = (): string => {
  const romPath = process.env.SKYEMU_ROM
  if (!romPath) {
    throw new Error(
      "SKYEMU_ROM is required. Point it at a ROM file, for example: SKYEMU_ROM=/path/to/game.gba pnpm test",
    )
  }
  return romPath
}
