import fs from "node:fs/promises"
import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { storyFlags } from "../harness/game-session/catalog"

const nursePosition = { map: "cherrygrove-pokemon-center" as const, x: 7, y: 3 }
const fixtureFlags = storyFlags as Record<string, number>
fixtureFlags["even-faster-joy"] = 0x960 // FLAG_EVEN_FASTER_JOY

const finishNativeNurseVisit = async (game: GameSession, name: string) => {
  await game.player.interact()
  await game.wait.until((state) => state.dialogueOpen, `${name} Nurse Joy rest prompt`)

  let tookParty = false

  for (let attempt = 0; attempt < 800; attempt++) {
    const state = await game.state.read()
    // The E2E Gen-3 text decoder intentionally omits apostrophes.
    if (state.dialogue.text.includes("Ill take your")) tookParty = true
    if (state.ready && !state.controlsLocked && !state.scriptActive && !state.dialogueOpen) {
      expect(tookParty, `${name} must use the animated Nurse Joy branch`).toBe(true)
      return state
    }

    // The native Yes/No menu does not count as an open dialogue in the E2E
    // state. Acknowledging every bounded poll accepts it and later return
    // messages; input during the healing effect itself is ignored.
    await game.controls.press("a")
    await game.wait.frames(6)
  }

  await fs.writeFile(
    `/tmp/trainer-only-pokemon-center-${name}-timeout.png`,
    await game.screenshot(),
  )
  throw new Error(
    `${name} nurse visit did not release field control: ${JSON.stringify(await game.state.read())}`,
  )
}

const assertFieldControlReleased = async (game: GameSession, expectedY: number) => {
  await game.player.move("down")
  await game.wait.until(
    (state) =>
      state.ready &&
      !state.controlsLocked &&
      !state.scriptActive &&
      !state.dialogueOpen &&
      state.player.y === expectedY,
    "field input after Nurse Joy",
    240,
  )
}

describe.sequential("Trainer-only Pokémon Center recovery", () => {
  it("returns an empty party from Nurse Joy without leaving the healing effect active", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: nursePosition },
        party: [],
        story: { flags: { ["even-faster-joy"]: false } as never },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })

      const released = await finishNativeNurseVisit(game, "empty-party")
      expect(released.party).toHaveLength(0)
      expect(released.battle.active).toBe(false)
      await assertFieldControlReleased(game, nursePosition.y + 1)
    } finally {
      await game.close()
    }
  })

  it("restores a real fainted party member and releases field controls", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "up", position: nursePosition },
        party: [{ species: "rattata", fainted: true }],
        story: { flags: { ["even-faster-joy"]: false } as never },
        determinism: { textSpeed: "instant", rngSeed: 1 },
      })

      const released = await finishNativeNurseVisit(game, "fainted-party")
      expect(released.party).toMatchObject([{ species: "rattata", fainted: false }])
      expect(released.partyVitals[0]?.hp).toBeGreaterThan(0)
      expect(released.battle.active).toBe(false)
      await assertFieldControlReleased(game, nursePosition.y + 1)
    } finally {
      await game.close()
    }
  })
})
