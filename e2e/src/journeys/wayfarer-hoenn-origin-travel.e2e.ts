import { describe, expect, it } from "webanvil/test"

import { GameSession, partyMenuActions, type GameMap } from "../harness/game-session"
import { openFieldPartyMenuActions, selectFieldPartyAction } from "../playbooks/field-party-menu"
import { advanceOpeningUntil, receiveBirchStarter } from "../playbooks/regional-opening"
import { beginWayfarerRegularAquaDeparture } from "../playbooks/wayfarer-ports"

const finishDeparture = async (game: GameSession, destination: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 100; attempt++) {
    await game.wait.frames(45)
    const state = await game.state.read()
    if (state.map.name === destination && state.ready) return
    await game.controls.press("a")
  }
  throw new Error(`Aqua did not reach ${destination}: ${JSON.stringify(await game.state.read())}`)
}

const depart = async (game: GameSession, destination: GameMap): Promise<void> => {
  await game.player.interact()
  await finishDeparture(game, destination)
}

const teleportToRecovery = async (game: GameSession): Promise<void> => {
  await openFieldPartyMenuActions(game)
  expect((await game.state.read()).partyMenu.actions).toContain(partyMenuActions.teleport)
  await selectFieldPartyAction(game, partyMenuActions.teleport)
  await advanceOpeningUntil(
    game,
    (state) => state.ready && !state.partyMenu.open,
    "Teleport did not return to the registered recovery location",
  )
}

const recoveryPositions = {
  "olivine-city": { x: 20, y: 20 },
  "vermilion-city": { x: 10, y: 10 },
  "slateport-city": { x: 19, y: 21 },
} as const

describe.sequential("Littleroot-origin regular Aqua circuit", () => {
  it("grants its Ticket, visits both HNS ports, and returns without maiden-voyage progress", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        party: [{ species: "pidgey", moves: ["teleport", "tackle"] }],
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
          await beginWayfarerRegularAquaDeparture(game)
          await finishDeparture(game, port)
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
        // Ordinary wild losses remain unhealed in the field. Teleport exercises
        // the same persisted recovery destination without changing that policy.
        // The port interiors do not offer the field action, so resume on that
        // port's outdoor map after its arrival has registered the target.
        await game.player.warp(
          recovery,
          recoveryPositions[recovery].x,
          recoveryPositions[recovery].y,
          "down",
        )
        await game.wait.forReady()
        await teleportToRecovery(game)
        expect((await game.state.read()).map.name).toBe(recovery)
        expect((await game.state.read()).origin).toMatchObject({
          id: 2,
          recovery: { map: recovery },
        })
        expect(await game.inventory.contains("ssTicket")).toBe(true)
        expect((await game.state.read()).party).toEqual(partner)
        await game.player.warp(port, state.player.x, state.player.y, "down")
      }
      expect((await game.state.read()).origin.visitedRegions).toBe(7)
    } finally {
      await game.close()
    }
  })
})
