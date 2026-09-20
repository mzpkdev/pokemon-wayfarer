import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"
import { advanceOpeningUntil } from "../playbooks/regional-opening"

const vermilionDock = { map: "vermilion-port-inside", x: 8, y: 9 } as const
const billPosition = { map: "route25-bills-house", x: 5, y: 6, facing: "left" } as const

// This is deliberately a named state fixture rather than an inferred map or
// Trainer ID. It models the only terminal Anne state: every independently
// saved receipt, scene, and Trainer victory has already been claimed.
const allAnneContentFlags = [
  "ssAnneItemTm31", "ssAnneItemStardust", "ssAnneItemXAttack", "ssAnneItemTm44",
  "ssAnneItemEther", "ssAnneItemSuperPotion", "ssAnneItemGreatBall", "ssAnneCaptainRewarded",
  "ssAnneItemHyperPotion", "ssAnneItemChestoBerry", "ssAnneItemPechaBerry",
  "ssAnneItemCheriBerry", "ssAnneBlueMet", "ssAnneTrainerTyler", "ssAnneTrainerAnn",
  "ssAnneTrainerArthur", "ssAnneTrainerThomas", "ssAnneTrainerDale", "ssAnneTrainerBrooks",
  "ssAnneTrainerLamar", "ssAnneTrainerDawn", "ssAnneTrainerBarny", "ssAnneTrainerPhillip",
  "ssAnneTrainerHuey", "ssAnneTrainerDylan", "ssAnneTrainerLeonard", "ssAnneTrainerDuncan",
  "ssAnneTrainerEdmond", "ssAnneTrainerTrevor",
] as const

const settleField = async (game: GameSession, description: string): Promise<void> => {
  let readyFrames = 0
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.battle.active) {
      readyFrames += 12
      if (readyFrames >= 60) return
      await game.wait.frames(12)
      continue
    }
    readyFrames = 0
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || state.battle.active)
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not release the field: ${JSON.stringify(await game.state.read())}`)
}

const waitForMap = async (game: GameSession, map: GameMap, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 160; attempt++) {
    const state = await game.state.read()
    if (state.map.name === map && state.ready) return
    await game.wait.frames(20)
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

const chooseVermilionTopLevel = async (game: GameSession, result: 0 | 1 | 2 | 3) => {
  await game.player.interact()
  await game.wait.frames(30)
  for (let row = 0; row < result; row++) {
    await game.controls.press("down")
    await game.wait.frames(10)
  }
  await game.controls.press("a")
}

const dockTravelState = async (game: GameSession) => ({
  aquaState: await game.story.var("ssAquaState"),
  ticket: await game.inventory.contains("ssTicket"),
  origin: (await game.state.read()).origin,
})

const arrangeAtDock = async (game: GameSession, ticket: boolean) => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "down", position: vermilionDock },
    story: { vars: { ssAquaState: 0 } },
    bag: { items: ticket ? { ssTicket: 1 } : {} },
    determinism: { textSpeed: "instant" },
  })
}

const boardAnne = async (game: GameSession) => {
  await chooseVermilionTopLevel(game, 2)
  await game.wait.frames(30)
  await game.controls.press("a")
  await waitForMap(game, "ss-anne-1f-corridor", "S.S. Anne did not board from top-level result 2")
}

const rescueBill = async (game: GameSession, description: string) => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  // Bill's rescue prompt defaults to Yes. The remaining presentation and the
  // Ticket transaction are intentionally completed through the real script.
  await game.controls.press("a")
  await settleField(game, description)
}

const finishCaptainReward = async (game: GameSession, description: string) => {
  for (let attempt = 0; attempt < 360; attempt++) {
    if (await game.story.flag("ssAnneCaptainRewarded")) {
      await settleField(game, description)
      return
    }
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`${description} did not set the captain receipt flag`)
}

const finishCaptainNoRoom = async (game: GameSession) => {
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text.toLowerCase().includes("no room")) {
      await game.controls.press("a")
      await settleField(game, "captain full-pocket refusal")
      return
    }
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error("captain did not report a full TM/HM pocket")
}

describe.sequential("Wayfarer S.S. Anne persistent adventure", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("keeps regular Aqua unavailable, then handles no-ticket, Cancel, and B without travel state", async () => {
    await arrangeAtDock(game, true)
    const beforeOtherDestinations = await dockTravelState(game)
    await chooseVermilionTopLevel(game, 1)
    await settleField(game, "ineligible Other Destinations refusal")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: vermilionDock.map },
      player: { x: vermilionDock.x, y: vermilionDock.y },
    })
    expect(await dockTravelState(game)).toEqual(beforeOtherDestinations)

    await arrangeAtDock(game, false)
    const beforeNoTicket = await dockTravelState(game)
    await chooseVermilionTopLevel(game, 2)
    await settleField(game, "S.S. Anne ticket refusal")
    expect(await dockTravelState(game)).toEqual(beforeNoTicket)

    await arrangeAtDock(game, true)
    const beforeCancel = await dockTravelState(game)
    await chooseVermilionTopLevel(game, 3)
    await settleField(game, "top-level Cancel")
    expect(await dockTravelState(game)).toEqual(beforeCancel)

    await game.player.interact()
    await game.wait.frames(30)
    await game.controls.press("b")
    await settleField(game, "top-level B cancellation")
    expect(await dockTravelState(game)).toEqual(beforeCancel)
  })

  it("boards at the direct corridor coordinate, returns to the dock, and preserves the Ticket", async () => {
    await arrangeAtDock(game, true)
    const before = await dockTravelState(game)
    await boardAnne(game)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "ss-anne-1f-corridor" },
      player: { x: 19, y: 2 },
    })
    expect(await dockTravelState(game)).toEqual(before)

    await game.wait.frames(120)
    await settleField(game, "Anne entry map presentation")
    await game.saveAndReload()
    await game.controls.press("up")
    await waitForMap(game, vermilionDock.map, "Anne corridor exit did not return to Vermilion dock")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: vermilionDock.map },
      player: { x: vermilionDock.x, y: vermilionDock.y },
    })
    expect(await dockTravelState(game)).toEqual(before)

    await boardAnne(game)
    await game.player.warp("ss-anne-1f-corridor", 19, 2, "up")
    await game.controls.press("up")
    await waitForMap(game, vermilionDock.map, "second Anne corridor exit did not return to dock")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: vermilionDock.map },
      player: { x: vermilionDock.x, y: vermilionDock.y },
    })
    expect(await dockTravelState(game)).toEqual(before)
  })

  it("uses normal blackout recovery rather than registering Anne as a heal location, then permits re-entry", async () => {
    await arrangeAtDock(game, true)
    await boardAnne(game)
    await game.battle.startWild({ species: "pidgey", level: 2, moves: ["tackle"] })
    await game.battle.lose()
    await advanceOpeningUntil(
      game,
      (state) => state.ready && state.map.name !== "ss-anne-1f-corridor",
      "blackout did not leave the Anne corridor",
    )

    expect(await game.inventory.contains("ssTicket")).toBe(true)
    await game.player.warp(vermilionDock.map, vermilionDock.x, vermilionDock.y, "down")
    await boardAnne(game)
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 19, y: 2 } })
  })

  it("keeps the completed captain accessible while any independent Anne content remains", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: vermilionDock },
      story: { flags: { ssAnneCaptainRewarded: true } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await boardAnne(game)
    await expect(game.story.flag("ssAnneCaptainRewarded")).resolves.toBe(true)
  })

  it("refuses result 2 after every one-time Anne receipt is complete without touching transport state", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: vermilionDock },
      story: { vars: { ssAquaState: 0 } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })
    for (const flag of allAnneContentFlags) await game.story.setFlag(flag, true)
    const before = await dockTravelState(game)

    await chooseVermilionTopLevel(game, 2)
    await settleField(game, "completed Anne refusal")

    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: vermilionDock.map },
      player: { x: vermilionDock.x, y: vermilionDock.y },
    })
    expect(await dockTravelState(game)).toEqual(before)
  })

  it("keeps Blue hidden before his scene so the captain warp is not blocked", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "ss-anne-2f-corridor", x: 30, y: 3 } },
      determinism: { textSpeed: "instant" },
    })

    await expect(game.story.flag("ssAnneBlueMet")).resolves.toBe(false)
    await game.controls.press("up")
    await game.wait.frames(30)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "ss-anne-2f-corridor" },
      player: { x: 30, y: 2 },
      battle: { active: false },
    })
    await expect(game.story.flag("ssAnneBlueMet")).resolves.toBe(false)
  })

  it("plays Blue's visitor scene once without a battle and leaves the captain available", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "ss-anne-2f-corridor", x: 30, y: 7 } },
      determinism: { textSpeed: "instant" },
    })

    await game.controls.press("up")
    await game.wait.frames(30)
    await settleField(game, "Blue visitor scene")
    await expect(game.story.flag("ssAnneBlueMet")).resolves.toBe(true)
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
    await expect(game.story.flag("ssAnneCaptainRewarded")).resolves.toBe(false)

    await game.saveAndReload()
    await game.player.warp("ss-anne-2f-corridor", 30, 7, "up")
    await game.controls.press("up")
    await game.wait.frames(30)
    await settleField(game, "Blue post-visitor approach")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })

    await game.player.warp("ss-anne-captains-office", 5, 5, "up")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishCaptainReward(game, "captain remains accessible after Blue")
    await expect(game.story.flag("ssAnneCaptainRewarded")).resolves.toBe(true)
  })

  it("makes the captain's full TM/HM-pocket failure retry-safe and does not duplicate prior Cut", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "ss-anne-captains-office", x: 5, y: 5 } },
      bag: { fullPockets: ["tmHm"] },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishCaptainNoRoom(game)
    await expect(game.story.flag("ssAnneCaptainRewarded")).resolves.toBe(false)
    await expect(game.inventory.contains("cut")).resolves.toBe(false)

    await game.saveAndReload()
    await game.inventory.freeSlot("tmHm")
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishCaptainReward(game, "captain Cut retry")
    await expect(game.story.flag("ssAnneCaptainRewarded")).resolves.toBe(true)
    await expect(game.inventory.contains("cut")).resolves.toBe(true)

    await game.saveAndReload()
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await game.wait.frames(30)
    await settleField(game, "captain post-reward interaction")
    await expect(game.inventory.contains("cut")).resolves.toBe(true)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "ss-anne-captains-office", x: 5, y: 5 } },
      bag: { hms: { cut: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await finishCaptainReward(game, "captain prior-Cut resolution")
    await expect(game.story.flag("ssAnneCaptainRewarded")).resolves.toBe(true)
    await expect(game.inventory.contains("cut")).resolves.toBe(true)
  })

  it("keeps a failed item ball available for a saved full-pocket retry", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "right", position: { map: "ss-anne-1f-room2", x: 4, y: 7 } },
      bag: { fullPockets: ["tmHm"] },
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    await settleField(game, "TM31 full-pocket refusal")
    await expect(game.story.flag("ssAnneItemTm31")).resolves.toBe(false)
    await expect(game.inventory.contains("tmBrickBreak")).resolves.toBe(false)

    await game.saveAndReload()
    await game.inventory.freeSlot("tmHm")
    await game.player.interact()
    await game.wait.frames(30)
    await settleField(game, "TM31 retry")
    await expect(game.story.flag("ssAnneItemTm31")).resolves.toBe(true)
    await expect(game.inventory.contains("tmBrickBreak")).resolves.toBe(true)
  })

  it("persists a defeated Anne Trainer without a rematch after saving and reloading", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: { map: "ss-anne-1f-room2", x: 0, y: 3 } },
      party: [{ species: "lapras", level: 100, moves: ["surf"] }],
      determinism: { textSpeed: "instant" },
    })

    await game.player.interact()
    for (let attempt = 0; attempt < 360; attempt++) {
      if ((await game.state.read()).battle.ui === "action-menu") break
      await game.controls.press("a")
      await game.wait.frames(12)
    }
    await expect(game.state.read()).resolves.toMatchObject({ battle: { ui: "action-menu" } })
    await game.battle.win()
    await settleField(game, "Tyler victory")
    await expect(game.story.flag("ssAnneTrainerTyler")).resolves.toBe(true)

    await game.saveAndReload()
    await game.player.interact()
    await settleField(game, "Tyler post-victory dialogue")
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false } })
  })

  it("rescues Bill locally and grants one retry-safe S.S. Ticket", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: billPosition, facing: billPosition.facing },
      determinism: { textSpeed: "instant" },
    })

    await rescueBill(game, "Bill rescue and ticket handoff")
    await expect(game.story.flag("billRescued")).resolves.toBe(true)
    await expect(game.story.flag("billSsTicketSettled")).resolves.toBe(true)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)

    await game.saveAndReload()
    await game.player.interact()
    await settleField(game, "Bill post-ticket dialogue")
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)
  })

  it("lets a Ticket holder board first, then rescue Bill without a duplicate", async () => {
    await arrangeAtDock(game, true)
    await boardAnne(game)
    await game.player.warp(billPosition.map, billPosition.x, billPosition.y, billPosition.facing)

    await rescueBill(game, "Bill rescue with a pre-existing Ticket")
    await expect(game.story.flag("billRescued")).resolves.toBe(true)
    await expect(game.story.flag("billSsTicketSettled")).resolves.toBe(true)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)
    await expect(game.story.var("ssAquaState")).resolves.toBe(0)
  })

  it("keeps Bill's Ticket pending while Key Items are full, then grants it after a saved retry", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: billPosition, facing: billPosition.facing },
      bag: { fullPockets: ["keyItems"] },
      determinism: { textSpeed: "instant" },
    })

    await rescueBill(game, "Bill full-Key-Items response")
    await expect(game.story.flag("billRescued")).resolves.toBe(true)
    await expect(game.story.flag("billSsTicketSettled")).resolves.toBe(false)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(false)

    await game.saveAndReload()
    await game.inventory.freeSlot("keyItems")
    await game.player.interact()
    await settleField(game, "Bill Ticket retry")
    await expect(game.story.flag("billSsTicketSettled")).resolves.toBe(true)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)
  })
})
