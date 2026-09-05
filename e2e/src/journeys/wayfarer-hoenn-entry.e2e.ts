import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction } from "../harness/game-session"

const moveOneTile = async (game: GameSession, direction: Direction): Promise<void> => {
  await game.wait.forReady()
  const before = await game.state.read()
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(12)
    const after = await game.state.read()
    if (
      after.map.name !== before.map.name ||
      after.player.x !== before.player.x ||
      after.player.y !== before.player.y
    )
      return
  }
  throw new Error(`Could not move ${direction}: ${JSON.stringify(await game.state.read())}`)
}

const completeDeparture = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 60; attempt++) {
    const state = await game.state.read()
    if (state.map.name === "slateport-city-harbor") {
      await game.wait.forMap("slateport-city-harbor")
      return
    }
    await game.wait.frames(30)
    await game.controls.press("a")
  }
  throw new Error(
    `S.S. Aqua did not reach Slateport Harbor: ${JSON.stringify(await game.state.read())}`,
  )
}

describe.sequential("Wayfarer Hoenn entry", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("sails from Vermilion to Slateport and preserves the Hoenn location on reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "vermilion-port-inside", x: 8, y: 9 },
      },
      story: { vars: { ssAquaState: 8 } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await completeDeparture(game)

    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "slateport-city-harbor" },
      player: { x: 9, y: 11 },
    })

    for (const direction of ["right", "right", "down", "down", "down", "down"] as const)
      await moveOneTile(game, direction)
    await game.wait.forMap("slateport-city")

    await game.saveAndReload()

    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "slateport-city" },
    })
    await expect(game.story.var("ssAquaState")).resolves.toBe(8)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)
  })
})
