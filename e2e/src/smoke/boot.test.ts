import { afterAll, beforeAll, describe, expect, it } from "webanvil/test"

import { createIsolatedRom, type IsolatedRom } from "../harness/isolated-rom"
import { startSkyEmu, type RunningSkyEmu } from "../harness/skyemu-server"
import { requireRomPath } from "../harness/utils"

describe.sequential("SkyEmu", () => {
  let rom: IsolatedRom | undefined
  let skyEmu: RunningSkyEmu | undefined

  beforeAll(async () => {
    rom = await createIsolatedRom(requireRomPath())
    skyEmu = await startSkyEmu(rom.path)
  })

  afterAll(async () => {
    if (skyEmu) await skyEmu.stop()
    if (rom) await rom.cleanup()
  })

  it("boots the supplied ROM", async () => {
    await expect(skyEmu?.client.health()).resolves.toEqual({ ready: true, romLoaded: true })
  })

  it("advances emulation frames", async () => {
    await expect(skyEmu?.client.step(60)).resolves.toBe("ok")
  })
})
