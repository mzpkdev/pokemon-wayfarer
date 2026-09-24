import * as fs from "node:fs"
import { beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type PartyMonFixture, type StoryFlag } from "../harness/game-session"

const challengers = [
  { name: "Cale", x: 17, y: 20, flag: "nuggetBridgeCaleDefeated" },
  { name: "Ali", x: 19, y: 17, flag: "nuggetBridgeAliDefeated" },
  { name: "Timmy", x: 19, y: 14, flag: "nuggetBridgeTimmyDefeated" },
  { name: "Reli", x: 17, y: 13, flag: "nuggetBridgeReliDefeated" },
  { name: "Ethan", x: 17, y: 7, flag: "nuggetBridgeEthanDefeated" },
] as const
const recruiter = { name: "Recruiter", x: 19, y: 4, flag: "nuggetBridgeRecruiterDefeated" } as const
const wonFlags = (count = 5): Partial<Record<StoryFlag, boolean>> =>
  Object.fromEntries(challengers.slice(0, count).map((trainer) => [trainer.flag, true]))

const arrange = async (
  game: GameSession,
  options: Partial<Parameters<GameSession["arrange"]>[0]> = {},
): Promise<void> => game.arrange({
  checkpoint: "new-bark-after-intro",
  player: { position: { map: "route-24", x: 17, y: 20 }, facing: "up" },
  party: [{ species: "lapras", level: 100 }],
  determinism: { textSpeed: "instant" },
  ...options,
})

const approach = async (game: GameSession, trainer: { x: number; y: number }): Promise<void> =>
  game.player.warp("route-24", trainer.x, trainer.y, "up")

const settle = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active) return
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`Nugget Bridge interaction did not settle: ${JSON.stringify(await game.state.read())}`)
}

const talk = async (game: GameSession, expectedText?: string): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  if (expectedText !== undefined)
    expect((await game.state.read()).dialogue.text.toLowerCase()).toContain(expectedText.toLowerCase())
  await settle(game)
}

const startBattle = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  for (let attempt = 0; attempt < 180; attempt++) {
    if ((await game.state.read()).battle.ui === "action-menu") return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`Nugget Bridge battle did not start: ${JSON.stringify(await game.state.read())}`)
}

const win = async (game: GameSession): Promise<void> => {
  await game.battle.win()
  await settle(game)
}

const assertOrder = async (game: GameSession, count: number): Promise<void> => {
  for (const [index, trainer] of challengers.entries())
    expect(await game.story.flag(trainer.flag)).toBe(index < count)
}

const giveFixtureLeftovers = async (game: GameSession): Promise<void> => {
  await game.controls.press("start")
  await game.wait.until((state) => state.ui.mode === "pause-menu", "open pause menu")
  await game.wait.frames(60)
  await game.controls.press("down")
  await game.controls.press("a")
  await game.wait.frames(90)
  for (let pocket = 0; pocket < 3; pocket++) {
    await game.controls.press("right")
    await game.wait.frames(30)
  }
  await game.controls.press("a")
  await game.wait.frames(30)
  await game.controls.press("right")
  await game.controls.press("a")
  await game.wait.until((state) => state.partyMenu.open, "choose a Pokémon to hold Leftovers")
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
  throw new Error("Giving fixture Leftovers did not return to the bridge")
}

const walkCenter = async (game: GameSession, targetY: number): Promise<void> => {
  for (let attempt = 0; attempt < 80; attempt++) {
    const state = await game.state.read()
    if (state.map.name === "route-24" && state.player.y === targetY) return
    expect(state.battle.active).toBe(false)
    expect(state.player.x).toBe(18)
    await game.controls.press(state.player.y > targetY ? "up" : "down")
    await game.wait.frames(20)
    await settle(game)
  }
  throw new Error(`Bridge center lane blocked: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer Nugget Bridge", () => {
  let game: GameSession
  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("requires all five challengers in order and preserves their completed dialogue", async () => {
    await arrange(game, { story: {
      vars: { kantoRocketStoryState: 5 },
      flags: { route25NuggetReceived: true },
    }, bag: { items: { bigNugget: 1 } } })
    for (const trainer of [...challengers.slice(1), recruiter]) {
      await approach(game, trainer)
      await talk(game, "Cale")
      await assertOrder(game, 0)
      expect(await game.story.flag("nuggetBridgePrizeReceived")).toBe(false)
    }
    for (const [index, trainer] of challengers.entries()) {
      await approach(game, trainer)
      await fs.promises.writeFile(`/tmp/wayfarer-nugget-${trainer.name}.png`, await game.screenshot())
      await startBattle(game)
      await win(game)
      await assertOrder(game, index + 1)
      await talk(game)
      expect((await game.state.read()).battle.active).toBe(false)
      expect(await game.story.var("kantoRocketStoryState")).toBe(5)
      expect(await game.story.flag("route25NuggetReceived")).toBe(true)
      expect(await game.inventory.count("bigNugget")).toBe(1)
    }
    await game.saveAndReload()
    await assertOrder(game, 5)
    expect(await game.story.flag("nuggetBridgePrizeReceived")).toBe(false)
  })

  it("keeps a lost challenger current and later challengers unavailable", async () => {
    await arrange(game, { party: [{ species: "lapras", level: 1 }] })
    await startBattle(game)
    await game.battle.lose()
    await settle(game)
    await assertOrder(game, 0)
    await approach(game, challengers[1])
    await talk(game)
    await assertOrder(game, 0)
    await approach(game, challengers[0])
    await startBattle(game)
  })

  it("refuses every unusable party shape at an eligible challenger", async () => {
    for (const party of [
      [], [{ species: "lapras", fainted: true }], [{ species: "lapras", egg: true }],
      [{ species: "lapras", fainted: true }, { species: "lapras", egg: true }],
    ] as PartyMonFixture[][]) {
      for (const [index, trainer] of challengers.entries()) {
        await arrange(game, { party, story: { flags: wonFlags(index) } })
        await approach(game, trainer)
        await talk(game)
        await assertOrder(game, index)
        expect((await game.state.read()).battle.active).toBe(false)
      }
    }
  })

  it("permits a mixed party with one usable Pokémon", async () => {
    await arrange(game, { party: [
      { species: "lapras", fainted: true }, { species: "lapras", egg: true },
      { species: "lapras", level: 100 },
    ] })
    await startBattle(game)
  })

  it("grants the prize before refusing an unusable recruiter party and persists it", async () => {
    for (const party of [
      [], [{ species: "lapras", fainted: true }], [{ species: "lapras", egg: true }],
      [{ species: "lapras", fainted: true }, { species: "lapras", egg: true }],
    ] as PartyMonFixture[][]) {
      await arrange(game, { party, story: { flags: wonFlags() } })
      await approach(game, recruiter)
      await talk(game)
      expect(await game.inventory.count("nugget")).toBe(1)
      expect(await game.story.flag("nuggetBridgePrizeReceived")).toBe(true)
      expect(await game.story.flag("nuggetBridgeRecruiterDefeated")).toBe(false)
      expect((await game.state.read()).battle.active).toBe(false)
      await game.saveAndReload()
      await talk(game)
      expect(await game.inventory.count("nugget")).toBe(1)
      expect(await game.story.flag("route25NuggetReceived")).toBe(false)
    }
  })

  it("retries a full Items pocket after reload and freeing one slot through the Bag", async () => {
    await arrange(game, { story: { flags: wonFlags() }, bag: { fullPockets: ["items"] } })
    await approach(game, recruiter)
    await talk(game)
    expect(await game.inventory.count("nugget")).toBe(0)
    expect(await game.story.flag("nuggetBridgePrizeReceived")).toBe(false)
    expect(await game.story.flag("nuggetBridgeRecruiterDefeated")).toBe(false)
    await game.saveAndReload()
    await talk(game)
    expect(await game.story.flag("nuggetBridgePrizeReceived")).toBe(false)
    const count = await game.inventory.count("leftovers")
    await giveFixtureLeftovers(game)
    expect(await game.inventory.count("leftovers")).toBe(count - 1)
    await startBattle(game)
    expect(await game.inventory.count("nugget")).toBe(1)
    expect(await game.story.flag("nuggetBridgePrizeReceived")).toBe(true)
    await win(game)
    expect(await game.story.flag("nuggetBridgeRecruiterDefeated")).toBe(true)
    await talk(game)
    expect(await game.inventory.count("nugget")).toBe(1)
  })

  it("adds one prize to a pre-owned stack and never repeats it after a recruiter loss", async () => {
    await arrange(game, {
      story: { flags: wonFlags() },
      party: [{ species: "lapras", level: 1 }],
      bag: { items: { nugget: 1 }, fullPockets: ["items"] },
    })
    await approach(game, recruiter)
    await fs.promises.writeFile("/tmp/wayfarer-nugget-Recruiter.png", await game.screenshot())
    await startBattle(game)
    expect(await game.inventory.count("nugget")).toBe(2)
    await game.battle.lose()
    await settle(game)
    expect(await game.story.flag("nuggetBridgePrizeReceived")).toBe(true)
    expect(await game.story.flag("nuggetBridgeRecruiterDefeated")).toBe(false)
    expect(await game.story.flag("route25NuggetReceived")).toBe(false)
    expect(await game.inventory.count("bigNugget")).toBe(0)
    await approach(game, recruiter)
    await game.saveAndReload()
    await startBattle(game)
    expect(await game.inventory.count("nugget")).toBe(2)
    await win(game)
    await game.saveAndReload()
    await talk(game)
    expect(await game.inventory.count("nugget")).toBe(2)
  })

  for (const storyState of [3, 4, 5]) {
    it(`preserves the center lane and Machine Part scene from state ${storyState}`, async () => {
      await arrange(game, {
        player: { position: { map: "route-24", x: 18, y: 20 }, facing: "up" },
        story: {
          vars: { kantoRocketStoryState: storyState },
          flags: { hideCeruleanCapeRocket: storyState === 5, hiddenMachinePart: true },
        },
      })
      await walkCenter(game, 0)
      expect(await game.story.var("kantoRocketStoryState")).toBe(storyState === 3 ? 4 : storyState)
      await assertOrder(game, 0)
      expect(await game.story.flag("nuggetBridgePrizeReceived")).toBe(false)
      expect(await game.story.flag("hiddenMachinePart")).toBe(true)
      await game.controls.press("up")
      await game.wait.forMap("route-25")
      await game.player.warp("route-24", 18, 21, "down")
      for (let attempt = 0; attempt < 8; attempt++) {
        if ((await game.state.read()).map.name === "cerulean-city") break
        await game.controls.press("down")
        await game.wait.frames(20)
      }
      await game.wait.forMap("cerulean-city")
      await assertOrder(game, 0)
    })
  }
})
