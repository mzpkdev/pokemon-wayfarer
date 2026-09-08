import { encodeGiftStorageCapacityRequest } from "../protocol"
import { type SessionRuntime } from "../runtime"
import { type MailboxApi } from "../mailbox"
import { type GameState, type StateApi } from "./state"
import { type WaitApi } from "./wait"

export type StorageApi = {
  arrangeCapacity: (capacity: "full" | "one-pc-slot-free") => Promise<void>
  waitForOpen: (maxFrames?: number) => Promise<void>
  waitForReady: (maxFrames?: number) => Promise<void>
  waitForClosed: (maxFrames?: number) => Promise<void>
  slot: (box: number, slot: number) => Promise<GameState["pc"]["slots"][number]>
}

export const createStorageApi = (
  state: StateApi,
  wait: WaitApi,
  runtime: SessionRuntime,
  mailbox: MailboxApi,
): StorageApi => ({
  arrangeCapacity: async (capacity) => {
    await mailbox.execute(
      (requestId) => encodeGiftStorageCapacityRequest(runtime.abi, requestId, capacity === "full"),
      `arrange gift storage capacity ${capacity}`,
    )
  },
  waitForOpen: (maxFrames = 3_600) =>
    wait.until((current) => current.storage.open, "open Pokémon storage", maxFrames),
  waitForReady: (maxFrames = 3_600) =>
    wait.until((current) => current.storage.ui === "ready", "ready Pokémon storage", maxFrames),
  waitForClosed: (maxFrames = 3_600) =>
    wait.until((current) => !current.storage.open, "closed Pokémon storage", maxFrames),
  slot: async (box, slot) => {
    const observed = (await state.read()).pc.slots.find(
      (candidate) => candidate.box === box && candidate.slot === slot,
    )
    if (!observed) throw new Error(`PC slot ${box}:${slot} was not requested by the fixture`)
    return observed
  },
})
