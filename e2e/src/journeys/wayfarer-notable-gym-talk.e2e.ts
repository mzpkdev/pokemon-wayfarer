import { mkdir, writeFile } from "node:fs/promises"
import path from "node:path"

import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

// A Gym Leader's own Gym (notable-world-simulation.md#interactions, "Own Gym"): while the Gym still owes the
// player something the script runs unchanged (here, the badge battle); once nothing is owed the talk is the
// friendship greeting, the Gym's existing post-battle line and the farewell. Brock's Gym in Pewter.
const brockIntro = "PEWTER GYM LEADER"
const brockAgain = "Oh, hi again!"
const brockPostBattle = "The world is huge"
const brockBye = "keep your"

const evidenceDirectory = process.env.E2E_EVIDENCE_DIR

// Presses through whatever the script shows until the field is free again; returns each distinct text seen.
const settle = async (game: GameSession, description: string): Promise<string[]> => {
  const seen: string[] = []
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.fullText && seen.at(-1) !== state.dialogue.fullText) seen.push(state.dialogue.fullText)
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.battle.active) return seen
    if (state.battle.active || state.dialogueOpen || state.scriptActive || state.controlsLocked) {
      await game.wait.frames(20)
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(`${description} did not release the field: ${JSON.stringify(await game.state.read())}`)
}

const capture = async (game: GameSession, name: string): Promise<void> => {
  if (!evidenceDirectory) return
  await mkdir(evidenceDirectory, { recursive: true })
  await writeFile(path.join(evidenceDirectory, `${name}.png`), await game.screenshot())
}

describe.sequential("Wayfarer own-Gym talk", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("keeps the badge battle while owed, then greets, repeats the post-battle line and says goodbye", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      // Brock stands at (6, 5); the dojo copy is hidden, so the trade does not wait for the player.
      player: { facing: "up", position: { map: "pewter-gym", x: 6, y: 6 } },
      party: [{ species: "lapras", level: 100 }],
      story: { flags: { disableTrainers: true, hideDojoBrock: true } },
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })

    // Unbeaten: the Gym script still owns the talk and starts the badge battle, with no greeting before it.
    await game.player.interact()
    const said: string[] = []
    for (let attempt = 0; attempt < 600; attempt++) {
      const state = await game.state.read()
      if (state.dialogue.fullText && said.at(-1) !== state.dialogue.fullText) said.push(state.dialogue.fullText)
      if (state.battle.ui === "action-menu") break
      if (state.battle.ui === "text" || (!state.battle.active && (state.dialogueOpen || state.scriptActive)))
        await game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
      else await game.wait.frames(4)
    }
    expect((await game.state.read()).battle.ui).toBe("action-menu")
    expect(said.join("\n")).toContain(brockIntro)
    expect(said.join("\n")).not.toContain(brockAgain)
    await capture(game, "gym-talk-1-unbeaten-battle")

    await game.battle.win()
    await settle(game, "Brock's win")
    expect(await game.story.flag("defeatedPewterGym")).toBe(true)

    // Beaten, nothing owed: greeting by friendship (the win was credited), the existing line, the farewell.
    await game.player.interact()
    await game.wait.frames(120)
    await capture(game, "gym-talk-2-greeting")
    const texts: string[] = []
    for (let attempt = 0; attempt < 400; attempt++) {
      const state = await game.state.read()
      if (state.dialogue.fullText && texts.at(-1) !== state.dialogue.fullText) {
        texts.push(state.dialogue.fullText)
        if (texts.length > 1) await capture(game, `gym-talk-2-line-${texts.length}`)
      }
      if (state.ready && !state.dialogueOpen && !state.scriptActive && texts.length > 0) break
      if (state.dialogueOpen || state.scriptActive) await game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
      await game.wait.frames(20)
    }
    expect(texts).toHaveLength(3)
    expect(texts[0]).toContain(brockAgain)
    expect(texts[1]).toContain(brockPostBattle)
    expect(texts[2]).toContain(brockBye)
  })
})
