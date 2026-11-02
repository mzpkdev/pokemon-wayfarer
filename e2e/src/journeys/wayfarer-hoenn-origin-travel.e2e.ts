import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { advanceOpeningUntil, receiveBirchStarter } from "../playbooks/regional-opening"
import {
  boardFerryAtDock,
  goAshoreFromExitSailor,
  sailFerryLeg,
  waitAboard,
} from "../playbooks/wayfarer-ports"

describe.sequential("Littleroot-origin ferry network", () => {
  it("grants its Ticket at Slateport, rides the S.S. Aqua to Olivine and Vermilion and keeps its own origin state", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveBirchStarter(game, 1)
      const partner = (await game.state.read()).party
      // Slateport's ship at night is the Aqua (Lilycove -> Olivine -> Vermilion lie on its loop). Starting at 4:00
      // leaves room for three legs before midnight, and the clock never crosses it.
      await game.clock.setTimeOfDay({ hour: 4, minute: 0, second: 0 })
      // The Hoenn start's Ticket moved to the Slateport gangway sailor (the attendant at (8,10)).
      await game.player.warp("slateport-city-harbor", 8, 11, "up")
      expect(await game.inventory.contains("ssTicket")).toBe(false)
      await boardFerryAtDock(game)
      await waitAboard(game, "ss-aqua-1f")
      expect(await game.inventory.contains("ssTicket")).toBe(true)

      // Slateport -> Lilycove (stay aboard) -> Olivine.
      await sailFerryLeg(game)
      await sailFerryLeg(game)
      await goAshoreFromExitSailor(game, "up", "olivine-port-inside")

      for (const [port, recovery] of [
        ["olivine-port-inside", "olivine-city"],
        ["vermilion-port-inside", "vermilion-city"],
      ] as const) {
        if (port === "vermilion-port-inside") {
          // Olivine -> Vermilion by day on the same ship.
          await game.player.warp("olivine-port-inside", 8, 16, "down")
          await boardFerryAtDock(game)
          await waitAboard(game, "ss-aqua-1f")
          await sailFerryLeg(game)
          await goAshoreFromExitSailor(game, "up", port)
        }
        const state = await game.state.read()
        expect(state).toMatchObject({
          map: { name: port },
          origin: {
            id: 2,
            maidenVoyageState: 0,
            hoennInitialized: true,
            hoennChoice: 1,
            hoennReceived: true,
            johtoCommitted: false,
            recovery: { map: recovery },
          },
          circuit: {
            badges: { total: 0 },
            clears: { indigo: false, masters: false, hoenn: false },
          },
        })
        expect(state.party).toEqual(partner)
        expect(await game.inventory.contains("ssTicket")).toBe(true)
        await game.saveAndReload()
        expect((await game.state.read()).origin).toEqual(state.origin)
        await game.battle.startWild({ species: "pidgey", level: 2, moves: ["tackle"] })
        await game.battle.lose()
        await advanceOpeningUntil(
          game,
          (current) => current.ready && current.map.name === recovery,
          `blackout did not reach the local recovery point after ${port}`,
        )
        expect((await game.state.read()).origin.id).toBe(2)
        expect(await game.inventory.contains("ssTicket")).toBe(true)
        expect((await game.state.read()).party.every((mon) => !mon.fainted)).toBe(true)
      }
      // Johto, Kanto and Hoenn have all been visited.
      expect((await game.state.read()).origin.visitedRegions).toBe(7)
    } finally {
      await game.close()
    }
  })
})
