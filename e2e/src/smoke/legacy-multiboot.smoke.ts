import { beforeAll, describe, expect, it } from "webanvil/test"

import type { SkyEmuInput } from "../harness/skyemu/client"
import { createIsolatedRom } from "../harness/skyemu/isolated-rom"
import { startSkyEmu, type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols, type SkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireRomPath, requireSymbolsPath } from "../harness/skyemu/utils"

describe.sequential("Wayfarer startup without legacy multiboot", () => {
  let running: RunningSkyEmu
  let symbols: SkyEmuSymbols

  // Task layout is defined in game/include/task.h: 16 entries of 40 bytes.
  const taskAddress = async (name: string): Promise<number | undefined> => {
    const expected = symbols.address(name) & ~1
    const base = symbols.address("gTasks")
    const bytes = new Uint8Array(16 * 40)
    for (let offset = 0; offset < bytes.length; offset += 160)
      bytes.set(await running.client.readBytes(base + offset, 160), offset)
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength)
    for (let offset = 0; offset < bytes.length; offset += 40) {
      if (bytes[offset + 4] && (view.getUint32(offset, true) & ~1) === expected)
        return base + offset
    }
    return undefined
  }

  const waitForTask = async (name: string, limit = 1200): Promise<number> => {
    for (let elapsed = 0; elapsed < limit; elapsed += 10) {
      const address = await taskAddress(name)
      if (address !== undefined) return address
      await running.client.step(10)
    }
    throw new Error(`Startup did not reach ${name} within ${limit} frames`)
  }

  const press = async (inputs: SkyEmuInput, frames = 2): Promise<void> => {
    expect(await running.client.input(inputs)).toBe("ok")
    try {
      await running.client.step(frames)
    } finally {
      const released: SkyEmuInput = {}
      for (const button of Object.keys(inputs) as (keyof SkyEmuInput)[]) released[button] = 0
      expect(await running.client.input(released)).toBe("ok")
    }
    await running.client.step(2)
  }

  beforeAll(async () => {
    const rom = await createIsolatedRom(requireRomPath())
    try {
      symbols = await readSkyEmuSymbols(requireSymbolsPath())
      running = await startSkyEmu(rom.path)
    } catch (error) {
      await rom.cleanup()
      throw error
    }
    return async () => {
      await running.stop()
      await rom.cleanup()
    }
  })

  it("passes copyright and leaves B+Select inactive on the title screen", async () => {
    await waitForTask("Task_TitleScreenPhase3", 12_000)
    const callback = await running.client.readUint32LE(symbols.address("gMain") + 4)
    await press({ B: 1, Select: 1 }, 60)
    expect(await taskAddress("Task_TitleScreenPhase3")).toBeDefined()
    expect(await running.client.readUint32LE(symbols.address("gMain") + 4)).toBe(callback)
    expect((await running.client.readUint32LE(symbols.address("gMain") + 0x18)) & ~1).toBe(
      symbols.address("SerialCB") & ~1,
    )
  })

  it("keeps the disabled reset-RTC chord inactive on a fresh save", async () => {
    await press({ B: 1, Select: 1, Left: 1 }, 30)
    expect(await taskAddress("Task_TitleScreenPhase3")).toBeDefined()
  })

  it("opens Options from the main menu and returns", async () => {
    await press({ A: 1 })
    for (let attempt = 0; attempt < 120; attempt++) {
      if ((await taskAddress("Task_HandleMainMenuInput")) !== undefined) break
      if (
        (await taskAddress("Task_WaitForSaveFileErrorWindow")) !== undefined ||
        (await taskAddress("Task_WaitForBatteryDryErrorWindow")) !== undefined
      )
        await press({ A: 1 })
      await running.client.step(10)
    }
    const menu = await waitForTask("Task_HandleMainMenuInput")
    expect(await running.client.readUint16LE(menu + 8)).toBe(0)
    await press({ Down: 1 })
    expect(await running.client.readUint16LE(menu + 10)).toBe(1)
    await press({ A: 1 })
    await running.client.step(180)
    expect(await taskAddress("Task_HandleMainMenuInput")).toBeUndefined()
    expect((await running.client.readUint32LE(symbols.address("gMain") + 8)) & ~1).toBe(
      symbols.address("CB2_ReinitMainMenu") & ~1,
    )
    await press({ B: 1 })
    await waitForTask("Task_HandleMainMenuInput")
    await press({ B: 1 })
    await waitForTask("Task_TitleScreenPhase3")
  })

  it("retains the clear-save chord and allows cancelling its prompt", async () => {
    await press({ B: 1, Select: 1, Up: 1 })
    await waitForTask("Task_ClearSaveDataScreenYesNoChoice")
    await press({ B: 1 })
    await waitForTask("Task_TitleScreenPhase3", 12_000)
  })
})
