import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, seviiImportedMaps, type GameMap } from "../harness/game-session"

const vermilionDock = { map: "vermilion-port-inside", x: 8, y: 9 } as const

const waitForFerry = async (game: GameSession, destination: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 100; attempt++) {
    const state = await game.state.read()
    // A ferry warp reports its target map before overworld controls are ready.
    // Do not press A during that transition: on arrival the player faces the
    // harbor sailor and would immediately select a second destination.
    if (state.map.name === destination) {
      await game.wait.forMap(destination)
      return
    }
    await game.wait.frames(30)
    await game.controls.press("a")
  }
  throw new Error(
    `Seagallop did not reach ${destination}: ${JSON.stringify(await game.state.read())}`,
  )
}

const openSeviiMenu = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.wait.frames(30)
  // SEVII ISLANDS is the fixed first result of the Wayfarer-only port menu.
  await game.controls.press("a")
  await game.wait.frames(30)
}

const chooseSeviiRow = async (game: GameSession, row: number): Promise<void> => {
  for (let index = 0; index < row; index++) {
    await game.controls.press("down")
    await game.wait.frames(10)
  }
  await game.controls.press("a")
}

const arrangeAtVermilion = async (
  game: GameSession,
  options: {
    aqua?: boolean
    auroraTicket?: boolean
    mysticTicket?: boolean
    gameClear?: boolean
  } = {},
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "down", position: vermilionDock },
    story: {
      vars: { ssAquaState: options.aqua ? 8 : 0 },
      flags: { systemGameClear: options.gameClear ?? false },
    },
    bag: {
      items: {
        ...(options.aqua ? { ssTicket: 1 } : {}),
        ...(options.auroraTicket ? { auroraTicket: 1 } : {}),
        ...(options.mysticTicket ? { mysticTicket: 1 } : {}),
      },
    },
    determinism: { textSpeed: "instant" },
  })
}

const dockSnapshot = async (game: GameSession) => ({
  aquaState: await game.story.var("ssAquaState"),
  gameClear: await game.story.flag("systemGameClear"),
  ssTicket: await game.inventory.contains("ssTicket"),
  auroraTicket: await game.inventory.contains("auroraTicket"),
  mysticTicket: await game.inventory.contains("mysticTicket"),
  origin: (await game.state.read()).origin,
})

describe.sequential("Wayfarer Sevii exploration", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("loads every map in the frozen 135-map imported catalog", async () => {
    expect(seviiImportedMaps).toHaveLength(135)

    for (const location of seviiImportedMaps) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: location },
        determinism: { textSpeed: "instant" },
      })
      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: location.map },
      })
    }
  })

  it("loads representative numbered-island, dungeon, service, and tower maps", async () => {
    const locations = [
      { map: "sevii-one-island", x: 12, y: 18 },
      { map: "sevii-four-island-pokemon-center-1-f", x: 7, y: 8 },
      { map: "sevii-mt-ember-exterior", x: 13, y: 39 },
      { map: "sevii-four-island-icefall-cave-entrance", x: 14, y: 8 },
      { map: "sevii-five-island-lost-cave-entrance", x: 5, y: 5 },
      { map: "sevii-seven-island-sevault-canyon-tanoby-key", x: 7, y: 8 },
      { map: "sevii-trainer-tower-lobby", x: 8, y: 13 },
    ] as const

    for (const location of locations) {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: location },
        determinism: { textSpeed: "instant" },
      })
      await expect(game.state.read()).resolves.toMatchObject({
        ready: true,
        map: { name: location.map },
      })
    }
  })

  it("sails to One Island without an Aqua credential and preserves the regular route state", async () => {
    await arrangeAtVermilion(game)
    const before = await dockSnapshot(game)

    await openSeviiMenu(game)
    await chooseSeviiRow(game, 0)
    await waitForFerry(game, "sevii-one-island-harbor")

    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "sevii-one-island-harbor" },
      player: { x: 8, y: 5 },
    })
    expect(await game.story.var("ssAquaState")).toBe(before.aquaState)
    expect(await game.inventory.contains("ssTicket")).toBe(false)
  })

  it("cancels the top-level dock selector without moving or changing travel state", async () => {
    await arrangeAtVermilion(game, { aqua: true, auroraTicket: true, gameClear: true })
    const before = await dockSnapshot(game)

    await game.player.interact()
    await game.wait.frames(30)
    await game.controls.press("b")
    await game.wait.forReady()

    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: vermilionDock.map },
      player: { x: vermilionDock.x, y: vermilionDock.y },
    })
    expect(await dockSnapshot(game)).toEqual(before)
  })

  it("hides Birth Island unless both the regular Aqua predicate and Aurora Ticket hold", async () => {
    for (const options of [
      { aqua: false, auroraTicket: false },
      { aqua: false, auroraTicket: true },
      { aqua: true, auroraTicket: false },
    ]) {
      await arrangeAtVermilion(game, options)
      const before = await dockSnapshot(game)
      await openSeviiMenu(game)
      // The only row after ONE ISLAND is CANCEL when Birth Island is hidden.
      await chooseSeviiRow(game, 1)
      await game.wait.forReady()
      await expect(game.state.read()).resolves.toMatchObject({ map: { name: vermilionDock.map } })
      expect(await dockSnapshot(game)).toEqual(before)
    }

    await arrangeAtVermilion(game, { aqua: true, auroraTicket: true })
    const before = await dockSnapshot(game)
    await openSeviiMenu(game)
    await chooseSeviiRow(game, 1)
    await waitForFerry(game, "birth-island-harbor")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "birth-island-harbor" },
      player: { x: 8, y: 5 },
    })
    await game.saveAndReload()
    await game.wait.forMap("birth-island-harbor")
    expect(await dockSnapshot(game)).toEqual(before)
  })

  it("hides Navel Rock unless both the League-clear flag and Mystic Ticket hold", async () => {
    for (const options of [
      { gameClear: false, mysticTicket: false },
      { gameClear: false, mysticTicket: true },
      { gameClear: true, mysticTicket: false },
    ]) {
      await arrangeAtVermilion(game, options)
      const before = await dockSnapshot(game)
      await openSeviiMenu(game)
      // No Aurora Ticket is arranged, so the second row is CANCEL if Navel is hidden.
      await chooseSeviiRow(game, 1)
      await game.wait.forReady()
      await expect(game.state.read()).resolves.toMatchObject({ map: { name: vermilionDock.map } })
      expect(await dockSnapshot(game)).toEqual(before)
    }

    await arrangeAtVermilion(game, { gameClear: true, mysticTicket: true })
    const before = await dockSnapshot(game)
    await openSeviiMenu(game)
    await chooseSeviiRow(game, 1)
    await waitForFerry(game, "navel-rock-harbor")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "navel-rock-harbor" },
      player: { x: 8, y: 5 },
    })
    await game.saveAndReload()
    await game.wait.forMap("navel-rock-harbor")
    const after = await dockSnapshot(game)
    expect({ ...after, origin: undefined }).toEqual({ ...before, origin: undefined })
    // Navel Rock is existing Emerald content. Loading it has always selected
    // Wayfarer's Hoenn context; the Vermilion connection must not rewrite that
    // legacy region-map/Fly classification.
    expect(after.origin).toMatchObject({
      ...before.origin,
      currentRegion: 3,
      visitedRegions: before.origin.visitedRegions | 4,
    })
  })
})
