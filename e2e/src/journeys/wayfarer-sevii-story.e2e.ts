import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction, type GameMap } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

type Location = { map: GameMap; x: number; y: number; facing: Direction }

const strongParty = () => [{ species: "lapras", moves: ["surf"], level: 100 }] as const

const advanceUntil = async (
  game: GameSession,
  predicate: (state: Awaited<ReturnType<GameSession["state"]["read"]>>) => boolean,
  description: string,
  maxAttempts = 240,
  advanceActionMenu = false,
): Promise<void> => {
  for (let attempt = 0; attempt < maxAttempts; attempt++) {
    const state = await game.state.read()
    if (predicate(state)) return
    await game.wait.frames(state.dialogueOpen || state.scriptActive ? 24 : 12)
    const nextState = await game.state.read()
    if (advanceActionMenu || !nextState.battle.active || nextState.battle.ui === "text")
      await game.controls.press("a")
  }
  throw new Error(`${description} not reached: ${JSON.stringify(await game.state.read())}`)
}

const finishScript = (
  game: GameSession,
  description: string,
  advanceActionMenu = false,
): Promise<void> =>
  advanceUntil(
    game,
    (state) => state.ready && !state.dialogueOpen,
    description,
    240,
    advanceActionMenu,
  )

const warpTo = async (game: GameSession, location: Location): Promise<void> => {
  await game.player.warp(location.map, location.x, location.y, location.facing)
  await game.wait.forMap(location.map)
}

const startObjectiveBattle = async (
  game: GameSession,
  location: Location,
  description: string,
): Promise<void> => {
  await warpTo(game, location)
  await game.player.interact()
  await advanceUntil(
    game,
    (state) => state.battle.ui === "action-menu",
    `${description} battle action menu`,
  )
}

const winObjectiveBattle = async (
  game: GameSession,
  location: Location,
  description: string,
): Promise<void> => {
  await startObjectiveBattle(game, location, description)
  await game.battle.win()
  await finishScript(game, `${description} victory script`, true)
}

// One tile in `direction`, retrying until the avatar actually moves (a held button
// can be swallowed while the previous step animation finishes).
const walk = async (game: GameSession, direction: Direction, tiles = 1): Promise<void> => {
  for (let tile = 0; tile < tiles; tile++) {
    const before = (await game.state.read()).player
    let moved = false
    for (let attempt = 0; attempt < 8 && !moved; attempt++) {
      await game.player.move(direction)
      await game.wait.frames(20)
      const after = (await game.state.read()).player
      moved = after.x !== before.x || after.y !== before.y
    }
    if (!moved) throw new Error(`Could not walk ${direction} from ${before.x},${before.y}`)
  }
}

// Turn toward a blocker without moving: the avatar only faces an occupied tile.
const turn = async (game: GameSession, direction: Direction): Promise<void> => {
  await game.player.move(direction)
  await game.wait.frames(12)
}

const expectAt = async (game: GameSession, x: number, y: number): Promise<void> => {
  expect((await game.state.read()).player).toMatchObject({ x, y })
}

const blue = 9 // NOTABLE_TRAINER_BLUE

const gameOf = (game: GameSession): RunningSkyEmu =>
  (game as unknown as { running: RunningSkyEmu }).running

// Marks a notable trainer introduced, as beating them at the Indigo League would.
const markIntroduced = async (game: GameSession, characterId: number): Promise<void> => {
  const symbols = await readSkyEmuSymbols(requireSymbolsPath())
  const client = gameOf(game).client
  const saveBlock3 = new DataView(
    (await client.readBytes(symbols.address("gSaveBlock3Ptr"), 4)).buffer,
  ).getUint32(0, true)
  const offset = new DataView(
    (await client.readBytes(symbols.address("gE2ENotableIntroducedOffset"), 2)).buffer,
  ).getUint16(0, true)
  const address = saveBlock3 + offset + Math.floor((characterId - 1) / 8)
  const byte = (await client.readBytes(address, 1))[0]!
  await client.writeBytes(address, new Uint8Array([byte | (1 << ((characterId - 1) % 8))]))
}

describe.sequential("Wayfarer Sevii independent story journeys", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("plays the post-League rival scene on Four Island and leaves no Blue behind", async () => {
    // Arrive through the Pokémon Center door: the scene runs on Four Island's
    // first frame, which would otherwise hold the arrange mailbox open.
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "sevii-four-island-pokemon-center-1-f", x: 7, y: 7 },
      },
      story: { flags: { seviiRivalSceneSeen: false, seviiHideRivals: true } },
      circuit: { clears: { indigo: true, masters: false, hoenn: false } },
      determinism: { textSpeed: "instant" },
    })
    // A Johto-origin player gets the scene only once Blue knows them; clearing Indigo
    // means beating him, which introduces him.
    await markIntroduced(game, blue)
    // Step onto the exit mat, then south off it to leave.
    for (let step = 0; step < 10; step++) {
      if ((await game.state.read()).map.name === "sevii-four-island") break
      await game.player.move("down")
      await game.wait.frames(20)
    }
    await advanceUntil(
      game,
      (state) =>
        state.map.name === "sevii-four-island" &&
        state.ready &&
        !state.dialogueOpen &&
        !state.scriptActive,
      "Four Island rival scene",
    )
    expect(await game.story.flag("seviiRivalSceneSeen")).toBe(true)
    // The scene's actor leaves with it: Blue is never left standing on two islands.
    expect(await game.story.flag("seviiHideRivals")).toBe(true)
    const { objects } = await game.state.read()
    expect(objects).not.toContainEqual(expect.objectContaining({ x: 8, y: 25, visible: true }))
  })

  it("keeps Bill's Meteorite pending when Key Items are full, then commits it across reload", async () => {
    const bill = {
      map: "sevii-one-island-pokemon-center-1-f",
      x: 14,
      y: 7,
      facing: "up",
    } as const

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: bill, facing: bill.facing },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      bag: { fullPockets: ["keyItems"] },
      determinism: { textSpeed: "instant" },
    })
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(false)
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await game.controls.press("a")
    await finishScript(game, "Bill full-pocket response")
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(false)
    await expect(game.inventory.contains("meteorite")).resolves.toBe(false)

    // Celio's request is independently usable while Bill's transaction is pending.
    await warpTo(game, {
      map: "sevii-one-island-pokemon-center-1-f",
      x: 15,
      y: 7,
      facing: "up",
    })
    await game.player.interact()
    await finishScript(game, "Celio investigation acceptance")
    await expect(game.story.flag("seviiCelioGemsStarted")).resolves.toBe(true)
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(false)

    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: bill, facing: bill.facing },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      determinism: { textSpeed: "instant" },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await game.controls.press("a")
    await finishScript(game, "Bill successful Meteorite handoff")
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(true)
    await expect(game.inventory.contains("meteorite")).resolves.toBe(true)

    await game.saveAndReload()
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(true)
    await expect(game.inventory.contains("meteorite")).resolves.toBe(true)
  })

  it("starts Lostelle locally and preserves ordered battles through loss, blackout, and reload", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        position: { map: "sevii-two-island-joyful-game-corner", x: 5, y: 6 },
        facing: "up",
      },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      determinism: { textSpeed: "instant" },
    })
    // The gang only arrives once Lostelle's father asks for help: until then
    // Three Island is open from the Port.
    await expect(game.story.flag("seviiHideThreeIslandBikers")).resolves.toBe(true)
    await game.player.interact()
    await finishScript(game, "Lostelle's local introduction")
    await expect(game.story.flag("seviiLostelleStarted")).resolves.toBe(true)
    await expect(game.story.flag("seviiHideThreeIslandBikers")).resolves.toBe(false)
    await expect(game.story.flag("seviiMeteoriteReceived")).resolves.toBe(false)

    // Arrive from the Port and walk north across the connection to the blockade.
    await warpTo(game, { map: "sevii-three-island-port", x: 11, y: 2, facing: "up" })
    await walk(game, "up", 3)
    await game.wait.forMap("sevii-three-island")
    await walk(game, "up", 13)
    await expectAt(game, 11, 26)

    // Biker Goon 2 (10,24) refuses to fight before Goon 1 is beaten.
    await walk(game, "left", 1)
    await walk(game, "up")
    await expectAt(game, 10, 25)
    await turn(game, "up")
    await game.player.interact()
    await advanceUntil(
      game,
      (state) => state.dialogue.text.includes("Another biker"),
      "out-of-order biker refusal",
    )
    expect((await game.state.read()).battle.active).toBe(false)
    await finishScript(game, "out-of-order biker refusal")

    // Goon 1 stands at (9,24): fight from (9,25) and lose once.
    await walk(game, "left")
    await turn(game, "up")
    await game.player.interact()
    await advanceUntil(
      game,
      (state) => state.battle.ui === "action-menu",
      "first biker battle action menu",
    )
    await game.battle.lose()
    await advanceUntil(
      game,
      (state) => state.ready && !state.battle.active,
      "first biker blackout recovery",
      240,
      true,
    )
    await expect(game.story.flag("seviiBikersCleared")).resolves.toBe(false)
    await game.saveAndReload()

    // Beat the four required bikers in order, each one reached on foot from the
    // Port side. The approach tiles lie south of the line (y 25) and Goon 3 sits
    // on that line now, so no one is boxed in.
    const fightFrom = async (x: number, description: string): Promise<void> => {
      await warpTo(game, { map: "sevii-three-island", x, y: 25, facing: "up" })
      await game.player.interact()
      await advanceUntil(
        game,
        (state) => state.battle.ui === "action-menu",
        `${description} battle action menu`,
      )
      await game.battle.win()
      await finishScript(game, `${description} victory script`, true)
    }
    await fightFrom(9, "biker goon 1")
    await walk(game, "right")
    await turn(game, "up")
    await game.player.interact()
    await advanceUntil(game, (state) => state.battle.ui === "action-menu", "biker goon 2")
    await game.battle.win()
    await finishScript(game, "biker goon 2 victory script", true)
    await walk(game, "right")
    await turn(game, "up")
    await game.player.interact()
    await advanceUntil(game, (state) => state.battle.ui === "action-menu", "biker goon 3")
    await game.battle.win()
    await finishScript(game, "biker goon 3 victory script", true)
    await walk(game, "left", 3)
    await turn(game, "up")
    await game.player.interact()
    await advanceUntil(game, (state) => state.battle.ui === "action-menu", "Cue Ball Paxton")
    await game.battle.win()
    await finishScript(game, "Cue Ball Paxton victory script", true)

    await expect(game.story.flag("seviiBikersCleared")).resolves.toBe(true)
    await game.saveAndReload()
    await expect(game.story.flag("seviiBikersCleared")).resolves.toBe(true)

    const lostelle = {
      map: "sevii-three-island-berry-forest",
      x: 4,
      y: 9,
      facing: "up",
    } as const
    await startObjectiveBattle(game, lostelle, "Lostelle's Hypno")
    expect((await game.state.read()).battle.enemy).toMatchObject({ species: "hypno", level: 30 })
    await game.battle.lose()
    await advanceUntil(
      game,
      (state) => state.ready && !state.battle.active,
      "Hypno blackout recovery",
      240,
      true,
    )
    await expect(game.story.flag("seviiLostelleFound")).resolves.toBe(false)
    await expect(game.story.flag("seviiLostelleRescued")).resolves.toBe(false)
    await game.saveAndReload()

    await winObjectiveBattle(game, lostelle, "retried Hypno")
    await expect(game.story.flag("seviiLostelleFound")).resolves.toBe(true)
    await expect(game.story.flag("seviiLostelleRescued")).resolves.toBe(true)
    await expect(game.story.flag("seviiIapapaBerryReceived")).resolves.toBe(true)
  })

  it("keeps Ruby Path open and lets the Mt. Ember guards go once both are beaten", async () => {
    const approach = {
      map: "sevii-mt-ember-exterior",
      x: 42,
      y: 42,
      facing: "up",
    } as const
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: approach, facing: approach.facing },
      story: { flags: { seviiCelioGemsStarted: true, seviiHideRubyGuards: false } },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      determinism: { textSpeed: "instant" },
    })

    // Guard 1 stands at (41,40) beside the cave mouth and guard 2 at (41,41):
    // neither blocks (42,40), so the cave is open even while they hold their post.
    await walk(game, "up", 2)
    await expectAt(game, 42, 40)
    await walk(game, "down", 2)

    await winObjectiveBattle(
      game,
      { map: "sevii-mt-ember-exterior", x: 42, y: 40, facing: "left" },
      "Mt. Ember guard 1",
    )
    await expect(game.story.flag("seviiHideRubyGuards")).resolves.toBe(false)
    await winObjectiveBattle(
      game,
      { map: "sevii-mt-ember-exterior", x: 42, y: 41, facing: "left" },
      "Mt. Ember guard 2",
    )
    // Both beaten: the guards leave their post and stay gone after a reload.
    await expect(game.story.flag("seviiHideRubyGuards")).resolves.toBe(true)
    await game.saveAndReload()
    const { objects } = await game.state.read()
    expect(objects).not.toContainEqual(expect.objectContaining({ x: 41, y: 41, visible: true }))
    expect(objects).not.toContainEqual(expect.objectContaining({ x: 41, y: 40, visible: true }))

    await warpTo(game, approach)
    await walk(game, "up", 3)
    await game.wait.forMap("sevii-mt-ember-ruby-path-1-f")
  })

  it("gates Moltres at TR 119 and resolves only a TR 120 knockout across reload", async () => {
    const moltres = {
      map: "sevii-mt-ember-summit",
      x: 9,
      y: 7,
      facing: "up",
    } as const
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { position: moltres, facing: moltres.facing },
      story: { vars: { trainerRating: 119 } },
      party: [...strongParty()].map((mon) => ({ ...mon, moves: [...mon.moves] })),
      determinism: { textSpeed: "instant" },
    })
    expect((await game.state.read()).circuit.trainerRating).toBe(119)
    await game.player.interact()
    await advanceUntil(
      game,
      (state) => state.dialogue.text.includes("radiates overwhelming"),
      "Moltres TR 119 refusal",
    )
    expect((await game.state.read()).battle.active).toBe(false)
    await finishScript(game, "Moltres TR 119 refusal")
    await expect(game.story.flag("seviiMoltresResolved")).resolves.toBe(false)

    await game.story.setVar("trainerRating", 120)
    await startObjectiveBattle(game, moltres, "Moltres at TR 120")
    expect((await game.state.read()).battle.enemy).toMatchObject({ species: "moltres", level: 50 })
    await game.battle.lose()
    await advanceUntil(
      game,
      (state) => state.ready && !state.battle.active,
      "Moltres blackout recovery",
      240,
      true,
    )
    await expect(game.story.flag("seviiMoltresResolved")).resolves.toBe(false)
    await game.saveAndReload()

    await startObjectiveBattle(game, moltres, "retried Moltres")
    await game.battle.win()
    await finishScript(game, "Moltres knockout resolution", true)
    await expect(game.story.flag("seviiMoltresResolved")).resolves.toBe(true)
    await game.saveAndReload()
    await expect(game.story.flag("seviiMoltresResolved")).resolves.toBe(true)
  })
})
