import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

describe.sequential("HNS native HM route boundaries with synthetic learned-move fixtures", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("Surfs from Blackthorn's fishing bank and returns without HM03 or badge 8", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "blackthorn-city", x: 18, y: 26 } },
      story: { vars: { blackthornCityState: 2 }, flags: { badge8: false } },
      party: [{ species: "poliwag", level: 10, moves: ["surf"] }],
      determinism: { textSpeed: "instant" },
    })

    await expect(game.state.read()).resolves.toMatchObject({
      player: { surfing: false },
      bag: { hms: { surf: 0 } },
    })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await expect(game.state.read()).resolves.toMatchObject({
      dialogue: { message: "want-to-use-surf" },
      fieldMove: { move: "surf", user: 0, userSpecies: "poliwag", result: "found" },
    })
    await game.wait.until(
      (state) => state.dialogue.message === "want-to-use-surf" && !state.dialogueOpen,
      "Blackthorn Surf prompt",
    )
    await game.wait.frames(12)
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.dialogue.message === "player-used-surf",
      "Blackthorn Surf confirmation",
    )
    await game.dialogue.waitForClosed()
    await game.wait.frames(60)
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.ready && state.player.surfing && state.player.y === 25,
      "enter Blackthorn's northern pool",
      3_600,
    )

    await game.player.move("down")
    await game.wait.until(
      (state) => state.ready && state.player.facing === "down",
      "face Blackthorn's fishing bank",
    )
    await game.wait.frames(20)
    if ((await game.state.read()).player.y === 25) await game.player.move("down")
    await game.wait.until(
      (state) => state.ready && !state.player.surfing && state.player.y === 26,
      "return to Blackthorn's fishing bank",
    )
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "blackthorn-city" },
      player: { x: 18, y: 26, surfing: false },
      bag: { hms: { surf: 0 } },
      party: [{ species: "poliwag", moves: expect.arrayContaining(["surf"]) }],
    })
    await expect(game.story.flag("badge8")).resolves.toBe(false)
    await expect(game.story.var("blackthornCityState")).resolves.toBe(2)
  })

  for (const x of [15, 16]) {
    it(`crosses the Den's southwest Whirlpool lane x=${x} and returns without HM06 or badge 8`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: "dragons-den-cavern", x, y: 37 } },
        story: { vars: { blackthornCityState: 2 }, flags: { badge8: false } },
        party: [
          { species: "poliwag", level: 10, moves: ["surf"] },
          { species: "dratini", level: 6, moves: ["whirlpool"] },
        ],
        determinism: { textSpeed: "instant" },
      })

      for (const y of [40, 37]) {
        if (y === 37) {
          await game.player.move("up")
          await game.wait.frames(12)
        }
        await game.player.interact()
        await game.dialogue.waitForOpen()
        await expect(game.state.read()).resolves.toMatchObject({
          dialogue: { message: "field-move-used", text: "DRATINI used WHIRLPOOL!" },
          fieldMove: { move: "whirlpool", user: 1, userSpecies: "dratini", result: "found" },
          bag: { hms: { surf: 0, whirlpool: 0 } },
        })
        await game.wait.frames(60)
        await game.controls.press("a")
        await game.wait.until(
          (state) => state.ready && state.player.x === x && state.player.y === y,
          `cross Den Whirlpool to ${x}:${y}`,
        )
        await expect(game.story.flag("badge8")).resolves.toBe(false)
      }

      await expect(game.state.read()).resolves.toMatchObject({
        map: { name: "dragons-den-cavern" },
        player: { x, y: 37 },
      })
      await expect(game.story.var("blackthornCityState")).resolves.toBe(2)
    })

    it(`blocks the Den's southwest Whirlpool lane x=${x} without a carrier or HM`, async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: "dragons-den-cavern", x, y: 37 } },
        story: { vars: { blackthornCityState: 2 }, flags: { badge8: false } },
        party: [{ species: "poliwag", level: 10, moves: ["surf"] }],
        determinism: { textSpeed: "instant" },
      })

      await game.player.move("down")
      await game.wait.frames(12)
      await game.player.interact()
      await game.dialogue.waitForOpen()
      await expect(game.state.read()).resolves.toMatchObject({
        dialogue: { message: "field-move-needs-hm" },
        fieldMove: { move: "whirlpool", result: "missing-item" },
        player: { x, y: 37 },
        bag: { hms: { whirlpool: 0 } },
      })
      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.forReady()
      await expect(game.story.flag("badge8")).resolves.toBe(false)
    })
  }
})
