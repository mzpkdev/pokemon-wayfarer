import * as childProcess from "node:child_process"
import * as fs from "node:fs"
import * as net from "node:net"

import { requireRomPath, skyEmuBinary } from "./paths.js"
import { SkyEmuClient } from "./skyemu.js"

const waitFor = async (condition: () => Promise<boolean>, timeoutMs: number): Promise<void> => {
  const deadline = Date.now() + timeoutMs
  while (Date.now() < deadline) {
    if (await condition()) return
    await new Promise((resolve) => setTimeout(resolve, 250))
  }
  throw new Error(`SkyEmu did not become ready within ${timeoutMs / 1000} seconds`)
}

const reservePort = async (): Promise<number> =>
  new Promise((resolve, reject) => {
    const server = net.createServer()
    server.once("error", reject)
    server.listen(0, "127.0.0.1", () => {
      const address = server.address()
      if (!address || typeof address === "string") {
        reject(new Error("Could not reserve a TCP port for SkyEmu"))
        return
      }
      server.close((error) => (error ? reject(error) : resolve(address.port)))
    })
  })

const configuredPort = async (): Promise<number> => {
  const value = process.env.SKYEMU_PORT
  if (!value) return reservePort()
  const port = Number(value)
  if (!Number.isInteger(port) || port < 1 || port > 65_535) {
    throw new Error(`SKYEMU_PORT must be an integer between 1 and 65535, received ${value}`)
  }
  return port
}

export type RunningSkyEmu = {
  client: SkyEmuClient
  stop: () => Promise<void>
}

export const startSkyEmu = async (): Promise<RunningSkyEmu> => {
  await fs.promises.access(skyEmuBinary)
  const port = await configuredPort()
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

const stopProcess = async (child: childProcess.ChildProcess): Promise<void> => {
  if (child.exitCode !== null) return
  child.kill()
  await new Promise<void>((resolve) => child.once("exit", () => resolve()))
}
