import * as fs from "node:fs/promises"
import { describe, expect, it } from "webanvil/test"
import { GameSession } from "../harness/game-session"

describe.sequential("Trainer-only natural encounter entry", () => {
  it("enters the dedicated controller by walking ordinary grass with an empty party", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "route-30", x: 8, y: 10 }, facing: "left" },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      // These neighboring tiles are ordinary tall grass in the live Route 30
      // layout. Only movement triggers encounter generation; no battle fixture.
      let steps = 0
      for (; steps < 120; steps++) {
        if ((await game.state.read()).battle.active) break
        await game.player.move(steps % 2 === 0 ? "left" : "right")
        await game.wait.frames(30)
      }
      expect(steps).toBeLessThan(120)
      for (let attempt = 0; attempt < 300; attempt++) {
        const state = await game.state.read()
        if (state.battle.ui === "action-menu") break
        if (state.battle.ui === "text") await game.controls.press("a")
        else await game.wait.frames(12)
      }
      const state = await game.state.read()
      expect(state.party).toHaveLength(0)
      expect(state.battle.ui).toBe("action-menu")
      expect(state.battle.trainerOnly).toMatchObject({
        active: true,
        completedTurns: 0,
        anger: 0,
        rocks: 0,
      })
      await fs.writeFile("/tmp/trainer-only-natural-grass-menu.png", await game.screenshot())
    } finally {
      await game.close()
    }
  })
})
