import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { playThroughNewGameIntro } from "../playbooks/new-game-intro"
import {
  advanceOpeningUntil,
  playHoennHousehold,
  receiveBirchStarter,
} from "../playbooks/regional-opening"

describe.sequential("Wayfarer native Littleroot household", () => {
  it("plays truck, Mom, clock, rival, rescue, and Birch acknowledgement from a fresh game", async () => {
    const game = await GameSession.launch()
    try {
      await playThroughNewGameIntro(game, "hoenn")
      await fs.promises.writeFile("/tmp/wayfarer-hoenn-initial-map.png", await game.screenshot())
      await playHoennHousehold(game)
      await receiveBirchStarter(game, 1, "win", null)
      expect(await game.state.read()).toMatchObject({
        map: { name: "birch-lab" },
        origin: { id: 2, hoennChoice: 1, hoennReceived: true, johtoCommitted: false },
        party: [{ species: "torchic" }],
        circuit: { badges: { total: 0 }, clears: { kanto: false, johto: false, hoenn: false } },
      })
      await game.saveAndReload()
      expect((await game.state.read()).party).toHaveLength(1)
      const home =
        (await game.state.read()).origin.gender === 1 ? "mays-house-2f" : "brendans-house-2f"
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
        "/tmp/wayfarer-hoenn-household-failure.png",
        await game.screenshot(),
      )
      throw error
    } finally {
      await game.close()
    }
  }, 180_000)
})
