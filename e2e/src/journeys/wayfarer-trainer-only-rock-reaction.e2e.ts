import { arrangeTrainerOnly } from "../playbooks/trainer-only-scenario"
import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"
import { GameSession } from "../harness/game-session"

describe.sequential("Trainer-only surviving Rock presentation", () => {
  it("shows native impact and anger motion before committing the completed turn", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeTrainerOnly(game, {
        checkpoint: "new-bark-after-intro",
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      // A level-five target takes the Rock helper's visible 20% cap, making
      // the native health-bar drain reviewable without a test-only HP reader.
      await game.battle.startWild({ species: "pidgey", level: 5 })
      for (let attempt = 0; attempt < 300; attempt++) {
        const state = await game.state.read()
        if (state.battle.ui === "action-menu") break
        if (state.battle.ui === "text") await game.controls.press("a")
        else await game.wait.frames(12)
      }
      expect((await game.state.read()).battle.cursor).toBe(0)
      await fs.promises.writeFile(
        "/tmp/trainer-only-rock-before-impact.png",
        await game.screenshot(),
      )
      await game.controls.press("a")

      // A Rock first plays the existing impact flash and native health-bar
      // drain while state is still uncommitted. Retain sampled frames as visual
      // evidence during that interval.
      for (let attempt = 0; attempt < 300; attempt++) {
        const state = await game.state.read()
        if (state.battle.trainerOnly.rocks === 1) break
        expect(state.battle.trainerOnly.completedTurns).toBe(0)
        if (attempt % 6 === 0) {
          const screenshot = await game.screenshot()
          await fs.promises.writeFile(
            `/tmp/trainer-only-rock-native-impact-${String(attempt / 6).padStart(2, "0")}.png`,
            screenshot,
          )
        }
        await game.wait.frames(1)
      }
      expect((await game.state.read()).battle.trainerOnly).toMatchObject({
        rocks: 1,
        anger: 25,
        completedTurns: 0,
      })

      // The anger reaction remains on the real wild sprite for 44 frames.
      // Four-frame sampling observes both x2 offsets instead of aliasing the
      // task's `data[1] & 4` cadence at the same phase every time.
      for (let sample = 0; sample < 10; sample++) {
        const state = await game.state.read()
        expect(state.battle.trainerOnly.completedTurns).toBe(0)
        const screenshot = await game.screenshot()
        await fs.promises.writeFile(
          `/tmp/trainer-only-rock-anger-motion-${String(sample).padStart(2, "0")}.png`,
          screenshot,
        )
        await game.wait.frames(4)
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
