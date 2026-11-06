import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  receiveBirchStarter,
} from "../playbooks/regional-opening"
import { rideAquaFromVermilionToSlateport } from "../playbooks/wayfarer-ports"

// Birch rates the Pokédex once the Petalburg Gym has been visited. The script he shares with
// Elm's old lab used to answer with Elm's errand to Mr. Pokémon instead. He never offers to
// register on the phone, which has no contact for him.

/** Reads on with B, which never starts a new talk, until the field is free again. */
const closeTalk = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 60; attempt++) {
    await game.wait.frames(45)
    const state = await game.state.read()
    if (state.ready && !state.scriptActive && !state.controlsLocked && !state.dialogueOpen) return
    await game.controls.press("b")
  }
  throw new Error(`Birch's talk did not end: ${JSON.stringify(await game.state.read())}`)
}

const talkToBirch = async (game: GameSession): Promise<string> => {
  await game.player.interact()
  await game.wait.until((state) => state.dialogueOpen, "Birch's line")
  return (await game.state.read()).dialogue.text
}

/** Enters the lab below Birch's desk, as someone who has been to the Petalburg Gym, and asks for the rating. */
const expectBirchRatesPokedex = async (game: GameSession): Promise<void> => {
  await game.story.setVar("hoennPetalburgGymState", 1)
  await game.player.warp("birch-lab", 6, 5, "up")
  await game.wait.forReady()

  // His first words are the rating question, not Emerald's offer to register on the phone.
  expect(await talkToBirch(game)).toContain("PROF. BIRCH: Ah,")

  // A reads on to the question and answers yes: he counts the Pokédex, then rates it.
  await advanceOpeningUntil(
    game,
    (state) => state.dialogue.text.startsWith("Hmhm"),
    "Birch did not count the Pokédex",
  )
  // Both players have caught fewer than ten.
  await advanceOpeningUntil(
    game,
    (state) => state.dialogue.text.startsWith("Look for POK"),
    "Birch did not rate the Pokédex",
  )
  await closeTalk(game)
  expect(await game.state.read()).toMatchObject({
    map: { name: "birch-lab" },
    player: { x: 6, y: 5 },
  })
  await expect(game.story.flag("hoennBirchRegistered")).resolves.toBe(false)
}

describe("Wayfarer Hoenn: Birch's Pokédex rating", () => {
  it("rates the Pokédex of a player who started in Hoenn", { timeout: 300_000 }, async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "hoenn-before-rescue",
        player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
        determinism: { textSpeed: "instant" },
      })
      await receiveBirchStarter(game, 1)
      // The starter comes with the Pokégear. Emerald's Birch then registers himself on the phone,
      // which has no Birch here: before the Gym he only says where the rival went.
      await game.player.warp("birch-lab", 6, 5, "up")
      await game.wait.forReady()
      expect(await talkToBirch(game)).toContain("Gone home")
      await closeTalk(game)
      await expectBirchRatesPokedex(game)
    } finally {
      await game.close()
    }
  })

  it("rates the Pokédex of a visitor who took his starter", { timeout: 300_000 }, async () => {
    const game = await GameSession.launch()
    try {
      // A Johto start that sails to Slateport, then the visitor's rescue and Birch's offer.
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: "vermilion-port-inside", x: 8, y: 9 } },
        story: {
          vars: { ssAquaState: 8 },
          flags: { disableEncounters: true, disableTrainers: true },
        },
        bag: { items: { ssTicket: 1 } },
        party: [{ species: "salamence", level: 60 }],
        determinism: { textSpeed: "instant" },
      })
      await rideAquaFromVermilionToSlateport(game)
      await game.player.warp("route-101", 9, 19, "right")
      await game.wait.forReady()
      await game.player.move("right")
      await finishOpeningDialogue(game)
      await game.player.warp("route-101", 7, 15, "up")
      await game.player.interact()
      await advanceOpeningUntil(
        game,
        (state) => state.battle.ui === "action-menu",
        "The visitor's rescue battle did not start",
      )
      await game.battle.win()
      await advanceOpeningUntil(
        game,
        (state) => state.dialogue.text.includes("You handled"),
        "Birch did not offer the visitor a starter",
      )
      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.frames(45)
      await game.controls.press("a")
      await game.wait.frames(45)
      await game.controls.press("down")
      await game.controls.press("a")
      await advanceOpeningUntil(
        game,
        (state) => state.dialogue.text.includes("will guide"),
        "Birch did not offer the chosen starter",
      )
      await game.wait.frames(45)
      await game.controls.press("a")
      await game.wait.frames(45)
      await game.controls.press("a")
      // B from here: it reads the messages on and answers the nickname question with no.
      await advanceOpeningUntil(
        game,
        (state) => state.ready && state.origin.hoennReceived,
        "Birch did not hand over the starter",
        "b",
      )
      await closeTalk(game)
      await expectBirchRatesPokedex(game)
    } finally {
      await game.close()
    }
  })
})
