import * as fs from "node:fs/promises"
import * as path from "node:path"
import { expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { reachAppearanceQuestion, selectAppearance } from "../playbooks/new-game-intro"

it("resets an abandoned May setup to unconfirmed Gold in the same emulator", async () => {
  const game = await GameSession.launch()
  try {
    await reachAppearanceQuestion(game)
    await selectAppearance(game, 4)
    await game.wait.frames(240)
    await game.controls.press("a")
    await game.wait.frames(240)
    expect((await game.state.read()).appearance).toMatchObject({ candidate: 6, confirmed: 6 })

    const client = (game as unknown as { running: RunningSkyEmu }).running.client
    expect(await client.input({ A: 1, B: 1, Start: 1, Select: 1 })).toBe("ok")
    await game.wait.frames(2)
    expect(await client.input({ A: 0, B: 0, Start: 0, Select: 0 })).toBe("ok")
    await reachAppearanceQuestion(game)
    expect((await game.state.read()).appearance).toMatchObject({ candidate: 1, confirmed: 0, introStage: 1 })
    const directory = process.env.SKYEMU_CAPTURE_DIR
    if (directory) {
      await fs.mkdir(directory, { recursive: true })
      await fs.writeFile(path.join(directory, "fresh-after-abandoned-may.png"), await game.screenshot())
    }
  } finally {
    await game.close()
  }
})
