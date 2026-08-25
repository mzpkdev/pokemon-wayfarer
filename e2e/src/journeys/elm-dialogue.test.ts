import { afterAll, beforeAll, describe, expect, it } from "webanvil/test"

import {
  areFieldControlsUnlocked,
  isTaskActive,
  moveUntil,
  moveTo,
  pressUntil,
  waitFor,
  waitForControlsLocked,
  waitForControlsUnlocked,
  waitForMap,
} from "../actions/field"
import { readMapLocation, readSavedVar } from "../actions/map"
import { press } from "../actions/input"
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

  it("reaches Elm's opening dialogue", async () => {
    if (!skyEmu) throw new Error("SkyEmu did not start")
    if (!symbols) throw new Error("SkyEmu symbols did not load")
    const client = skyEmu.client
    const romSymbols = symbols

    await startNewGame(client)
    await waitForControlsUnlocked(client, romSymbols)

    await moveTo(client, romSymbols, { x: 13 })
    await moveTo(client, romSymbols, { y: 12 })
    await moveTo(client, romSymbols, { x: 17 })
    await moveTo(client, romSymbols, { y: 10 })
    await press(client, "Left", 3, 1)
    await pressUntil(
      client,
      romSymbols,
      "A",
      () => isTaskActive(client, romSymbols, "Task_SetClock_HandleInput"),
      "wall-clock input",
    )
    await press(client, "A")
    await waitFor(
      client,
      romSymbols,
      () => isTaskActive(client, romSymbols, "Task_SetClock_HandleConfirmInput"),
      "wall-clock confirmation",
    )
    await press(client, "Up")
    await press(client, "A")
    await pressUntil(
      client,
      romSymbols,
      "A",
      () => areFieldControlsUnlocked(client, romSymbols),
      "unlocked field controls after setting the clock",
    )
    await waitFor(
      client,
      romSymbols,
      async () => (await readSavedVar(client, romSymbols, 0x4075)) === 1,
      "New Bark Town clock state",
    )

    await moveTo(client, romSymbols, { x: 16, y: 10 })
    await press(client, "Up", 16, 2)
    await moveTo(client, romSymbols, { x: 16, y: 9 })
    await press(client, "Left", 16, 2)
    await waitForMap(client, romSymbols, { mapGroup: 1, mapNum: 3 })
    await waitForControlsLocked(client, romSymbols)
    await waitForControlsUnlocked(client, romSymbols)

    await moveTo(client, romSymbols, { x: 17, y: 9 })
    await moveUntil(
      client,
      romSymbols,
      "Left",
      async () => !(await areFieldControlsUnlocked(client, romSymbols)),
      "Mom's introduction",
    )
    await pressUntil(
      client,
      romSymbols,
      "A",
      async () => (await readSavedVar(client, romSymbols, 0x4075)) === 2,
      "Mom's introduction",
    )
    await waitForControlsUnlocked(client, romSymbols)

    await moveTo(client, romSymbols, { x: 16, y: 14 })
    await moveUntil(
      client,
      romSymbols,
      "Down",
      async () => {
        const map = await readMapLocation(client, romSymbols)
        return map.mapGroup === 0 && map.mapNum === 0
      },
      "New Bark Town",
    )
    await waitForControlsLocked(client, romSymbols)
    await waitForControlsUnlocked(client, romSymbols)

    await moveTo(client, romSymbols, { x: 17, y: 17 })
    await moveUntil(
      client,
      romSymbols,
      "Up",
      async () => {
        const map = await readMapLocation(client, romSymbols)
        return map.mapGroup === 1 && map.mapNum === 0
      },
      "Elm's Lab",
    )
    await waitForControlsLocked(client, romSymbols)
    await waitForControlsUnlocked(client, romSymbols)

    await moveTo(client, romSymbols, { x: 13, y: 15 })
    await moveUntil(
      client,
      romSymbols,
      "Up",
      async () => !(await areFieldControlsUnlocked(client, romSymbols)),
      "Elm's opening dialogue",
    )
    await expect(readSavedVar(client, romSymbols, 0x4074)).resolves.toBe(0)
  })
})
