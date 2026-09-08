import * as fs from "node:fs"
import { describe, expect, it } from "webanvil/test"
import { items, maps, storyVars, type GameMap } from "../harness/game-session/catalog"
import { GameSession } from "../harness/game-session"

// These Kanto Safari maps are intentionally not public fixture destinations. Register
// them only for this journey so it enters through the actual attendant script instead
// of setting Safari state in the test hook.
const safariEntrance = "test-kanto-safari-entrance" as GameMap
const safariBeach = "test-kanto-safari-beach" as GameMap
const fixtureMaps = maps as Record<string, { mapGroup: number; mapNum: number }>
fixtureMaps[safariEntrance] = { mapGroup: 30, mapNum: 103 }
fixtureMaps[safariBeach] = { mapGroup: 30, mapNum: 104 }

// The fixture API deliberately exposes only the story variables and items used
// by existing journeys. These registrations provide the published HNS values
// needed to reach the otherwise ordinary Kanto Safari attendant.
const safariProgress = "test-kanto-safari-progress" as keyof typeof storyVars
;(storyVars as Record<string, number>)[safariProgress] = 0x406d
;(items as Record<string, number>).testPokeblockCase = 722

const dismissUntil = async (
  game: GameSession,
  predicate: () => Promise<boolean>,
  description: string,
): Promise<void> => {
  for (let frame = 0; frame < 7_200; frame += 30) {
    if (await predicate()) return
    const state = await game.state.read()
    // Field yes/no prompts and money boxes are not exposed as dialogue-open;
    // accepting is the authentic Safari-counter path.
    if (!state.ready || state.dialogueOpen || state.battle.ui === "text")
      await game.controls.press("a")
    else await game.wait.frames(30)
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

const enterKantoSafari = async (game: GameSession, rngSeed = 1): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "up", position: { map: safariEntrance, x: 5, y: 6 } },
    story: { vars: { [safariProgress]: 1 } },
    // The Safari controller must leave this real party Pokémon untouched.
    // Wailord provides enough HP for the journey to expose any accidental
    // normal-battle routing without ending the fixture prematurely.
    party: [{ species: "wailord", level: 100 }],
    bag: { items: { testPokeblockCase: 1 } as never },
    determinism: { textSpeed: "instant", rngSeed },
  })

  // The first actual visit gives the Pokéblock Case and arms the counter.
  await dismissUntil(game, async () => (await game.state.read()).ready, "Safari entrance opening")
  await game.player.move("up")
  await dismissUntil(
    game,
    async () => {
      const state = await game.state.read()
      return state.ready && state.map.mapGroup === 30 && state.map.mapNum === 104
    },
    "Safari attendant entry",
  )
}

const waitForSafariText = async (game: GameSession, description: string): Promise<boolean> => {
  for (let frame = 0; frame < 1_800; frame += 15) {
    const state = await game.state.read()
    if (!state.battle.active) return false
    if (state.battle.ui === "text") return true
    await game.wait.frames(15)
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

const settleSafariTurn = async (game: GameSession, description: string): Promise<boolean> => {
  let acknowledgedText = false
  for (let frame = 0; frame < 3_600; frame += 15) {
    const state = await game.state.read()
    if (!state.battle.active) return false
    if (state.battle.ui === "text") {
      acknowledgedText = true
      await game.controls.press("a")
      continue
    }
    if (acknowledgedText) {
      // Let the opposing Safari AI complete before issuing another menu input.
      await game.wait.frames(360)
      return (await game.state.read()).battle.active
    }
    await game.wait.frames(15)
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

const selectSafariAction = async (game: GameSession, action: "go-near" | "run") => {
  // The standard Safari controller keeps its own menu state outside ABI 14.
  // Normalize from any previous cursor position with real directional inputs.
  if (action === "go-near") {
    await game.controls.press("left")
    await game.controls.press("down")
  } else {
    await game.controls.press("right")
    await game.controls.press("down")
  }
  await game.controls.press("a")
}

const startNaturalSafariBattle = async (game: GameSession): Promise<void> => {
  // The attendant puts the player at the beach exit, whose nearby left turn
  // is blocked by the live layout. Relocate with the fixture only after the
  // real counter has armed Safari mode, to the proven grass strip at 10..12,
  // 17..19, then use normal movement and the map's live encounter table.
  await game.player.warp(safariBeach, 11, 18, "left")
  await fs.promises.writeFile("/tmp/wayfarer-safari-grass-fixture.png", await game.screenshot())
  for (let step = 0; step < 120; step++) {
    const state = await game.state.read()
    if (state.battle.active) break
    await game.player.move(step % 2 === 0 ? "left" : "right")
    await game.wait.frames(30)
  }
  await game.wait.until((state) => state.battle.active, "natural Safari grass encounter", 1_200)
  // A natural encounter is first observed during its intro animation. Wait
  // for its text box before acknowledging it; pressing during that animation
  // is intentionally ignored by the live controller.
  await game.wait.frames(240)
  await game.controls.press("a")
  await game.wait.frames(240)
}

describe.sequential("Wayfarer Safari regressions", () => {
  it("preserves Safari entry, closest Go Near, Run, and retirement outside trainer-only mode", async () => {
    const game = await GameSession.launch()
    try {
      // Seed 5 gives three surviving approaches. The fourth is the closest
      // action: its live HNS flee roll is deliberately 95%, so this journey
      // records the committed action before that ordinary Safari outcome.
      await enterKantoSafari(game, 5)
      const entered = await game.state.read()
      expect(entered.map.mapGroup).toBe(30)
      expect(entered.map.mapNum).toBe(104)
      await fs.promises.writeFile("/tmp/wayfarer-safari-beach-entry.png", await game.screenshot())

      await startNaturalSafariBattle(game)
      await fs.promises.writeFile("/tmp/wayfarer-safari-entry-menu.png", await game.screenshot())
      const battle = await game.state.read()
      expect(battle.battle.active).toBe(true)
      expect(battle.battle.trainerOnly.active).toBe(false)
      const partyVitals = battle.partyVitals

      for (let attempt = 0; attempt < 3; attempt++) {
        await selectSafariAction(game, "go-near")
        const reachedText = await waitForSafariText(game, `Safari Go Near ${attempt + 1}`)
        if (!reachedText)
          throw new Error(`Safari wild fled before closest Go Near at attempt ${attempt + 1}`)
        const ongoing = await settleSafariTurn(game, `settle Safari Go Near ${attempt + 1}`)
        if (!ongoing)
          throw new Error(`Safari wild fled before closest Go Near at attempt ${attempt + 1}`)
      }
      expect((await game.state.read()).partyVitals).toEqual(partyVitals)

      // Approach 4 is the closest-distance attempt (then the live Safari
      // flee calculation may end the encounter). This must still be accepted
      // as a turn; it is not a free no-op at the closest distance.
      await selectSafariAction(game, "go-near")
      await game.wait.frames(15)
      await fs.promises.writeFile(
        "/tmp/wayfarer-safari-closest-go-near.png",
        await game.screenshot(),
      )
      await dismissUntil(
        game,
        async () => {
          const state = await game.state.read()
          return state.ready && !state.battle.active
        },
        "closest Safari Go Near resolution",
      )

      // Enter a second live Safari encounter to exercise player Run separately
      // from the deliberate closest-distance flee outcome above.
      await startNaturalSafariBattle(game)
      await selectSafariAction(game, "run")
      await dismissUntil(
        game,
        async () => {
          const state = await game.state.read()
          return (
            state.ready &&
            !state.battle.active &&
            state.map.mapGroup === 30 &&
            state.map.mapNum === 104
          )
        },
        "Safari Run return",
      )

      // RETIRE is the first Safari start-menu action. Confirm the actual prompt,
      // then verify a later wild battle uses ordinary routing after Safari ends.
      await game.controls.press("start")
      await game.wait.frames(120)
      await fs.promises.writeFile("/tmp/wayfarer-safari-retire-menu.png", await game.screenshot())
      await game.controls.press("a")
      await dismissUntil(
        game,
        async () => {
          const state = await game.state.read()
          return state.ready && state.map.mapGroup === 30 && state.map.mapNum === 103
        },
        "Safari retirement exit",
      )

      await game.battle.startWild({ species: "rattata", level: 5 })
      await game.controls.press("a")
      await game.wait.frames(240)
      const afterRetirement = await game.state.read()
      expect(afterRetirement.battle.active).toBe(true)
      expect(afterRetirement.battle.trainerOnly.active).toBe(false)
      await fs.promises.writeFile(
        "/tmp/wayfarer-safari-post-retirement-wild.png",
        await game.screenshot(),
      )
    } finally {
      await game.close()
    }
  }, 240_000)
})
