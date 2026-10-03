import { describe, expect, it } from "webanvil/test"

import { type SessionRuntime } from "../runtime"
import { createStateApi } from "./state"

const stateAddress = 0x0200_1000
const stateSize = 1756
const committedFrameOffset = 1752
const partyCountOffset = 105

const fakeRuntime = (snapshots: Uint8Array[]) => {
  let frames = 0
  const runtime = {
    abi: { stateSize } as SessionRuntime["abi"],
    address: (symbol: string) => {
      if (symbol === "gE2ETestState") return stateAddress
      throw new Error(`Unexpected symbol ${symbol}`)
    },
    readBytes: async (address: number, length: number) => {
      expect(address).toBe(stateAddress)
      expect(length).toBe(stateSize)
      return snapshots[Math.min(frames, snapshots.length - 1)]!
    },
    readUint16: async () => 0,
    readUint32: async () => 0,
    writeBytes: async () => {},
    advance: async (count: number) => {
      frames += count
    },
    press: async () => {},
  } satisfies SessionRuntime
  return { runtime, frames: () => frames }
}

const snapshot = (frame: number, committedFrame: number, partyCount: number): Uint8Array => {
  const bytes = new Uint8Array(stateSize)
  const view = new DataView(bytes.buffer)
  view.setUint32(0, frame, true)
  view.setUint32(committedFrameOffset, committedFrame, true)
  bytes[partyCountOffset] = partyCount
  return bytes
}

describe("game-session state", () => {
  it("reads a committed snapshot without advancing the emulator", async () => {
    const fake = fakeRuntime([snapshot(7, 7, 2)])
    const state = await createStateApi(fake.runtime).read()

    expect(state.frame).toBe(7)
    expect(state.party).toHaveLength(2)
    expect(fake.frames()).toBe(0)
  })

  it("advances past a snapshot the ROM was still writing", async () => {
    const fake = fakeRuntime([snapshot(8, 7, 0), snapshot(9, 8, 0), snapshot(9, 9, 2)])
    const state = await createStateApi(fake.runtime).read()

    expect(state.frame).toBe(9)
    expect(state.party).toHaveLength(2)
    expect(fake.frames()).toBe(2)
  })

  it("fails when the ROM never commits a snapshot", async () => {
    const fake = fakeRuntime([snapshot(8, 7, 0)])

    await expect(createStateApi(fake.runtime).read()).rejects.toThrow("stayed mid-update")
  })
})
