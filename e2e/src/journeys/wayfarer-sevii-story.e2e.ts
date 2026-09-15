import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction, type GameMap } from "../harness/game-session"

type Location = { map: GameMap; x: number; y: number; facing: Direction }

const strongParty = () => [{ species: "lapras", moves: ["surf"], level: 100 }] as const

const advanceUntil = async (
  game: GameSession,
  predicate: (state: Awaited<ReturnType<GameSession["state"]["read"]>>) => boolean,
  description: string,
  maxAttempts = 240,
  advanceActionMenu = false,
): Promise<void> => {
  for (let attempt = 0; attempt < maxAttempts; attempt++) {
    const state = await game.state.read()
    if (predicate(state)) return
    await game.wait.frames(state.dialogueOpen || state.scriptActive ? 24 : 12)
    const nextState = await game.state.read()
    if (advanceActionMenu || !nextState.battle.active || nextState.battle.ui === "text")
      await game.controls.press("a")
  }
  throw new Error(`${description} not reached: ${JSON.stringify(await game.state.read())}`)
}

const finishScript = (
  game: GameSession,
  description: string,
  advanceActionMenu = false,
): Promise<void> =>
  advanceUntil(game, (state) => state.ready && !state.dialogueOpen, description, 240, advanceActionMenu)

const warpTo = async (game: GameSession, location: Location): Promise<void> => {
  await game.player.warp(location.map, location.x, location.y, location.facing)
  await game.wait.forMap(location.map)
}

const startObjectiveBattle = async (
  game: GameSession,
  location: Location,
  description: string,
): Promise<void> => {
  await warpTo(game, location)
  await game.player.interact()
  await advanceUntil(
    game,
    (state) => state.battle.ui === "action-menu",
    `${description} battle action menu`,
  )
}

const winObjectiveBattle = async (
  game: GameSession,
  location: Location,
  description: string,
): Promise<void> => {
  await startObjectiveBattle(game, location, description)
  await game.battle.win()
  await finishScript(game, `${description} victory script`, true)
}

describe.sequential("Wayfarer Sevii independent story journeys", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("keeps Bill's Meteorite pending when Key Items are full, then commits it across reload", async () => {
    const bill = {
      map: "sevii-one-island-pokemon-center-1-f",
      x: 14,
      y: 7,
      facing: "up",
    } as const

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: bill, facing: bill.facing },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      bag: { fullPockets: ["keyItems"] },
      determinism: { textSpeed: "instant" },
    })
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(false)
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await game.controls.press("a")
    await finishScript(game, "Bill full-pocket response")
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(false)
    await expect(game.inventory.contains("meteorite")).resolves.toBe(false)

    // Celio's request is independently usable while Bill's transaction is pending.
    await warpTo(game, {
      map: "sevii-one-island-pokemon-center-1-f",
      x: 15,
      y: 7,
      facing: "up",
    })
    await game.player.interact()
    await finishScript(game, "Celio investigation acceptance")
    await expect(game.story.flag("seviiCelioGemsStarted")).resolves.toBe(true)
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(false)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: bill, facing: bill.facing },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await game.controls.press("a")
    await finishScript(game, "Bill successful Meteorite handoff")
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(true)
    await expect(game.inventory.contains("meteorite")).resolves.toBe(true)

    await game.saveAndReload()
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(true)
    await expect(game.inventory.contains("meteorite")).resolves.toBe(true)
  })

  it("starts Lostelle locally and preserves ordered battles through loss, blackout, and reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        position: { map: "sevii-two-island-joyful-game-corner", x: 5, y: 6 },
        facing: "up",
      },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await finishScript(game, "Lostelle's local introduction")
    await expect(game.story.flag("seviiLostelleStarted")).resolves.toBe(true)
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(false)

    const bikers = [
      { map: "sevii-three-island", x: 9, y: 25, facing: "up" },
      { map: "sevii-three-island", x: 11, y: 24, facing: "left" },
      { map: "sevii-three-island", x: 9, y: 22, facing: "down" },
      { map: "sevii-three-island", x: 8, y: 25, facing: "up" },
    ] as const satisfies readonly Location[]

    await warpTo(game, bikers[1])
    await game.player.interact()
    await advanceUntil(
      game,
      (state) => state.dialogue.text.includes("Another biker"),
      "out-of-order biker refusal",
    )
    expect((await game.state.read()).battle.active).toBe(false)
    await finishScript(game, "out-of-order biker refusal")

    await startObjectiveBattle(game, bikers[0], "first biker")
    await game.battle.lose()
    await advanceUntil(
      game,
      (state) => state.ready && !state.battle.active,
      "first biker blackout recovery",
      240,
      true,
    )
    await expect(game.story.flag("seviiBikersCleared")).resolves.toBe(false)
    await game.saveAndReload()

    for (const [index, location] of bikers.entries())
      await winObjectiveBattle(game, location, `ordered biker ${index + 1}`)

    await expect(game.story.flag("seviiBikersCleared")).resolves.toBe(true)
    await game.saveAndReload()
    await expect(game.story.flag("seviiBikersCleared")).resolves.toBe(true)

    const lostelle = {
      map: "sevii-three-island-berry-forest",
      x: 4,
      y: 9,
      facing: "up",
    } as const
    await startObjectiveBattle(game, lostelle, "Lostelle's Hypno")
    expect((await game.state.read()).battle.enemy).toMatchObject({ species: "hypno", level: 30 })
    await game.battle.lose()
    await advanceUntil(
      game,
      (state) => state.ready && !state.battle.active,
      "Hypno blackout recovery",
      240,
      true,
    )
    await expect(game.story.flag("seviiLostelleFound")).resolves.toBe(false)
    await expect(game.story.flag("seviiLostelleRescued")).resolves.toBe(false)
    await game.saveAndReload()

    await winObjectiveBattle(game, lostelle, "retried Hypno")
    await expect(game.story.flag("seviiLostelleFound")).resolves.toBe(true)
    await expect(game.story.flag("seviiLostelleRescued")).resolves.toBe(true)
    await expect(game.story.flag("seviiIapapaBerryReceived")).resolves.toBe(true)
  })

  it("gates Moltres at TR 54 and resolves only a TR 55 knockout across reload", async () => {
    const moltres = {
      map: "sevii-mt-ember-summit",
      x: 9,
      y: 7,
      facing: "up",
    } as const
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: moltres, facing: moltres.facing },
      story: { vars: { trainerRating: 54 } },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      determinism: { textSpeed: "instant" },
    })
    expect((await game.state.read()).circuit.trainerRating).toBe(54)
    await game.player.interact()
    await advanceUntil(
      game,
      (state) => state.dialogue.text.includes("radiates overwhelming"),
      "Moltres TR 54 refusal",
    )
    expect((await game.state.read()).battle.active).toBe(false)
    await finishScript(game, "Moltres TR 54 refusal")
    await expect(game.story.flag("seviiMoltresResolved")).resolves.toBe(false)

    await game.story.setVar("trainerRating", 55)
    await startObjectiveBattle(game, moltres, "Moltres at TR 55")
    expect((await game.state.read()).battle.enemy).toMatchObject({ species: "moltres", level: 50 })
    await game.battle.lose()
    await advanceUntil(
      game,
      (state) => state.ready && !state.battle.active,
      "Moltres blackout recovery",
      240,
      true,
    )
    await expect(game.story.flag("seviiMoltresResolved")).resolves.toBe(false)
    await game.saveAndReload()

    await startObjectiveBattle(game, moltres, "retried Moltres")
    await game.battle.win()
    await finishScript(game, "Moltres knockout resolution", true)
    await expect(game.story.flag("seviiMoltresResolved")).resolves.toBe(true)
    await game.saveAndReload()
    await expect(game.story.flag("seviiMoltresResolved")).resolves.toBe(true)
  })
})
