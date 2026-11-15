import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

// Elm's egg call (.product/specs/johto-independent-story-beats.md#elms-egg-call). The Violet Egg
// handoff owes the call; the first Johto town loaded afterwards plays it once, whether or not the
// player has been there before, and frees the field.

// The player stands inside Cherrygrove's Pokémon Center (no call there) and walks out of the door, so
// Cherrygrove loads the way it does in play. The Guide Gent's tour is already over.
const center = { map: "cherrygrove-pokemon-center", x: 7, y: 7 } as const
const cherrygroveToured = { cherrygroveCityState: 3 }

// Walks out of the door and answers whatever call rings in Cherrygrove: each text the field and
// the phone's call box showed, or an empty list when nothing rings.
const walkOutside = async (game: GameSession): Promise<string[]> => {
  for (let attempt = 0; attempt < 20; attempt++) {
    if ((await game.state.read()).map.name === "cherrygrove-city") return game.leagues.answerCall()
    await game.player.move("down")
    await game.wait.frames(16)
  }
  throw new Error(`Did not leave the Pokémon Center: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Johto Elm egg call", () => {
  it("plays the owed call on entering a Johto town, once", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: center },
        story: {
          vars: cherrygroveToured,
          flags: { receivedTogepiEgg: true, elmEggCallOwed: true },
        },
        party: [{ species: "rattata", level: 5 }],
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })
      const said = (await walkOutside(game)).join("\n")
      expect(said).toMatch(/Has that EGG hatched yet\?/)
      await expect(game.story.flag("elmEggCallOwed")).resolves.toBe(false)
      expect((await game.state.read()).map.name).toBe("cherrygrove-city")

      // Settled for good: a save and reload in town plays nothing.
      await game.saveAndReload()
      expect((await game.leagues.answerCall()).join("\n")).not.toMatch(/PROF\.ELM/)
      await expect(game.story.flag("elmEggCallOwed")).resolves.toBe(false)
    } finally {
      await game.close()
    }
  }, 600_000)

  it("hears the Egg has hatched when Togepi is in the party", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: center },
        story: {
          vars: cherrygroveToured,
          flags: { receivedTogepiEgg: true, elmEggCallOwed: true },
        },
        party: [
          { species: "rattata", level: 5 },
          { species: "togepi", level: 1 },
        ],
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })
      const said = (await walkOutside(game)).join("\n")
      expect(said).toMatch(/It hatched\?/)
      expect(said).not.toMatch(/Has that EGG hatched yet\?/)
      await expect(game.story.flag("elmEggCallOwed")).resolves.toBe(false)
    } finally {
      await game.close()
    }
  }, 600_000)

  it("plays nothing once Elm has seen the Togepi", async () => {
    const game = await GameSession.launch()
    try {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: center },
        story: {
          vars: cherrygroveToured,
          flags: {
            receivedTogepiEgg: true,
            elmEggCallOwed: true,
            shownElmTogepi: true,
          },
        },
        party: [{ species: "rattata", level: 5 }],
        determinism: { rngSeed: 1, textSpeed: "instant" },
      })
      expect((await walkOutside(game)).join("\n")).not.toMatch(/PROF\.ELM/)
      await expect(game.story.flag("elmEggCallOwed")).resolves.toBe(false)
    } finally {
      await game.close()
    }
  }, 600_000)
})
