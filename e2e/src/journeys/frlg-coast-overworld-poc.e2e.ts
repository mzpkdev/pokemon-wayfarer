import * as fs from "node:fs/promises"
import * as path from "node:path"

import { expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type Direction, type GameMap } from "../harness/game-session/catalog"

const artifacts = process.env.FRLG_COAST_POC_ARTIFACTS

const capture = async (game: GameSession, name: string): Promise<void> => {
  if (!artifacts) return
  await fs.mkdir(artifacts, { recursive: true })
  await fs.writeFile(path.join(artifacts, `${name}.png`), await game.screenshot())
  await fs.writeFile(path.join(artifacts, `${name}.json`), JSON.stringify(await game.state.read(), null, 2))
}

const step = async (game: GameSession, direction: Direction): Promise<void> => {
  const before = await game.state.read()
  for (let attempt = 0; attempt < 4; attempt++) {
    await game.wait.forReady()
    await game.player.move(direction)
    await game.wait.frames(15)
    const after = await game.state.read()
    if (
      after.player.x !== before.player.x ||
      after.player.y !== before.player.y ||
      after.map.name !== before.map.name
    ) return
  }
  throw new Error(`Blocked moving ${direction} from ${before.map.name} ${before.player.x}:${before.player.y}`)
}

const cross = async (
  game: GameSession,
  from: { map: GameMap; x: number; y: number; facing: Direction },
  direction: Direction,
  destination: GameMap,
): Promise<void> => {
  await game.wait.forReady()
  await game.player.warp(from.map, from.x, from.y, from.facing)
  await game.wait.forReady()
  await step(game, direction)
  await game.wait.until(
    (state) => state.map.name === destination,
    `${from.map} to ${destination}`,
    300,
  )
  await game.wait.forReady()
  expect((await game.state.read()).map.name).toBe(destination)
  await capture(game, `${from.map}-${from.x}-${from.y}-to-${destination}`)
}

const crossFromFixture = async (
  from: { map: GameMap; x: number; y: number; facing: Direction },
  direction: Direction,
  destination: GameMap,
  saveReload = false,
): Promise<void> => {
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: from.facing, position: from },
      story: { flags: { disableEncounters: true } },
      determinism: { textSpeed: "instant" },
    })
    await game.wait.forReady()
    for (let attempt = 0; attempt < 4; attempt++) {
      if ((await game.state.read()).map.name === destination) break
      await step(game, direction)
      await game.wait.frames(60)
    }
    await game.wait.until((state) => state.map.name === destination, `${from.map} to ${destination}`, 300)
    await capture(game, `${from.map}-${from.x}-${from.y}-to-${destination}`)
    if (saveReload) {
      await game.saveAndReload()
      expect((await game.state.read()).map.name).toBe(destination)
      await capture(game, "seafoam-b4f-reloaded")
    }
  } finally {
    await game.close()
  }
}

const startSurf = async (game: GameSession): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "right", position: { map: "cinnabar-island-frlg", x: 22, y: 10 } },
    story: { flags: { disableEncounters: true } },
    party: [{ species: "lapras", moves: ["surf"] }],
    determinism: { textSpeed: "instant" },
  })
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.wait.until(
    (state) => state.dialogue.message === "want-to-use-surf" && !state.dialogueOpen,
    "native Surf prompt",
  )
  await game.wait.frames(12)
  await game.controls.press("a")
  await game.wait.until((state) => state.dialogue.message === "player-used-surf", "Surf confirmation")
  await game.dialogue.waitForClosed()
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => state.player.surfing, "surfing without a badge", 3_600)
  await game.wait.forReady()
}

it("crosses both complete FRLG coast approaches and returns through every route boundary", async () => {
  const game = await GameSession.launch()
  try {
    await startSurf(game)
    for (const [from, direction, destination] of [
      [{ map: "pallet-town", x: 7, y: 19, facing: "down" }, "down", "route-21-north-frlg"],
      [{ map: "route-21-north-frlg", x: 7, y: 0, facing: "up" }, "up", "pallet-town"],
      [{ map: "route-21-north-frlg", x: 7, y: 49, facing: "down" }, "down", "route-21-south-frlg"],
      [{ map: "route-21-south-frlg", x: 7, y: 0, facing: "up" }, "up", "route-21-north-frlg"],
      [{ map: "route-21-south-frlg", x: 2, y: 49, facing: "down" }, "down", "cinnabar-island-frlg"],
      [{ map: "cinnabar-island-frlg", x: 2, y: 0, facing: "up" }, "up", "route-21-south-frlg"],
      [{ map: "cinnabar-island-frlg", x: 23, y: 10, facing: "right" }, "right", "route-20-frlg"],
      [{ map: "route-20-frlg", x: 0, y: 10, facing: "left" }, "left", "cinnabar-island-frlg"],
      [{ map: "route-20-frlg", x: 119, y: 10, facing: "right" }, "right", "route-19-frlg"],
      [{ map: "route-19-frlg", x: 0, y: 50, facing: "left" }, "left", "route-20-frlg"],
      [{ map: "route-19-frlg", x: 12, y: 0, facing: "up" }, "up", "fuchsia-city"],
      [{ map: "fuchsia-city", x: 24, y: 39, facing: "down" }, "down", "route-19-frlg"],
    ] as const) {
      await cross(game, from, direction, destination)
    }
    await game.player.warp("route-20-frlg", 60, 9, "up")
    await game.wait.forReady()
    await game.saveAndReload()
    expect((await game.state.read()).map.name).toBe("route-20-frlg")
    await capture(game, "route20-reloaded")
  } finally {
    await game.close()
  }
})

it("enters and exits both Seafoam doors and traverses all five FRLG floors", async () => {
  const passages = [
    [{ map: "route-20-frlg", x: 60, y: 9, facing: "up" }, "up", "seafoam-islands-1f-frlg"],
    [{ map: "seafoam-islands-1f-frlg", x: 6, y: 20, facing: "down" }, "down", "route-20-frlg"],
    [{ map: "route-20-frlg", x: 72, y: 15, facing: "up" }, "up", "seafoam-islands-1f-frlg"],
    [{ map: "seafoam-islands-1f-frlg", x: 32, y: 20, facing: "down" }, "down", "route-20-frlg"],
    [{ map: "seafoam-islands-1f-frlg", x: 10, y: 7, facing: "up" }, "up", "seafoam-islands-b1f-frlg"],
    [{ map: "seafoam-islands-b1f-frlg", x: 7, y: 4, facing: "up" }, "up", "seafoam-islands-b2f-frlg"],
    [{ map: "seafoam-islands-b2f-frlg", x: 7, y: 16, facing: "down" }, "down", "seafoam-islands-b3f-frlg"],
    [{ map: "seafoam-islands-b3f-frlg", x: 12, y: 10, facing: "up" }, "up", "seafoam-islands-b4f-frlg"],
  ] as const
  for (const [index, [from, direction, destination]] of passages.entries()) {
    await crossFromFixture(from, direction, destination, index === passages.length - 1)
  }
})
