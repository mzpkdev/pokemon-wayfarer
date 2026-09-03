import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

describe.sequential("Wayfarer Hoenn map loading", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("loads an outdoor and indoor Hoenn map through the normal transition path", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "petalburg-city", x: 10, y: 20 },
      },
      determinism: { textSpeed: "instant" },
    })

    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "petalburg-city" },
    })

    await game.player.move("up")
    await game.wait.forMap("petalburg-house-1")
    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "petalburg-house-1" },
    })

    await game.player.move("down")
    await game.wait.forMap("petalburg-city")
    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "petalburg-city" },
    })
  })
})
