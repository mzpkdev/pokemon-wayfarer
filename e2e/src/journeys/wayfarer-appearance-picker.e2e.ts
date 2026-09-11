import * as fs from "node:fs/promises"
import * as path from "node:path"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { appearanceStyles, reachAppearanceQuestion, selectAppearance, advanceToOriginQuestion, finishOriginIntroduction } from "../playbooks/new-game-intro"

const styles = [1, 2, 3, 4] as const

describe.sequential("Wayfarer appearance picker", () => {
  it("keeps B and horizontal input on the list, wraps, and captures each preview", async () => {
    const game = await GameSession.launch()
    try {
      await reachAppearanceQuestion(game)
      expect((await game.state.read()).appearance).toMatchObject({ candidate: 1, confirmed: 0 })
      for (const button of ["b", "left", "right"] as const) {
        await game.controls.press(button)
        expect((await game.state.read()).appearance).toMatchObject({
          candidate: 1,
          introStage: 1,
          confirmed: 0,
        })
      }
      await game.controls.press("down", { holdFrames: 1, releaseFrames: 0 })
      await game.controls.press("a", { holdFrames: 30 })
      expect((await game.state.read()).appearance).toMatchObject({ candidate: 2, confirmed: 0 })
      await game.controls.press("up")
      await game.wait.until((state) => state.appearance.introStage === 1, "return preview", 600)
      await game.controls.press("up")
      await game.wait.until((state) => state.appearance.introStage === 1, "wrap preview", 600)
      expect((await game.state.read()).appearance.candidate).toBe(6)
      for (const style of styles) {
        await game.controls.press("down")
        await game.wait.until((state) => state.appearance.introStage === 1, "style preview", 600)
        expect((await game.state.read()).appearance.candidate).toBe(appearanceStyles[style].id)
        const directory = process.env.SKYEMU_CAPTURE_DIR
        if (directory) {
          await fs.mkdir(directory, { recursive: true })
          await fs.writeFile(
            path.join(directory, `appearance-style-${style}.png`),
            await game.screenshot(),
          )
        }
      }
      await game.controls.press("a")
      expect((await game.state.read()).appearance.confirmed).toBe(6)
    } finally {
      await game.close()
    }
  })
})


describe.sequential("Wayfarer appearance naming lifecycle", () => {
  it("restores the selected style after name rejection and keeps a replacement through callbacks", async () => {
    const game = await GameSession.launch()
    const capture = async (name: string) => {
      const directory = process.env.SKYEMU_CAPTURE_DIR
      if (directory) {
        await fs.mkdir(directory, { recursive: true })
        await fs.writeFile(path.join(directory, `${name}.png`), await game.screenshot())
      }
    }
    try {
      await reachAppearanceQuestion(game)
      await selectAppearance(game, 4)
      await game.wait.frames(240)
      await game.controls.press("a")
      await game.wait.frames(240)
      await capture("naming-may")
      await game.controls.press("start")
      await game.controls.press("a")
      await game.wait.frames(240)
      await capture("name-confirmation-may")
      await game.controls.press("b")
      await game.wait.until((state) => state.appearance.introStage === 1, "appearance after name rejection", 1200)
      expect((await game.state.read()).appearance).toMatchObject({ candidate: 6, confirmed: 6 })
      await capture("name-rejection-may")
      await selectAppearance(game, 3)
      await advanceToOriginQuestion(game)
      expect((await game.state.read()).appearance).toMatchObject({ candidate: 5, confirmed: 5 })
      await capture("challenge-return-brendan")
      await game.controls.press("a")
      await finishOriginIntroduction(game, "johto")
      expect((await game.state.read()).appearance.id).toBe(5)
    } finally { await game.close() }
  })
})
