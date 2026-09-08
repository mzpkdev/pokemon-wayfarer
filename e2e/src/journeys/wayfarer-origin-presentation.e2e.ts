import * as fs from "node:fs"
import * as path from "node:path"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { reachOriginQuestion, waitForOriginStage } from "../playbooks/new-game-intro"

describe.sequential("Oak's origin presentation", () => {
  it("captures the ordinary origin list and Hoenn town confirmation", async () => {
    const game = await GameSession.launch()
    try {
      const directory = process.env.E2E_ARTIFACT_DIR ?? "/tmp/wayfarer-origin-presentation"
      await fs.promises.mkdir(directory, { recursive: true })
      await reachOriginQuestion(game)
      await fs.promises.writeFile(
        path.join(directory, "oak-origin-question.png"),
        await game.screenshot(),
      )
      await game.controls.press("down")
      await game.controls.press("a")
      await waitForOriginStage(game, 2)
      await fs.promises.writeFile(
        path.join(directory, "oak-hoenn-confirmation.png"),
        await game.screenshot(),
      )
      expect((await game.state.read()).origin.introStage).toBe(2)
    } finally {
      await game.close()
    }
  })
})
