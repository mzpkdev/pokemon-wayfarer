import { type GameSession } from "../harness/game-session"

export type StorageMode = "move" | "deposit" | "withdraw"

const modeIndex: Record<StorageMode, number> = { move: 0, deposit: 1, withdraw: 2 }
const boxColumns = 6

export const openPcStorage = async (game: GameSession, mode: StorageMode): Promise<void> => {
  await game.wait.forReady()
  let sequence = (await game.state.read()).dialogue.sequence
  await game.player.interact()
  await game.wait.until((state) => state.dialogue.sequence > sequence, "PC boot message")
  sequence = (await game.state.read()).dialogue.sequence
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => state.scriptActive && !state.dialogueOpen, "outer PC menu")
  await game.wait.frames(12)
  await game.controls.press("a")
  await game.wait.until((state) => state.dialogue.sequence > sequence, "Bill's PC message")
  sequence = (await game.state.read()).dialogue.sequence
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => state.dialogue.sequence > sequence, "storage opened message")
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "pc-menu", "storage mode menu")
  const pcMenuState = await game.state.read()
  if (pcMenuState.storage.ui !== "pc-menu")
    throw new Error(
      `Real PC script did not reach the storage mode menu; storage=${pcMenuState.storage.ui}, scriptActive=${pcMenuState.scriptActive}, dialogueOpen=${pcMenuState.dialogueOpen}, dialogueSequence=${pcMenuState.dialogue.sequence}`,
    )
  const selected = (await game.state.read()).storage.cursor.position ?? 0
  for (let index = selected; index < modeIndex[mode]; index++) await game.controls.press("down")
  for (let index = selected; index > modeIndex[mode]; index--) await game.controls.press("up")
  await game.controls.press("a")
  await game.storage.waitForReady()
}

const moveBoxCursor = async (game: GameSession, target: number): Promise<void> => {
  const state = await game.state.read()
  const start = state.storage.cursor.position
  if (start === null) throw new Error("Storage cursor position is unavailable")
  let row = Math.floor(start / boxColumns)
  let column = start % boxColumns
  const targetRow = Math.floor(target / boxColumns)
  const targetColumn = target % boxColumns
  while (row < targetRow) {
    await game.controls.press("down")
    row++
  }
  while (row > targetRow) {
    await game.controls.press("up")
    row--
  }
  while (column < targetColumn) {
    await game.controls.press("right")
    column++
  }
  while (column > targetColumn) {
    await game.controls.press("left")
    column--
  }
  await game.wait.until(
    (current) => current.storage.ready && current.storage.cursor.position === target,
    `storage cursor at slot ${target}`,
  )
}

export const depositPartyMon = async (game: GameSession, partyIndex: number): Promise<void> => {
  await game.storage.waitForReady()
  const cursor = (await game.state.read()).storage.cursor.position
  if (cursor === null) throw new Error("Storage party cursor position is unavailable")
  for (let index = cursor; index < partyIndex; index++) await game.controls.press("down")
  for (let index = cursor; index > partyIndex; index--) await game.controls.press("up")
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "mon-menu", "deposit Pokémon menu")
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "deposit-box", "deposit box picker")
  await game.controls.press("a")
  await game.storage.waitForReady()
}

export const withdrawSlot = async (game: GameSession, slot: number): Promise<void> => {
  await game.storage.waitForReady()
  await moveBoxCursor(game, slot)
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "mon-menu", "withdraw Pokémon menu")
  await game.controls.press("a")
  await game.wait.until(
    (state) =>
      state.storage.ui === "ready" &&
      state.pc.slots.some(
        (observed) =>
          observed.box === state.pc.currentBox && observed.slot === slot && observed.mon === null,
      ),
    "withdraw Pokémon into the party",
    3_600,
  )
}

export const moveSlot = async (game: GameSession, from: number, to: number): Promise<void> => {
  await game.storage.waitForReady()
  await moveBoxCursor(game, from)
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "mon-menu", "move Pokémon menu")
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until(
    (state) => state.storage.movingMon && state.storage.ready,
    "pick up boxed Pokémon",
  )
  await moveBoxCursor(game, to)
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "mon-menu", "place Pokémon menu")
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => {
    const source = state.pc.slots.find(
      (observed) => observed.box === state.pc.currentBox && observed.slot === from,
    )
    const destination = state.pc.slots.find(
      (observed) => observed.box === state.pc.currentBox && observed.slot === to,
    )
    return source?.mon === null && destination?.mon !== null
  }, "place boxed Pokémon")
  if ((await game.state.read()).storage.ui === "mon-menu") await game.controls.press("b")
  await game.storage.waitForReady()
}

export const releaseSlot = async (game: GameSession, slot: number): Promise<void> => {
  await game.storage.waitForReady()
  await moveBoxCursor(game, slot)
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "mon-menu", "release Pokémon menu")
  await game.wait.frames(60)
  for (let index = 0; index < 4; index++) await game.controls.press("down")
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "release-confirm", "release confirmation")
  await game.controls.press("up")
  await game.controls.press("a")
  await game.wait.until((state) => state.storage.ui === "released", "successful release", 3_600)
  await game.controls.press("a")
  await game.wait.frames(30)
  await game.controls.press("a")
  await game.storage.waitForReady()
}

export const switchStorageMode = async (game: GameSession, to: StorageMode): Promise<void> => {
  await game.controls.press("b")
  await game.wait.frames(20)
  await game.controls.press("b")
  await game.wait.until((state) => state.storage.ui === "pc-menu", "return to storage mode menu")
  const selected = (await game.state.read()).storage.cursor.position
  if (selected === null) throw new Error("Storage mode cursor position is unavailable")
  const delta = modeIndex[to] - selected
  const direction = delta > 0 ? "down" : "up"
  for (let index = 0; index < Math.abs(delta); index++) await game.controls.press(direction)
  await game.controls.press("a")
  await game.storage.waitForReady()
}

export const closePcStorage = async (game: GameSession): Promise<void> => {
  await game.controls.press("b")
  await game.wait.frames(20)
  await game.controls.press("b")
  await game.wait.until((state) => state.storage.ui === "pc-menu", "return to storage mode menu")
  await game.controls.press("b")
  await game.storage.waitForClosed()
  await game.wait.frames(120)
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.controls.press("b")
    await game.wait.frames(60)
  }
  await game.wait.forReady(3_600)
}

export const walkFromCherrygrovePcToSurfShore = async (game: GameSession): Promise<void> => {
  const step = async (direction: "down" | "left"): Promise<void> => {
    const before = await game.state.read()
    for (let attempt = 0; attempt < 3; attempt++) {
      await game.player.move(direction)
      await game.wait.frames(12)
      const state = await game.state.read()
      if (
        state.map.mapGroup !== before.map.mapGroup ||
        state.map.mapNum !== before.map.mapNum ||
        state.player.x !== before.player.x ||
        state.player.y !== before.player.y
      ) {
        await game.wait.frames(8)
        await game.wait.forReady()
        return
      }
    }
    throw new Error(
      `Could not walk ${direction} from ${before.map.name} ${before.player.x}:${before.player.y}`,
    )
  }

  const centerExit = [
    "down",
    "down",
    "down",
    "left",
    "left",
    "left",
    "left",
    "down",
    "down",
    "down",
  ] as const
  for (const direction of centerExit) await step(direction)
  await game.player.move("down")
  await game.wait.until(
    (state) =>
      state.map.name === "cherrygrove-city" &&
      state.player.x === 47 &&
      state.player.y === 8 &&
      state.ready,
    "exit Cherrygrove Pokémon Center",
  )
  await step("down")
  for (let index = 0; index < 21; index++) await step("left")
  await step("down")
  await step("left")
}
