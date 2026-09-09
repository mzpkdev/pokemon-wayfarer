import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { appearanceStyles, playThroughNewGameIntro } from "../playbooks/new-game-intro"

const styles = [1, 2, 3, 4] as const

for (const origin of ["johto", "hoenn"] as const) {
  for (const style of styles) {
    describe.sequential(`Wayfarer Style ${style}, ${origin}`, () => {
      it("preserves appearance and story gender through origin initialization and Continue", async () => {
        const game = await GameSession.launch()
        try {
          await playThroughNewGameIntro(game, origin, style)
          const before = await game.state.read()
          expect(before.appearance.id).toBe(appearanceStyles[style].id)
          expect(before.origin.gender).toBe(appearanceStyles[style].gender)
          expect(before.origin.id).toBe(origin === "johto" ? 1 : 2)
          expect(before.challenge.hmsOverwrite).toBe(false)
          await game.saveAndReload()
          const after = await game.state.read()
          expect(after.appearance.id).toBe(appearanceStyles[style].id)
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
