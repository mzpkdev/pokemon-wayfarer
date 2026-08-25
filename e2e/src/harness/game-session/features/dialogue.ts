import { type StateApi } from "./state"
import { type WaitApi } from "./wait"

export type DialogueApi = {
  state: () => Promise<{ open: boolean }>
  waitForOpen: (maxFrames?: number) => Promise<void>
  waitForClosed: (maxFrames?: number) => Promise<void>
}

export const createDialogueApi = (state: StateApi, wait: WaitApi): DialogueApi => ({
  state: async () => ({ open: (await state.read()).dialogueOpen }),
  waitForOpen: (maxFrames = 1_200) =>
    wait.until((current) => current.dialogueOpen, "open dialogue", maxFrames),
  waitForClosed: (maxFrames = 1_200) =>
    wait.until((current) => !current.dialogueOpen, "closed dialogue", maxFrames),
})
