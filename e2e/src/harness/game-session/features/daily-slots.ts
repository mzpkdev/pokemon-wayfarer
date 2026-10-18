import { type SessionRuntime } from "../runtime"

// gDailySlotsDebugFlags bits (include/wayfarer_daily_slots.h).
const pinDay = 1 << 0
const pinSeed = 1 << 1
const noEmptyDays = 1 << 2

const uint32Bytes = (value: number): Uint8Array =>
  new Uint8Array([value & 0xff, (value >>> 8) & 0xff, (value >>> 16) & 0xff, (value >>> 24) & 0xff])

export type DailySlotsApi = {
  /**
   * Pins the day (and optionally the save seed) the daily world slots draw from. `noEmpty` makes
   * every dynamic spot hold a find, so a journey does not have to search for a lucky day. The pin
   * lives in EWRAM, so it applies to every map load until `unpin()` and is lost across a reset.
   * A map load stamps the day into the save, and draws key off that stamp, so a pinned day
   * survives `saveAndReload()` on its own.
   */
  pin: (pins: { day: number; seed?: number; noEmpty?: boolean }) => Promise<void>
  unpin: () => Promise<void>
  /** Writes the last pin again. A reset clears EWRAM, so a reload calls this before Continue loads the map. */
  reapply: () => Promise<void>
  /** What the last managed pickup gave and how many there were since boot. */
  found: () => Promise<{ count: number; item: number }>
  /**
   * Who stands in the current map's rotating trainer slots: each entry's slot row and its occupant's
   * slot row (the same row when the slot's own trainer stands there, `away` when it is empty today).
   */
  trainerOccupants: () => Promise<{ slot: number; occupant: number; away: boolean }[]>
}

const awayOccupant = 0xfffe

export const createDailySlotsApi = (runtime: SessionRuntime): DailySlotsApi => {
  let lastPin: { day: number; seed?: number; noEmpty?: boolean } | undefined
  const api: DailySlotsApi = {
    pin: async (pins) => {
      lastPin = pins
      const { day, seed, noEmpty } = pins
      await runtime.writeBytes(runtime.address("gDailySlotsDebugDay"), uint32Bytes(day))
      if (seed !== undefined)
        await runtime.writeBytes(runtime.address("gDailySlotsDebugSeed"), uint32Bytes(seed))
      await runtime.writeBytes(
        runtime.address("gDailySlotsDebugFlags"),
        new Uint8Array([pinDay | (seed === undefined ? 0 : pinSeed) | (noEmpty ? noEmptyDays : 0)]),
      )
    },
    unpin: async () => {
      lastPin = undefined
      await runtime.writeBytes(runtime.address("gDailySlotsDebugFlags"), new Uint8Array([0]))
    },
    reapply: async () => {
      if (lastPin) await api.pin(lastPin)
    },
    trainerOccupants: async () => {
      const count = (await runtime.readBytes(runtime.address("gDailyTrainersDebugOccupantCount"), 1))[0]!
      if (count === 0) return []
      const bytes = await runtime.readBytes(runtime.address("gDailyTrainersDebugOccupants"), count * 4)
      const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength)
      return Array.from({ length: count }, (_, index) => {
        const occupant = view.getUint16(index * 4 + 2, true)
        return { slot: view.getUint16(index * 4, true), occupant, away: occupant === awayOccupant }
      })
    },
    found: async () => ({
      count: await runtime.readUint16(runtime.address("gDailySlotsDebugFoundCount")),
      item: await runtime.readUint16(runtime.address("gDailySlotsDebugFoundItem")),
    }),
  }
  return api
}
