import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"

const finishInteraction = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen) return
    await game.wait.frames(state.dialogueOpen ? 30 : 12)
    await game.controls.press("a")
  }
  throw new Error("Magnet Train attendant did not release control")
}

const advanceUntilMap = async (game: GameSession, destination: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 80; attempt++) {
    const state = await game.state.read()
    if (state.ready && state.map.name === destination) return
    await game.wait.frames(30)
    await game.controls.press("a")
  }
  throw new Error(`Magnet Train did not reach ${destination}`)
}

const moveTo = async (
  game: GameSession,
  direction: "up" | "down" | "left" | "right",
  x: number,
  y: number,
): Promise<void> => {
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(16)
    const state = await game.state.read()
    if (state.ready && state.player.x === x && state.player.y === y) return
  }
  throw new Error(`Player did not move ${direction} to ${x}:${y}`)
}

const stations = [
  {
    map: "saffron-train-station" as const,
    arrangeMap: "saffron-city" as const,
    position: { x: 17, y: 15 },
    entersFromExterior: true,
    destination: "goldenrod-train-station" as const,
    startsRestoration: true,
  },
  {
    map: "goldenrod-train-station" as const,
    arrangeMap: "goldenrod-train-station" as const,
    position: { x: 16, y: 21 },
    destination: "saffron-train-station" as const,
    startsRestoration: false,
    entersFromExterior: false,
  },
]

describe.sequential("HNS Magnet Train credential matrix", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  for (const station of stations) {
    for (const returnedPart of [false, true]) {
      for (const hasPass of [false, true]) {
        it(`${station.map} requires returned part=${returnedPart} and Pass=${hasPass}`, async () => {
          await game.arrange({
            checkpoint: "new-bark-after-intro",
            player: {
              facing: "up",
              position: { map: station.arrangeMap, ...station.position },
            },
            story: {
              flags: {
                returnedMachinePart: returnedPart,
                magnetTrainRestorationStarted: false,
              },
            },
            bag: { items: hasPass ? { pass: 1 } : undefined },
            determinism: { textSpeed: "instant" },
          })

          if (station.entersFromExterior) {
            await game.player.move("up")
            await game.wait.until(
              (state) => state.ready && state.map.name === station.map,
              "Saffron Train Station entrance",
            )
            await moveTo(game, "up", 137, 25)
            await moveTo(game, "right", 138, 25)
            await moveTo(game, "right", 139, 25)
            for (let y = 24; y >= 21; y--) await moveTo(game, "up", 139, y)
            await moveTo(game, "left", 138, 21)
            await moveTo(game, "left", 137, 21)
            await game.player.move("up")
            await expect(game.state.read()).resolves.toMatchObject({
              player: { x: 137, y: 21, facing: "up" },
            })
          }

          await game.player.interact()
          if (returnedPart && hasPass) {
            await advanceUntilMap(game, station.destination)
          } else {
            await finishInteraction(game)
            await expect(game.state.read()).resolves.toMatchObject({
              map: { name: station.map },
            })
            await expect(game.story.flag("magnetTrainRestorationStarted")).resolves.toBe(
              station.startsRestoration && !returnedPart,
            )
          }
        })
      }
    }
  }
})

describe.sequential("HNS Magnet Train restoration", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("crosses the removed Power Plant trigger without changing the Rocket campaign", async () => {
    for (const flags of [
      { magnetTrainRestorationStarted: false, returnedMachinePart: false },
      { magnetTrainRestorationStarted: true, returnedMachinePart: false },
      { magnetTrainRestorationStarted: true, returnedMachinePart: true },
    ]) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "power-plant-entrance", x: 6, y: 16 },
        },
        story: {
          vars: { kantoRocketStoryState: 1 },
          flags,
        },
        determinism: { textSpeed: "instant" },
      })

      await moveTo(game, "up", 6, 15)
      await moveTo(game, "up", 6, 14)

      await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(1)
      await expect(game.state.read()).resolves.toMatchObject({
        player: { x: 6, y: 14 },
        ready: true,
        scriptActive: false,
        dialogueOpen: false,
      })
    }
  })

  it("exposes the Machine Part only after the Saffron restoration errand starts", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "power-plant-back-room", x: 11, y: 6 },
      },
      story: {
        vars: { kantoRocketStoryState: 4 },
        flags: {
          magnetTrainRestorationStarted: false,
          returnedMachinePart: false,
          hiddenMachinePart: true,
        },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(true)
    await expect(game.inventory.contains("machinePart")).resolves.toBe(false)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "power-plant-back-room", x: 11, y: 6 },
      },
      story: {
        vars: { kantoRocketStoryState: 4 },
        flags: {
          magnetTrainRestorationStarted: true,
          returnedMachinePart: false,
          hiddenMachinePart: true,
        },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(false)
    await expect(game.inventory.contains("machinePart")).resolves.toBe(false)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)
  })

  it("keeps the Machine Part available after a full Key Items failure and grants it on retry", async () => {
    const arrangePickup = async (full: boolean): Promise<void> => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "left",
          position: { map: "cerulean-gym", x: full ? 6 : 1, y: 13 },
        },
        story: {
          vars: { kantoRocketStoryState: 2 },
          flags: {
            magnetTrainRestorationStarted: true,
            returnedMachinePart: false,
            hiddenMachinePart: false,
          },
        },
        bag: { fullPockets: full ? ["keyItems"] : undefined },
        determinism: { textSpeed: "instant" },
      })
    }

    await arrangePickup(true)
    const dialogueBeforeFailure = (await game.state.read()).dialogue.sequence
    await game.player.interact()
    await finishInteraction(game)
    expect((await game.state.read()).dialogue.sequence).toBeGreaterThan(dialogueBeforeFailure)
    await expect(game.inventory.contains("machinePart")).resolves.toBe(false)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(false)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(2)

    await arrangePickup(false)
    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("machinePart")).resolves.toBe(true)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(true)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(2)
  })

  it("commits the repair only after the TM reward and keeps the manager terminal", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "power-plant-back-room", x: 11, y: 6 },
      },
      story: {
        vars: { kantoRocketStoryState: 2, fanClubClefairy: 0 },
        flags: {
          magnetTrainRestorationStarted: true,
          returnedMachinePart: false,
          hiddenMachinePart: true,
        },
      },
      bag: {
        items: { machinePart: 1 },
        fullPockets: ["tmHm"],
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("machinePart")).resolves.toBe(true)
    await expect(game.inventory.contains("tmThunder")).resolves.toBe(false)
    await expect(game.story.flag("returnedMachinePart")).resolves.toBe(false)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(true)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(2)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(0)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "power-plant-back-room", x: 11, y: 6 },
      },
      story: {
        vars: { kantoRocketStoryState: 2, fanClubClefairy: 0 },
        flags: {
          magnetTrainRestorationStarted: true,
          returnedMachinePart: false,
          hiddenMachinePart: true,
        },
      },
      bag: { items: { machinePart: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("machinePart")).resolves.toBe(false)
    await expect(game.inventory.contains("tmThunder")).resolves.toBe(true)
    await expect(game.story.flag("returnedMachinePart")).resolves.toBe(true)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(true)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(2)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(0)

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("machinePart")).resolves.toBe(false)
    await expect(game.inventory.contains("tmThunder")).resolves.toBe(true)
    await expect(game.story.flag("returnedMachinePart")).resolves.toBe(true)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(true)
  })

  it("does not duplicate an already-owned TM when committing the repair", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "power-plant-back-room", x: 11, y: 6 },
      },
      story: {
        flags: {
          magnetTrainRestorationStarted: true,
          returnedMachinePart: false,
          hiddenMachinePart: true,
        },
      },
      bag: { items: { machinePart: 1, tmThunder: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.state.read()).resolves.toMatchObject({
      bag: { items: { machinePart: 0, tmThunder: 1 } },
    })
    await expect(game.story.flag("returnedMachinePart")).resolves.toBe(true)
  })

  it("keeps the Lost Item pending after a full Key Items failure and grants it on retry", async () => {
    const arrangeFanClub = async (full: boolean): Promise<void> => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "left",
          position: { map: "vermilion-fan-club", x: 5, y: 5 },
        },
        story: {
          vars: { fanClubClefairy: 1 },
          flags: { hideFanClubClefairyDoll: false },
        },
        bag: { fullPockets: full ? ["keyItems"] : undefined },
        determinism: { textSpeed: "instant" },
      })
    }

    await arrangeFanClub(true)
    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("lostItem")).resolves.toBe(false)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(1)
    await expect(game.story.flag("hideFanClubClefairyDoll")).resolves.toBe(false)

    await arrangeFanClub(false)
    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("lostItem")).resolves.toBe(true)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(2)
    await expect(game.story.flag("hideFanClubClefairyDoll")).resolves.toBe(true)

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("lostItem")).resolves.toBe(true)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(2)
  })

  it("atomically replaces the Lost Item with the Pass and keeps Copycat terminal", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "copycats-house-2f", x: 3, y: 5 },
      },
      story: {
        vars: { fanClubClefairy: 2 },
        flags: {
          returnedMachinePart: true,
          hideCopycatClefairyDoll: true,
        },
      },
      bag: {
        items: { lostItem: 1 },
        fullPockets: ["keyItems"],
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("lostItem")).resolves.toBe(false)
    await expect(game.inventory.contains("pass")).resolves.toBe(true)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(3)
    await expect(game.story.flag("hideCopycatClefairyDoll")).resolves.toBe(false)

    await game.player.interact()
    await finishInteraction(game)
    await expect(game.inventory.contains("lostItem")).resolves.toBe(false)
    await expect(game.inventory.contains("pass")).resolves.toBe(true)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(3)
    await expect(game.story.flag("hideCopycatClefairyDoll")).resolves.toBe(false)
  })
})
