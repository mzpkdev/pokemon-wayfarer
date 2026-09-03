import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction } from "../harness/game-session"
import { type GameMap, type StoryFlag } from "../harness/game-session/catalog"

const moveOneTile = async (game: GameSession, direction: Direction): Promise<void> => {
  const before = await game.state.read()
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(12)
    const after = await game.state.read()
    if (
      after.map.mapGroup !== before.map.mapGroup ||
      after.map.mapNum !== before.map.mapNum ||
      after.player.x !== before.player.x ||
      after.player.y !== before.player.y
    )
      return
  }
  throw new Error(`Could not move ${direction}: ${JSON.stringify(await game.state.read())}`)
}

const moveUntilMap = async (
  game: GameSession,
  direction: Direction,
  destination: GameMap,
): Promise<void> => {
  for (let attempt = 0; attempt < 4; attempt++) {
    if ((await game.state.read()).map.name === destination) return
    await game.player.move(direction)
    await game.wait.frames(90)
  }
  await game.wait.forMap(destination)
}

const movePath = async (game: GameSession, path: readonly Direction[]): Promise<void> => {
  for (const direction of path) await moveOneTile(game, direction)
}

const startPreparedSurf = async (
  game: GameSession,
  position: { map: GameMap; x: number; y: number },
  facing: Direction,
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing, position },
    party: [{ species: "lapras", moves: ["surf"] }],
    determinism: { textSpeed: "instant" },
  })

  await game.player.interact()
  await game.dialogue.waitForOpen()
  await expect(game.state.read()).resolves.toMatchObject({
    dialogue: { message: "want-to-use-surf" },
    fieldMove: { move: "surf", user: 0, userSpecies: "lapras", result: "found" },
  })
  await game.wait.until(
    (state) => state.dialogue.message === "want-to-use-surf" && !state.dialogueOpen,
    "prepared native Surf prompt",
  )
  await game.wait.frames(12)
  await game.controls.press("a")
  await game.wait.until(
    (state) => state.dialogue.message === "player-used-surf",
    "prepared native Surf confirmation",
  )
  await game.dialogue.waitForClosed()
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.until((state) => state.player.surfing, "start prepared native Surf", 3_600)
  await game.wait.forReady()
}

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 120; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen) return
    await game.wait.frames(state.dialogueOpen ? 30 : 12)
    await game.controls.press("a")
  }
  throw new Error(`${description} did not release control`)
}

const enterRoad = async (
  game: GameSession,
  gate: "celadon-route-16-gate" | "fuchsia-route-18-gate",
  bicycle: boolean,
): Promise<number> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "left", position: { map: gate, x: 7, y: 5 } },
    story: { flags: { cyclingRoad: false } },
    bag: { items: bicycle ? { bicycle: 1 } : undefined },
    determinism: { textSpeed: "instant" },
  })

  await game.player.move("left")
  await game.dialogue.waitForOpen()
  await game.controls.press("a")
  await finishFieldScript(game, "Cycling Road gate")
  const state = await game.state.read()
  expect(state.player).toMatchObject({ x: 5, y: 5 })
  await expect(game.story.flag("cyclingRoad")).resolves.toBe(true)
  return state.player.avatarFlags
}

const exitRoad = async (game: GameSession): Promise<void> => {
  // Entry leaves the mounted avatar facing west. The first east input turns
  // the bicycle around; the second crosses the gate trigger.
  await game.player.move("right")
  await game.wait.until(
    (state) => state.ready && state.player.x === 5 && state.player.facing === "right",
    "bicycle turned toward the gate",
  )
  await game.player.move("right")
  await game.wait.until((state) => state.ready && state.player.x === 7, "Cycling Road gate exit")
}

const crossMainlandSeam = async (
  game: GameSession,
  options: {
    start: { map: GameMap; x: number; y: number }
    entryDirection: "up" | "left" | "right"
    gateDirection: "up" | "left" | "right"
    destination: GameMap
  },
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: options.entryDirection, position: options.start },
    story: {
      flags: { returnedMachinePart: false, magnetTrainRestorationStarted: false },
      vars: { saffronCityState: 1, kantoRocketStoryState: 0 },
    },
  })

  const source = await game.state.read()
  for (let attempt = 0; attempt < 3; attempt++) {
    const state = await game.state.read()
    if (state.map.mapGroup !== source.map.mapGroup || state.map.mapNum !== source.map.mapNum) break
    await game.player.move(options.entryDirection)
    await game.wait.frames(30)
  }
  await game.wait.until(
    (state) => state.map.mapGroup !== source.map.mapGroup || state.map.mapNum !== source.map.mapNum,
    "enter Saffron gate",
  )

  for (let step = 0; step < 16; step++) {
    if ((await game.state.read()).map.name === options.destination) return
    await game.player.move(options.gateDirection)
    await game.wait.frames(30)
  }
  await game.wait.forMap(options.destination)
}

const arrangeMtMoonSilver = async (
  game: GameSession,
  party: Parameters<GameSession["arrange"]>[0]["party"],
  extraFlags: Partial<Record<StoryFlag, boolean>> = {},
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "up", position: { map: "mt-moon-cave", x: 9, y: 12 } },
    story: {
      vars: {
        pewterCityState: 1,
        kantoRocketStoryState: 4,
        fanClubClefairy: 2,
        ssAquaState: 8,
      },
      flags: {
        hideMtMoonSilver: false,
        returnedMachinePart: false,
        visitedKanto: true,
        ...extraFlags,
      },
    },
    party,
    determinism: { textSpeed: "instant" },
  })
}

const acceptSilverBattle = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  for (let attempt = 0; attempt < 40; attempt++) {
    if ((await game.state.read()).battle.active) return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error("Mt. Moon Silver battle did not start")
}

const finishSilverBattle = async (game: GameSession): Promise<void> => {
  let handledFaintedPartyCount = 0
  for (let attempt = 0; attempt < 4_000; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready) return
    const faintedPartyCount = state.party.filter((mon) => mon.fainted).length
    if (state.battle.ui === "other" && faintedPartyCount > handledFaintedPartyCount) {
      await game.controls.press("a")
      await game.wait.frames(60)
      await game.controls.press("b")
      await game.wait.frames(30)
      await game.controls.press("right")
      await game.wait.frames(30)
      await game.controls.press("a")
      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.frames(120)
      if ((await game.state.read()).battle.ui !== "other")
        handledFaintedPartyCount = faintedPartyCount
    } else if (state.battle.ui === "action-menu") {
      handledFaintedPartyCount = faintedPartyCount
      if (state.battle.cursor === 1 || state.battle.cursor === 3) await game.controls.press("left")
      if (state.battle.cursor === 2 || state.battle.cursor === 3) await game.controls.press("up")
      await game.controls.press("a")
    } else if (state.battle.ui === "bag" || state.battle.ui === "bag-context")
      await game.controls.press("b")
    else if (state.battle.ui === "other") await game.controls.press("a")
    else if (state.battle.ui === "text" || state.dialogueOpen) await game.controls.press("a")
    else await game.wait.frames(12)
  }
  const state = await game.state.read()
  throw new Error(
    `Mt. Moon Silver battle did not return to the overworld: active=${state.battle.active}, ui=${state.battle.ui}, cursor=${state.battle.cursor}, party=${state.party.map((mon) => `${mon.species}:${mon.fainted}`).join(",")}`,
  )
}

const finishSilverVictoryScript = async (game: GameSession): Promise<void> => {
  await game.wait.frames(60)
  for (let attempt = 0; attempt < 240; attempt++) {
    if ((await game.story.var("pewterCityState")) === 2) {
      await game.wait.forReady()
      return
    }
    await game.controls.press("a")
    await game.wait.frames(30)
  }
  throw new Error(
    `Mt. Moon Silver victory script did not commit: ${JSON.stringify(await game.state.read())}`,
  )
}

describe.sequential("HNS Kanto traversal", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  describe("Mt. Moon Silver", () => {
    it("crosses Silver's old trigger row without interaction or unrelated state", async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "right", position: { map: "mt-moon-cave", x: 8, y: 12 } },
        story: {
          vars: { pewterCityState: 0, kantoRocketStoryState: 4, ssAquaState: 8 },
          flags: { hideMtMoonSilver: false, returnedMachinePart: false },
        },
      })

      await game.wait.forReady()
      await game.player.move("right")

      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        dialogueOpen: false,
        battle: { active: false },
        player: { x: 9, y: 12 },
      })
      await expect(game.story.var("pewterCityState")).resolves.toBe(1)
      await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)
      await expect(game.story.var("ssAquaState")).resolves.toBe(8)
      await expect(game.story.flag("hideMtMoonSilver")).resolves.toBe(false)
      await expect(game.story.flag("returnedMachinePart")).resolves.toBe(false)
    })

    it("keeps Silver available and local state unchanged after declining", async () => {
      await arrangeMtMoonSilver(game, [{ species: "lapras", level: 100 }])

      await game.player.interact()
      await game.dialogue.waitForOpen()
      for (let attempt = 0; attempt < 20; attempt++) {
        if ((await game.state.read()).ready) break
        await game.controls.press("b")
        await game.wait.frames(12)
      }
      await game.wait.forReady()

      await expect(game.story.var("pewterCityState")).resolves.toBe(1)
      await expect(game.story.flag("hideMtMoonSilver")).resolves.toBe(false)
      await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)
      await expect(game.story.var("fanClubClefairy")).resolves.toBe(2)
      await expect(game.story.var("ssAquaState")).resolves.toBe(8)
    })

    it("keeps Silver available after a trainer-battle loss and clears transport state", async () => {
      await arrangeMtMoonSilver(game, [{ species: "rattata", moves: ["tackle"], level: 1 }], {
        cyclingRoad: true,
      })
      await acceptSilverBattle(game)
      await finishSilverBattle(game)

      await expect(game.story.var("pewterCityState")).resolves.toBe(1)
      await expect(game.story.flag("hideMtMoonSilver")).resolves.toBe(false)
      await expect(game.story.flag("cyclingRoad")).resolves.toBe(false)
      await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)
      await expect(game.story.var("fanClubClefairy")).resolves.toBe(2)
      await expect(game.story.var("ssAquaState")).resolves.toBe(8)
    })

    it("commits only Silver's local Mt. Moon state after victory", async () => {
      await arrangeMtMoonSilver(game, [{ species: "lapras", level: 100 }], {
        hideIndigoPlateauSilver: true,
      })
      await acceptSilverBattle(game)
      await game.battle.win()
      await finishSilverVictoryScript(game)

      await expect(game.story.var("pewterCityState")).resolves.toBe(2)
      await expect(game.story.flag("hideMtMoonSilver")).resolves.toBe(true)
      await expect(game.story.flag("hideIndigoPlateauSilver")).resolves.toBe(false)
      await expect(game.story.flag("returnedMachinePart")).resolves.toBe(false)
      await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)
      await expect(game.story.var("fanClubClefairy")).resolves.toBe(2)
      await expect(game.story.var("ssAquaState")).resolves.toBe(8)
    })
  })

  describe("Cycling Road loan", () => {
    it("loans from Celadon, preserves the live loan across reload, and cleans up at the gate", async () => {
      const mountedFlags = await enterRoad(game, "celadon-route-16-gate", false)
      await expect(game.inventory.contains("bicycle")).resolves.toBe(false)

      await game.saveAndReload()

      const reloaded = await game.state.read()
      expect(reloaded).toMatchObject({
        map: { name: "celadon-route-16-gate" },
        player: { x: 5, y: 5 },
      })
      expect(reloaded.player.avatarFlags & mountedFlags).toBe(mountedFlags)
      await expect(game.story.flag("cyclingRoad")).resolves.toBe(true)
      await expect(game.inventory.contains("bicycle")).resolves.toBe(false)

      await exitRoad(game)

      const state = await game.state.read()
      expect(state.player).toMatchObject({ x: 7, y: 5 })
      expect(state.player.avatarFlags & mountedFlags).toBe(0)
      await expect(game.story.flag("cyclingRoad")).resolves.toBe(false)
      await expect(game.inventory.contains("bicycle")).resolves.toBe(false)
    })

    it("offers the same loan and cleanup from the Fuchsia gate", async () => {
      const mountedFlags = await enterRoad(game, "fuchsia-route-18-gate", false)

      await exitRoad(game)

      const state = await game.state.read()
      expect(state.player).toMatchObject({ x: 7, y: 5 })
      expect(state.player.avatarFlags).not.toBe(mountedFlags)
      await expect(game.story.flag("cyclingRoad")).resolves.toBe(false)
      await expect(game.inventory.contains("bicycle")).resolves.toBe(false)
    })

    it("preserves an owned Bicycle and riding state when leaving the road", async () => {
      const mountedFlags = await enterRoad(game, "celadon-route-16-gate", true)

      await exitRoad(game)

      const state = await game.state.read()
      expect(state.player).toMatchObject({ x: 7, y: 5, avatarFlags: mountedFlags })
      await expect(game.story.flag("cyclingRoad")).resolves.toBe(false)
      await expect(game.inventory.contains("bicycle")).resolves.toBe(true)
    })
  })

  describe("Kanto mainland routes", () => {
    const expectRestorationUnset = async (): Promise<void> => {
      await expect(game.story.flag("returnedMachinePart")).resolves.toBe(false)
      await expect(game.story.flag("magnetTrainRestorationStarted")).resolves.toBe(false)
      await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(0)
      await expect(game.story.var("saffronCityState")).resolves.toBe(1)
    }

    it("crosses Route 6 into Saffron with the Machine Part errand unset", async () => {
      await crossMainlandSeam(game, {
        start: { map: "route-6", x: 40, y: 6 },
        entryDirection: "up",
        gateDirection: "up",
        destination: "saffron-city",
      })
      await expectRestorationUnset()
    })

    it("continues through Route 5 without restoring the Power Plant", async () => {
      await crossMainlandSeam(game, {
        start: { map: "saffron-city", x: 28, y: 6 },
        entryDirection: "up",
        gateDirection: "up",
        destination: "route-5",
      })
      await expectRestorationUnset()
    })

    const crossSaffronGate = async (
      gate: "saffron-route-7-gate" | "saffron-route-8-gate",
      startX: number,
      direction: "left" | "right",
      destination: "saffron-city" | "route-7" | "route-8",
    ): Promise<void> => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: direction, position: { map: gate, x: startX, y: 5 } },
        story: {
          flags: { returnedMachinePart: false, magnetTrainRestorationStarted: false },
          vars: { saffronCityState: 1, kantoRocketStoryState: 0 },
        },
      })

      await moveUntilMap(game, direction, destination)
      await expectRestorationUnset()
    }

    it("crosses the Route 7 gate into Saffron without restoration state", async () => {
      await crossSaffronGate("saffron-route-7-gate", 10, "right", "saffron-city")
    })

    it("crosses the Route 7 gate out of Saffron without restoration state", async () => {
      await crossSaffronGate("saffron-route-7-gate", 2, "left", "route-7")
    })

    it("crosses the Route 8 gate into Saffron without restoration state", async () => {
      await crossSaffronGate("saffron-route-8-gate", 2, "left", "saffron-city")
    })

    it("crosses the Route 8 gate out of Saffron without restoration state", async () => {
      await crossSaffronGate("saffron-route-8-gate", 10, "right", "route-8")
    })
  })

  describe("prepared native Surf routes", () => {
    it("crosses the Pallet end of Route 21 in both directions without the HM item", async () => {
      await startPreparedSurf(game, { map: "pallet-town", x: 7, y: 16 }, "down")
      await movePath(game, ["down", "down"])
      await moveUntilMap(game, "down", "route-21")
      await moveUntilMap(game, "up", "pallet-town")

      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "pallet-town" },
        player: { surfing: true },
      })
    })

    it("crosses the Cinnabar end of Route 21 in both directions without the HM item", async () => {
      await startPreparedSurf(game, { map: "route-21", x: 6, y: 89 }, "left")
      await movePath(game, [
        "down",
        "down",
        "left",
        "down",
        "down",
        "down",
        "right",
        "down",
        "down",
        "down",
        "down",
        "down",
      ])
      await moveUntilMap(game, "down", "cinnabar-island")
      await moveUntilMap(game, "up", "route-21")

      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "route-21" },
        player: { surfing: true },
      })
    })

    it("crosses the Olivine end of Route 40 in both directions without the HM item", async () => {
      await startPreparedSurf(game, { map: "olivine-city", x: 0, y: 55 }, "down")
      await moveUntilMap(game, "left", "route-40")
      await moveUntilMap(game, "right", "olivine-city")

      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "olivine-city" },
        player: { surfing: true },
      })
    })

    it("crosses the Route 40 and Route 41 seam in both directions without the HM item", async () => {
      await startPreparedSurf(game, { map: "route-41", x: 43, y: 26 }, "left")
      await movePath(game, [
        "left",
        "left",
        "left",
        "up",
        "left",
        "up",
        "up",
        "left",
        "up",
        "up",
        "up",
        "up",
        "up",
        "up",
        "up",
        "up",
        "up",
        "up",
      ])
      await moveUntilMap(game, "up", "route-40")
      await moveUntilMap(game, "down", "route-41")

      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "route-41" },
        player: { surfing: true },
      })
    })

    it("crosses the Cianwood end of Route 41 in both directions without the HM item", async () => {
      await startPreparedSurf(game, { map: "cianwood-city", x: 34, y: 45 }, "right")
      await moveOneTile(game, "right")
      await moveUntilMap(game, "right", "route-41")
      await moveUntilMap(game, "left", "cianwood-city")

      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "cianwood-city" },
        player: { surfing: true },
      })
    })
  })
})
