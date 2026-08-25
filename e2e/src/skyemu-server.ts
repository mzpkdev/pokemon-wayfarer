import * as childProcess from "node:child_process"
import * as fs from "node:fs"
import { skyEmuBinary } from "skyemu-static"

import { SkyEmuClient } from "./skyemu"
import { requireRomPath, reserveTcpPort, stopProcess, waitFor } from "./utils"

export type RunningSkyEmu = {
  client: SkyEmuClient
  stop: () => Promise<void>
}

export const startSkyEmu = async (): Promise<RunningSkyEmu> => {
  await fs.promises.access(skyEmuBinary)
  const port = await reserveTcpPort()
  const romPath = requireRomPath()
  const child = childProcess.spawn(
    "xvfb-run",
    ["--auto-servernum", skyEmuBinary, "http_server", `${port}`, romPath],
    { stdio: "inherit" },
  )
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
