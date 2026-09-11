import { afterEach, beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type ArrangeGame, type GameMap, type Species } from "../harness/game-session"

const circuitState = async (
  game: GameSession,
  badges: NonNullable<ArrangeGame["circuit"]>["badges"],
  clears: NonNullable<ArrangeGame["circuit"]>["clears"] = {},
) => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    party: [{ species: "lapras", level: 100 }],
    circuit: { badges, clears },
  })
  return game.state.read()
}

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.scriptActive && !state.dialogueOpen && !state.battle.active) return
    if (state.battle.active) throw new Error(`${description} unexpectedly entered battle`)
    if (state.dialogueOpen || state.scriptActive) {
      await game.wait.frames(20)
      await game.controls.press("a")
    } else await game.wait.frames(10)
  }
  throw new Error(`${description} did not release control`)
}

const finishVictoryScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready && !state.dialogueOpen) return
    if (state.battle.ui === "action-menu") {
      if (state.battle.cursor === 1 || state.battle.cursor === 3) await game.controls.press("left")
      if (state.battle.cursor === 2 || state.battle.cursor === 3) await game.controls.press("up")
      await game.controls.press("a")
    } else if (
      state.dialogueOpen ||
      state.scriptActive ||
      state.battle.ui === "text" ||
      state.battle.ui === "other"
    ) {
      await game.wait.frames(20)
      await game.controls.press("a")
    } else await game.wait.frames(10)
  }
  throw new Error(
    `${description} did not return to the overworld: ${JSON.stringify(await game.state.read())}`,
  )
}

const finishPendingArrange = async (
  game: GameSession,
  arrange: Promise<void>,
  description: string,
): Promise<void> => {
  let complete = false
  let failure: unknown
  void arrange.then(
    () => {
      complete = true
    },
    (error: unknown) => {
      failure = error
    },
  )
  for (let attempt = 0; attempt < 360 && !complete && !failure; attempt++) {
    const state = await game.state.read()
    if (state.dialogueOpen || state.scriptActive) await game.controls.press("a")
    else await game.wait.frames(10)
  }
  if (failure) throw failure
  if (!complete) throw new Error(`${description} did not finish arranging the map`)
  await arrange
}

const startTrainerBattle = async (
  game: GameSession,
  description: string,
  interact = true,
): Promise<void> => {
  if (interact) await game.player.interact()
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(10)
  }
  throw new Error(`${description} did not start: ${JSON.stringify(await game.state.read())}`)
}

const leagueBaseline = (rating: number): number => {
  const anchors = [
    [0, 15],
    [4, 16],
    [8, 18],
    [16, 23],
    [30, 30],
    [40, 42],
    [55, 60],
    [65, 80],
    [80, 100],
  ] as const
  for (let index = 1; index < anchors.length; index++) {
    const [r1, l1] = anchors[index]!
    const [r0, l0] = anchors[index - 1]!
    if (rating <= r1)
      return l0 + Math.floor(((rating - r0) * (l1 - l0) * 2 + r1 - r0) / (2 * (r1 - r0)))
  }
  return 100
}

const walkTo = async (game: GameSession, x: number, y: number): Promise<void> => {
  const map = (await game.state.read()).map.name
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.map.name !== map) throw new Error(`Left ${map} while walking to ${x},${y}`)
    if (state.player.x === x && state.player.y === y) {
      await game.wait.frames(16)
      return
    }
    await game.player.move(
      state.player.x < x
        ? "right"
        : state.player.x > x
          ? "left"
          : state.player.y < y
            ? "down"
            : "up",
    )
  }
  throw new Error(`Could not walk to ${x},${y}: ${JSON.stringify(await game.state.read())}`)
}

const walkNorthToMap = async (game: GameSession, map: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 200; attempt++) {
    if ((await game.state.read()).map.name === map) {
      await game.wait.forMap(map)
      await finishFieldScript(game, `enter ${map}`)
      return
    }
    await game.player.move("up")
  }
  throw new Error(`Did not reach ${map}: ${JSON.stringify(await game.state.read())}`)
}

const admitIndigo = async (game: GameSession, region: "kanto" | "johto"): Promise<number> => {
  if ((await game.state.read()).map.name === "indigo-plateau") {
    await walkNorthToMap(game, "indigo-league-lobby")
    await walkTo(game, 19, 12)
    await walkTo(game, 31, 12)
  }
  await walkTo(game, 31, 4)
  await walkTo(game, 32, 4)
  const rating = (await game.state.read()).circuit.trainerRating
  await walkNorthToMap(game, "league-will")
  await expect(game.state.read()).resolves.toMatchObject({
    circuit: { trainerRating: rating, run: { active: true, region, ratingAtEntry: rating } },
  })
  return rating
}

const indigoRooms = [
  "league-will",
  "league-koga",
  "league-bruno",
  "league-karen",
  "league-lance",
] as const
const indigoLeads: Record<"kanto" | "johto", readonly Species[]> = {
  kanto: ["girafarig", "ariados", "hitmonchan", "umbreon", "gyarados"],
  johto: ["gardevoir", "tentacruel", "steelix", "umbreon", "salamence"],
}
const indigoLeadOffsets = { kanto: [-6, -5, -3, -2, -1], johto: [-6, -4, -3, -2, 0] } as const

const finishIndigoRun = async (
  game: GameSession,
  region: "kanto" | "johto",
  rating: number,
): Promise<void> => {
  for (const [index, map] of indigoRooms.entries()) {
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: map },
      circuit: { run: { active: true, region, ratingAtEntry: rating } },
    })
    await walkTo(game, 6, index === 4 ? 9 : 7)
    await startTrainerBattle(game, `${region} ${map}`)
    await expect(game.state.read()).resolves.toMatchObject({
      battle: {
        enemy: {
          species: indigoLeads[region][index],
          level: Math.min(
            100,
            Math.max(1, leagueBaseline(rating) + indigoLeadOffsets[region][index]!),
          ),
        },
      },
      circuit: {
        trainerRating: rating,
        clears: { [region]: false },
        run: { active: true, region, ratingAtEntry: rating },
      },
    })
    await game.battle.win()
    if (index === 4) break
    await finishVictoryScript(game, `${region} ${map} victory`)
    await expect(game.story.var("leagueState")).resolves.toBe(index + 2)
    if (index === 0 || index === 2) {
      await game.saveAndReload()
      await expect(game.story.var("leagueState")).resolves.toBe(index + 2)
      await expect(game.state.read()).resolves.toMatchObject({
        circuit: {
          clears: { [region]: false },
          run: { active: true, region, ratingAtEntry: rating },
        },
      })
    }
    await walkTo(game, 5, 7)
    await walkTo(game, 5, 3)
    await walkTo(game, 6, 3)
    await walkNorthToMap(game, indigoRooms[index + 1]!)
  }
  for (let attempt = 0; attempt < 900; attempt++) {
    const state = await game.state.read()
    if (state.ready && state.map.name === "indigo-plateau" && state.circuit.clears[region]) {
      expect(state.circuit.run.active).toBe(false)
      await expect(game.story.var("leagueState")).resolves.toBe(1)
      await game.saveAndReload()
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "indigo-plateau" },
        circuit: {
          clears: { [region]: true },
          trainerRating: state.circuit.trainerRating,
          run: { active: false },
        },
      })
      return
    }
    await game.wait.frames(30)
    await game.controls.press("a")
  }
  throw new Error(
    `${region} Hall of Fame did not save and return: ${JSON.stringify(await game.state.read())}`,
  )
}

describe.sequential("Wayfarer League Circuit", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
  })

  afterEach(async () => {
    await game.close()
  })

  it("keeps badge collection independent and enforces the ordered 8/16/24 itinerary", async () => {
    await expect(circuitState(game, { johto: 3, hoenn: 4 })).resolves.toMatchObject({
      circuit: {
        badges: { total: 7 },
        leagues: { kanto: "locked", johto: "locked", hoenn: "locked" },
      },
    })
    await expect(circuitState(game, { johto: 4, hoenn: 4 })).resolves.toMatchObject({
      circuit: {
        badges: { kanto: 0, johto: 4, hoenn: 4, total: 8 },
        leagues: { kanto: "available", johto: "locked", hoenn: "locked" },
        trainerRating: 40,
      },
    })
    await expect(circuitState(game, { johto: 8, hoenn: 8 })).resolves.toMatchObject({
      circuit: {
        badges: { total: 16 },
        leagues: { kanto: "available", johto: "locked", hoenn: "locked" },
      },
    })
    await expect(circuitState(game, { kanto: 8, johto: 8, hoenn: 8 })).resolves.toMatchObject({
      circuit: {
        badges: { total: 24 },
        clears: { kanto: false, johto: false, hoenn: false },
        leagues: { kanto: "available", johto: "locked", hoenn: "locked" },
        trainerRating: 56,
      },
    })

    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: {
        badges: { total: 24 },
        clears: { kanto: false, johto: false, hoenn: false },
        trainerRating: 56,
      },
    })

    await game.controls.press("start")
    await game.wait.until((state) => state.ui.mode === "pause-menu", "open pause menu")
    for (let index = 0; index < 8; index++) await game.controls.press("up")
    await game.controls.press("down")
    await game.controls.press("down")
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.ui.mode === "trainer-card" && state.ui.trainerCard === "front",
      "open Trainer Card",
    )
    await game.controls.press("select")
    await game.wait.until(
      (state) => state.ui.mode === "trainer-card" && state.ui.trainerCard === "circuit",
      "open circuit view",
    )
    await expect(game.state.read()).resolves.toMatchObject({
      ui: { mode: "trainer-card", trainerCard: "circuit" },
      circuit: {
        badges: { total: 24 },
        leagues: { kanto: "available", johto: "locked", hoenn: "locked" },
      },
    })
    await game.controls.press("b")
    await game.wait.until(
      (state) => state.ui.mode === "trainer-card" && state.ui.trainerCard === "front",
      "return to Trainer Card",
    )
    // A short input pulse can be missed after observing the restored front
    // page. Retry only while that page still owns input; never send B
    // during closing or after the pause menu has opened.
    for (let attempt = 0; attempt < 8; attempt++) {
      const state = await game.state.read()
      if (state.ui.mode !== "trainer-card" || state.ui.trainerCard !== "front") break
      await game.controls.press("b")
    }
    await game.wait.until((state) => state.ui.mode === "pause-menu", "close Trainer Card")
    await game.wait.frames(60)
    await game.controls.press("b")
    await game.wait.until(
      (state) => state.ui.mode === "overworld" && state.ready,
      "close pause menu",
    )

    await expect(
      circuitState(game, { kanto: 8, johto: 8, hoenn: 8 }, { kanto: true }),
    ).resolves.toMatchObject({
      circuit: {
        leagues: { kanto: "cleared", johto: "available", hoenn: "locked" },
        trainerRating: 71,
      },
    })
    await expect(
      circuitState(game, { kanto: 8, johto: 8, hoenn: 8 }, { kanto: true, johto: true }),
    ).resolves.toMatchObject({
      circuit: {
        leagues: { kanto: "cleared", johto: "cleared", hoenn: "available" },
        trainerRating: 76,
      },
    })
    await expect(
      circuitState(
        game,
        { kanto: 8, johto: 8, hoenn: 8 },
        { kanto: true, johto: true, hoenn: true },
      ),
    ).resolves.toMatchObject({
      circuit: {
        leagues: { kanto: "cleared", johto: "cleared", hoenn: "cleared" },
        trainerRating: 80,
      },
    })
  })

  it("recovers mixed-order Rocket, Blue, Wattson, Whitney, and Clair states", async () => {
    await finishPendingArrange(
      game,
      game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "mahogany-town", x: 30, y: 13 } },
        story: {
          vars: {
            numBadges: 12,
            mahoganyTownState: 15,
            goldenrodCityState: 1,
            triggerElmRocketCall: 0,
          },
        },
        circuit: { badges: { kanto: 5, johto: 7 } },
      }),
      "late Rocket takeover recovery",
    )
    await expect(game.story.var("triggerElmRocketCall")).resolves.toBe(2)
    await expect(game.story.var("goldenrodCityState")).resolves.toBe(6)
    await expect(game.story.var("mahoganyTownState")).resolves.toBe(16)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "mahogany-town", x: 30, y: 13 } },
      story: {
        vars: {
          numBadges: 20,
          mahoganyTownState: 17,
          goldenrodCityState: 8,
          triggerElmRocketCall: 2,
        },
      },
      circuit: { badges: { kanto: 8, johto: 7, hoenn: 5 } },
    })
    await expect(game.story.var("triggerElmRocketCall")).resolves.toBe(2)
    await expect(game.story.var("goldenrodCityState")).resolves.toBe(8)
    await expect(game.story.var("mahoganyTownState")).resolves.toBe(17)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "cinnabar-island", x: 40, y: 22 } },
      story: { vars: { numBadges: 4 }, flags: { hideCinnabarBlue: false, hideViridianBlue: true } },
      circuit: { badges: { hoenn: 4 } },
    })
    await game.player.interact()
    await finishFieldScript(game, "Blue invitation")
    await expect(game.story.flag("hideCinnabarBlue")).resolves.toBe(true)
    await expect(game.story.flag("hideViridianBlue")).resolves.toBe(false)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "goldenrod-gym", x: 15, y: 9 } },
      story: {
        vars: { numBadges: 14, goldenrodCityState: 9 },
        flags: { defeatedWhitneyTrainer: true, defeatedGoldenrodGym: false },
      },
      circuit: { badges: { kanto: 8, johto: 2, hoenn: 4 } },
    })
    await game.player.interact()
    await finishFieldScript(game, "Whitney deferred award")
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: { badges: { johto: 3, total: 15 } },
    })
    await expect(game.story.var("goldenrodCityState")).resolves.toBe(9)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "dragons-den-shrine", x: 6, y: 15 } },
      story: {
        vars: { blackthornCityState: 2 },
        flags: { hideDragonsDenShrineClair: false, hideDragonsDenCavernClair: true },
      },
      circuit: { badges: { kanto: 8, johto: 8, hoenn: 8 } },
    })
    await expect(game.story.var("blackthornCityState")).resolves.toBe(3)
    await expect(game.story.flag("hideDragonsDenShrineClair")).resolves.toBe(true)
    await expect(game.story.flag("hideDragonsDenCavernClair")).resolves.toBe(false)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "mauville-gym", x: 5, y: 3 } },
      story: {
        flags: {
          defeatedPetalburgGym: true,
          hideMauvilleGymWattson: false,
          hideMauvilleCityWattson: true,
        },
      },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { hoenn: 2 } },
    })
    await game.saveAndReload()
    await expect(game.story.flag("hideMauvilleGymWattson")).resolves.toBe(false)
    await startTrainerBattle(game, "Wattson battle after Norman")
    await game.battle.win()
    await finishVictoryScript(game, "Wattson badge and relocation")
    await expect(game.state.read()).resolves.toMatchObject({ circuit: { badges: { hoenn: 3 } } })
    await expect(game.story.flag("hideMauvilleGymWattson")).resolves.toBe(true)
    await expect(game.story.flag("hideMauvilleCityWattson")).resolves.toBe(false)
  })

  for (const route of [
    { name: "earliest Kanto", badges: { johto: 4, hoenn: 4 }, clears: {}, region: "kanto" },
    {
      name: "intermediate Kanto",
      badges: { kanto: 4, johto: 4, hoenn: 4 },
      clears: {},
      region: "kanto",
    },
    {
      name: "earliest Johto",
      badges: { kanto: 8, johto: 8 },
      clears: { kanto: true },
      region: "johto",
    },
    {
      name: "intermediate Johto",
      badges: { kanto: 8, johto: 8, hoenn: 4 },
      clears: { kanto: true },
      region: "johto",
    },
    {
      name: "all-badges consecutive Indigo",
      badges: { kanto: 8, johto: 8, hoenn: 8 },
      clears: {},
      region: "kanto",
    },
  ] as const) {
    it(`preserves admission levels through every room and save/load: ${route.name}`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "indigo-league-lobby", x: 31, y: 4 } },
        party: [{ species: "lapras", level: 100, moves: ["surf"] }],
        circuit: { badges: route.badges, clears: route.clears },
        story: { vars: { ssAquaState: 8 } },
      })
      const rating = await admitIndigo(game, route.region)
      await game.saveAndReload()
      await expect(game.story.var("leagueState")).resolves.toBe(1)
      await finishIndigoRun(game, route.region, rating)
      await expect(game.story.var("ssAquaState")).resolves.toBe(8)
      if (route.region === "kanto") {
        await expect(game.story.flag("isKantoChampion")).resolves.toBe(true)
        await expect(game.story.flag("isChampion")).resolves.toBe(false)
      }
      if (route.name === "all-badges consecutive Indigo") {
        const johtoRating = await admitIndigo(game, "johto")
        expect(johtoRating).toBeGreaterThan(rating)
        await finishIndigoRun(game, "johto", johtoRating)
        await expect(game.story.var("ssAquaState")).resolves.toBe(8)
        await expect(game.story.flag("isKantoChampion")).resolves.toBe(true)
        await expect(game.story.flag("isChampion")).resolves.toBe(true)
        await expect(game.state.read()).resolves.toMatchObject({
          circuit: {
            clears: { kanto: true, johto: true, hoenn: false },
            leagues: { hoenn: "available" },
            run: { active: false },
          },
        })
      }
    }, 600_000)
  }

  it("keeps Hoenn admission levels through five battles, halls, and completion reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "hoenn-league-lobby", x: 9, y: 3 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { kanto: 8, johto: 8, hoenn: 8 }, clears: { kanto: true, johto: true } },
    })
    const rating = (await game.state.read()).circuit.trainerRating
    await game.player.interact()
    await finishFieldScript(game, "Hoenn guards grant admission")
    await walkNorthToMap(game, "hoenn-league-hall5")
    await walkNorthToMap(game, "league-sidney")
    // The room's frame script closes the entrance after the map first becomes ready.
    await game.wait.until(
      async () => (await game.story.var("hoennEliteFourState")) === 1,
      "Sidney entrance door closed",
    )
    await finishFieldScript(game, "Sidney entrance scene")
    await game.saveAndReload()
    const rooms = [
      "league-sidney",
      "league-phoebe",
      "league-glacia",
      "league-drake",
      "league-wallace",
    ] as const
    const leads = ["mightyena", "dusclops", "sealeo", "shelgon", "wailord"] as const
    const halls = [
      "hoenn-league-hall1",
      "hoenn-league-hall2",
      "hoenn-league-hall3",
      "hoenn-league-hall4",
    ] as const
    const offsets = [-5, -4, -3, -2, 0]
    for (const [index, room] of rooms.entries()) {
      if (index < 4) {
        await walkTo(game, 6, 6)
        await startTrainerBattle(game, room)
      } else {
        await startTrainerBattle(game, "automatic Wallace battle", false)
      }
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: room },
        battle: {
          enemy: {
            species: leads[index],
            level: Math.min(100, leagueBaseline(rating) + offsets[index]!),
          },
        },
        circuit: {
          trainerRating: rating,
          clears: { hoenn: false },
          run: { active: true, region: "hoenn", ratingAtEntry: rating },
        },
      })
      await game.battle.win()
      if (index === 4) break
      await finishVictoryScript(game, `${room} victory`)
      await game.saveAndReload()
      await walkTo(game, 5, 6)
      await walkTo(game, 5, 3)
      await walkTo(game, 6, 3)
      await walkNorthToMap(game, halls[index]!)
      if (index < 3) await walkNorthToMap(game, rooms[index + 1]!)
      else {
        // Wallace's room starts its battle script immediately on entry.
        for (let attempt = 0; attempt < 400; attempt++) {
          if ((await game.state.read()).map.name === "league-wallace") break
          await game.player.move("up")
        }
      }
    }
    for (let attempt = 0; attempt < 900; attempt++) {
      const state = await game.state.read()
      if (state.ready && state.map.name === "ever-grande-city" && state.circuit.clears.hoenn) break
      await game.wait.frames(30)
      await game.controls.press("a")
    }
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "ever-grande-city" },
      circuit: { clears: { kanto: true, johto: true, hoenn: true }, run: { active: false } },
    })
    const completedRating = (await game.state.read()).circuit.trainerRating
    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: { trainerRating: completedRating, clears: { hoenn: true }, run: { active: false } },
    })
  }, 600_000)

  it("clears a lost attempt before save/load and admits a fresh retry", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "indigo-league-lobby", x: 31, y: 4 } },
      party: [{ species: "pidgey", level: 1, moves: ["tackle"] }],
      circuit: { badges: { johto: 8 } },
    })
    const rating = await admitIndigo(game, "kanto")
    await walkTo(game, 6, 7)
    await startTrainerBattle(game, "losing Kanto attempt")
    for (let attempt = 0; attempt < 900; attempt++) {
      const state = await game.state.read()
      if (state.ready && state.map.name === "indigo-plateau") break
      await game.wait.frames(30)
      await game.controls.press("a")
    }
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "indigo-plateau" },
      circuit: { trainerRating: rating, clears: { kanto: false }, run: { active: false } },
    })
    await game.saveAndReload()
    await expect(game.story.var("leagueState")).resolves.toBe(1)
    expect(await admitIndigo(game, "kanto")).toBe(rating)
    await expect(game.story.var("leagueState")).resolves.toBe(1)
  }, 240_000)

  it("recovers inconsistent saved room progression without awarding a clear", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "indigo-league-lobby", x: 31, y: 4 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { johto: 8 } },
    })
    const rating = await admitIndigo(game, "kanto")
    await game.story.setVar("leagueState", 6)
    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "indigo-league-lobby" },
      circuit: { trainerRating: rating, clears: { kanto: false }, run: { active: false } },
    })
    await expect(game.story.var("leagueState")).resolves.toBe(1)
  })

  it("rejects a Hall of Fame injection without an admitted run", async () => {
    await finishPendingArrange(
      game,
      game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "hall-of-fame", x: 5, y: 12 } },
        story: { vars: { leagueState: 6 } },
        party: [{ species: "lapras", level: 100 }],
        circuit: { badges: { kanto: 8, johto: 8, hoenn: 8 } },
      }),
      "invalid Hall of Fame recovery",
    )
    await finishFieldScript(game, "invalid Hall of Fame recovery")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "indigo-league-lobby" },
      circuit: {
        trainerRating: 56,
        clears: { kanto: false, johto: false, hoenn: false },
        run: { active: false },
      },
    })
  })

  it("denies entry without eight badges and creates no run", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "indigo-league-lobby", x: 32, y: 4 } },
      circuit: { badges: { johto: 7 } },
    })
    const rating = (await game.state.read()).circuit.trainerRating
    for (let attempt = 0; attempt < 100; attempt++) {
      const state = await game.state.read()
      if (state.dialogueOpen) break
      await game.player.move("up")
    }
    await finishFieldScript(game, "denied Indigo admission")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "indigo-league-lobby" },
      circuit: { trainerRating: rating, clears: { kanto: false }, run: { active: false } },
    })
  })
})
