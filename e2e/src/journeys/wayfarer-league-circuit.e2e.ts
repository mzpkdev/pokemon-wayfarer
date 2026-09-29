import * as fs from "node:fs"
import * as path from "node:path"
import { afterEach, beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type ArrangeGame, type GameMap } from "../harness/game-session"

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
      state.controlsLocked ||
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

const declineDojoBattle = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.wait.until((state) => state.controlsLocked || state.dialogueOpen, "Blue Dojo prompt")
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.battle.active) throw new Error("Declining Blue unexpectedly started a battle")
    if (state.ready) return
    await game.controls.press("b")
    await game.wait.frames(10)
  }
  throw new Error(`Blue Dojo decline did not finish: ${JSON.stringify(await game.state.read())}`)
}

const finishDojoWin = async (game: GameSession): Promise<void> => {
  const before = await game.inventory.battlePoints()
  await game.battle.win()
  await finishVictoryScript(game, "Blue Dojo victory and BP")
  await expect(game.inventory.battlePoints()).resolves.toBe(before + 10)
}

const enterDojo = async (game: GameSession): Promise<void> => {
  await game.player.warp("fighting-dojo-vip", 18, 21, "up")
  await game.wait.forMap("fighting-dojo-vip")
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
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || !state.ready)
      await game.controls.press("a")
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
    // Inside a battle, advance text only; a press while the action menu
    // opens would pick FIGHT before the menu is ever observed.
    if (state.battle.active) {
      if (state.battle.ui === "text") await game.controls.press("a")
      else await game.wait.frames(4)
    } else if (state.dialogueOpen || state.scriptActive || state.controlsLocked || !state.ready)
      await game.controls.press("a")
    else await game.wait.frames(10)
  }
  throw new Error(`${description} did not start: ${JSON.stringify(await game.state.read())}`)
}

const finishGiovanniFinale = async (game: GameSession): Promise<void> => {
  for (const flag of [
    "celadonHideoutGiovanniTrainerDefeated",
    "celadonHideoutScopeReceived",
    "silphGiovanniDefeated",
    "silphLiberated",
  ] as const)
    await game.story.setFlag(flag, true)
  await game.player.warp("viridian-gym", 2, 3, "up")
  await game.wait.forMap("viridian-gym")
  await startTrainerBattle(game, "Viridian Giovanni finale")
  await expect(game.state.read()).resolves.toMatchObject({
    battle: { enemy: { species: "marowak" } },
  })
  await game.battle.win()
  await finishVictoryScript(game, "Viridian Giovanni finale")
  await expect(game.story.flag("badge16")).resolves.toBe(true)
  await expect(game.story.flag("viridianGiovanniDeparted")).resolves.toBe(true)
  await expect(game.inventory.count("tmEarthquake")).resolves.toBe(1)
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

const walkNorthToMap = async (
  game: GameSession,
  map: GameMap,
  expectAutomaticBattle = false,
): Promise<void> => {
  for (let attempt = 0; attempt < 200; attempt++) {
    if ((await game.state.read()).map.name === map) {
      await game.wait.forMap(map)
      // The room's on-frame entrance script starts one tick after the map first reports ready.
      await game.wait.frames(2)
      if (expectAutomaticBattle) {
        await startTrainerBattle(game, `enter ${map}`, false)
        return
      }
      await finishFieldScript(game, `enter ${map}`)
      return
    }
    await game.player.move("up")
  }
  throw new Error(`Did not reach ${map}: ${JSON.stringify(await game.state.read())}`)
}

const rooms = {
  indigo: ["indigo-lorelei", "indigo-bruno", "indigo-agatha", "indigo-lance", "indigo-blue"],
  masters: ["league-will", "league-koga", "league-bruno", "league-karen", "league-lance"],
  hoenn: ["league-sidney", "league-phoebe", "league-glacia", "league-drake", "league-wallace"],
} as const
const admitIndigo = async (game: GameSession, replay = false): Promise<number> => {
  const start = await game.state.read()
  if (start.map.name === "indigo-league-lobby" && (start.player.x !== 32 || start.player.y !== 4)) {
    await walkTo(game, 19, 12)
    await walkTo(game, 31, 12)
    await walkTo(game, 31, 4)
    await walkTo(game, 32, 4)
  }
  const rating = (await game.state.read()).circuit.trainerRating
  await walkTo(game, 32, 3)
  await finishFieldScript(game, "Indigo event acceptance")
  await expect(game.state.read()).resolves.toMatchObject({
    circuit: { event: { acceptedLeague: "indigo", worldProgress: rating } },
  })
  await walkNorthToMap(game, "indigo-lorelei")
  await expect(game.state.read()).resolves.toMatchObject({
    circuit: { run: { active: true, stage: "indigo", replay, ratingAtEntry: rating } },
  })
  if (process.env.SKYEMU_CAPTURE_DIR && !replay) {
    await fs.promises.mkdir(process.env.SKYEMU_CAPTURE_DIR, { recursive: true })
    const eventId = (await game.state.read()).circuit.event.id
    await fs.promises.writeFile(
      path.join(process.env.SKYEMU_CAPTURE_DIR, `indigo-selected-${rating}-${eventId}.png`),
      await game.screenshot(),
    )
  }
  return rating
}

const admitMasters = async (game: GameSession, replay = false): Promise<number> => {
  const rating = (await game.state.read()).circuit.trainerRating
  await walkTo(game, 3, 3)
  await game.player.interact()
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.map.name === "league-will") {
      await finishFieldScript(game, "Masters Will entrance")
      await expect(game.state.read()).resolves.toMatchObject({
        circuit: { run: { active: true, stage: "masters", replay, ratingAtEntry: rating } },
      })
      return rating
    }
    if (state.battle.active) throw new Error("Masters admission entered a battle")
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || !state.ready)
      await game.controls.press("a")
    else await game.wait.frames(10)
  }
  throw new Error(
    `Masters admission did not reach Will: ${JSON.stringify(await game.state.read())}`,
  )
}

const finishRoomChain = async (
  game: GameSession,
  stage: "indigo" | "masters",
  rating: number,
  replay = false,
  champion: "win" | "lose" = "win",
): Promise<void> => {
  const accepted = (await game.state.read()).circuit.event
  expect(accepted.acceptedLeague).toBe(stage)
  expect(accepted.lineup.map((member) => member.characterId).every((id) => id > 0)).toBe(true)
  expect(new Set(accepted.lineup.map((member) => member.characterId)).size).toBe(5)
  for (const [index, map] of rooms[stage].entries()) {
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: map },
      circuit: { run: { active: true, stage, replay, ratingAtEntry: rating } },
    })
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: { event: { id: accepted.id, worldProgress: rating, lineup: accepted.lineup } },
    })
    const automaticBattle = (await game.state.read()).battle.active
    if (!automaticBattle) {
      await walkTo(
        game,
        6,
        stage === "indigo" ? (index === 4 ? 9 : index === 3 ? 9 : 6) : index === 4 ? 9 : 7,
      )
      if (stage === "indigo") await game.player.move("up")
      await startTrainerBattle(game, `${stage} ${map}`)
    }
    await expect(game.state.read()).resolves.toMatchObject({
      battle: {
        enemy: {
          species: accepted.lineup[index]!.leadSpecies,
        },
      },
      circuit: { run: { active: true, stage, replay, ratingAtEntry: rating } },
    })
    // Starting the Champion battle is not a committed Indigo victory.
    if (stage === "indigo" && index === 4 && !replay)
      await expect(game.story.flag("hideDojoBlue")).resolves.toBe(true)
    if (index === 4 && champion === "lose") {
      await game.battle.lose()
      for (let attempt = 0; attempt < 900; attempt++) {
        const state = await game.state.read()
        if (state.ready && state.map.name === "indigo-league-lobby") return
        await game.wait.frames(30)
        await game.controls.press("a")
      }
      throw new Error(
        `Lost ${stage} Champion did not return: ${JSON.stringify(await game.state.read())}`,
      )
    }
    await game.battle.win()
    if (index === 4) break
    await finishVictoryScript(game, `${stage} ${map} victory`)
    const expectedLeagueState = index + (stage === "indigo" ? 1 : 2)
    await expect(game.story.var("leagueState")).resolves.toBe(expectedLeagueState)
    if (index === 0 || index === 2) {
      await game.saveAndReload()
      await expect(game.story.var("leagueState")).resolves.toBe(expectedLeagueState)
      await expect(game.state.read()).resolves.toMatchObject({
        circuit: { run: { active: true, stage, replay, ratingAtEntry: rating } },
      })
    }
    if (stage === "indigo") {
      await walkTo(game, 5, index === 3 ? 9 : 6)
      await walkTo(game, 5, index === 3 ? 6 : 3)
      await walkTo(game, 6, index === 3 ? 6 : 3)
    } else {
      await walkTo(game, 5, 7)
      await walkTo(game, 5, 3)
      await walkTo(game, 6, 3)
    }
    await walkNorthToMap(game, rooms[stage][index + 1]!, stage === "indigo" && index + 1 === 4)
  }
  const expectedReturn =
    stage === "indigo" ? "indigo-league-lobby" : "sevii-seven-island-house-room1"
  for (let attempt = 0; attempt < 900; attempt++) {
    const state = await game.state.read()
    if (state.ready && state.map.name === expectedReturn && state.circuit.clears[stage]) {
      expect(state.circuit.run.active).toBe(false)
      await expect(game.story.var("leagueState")).resolves.toBe(1)
      await game.saveAndReload()
      const expectedReload = stage === "indigo" ? "indigo-plateau" : expectedReturn
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: expectedReload },
        circuit: { clears: { [stage]: true }, run: { active: false } },
      })
      return
    }
    await game.wait.frames(30)
    await game.controls.press("a")
  }
  throw new Error(`${stage} ceremony did not return: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer League Circuit", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
  })

  afterEach(async () => {
    await game.close()
  })

  it("keeps Dojo Blue hidden until the committed Indigo clear", async () => {
    const dojoFixture = {
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "fighting-dojo-vip", x: 18, y: 21 }, facing: "up" },
    } as const
    const earthBadgeFlags = {
      badge16: true,
      defeatedViridianGym: true,
      viridianGiovanniDeparted: true,
    } as const
    // Badges, Giovanni, and a regional title projection are not a committed
    // Indigo victory.
    await game.arrange({
      ...dojoFixture,
      story: { flags: { ...earthBadgeFlags, isKantoChampion: true, hideDojoBlue: false } },
      circuit: { badges: { kanto: 8 } },
    })
    await expect(game.story.flag("hideDojoBlue")).resolves.toBe(true)
    await game.saveAndReload()
    await expect(game.story.flag("hideDojoBlue")).resolves.toBe(true)

    await game.arrange({
      ...dojoFixture,
      story: { flags: { ...earthBadgeFlags, hideDojoBlue: true } },
      circuit: { badges: { kanto: 8 }, clears: { indigo: true } },
    })
    await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
    await game.saveAndReload()
    await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
  })

  it("keeps Blue hidden after Giovanni, then unlocks him on the first Indigo commit", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "viridian-gym", x: 2, y: 3 }, facing: "up" },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { johto: 4, hoenn: 4 } },
      determinism: { textSpeed: "instant" },
    })
    await finishGiovanniFinale(game)
    await enterDojo(game)
    await expect(game.story.flag("hideDojoBlue")).resolves.toBe(true)
    await game.saveAndReload()
    await expect(game.story.flag("hideDojoBlue")).resolves.toBe(true)

    await game.player.warp("indigo-league-lobby", 32, 4, "up")
    await game.wait.forMap("indigo-league-lobby")
    const rating = await admitIndigo(game)
    await finishRoomChain(game, "indigo", rating)
    await expect(game.story.flag("badge16")).resolves.toBe(true)
    await expect(game.inventory.battlePoints()).resolves.toBe(0)
    await enterDojo(game)
    await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
    await startTrainerBattle(game, "Blue Dojo after Giovanni then Indigo")
    await expect(game.state.read()).resolves.toMatchObject({
      battle: { enemy: { species: "scizor" } },
    })
    await finishDojoWin(game)
  }, 600_000)

  it("qualifies Indigo and Hoenn independently at TR 80, then unlocks Masters after both clears", async () => {
    await expect(circuitState(game, { johto: 3, hoenn: 4 })).resolves.toMatchObject({
      circuit: {
        badges: { total: 7 },
        trainerRating: 70,
        leagues: { indigo: "locked", masters: "locked", hoenn: "locked" },
      },
    })
    await expect(circuitState(game, { johto: 4, hoenn: 4 })).resolves.toMatchObject({
      circuit: {
        badges: { total: 8 },
        trainerRating: 80,
        leagues: { indigo: "available", masters: "locked", hoenn: "available" },
      },
    })
    await expect(circuitState(game, { hoenn: 8 })).resolves.toMatchObject({
      circuit: { leagues: { indigo: "locked", masters: "locked", hoenn: "available" } },
    })
    await expect(circuitState(game, { johto: 8 })).resolves.toMatchObject({
      circuit: { leagues: { indigo: "available", masters: "locked", hoenn: "locked" } },
    })
    await expect(circuitState(game, { kanto: 8, johto: 8, hoenn: 8 })).resolves.toMatchObject({
      circuit: {
        badges: { total: 24 },
        trainerRating: 160,
        leagues: { indigo: "available", masters: "locked", hoenn: "available" },
      },
    })
    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: {
        badges: { total: 24 },
        trainerRating: 160,
        leagues: { indigo: "available", masters: "locked", hoenn: "available" },
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
      circuit: { leagues: { indigo: "available", masters: "locked", hoenn: "available" } },
    })
    await game.controls.press("b")
    await game.wait.until(
      (state) => state.ui.mode === "trainer-card" && state.ui.trainerCard === "front",
      "return to Trainer Card",
    )
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
      circuitState(game, { johto: 4, hoenn: 4 }, { indigo: true }),
    ).resolves.toMatchObject({
      circuit: { leagues: { indigo: "cleared", masters: "locked", hoenn: "available" } },
    })
    await expect(
      circuitState(game, { johto: 4, hoenn: 4 }, { hoenn: true }),
    ).resolves.toMatchObject({
      circuit: { leagues: { indigo: "available", masters: "locked", hoenn: "cleared" } },
    })
    await expect(
      circuitState(game, { johto: 4, hoenn: 4 }, { indigo: true, hoenn: true }),
    ).resolves.toMatchObject({
      circuit: { leagues: { indigo: "cleared", masters: "available", hoenn: "cleared" } },
    })
    await expect(
      circuitState(game, { johto: 4, hoenn: 4 }, { indigo: true, masters: true, hoenn: true }),
    ).resolves.toMatchObject({
      circuit: { leagues: { indigo: "cleared", masters: "cleared", hoenn: "cleared" } },
    })
  })

  it("persists a 32-bit player rating and freezes it at League admission", async () => {
    const rating = 0x10001
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "indigo-league-lobby", x: 32, y: 4 } },
      party: [{ species: "lapras", level: 100 }],
      story: { vars: { trainerRating: rating } },
      circuit: { badges: { kanto: 8 } },
    })
    expect(await game.story.var("trainerRating")).toBe(rating)
    await expect(game.state.read()).resolves.toMatchObject({ circuit: { trainerRating: rating } })
    await admitIndigo(game)
    const accepted = (await game.state.read()).circuit.event
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: {
        event: { worldProgress: rating, acceptedLeague: "indigo" },
        run: { active: true, ratingAtEntry: rating },
      },
    })
    await game.story.setVar("trainerRating", rating + 123)
    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: {
        trainerRating: rating + 123,
        event: { id: accepted.id, worldProgress: rating, lineup: accepted.lineup },
        run: { active: true, ratingAtEntry: rating },
      },
    })
  })

  it("keeps Red's authored completion live after all three circuit clears", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "mt-silver-summit", x: 10, y: 7 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: {
        badges: { kanto: 8, johto: 8, hoenn: 8 },
        clears: { indigo: true, masters: true, hoenn: true },
      },
      story: {
        flags: { hideMtSilverRed: false, defeatedRed: false, endNuzlocke: false },
      },
    })
    await startTrainerBattle(game, "Red after circuit completion")
    await game.battle.win()
    for (let attempt = 0; attempt < 600; attempt++) {
      const state = await game.state.read()
      if ((await game.story.flag("defeatedRed")) && state.phase === "boot") break
      if (state.battle.ui === "action-menu") {
        if (state.battle.cursor === 1 || state.battle.cursor === 3)
          await game.controls.press("left")
        if (state.battle.cursor === 2 || state.battle.cursor === 3) await game.controls.press("up")
        await game.controls.press("a")
      } else if (
        state.dialogueOpen ||
        state.scriptActive ||
        state.battle.ui === "text" ||
        state.battle.ui === "other"
      )
        await game.controls.press("a")
      else await game.wait.frames(20)
    }
    if (!(await game.story.flag("defeatedRed")))
      throw new Error(
        `Red completion did not set its authored flag: ${JSON.stringify(await game.state.read())}`,
      )
    await expect(game.state.read()).resolves.toMatchObject({
      phase: "boot",
      circuit: {
        clears: { indigo: true, masters: true, hoenn: true },
        trainerRating: 160,
        run: { active: false },
      },
    })
  }, 600_000)

  it("recovers mixed-order Rocket, Wattson, Whitney, and Clair states", async () => {
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
    { name: "mixed eight badges", badges: { johto: 4, hoenn: 4 } },
    { name: "all badges before any clear", badges: { kanto: 8, johto: 8, hoenn: 8 } },
  ] as const) {
    it(`runs FRLG Indigo with one shared Champion commit: ${route.name}`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "indigo-league-lobby", x: 32, y: 4 } },
        party: [{ species: "lapras", level: 100, moves: ["surf"] }],
        circuit: { badges: route.badges },
        story: {
          vars: { ssAquaState: 8 },
        },
      })
      const rating = await admitIndigo(game)
      await game.saveAndReload()
      await expect(game.story.var("leagueState")).resolves.toBe(1)
      await finishRoomChain(game, "indigo", rating)
      await expect(game.story.var("ssAquaState")).resolves.toBe(8)
      await expect(game.story.flag("isKantoChampion")).resolves.toBe(true)
      await expect(game.story.flag("isChampion")).resolves.toBe(true)
      await expect(game.state.read()).resolves.toMatchObject({
        circuit: {
          clears: { indigo: true, masters: false, hoenn: false },
          regionalChampions: { kanto: true, johto: true, hoenn: false },
          trainerRating: rating,
          run: { active: false },
        },
      })
      await expect(game.inventory.battlePoints()).resolves.toBe(0)
      if (route.name === "mixed eight badges") {
        // This route has no Earth Badge or Giovanni victory. The committed
        // Indigo clear alone makes Blue available for a repeatable match.
        await expect(game.story.flag("badge16")).resolves.toBe(false)
        // Dojo entry projects the committed victory onto Blue's visibility.
        await enterDojo(game)
        await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
        await declineDojoBattle(game)
        await expect(game.inventory.battlePoints()).resolves.toBe(0)
        await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
        await startTrainerBattle(game, "Blue Dojo loss after Indigo")
        await game.battle.lose()
        await finishVictoryScript(game, "Blue Dojo loss")
        await expect(game.inventory.battlePoints()).resolves.toBe(0)
        await enterDojo(game)
        await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
        await startTrainerBattle(game, "Blue Dojo battle after Indigo")
        await expect(game.state.read()).resolves.toMatchObject({
          battle: { enemy: { species: "scizor" } },
        })
        await finishDojoWin(game)
        await game.saveAndReload()
        await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
        await expect(game.inventory.battlePoints()).resolves.toBe(10)
        await startTrainerBattle(game, "Blue Dojo repeat battle")
        await expect(game.state.read()).resolves.toMatchObject({
          battle: { enemy: { species: "scizor" } },
        })
        await finishDojoWin(game)
        await expect(game.inventory.battlePoints()).resolves.toBe(20)
        await finishGiovanniFinale(game)
        await enterDojo(game)
        await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
        await game.saveAndReload()
        await expect(game.story.flag("hideDojoBlue")).resolves.toBe(false)
        await expect(game.inventory.battlePoints()).resolves.toBe(20)
      }
    }, 600_000)
  }

  it("admits Masters after both regional clears and replays without another reward", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "sevii-seven-island-house-room1", x: 4, y: 5 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { kanto: 8, johto: 7 }, clears: { indigo: true } },
    })
    const lockedRating = (await game.state.read()).circuit.trainerRating
    await game.player.interact()
    await finishFieldScript(game, "Masters refusal before both regional clears")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "sevii-seven-island-house-room1" },
      circuit: { trainerRating: lockedRating, clears: { masters: false }, run: { active: false } },
    })

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "sevii-seven-island-house-room1", x: 4, y: 5 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { kanto: 8, johto: 8, hoenn: 1 }, clears: { indigo: true, hoenn: true } },
    })
    const rating = await admitMasters(game)
    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: { run: { active: true, stage: "masters", replay: false, ratingAtEntry: rating } },
    })
    await finishRoomChain(game, "masters", rating)
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: {
        clears: { indigo: true, masters: true, hoenn: true },
        regionalChampions: { kanto: true, johto: true, hoenn: true },
        trainerRating: rating,
      },
    })

    await walkTo(game, 4, 5)
    const replayRating = await admitMasters(game, true)
    expect(replayRating).toBe(rating)
    await finishRoomChain(game, "masters", replayRating, true)
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: { trainerRating: replayRating, clears: { masters: true }, run: { active: false } },
    })
  }, 600_000)

  it("keeps Hoenn admission levels through five battles, halls, and completion reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "hoenn-league-lobby", x: 9, y: 3 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { hoenn: 8 } },
    })
    const rating = (await game.state.read()).circuit.trainerRating
    await game.player.interact()
    await finishFieldScript(game, "Hoenn guards grant admission")
    await walkTo(game, 9, 2)
    await finishFieldScript(game, "Hoenn event acceptance")
    const accepted = (await game.state.read()).circuit.event
    expect(accepted.acceptedLeague).toBe("hoenn")
    expect(accepted.worldProgress).toBe(rating)
    expect(new Set(accepted.lineup.map((member) => member.characterId)).size).toBe(5)
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
    const halls = [
      "hoenn-league-hall1",
      "hoenn-league-hall2",
      "hoenn-league-hall3",
      "hoenn-league-hall4",
    ] as const
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
            species: accepted.lineup[index]!.leadSpecies,
            level: accepted.lineup[index]!.leadLevel,
          },
        },
        circuit: {
          trainerRating: rating,
          clears: { hoenn: false },
          event: { id: accepted.id, worldProgress: rating, lineup: accepted.lineup },
          run: { active: true, stage: "hoenn", replay: false, ratingAtEntry: rating },
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
      circuit: {
        clears: { indigo: false, masters: false, hoenn: true },
        run: { active: false },
        event: { id: 0, acceptedLeague: null },
      },
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
      player: { position: { map: "indigo-league-lobby", x: 32, y: 4 } },
      party: [{ species: "pidgey", level: 1, moves: ["tackle"] }],
      circuit: { badges: { johto: 8 } },
    })
    const rating = await admitIndigo(game)
    const firstEventId = (await game.state.read()).circuit.event.id
    await walkTo(game, 6, 6)
    await startTrainerBattle(game, "losing Indigo attempt")
    for (let attempt = 0; attempt < 900; attempt++) {
      const state = await game.state.read()
      if (state.ready && state.map.name === "indigo-league-lobby") break
      await game.wait.frames(30)
      await game.controls.press("a")
    }
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "indigo-league-lobby" },
      circuit: {
        trainerRating: rating,
        clears: { indigo: false },
        run: { active: false },
        event: { id: 0, acceptedLeague: null },
      },
    })
    await game.saveAndReload()
    await expect(game.story.var("leagueState")).resolves.toBe(1)
    expect(await admitIndigo(game)).toBe(rating)
    expect((await game.state.read()).circuit.event.id).toBeGreaterThan(firstEventId)
    await expect(game.story.var("leagueState")).resolves.toBe(1)
  }, 240_000)

  it("does not unlock Dojo Blue when the Indigo Champion battle starts or is lost", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "indigo-league-lobby", x: 32, y: 4 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { johto: 8 } },
    })
    const rating = await admitIndigo(game)
    await finishRoomChain(game, "indigo", rating, false, "lose")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "indigo-league-lobby" },
      circuit: {
        trainerRating: rating,
        clears: { indigo: false },
        run: { active: false },
        event: { id: 0, acceptedLeague: null },
      },
    })
    await game.saveAndReload()
    await enterDojo(game)
    await expect(game.story.flag("hideDojoBlue")).resolves.toBe(true)
  }, 600_000)

  it("recovers inconsistent saved room progression without awarding a clear", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "indigo-league-lobby", x: 32, y: 4 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      circuit: { badges: { johto: 8 } },
    })
    const rating = await admitIndigo(game)
    const accepted = (await game.state.read()).circuit.event
    await game.story.setVar("leagueState", 6)
    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "indigo-league-lobby" },
      circuit: {
        trainerRating: rating,
        clears: { indigo: false },
        run: { active: false },
        event: accepted,
      },
    })
    await expect(game.story.var("leagueState")).resolves.toBe(1)
  })

  it("rejects a Hall of Fame injection without an admitted run", async () => {
    await finishPendingArrange(
      game,
      game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "indigo-hall-of-fame", x: 5, y: 12 } },
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
        trainerRating: 160,
        clears: { indigo: false, masters: false, hoenn: false },
        run: { active: false },
      },
    })
  })

  it("denies entry below TR 80 and creates no event or run", async () => {
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
      circuit: {
        trainerRating: rating,
        clears: { indigo: false },
        run: { active: false },
        event: { id: 0, acceptedLeague: null },
      },
    })
  })

  it("keeps one accepted event while declining admission to another League", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "indigo-league-lobby", x: 32, y: 4 }, facing: "up" },
      party: [{ species: "lapras", level: 100 }],
      circuit: { badges: { johto: 4, hoenn: 4 }, clears: { indigo: true, hoenn: true } },
    })
    await walkTo(game, 32, 3)
    await finishFieldScript(game, "Indigo event acceptance")
    const accepted = (await game.state.read()).circuit.event
    expect(accepted.id).toBeGreaterThan(0)
    expect(accepted.acceptedLeague).toBe("indigo")
    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({ circuit: { event: accepted } })

    await game.player.warp("sevii-seven-island-house-room1", 4, 5, "up")
    await game.wait.forMap("sevii-seven-island-house-room1")
    await game.player.interact()
    await finishFieldScript(game, "Masters event elsewhere denial")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "sevii-seven-island-house-room1" },
      circuit: {
        leagues: { masters: "locked", hoenn: "cleared" },
        run: { active: false },
        event: accepted,
      },
    })

    await game.player.warp("indigo-league-lobby", 32, 4, "up")
    await game.wait.forMap("indigo-league-lobby")
    await walkTo(game, 32, 3)
    await finishFieldScript(game, "return to accepted Indigo event")
    await walkNorthToMap(game, "indigo-lorelei")
    await expect(game.state.read()).resolves.toMatchObject({
      circuit: { event: accepted, run: { active: true, stage: "indigo" } },
    })
  })
})
