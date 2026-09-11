import { type GameSession } from "../harness/game-session"

export type StartingOrigin = "johto" | "hoenn"

export const appearanceStyles = {
  1: { id: 1, gender: 0 },
  2: { id: 2, gender: 1 },
  3: { id: 5, gender: 0 },
  4: { id: 6, gender: 1 },
} as const

export type AppearanceStyle = keyof typeof appearanceStyles

export const reachAppearanceQuestion = async (game: GameSession): Promise<void> => {
  await game.wait.frames(3_600)
  await game.controls.press("start")
  await game.wait.frames(240)
  await game.controls.press("a")
  await game.wait.frames(4_800)
  for (let interaction = 0; interaction < 40; interaction++) {
    if ((await game.state.read()).appearance.introStage === 1) return
    await game.controls.press("a")
    await game.wait.frames(60)
  }
  throw new Error(`Appearance list did not appear: ${JSON.stringify(await game.state.read())}`)
}

export const selectAppearance = async (
  game: GameSession,
  style: AppearanceStyle,
): Promise<void> => {
  await game.wait.until((state) => state.appearance.introStage === 1, "appearance picker", 1_800)
  for (let step = 0; step < Object.keys(appearanceStyles).length; step++) {
    if ((await game.state.read()).appearance.candidate === appearanceStyles[style].id) {
      await game.controls.press("a")
      await game.wait.until(
        (state) => state.appearance.confirmed === appearanceStyles[style].id,
        "confirmed appearance",
        600,
      )
      return
    }
    await game.controls.press("down")
    await game.wait.until(
      (state) => state.appearance.introStage === 1,
      "appearance previews settled",
      600,
    )
  }
  throw new Error(`Could not highlight Style ${style}`)
}

export const reachOriginQuestion = async (
  game: GameSession,
  style: AppearanceStyle = 1,
): Promise<void> => {
  await reachAppearanceQuestion(game)
  await selectAppearance(game, style)
  await advanceToOriginQuestion(game)
}

export const advanceToOriginQuestion = async (game: GameSession): Promise<void> => {
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
  style: AppearanceStyle = 1,
): Promise<void> => {
  await reachOriginQuestion(game, style)
  if (origin === "hoenn") await game.controls.press("down")
  await game.controls.press("a")
  await finishOriginIntroduction(game, origin)
  const state = await game.state.read()
  if (state.appearance.id !== appearanceStyles[style].id ||
    state.origin.gender !== appearanceStyles[style].gender)
    throw new Error(`Appearance handoff failed: ${JSON.stringify(state)}`)
}
