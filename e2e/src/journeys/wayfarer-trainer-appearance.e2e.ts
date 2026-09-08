import * as fs from "node:fs/promises"
import * as path from "node:path"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { playThroughNewGameIntro, reachAppearanceQuestion } from "../playbooks/new-game-intro"

const styles = [1, 2, 3, 4, 5, 6] as const

for (const origin of ["johto", "hoenn"] as const) {
  for (const style of styles) {
    describe.sequential(`Wayfarer Style ${style}, ${origin}`, () => {
      it("preserves appearance and story gender through origin initialization and Continue", async () => {
        const game = await GameSession.launch()
        try {
          await playThroughNewGameIntro(game, origin, style)
          const before = await game.state.read()
          expect(before.appearance.id).toBe(style)
          expect(before.origin.gender).toBe(style % 2 === 0 ? 1 : 0)
          expect(before.origin.id).toBe(origin === "johto" ? 1 : 2)
          expect(before.challenge.hmsOverwrite).toBe(false)
          await game.saveAndReload()
          const after = await game.state.read()
          expect(after.appearance.id).toBe(style)
          expect(after.appearance.introStage).toBe(0)
          expect(after.origin).toEqual(before.origin)
          expect(after.challenge).toEqual(before.challenge)
        } finally {
          await game.close()
        }
      })
    })
  }
}

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
        expect((await game.state.read()).appearance.candidate).toBe(style)
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
