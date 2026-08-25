import { afterAll, beforeAll, describe, expect, it } from "webanvil/test"

import { startSkyEmu, type RunningSkyEmu } from "./skyemu-server"
import { requireRomPath } from "./utils"

import "./smoke"

describe.sequential("SkyEmu", () => {
  let skyEmu: RunningSkyEmu | undefined

  beforeAll(async () => {
    skyEmu = await startSkyEmu(requireRomPath())
  })

  afterAll(async () => {
    if (skyEmu) await skyEmu.stop()
  })

  it("boots the supplied ROM", async () => {
    await expect(skyEmu?.client.health()).resolves.toEqual({ ready: true, romLoaded: true })
  })

  it("advances emulation frames", async () => {
    await expect(skyEmu?.client.step(60)).resolves.toBe("ok")
  })
})
