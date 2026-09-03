import { describe, expect, it } from "webanvil/test"

import { type SessionRuntime } from "../runtime"
import { createInventoryApi } from "./inventory"

const pocketsAddress = 0x0200_1000
const keyItemsAddress = 0x0200_2000

const uint16Bytes = (value: number): number[] => [value & 0xff, value >> 8]

describe("game-session inventory", () => {
  it("finds traversal items across the HNS Items, TM/HM, and Key Items pockets", async () => {
    const pocketContents = new Map([
      [3, { address: 0x0200_3000, capacity: 4, item: 441 }],
      [4, { address: 0x0200_4000, capacity: 4, item: 606 }],
      [5, { address: 0x0200_5000, capacity: 60, item: 727 }],
    ])
    const bagReadLengths: number[] = []
    const runtime = {
      abi: {} as SessionRuntime["abi"],
      address: (symbol: string) => {
        if (symbol !== "gBagPockets") throw new Error(`Unexpected symbol ${symbol}`)
        return pocketsAddress
      },
      readBytes: async (address: number, length: number) => {
        for (const [id, pocket] of pocketContents) {
          if (address === pocketsAddress + id * 8) {
            const descriptor = new Uint8Array(length)
            new DataView(descriptor.buffer).setUint32(0, pocket.address, true)
            new DataView(descriptor.buffer).setUint16(4, (id << 10) | pocket.capacity, true)
            return descriptor
          }
          const pocketLength = pocket.capacity * 4
          if (address >= pocket.address && address + length <= pocket.address + pocketLength) {
            bagReadLengths.push(length)
            const contents = new Uint8Array(pocketLength)
            contents.set(uint16Bytes(pocket.item), 0)
            return contents.slice(address - pocket.address, address - pocket.address + length)
          }
        }
        return new Uint8Array(length)
      },
      readUint16: async () => 0,
      readUint32: async () => 0,
      writeBytes: async () => {},
      advance: async () => {},
      press: async () => {},
    } satisfies SessionRuntime
    const inventory = createInventoryApi(runtime)

    await expect(inventory.contains("metalCoat")).resolves.toBe(true)
    await expect(inventory.contains("tmThunder")).resolves.toBe(true)
    await expect(inventory.contains("ssTicket")).resolves.toBe(true)
    await expect(inventory.contains("pass")).resolves.toBe(false)
    expect(Math.max(...bagReadLengths)).toBeLessThanOrEqual(32)
  })

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
        if (address >= keyItemsAddress && address + length <= keyItemsAddress + keyItems.length)
          return keyItems.slice(address - keyItemsAddress, address - keyItemsAddress + length)
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
