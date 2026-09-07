import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type ArrangeGame } from "../harness/game-session"

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
    if (state.ready && !state.dialogueOpen && !state.battle.active) return
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

const startTrainerBattle = async (game: GameSession, description: string): Promise<void> => {
  await game.player.interact()
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(10)
  }
  throw new Error(`${description} did not start: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer League Circuit", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
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
    await game.wait.until((state) => state.ui.trainerCard === "front", "open Trainer Card")
    await game.controls.press("select")
    await game.wait.until((state) => state.ui.trainerCard === "circuit", "open circuit view")
    await expect(game.state.read()).resolves.toMatchObject({
      ui: { mode: "trainer-card", trainerCard: "circuit" },
      circuit: {
        badges: { total: 24 },
        leagues: { kanto: "available", johto: "locked", hoenn: "locked" },
      },
    })
    await game.controls.press("b")
    await game.wait.until((state) => state.ui.trainerCard === "front", "return to Trainer Card")
    await game.controls.press("b")
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
})
