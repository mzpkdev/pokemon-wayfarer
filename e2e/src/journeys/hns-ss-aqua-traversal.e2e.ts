import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"

const finishScript = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen) return
    await game.wait.frames(state.dialogueOpen ? 30 : 12)
    await game.controls.press("a")
  }
  throw new Error(`S.S. Aqua reward script did not release control`)
}

const winTrainerBattleWithFirstMove = async (game: GameSession): Promise<void> => {
  let battleStarted = false
  for (let attempt = 0; attempt < 900; attempt++) {
    const state = await game.state.read()
    battleStarted ||= state.battle.active
    if (!state.battle.active && state.ready) {
      if (!battleStarted) throw new Error("Stanly battle did not start")
      await finishScript(game)
      return
    }
    if (state.battle.ui === "action-menu") {
      await game.controls.press("a")
      await game.wait.frames(30)
      await game.controls.press("a")
      await game.wait.frames(120)
    } else if (state.controlsLocked || state.dialogueOpen || state.battle.ui === "text") {
      await game.controls.press("a")
      await game.wait.frames(30)
    } else {
      await game.wait.frames(30)
      if ((await game.state.read()).battle.active) await game.controls.press("a")
    }
  }
  throw new Error(`Stanly battle did not finish: ${JSON.stringify(await game.state.read())}`)
}

const finishFirstLeagueClearFlags = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    if ((await game.story.var("leagueState")) === 1) return
    await game.wait.frames(30)
    await game.controls.press("a")
  }
  throw new Error(
    `first League clear flags were not committed: ${JSON.stringify(await game.state.read())}`,
  )
}

const advanceUntilMap = async (game: GameSession, map: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 60; attempt++) {
    const state = await game.state.read()
    if (state.map.name === map) {
      await game.wait.forMap(map)
      return
    }
    await game.wait.frames(30)
    if ((await game.state.read()).map.name === map) {
      await game.wait.forMap(map)
      return
    }
    await game.controls.press("a")
  }
  throw new Error(`S.S. Aqua did not reach ${map}: ${JSON.stringify(await game.state.read())}`)
}

const arrangeAtGrandfather = async (
  game: GameSession,
  state: 4 | 5 | 6,
  options: {
    items?: Partial<{ metalCoat: number; ssTicket: number }>
    fullPockets?: ("items" | "keyItems")[]
  } = {},
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: {
      facing: "up",
      position: { map: "ss-aqua-room-sse", x: 1, y: 5 },
    },
    story: { vars: { ssAquaState: state } },
    bag: options,
    determinism: { textSpeed: "instant" },
  })
}

describe.sequential("HNS S.S. Aqua voyage rewards", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("boards the maiden voyage at state 0 without an S.S. Ticket", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "olivine-port-inside", x: 8, y: 16 },
      },
      story: { vars: { ssAquaState: 0 } },
      determinism: { textSpeed: "instant" },
    })

    await expect(game.inventory.contains("ssTicket")).resolves.toBe(false)
    await game.player.interact()
    await advanceUntilMap(game, "ss-aqua-1f")

    await expect(game.story.var("ssAquaState")).resolves.toBe(1)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(false)
  })

  it("resumes the maiden voyage without a Ticket in every state from 1 through 7", async () => {
    for (let voyageState = 1; voyageState <= 7; voyageState++) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "down",
          position: { map: "olivine-port-inside", x: 8, y: 16 },
        },
        story: { vars: { ssAquaState: voyageState } },
        determinism: { textSpeed: "instant" },
      })

      await expect(game.inventory.contains("ssTicket")).resolves.toBe(false)
      await game.player.interact()
      await advanceUntilMap(game, "ss-aqua-1f")
      if (voyageState === 6) {
        await game.wait.frames(60)
        await finishScript(game)
      }

      await expect(game.state.read()).resolves.toMatchObject({ map: { name: "ss-aqua-1f" } })
      await expect(game.inventory.contains("ssTicket")).resolves.toBe(false)
      await expect(game.story.var("ssAquaState")).resolves.toBe(voyageState === 6 ? 7 : voyageState)
    }
  })

  it("leaves the voyage state unchanged after Stanly's optional battle", async () => {
    const stanlyGame = await GameSession.launch()
    try {
      await stanlyGame.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "ss-aqua-room-nw", x: 2, y: 7 },
        },
        story: { vars: { ssAquaState: 1 } },
        party: [{ species: "lapras", level: 100, moves: ["surf"] }],
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })

      await stanlyGame.player.interact()
      await winTrainerBattleWithFirstMove(stanlyGame)

      await expect(stanlyGame.story.var("ssAquaState")).resolves.toBe(1)
    } finally {
      await stanlyGame.close()
    }
  })

  it("keeps state 4 and the Ticket pending when Key Items are full", async () => {
    await arrangeAtGrandfather(game, 4, { fullPockets: ["keyItems"] })

    await game.player.interact()
    await finishScript(game)

    await expect(game.story.var("ssAquaState")).resolves.toBe(4)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(false)
  })

  it("keeps state 5 and the Metal Coat pending when Items are full", async () => {
    await arrangeAtGrandfather(game, 5, { fullPockets: ["items"] })

    await game.player.interact()
    await finishScript(game)

    await expect(game.story.var("ssAquaState")).resolves.toBe(5)
    await expect(game.inventory.contains("metalCoat")).resolves.toBe(false)
  })

  it("adds exactly one Metal Coat, commits both rewards, and never repeats them", async () => {
    await arrangeAtGrandfather(game, 4, { items: { metalCoat: 1 } })

    await game.player.interact()
    await finishScript(game)

    await expect(game.story.var("ssAquaState")).resolves.toBe(6)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)
    await expect(game.state.read()).resolves.toMatchObject({
      bag: { items: { metalCoat: 2 } },
    })

    await game.player.interact()
    await finishScript(game)
    await expect(game.story.var("ssAquaState")).resolves.toBe(6)
    await expect(game.state.read()).resolves.toMatchObject({
      bag: { items: { metalCoat: 2 } },
    })
  })

  it("treats an existing Ticket as committed but still delivers the reunion Metal Coat", async () => {
    await arrangeAtGrandfather(game, 4, { items: { ssTicket: 1 } })

    await game.player.interact()
    await finishScript(game)

    await expect(game.story.var("ssAquaState")).resolves.toBe(6)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)
    await expect(game.inventory.contains("metalCoat")).resolves.toBe(true)
  })

  it("retries both pending rewards after saving and reloading at state 4", async () => {
    const reloadGame = await GameSession.launch()
    try {
      await arrangeAtGrandfather(reloadGame, 4)

      await reloadGame.saveAndReload()
      await reloadGame.wait.forMap("ss-aqua-room-sse")

      await expect(reloadGame.story.var("ssAquaState")).resolves.toBe(4)
      await expect(reloadGame.inventory.contains("ssTicket")).resolves.toBe(false)
      await expect(reloadGame.inventory.contains("metalCoat")).resolves.toBe(false)
      await reloadGame.player.interact()
      await finishScript(reloadGame)

      await expect(reloadGame.story.var("ssAquaState")).resolves.toBe(6)
      await expect(reloadGame.inventory.contains("ssTicket")).resolves.toBe(true)
      await expect(reloadGame.inventory.contains("metalCoat")).resolves.toBe(true)
    } finally {
      await reloadGame.close()
    }
  })

  it("retries the pending Metal Coat after saving and reloading at state 5", async () => {
    const reloadGame = await GameSession.launch()
    try {
      await arrangeAtGrandfather(reloadGame, 5, { items: { ssTicket: 1 } })

      await reloadGame.saveAndReload()
      await reloadGame.wait.forMap("ss-aqua-room-sse")

      await expect(reloadGame.story.var("ssAquaState")).resolves.toBe(5)
      await expect(reloadGame.inventory.contains("ssTicket")).resolves.toBe(true)
      await expect(reloadGame.inventory.contains("metalCoat")).resolves.toBe(false)
      await reloadGame.player.interact()
      await finishScript(reloadGame)

      await expect(reloadGame.story.var("ssAquaState")).resolves.toBe(6)
      await expect(reloadGame.inventory.contains("ssTicket")).resolves.toBe(true)
      await expect(reloadGame.inventory.contains("metalCoat")).resolves.toBe(true)
    } finally {
      await reloadGame.close()
    }
  })

  it("commits only the Kanto and Vermilion visit state on first disembarkation", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "ss-aqua-room-sse", x: 3, y: 6 },
      },
      story: {
        vars: {
          ssAquaState: 6,
          kantoRocketStoryState: 4,
          fanClubClefairy: 2,
          numBadges: 3,
          vermilionCityState: 5,
        },
        flags: {
          returnedMachinePart: false,
          kantoRadioGot: false,
          hideVermilionSnorlax: false,
          badge9: false,
          visitedKanto: false,
          visitedVermilionCity: false,
        },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.move("down")
    await game.wait.frames(30)
    await game.player.move("down")
    await game.wait.frames(120)
    await expect(game.state.read()).resolves.toMatchObject({ map: { name: "ss-aqua-1f" } })
    await finishScript(game)

    await expect(game.story.var("ssAquaState")).resolves.toBe(7)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(2)
    await expect(game.story.var("numBadges")).resolves.toBe(3)
    await expect(game.story.var("vermilionCityState")).resolves.toBe(5)
    await expect(game.story.flag("visitedKanto")).resolves.toBe(false)
    await expect(game.story.flag("visitedVermilionCity")).resolves.toBe(false)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "ss-aqua-1f", x: 29, y: 3 },
      },
      story: {
        vars: {
          ssAquaState: 7,
          kantoRocketStoryState: 4,
          fanClubClefairy: 2,
          numBadges: 3,
          vermilionCityState: 5,
        },
        flags: {
          returnedMachinePart: false,
          kantoRadioGot: false,
          hideVermilionSnorlax: false,
          badge9: false,
          visitedKanto: false,
          visitedVermilionCity: false,
        },
      },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await advanceUntilMap(game, "vermilion-port-inside")

    await expect(game.story.var("ssAquaState")).resolves.toBe(8)
    await expect(game.story.flag("visitedKanto")).resolves.toBe(true)
    await expect(game.story.flag("visitedVermilionCity")).resolves.toBe(true)
    await expect(game.story.var("kantoRocketStoryState")).resolves.toBe(4)
    await expect(game.story.var("fanClubClefairy")).resolves.toBe(2)
    await expect(game.story.var("numBadges")).resolves.toBe(3)
    await expect(game.story.var("vermilionCityState")).resolves.toBe(5)
    await expect(game.story.flag("returnedMachinePart")).resolves.toBe(false)
    await expect(game.story.flag("kantoRadioGot")).resolves.toBe(false)
    await expect(game.story.flag("hideVermilionSnorlax")).resolves.toBe(false)
    await expect(game.story.flag("badge9")).resolves.toBe(false)
  })

  it("preserves the completed voyage while the first League clear is committed", async () => {
    const leagueGame = await GameSession.launch()
    try {
      await leagueGame.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "hall-of-fame", x: 5, y: 12 },
        },
        story: {
          vars: { leagueState: 5, ssAquaState: 8 },
          flags: {
            isChampion: false,
            visitedKanto: true,
            visitedVermilionCity: true,
          },
        },
        party: [{ species: "lapras", level: 100, moves: ["surf"] }],
        determinism: { textSpeed: "instant" },
      })

      await leagueGame.story.setVar("leagueState", 6)
      await finishFirstLeagueClearFlags(leagueGame)

      await expect(leagueGame.story.flag("isChampion")).resolves.toBe(true)
      await expect(leagueGame.story.var("ssAquaState")).resolves.toBe(8)
      await expect(leagueGame.story.flag("visitedKanto")).resolves.toBe(true)
      await expect(leagueGame.story.flag("visitedVermilionCity")).resolves.toBe(true)
    } finally {
      await leagueGame.close()
    }
  })

  it("runs the unlocked ferry from Olivine to Vermilion with only the Ticket", async () => {
    const ferryGame = await GameSession.launch()
    try {
      await ferryGame.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "down",
          position: { map: "olivine-port-inside", x: 8, y: 16 },
        },
        story: {
          vars: { ssAquaState: 8 },
          flags: { returnedMachinePart: false },
        },
        bag: { items: { ssTicket: 1 } },
        determinism: { textSpeed: "instant" },
      })

      await ferryGame.player.interact()
      await advanceUntilMap(ferryGame, "vermilion-port-inside")
      await expect(ferryGame.story.flag("returnedMachinePart")).resolves.toBe(false)
      await expect(ferryGame.inventory.contains("ssTicket")).resolves.toBe(true)
      await expect(ferryGame.story.var("ssAquaState")).resolves.toBe(8)
    } finally {
      await ferryGame.close()
    }
  })

  it("denies repeat ferry travel without the Ticket from either port", async () => {
    const ports = [
      { map: "olivine-port-inside" as const, x: 8, y: 16, facing: "down" as const },
      { map: "vermilion-port-inside" as const, x: 8, y: 9, facing: "down" as const },
    ]

    for (const port of ports) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: port.facing,
          position: { map: port.map, x: port.x, y: port.y },
        },
        story: {
          vars: { ssAquaState: 8 },
          flags: { returnedMachinePart: false },
        },
        determinism: { textSpeed: "instant" },
      })

      await expect(game.inventory.contains("ssTicket")).resolves.toBe(false)
      await game.player.interact()
      await finishScript(game)

      await expect(game.state.read()).resolves.toMatchObject({ map: { name: port.map } })
      await expect(game.story.var("ssAquaState")).resolves.toBe(8)
      await expect(game.story.flag("returnedMachinePart")).resolves.toBe(false)
    }
  })

  it("preserves the completed voyage state through an actual flash save and ROM reload", async () => {
    const reloadGame = await GameSession.launch()
    try {
      await reloadGame.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "hall-of-fame", x: 5, y: 10 } },
        story: {
          vars: { ssAquaState: 8 },
          flags: { visitedKanto: true, visitedVermilionCity: true },
        },
      })

      await reloadGame.saveAndReload()
      await reloadGame.wait.forMap("hall-of-fame")

      await expect(reloadGame.story.var("ssAquaState")).resolves.toBe(8)
      await expect(reloadGame.story.flag("visitedKanto")).resolves.toBe(true)
      await expect(reloadGame.story.flag("visitedVermilionCity")).resolves.toBe(true)
    } finally {
      await reloadGame.close()
    }
  })
})
