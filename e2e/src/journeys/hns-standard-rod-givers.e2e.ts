import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Button, type GameMap } from "../harness/game-session"
import { beginWayfarerRegularAquaDeparture } from "../playbooks/wayfarer-ports"

const emptyRods = { oldRod: 0, goodRod: 0, superRod: 0 }

const contributionFlagNames = [
  "standardRodRoute32Contributed",
  "standardRodOlivineContributed",
  "standardRodRoute12Contributed",
  "standardRodDewfordContributed",
  "standardRodRoute118Contributed",
  "standardRodMossdeepContributed",
] as const

type ContributorFlag = (typeof contributionFlagNames)[number]

const contributionState = (...enabled: ContributorFlag[]): Record<ContributorFlag, boolean> =>
  Object.fromEntries(contributionFlagNames.map((flag) => [flag, enabled.includes(flag)])) as Record<
    ContributorFlag,
    boolean
  >

const contributionFlags = (game: GameSession): Promise<boolean[]> =>
  Promise.all(contributionFlagNames.map((flag) => game.story.flag(flag)))

const expectedContributionFlags = (...enabled: ContributorFlag[]): boolean[] =>
  contributionFlagNames.map((flag) => enabled.includes(flag))

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
  throw new Error(
    `Dialogue did not reach ${JSON.stringify(expectedText)}: ${JSON.stringify(await game.state.read())}`,
  )
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

const hoennGivers: ReadonlyArray<{
  name: string
  flag: ContributorFlag
  map: GameMap
  x: number
  y: number
  facing: "up" | "down" | "left" | "right"
}> = [
  {
    name: "Dewford",
    flag: "standardRodDewfordContributed",
    map: "dewford-town",
    x: 11,
    y: 14,
    facing: "right",
  },
  {
    name: "Route 118",
    flag: "standardRodRoute118Contributed",
    map: "route-118",
    x: 29,
    y: 8,
    facing: "left",
  },
  {
    name: "Mossdeep",
    flag: "standardRodMossdeepContributed",
    map: "mossdeep-house-3",
    x: 3,
    y: 4,
    facing: "right",
  },
]

describe.sequential("Wayfarer Standard Rod givers", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("shares Old, Good, and Super progression through a Hoenn-first mixed route", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "right", position: { map: "dewford-town", x: 11, y: 14 } },
      story: { flags: contributionState() },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    const declinedOffer = (await game.state.read()).dialogue.sequence
    await waitForNextDialogue(game, declinedOffer, "b")
    await expect(contributionFlags(game)).resolves.toEqual(expectedContributionFlags())
    await expect(game.inventory.rodSlots()).resolves.toEqual(emptyRods)
    await finishScript(game)

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await waitForDialogueText(game, "Obtained the OLD ROD!")
    await expect(contributionFlags(game)).resolves.toEqual(
      expectedContributionFlags("standardRodDewfordContributed"),
    )
    await expect(game.inventory.rodSlots()).resolves.toEqual({ oldRod: 1, goodRod: 0, superRod: 0 })
    await waitForDialogueText(game, "And, as an added bonus")
    await finishScript(game)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "route-12-house", x: 7, y: 5 } },
      story: { flags: contributionState("standardRodDewfordContributed") },
      bag: { items: { oldRod: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await waitForDialogueText(game, "Obtained the GOOD ROD!")
    await expect(contributionFlags(game)).resolves.toEqual(
      expectedContributionFlags("standardRodRoute12Contributed", "standardRodDewfordContributed"),
    )
    await expect(game.inventory.rodSlots()).resolves.toEqual({ oldRod: 0, goodRod: 1, superRod: 0 })
    await finishScript(game)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "right", position: { map: "mossdeep-house-3", x: 3, y: 4 } },
      story: {
        flags: contributionState("standardRodRoute12Contributed", "standardRodDewfordContributed"),
      },
      bag: { items: { goodRod: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await waitForDialogueText(game, "Obtained the SUPER ROD!")
    await expect(contributionFlags(game)).resolves.toEqual(
      expectedContributionFlags(
        "standardRodRoute12Contributed",
        "standardRodDewfordContributed",
        "standardRodMossdeepContributed",
      ),
    )
    await expect(game.inventory.rodSlots()).resolves.toEqual({ oldRod: 0, goodRod: 0, superRod: 1 })
    await finishScript(game)
  })

  for (const giver of hoennGivers) {
    it(`${giver.name} uses its dedicated contribution flag for the first rod`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: giver.facing,
          position: { map: giver.map, x: giver.x, y: giver.y },
        },
        story: { flags: contributionState() },
        determinism: { textSpeed: "instant" },
      })

      await game.player.interact()
      await game.dialogue.waitForOpen()
      await waitForDialogueText(game, "Obtained the OLD ROD!")
      await expect(contributionFlags(game)).resolves.toEqual(expectedContributionFlags(giver.flag))
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
      await expect(contributionFlags(game)).resolves.toEqual(expectedContributionFlags(giver.flag))
      await expect(game.inventory.rodSlots()).resolves.toEqual({
        oldRod: 1,
        goodRod: 0,
        superRod: 0,
      })
      await finishScript(game)
    })
  }

  it("keeps Dewford's tutorial available for an unused giver after the cap", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "right", position: { map: "dewford-town", x: 11, y: 14 } },
      story: {
        flags: contributionState(
          "standardRodRoute32Contributed",
          "standardRodOlivineContributed",
          "standardRodRoute12Contributed",
        ),
      },
      bag: { items: { superRod: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await waitForDialogueText(game, "And, as an added bonus")
    await expect(contributionFlags(game)).resolves.toEqual(
      expectedContributionFlags(
        "standardRodRoute32Contributed",
        "standardRodOlivineContributed",
        "standardRodRoute12Contributed",
      ),
    )
    await expect(game.inventory.rodSlots()).resolves.toEqual({ oldRod: 0, goodRod: 0, superRod: 1 })
    await finishScript(game)
  })

  it("keeps all six contributor states while traveling from Kanto to Hoenn", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "vermilion-port-inside", x: 8, y: 9 },
      },
      story: {
        flags: contributionState(
          "standardRodDewfordContributed",
          "standardRodRoute118Contributed",
          "standardRodMossdeepContributed",
        ),
        vars: { ssAquaState: 8 },
      },
      bag: { items: { superRod: 1, ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await beginWayfarerRegularAquaDeparture(game)
    await game.dialogue.waitForOpen()
    await advanceUntilMap(game, "slateport-city-harbor")

    await expect(contributionFlags(game)).resolves.toEqual(
      expectedContributionFlags(
        "standardRodDewfordContributed",
        "standardRodRoute118Contributed",
        "standardRodMossdeepContributed",
      ),
    )
    await expect(game.inventory.rodSlots()).resolves.toEqual({ oldRod: 0, goodRod: 0, superRod: 1 })

    await game.saveAndReload()
    await expect(contributionFlags(game)).resolves.toEqual(
      expectedContributionFlags(
        "standardRodDewfordContributed",
        "standardRodRoute118Contributed",
        "standardRodMossdeepContributed",
      ),
    )
    await expect(game.inventory.rodSlots()).resolves.toEqual({ oldRod: 0, goodRod: 0, superRod: 1 })
  })
})
