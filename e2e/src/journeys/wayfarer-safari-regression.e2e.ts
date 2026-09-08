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

const settleSafariTurn = async (game: GameSession, description: string): Promise<boolean> => {
  // Safari's shared battle-string controller clears before a 15-frame E2E
  // poll can observe it at instant text speed. The action itself is already
  // selected, so acknowledge its live message then allow the Safari AI to
  // finish the committed turn before inspecting the result.
  await game.wait.frames(60)
  if (!(await game.state.read()).battle.active) return false
  await game.controls.press("a")
  await game.wait.frames(360)
  const state = await game.state.read()
  if (state.battle.active || state.ready) return state.battle.active
  throw new Error(`${description}: ${JSON.stringify(state)}`)
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
  // The Safari menu is visible before its controller finishes the background
  // DMA that enables directional input. Let that live transition settle so
  // the action below is not discarded by HandleChooseActionAfterDma3.
  await game.wait.frames(30)
}

describe.sequential("Wayfarer Safari regressions", () => {
  it("preserves Safari entry, Go Near, Run, and retirement outside trainer-only mode", async () => {
    const game = await GameSession.launch()
    try {
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

      // A wild Safari response is intentionally random. This confirms a real
      // Go Near action commits, then accepts either ordinary continuation or
      // the normal Safari field return. The shared mechanics suite covers the
      // deterministic +4/+3/+2/+1 and saturated repeat sequence.
      await selectSafariAction(game, "go-near")
      await game.wait.frames(15)
      await fs.promises.writeFile("/tmp/wayfarer-safari-go-near-action.png", await game.screenshot())
      const ongoing = await settleSafariTurn(game, "settle Safari Go Near")
      expect((await game.state.read()).partyVitals).toEqual(partyVitals)

      // Use a live continuing battle when possible, otherwise enter a second
      // ordinary Safari encounter through its real grass table for Run.
      if (!ongoing) {
        await dismissUntil(
          game,
          async () => {
            const state = await game.state.read()
            return state.ready && !state.battle.active
          },
          "Safari Go Near field return",
        )
        await startNaturalSafariBattle(game)
      }
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
