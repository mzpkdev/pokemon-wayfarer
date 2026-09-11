import * as fs from "node:fs/promises"
import * as path from "node:path"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

const styles = [1, 2, 3, 4] as const
const ids = [1, 2, 5, 6] as const
const evidence = path.resolve("artifacts/trainer-appearance")

describe.sequential("Wayfarer underwater appearance", () => {
  for (const style of styles) {
    it(`Style ${style}: underwater movement and real surface/dive round trip`, async () => {
      const game = await GameSession.launch()
      try {
        const symbols = await readSkyEmuSymbols(requireSymbolsPath())
        const client = (game as unknown as { running: RunningSkyEmu }).running.client
        const flags = async () => (await client.readBytes(symbols.address("gPlayerAvatar"), 1))[0]!
        const capture = async (label: string) => {
          await fs.mkdir(evidence, { recursive: true })
          await fs.writeFile(
            path.join(evidence, `style-${style}-underwater-${label}.png`),
            await game.screenshot(),
          )
        }
        // The initial underwater location is a fixture; both subsequent warps use controller input.
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: {
            appearanceStyle: style,
            facing: "down",
            position: { map: "underwater-route-124", x: 10, y: 5 },
          },
          party: [{ species: "lapras", level: 50, moves: ["dive", "surf"] }],
          story: { flags: { hoennDiveAuthorized: true } },
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        await capture("fixture")
        expect((await game.state.read()).appearance.id).toBe(ids[style - 1])
        expect((await flags()) & 16).toBe(16)
        for (const direction of ["down", "right", "up", "left"] as const) {
          await game.controls.press(direction)
          await game.wait.frames(20)
          await capture(direction)
          expect((await flags()) & 16).toBe(16)
        }
        await game.controls.press("b")
        await game.wait.frames(40)
        await capture("surface-prompt")
        for (
          let step = 0;
          step < 8 && (await game.state.read()).map.name === "underwater-route-124";
          step++
        ) {
          await game.controls.press("a")
          await game.wait.frames(60)
        }
        await game.wait.forReady()
        await capture("surfaced")
        await fs.writeFile(
          path.join(evidence, `style-${style}-underwater-state.json`),
          JSON.stringify(await game.state.read(), null, 2),
        )
        expect((await game.state.read()).map.name).not.toBe("underwater-route-124")
        expect((await flags()) & 8).toBe(8)
        expect((await flags()) & 16).toBe(0)
        await game.controls.press("a")
        await game.wait.frames(40)
        await capture("dive-prompt")
        for (
          let step = 0;
          step < 8 && (await game.state.read()).map.name !== "underwater-route-124";
          step++
        ) {
          await game.controls.press("a")
          await game.wait.frames(60)
        }
        await game.wait.forReady()
        await capture("returned")
        expect((await game.state.read()).map.name).toBe("underwater-route-124")
        expect((await flags()) & 16).toBe(16)
        expect((await game.state.read()).appearance.id).toBe(ids[style - 1])
      } finally {
        await game.close()
      }
    })
  }
})
