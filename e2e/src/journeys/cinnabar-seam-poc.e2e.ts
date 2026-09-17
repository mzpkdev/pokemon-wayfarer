import * as fs from "node:fs/promises"
import * as os from "node:os"
import * as path from "node:path"

import { expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type Direction } from "../harness/game-session/catalog"

const output = process.env.CINNABAR_POC_ARTIFACTS
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
  throw new Error(`Could not move ${direction} from ${before.player.x}:${before.player.y}`)
}

pocIt("prepares a surf save and crosses the Route 21 Cinnabar seam", async () => {
  if (!output) throw new Error("CINNABAR_POC_ARTIFACTS must name the artifact directory")
  await fs.mkdir(output, { recursive: true })
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "left", position: { map: "route-21", x: 6, y: 89 } },
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

    for (const direction of [
      "down", "down", "left", "down", "down", "down", "right", "down", "down", "down", "down", "down",
      "left", "left", "left",
    ] as const) {
      await moveOneTile(game, direction)
    }
    const approach = await game.state.read()
    expect(approach).toMatchObject({ map: { name: "route-21" }, player: { surfing: true } })
    await fs.writeFile(path.join(output, "route21-approach.png"), await game.screenshot())
    await fs.writeFile(path.join(output, "route21-approach.json"), JSON.stringify(approach, null, 2))

    await game.saveAndReload()
    const saved = await game.state.read()
    await fs.writeFile(path.join(output, "route21-reloaded.json"), JSON.stringify(saved, null, 2))
    await fs.writeFile(path.join(output, "route21-reloaded.png"), await game.screenshot())
    const romPath = (game as unknown as { rom: { path: string } }).rom.path
    const tempContents = await fs.readdir(path.dirname(romPath))
    const dataDirs = (await fs.readdir(os.tmpdir())).filter((name) => name.startsWith("wayfarer-skyemu-data-"))
    await fs.writeFile(path.join(output, "skyemu-files.json"), JSON.stringify({ romPath, tempContents, dataDirs }, null, 2))
    await fs.copyFile(path.join(path.dirname(romPath), "wayfarer.sav"), path.join(output, "route21-approach.sav"))
    await fs.copyFile(path.join(output, "route21-approach.sav"), path.join(output, "cinnabar-seam-poc.sav"))
    await fs.copyFile(process.env.SKYEMU_ROM!, path.join(output, "cinnabar-seam-poc.gba"))
    await fs.copyFile(process.env.SKYEMU_SYMS!, path.join(output, "cinnabar-seam-poc.sym"))

    await moveOneTile(game, "down")
    await game.wait.frames(90)
    const crossed = await game.state.read()
    await fs.writeFile(path.join(output, "route21-crossed.json"), JSON.stringify(crossed, null, 2))
    await fs.writeFile(path.join(output, "route21-crossed.png"), await game.screenshot())
    expect(crossed.map.name).toBe("cinnabar-seam-poc")
    await game.wait.frames(90)
    await fs.writeFile(path.join(output, "route21-crossed-settled.png"), await game.screenshot())
    await moveOneTile(game, "up")
    expect(await game.state.read()).toMatchObject({ map: { name: "route-21" }, player: { surfing: true } })
    await moveOneTile(game, "down")
    expect(await game.state.read()).toMatchObject({ map: { name: "cinnabar-seam-poc" }, player: { surfing: true } })
    await game.player.warp("cinnabar-seam-poc", crossed.player.x, crossed.player.y, "down")
    await game.wait.forReady()
    await game.wait.frames(90)
    await fs.writeFile(path.join(output, "cinnabar-full-load.png"), await game.screenshot())
  } finally {
    await game.close()
  }
})

pocIt("boots the packaged Route 21 save in a new SkyEmu process", async () => {
  if (!output) throw new Error("CINNABAR_POC_ARTIFACTS must name the artifact directory")
  const game = await GameSession.launch(path.join(output, "cinnabar-seam-poc.sav"))
  try {
    for (let attempt = 0; attempt < 40; attempt++) {
      await game.wait.frames(30)
      const state = await game.state.read()
      if (state.ready) break
      await game.controls.press("a")
    }
    const state = await game.state.read()
    await fs.writeFile(path.join(output, "packaged-save-boot.json"), JSON.stringify(state, null, 2))
    await fs.writeFile(path.join(output, "packaged-save-boot.png"), await game.screenshot())
    expect(state).toMatchObject({
      ready: true,
      map: { name: "route-21" },
      player: { surfing: true, x: 2, y: 99 },
    })
  } finally {
    await game.close()
  }
})

pocIt("captures the Pallet Town preview from Route 21", async () => {
  if (!output) throw new Error("CINNABAR_POC_ARTIFACTS must name the artifact directory")
  await fs.mkdir(output, { recursive: true })
  const game = await GameSession.launch()
  try {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "route-21", x: 12, y: 1 } },
      story: { flags: { disableEncounters: true } },
    })
    await game.wait.forReady()
    await fs.writeFile(path.join(output, "route21-pallet-preview.png"), await game.screenshot())
  } finally {
    await game.close()
  }
})

pocIt("crosses the Route 20 Cinnabar seam while surfing", async () => {
  if (!output) throw new Error("CINNABAR_POC_ARTIFACTS must name the artifact directory")
  const game = await GameSession.launch()
  try {
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
      "Route 20 Surf prompt",
    )
    await game.wait.frames(12)
    await game.controls.press("a")
    await game.wait.until((state) => state.dialogue.message === "player-used-surf", "Route 20 Surf confirmation")
    await game.dialogue.waitForClosed()
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.until((state) => state.player.surfing, "Route 20 surfing", 3_600)
    await game.wait.forReady()
    const approach = await game.state.read()
    expect(approach).toMatchObject({ map: { name: "cinnabar-seam-poc" }, player: { surfing: true } })
    await fs.writeFile(path.join(output, "route20-approach.json"), JSON.stringify(approach, null, 2))
    await fs.writeFile(path.join(output, "route20-approach.png"), await game.screenshot())
    await game.saveAndReload()
    const saved = await game.state.read()
    await fs.writeFile(path.join(output, "route20-reloaded.json"), JSON.stringify(saved, null, 2))
    const romPath = (game as unknown as { rom: { path: string } }).rom.path
    await fs.copyFile(path.join(path.dirname(romPath), "wayfarer.sav"), path.join(output, "route20-approach.sav"))
    await fs.copyFile(path.join(output, "route20-approach.sav"), path.join(output, "cinnabar-seam-route20.sav"))
    await fs.copyFile(process.env.SKYEMU_ROM!, path.join(output, "cinnabar-seam-route20.gba"))
    await moveOneTile(game, "right")
    const crossed = await game.state.read()
    await fs.writeFile(path.join(output, "route20-crossed.json"), JSON.stringify(crossed, null, 2))
    await fs.writeFile(path.join(output, "route20-crossed.png"), await game.screenshot())
    expect(crossed).toMatchObject({ map: { name: "route-20" }, player: { surfing: true } })
    await moveOneTile(game, "left")
    const returned = await game.state.read()
    await fs.writeFile(path.join(output, "route20-returned-cinnabar.json"), JSON.stringify(returned, null, 2))
    await fs.writeFile(path.join(output, "route20-returned-cinnabar.png"), await game.screenshot())
    expect(returned).toMatchObject({ map: { name: "cinnabar-seam-poc" }, player: { surfing: true } })
    await game.player.warp("cinnabar-seam-poc", returned.player.x, returned.player.y, "left")
    await game.wait.forReady()
    await game.wait.frames(90)
    await fs.writeFile(path.join(output, "route20-returned-full-load.png"), await game.screenshot())
  } finally {
    await game.close()
  }
})

pocIt("boots the packaged Route 20 save in a new SkyEmu process", async () => {
  if (!output) throw new Error("CINNABAR_POC_ARTIFACTS must name the artifact directory")
  const game = await GameSession.launch(path.join(output, "cinnabar-seam-route20.sav"))
  try {
    for (let attempt = 0; attempt < 40; attempt++) {
      await game.wait.frames(30)
      const state = await game.state.read()
      if (state.ready) break
      await game.controls.press("a")
    }
    const state = await game.state.read()
    await fs.writeFile(path.join(output, "route20-packaged-save-boot.json"), JSON.stringify(state, null, 2))
    await fs.writeFile(path.join(output, "route20-packaged-save-boot.png"), await game.screenshot())
    expect(state).toMatchObject({
      ready: true,
      map: { name: "cinnabar-seam-poc" },
      player: { surfing: true, x: 23, y: 10 },
    })
  } finally {
    await game.close()
  }
})
