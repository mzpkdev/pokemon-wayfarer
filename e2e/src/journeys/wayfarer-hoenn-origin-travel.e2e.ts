import { describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"
import { advanceOpeningUntil, receiveBirchStarter } from "../playbooks/regional-opening"

const depart = async (game: GameSession, destination: GameMap): Promise<void> => {
  await game.player.interact()
  for (let attempt = 0; attempt < 100; attempt++) {
    await game.wait.frames(45)
    const state = await game.state.read()
    if (state.map.name === destination && state.ready) return
    await game.controls.press("a")
  }
  throw new Error(`Aqua did not reach ${destination}: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Littleroot-origin regular Aqua circuit", () => {
  it("grants its Ticket, visits both HNS ports, and returns without maiden-voyage progress", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveBirchStarter(game, 1)
      const partner = (await game.state.read()).party
      await game.player.warp("slateport-city-harbor", 14, 11, "right")
      await depart(game, "olivine-port-inside")
      expect(await game.inventory.contains("ssTicket")).toBe(true)
      for (const [port, recovery] of [
        ["olivine-port-inside", "olivine-city"],
        ["vermilion-port-inside", "vermilion-city"],
        ["slateport-city-harbor", "slateport-city"],
      ] as const) {
        if (port === "vermilion-port-inside") {
          await game.controls.press("down")
          await game.wait.frames(24)
          await depart(game, port)
        }
        if (port === "slateport-city-harbor") {
          await game.controls.press("down")
          await game.wait.frames(24)
          await depart(game, port)
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
          circuit: { badges: { total: 0 }, clears: { kanto: false, johto: false, hoenn: false } },
        })
        expect(state.party).toEqual(partner)
        expect(await game.inventory.contains("ssTicket")).toBe(true)
        await game.saveAndReload()
        expect((await game.state.read()).origin).toEqual(state.origin)
        await game.battle.startWild({ species: "pidgey", level: 2, moves: ["tackle"] })
        await game.battle.lose()
        const center = port === "slateport-city-harbor" ? "slateport-pokemon-center" : recovery
        await advanceOpeningUntil(
          game,
          (current) => current.ready && current.map.name === center,
          `blackout did not reach the local recovery point after ${port}`,
        )
        expect((await game.state.read()).origin.id).toBe(2)
        expect(await game.inventory.contains("ssTicket")).toBe(true)
        expect((await game.state.read()).party.every((mon) => !mon.fainted)).toBe(true)
        await game.player.warp(port, state.player.x, state.player.y, "down")
      }
      expect((await game.state.read()).origin.visitedRegions).toBe(7)
    } finally {
      await game.close()
    }
  })
})
