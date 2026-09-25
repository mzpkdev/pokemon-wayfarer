import { type Direction } from "../harness/game-session/catalog"
import { type GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"

export const expectFidelityState = (
  state: GameState,
  expected: Partial<{
    map: GameState["map"]["name"]
    x: number
    y: number
    facing: GameState["player"]["facing"]
    phase: number
    partySize: number
  }>,
  context: string,
): void => {
  const observed = {
    map: state.map.name,
    x: state.player.x,
    y: state.player.y,
    facing: state.player.facing,
    phase: state.origin.pallet.phase,
    partySize: state.party.length,
  }
  for (const [key, value] of Object.entries(expected)) {
    if (observed[key as keyof typeof observed] !== value)
      throw new Error(
        `${context}: expected ${key}=${JSON.stringify(value)}, observed ${JSON.stringify(observed)}`,
      )
  }
}

export const expectExactDialogue = async (
  game: GameSession,
  expected: string,
  context: string,
): Promise<number> => {
  await game.dialogue.waitForOpen()
  const state = await game.state.read()
  if (state.dialogue.fullText !== expected)
    throw new Error(
      `${context}: FRLG dialogue mismatch\nexpected: ${JSON.stringify(expected)}\nobserved: ${JSON.stringify(state.dialogue.fullText)}\nraw: ${state.dialogue.fullRawText.join(",")}`,
    )
  return state.dialogue.sequence
}

export const advanceOneExactDialogue = async (
  game: GameSession,
  sequence: number,
  context: string,
): Promise<void> => {
  for (let page = 0; page < 16; page++) {
    const before = await game.state.read()
    if (!before.dialogueOpen || before.dialogue.sequence > sequence) return
    if (before.dialogue.sequence !== sequence)
      throw new Error(`${context}: dialogue sequence moved backwards or skipped`)
    if (before.choice.kind !== "none")
      throw new Error(`${context}: encountered unhandled ${before.choice.kind} choice`)
    await game.controls.press("a")
    await game.wait.frames(2)
  }
  throw new Error(`${context}: dialogue did not finish after 16 observed pages`)
}

export const expectExactSequence = async (
  game: GameSession,
  expected: readonly string[],
  context: string,
): Promise<void> => {
  for (const [index, text] of expected.entries()) {
    const sequence = await expectExactDialogue(game, text, `${context} message ${index + 1}`)
    await advanceOneExactDialogue(game, sequence, `${context} message ${index + 1}`)
  }
}

export const expectYesNo = async (
  game: GameSession,
  displayedSpecies: GameState["presentation"]["displayedMonSpecies"],
  context: string,
): Promise<void> => {
  const state = await game.state.read()
  if (
    state.choice.kind !== "yes-no" ||
    state.choice.cursor !== 0 ||
    state.choice.optionCount !== 2 ||
    state.presentation.displayedMonSpecies !== displayedSpecies
  )
    throw new Error(
      `${context}: expected YES/NO cursor 0 with ${displayedSpecies} displayed, observed ${JSON.stringify({ choice: state.choice, presentation: state.presentation })}`,
    )
}

export const answerYesNo = async (
  game: GameSession,
  answer: "yes" | "no",
  context: string,
): Promise<void> => {
  const before = await game.state.read()
  if (before.choice.kind !== "yes-no" || before.choice.cursor !== 0)
    throw new Error(`${context}: YES/NO was not open at its default selection`)
  if (answer === "no") {
    await game.controls.press("down")
    await game.wait.until(
      (state) => state.choice.kind === "yes-no" && state.choice.cursor === 1,
      `${context} NO selection`,
      120,
    )
  }
  await game.controls.press("a")
  await game.wait.until(
    (state) => state.choice.result === (answer === "yes" ? 1 : 0),
    `${context} ${answer.toUpperCase()} result`,
    120,
  )
}

export const walkExact = async (
  game: GameSession,
  directions: readonly Direction[],
  context: string,
): Promise<void> => {
  for (const [index, direction] of directions.entries()) {
    const before = await game.state.read()
    await game.player.move(direction)
    await game.wait.frames(2)
    const after = await game.state.read()
    const transitioned = after.map.name !== before.map.name
    const moved = after.player.x !== before.player.x || after.player.y !== before.player.y
    if (!transitioned && !moved)
      throw new Error(
        `${context} step ${index + 1} (${direction}) was blocked at ${before.map.name} ${before.player.x}:${before.player.y}`,
      )
    if (after.dialogueOpen || after.battle.active)
      throw new Error(
        `${context} step ${index + 1} unexpectedly opened ${after.battle.active ? "a battle" : "dialogue"}`,
      )
  }
}

export const interactForExactDialogue = async (
  game: GameSession,
  expected: string,
  context: string,
): Promise<void> => {
  await game.player.interact()
  const sequence = await expectExactDialogue(game, expected, context)
  await advanceOneExactDialogue(game, sequence, context)
}

export const visibleObject = (state: GameState, localId: number, context: string) => {
  const object = state.objects.find((candidate) => candidate.localId === localId)
  if (!object?.visible)
    throw new Error(
      `${context}: local object ${localId} was not visible: ${JSON.stringify(state.objects)}`,
    )
  return object
}
