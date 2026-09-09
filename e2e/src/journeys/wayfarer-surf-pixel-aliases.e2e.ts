import * as fs from "node:fs/promises"
import * as path from "node:path"
import { beforeAll, beforeEach, afterEach, describe, expect, it } from "webanvil/test"

import { GameSession, species, type FixtureSpecies, type Direction } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols, type SkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

const representatives = [
  ["squirtle", 7],
  ["raichuAlola", 958],
  ["wartortle", 8],
  ["qwilfishHisui", 998],
  ["lugia", 249],
  ["gyarados", 130],
] as const

const selectedCase = process.env.SURF_ALIAS_CASE
const caseNames = representatives.flatMap(([name]) => [`${name}-normal`, `${name}-shiny`])
if (selectedCase && !caseNames.includes(selectedCase)) {
  throw new Error(`Unknown SURF_ALIAS_CASE: ${selectedCase}`)
}

// These fixture names stay local to this driver; the ROM accepts numerical species.
const fixtureSpecies = species as Record<string, number>
const evidenceDirectory = process.env.SURF_ALIAS_EVIDENCE ?? "/tmp/surf-pixel-alias-runtime"

describe.sequential("Wayfarer representative Surf pixel aliases", () => {
  let game: GameSession
  let partyAddress: number
  let symbols: SkyEmuSymbols

  beforeAll(async () => {
    const previous = new Map(representatives.map(([name]) => [name, fixtureSpecies[name]]))
    await fs.mkdir(evidenceDirectory, { recursive: true })
    symbols = await readSkyEmuSymbols(requireSymbolsPath())
    partyAddress = symbols.address("gPlayerParty")
    for (const [name, id] of representatives) fixtureSpecies[name] = id
    return async () => {
      for (const [name, value] of previous) {
        if (value === undefined) delete fixtureSpecies[name]
        else fixtureSpecies[name] = value
      }
    }
  })

  beforeEach(async () => {
    game = await GameSession.launch()
  })
  afterEach(async () => {
    await game?.close()
  })

  const capture = async (name: string) => {
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

  for (const [name] of representatives) {
    for (const shiny of [false, true]) {
      if (
        process.env.SURF_ALIAS_CASE &&
        process.env.SURF_ALIAS_CASE !== `${name}-${shiny ? "shiny" : "normal"}`
      )
        continue
      it(`${name} ${shiny ? "shiny" : "normal"}: enters, moves, bobs, dismounts and remounts`, async () => {
        const label = `${name}-${shiny ? "shiny" : "normal"}`
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: { facing: "up", position: { map: "blackthorn-city", x: 18, y: 26 } },
          // This is a presentation journey. Suppress random water encounters so
          // its movement, bobbing, fishing animation, dismount, and connection
          // assertions cannot be interrupted by unrelated battle routing.
          story: { flags: { disableEncounters: true }, vars: { blackthornCityState: 2 } },
          party: [{ species: name as FixtureSpecies, level: 30, moves: ["surf"] }],
          bag: { items: { oldRod: 1 } },
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })

        // Fixture slot zero has PID/OT zero (shiny). PID 24 preserves the secure
        // substructure order; rekeying all twelve words preserves its checksum.
        const client = (game as unknown as { running: RunningSkyEmu }).running.client
        const box = await client.readBytes(partyAddress, 80)
        const view = new DataView(box.buffer, box.byteOffset, box.byteLength)
        expect(view.getUint32(0, true)).toBe(0)
        expect(view.getUint32(4, true)).toBe(0)
        if (!shiny) {
          view.setUint32(0, 24, true)
          for (let offset = 32; offset < 80; offset += 4) {
            view.setUint32(offset, (view.getUint32(offset, true) ^ 24) >>> 0, true)
          }
          await client.writeBytes(partyAddress, box)
        }

        const patched = await client.readBytes(partyAddress, 80)
        const patchedView = new DataView(patched.buffer, patched.byteOffset, patched.byteLength)
        const pid = patchedView.getUint32(0, true)
        const ot = patchedView.getUint32(4, true)
        const shinyValue = (pid >>> 16) ^ (pid & 0xffff) ^ (ot >>> 16) ^ (ot & 0xffff)
        const shinyModifier = (patchedView.getUint16(30, true) >>> 14) & 1
        expect(Boolean(Number(shinyValue < 8) ^ shinyModifier)).toBe(shiny)

        try {
          await mount()
          await expect(game.state.read()).resolves.toMatchObject({
            fieldMove: { move: "surf", user: 0, userSpecies: name },
            party: [{ species: name }],
          })
          await capture(`${label}-entry`)
          // First leave the bank, then capture each direction once on open water.
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
          if (process.env.SURF_ALIAS_FISHING !== "0") await fish(label)

          for (let steps = 0; steps < 8 && (await game.state.read()).player.surfing; steps++) {
            await game.player.move("down")
            await game.wait.frames(30)
          }
          await game.wait.until((state) => state.ready && !state.player.surfing, "leave pool")
          await capture(`${label}-dismount`)
          await expect(game.state.read()).resolves.toMatchObject({
            player: { x: 18, y: 26, surfing: false },
          })
          await game.player.move("up")
          await game.wait.frames(20)
          await mount()
          await capture(`${label}-remount`)
          if (process.env.SURF_ALIAS_TRANSITION !== "0") {
            // The fixture warp only positions us; the next input crosses the real connection.
            await game.player.warp("route-40", 18, 70, "down")
            expect((await game.state.read()).player.surfing).toBe(true)
            await capture(`${label}-before-transition`)
            await moveTile("down")
            await game.wait.until(
              (state) => state.ready && state.map.name === "route-41",
              "Surf across Route 40/41 connection",
            )
            expect((await game.state.read()).player.surfing).toBe(true)
            await capture(`${label}-after-transition`)
          }
          await fs.writeFile(
            path.join(evidenceDirectory, `${label}.json`),
            JSON.stringify(
              {
                label,
                state: await game.state.read(),
                automated: [
                  "entry",
                  "four-direction-movement",
                  "bobbing-wait",
                  "dismount",
                  "remount",
                ],
                pending: [
                  "visual-review",
                  "baseline-comparison",
                  ...(process.env.SURF_ALIAS_FISHING !== "0" ? [] : ["fishing"]),
                  ...(process.env.SURF_ALIAS_TRANSITION !== "0" ? [] : ["map-transition"]),
                ],
              },
              null,
              2,
            ),
          )
        } catch (error) {
          await capture(`${label}-failure`)
          throw error
        }
      })
    }
  }
})
