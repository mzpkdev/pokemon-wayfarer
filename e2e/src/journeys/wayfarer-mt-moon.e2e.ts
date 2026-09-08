import { beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap, type PartyMonFixture } from "../harness/game-session"

const settle = async (game: GameSession, map: GameMap, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 180; attempt++) {
    const state = await game.state.read()
    if (state.ready && state.map.name === map) return
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || state.battle.ui === "text")
      await game.controls.press("a")
    else
      await game.wait.frames(12)
  }
  throw new Error(`${description} did not return control: ${JSON.stringify(await game.state.read())}`)
}

const arrangeMiguel = async (
  game: GameSession,
  party: PartyMonFixture[],
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { position: { map: "mt-moon-cave", x: 25, y: 16 }, facing: "up" },
    party,
    determinism: { textSpeed: "instant" },
  })
}

const arrangePendingChoice = async (
  game: GameSession,
  options: Parameters<GameSession["arrange"]>[0]["bag"] = undefined,
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { position: { map: "mt-moon-cave", x: 24, y: 16 }, facing: "up" },
    story: { flags: { mtMoonMiguelDefeated: true } },
    party: [{ species: "lapras" }],
    bag: options,
    determinism: { textSpeed: "instant" },
  })
}

const startMiguelBattle = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  for (let attempt = 0; attempt < 180; attempt++) {
    if ((await game.state.read()).battle.ui === "action-menu") return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error("Miguel battle did not start")
}

const chooseHelix = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.controls.press("a")
  await game.wait.frames(60)
  await game.wait.frames(20)
  await game.controls.press("a")
  await settle(game, "mt-moon-cave", "Helix fossil selection")
}

const chooseDome = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.controls.press("a")
  await game.wait.frames(80)
  await game.controls.press("a")
  await game.wait.frames(20)
  await game.controls.press("down")
  await game.wait.frames(20)
  await game.controls.press("a")
  await settle(game, "mt-moon-cave", "Dome fossil selection")
}

const chooseExit = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.controls.press("a")
  await game.wait.frames(80)
  await game.controls.press("a")
  await game.wait.frames(20)
  await game.controls.press("down")
  await game.wait.frames(12)
  await game.controls.press("down")
  await game.wait.frames(12)
  await game.controls.press("a")
  await settle(game, "mt-moon-cave", "Mt. Moon fossil-menu exit")
}

const settleOverworld = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 240; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.battle.active) return
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || state.battle.ui === "text")
      await game.controls.press("a")
    else
      await game.wait.frames(12)
  }
  throw new Error(`${description} did not return to an overworld: ${JSON.stringify(await game.state.read())}`)
}

const startGrunt1SightBattle = async (game: GameSession): Promise<void> => {
  await game.controls.press("up")
  await game.dialogue.waitForOpen()
  for (let attempt = 0; attempt < 180; attempt++) {
    if ((await game.state.read()).battle.ui === "action-menu") return
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error("Mt. Moon Grunt 1 sight battle did not start")
}

describe.sequential("Wayfarer Mt. Moon fossils", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("keeps Miguel and both fossils pending for an empty party", async () => {
    await arrangeMiguel(game, [])
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(true)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(true)
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settle(game, "mt-moon-cave", "Miguel empty-party refusal")
    expect((await game.state.read()).battle.active).toBe(false)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(true)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(true)
  })

  it("reveals both loaded fossils immediately after Miguel wins", async () => {
    await arrangeMiguel(game, [{ species: "lapras", level: 100 }])
    await startMiguelBattle(game)
    await game.battle.win()
    await game.wait.frames(180)
    await settle(game, "mt-moon-cave", "Miguel victory")
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(false)
    expect((await game.state.read()).player).toMatchObject({ x: 25, y: 16 })
    await game.controls.press("right")
    await game.wait.frames(12)
    await game.controls.press("right")
    await game.wait.frames(12)
    expect((await game.state.read()).player).toMatchObject({ x: 26, y: 16 })
    await game.controls.press("up")
    await game.wait.frames(12)
    await chooseHelix(game)
    expect(await game.inventory.contains("helixFossil")).toBe(true)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(true)
  })

  it("keeps Miguel and fossils pending after a loss, then permits a retry", async () => {
    await arrangeMiguel(game, [{ species: "lapras", level: 1 }])
    await startMiguelBattle(game)
    await game.battle.lose()
    await settleOverworld(game, "Miguel loss")
    expect(await game.story.flag("mtMoonMiguelDefeated")).toBe(false)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(true)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(true)
    await game.player.warp("mt-moon-cave", 25, 16, "up")
    await startMiguelBattle(game)
    expect((await game.state.read()).battle.ui).toBe("action-menu")
  })

  it("makes the pending choice immediately interactable and persists Helix", async () => {
    await arrangePendingChoice(game)
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(false)
    await chooseHelix(game)
    expect(await game.inventory.contains("helixFossil")).toBe(true)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(true)
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(true)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(true)
    await game.saveAndReload()
    expect(await game.inventory.contains("helixFossil")).toBe(true)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(true)
  })

  it("keeps a full Items-pocket choice pending across reload", async () => {
    await arrangePendingChoice(game, { fullPockets: ["items"] })
    await chooseHelix(game)
    expect(await game.inventory.contains("helixFossil")).toBe(false)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(false)
    await game.saveAndReload()
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(false)
  })

  it("reconciles a selected pre-owned Helix without preventing the local choice", async () => {
    await arrangePendingChoice(game, { items: { helixFossil: 1 } })
    await chooseHelix(game)
    expect(await game.inventory.contains("helixFossil")).toBe(true)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(true)
  })

  it("takes Dome from the pending local choice", async () => {
    await arrangePendingChoice(game)
    await chooseDome(game)
    expect(await game.inventory.contains("domeFossil")).toBe(true)
    expect(await game.inventory.contains("helixFossil")).toBe(false)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(true)
  })

  it("keeps both fossils available when the local choice exits", async () => {
    await arrangePendingChoice(game)
    await chooseExit(game)
    expect(await game.inventory.contains("helixFossil")).toBe(false)
    expect(await game.inventory.contains("domeFossil")).toBe(false)
    expect(await game.story.flag("receivedMtMoonFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonDomeFossil")).toBe(false)
    expect(await game.story.flag("hideMtMoonHelixFossil")).toBe(false)
  })

  it("does not start Grunt 1's sight battle without a party", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "mt-moon-cave", x: 10, y: 16 }, facing: "up" },
      party: [],
      determinism: { textSpeed: "instant" },
    })
    await game.controls.press("up")
    await game.wait.frames(60)
    expect(await game.state.read()).toMatchObject({ ready: true, dialogueOpen: false })
    expect((await game.state.read()).battle.active).toBe(false)
    expect(await game.story.flag("mtMoonGrunt1Defeated")).toBe(false)
  })

  it("shows Grunt 1's ordinary refusal when directly talked to without a party", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "mt-moon-cave", x: 10, y: 15 }, facing: "up" },
      party: [],
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await settle(game, "mt-moon-cave", "Mt. Moon Grunt 1 direct empty-party refusal")
    expect((await game.state.read()).battle.active).toBe(false)
    expect(await game.story.flag("mtMoonGrunt1Defeated")).toBe(false)
  })

  it("starts and completes Grunt 1's normal sight battle", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "mt-moon-cave", x: 10, y: 16 }, facing: "up" },
      party: [{ species: "lapras", level: 100 }],
      determinism: { textSpeed: "instant" },
    })
    await startGrunt1SightBattle(game)
    await game.battle.win()
    await game.wait.frames(180)
    await settle(game, "mt-moon-cave", "Mt. Moon Grunt 1 victory")
    expect(await game.story.flag("mtMoonGrunt1Defeated")).toBe(true)
    await game.saveAndReload()
    await game.controls.press("up")
    await game.wait.frames(12)
    await game.controls.press("up")
    await game.wait.frames(60)
    expect(await game.state.read()).toMatchObject({ ready: true, dialogueOpen: false })
    expect((await game.state.read()).battle.active).toBe(false)
  })

})
