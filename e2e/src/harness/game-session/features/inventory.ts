import { items, type Item } from "../catalog"
import { type SessionRuntime } from "../runtime"

const keyItemsPocketId = 5
const keyItemsCapacity = 60
const itemSlotSize = 4
// SkyEmu encodes each requested byte as a query parameter. Keep requests
// comfortably below common HTTP request-line limits.
const maxBagReadBytes = 32

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
  contains: (item: Item) => Promise<boolean>
  rodSlots: () => Promise<Record<StandardRod, number>>
}

type BagPocket = {
  itemSlots: number
  capacity: number
}

const readBagPocket = async (
  runtime: SessionRuntime,
  pocketId: number,
  expectedCapacity?: number,
): Promise<BagPocket> => {
  const pockets = runtime.address("gBagPockets")
  const candidates: BagPocket[] = []

  // GCC may retain the pointer's natural alignment or honor ALIGNED(2).
  // Validate the encoded pocket id and capacity instead of assuming a stride.
  for (const stride of [6, 8]) {
    const bytes = await runtime.readBytes(pockets + pocketId * stride, 6)
    const descriptor = uint16(bytes, 4)
    const capacity = descriptor & 0x03ff
    const id = descriptor >> 10
    const itemSlots = uint32(bytes, 0)
    if (
      id === pocketId &&
      capacity !== 0 &&
      (expectedCapacity === undefined || capacity === expectedCapacity) &&
      itemSlots !== 0
    )
      candidates.push({ itemSlots, capacity })
  }

  const unique = [
    ...new Map(candidates.map((candidate) => [candidate.itemSlots, candidate])).values(),
  ]
  if (unique.length !== 1) {
    throw new Error(`Unable to resolve HNS Bag pocket ${pocketId} from ${unique.length} candidates`)
  }
  return unique[0]!
}

const readKeyItemsPocket = async (runtime: SessionRuntime): Promise<BagPocket> =>
  readBagPocket(runtime, keyItemsPocketId, keyItemsCapacity)

const readPocketSlots = async (runtime: SessionRuntime, pocket: BagPocket): Promise<Uint8Array> => {
  const length = pocket.capacity * itemSlotSize
  const slots = new Uint8Array(length)
  for (let offset = 0; offset < length; offset += maxBagReadBytes) {
    const chunkLength = Math.min(maxBagReadBytes, length - offset)
    slots.set(await runtime.readBytes(pocket.itemSlots + offset, chunkLength), offset)
  }
  return slots
}

export const createInventoryApi = (runtime: SessionRuntime): InventoryApi => ({
  contains: async (name) => {
    const item = items[name]
    // HNS traversal credentials and rewards live in Items, TM/HM, or Key Items.
    for (const pocketId of [3, 4, 5]) {
      const pocket = await readBagPocket(runtime, pocketId)
      const slots = await readPocketSlots(runtime, pocket)
      for (let slot = 0; slot < pocket.capacity; slot++) {
        if (uint16(slots, slot * itemSlotSize) === item) return true
      }
    }
    return false
  },
  rodSlots: async () => {
    const pocket = await readKeyItemsPocket(runtime)
    const slots = await readPocketSlots(runtime, pocket)
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
