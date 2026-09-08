import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"
import { GameSession } from "../harness/game-session"

describe.sequential("Trainer-only surviving Rock presentation", () => {
  it("plays the wild reaction before committing the completed turn", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.battle.startWild({ species: "pidgey", level: 100 })
      for (let attempt = 0; attempt < 300; attempt++) {
        const state = await game.state.read()
        if (state.battle.ui === "action-menu") break
        if (state.battle.ui === "text") await game.controls.press("a")
        else await game.wait.frames(12)
      }
      expect((await game.state.read()).battle.cursor).toBe(0)
      await game.controls.press("a")
      for (let attempt = 0; attempt < 300; attempt++) {
        const state = await game.state.read()
        if (state.battle.trainerOnly.rocks === 1) break
        await game.wait.frames(1)
      }
      expect((await game.state.read()).battle.trainerOnly).toMatchObject({
        rocks: 1,
        anger: 25,
        completedTurns: 0,
      })
      for (let sample = 0; sample < 8; sample++) {
        await fs.promises.writeFile(
          `/tmp/trainer-only-rock-reaction-${sample}.png`,
          await game.screenshot(),
        )
        await game.wait.frames(8)
      }
      for (let attempt = 0; attempt < 300; attempt++) {
        const state = await game.state.read()
        if (!state.battle.active || state.battle.trainerOnly.completedTurns === 1) break
        await game.wait.frames(12)
      }
      const final = await game.state.read()
      expect(final.party).toHaveLength(0)
      if (final.battle.active) {
        expect(final.battle.trainerOnly).toMatchObject({ rocks: 1, completedTurns: 1 })
        await game.wait.frames(120)
        expect((await game.state.read()).battle.trainerOnly.completedTurns).toBe(1)
      }
    } finally {
      await game.close()
    }
  })
})
