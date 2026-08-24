import { afterAll, beforeAll, describe, expect, it } from "webanvil/test"

import { startSkyEmu, type RunningSkyEmu } from "./skyemu-server"

describe.sequential("SkyEmu", () => {
  let skyEmu: RunningSkyEmu

  beforeAll(async () => {
    skyEmu = await startSkyEmu()
  })

  afterAll(async () => {
    await skyEmu.stop()
  })

  it("boots the supplied ROM", async () => {
    await expect(skyEmu.client.ping()).resolves.toBe("pong")
    await expect(skyEmu.client.status()).resolves.toMatchObject({ "rom-loaded": true })
  })

  it("advances emulation frames", async () => {
    await expect(skyEmu.client.step(60)).resolves.toBe("ok")
  })
})
