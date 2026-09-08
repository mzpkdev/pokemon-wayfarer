import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  finishOriginIntroduction,
  playThroughNewGameIntro,
  reachOriginQuestion,
  waitForOriginStage,
} from "../playbooks/new-game-intro"

for (const origin of ["johto", "hoenn"] as const) {
  describe.sequential(`Wayfarer ${origin} new game`, () => {
    it("initializes the selected opening and preserves it through Continue", async () => {
      const game = await GameSession.launch()
      try {
        await playThroughNewGameIntro(game, origin)
        const before = await game.state.read()
        expect(before).toMatchObject({
          ready: true,
          map: { name: origin === "johto" ? "players-bedroom" : "inside-of-truck" },
          party: [],
          origin: {
            id: origin === "johto" ? 1 : 2,
            currentRegion: origin === "johto" ? 2 : 3,
            visitedRegions: origin === "johto" ? 2 : 4,
            hoennInitialized: origin === "hoenn",
            johtoCommitted: false,
            johtoReceived: false,
            hoennReceived: false,
            maidenVoyageState: 0,
            runningShoes: false,
            pokedex: false,
            recovery: {
              map:
                origin === "johto"
                  ? "players-bedroom"
                  : before.origin.gender === 0
                    ? "brendans-house-2f"
                    : "mays-house-2f",
            },
          },
          circuit: {
            trainerRating: 0,
            badges: { total: 0 },
            clears: { kanto: false, johto: false, hoenn: false },
          },
        })
        await game.saveAndReload()
        const after = await game.state.read()
        expect(after.map).toEqual(before.map)
        expect(after.origin).toEqual(before.origin)
        expect(after.party).toEqual([])
      } finally {
        await game.close()
      }
    })
  })
}

describe.sequential("Wayfarer origin confirmation", () => {
  it("keeps B on the list, cancels with No and B, and preserves the Hoenn candidate", async () => {
    const game = await GameSession.launch()
    try {
      await reachOriginQuestion(game)
      await game.controls.press("b")
      await game.wait.frames(60)
      expect((await game.state.read()).origin.introStage).toBe(1)
      await game.controls.press("down")
      await game.controls.press("a")
      await waitForOriginStage(game, 2)
      await game.wait.frames(120)
      expect((await game.state.read()).origin.introStage).toBe(2)
      await game.controls.press("down")
      await game.controls.press("a")
      await waitForOriginStage(game, 1)
      await game.controls.press("a")
      await waitForOriginStage(game, 2)
      await game.controls.press("b")
      await waitForOriginStage(game, 1)
      await game.controls.press("a")
      await finishOriginIntroduction(game, "hoenn")
      expect((await game.state.read()).origin.id).toBe(2)
    } finally {
      await game.close()
    }
  })
})
