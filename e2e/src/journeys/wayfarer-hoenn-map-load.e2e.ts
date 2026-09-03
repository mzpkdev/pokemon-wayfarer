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

  it("loads representative campaign maps from each Hoenn traversal class", async () => {
    const locations = [
      { map: "littleroot-town", x: 10, y: 12 },
      { map: "granite-cave-1f", x: 36, y: 11 },
      { map: "mt-chimney", x: 18, y: 37 },
      { map: "lilycove-city", x: 24, y: 15 },
      { map: "mossdeep-city", x: 28, y: 17 },
      { map: "underwater-route-124", x: 10, y: 10 },
      { map: "victory-road-1f", x: 15, y: 39 },
      { map: "ever-grande-city", x: 27, y: 49 },
    ] as const

    for (const location of locations) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: location },
        determinism: { textSpeed: "instant" },
      })

      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: location.map },
      })
    }
  })

  it("preserves a Hoenn dungeon location through a real save and reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "granite-cave-1f", x: 36, y: 11 },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "granite-cave-1f" },
      player: { x: 36, y: 11 },
    })
  })
})
