import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  finishOpeningDialogue,
  receiveBirchStarter,
  stepOpening,
} from "../playbooks/regional-opening"

describe.sequential("Littleroot trainer visiting Elm", () => {
  it("meets an ordinary professor who offers and writes nothing", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveBirchStarter(game, 1)
      const partner = (await game.state.read()).party[0]
      const untouched = async (): Promise<void> => {
        const state = await game.state.read()
        expect(state.party).toEqual([partner])
        expect(state.origin).toMatchObject({
          id: 2,
          johtoChoice: 0,
          johtoCommitted: false,
          johtoReceived: false,
          hoennChoice: 1,
          hoennReceived: true,
        })
        expect(await game.story.var("newBarkTownLabState")).toBe(0)
        expect(await game.story.var("newBarkTownState")).toBe(2)
      }

      // The lab's entry triggers end cleanly instead of running the native introduction.
      await game.player.warp("elm-lab", 6, 8, "up")
      await stepOpening(game, "up")
      await finishOpeningDialogue(game)
      await untouched()

      // Elm talks like a professor and releases the player; the starter Balls stay shut.
      await game.player.warp("elm-lab", 3, 3, "up")
      await game.player.interact()
      await finishOpeningDialogue(game)
      await untouched()
      await game.player.warp("elm-lab", 10, 5, "up")
      await game.player.interact()
      await finishOpeningDialogue(game)
      await untouched()

      // The Silver of New Bark's lab corner never appears for a visitor.
      await game.player.warp("new-bark-town", 10, 12, "up")
      expect(await game.story.flag("hideSilverInNewBark")).toBe(true)
      await game.saveAndReload()
      await untouched()
    } finally {
      await game.close()
    }
  })
})
