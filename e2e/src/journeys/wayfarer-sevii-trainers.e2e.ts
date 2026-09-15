import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

const tommyPosition = {
  map: "sevii-one-island-kindle-road",
  x: 14,
  y: 25,
} as const

const arrangeAtTommy = async (
  game: GameSession,
  party: Parameters<GameSession["arrange"]>[0]["party"],
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "right", position: tommyPosition },
    party,
    determinism: { rngSeed: 1, textSpeed: "instant" },
  })
}

const waitForTommyBattle = async (game: GameSession, description: string): Promise<void> => {
  await game.player.interact()
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not reach its action menu: ${JSON.stringify(await game.state.read())}`,
  )
}

const waitForDialogueText = async (
  game: GameSession,
  expectedText: string,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.dialogueOpen && state.dialogue.text.includes(expectedText)) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(`${description} was not shown: ${JSON.stringify(await game.state.read())}`)
}

const finishBlackout = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 4_000; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active && state.map.name !== tommyPosition.map) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive) {
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `Tommy loss did not finish its normal blackout: ${JSON.stringify(await game.state.read())}`,
  )
}

describe.sequential("Wayfarer Sevii ordinary Trainers", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("keeps Fisherman Tommy defeated and talkable after saving and reloading", async () => {
    await arrangeAtTommy(game, [{ species: "lapras", level: 100, moves: ["surf"] }])

    await waitForTommyBattle(game, "Fisherman Tommy talk encounter")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: tommyPosition.map },
      battle: { active: true, enemy: { species: "goldeen" } },
    })
    await game.battle.win()
    await waitForDialogueText(game, "Not only did I lose", "Tommy post-battle dialogue")
    await game.controls.press("a")
    await game.wait.forReady()

    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: tommyPosition.map },
      battle: { active: false },
    })

    await game.player.interact()
    await waitForDialogueText(game, "Not only did I lose", "reloaded Tommy post-battle dialogue")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
  })

  it("uses a normal blackout and leaves Tommy available after a loss", async () => {
    await arrangeAtTommy(game, [{ species: "rattata", level: 1, moves: ["tackle"] }])

    await waitForTommyBattle(game, "first Fisherman Tommy attempt")
    await game.battle.lose()
    await finishBlackout(game)

    const recovered = await game.state.read()
    expect(recovered.map.name).not.toBe(tommyPosition.map)
    expect(recovered.party.every((mon) => !mon.fainted)).toBe(true)

    await game.player.warp(tommyPosition.map, tommyPosition.x, tommyPosition.y, "right")
    await waitForTommyBattle(game, "Fisherman Tommy retry after blackout")
    await expect(game.state.read()).resolves.toMatchObject({
      battle: { active: true, enemy: { species: "goldeen" } },
    })
  })
})
