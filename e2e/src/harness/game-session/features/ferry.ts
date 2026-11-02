import { type SessionRuntime } from "../runtime"

export type FerryApi = {
  /**
   * Pins the two cabin hosts the next boarding draws: each is a row of the host table in
   * game/src/data/wayfarer_ferry_hosts.h (Aqua rows 0-6, Anne rows 7-14, Tidal rows 15-22) with the index of the
   * parameter in the host's list. The pin lives in EWRAM, so it applies to every boarding until `unpin()`.
   */
  pinHosts: (
    first: { host: number; param?: number },
    second: { host: number; param?: number },
  ) => Promise<void>
  unpinHosts: () => Promise<void>
}

export const createFerryApi = (runtime: SessionRuntime): FerryApi => ({
  pinHosts: async (first, second) => {
    await runtime.writeBytes(
      runtime.address("gFerryDebugHosts"),
      new Uint8Array([first.host + 1, second.host + 1, first.param ?? 0, second.param ?? 0]),
    )
  },
  unpinHosts: async () => {
    await runtime.writeBytes(runtime.address("gFerryDebugHosts"), new Uint8Array(4))
  },
})
