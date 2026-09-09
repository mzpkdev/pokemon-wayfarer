import * as fs from "node:fs/promises"
import * as path from "node:path"
import { beforeAll, beforeEach, afterEach, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols, type SkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

const styles = [1, 2, 3, 4] as const
const ids = { 1: 1, 2: 2, 3: 5, 4: 6 } as const
const evidenceDirectory = process.env.SKYEMU_CAPTURE_DIR ?? "artifacts/trainer-appearance"

describe.sequential("Wayfarer appearance Surf and fishing", () => {
  let game: GameSession
  let symbols: SkyEmuSymbols
  let expectedAppearance: number

  beforeAll(async () => {
    await fs.mkdir(evidenceDirectory, { recursive: true })
    symbols = await readSkyEmuSymbols(requireSymbolsPath())
  })
  beforeEach(async () => {
    game = await GameSession.launch()
  })
  afterEach(async () => {
    await game?.close()
  })

  const capture = async (name: string) => {
    expect((await game.state.read()).appearance.id).toBe(expectedAppearance)
    await fs.writeFile(path.join(evidenceDirectory, `${name}.png`), await game.screenshot())
  }

  const fish = async (label: string) => {
    const client = (game as unknown as { running: RunningSkyEmu }).running.client
    const byte = async (symbol: string, offset = 0) =>
      (await client.readBytes(symbols.address(symbol) + offset, 1))[0]
    await game.controls.press("start")
    await game.wait.until((state) => state.ui.mode === "pause-menu", "open pause menu")
    await game.wait.frames(30)
    const actions = await client.readBytes(symbols.address("sCurrentStartMenuActions"), 9)
    const bagIndex = actions.indexOf(2)
    expect(bagIndex).toBeGreaterThanOrEqual(0)
    for (let tries = 0; tries < 9 && (await byte("sStartMenuCursorPos")) !== bagIndex; tries++) {
      await game.controls.press("down")
    }
    expect(await byte("sStartMenuCursorPos")).toBe(bagIndex)
    await game.controls.press("a")
    await game.wait.frames(120)
    for (let tries = 0; tries < 6 && (await byte("gBagPosition", 5)) !== 5; tries++) {
      await game.controls.press("right")
      await game.wait.frames(40)
    }
    expect(await byte("gBagPosition", 5)).toBe(5)
    await capture(`${label}-rod-bag`)
    await game.controls.press("a")
    await game.wait.frames(30)
    expect(await client.readUint16LE(symbols.address("gSpecialVar_ItemId"))).toBe(709)
    await game.controls.press("a")
    const isFishing = async () => {
      for (let offset = 0; offset < 16 * 40; offset += 40) {
        const tasks = await client.readBytes(symbols.address("gTasks") + offset, 8)
        const view = new DataView(tasks.buffer, tasks.byteOffset, tasks.byteLength)
        if (tasks[4] && (view.getUint32(0, true) & ~1) === (symbols.address("Task_Fishing") & ~1))
          return true
      }
      return false
    }
    for (let tries = 0; tries < 30 && !(await isFishing()); tries++) await game.wait.frames(10)
    expect(await isFishing()).toBe(true)
    await game.wait.frames(45)
    await capture(`${label}-fishing`)
    for (const frame of [90, 135]) {
      await game.wait.frames(45)
      expect(await isFishing()).toBe(true)
      await capture(`${label}-fishing-${frame}`)
    }
    // Allow a missed bite, then dismiss the message or confirm a bite that has no timeout.
    await game.wait.frames(900)
    for (let tries = 0; tries < 12 && (await isFishing()); tries++) {
      await game.controls.press("a")
      await game.wait.frames(60)
    }
    expect(await isFishing()).toBe(false)
    // Some challenge settings wait indefinitely for a bite confirmation. If
    // confirming it starts a wild encounter, leave through the ordinary Run UI.
    for (let attempts = 0; attempts < 240; attempts++) {
      const state = await game.state.read()
      if (state.ready && !state.battle.active) break
      if (state.battle.ui === "action-menu") {
        const cursor = state.battle.cursor ?? 0
        if ((cursor & 1) === 0) await game.controls.press("right")
        if ((cursor & 2) === 0) await game.controls.press("down")
        await game.controls.press("a")
      } else if (state.battle.ui === "text") await game.controls.press("a")
      await game.wait.frames(15)
    }
    await game.wait.forReady()
    expect((await game.state.read()).player.surfing).toBe(true)
    await capture(`${label}-fishing-ended`)
  }

  const moveTile = async (direction: Direction) => {
    const before = (await game.state.read()).player
    for (let attempt = 0; attempt < 3; attempt++) {
      await game.player.move(direction)
      await game.wait.frames(30)
      const after = (await game.state.read()).player
      if (after.x !== before.x || after.y !== before.y) return
    }
    throw new Error(`Did not move ${direction} from ${before.x},${before.y}`)
  }

  const mount = async () => {
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await game.wait.until(
      (state) => state.dialogue.message === "want-to-use-surf" && !state.dialogueOpen,
      "Surf prompt",
    )
    await game.wait.frames(12)
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.dialogue.message === "player-used-surf",
      "Surf confirmation",
    )
    await game.dialogue.waitForClosed()
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.ready && state.player.surfing && state.player.y === 25,
      "enter Blackthorn pool",
      3_600,
    )
  }

  for (const style of styles) {
    it(`Style ${style}: Surf entry, four directions, bobbing, fishing and dismount`, async () => {
      const label = `style-${style}-water`
      expectedAppearance = ids[style]
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          appearanceStyle: style,
          facing: "up",
          position: { map: "blackthorn-city", x: 18, y: 26 },
        },
        story: { vars: { blackthornCityState: 2 } },
        party: [{ species: "lapras", level: 50, moves: ["surf"] }],
        bag: { items: { oldRod: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      try {
        await capture(`${label}-before-entry`)
        await mount()
        await expect(game.state.read()).resolves.toMatchObject({
          appearance: { id: expectedAppearance },
          fieldMove: { move: "surf", user: 0, userSpecies: "lapras" },
          player: { surfing: true },
        })
        await capture(`${label}-entry`)
        await moveTile("up")
        await game.wait.forReady()
        await game.wait.frames(20)
        for (const direction of ["left", "up", "right", "down"] as const) {
          await moveTile(direction)
          await game.wait.forReady()
          await game.wait.frames(20)
          expect((await game.state.read()).player.surfing).toBe(true)
          await capture(`${label}-${direction}`)
        }
        await capture(`${label}-bob-0`)
        await game.wait.frames(8)
        await capture(`${label}-bob-8`)
        await game.wait.frames(8)
        await capture(`${label}-bob-16`)
        // Lapras's forward overlay obscures the down-facing trainer. Cast to
        // the right so the captured fishing pose remains visible beside it.
        await moveTile("right")
        await game.wait.forReady()
        await fish(label)
        await moveTile("left")
        for (let steps = 0; steps < 8 && (await game.state.read()).player.surfing; steps++) {
          await game.player.move("down")
          await game.wait.frames(30)
        }
        await game.wait.until((state) => state.ready && !state.player.surfing, "leave pool")
        await capture(`${label}-dismount`)
        await expect(game.state.read()).resolves.toMatchObject({
          appearance: { id: expectedAppearance },
          player: { x: 18, y: 26, surfing: false },
        })
        await fs.writeFile(
          path.join(evidenceDirectory, `${label}.json`),
          JSON.stringify(
            {
              style,
              savedAppearanceId: expectedAppearance,
              state: await game.state.read(),
              automated: [
                "surf-entry",
                "four-direction-movement",
                "bobbing-wait",
                "fishing",
                "dismount",
                "appearance-preserved-at-every-capture",
              ],
              pending: [
                "manual-image-review",
                "dive",
                "watering",
                "field-move-poses",
                "reflections",
              ],
            },
            null,
            2,
          ),
        )
      } catch (error) {
        await fs.writeFile(
          path.join(evidenceDirectory, `${label}-failure.json`),
          JSON.stringify(
            {
              style,
              savedAppearanceId: expectedAppearance,
              state: await game.state.read(),
              error: error instanceof Error ? error.message : String(error),
            },
            null,
            2,
          ),
        )
        await capture(`${label}-failure`)
        throw error
      }
    })
  }
})
