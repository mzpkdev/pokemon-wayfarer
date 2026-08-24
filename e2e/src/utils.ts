import * as childProcess from "node:child_process"
import * as net from "node:net"

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

const reserveTcpPort = async (): Promise<number> =>
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

export const choosePort = async (value: string | undefined): Promise<number> => {
  if (!value) return reserveTcpPort()
  const port = Number(value)
  if (!Number.isInteger(port) || port < 1 || port > 65_535) {
    throw new Error(`SKYEMU_PORT must be an integer between 1 and 65535, received ${value}`)
  }
  return port
}

export const stopProcess = async (child: childProcess.ChildProcess): Promise<void> => {
  if (child.exitCode !== null) return
  child.kill()
  await new Promise<void>((resolve) => child.once("exit", () => resolve()))
}
