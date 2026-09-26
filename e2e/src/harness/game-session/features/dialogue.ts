import { type StateApi } from "./state"
import { type WaitApi } from "./wait"

export type DialogueApi = {
  isOpen: () => Promise<boolean>
  text: () => Promise<string>
  waitForOpen: (maxFrames?: number) => Promise<void>
  waitForClosed: (maxFrames?: number) => Promise<void>
  waitForText: (expected: string, maxFrames?: number) => Promise<void>
  waitForNext: (sequence: number, maxFrames?: number) => Promise<void>
  battleText: () => Promise<string>
  waitForBattleText: (expected: string, maxFrames?: number) => Promise<void>
  waitForNextBattleText: (sequence: number, maxFrames?: number) => Promise<void>
}

export const createDialogueApi = (state: StateApi, wait: WaitApi): DialogueApi => ({
  isOpen: async () => (await state.read()).dialogueOpen,
  text: async () => (await state.read()).dialogue.fullText,
  waitForOpen: (maxFrames = 1_200) =>
    wait.until((current) => current.dialogueOpen, "open dialogue", maxFrames),
  waitForClosed: (maxFrames = 1_200) =>
    wait.until((current) => !current.dialogueOpen, "closed dialogue", maxFrames),
  waitForText: (expected, maxFrames = 1_200) =>
    wait.until(
      (current) => current.dialogue.fullText === expected,
      `dialogue text ${JSON.stringify(expected)}`,
      maxFrames,
    ),
  waitForNext: (sequence, maxFrames = 1_200) =>
    wait.until(
      (current) => current.dialogue.sequence > sequence,
      `dialogue after sequence ${sequence}`,
      maxFrames,
    ),
  battleText: async () => (await state.read()).battle.dialogue.text,
  waitForBattleText: (expected, maxFrames = 1_200) =>
    wait.until(
      (current) => current.battle.dialogue.text === expected,
      `battle dialogue text ${JSON.stringify(expected)}`,
      maxFrames,
    ),
  waitForNextBattleText: (sequence, maxFrames = 1_200) =>
    wait.until(
      (current) => current.battle.dialogue.sequence > sequence,
      `battle dialogue after sequence ${sequence}`,
      maxFrames,
    ),
})
