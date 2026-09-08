import { type GameSession } from "../harness/game-session"

export type StartingOrigin = "johto" | "hoenn"

export const reachOriginQuestion = async (game: GameSession): Promise<void> => {
  await game.wait.frames(3_600)
  await game.controls.press("start")
  await game.wait.frames(240)
  await game.controls.press("a")
  await game.wait.frames(4_800)
  for (let interaction = 0; interaction < 32; interaction++) {
    if ((await game.state.read()).origin.introStage === 1) return
    await game.controls.press("a")
    await game.wait.frames(180)
  }
  for (let tab = 0; tab < 6; tab++) {
    await game.controls.press("r")
    await game.wait.frames(30)
  }
  await game.controls.press("a")
  for (let interaction = 0; interaction < 40; interaction++) {
    await game.wait.frames(180)
    if ((await game.state.read()).origin.introStage === 1) return
    await game.controls.press("a")
  }
  throw new Error(`Origin list did not appear: ${JSON.stringify(await game.state.read())}`)
}

export const waitForOriginStage = async (game: GameSession, stage: number): Promise<void> => {
  await game.wait.until(
    (state) => state.origin.introStage === stage,
    `origin introduction stage ${stage}`,
    1_800,
  )
}

export const finishOriginIntroduction = async (
  game: GameSession,
  origin: StartingOrigin,
): Promise<void> => {
  await waitForOriginStage(game, 2)
  await game.controls.press("a")
  await waitForOriginStage(game, 3)
  const destination = origin === "johto" ? "players-bedroom" : "inside-of-truck"
  for (let interaction = 0; interaction < 60; interaction++) {
    await game.wait.frames(90)
    const state = await game.state.read()
    if (state.map.name === destination && state.ready) {
      if (state.origin.id !== (origin === "johto" ? 1 : 2))
        throw new Error(`Wrong saved origin at ${destination}: ${JSON.stringify(state.origin)}`)
      return
    }
    if (state.origin.introStage === 1 || state.origin.introStage === 2)
      throw new Error("Origin selection reopened during Oak's closing passage")
    await game.controls.press("a")
  }
  throw new Error(
    `Opening did not reach ${destination}: ${JSON.stringify(await game.state.read())}`,
  )
}

export const playThroughNewGameIntro = async (
  game: GameSession,
  origin: StartingOrigin,
): Promise<void> => {
  await reachOriginQuestion(game)
  if (origin === "hoenn") await game.controls.press("down")
  await game.controls.press("a")
  await finishOriginIntroduction(game, origin)
}
