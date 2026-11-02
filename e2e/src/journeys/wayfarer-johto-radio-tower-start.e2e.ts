import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

const settleField = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.scriptActive && !state.dialogueOpen && !state.battle.active) return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not release the field: ${JSON.stringify(await game.state.read())}`,
  )
}

// Talk to the scout and advance his text until the follow prompt is on screen.
const talkToScout = async (game: GameSession): Promise<void> => {
  const before = (await game.state.read()).dialogue.sequence
  await game.player.interact()
  for (let attempt = 0; attempt < 120; attempt++) {
    await game.wait.frames(20)
    const state = await game.state.read()
    // The yes/no menu reports no open dialogue, so the prompt is recognised by its text.
    if (state.dialogue.sequence > before && /Follow the suspicious/.test(state.dialogue.fullText)) {
      await game.wait.frames(30)
      return
    }
    if (state.dialogueOpen || state.scriptActive) await game.controls.press("a")
  }
  throw new Error(`The scout never asked to be followed: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Johto Radio Tower start", () => {
  it("starts the occupation only when the scout is followed, and keeps it through save and reload", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "goldenrod-city", x: 18, y: 23 } },
        // Elm's first-visit call is skipped so nothing else holds the player on arrival.
        story: { vars: { goldenrodCityState: 1 }, flags: { shownElmTogepi: true } },
        party: [{ species: "rattata", level: 5 }],
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })
      await game.saveAndReload()
      await expect(game.story.flag("hideGoldenrodRocketScout")).resolves.toBe(false)

      // Declining changes nothing.
      await talkToScout(game)
      await game.controls.press("b")
      await settleField(game, "declined scout")
      await expect(game.story.var("goldenrodCityState")).resolves.toBe(1)
      await expect(game.story.flag("hideGoldenrodRocketScout")).resolves.toBe(false)

      // Accepting starts Goldenrod's occupation and writes nothing in Mahogany.
      await talkToScout(game)
      await game.controls.press("a")
      await game.wait.frames(240)
      await settleField(game, "accepted scout")
      await expect(game.story.var("goldenrodCityState")).resolves.toBe(6)
      await expect(game.story.flag("hideGoldenrodRocketScout")).resolves.toBe(true)
      await expect(game.story.var("mahoganyTownState")).resolves.toBe(0)
      await expect(game.story.var("triggerElmRocketCall")).resolves.toBe(0)
      expect((await game.state.read()).map.name).toBe("goldenrod-city")

      await game.saveAndReload()
      await expect(game.story.var("goldenrodCityState")).resolves.toBe(6)
      await expect(game.story.flag("hideGoldenrodRocketScout")).resolves.toBe(true)
    } finally {
      await game.close()
    }
  }, 600_000)
})
