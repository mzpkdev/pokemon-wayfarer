import { beforeAll, describe, expect, it } from "webanvil/test"

import { type GameMap, GameSession } from "../harness/game-session"
import { type ArrangeGame } from "../harness/game-session/features/arrange"
import { type GameState } from "../harness/game-session/features/state"
import { advanceOpeningUntil, stepOpening } from "../playbooks/regional-opening"

// Hoenn's villain story in any order: Mt. Pyre waits for Mt. Chimney, Tate & Liza's win only awards
// the badge, the Space Center invasion arms on entering Mossdeep, and the Seafloor Cavern opens once
// Aqua has escaped and the player has Steven's Dive.

const summitArchie = 2
const mossdeepMaxie = 14
const hideoutMatt = 1
const seafloorGrunt = 1

const sees = (state: GameState, localId: number): boolean =>
  state.objects.some((object) => object.localId === localId && object.visible)

const expectFree = async (game: GameSession): Promise<void> => {
  const state = await game.state.read()
  expect(state).toMatchObject({
    ready: true,
    controlsLocked: false,
    scriptActive: false,
    dialogueOpen: false,
  })
  expect(state.battle.active).toBe(false)
}

/** Reads a scene through with A until `done`, then until the field is free again. */
const playSceneUntil = async (
  game: GameSession,
  done: () => Promise<boolean>,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 160; attempt++) {
    await game.wait.frames(45)
    const state = await game.state.read()
    // The ready bit can report true between two pages, so the scene's own state decides.
    if (state.ready && !state.scriptActive && !state.controlsLocked && (await done())) return
    await game.controls.press("a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

const arrangeAt = async (
  game: GameSession,
  map: GameMap,
  x: number,
  y: number,
  facing: "up" | "down" | "left" | "right",
  story: NonNullable<ArrangeGame["story"]>["flags"] = {},
  vars: NonNullable<ArrangeGame["story"]>["vars"] = {},
  bag: ArrangeGame["bag"] = {},
): Promise<void> => {
  await game.arrange({
    checkpoint: "hoenn-before-rescue",
    player: { facing, position: { map, x, y } },
    story: {
      flags: { disableEncounters: true, disableTrainers: true, ...story },
      vars,
    },
    party: [
      { species: "salamence", level: 100 },
      { species: "salamence", level: 100 },
    ],
    bag,
    determinism: { textSpeed: "instant" },
  })
  await game.wait.forReady()
}

/** Talks to whoever the player faces, reads the line through, and returns its opening text. */
const listen = async (game: GameSession): Promise<string> => {
  await game.player.interact()
  await game.wait.until((state) => state.dialogueOpen, "a line")
  let heard = ""
  for (let attempt = 0; attempt < 40; attempt++) {
    await game.wait.frames(30)
    const state = await game.state.read()
    if (!state.dialogueOpen && !state.scriptActive && !state.controlsLocked) return heard
    heard += ` ${state.dialogue.fullText}`
    await game.controls.press("a")
  }
  throw new Error(`The line did not end: ${heard}`)
}

const battleThroughTateAndLiza = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await advanceOpeningUntil(game, (state) => state.battle.active, "Tate & Liza did not battle")
  await game.battle.win()
  await playSceneUntil(
    game,
    () => game.story.flag("hoennMindBadge"),
    "The Mind Badge scene did not finish",
  )
}

/** Walks out of the Gym's door, from the tile above its doormat. */
const stepOutOfGym = async (game: GameSession): Promise<void> => {
  await stepOpening(game, "down")
  await game.player.move("down")
  await game.wait.forMap("mossdeep-city")
  await game.wait.forReady()
}

const leaveGym = async (game: GameSession): Promise<void> => {
  await game.player.warp("mossdeep-gym", 7, 34, "down")
  await game.wait.forReady()
  await stepOutOfGym(game)
}

describe.sequential("Wayfarer Hoenn villain story order", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it(
    "keeps Mt. Pyre's summit quiet until Mt. Chimney, then plays the theft once",
    { timeout: 240_000 },
    async () => {
      // Before Mt. Chimney the summit is open: the theft's triggers do nothing.
      await arrangeAt(game, "mt-pyre-summit", 23, 9, "up")
      expect(sees(await game.state.read(), summitArchie)).toBe(false)
      for (let step = 0; step < 3; step++) await stepOpening(game, "up")
      await game.wait.frames(60)
      await expectFree(game)
      await expect(game.story.var("hoennMtPyreState")).resolves.toBe(0)
      expect(await listen(game)).toContain("in our care")

      // With Mt. Chimney done, Archie is there and the same step plays the theft.
      await arrangeAt(game, "mt-pyre-summit", 23, 8, "up", { hoennMtChimneyDone: true })
      expect(sees(await game.state.read(), summitArchie)).toBe(true)
      await game.player.move("up")
      await playSceneUntil(
        game,
        () => game.story.flag("hoennReceivedOrb"),
        "The theft did not finish",
      )
      await expect(game.story.var("hoennMtPyreState")).resolves.toBe(1)
      await expect(game.inventory.contains("magmaEmblem")).resolves.toBe(true)

      // She mourns the orbs now, and the save keeps it that way.
      await game.saveAndReload()
      expect(await listen(game)).toContain("sinister")
    },
  )

  it(
    "leaves the Magma Emblem with the old lady when the Bag was full",
    { timeout: 120_000 },
    async () => {
      await arrangeAt(
        game,
        "mt-pyre-summit",
        23,
        8,
        "up",
        { hoennMtChimneyDone: true },
        {},
        {
          fullPockets: ["keyItems"],
        },
      )
      await game.player.move("up")
      await playSceneUntil(
        game,
        () => game.story.flag("hoennReceivedOrb"),
        "The theft did not finish",
      )
      await expect(game.inventory.contains("magmaEmblem")).resolves.toBe(false)
      expect(await listen(game)).toContain("left this behind")
      await expect(game.inventory.contains("magmaEmblem")).resolves.toBe(false)

      // With room, she hands it over once.
      await arrangeAt(
        game,
        "mt-pyre-summit",
        23,
        6,
        "up",
        {
          hoennMtChimneyDone: true,
          hoennReceivedOrb: true,
        },
        { hoennMtPyreState: 1 },
      )
      await listen(game)
      await expect(game.inventory.contains("magmaEmblem")).resolves.toBe(true)
      expect(await listen(game)).toContain("sinister")
    },
  )

  it("names Mt. Pyre at the hideout door only while it is a true lead", async () => {
    await arrangeAt(game, "aqua-hideout-1f", 14, 12, "up")
    expect(await listen(game)).not.toContain("MT. PYRE")
    await arrangeAt(game, "aqua-hideout-1f", 14, 12, "up", { hoennMtChimneyDone: true })
    expect(await listen(game)).toContain("MT. PYRE")
    await arrangeAt(game, "aqua-hideout-1f", 14, 12, "up", {
      hoennMtChimneyDone: true,
      hoennReceivedOrb: true,
    })
    expect(await listen(game)).not.toContain("MT. PYRE")
  })

  it("has Archie recall the museum only after the museum", { timeout: 180_000 }, async () => {
    for (const [delivered, line] of [
      [false, "who might you be"],
      [true, "see you before"],
    ] as const) {
      await arrangeAt(game, "meteor-falls-1f-1r", 14, 19, "up", {
        hoennDeliveredDevonGoods: delivered,
      })
      await game.player.move("up")
      await advanceOpeningUntil(
        game,
        (state) => state.dialogue.fullText.includes(line),
        `Archie did not say "${line}"`,
      )
      await playSceneUntil(
        game,
        async () => (await game.story.var("hoennMeteorFallsState")) === 1,
        "The Meteor Falls scene did not finish",
      )
    }
  })

  it(
    "gives only the badge for Tate & Liza's win before Magma Hideout, and keeps the Aqua branch whole",
    { timeout: 420_000 },
    async () => {
      await arrangeAt(game, "mossdeep-gym", 23, 8, "up", { hoennHideHarborPatrons: true })
      await battleThroughTateAndLiza(game)

      // The win touches nothing of the villain story.
      await expect(game.story.var("hoennMossdeepCityState")).resolves.toBe(0)
      await expect(game.story.var("hoennMossdeepSpaceCenterState")).resolves.toBe(0)
      await expect(game.story.flag("hoennHideAquaHideoutGrunts")).resolves.toBe(false)
      await expect(game.story.flag("hoennHideHarborPatrons")).resolves.toBe(true)
      await expect(game.story.flag("hoennHideShipyardBriney")).resolves.toBe(true)

      // Mossdeep stays quiet, including across a save, and the road by the Space Center is open.
      await game.saveAndReload()
      await leaveGym(game)
      await game.player.warp("mossdeep-city", 38, 25, "right")
      await game.wait.forReady()
      expect(sees(await game.state.read(), mossdeepMaxie)).toBe(false)
      for (let step = 0; step < 3; step++) await stepOpening(game, "right")
      await game.wait.frames(60)
      await expectFree(game)
      await expect(game.story.var("hoennMossdeepCityState")).resolves.toBe(0)

      // Lilycove talks of the hideout's fall by Aqua's escape, not by the badge.
      await game.player.warp("lilycove-pokemon-center", 10, 8, "right")
      await game.wait.forReady()
      expect(await listen(game)).toContain("been hearing")

      // Matt is still at home, and his defeat is what empties the hideout.
      await game.player.warp("aqua-hideout-b2f", 24, 19, "left")
      await game.wait.forReady()
      expect(sees(await game.state.read(), hideoutMatt)).toBe(true)
      await game.player.interact()
      await advanceOpeningUntil(game, (state) => state.battle.active, "Matt did not battle")
      await game.battle.win()
      await playSceneUntil(game, () => game.story.flag("hoennAquaEscaped"), "Aqua did not escape")
      await expect(game.story.flag("hoennHideAquaHideoutGrunts")).resolves.toBe(true)
      await game.player.warp("aqua-hideout-b2f", 24, 19, "left")
      await game.wait.forReady()
      expect(sees(await game.state.read(), hideoutMatt)).toBe(false)

      await game.player.warp("lilycove-pokemon-center", 10, 8, "right")
      await game.wait.forReady()
      expect(await listen(game)).toContain("recently")
    },
  )

  it(
    "starts the invasion on the first visit once both are true, in either order, and only once",
    { timeout: 420_000 },
    async () => {
      // Groudon first: the Gym win stays quiet inside, and walking out arms the invasion.
      await arrangeAt(game, "mossdeep-gym", 23, 8, "up", { hoennGroudonAwake: true })
      await battleThroughTateAndLiza(game)
      await expect(game.story.var("hoennMossdeepCityState")).resolves.toBe(0)
      await leaveGym(game)
      await expect(game.story.var("hoennMossdeepCityState")).resolves.toBe(1)
      await expect(game.story.var("hoennMossdeepSpaceCenterState")).resolves.toBe(1)
      await expect(game.story.flag("hoennHideAquaHideoutGrunts")).resolves.toBe(false)

      // Magma stands by the Space Center and walks in as the player comes near.
      await game.player.warp("mossdeep-city", 38, 25, "right")
      await game.wait.forReady()
      expect(sees(await game.state.read(), mossdeepMaxie)).toBe(true)
      await stepOpening(game, "right")
      await game.player.move("right")
      await playSceneUntil(
        game,
        async () => (await game.story.var("hoennMossdeepCityState")) === 2,
        "Team Magma did not enter the Space Center",
      )

      // Entering again does not start it over.
      await game.player.warp("mossdeep-city", 38, 25, "right")
      await game.wait.forReady()
      await expect(game.story.var("hoennMossdeepCityState")).resolves.toBe(2)
      expect(sees(await game.state.read(), mossdeepMaxie)).toBe(false)

      // The badge won long ago: the first visit after Magma Hideout arms it.
      await arrangeAt(game, "mossdeep-gym", 7, 34, "down", {
        hoennMindBadge: true,
        hoennGroudonAwake: true,
      })
      await stepOutOfGym(game)
      await expect(game.story.var("hoennMossdeepCityState")).resolves.toBe(1)
    },
  )

  it("keeps the Seafloor Cavern guard until Aqua has escaped", { timeout: 180_000 }, async () => {
    // Steven's Dive alone: the guard holds the only tile to the door.
    await arrangeAt(game, "seafloor-cavern-entrance", 10, 3, "up", { hoennDiveAuthorized: true })
    expect(sees(await game.state.read(), seafloorGrunt)).toBe(true)
    expect(await listen(game)).toContain("LILYCOVE")

    // Both branches done: he is gone and the door works.
    await arrangeAt(game, "seafloor-cavern-entrance", 10, 3, "up", {
      hoennDiveAuthorized: true,
      hoennAquaEscaped: true,
    })
    expect(sees(await game.state.read(), seafloorGrunt)).toBe(false)
    await stepOpening(game, "up")
    await game.player.move("up")
    await game.wait.forMap("seafloor-cavern-room-1")

    // Room 9's scene starts only then.
    await arrangeAt(game, "seafloor-cavern-room-9", 17, 43, "up", { hoennDiveAuthorized: true })
    await stepOpening(game, "up")
    await game.wait.frames(60)
    await expectFree(game)
    await expect(game.story.var("hoennSeafloorCavernState")).resolves.toBe(0)
    await arrangeAt(game, "seafloor-cavern-room-9", 17, 43, "up", {
      hoennDiveAuthorized: true,
      hoennAquaEscaped: true,
    })
    await game.player.move("up")
    await game.wait.until((state) => state.dialogueOpen, "Archie's challenge")
    expect(await game.dialogue.text()).toContain("Hold it")
  })
})
