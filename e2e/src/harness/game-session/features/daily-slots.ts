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
  /** What the last managed pickup gave and how many there were since boot. */
  found: () => Promise<{ count: number; item: number }>
}

export const createDailySlotsApi = (runtime: SessionRuntime): DailySlotsApi => ({
  pin: async ({ day, seed, noEmpty }) => {
    await runtime.writeBytes(runtime.address("gDailySlotsDebugDay"), uint32Bytes(day))
    if (seed !== undefined)
      await runtime.writeBytes(runtime.address("gDailySlotsDebugSeed"), uint32Bytes(seed))
    await runtime.writeBytes(
      runtime.address("gDailySlotsDebugFlags"),
      new Uint8Array([pinDay | (seed === undefined ? 0 : pinSeed) | (noEmpty ? noEmptyDays : 0)]),
    )
  },
  unpin: async () => {
    await runtime.writeBytes(runtime.address("gDailySlotsDebugFlags"), new Uint8Array([0]))
  },
  found: async () => ({
    count: await runtime.readUint16(runtime.address("gDailySlotsDebugFoundCount")),
    item: await runtime.readUint16(runtime.address("gDailySlotsDebugFoundItem")),
  }),
})
