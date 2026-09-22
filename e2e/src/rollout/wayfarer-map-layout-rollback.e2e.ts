import * as crypto from "node:crypto"
import * as fs from "node:fs"
import * as path from "node:path"

import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

const mode = process.env.MAP_LAYOUT_ROLLBACK_MODE
const savePath = process.env.MAP_LAYOUT_ROLLBACK_SAVE
const recordPath = process.env.MAP_LAYOUT_ROLLBACK_RECORD
const sourceRevision = process.env.MAP_LAYOUT_SOURCE_REVISION

const requireConfiguration = (): {
  mode: "create" | "verify"
  recordPath: string
  savePath: string
  sourceRevision: string
} => {
  if (mode !== "create" && mode !== "verify")
    throw new Error("MAP_LAYOUT_ROLLBACK_MODE must be create or verify")
  if (!savePath || !recordPath || !sourceRevision)
    throw new Error(
      "MAP_LAYOUT_ROLLBACK_SAVE, MAP_LAYOUT_ROLLBACK_RECORD, and MAP_LAYOUT_SOURCE_REVISION are required",
    )
  return { mode, recordPath, savePath, sourceRevision }
}

const sha256 = async (file: string): Promise<string> =>
  crypto.createHash("sha256").update(await fs.promises.readFile(file)).digest("hex")

const readRecord = async (file: string): Promise<Record<string, unknown>> =>
  JSON.parse(await fs.promises.readFile(file, "utf8")) as Record<string, unknown>

const writeRecord = async (file: string, value: Record<string, unknown>): Promise<void> => {
  await fs.promises.mkdir(path.dirname(file), { recursive: true })
  await fs.promises.writeFile(file, `${JSON.stringify(value, null, 2)}\n`)
}

const rolloutDescribe = mode === undefined ? describe.skip : describe.sequential

rolloutDescribe("Wayfarer map-layout raw rollback drill", () => {
  it("continues a hybrid canary save in the raw-control ROM", async () => {
    const config = requireConfiguration()
    const romPath = process.env.SKYEMU_ROM
    if (!romPath) throw new Error("SKYEMU_ROM is required")
    if (config.mode === "create") {
      const game = await GameSession.launch()
      let exported = false
      try {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: {
            facing: "down",
            position: { map: "olivine-cafe", x: 3, y: 6 },
          },
          secretBase: { decorated: true },
          determinism: { textSpeed: "instant" },
        })
        await game.saveAndReload()
        const state = await game.state.read()
        expect(state).toMatchObject({
          ready: true,
          map: { name: "olivine-cafe" },
          player: { x: 3, y: 6 },
          secretBase: { decorationCount: 2 },
        })
        await game.close(config.savePath)
        exported = true
        await writeRecord(config.recordPath, {
          schema_version: 1,
          source_revision: config.sourceRevision,
          roms: { hybrid: { rom_sha256: await sha256(romPath) } },
          hybrid_save: {
            map: "olivine-cafe",
            player: { x: 3, y: 6 },
            save_sha256: await sha256(config.savePath),
          },
          decorated_secret_base: {
            map: "secret-base-red-cave-1",
            decoration_count: state.secretBase.decorationCount,
            decoration_fingerprint: state.secretBase.decorationFingerprint,
            save_sha256: await sha256(config.savePath),
          },
        })
      } finally {
        if (!exported) await game.close()
      }
      return
    }

    const record = await readRecord(config.recordPath)
    expect(record).toMatchObject({
      schema_version: 1,
      source_revision: config.sourceRevision,
      roms: { hybrid: { rom_sha256: expect.any(String) } },
      hybrid_save: { map: "olivine-cafe", player: { x: 3, y: 6 } },
    })
    const hybridSave = record.hybrid_save as { save_sha256: string }
    const decoratedSecretBase = record.decorated_secret_base as {
      decoration_count: number
      decoration_fingerprint: number
      save_sha256: string
    }
    expect(await sha256(config.savePath)).toBe(hybridSave.save_sha256)
    expect(await sha256(config.savePath)).toBe(decoratedSecretBase.save_sha256)

    const game = await GameSession.launch(config.savePath)
    try {
      await game.continueSavedGame()
      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: "olivine-cafe" },
        player: { x: 3, y: 6 },
        secretBase: {
          decorationCount: decoratedSecretBase.decoration_count,
          decorationFingerprint: decoratedSecretBase.decoration_fingerprint,
        },
      })
      await game.saveAndReload()
      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: "olivine-cafe" },
        player: { x: 3, y: 6 },
        secretBase: {
          decorationCount: decoratedSecretBase.decoration_count,
          decorationFingerprint: decoratedSecretBase.decoration_fingerprint,
        },
      })
    } finally {
      await game.close()
    }
    await writeRecord(config.recordPath, {
      ...record,
      roms: {
        ...(record.roms as Record<string, unknown>),
        raw: { rom_sha256: await sha256(romPath) },
      },
      raw_continue: {
        map: "olivine-cafe",
        player: { x: 3, y: 6 },
        resaved_and_reloaded: true,
      },
      raw_decorated_secret_base_continue: {
        map: "secret-base-red-cave-1",
        decoration_count: decoratedSecretBase.decoration_count,
        decoration_fingerprint: decoratedSecretBase.decoration_fingerprint,
        resaved_and_reloaded: true,
      },
    })
  })
})
