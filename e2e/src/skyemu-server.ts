import * as childProcess from "node:child_process"
import * as fs from "node:fs"

import { requireRomPath, skyEmuBinary } from "./paths"
import { SkyEmuClient } from "./skyemu"
import { choosePort, stopProcess, waitFor } from "./utils"

export type RunningSkyEmu = {
  client: SkyEmuClient
  stop: () => Promise<void>
}

export const startSkyEmu = async (): Promise<RunningSkyEmu> => {
  await fs.promises.access(skyEmuBinary)
  const port = await choosePort(process.env.SKYEMU_PORT)
  const romPath = requireRomPath()
  const command = process.env.DISPLAY ? skyEmuBinary : "xvfb-run"
  const args = process.env.DISPLAY
    ? ["http_server", `${port}`, romPath]
    : ["--auto-servernum", skyEmuBinary, "http_server", `${port}`, romPath]
  const child = childProcess.spawn(command, args, { stdio: "inherit" })
  const client = new SkyEmuClient(port)

  try {
    await waitFor(async () => {
      if (child.exitCode !== null) {
        throw new Error(`SkyEmu exited with code ${child.exitCode} before becoming ready`)
      }
      try {
        return (await client.ping()) === "pong"
      } catch {
        return false
      }
    }, 30_000)
  } catch (error) {
    child.kill()
    throw error
  }

  return { client, stop: () => stopProcess(child) }
}
