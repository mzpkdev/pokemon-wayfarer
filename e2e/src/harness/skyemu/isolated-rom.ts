import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

export type IsolatedRom = {
  cleanup: () => Promise<void>
  path: string
}

export const createIsolatedRom = async (sourcePath: string): Promise<IsolatedRom> => {
  const directory = await fs.promises.mkdtemp(path.join(os.tmpdir(), "wayfarer-skyemu-smoke-"))
  const romPath = path.join(directory, "wayfarer.gba")
  await fs.promises.copyFile(sourcePath, romPath)

  return {
    cleanup: () => fs.promises.rm(directory, { force: true, recursive: true }),
    path: romPath,
  }
}
