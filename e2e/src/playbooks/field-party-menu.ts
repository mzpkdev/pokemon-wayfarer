import { type GameSession } from "../harness/game-session"

export const openFieldPartyMenuActions = async (game: GameSession): Promise<void> => {
  await game.wait.forReady()
  await game.controls.press("start")
  await game.wait.until((state) => state.ui.mode === "pause-menu", "open pause menu")
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => state.partyMenu.open, "open party menu")
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => state.partyMenu.actions.length > 0, "open party field actions")
}

export const selectFieldPartyAction = async (game: GameSession, action: number): Promise<void> => {
  const actions = (await game.state.read()).partyMenu.actions
  const actionIndex = actions.indexOf(action)
  if (actionIndex === -1) throw new Error(`field party action ${action} is unavailable`)

  for (let index = 0; index < actionIndex; index++) await game.controls.press("down")
  await game.controls.press("a")
}
