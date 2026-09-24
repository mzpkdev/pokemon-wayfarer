import { afterEach, beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"

const settleField = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.scriptActive && !state.dialogueOpen && !state.battle.active) return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not release the field: ${JSON.stringify(await game.state.read())}`,
  )
}

const waitForBattle = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not start a battle: ${JSON.stringify(await game.state.read())}`,
  )
}

const defeatTrainer = async (
  game: GameSession,
  map: GameMap,
  x: number,
  y: number,
  description: string,
): Promise<void> => {
  await game.player.warp(map, x, y, "up")
  await game.player.interact()
  await waitForBattle(game, description)
  await game.battle.win()
  await settleField(game, `${description} victory`)
}

const enterTower = async (game: GameSession): Promise<void> => {
  await game.player.warp("lavender-radio-lobby", 17, 5, "up")
  await game.player.interact()
  for (let attempt = 0; attempt < 120; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text.includes("Would you like to visit")) {
      await game.controls.press("a")
      await game.wait.forMap("pokemon-tower-2f")
      return
    }
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`Tower guard did not offer entry: ${JSON.stringify(await game.state.read())}`)
}

const triggerMarowak = async (game: GameSession): Promise<void> => {
  await game.player.warp("pokemon-tower-6f", 11, 14, "down")
  const sequence = (await game.state.read()).dialogue.sequence
  for (let attempt = 0; attempt < 5; attempt++) {
    await game.wait.forReady()
    await game.player.move("down")
    await game.wait.frames(24)
    if ((await game.state.read()).dialogue.sequence > sequence) {
      await waitForBattle(game, "identified Marowak")
      return
    }
  }
  throw new Error(`Marowak did not appear: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer Kanto Scope handoff", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
  })

  afterEach(async () => {
    await game.close()
  })

  it("carries Giovanni's physical Scope through Marowak and Fuji's physical Flute", async () => {
    // Begin at the Hideout's final room to bound travel and combat setup. Every
    // prerequisite battle and the Scope reward still run through the live scripts.
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b4f", x: 16, y: 15 }, facing: "up" },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      story: { flags: { disableEncounters: true } },
      determinism: { textSpeed: "instant", rngSeed: 1 },
    })
    expect(await game.inventory.contains("silphScope")).toBe(false)
    expect(await game.story.flag("returnedMachinePart")).toBe(false)
    expect(await game.story.flag("kantoRadioGot")).toBe(false)

    await defeatTrainer(game, "celadon-hideout-b4f", 16, 15, "left door guard")
    await defeatTrainer(game, "celadon-hideout-b4f", 19, 15, "right door guard")
    expect(await game.story.flag("celadonHideoutLeftGuardDefeated")).toBe(true)
    expect(await game.story.flag("celadonHideoutRightGuardDefeated")).toBe(true)
    await game.saveAndReload()
    await defeatTrainer(game, "celadon-hideout-b4f", 19, 5, "Hideout Giovanni")
    expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(true)
    expect(await game.inventory.contains("silphScope")).toBe(false)

    await game.player.warp("celadon-hideout-b4f", 20, 6, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "Giovanni's Scope reward")
    expect(await game.story.flag("celadonHideoutScopeReceived")).toBe(true)
    expect(await game.inventory.contains("silphScope")).toBe(true)
    await game.saveAndReload()
    expect(await game.inventory.contains("silphScope")).toBe(true)
    expect(await game.story.flag("returnedMachinePart")).toBe(false)

    await enterTower(game)
    await triggerMarowak(game)
    await game.battle.win()
    await settleField(game, "Marowak resolution")
    expect(await game.story.flag("towerMarowakCalmed")).toBe(true)
    expect(await game.inventory.contains("silphScope")).toBe(true)
    await game.saveAndReload()

    const rockets = [
      [9, 11, "towerRocket1Cleared"],
      [13, 9, "towerRocket2Cleared"],
      [9, 7, "towerRocket3Cleared"],
    ] as const
    for (const [x, y, flag] of rockets) {
      await defeatTrainer(game, "pokemon-tower-7f", x, y, `Tower Rocket at ${x}:${y}`)
      expect(await game.story.flag(flag)).toBe(true)
    }
    expect(await game.story.flag("towerFujiRescued")).toBe(false)
    await game.player.warp("pokemon-tower-7f", 11, 5, "up")
    await game.wait.frames(60)
    let reachedFuji = false
    for (let attempt = 0; attempt < 5; attempt++) {
      await game.wait.forReady()
      const beforeDialogue = (await game.state.read()).dialogue.sequence
      await game.player.interact()
      await game.wait.frames(30)
      const state = await game.state.read()
      if (
        state.scriptActive ||
        state.dialogue.sequence > beforeDialogue ||
        state.map.name === "lavender-house1"
      ) {
        reachedFuji = true
        break
      }
    }
    expect(reachedFuji).toBe(true)
    await settleField(game, "Fuji rescue")
    await game.wait.forMap("lavender-house1")
    expect(await game.story.flag("towerFujiRescued")).toBe(true)
    expect(await game.story.flag("towerFluteClaimed")).toBe(false)

    await game.player.warp("lavender-house1", 8, 5, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "Fuji's Flute gift")
    expect(await game.story.flag("towerFluteClaimed")).toBe(true)
    expect(await game.inventory.contains("pokeFlute")).toBe(true)
    await game.saveAndReload()
    expect(await game.inventory.contains("silphScope")).toBe(true)
    expect(await game.inventory.contains("pokeFlute")).toBe(true)
    expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(true)
    expect(await game.story.flag("towerMarowakCalmed")).toBe(true)
    expect(await game.story.flag("towerFujiRescued")).toBe(true)
    expect(await game.story.flag("returnedMachinePart")).toBe(false)
    expect(await game.story.flag("kantoRadioGot")).toBe(false)

    // Radio repair remains an independent errand after the entire Tower story.
    await game.player.warp("lavender-radio-lobby", 12, 3, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "radio director before Machine Part")
    expect(await game.story.flag("kantoRadioGot")).toBe(false)
    await game.story.setFlag("returnedMachinePart", true)
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "radio director after Machine Part")
    expect(await game.story.flag("kantoRadioGot")).toBe(true)
    expect(await game.story.flag("towerFluteClaimed")).toBe(true)
    expect(await game.story.flag("celadonHideoutScopeReceived")).toBe(true)
  })
})
