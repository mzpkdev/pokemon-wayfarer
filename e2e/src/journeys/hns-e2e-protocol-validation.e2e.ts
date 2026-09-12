import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  launchProtocolTestSession,
  type ProtocolTestSession,
} from "../harness/game-session/protocol-test-support"
import { closePcStorage, openPcStorage } from "../playbooks/pc-storage"

const finishBattle = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active) return
    if (state.battle.ui === "text" || state.dialogueOpen) await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not return to the field`)
}

describe.sequential("HNS E2E command validation", () => {
  let game: GameSession
  let protocol: ProtocolTestSession["protocol"]

  beforeAll(async () => {
    const session = await launchProtocolTestSession()
    game = session.game
    protocol = session.protocol
    return () => game.close()
  })

  it("rejects invalid box, slot, species, and item quantity fixtures", async () => {
    await expect(protocol.rejectedFixtureError("current-box")).resolves.toBe("pc-box")
    await expect(protocol.rejectedFixtureError("pc-box")).resolves.toBe("pc-box")
    await expect(protocol.rejectedFixtureError("pc-slot")).resolves.toBe("pc-slot")
    await expect(protocol.rejectedFixtureError("species")).resolves.toBe("species")
    await expect(protocol.rejectedFixtureError("item-quantity")).resolves.toBe("item-quantity")
    await expect(protocol.rejectedFixtureError("item-quantity-high")).resolves.toBe("item-quantity")
    await expect(protocol.rejectedFixtureError("badge-count")).resolves.toBe("circuit")
  })

  it("rejects a command while the storage state machine owns the game", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "cherrygrove-pokemon-center", x: 11, y: 2 },
      },
      party: [{ species: "rattata" }, { species: "pidgey" }],
      pc: { currentBox: 0, observedSlots: [{ box: 0, slot: 0, mon: null }] },
      determinism: { textSpeed: "instant" },
    })
    await openPcStorage(game, "move")

    await expect(game.arrange({ checkpoint: "new-bark-after-intro" })).rejects.toThrow(
      "Test ROM command arrange game failed during validate: busy",
    )
    await expect(game.state.read()).resolves.toMatchObject({
      storage: { open: true, ready: true, ui: "ready", mode: "move" },
    })
    await closePcStorage(game)
  })

  it("rejects a command while the battle state machine owns the game", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "route-41", x: 9, y: 21 } },
      party: [{ species: "lapras" }],
    })
    await game.battle.startWild({ species: "rattata", level: 5 })

    await expect(game.arrange({ checkpoint: "new-bark-after-intro" })).rejects.toThrow(
      "Test ROM command arrange game failed during validate: busy",
    )
    await game.battle.win()
    await finishBattle(game, "protocol busy-command battle")
  })

  it("observes ordinary and Hoenn-banked vars, and changes a banked var only from settled field state", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "route-41", x: 9, y: 21 } },
      party: [{ species: "rattata" }],
      story: { vars: { cherrygroveCityState: 3, hoennStarterChoice: 2 } },
    })
    await expect(game.story.var("cherrygroveCityState")).resolves.toBe(3)
    await expect(game.story.var("hoennStarterChoice")).resolves.toBe(2)

    await game.story.setVar("hoennStarterChoice", 1)
    await expect(game.story.var("hoennStarterChoice")).resolves.toBe(1)

    await game.battle.startWild({ species: "rattata", level: 5 })
    await expect(game.story.var("hoennStarterChoice")).resolves.toBe(1)
    await game.battle.win()
    await finishBattle(game, "protocol variable battle")
  })
})
