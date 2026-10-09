import { type SessionRuntime } from "../runtime"

// gDailySlotsDebugFlags bits (include/wayfarer_daily_slots.h).
const pinDay = 1 << 0
const pinSeed = 1 << 1

const uint32Bytes = (value: number): Uint8Array =>
  new Uint8Array([value & 0xff, (value >>> 8) & 0xff, (value >>> 16) & 0xff, (value >>> 24) & 0xff])

export type DailySlotsApi = {
  /**
   * Pins the day (and optionally the save seed) the daily world slots draw from.
   * The pin lives in EWRAM, so it applies to every map load until `unpin()` and
   * is lost across a reset. A map load stamps the day into the save, and draws
   * key off that stamp, so a pinned day survives `saveAndReload()` on its own.
   */
  pin: (pins: { day: number; seed?: number }) => Promise<void>
  unpin: () => Promise<void>
}

export const createDailySlotsApi = (runtime: SessionRuntime): DailySlotsApi => ({
  pin: async ({ day, seed }) => {
    await runtime.writeBytes(runtime.address("gDailySlotsDebugDay"), uint32Bytes(day))
    if (seed !== undefined)
      await runtime.writeBytes(runtime.address("gDailySlotsDebugSeed"), uint32Bytes(seed))
    await runtime.writeBytes(
      runtime.address("gDailySlotsDebugFlags"),
      new Uint8Array([pinDay | (seed === undefined ? 0 : pinSeed)]),
    )
  },
  unpin: async () => {
    await runtime.writeBytes(runtime.address("gDailySlotsDebugFlags"), new Uint8Array([0]))
  },
})
