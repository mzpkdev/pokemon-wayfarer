import { describe, expect, it } from "webanvil/test"

import { type SessionRuntime } from "../runtime"
import { createInventoryApi } from "./inventory"

const pocketsAddress = 0x0200_1000
const keyItemsAddress = 0x0200_2000

const uint16Bytes = (value: number): number[] => [value & 0xff, value >> 8]

describe("game-session inventory", () => {
  it("counts Standard Rod slots through the validated HNS Key Items pocket", async () => {
    const keyItems = new Uint8Array(60 * 4)
    keyItems.set(uint16Bytes(709), 0)
    keyItems.set(uint16Bytes(710), 4)
    keyItems.set(uint16Bytes(711), 8)

    const runtime = {
      abi: {} as SessionRuntime["abi"],
      address: (symbol: string) => {
        if (symbol !== "gBagPockets") throw new Error(`Unexpected symbol ${symbol}`)
        return pocketsAddress
      },
      readBytes: async (address: number, length: number) => {
        if (address === pocketsAddress + 5 * 6) return new Uint8Array(length)
        if (address === pocketsAddress + 5 * 8) {
          const pocket = new Uint8Array(length)
          new DataView(pocket.buffer).setUint32(0, keyItemsAddress, true)
          new DataView(pocket.buffer).setUint16(4, (5 << 10) | 60, true)
          return pocket
        }
        if (address === keyItemsAddress && length === keyItems.length) return keyItems
        throw new Error(`Unexpected read at ${address.toString(16)} (${length} bytes)`)
      },
      readUint16: async () => 0,
      readUint32: async () => 0,
      writeBytes: async () => {},
      advance: async () => {},
      press: async () => {},
    } satisfies SessionRuntime

    await expect(createInventoryApi(runtime).rodSlots()).resolves.toEqual({
      oldRod: 1,
      goodRod: 1,
      superRod: 1,
    })
  })
})
