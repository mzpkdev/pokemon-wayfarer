import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

describe.sequential("HNS HM party management", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("lets the Move Deleter remove Surf from the party's last Surf user", async () => {
    const dismissCurrentDialogue = async (description: string): Promise<void> => {
      for (let attempt = 0; attempt < 4; attempt++) {
        await game.controls.press("a")
        await game.wait.frames(12)
        if (!(await game.state.read()).dialogueOpen) return
      }
      throw new Error(`${description} did not finish after four confirmations`)
    }

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "blackthorn-move-deleter", x: 4, y: 5 },
      },
      party: [
        { species: "lapras", moves: ["surf", "waterfall", "none", "none"] },
        { species: "rattata", moves: ["cut", "none", "none", "none"] },
      ],
      challenge: { hmsOverwrite: true },
      determinism: { textSpeed: "instant" },
    })

    await expect(game.state.read()).resolves.toMatchObject({
      party: [
        { species: "lapras", moves: ["surf", "waterfall", "none", "none"] },
        { species: "rattata", moves: ["cut", "none", "none", "none"] },
      ],
      challenge: { hmsOverwrite: true },
    })

    let dialogueSequence = (await game.state.read()).dialogue.sequence
    await game.player.interact()
    dialogueSequence++
    await game.wait.until(
      (state) => state.dialogue.sequence === dialogueSequence && state.dialogueOpen,
      "Move Deleter invitation",
    )
    await dismissCurrentDialogue("Move Deleter invitation")
    await game.wait.frames(12)
    await game.controls.press("a")

    // Dismiss the prompt that asks which Pokémon should forget a move before
    // the ordinary party selector fades in.
    dialogueSequence++
    await game.wait.until(
      (state) => state.dialogue.sequence === dialogueSequence && state.dialogueOpen,
      "Move Deleter party prompt",
    )
    await dismissCurrentDialogue("Move Deleter party prompt")
    await game.controls.press("a")
    await game.wait.until((state) => state.ui.mode === "party-menu", "Move Deleter party selector")

    // The selector's first slot is Lapras, the sole party member that knows
    // Surf. Choosing it opens the move-selection prompt.
    await game.wait.frames(120)
    await game.controls.press("a")
    dialogueSequence++
    await game.wait.until(
      (state) => state.dialogue.sequence === dialogueSequence,
      "Move Deleter move prompt",
    )
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.until((state) => state.ui.mode === "summary", "Move Deleter move selector")

    // The summary-screen move chooser starts with Surf in move slot one.
    await game.wait.frames(120)
    await game.controls.press("a")
    dialogueSequence++
    await game.wait.until(
      (state) => state.dialogue.sequence === dialogueSequence && state.dialogueOpen,
      "Move Deleter confirmation",
    )
    await dismissCurrentDialogue("Move Deleter confirmation")
    await game.wait.frames(12)
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.party[0]?.moves[0] === "waterfall",
      "Surf is removed from Lapras",
    )

    await expect(game.state.read()).resolves.toMatchObject({
      party: [
        { species: "lapras", moves: ["waterfall", "none", "none", "none"] },
        { species: "rattata", moves: ["cut", "none", "none", "none"] },
      ],
    })
  })
})
