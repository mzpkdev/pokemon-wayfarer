import { beforeAll, describe, expect, it } from "webanvil/test"

import { type GameMap, GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"

// The Weather Institute plays in any order, so Team Aqua names Mt. Pyre only while that is true.
// The late captures wait for the crisis and the legendary Trainer Rating, not a Hall of Fame.

const READY_TR = 120 // WAYFARER_LEGENDARY_TR on the v0 scale the Wayfarer build runs on.
const CHANGING_WEATHER = "ridiculous to think"
const REPORT = "I track weather patterns"
const NO_ROOM = "no more room"

const instituteShelly = 3
const instituteScientist = 5
const rayquaza = 2

type Arrange = Parameters<GameSession["arrange"]>[0]
type Story = NonNullable<Arrange["story"]>

const arrange = (
  game: GameSession,
  map: GameMap,
  x: number,
  y: number,
  facing: "up" | "down" | "left" | "right",
  story: Story,
): Promise<void> =>
  game.arrange({
    checkpoint: "hoenn-before-rescue",
    player: { facing, position: { map, x, y } },
    story: {
      flags: { disableEncounters: true, disableTrainers: true, ...story.flags },
      vars: story.vars,
    },
    party: [{ species: "salamence", level: 100 }],
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })

const objectAt = (state: GameState, localId: number) =>
  state.objects.find((object) => object.localId === localId && object.visible)

/** Reads a scene through with A until `done` holds on the free field or in battle, returning every message. */
const readScene = async (
  game: GameSession,
  done: (state: GameState, transcript: string) => boolean,
  description: string,
): Promise<string> => {
  const messages: string[] = []
  let sequence = -1
  for (let attempt = 0; attempt < 240; attempt++) {
    await game.wait.frames(30)
    const state = await game.state.read()
    if (state.dialogueOpen && state.dialogue.sequence !== sequence) {
      sequence = state.dialogue.sequence
      messages.push(state.dialogue.fullText)
    }
    const free = state.ready && !state.scriptActive && !state.controlsLocked
    if ((free || state.battle.active) && done(state, messages.join("\n")))
      return messages.join("\n")
    await game.controls.press(state.dialogue.fullText.includes("nickname") ? "b" : "a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

const talk = async (game: GameSession, description: string): Promise<string> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  return readScene(game, (state) => !state.dialogueOpen, description)
}

const CASTFORM_EXPLAINED = "changes shape according"
const castformExplained = (state: GameState, transcript: string): boolean =>
  !state.dialogueOpen && transcript.includes(CASTFORM_EXPLAINED)

/** Beats Shelly from the tile her battle is staged on and reads the scene to Castform's handoff. */
const clearInstitute = async (game: GameSession): Promise<string> => {
  await game.player.interact()
  await readScene(game, (state) => state.battle.active, "Shelly's battle")
  await game.battle.win()
  const scene = await readScene(game, castformExplained, "the institute scene")
  await expect(game.story.flag("hoennReceivedCastform")).resolves.toBe(true)
  return scene
}

/** Runs from a battle through its ordinary menu. */
const flee = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active) return
    if (state.battle.ui === "action-menu") {
      const cursor = state.battle.cursor ?? 0
      if ((cursor & 1) === 0) await game.controls.press("right")
      if ((cursor & 2) === 0) await game.controls.press("down")
      await game.controls.press("a")
    } else if (state.battle.ui === "text") await game.controls.press("a")
    await game.wait.frames(15)
  }
  throw new Error(`could not flee: ${JSON.stringify(await game.state.read())}`)
}

const cleared = { hoennReceivedCastform: true }
const afterCrisis = { hoennSkyPillarState: 2, hoennSkyPillarCryDone: 1 }

describe.sequential("Wayfarer Hoenn Weather Institute and late legendaries", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("names Mt. Pyre only while it is a true lead", { timeout: 300_000 }, async () => {
    // Before Mt. Chimney the summit is empty: the order is to pull out.
    await arrange(game, "weather-institute-2f", 5, 6, "left", {})
    expect(objectAt(await game.state.read(), instituteShelly)).toBeDefined()
    let scene = await clearInstitute(game)
    expect(scene).toContain("pull out")
    expect(scene).not.toContain("MT. PYRE")
    await expect(game.story.var("hoennWeatherInstituteState")).resolves.toBe(1)

    // After Mt. Chimney and before the theft, Team Magma is on its way.
    await arrange(game, "weather-institute-2f", 5, 6, "left", {
      flags: { hoennMtChimneyDone: true },
    })
    scene = await clearInstitute(game)
    expect(scene).toContain("headed for\nMT. PYRE!")
    expect(scene).toContain("hurry to MT. PYRE")

    // After the theft the plan has been carried out.
    await arrange(game, "weather-institute-2f", 5, 6, "left", {
      flags: { hoennMtChimneyDone: true, hoennReceivedOrb: true },
    })
    scene = await clearInstitute(game)
    expect(scene).toContain("pull out")
    expect(scene).not.toContain("MT. PYRE")
  })

  it(
    "stages the occupation after the crisis with the scientist among the captives",
    {
      timeout: 120_000,
    },
    async () => {
      await arrange(game, "weather-institute-2f", 5, 6, "left", {
        flags: { hoennMtChimneyDone: true, hoennReceivedOrb: true },
        vars: { ...afterCrisis, trainerRating: READY_TR },
      })
      expect(objectAt(await game.state.read(), instituteScientist)).toMatchObject({ x: 1, y: 6 })
      await clearInstitute(game)
      expect(objectAt(await game.state.read(), instituteScientist)).toMatchObject({ x: 4, y: 6 })
      expect((await game.state.read()).party).toHaveLength(2)
    },
  )

  it(
    "keeps an unclaimed Castform at the monitor once the reports begin",
    {
      timeout: 120_000,
    },
    async () => {
      await arrange(game, "weather-institute-2f", 2, 3, "up", {
        vars: { ...afterCrisis, hoennWeatherInstituteState: 1, trainerRating: READY_TR },
      })
      expect(objectAt(await game.state.read(), instituteScientist)).toMatchObject({ x: 2, y: 2 })
      await game.player.interact()
      await readScene(game, castformExplained, "the Castform handoff")
      expect((await game.state.read()).party).toHaveLength(2)
      expect(await talk(game, "the report after Castform")).toContain(REPORT)
      expect((await game.state.read()).party).toHaveLength(2)
    },
  )

  it("keeps Castform claimable once after a full party and box", { timeout: 180_000 }, async () => {
    await arrange(game, "weather-institute-2f", 5, 6, "left", {})
    await game.storage.arrangeCapacity("full")

    await game.player.interact()
    await readScene(game, (state) => state.battle.active, "Shelly's battle")
    await game.battle.win()
    await readScene(
      game,
      (state, transcript) => !state.dialogueOpen && transcript.includes(NO_ROOM),
      "the full handoff",
    )
    await expect(game.story.flag("hoennReceivedCastform")).resolves.toBe(false)
    await expect(game.story.var("hoennWeatherInstituteState")).resolves.toBe(1)
    await game.saveAndReload()

    // A freed slot makes the same scientist hand it over, and only once.
    await game.storage.arrangeCapacity("one-pc-slot-free")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await readScene(game, castformExplained, "the claim after making room")
    await expect(game.story.flag("hoennReceivedCastform")).resolves.toBe(true)
    expect((await game.storage.slot(0, 0)).mon).not.toBeNull()
    expect((await game.state.read()).party).toHaveLength(6)
    expect(await talk(game, "the scientist afterwards")).toContain(CHANGING_WEATHER)
  })

  it(
    "reports abnormal weather on the crisis and the rating, never a Hall of Fame",
    {
      timeout: 240_000,
    },
    async () => {
      const keepsTheOldLine = async (flags: Story["flags"], vars: Story["vars"]) => {
        await arrange(game, "weather-institute-2f", 17, 6, "right", {
          flags: { ...cleared, ...flags },
          vars: { hoennWeatherInstituteState: 2, ...vars },
        })
        expect(objectAt(await game.state.read(), instituteScientist)).toMatchObject({ x: 18, y: 6 })
        expect(await talk(game, "the scientist")).toContain(CHANGING_WEATHER)
        await expect(game.story.var("hoennAbnormalWeatherLocation")).resolves.toBe(0)
      }
      const halls = { hoennGameClear: true, systemGameClear: true }

      await keepsTheOldLine({}, { ...afterCrisis, trainerRating: READY_TR - 1 })
      await keepsTheOldLine(halls, { ...afterCrisis, trainerRating: READY_TR - 1 })
      await keepsTheOldLine(halls, { trainerRating: READY_TR })
    },
  )

  it("follows the same rule in the first-floor hint", { timeout: 120_000 }, async () => {
    const hint = async (flags: Story["flags"], rating: number) => {
      await arrange(game, "weather-institute-1f", 2, 12, "up", {
        flags: { ...flags, hoennHideInstituteDownstairsWorkers: false },
        vars: { ...afterCrisis, hoennWeatherInstituteState: 2, trainerRating: rating },
      })
      return talk(game, "the first-floor worker")
    }

    expect(await hint({ hoennGameClear: true }, READY_TR - 1)).toContain("loves rain")
    expect(await hint({}, READY_TR)).toContain("droughts and\nheavy rain")
    expect(
      await hint({ hoennDefeatedKyogre: true, hoennDefeatedGroudon: true }, READY_TR),
    ).toContain("loves rain")
  })

  it("follows the same rule in the harbor Sailor's rumor", { timeout: 120_000 }, async () => {
    const rumor = async (flags: Story["flags"], rating: number) => {
      await arrange(game, "slateport-city-harbor", 5, 12, "left", {
        flags: { ...flags, hoennHideHarborPatrons: false },
        vars: { ...afterCrisis, trainerRating: rating },
      })
      return talk(game, "the harbor Sailor")
    }

    expect(await rumor({ hoennGameClear: true }, READY_TR - 1)).toContain("bottom of the sea")
    expect(await rumor({}, READY_TR)).toContain("WEATHER INSTITUTE")
    expect(
      await rumor({ hoennDefeatedKyogre: true, hoennDefeatedGroudon: true }, READY_TR),
    ).toContain("bottom of the sea")
  })

  it("opens Terra Cave from the scientist's report", { timeout: 180_000 }, async () => {
    // Kyogre beaten, so the report can only be a drought over one of these eight entrances.
    const entrances: [GameMap, number, number][] = [
      ["route-114", 7, 4],
      ["route-114", 6, 46],
      ["route-115", 21, 6],
      ["route-115", 36, 10],
      ["route-116", 59, 13],
      ["route-116", 79, 6],
      ["route-118", 42, 6],
      ["route-118", 9, 6],
    ]
    await arrange(game, "weather-institute-2f", 2, 3, "up", {
      flags: { ...cleared, hoennDefeatedKyogre: true },
      vars: { ...afterCrisis, hoennWeatherInstituteState: 2, trainerRating: READY_TR },
    })
    const report = await talk(game, "the scientist's report")
    expect(report).toContain("a drought has been recorded")

    const location = await game.story.var("hoennAbnormalWeatherLocation")
    const [route, x, y] = entrances[location - 1]!
    await game.player.warp(route, x, y + 1, "up")
    await game.wait.forReady()
    await game.player.move("up")
    await game.wait.forMap("terra-cave-entrance")
    await game.wait.forReady()

    // The way out returns to the same entrance.
    await game.player.move("down")
    await game.wait.forMap(route)
    await game.wait.forReady()
    expect((await game.state.read()).player).toMatchObject({ x, y: y + 1 })
  })

  it("keeps Groudon in its cave after the player flees", { timeout: 120_000 }, async () => {
    const meetGroudon = async () => {
      await game.player.move("up")
      await readScene(game, (state) => state.battle.active, "Groudon's battle")
      expect((await game.state.read()).battle.enemy?.species).toBe("groudon")
    }

    await arrange(game, "terra-cave-end", 17, 27, "up", {
      vars: { ...afterCrisis, hoennAbnormalWeatherLocation: 1, trainerRating: READY_TR },
    })
    await meetGroudon()
    await flee(game)
    await readScene(game, (state) => !state.battle.active && !state.dialogueOpen, "the flight")
    await expect(game.story.flag("hoennDefeatedGroudon")).resolves.toBe(false)
    await expect(game.story.var("hoennAbnormalWeatherLocation")).resolves.toBe(1)

    await game.player.warp("terra-cave-end", 17, 27, "up")
    await game.wait.forReady()
    await meetGroudon()
    await flee(game)
    await readScene(game, (state) => !state.battle.active && !state.dialogueOpen, "the flight")
  })

  it(
    "ends the weather on its route, and for a player leaving the cave",
    {
      timeout: 120_000,
    },
    async () => {
      const readEnding = async () => {
        await readScene(game, (state) => !state.dialogueOpen, "the weather's end")
        expect((await game.state.read()).dialogue.fullText).toContain("appears to\nhave subsided")
        await expect(game.story.var("hoennAbnormalWeatherLocation")).resolves.toBe(0)
        await expect(game.story.var("hoennAbnormalWeatherShouldEnd")).resolves.toBe(0)
      }

      // The event's last step, taken on the road below the entrance at (7,4).
      await arrange(game, "route-114", 7, 6, "up", {
        vars: { hoennAbnormalWeatherLocation: 1, hoennAbnormalWeatherSteps: 999 },
      })
      await game.player.move("up")
      await readEnding()

      // The entrance is a wall again.
      await game.player.move("up")
      await game.wait.frames(60)
      expect(await game.state.read()).toMatchObject({
        map: { name: "route-114" },
        player: { x: 7, y: 5 },
      })

      // Marked to end while the player was inside: the route clears it once they are back.
      await arrange(game, "route-114", 7, 9, "up", {
        vars: { hoennAbnormalWeatherLocation: 1 },
      })
      await game.story.setVar("hoennAbnormalWeatherShouldEnd", 1)
      await game.wait.frames(60)
      await readEnding()
    },
  )

  it(
    "leaves Rayquaza at the top of Sky Pillar until the player is ready",
    {
      timeout: 180_000,
    },
    async () => {
      await arrange(game, "sky-pillar-top", 14, 7, "up", {
        flags: { hoennGameClear: true },
        vars: { ...afterCrisis, trainerRating: READY_TR - 1 },
      })
      expect(objectAt(await game.state.read(), rayquaza)).toBeDefined()
      const notReady = await talk(game, "Rayquaza's not-ready line")
      expect(notReady).toContain("RAYQUAZA watches you warily")
      expect((await game.state.read()).battle.active).toBe(false)
      expect(objectAt(await game.state.read(), rayquaza)).toBeDefined()

      await game.story.setVar("trainerRating", READY_TR)
      await game.player.interact()
      await readScene(game, (state) => state.battle.active, "Rayquaza's battle")
      expect((await game.state.read()).battle.enemy?.species).toBe("rayquaza")

      // Fleeing keeps today's behavior: it flies off, and is back on the next visit.
      await flee(game)
      await readScene(game, (state) => !state.battle.active && !state.dialogueOpen, "the flight")
      expect(objectAt(await game.state.read(), rayquaza)).toBeUndefined()
      await expect(game.story.flag("hoennDefeatedRayquaza")).resolves.toBe(false)
      await game.player.warp("sky-pillar-top", 16, 13, "up")
      await game.wait.forReady()
      expect(objectAt(await game.state.read(), rayquaza)).toBeDefined()
    },
  )
})
