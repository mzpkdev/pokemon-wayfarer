import { describe, expect, it } from "webanvil/test"

import { type SessionRuntime } from "../runtime"
import { createStateApi } from "./state"

const indexAddress = 0x0200_0ff0
const buffersAddress = 0x0200_1000
const stateSize = 1756
const committedFrameOffset = 1752
const partyCountOffset = 105

// The ROM's double buffer: reads of the index and of either buffer, plus a
// hook to run between reads (another caller's step).
const fakeRuntime = (buffers: [Uint8Array, Uint8Array], index: number, onRead?: (reads: number) => void) => {
  let frames = 0
  let reads = 0
  const memory = { index, buffers }
  const runtime = {
    abi: { stateSize } as SessionRuntime["abi"],
    address: (symbol: string) => {
      if (symbol === "gE2ETestStateCommittedIndex") return indexAddress
      if (symbol === "gE2ETestStateCommitted") return buffersAddress
      throw new Error(`Unexpected symbol ${symbol}`)
    },
    readBytes: async (address: number, length: number) => {
      reads++
      onRead?.(reads)
      if (address === indexAddress) {
        expect(length).toBe(1)
        return Uint8Array.of(memory.index)
      }
      expect(length).toBe(stateSize)
      const slot = (address - buffersAddress) / stateSize
      expect(slot === 0 || slot === 1).toBe(true)
      return memory.buffers[slot]!
    },
    readUint16: async () => 0,
    readUint32: async () => 0,
    writeBytes: async () => {},
    advance: async (count: number) => {
      frames += count
    },
    press: async () => {},
  } satisfies SessionRuntime
  return { runtime, memory, frames: () => frames }
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
  it("reads the published snapshot without advancing the emulator", async () => {
    const fake = fakeRuntime([snapshot(6, 6, 1), snapshot(7, 7, 2)], 1)
    const state = await createStateApi(fake.runtime).read()

    expect(state.frame).toBe(7)
    expect(state.party).toHaveLength(2)
    expect(fake.frames()).toBe(0)
  })

  it("re-reads, without stepping, when a step flips the buffers between its reads", async () => {
    const fake = fakeRuntime([snapshot(8, 8, 0), snapshot(7, 7, 0)], 1, (reads) => {
      // After the first buffer read a step lands: buffer 0 now holds frame 9.
      if (reads === 2) {
        fake.memory.buffers[0] = snapshot(9, 9, 2)
        fake.memory.index = 0
      }
    })
    const state = await createStateApi(fake.runtime).read()

    expect(state.frame).toBe(9)
    expect(state.party).toHaveLength(2)
    expect(fake.frames()).toBe(0)
  })

  it("never returns a half-copied snapshot", async () => {
    const fake = fakeRuntime([snapshot(8, 7, 0), snapshot(8, 7, 0)], 0)

    await expect(createStateApi(fake.runtime).read()).rejects.toThrow("changed under")
    expect(fake.frames()).toBe(0)
  })
})
