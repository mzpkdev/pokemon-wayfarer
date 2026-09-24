import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction, type GameMap } from "../harness/game-session"
import { catchWithMasterBallAndSwap } from "../playbooks/battle-catch-swap"

const lobby = "power-plant-entrance" as const
const hall = "power-plant-old-hall" as const
type ArrangeOptions = Parameters<GameSession["arrange"]>[0]

const advanceUntil = async (
  game: GameSession,
  predicate: (state: Awaited<ReturnType<GameSession["state"]["read"]>>) => boolean,
  description: string,
  maxAttempts = 300,
): Promise<void> => {
  for (let attempt = 0; attempt < maxAttempts; attempt++) {
    const state = await game.state.read()
    if (predicate(state)) return
    if (state.battle.active && state.battle.ui === "action-menu") await game.controls.press("a")
    else if (state.battle.active && state.battle.ui === "move-menu") await game.controls.press("a")
    else if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} was not reached: ${JSON.stringify(await game.state.read())}`)
}

const settleField = (game: GameSession, description: string): Promise<void> =>
  advanceUntil(
    game,
    (state) => state.ready && !state.scriptActive && !state.dialogueOpen && !state.battle.active,
    description,
  )

const waitForBattle = (game: GameSession, description: string): Promise<void> =>
  advanceUntil(game, (state) => state.battle.ui === "action-menu", `${description} action menu`)

const arrangeAt = async (
  game: GameSession,
  position: { map: GameMap; x: number; y: number },
  facing: Direction,
  options: {
    flags?: NonNullable<ArrangeOptions["story"]>["flags"]
    vars?: NonNullable<ArrangeOptions["story"]>["vars"]
    encounters?: boolean
    fullPocket?: "items" | "tmHm"
    party?: ArrangeOptions["party"]
    items?: NonNullable<ArrangeOptions["bag"]>["items"]
  } = {},
): Promise<void> => {
  const arrangePosition = position.map === hall ? ({ map: lobby, x: 4, y: 20 } as const) : position
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { position: arrangePosition, facing },
    story: {
      flags: {
        disableEncounters: !options.encounters,
        ...options.flags,
      },
      vars: options.vars,
    },
    party: options.party ?? [{ species: "lapras", level: 100, moves: ["tackle"] }],
    bag: {
      items: options.items,
      fullPockets: options.fullPocket ? [options.fullPocket] : [],
    },
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
  if (position.map === hall) {
    // Load the hall at the requested camera position. A same-map injected
    // warp does not instantiate far-away object templates in the new view.
    await game.player.warp(hall, position.x, position.y, facing)
    await game.wait.forReady()
  }
}

const waitForWorkerOffer = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await advanceUntil(
    game,
    (state) => state.dialogue.text.toLowerCase().includes("old generating hall"),
    "old hall worker offer",
  )
  await game.wait.frames(60)
  await game.controls.press("a")
  await game.wait.frames(60)
}

const chooseWorkerOffer = async (game: GameSession, enter: boolean): Promise<void> => {
  await waitForWorkerOffer(game)
  if (!enter) await game.controls.press("down")
  await game.controls.press("a")
}

const walkToMap = async (
  game: GameSession,
  from: { map: GameMap; x: number; y: number; facing: Direction },
  direction: Direction,
  destination: GameMap,
): Promise<void> => {
  await game.player.warp(from.map, from.x, from.y, from.facing)
  await game.wait.forMap(from.map)
  await game.wait.forReady()
  for (let attempt = 0; attempt < 4; attempt++) {
    if ((await game.state.read()).map.name === destination) break
    await game.player.move(direction)
    await game.wait.frames(45)
  }
  await game.wait.until(
    (state) => state.map.name === destination,
    `${from.map} ${from.x}:${from.y} to ${destination}`,
    300,
  )
  await game.wait.forReady()
}

const crossWarpRecord = async (
  game: GameSession,
  warp: { x: number; y: number },
  inward: Direction,
  outward: Direction,
): Promise<void> => {
  await game.player.warp(hall, warp.x, warp.y, inward)
  await game.wait.forReady()
  await game.player.move(inward)
  await game.wait.frames(30)
  expect((await game.state.read()).map.name).toBe(hall)
  for (let attempt = 0; attempt < 3; attempt++) {
    if ((await game.state.read()).map.name === lobby) break
    await game.player.move(outward)
    await game.wait.frames(30)
  }
  await game.wait.until(
    (state) => state.map.name === lobby,
    `Power Plant warp record ${warp.x}:${warp.y}`,
    300,
  )
  await game.wait.forReady()
}

const pickup = async (
  game: GameSession,
  location: { x: number; y: number; facing: Direction },
  description: string,
): Promise<void> => {
  await game.player.warp(hall, location.x, location.y, location.facing)
  await game.wait.forReady()
  await game.player.interact()
  await advanceUntil(
    game,
    (state) => state.dialogueOpen || state.scriptActive,
    `${description} script start`,
  )
  await settleField(game, description)
}

const walkOneTile = async (game: GameSession, direction: Direction): Promise<void> => {
  const before = await game.state.read()
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(15)
    const after = await game.state.read()
    if (after.player.x !== before.player.x || after.player.y !== before.player.y) {
      expect(
        Math.abs(after.player.x - before.player.x) + Math.abs(after.player.y - before.player.y),
      ).toBe(1)
      return
    }
  }
  throw new Error(
    `Power Plant route blocked moving ${direction} from ${before.player.x}:${before.player.y}`,
  )
}

const interactWithZapdos = async (game: GameSession, description: string): Promise<void> => {
  for (const location of [
    { x: 5, y: 10, facing: "down" },
    { x: 5, y: 12, facing: "up" },
    { x: 6, y: 11, facing: "left" },
    { x: 4, y: 11, facing: "right" },
  ] as const) {
    await game.player.warp(hall, location.x, location.y, location.facing)
    await game.wait.forReady()
    const sequence = (await game.state.read()).dialogue.sequence
    await game.player.interact()
    await game.wait.frames(60)
    const state = await game.state.read()
    if (state.battle.active || state.dialogue.sequence > sequence || state.scriptActive) return
  }
  throw new Error(`${description} could not interact with Zapdos`)
}

const startZapdos = async (game: GameSession, description: string): Promise<void> => {
  await interactWithZapdos(game, description)
  await waitForBattle(game, description)
  expect((await game.state.read()).battle.enemy).toMatchObject({ species: "zapdos", level: 50 })
}

const reenterHall = async (game: GameSession): Promise<void> => {
  await game.player.warp(lobby, 4, 20, "down")
  await game.wait.forReady()
  await game.player.warp(hall, 5, 10, "down")
  await game.wait.forReady()
}

const runFromBattle = async (game: GameSession): Promise<void> => {
  await waitForBattle(game, "Zapdos Run")
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (cursor % 2 !== 1) await game.controls.press("right")
  if (Math.floor(cursor / 2) !== 1) await game.controls.press("down")
  await game.controls.press("a")
  await settleField(game, "Zapdos Run return")
}

const useBattleTeleport = async (game: GameSession): Promise<void> => {
  await waitForBattle(game, "Zapdos Teleport")
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (cursor % 2 === 1) await game.controls.press("left")
  if (Math.floor(cursor / 2) === 1) await game.controls.press("up")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "move-menu", "Zapdos move menu", 1_200)
  await game.controls.press("a")
  await settleField(game, "Zapdos Teleport return")
}

describe.sequential("Wayfarer Power Plant old generating hall", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("preserves the worker decline and enters before, during, and after repair", async () => {
    for (const phase of [
      { state: 0, repaired: false },
      { state: 4, repaired: false },
      { state: 7, repaired: true },
    ]) {
      await arrangeAt(game, { map: lobby, x: 3, y: 13 }, "up", {
        flags: { returnedMachinePart: phase.repaired },
        vars: { kantoRocketStoryState: phase.state },
      })
      await chooseWorkerOffer(game, false)
      await settleField(game, `worker decline at repair state ${phase.state}`)
      expect((await game.state.read()).map.name).toBe(lobby)
      expect(await game.story.flag("returnedMachinePart")).toBe(phase.repaired)

      await chooseWorkerOffer(game, true)
      await game.wait.forMap(hall)
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: hall },
        player: { x: 5, y: 36 },
      })
      expect(await game.story.flag("powerPlantVisited")).toBe(true)
      expect(await game.story.flag("returnedMachinePart")).toBe(phase.repaired)
      await walkToMap(game, { map: hall, x: 5, y: 36, facing: "down" }, "down", lobby)
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: lobby },
        player: { x: 4, y: 20 },
      })
      expect(await game.story.flag("returnedMachinePart")).toBe(phase.repaired)
    }
  })

  it("preserves Lorenzo's existing Magneton trade offer", async () => {
    await arrangeAt(game, { map: lobby, x: 18, y: 15 }, "down")
    await game.player.interact()
    await game.wait.until(
      (state) => state.dialogue.text.toLowerCase().includes("i work here"),
      "Lorenzo opening trade offer",
    )
    // Three-paragraph offer; the harness exposes only the first rendered line.
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.frames(60)
    await game.controls.press("down")
    await game.controls.press("a")
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.until((state) => state.ready, "Lorenzo trade decline", 600)
  })

  it("completes Lorenzo's unchanged Dugtrio-for-Magneton trade", async () => {
    await arrangeAt(game, { map: lobby, x: 18, y: 15 }, "down", {
      flags: { powerPlantMagnetonTradeCompleted: false },
      party: [{ species: "dugtrio", level: 30, moves: ["tackle"] }],
    })
    await game.player.interact()
    await game.wait.until(
      (state) => state.dialogue.text.toLowerCase().includes("i work here"),
      "Lorenzo opening trade offer",
    )
    for (let page = 0; page < 3; page++) {
      await game.wait.frames(60)
      await game.controls.press("a")
    }
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.storage.waitForReady()
    for (let row = 0; row < 5; row++) {
      await game.controls.press("down")
      await game.wait.frames(30)
    }
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.storage.ready && state.storage.cursor.area === 1,
      "Lorenzo party selector",
    )
    await game.controls.press("a")
    await game.wait.until((state) => state.storage.ui === "mon-menu", "Lorenzo Dugtrio menu")
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.storage.waitForClosed()
    for (let attempt = 0; attempt < 600; attempt++) {
      if (await game.story.flag("powerPlantMagnetonTradeCompleted")) break
      await game.wait.frames(30)
      await game.controls.press("a")
    }
    expect(await game.story.flag("powerPlantMagnetonTradeCompleted")).toBe(true)
    expect((await game.state.read()).party.some((mon) => mon.species === "magneton")).toBe(true)
  })

  it("traverses the staffed lobby and keeps the back-room west exit repair-gated", async () => {
    await arrangeAt(game, { map: lobby, x: 16, y: 3 }, "right", {
      flags: { hidePowerPlantEngineer: false, returnedMachinePart: false },
      vars: { kantoRocketStoryState: 6 },
    })
    await walkToMap(
      game,
      { map: lobby, x: 16, y: 3, facing: "right" },
      "right",
      "power-plant-back-room",
    )
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "power-plant-back-room" },
      player: { x: 12, y: 12 },
    })
    await walkOneTile(game, "up")
    for (let attempt = 0; attempt < 4 && (await game.state.read()).map.name !== lobby; attempt++) {
      await game.player.move("down")
      await game.wait.frames(45)
    }
    await game.wait.forMap(lobby)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: lobby },
      player: { x: 17, y: 4 },
    })

    await game.player.warp("power-plant-back-room", 3, 4, "left")
    await game.wait.forReady()
    await game.player.move("left")
    await game.wait.frames(30)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "power-plant-back-room" },
      player: { x: 3, y: 4 },
    })
    await game.player.interact()
    await settleField(game, "back-room engineer block")
    expect((await game.state.read()).map.name).toBe("power-plant-back-room")

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: "power-plant-back-room", x: 3, y: 4 }, facing: "left" },
      story: {
        flags: { disableEncounters: true, hidePowerPlantEngineer: true, returnedMachinePart: true },
        vars: { kantoRocketStoryState: 7 },
      },
      determinism: { textSpeed: "instant", rngSeed: 1 },
    })
    await walkToMap(
      game,
      { map: "power-plant-back-room", x: 3, y: 4, facing: "left" },
      "left",
      "route-10",
    )
  })

  it("retries the Machine Part and manager reward with full pockets after hall exploration", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: { map: lobby, x: 3, y: 13 }, facing: "up" },
      story: {
        flags: {
          disableEncounters: true,
          powerPlantVisited: false,
          returnedMachinePart: false,
          hiddenMachinePart: false,
          hidePowerPlantEngineer: false,
          hideRoute25Misty: true,
        },
        vars: { kantoRocketStoryState: 5, ceruleanCityState: 0 },
      },
      bag: { fullPockets: ["keyItems", "tmHm"] },
      party: [{ species: "lapras", level: 100, moves: ["tackle"] }],
      determinism: { textSpeed: "instant", rngSeed: 1 },
    })
    await chooseWorkerOffer(game, true)
    await game.wait.forMap(hall)
    expect(await game.story.flag("powerPlantVisited")).toBe(true)
    await walkToMap(game, { map: hall, x: 5, y: 36, facing: "down" }, "down", lobby)

    await game.player.warp("cerulean-gym", 6, 13, "left")
    await game.wait.forReady()
    await game.player.interact()
    await settleField(game, "full Key Items Machine Part")
    expect(await game.inventory.contains("machinePart")).toBe(false)
    expect(await game.story.flag("hiddenMachinePart")).toBe(false)
    expect(await game.story.var("kantoRocketStoryState")).toBe(5)
    await game.saveAndReload()
    await game.inventory.freeSlot("keyItems")
    await game.player.interact()
    await settleField(game, "Machine Part retry")
    expect(await game.inventory.contains("machinePart")).toBe(true)
    expect(await game.story.flag("hiddenMachinePart")).toBe(true)
    expect(await game.story.var("kantoRocketStoryState")).toBe(6)

    await game.player.warp("power-plant-back-room", 11, 6, "up")
    await game.wait.forReady()
    await game.player.interact()
    await settleField(game, "full TM/HM manager reward")
    expect(await game.inventory.contains("machinePart")).toBe(true)
    expect(await game.inventory.contains("tmThunder")).toBe(false)
    expect(await game.story.flag("returnedMachinePart")).toBe(false)
    expect(await game.story.var("kantoRocketStoryState")).toBe(6)
    await game.saveAndReload()
    await game.inventory.freeSlot("tmHm")
    await game.player.interact()
    await settleField(game, "manager reward retry")
    expect(await game.inventory.contains("machinePart")).toBe(false)
    expect(await game.inventory.contains("tmThunder")).toBe(true)
    expect(await game.story.flag("returnedMachinePart")).toBe(true)
    expect(await game.story.flag("hidePowerPlantEngineer")).toBe(true)
    expect(await game.story.var("kantoRocketStoryState")).toBe(7)
    expect(await game.story.var("ceruleanCityState")).toBe(2)
    expect(await game.story.flag("powerPlantVisited")).toBe(true)
  })

  it("walks continuously from the worker entrance through the hall to Zapdos", async () => {
    await arrangeAt(game, { map: lobby, x: 3, y: 13 }, "up", {
      flags: { powerPlantZapdosHidden: false, powerPlantZapdosResolved: false },
      vars: { trainerRating: 54 },
    })
    await chooseWorkerOffer(game, true)
    await game.wait.forMap(hall)
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 5, y: 36 } })
    const route: readonly [Direction, number][] = [
      ["up", 5],
      ["right", 1],
      ["up", 5],
      ["left", 3],
      ["up", 4],
      ["right", 2],
      ["up", 3],
      ["right", 9],
      ["down", 4],
      ["left", 1],
      ["down", 3],
      ["right", 6],
      ["down", 5],
      ["right", 2],
      ["up", 1],
      ["right", 7],
      ["down", 1],
      ["right", 1],
      ["down", 1],
      ["right", 2],
      ["up", 1],
      ["right", 2],
      ["up", 1],
      ["right", 5],
      ["down", 1],
      ["right", 2],
      ["up", 1],
      ["right", 5],
      ["up", 14],
      ["left", 13],
      ["up", 11],
      ["left", 8],
      ["down", 7],
      ["left", 5],
      ["up", 1],
      ["left", 2],
      ["up", 1],
      ["left", 1],
      ["up", 1],
      ["left", 1],
      ["up", 3],
      ["left", 6],
      ["down", 1],
      ["left", 4],
      ["down", 3],
    ]
    for (const [direction, count] of route)
      for (let step = 0; step < count; step++) await walkOneTile(game, direction)
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 5, y: 10 } })
    await interactWithZapdos(game, "controller-walked Zapdos")
    await advanceUntil(
      game,
      (state) => state.dialogue.text.toLowerCase().includes("watches you warily"),
      "controller-walked Zapdos refusal",
    )
    await settleField(game, "controller-walked Zapdos refusal")
  })

  it("returns safely through all five source exit records and survives reload", async () => {
    await arrangeAt(game, { map: hall, x: 5, y: 36 }, "down")
    const exits = [
      [{ x: 4, y: 37 }, "up", "down"],
      [{ x: 5, y: 37 }, "up", "down"],
      [{ x: 6, y: 37 }, "up", "down"],
      [{ x: 1, y: 11 }, "right", "left"],
      [{ x: 1, y: 13 }, "right", "left"],
    ] as const
    for (const [warp, inward, outward] of exits) {
      await crossWarpRecord(game, warp, inward, outward)
      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: lobby },
        player: { x: 4, y: 20 },
      })
    }
    await game.saveAndReload()
    await expect(game.state.read()).resolves.toMatchObject({ map: { name: lobby } })
  })

  it("uses an Escape Rope from the old hall to return to the staffed lobby", async () => {
    await arrangeAt(game, { map: hall, x: 20, y: 20 }, "down", {
      items: { escapeRope: 1 },
    })
    await game.controls.press("start")
    await game.wait.until((state) => state.ui.mode === "pause-menu", "open pause menu")
    await game.wait.frames(60)
    // The checkpoint has a party but no Pokédex, so Bag is the second entry.
    await game.controls.press("down")
    await game.controls.press("a")
    await game.wait.frames(120)
    for (let pocket = 0; pocket < 3; pocket++) {
      await game.controls.press("right")
      await game.wait.frames(30)
    }
    for (let slot = 0; slot < 20; slot++) await game.controls.press("up")
    // Escape Rope is the only item in the ordinary Items pocket.
    await game.controls.press("a")
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.map.name === lobby,
      "Escape Rope return to the Power Plant lobby",
      3_600,
    )
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: lobby },
      player: { x: 4, y: 20 },
    })
  })

  it("selects ordinary Power Plant encounters while traversing live floor tiles", async () => {
    await arrangeAt(game, { map: hall, x: 3, y: 11 }, "right", {
      encounters: true,
      flags: { powerPlantZapdosHidden: true, powerPlantZapdosResolved: true },
    })
    const directions = ["right", "down", "left", "up"] as const
    let movedSteps = 0
    for (let step = 0; step < 500; step++) {
      const before = await game.state.read()
      if (before.phase === "battle" || before.battle.ui !== "none") break
      if (!before.ready) {
        await game.wait.frames(12)
        continue
      }
      const direction = directions[step % directions.length]!
      for (let attempt = 0; attempt < 2; attempt++) {
        const current = await game.state.read()
        if (current.phase === "battle" || current.battle.ui !== "none") break
        await game.player.move(direction)
        await game.wait.frames(15)
      }
      const state = await game.state.read()
      if (state.player.x !== before.player.x || state.player.y !== before.player.y) movedSteps++
    }
    expect(movedSteps).toBeGreaterThan(0)
    await game.wait.until((state) => state.battle.active, "Power Plant wild encounter", 300)
    await waitForBattle(game, "Power Plant wild encounter")
    expect(["voltorb", "magnemite", "pikachu", "magneton", "electabuzz"]).toContain(
      (await game.state.read()).battle.enemy?.species,
    )
    await game.battle.win()
    await settleField(game, "Power Plant wild encounter victory")
  })

  it("persists all five visible items and retries a full TM/HM-pocket pickup", async () => {
    await arrangeAt(game, { map: hall, x: 40, y: 23 }, "up", {
      fullPocket: "tmHm",
      flags: {
        powerPlantMaxPotionClaimed: false,
        powerPlantTmProtectClaimed: false,
        powerPlantTmThunderClaimed: false,
        powerPlantThunderStoneClaimed: false,
        powerPlantElixirClaimed: false,
      },
    })
    await pickup(game, { x: 40, y: 23, facing: "up" }, "full-pocket TM Protect")
    expect(await game.story.flag("powerPlantTmProtectClaimed")).toBe(false)
    expect(await game.inventory.contains("tmProtect")).toBe(false)
    await game.saveAndReload()
    await game.inventory.freeSlot("tmHm")
    await pickup(game, { x: 40, y: 23, facing: "up" }, "TM Protect retry")
    expect(await game.story.flag("powerPlantTmProtectClaimed")).toBe(true)
    expect(await game.inventory.contains("tmProtect")).toBe(true)

    for (const entry of [
      [{ x: 7, y: 28, facing: "up" }, "powerPlantMaxPotionClaimed", "maxPotion"],
      [{ x: 45, y: 5, facing: "up" }, "powerPlantThunderStoneClaimed", "thunderStone"],
      [{ x: 26, y: 23, facing: "up" }, "powerPlantElixirClaimed", "elixir"],
    ] as const) {
      const [location, flag, item] = entry
      await pickup(game, location, item)
      expect(await game.story.flag(flag), `${item} object flag`).toBe(true)
      expect(await game.inventory.contains(item), `${item} inventory`).toBe(true)
    }
    // Change maps before jumping to the remote southeast pickup so its
    // object template is instantiated in the destination camera viewport.
    await game.inventory.freeSlot("tmHm")
    await game.player.warp(lobby, 4, 20, "down")
    await game.wait.forReady()
    for (const location of [
      { x: 46, y: 36, facing: "down" },
      { x: 46, y: 38, facing: "up" },
      { x: 45, y: 37, facing: "right" },
      { x: 47, y: 37, facing: "left" },
    ] as const) {
      await game.player.warp(hall, location.x, location.y, location.facing)
      await game.wait.forReady()
      const sequence = (await game.state.read()).dialogue.sequence
      await game.player.interact()
      await game.wait.frames(60)
      if ((await game.state.read()).dialogue.sequence > sequence)
        await settleField(game, "TM Thunder")
      if (await game.story.flag("powerPlantTmThunderClaimed")) break
    }
    expect(await game.story.flag("powerPlantTmThunderClaimed")).toBe(true)
    expect(await game.inventory.contains("tmThunder")).toBe(true)
    expect(await game.story.flag("powerPlantTmProtectClaimed")).toBe(true)
    await game.saveAndReload()
    for (const flag of [
      "powerPlantMaxPotionClaimed",
      "powerPlantTmProtectClaimed",
      "powerPlantTmThunderClaimed",
      "powerPlantThunderStoneClaimed",
      "powerPlantElixirClaimed",
    ] as const)
      expect(await game.story.flag(flag)).toBe(true)
  })

  it("persists both hidden items and both Electrode decoys", async () => {
    await arrangeAt(game, { map: hall, x: 8, y: 13 }, "up", {
      fullPocket: "items",
      flags: {
        powerPlantElectrode1Resolved: false,
        powerPlantElectrode2Resolved: false,
        powerPlantHiddenMaxElixirClaimed: false,
        powerPlantHiddenThunderStoneClaimed: false,
      },
    })
    await pickup(game, { x: 8, y: 13, facing: "up" }, "full-pocket hidden Thunder Stone")
    expect(await game.story.flag("powerPlantHiddenThunderStoneClaimed")).toBe(false)
    expect(await game.inventory.contains("thunderStone")).toBe(false)
    await game.saveAndReload()
    await game.inventory.freeSlot("items")
    await pickup(game, { x: 8, y: 13, facing: "up" }, "hidden Thunder Stone retry")
    expect(await game.story.flag("powerPlantHiddenThunderStoneClaimed")).toBe(true)
    expect(await game.inventory.contains("thunderStone")).toBe(true)
    await pickup(game, { x: 29, y: 17, facing: "up" }, "hidden Max Elixir")
    expect(await game.story.flag("powerPlantHiddenMaxElixirClaimed")).toBe(true)
    expect(await game.inventory.contains("maxElixir")).toBe(true)

    for (const [location, flag] of [
      [{ x: 36, y: 6, facing: "up" }, "powerPlantElectrode2Resolved"],
      [{ x: 30, y: 37, facing: "down" }, "powerPlantElectrode1Resolved"],
    ] as const) {
      await game.player.warp(hall, location.x, location.y, location.facing)
      await game.wait.forReady()
      await game.player.interact()
      await waitForBattle(game, flag)
      expect((await game.state.read()).battle.enemy).toMatchObject({
        species: "electrode",
        level: 34,
      })
      await game.battle.win()
      await settleField(game, `${flag} victory`)
      expect(await game.story.flag(flag)).toBe(true)
    }
    await game.saveAndReload()
    expect(await game.story.flag("powerPlantHiddenMaxElixirClaimed")).toBe(true)
    expect(await game.story.flag("powerPlantHiddenThunderStoneClaimed")).toBe(true)
    expect(await game.inventory.contains("maxElixir")).toBe(true)
    expect(await game.inventory.contains("thunderStone")).toBe(true)
    expect(await game.story.flag("powerPlantElectrode1Resolved")).toBe(true)
    expect(await game.story.flag("powerPlantElectrode2Resolved")).toBe(true)
    await reenterHall(game)
    await game.player.warp(hall, 36, 6, "up")
    await game.player.interact()
    await game.wait.frames(60)
    expect((await game.state.read()).battle.active).toBe(false)
  })

  it("refuses TR 54 and starts a fixed level-50 Zapdos battle at TR 55", async () => {
    await arrangeAt(game, { map: hall, x: 5, y: 10 }, "down", {
      flags: { powerPlantZapdosHidden: false, powerPlantZapdosResolved: false },
      vars: { trainerRating: 54 },
    })
    expect(await game.story.flag("powerPlantZapdosHidden")).toBe(false)
    await interactWithZapdos(game, "Zapdos TR 54")
    await advanceUntil(
      game,
      (state) => state.dialogue.text.toLowerCase().includes("watches you warily"),
      "Zapdos TR 54 refusal",
    )
    expect((await game.state.read()).battle.active).toBe(false)
    expect(await game.story.flag("powerPlantZapdosResolved")).toBe(false)
    await settleField(game, "Zapdos TR 54 refusal")

    await game.story.setVar("trainerRating", 55)
    await startZapdos(game, "Zapdos TR 55")
    await game.battle.lose()
    await advanceUntil(
      game,
      (state) => state.ready && !state.battle.active,
      "Zapdos blackout recovery",
    )
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "players-bedroom" },
      player: { x: 4, y: 4 },
    })
    expect(await game.story.flag("powerPlantZapdosResolved")).toBe(false)
  })

  it("permanently resolves a Zapdos knockout across save, reentry, and champion state", async () => {
    await arrangeAt(game, { map: hall, x: 5, y: 10 }, "down", {
      flags: { powerPlantZapdosHidden: false, powerPlantZapdosResolved: false },
      vars: { trainerRating: 55 },
    })
    await startZapdos(game, "Zapdos knockout")
    await game.battle.win()
    await settleField(game, "Zapdos knockout resolution")
    expect(await game.story.flag("powerPlantZapdosResolved")).toBe(true)
    await game.story.setFlag("isKantoChampion", true)
    await game.saveAndReload()
    await reenterHall(game)
    expect(await game.story.flag("powerPlantZapdosResolved")).toBe(true)
    await game.player.interact()
    await game.wait.frames(60)
    expect((await game.state.read()).battle.active).toBe(false)
  })

  it("permanently resolves a Zapdos capture across save and reentry", async () => {
    await arrangeAt(game, { map: hall, x: 5, y: 10 }, "down", {
      flags: { powerPlantZapdosHidden: false, powerPlantZapdosResolved: false },
      vars: { trainerRating: 55 },
      items: { masterBall: 1 },
      party: Array.from({ length: 6 }, () => ({ species: "pidgey" as const, level: 50 })),
    })
    await startZapdos(game, "Zapdos capture")
    await catchWithMasterBallAndSwap(game, { outgoingPartyIndex: 5 })
    expect(await game.story.flag("powerPlantZapdosResolved")).toBe(true)
    expect((await game.state.read()).party.some((mon) => mon.species === "zapdos")).toBe(true)
    await game.saveAndReload()
    await reenterHall(game)
    await game.player.interact()
    await game.wait.frames(60)
    expect((await game.state.read()).battle.active).toBe(false)
  })

  it("returns Zapdos after Run and player Teleport map reentry", async () => {
    await arrangeAt(game, { map: hall, x: 5, y: 10 }, "down", {
      flags: { powerPlantZapdosHidden: false, powerPlantZapdosResolved: false },
      vars: { trainerRating: 55 },
      party: [{ species: "pidgey", level: 100, moves: ["teleport", "tackle"] }],
    })
    await startZapdos(game, "Zapdos Run")
    await runFromBattle(game)
    expect(await game.story.flag("powerPlantZapdosResolved")).toBe(false)
    expect(await game.story.flag("powerPlantZapdosHidden")).toBe(true)
    await reenterHall(game)
    await interactWithZapdos(game, "Zapdos after Run")
    await waitForBattle(game, "Zapdos after Run")
    await useBattleTeleport(game)
    expect(await game.story.flag("powerPlantZapdosResolved")).toBe(false)
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "players-bedroom" },
      player: { x: 4, y: 4 },
    })
    await reenterHall(game)
    await startZapdos(game, "Zapdos after Teleport")
    await game.battle.win()
    await settleField(game, "Zapdos after Teleport cleanup")
  })

  it("keeps Zapdos retryable after a real blackout and has no exterior duplicate", async () => {
    await arrangeAt(game, { map: hall, x: 5, y: 10 }, "down", {
      flags: { powerPlantZapdosHidden: false, powerPlantZapdosResolved: false },
      vars: { trainerRating: 55 },
      party: [{ species: "pidgey", level: 1, moves: ["tackle"] }],
    })
    await startZapdos(game, "Zapdos blackout")
    await game.battle.lose()
    await advanceUntil(game, (state) => state.ready && !state.battle.active, "real Zapdos blackout")
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "players-bedroom" },
      player: { x: 4, y: 4 },
    })
    expect(await game.story.flag("powerPlantZapdosResolved")).toBe(false)
    await startZapdos(game, "Zapdos after blackout")
    await game.battle.win()
    await settleField(game, "Zapdos blackout retry victory")

    await arrangeAt(game, { map: "route-10", x: 5, y: 49 }, "up", {
      vars: { trainerRating: 55 },
    })
    await game.player.interact()
    await game.wait.frames(90)
    expect((await game.state.read()).battle.active).toBe(false)
    expect((await game.state.read()).map.name).toBe("route-10")
  })
})
