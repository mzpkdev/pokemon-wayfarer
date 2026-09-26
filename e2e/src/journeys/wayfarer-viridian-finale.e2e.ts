import * as fs from "node:fs"
import { beforeEach, describe, expect, it } from "webanvil/test"

import {
  GameSession,
  type ArrangeGame,
  type Direction,
  type GameMap,
  type StoryFlag,
} from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

const gym = "viridian-gym" as const
const city = "viridian-city" as const
const ordinary = [
  { name: "Takashi", flag: "viridianTakashiDefeated", x: 9, y: 2, facing: "right" },
  { name: "Yuji", flag: "viridianYujiDefeated", x: 11, y: 10, facing: "right" },
  { name: "Atsushi", flag: "viridianAtsushiDefeated", x: 10, y: 14, facing: "right" },
  { name: "Jason", flag: "viridianJasonDefeated", x: 9, y: 10, facing: "right" },
  { name: "Cole", flag: "viridianColeDefeated", x: 2, y: 22, facing: "up" },
  { name: "Kiyo", flag: "viridianKiyoDefeated", x: 3, y: 10, facing: "down" },
  { name: "Samuel", flag: "viridianSamuelDefeated", x: 6, y: 7, facing: "down" },
  { name: "Warren", flag: "viridianWarrenDefeated", x: 13, y: 6, facing: "down" },
] as const satisfies readonly {
  name: string
  flag: StoryFlag
  x: number
  y: number
  facing: Direction
}[]

const resolvedInvestigations = {
  celadonHideoutGiovanniTrainerDefeated: true,
  celadonHideoutScopeReceived: true,
  silphGiovanniDefeated: true,
  silphLiberated: true,
} as const

const giovanniObject = {
  localId: 8,
  graphicsId: 309,
  mapNum: 1,
  mapGroup: 70,
  elevation: 3,
  x: 2,
  y: 2,
  objectInvisible: false,
  spriteActive: true,
  spriteInvisible: false,
} as const

const visibleGymObjects = async (game: GameSession) => {
  const symbols = await readSkyEmuSymbols(requireSymbolsPath())
  const client = (game as unknown as { running: RunningSkyEmu }).running.client
  const objects = []
  for (let index = 0; index < 16; index++) {
    const bytes = await client.readBytes(symbols.address("gObjectEvents") + index * 0x24, 0x24)
    if (!(bytes[0]! & 1)) continue
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength)
    const spriteId = bytes[0x23]!
    const sprite = await client.readBytes(symbols.address("gSprites") + spriteId * 0x44, 0x40)
    objects.push({
      localId: bytes[8],
      graphicsId: view.getUint16(4, true) & 0xfff,
      mapNum: bytes[9],
      mapGroup: bytes[0xa],
      elevation: bytes[0xb]! & 0xf,
      x: view.getInt16(0x10, true) - 7,
      y: view.getInt16(0x12, true) - 7,
      objectInvisible: Boolean(bytes[1]! & 0x20),
      spriteActive: Boolean(sprite[0x3e]! & 1),
      spriteInvisible: Boolean(sprite[0x3e]! & 4),
    })
  }
  return objects
}

const expectGiovanniPresent = async (game: GameSession): Promise<void> => {
  expect(await visibleGymObjects(game)).toContainEqual(giovanniObject)
}

const defeatedOrdinary = Object.fromEntries(
  ordinary.map((trainer) => [trainer.flag, true]),
) as NonNullable<ArrangeGame["story"]>["flags"]

const arrangeAt = async (
  game: GameSession,
  x: number,
  y: number,
  facing: Direction,
  options: {
    map?: GameMap
    flags?: NonNullable<ArrangeGame["story"]>["flags"]
    bag?: ArrangeGame["bag"]
    badges?: NonNullable<ArrangeGame["circuit"]>["badges"]
    party?: ArrangeGame["party"]
  } = {},
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { position: { map: options.map ?? gym, x, y }, facing },
    story: { flags: options.flags },
    party: options.party ?? [{ species: "lapras", level: 100, moves: ["surf"] }],
    bag: options.bag,
    circuit: { badges: options.badges },
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
}

const settleField = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 300; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.battle.active) return
    if (state.dialogueOpen || state.scriptActive || state.battle.active)
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not settle: ${JSON.stringify(await game.state.read())}`)
}

const waitBattle = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 360; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} battle did not start: ${JSON.stringify(await game.state.read())}`)
}

const interact = async (game: GameSession, description: string): Promise<string> => {
  await game.player.interact()
  await game.wait.frames(12)
  const text = (await game.state.read()).dialogue.text
  await settleField(game, description)
  return text
}

const startGiovanni = async (game: GameSession): Promise<void> => {
  await game.player.warp(gym, 2, 3, "up")
  await game.wait.forReady()
  await expectGiovanniPresent(game)
  await game.player.interact()
  await game.dialogue.waitForOpen()
  expect((await game.state.read()).dialogue.text).toContain("GIOVANNI")
  await waitBattle(game, "Viridian Giovanni")
}

const stepTo = async (
  game: GameSession,
  direction: Direction,
  x: number,
  y: number,
): Promise<void> => {
  await game.wait.forReady()
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(12)
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y && state.map.name === gym) return
  }
  throw new Error(
    `Gym collision blocked ${direction} to ${x}:${y}: ${JSON.stringify(await game.state.read())}`,
  )
}

const walkLine = async (
  game: GameSession,
  direction: Direction,
  endX: number,
  endY: number,
): Promise<void> => {
  const offsets: Record<Direction, readonly [number, number]> = {
    up: [0, -1],
    down: [0, 1],
    left: [-1, 0],
    right: [1, 0],
  }
  const [dx, dy] = offsets[direction]
  for (let step = 0; step < 24; step++) {
    const { player } = await game.state.read()
    if (player.x === endX && player.y === endY) return
    await stepTo(game, direction, player.x + dx, player.y + dy)
  }
  throw new Error(`Gym line did not reach ${endX}:${endY}`)
}

const enterFromCity = async (game: GameSession): Promise<void> => {
  await game.player.warp(city, 43, 15, "up")
  await game.wait.forReady()
  await game.player.move("up")
  await game.wait.forMap(gym)
  await game.wait.frames(90)
  expect((await game.state.read()).map).toMatchObject({ name: gym, mapGroup: 70, mapNum: 1 })
}

const exitToCity = async (game: GameSession, x: 16 | 17 | 18): Promise<void> => {
  await game.player.warp(gym, x, 21, "down")
  await game.wait.forReady()
  await game.wait.frames(90)
  for (let attempt = 0; attempt < 4; attempt++) {
    await game.player.move("down")
    await game.wait.frames(45)
    if ((await game.state.read()).map.name === city) break
  }
  await game.wait.forMap(city, 300)
  expect((await game.state.read()).player).toMatchObject({ x: 43, y: 15 })
  const departed = await game.story.flag("viridianGiovanniDeparted")
  await capture(game, `gym-exit-${x}-${departed ? "departed" : "early"}`)
}

const capture = async (game: GameSession, name: string): Promise<void> => {
  const directory = process.env.VIRIDIAN_EVIDENCE_DIR
  if (!directory) return
  await fs.promises.mkdir(directory, { recursive: true })
  await fs.promises.writeFile(`${directory}/${name}.png`, await game.screenshot())
}

describe.sequential("Wayfarer Viridian Giovanni finale", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("enters the FRLG maze from the HNS doorway and returns from every exit before and after departure", async () => {
    await arrangeAt(game, 43, 15, "up", { map: city })
    await enterFromCity(game)
    await capture(game, "gym-early-entry")
    await game.player.warp(gym, 16, 21, "up")
    await game.wait.frames(90)
    expect(await interact(game, "Gym guide")).toContain("Champ")
    expect((await game.state.read()).map.name).toBe(gym)
    for (const x of [16, 17, 18] as const) {
      await exitToCity(game, x)
      await enterFromCity(game)
    }
    await game.saveAndReload()
    await exitToCity(game, 17)
    await enterFromCity(game)

    await game.story.setFlag("viridianGiovanniDeparted", true)
    await exitToCity(game, 18)
    await enterFromCity(game)
    await capture(game, "gym-after-departure")
  })

  it("crosses the removed Viridian Blue introduction on the way to the Gym", async () => {
    await arrangeAt(game, 41, 15, "right", {
      map: city,
      flags: { hideViridianBlueIntro: false, viridianBlueIntroduced: false },
    })
    await game.player.move("right")
    await game.wait.frames(60)
    const state = await game.state.read()
    expect(state).toMatchObject({
      map: { name: city },
      player: { x: 42, y: 15 },
      battle: { active: false },
      dialogueOpen: false,
      scriptActive: false,
    })
    expect(await game.story.flag("viridianBlueIntroduced")).toBe(false)
    await enterFromCity(game)
  })

  it("walks the full Gym interior from its doorway to Giovanni", async () => {
    await arrangeAt(game, 43, 15, "up", { map: city, flags: defeatedOrdinary })
    await enterFromCity(game)
    await game.story.setFlag("celadonHideoutGiovanniTrainerDefeated", true)
    await game.story.setFlag("celadonHideoutScopeReceived", true)
    await game.story.setFlag("silphGiovanniDefeated", true)
    await game.story.setFlag("silphLiberated", true)
    const arrival = await game.state.read()
    expect(arrival.player.y).toBe(22)
    if (arrival.player.x !== 16)
      throw new Error(`Unexpected FRLG Gym arrival: ${arrival.player.x}:${arrival.player.y}`)
    for (const [direction, x, y] of [
      ["up", 16, 21],
      ["left", 14, 21],
      ["up", 14, 13],
      ["left", 13, 13],
      ["up", 13, 10],
      ["right", 15, 10],
      ["up", 15, 6],
      ["left", 12, 6],
      ["down", 12, 7],
      ["left", 10, 7],
      ["up", 10, 3],
      ["left", 7, 3],
      ["down", 7, 6],
      ["left", 3, 6],
      ["up", 3, 3],
      ["left", 2, 3],
    ] as const)
      await walkLine(game, direction, x, y)
    await expectGiovanniPresent(game)
    await stepTo(game, "down", 2, 4)
    await game.wait.frames(90)
    await game.wait.forReady()
    await capture(game, "gym-leader-approach")
    await stepTo(game, "up", 2, 3)
    expect((await game.state.read()).player).toMatchObject({ x: 2, y: 3, facing: "up" })
    await game.wait.frames(90)
    await game.wait.forReady()
    expect(await game.story.flag("viridianGiovanniDeparted")).toBe(false)
    expect(await game.story.flag("viridianGiovanniDefeated")).toBe(false)
    await game.player.interact()
    await game.dialogue.waitForOpen()
    expect((await game.state.read()).dialogue.text).toContain("GIOVANNI")
    await waitBattle(game, "Giovanni after full Gym traversal")
    expect((await game.state.read()).battle.enemy).toMatchObject({ species: "rhyhorn", level: 14 })
  })

  for (const probe of [
    { name: "lower west row 21", x: 14, y: 21, direction: "left", endX: 5, endY: 21 },
    { name: "lower west row 22", x: 14, y: 22, direction: "left", endX: 1, endY: 22 },
    { name: "lower east row 18", x: 5, y: 18, direction: "left", endX: 13, endY: 18 },
    { name: "lower east row 17", x: 4, y: 17, direction: "right", endX: 13, endY: 17 },
    { name: "middle south", x: 4, y: 9, direction: "down", endX: 4, endY: 17 },
    { name: "east south short", x: 16, y: 12, direction: "down", endX: 16, endY: 15 },
    { name: "east south long", x: 18, y: 2, direction: "down", endX: 18, endY: 14 },
    { name: "east north", x: 19, y: 15, direction: "up", endX: 19, endY: 3 },
    { name: "north east", x: 10, y: 3, direction: "right", endX: 17, endY: 3 },
    { name: "north west", x: 19, y: 3, direction: "up", endX: 11, endY: 2 },
    { name: "west north row 0", x: 0, y: 21, direction: "up", endX: 0, endY: 11 },
    { name: "west north row 1", x: 1, y: 21, direction: "up", endX: 1, endY: 13 },
  ] as const) {
    it(`follows ${probe.name} spinner to its stop tile`, async () => {
      await arrangeAt(game, probe.x, probe.y, probe.direction, { flags: defeatedOrdinary })
      await game.player.move(probe.direction)
      await game.wait.frames(180)
      const state = await game.state.read()
      expect(state.map.name).toBe(gym)
      expect(state.ready).toBe(true)
      expect(state.player).toMatchObject({ x: probe.endX, y: probe.endY })
      await capture(game, `arrow-${probe.name.replaceAll(" ", "-")}`)
    })
  }

  it("keeps the finale locked until both local investigations resolve in either order", async () => {
    for (const first of ["hideout", "silph"] as const) {
      await arrangeAt(game, 2, 3, "up")
      const baseline = await game.state.read()
      expect(await interact(game, "Giovanni before investigations")).toContain("GIOVANNI")
      expect(await game.story.flag("defeatedViridianGym")).toBe(false)
      expect((await game.state.read()).circuit.badges).toEqual(baseline.circuit.badges)

      if (first === "hideout") {
        await game.story.setFlag("celadonHideoutGiovanniTrainerDefeated", true)
        await game.story.setFlag("celadonHideoutScopeReceived", true)
      } else {
        await game.story.setFlag("silphGiovanniDefeated", true)
        await game.story.setFlag("silphLiberated", true)
      }
      expect(await interact(game, `Giovanni after ${first} only`)).toContain("GIOVANNI")
      expect(await game.story.flag("defeatedViridianGym")).toBe(false)
      await game.saveAndReload()

      if (first === "hideout") {
        await game.story.setFlag("silphGiovanniDefeated", true)
        await game.story.setFlag("silphLiberated", true)
      } else {
        await game.story.setFlag("celadonHideoutGiovanniTrainerDefeated", true)
        await game.story.setFlag("celadonHideoutScopeReceived", true)
      }
      await startGiovanni(game)
      expect((await game.state.read()).battle.enemy).toMatchObject({
        species: "rhyhorn",
        level: 14,
      })
      await game.battle.lose()
      await settleField(game, `Giovanni after ${first} prerequisites`)
    }
  })

  it("starts an ordinary Trainer from sight and suppresses his approach after victory", async () => {
    await arrangeAt(game, 10, 6, "up")
    await game.player.move("up")
    await waitBattle(game, "Takashi sight encounter")
    await game.battle.win()
    await settleField(game, "Takashi sight victory")
    expect(await game.story.flag("viridianTakashiDefeated")).toBe(true)
    await game.wait.frames(90)
    await game.saveAndReload()
    await game.player.warp(gym, 10, 6, "up")
    await game.wait.forReady()
    await game.player.move("up")
    await game.wait.frames(90)
    const state = await game.state.read()
    expect(state).toMatchObject({
      map: { name: gym },
      battle: { active: false },
      dialogueOpen: false,
      scriptActive: false,
      controlsLocked: false,
    })
  })

  it("keeps all eight ordinary Trainers one-time and Giovanni retryable after a loss", async () => {
    await arrangeAt(game, 2, 3, "up", { flags: resolvedInvestigations })
    for (const trainer of ordinary) {
      await game.player.warp(gym, trainer.x, trainer.y, trainer.facing)
      await game.wait.forReady()
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await waitBattle(game, trainer.name)
      await game.battle.win()
      await settleField(game, trainer.name)
      expect(await game.story.flag(trainer.flag)).toBe(true)
      await game.wait.frames(90)
    }
    await game.saveAndReload()
    for (const trainer of ordinary) {
      await game.player.warp(gym, trainer.x, trainer.y, trainer.facing)
      await interact(game, `${trainer.name} rematch check`)
      expect(await game.story.flag(trainer.flag)).toBe(true)
    }
    await startGiovanni(game)
    await game.battle.lose()
    await settleField(game, "Giovanni loss")
    expect(await game.story.flag("defeatedViridianGym")).toBe(false)
    expect(await game.story.flag("badge16")).toBe(false)
    expect(await game.story.flag("viridianGiovanniDeparted")).toBe(false)
    expect(await game.inventory.count("tmEarthquake")).toBe(0)
    await startGiovanni(game)
    expect((await game.state.read()).battle.enemy?.species).toBe("rhyhorn")
  })

  it("awards Earth Badge, Trainer Rating and Earthquake once, then Giovanni leaves without Rocket cleanup", async () => {
    await arrangeAt(game, 2, 3, "up", {
      flags: { ...resolvedInvestigations, silphMasterBallPending: true },
    })
    const before = await game.state.read()
    await startGiovanni(game)
    expect((await game.state.read()).battle.enemy).toMatchObject({ species: "rhyhorn", level: 14 })
    await game.battle.win()
    await settleField(game, "Giovanni victory and TM")
    expect(await game.story.flag("viridianGiovanniDefeated")).toBe(true)
    expect(await game.story.flag("defeatedViridianGym")).toBe(true)
    expect(await game.story.flag("badge16")).toBe(true)
    expect(await game.story.flag("viridianGiovanniDeparted")).toBe(true)
    expect(await game.inventory.count("tmEarthquake")).toBe(1)
    const after = await game.state.read()
    expect(after.circuit.badges).toMatchObject({
      kanto: before.circuit.badges.kanto + 1,
      total: before.circuit.badges.total + 1,
    })
    expect(after.circuit.trainerRating).toBe(before.circuit.trainerRating + 4)
    for (const flag of [
      "silphMasterBallPending",
      "celadonHideoutScopeReceived",
      "silphLiberated",
    ] as const)
      expect(await game.story.flag(flag)).toBe(true)
    for (const flag of [
      "mtMoonGrunt1Defeated",
      "nuggetBridgeRecruiterDefeated",
      "towerFujiRescued",
      "hideVermilionSnorlax",
    ] as const)
      expect(await game.story.flag(flag)).toBe(false)
    await capture(game, "giovanni-departed")
    await game.saveAndReload()
    expect(await game.inventory.count("tmEarthquake")).toBe(1)
    expect(await game.story.flag("badge16")).toBe(true)
    await game.player.warp(gym, 2, 3, "up")
    await game.player.move("up")
    await game.wait.frames(30)
    expect((await game.state.read()).player).toMatchObject({ x: 2, y: 2 })
    expect((await game.state.read()).battle.active).toBe(false)
    expect(await game.inventory.count("tmEarthquake")).toBe(1)
    for (const x of [16, 17, 18] as const) {
      await exitToCity(game, x)
      await enterFromCity(game)
    }
    await game.player.warp(gym, 16, 21, "up")
    await game.wait.frames(90)
    expect(await interact(game, "guide after Giovanni departure")).toContain("GIOVANNI has left")
    expect(await game.story.flag("viridianGiovanniDeparted")).toBe(true)
    expect(await game.inventory.count("tmEarthquake")).toBe(1)
    await game.player.warp(gym, 2, 3, "up")
    expect(await game.story.flag("viridianMachoBraceClaimed")).toBe(false)
    expect(await game.story.flag("sproutTowerEscapeRopeClaimed")).toBe(false)
    await interact(game, "Giovanni's hidden Macho Brace")
    expect(await game.inventory.count("machoBrace")).toBe(1)
    expect(await game.story.flag("viridianMachoBraceClaimed")).toBe(true)
    expect(await game.story.flag("sproutTowerEscapeRopeClaimed")).toBe(false)
    await game.saveAndReload()
    expect(await game.inventory.count("machoBrace")).toBe(1)
    expect(await game.story.flag("viridianMachoBraceClaimed")).toBe(true)
  })

  it("holds Giovanni in place after a full TM pocket until a reload-safe claim succeeds", async () => {
    await arrangeAt(game, 2, 3, "up", {
      flags: resolvedInvestigations,
      bag: { fullPockets: ["tmHm"] },
    })
    await startGiovanni(game)
    await game.battle.win()
    await settleField(game, "full Bag Earthquake offer")
    expect(await game.story.flag("badge16")).toBe(true)
    expect(await game.story.flag("viridianGiovanniDeparted")).toBe(false)
    expect(await game.inventory.count("tmEarthquake")).toBe(0)
    const afterBadge = await game.state.read()
    await game.saveAndReload()
    await game.inventory.freeSlot("tmHm")
    await game.player.warp(gym, 2, 3, "up")
    await interact(game, "deferred Earthquake handoff")
    expect(await game.inventory.count("tmEarthquake")).toBe(1)
    expect(await game.story.flag("viridianGiovanniDeparted")).toBe(true)
    expect((await game.state.read()).circuit.badges).toEqual(afterBadge.circuit.badges)
    expect((await game.state.read()).circuit.trainerRating).toBe(afterBadge.circuit.trainerRating)
  })

  it("delivers one additional Earthquake TM when the player already owns it", async () => {
    await arrangeAt(game, 2, 3, "up", {
      flags: resolvedInvestigations,
      bag: { items: { tmEarthquake: 1 } },
    })
    await startGiovanni(game)
    await game.battle.win()
    await settleField(game, "prior-owned Earthquake handoff")
    expect(await game.inventory.count("tmEarthquake")).toBe(2)
    expect(await game.story.flag("viridianGiovanniDeparted")).toBe(true)
    await game.saveAndReload()
    expect(await game.inventory.count("tmEarthquake")).toBe(2)
    expect(await game.story.flag("badge16")).toBe(true)
  })
})
