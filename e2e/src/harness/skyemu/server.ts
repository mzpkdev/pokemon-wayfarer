import * as childProcess from "node:child_process"
import * as fs from "node:fs"
import { skyEmuBinary } from "skyemu-static"

import { SkyEmuClient } from "./client"
import { captureOutputTail, reserveTcpPort, stopProcess, waitFor } from "./utils"

export type RunningSkyEmu = {
  client: SkyEmuClient
  stop: () => Promise<void>
}

export const startSkyEmu = async (romPath: string): Promise<RunningSkyEmu> => {
  await fs.promises.access(skyEmuBinary)
  const port = await reserveTcpPort()
  const child = childProcess.spawn(
    "xvfb-run",
    ["--auto-servernum", skyEmuBinary, "http_server", `${port}`, romPath],
    { detached: true, stdio: ["ignore", "pipe", "pipe"] },
  )
  let launchError: Error | undefined
  child.once("error", (error) => {
    launchError = error
  })
  const output = captureOutputTail([child.stdout, child.stderr])
  const client = new SkyEmuClient(port)

  try {
    await waitFor(async () => {
      if (launchError) {
        throw new Error(`Could not launch SkyEmu: ${launchError.message}`, { cause: launchError })
      }
      if (child.exitCode !== null || child.signalCode !== null) {
        await output.closed
        const result =
          child.exitCode !== null ? `code ${child.exitCode}` : `signal ${child.signalCode}`
        const diagnostic = output.read()
        throw new Error(
          `SkyEmu exited with ${result} before becoming ready${diagnostic ? `:\n${diagnostic}` : ""}`,
        )
      }
      try {
        return (await client.health()).ready
      } catch {
        return false
      }
    }, 30_000)
    const loaded = await client.loadRom(romPath)
    if (loaded !== "ok") throw new Error(`SkyEmu failed to load the ROM paused: ${loaded}`)
    const released = await client.input({
      A: 0,
      B: 0,
      Down: 0,
      L: 0,
      Left: 0,
      R: 0,
      Right: 0,
      Select: 0,
      Start: 0,
      Up: 0,
    })
    if (released !== "ok") throw new Error(`SkyEmu failed to clear controller input: ${released}`)
  } catch (error) {
    await stopProcess(child)
    throw error
  }

  return { client, stop: () => stopProcess(child) }
}
