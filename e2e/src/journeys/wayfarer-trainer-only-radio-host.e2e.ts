import { describe, expect, it } from "webanvil/test"

import { GameSession, type GameState } from "../harness/game-session"
import { storyFlags, storyVars } from "../harness/game-session/catalog"

// These are deliberately file-local: the two probes exercise a narrow Radio
// continuation without making campaign-specific setup part of the shared API.
const fixtureFlags = storyFlags as Record<string, number>
const fixtureVars = storyVars as Record<string, number>

const registerFlag = (name: string, id: number): string => {
  fixtureFlags[name] = id
  return name
}

const registerVar = (name: string, id: number): string => {
  fixtureVars[name] = id
  return name
}

const lugiaOrHooh = registerVar("radio-host-lugia-or-hooh", 0x4071)
const hideGoldenrodDirector = registerFlag("radio-host-hide-director", 0x069)
const hideGoldenrodPetrel = registerFlag("radio-host-hide-petrel", 0x06b)
const hideGoldenrodRockets = registerFlag("radio-host-hide-rockets", 0x06c)
const hideUndergroundSilver = registerFlag("radio-host-hide-underground-silver", 0x06f)
const silverGoldenrodComplete = "silverGoldenrodUndergroundComplete"

const waitForDialogue = async (
  game: GameSession,
  expected: string,
  description: string,
  advance: boolean,
): Promise<GameState> => {
  const observed = new Set<string>()
  for (let attempt = 0; attempt < 480; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text) observed.add(state.dialogue.text)
    if (state.dialogue.text.includes(expected)) return state
    if (advance && (state.dialogueOpen || state.battle.ui === "text" || state.scriptActive))
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not show ${JSON.stringify(expected)}; observed ${JSON.stringify([...observed])}`,
  )
}

const waitForTrainerBattle = async (game: GameSession, description: string): Promise<GameState> => {
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.battle.active) {
      expect(state.battle.trainerOnly.active).toBe(false)
      expect(state.battle.enemy).not.toBeNull()
      return state
    }
    if (state.dialogueOpen || state.battle.ui === "text" || state.scriptActive)
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not enter a live trainer battle: ${JSON.stringify(await game.state.read())}`,
  )
}

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready && !state.dialogueOpen && !state.scriptActive) return
    if (
      state.ui.mode === "party-menu" ||
      state.dialogueOpen ||
      state.battle.ui === "text" ||
      state.scriptActive
    )
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not finish: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer Radio Tower native continuations", () => {
  it("uses forced native wins to prove Archer's host writer and the pending Silver return writer", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "goldenrod-radio-tower-5f", x: 25, y: 13 },
        },
        story: {
          vars: { goldenrodCityState: 9, starterMon: 0, [lugiaOrHooh]: 1 } as never,
          flags: {
            johtoStarterChoiceCommitted: true,
            [hideGoldenrodRockets]: false,
            [hideGoldenrodDirector]: true,
            [hideGoldenrodPetrel]: false,
            [hideUndergroundSilver]: true,
            [silverGoldenrodComplete]: false,
          } as never,
        },
        party: [{ species: "rattata", level: 5, moves: ["tackle"] }],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })

      await game.player.interact()
      await waitForTrainerBattle(game, "Radio Tower Archer")

      // This deliberately bypasses battle tactics. It proves that a real Archer
      // trainerbattle resumes the authored native winner continuation, rather
      // than treating an arranged state as evidence of that writer.
      await game.battle.win()
      await waitForDialogue(game, "SILVER WING", "Archer native Wing handoff", true)
      await finishFieldScript(game, "Archer native occupation completion")

      expect(await game.story.var("goldenrodCityState")).toBe(10)
      expect(await game.story.flag(hideGoldenrodRockets as never)).toBe(true)
      expect(await game.story.flag(hideGoldenrodPetrel as never)).toBe(true)
      expect(await game.story.flag(hideGoldenrodDirector as never)).toBe(false)
      expect(await game.story.flag(silverGoldenrodComplete as never)).toBe(false)

      await game.player.warp("goldenrod-underground-switches", 34, 2, "right")
      await game.wait.forReady()
      expect(await game.story.flag(hideUndergroundSilver as never)).toBe(false)
      await game.player.interact()
      await waitForTrainerBattle(game, "pending Underground Silver after Archer")

      // A second forced outcome proves Silver's distinct return writer: it may
      // complete Silver's chapter but must not rewind Archer's host state.
      await game.battle.win()
      await finishFieldScript(game, "Underground Silver return completion")
      expect(await game.story.flag(silverGoldenrodComplete as never)).toBe(true)
      expect(await game.story.var("goldenrodCityState")).toBe(10)
    } finally {
      await game.close()
    }
  })

  it("uses a forced native loss to restore the fake Director after Petrel transforms", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: {
          facing: "up",
          position: { map: "goldenrod-radio-tower-5f", x: 11, y: 10 },
        },
        story: {
          vars: { goldenrodCityState: 6 },
          flags: {
            johtoStarterChoiceCommitted: true,
            [hideGoldenrodDirector]: false,
            [hideGoldenrodPetrel]: true,
            [hideGoldenrodRockets]: false,
          } as never,
        },
        party: [{ species: "rattata", level: 5, moves: ["tackle"] }],
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })

      const before = await game.state.read()
      await game.player.interact()
      await waitForDialogue(game, "Y-you", "fake Director opening", false)
      const transformed = await waitForDialogue(
        game,
        "Is that what you were expecting",
        "Petrel transformation",
        true,
      )
      expect(transformed.battle.active).toBe(false)
      expect(await game.story.flag(hideGoldenrodDirector as never)).toBe(true)
      expect(await game.story.flag(hideGoldenrodPetrel as never)).toBe(false)

      await game.controls.press("a")
      await waitForTrainerBattle(game, "transformed Radio Tower Petrel")

      // This is intentionally a forced loss continuation probe. The existing
      // actual-loss journeys own normal damage and money behavior; this unique
      // Radio case verifies the authored actor restoration after transformation.
      await game.battle.lose()
      await finishFieldScript(game, "fake Director Petrel retreat")

      const restored = await game.state.read()
      expect(restored).toMatchObject({
        map: before.map,
        player: before.player,
        ready: true,
        battle: { active: false },
      })
      expect(await game.story.var("goldenrodCityState")).toBe(6)
      expect(await game.story.flag(hideGoldenrodDirector as never)).toBe(false)
      expect(await game.story.flag(hideGoldenrodPetrel as never)).toBe(true)

      const moneyAfterRetreat = restored.money
      await game.player.interact()
      await waitForDialogue(
        game,
        "This is no place for you",
        "restored fake Director refusal",
        false,
      )
      await finishFieldScript(game, "restored fake Director refusal")
      expect((await game.state.read()).money).toBe(moneyAfterRetreat)
    } finally {
      await game.close()
    }
  })
})
