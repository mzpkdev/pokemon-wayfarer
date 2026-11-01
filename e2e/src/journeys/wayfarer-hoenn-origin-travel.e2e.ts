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
  it("grants its Ticket at the Slateport gangway, rides the S.S. Tidal to Lilycove and keeps its own origin state", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveBirchStarter(game, 1)
      const partner = (await game.state.read()).party
      // The Hoenn start's Ticket moved to the Slateport gangway sailor (the attendant at (8,10)). The
      // Tidal needs no League: it is Slateport's ship by day.
      await game.player.warp("slateport-city-harbor", 8, 11, "up")
      expect(await game.inventory.contains("ssTicket")).toBe(false)
      await boardFerryAtDock(game)
      await waitAboard(game, "ss-tidal-corridor")
      expect(await game.inventory.contains("ssTicket")).toBe(true)
      // Boarding the Tidal initialized Hoenn without touching the Johto-side travel state.
      expect((await game.state.read()).origin).toMatchObject({
        id: 2,
        maidenVoyageState: 0,
        hoennInitialized: true,
      })

      await sailFerryLeg(game)
      await goAshoreFromExitSailor(game, "down", "lilycove-city-harbor")

      const state = await game.state.read()
      expect(state).toMatchObject({
        map: { name: "lilycove-city-harbor" },
        player: { x: 8, y: 11 },
        origin: {
          id: 2,
          maidenVoyageState: 0,
          hoennInitialized: true,
          hoennChoice: 1,
          hoennReceived: true,
          johtoCommitted: false,
          recovery: { map: "lilycove-city" },
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
        (current) => current.ready && current.map.name === "lilycove-pokemon-center",
        "blackout did not reach the Lilycove recovery point after the Tidal",
      )
      expect((await game.state.read()).origin.id).toBe(2)
      expect(await game.inventory.contains("ssTicket")).toBe(true)
      expect((await game.state.read()).party.every((mon) => !mon.fainted)).toBe(true)
    } finally {
      await game.close()
    }
  })
})
