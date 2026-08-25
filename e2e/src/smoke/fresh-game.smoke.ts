import { beforeAll, context, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { startNewGame } from "../playbooks/new-game"

describe.sequential("a new game", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  context("when started from the title screen", () => {
    it("reaches the player's bedroom in New Bark Town", async () => {
      await startNewGame(game)
      await game.wait.forMap("players-bedroom")

      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: "players-bedroom" },
      })
    })
  })
})
