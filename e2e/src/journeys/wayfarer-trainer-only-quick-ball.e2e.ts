import fs from "node:fs/promises"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

const waitForActionMenu = async (game: GameSession, description: string) => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.battle.ui === "text") await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description}: battle introduction did not finish`)
}

const waitForFailedQuickBall = async (game: GameSession, trainerOnly: boolean) => {
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (
      state.battle.active &&
      state.battle.ui === "action-menu" &&
      state.battle.lastUsedItem === "quickBall" &&
      (!trainerOnly || state.battle.trainerOnly.completedTurns === 1)
    )
      return state
    if (state.battle.ui === "text") await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(
    `Quick Ball did not return to the action menu: ${JSON.stringify(await game.state.read())}`,
  )
}

describe.sequential("Trainer-only R Quick Ball widget", () => {
  it("uses its only displayed Quick Ball and commits one failed trainer-only throw", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [],
        bag: { items: { quickBall: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.battle.startWild({ species: "kyogre", level: 100 })
      await waitForActionMenu(game, "trainer-only Quick Ball widget")
      await game.wait.frames(60)
      await fs.writeFile("/tmp/trainer-only-r-quickball-widget.png", await game.screenshot())

      await game.controls.press("r")
      const resolved = await waitForFailedQuickBall(game, true)
      expect(resolved.battle).toMatchObject({
        active: true,
        lastUsedItem: "quickBall",
        caughtSpecies: "none",
        trainerOnly: { completedTurns: 1 },
      })
      expect(resolved.bag.items.quickBall).toBe(0)
    } finally {
      await game.close()
    }
  })

  it("leaves a trainer-only turn untouched when R has no owned ball", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.battle.startWild({ species: "pidgey", level: 5 })
      await waitForActionMenu(game, "empty R Quick Ball menu")
      await fs.writeFile("/tmp/trainer-only-r-no-ball-menu.png", await game.screenshot())
      const before = await game.state.read()

      await game.controls.press("r")
      await game.wait.frames(120)
      const after = await game.state.read()
      expect(after.battle.ui).toBe("action-menu")
      expect(after.battle.trainerOnly).toEqual(before.battle.trainerOnly)
      expect(after.bag.items.quickBall).toBe(0)
    } finally {
      await game.close()
    }
  })

  it("keeps the ordinary party battle R Quick Ball path available", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        party: [{ species: "wailord", level: 100 }],
        bag: { items: { quickBall: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.battle.startWild({ species: "kyogre", level: 100 })
      await waitForActionMenu(game, "ordinary Quick Ball widget")
      expect((await game.state.read()).battle.trainerOnly.active).toBe(false)

      await game.controls.press("r")
      const resolved = await waitForFailedQuickBall(game, false)
      expect(resolved.battle).toMatchObject({
        active: true,
        lastUsedItem: "quickBall",
        caughtSpecies: "none",
        trainerOnly: { active: false },
      })
      expect(resolved.bag.items.quickBall).toBe(0)
    } finally {
      await game.close()
    }
  })
})
