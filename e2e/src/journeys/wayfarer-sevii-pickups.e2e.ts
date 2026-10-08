import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction } from "../harness/game-session"

const mtEmber = "sevii-mt-ember-exterior" as const

const settleField = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.scriptActive && !state.dialogueOpen) return
    if (state.dialogueOpen || state.scriptActive) await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not settle: ${JSON.stringify(await game.state.read())}`)
}

const arrangeAt = async (
  game: GameSession,
  x: number,
  y: number,
  facing: Direction,
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing, position: { map: mtEmber, x, y } },
    story: {
      flags: {
        disableEncounters: true,
        seviiItemMtEmberExteriorUltraBall: false,
        seviiHiddenMtEmberExteriorFireStone: false,
      },
    },
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
}

describe.sequential("Wayfarer Sevii restored pickups", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("picks up a Mt. Ember item ball once, across a save and reload", async () => {
    // FRLG places this Ultra Ball at (13, 6); the player faces it from below.
    await arrangeAt(game, 13, 7, "up")
    expect(await game.inventory.contains("ultraBall")).toBe(false)

    await game.player.interact()
    await settleField(game, "Ultra Ball pickup")
    expect(await game.inventory.contains("ultraBall")).toBe(true)
    expect(await game.story.flag("seviiItemMtEmberExteriorUltraBall")).toBe(true)

    await game.saveAndReload()
    expect(await game.story.flag("seviiItemMtEmberExteriorUltraBall")).toBe(true)
    // The ball stays gone: stepping onto its tile now succeeds.
    await game.player.move("up")
    await game.wait.frames(30)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: mtEmber },
      player: { x: 13, y: 6 },
    })
  })

  it("finds a Mt. Ember hidden item once, across a save and reload", async () => {
    // FRLG hides a Fire Stone in the rock at (18, 17).
    await arrangeAt(game, 18, 16, "down")
    await game.player.interact()
    await settleField(game, "hidden Fire Stone pickup")
    expect(await game.story.flag("seviiHiddenMtEmberExteriorFireStone")).toBe(true)
    expect(await game.story.flag("seviiItemMtEmberExteriorUltraBall")).toBe(false)

    await game.saveAndReload()
    expect(await game.story.flag("seviiHiddenMtEmberExteriorFireStone")).toBe(true)
    const sequence = (await game.state.read()).dialogue.sequence
    await game.player.interact()
    await game.wait.frames(30)
    // Nothing is left to find, so no pickup dialogue opens.
    expect((await game.state.read()).dialogue.sequence).toBe(sequence)
  })
})
