import { describe, expect, it } from "webanvil/test"

import { GameSession, type GameState } from "../harness/game-session"
import { storyFlags, storyVars } from "../harness/game-session/catalog"

// The Route 119 writer awards Fly and advances Scott only after a native win.
// Keep its Emerald-bank aliases local to this focused continuation probe.
const route119ScottState = "route119-scott-state"
const receivedHmFly = "route119-received-hm-fly"
;(storyVars as Record<string, number>)[route119ScottState] = 0x70d1
;(storyFlags as Record<string, number>)[receivedHmFly] = 0x606e

const waitForDialogueText = async (
  game: GameSession,
  expected: string,
  description: string,
): Promise<GameState> => {
  const observed = new Set<string>()
  for (let attempt = 0; attempt < 180; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text) observed.add(state.dialogue.text)
    if (state.dialogue.text.includes(expected)) return state
    await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not show ${JSON.stringify(expected)}; observed ${JSON.stringify([...observed])}`,
  )
}

const advanceToDialogueText = async (
  game: GameSession,
  expected: string,
  description: string,
): Promise<GameState> => {
  const observed = new Set<string>()
  for (let attempt = 0; attempt < 480; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text) observed.add(state.dialogue.text)
    if (state.dialogue.text.includes(expected)) return state
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not show ${JSON.stringify(expected)}; observed ${JSON.stringify([...observed])}`,
  )
}

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 480; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready && !state.dialogueOpen && !state.scriptActive) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive)
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not release: ${JSON.stringify(await game.state.read())}`)
}

const waitForTrainerBattle = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.battle.active) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive)
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not enter battle: ${JSON.stringify(await game.state.read())}`)
}

const waitForRoute119TrainerBattle = async (
  game: GameSession,
  description: string,
): Promise<void> => {
  // Route 119 starts BattleMain behind May's authored pre-battle message. Clear
  // it and observe a settled action menu before forcing an outcome, so the
  // battle initializer cannot overwrite the test hook's result.
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.battle.active && state.battle.ui === "action-menu" && state.battle.enemy !== null) {
      await game.wait.frames(24)
      const settled = await game.state.read()
      if (
        settled.battle.active &&
        settled.battle.ui === "action-menu" &&
        settled.battle.enemy !== null
      ) {
        return
      }
    }
    if (
      state.battle.ui === "text" ||
      state.dialogueOpen ||
      state.scriptActive ||
      state.controlsLocked
    )
      await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`${description} did not reach a settled trainer action menu`)
}

const waitForBattleActionMenu = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.battle.ui === "text") await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(
    `trainer-only recovery action menu unavailable: ${JSON.stringify(await game.state.read())}`,
  )
}

const step = async (
  game: GameSession,
  direction: "up" | "down" | "left" | "right",
  x: number,
  y: number,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y) return
    await game.controls.press(direction)
    await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not reach ${x},${y}: ${JSON.stringify(await game.state.read())}`,
  )
}

const surfStep = async (
  game: GameSession,
  direction: "up" | "down" | "left" | "right",
  description: string,
): Promise<void> => {
  const before = await game.state.read()
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(90)
    const after = await game.state.read()
    if (
      after.map.mapGroup !== before.map.mapGroup ||
      after.map.mapNum !== before.map.mapNum ||
      after.player.x !== before.player.x ||
      after.player.y !== before.player.y
    )
      return
  }
  throw new Error(`${description} did not move: ${JSON.stringify(await game.state.read())}`)
}

const reviveRouteRival = async (game: GameSession): Promise<void> => {
  await game.battle.startWild({ species: "pidgey", level: 5 })
  await waitForBattleActionMenu(game)
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (Math.floor(cursor / 2) !== 0) await game.controls.press("up")
  if (cursor % 2 === 0) await game.controls.press("right")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag", "trainer-only recovery Bag")
  for (let pocket = 0; pocket < 5; pocket++) {
    if ((await game.state.read()).battle.bag.item === "revive") break
    await game.controls.press("right")
    await game.wait.frames(60)
  }
  expect((await game.state.read()).battle.bag.item).toBe("revive")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag-context", "Revive context")
  await game.controls.press("a")
  await game.wait.until((state) => state.ui.mode === "party-menu", "Revive target picker")
  // Party-menu allocations are live before their first rendered frame. Let the
  // real selector settle, then dismiss its item message until recovery closes
  // the trainer-only encounter.
  await game.wait.frames(60)
  await game.controls.press("a")
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready) break
    await game.controls.press("a")
    await game.wait.frames(60)
  }
  await finishFieldScript(game, "trainer-only route rival Revive recovery")
}

const arrangeDeferredRouteRival = async (
  game: GameSession,
  map: "route-110" | "route-119",
  position: { x: number; y: number },
  hoennStarterChoice: 0 | 1 | 2,
  checkpoint:
    | "new-bark-after-intro"
    | "hoenn-before-rescue"
    | "hoenn-female-before-rescue" = "new-bark-after-intro",
) => {
  await game.arrange({
    checkpoint,
    player: { facing: "down", position: { map, ...position } },
    story: {
      vars:
        map === "route-110"
          ? { route110State: 0, hoennStarterChoice }
          : { route119State: 0, hoennStarterChoice },
    },
    party: [{ species: "rattata", fainted: true }],
    bag: { items: { revive: 1 } },
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
}

const assertDeferredRivalLaneRemainsOpen = async (
  game: GameSession,
  map: "route-110" | "route-119",
  x: number,
  hoennStarterChoice: 0 | 1 | 2,
  checkpoint: "hoenn-before-rescue" | "hoenn-female-before-rescue",
  expectedGender: 0 | 1,
): Promise<void> => {
  const stateVar = map === "route-110" ? "route110State" : "route119State"
  const rivalFlag = map === "route-110" ? "hideRoute110Rival" : "hideRoute119Rival"
  const bikeFlag = map === "route-110" ? "hideRoute110RivalOnBike" : "hideRoute119RivalOnBike"
  const triggerY = map === "route-110" ? 56 : 31
  const startY = triggerY - 1
  const farY = triggerY + 1

  await game.arrange({
    checkpoint,
    player: { facing: "down", position: { map, x, y: startY } },
    story: {
      vars: { [stateVar]: 0, hoennStarterChoice },
      flags: { [rivalFlag]: false, [bikeFlag]: false },
    },
    party: [],
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
  expect((await game.state.read()).origin.gender).toBe(expectedGender)
  const before = await game.state.read()

  await step(game, "down", x, triggerY, `${map} no-party forward trigger at ${x}`)
  await game.wait.forReady()
  expect(await game.state.read()).toMatchObject({
    map: before.map,
    player: { x, y: triggerY },
    battle: { active: false },
    dialogue: { sequence: before.dialogue.sequence },
  })
  expect(await game.story.var(stateVar)).toBe(0)
  expect(await game.story.flag(rivalFlag)).toBe(false)
  expect(await game.story.flag(bikeFlag)).toBe(false)

  // The public route stays traversable in both directions while the pending
  // scene is deferred. Crossing the trigger again cannot write its chapter.
  await step(game, "down", x, farY, `${map} no-party forward lane at ${x}`)
  await step(game, "up", x, triggerY, `${map} no-party return trigger at ${x}`)
  await game.wait.forReady()
  await step(game, "up", x, startY, `${map} no-party return lane at ${x}`)
  expect(await game.state.read()).toMatchObject({
    player: { x, y: startY },
    battle: { active: false },
  })
  expect(await game.story.var(stateVar)).toBe(0)
  expect(await game.story.flag(rivalFlag)).toBe(false)
  expect(await game.story.flag(bikeFlag)).toBe(false)
}

describe.sequential("Wayfarer Hoenn trainer-only story encounters", () => {
  const rivalGenders = [
    { checkpoint: "hoenn-before-rescue" as const, expectedGender: 0 as const, rival: "May" },
    {
      checkpoint: "hoenn-female-before-rescue" as const,
      expectedGender: 1 as const,
      rival: "Brendan",
    },
  ]

  // Each physical entry path and player gender is exercised. The encounter is
  // deferred before the starter-specific branch, so starters are distributed
  // across those entries instead of forming a redundant Cartesian product.
  const deferredRivalApproaches = [
    { map: "route-110" as const, x: 33, hoennStarterChoice: 0 as const, ...rivalGenders[0]! },
    { map: "route-110" as const, x: 33, hoennStarterChoice: 1 as const, ...rivalGenders[1]! },
    { map: "route-110" as const, x: 34, hoennStarterChoice: 2 as const, ...rivalGenders[0]! },
    { map: "route-110" as const, x: 34, hoennStarterChoice: 0 as const, ...rivalGenders[1]! },
    { map: "route-110" as const, x: 35, hoennStarterChoice: 1 as const, ...rivalGenders[0]! },
    { map: "route-110" as const, x: 35, hoennStarterChoice: 2 as const, ...rivalGenders[1]! },
    { map: "route-119" as const, x: 25, hoennStarterChoice: 0 as const, ...rivalGenders[0]! },
    { map: "route-119" as const, x: 25, hoennStarterChoice: 1 as const, ...rivalGenders[1]! },
    { map: "route-119" as const, x: 26, hoennStarterChoice: 2 as const, ...rivalGenders[0]! },
    { map: "route-119" as const, x: 26, hoennStarterChoice: 0 as const, ...rivalGenders[1]! },
  ]

  for (const approach of deferredRivalApproaches)
    it(`keeps ${approach.map} approach ${approach.x} open in both directions for ${approach.rival}, starter ${approach.hoennStarterChoice}, before starter dispatch`, async () => {
      const game = await GameSession.launch()
      try {
        await assertDeferredRivalLaneRemainsOpen(
          game,
          approach.map,
          approach.x,
          approach.hoennStarterChoice,
          approach.checkpoint,
          approach.expectedGender,
        )
      } finally {
        await game.close()
      }
    })

  it("keeps Route 110 open without a party, then restores a recovered starter variant after re-entry", async () => {
    const game = await GameSession.launch()
    try {
      const hoennStarterChoice = 1
      await arrangeDeferredRouteRival(game, "route-110", { x: 33, y: 55 }, hoennStarterChoice)
      const before = await game.state.read()
      await step(game, "down", 33, 56, "Route 110 no-party crossing")
      await game.wait.forReady()
      expect(await game.state.read()).toMatchObject({
        map: before.map,
        player: { x: 33, y: 56 },
        battle: { active: false },
        dialogue: { sequence: before.dialogue.sequence },
      })
      expect(await game.story.var("route110State")).toBe(0)

      await reviveRouteRival(game)
      expect(await game.state.read()).toMatchObject({
        party: [{ species: "rattata", fainted: false, egg: false }],
      })
      await step(game, "up", 33, 55, "Route 110 clear trigger")
      await step(game, "up", 33, 54, "Route 110 leave trigger")
      await step(game, "down", 33, 55, "Route 110 re-enter approach")
      expect(await game.story.var("route110State")).toBe(0)
      await step(game, "down", 33, 56, "Route 110 recovered trigger")
      await waitForTrainerBattle(game, `recovered Route 110 rival starter ${hoennStarterChoice}`)
    } finally {
      await game.close()
    }
  })

  it("uses forced outcomes to verify Route 119's safe loss retreat and native Fly/Scott victory continuation", async () => {
    const game = await GameSession.launch()
    try {
      const hoennStarterChoice = 2
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: "route-119", x: 25, y: 30 } },
        story: {
          vars: { route119State: 0, hoennStarterChoice, [route119ScottState]: 0 } as never,
          flags: {
            hideRoute119Rival: false,
            hideRoute119RivalOnBike: false,
            [receivedHmFly]: false,
          } as never,
        },
        party: [{ species: "rattata", fainted: true }],
        bag: { items: { revive: 2 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const before = await game.state.read()
      await step(game, "down", 25, 31, "Route 119 no-party crossing")
      await game.wait.forReady()
      expect(await game.state.read()).toMatchObject({
        map: before.map,
        player: { x: 25, y: 31 },
        battle: { active: false },
        dialogue: { sequence: before.dialogue.sequence },
      })
      expect(await game.story.var("route119State")).toBe(0)

      await reviveRouteRival(game)
      await step(game, "up", 25, 30, "Route 119 clear trigger")
      await step(game, "up", 25, 29, "Route 119 leave trigger")
      await step(game, "down", 25, 30, "Route 119 re-enter approach")
      await step(game, "down", 25, 31, "Route 119 recovered trigger")
      await waitForRoute119TrainerBattle(
        game,
        `recovered Route 119 rival starter ${hoennStarterChoice}`,
      )

      // A forced loss isolates this caller's retreat continuation; the actual
      // no-party and medicine mechanics are covered by the real setup above.
      await game.battle.lose()
      await game.controls.press("a")
      await finishFieldScript(game, "Route 119 loss retreat")
      expect((await game.state.read()).map.name).toBe("route-119")
      expect(await game.story.var("route119State")).toBe(0)
      expect(await game.story.var(route119ScottState as never)).toBe(0)
      expect(await game.story.flag(receivedHmFly as never)).toBe(false)
      expect((await game.state.read()).bag.hms.fly).toBe(0)

      await reviveRouteRival(game)
      await step(game, "up", 25, 30, "Route 119 leave loss retreat")
      await step(game, "up", 25, 29, "Route 119 clear loss retreat")
      await step(game, "down", 25, 30, "Route 119 re-enter after loss")
      await step(game, "down", 25, 31, "Route 119 retry after loss")
      await waitForRoute119TrainerBattle(game, "Route 119 recovered retry")
      // The forced win reaches the actual native writer, including Fly and
      // Scott, without duplicating a full combat-input driver.
      await game.battle.win()
      await game.controls.press("a")
      await finishFieldScript(game, "Route 119 native Fly and Scott continuation")
      expect(await game.story.var("route119State")).toBe(1)
      expect(await game.story.var(route119ScottState as never)).toBe(1)
      expect(await game.story.flag(receivedHmFly as never)).toBe(true)
    } finally {
      await game.close()
    }
  })

  it("refuses the Space Center stair guard before its battle, guard movement, or completion write", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "mossdeep-space-center-1f", x: 13, y: 3 } },
        story: {
          vars: { mossdeepCityState: 2, spaceCenterStairGuardState: 0 },
          flags: {
            hideMossdeepSpaceCenter1fTeamMagma: false,
            defeatedMossdeepSpaceCenter1fGrunt: false,
          },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.player.interact()
      const refusal = await waitForDialogueText(
        game,
        "Stay out of our way",
        "Space Center stair guard refusal",
      )
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Space Center stair guard refusal")
      expect(await game.story.var("mossdeepCityState")).toBe(2)
      expect(await game.story.var("spaceCenterStairGuardState")).toBe(0)
      expect(await game.story.flag("defeatedMossdeepSpaceCenter1fGrunt")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("refuses the Route 103 tutorial before its starter battle staging", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "right", position: { map: "route-103", x: 9, y: 3 } },
        story: {
          vars: { hoennStarterChoice: 0 },
          flags: { hideRoute103Rival: false, defeatedRivalRoute103: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.player.interact()
      const refusal = await waitForDialogueText(
        game,
        "Come back when",
        "Route 103 tutorial refusal",
      )
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Route 103 tutorial refusal")
      expect(await game.story.flag("defeatedRivalRoute103")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("refuses the Petalburg Wally tutorial before party substitution", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: "petalburg-city", x: 15, y: 9 } },
        story: {
          // Arrange must settle without the on-frame tutorial claiming control.
          // Set the authored tutorial state only after the harness releases it.
          vars: { petalburgCityState: 3 },
          flags: { hidePetalburgCityWally: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.story.setVar("petalburgCityState", 2)
      const refusal = await waitForDialogueText(
        game,
        "Come back when",
        "Petalburg tutorial refusal",
      )
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Petalburg tutorial refusal")
      expect(await game.story.var("petalburgCityState")).toBe(2)
    } finally {
      await game.close()
    }
  })

  it("refuses Captain Stern before the Museum confrontation spawns", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "slateport-oceanic-museum-2f", x: 13, y: 7 } },
        story: {
          vars: { slateportMuseum1fState: 0 },
          flags: { hideOceanicMuseum2fCaptainStern: false, deliveredDevonGoods: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.player.interact()
      const refusal = await waitForDialogueText(game, "bring a", "Museum Stern refusal")
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Museum Stern refusal")
      expect(await game.story.flag("deliveredDevonGoods")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("refuses Matt before his notice trigger locks or stages the submarine confrontation", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "aqua-hideout-b2f", x: 28, y: 18 } },
        story: {
          flags: { hideAquaHideoutGrunts: false, teamAquaEscapedSubmarine: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await step(game, "up", 28, 17, "Matt no-party notice")
      const refusal = await waitForDialogueText(game, "over your head", "Matt notice refusal")
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Matt notice refusal")
      expect(await game.story.flag("teamAquaEscapedSubmarine")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("keeps the Route 120 Kecleon objective safe without granting the Devon Scope", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "route-120", x: 13, y: 16 } },
        story: {
          flags: { hideRoute120Steven: false, receivedDevonScope: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.player.interact()
      const refusal = await waitForDialogueText(game, "You need a POK", "Route 120 refusal")
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Route 120 refusal")
      expect(await game.story.flag("receivedDevonScope")).toBe(false)
      expect((await game.state.read()).battle.trainerOnly.active).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("silently suppresses Lilycove and Mauville's pending rivals without completing either chapter", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "lilycove-city", x: 27, y: 8 } },
        story: {
          flags: { hideLilycoveCityRival: false, metRivalLilycove: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const lilycove = await game.state.read()
      await step(game, "up", 27, 7, "Lilycove no-party approach")
      await game.wait.forReady()
      expect(await game.state.read()).toMatchObject({
        player: { x: 27, y: 7 },
        battle: { active: false },
        dialogue: { sequence: lilycove.dialogue.sequence },
      })
      expect(await game.story.flag("metRivalLilycove")).toBe(false)

      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "mauville-city", x: 8, y: 7 } },
        story: {
          flags: { hideMauvilleCityWally: false, defeatedWallyMauville: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const mauville = await game.state.read()
      await step(game, "up", 8, 6, "Mauville no-party approach")
      await game.wait.forReady()
      expect(await game.state.read()).toMatchObject({
        player: { x: 8, y: 6 },
        battle: { active: false },
        dialogue: { sequence: mauville.dialogue.sequence },
      })
      expect(await game.story.flag("defeatedWallyMauville")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("restores Lilycove's rival after medicine recovery without writing the chapter before battle", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "lilycove-city", x: 27, y: 8 } },
        story: {
          vars: { hoennStarterChoice: 0 },
          flags: { hideLilycoveCityRival: false, metRivalLilycove: false },
        },
        party: [{ species: "rattata", fainted: true }],
        bag: { items: { revive: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await step(game, "up", 27, 7, "Lilycove no-party approach")
      await game.wait.forReady()
      await reviveRouteRival(game)
      await step(game, "down", 27, 8, "Lilycove clear restored rival tile")
      // The recovered Rattata follows into the actor template tile. Move one
      // further tile before returning so A targets the restored native rival.
      await step(game, "down", 27, 9, "Lilycove clear follower from rival tile")
      await step(game, "up", 27, 8, "Lilycove return beside restored rival")
      expect(await game.story.flag("metRivalLilycove")).toBe(false)
      await game.controls.press("up")
      await game.player.interact()
      const intro = await waitForDialogueText(
        game,
        "MAY Oh, hey",
        "restored Lilycove rival interaction",
      )
      expect(intro.battle.active).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("restores Mauville Wally after medicine recovery without writing the chapter before battle", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "mauville-city", x: 8, y: 7 } },
        story: { flags: { hideMauvilleCityWally: false, defeatedWallyMauville: false } },
        party: [{ species: "rattata", fainted: true }],
        bag: { items: { revive: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await step(game, "up", 8, 6, "Mauville no-party approach")
      await game.wait.forReady()
      await reviveRouteRival(game)
      await step(game, "down", 8, 7, "Mauville clear restored Wally tile")
      await step(game, "down", 8, 8, "Mauville clear follower from Wally tile")
      await step(game, "up", 8, 7, "Mauville return beside restored Wally")
      expect(await game.story.flag("defeatedWallyMauville")).toBe(false)
      await game.controls.press("up")
      await game.player.interact()
      const intro = await waitForDialogueText(
        game,
        "WALLY Aww, UNCLE",
        "restored Mauville Wally interaction",
      )
      expect(intro.battle.active).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("keeps every Rustboro rival approach open without advancing its pending battle chapter", async () => {
    const game = await GameSession.launch()
    try {
      for (const x of [12, 13, 14, 15, 16, 17, 18, 19]) {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: { facing: "down", position: { map: "rustboro-city", x, y: 52 } },
          story: {
            vars: { rustboroCityState: 7, hoennStarterChoice: 0 },
            flags: { hideRustboroCityRival: false, defeatedRivalRustboro: false },
          },
          party: [],
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        const before = await game.state.read()
        await step(game, "down", x, 53, `Rustboro no-party approach ${x}`)
        await game.wait.forReady()
        expect(await game.state.read()).toMatchObject({
          player: { x, y: 53 },
          battle: { active: false },
          dialogue: { sequence: before.dialogue.sequence },
        })
        expect(await game.story.var("rustboroCityState")).toBe(7)
        expect(await game.story.flag("defeatedRivalRustboro")).toBe(false)
      }
    } finally {
      await game.close()
    }
  })

  it("keeps both Victory Road Wally entrances silent without changing League state", async () => {
    const game = await GameSession.launch()
    try {
      for (const x of [2, 3]) {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: { facing: "down", position: { map: "victory-road-1f", x, y: 22 } },
          story: {
            vars: { victoryRoad1fState: 0, hoennStarterChoice: 0 },
            flags: { defeatedWallyVictoryRoad: false, hideVictoryRoadEntranceWally: true },
          },
          party: [],
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        const before = await game.state.read()
        await step(game, "down", x, 23, `Victory Road no-party entrance ${x}`)
        await game.wait.forReady()
        expect(await game.state.read()).toMatchObject({
          player: { x, y: 23 },
          battle: { active: false },
          dialogue: { sequence: before.dialogue.sequence },
        })
        expect(await game.story.var("victoryRoad1fState")).toBe(0)
        expect(await game.story.flag("defeatedWallyVictoryRoad")).toBe(false)
      }
    } finally {
      await game.close()
    }
  })

  it("keeps Rustboro's eligible MatchCall conversation while refusing the no-party battle", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "up", position: { map: "rustboro-city", x: 16, y: 51 } },
        story: {
          vars: { rustboroCityState: 7, hoennStarterChoice: 0 },
          flags: { hideRustboroCityRival: false, defeatedRivalRustboro: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.player.interact()
      const refusal = await advanceToDialogueText(
        game,
        "Well battle when",
        "Rustboro MatchCall battle refusal",
      )
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Rustboro MatchCall conversation and refusal")
      expect(await game.story.var("rustboroCityState")).toBe(8)
      expect(await game.story.flag("metRivalRustboro")).toBe(true)
      expect(await game.story.flag("enableRivalMatchCall")).toBe(true)
      expect(await game.story.flag("defeatedRivalRustboro")).toBe(false)
      expect((await game.state.read()).battle.active).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("hands over Go Goggles to both player genders with no usable party", async () => {
    for (const checkpoint of ["hoenn-before-rescue", "hoenn-female-before-rescue"] as const) {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint,
          player: { facing: "up", position: { map: "lavaridge-town", x: 9, y: 9 } },
          story: {
            vars: { lavaridgeTownState: 0 },
            flags: {
              defeatedEvilTeamMtChimney: true,
              receivedGoGoggles: false,
              hideLavaridgeTownRival: false,
              hideLavaridgeTownRivalOnBike: true,
            },
          },
          party: [],
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        await game.story.setVar("lavaridgeTownState", 1)
        await game.wait.until(
          (state) => state.scriptActive || state.dialogueOpen,
          `Go Goggles handoff starts from ${checkpoint}`,
        )
        await finishFieldScript(game, `Go Goggles no-party handoff from ${checkpoint}`)
        expect(await game.story.var("lavaridgeTownState")).toBe(2)
        expect(await game.story.flag("receivedGoGoggles")).toBe(true)
        expect(await game.state.read()).toMatchObject({ party: [], battle: { active: false } })
      } finally {
        await game.close()
      }
    }
  })

  it("keeps Lilycove's east sea outlet open before Matt's submarine completion in both directions", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "right", position: { map: "lilycove-city", x: 69, y: 22 } },
        story: { flags: { teamAquaEscapedSubmarine: false, disableEncounters: true } },
        party: [{ species: "lapras", moves: ["surf"] }],
        bag: { hms: { surf: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.player.interact()
      await game.dialogue.waitForOpen()
      expect(await game.state.read()).toMatchObject({
        fieldMove: { move: "surf", user: 0, userSpecies: "lapras", result: "found" },
      })
      await game.wait.until(
        (state) => state.dialogue.message === "want-to-use-surf" && !state.dialogueOpen,
        "Lilycove east outlet Surf prompt",
      )
      await game.wait.frames(12)
      await game.controls.press("a")
      await game.wait.until(
        (state) => state.dialogue.message === "player-used-surf",
        "Lilycove east outlet Surf confirmation",
      )
      await game.dialogue.waitForClosed()
      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.until(
        (state) => state.player.surfing,
        "Lilycove east outlet Surf start",
        3_600,
      )

      // The beach feeds a real water corridor north-east to the Route 124
      // connection; the direct east shoreline is a rock wall, not a story
      // gate. These are the native water tiles from (70,22) to (79,17).
      const eastSeaPath = [
        "right",
        "right",
        "right",
        "right",
        "right",
        "up",
        "up",
        "up",
        "up",
        "right",
        "right",
        "up",
        "right",
        "right",
      ] as const
      for (const direction of eastSeaPath) {
        await surfStep(game, direction, `Lilycove east sea ${direction}`)
      }
      for (
        let attempt = 0;
        attempt < 3 && (await game.state.read()).map.name !== "route-124";
        attempt++
      ) {
        await game.player.move("right")
        await game.wait.frames(90)
      }
      expect(await game.state.read()).toMatchObject({
        map: { name: "route-124" },
        player: { surfing: true },
      })
      expect(await game.story.flag("teamAquaEscapedSubmarine")).toBe(false)

      for (
        let attempt = 0;
        attempt < 3 && (await game.state.read()).map.name !== "lilycove-city";
        attempt++
      ) {
        await game.player.move("left")
        await game.wait.frames(90)
      }
      expect((await game.state.read()).map.name).toBe("lilycove-city")
      for (const direction of [...eastSeaPath].reverse().map(
        (direction) =>
          ({
            right: "left",
            left: "right",
            up: "down",
            down: "up",
          })[direction] as "up" | "down" | "left" | "right",
      )) {
        await surfStep(game, direction, `Lilycove west sea ${direction}`)
      }
      expect(await game.state.read()).toMatchObject({
        map: { name: "lilycove-city" },
        player: { surfing: true },
      })
      expect(await game.story.flag("teamAquaEscapedSubmarine")).toBe(false)
    } finally {
      await game.close()
    }
  })
})
