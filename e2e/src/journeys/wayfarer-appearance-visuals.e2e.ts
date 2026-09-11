import * as fs from "node:fs/promises"
import * as path from "node:path"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

const styles = [1, 2, 3, 4] as const
const ids = [1, 2, 5, 6] as const
const evidence = process.env.SKYEMU_CAPTURE_DIR ?? "/tmp/trainer-appearance-visuals"

// These journeys use real menus and controller input after fixture arrangement.
// Screenshots are evidence for manual review, not pixel-baseline assertions.
describe.sequential("Wayfarer appearance visual surfaces", () => {
  for (const style of styles) {
    it(`Style ${style}: walking, Trainer Card and battle`, async () => {
      const game = await GameSession.launch()
      try {
        const symbols = await readSkyEmuSymbols(requireSymbolsPath())
        const client = (game as unknown as { running: RunningSkyEmu }).running.client
        const capture = async (label: string) => {
          await fs.mkdir(evidence, { recursive: true })
          await fs.writeFile(
            path.join(evidence, `style-${style}-${label}.png`),
            await game.screenshot(),
          )
        }
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: {
            appearanceStyle: style,
            facing: "up",
            position: { map: "blackthorn-city", x: 18, y: 26 },
          },
          story: { vars: { blackthornCityState: 2 } },
          party: [{ species: "lapras", level: 50, moves: ["surf"] }],
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        expect((await game.state.read()).appearance.id).toBe(ids[style - 1])
        for (const direction of ["up", "right", "down", "left"] as const) {
          await game.controls.press(direction)
          await game.wait.frames(20)
          await capture(`walking-${direction}`)
        }
        await game.controls.press("start")
        await game.wait.until((state) => state.ui.mode === "pause-menu", "open pause menu")
        await game.wait.frames(20)
        const actions = await client.readBytes(symbols.address("sCurrentStartMenuActions"), 9)
        const cardIndex = actions.indexOf(4)
        expect(cardIndex).toBeGreaterThanOrEqual(0)
        for (let n = 0; n < 9; n++) {
          const cursor = (await client.readBytes(symbols.address("sStartMenuCursorPos"), 1))[0]
          if (cursor === cardIndex) break
          await game.controls.press("down")
        }
        await game.controls.press("a")
        await game.wait.until((state) => state.ui.trainerCard === "front", "open Trainer Card")
        await game.wait.frames(30)
        await capture("trainer-card")
        await game.controls.press("b")
        await game.wait.until((state) => state.ui.mode === "pause-menu", "close Trainer Card")
        await game.wait.frames(80)
        await game.controls.press("b")
        await game.wait.forReady()
        await game.battle.startWild({ species: "pidgey", level: 2 })
        await game.wait.frames(90)
        await capture("battle-back-ready")
        await game.controls.press("a")
        for (let frame = 0; frame < 8; frame++) {
          await capture(`battle-intro-${frame}`)
          await game.wait.frames(8)
        }
        await game.wait.until(
          (state) => state.battle.ui === "action-menu",
          "trainer throw finished",
        )
        // Finish the encounter through Fight and the fixture's first move.
        await game.controls.press("a")
        await game.wait.frames(30)
        await game.controls.press("a")
        for (let n = 0; n < 100 && !(await game.state.read()).ready; n++) {
          await game.controls.press("a")
          await game.wait.frames(30)
        }
        await capture("battle-return")
        await game.wait.forReady()
        expect((await game.state.read()).appearance.id).toBe(ids[style - 1])
      } finally {
        await game.close()
      }
    })

    for (const bike of ["machBike", "acroBike"] as const) {
      it(`Style ${style}: ${bike} and restoration`, async () => {
        const game = await GameSession.launch()
        try {
          const symbols = await readSkyEmuSymbols(requireSymbolsPath())
          const client = (game as unknown as { running: RunningSkyEmu }).running.client
          const capture = async (label: string) => {
            await fs.mkdir(evidence, { recursive: true })
            await fs.writeFile(
              path.join(evidence, `style-${style}-${bike}-${label}.png`),
              await game.screenshot(),
            )
          }
          await game.arrange({
            checkpoint: "new-bark-after-intro",
            player: {
              appearanceStyle: style,
              facing: "down",
              position: { map: "blackthorn-city", x: 18, y: 26 },
            },
            story: { vars: { blackthornCityState: 2 } },
            bag: { items: { [bike]: 1 } },
            determinism: { textSpeed: "instant", rngSeed: 1 },
          })
          await game.controls.press("start")
          await game.wait.until((state) => state.ui.mode === "pause-menu", "open pause menu")
          await game.wait.frames(20)
          const actions = await client.readBytes(symbols.address("sCurrentStartMenuActions"), 9)
          const bagIndex = actions.indexOf(2)
          expect(bagIndex).toBeGreaterThanOrEqual(0)
          for (let n = 0; n < 9; n++) {
            if ((await client.readBytes(symbols.address("sStartMenuCursorPos"), 1))[0] === bagIndex)
              break
            await game.controls.press("down")
          }
          await game.controls.press("a")
          await game.wait.frames(120)
          for (let n = 0; n < 6; n++) {
            if ((await client.readBytes(symbols.address("gBagPosition") + 5, 1))[0] === 5) break
            await game.controls.press("right")
            await game.wait.frames(40)
          }
          await game.controls.press("a")
          await game.wait.frames(30)
          expect(await client.readUint16LE(symbols.address("gSpecialVar_ItemId"))).toBe(
            bike === "machBike" ? 707 : 708,
          )
          await game.controls.press("a")
          await game.wait.frames(120)
          const flag = bike === "machBike" ? 2 : 4
          expect(
            ((await client.readBytes(symbols.address("gPlayerAvatar"), 1))[0] ?? 0) & flag,
          ).toBe(flag)
          for (const direction of ["down", "left", "up", "right"] as const) {
            await game.controls.press(direction)
            await game.wait.frames(20)
            await capture(direction)
          }
          if (bike === "acroBike") {
            await client.input({ B: 1 })
            await game.wait.frames(20)
            await capture("wheelie")
            await game.wait.frames(60)
            await capture("hop")
            await client.input({ B: 0 })
            await game.wait.frames(20)
            expect(
              ((await client.readBytes(symbols.address("gPlayerAvatar"), 1))[0] ?? 0) & 6,
            ).toBe(4)
          }
          expect((await game.state.read()).appearance.id).toBe(ids[style - 1])
        } finally {
          await game.close()
        }
      })
    }
  }
})
