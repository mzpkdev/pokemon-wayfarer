import * as fs from "node:fs"
import { beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap, type PartyMonFixture } from "../harness/game-session"

const finishInteraction = async (game: GameSession, map: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 100; attempt++) {
    await game.wait.frames(20)
    const state = await game.state.read()
    if (state.ready && state.map.name === map) return
    await game.controls.press("a")
  }
  throw new Error(`Hideout interaction did not settle: ${JSON.stringify(await game.state.read())}`)
}

const startBattle = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(10)
  }
  throw new Error(`Hideout battle did not start: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer Celadon Rocket Hideout", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  const arrangeEntrance = async (party: PartyMonFixture[]): Promise<void> => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        position: { map: "celadon-game-corner", x: 11, y: 2 },
        facing: "left",
      },
      party,
      determinism: { textSpeed: "instant" },
    })
  }

  it("keeps the wall switch unavailable before the entrance grunt is defeated", async () => {
    await arrangeEntrance([{ species: "lapras" }])
    await game.controls.press("up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "celadon-game-corner")
    expect((await game.state.read()).battle.active).toBe(false)
    expect(await game.inventory.contains("liftKey")).toBe(false)
    expect(await game.inventory.contains("silphScope")).toBe(false)
  })

  it("refuses the entrance battle for empty, fainted and Eggs-only parties", async () => {
    const parties: PartyMonFixture[][] = [
      [],
      [{ species: "lapras", fainted: true }],
      [{ species: "lapras", egg: true }],
    ]
    for (const party of parties) {
      await arrangeEntrance(party)
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await finishInteraction(game, "celadon-game-corner")
      const state = await game.state.read()
      expect(state.battle.active).toBe(false)
      expect(state.player).toMatchObject({ x: 11, y: 2 })
      expect(await game.inventory.contains("silphScope")).toBe(false)
    }
  })

  it("discovers the entrance after a real win and returns through save/reload", async () => {
    await arrangeEntrance([{ species: "lapras" }])
    await startBattle(game)
    await game.battle.win()
    await finishInteraction(game, "celadon-game-corner")
    await game.controls.press("up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "celadon-hideout-b1f")
    expect((await game.state.read()).player).toMatchObject({ x: 12, y: 2 })
    await fs.promises.writeFile("/tmp/wayfarer-hideout-arrival.png", await game.screenshot())
    await game.saveAndReload()
    // The authored FRLG stair exits west from its upper-left stair tile.
    await game.controls.press("left")
    await game.wait.frames(20)
    await game.controls.press("left")
    await finishInteraction(game, "celadon-game-corner")
    expect((await game.state.read()).player).toMatchObject({ x: 11, y: 2 })
    await game.controls.press("up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "celadon-hideout-b1f")
    expect(await game.inventory.contains("liftKey")).toBe(false)
    expect(await game.inventory.contains("silphScope")).toBe(false)
  })

  it("reveals the Lift Key only after its guard loses and persists the handoff", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b4f", x: 5, y: 2 }, facing: "left" },
      party: [{ species: "lapras" }],
      determinism: { textSpeed: "instant" },
    })
    expect(await game.story.flag("celadonHideoutKeyReceived")).toBe(false)
    await startBattle(game)
    await game.battle.win()
    await finishInteraction(game, "celadon-hideout-b4f")
    expect(await game.story.flag("celadonHideoutKeyGruntDefeated")).toBe(true)
    expect(await game.inventory.contains("liftKey")).toBe(false)
    await game.player.warp("celadon-hideout-b4f", 3, 3, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "celadon-hideout-b4f")
    expect(await game.inventory.contains("liftKey")).toBe(true)
    expect(await game.story.flag("celadonHideoutKeyReceived")).toBe(true)
    expect(await game.story.flag("celadonHideoutLiftEnabled")).toBe(true)
    expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(false)
    await game.saveAndReload()
    expect(await game.inventory.contains("liftKey")).toBe(true)
    expect(await game.story.flag("celadonHideoutKeyReceived")).toBe(true)
  })

  it("keeps a full-pocket Lift Key handoff retryable across reloads", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b4f", x: 3, y: 3 }, facing: "up" },
      story: { flags: { celadonHideoutKeyGruntDefeated: true } },
      bag: { fullPockets: ["keyItems"] },
      determinism: { textSpeed: "instant" },
    })
    for (let attempt = 0; attempt < 2; attempt++) {
      await game.controls.press("up")
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await finishInteraction(game, "celadon-hideout-b4f")
      expect(await game.inventory.contains("liftKey")).toBe(false)
      expect(await game.story.flag("celadonHideoutKeyReceived")).toBe(false)
      expect(await game.story.flag("celadonHideoutLiftEnabled")).toBe(false)
      await game.saveAndReload()
    }
  })

  it("requires the local guard victory even with a pre-owned Lift Key", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-elevator", x: 1, y: 2 }, facing: "left" },
      bag: { items: { liftKey: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "celadon-hideout-elevator")
    expect(await game.story.flag("celadonHideoutLiftEnabled")).toBe(false)
    expect(await game.story.flag("celadonHideoutKeyReceived")).toBe(false)
    expect(await game.inventory.contains("liftKey")).toBe(true)
  })

  it("requires both actual door-guard victories before Giovanni", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b4f", x: 19, y: 5 }, facing: "up" },
      party: [{ species: "lapras" }],
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "celadon-hideout-b4f")
    expect((await game.state.read()).battle.active).toBe(false)
    for (const x of [16, 19]) {
      await game.player.warp("celadon-hideout-b4f", x, 15, "up")
      await startBattle(game)
      await game.battle.win()
      await finishInteraction(game, "celadon-hideout-b4f")
    }
    expect(await game.story.flag("celadonHideoutLeftGuardDefeated")).toBe(true)
    expect(await game.story.flag("celadonHideoutRightGuardDefeated")).toBe(true)
    await game.saveAndReload()
    await game.player.warp("celadon-hideout-b4f", 19, 5, "up")
    await startBattle(game)
    await game.battle.win()
    await finishInteraction(game, "celadon-hideout-b4f")
    expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(true)
    expect(await game.inventory.contains("silphScope")).toBe(false)
    await game.player.warp("celadon-hideout-b4f", 20, 6, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "celadon-hideout-b4f")
    expect(await game.inventory.contains("silphScope")).toBe(true)
    expect(await game.story.flag("celadonHideoutScopeReceived")).toBe(true)
    await game.saveAndReload()
    expect(await game.inventory.contains("silphScope")).toBe(true)
  })

  it("keeps the Scope available after a full-pocket refusal and reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b4f", x: 20, y: 6 }, facing: "up" },
      story: {
        flags: {
          celadonHideoutGiovanniDefeated: true,
          celadonHideoutGiovanniTrainerDefeated: true,
        },
      },
      bag: { fullPockets: ["keyItems"] },
      determinism: { textSpeed: "instant" },
    })
    for (let attempt = 0; attempt < 2; attempt++) {
      await game.controls.press("up")
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await finishInteraction(game, "celadon-hideout-b4f")
      expect(await game.inventory.contains("silphScope")).toBe(false)
      expect(await game.story.flag("celadonHideoutScopeReceived")).toBe(false)
      await game.saveAndReload()
    }
  })

  it("keeps Giovanni retryable after a loss without granting the Scope", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b4f", x: 19, y: 5 }, facing: "up" },
      story: {
        flags: {
          celadonHideoutLeftGuardDefeated: true,
          celadonHideoutRightGuardDefeated: true,
        },
      },
      party: [{ species: "lapras" }],
      determinism: { textSpeed: "instant" },
    })
    await startBattle(game)
    await game.battle.lose()
    for (let attempt = 0; attempt < 360; attempt++) {
      await game.wait.frames(20)
      const state = await game.state.read()
      if (state.ready && !state.battle.active) break
      await game.controls.press("a")
    }
    expect((await game.state.read()).ready).toBe(true)
    expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(false)
    expect(await game.story.flag("celadonHideoutScopeReceived")).toBe(false)
    expect(await game.inventory.contains("silphScope")).toBe(false)
    await game.player.warp("celadon-hideout-b4f", 19, 5, "up")
    await startBattle(game)
    await game.battle.win()
    await finishInteraction(game, "celadon-hideout-b4f")
    expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(true)
    expect(await game.story.flag("celadonHideoutScopeReceived")).toBe(false)
  })

  it("locates Celadon's hideout separately from Mahogany on the active map", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b1f", x: 12, y: 2 } },
    })
    expect(await game.regionMap.entry("johto", 58)).toEqual({
      x: 11,
      y: 6,
      width: 1,
      height: 1,
    })
    expect(await game.regionMap.entry("combined", 58)).toEqual({
      x: 22,
      y: 5,
      width: 1,
      height: 1,
    })
    expect(await game.regionMap.observePokedex()).toMatchObject({
      layout: "combined",
      playerMarker: { x: 23, y: 7 },
    })
  })

  it("uses the keyed elevator and preserves the selected floor through reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b1f", x: 24, y: 26 }, facing: "up" },
      story: {
        vars: { farawayIslandStepCounter: 173 },
        flags: {
          celadonHideoutKeyGruntDefeated: true,
          celadonHideoutKeyReceived: true,
          celadonHideoutLiftEnabled: true,
        },
      },
      bag: { items: { liftKey: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await game.controls.press("up")
    await game.wait.forMap("celadon-hideout-elevator")
    await game.player.warp("celadon-hideout-elevator", 1, 2, "left")
    await game.player.interact()
    await game.wait.frames(40)
    await game.controls.press("down")
    await game.controls.press("a")
    await finishInteraction(game, "celadon-hideout-elevator")
    await game.saveAndReload()
    await game.player.warp("celadon-hideout-elevator", 2, 4, "down")
    await game.controls.press("down")
    await game.wait.frames(20)
    await game.controls.press("down")
    await game.wait.forMap("celadon-hideout-b2f")
    expect(await game.story.var("farawayIslandStepCounter")).toBe(173)
    expect(await game.inventory.contains("liftKey")).toBe(true)
    expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(false)
  })

  it("does not trigger a defeated guard again when crossing its sight range", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b4f", x: 4, y: 3 }, facing: "up" },
      story: { flags: { celadonHideoutKeyGruntDefeated: true } },
      party: [{ species: "lapras" }],
      determinism: { textSpeed: "instant" },
    })
    await game.wait.frames(60)
    expect((await game.state.read()).ready).toBe(true)
    expect((await game.state.read()).battle.active).toBe(false)
  })

  it("does not approach unusable parties in a guard's sight range", async () => {
    for (const party of [
      [],
      [{ species: "lapras", fainted: true }],
      [{ species: "lapras", egg: true }],
    ] as PartyMonFixture[][]) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "celadon-hideout-b4f", x: 4, y: 3 }, facing: "up" },
        party,
        determinism: { textSpeed: "instant" },
      })
      await game.wait.frames(60)
      const state = await game.state.read()
      expect(state.ready).toBe(true)
      expect(state.battle.active).toBe(false)
      expect(await game.story.flag("celadonHideoutKeyGruntDefeated")).toBe(false)
    }
  })

  it("starts an undefeated guard's sight battle for a usable party", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "celadon-hideout-b4f", x: 4, y: 4 }, facing: "up" },
      party: [{ species: "lapras" }],
      determinism: { textSpeed: "instant" },
    })
    await game.controls.press("up")
    await game.dialogue.waitForOpen()
    for (let attempt = 0; attempt < 360; attempt++) {
      if ((await game.state.read()).battle.ui === "action-menu") break
      await game.controls.press("a")
      await game.wait.frames(10)
    }
    expect((await game.state.read()).battle.ui).toBe("action-menu")
    await game.battle.win()
    await finishInteraction(game, "celadon-hideout-b4f")
    expect(await game.story.flag("celadonHideoutKeyGruntDefeated")).toBe(true)
  })
})
