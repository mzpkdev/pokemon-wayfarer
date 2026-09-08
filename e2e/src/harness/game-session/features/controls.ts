import { buttons, type Button } from "../catalog"
import { type SessionRuntime } from "../runtime"

export type ControlsApi = {
  press: (button: Button, timing?: { holdFrames?: number; releaseFrames?: number }) => Promise<void>
}

export const createControlsApi = (runtime: SessionRuntime): ControlsApi => ({
  press: (button, timing) => {
    for (const frames of [timing?.holdFrames, timing?.releaseFrames])
      if (frames !== undefined && (!Number.isInteger(frames) || frames < 0 || frames > 600))
        throw new Error("Controller frame durations must be integers from 0 to 600")
    return runtime.press(buttons[button], timing?.holdFrames, timing?.releaseFrames)
  },
})
