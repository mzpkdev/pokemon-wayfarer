import { afterAll, beforeAll, describe, expect, it } from "webanvil/test"

import { TestRom } from "../harness/test-rom"

describe.sequential("SkyEmu", () => {
  let game: TestRom | undefined

  beforeAll(async () => {
    game = await TestRom.launch()
  })

  afterAll(async () => {
    if (game) await game.close()
  })

  it("boots the supplied ROM", async () => {
    if (!game) throw new Error("Test ROM did not start")

    const state = await game.state.read()
    expect(state.frame).toBeGreaterThan(0)
  })

  it("advances emulation frames", async () => {
    if (!game) throw new Error("Test ROM did not start")
    const before = await game.state.read()

    await game.wait.frames(60)

    const after = await game.state.read()
    expect(after.frame).toBeGreaterThan(before.frame)
  })
})
