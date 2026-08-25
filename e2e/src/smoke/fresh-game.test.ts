import { afterAll, beforeAll, describe, expect, it } from "webanvil/test"

import { newBarkPlayersHouse2F, readMapLocation } from "../actions/map"
import { createIsolatedRom, type IsolatedRom } from "../harness/isolated-rom"
import { startSkyEmu, type RunningSkyEmu } from "../harness/skyemu-server"
import { readSkyEmuSymbols, type SkyEmuSymbols } from "../harness/symbols"
import { requireRomPath, requireSymbolsPath } from "../harness/utils"
import { startNewGame } from "../playbooks/new-game"

describe.sequential("fresh-game smoke test", () => {
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

  it("reaches the New Bark Town overworld from a new game", async () => {
    if (!skyEmu) throw new Error("SkyEmu did not start")
    if (!symbols) throw new Error("SkyEmu symbols did not load")

    await startNewGame(skyEmu.client)

    await expect(readMapLocation(skyEmu.client, symbols)).resolves.toEqual(newBarkPlayersHouse2F)
  })
})
