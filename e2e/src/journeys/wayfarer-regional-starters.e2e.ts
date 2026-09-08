import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { receiveBirchStarter, receiveElmStarter } from "../playbooks/regional-opening"

for (const slot of [0, 1, 2] as const) {
  describe.sequential(`Wayfarer native starter slot ${slot}`, () => {
    it("commits Elm's local slot once without touching Hoenn", async () => {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint: "elm-lab-before-intro",
          determinism: { textSpeed: "instant" },
        })
        const before = (await game.state.read()).origin
        await receiveElmStarter(game, slot)
        const state = await game.state.read()
        expect(state.party).toHaveLength(1)
        expect(state.party[0]?.species).toBe(["chikorita", "cyndaquil", "totodile"][slot])
        expect(state.origin).toMatchObject({
          id: 1,
          johtoChoice: slot,
          johtoCommitted: true,
          johtoReceived: true,
          hoennChoice: before.hoennChoice,
          hoennReceived: false,
        })
        await game.saveAndReload()
        expect((await game.state.read()).origin).toEqual(state.origin)
        expect((await game.state.read()).party).toHaveLength(1)
      } finally {
        await game.close()
      }
    })

    it("uses the bag partner in Birch's first battle and never grants it again in the lab", async () => {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint: slot === 1 ? "hoenn-female-before-rescue" : "hoenn-before-rescue",
          player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
          determinism: { textSpeed: "instant" },
        })
        expect((await game.state.read()).origin).toMatchObject({
          gender: slot === 1 ? 1 : 0,
          recovery: { map: slot === 1 ? "mays-house-2f" : "brendans-house-2f" },
        })
        await receiveBirchStarter(game, slot)
        const state = await game.state.read()
        expect(state.map.name).toBe("birch-lab")
        expect(state.party).toHaveLength(1)
        expect(state.party[0]?.species).toBe(["treecko", "torchic", "mudkip"][slot])
        expect(state.origin).toMatchObject({
          id: 2,
          hoennChoice: slot,
          hoennReceived: true,
          johtoChoice: 0,
          johtoCommitted: false,
          johtoReceived: false,
          maidenVoyageState: 0,
        })
        await game.saveAndReload()
        await game.player.warp("birch-lab", 6, 5, "up")
        await game.player.interact()
        expect((await game.state.read()).party).toHaveLength(1)
        expect((await game.state.read()).origin.hoennChoice).toBe(slot)
      } finally {
        await game.close()
      }
    })
  })
}
