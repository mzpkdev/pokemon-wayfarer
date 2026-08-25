import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

describe.sequential("Elm dialogue journey", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("triggers Elm's opening dialogue", async () => {
    await game.arrange({
      checkpoint: "elm-lab-before-intro",
      player: {
        facing: "up",
        position: { map: "elm-lab", x: 6, y: 8 },
      },
      story: {
        flags: {
          hideSilverInNewBark: true,
        },
        vars: {
          newBarkTownLabState: 0,
          newBarkTownState: 2,
        },
      },
      determinism: {
        rngSeed: 1,
        textSpeed: "instant",
      },
    })

    await expect(game.state.read()).resolves.toMatchObject({
      phase: "overworld",
      ready: true,
      map: { mapGroup: 1, mapNum: 0 },
      player: { x: 6, y: 8, facing: "up" },
    })

    await game.player.move("up")
    await game.dialogue.waitForOpen()

    await expect(game.state.read()).resolves.toMatchObject({
      phase: "dialogue",
      dialogueOpen: true,
      controlsLocked: true,
    })
    await expect(game.story.flag("hideSilverInNewBark")).resolves.toBe(true)
    await expect(game.story.var("newBarkTownLabState")).resolves.toBe(0)
  })
})
