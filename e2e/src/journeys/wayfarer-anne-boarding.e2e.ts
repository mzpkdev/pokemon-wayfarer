import * as fs from "node:fs"
import { beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"

const chooseHarborSlot = async (
  game: GameSession,
  slot: number,
  capture = false,
): Promise<void> => {
  await game.player.interact()
  await game.wait.frames(30)
  await game.controls.press("a")
  await game.wait.frames(30)
  if (capture)
    await fs.promises.writeFile("/tmp/wayfarer-anne-harbor-menu.png", await game.screenshot())
  for (let index = 0; index < slot; index++) await game.controls.press("down")
  await game.controls.press("a")
}

const saveAndReload = async (game: GameSession): Promise<void> => {
  try {
    await game.saveAndReload()
  } catch (error) {
    await fs.promises.writeFile("/tmp/wayfarer-anne-reload-failure.png", await game.screenshot())
    throw error
  }
}

const finishInteraction = async (game: GameSession, destination: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 80; attempt++) {
    await game.wait.frames(20)
    const state = await game.state.read()
    if (state.ready && state.map.name === destination) return
    await game.controls.press("a")
  }
  throw new Error(`Anne interaction did not settle: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer persistent Anne boarding", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  const arrangeDock = async (ticket: boolean): Promise<void> => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "vermilion-port-inside", x: 8, y: 9 },
      },
      story: { vars: { ssAquaState: 0 } },
      bag: { items: ticket ? { ssTicket: 1 } : {} },
      determinism: { textSpeed: "instant" },
    })
  }

  it("keeps every old service unavailable before regular Aqua eligibility", async () => {
    for (let slot = 0; slot < 5; slot++) {
      await arrangeDock(true)
      await chooseHarborSlot(game, slot)
      await finishInteraction(game, "vermilion-port-inside")
      expect((await game.state.read()).player).toMatchObject({ x: 8, y: 9 })
      expect(await game.story.var("ssAquaState")).toBe(0)
      expect(await game.inventory.contains("ssTicket")).toBe(true)
    }
  })

  it("preserves Exit at slot 5 and supports B cancellation", async () => {
    await arrangeDock(true)
    await chooseHarborSlot(game, 5)
    await finishInteraction(game, "vermilion-port-inside")
    await game.player.interact()
    await game.wait.frames(30)
    await game.controls.press("a")
    await game.wait.frames(30)
    await game.controls.press("b")
    await finishInteraction(game, "vermilion-port-inside")
    expect(await game.story.var("ssAquaState")).toBe(0)
    expect(await game.inventory.contains("ssTicket")).toBe(true)
  })

  it("refuses Anne at slot 6 without a Ticket and leaves the player at the dock", async () => {
    await arrangeDock(false)
    await chooseHarborSlot(game, 6)
    await finishInteraction(game, "vermilion-port-inside")
    expect((await game.state.read()).player).toMatchObject({ x: 8, y: 9 })
    expect(await game.story.var("ssAquaState")).toBe(0)
    expect(await game.inventory.contains("ssTicket")).toBe(false)
  })

  it("boards with a Ticket before Aqua eligibility and returns after save/reload", async () => {
    await arrangeDock(true)
    await chooseHarborSlot(game, 6, true)
    await finishInteraction(game, "ss-anne-1f")
    expect((await game.state.read()).player).toMatchObject({ x: 19, y: 2 })
    await fs.promises.writeFile("/tmp/wayfarer-anne-arrival.png", await game.screenshot())
    expect(await game.story.var("ssAquaState")).toBe(0)
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(false)
    await saveAndReload(game)
    await game.controls.press("up")
    await finishInteraction(game, "vermilion-port-inside")
    expect((await game.state.read()).player).toMatchObject({ x: 8, y: 9 })
    expect(await game.inventory.contains("ssTicket")).toBe(true)
    expect(await game.story.var("ssAquaState")).toBe(0)
    await game.controls.press("down")
    await chooseHarborSlot(game, 6)
    await finishInteraction(game, "ss-anne-1f")
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(false)
  })

  it("keeps the captain reward and boarding available after the local adventure", async () => {
    await arrangeDock(true)
    await chooseHarborSlot(game, 6)
    await finishInteraction(game, "ss-anne-1f")
    await game.player.warp("ss-anne-captains-office", 5, 5, "up")
    await fs.promises.writeFile("/tmp/wayfarer-anne-captain.png", await game.screenshot())
    expect(await game.inventory.contains("cut")).toBe(false)
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "ss-anne-captains-office")
    await fs.promises.writeFile("/tmp/wayfarer-anne-captain-after.png", await game.screenshot())
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(true)
    expect(await game.inventory.contains("cut")).toBe(true)
    await saveAndReload(game)
    await game.controls.press("up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "ss-anne-captains-office")
    expect(await game.inventory.contains("cut")).toBe(true)
    await game.player.warp("ss-anne-1f", 19, 2, "up")
    await game.controls.press("up")
    await finishInteraction(game, "vermilion-port-inside")
    await game.controls.press("down")
    await chooseHarborSlot(game, 6)
    await finishInteraction(game, "ss-anne-1f")
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(true)
    expect(await game.inventory.contains("ssTicket")).toBe(true)
    expect(await game.story.var("ssAquaState")).toBe(0)
  })

  it("lets visitors pass Blue without a rival battle or captain completion", async () => {
    await arrangeDock(true)
    await chooseHarborSlot(game, 6)
    await finishInteraction(game, "ss-anne-1f")
    await game.player.warp("ss-anne-2f", 30, 7, "up")
    await game.controls.press("up")
    await game.dialogue.waitForOpen()
    await fs.promises.writeFile("/tmp/wayfarer-anne-blue.png", await game.screenshot())
    await finishInteraction(game, "ss-anne-2f")
    expect((await game.state.read()).phase).toBe("overworld")
    expect(await game.story.flag("ssAnneBlueMet")).toBe(true)
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(false)
    expect(await game.inventory.contains("cut")).toBe(false)
    await saveAndReload(game)
    await game.player.warp("ss-anne-2f", 30, 7, "up")
    await game.controls.press("up")
    await game.wait.forReady()
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(false)
  })

  it("requires the captain interaction even when Cut is already owned", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "ss-anne-captains-office", x: 5, y: 5 },
      },
      bag: { hms: { cut: 1 }, items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(false)
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "ss-anne-captains-office")
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(true)
    expect(await game.inventory.contains("cut")).toBe(true)
    expect((await game.state.read()).bag.hms.cut).toBe(1)
  })

  it("retains an ordinary trainer battle and its victory after save/reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "ss-anne-1f-room-2", x: 0, y: 3 },
      },
      party: [{ species: "lapras", level: 30, moves: ["surf"] }],
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    for (let attempt = 0; attempt < 360; attempt++) {
      const state = await game.state.read()
      if (state.battle.ui === "action-menu") break
      if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
        await game.controls.press("a")
      else await game.wait.frames(10)
    }
    expect((await game.state.read()).battle.ui).toBe("action-menu")
    await game.battle.win()
    await finishInteraction(game, "ss-anne-1f-room-2")
    await saveAndReload(game)
    await game.controls.press("down")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishInteraction(game, "ss-anne-1f-room-2")
    expect((await game.state.read()).battle.active).toBe(false)
    expect(await game.story.flag("ssAnneCaptainReward")).toBe(false)
    expect(await game.story.flag("ssAnneBlueMet")).toBe(false)
  })

  it("does not complete the captain reward when the TM/HM pocket is full", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "ss-anne-captains-office", x: 5, y: 5 },
      },
      bag: { fullPockets: ["tmHm"], items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })
    for (let visit = 0; visit < 2; visit++) {
      await game.controls.press("up")
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await finishInteraction(game, "ss-anne-captains-office")
      expect(await game.story.flag("ssAnneCaptainReward")).toBe(false)
      expect(await game.inventory.contains("cut")).toBe(false)
      expect(await game.inventory.contains("ssTicket")).toBe(true)
      await saveAndReload(game)
    }
  })

  it("keeps both map tables valid and locates Anne on the combined map", async () => {
    for (const visitedKanto of [false, true]) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { position: { map: "ss-anne-1f", x: 19, y: 2 } },
        story: { flags: { visitedKanto } },
      })
      const layout = visitedKanto ? "combined" : "johto"
      const position = visitedKanto ? { x: 24, y: 7 } : { x: 14, y: 9 }
      expect(await game.regionMap.entry(layout, 125)).toEqual({
        ...position,
        width: 1,
        height: 1,
      })
      expect(await game.regionMap.observePokedex()).toMatchObject({
        layout: "combined",
        playerMarker: { x: 25, y: 9 },
      })
    }
  })
})
