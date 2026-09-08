import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { receiveBirchStarter } from "../playbooks/regional-opening"

describe.sequential("Native Birch first-battle loss", () => {
  it("heals the granted partner and continues to Birch without a second grant", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveBirchStarter(game, 1, "lose")
      expect(await game.state.read()).toMatchObject({
        map: { name: "birch-lab" },
        party: [{ species: "torchic", fainted: false }],
        origin: { id: 2, hoennChoice: 1, hoennReceived: true, johtoCommitted: false },
      })
    } finally {
      await game.close()
    }
  })
})
