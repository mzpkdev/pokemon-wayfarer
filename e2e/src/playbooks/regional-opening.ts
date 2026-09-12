import { type Direction, type GameMap, type GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"

export const stepOpening = async (game: GameSession, direction: Direction): Promise<void> => {
  const before = await game.state.read()
  for (let attempt = 0; attempt < 5; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(24)
    const after = await game.state.read()
    if (
      after.map.name !== before.map.name ||
      after.player.x !== before.player.x ||
      after.player.y !== before.player.y
    )
      return
  }
  throw new Error(`Could not step ${direction}: ${JSON.stringify(await game.state.read())}`)
}

export const enterOpeningMap = async (
  game: GameSession,
  direction: Direction,
  destination: GameMap,
): Promise<void> => {
  for (let attempt = 0; attempt < 40; attempt++) {
    const state = await game.state.read()
    if (state.map.name === destination && state.phase !== "boot") return
    if (state.map.name !== destination) await game.controls.press(direction)
    await game.wait.frames(30)
  }
  throw new Error(`Could not enter ${destination}: ${JSON.stringify(await game.state.read())}`)
}

export const advanceOpeningUntil = async (
  game: GameSession,
  predicate: (state: GameState) => boolean,
  description: string,
  button: "a" | "b" = "a",
): Promise<void> => {
  for (let attempt = 0; attempt < 100; attempt++) {
    await game.wait.frames(45)
    const state = await game.state.read()
    if (predicate(state)) return
    if (state.phase === "boot" && state.dialogue.text.includes("clock is stopped"))
      await game.controls.press("up")
    await game.controls.press(button)
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

export const finishOpeningDialogue = async (game: GameSession): Promise<void> => {
  await advanceOpeningUntil(game, (state) => state.ready, "opening dialogue did not finish")
}

const finishNativeHoennScene = async (
  game: GameSession,
  townState: number,
  description: string,
): Promise<void> => {
  await game.wait.frames(90)
  // The house and rival scenes contain a long, paged native dialogue. The
  // E2E ready bit can briefly report true between pages, so exhaust the
  // dialogue until the actual Hoenn state transition commits.
  for (let attempt = 0; attempt < 100; attempt++) {
    const state = await game.state.read()
    if (state.origin.littlerootTownState >= townState) {
      await game.wait.frames(90)
      const settled = await game.state.read()
      if (settled.ready && !settled.controlsLocked && !settled.scriptActive) return
    }
    await game.controls.press("a")
    await game.wait.frames(45)
  }
  const state = await game.state.read()
  if (
    state.ready &&
    !state.controlsLocked &&
    !state.scriptActive &&
    state.origin.littlerootTownState >= townState
  )
    return
  throw new Error(`${description}: ${JSON.stringify(state)}`)
}

const finishNativeFieldScene = async (game: GameSession, description: string): Promise<void> => {
  let sceneObserved = false
  for (let attempt = 0; attempt < 100; attempt++) {
    const state = await game.state.read()
    sceneObserved ||=
      !state.ready || state.controlsLocked || state.scriptActive || state.dialogueOpen
    if (sceneObserved && state.ready && !state.controlsLocked && !state.scriptActive) return
    await game.controls.press("a")
    await game.wait.frames(45)
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

export const receiveElmStarter = async (game: GameSession, slot: 0 | 1 | 2): Promise<void> => {
  await game.player.warp("elm-lab", 6, 8, "up")
  await stepOpening(game, "up")
  await finishOpeningDialogue(game)
  await game.player.warp("elm-lab", 8 + slot, 5, "up")
  await game.player.interact()
  await advanceOpeningUntil(
    game,
    (state) => state.origin.johtoReceived,
    "Elm did not grant starter",
  )
  await advanceOpeningUntil(game, (state) => state.ready, "Elm nickname did not close", "b")
}

export const receiveBirchStarter = async (
  game: GameSession,
  slot: 0 | 1 | 2,
  outcome: "win" | "lose" = "win",
  approach: Direction | null = "right",
): Promise<void> => {
  if (approach !== null) await stepOpening(game, approach)
  await finishOpeningDialogue(game)
  await game.player.warp("route-101", 7, 15, "up")
  await game.player.interact()
  await game.wait.until((state) => state.origin.starterChooseStage === 1, "Birch's bag selection")
  if (slot === 0) await game.controls.press("left")
  if (slot === 2) await game.controls.press("right")
  await game.controls.press("a")
  await game.wait.until((state) => state.origin.starterChooseStage === 2, "starter confirmation")
  await game.controls.press("a")
  await advanceOpeningUntil(
    game,
    (state) => state.battle.active,
    "native first battle did not start",
  )
  await game.battle[outcome]()
  await advanceOpeningUntil(
    game,
    (state) => state.map.name === "birch-lab" && state.dialogue.text.includes("While"),
    "Birch did not acknowledge the rescued partner",
  )
  await advanceOpeningUntil(
    game,
    (state) => state.dialogue.text.includes("If you work"),
    "Birch's nickname choice did not close",
    "b",
  )
  await finishOpeningDialogue(game)
}

export const walkOpeningTo = async (game: GameSession, x: number, y: number): Promise<void> => {
  for (let attempt = 0; attempt < 100; attempt++) {
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y) return
    if (!state.ready) {
      await finishOpeningDialogue(game)
      continue
    }
    const direction =
      state.player.x < x
        ? "right"
        : state.player.x > x
          ? "left"
          : state.player.y < y
            ? "down"
            : "up"
    await stepOpening(game, direction)
    await game.wait.frames(24)
  }
  throw new Error(`Could not walk to ${x},${y}: ${JSON.stringify(await game.state.read())}`)
}

export const playHoennHousehold = async (game: GameSession): Promise<void> => {
  const female = (await game.state.read()).origin.gender === 1
  const home1f = female ? "mays-house-1f" : "brendans-house-1f"
  const home2f = female ? "mays-house-2f" : "brendans-house-2f"
  for (let attempt = 0; attempt < 100; attempt++) {
    const state = await game.state.read()
    if (state.map.name === home1f) break
    if (state.map.name === "inside-of-truck") await game.player.move("right")
    else await game.controls.press("a")
    await game.wait.frames(60)
  }
  await finishOpeningDialogue(game)
  if ((await game.state.read()).map.name !== home1f)
    throw new Error("Truck arrival did not enter home")
  await walkOpeningTo(game, female ? 2 : 8, 3)
  await enterOpeningMap(game, "up", home2f)
  await game.wait.forMap(home2f)
  await walkOpeningTo(game, female ? 3 : 5, 2)
  await game.controls.press("up")
  await game.player.interact()
  await finishOpeningDialogue(game)
  await game.saveAndReload()
  await walkOpeningTo(game, female ? 1 : 7, 2)
  await enterOpeningMap(game, "up", home1f)
  await game.wait.frames(90)
  await finishOpeningDialogue(game)
  await walkOpeningTo(game, female ? 2 : 8, 7)
  await enterOpeningMap(game, "down", "littleroot-town")
  await game.wait.forMap("littleroot-town")
  await walkOpeningTo(game, female ? 5 : 14, 9)
  await enterOpeningMap(game, "up", female ? "brendans-house-1f" : "mays-house-1f")
  await game.wait.frames(90)
  await finishOpeningDialogue(game)
  await walkOpeningTo(game, female ? 8 : 2, 3)
  await enterOpeningMap(game, "up", female ? "brendans-house-2f" : "mays-house-2f")
  await game.wait.forMap(female ? "brendans-house-2f" : "mays-house-2f")
  // Both rival bedrooms put the optional Poké Ball directly between the
  // stairs and the rival. Walk around it instead of treating the blocked
  // tile as a movement failure.
  await walkOpeningTo(game, 4, 3)
  await walkOpeningTo(game, 4, 5)
  await walkOpeningTo(game, female ? 3 : 5, 5)
  await game.controls.press("up")
  await game.player.interact()
  await finishNativeHoennScene(game, 1, "native rival scene did not finish")
  await walkOpeningTo(game, female ? 7 : 1, 2)
  await enterOpeningMap(game, "up", female ? "brendans-house-1f" : "mays-house-1f")
  await game.wait.forMap(female ? "brendans-house-1f" : "mays-house-1f")
  await walkOpeningTo(game, female ? 8 : 2, 7)
  await enterOpeningMap(game, "down", "littleroot-town")
  await game.wait.forMap("littleroot-town")
  // The native first-contact trigger is at the Route 101 entrance's east
  // tile. The adjacent west tile is blocked, so approach from 11,2.
  await walkOpeningTo(game, 11, 2)
  await stepOpening(game, "up")
  await finishNativeHoennScene(game, 2, "Birch rescue scene did not begin")
  await enterOpeningMap(game, "up", "route-101")
  await finishNativeFieldScene(game, "Birch rescue scene did not finish")
}
