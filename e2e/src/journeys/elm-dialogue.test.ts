import { afterAll, beforeAll, describe, expect, it } from "webanvil/test"

import { advance, walk } from "../actions/input"
import { readMapLocation, readPlayerPosition } from "../actions/map"
import { createIsolatedRom, type IsolatedRom } from "../harness/isolated-rom"
import { startSkyEmu, type RunningSkyEmu } from "../harness/skyemu-server"
import { readSkyEmuSymbols, type SkyEmuSymbols } from "../harness/symbols"
import { requireRomPath, requireSymbolsPath } from "../harness/utils"
import { startNewGame } from "../playbooks/new-game"

describe.sequential("Elm dialogue journey", () => {
  let rom: IsolatedRom | undefined
  let skyEmu: RunningSkyEmu | undefined
  let symbols: SkyEmuSymbols | undefined

  beforeAll(async () => {
    rom = await createIsolatedRom(requireRomPath())
    symbols = await readSkyEmuSymbols(requireSymbolsPath())
    skyEmu = await startSkyEmu(rom.path)
  })

  afterAll(async () => {
    if (skyEmu) await skyEmu.stop()
    if (rom) await rom.cleanup()
  })

  it("sets the clock", async () => {
    if (!skyEmu) throw new Error("SkyEmu did not start")
    if (!symbols) throw new Error("SkyEmu symbols did not load")

    await startNewGame(skyEmu.client)
    await advance(skyEmu.client, 600)

    await expect(readMapLocation(skyEmu.client, symbols)).resolves.toEqual({
      mapGroup: 1,
      mapNum: 4,
    })
    await expect(readPlayerPosition(skyEmu.client, symbols)).resolves.toEqual({ x: 8, y: 13 })

    await walk(skyEmu.client, "Right", 5)
    await walk(skyEmu.client, "Up", 1)
    await walk(skyEmu.client, "Right", 4)
    await walk(skyEmu.client, "Up", 2)
    await expect(readPlayerPosition(skyEmu.client, symbols)).resolves.toEqual({ x: 17, y: 10 })

    await walk(skyEmu.client, "Left", 1)
    await advance(skyEmu.client, 600)
    for (let interaction = 0; interaction < 12; interaction++) {
      await skyEmu.client.input({ A: 1 })
      await advance(skyEmu.client, 2)
      await skyEmu.client.input({ A: 0 })
      await advance(skyEmu.client, 240)
    }
    await skyEmu.client.input({ A: 1 })
    await advance(skyEmu.client, 2)
    await skyEmu.client.input({ A: 0 })
    await advance(skyEmu.client, 240)
    await skyEmu.client.input({ Up: 1 })
    await advance(skyEmu.client, 2)
    await skyEmu.client.input({ Up: 0 })
    await advance(skyEmu.client, 2)
    await skyEmu.client.input({ A: 1 })
    await advance(skyEmu.client, 2)
    await skyEmu.client.input({ A: 0 })
    await advance(skyEmu.client, 600)
    for (let interaction = 0; interaction < 4; interaction++) {
      await skyEmu.client.input({ A: 1 })
      await advance(skyEmu.client, 2)
      await skyEmu.client.input({ A: 0 })
      await advance(skyEmu.client, 300)
    }
    await walk(skyEmu.client, "Left", 1)
    await walk(skyEmu.client, "Down", 1)
    await skyEmu.client.input({ Up: 1 })
    await advance(skyEmu.client, 32)
    await skyEmu.client.input({ Up: 0 })
    await advance(skyEmu.client, 600)

    await expect(readMapLocation(skyEmu.client, symbols)).resolves.toEqual({
      mapGroup: 1,
      mapNum: 3,
    })
  })
})
