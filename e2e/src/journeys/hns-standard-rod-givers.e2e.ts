import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Button, type GameMap } from "../harness/game-session"

const emptyRods = { oldRod: 0, goodRod: 0, superRod: 0 }

const contributionFlags = (game: GameSession): Promise<boolean[]> =>
  Promise.all([
    game.story.flag("standardRodRoute32Contributed"),
    game.story.flag("standardRodOlivineContributed"),
    game.story.flag("standardRodRoute12Contributed"),
  ])

const waitForNextDialogue = async (
  game: GameSession,
  sequence: number,
  button: Button,
): Promise<void> => {
  for (let attempt = 0; attempt < 20; attempt++) {
    await game.wait.frames(30)
    await game.controls.press(button)
    await game.wait.frames(8)
    if ((await game.state.read()).dialogue.sequence > sequence) return
  }
  throw new Error(`Dialogue did not advance from sequence ${sequence}`)
}

const waitForDialogueText = async (game: GameSession, expectedText: string): Promise<void> => {
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text.includes(expectedText)) return
    await game.wait.frames(30)
    await game.controls.press("a")
    await game.wait.frames(8)
  }
  throw new Error(`Dialogue did not reach ${JSON.stringify(expectedText)}`)
}

const finishScript = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen) return
    await game.wait.frames(state.dialogueOpen ? 30 : 12)
    await game.controls.press("a")
  }
  throw new Error(
    `Field script did not return control to the player: ${JSON.stringify(await game.state.read())}`,
  )
}

const advanceUntilMap = async (game: GameSession, map: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 60; attempt++) {
    const state = await game.state.read()
    if (state.ready && state.map.name === map) return
    await game.wait.frames(30)
    await game.controls.press("a")
  }
  throw new Error(`Field script did not reach ${map}: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("HNS Standard Rod givers", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("awards Old, Good, then Super Rod in the Route 12, Route 32, Olivine order", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "route-12-house", x: 7, y: 5 },
      },
      story: {
        flags: {
          standardRodRoute32Contributed: false,
          standardRodOlivineContributed: false,
          standardRodRoute12Contributed: false,
        },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    const declinedOffer = (await game.state.read()).dialogue.sequence
    await waitForNextDialogue(game, declinedOffer, "b")
    await expect(contributionFlags(game)).resolves.toEqual([false, false, false])
    await expect(game.inventory.rodSlots()).resolves.toEqual(emptyRods)
    await finishScript(game)

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await waitForDialogueText(game, "Obtained the OLD ROD!")
    await expect(contributionFlags(game)).resolves.toEqual([false, false, true])
    await expect(game.inventory.rodSlots()).resolves.toEqual({
      oldRod: 1,
      goodRod: 0,
      superRod: 0,
    })
    await finishScript(game)

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await expect(game.state.read()).resolves.toMatchObject({
      dialogue: { text: expect.not.stringContaining("Obtained the") },
    })
    await finishScript(game)
    await expect(contributionFlags(game)).resolves.toEqual([false, false, true])
    await expect(game.inventory.rodSlots()).resolves.toEqual({
      oldRod: 1,
      goodRod: 0,
      superRod: 0,
    })

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "left",
        position: { map: "route-32-pokemon-center", x: 3, y: 5 },
      },
      story: {
        flags: {
          standardRodRoute32Contributed: false,
          standardRodOlivineContributed: false,
          standardRodRoute12Contributed: true,
        },
      },
      bag: { items: { oldRod: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await waitForDialogueText(game, "Obtained the GOOD ROD!")
    await expect(contributionFlags(game)).resolves.toEqual([true, false, true])
    await expect(game.inventory.rodSlots()).resolves.toEqual({
      oldRod: 0,
      goodRod: 1,
      superRod: 0,
    })
    await finishScript(game)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "olivine-house-3", x: 4, y: 5 },
      },
      story: {
        flags: {
          standardRodRoute32Contributed: true,
          standardRodOlivineContributed: false,
          standardRodRoute12Contributed: true,
        },
      },
      bag: { items: { goodRod: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await waitForDialogueText(game, "Obtained the SUPER ROD!")
    await expect(contributionFlags(game)).resolves.toEqual([true, true, true])
    await expect(game.inventory.rodSlots()).resolves.toEqual({
      oldRod: 0,
      goodRod: 0,
      superRod: 1,
    })
    await finishScript(game)
  })

  it("keeps all three contribution flags through the S.S. Aqua Kanto transition", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "ss-aqua-1f", x: 29, y: 3 },
      },
      story: {
        flags: {
          standardRodRoute32Contributed: true,
          standardRodOlivineContributed: true,
          standardRodRoute12Contributed: true,
        },
        vars: { ssAquaState: 7 },
      },
      bag: { items: { superRod: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await advanceUntilMap(game, "vermilion-port-inside")

    await expect(contributionFlags(game)).resolves.toEqual([true, true, true])
    await expect(game.inventory.rodSlots()).resolves.toEqual({
      oldRod: 0,
      goodRod: 0,
      superRod: 1,
    })
  })
})
