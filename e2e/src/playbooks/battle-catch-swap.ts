import { type GameSession } from "../harness/game-session"

const advanceTextUntil = async (
  game: GameSession,
  predicate: () => Promise<boolean>,
  description: string,
  maxAttempts = 240,
): Promise<void> => {
  for (let attempt = 0; attempt < maxAttempts; attempt++) {
    if (await predicate()) return
    if ((await game.state.read()).battle.ui === "text") await game.controls.press("a")
    else await game.wait.frames(15)
  }
  throw new Error(`${description} was not reached while advancing battle text`)
}

export const catchWithMasterBallAndSwap = async (
  game: GameSession,
  options: { outgoingPartyIndex: number },
): Promise<void> => {
  const enemy = (await game.state.read()).battle.enemy
  if (!enemy || enemy.species === "unknown" || enemy.species === "none")
    throw new Error("A known wild opponent is required before capture")
  const caughtSpecies = enemy.species

  await advanceTextUntil(
    game,
    async () => (await game.state.read()).battle.ui === "action-menu",
    "wild battle action menu",
  )

  // The battle action grid starts on Fight. Bag is one column to the right.
  await game.controls.press("right")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag", "battle Bag", 3_600)
  for (let pocket = 0; pocket < 5; pocket++) {
    if ((await game.state.read()).battle.bag.item === "masterBall") break
    const previousPocket = (await game.state.read()).battle.bag.pocket
    await game.controls.press("right")
    await game.wait.until(
      (state) =>
        state.battle.ui === "bag" &&
        (state.battle.bag.item === "masterBall" || state.battle.bag.pocket !== previousPocket),
      "next battle Bag pocket",
    )
  }
  if ((await game.state.read()).battle.bag.item !== "masterBall")
    throw new Error("Master Ball was not found in the battle Bag")
  await game.controls.press("a")
  await game.wait.until(
    (state) => state.battle.ui === "bag-context",
    "Master Ball context menu",
    1_200,
  )
  await game.controls.press("a")

  await advanceTextUntil(
    game,
    async () => (await game.state.read()).battle.caughtSpecies === caughtSpecies,
    `capture ${caughtSpecies}`,
  )
  await advanceTextUntil(
    game,
    async () => (await game.state.read()).battle.ui === "caught-dex",
    "caught Pokédex page",
  )
  await game.controls.press("a")

  await advanceTextUntil(
    game,
    async () => (await game.state.read()).battle.ui === "nickname",
    "nickname prompt",
  )
  await game.controls.press("b")

  await advanceTextUntil(
    game,
    async () => (await game.state.read()).battle.ui === "catch-swap-prompt",
    "catch-swap prompt",
  )
  await game.controls.press("a")
  await game.wait.until(
    (state) => state.battle.ui === "catch-swap-party",
    "catch-swap party picker",
    3_600,
  )

  const cursor = (await game.state.read()).battle.cursor ?? 0
  for (let index = cursor; index < options.outgoingPartyIndex; index++)
    await game.controls.press("down")
  for (let index = cursor; index > options.outgoingPartyIndex; index--)
    await game.controls.press("up")
  await game.controls.press("a")

  await advanceTextUntil(
    game,
    async () => {
      const state = await game.state.read()
      return !state.battle.active && state.ready
    },
    "return to the overworld after catch-swap",
    360,
  )
}
