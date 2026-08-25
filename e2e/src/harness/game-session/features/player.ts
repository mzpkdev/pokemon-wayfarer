import { buttons, type Direction } from "../catalog"
import { type SessionRuntime } from "../runtime"

export type PlayerApi = {
  interact: () => Promise<void>
  move: (direction: Direction) => Promise<void>
}

export const createPlayerApi = (runtime: SessionRuntime): PlayerApi => ({
  interact: () => runtime.press("A"),
  move: (direction) => runtime.press(buttons[direction], 3, 1),
})
