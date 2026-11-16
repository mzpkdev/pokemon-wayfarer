import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction, type GameMap } from "../harness/game-session"

// Legendary access in the Wayfarer build (.product/prds/frlg-kanto-independent-story-beats.md,
// .product/specs/ss-ferries.md): Moltres and Mewtwo wait for the legendary Trainer Rating (120 on the
// v0 scale), Steven's Vermilion Lati scene waits for a Kanto champion, the Vermilion sailor never
// offers Navel Rock, and the Southern Island Lati no longer needs the Mystery Event ship flag.

type State = Awaited<ReturnType<GameSession["state"]["read"]>>

const advanceUntil = async (
  game: GameSession,
  predicate: (state: State) => boolean,
  description: string,
  maxAttempts = 300,
): Promise<void> => {
  for (let attempt = 0; attempt < maxAttempts; attempt++) {
    const state = await game.state.read()
    if (predicate(state)) return
    if (state.battle.active && state.battle.ui === "action-menu") return
    if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} was not reached: ${JSON.stringify(await game.state.read())}`)
}

type ArrangeOptions = Parameters<GameSession["arrange"]>[0]

const arrangeAt = (
  game: GameSession,
  position: { map: GameMap; x: number; y: number },
  facing: Direction,
  story: NonNullable<ArrangeOptions["story"]> = {},
  items?: NonNullable<ArrangeOptions["bag"]>["items"],
): Promise<void> =>
  game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { position, facing },
    story: { ...story, flags: { disableEncounters: true, ...story.flags } },
    party: [{ species: "lapras", level: 100, moves: ["tackle"] }],
    bag: { items },
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })

const expectRefusal = async (game: GameSession, name: string): Promise<void> => {
  await game.player.interact()
  await advanceUntil(
    game,
    (state) => state.dialogue.fullText.includes("watching you warily"),
    `${name} refusal`,
  )
  const state = await game.state.read()
  expect(state.dialogue.fullText).toContain(`${name} is watching you warily`)
  expect(state.battle.active).toBe(false)
  // Read the message through: it ends with the field free and no battle.
  for (let attempt = 0; attempt < 30; attempt++) {
    const now = await game.state.read()
    if (now.ready && !now.scriptActive && !now.dialogueOpen) break
    await game.controls.press("a")
    await game.wait.frames(20)
  }
  const after = await game.state.read()
  expect(after.battle.active).toBe(false)
  expect(after.ready).toBe(true)
}

const expectBattle = async (
  game: GameSession,
  species: "moltres" | "mewtwo" | "latias" | "latios",
  level: number,
): Promise<void> => {
  await game.player.interact()
  await advanceUntil(game, (state) => state.battle.ui === "action-menu", `${species} battle`)
  expect((await game.state.read()).battle.enemy).toMatchObject({ species, level })
  await runFromBattle(game)
}

// Leaves the battle so the next arrange finds a free field (arrange is refused while in battle). Run can
// fail, so keep choosing it until the battle is over.
const runFromBattle = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 400; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready && !state.scriptActive && !state.dialogueOpen) break
    if (state.battle.ui === "action-menu") {
      const cursor = state.battle.cursor ?? 0
      if (cursor % 2 !== 1) await game.controls.press("right")
      if (Math.floor(cursor / 2) !== 1) await game.controls.press("down")
      await game.controls.press("a")
      await game.wait.frames(20)
    } else if (state.battle.ui === "move-menu") await game.controls.press("b")
    else if (state.battle.active || state.dialogueOpen || state.scriptActive)
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  const done = await game.state.read()
  if (done.battle.active) throw new Error(`Could not leave the battle: ${JSON.stringify(done)}`)
  // The map script resumes a moment after the battle; let it finish before the next arrange.
  await game.wait.frames(120)
}

describe.sequential("Wayfarer legendary access", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("keeps Moltres wary below TR 120 and battles at TR 120", async () => {
    const at = { map: "mt-silver-moltres-room", x: 17, y: 21 } as const
    await arrangeAt(game, at, "up", {
      flags: { hideMoltres: false },
      vars: { trainerRating: 119 },
    })
    await expectRefusal(game, "MOLTRES")

    await arrangeAt(game, at, "up", {
      flags: { hideMoltres: false },
      vars: { trainerRating: 120 },
    })
    await expectBattle(game, "moltres", 50)
  })

  it("keeps Mewtwo wary below TR 120 and battles at TR 120", async () => {
    const at = { map: "cerulean-cave-b2f", x: 8, y: 15 } as const
    await arrangeAt(game, at, "up", {
      flags: { hideMewtwo: false },
      vars: { trainerRating: 119 },
    })
    await expectRefusal(game, "MEWTWO")

    await arrangeAt(game, at, "up", {
      flags: { hideMewtwo: false },
      vars: { trainerRating: 120 },
    })
    await expectBattle(game, "mewtwo", 80)
  })

  it("starts Steven's Vermilion scene only for a Kanto champion", async () => {
    const at = { map: "vermilion-city", x: 33, y: 3 } as const
    await arrangeAt(game, at, "up", { flags: { isKantoChampion: false } })
    await game.player.move("up")
    await game.wait.frames(90)
    let state = await game.state.read()
    expect(state.map.name).toBe("vermilion-city")
    expect(state.scriptActive).toBe(false)
    expect(state.dialogueOpen).toBe(false)
    // The player walked onto the trigger tile and was left in control.
    expect(state.player.y).toBe(2)
    await expect(game.story.var("vermilionCityStevenState")).resolves.toBe(0)

    await arrangeAt(game, at, "up", { flags: { isKantoChampion: true } })
    await game.player.move("up")
    // The scene reads Steven's introduction, then offers the Latias/Latios choice.
    for (let attempt = 0; attempt < 120; attempt++) {
      state = await game.state.read()
      if (state.choice.kind === "multichoice") break
      if (state.dialogueOpen || state.scriptActive) await game.controls.press("a")
      await game.wait.frames(30)
    }
    expect((await game.state.read()).choice.kind).toBe("multichoice")
    // Pick the first row and read Steven's farewell through: the scene then marks itself done.
    await game.controls.press("a")
    for (let attempt = 0; attempt < 120; attempt++) {
      if ((await game.story.var("vermilionCityStevenState")) === 1) break
      state = await game.state.read()
      if (state.dialogueOpen || state.scriptActive) await game.controls.press("a")
      await game.wait.frames(30)
    }
    await expect(game.story.var("vermilionCityStevenState")).resolves.toBe(1)
  })

  it("never offers Navel Rock from the Vermilion sailor's Sevii menu", async () => {
    const waitForMenu = async (): Promise<State> => {
      for (let attempt = 0; attempt < 60; attempt++) {
        await game.wait.frames(20)
        const state = await game.state.read()
        if (state.choice.kind === "multichoice") return state
        if (state.dialogueOpen || state.scriptActive) await game.controls.press("a")
      }
      throw new Error(`No menu: ${JSON.stringify(await game.state.read())}`)
    }
    const rows = async (items: NonNullable<ArrangeOptions["bag"]>["items"]): Promise<number> => {
      await arrangeAt(
        game,
        { map: "vermilion-port-inside", x: 8, y: 9 },
        "down",
        { flags: { systemGameClear: true } },
        items,
      )
      await game.player.interact()
      // Port menu without a ship in berth: SEVII ISLANDS, BOARD S.S. ANNE, CANCEL.
      const port = await waitForMenu()
      expect(port.choice.optionCount).toBe(3)
      await game.controls.press("a")
      await game.wait.frames(30)
      const sevii = await waitForMenu()
      return sevii.choice.optionCount
    }
    // One Island and Cancel, even holding the Mystic Ticket after the game is cleared.
    await expect(rows({ mysticTicket: 1 })).resolves.toBe(2)
    // Birth Island joins with the Aurora Ticket; Navel Rock still never does.
    await expect(rows({ mysticTicket: 1, auroraTicket: 1 })).resolves.toBe(3)
  })

  it("starts the Southern Island Lati battle without the ship flag", async () => {
    await arrangeAt(game, { map: "southern-island-interior", x: 13, y: 12 }, "up", {})
    await game.player.interact()
    await advanceUntil(game, (state) => state.battle.ui === "action-menu", "Lati battle", 600)
    const enemy = (await game.state.read()).battle.enemy
    expect(["latias", "latios"]).toContain(enemy?.species)
  })
})
