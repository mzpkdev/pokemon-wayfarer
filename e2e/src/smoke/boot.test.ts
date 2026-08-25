import { beforeAll, describe, expect, it } from "webanvil/test"

import { TestRom } from "../harness/test-rom"

describe.sequential("SkyEmu", () => {
  let game: TestRom

  beforeAll(async () => {
    game = await TestRom.launch()
    return () => game.close()
  })

  it("boots the supplied ROM", async () => {
    const state = await game.state.read()
    expect(state.frame).toBeGreaterThan(0)
  })

  it("advances emulation frames", async () => {
    const before = await game.state.read()

    await game.wait.frames(60)

    const after = await game.state.read()
    expect(after.frame).toBeGreaterThan(before.frame)
  })
})
