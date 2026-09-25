import { type GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"
import { type AppearanceStyle, appearanceStyles, playThroughNewGameIntro } from "./new-game-intro"

export const palletPhases = {
  home: 0,
  labChoice: 2,
  starterReceived: 3,
  battleResolved: 4,
  parcelReceived: 5,
  complete: 9,
} as const

const sceneFinished = (state: GameState): boolean =>
  state.ready && !state.scriptActive && !state.controlsLocked && !state.dialogueOpen

export const advanceKantoScene = async (
  game: GameSession,
  predicate: (state: GameState) => boolean,
  description: string,
): Promise<GameState> => {
  for (let attempt = 0; attempt < 160; attempt++) {
    const state = await game.state.read()
    if (predicate(state)) return state
    if (state.battle.active) throw new Error(`${description}: unexpected battle`)
    await game.controls.press("a")
    await game.wait.frames(45)
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

export const beginPalletOpening = async (
  game: GameSession,
  style: AppearanceStyle,
): Promise<void> => {
  await playThroughNewGameIntro(game, "kanto", style)
  const initial = await game.state.read()
  if (
    initial.map.name !== "reds-house-2f" ||
    initial.appearance.id !== appearanceStyles[style].id ||
    initial.origin.gender !== appearanceStyles[style].gender ||
    initial.origin.pallet.phase !== palletPhases.home
  )
    throw new Error(`Pallet intro handoff: ${JSON.stringify(initial)}`)

  // Use the real stairs and door. These staging positions skip bedroom furniture.
  await game.player.warp("reds-house-2f", 9, 3, "up")
  await game.player.move("up")
  await game.wait.forMap("reds-house-1f")
  await game.player.warp("reds-house-1f", 7, 5, "up")
  await game.player.interact()
  await advanceKantoScene(game, sceneFinished, "Pallet Mom conversation")
  await game.player.warp("reds-house-1f", 9, 7, "down")
  await game.player.move("down")
  await game.wait.forMap("pallet-town")

  await game.player.warp("pallet-town", 12, 2, "up")
  for (let step = 0; step < 3; step++) {
    if ((await game.state.read()).map.name === "oak-lab") break
    await game.player.move("up")
    await game.wait.frames(45)
    await advanceKantoScene(
      game,
      (state) => sceneFinished(state) || state.map.name === "oak-lab",
      "Oak interception",
    )
  }
  await advanceKantoScene(
    game,
    (state) =>
      state.map.name === "oak-lab" &&
      state.origin.pallet.phase >= palletPhases.labChoice &&
      sceneFinished(state),
    "Oak lab staging",
  )
  await game.wait.frames(120)

  // Leaving the lab without choosing is allowed, but Route 1 must remain sealed.
  await game.player.warp("oak-lab", 13, 19, "down")
  await game.player.move("down")
  await game.wait.forMap("pallet-town")
  await game.player.warp("pallet-town", 12, 2, "up")
  await game.player.move("up")
  await advanceKantoScene(
    game,
    (state) =>
      state.map.name === "oak-lab" &&
      state.origin.pallet.phase === palletPhases.labChoice &&
      sceneFinished(state),
    "starterless Route 1 refusal",
  )
  const refused = await game.state.read()
  if (refused.party.length !== 0)
    throw new Error(
      `Starterless Route 1 refusal changed the party: ${JSON.stringify(refused.party)}`,
    )
  await game.wait.frames(60)
}

export const receivePalletStarter = async (game: GameSession): Promise<void> => {
  // Bulbasaur is the leftmost local slot; the selected slot survives challenge substitution.
  await game.player.warp("oak-lab", 10, 19, "up")
  await game.player.interact()
  await advanceKantoScene(
    game,
    (state) => state.origin.pallet.phase >= palletPhases.starterReceived,
    "Kanto starter receipt",
  )
  await advanceKantoScene(game, sceneFinished, "Kanto starter conversation")
}

const finishKantoBattle = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active) return
    await game.wait.frames(24)
    await game.controls.press("a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

export const resolvePalletBlueBattle = async (
  game: GameSession,
  outcome: "win" | "lose",
): Promise<void> => {
  // Approach beside the center aisle so the lateral door path is covered too.
  await game.player.warp("oak-lab", 12, 19, "down")
  await game.player.move("down")
  const battle = await advanceKantoScene(
    game,
    (state) => state.battle.active,
    "first Blue lab battle",
  )
  if (battle.battle.enemy?.species !== "charmander")
    throw new Error(
      `Blue did not choose Bulbasaur's counter: ${JSON.stringify(battle.battle.enemy)}`,
    )
  await game.battle[outcome]()
  await finishKantoBattle(game, "first Blue battle did not finish")
  await advanceKantoScene(
    game,
    (state) => state.origin.pallet.phase >= palletPhases.battleResolved,
    "first Blue battle resolution",
  )
  await advanceKantoScene(game, sceneFinished, "first Blue battle aftermath")
}

const checkRouteOneTrainers = async (game: GameSession): Promise<void> => {
  const encountersDisabled = await game.story.flag("disableEncounters")
  await game.story.setFlag("disableEncounters", true)
  try {
    // The two sight trainers occupy (20,27) and (29,11). Re-entering the
    // map for each approach also verifies that saved opening state restages them.
    for (const approach of [
      { x: 20, y: 29 },
      { x: 29, y: 13 },
    ]) {
      await game.player.warp("route-1", approach.x, approach.y, "up")
      await game.player.move("up")
      await game.wait.frames(90)
      const state = await game.state.read()
      if (state.map.name !== "route-1" || state.player.y !== approach.y - 1 || state.battle.active)
        throw new Error(`Route 1 trainer interrupted Parcel travel: ${JSON.stringify(state)}`)
    }
  } finally {
    await game.story.setFlag("disableEncounters", encountersDisabled)
  }
}

export const checkViridianOpeningBoundary = async (game: GameSession): Promise<void> => {
  const blueBefore = await game.story.flag("viridianBlueIntroduced")
  const gymBefore = await game.story.flag("defeatedViridianGym")
  const badgesBefore = (await game.state.read()).circuit.badges.total

  // The exterior Blue object must be absent and direct interaction inert.
  await game.player.warp("viridian-city", 42, 16, "up")
  await game.player.interact()
  await game.wait.frames(90)
  if ((await game.state.read()).battle.active)
    throw new Error("Viridian exterior Blue started a battle during the Pallet opening")

  await game.player.warp("viridian-city", 34, 12, "up")
  await game.player.interact()
  await game.dialogue.waitForOpen()
  const gramps = await game.state.read()
  if (!gramps.dialogue.text.includes("GYM is closed"))
    throw new Error(`Viridian Gym gramps used later-story dialogue: ${gramps.dialogue.text}`)
  await advanceKantoScene(game, sceneFinished, "Viridian Gym gramps")

  // Both the sign and doorway must describe or enforce the closed Gym.
  await game.player.warp("viridian-city", 45, 15, "up")
  await game.player.interact()
  await game.dialogue.waitForOpen()
  const sign = await game.state.read()
  if (sign.dialogue.text.includes("BLUE"))
    throw new Error(`Viridian Gym sign leaked Blue's future role: ${sign.dialogue.text}`)
  await advanceKantoScene(game, sceneFinished, "Viridian Gym sign")

  await game.player.warp("viridian-city", 43, 15, "up")
  await game.player.move("up")
  await game.wait.until(
    (state) => state.dialogueOpen && state.dialogue.text.includes("GYM is closed"),
    "Viridian Gym opening refusal",
    1_800,
  )
  await advanceKantoScene(
    game,
    (state) => state.map.name === "viridian-city" && sceneFinished(state),
    "Viridian Gym return",
  )

  const after = await game.state.read()
  if (
    after.battle.active ||
    after.circuit.badges.total !== badgesBefore ||
    (await game.story.flag("viridianBlueIntroduced")) !== blueBefore ||
    (await game.story.flag("defeatedViridianGym")) !== gymBefore
  )
    throw new Error(`Viridian Blue/Gym state changed: ${JSON.stringify(after)}`)
}

export const receivePalletParcel = async (game: GameSession): Promise<void> => {
  await game.player.warp("pallet-town", 12, 2, "up")
  await game.player.move("up")
  await game.wait.frames(60)
  const released = await game.state.read()
  if (
    released.map.name !== "pallet-town" ||
    released.player.x !== 12 ||
    released.player.y !== 1 ||
    released.party.length !== 1 ||
    released.origin.pallet.phase < palletPhases.battleResolved
  )
    throw new Error(
      `Received starter did not release the Route 1 guard: ${JSON.stringify(released)}`,
    )
  // Re-stage the short-input harness after the coord script, then exercise
  // the real HNS map connection from its boundary tile.
  await game.player.warp("pallet-town", 12, 0, "up")
  await game.player.move("up")
  await game.wait.forMap("route-1")
  await checkRouteOneTrainers(game)
  await game.player.warp("route-1", 24, 0, "up")
  await game.player.move("up")
  await game.wait.forMap("viridian-city")
  await checkViridianOpeningBoundary(game)
  await game.player.warp("viridian-city", 40, 28, "up")
  await game.player.move("up")
  await game.wait.forMap("viridian-mart")
  await advanceKantoScene(
    game,
    (state) => state.origin.pallet.phase >= palletPhases.parcelReceived,
    "Viridian Mart Parcel receipt",
  )
  await advanceKantoScene(game, sceneFinished, "Viridian Mart Parcel conversation")
}

export const deliverPalletParcel = async (game: GameSession): Promise<void> => {
  await game.player.warp("viridian-city", 26, 49, "down")
  await game.player.move("down")
  await game.wait.forMap("route-1")
  await checkRouteOneTrainers(game)
  await game.player.warp("route-1", 24, 39, "down")
  await game.player.move("down")
  await game.wait.forMap("pallet-town")
  await game.wait.frames(60)
  await game.player.move("down")
  await game.wait.frames(60)
  const returned = await game.state.read()
  if (returned.map.name !== "pallet-town" || returned.player.x !== 12 || returned.player.y !== 1)
    throw new Error(`Route 1 return did not release into Pallet: ${JSON.stringify(returned)}`)
  await game.player.warp("pallet-town", 16, 14, "up")
  await game.player.move("up")
  await game.wait.forMap("oak-lab")
  await game.player.warp("oak-lab", 13, 12, "up")
  await game.player.interact()
  await advanceKantoScene(
    game,
    (state) => state.origin.pallet.phase >= palletPhases.complete,
    "Oak Parcel handoff",
  )
  await advanceKantoScene(game, sceneFinished, "Oak Parcel aftermath")
}
