import { describe, expect, it } from "webanvil/test"
import { GameSession, type GameState } from "../harness/game-session"
import { arrangeTrainerOnly } from "../playbooks/trainer-only-scenario"

const advance = async (game: GameSession, predicate: (state: GameState) => boolean) => {
  for (let frame = 0; frame < 9_600; frame += 15) {
    const state = await game.state.read()
    if (predicate(state)) return state
    if (state.battle.ui === "text" || state.dialogueOpen || state.controlsLocked)
      await game.controls.press("a")
    else await game.wait.frames(15)
  }
  throw new Error(`Flag-only transition timed out: ${JSON.stringify(await game.state.read())}`)
}

const menu = (game: GameSession) => advance(game, (state) => state.battle.ui === "action-menu")

describe.sequential("Explicit trainer-only scenario flag", () => {
  it("defaults off and does not turn unprotected grass encounters into trainer-only combat", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "route-30", x: 8, y: 10 }, facing: "left" },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      expect(await game.story.flag("trainerOnlyEnabled")).toBe(false)
      const initial = await game.state.read()
      for (let step = 0; step < 120; step++) {
        const state = await game.state.read()
        expect(state.battle.trainerOnly.active).toBe(false)
        if (state.map.name !== initial.map.name) break
        await game.player.move(step % 2 === 0 ? "left" : "right")
        await game.wait.frames(30)
      }
      expect((await game.state.read()).battle.trainerOnly.active).toBe(false)
      expect(await game.story.flag("trainerOnlyEnabled")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("clears the scenario flag when changing maps", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeTrainerOnly(game, {
        checkpoint: "new-bark-after-intro",
        party: [{ species: "pidgey", level: 5 }],
        determinism: { textSpeed: "instant" },
      })
      expect(await game.story.flag("trainerOnlyEnabled")).toBe(true)
      await game.player.warp("cherrygrove-pokemon-center", 7, 8, "up")
      await game.wait.forReady()
      expect(await game.story.flag("trainerOnlyEnabled")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("allows authors to clear the flag and leaves a usable party on ordinary battle routing", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeTrainerOnly(game, {
        checkpoint: "new-bark-after-intro",
        party: [{ species: "pidgey", level: 5 }],
        determinism: { textSpeed: "instant" },
      })
      await game.story.setFlag("trainerOnlyEnabled", false)
      expect(await game.story.flag("trainerOnlyEnabled")).toBe(false)
      await game.story.setFlag("trainerOnlyEnabled", true)
      await game.battle.startWild({ species: "rattata", level: 5 })
      expect((await menu(game)).battle.trainerOnly.active).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("restores native recovery after losing an ordinary wild battle", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "route-30", x: 8, y: 10 } },
        party: [{ species: "rattata", level: 1, moves: ["tackle"] }],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const before = await game.state.read()
      await game.battle.startWild({ species: "pidgey", level: 100, moves: ["tackle"] })
      await menu(game)
      await game.controls.press("a")
      await game.wait.until((state) => state.battle.ui === "move-menu", "ordinary move menu")
      await game.controls.press("a")
      const recovered = await advance(game, (state) => state.ready && !state.battle.active)
      expect(recovered.map.name).toBe(before.origin.recovery.map)
      expect(recovered.party).toMatchObject([{ species: "rattata", fainted: false }])
      expect(recovered.partyVitals[0]!.hp).toBeGreaterThan(0)
      expect(recovered.money).toBe(before.money - Math.min(before.money, 8))
      expect(recovered.battle.trainerOnly.active).toBe(false)
      expect(await game.story.flag("trainerOnlyEnabled")).toBe(false)
    } finally {
      await game.close()
    }
  })

})
