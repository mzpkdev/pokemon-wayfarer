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
  bag?: NonNullable<Parameters<GameSession["arrange"]>[0]["bag"]>,
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing, position: { map: mtEmber, x, y } },
    bag,
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

  it("leaves a dynamic ball in place when the bag is full, then gives it once there is room", async () => {
    const pockets = ["balls", "items", "tmHm", "keyItems"] as const
    // The find's pocket depends on the day, and two pockets cannot be filled by the harness: try days
    // until one lands in a full pocket.
    for (let day = 20000; day < 20000 + maxDaysTried; day++) {
      await game.dailySlots.pin({ day, noEmpty: true })
      await arrangeAt(game, 13, 7, "up", { fullPockets: [...pockets] })
      const before = (await game.dailySlots.found()).count
      await game.player.interact()
      await game.wait.frames(30)
      await settleField(game, "full-bag pickup")
      if ((await game.dailySlots.found()).count !== before) continue
      // No room: the ball stays, and nothing is marked picked up.
      await game.saveAndReload()
      for (const pocket of pockets) await game.inventory.freeSlot(pocket)
      await game.player.interact()
      await game.wait.frames(30)
      await settleField(game, "pickup with room")
      expect((await game.dailySlots.found()).count).toBe(1)
      return
    }
    throw new Error(`no find landed in a full pocket on ${maxDaysTried} consecutive days`)
  })

  it("finds Cape Brink's PP Max prize from the water", async () => {
    // The lone tile at (16, 28) is walled in on two sides and open water on the others, so the
    // hidden item has elevation 0 and is found while surfing from (15, 28), facing east.
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "right", position: { map: "sevii-two-island-cape-brink", x: 15, y: 28 } },
      story: { flags: { disableEncounters: true } },
      determinism: { textSpeed: "instant", rngSeed: 1 },
    })
    await game.wait.forReady()
    expect((await game.state.read()).player.surfing).toBe(true)
    expect(await game.inventory.contains("ppMax")).toBe(false)
    await game.player.interact()
    await settleField(game, "Cape Brink PP Max")
    expect(await game.inventory.contains("ppMax")).toBe(true)
  })
})
