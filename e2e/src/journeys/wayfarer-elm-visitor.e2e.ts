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
      await game.player.warp("elm-lab", 6, 4, "up")
      await game.player.interact()
      await game.wait.frames(30)
      expect((await game.state.read()).dialogue.text).toContain("ELM: Hello!")
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

      // After the Dragon's Den, Blackthorn's Elm call is a frame script. A visitor gets no
      // call, but the city must still move on, or the player is held in place on arrival.
      // (Blackthorn's first-load setup leaves a started city's state alone.)
      await game.story.setVar("blackthornCityState", 4)
      await game.player.warp("blackthorn-city", 27, 49, "down")
      await game.wait.frames(60)
      expect(await game.story.var("blackthornCityState")).toBe(5)
      const arrived = await game.state.read()
      expect(arrived.scriptActive).toBe(false)
      let walked = false
      for (const direction of ["down", "left", "right", "up"] as const) {
        await game.player.move(direction)
        await game.wait.frames(30)
        const now = await game.state.read()
        if (now.player.x !== arrived.player.x || now.player.y !== arrived.player.y) {
          walked = true
          break
        }
      }
      expect(walked).toBe(true)
    } finally {
      await game.close()
    }
  })
})
