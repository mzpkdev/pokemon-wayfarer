import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  receiveBirchStarter,
} from "../playbooks/regional-opening"

describe.sequential("Littleroot trainer visiting Elm", () => {
  it("postpones the choice, declines the gift, and reclaims the same slot after reloading", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveBirchStarter(game, 1)
      const partner = (await game.state.read()).party[0]
      await game.player.warp("players-house-1f", 2, 3, "up")
      await game.player.interact()
      await finishOpeningDialogue(game)
      expect(await game.story.var("newBarkTownState")).toBe(2)
      expect(await game.story.var("newBarkTownLabState")).toBe(0)
      await game.player.warp("elm-lab", 6, 4, "up")
      await game.player.interact()
      await advanceOpeningUntil(game, (state) => state.ready, "Elm did not allow postponement", "b")
      expect((await game.state.read()).origin.johtoCommitted).toBe(false)
      expect(await game.story.var("newBarkTownLabState")).toBe(0)
      await game.player.interact()
      await finishOpeningDialogue(game)
      await game.player.warp("elm-lab", 10, 5, "up")
      await game.player.interact()
      await advanceOpeningUntil(
        game,
        (state) => state.origin.johtoCommitted,
        "Elm choice did not commit",
      )
      await advanceOpeningUntil(
        game,
        (state) => state.ready,
        "Elm gift decline did not return control",
        "b",
      )
      expect((await game.state.read()).origin).toMatchObject({
        id: 2,
        johtoChoice: 2,
        johtoCommitted: true,
        johtoReceived: false,
        hoennChoice: 1,
        hoennReceived: true,
      })
      expect((await game.state.read()).party).toEqual([partner])
      expect(await game.story.var("newBarkTownLabState")).toBe(2)
      expect(await game.story.var("newBarkTownState")).toBe(3)
      await game.saveAndReload()
      await game.player.warp("elm-lab", 3, 3, "up")
      await game.player.interact()
      await advanceOpeningUntil(
        game,
        (state) => state.origin.johtoReceived,
        "Elm did not deliver the saved choice",
      )
      await advanceOpeningUntil(
        game,
        (state) => state.ready,
        "Elm gift handoff did not finish",
        "b",
      )
      const state = await game.state.read()
      expect(state.party).toHaveLength(2)
      expect(state.party[0]).toEqual(partner)
      expect(state.party[1]?.species).toBe("totodile")
      expect(state.origin).toMatchObject({ id: 2, johtoChoice: 2, hoennChoice: 1 })
      expect(await game.story.var("newBarkTownLabState")).toBeGreaterThanOrEqual(2)
      await game.saveAndReload()
      expect((await game.state.read()).party).toEqual(state.party)
    } finally {
      await game.close()
    }
  })
})
