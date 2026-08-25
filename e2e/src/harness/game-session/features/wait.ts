import { type GameMap } from "../catalog"
import { type SessionRuntime } from "../runtime"
import { describeState, type GameState, type StateApi } from "./state"

export type WaitApi = {
  frames: (frames: number) => Promise<void>
  forMap: (map: GameMap, maxFrames?: number) => Promise<void>
  forReady: (maxFrames?: number) => Promise<void>
  until: (
    predicate: (state: GameState) => boolean | Promise<boolean>,
    description: string,
    maxFrames?: number,
  ) => Promise<void>
}

export const createWaitApi = (runtime: SessionRuntime, state: StateApi): WaitApi => {
  const until: WaitApi["until"] = async (predicate, description, maxFrames = 1_200) => {
    for (let elapsed = 0; elapsed <= maxFrames; elapsed += 2) {
      const current = await state.read()
      if (await predicate(current)) return
      await runtime.advance(2)
    }
    throw new Error(
      `${description} not reached in ${maxFrames} frames; ${describeState(await state.read())}`,
    )
  }

  return {
    frames: (frames) => runtime.advance(frames),
    forMap: async (map, maxFrames = 1_200) => {
      await until(
        (current) => current.ready && current.map.name === map,
        `ready map ${map}`,
        maxFrames,
      )
    },
    forReady: (maxFrames = 1_200) =>
      until((current) => current.ready, "ready overworld", maxFrames),
    until,
  }
}
