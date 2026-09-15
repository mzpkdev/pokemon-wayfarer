import { afterEach, beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

describe("Wayfarer Trainer Tower run lifecycle", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
  })

  afterEach(async () => {
    await game.close()
  })

  it("starts through the lobby entry and abandons through the exterior door", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      party: [{ species: "lapras", level: 55 }],
      player: {
        facing: "up",
        position: { map: "sevii-trainer-tower-lobby", x: 9, y: 8 },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.move("up")
    await game.wait.until(
      (state) => state.dialogueOpen && state.dialogue.text.startsWith("Would you like"),
      "Trainer Tower challenge prompt",
    )
    await game.controls.press("a")
    await game.wait.frames(30)
    await game.controls.press("a")
    await game.wait.frames(30)
    await game.controls.press("a") // Single Battle
    await game.wait.until(
      (state) => state.dialogueOpen && state.dialogue.text.startsWith("On your marks"),
      "Trainer Tower start clock",
    )
    await game.controls.press("a")
    await game.wait.frames(30)
    await game.controls.press("a")
    await game.wait.forReady()

    await game.player.warp("sevii-trainer-tower-lobby", 9, 14, "down")
    await game.player.move("down")
    await game.wait.frames(16)
    await game.player.move("down")
    await game.wait.until(
      (state) => state.dialogueOpen && state.dialogue.text.startsWith("Leave the challenge"),
      "active Trainer Tower exit confirmation",
    )
    expect((await game.state.read()).map.name).toBe("sevii-trainer-tower-lobby")

    await game.controls.press("a")
    await game.wait.frames(30)
    await game.controls.press("a")
    await game.wait.forMap("sevii-seven-island-trainer-tower")
    expect((await game.state.read()).dialogueOpen).toBe(false)

    await game.player.warp("sevii-trainer-tower-lobby", 9, 14, "down")
    await game.player.move("down")
    await game.wait.frames(16)
    await game.player.move("down")
    await game.wait.forMap("sevii-seven-island-trainer-tower")
  })
})
