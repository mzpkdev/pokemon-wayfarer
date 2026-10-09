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

// Daily world slots: these Mt. Ember spots are dynamic, so what they hold depends on the day and
// they set no permanent flag. The journey pins the day and tries successive days until the spot
// holds a find (a quarter of days are empty), then checks the pickup across a save and reload.
const maxDaysTried = 12

describe.sequential("Wayfarer Sevii restored pickups", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  const pickUpOnSomeDay = async (x: number, y: number, facing: Direction): Promise<void> => {
    for (let day = 20000; day < 20000 + maxDaysTried; day++) {
      await game.dailySlots.pin({ day })
      await arrangeAt(game, x, y, facing)
      const sequence = (await game.state.read()).dialogue.sequence
      await game.player.interact()
      await game.wait.frames(30)
      if ((await game.state.read()).dialogue.sequence !== sequence) {
        await settleField(game, "pickup")
        return
      }
    }
    throw new Error(`no find at (${x}, ${y}) on ${maxDaysTried} consecutive days`)
  }

  it("picks up a Mt. Ember item ball once, across a save and reload, with no permanent flag", async () => {
    // FRLG places an Ultra Ball at (13, 6); the player faces it from below.
    await pickUpOnSomeDay(13, 7, "up")
    expect(await game.story.flag("seviiItemMtEmberExteriorUltraBall")).toBe(false)

    await game.saveAndReload()
    expect(await game.story.flag("seviiItemMtEmberExteriorUltraBall")).toBe(false)
    // The ball stays gone for the day: stepping onto its tile now succeeds.
    await game.player.move("up")
    await game.wait.frames(30)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: mtEmber },
      player: { x: 13, y: 6 },
    })
  })

  it("finds a Mt. Ember hidden item once, across a save and reload, with no permanent flag", async () => {
    // FRLG hides a Fire Stone in the rock at (18, 17).
    await pickUpOnSomeDay(18, 16, "down")
    expect(await game.story.flag("seviiHiddenMtEmberExteriorFireStone")).toBe(false)

    await game.saveAndReload()
    expect(await game.story.flag("seviiHiddenMtEmberExteriorFireStone")).toBe(false)
    const sequence = (await game.state.read()).dialogue.sequence
    await game.player.interact()
    await game.wait.frames(30)
    // Nothing is left to find today, so no pickup dialogue opens.
    expect((await game.state.read()).dialogue.sequence).toBe(sequence)
  })

  it("keeps a picked-up ball gone on camera steps after Continue", async () => {
    await pickUpOnSomeDay(13, 7, "up")
    await game.saveAndReload()
    // Walk away and back, ending in front of the tile: every camera step re-checks the spawn.
    for (const direction of ["down", "down", "up", "up"] as const) await game.player.move(direction)
    await game.wait.frames(30)
    const sequence = (await game.state.read()).dialogue.sequence
    await game.player.interact()
    await game.wait.frames(30)
    expect((await game.state.read()).dialogue.sequence).toBe(sequence)
  })
})
