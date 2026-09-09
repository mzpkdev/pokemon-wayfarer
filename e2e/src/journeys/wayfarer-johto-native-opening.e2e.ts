import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { playThroughNewGameIntro } from "../playbooks/new-game-intro"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  receiveElmStarter,
  walkOpeningTo,
  stepOpening,
  enterOpeningMap,
} from "../playbooks/regional-opening"

describe.sequential("Wayfarer native New Bark household", () => {
  it("plays the clock and Mom handoff before receiving Elm's first partner", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "johto")
      await fs.promises.writeFile("/tmp/wayfarer-johto-initial-map.png", await game.screenshot())
      await walkOpeningTo(game, 5, 6)
      await walkOpeningTo(game, 5, 4)
      await walkOpeningTo(game, 10, 4)
      await walkOpeningTo(game, 10, 3)
      await finishOpeningDialogue(game)
      await game.saveAndReload()
      await walkOpeningTo(game, 10, 2)
      await enterOpeningMap(game, "left", "players-house-1f")
      await game.wait.forMap("players-house-1f")
      await stepOpening(game, "down")
      await finishOpeningDialogue(game)
      expect((await game.state.read()).origin.runningShoes).toBe(true)
      await walkOpeningTo(game, 9, 7)
      await enterOpeningMap(game, "down", "new-bark-town")
      await game.wait.forMap("new-bark-town")
      await walkOpeningTo(game, 10, 12)
      await walkOpeningTo(game, 10, 10)
      await enterOpeningMap(game, "up", "elm-lab")
      await game.wait.forMap("elm-lab")
      await receiveElmStarter(game, 0)
      expect(await game.state.read()).toMatchObject({
        origin: {
          id: 1,
          johtoChoice: 0,
          johtoCommitted: true,
          johtoReceived: true,
          hoennReceived: false,
        },
        party: [{ species: "chikorita" }],
      })
      await game.saveAndReload()
      expect((await game.state.read()).party).toHaveLength(1)
      const home = "players-bedroom"
      const beforeLoss = await game.state.read()
      await game.battle.startWild({ species: "pidgey", level: 2, moves: ["tackle"] })
      await game.battle.lose()
      await advanceOpeningUntil(
        game,
        (state) => state.ready && !state.battle.active,
        "ordinary wild loss did not return to the field",
      )
      const defeated = await game.state.read()
      expect(defeated.map).toEqual(beforeLoss.map)
      expect(defeated.player).toEqual(beforeLoss.player)
      expect(defeated.origin.recovery.map).toBe(home)
      expect(defeated.partyVitals).toEqual([{ hp: 0, status: 0 }])
      // The first partner is level 5; retain the native first-badge loss fee.
      expect(defeated.money).toBe(beforeLoss.money - Math.min(beforeLoss.money, 8 * 5))
      await game.saveAndReload()
      expect(await game.state.read()).toMatchObject({
        map: beforeLoss.map,
        player: beforeLoss.player,
        origin: { recovery: { map: home } },
        partyVitals: [{ hp: 0, status: 0 }],
        money: defeated.money,
      })
    } catch (error) {
      await fs.promises.writeFile(
        "/tmp/wayfarer-johto-household-failure.png",
        await game.screenshot(),
      )
      throw error
    } finally {
      await game.close()
    }
  })
})
