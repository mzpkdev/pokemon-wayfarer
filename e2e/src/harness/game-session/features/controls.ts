import { buttons, type Button } from "../catalog"
import { type SessionRuntime } from "../runtime"

export type ControlsApi = {
  press: (button: Button) => Promise<void>
}

export const createControlsApi = (runtime: SessionRuntime): ControlsApi => ({
  press: (button) => runtime.press(buttons[button]),
})
