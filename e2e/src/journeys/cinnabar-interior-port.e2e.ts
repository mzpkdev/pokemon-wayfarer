import { expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { maps, type Direction } from "../harness/game-session/catalog"

const travel = async (
  map: keyof typeof maps,
  x: number,
  y: number,
  direction: Direction,
  destination: string,
): Promise<void> => {
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: direction, position: { map, x, y } },
      story: { flags: { disableEncounters: true } },
    })
    for (let attempt = 0; attempt < 4; attempt++) {
      await game.wait.forReady()
      await game.player.move(direction)
      await game.wait.frames(15)
      if ((await game.state.read()).map.name === destination) break
    }
    const after = await game.state.read()
    expect(
      after.map.name,
      `${map} at ${x}:${y} going ${direction}; ended at ${after.player.x}:${after.player.y}`,
    ).toBe(destination)
  } finally {
    await game.close()
  }
}

it("enters each Cinnabar building from the exterior", async () => {
  for (const entrance of [
    { x: 8, y: 4, destination: "pokemon-mansion-1f-port" },
    { x: 20, y: 5, destination: "cinnabar-gym-port" },
    { x: 8, y: 10, destination: "cinnabar-lab-entrance-port" },
    { x: 14, y: 12, destination: "cinnabar-center-1f-port" },
    { x: 19, y: 12, destination: "cinnabar-mart-port" },
  ] as const) {
    await travel("cinnabar-seam-poc", entrance.x, entrance.y, "up", entrance.destination)
  }
})

it("returns from each Cinnabar building to the exterior", async () => {
  for (const exit of [
    { map: "pokemon-mansion-1f-port", x: 8, y: 32 },
    { map: "cinnabar-gym-port", x: 25, y: 22 },
    { map: "cinnabar-lab-entrance-port", x: 4, y: 8 },
    { map: "cinnabar-center-1f-port", x: 7, y: 7 },
    { map: "cinnabar-mart-port", x: 4, y: 6 },
  ] as const) {
    await travel(exit.map, exit.x, exit.y, "down", "cinnabar-seam-poc")
  }
})

it("uses the wide side exits and Mansion cave passage", async () => {
  for (const passage of [
    {
      map: "cinnabar-center-1f-port",
      x: 6,
      y: 7,
      direction: "down",
      destination: "cinnabar-seam-poc",
    },
    { map: "cinnabar-mart-port", x: 3, y: 6, direction: "down", destination: "cinnabar-seam-poc" },
    {
      map: "cinnabar-lab-entrance-port",
      x: 3,
      y: 8,
      direction: "down",
      destination: "cinnabar-seam-poc",
    },
    { map: "cinnabar-gym-port", x: 24, y: 22, direction: "down", destination: "cinnabar-seam-poc" },
    {
      map: "pokemon-mansion-1f-port",
      x: 7,
      y: 32,
      direction: "down",
      destination: "cinnabar-seam-poc",
    },
    {
      map: "pokemon-mansion-2f-port",
      x: 27,
      y: 18,
      direction: "up",
      destination: "pokemon-mansion-3f-port",
    },
  ] as const) {
    await travel(passage.map, passage.x, passage.y, passage.direction, passage.destination)
  }
})

it("connects the Center upper floor", async () => {
  await travel("cinnabar-center-1f-port", 2, 6, "left", "cinnabar-center-2f-port")
  await travel("cinnabar-center-2f-port", 2, 6, "left", "cinnabar-center-1f-port")
})

it("connects the Lab rooms", async () => {
  for (const room of [
    { x: 13, destination: "cinnabar-lab-lounge-port" },
    { x: 19, destination: "cinnabar-lab-research-port" },
    { x: 25, destination: "cinnabar-lab-experiment-port" },
  ] as const) {
    await travel("cinnabar-lab-entrance-port", room.x, 6, "up", room.destination)
    await travel(room.destination, 7, 8, "down", "cinnabar-lab-entrance-port")
  }
})

it("connects the Mansion floors and basement", async () => {
  for (const passage of [
    {
      map: "pokemon-mansion-1f-port",
      x: 9,
      y: 13,
      direction: "right",
      destination: "pokemon-mansion-2f-port",
    },
    {
      map: "pokemon-mansion-2f-port",
      x: 7,
      y: 14,
      direction: "left",
      destination: "pokemon-mansion-1f-port",
    },
    {
      map: "pokemon-mansion-2f-port",
      x: 8,
      y: 3,
      direction: "right",
      destination: "pokemon-mansion-3f-port",
    },
    {
      map: "pokemon-mansion-3f-port",
      x: 9,
      y: 3,
      direction: "left",
      destination: "pokemon-mansion-2f-port",
    },
    {
      map: "pokemon-mansion-1f-port",
      x: 26,
      y: 27,
      direction: "left",
      destination: "pokemon-mansion-b1f-port",
    },
    {
      map: "pokemon-mansion-b1f-port",
      x: 33,
      y: 29,
      direction: "right",
      destination: "pokemon-mansion-1f-port",
    },
    {
      map: "pokemon-mansion-1f-port",
      x: 19,
      y: 21,
      direction: "down",
      destination: "pokemon-mansion-3f-port",
    },
  ] as const) {
    await travel(passage.map, passage.x, passage.y, passage.direction, passage.destination)
  }
})

it("heals a fainted party member at the Cinnabar Center", async () => {
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "cinnabar-center-1f-port", x: 7, y: 3 } },
      party: [{ species: "pidgey", fainted: true }, { species: "lapras" }],
      determinism: { textSpeed: "instant" },
    })
    expect((await game.state.read()).party[0]?.fainted).toBe(true)
    await game.player.interact()
    for (let attempt = 0; attempt < 12; attempt++) {
      if (!(await game.state.read()).party[0]?.fainted) break
      await game.wait.frames(20)
      await game.controls.press("a")
    }
    expect((await game.state.read()).party[0]?.fainted).toBe(false)
  } finally {
    await game.close()
  }
})

it("sells an Ultra Ball at the Cinnabar Mart", async () => {
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "left", position: { map: "cinnabar-mart-port", x: 3, y: 3 } },
      determinism: { textSpeed: "instant" },
    })
    const before = await game.state.read()
    expect(before.money).toBeGreaterThanOrEqual(1200)
    expect(await game.inventory.contains("ultraBall")).toBe(false)
    await game.player.interact()
    for (let step = 0; step < 8; step++) {
      await game.wait.frames(30)
      await game.controls.press("a")
      if (await game.inventory.contains("ultraBall")) break
    }
    expect(await game.inventory.contains("ultraBall")).toBe(true)
    expect((await game.state.read()).money).toBeLessThan(before.money)
  } finally {
    await game.close()
  }
})
