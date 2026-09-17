import * as fs from "node:fs/promises"
import * as path from "node:path"

import { expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type Direction, type GameMap } from "../harness/game-session/catalog"

const output = process.env.FRLG_COAST_POC_ARTIFACTS
const pocIt = output ? it : it.skip

const moveOneTile = async (game: GameSession, direction: Direction): Promise<void> => {
  const before = await game.state.read()
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.wait.forReady()
    await game.player.move(direction)
    await game.wait.frames(12)
    const after = await game.state.read()
    if (after.player.x !== before.player.x || after.player.y !== before.player.y || after.map.name !== before.map.name)
      return
  }
  throw new Error(`Could not move ${direction} from ${before.map.name} ${before.player.x}:${before.player.y}`)
}

const expectMap = async (game: GameSession, map: GameMap): Promise<void> => {
  await game.wait.forReady()
  expect((await game.state.read()).map.name).toBe(map)
}

const startSurf = async (game: GameSession): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "right", position: { map: "cinnabar-seam-poc", x: 22, y: 10 } },
    story: { flags: { disableEncounters: true } },
    party: [{ species: "lapras", moves: ["surf"] }],
    determinism: { textSpeed: "instant" },
  })
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.wait.until(
    (state) => state.dialogue.message === "want-to-use-surf" && !state.dialogueOpen,
    "Surf prompt",
  )
  await game.wait.frames(12)
  await game.controls.press("a")
  await game.wait.until((state) => state.dialogue.message === "player-used-surf", "Surf confirmation")
  await game.dialogue.waitForClosed()
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => state.player.surfing, "surfing", 3_600)
  await game.wait.forReady()
}

const screenshot = async (game: GameSession, name: string): Promise<void> => {
  if (!output) throw new Error("FRLG_COAST_POC_ARTIFACTS must name the artifact directory")
  await fs.writeFile(path.join(output, `${name}.png`), await game.screenshot())
  await fs.writeFile(path.join(output, `${name}.json`), JSON.stringify(await game.state.read(), null, 2))
}

const cross = async (
  game: GameSession,
  from: { map: GameMap; x: number; y: number; facing: Direction },
  direction: Direction,
  destination: GameMap,
  name: string,
): Promise<void> => {
  await game.player.warp(from.map, from.x, from.y, from.facing)
  await game.wait.forReady()
  await moveOneTile(game, direction)
  await expectMap(game, destination)
  await screenshot(game, name)
}

pocIt("crosses the complete FRLG coast and packages a surf save", async () => {
  if (!output) throw new Error("FRLG_COAST_POC_ARTIFACTS must name the artifact directory")
  await fs.mkdir(output, { recursive: true })
  const game = await GameSession.launch()
  try {
    await startSurf(game)

    await cross(
      game,
      { map: "pallet-town", x: 7, y: 19, facing: "down" },
      "down",
      "route-21-north-coast-poc",
      "pallet-to-route21-north",
    )
    await cross(
      game,
      { map: "route-21-north-coast-poc", x: 7, y: 0, facing: "up" },
      "up",
      "pallet-town",
      "pallet-returned-from-route21-north",
    )
    await cross(
      game,
      { map: "route-21-north-coast-poc", x: 7, y: 49, facing: "down" },
      "down",
      "route-21-south-coast-poc",
      "route21-north-to-south",
    )
    await cross(
      game,
      { map: "route-21-south-coast-poc", x: 7, y: 0, facing: "up" },
      "up",
      "route-21-north-coast-poc",
      "route21-south-returned-north",
    )
    await cross(
      game,
      { map: "route-21-south-coast-poc", x: 2, y: 49, facing: "down" },
      "down",
      "cinnabar-seam-poc",
      "route21-south-to-cinnabar",
    )
    await cross(
      game,
      { map: "cinnabar-seam-poc", x: 2, y: 0, facing: "up" },
      "up",
      "route-21-south-coast-poc",
      "cinnabar-returned-to-route21-south",
    )
    await cross(
      game,
      { map: "cinnabar-seam-poc", x: 23, y: 10, facing: "right" },
      "right",
      "route-20-coast-poc",
      "cinnabar-to-route20",
    )
    await cross(
      game,
      { map: "route-20-coast-poc", x: 0, y: 10, facing: "left" },
      "left",
      "cinnabar-seam-poc",
      "route20-returned-to-cinnabar",
    )
    await cross(
      game,
      { map: "route-20-coast-poc", x: 119, y: 10, facing: "right" },
      "right",
      "route-19-coast-poc",
      "route20-to-route19",
    )
    await cross(
      game,
      { map: "route-19-coast-poc", x: 0, y: 50, facing: "left" },
      "left",
      "route-20-coast-poc",
      "route19-returned-to-route20",
    )
    await cross(
      game,
      { map: "route-19-coast-poc", x: 12, y: 0, facing: "up" },
      "up",
      "fuchsia-city",
      "route19-to-fuchsia",
    )
    await cross(
      game,
      { map: "fuchsia-city", x: 24, y: 39, facing: "down" },
      "down",
      "route-19-coast-poc",
      "fuchsia-to-route19",
    )

    await game.player.warp("route-20-coast-poc", 60, 9, "up")
    await game.wait.forReady()
    await screenshot(game, "route20-west-door-approach")
    await game.saveAndReload()
    await expectMap(game, "route-20-coast-poc")
    await screenshot(game, "route20-west-door-reloaded")
    const romPath = (game as unknown as { rom: { path: string } }).rom.path
    await fs.copyFile(path.join(path.dirname(romPath), "wayfarer.sav"), path.join(output, "frlg-coast-overworld-poc.sav"))
    await fs.copyFile(process.env.SKYEMU_ROM!, path.join(output, "frlg-coast-overworld-poc.gba"))
    await fs.copyFile(process.env.SKYEMU_SYMS!, path.join(output, "frlg-coast-overworld-poc.sym"))
  } finally {
    await game.close()
  }
})

pocIt("packages a Cinnabar continuation save", async () => {
  if (!output) throw new Error("FRLG_COAST_POC_ARTIFACTS must name the artifact directory")
  await fs.mkdir(output, { recursive: true })
  const game = await GameSession.launch()
  try {
    await startSurf(game)
    await screenshot(game, "cinnabar-continuation-start")
    await game.saveAndReload()
    await expectMap(game, "cinnabar-seam-poc")
    expect((await game.state.read()).player.surfing).toBe(true)
    const romPath = (game as unknown as { rom: { path: string } }).rom.path
    await fs.copyFile(path.join(path.dirname(romPath), "wayfarer.sav"), path.join(output, "cinnabar-test.sav"))
    await fs.copyFile(process.env.SKYEMU_ROM!, path.join(output, "cinnabar-test.gba"))
  } finally {
    await game.close()
  }
})

pocIt("captures direct visual loads for every Seafoam floor", async () => {
  if (!output) throw new Error("FRLG_COAST_POC_ARTIFACTS must name the artifact directory")
  await fs.mkdir(output, { recursive: true })
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      story: { flags: { disableEncounters: true } },
      determinism: { textSpeed: "instant" },
    })
    for (const [map, name] of [
      ["seafoam-islands-1f-coast-poc", "seafoam-1f-direct-load"],
      ["seafoam-islands-b1f-coast-poc", "seafoam-b1f-direct-load"],
      ["seafoam-islands-b2f-coast-poc", "seafoam-b2f-direct-load"],
      ["seafoam-islands-b3f-coast-poc", "seafoam-b3f-direct-load"],
      ["seafoam-islands-b4f-coast-poc", "seafoam-b4f-direct-load"],
    ] as const) {
      await game.player.warp(map, 16, 10)
      await game.wait.forReady()
      await game.wait.frames(60)
      await screenshot(game, name)
    }
  } finally {
    await game.close()
  }
})
