import * as crypto from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"

import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

const configuration = process.env.MAP_LAYOUT_TIMING_CONFIGURATION
const outputPath = process.env.MAP_LAYOUT_TIMING_OUTPUT
const sourceRevision = process.env.MAP_LAYOUT_SOURCE_REVISION
const sampleCount = 100

const canaries = [
  {
    caseName: "black-screen-warp-emerald-canary",
    map: "fortree-city",
    position: { x: 10, y: 4 },
    direction: "up",
    destination: "fortree-house-1",
  },
  {
    caseName: "black-screen-warp-hns-canary",
    map: "olivine-city",
    position: { x: 8, y: 44 },
    direction: "up",
    destination: "olivine-cafe",
  },
  {
    caseName: "black-screen-warp-frlg-canary",
    map: "sevii-five-island-resort-gorgeous",
    position: { x: 39, y: 9 },
    direction: "up",
    destination: "sevii-five-island-resort-gorgeous-house",
  },
] as const

const rolloutDescribe = configuration === undefined ? describe.skip : describe.sequential

const requireConfiguration = (): "legacy" | "raw" | "hybrid" => {
  if (configuration !== "legacy" && configuration !== "raw" && configuration !== "hybrid")
    throw new Error("MAP_LAYOUT_TIMING_CONFIGURATION must be legacy, raw, or hybrid")
  if (!outputPath || !sourceRevision)
    throw new Error("MAP_LAYOUT_TIMING_OUTPUT and MAP_LAYOUT_SOURCE_REVISION are required")
  return configuration
}

const hashFile = async (file: string): Promise<string> =>
  crypto.createHash("sha256").update(await fs.promises.readFile(file)).digest("hex")

rolloutDescribe("Wayfarer map-layout production-equivalent timing", () => {
  it("records paired complete transition samples", async () => {
    const mode = requireConfiguration()
    const romPath = process.env.SKYEMU_ROM
    if (!romPath || !outputPath || !sourceRevision) throw new Error("timing environment is incomplete")

    const game = await GameSession.launch()
    const cases: Record<string, number[]> = {}
    try {
      for (const canary of canaries) {
        const samples: number[] = []
        for (let sample = 0; sample < sampleCount; sample++) {
          await game.arrange({
            checkpoint: "new-bark-after-intro",
            player: {
              facing: canary.direction,
              position: { map: canary.map, ...canary.position },
            },
            determinism: { textSpeed: "instant" },
          })
          const start = game.measuredFrames()
          for (let attempt = 0; attempt < 4; attempt++) {
            await game.player.move(canary.direction)
            if ((await game.state.read()).map.name === canary.destination) break
            await game.wait.frames(2)
          }
          await game.wait.forMap(canary.destination)
          samples.push(game.measuredFrames() - start)
        }
        cases[canary.caseName] = samples
      }

      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "down",
          position: { map: "olivine-cafe", x: 3, y: 6 },
        },
        determinism: { textSpeed: "instant" },
      })
      await game.saveAndReload()
      const reloadSamples: number[] = []
      for (let sample = 0; sample < sampleCount; sample++) {
        const start = game.measuredFrames()
        await game.reloadSavedGame()
        reloadSamples.push(game.measuredFrames() - start)
      }
      cases["black-screen-save-reload-hns-canary"] = reloadSamples
      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: "olivine-cafe" },
        player: { x: 3, y: 6 },
      })
    } finally {
      await game.close()
    }

    let document: Record<string, unknown> = {}
    try {
      document = JSON.parse(await fs.promises.readFile(outputPath, "utf8")) as Record<
        string,
        unknown
      >
    } catch (error) {
      if ((error as NodeJS.ErrnoException).code !== "ENOENT") throw error
    }
    const previousRevision = document.source_revision
    if (previousRevision !== undefined && previousRevision !== sourceRevision)
      throw new Error(`timing output revision ${previousRevision} does not match ${sourceRevision}`)
    const configurations = (document.configurations ?? {}) as Record<string, unknown>
    const builds = (document.builds ?? {}) as Record<string, unknown>
    configurations[mode] = cases
    builds[mode] = { rom_sha256: await hashFile(romPath) }
    await fs.promises.mkdir(path.dirname(outputPath), { recursive: true })
    await fs.promises.writeFile(
      outputPath,
      `${JSON.stringify(
        {
          schema_version: 1,
          source_revision: sourceRevision,
          measurement_revision: "wayfarer-map-layout-timing-v1",
          device: "SkyEmu static 0.0.2 deterministic frame stepping",
          timer: "host-counted emulated frames from trigger through ready input state",
          builds,
          configurations,
        },
        null,
        2,
      )}\n`,
    )
  }, 600_000)
})
