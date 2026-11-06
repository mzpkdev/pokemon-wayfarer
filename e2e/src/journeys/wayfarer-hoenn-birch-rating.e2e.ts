import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  receiveBirchStarter,
} from "../playbooks/regional-opening"
import { rideAquaFromVermilionToSlateport } from "../playbooks/wayfarer-ports"

// Birch rates the Pokédex once his own opening is over: for a Hoenn-origin player once he has given
// the Pokédex, for a visitor once he has been rescued. The Petalburg Gym plays no part. He never
// offers to register on the phone, which has no contact for him.

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

/** A reads on to the question and answers yes: he counts the Pokédex, then rates it. */
const readRating = async (game: GameSession): Promise<void> => {
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
}

const talkToBirch = async (game: GameSession): Promise<string> => {
  await game.player.interact()
  await game.wait.until((state) => state.dialogueOpen, "Birch's line")
  return (await game.state.read()).dialogue.text
}

/** Asks for the rating, reading on from the question, with the Gym never entered. */
const expectBirchRatesPokedex = async (game: GameSession): Promise<void> => {
  // His first words are the rating question, not Emerald's offer to register on the phone.
  expect(await talkToBirch(game)).toContain("PROF. BIRCH: Ah,")
  await readRating(game)
  await expect(game.story.flag("hoennBirchRegistered")).resolves.toBe(false)
  await expect(game.story.var("hoennPetalburgGymState")).resolves.toBe(0)
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
      // Before the Pokédex he only says where the rival went.
      await game.player.warp("birch-lab", 6, 5, "up")
      await game.wait.forReady()
      expect(await talkToBirch(game)).toContain("Gone home")
      await closeTalk(game)

      // The Route 103 win sends the player back to the lab for the Pokédex.
      await game.player.warp("route-103", 10, 4, "up")
      await game.player.interact()
      await advanceOpeningUntil(game, (state) => state.battle.active, "The rival did not battle")
      await game.battle.win()
      await finishOpeningDialogue(game)
      await game.player.warp("littleroot-town", 7, 17, "up")
      await game.player.move("up")
      await advanceOpeningUntil(game, (state) => state.origin.pokedex, "Birch gave no Pokédex")
      await finishOpeningDialogue(game)

      // He rates it right away, in the same visit.
      await expect(game.story.var("hoennBirchLabState")).resolves.toBe(5)
      expect((await game.state.read()).map.name).toBe("birch-lab")
      await game.controls.press("up")
      await game.wait.frames(30)
      await expectBirchRatesPokedex(game)
    } finally {
      await game.close()
    }
  })

  it(
    "thanks and rates a visitor right after the rescue, and gives no starter",
    {
      timeout: 300_000,
    },
    async () => {
      const game = await GameSession.launch()
      try {
        // A Johto start that sails to Slateport, then the visitor's rescue.
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
          "Birch did not thank the visitor",
        )
        await readRating(game)
        expect(await game.state.read()).toMatchObject({
          map: { name: "birch-lab" },
          // This visitor skipped Mr. Pokémon, so Birch hands over a Pokédex to rate.
          origin: { hoennChoice: 0xffff, hoennReceived: false, pokedex: true },
          party: [{ species: "salamence" }],
        })

        // A later talk is the rating, not an offer.
        await expectBirchRatesPokedex(game)

        // A new visit changes nothing.
        await game.player.warp("birch-lab", 6, 5, "up")
        await game.wait.forReady()
        await expectBirchRatesPokedex(game)
      } finally {
        await game.close()
      }
    },
  )
})
