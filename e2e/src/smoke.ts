import * as fs from "node:fs"
import * as os from "node:os"
import * as path from "node:path"

import { afterAll, beforeAll, describe, expect, it } from "webanvil/test"

import { startSkyEmu, type RunningSkyEmu } from "./skyemu-server"
import { type SkyEmuButton, type SkyEmuClient } from "./skyemu"
import { requireRomPath } from "./utils"

const saveBlock1PointerAddress = 0x03002ef4
const saveBlock1LocationOffset = 0x8
const newBarkPlayersHouse2F = { mapGroup: 1, mapNum: 4 }

type IsolatedRom = {
  cleanup: () => Promise<void>
  path: string
}

const createIsolatedRom = async (): Promise<IsolatedRom> => {
  const directory = await fs.promises.mkdtemp(path.join(os.tmpdir(), "wayfarer-skyemu-smoke-"))
  const romPath = path.join(directory, "wayfarer.gba")
  await fs.promises.copyFile(requireRomPath(), romPath)

  return {
    cleanup: () => fs.promises.rm(directory, { force: true, recursive: true }),
    path: romPath,
  }
}

const press = async (client: SkyEmuClient, button: SkyEmuButton): Promise<void> => {
  expect(await client.input({ [button]: 1 })).toBe("ok")
  expect(await client.step(2)).toBe("ok")
  expect(await client.input({ [button]: 0 })).toBe("ok")
  expect(await client.step(2)).toBe("ok")
}

const readMapLocation = async (
  client: SkyEmuClient,
): Promise<{ mapGroup: number; mapNum: number }> => {
  const saveBlock1Address = await client.readUint32LE(saveBlock1PointerAddress)
  const location = await client.readBytes(saveBlock1Address + saveBlock1LocationOffset, 2)

  return { mapGroup: location[0]!, mapNum: location[1]! }
}

const startNewGame = async (client: SkyEmuClient): Promise<void> => {
  expect(await client.step(3_600)).toBe("ok")
  await press(client, "Start")
  expect(await client.step(240)).toBe("ok")
  await press(client, "A")

  expect(await client.step(4_800)).toBe("ok")
  for (let interaction = 0; interaction < 32; interaction++) {
    await press(client, "A")
    expect(await client.step(180)).toBe("ok")
  }
  for (let tab = 0; tab < 6; tab++) {
    await press(client, "R")
    expect(await client.step(30)).toBe("ok")
  }
  await press(client, "A")
  expect(await client.step(1_200)).toBe("ok")
  for (let interaction = 0; interaction < 24; interaction++) {
    await press(client, "A")
    expect(await client.step(300)).toBe("ok")
  }
}

describe.sequential("fresh-game smoke test", () => {
  let rom: IsolatedRom | undefined
  let skyEmu: RunningSkyEmu | undefined

  beforeAll(async () => {
    rom = await createIsolatedRom()
    skyEmu = await startSkyEmu(rom.path)
  })

  afterAll(async () => {
    if (skyEmu) await skyEmu.stop()
    if (rom) await rom.cleanup()
  })

  it("reaches the New Bark Town overworld from a new game", async () => {
    if (!skyEmu) throw new Error("SkyEmu did not start")

    await startNewGame(skyEmu.client)

    await expect(readMapLocation(skyEmu.client)).resolves.toEqual(newBarkPlayersHouse2F)
  })
})
