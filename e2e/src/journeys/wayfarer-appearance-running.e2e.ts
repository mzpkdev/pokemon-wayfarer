import * as fs from "node:fs/promises"
import * as path from "node:path"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

const styles = [1, 2, 3, 4] as const
const ids = { 1: 1, 2: 2, 3: 5, 4: 6 } as const
const directions = [
  ["down", "Down", 20],
  ["up", "Up", 21],
  ["left", "Left", 22],
  ["right", "Right", 23],
] as const
const evidence = process.env.SKYEMU_CAPTURE_DIR ?? "artifacts/trainer-appearance"

// Supply the normal running-shoes story flag through the existing fixture wire
// format. Running itself is driven by real held controller input.
describe.sequential("Wayfarer running poses and ice reflections", () => {
  for (const style of styles) {
    it(`Style ${style}: four running directions and local ice reflection`, async () => {
      const game = await GameSession.launch()
      try {
        await fs.mkdir(evidence, { recursive: true })
        const symbols = await readSkyEmuSymbols(requireSymbolsPath())
        const client = (game as unknown as { running: RunningSkyEmu }).running.client
        const capture = async (phase: string) => {
          await fs.writeFile(
            path.join(evidence, `style-${style}-running-${phase}.png`),
            await game.screenshot(),
          )
        }
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: {
            appearanceStyle: style,
            facing: "down",
            position: { map: "blackthorn-city", x: 18, y: 29 },
          },
          story: {
            flags: { runningShoes: true, disableEncounters: true },
            vars: { blackthornCityState: 2 },
          },
          party: [{ species: "lapras", level: 50, moves: ["surf"] }],
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        expect(await game.story.flag("runningShoes")).toBe(true)
        // ApplyHMsOverwriteFixture clears challenge settings after NewGame,
        // enabling autorun. Hold direction only; B would request walking.
        const samples: object[] = []
        for (const [direction, button, expectedAnim] of directions) {
          await game.player.warp("blackthorn-city", 18, 29, direction)
          const before = (await game.state.read()).player
          await client.input({ [button]: 1 })
          try {
            await game.wait.frames(5)
            for (const frame of [5, 7, 9]) {
              if (frame !== 5) await game.wait.frames(2)
              const avatar = await client.readBytes(symbols.address("gPlayerAvatar"), 6)
              const spriteId = avatar[4]!
              const anim = await client.readBytes(
                symbols.address("gSprites") + spriteId * 0x44 + 0x2a,
                2,
              )
              expect(avatar[0]! & 0x80).toBe(0x80)
              expect(anim[0]).toBe(expectedAnim)
              await capture(`${direction}-frame-${frame}`)
              samples.push({
                direction,
                frame,
                avatarFlags: avatar[0],
                animNum: anim[0],
                animCommand: anim[1],
              })
            }
          } finally {
            await client.input({ [button]: 0 })
          }
          await game.wait.frames(20)
          const after = await game.state.read()
          expect(after.appearance.id).toBe(ids[style])
          expect(after.player.x !== before.x || after.player.y !== before.y).toBe(true)
        }

        // This native Ice Path patch is passable MB_ICE (32); no map bytes are
        // changed. Its reflection comes from the engine's normal ground effects.
        await game.player.warp("ice-path-1f", 5, 7, "down")
        await game.wait.frames(30)
        const avatar = await client.readBytes(symbols.address("gPlayerAvatar"), 6)
        const reflectionCallback = symbols.address("UpdateObjectReflectionSprite") & ~1
        let reflectionCount = 0
        for (let index = 0; index < 64; index++) {
          const sprite = await client.readBytes(symbols.address("gSprites") + index * 0x44, 0x44)
          const view = new DataView(sprite.buffer, sprite.byteOffset, sprite.byteLength)
          if (
            (sprite[0x3e]! & 5) === 1 &&
            (view.getUint32(0x1c, true) & ~1) === reflectionCallback &&
            view.getUint16(0x2e, true) === avatar[5]
          )
            reflectionCount++
        }
        expect(reflectionCount).toBeGreaterThan(0)
        expect((await game.state.read()).appearance.id).toBe(ids[style])
        await capture("ice-reflection")
        await fs.writeFile(
          path.join(evidence, `style-${style}-running.json`),
          JSON.stringify(
            {
              style,
              savedAppearanceId: ids[style],
              samples,
              reflectionCount,
              automated: [
                "running-shoes-fixture",
                "fixture-autorun",
                "direction-held",
                "four-direction-running-animation-IDs",
                "midstep-captures",
                "local-visible-reflection-sprite",
              ],
              pending: ["manual-pixel-review"],
            },
            null,
            2,
          ),
        )
      } finally {
        await game.close()
      }
    })
  }
})
