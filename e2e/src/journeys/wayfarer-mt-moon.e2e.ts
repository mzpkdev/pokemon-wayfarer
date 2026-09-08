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

const chooseHelix = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.controls.press("a")
  await game.wait.frames(60)
  await game.wait.frames(20)
  await game.controls.press("a")
  await settle(game, "mt-moon-cave", "Helix fossil selection")
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

})
