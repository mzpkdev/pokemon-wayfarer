import * as fs from "node:fs/promises"
import { afterEach, beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction, type GameMap } from "../harness/game-session"

const lobby = "lavender-radio-lobby" as const
const house = "lavender-house1" as const

const capture = async (game: GameSession, name: string): Promise<void> => {
  await fs.writeFile(`/tmp/lavender-tower-${name}.png`, await game.screenshot())
}

const settleField = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.scriptActive && !state.dialogueOpen && !state.battle.active) return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`${description} did not release the field: ${JSON.stringify(await game.state.read())}`)
}

const waitForBattle = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`${description} did not start a battle: ${JSON.stringify(await game.state.read())}`)
}

const arrangeAt = async (
  game: GameSession,
  map: GameMap,
  x: number,
  y: number,
  facing: Direction,
  options: {
    scope?: boolean
    rescued?: boolean
    calmed?: boolean
    fullBag?: boolean
    encounters?: boolean
  } = {},
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { position: { map, x, y }, facing },
    story: {
      flags: {
        disableEncounters: !options.encounters,
        towerFujiRescued: options.rescued ?? false,
        towerMarowakCalmed: options.calmed ?? false,
      },
    },
    bag: {
      items: options.scope ? { silphScope: 1 } : {},
      fullPockets: options.fullBag ? ["keyItems"] : [],
    },
    party: [{ species: "lapras", level: 100, moves: ["surf"] }],
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
}

const guardChoice = async (game: GameSession, enter: boolean): Promise<void> => {
  await game.player.interact()
  for (let attempt = 0; attempt < 120; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text.includes("Would you like to visit")) {
      if (!enter) await game.controls.press("down")
      await game.controls.press("a")
      return
    }
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`Guard did not offer the memorial floors: ${JSON.stringify(await game.state.read())}`)
}

const stair = async (
  game: GameSession,
  from: GameMap,
  x: number,
  y: number,
  direction: Direction,
  to: GameMap,
): Promise<void> => {
  await game.player.warp(from, x, y, direction)
  await game.wait.forMap(from)
  await game.wait.frames(60)
  await walkToMap(game, direction, to, `${from} stair to ${to}`)
}

const walkToMap = async (
  game: GameSession,
  direction: Direction,
  destination: GameMap,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 5; attempt++) {
    await game.wait.forReady()
    if ((await game.state.read()).map.name === destination) {
      await game.wait.forMap(destination)
      return
    }
    await game.player.move(direction)
    await game.wait.frames(60)
    if ((await game.state.read()).map.name === destination) {
      await game.wait.forMap(destination)
      return
    }
  }
  throw new Error(`${description} did not reach ${destination}: ${JSON.stringify(await game.state.read())}`)
}

const walkOneTile = async (
  game: GameSession,
  direction: Direction,
  x: number,
  y: number,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 5; attempt++) {
    await game.wait.until(
      (state) => state.ready || state.phase === "battle",
      `${description} field or battle readiness`,
    )
    const before = await game.state.read()
    if (before.phase === "battle" || before.battle.active) return
    await game.player.move(direction)
    await game.wait.frames(24)
    const state = await game.state.read()
    if (state.phase === "battle" || state.battle.active || (state.player.x === x && state.player.y === y)) return
  }
  throw new Error(`${description} could not reach ${x}:${y}: ${JSON.stringify(await game.state.read())}`)
}

const walkIntoGhostTrigger = async (game: GameSession, description: string): Promise<void> => {
  const before = (await game.state.read()).dialogue.sequence
  for (let attempt = 0; attempt < 5; attempt++) {
    await game.wait.until(
      (state) => state.ready || state.dialogue.sequence > before,
      `${description} field or dialogue readiness`,
    )
    if ((await game.state.read()).dialogue.sequence > before) return
    await game.player.move("down")
    await game.wait.frames(24)
    if ((await game.state.read()).dialogue.sequence > before) return
  }
  throw new Error(`${description} did not show the ghost: ${JSON.stringify(await game.state.read())}`)
}

const naturalTowerEncounter = async (
  game: GameSession,
  floor: GameMap,
  leftX: number,
  y: number,
  maxSteps: number,
): Promise<void> => {
  await arrangeAt(game, floor, leftX, y, "right", { encounters: true, scope: true })
  expect(await game.inventory.contains("silphScope")).toBe(true)
  const rightX = leftX + 1
  for (let step = 0; step < maxSteps; step++) {
    if ((await game.state.read()).phase === "battle") break
    const direction = step % 2 === 0 ? "right" : "left"
    await walkOneTile(
      game,
      direction,
      direction === "right" ? rightX : leftX,
      y,
      `${floor} encounter step ${step}`,
    )
    const state = await game.state.read()
    if (state.phase === "battle" || state.battle.active) break
    expect(state.map.name).toBe(floor)
  }
  await game.wait.until(
    (state) => state.battle.active,
    `natural ${floor} wild encounter within ${maxSteps} steps`,
    120,
  )
  await waitForBattle(game, `natural ${floor} wild encounter`)
  const enemy = (await game.state.read()).battle.enemy
  expect(enemy).not.toBeNull()
  expect(["gastly", "haunter", "cubone"]).toContain(enemy!.species)
  expect(enemy!.level).toBeGreaterThan(0)
  await game.battle.win()
  await settleField(game, `natural ${floor} battle finish`)
  expect(await game.story.flag("towerMarowakCalmed")).toBe(false)
}

describe.sequential("Wayfarer Lavender Pokemon Tower", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
  })

  afterEach(async () => {
    await game.close()
  })

  it("offers guard entry before Scope and radio repair; decline and downward stair return preserve both stories", async () => {
    await arrangeAt(game, lobby, 17, 5, "up")
    await capture(game, "lobby-before-entry")
    expect(await game.inventory.contains("silphScope")).toBe(false)
    expect(await game.story.flag("returnedMachinePart")).toBe(false)
    await guardChoice(game, false)
    await settleField(game, "guard decline")
    await expect(game.state.read()).resolves.toMatchObject({ map: { name: lobby } })
    expect(await game.story.flag("towerMarowakCalmed")).toBe(false)
    expect(await game.story.flag("towerFujiRescued")).toBe(false)

    await guardChoice(game, true)
    await game.wait.forMap("pokemon-tower-2f")
    await capture(game, "2f-arrival")
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 18, y: 11 } })
    await game.saveAndReload()
    await walkToMap(game, "up", lobby, "saved 2F lobby return")
    await capture(game, "lobby-return")
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 17, y: 5 } })
    expect(await game.story.flag("towerFujiRescued")).toBe(false)
    expect(await game.story.flag("kantoRadioGot")).toBe(false)

    await game.player.warp(lobby, 12, 3, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "director before Machine Part repair")
    expect(await game.story.flag("kantoRadioGot")).toBe(false)
    await game.story.setFlag("returnedMachinePart", true)
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "director after Machine Part repair")
    expect(await game.story.flag("kantoRadioGot")).toBe(true)
    expect(await game.story.flag("towerFujiRescued")).toBe(false)
  })

  it("keeps every source stair edge and the lobby return walkable after save/reload", async () => {
    await arrangeAt(game, "pokemon-tower-2f", 18, 11, "up", { calmed: true })
    const edges = [
      ["pokemon-tower-2f", 4, 11, "up", "pokemon-tower-3f"],
      ["pokemon-tower-3f", 4, 11, "up", "pokemon-tower-2f"],
      ["pokemon-tower-3f", 18, 11, "up", "pokemon-tower-4f"],
      ["pokemon-tower-4f", 18, 11, "up", "pokemon-tower-3f"],
      ["pokemon-tower-4f", 4, 11, "up", "pokemon-tower-5f"],
      ["pokemon-tower-5f", 4, 11, "up", "pokemon-tower-4f"],
      ["pokemon-tower-5f", 18, 11, "up", "pokemon-tower-6f"],
      ["pokemon-tower-6f", 18, 11, "up", "pokemon-tower-5f"],
      ["pokemon-tower-6f", 11, 15, "down", "pokemon-tower-7f"],
      ["pokemon-tower-7f", 11, 15, "down", "pokemon-tower-6f"],
    ] as const
    for (const [from, x, y, direction, to] of edges)
      await stair(game, from, x, y, direction, to)

    await game.saveAndReload()
    await stair(game, "pokemon-tower-2f", 18, 11, "up", lobby)
  })

  it("remembers Blue without a visiting-origin battle and blocks an unidentified Marowak", async () => {
    await arrangeAt(game, "pokemon-tower-2f", 16, 6, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "Blue visitor introduction")
    expect(await game.story.flag("towerBlueMet")).toBe(true)
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
    await game.saveAndReload()
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "Blue revisit")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })

    await game.player.warp("pokemon-tower-6f", 11, 14, "down")
    await walkIntoGhostTrigger(game, "unidentified ghost")
    await settleField(game, "unidentified ghost")
    expect(await game.story.flag("towerMarowakCalmed")).toBe(false)
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
  })

  it("keeps Marowak unidentified before Scope while ordinary encounters remain enabled", async () => {
    await arrangeAt(game, "pokemon-tower-6f", 11, 14, "down", { encounters: true })
    expect(await game.inventory.contains("silphScope")).toBe(false)
    await walkIntoGhostTrigger(game, "6F pre-Scope ghost trigger")
    await settleField(game, "unidentified Marowak with encounters enabled")
    expect(await game.story.flag("towerMarowakCalmed")).toBe(false)
    await expect(game.state.read()).resolves.toMatchObject({
      player: { x: 11, y: 14 },
      battle: { active: false },
    })
  })

  it("starts live wild encounters on Tower 3F through 7F", async () => {
    // These pairs are clear 0x282 cave-encounter tiles at elevation 3 in the
    // selected FRLG map.bin files, away from warps, NPCs, and scripted tiles.
    await naturalTowerEncounter(game, "pokemon-tower-3f", 13, 14, 400)
    await naturalTowerEncounter(game, "pokemon-tower-4f", 10, 10, 300)
    await naturalTowerEncounter(game, "pokemon-tower-5f", 14, 13, 240)
    await naturalTowerEncounter(game, "pokemon-tower-6f", 4, 12, 200)
    await naturalTowerEncounter(game, "pokemon-tower-7f", 10, 12, 180)
  })

  it("keeps an ordinary Tower item and Channeler victory after reload", async () => {
    await arrangeAt(game, "pokemon-tower-3f", 13, 3, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "3F Escape Rope pickup")
    expect(await game.story.flag("towerEscapeRopeClaimed")).toBe(true)

    await game.player.warp("pokemon-tower-3f", 9, 14, "right")
    await game.player.interact()
    await waitForBattle(game, "3F Channeler")
    await game.battle.win()
    await settleField(game, "3F Channeler victory")
    await game.saveAndReload()
    expect(await game.story.flag("towerEscapeRopeClaimed")).toBe(true)
    await game.player.interact()
    await settleField(game, "3F Channeler revisit")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
  })

  it("keeps a 3F item claimable after a full Items pocket and reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "pokemon-tower-3f", x: 13, y: 3 }, facing: "up" },
      story: { flags: { disableEncounters: true } },
      bag: { fullPockets: ["items"] },
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "full Items pocket")
    expect(await game.story.flag("towerEscapeRopeClaimed")).toBe(false)
    expect(await game.inventory.contains("escapeRope")).toBe(false)

    await game.saveAndReload()
    await game.inventory.freeSlot("items")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "Escape Rope retry")
    expect(await game.story.flag("towerEscapeRopeClaimed")).toBe(true)
    expect(await game.inventory.contains("escapeRope")).toBe(true)
  })

  it("heals a fainted party member in the 5F purified zone", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "pokemon-tower-5f", x: 9, y: 8 }, facing: "right" },
      story: { flags: { disableEncounters: true } },
      party: [{ species: "pidgey", fainted: true }, { species: "lapras", level: 100 }],
      determinism: { textSpeed: "instant" },
    })
    expect((await game.state.read()).party[0]?.fainted).toBe(true)
    await walkOneTile(game, "right", 10, 8, "5F purified zone")
    await settleField(game, "5F healing zone")
    expect((await game.state.read()).party[0]?.fainted).toBe(false)
    expect((await game.state.read()).partyVitals[0]!.hp).toBeGreaterThan(0)
  })

  it("keeps a lost Marowak encounter retryable, then persists its resolution", async () => {
    await arrangeAt(game, "pokemon-tower-6f", 11, 14, "down", { scope: true })
    await walkIntoGhostTrigger(game, "first Marowak")
    await waitForBattle(game, "first Marowak")
    await game.battle.lose()
    await settleField(game, "lost Marowak encounter")
    expect(await game.story.flag("towerMarowakCalmed")).toBe(false)
    expect(await game.inventory.contains("silphScope")).toBe(true)

    await game.player.warp("pokemon-tower-6f", 11, 14, "down")
    await walkIntoGhostTrigger(game, "retried Marowak")
    await waitForBattle(game, "retried Marowak")
    await game.battle.win()
    await settleField(game, "calmed Marowak")
    expect(await game.story.flag("towerMarowakCalmed")).toBe(true)
    await game.saveAndReload()
    expect(await game.story.flag("towerMarowakCalmed")).toBe(true)
  })

  it("stages Fuji at his home only after rescue", async () => {
    await arrangeAt(game, house, 4, 6, "up")
    await capture(game, "house1-fuji-absent")
    expect(await game.story.flag("towerHouse1FujiHidden")).toBe(true)
    await game.player.warp("lavender-soul-house", 11, 16, "up")
    await game.wait.forMap("lavender-soul-house")
    await capture(game, "soul-house")
    await arrangeAt(game, house, 4, 6, "up", { rescued: true })
    expect(await game.story.flag("towerHouse1FujiHidden")).toBe(false)
    await capture(game, "house1-fuji-returned")
  })

  it("does not clear a lost Rocket, then rescues Fuji only after all three victories", async () => {
    await arrangeAt(game, "pokemon-tower-7f", 9, 11, "up", { calmed: true })
    await game.player.interact()
    await waitForBattle(game, "first Rocket")
    await game.battle.lose()
    await settleField(game, "lost Rocket encounter")
    expect(await game.story.flag("towerRocket1Cleared")).toBe(false)
    expect(await game.story.flag("towerFujiRescued")).toBe(false)

    const rockets = [
      [9, 11, "towerRocket1Cleared"],
      [13, 9, "towerRocket2Cleared"],
      [9, 7, "towerRocket3Cleared"],
    ] as const
    for (const [x, y, flag] of rockets) {
      await game.player.warp("pokemon-tower-7f", x, y, "up")
      await game.player.interact()
      await waitForBattle(game, `Rocket at ${x}:${y}`)
      await game.battle.win()
      await settleField(game, `Rocket at ${x}:${y} defeated`)
      expect(await game.story.flag(flag)).toBe(true)
    }
    expect(await game.story.flag("towerFujiRescued")).toBe(false)
    await game.player.warp("pokemon-tower-7f", 11, 5, "up")
    await game.wait.frames(60)
    await capture(game, "7f-fuji-before-rescue")
    expect(await game.story.flag("towerFujiRescued")).toBe(false)
    let reachedFuji = false
    for (let attempt = 0; attempt < 5; attempt++) {
      await game.wait.forReady()
      const beforeDialogue = (await game.state.read()).dialogue.sequence
      await game.player.interact()
      await game.wait.frames(30)
      const state = await game.state.read()
      if (state.scriptActive || state.dialogue.sequence > beforeDialogue || state.map.name === house) {
        reachedFuji = true
        break
      }
    }
    expect(reachedFuji).toBe(true)
    await settleField(game, "Fuji rescue return")
    await expect(game.state.read()).resolves.toMatchObject({ map: { name: house } })
    expect(await game.story.flag("towerFujiRescued")).toBe(true)
    expect(await game.story.flag("towerFluteClaimed")).toBe(false)
    await game.saveAndReload()
    expect(await game.story.flag("towerFujiRescued")).toBe(true)
    await game.player.warp("pokemon-tower-7f", 11, 5, "up")
    await game.wait.forReady()
    await game.wait.frames(60)
    await game.player.interact()
    await game.wait.frames(30)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "pokemon-tower-7f" },
      dialogueOpen: false,
    })
  })

  it("keeps Fuji's physical Flute claimable with a full Bag or an already-owned Flute", async () => {
    await arrangeAt(game, house, 8, 5, "up", { rescued: true, fullBag: true })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "full Bag Flute refusal")
    expect(await game.story.flag("towerFluteClaimed")).toBe(false)
    expect(await game.inventory.contains("pokeFlute")).toBe(false)
    await game.saveAndReload()
    await game.inventory.freeSlot("keyItems")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "Flute retry")
    expect(await game.story.flag("towerFluteClaimed")).toBe(true)
    expect(await game.inventory.contains("pokeFlute")).toBe(true)
    await game.saveAndReload()
    expect(await game.inventory.contains("pokeFlute")).toBe(true)

    // An independently acquired physical Flute must settle the one-time gift.
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: house, x: 8, y: 5 }, facing: "up" },
      story: { flags: { towerFujiRescued: true } },
      bag: { items: { pokeFlute: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settleField(game, "already-owned Flute")
    expect(await game.story.flag("towerFluteClaimed")).toBe(true)
  })

  it("keeps the memorial entrance permanently accessible after Fuji returns", async () => {
    await arrangeAt(game, lobby, 17, 5, "up", { rescued: true, calmed: true })
    await guardChoice(game, true)
    await game.wait.forMap("pokemon-tower-2f")
    await walkToMap(game, "up", lobby, "post-rescue lobby return")
    expect(await game.story.flag("towerFujiRescued")).toBe(true)
  })
})
