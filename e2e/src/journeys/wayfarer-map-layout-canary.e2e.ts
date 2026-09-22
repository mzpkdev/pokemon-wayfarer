import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

const canaries = [
  {
    map: "fortree-house-1",
    position: { x: 3, y: 4 },
    destination: "fortree-city",
  },
  {
    map: "olivine-cafe",
    position: { x: 3, y: 7 },
    destination: "olivine-city",
  },
  {
    map: "sevii-five-island-resort-gorgeous-house",
    position: { x: 4, y: 6 },
    destination: "sevii-five-island-resort-gorgeous",
  },
] as const

describe.sequential("Wayfarer map-layout canaries", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  for (const canary of canaries) {
    it(`loads and exits the ${canary.map} canary through a real warp`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: canary.map, ...canary.position } },
        determinism: { textSpeed: "instant" },
      })

      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: canary.map },
        player: canary.position,
      })
      for (let attempt = 0; attempt < 4; attempt++) {
        await game.player.move("down")
        await game.wait.frames(15)
        if ((await game.state.read()).map.name === canary.destination) break
      }
      await game.wait.forMap(canary.destination)
      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: canary.destination },
      })
    })
  }

  it("continues a real save on a compressed canary", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "olivine-cafe", x: 3, y: 6 },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "olivine-cafe" },
      player: { x: 3, y: 6 },
    })
  })
})
