import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction, type GameMap, type StoryVar } from "../harness/game-session"
import { catchWithMasterBallAndSwap } from "../playbooks/battle-catch-swap"

type LaneCase = {
  map: GameMap
  start: { x: number; y: number }
  direction: "up" | "down" | "right" | "left"
  end: { x: number; y: number }
  variable: StoryVar
  state: number
}

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

const arrangeAtSudowoodo = async (
  game: GameSession,
  options: { prepareCatch?: boolean } = {},
): Promise<void> => {
  const party = options.prepareCatch
    ? [
        { species: "lapras" as const, level: 100, moves: ["surf" as const] },
        ...Array.from({ length: 5 }, () => ({ species: "rattata" as const, level: 5 })),
      ]
    : [{ species: "lapras" as const, level: 100, moves: ["surf" as const] }]
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "up", position: { map: "route-36", x: 37, y: 18 } },
    story: { vars: { starterMon: 0 }, flags: { hideSudowoodo: false } },
    party,
    bag: { items: { squirtBottle: 1, ...(options.prepareCatch ? { masterBall: 1 } : {}) } },
    pc: options.prepareCatch
      ? { currentBox: 0, observedSlots: [{ box: 0, slot: 0, mon: null }] }
      : undefined,
    determinism: { rngSeed: 1, textSpeed: "instant" },
  })
}

const openSudowoodoPrompt = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.wait.frames(30)
  await game.controls.press("a")
  await game.wait.frames(30)
}

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

const finishScriptedBattle = async (game: GameSession, description: string): Promise<void> => {
  let handledFaintedPartyCount = 0
  for (let attempt = 0; attempt < 1_400; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready) return
    const faintedPartyCount = state.party.filter((mon) => mon.fainted).length
    if (state.battle.ui === "other" && faintedPartyCount > handledFaintedPartyCount) {
      await game.wait.frames(120)
      await game.controls.press("down")
      await game.wait.frames(30)
      await game.controls.press("a")
      handledFaintedPartyCount = faintedPartyCount
    } else if (state.battle.ui === "action-menu" || state.battle.ui === "other")
      await game.controls.press("a")
    else if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive) {
      await game.wait.frames(30)
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(`${description} did not finish: ${JSON.stringify(await game.state.read())}`)
}

const waitForBattleAction = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.battle.ui === "text") await game.controls.press("a")
    else await game.wait.frames(15)
  }
  throw new Error(`${description} was not reached: ${JSON.stringify(await game.state.read())}`)
}

const runFromBattle = async (game: GameSession): Promise<void> => {
  await waitForBattleAction(game, "Sudowoodo action menu")
  await game.controls.press("down")
  await game.wait.frames(8)
  await game.controls.press("right")
  await game.wait.frames(8)
  await game.controls.press("a")

  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active) {
      await finishFieldScript(game, "Sudowoodo run-away script")
      return
    }
    if (state.battle.ui === "text") await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(
    `Sudowoodo battle could not be escaped: ${JSON.stringify(await game.state.read())}`,
  )
}

const waitForDialogueText = async (game: GameSession, expectedText: string): Promise<void> => {
  const observed = new Set<string>()
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text) observed.add(state.dialogue.text)
    if (state.dialogue.text.includes(expectedText)) return
    await game.wait.frames(30)
    await game.controls.press("a")
    await game.wait.frames(8)
  }
  throw new Error(
    `Dialogue did not reach ${JSON.stringify(expectedText)}; observed ${JSON.stringify([...observed])}`,
  )
}

const walkStep = async (
  game: GameSession,
  from: { x: number; y: number },
  step: { direction: Direction; x: number; y: number },
): Promise<void> => {
  await game.wait.frames(12)
  await game.player.move(step.direction)
  await game.wait.until(
    (state) => state.ready && state.player.facing === step.direction,
    `face ${step.direction} from ${from.x}:${from.y}`,
  )
  const afterFirstInput = await game.state.read()
  if (afterFirstInput.player.x !== step.x || afterFirstInput.player.y !== step.y)
    await game.player.move(step.direction)
  await game.wait.until(
    (state) => state.ready && state.player.x === step.x && state.player.y === step.y,
    `walk from ${from.x}:${from.y} to ${step.x}:${step.y}`,
  )
}

const walkPath = async (
  game: GameSession,
  start: { x: number; y: number },
  steps: readonly { direction: Direction; x: number; y: number }[],
): Promise<void> => {
  let current = start
  for (const step of steps) {
    await walkStep(game, current, step)
    current = step
  }
}

const openLanes: LaneCase[] = [
  ...[2, 4].map((state) => ({
    map: "new-bark-town" as const,
    start: { x: 1, y: 13 },
    direction: "left" as const,
    end: { x: 0, y: 13 },
    variable: "newBarkTownState" as const,
    state,
  })),
  {
    map: "cherrygrove-city",
    start: { x: 55, y: 9 },
    direction: "right",
    end: { x: 56, y: 9 },
    variable: "cherrygroveCityState",
    state: 3,
  },
  {
    map: "azalea-town",
    start: { x: 10, y: 17 },
    direction: "right",
    end: { x: 11, y: 17 },
    variable: "azaleaTownState",
    state: 5,
  },
]

describe.sequential("HNS Johto traversal bypasses", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  for (const lane of openLanes) {
    it(`crosses ${lane.map} at local state ${lane.state} without advancing its story`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: lane.direction,
          position: { map: lane.map, ...lane.start },
        },
        story: { vars: { [lane.variable]: lane.state } },
      })

      await game.player.move(lane.direction)

      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: lane.map },
        player: lane.end,
        ready: true,
      })
      await expect(game.story.var(lane.variable)).resolves.toBe(lane.state)
    })
  }

  for (const state of [1, 2, 4]) {
    for (const x of [27, 28, 29]) {
      for (const crossing of [
        { startY: 11, direction: "up" as const },
        { startY: 9, direction: "down" as const },
      ]) {
        it(`crosses Route 32 lane x=${x} ${crossing.direction} in state ${state}`, async () => {
          await game.arrange({
            checkpoint: "new-bark-after-intro",
            player: {
              facing: crossing.direction,
              position: { map: "route-32", x, y: crossing.startY },
            },
            story: { vars: { violetCityState: state } },
          })

          await game.player.move(crossing.direction)

          await expect(game.state.read()).resolves.toMatchObject({
            map: { name: "route-32" },
            player: { x, y: 10 },
            ready: true,
          })
          await expect(game.story.var("violetCityState")).resolves.toBe(state)
        })
      }
    }
  }

  for (const y of [10, 11]) {
    it(`keeps Cherrygrove Silver available on retained row y=${y}`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "right", position: { map: "cherrygrove-city", x: 55, y } },
        story: {
          vars: { cherrygroveCityState: 3, starterMon: 0 },
          flags: { hideSilverCherrygrove: false },
        },
        party: [{ species: "rattata", level: 1, moves: ["tackle"] }],
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })

      await game.player.move("right")
      await game.dialogue.waitForOpen()
      await expect(game.story.var("cherrygroveCityState")).resolves.toBe(3)
      await expect(game.story.flag("hideSilverCherrygrove")).resolves.toBe(false)

      await startScriptedBattle(game, "Cherrygrove Silver battle")
      await finishScriptedBattle(game, "Cherrygrove Silver battle")
      await expect(game.story.var("cherrygroveCityState")).resolves.toBe(3)
      await expect(game.story.flag("hideSilverCherrygrove")).resolves.toBe(false)
    })
  }

  for (const crossing of [
    {
      direction: "up" as const,
      start: { x: 33, y: 26 },
      steps: [
        { direction: "up" as const, x: 33, y: 25 },
        { direction: "up" as const, x: 33, y: 24 },
        { direction: "up" as const, x: 33, y: 23 },
        { direction: "up" as const, x: 33, y: 22 },
      ],
    },
    {
      direction: "down" as const,
      start: { x: 33, y: 22 },
      steps: [
        { direction: "down" as const, x: 33, y: 23 },
        { direction: "down" as const, x: 33, y: 24 },
        { direction: "down" as const, x: 33, y: 25 },
        { direction: "down" as const, x: 33, y: 26 },
      ],
    },
  ]) {
    it(`walks Route 30 ${crossing.direction} past the staged battle before Mom`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: crossing.direction,
          position: { map: "route-30", ...crossing.start },
        },
        story: {
          vars: { newBarkTownState: 2, cherrygroveCityState: 1 },
          flags: { momVisited: false },
        },
      })

      await walkPath(game, crossing.start, crossing.steps)
      const destination = crossing.steps.at(-1)!

      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "route-30" },
        player: { x: destination.x, y: destination.y },
        ready: true,
      })
      await expect(game.story.var("newBarkTownState")).resolves.toBe(2)
      await expect(game.story.var("cherrygroveCityState")).resolves.toBe(1)
      await expect(game.story.flag("momVisited")).resolves.toBe(false)
    })
  }

  it("keeps Azalea Silver available on the retained upper row", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "right", position: { map: "azalea-town", x: 10, y: 16 } },
      story: {
        vars: { azaleaTownState: 5, starterMon: 0 },
        flags: { hideAzaleaSilver: true },
      },
      party: [{ species: "rattata", level: 1, moves: ["tackle"] }],
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    await game.player.move("right")
    await game.dialogue.waitForOpen()
    await expect(game.story.var("azaleaTownState")).resolves.toBe(5)
    await expect(game.story.flag("hideAzaleaSilver")).resolves.toBe(false)

    await startScriptedBattle(game, "Azalea Silver battle")
    await finishScriptedBattle(game, "Azalea Silver battle")
    await expect(game.story.var("azaleaTownState")).resolves.toBe(5)
    await expect(game.story.flag("hideAzaleaSilver")).resolves.toBe(false)
  })

  for (const prerequisite of [
    {
      name: "Sprout Tower",
      flags: {
        hideSproutTowerSilver: false,
        defeatedVioletGym: false,
        receivedTogepiEgg: false,
      },
      text: "SPROUT TOWER",
    },
    {
      name: "Violet Gym",
      flags: {
        hideSproutTowerSilver: true,
        defeatedVioletGym: false,
        receivedTogepiEgg: false,
      },
      text: "POKMON GY",
    },
    {
      name: "Togepi Egg",
      flags: {
        hideSproutTowerSilver: true,
        defeatedVioletGym: true,
        receivedTogepiEgg: false,
      },
      text: "Some guy wearing",
    },
  ] as const) {
    it(`checks the Route 32 guard's ${prerequisite.name} prerequisite in order`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "right", position: { map: "route-32", x: 25, y: 10 } },
        story: { vars: { violetCityState: 4 }, flags: prerequisite.flags },
        determinism: { textSpeed: "instant" },
      })

      await game.player.interact()
      await waitForDialogueText(game, prerequisite.text)
      await expect(game.story.var("violetCityState")).resolves.toBe(4)
      await expect(game.inventory.contains("miracleSeed")).resolves.toBe(false)
      await expect(game.state.read()).resolves.toMatchObject({ player: { x: 25, y: 10 } })
      await finishFieldScript(game, `Route 32 ${prerequisite.name} prerequisite script`)
    })
  }

  it("keeps the Route 32 guard reward pending when Items are full, then grants it once", async () => {
    const completedFlags = {
      hideSproutTowerSilver: true,
      defeatedVioletGym: true,
      receivedTogepiEgg: true,
    } as const
    const arrangeAtGuard = (state: number, full: boolean, miracleSeed = 0) =>
      game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "right" as const, position: { map: "route-32" as const, x: 25, y: 10 } },
        story: { vars: { violetCityState: state }, flags: completedFlags },
        bag: {
          items: miracleSeed ? { miracleSeed } : undefined,
          fullPockets: full ? (["items"] as const) : undefined,
        },
        determinism: { textSpeed: "instant" as const },
      })

    await arrangeAtGuard(4, true)
    await game.player.interact()
    await finishFieldScript(game, "Route 32 full-pocket reward attempt")
    await expect(game.story.var("violetCityState")).resolves.toBe(4)
    await expect(game.inventory.contains("miracleSeed")).resolves.toBe(false)
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 25, y: 10 } })

    await arrangeAtGuard(4, false)
    await game.player.interact()
    await finishFieldScript(game, "Route 32 reward retry")
    await expect(game.story.var("violetCityState")).resolves.toBe(5)
    await expect(game.inventory.contains("miracleSeed")).resolves.toBe(true)
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 25, y: 10 } })

    await arrangeAtGuard(5, false, 1)
    await game.player.interact()
    await finishFieldScript(game, "Route 32 completed reward interaction")
    await expect(game.state.read()).resolves.toMatchObject({ bag: { items: { miracleSeed: 1 } } })
  })

  it("walks through the removed Ilex Forest Cut-tree choke", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "ilex-forest", x: 32, y: 41 },
      },
    })

    await game.player.move("up")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "ilex-forest" },
      player: { x: 32, y: 40 },
      ready: true,
    })
  })

  it("crosses Route 36's old Sudowoodo junction while the encounter remains active", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "right",
        position: { map: "route-36", x: 38, y: 19 },
      },
      story: { flags: { hideSudowoodo: false } },
    })

    await game.player.move("right")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "route-36" },
      player: { x: 39, y: 19 },
      ready: true,
    })
    await expect(game.story.flag("hideSudowoodo")).resolves.toBe(false)
  })

  it("keeps Sudowoodo retryable after declining the SquirtBottle prompt", async () => {
    await arrangeAtSudowoodo(game)
    await openSudowoodoPrompt(game)
    await game.controls.press("b")
    await finishFieldScript(game, "Sudowoodo decline script")

    await expect(game.story.flag("hideSudowoodo")).resolves.toBe(false)
    await openSudowoodoPrompt(game)
    await game.controls.press("b")
    await finishFieldScript(game, "Sudowoodo retry decline script")
  })

  it("keeps Sudowoodo retryable after running from the encounter", async () => {
    await arrangeAtSudowoodo(game)
    await openSudowoodoPrompt(game)
    await game.controls.press("a")
    await startScriptedBattle(game, "Sudowoodo encounter")
    await runFromBattle(game)

    await expect(game.story.flag("hideSudowoodo")).resolves.toBe(false)
    await openSudowoodoPrompt(game)
    await game.controls.press("b")
    await finishFieldScript(game, "Sudowoodo post-run retry script")
  })

  it("removes Sudowoodo only after winning the encounter", async () => {
    await arrangeAtSudowoodo(game)
    await openSudowoodoPrompt(game)
    await game.controls.press("a")
    await startScriptedBattle(game, "Sudowoodo encounter")
    await finishScriptedBattle(game, "Sudowoodo encounter")

    await expect(game.story.flag("hideSudowoodo")).resolves.toBe(true)
    await game.player.move("up")
    await expect(game.state.read()).resolves.toMatchObject({
      player: { x: 37, y: 17 },
      ready: true,
    })
  })

  it("removes Sudowoodo after catching it", async () => {
    await arrangeAtSudowoodo(game, { prepareCatch: true })
    await openSudowoodoPrompt(game)
    await game.controls.press("a")
    await startScriptedBattle(game, "Sudowoodo encounter")
    await waitForBattleAction(game, "Sudowoodo action menu")
    await catchWithMasterBallAndSwap(game, { outgoingPartyIndex: 5 })

    await expect(game.story.flag("hideSudowoodo")).resolves.toBe(true)
    await game.player.move("up")
    await expect(game.state.read()).resolves.toMatchObject({
      player: { x: 37, y: 17 },
      ready: true,
    })
  })
})
