import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import type { Direction, GameMap } from "../harness/game-session/catalog"

type Crossing = {
  name: string
  source: GameMap
  destination: GameMap
  position: { x: number; y: number }
  outward: Direction
  inward: Direction
}

const crossings: readonly Crossing[] = [
  {
    name: "north",
    source: "mauville-city",
    destination: "route-111",
    position: { x: 8, y: 0 },
    outward: "up",
    inward: "down",
  },
  {
    name: "south",
    source: "mauville-city",
    destination: "route-110",
    position: { x: 15, y: 19 },
    outward: "down",
    inward: "up",
  },
  {
    name: "west",
    source: "mauville-city",
    destination: "route-117",
    position: { x: 0, y: 10 },
    outward: "left",
    inward: "right",
  },
  {
    name: "east",
    source: "mauville-city",
    destination: "route-118",
    position: { x: 39, y: 10 },
    outward: "right",
    inward: "left",
  },
  {
    name: "connection-heavy",
    source: "route-124",
    destination: "route-125",
    position: { x: 79, y: 10 },
    outward: "right",
    inward: "left",
  },
]

const cross = async (game: GameSession, direction: Direction, destination: GameMap) => {
  for (let attempt = 0; attempt < 4; attempt++) {
    await game.player.move(direction)
    if ((await game.state.read()).map.name === destination) break
    await game.wait.frames(2)
  }
  await game.wait.forMap(destination)
  await expect(game.state.read()).resolves.toMatchObject({
    ready: true,
    controlsLocked: false,
    map: { name: destination },
  })
}

describe.sequential("Wayfarer connected map layouts", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  for (const crossing of crossings) {
    it(`crosses the ${crossing.name} seam in both directions`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: crossing.outward,
          position: { map: crossing.source, ...crossing.position },
        },
        determinism: { textSpeed: "instant" },
      })
      await cross(game, crossing.outward, crossing.destination)
      await cross(game, crossing.inward, crossing.source)
    })
  }
})
