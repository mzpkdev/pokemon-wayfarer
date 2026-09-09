import * as fs from "node:fs"
import { beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type Checkpoint, type Direction, type PartyMonFixture } from "../harness/game-session"

const settle = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active) return
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`Burglary interaction did not settle: ${JSON.stringify(await game.state.read())}`)
}

const arrange = async (
  game: GameSession,
  options: Partial<Parameters<GameSession["arrange"]>[0]> = {},
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { position: { map: "cerulean-city", x: 36, y: 23 }, facing: "right" },
    party: [{ species: "lapras", level: 100 }],
    determinism: { textSpeed: "instant" },
    ...options,
  })
}

const moveTo = async (game: GameSession, direction: Direction, x: number, y: number): Promise<void> => {
  for (let attempt = 0; attempt < 8; attempt++) {
    if ((await game.state.read()).player.x === x && (await game.state.read()).player.y === y) return
    await game.controls.press(direction)
    await game.wait.frames(20)
  }
  expect((await game.state.read()).player).toMatchObject({ x, y })
}

const startBattle = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  for (let attempt = 0; attempt < 180; attempt++) {
    if ((await game.state.read()).battle.ui === "action-menu") return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`Burglary battle did not start: ${JSON.stringify(await game.state.read())}`)
}

const expectDialogue = async (game: GameSession, snippet: string): Promise<void> => {
  for (let page = 0; page < 12; page++) {
    if ((await game.state.read()).dialogue.text.toLowerCase().includes(snippet)) return
    await game.controls.press("a")
    await game.wait.frames(20)
  }
  throw new Error(`Missing dialogue ${snippet}: ${JSON.stringify((await game.state.read()).dialogue)}`)
}

const talk = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await settle(game)
}

const giveFixtureTm = async (game: GameSession): Promise<void> => {
  await game.controls.press("start")
  await game.wait.until((state) => state.ui.mode === "pause-menu", "open pause menu")
  await game.wait.frames(60)
  await game.controls.press("down")
  await game.controls.press("a")
  await game.wait.frames(90)
  for (let pocket = 0; pocket < 4; pocket++) {
    await game.controls.press("right")
    await game.wait.frames(30)
  }
  await game.controls.press("a")
  await game.wait.frames(30)
  await game.controls.press("right")
  await game.controls.press("a")
  await game.wait.until((state) => state.partyMenu.open, "choose a Pokémon to hold TM01")
  await game.wait.frames(30)
  await game.controls.press("a")
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.frames(90)
  for (let attempt = 0; attempt < 8; attempt++) {
    if ((await game.state.read()).ready) return
    await game.controls.press("b")
    await game.wait.frames(30)
  }
  throw new Error(`Giving fixture TM did not settle: ${JSON.stringify(await game.state.read())}`)
}

const assertPending = async (game: GameSession, defeated: boolean): Promise<void> => {
  expect(await game.story.flag("ceruleanBurglaryGruntDefeated")).toBe(defeated)
  expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(false)
  expect(await game.inventory.contains("tmDig")).toBe(false)
}

const assertMachinePart = async (game: GameSession): Promise<void> => {
  expect(await game.story.var("ceruleanCityState")).toBe(4)
  expect(await game.story.var("kantoRocketStoryState")).toBe(2)
  expect(await game.story.flag("returnedMachinePart")).toBe(false)
  expect(await game.story.flag("hiddenMachinePart")).toBe(true)
  expect(await game.story.flag("hideCeruleanGymRocket")).toBe(true)
  expect(await game.story.flag("hideCeruleanCapeRocket")).toBe(true)
  expect(await game.story.flag("hideRoute25Misty")).toBe(false)
  expect(await game.inventory.contains("machinePart")).toBe(true)
}

const assertGruntGone = async (game: GameSession): Promise<void> => {
  await game.player.warp("cerulean-city", 36, 23, "right")
  await game.controls.press("right")
  await game.wait.frames(30)
  expect((await game.state.read()).player).toMatchObject({ x: 37, y: 23 })
  expect((await game.state.read()).battle.active).toBe(false)
}

describe.sequential("Wayfarer Cerulean burglary", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("refuses empty, fainted-only, and Eggs-only parties without starting the theft battle", async () => {
    for (const party of [
      [],
      [{ species: "lapras", fainted: true }],
      [{ species: "lapras", egg: true }],
      [{ species: "lapras", fainted: true }, { species: "lapras", egg: true }],
    ] as PartyMonFixture[][]) {
      await arrange(game, { party })
      await talk(game)
      expect((await game.state.read()).battle.active).toBe(false)
      await assertPending(game, false)
    }
  })

  it("permits a mixed party with one usable Pokémon", async () => {
    await arrange(game, { party: [
      { species: "lapras", fainted: true },
      { species: "lapras", egg: true },
      { species: "lapras", level: 100 },
    ] })
    await startBattle(game)
    expect((await game.state.read()).battle.ui).toBe("action-menu")
    expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(false)
  })

  for (const checkpoint of ["new-bark-after-intro", "hoenn-before-rescue"] as Checkpoint[]) {
    it(`recovers Dig once for ${checkpoint} while preserving Machine Part progress`, async () => {
      await arrange(game, {
        checkpoint,
        bag: { items: { machinePart: 1 } },
        story: {
          vars: { ceruleanCityState: 4, kantoRocketStoryState: 2 },
          flags: {
            hiddenMachinePart: true,
            hideCeruleanGymRocket: true,
            hideCeruleanCapeRocket: true,
            hideRoute25Misty: false,
          },
        },
      })
      await fs.promises.writeFile(`/tmp/wayfarer-cerulean-burglary-${checkpoint}.png`, await game.screenshot())
      const before = await game.state.read()
      await startBattle(game)
      await game.battle.win()
      await settle(game)
      expect(await game.story.flag("ceruleanBurglaryGruntDefeated")).toBe(true)
      expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(true)
      expect(await game.inventory.contains("tmDig")).toBe(true)
      await assertMachinePart(game)
      await assertGruntGone(game)
      await game.saveAndReload()
      await assertMachinePart(game)
      expect(await game.inventory.contains("tmDig")).toBe(true)
      expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(true)
      const after = await game.state.read()
      expect(after.origin.id).toBe(before.origin.id)
      expect(after.origin.johtoCommitted).toBe(before.origin.johtoCommitted)
      expect(after.origin.hoennReceived).toBe(before.origin.hoennReceived)
      expect(after.circuit.badges).toEqual(before.circuit.badges)
      expect(after.circuit.clears).toEqual(before.circuit.clears)
      await assertGruntGone(game)
    })
  }

  it("keeps a lost confrontation retryable", async () => {
    await arrange(game, { party: [{ species: "lapras", level: 1 }] })
    await startBattle(game)
    await game.battle.lose()
    await settle(game)
    await assertPending(game, false)
    await game.player.warp("cerulean-city", 36, 23, "right")
    await startBattle(game)
    expect((await game.state.read()).battle.ui).toBe("action-menu")
  })

  it("retains a full TM/HM-pocket handoff after victory and reload", async () => {
    await arrange(game, { bag: { fullPockets: ["tmHm"] } })
    await startBattle(game)
    await game.battle.win()
    await settle(game)
    await assertPending(game, true)
    await game.saveAndReload()
    await assertPending(game, true)
    await talk(game)
    await assertPending(game, true)
    expect((await game.state.read()).battle.active).toBe(false)
    const tmCount = await game.inventory.count("tmFocusPunch")
    await giveFixtureTm(game)
    expect(await game.inventory.count("tmFocusPunch")).toBe(tmCount - 1)
    await talk(game)
    expect(await game.inventory.contains("tmDig")).toBe(true)
    expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(true)
    await assertGruntGone(game)
  })

  it("hands over Dig on a defeated Grunt's retry without another battle", async () => {
    await arrange(game, { story: { flags: { ceruleanBurglaryGruntDefeated: true } } })
    await talk(game)
    expect(await game.inventory.contains("tmDig")).toBe(true)
    expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(true)
    await assertGruntGone(game)
  })

  it("grants exactly one local copy after confronting the thief with pre-owned Dig", async () => {
    await arrange(game, { bag: { items: { tmDig: 1 }, fullPockets: ["tmHm"] } })
    expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(false)
    expect(await game.story.flag("ceruleanBurglaryGruntDefeated")).toBe(false)
    await startBattle(game)
    expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(false)
    await game.battle.win()
    await settle(game)
    expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(true)
    expect(await game.inventory.count("tmDig")).toBe(2)
    await assertGruntGone(game)
    await game.saveAndReload()
    expect(await game.inventory.count("tmDig")).toBe(2)
    await assertGruntGone(game)
  })

  it("keeps the front door usable and lets residents describe theft and recovery", async () => {
    await arrange(game, {
      player: { position: { map: "cerulean-city", x: 35, y: 23 }, facing: "up" },
    })
    await game.controls.press("up")
    await game.wait.forMap("cerulean-house-2")
    await game.wait.forReady()
    for (const resident of [{ x: 7, y: 5, facing: "right" }, { x: 1, y: 3, facing: "up" }] as const) {
      await game.player.warp("cerulean-house-2", resident.x, resident.y, resident.facing)
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await expectDialogue(game, resident.x === 7 ? "held up" : "stole my tm")
      await settle(game)
      await assertPending(game, false)
    }
    await game.player.warp("cerulean-house-2", 3, 6, "down")
    await game.controls.press("down")
    await game.wait.frames(20)
    await game.controls.press("down")
    await game.wait.forMap("cerulean-city")
    await game.wait.forReady()
    await game.player.warp("cerulean-city", 36, 23, "right")
    await startBattle(game)
    await game.battle.win()
    await settle(game)
    await game.player.warp("cerulean-city", 35, 23, "up")
    await game.controls.press("up")
    await game.wait.forMap("cerulean-house-2")
    await game.wait.forReady()
    for (const resident of [{ x: 7, y: 5, facing: "right" }, { x: 1, y: 3, facing: "up" }] as const) {
      await game.player.warp("cerulean-house-2", resident.x, resident.y, resident.facing)
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await expectDialogue(game, resident.x === 7 ? "stopped that thief" : "recovered the tm")
      await settle(game)
    }
  })

  for (const stage of ["pending", "reward-pending", "complete"] as const) {
    it(`keeps the rear route open in both directions when ${stage}`, async () => {
      await arrange(game, {
        player: { position: { map: "cerulean-house-2", x: 4, y: 3 }, facing: "up" },
        story: { flags: {
          ceruleanBurglaryGruntDefeated: stage !== "pending",
          ceruleanBurglaryRecovered: stage === "complete",
        } },
      })
      await game.controls.press("up")
      await game.wait.frames(20)
      expect((await game.state.read()).player).toMatchObject({ x: 4, y: 2 })
      await game.controls.press("up")
      await game.wait.forMap("cerulean-city")
      expect((await game.state.read()).player).toMatchObject({ x: 35, y: 18 })
      await fs.promises.writeFile(`/tmp/wayfarer-cerulean-rear-${stage}.png`, await game.screenshot())
      await game.saveAndReload()
      for (const y of [17, 16]) {
        await moveTo(game, "up", 35, y)
      }
      for (const y of [17, 18]) {
        await moveTo(game, "down", 35, y)
      }
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await settle(game)
      expect((await game.state.read()).map.name).toBe("cerulean-house-2")
      expect((await game.state.read()).player).toMatchObject({ x: 4, y: 2 })
      await game.controls.press("down")
      await game.wait.frames(20)
      expect((await game.state.read()).player).toMatchObject({ x: 4, y: 3 })
      await game.saveAndReload()
      expect(await game.story.flag("ceruleanBurglaryGruntDefeated")).toBe(stage !== "pending")
      expect(await game.story.flag("ceruleanBurglaryRecovered")).toBe(stage === "complete")
      expect(await game.inventory.contains("tmDig")).toBe(false)
    })
  }

  it("preserves Diglett in the imported robbed house", async () => {
    await arrange(game, {
      player: { position: { map: "cerulean-house-2", x: 2, y: 5 }, facing: "up" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await expectDialogue(game, "dug dug")
    await settle(game)
    await assertPending(game, false)
    await fs.promises.writeFile("/tmp/wayfarer-cerulean-robbed-house.png", await game.screenshot())
  })


  it("lets both front-door side tiles reach the authored center exit", async () => {
    await arrange(game)
    for (const x of [2, 3, 4]) {
      await game.player.warp("cerulean-house-2", x, 6, "down")
      await moveTo(game, "down", x, 7)
      if (x !== 3) await moveTo(game, x < 3 ? "right" : "left", 3, 7)
      for (let attempt = 0; attempt < 8; attempt++) {
        if ((await game.state.read()).map.name === "cerulean-city") break
        await game.controls.press("down")
        await game.wait.frames(30)
      }
      await game.wait.forMap("cerulean-city")
      expect((await game.state.read()).player).toMatchObject({ x: 35, y: 23 })
      await assertPending(game, false)
    }
  })

})
