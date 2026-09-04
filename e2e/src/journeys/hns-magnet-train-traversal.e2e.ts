import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready && !state.dialogueOpen) return
    if (state.dialogueOpen || state.battle.ui === "text" || state.scriptActive) {
      await game.wait.frames(30)
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not release control: ${JSON.stringify(await game.state.read())}`,
  )
}

const waitForFieldScriptStart = (game: GameSession, description: string): Promise<void> =>
  game.wait.until(
    (state) => state.scriptActive || state.dialogueOpen || state.battle.active,
    `${description} start`,
  )

const startScriptedBattle = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.battle.active) return
    if (state.dialogueOpen || state.scriptActive) {
      await game.wait.frames(30)
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(`${description} did not start: ${JSON.stringify(await game.state.read())}`)
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

const enterSaffronStation = async (game: GameSession): Promise<void> => {
  await game.player.move("up")
  await game.wait.until(
    (state) => state.ready && state.map.name === "saffron-train-station",
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

describe.sequential("HNS Magnet Train restoration", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("runs the restored Rocket story from the manager through Misty's return", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "power-plant-back-room", x: 11, y: 6 },
      },
      story: {
        vars: { kantoRocketStoryState: 0, ceruleanCityState: 0, fanClubClefairy: 0 },
        flags: {
          returnedMachinePart: false,
          hiddenMachinePart: true,
          hideCeruleanGymRocket: true,
          hideCeruleanCapeRocket: true,
          hideRoute25Misty: true,
          hidePowerPlantEngineer: false,
          kantoRadioGot: false,
        },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishFieldScript(game, "Power Plant manager start")
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(1)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "power-plant-entrance", x: 6, y: 16 },
      },
      story: {
        vars: { kantoRocketStoryState: 1 },
        flags: { hideCeruleanGymRocket: true },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.move("up")
    await waitForFieldScriptStart(game, "Power Plant officer report")
    await finishFieldScript(game, "Power Plant officer report")
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(2)
    await expect(game.story.flag("hideCeruleanGymRocket")).resolves.toBe(false)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "cerulean-city", x: 34, y: 32 } },
      story: {
        vars: { kantoRocketStoryState: 2 },
        flags: { hideCeruleanGymRocket: false, hideCeruleanCapeRocket: true },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.move("up")
    await waitForFieldScriptStart(game, "Cerulean Gym Rocket escape")
    await finishFieldScript(game, "Cerulean Gym Rocket escape")
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(3)
    await expect(game.story.flag("hideCeruleanGymRocket")).resolves.toBe(true)
    await expect(game.story.flag("hideCeruleanCapeRocket")).resolves.toBe(false)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "route-24", x: 17, y: 12 } },
      story: {
        vars: { kantoRocketStoryState: 3 },
        flags: { hideCeruleanCapeRocket: false },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.move("up")
    await waitForFieldScriptStart(game, "Route 24 Rocket staging")
    await finishFieldScript(game, "Route 24 Rocket staging")
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "left", position: { map: "route-24", x: 17, y: 8 } },
      story: {
        vars: { kantoRocketStoryState: 4 },
        flags: { hideCeruleanCapeRocket: false, hiddenMachinePart: true },
      },
      party: [{ species: "cyndaquil", level: 100, moves: ["tackle"] }],
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await startScriptedBattle(game, "Route 24 Rocket battle")
    await game.battle.win()
    await finishFieldScript(game, "Route 24 Rocket victory")
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(5)
    await expect(game.story.flag("hideCeruleanCapeRocket")).resolves.toBe(true)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(false)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "left", position: { map: "cerulean-gym", x: 6, y: 13 } },
      story: { vars: { kantoRocketStoryState: 5 }, flags: { hiddenMachinePart: false } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishFieldScript(game, "Machine Part pickup")
    await expect(game.inventory.contains("machinePart")).resolves.toBe(true)
    await expect(game.story.flag("hiddenMachinePart")).resolves.toBe(true)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(6)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "power-plant-back-room", x: 11, y: 6 },
      },
      story: {
        vars: { kantoRocketStoryState: 6, ceruleanCityState: 0, fanClubClefairy: 0 },
        flags: {
          returnedMachinePart: false,
          hiddenMachinePart: true,
          hideRoute25Misty: true,
          hidePowerPlantEngineer: false,
          kantoRadioGot: false,
        },
      },
      bag: { items: { machinePart: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await finishFieldScript(game, "Machine Part turn-in")
    await expect(game.inventory.contains("machinePart")).resolves.toBe(false)
    await expect(game.inventory.contains("tmThunder")).resolves.toBe(true)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(7)
    await expect(game.story.var("ceruleanCityState")).resolves.toBe(2)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(0)
    await expect(game.story.flag("returnedMachinePart")).resolves.toBe(true)
    await expect(game.story.flag("hideRoute25Misty")).resolves.toBe(false)
    await expect(game.story.flag("hidePowerPlantEngineer")).resolves.toBe(true)
    await expect(game.story.flag("kantoRadioGot")).resolves.toBe(false)
    await expect(game.story.var("numBadges")).resolves.toBe(0)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "route-25", x: 98, y: 22 } },
      story: {
        vars: { ceruleanCityState: 2, numBadges: 0 },
        flags: {
          hideRoute25Misty: false,
          hideCeruleanGymTrainers: true,
          hideCeruleanGymPokemon: false,
        },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.move("up")
    await waitForFieldScriptStart(game, "Route 25 Misty return")
    await finishFieldScript(game, "Route 25 Misty return")
    await expect(game.story.var("ceruleanCityState")).resolves.toBe(3)
    await expect(game.story.var("numBadges")).resolves.toBe(0)
    await expect(game.story.flag("hideRoute25Misty")).resolves.toBe(true)
    await expect(game.story.flag("hideCeruleanGymTrainers")).resolves.toBe(false)
    await expect(game.story.flag("hideCeruleanGymPokemon")).resolves.toBe(true)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "saffron-city", x: 17, y: 15 } },
      story: { flags: { returnedMachinePart: true } },
      determinism: { textSpeed: "instant" },
    })

    await enterSaffronStation(game)
    await game.player.interact()
    await finishFieldScript(game, "Pass-less Magnet Train attendant")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "saffron-train-station" },
    })

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "saffron-city", x: 17, y: 15 } },
      story: { flags: { returnedMachinePart: true } },
      bag: { items: { pass: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await enterSaffronStation(game)
    await game.player.interact()
    await advanceUntilMap(game, "goldenrod-train-station")
  })
})
