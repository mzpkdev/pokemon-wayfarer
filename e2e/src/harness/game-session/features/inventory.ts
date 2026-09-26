import { hms, items, type Hm, type Item } from "../catalog"
import { type SessionRuntime } from "../runtime"

const keyItemsPocketId = 5
const saveBlock2EncryptionKeyOffset = 0xac
// global.h gives SaveBlock2.frontier.battlePoints an absolute save offset of 0xEB8.
const saveBlock2BattlePointsOffset = 0xeb8
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
export type FillablePocket = "items" | "keyItems" | "tmHm" | "balls"

export type InventoryApi = {
  /**
   * Looks in every bag pocket, including TM/HM. This keeps reward journeys
   * from treating a received HM as a normal Items-pocket reward.
   */
  contains: (item: Item | Hm) => Promise<boolean>
  count: (item: Item | Hm) => Promise<number>
  battlePoints: () => Promise<number>
  /**
   * Test-fixture support for a retry-safe reward: make exactly one native Bag
   * slot available without resetting the save or any event flags.
   */
  freeSlot: (pocket: FillablePocket) => Promise<void>
  rodSlots: () => Promise<Record<StandardRod, number>>
}

const fillablePocketIds = {
  balls: 1,
  items: 3,
  tmHm: 4,
  keyItems: 5,
} as const satisfies Record<FillablePocket, number>

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
    const item = { ...items, ...hms }[name]
    for (const pocketId of [0, 1, 2, 3, 4, 5]) {
      const pocket = await readBagPocket(runtime, pocketId)
      const slots = await readPocketSlots(runtime, pocket)
      for (let slot = 0; slot < pocket.capacity; slot++) {
        if (uint16(slots, slot * itemSlotSize) === item) return true
      }
    }
    return false
  },
  count: async (name) => {
    const item = { ...items, ...hms }[name]
    const saveBlock2 = await runtime.readUint32(runtime.address("gSaveBlock2Ptr"))
    const encryptionKey = await runtime.readUint16(saveBlock2 + saveBlock2EncryptionKeyOffset)
    let count = 0
    for (const pocketId of [0, 1, 2, 3, 4, 5]) {
      const pocket = await readBagPocket(runtime, pocketId)
      const slots = await readPocketSlots(runtime, pocket)
      for (let slot = 0; slot < pocket.capacity; slot++) {
        if (uint16(slots, slot * itemSlotSize) === item)
          count += uint16(slots, slot * itemSlotSize + 2) ^ encryptionKey
      }
    }
    return count
  },
  battlePoints: async () => {
    const saveBlock2 = await runtime.readUint32(runtime.address("gSaveBlock2Ptr"))
    return runtime.readUint16(saveBlock2 + saveBlock2BattlePointsOffset)
  },
  freeSlot: async (name) => {
    const pocket = await readBagPocket(runtime, fillablePocketIds[name])
    const slots = await readPocketSlots(runtime, pocket)
    for (let slot = 0; slot < pocket.capacity; slot++) {
      if (uint16(slots, slot * itemSlotSize) !== 0) {
        await runtime.writeBytes(
          pocket.itemSlots + slot * itemSlotSize,
          new Uint8Array(itemSlotSize),
        )
        return
      }
    }
    throw new Error(`Cannot free a slot in already empty ${name} Bag pocket`)
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
