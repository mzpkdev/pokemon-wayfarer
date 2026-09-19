import { expect, it } from "webanvil/test"

import { GameSession, partyMenuActions } from "../harness/game-session"
import { maps, type Direction } from "../harness/game-session/catalog"
import { openFieldPartyMenuActions, selectFieldPartyAction } from "../playbooks/field-party-menu"

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
    { x: 8, y: 4, destination: "pokemon-mansion-1f-frlg" },
    { x: 8, y: 10, destination: "cinnabar-lab-entrance-frlg" },
    { x: 14, y: 12, destination: "cinnabar-center-1f-frlg" },
    { x: 19, y: 12, destination: "cinnabar-mart-frlg" },
  ] as const) {
    await travel("cinnabar-island-frlg", entrance.x, entrance.y, "up", entrance.destination)
  }
})

it("returns from each Cinnabar building to the exterior", async () => {
  for (const exit of [
    { map: "pokemon-mansion-1f-frlg", x: 8, y: 32 },
    { map: "cinnabar-gym-frlg", x: 25, y: 22 },
    { map: "cinnabar-lab-entrance-frlg", x: 4, y: 8 },
    { map: "cinnabar-center-1f-frlg", x: 7, y: 7 },
    { map: "cinnabar-mart-frlg", x: 4, y: 6 },
  ] as const) {
    await travel(exit.map, exit.x, exit.y, "down", "cinnabar-island-frlg")
  }
})

it("connects both Route 20 entrances to the Seafoam cave", async () => {
  await travel("route-20-frlg", 60, 9, "up", "seafoam-islands-1f-frlg")
  await travel("route-20-frlg", 72, 15, "up", "seafoam-islands-1f-frlg")
  await travel("seafoam-islands-1f-frlg", 6, 20, "down", "route-20-frlg")
  await travel("seafoam-islands-1f-frlg", 32, 20, "down", "route-20-frlg")
})

it("keeps the Gym locked until the Mansion Secret Key is taken, including after reload", async () => {
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "cinnabar-island-frlg", x: 20, y: 6 } },
      story: { flags: { disableEncounters: true } },
      determinism: { textSpeed: "instant" },
    })
    await game.player.move("up")
    await game.wait.frames(30)
    expect((await game.state.read()).map.name).toBe("cinnabar-island-frlg")
    await game.wait.until(
      (state) => state.dialogue.text.startsWith("The GYM door is locked."),
      "locked Gym door message",
    )
    await game.controls.press("a")
    await game.wait.forReady()

    await game.player.warp("pokemon-mansion-b1f-frlg", 5, 8, "up")
    await game.wait.forReady()
    await game.player.interact()
    for (let attempt = 0; attempt < 12 && !(await game.inventory.contains("secretKey")); attempt++) {
      await game.wait.frames(20)
      await game.controls.press("a")
    }
    expect(await game.inventory.contains("secretKey")).toBe(true)
    for (let attempt = 0; attempt < 12 && !(await game.state.read()).ready; attempt++) {
      await game.controls.press("a")
      await game.wait.frames(20)
    }
    await game.wait.forReady()
    await game.saveAndReload()
    expect(await game.inventory.contains("secretKey")).toBe(true)

    await game.player.warp("cinnabar-island-frlg", 20, 5, "up")
    await game.wait.forReady()
    await game.player.move("up")
    await game.wait.until(
      (state) => state.map.name === "cinnabar-gym-frlg",
      "Secret Key opens the Cinnabar Gym",
      1_200,
    )
  } finally {
    await game.close()
  }
})

it("flies to the visited Cinnabar exterior", async () => {
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      story: { flags: { visitedKanto: true, visitedCinnabarIsland: true } },
      party: [{ species: "pidgey", moves: ["fly"] }],
    })
    await openFieldPartyMenuActions(game)
    await selectFieldPartyAction(game, partyMenuActions.fly)
    let flyMapOpen = false
    for (let elapsed = 0; elapsed < 600; elapsed += 4) {
      await game.wait.frames(4)
      try {
        if ((await game.regionMap.active()).layout === "combined") {
          flyMapOpen = true
          break
        }
      } catch {
        // The Fly map is allocated before its layout is initialized.
      }
    }
    expect(flyMapOpen).toBe(true)
    await game.wait.frames(60)
    for (const x of [15, 16, 17, 18, 19, 20]) {
      await game.controls.press("right")
      await game.wait.until(
        async () => (await game.regionMap.active()).cursor.x === x,
        `Fly cursor at x=${x}`,
        90,
      )
    }
    for (const y of [14, 15]) {
      await game.controls.press("down")
      await game.wait.until(
        async () => (await game.regionMap.active()).cursor.y === y,
        `Fly cursor at y=${y}`,
        90,
      )
    }
    await expect(game.regionMap.active()).resolves.toMatchObject({
      mapSectionType: 2,
      cursor: { x: 20, y: 15 },
    })
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.ready && state.map.name === "cinnabar-island-frlg",
      "Fly to Cinnabar",
      3_600,
    )
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "cinnabar-island-frlg" },
      player: { x: 14, y: 12 },
    })
  } finally {
    await game.close()
  }
})

it("connects the Center upper floor", async () => {
  await travel("cinnabar-center-1f-frlg", 2, 6, "left", "cinnabar-center-2f-frlg")
  await travel("cinnabar-center-2f-frlg", 2, 6, "left", "cinnabar-center-1f-frlg")
})

it("connects the Lab rooms", async () => {
  for (const room of [
    { x: 13, destination: "cinnabar-lab-lounge-frlg" },
    { x: 19, destination: "cinnabar-lab-research-frlg" },
    { x: 25, destination: "cinnabar-lab-experiment-frlg" },
  ] as const) {
    await travel("cinnabar-lab-entrance-frlg", room.x, 6, "up", room.destination)
    await travel(room.destination, 7, 8, "down", "cinnabar-lab-entrance-frlg")
  }
})

it("connects the Mansion floors and basement", async () => {
  for (const passage of [
    {
      map: "pokemon-mansion-1f-frlg",
      x: 9,
      y: 13,
      direction: "right",
      destination: "pokemon-mansion-2f-frlg",
    },
    {
      map: "pokemon-mansion-2f-frlg",
      x: 7,
      y: 14,
      direction: "left",
      destination: "pokemon-mansion-1f-frlg",
    },
    {
      map: "pokemon-mansion-2f-frlg",
      x: 8,
      y: 3,
      direction: "right",
      destination: "pokemon-mansion-3f-frlg",
    },
    {
      map: "pokemon-mansion-3f-frlg",
      x: 9,
      y: 3,
      direction: "left",
      destination: "pokemon-mansion-2f-frlg",
    },
    {
      map: "pokemon-mansion-1f-frlg",
      x: 26,
      y: 27,
      direction: "left",
      destination: "pokemon-mansion-b1f-frlg",
    },
    {
      map: "pokemon-mansion-b1f-frlg",
      x: 33,
      y: 29,
      direction: "right",
      destination: "pokemon-mansion-1f-frlg",
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
      player: { facing: "up", position: { map: "cinnabar-center-1f-frlg", x: 7, y: 3 } },
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

it("sells a Poké Ball from the Cinnabar Mart's rating-zero catalog", async () => {
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "left", position: { map: "cinnabar-mart-frlg", x: 3, y: 3 } },
      determinism: { textSpeed: "instant" },
    })
    const before = await game.state.read()
    expect(before.money).toBeGreaterThanOrEqual(1200)
    expect(await game.inventory.contains("pokeBall")).toBe(false)
    await game.player.interact()
    // Buy is the first clerk choice and Poké Ball is the first rating-zero item.
    for (let step = 0; step < 10; step++) {
      await game.wait.frames(30)
      await game.controls.press("a")
      if (await game.inventory.contains("pokeBall")) break
    }
    expect(await game.inventory.contains("pokeBall")).toBe(true)
    // The shop adds the item before its thank-you message is acknowledged.
    await game.controls.press("a")
    await game.wait.until((state) => state.money < before.money, "Mart purchase charged", 300)
    expect((await game.state.read()).money).toBeLessThan(before.money)
  } finally {
    await game.close()
  }
})
