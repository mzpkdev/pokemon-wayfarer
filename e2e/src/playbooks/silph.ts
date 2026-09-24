import { expect } from "webanvil/test"
import { GameSession, type Direction, type GameMap } from "../harness/game-session"
import { silphDoors } from "./silph-fixtures"

export const silphFloorFlags = (floor: GameMap) =>
  Object.fromEntries(silphDoors.filter((door) => door.floor === floor).map((door) => [door.flag, true]))

export const arrangeSilph = async (
  game: GameSession,
  options: Partial<Parameters<GameSession["arrange"]>[0]> = {},
): Promise<void> => game.arrange({
  checkpoint: "new-bark-after-intro",
  player: { position: { map: "silph-2f", x: 30, y: 3 }, facing: "up" },
  party: [{ species: "lapras", level: 100 }],
  determinism: { textSpeed: "instant" },
  ...options,
})

export const settleSilph = async (game: GameSession, cancel = false): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active) return
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || state.battle.ui === "text")
      await game.controls.press(cancel ? "b" : "a")
    else await game.wait.frames(12)
  }
  throw new Error(`Silph interaction did not settle: ${JSON.stringify(await game.state.read())}`)
}

export const talkSilph = async (game: GameSession, cancel = false): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await settleSilph(game, cancel)
}

export const moveSilph = async (game: GameSession, direction: Direction, x: number, y: number): Promise<void> => {
  for (let attempt = 0; attempt < 8; attempt++) {
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y) return
    await game.controls.press(direction)
    await game.wait.frames(20)
  }
  expect((await game.state.read()).player).toMatchObject({ x, y })
}

export const waitSilphBattle = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    if ((await game.state.read()).battle.ui === "action-menu") return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`Silph battle did not start: ${JSON.stringify(await game.state.read())}`)
}

export const triggerSilphGiovanni = async (game: GameSession, waitForDialogue = true): Promise<void> => {
  await game.controls.press("up")
  await game.wait.frames(20)
  if (!(await game.state.read()).scriptActive) await game.controls.press("up")
  if (waitForDialogue) await game.dialogue.waitForOpen()
}

export const enterSilphMap = async (game: GameSession, direction: Direction, destination: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 12; attempt++) {
    const state = await game.state.read()
    if (state.map.name === destination) break
    if (state.ready) await game.controls.press(direction)
    await game.wait.frames(30)
  }
  await game.wait.forMap(destination)
}
