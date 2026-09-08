import { describe, expect, it } from "webanvil/test"

import { GameSession, partyMenuActions } from "../harness/game-session"
import { openFieldPartyMenuActions, selectFieldPartyAction } from "../playbooks/field-party-menu"
import { advanceOpeningUntil } from "../playbooks/regional-opening"

const teleport = async (game: GameSession): Promise<void> => {
  await openFieldPartyMenuActions(game)
  expect((await game.state.read()).partyMenu.actions).toContain(partyMenuActions.teleport)
  await selectFieldPartyAction(game, partyMenuActions.teleport)
  await advanceOpeningUntil(
    game,
    (state) => state.ready && !state.partyMenu.open,
    "Teleport did not finish returning to recovery",
  )
}

describe.sequential("Origin home and local Teleport recovery", () => {
  for (const checkpoint of ["new-bark-after-intro", "hoenn-before-rescue"] as const) {
    it(`${checkpoint}: Teleport uses home, then the latest Center without changing origin`, async () => {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint,
          player: { position: { map: "olivine-city", x: 20, y: 20 } },
          party: [{ species: "pidgey", moves: ["teleport", "tackle"] }],
          determinism: { textSpeed: "instant" },
        })
        const before = await game.state.read()
        await teleport(game)
        expect((await game.state.read()).map.name).toBe(before.origin.recovery.map)
        expect((await game.state.read()).origin.id).toBe(before.origin.id)
        expect((await game.state.read()).party).toEqual(before.party)

        await game.player.warp("slateport-pokemon-center", 7, 8, "up")
        await game.wait.forReady()
        expect((await game.state.read()).origin.recovery.map).toBe("slateport-city")
        await game.player.warp("olivine-city", 20, 20)
        await game.saveAndReload()
        await teleport(game)
        expect(await game.state.read()).toMatchObject({
          map: { name: "slateport-city" },
          origin: { id: before.origin.id, recovery: { map: "slateport-city" } },
        })
        expect((await game.state.read()).party).toEqual(before.party)
      } finally {
        await game.close()
      }
    })
  }
})

describe.sequential("Hoenn field-poison recovery exception", () => {
  for (const source of ["slateport-city", "olivine-city"] as const) {
    it(`${source}: applies Lavaridge recovery only while physically in Hoenn`, async () => {
      const game = await GameSession.launch()
      try {
        // The checkpoint supplies a poisoned partner at 1 HP with poison survival
        // disabled. Walking still runs the real field-poison and whiteout scripts.
        await game.arrange({
          checkpoint: "hoenn-before-poison-whiteout",
          player: {
            facing: "right",
            position: { map: source, x: 19, y: 21 },
          },
          party: [{ species: "pidgey", moves: ["tackle"] }],
          story: { flags: { hoennWhiteoutToLavaridge: true } },
          determinism: { textSpeed: "instant" },
        })
        if (source === "olivine-city") {
          await game.player.warp("olivine-pokemon-center", 7, 8, "up")
          await game.player.warp("olivine-city", 19, 21, "right")
        }
        const before = await game.state.read()
        expect(before.party[0]?.fainted).toBe(false)
        expect(await game.story.flag("hoennWhiteoutToLavaridge")).toBe(true)
        for (const target of [
          { x: 20, y: 21 },
          { x: 20, y: 22 },
          { x: 19, y: 22 },
          { x: 19, y: 21 },
          { x: 20, y: 21 },
        ]) {
          for (let attempt = 0; attempt < 10; attempt++) {
            const state = await game.state.read()
            if (state.party[0]?.fainted) break
            if (state.player.x === target.x && state.player.y === target.y) break
            const direction =
              state.player.x < target.x
                ? "right"
                : state.player.x > target.x
                  ? "left"
                  : state.player.y < target.y
                    ? "down"
                    : "up"
            await game.player.move(direction)
            await game.wait.frames(24)
          }
          if ((await game.state.read()).party[0]?.fainted) break
        }
        expect((await game.state.read()).party[0]?.fainted).toBe(true)
        const destination =
          source === "slateport-city" ? "lavaridge-pokemon-center" : before.origin.recovery.map
        await advanceOpeningUntil(
          game,
          (state) => state.ready && state.map.name === destination,
          "field-poison whiteout did not reach the source-appropriate recovery point",
        )
        expect(await game.state.read()).toMatchObject({
          origin: { id: 2, hoennReceived: false, johtoCommitted: false, maidenVoyageState: 0 },
          party: [{ species: "pidgey", fainted: false }],
          circuit: { badges: { total: 0 } },
        })
        expect((await game.state.read()).origin.recovery.map).toBe(
          source === "slateport-city" ? "lavaridge-town" : before.origin.recovery.map,
        )
        await game.saveAndReload()
        expect((await game.state.read()).map.name).toBe(destination)
        expect((await game.state.read()).origin.id).toBe(2)
      } finally {
        await game.close()
      }
    })
  }
})
