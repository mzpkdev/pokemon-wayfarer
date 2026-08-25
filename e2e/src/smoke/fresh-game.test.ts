import { beforeAll, describe, expect, it } from "webanvil/test"

import { TestRom } from "../harness/test-rom"
import { startNewGame } from "../playbooks/new-game"

describe.sequential("fresh-game smoke test", () => {
  let game: TestRom

  beforeAll(async () => {
    game = await TestRom.launch()
    return () => game.close()
  })

  it("reaches the New Bark Town overworld from a new game", async () => {
    await startNewGame(game)
    await game.wait.forMap("players-bedroom")

    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { mapGroup: 1, mapNum: 4 },
    })
  })
})
