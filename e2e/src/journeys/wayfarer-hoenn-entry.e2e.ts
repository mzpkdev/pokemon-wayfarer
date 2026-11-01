import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession, type Direction } from "../harness/game-session"
import {
  boardFerryAtDock,
  goAshoreFromExitSailor,
  rideAquaFromVermilionToSlateport,
  sailFerryLeg,
  waitAboard,
} from "../playbooks/wayfarer-ports"

const moveOneTile = async (game: GameSession, direction: Direction): Promise<void> => {
  await game.wait.forReady()
  const before = await game.state.read()
  for (let attempt = 0; attempt < 3; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(12)
    const after = await game.state.read()
    if (
      after.map.name !== before.map.name ||
      after.player.x !== before.player.x ||
      after.player.y !== before.player.y
    )
      return
  }
  throw new Error(`Could not move ${direction}: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer Hoenn entry", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("rides the S.S. Aqua to Slateport and the S.S. Tidal on to Lilycove, entering Hoenn on the way", async () => {
    // The Johto start with its maiden voyage done and the Ticket in the Bag; a new game starts at 8:00.
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: {
        facing: "down",
        position: { map: "vermilion-port-inside", x: 8, y: 9 },
      },
      story: { vars: { ssAquaState: 8 } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })

    // Aqua by day: Vermilion -> Slateport, one leg of six in-game hours.
    await rideAquaFromVermilionToSlateport(game)

    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "slateport-city-harbor" },
      player: { x: 9, y: 11 },
    })
    // Going ashore in Hoenn for the first time set up Hoenn and the respawn.
    expect((await game.state.read()).origin.hoennInitialized).toBe(true)

    await game.saveAndReload()

    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "slateport-city-harbor" },
      player: { x: 9, y: 11 },
    })
    await expect(game.story.var("ssAquaState")).resolves.toBe(8)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)

    // The Tidal is Slateport's ship by day: step to the attendant at (8,10), who boards with no question.
    await moveOneTile(game, "left")
    await game.controls.press("up")
    await game.wait.frames(12)
    await boardFerryAtDock(game)
    await waitAboard(game, "ss-tidal-corridor")
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 1, y: 10 } })

    // Saving aboard resumes on the same leg.
    await game.saveAndReload()
    await game.wait.forMap("ss-tidal-corridor")

    // Tidal: Slateport -> Lilycove, and the exit sailor is below the entry tile.
    await sailFerryLeg(game)
    await goAshoreFromExitSailor(game, "down", "lilycove-city-harbor")
    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "lilycove-city-harbor" },
      player: { x: 8, y: 11 },
    })
    await expect(game.story.var("ssAquaState")).resolves.toBe(8)
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)
  })
})
