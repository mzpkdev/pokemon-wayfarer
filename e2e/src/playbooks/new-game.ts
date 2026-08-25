import { type GameSession } from "../harness/game-session"

export const startNewGame = async (game: GameSession): Promise<void> => {
  await game.wait.frames(3_600)
  await game.controls.press("start")
  await game.wait.frames(240)
  await game.controls.press("a")

  await game.wait.frames(4_800)
  for (let interaction = 0; interaction < 32; interaction++) {
    await game.controls.press("a")
    await game.wait.frames(180)
  }
  for (let tab = 0; tab < 6; tab++) {
    await game.controls.press("r")
    await game.wait.frames(30)
  }
  await game.controls.press("a")
  await game.wait.frames(1_200)
  for (let interaction = 0; interaction < 24; interaction++) {
    await game.controls.press("a")
    await game.wait.frames(300)
  }
}
