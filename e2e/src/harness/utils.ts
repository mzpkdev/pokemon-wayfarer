import * as childProcess from "node:child_process"
import * as net from "node:net"

export const requireRomPath = (): string => {
  const romPath = process.env.SKYEMU_ROM
  if (!romPath) {
    throw new Error(
      "SKYEMU_ROM is required. Point it at a ROM file, for example: SKYEMU_ROM=/path/to/game.gba pnpm test",
    )
  }
  return romPath
}

export const waitFor = async (
  condition: () => Promise<boolean>,
  timeoutMs: number,
): Promise<void> => {
  const deadline = Date.now() + timeoutMs
  while (Date.now() < deadline) {
    if (await condition()) return
    await new Promise((resolve) => setTimeout(resolve, 250))
  }
  throw new Error(`SkyEmu did not become ready within ${timeoutMs / 1000} seconds`)
}

export const reserveTcpPort = async (): Promise<number> =>
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

export const stopProcess = async (child: childProcess.ChildProcess): Promise<void> => {
  if (child.exitCode !== null) return
  child.kill()
  await new Promise<void>((resolve) => child.once("exit", () => resolve()))
}
