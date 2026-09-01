import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { catchWithMasterBallAndSwap } from "../playbooks/battle-catch-swap"
import {
  closePcStorage,
  depositPartyMon,
  moveSlot,
  openPcStorage,
  releaseSlot,
  switchStorageMode,
  walkFromCherrygrovePcToSurfShore,
  withdrawSlot,
} from "../playbooks/pc-storage"

describe.sequential("HNS HM battle and storage paths", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("catch-swaps the sole Surf user and loses real field access", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "route-41", x: 9, y: 21 } },
      party: [
        { species: "lapras", moves: ["surf"] },
        { species: "pidgey" },
        { species: "geodude" },
        { species: "onix" },
        { species: "chikorita" },
        { species: "zubat" },
      ],
      bag: { hms: { surf: 1 }, items: { masterBall: 1 } },
      pc: { currentBox: 0, observedSlots: [{ box: 0, slot: 0, mon: null }] },
      determinism: { textSpeed: "instant" },
    })

    await game.battle.startWild({ species: "cyndaquil", level: 7, moves: ["tackle"] })
    await expect(game.state.read()).resolves.toMatchObject({
      battle: {
        active: true,
        enemy: {
          species: "cyndaquil",
          level: 7,
          moves: ["tackle", "none", "none", "none"],
        },
      },
    })
    await catchWithMasterBallAndSwap(game, { outgoingPartyIndex: 0 })

    const postCatch = await game.state.read()
    expect(postCatch).toMatchObject({
      battle: {
        active: false,
        caughtSpecies: "cyndaquil",
        lastUsedItem: "masterBall",
        catchSwap: { state: "resolved", selectedParty: 0, box: 0, slot: 0 },
      },
      pc: {
        slots: [
          {
            box: 0,
            slot: 0,
            mon: { species: "lapras", moves: ["surf", "none", "none", "none"] },
          },
        ],
      },
    })
    expect(postCatch.party[0]).toMatchObject({ species: "cyndaquil" })
    expect(postCatch.party.map((mon) => mon.species)).not.toContain("lapras")

    await game.player.interact()
    await game.dialogue.waitForOpen()
    await expect(game.state.read()).resolves.toMatchObject({
      dialogue: { message: "field-move-no-eligible-mon" },
      fieldMove: { move: "surf", result: "no-eligible-mon" },
      player: { surfing: false },
    })
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.forReady()
  })

  it("deposits and withdraws the Surf user, then recovers real field access", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "cherrygrove-pokemon-center", x: 11, y: 2 },
      },
      party: [{ species: "lapras", moves: ["surf"] }, { species: "rattata" }],
      bag: { hms: { surf: 1 } },
      pc: { currentBox: 0, observedSlots: [{ box: 0, slot: 0, mon: null }] },
      determinism: { textSpeed: "instant" },
    })

    await openPcStorage(game, "deposit")
    await depositPartyMon(game, 0)
    const postDeposit = await game.state.read()
    await expect(game.storage.slot(0, 0)).resolves.toMatchObject({
      mon: {
        species: "lapras",
        moves: ["surf", "none", "none", "none"],
      },
    })
    expect(postDeposit.party[0]).toMatchObject({ species: "rattata" })
    expect(postDeposit.party.map((mon) => mon.species)).not.toContain("lapras")

    await switchStorageMode(game, "withdraw")
    await withdrawSlot(game, 0)
    await expect(game.storage.slot(0, 0)).resolves.toMatchObject({ mon: null })

    await closePcStorage(game)
    const postWithdraw = await game.state.read()
    expect(postWithdraw.party[1]).toMatchObject({
      species: "lapras",
      moves: ["surf", "none", "none", "none"],
    })
    expect(postWithdraw).toMatchObject({
      ready: true,
      map: { name: "cherrygrove-pokemon-center" },
      player: { x: 11, y: 2 },
      storage: { open: false },
    })
    await walkFromCherrygrovePcToSurfShore(game)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "cherrygrove-city" },
      player: { x: 25, y: 10, facing: "left" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await expect(game.state.read()).resolves.toMatchObject({
      dialogue: { message: "want-to-use-surf" },
      fieldMove: { move: "surf", result: "found", userSpecies: "lapras" },
    })
    await game.controls.press("b")
  })

  it("moves the Surf user between exact requested box slots", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "cherrygrove-pokemon-center", x: 11, y: 2 },
      },
      party: [{ species: "rattata" }, { species: "pidgey" }],
      pc: {
        currentBox: 0,
        observedSlots: [
          { box: 0, slot: 0, mon: { species: "lapras", moves: ["surf"] } },
          { box: 0, slot: 1, mon: null },
        ],
      },
      determinism: { textSpeed: "instant" },
    })

    await openPcStorage(game, "move")
    await moveSlot(game, 0, 1)
    await expect(game.storage.slot(0, 0)).resolves.toMatchObject({ mon: null })
    await expect(game.storage.slot(0, 1)).resolves.toMatchObject({
      mon: {
        species: "lapras",
        moves: ["surf", "none", "none", "none"],
      },
    })
    await closePcStorage(game)
  })

  it("releases a Surf user without applying an HM restriction", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "up",
        position: { map: "cherrygrove-pokemon-center", x: 11, y: 2 },
      },
      party: [{ species: "rattata" }, { species: "pidgey" }],
      pc: {
        currentBox: 0,
        observedSlots: [{ box: 0, slot: 0, mon: { species: "lapras", moves: ["surf"] } }],
      },
      determinism: { textSpeed: "instant" },
    })

    await openPcStorage(game, "move")
    await releaseSlot(game, 0)
    await expect(game.storage.slot(0, 0)).resolves.toMatchObject({ mon: null })
    await expect(game.state.read()).resolves.toMatchObject({
      storage: { ready: true, ui: "ready" },
    })
  })
})
