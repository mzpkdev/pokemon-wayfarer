import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"

import { GameSession, type GameState, type PartyMonFixture } from "../harness/game-session"

// Field-message telemetry is ABI-bounded to 32 encoded bytes. This is the exact
// visible prefix of the neutral stable assignment; the repository mechanics test
// separately owns the complete text resource.
const ordinaryRefusal = "Come back when you have a\nPOKM"
// The same bounded telemetry drops apostrophes. The complete resource remains
// asserted by the mechanics suite and is captured in the runtime evidence.
const retreatText = "You cant keep battling"

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

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 480; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready && !state.dialogueOpen && !state.scriptActive) return
    if (
      state.ui.mode === "party-menu" ||
      state.battle.ui === "text" ||
      state.dialogueOpen ||
      state.scriptActive
    )
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

const chooseFightMove = async (game: GameSession, description: string): Promise<void> => {
  await waitForBattleActionMenu(game)
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (cursor % 2 !== 0) await game.controls.press("left")
  if (Math.floor(cursor / 2) !== 0) await game.controls.press("up")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "move-menu", `${description} move selection`)
  await game.controls.press("a")
}

const selectTrainerOnlyAction = async (game: GameSession, target: number): Promise<void> => {
  await waitForBattleActionMenu(game)
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (cursor % 2 !== target % 2) await game.controls.press("right")
  if (Math.floor(cursor / 2) !== Math.floor(target / 2)) await game.controls.press("down")
  await game.controls.press("a")
}

const useTrainerOnlyBagItem = async (game: GameSession, item: "revive"): Promise<void> => {
  await selectTrainerOnlyAction(game, 1)
  await game.wait.until((state) => state.battle.ui === "bag", "trainer-only recovery Bag")
  for (let pocket = 0; pocket < 5; pocket++) {
    if ((await game.state.read()).battle.bag.item === item) break
    await game.controls.press("right")
    await game.wait.frames(60)
  }
  expect((await game.state.read()).battle.bag.item).toBe(item)
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag-context", "Revive context")
  await game.controls.press("a")
}

const loseActualTrainerBattle = async (game: GameSession, description: string): Promise<void> => {
  await chooseFightMove(game, description)
  let sawFaintedParty = false
  for (let attempt = 0; attempt < 1_200; attempt++) {
    const state = await game.state.read()
    sawFaintedParty ||= state.partyVitals.some((vital) => vital.hp === 0)
    if (!state.battle.active) {
      expect(sawFaintedParty).toBe(true)
      return
    }
    if (state.battle.ui === "action-menu") await chooseFightMove(game, description)
    else if (
      state.battle.ui === "move-menu" ||
      state.battle.ui === "text" ||
      state.battle.ui === "other"
    )
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not finish after an actual party faint: ${JSON.stringify(await game.state.read())}`,
  )
}

const chooseRevive = async (game: GameSession): Promise<void> => {
  await useTrainerOnlyBagItem(game, "revive")
  await game.wait.until((state) => state.ui.mode === "party-menu", "Revive target picker")
  await game.wait.frames(60)
  await fs.promises.writeFile("/tmp/trainer-only-story-revive-picker.png", await game.screenshot())
  await game.controls.press("a")
  let revived = false
  let returnedToField = false
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    revived ||= state.partyVitals.some((vital) => vital.hp > 0)
    if (!state.battle.active && state.ready) {
      returnedToField = true
      break
    }
    if (state.ui.mode === "party-menu") await game.controls.press("a")
    await game.wait.frames(12)
  }
  if (!returnedToField)
    throw new Error(
      `trainer-only Revive recovery did not return to field: ${JSON.stringify(await game.state.read())}`,
    )
  await game.wait.frames(120)
  expect(revived).toBe(true)
  const stable = await game.state.read()
  expect(stable).toMatchObject({
    ready: true,
    battle: { active: false },
  })
  expect(stable.partyVitals.some((vital) => vital.hp > 0)).toBe(true)
}

const moveOneTile = async (
  game: GameSession,
  direction: "up" | "down" | "left" | "right",
  x: number,
  y: number,
  description: string,
): Promise<void> => {
  // Overworld input can consume a short press to turn or begin a step.
  // Keep the driver field-only and bounded until the requested tile arrives.
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y) break
    await game.player.move(direction)
    await game.wait.frames(12)
  }
  const arrived = await game.state.read()
  if (arrived.player.x !== x || arrived.player.y !== y)
    throw new Error(`${description} did not reach ${x}:${y}: ${JSON.stringify(arrived)}`)
  await game.wait.until(
    (state) => state.ready && !state.battle.active && state.player.x === x && state.player.y === y,
    description,
  )
}

const moveIntoTrainerSight = async (
  game: GameSession,
  direction: "up" | "down" | "left" | "right",
  x: number,
  y: number,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y) return
    if (state.battle.active)
      throw new Error(`${description} started before reaching ${x}:${y}: ${JSON.stringify(state)}`)
    await game.player.move(direction)
    await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not reach ${x}:${y}: ${JSON.stringify(await game.state.read())}`,
  )
}

const arrangeRoute30Mikey = async (
  game: GameSession,
  party: PartyMonFixture[],
  bag?: { items: { revive: number } },
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "right", position: { map: "route-30", x: 22, y: 22 } },
    story: { flags: { hideRoute30Npcs: false, momVisited: false } },
    party,
    bag,
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
}

const arrangeRoute30Joey = async (
  game: GameSession,
  party: PartyMonFixture[],
  bag?: { items: { revive: number } },
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "down", position: { map: "route-30", x: 20, y: 27 } },
    story: { flags: { hideRoute30Npcs: false, momVisited: false } },
    party,
    bag,
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
}

describe.sequential("Wayfarer trainer-only story encounters", () => {
  for (const fixture of [
    { name: "empty", party: [] },
    { name: "fainted", party: [{ species: "rattata", fainted: true }] },
    { name: "Egg-only", party: [{ species: "rattata", egg: true }] },
  ] as const) {
    it(`uses Route 30 Mikey's stable ordinary refusal for a ${fixture.name} party`, async () => {
      const game = await GameSession.launch()
      try {
        await arrangeRoute30Mikey(game, [...fixture.party])
        const before = await game.state.read()
        await game.player.interact()
        const refusal = await waitForDialogueText(
          game,
          ordinaryRefusal,
          `Route 30 Mikey ${fixture.name} refusal`,
        )
        expect(refusal.battle.active).toBe(false)
        expect(refusal.dialogue.text).toBe(ordinaryRefusal)
        if (fixture.name === "empty")
          await fs.promises.writeFile(
            "/tmp/trainer-only-story-ordinary-refusal.png",
            await game.screenshot(),
          )
        expect(await game.story.flag("defeatedRoute30Mikey")).toBe(false)
        await finishFieldScript(game, `Route 30 Mikey ${fixture.name} refusal`)
        expect(await game.state.read()).toMatchObject({
          map: before.map,
          player: before.player,
          ready: true,
          battle: { active: false },
        })
      } finally {
        await game.close()
      }
    })
  }

  it("routes a native-ready Joey rematch into its reviewed no-party refusal", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: "route-30", x: 20, y: 27 } },
        story: {
          flags: { hideRoute30Npcs: false, momVisited: false },
          rematchTrainer: "joey-hns",
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      // The fixture sets the real first-battle flag and asks the normal
      // rematch table to select the first available rematch team. The base
      // caller therefore skips to Route30_EventScript_RematchJoey, whose
      // distinct registry entry must still require a usable party.
      expect(await game.story.flag("defeatedRoute30Joey")).toBe(true)
      expect(await game.story.flag("registeredRoute30Joey")).toBe(false)
      await game.player.interact()
      const refusal = await waitForDialogueText(
        game,
        ordinaryRefusal,
        "Route 30 Joey rematch refusal",
      )
      expect(refusal).toMatchObject({
        battle: { active: false },
        dialogue: { text: ordinaryRefusal },
      })
      await finishFieldScript(game, "Route 30 Joey rematch refusal")
      expect(await game.story.flag("registeredRoute30Joey")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("renders every frozen ordinary dialogue assignment across repeats and reload", async () => {
    const fixtures = [
      { name: "relaxed", x: 15, y: 45, facing: "left", text: "No POKMON ready? We can\nbattle" },
      { name: "eager", x: 29, y: 63, facing: "left", text: "I was hoping for a battle!\nMayb" },
      { name: "kind", x: 15, y: 64, facing: "up", text: "Dont worry about a battle.\nTak" },
      { name: "neutral", x: 24, y: 96, facing: "up", text: ordinaryRefusal },
    ] as const

    for (const fixture of fixtures) {
      const game = await GameSession.launch()
      try {
        await game.arrange({
          checkpoint: "new-bark-after-intro",
          player: {
            facing: fixture.facing,
            position: { map: "route-32", x: fixture.x, y: fixture.y },
          },
          party: [],
          determinism: { textSpeed: "instant", rngSeed: 1 },
        })
        await game.player.interact()
        const first = await waitForDialogueText(
          game,
          fixture.text,
          `Route 32 ${fixture.name} refusal`,
        )
        expect(first.battle.active).toBe(false)
        expect(first.dialogue.text).toBe(fixture.text)
        await game.wait.frames(12)
        await fs.promises.writeFile(
          `/tmp/trainer-only-story-ordinary-${fixture.name}.png`,
          await game.screenshot(),
        )
        await finishFieldScript(game, `Route 32 ${fixture.name} first refusal`)

        if (fixture.name === "relaxed") {
          await game.player.interact()
          const repeat = await waitForDialogueText(
            game,
            fixture.text,
            `Route 32 ${fixture.name} repeated refusal`,
          )
          expect(repeat).toMatchObject({
            battle: { active: false },
            dialogue: { text: first.dialogue.text },
          })
          await finishFieldScript(game, `Route 32 ${fixture.name} repeated refusal`)

          await game.saveAndReload()
          expect(await game.state.read()).toMatchObject({
            map: { name: "route-32" },
            player: { x: fixture.x, y: fixture.y },
            ready: true,
            battle: { active: false },
          })
          await game.player.interact()
          const reloaded = await waitForDialogueText(
            game,
            fixture.text,
            "Route 32 relaxed reload refusal",
          )
          expect(reloaded).toMatchObject({
            battle: { active: false },
            dialogue: { text: first.dialogue.text },
          })
          await finishFieldScript(game, "Route 32 relaxed reload refusal")
        }
      } finally {
        await game.close()
      }
    }
  })

  it("suppresses Route 30 Mikey and Joey sight encounters until the player talks", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "route-30", x: 23, y: 25 } },
        story: { flags: { hideRoute30Npcs: false, momVisited: true } },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const beforeMikey = await game.state.read()
      await game.player.move("up")
      await game.wait.until(
        (state) => state.ready && state.player.x === 23 && state.player.y === 24,
        "Route 30 Mikey sight first tile",
      )
      expect(await game.state.read()).toMatchObject({
        player: { x: 23, y: 24 },
        battle: { active: false },
        dialogue: { sequence: beforeMikey.dialogue.sequence },
      })

      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "left", position: { map: "route-30", x: 26, y: 28 } },
        story: { flags: { hideRoute30Npcs: false, momVisited: true } },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const beforeJoey = await game.state.read()
      await game.player.move("left")
      await game.wait.until(
        (state) => state.ready && state.player.x === 25 && state.player.y === 28,
        "Route 30 Joey sight tile",
      )
      expect(await game.state.read()).toMatchObject({
        player: { x: 25, y: 28 },
        battle: { active: false },
        dialogue: { sequence: beforeJoey.dialogue.sequence },
      })
    } finally {
      await game.close()
    }
  })

  it("keeps a completed Route 30 trainer's authored after-battle text", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "route-30", x: 23, y: 23 } },
        story: {
          flags: {
            hideRoute30Npcs: false,
            momVisited: false,
            defeatedRoute30Mikey: true,
          },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      await game.player.interact()
      const dialogue = await waitForDialogueText(
        game,
        "Becoming a good trainer",
        "completed Route 30 Mikey text",
      )
      expect(dialogue.battle.active).toBe(false)
      expect(dialogue.dialogue.text).not.toContain("Come back when you have a")
      await finishFieldScript(game, "completed Route 30 Mikey text")
    } finally {
      await game.close()
    }
  })

  it("returns from an actual allowlisted Route 30 loss in the field with one normal charge and no credit", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeRoute30Mikey(game, [{ species: "rattata", level: 1, moves: ["tackle"] }], {
        items: { revive: 1 },
      })
      const before = await game.state.read()
      await game.player.interact()
      await waitForTrainerBattle(game, "Route 30 Mikey supported battle")
      await loseActualTrainerBattle(game, "Route 30 Mikey actual loss")
      const retreat = await waitForDialogueText(game, retreatText, "Route 30 Mikey loss retreat")
      expect(retreat.dialogue.text).not.toContain("That's strange")
      await finishFieldScript(game, "Route 30 Mikey loss retreat")
      const after = await game.state.read()
      expect(after).toMatchObject({
        map: before.map,
        player: before.player,
        ready: true,
        battle: { active: false },
      })
      expect(after.partyVitals[0]?.hp).toBe(0)
      expect(after.money).toBe(before.money - Math.min(before.money, 8))
      expect(await game.story.flag("defeatedRoute30Mikey")).toBe(false)

      await game.player.interact()
      const refusal = await waitForDialogueText(
        game,
        ordinaryRefusal,
        "Route 30 Mikey post-loss refusal",
      )
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Route 30 Mikey post-loss refusal")
      expect((await game.state.read()).money).toBe(after.money)

      await game.battle.startWild({ species: "pidgey", level: 5 })
      await chooseRevive(game)
      await game.player.interact()
      await waitForTrainerBattle(game, "Route 30 Mikey direct retry after recovery")
      expect((await game.state.read()).battle.trainerOnly.active).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("intercepts Route 30 Joey's actual continuation loss before phone registration or trainer credit", async () => {
    const game = await GameSession.launch()
    try {
      await arrangeRoute30Joey(game, [{ species: "rattata", level: 1, moves: ["tackle"] }], {
        items: { revive: 1 },
      })
      const before = await game.state.read()
      await game.player.interact()
      await waitForTrainerBattle(game, "Route 30 Joey continuation battle")
      await loseActualTrainerBattle(game, "Route 30 Joey actual continuation loss")
      const retreat = await waitForDialogueText(
        game,
        retreatText,
        "Route 30 Joey continuation loss retreat",
      )
      expect(retreat.dialogue.text).not.toContain("RATTATA")
      await finishFieldScript(game, "Route 30 Joey continuation loss retreat")
      const after = await game.state.read()
      expect(after).toMatchObject({
        map: before.map,
        player: before.player,
        ready: true,
        battle: { active: false },
      })
      expect(after.partyVitals[0]?.hp).toBe(0)
      expect(after.money).toBe(before.money - Math.min(before.money, 8))
      await expect(game.story.flag("defeatedRoute30Joey")).resolves.toBe(false)
      await expect(game.story.flag("registeredRoute30Joey")).resolves.toBe(false)

      await game.player.interact()
      const refusal = await waitForDialogueText(
        game,
        ordinaryRefusal,
        "Route 30 Joey post-loss refusal",
      )
      expect(refusal.battle.active).toBe(false)
      await finishFieldScript(game, "Route 30 Joey post-loss refusal")
      expect((await game.state.read()).money).toBe(after.money)

      await game.battle.startWild({ species: "pidgey", level: 5 })
      await chooseRevive(game)
      await moveOneTile(game, "up", 20, 26, "Route 30 Joey leaves recovery sight line")
      await moveIntoTrainerSight(game, "down", 20, 27, "Route 30 Joey re-enters recovery sight")
      await waitForTrainerBattle(game, "Route 30 Joey automatic retry after leaving recovery sight")
    } finally {
      await game.close()
    }
  })

  it("silently suppresses Cherrygrove Silver without chapter writes, then re-arms after a real recovery and leave/re-entry", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "right", position: { map: "cherrygrove-city", x: 55, y: 10 } },
        story: {
          vars: { cherrygroveCityState: 3, starterMon: 0 },
          flags: {
            johtoStarterChoiceCommitted: true,
            hideSilverCherrygrove: false,
          },
        },
        party: [{ species: "rattata", fainted: true }],
        bag: { items: { revive: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      const before = await game.state.read()
      await game.player.move("right")
      await game.wait.forReady()
      const suppressed = await game.state.read()
      expect(suppressed).toMatchObject({
        player: { x: 56, y: 10 },
        ready: true,
        battle: { active: false },
        dialogue: { sequence: before.dialogue.sequence },
      })
      expect(await game.story.var("cherrygroveCityState")).toBe(3)
      expect(await game.story.flag("hideSilverCherrygrove")).toBe(false)

      await game.battle.startWild({ species: "pidgey", level: 5 })
      await chooseRevive(game)
      expect(await game.state.read()).toMatchObject({
        party: [{ species: "rattata", fainted: false, egg: false }],
        battle: { active: false },
      })
      await game.wait.frames(120)
      expect(await game.state.read()).toMatchObject({
        map: { name: "cherrygrove-city" },
        player: { x: 56, y: 10 },
        ready: true,
        battle: { active: false },
      })
      expect(await game.story.var("cherrygroveCityState")).toBe(3)
      expect(await game.story.flag("hideSilverCherrygrove")).toBe(false)

      await game.player.warp("route-30", 33, 22)
      await game.wait.forMap("route-30")
      await game.player.warp("cherrygrove-city", 55, 10, "right")
      await game.wait.forMap("cherrygrove-city")
      await game.player.move("right")
      await waitForTrainerBattle(game, "recovered Cherrygrove Silver battle")
      expect(await game.story.var("cherrygroveCityState")).toBe(3)
      expect(await game.story.flag("hideSilverCherrygrove")).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("keeps Slowpoke Well Proton's local objective and reward state unchanged for no-party refusal", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "slowpoke-well-b1f", x: 14, y: 4 } },
        story: {
          vars: { azaleaTownState: 2 },
          flags: { hideAzaleaTownRockets: false, hideKurt1: true, defeatedProton1: false },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      expect(await game.story.flag("defeatedProton1")).toBe(false)
      await game.player.interact()
      const refusal = await waitForDialogueText(
        game,
        "Youve got no",
        "Slowpoke Well Proton refusal",
      )
      expect(refusal.battle.active).toBe(false)
      expect(refusal.dialogue.text).toContain("Youve got no")
      await game.wait.frames(24)
      await fs.promises.writeFile(
        "/tmp/trainer-only-story-proton-refusal.png",
        await game.screenshot(),
      )
      await finishFieldScript(game, "Slowpoke Well Proton refusal")
      expect(await game.story.var("azaleaTownState")).toBe(2)
      await expect(game.story.flag("hideAzaleaTownRockets")).resolves.toBe(false)
      await expect(game.story.flag("hideKurt1")).resolves.toBe(true)
      await expect(game.story.flag("defeatedProton1")).resolves.toBe(false)
    } finally {
      await game.close()
    }
  })

  it("returns an actual supported Slowpoke Well Proton loss to its local retreat without completing the objective", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "slowpoke-well-b1f", x: 14, y: 4 } },
        story: {
          vars: { azaleaTownState: 2 },
          flags: { hideAzaleaTownRockets: false, hideKurt1: true, defeatedProton1: false },
        },
        party: [{ species: "rattata", level: 1, moves: ["tackle"] }],
        bag: { items: { revive: 1 } },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      expect(await game.story.flag("defeatedProton1")).toBe(false)
      const before = await game.state.read()
      await game.player.interact()
      await waitForTrainerBattle(game, "Slowpoke Well Proton battle")
      await loseActualTrainerBattle(game, "Slowpoke Well Proton actual loss")
      await waitForDialogueText(game, retreatText, "Slowpoke Well Proton loss retreat")
      await finishFieldScript(game, "Slowpoke Well Proton loss retreat")
      expect(await game.state.read()).toMatchObject({
        map: before.map,
        player: before.player,
        ready: true,
        battle: { active: false },
      })
      expect((await game.state.read()).partyVitals[0]?.hp).toBe(0)
      expect((await game.state.read()).money).toBe(before.money - Math.min(before.money, 8))
      expect(await game.story.var("azaleaTownState")).toBe(2)
      await expect(game.story.flag("hideAzaleaTownRockets")).resolves.toBe(false)
      await expect(game.story.flag("hideKurt1")).resolves.toBe(true)
      await expect(game.story.flag("defeatedProton1")).resolves.toBe(false)

      await game.battle.startWild({ species: "pidgey", level: 5 })
      await chooseRevive(game)
      await game.player.interact()
      await waitForTrainerBattle(game, "Slowpoke Well Proton retry after loss")
    } finally {
      await game.close()
    }
  })

  it("refuses no-party Viridian Blue without a badge or Gym completion", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "viridian-gym", x: 5, y: 3 } },
        story: {
          vars: { numBadges: 0 },
          flags: {
            hideViridianBlue: false,
            defeatedViridianGym: false,
            badge16: false,
            defeatedViridianBlue: false,
          },
        },
        party: [],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      expect(await game.story.flag("defeatedViridianBlue")).toBe(false)
      await game.player.interact()
      const refusal = await waitForDialogueText(
        game,
        "Come back with a",
        "Viridian Blue no-party refusal",
      )
      expect(refusal.battle.active).toBe(false)
      await game.wait.frames(24)
      await fs.promises.writeFile(
        "/tmp/trainer-only-story-blue-gym-refusal.png",
        await game.screenshot(),
      )
      await finishFieldScript(game, "Viridian Blue no-party refusal")
      expect(await game.story.var("numBadges")).toBe(0)
      await expect(game.story.flag("badge16")).resolves.toBe(false)
      await expect(game.story.flag("defeatedViridianGym")).resolves.toBe(false)
      await expect(game.story.flag("defeatedViridianBlue")).resolves.toBe(false)
    } finally {
      await game.close()
    }
  })

  it("keeps an actual Viridian Blue Gym loss on its existing route without a badge or victory credit", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: { map: "viridian-gym", x: 5, y: 3 } },
        story: {
          vars: { numBadges: 0 },
          flags: {
            hideViridianBlue: false,
            defeatedViridianGym: false,
            badge16: false,
            defeatedViridianBlue: false,
          },
        },
        party: [{ species: "rattata", level: 1, moves: ["tackle"] }],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })
      expect(await game.story.flag("defeatedViridianBlue")).toBe(false)
      const before = await game.state.read()
      await game.player.interact()
      await waitForTrainerBattle(game, "Viridian Blue Gym battle")
      await loseActualTrainerBattle(game, "Viridian Blue actual Gym loss")
      await game.wait.until(
        (state) => !state.battle.active && state.ready && state.map.name !== "viridian-gym",
        "Viridian Blue's existing loss route",
        6_000,
      )
      expect((await game.state.read()).money).toBe(before.money - Math.min(before.money, 8))
      expect(await game.story.var("numBadges")).toBe(0)
      await expect(game.story.flag("badge16")).resolves.toBe(false)
      await expect(game.story.flag("defeatedViridianGym")).resolves.toBe(false)
      await expect(game.story.flag("defeatedViridianBlue")).resolves.toBe(false)
    } finally {
      await game.close()
    }
  })
})
