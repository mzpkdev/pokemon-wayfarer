import { type SkyEmuButton, type SkyEmuClient } from "../skyemu"
import { type SkyEmuSymbols } from "../symbols"
import { type SessionAbi } from "./protocol"

export type SessionRuntime = {
  abi: SessionAbi
  address: (symbol: string) => number
  readBytes: (address: number, length: number) => Promise<Uint8Array>
  readUint16: (address: number) => Promise<number>
  readUint32: (address: number) => Promise<number>
  writeBytes: (address: number, bytes: Uint8Array) => Promise<void>
  advance: (frames: number) => Promise<void>
  press: (button: SkyEmuButton, holdFrames?: number, releaseFrames?: number) => Promise<void>
}

export const createSessionRuntime = (
  client: SkyEmuClient,
  symbols: SkyEmuSymbols,
  abi: SessionAbi,
): SessionRuntime => {
  const advance = async (frames: number): Promise<void> => {
    const result = await client.step(frames)
    if (result !== "ok") throw new Error(`SkyEmu failed to advance ${frames} frames: ${result}`)
  }

  const press = async (button: SkyEmuButton, holdFrames = 2, releaseFrames = 2): Promise<void> => {
    const pressed = await client.input({ [button]: 1 })
    if (pressed !== "ok") throw new Error(`SkyEmu failed to press ${button}: ${pressed}`)
    await advance(holdFrames)
    const released = await client.input({ [button]: 0 })
    if (released !== "ok") throw new Error(`SkyEmu failed to release ${button}: ${released}`)
    await advance(releaseFrames)
  }

  return {
    abi,
    address: (symbol) => symbols.address(symbol),
    readBytes: (address, length) => client.readBytes(address, length),
    readUint16: (address) => client.readUint16LE(address),
    readUint32: (address) => client.readUint32LE(address),
    writeBytes: (address, bytes) => client.writeBytes(address, bytes),
    advance,
    press,
  }
}
