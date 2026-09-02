import { items } from "../catalog"
import { type SessionRuntime } from "../runtime"

const keyItemsPocketId = 5
const keyItemsCapacity = 60
const itemSlotSize = 4

const uint16 = (bytes: Uint8Array, offset: number): number =>
  bytes[offset]! | (bytes[offset + 1]! << 8)

const uint32 = (bytes: Uint8Array, offset: number): number =>
  (bytes[offset]! |
    (bytes[offset + 1]! << 8) |
    (bytes[offset + 2]! << 16) |
    (bytes[offset + 3]! << 24)) >>>
  0

export type StandardRod = "oldRod" | "goodRod" | "superRod"

export type InventoryApi = {
  rodSlots: () => Promise<Record<StandardRod, number>>
}

type BagPocket = {
  itemSlots: number
  capacity: number
}

const readKeyItemsPocket = async (runtime: SessionRuntime): Promise<BagPocket> => {
  const pockets = runtime.address("gBagPockets")
  const candidates: BagPocket[] = []

  // GCC may retain the pointer's natural alignment or honor ALIGNED(2).
  // Validate the encoded pocket id and capacity instead of assuming a stride.
  for (const stride of [6, 8]) {
    const bytes = await runtime.readBytes(pockets + keyItemsPocketId * stride, 6)
    const descriptor = uint16(bytes, 4)
    const capacity = descriptor & 0x03ff
    const id = descriptor >> 10
    const itemSlots = uint32(bytes, 0)
    if (id === keyItemsPocketId && capacity === keyItemsCapacity && itemSlots !== 0)
      candidates.push({ itemSlots, capacity })
  }

  const unique = [
    ...new Map(candidates.map((candidate) => [candidate.itemSlots, candidate])).values(),
  ]
  if (unique.length !== 1) {
    throw new Error(`Unable to resolve HNS Key Items pocket from ${unique.length} candidates`)
  }
  return unique[0]!
}

export const createInventoryApi = (runtime: SessionRuntime): InventoryApi => ({
  rodSlots: async () => {
    const pocket = await readKeyItemsPocket(runtime)
    const slots = await runtime.readBytes(pocket.itemSlots, pocket.capacity * itemSlotSize)
    const rods: Record<StandardRod, number> = { oldRod: 0, goodRod: 0, superRod: 0 }

    for (let slot = 0; slot < pocket.capacity; slot++) {
      const item = uint16(slots, slot * itemSlotSize)
      if (item === items.oldRod) rods.oldRod++
      else if (item === items.goodRod) rods.goodRod++
      else if (item === items.superRod) rods.superRod++
    }
    return rods
  },
})
