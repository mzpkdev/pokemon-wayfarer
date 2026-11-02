import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  boardFerryAtDock,
  goAshoreFromExitSailor,
  sailFerryLeg,
  waitAboard,
} from "../playbooks/wayfarer-ports"

// The S.S. ferries (.product/specs/ss-ferries.md), part 1: boarding, sailing, arriving and going ashore.
// A new game starts at 8:00 on day 1, and `clock.setTimeOfDay` never changes the date, so each test keeps
// to the morning and afternoon, or moves the date explicitly.

const readyOn = async (game: GameSession, map: string): Promise<boolean> => {
  const state = await game.state.read()
  return state.ready && state.map.name === map
}

describe.sequential("Wayfarer S.S. ferries", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("rides the S.S. Aqua from Olivine to Vermilion, saving and continuing at sea", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: { map: "olivine-port-inside", x: 8, y: 16 } },
      story: { vars: { ssAquaState: 8 } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })

    await boardFerryAtDock(game)
    await waitAboard(game, "ss-aqua-1f")
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 29, y: 3 } })

    // Saving aboard in port and continuing resumes on the same leg.
    await game.saveAndReload()
    await game.wait.forMap("ss-aqua-1f")

    // Cast-off and arrival, then ashore at the port the ship is at: Vermilion's berth at (8,9).
    await sailFerryLeg(game)
    await game.saveAndReload()
    await game.wait.forMap("ss-aqua-1f")
    await goAshoreFromExitSailor(game, "up", "vermilion-port-inside")
    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "vermilion-port-inside" },
      player: { x: 8, y: 9 },
    })
    await expect(game.inventory.contains("ssTicket")).resolves.toBe(true)
  })

  it("keeps the exit closed at sea: the exit sailor says so and nothing happens", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: { map: "olivine-port-inside", x: 8, y: 16 } },
      story: { vars: { ssAquaState: 8 } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await boardFerryAtDock(game)
    await waitAboard(game, "ss-aqua-1f")

    // Past cast-off (8:30) and before arrival (14:30): at sea.
    await game.clock.setTimeOfDay({ hour: 9, minute: 0, second: 0 })
    for (let attempt = 0; attempt < 20 && !(await readyOn(game, "ss-aqua-1f")); attempt++) {
      await game.wait.frames(30)
      await game.controls.press("a")
    }
    await game.wait.frames(120)
    await game.controls.press("a")
    await game.wait.frames(30)
    await game.controls.press("up")
    await game.wait.frames(12)
    await game.player.interact()
    for (let attempt = 0; attempt < 6; attempt++) {
      await game.wait.frames(30)
      await game.controls.press("a")
    }
    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "ss-aqua-1f" },
    })
  })

  it("catches up after a day away: the Anne leaves Vermilion at night and arrives where the clock says", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: { map: "vermilion-port-inside", x: 8, y: 9 } },
      story: { vars: { ssAquaState: 8 }, flags: { ssAnneCaptainRewarded: true } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })
    // Night at Vermilion: the Anne is in port and the first row of the sailor's menu.
    await game.clock.setTimeOfDay({ hour: 20, minute: 30, second: 0 })
    await game.wait.frames(60)
    await game.player.interact()
    await game.wait.frames(30)
    await game.controls.press("a")
    await waitAboard(game, "ss-anne-1f-corridor")
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 19, y: 2 } })

    // Cast-off half an hour later, then a whole day passes while the player is busy: every elapsed
    // transition is applied in order and only the last is announced.
    await game.clock.setTimeOfDay({ hour: 21, minute: 1, second: 0 })
    await game.wait.frames(120)
    await game.controls.press("a")
    await game.wait.frames(30)
    await game.clock.advanceDays(1)
    await game.wait.frames(240)
    for (let attempt = 0; attempt < 10; attempt++) {
      await game.controls.press("a")
      await game.wait.frames(30)
    }

    // At sea the gangway only returns the player to the corridor's own entry tile.
    await game.controls.press("up")
    await game.wait.frames(60)
    await expect(game.state.read()).resolves.toMatchObject({ map: { name: "ss-anne-1f-corridor" } })

    // Arrival at Vermilion (22:30), and the gangway leads to Vermilion's berth.
    await game.clock.setTimeOfDay({ hour: 22, minute: 31, second: 0 })
    await game.wait.frames(120)
    for (let attempt = 0; attempt < 6; attempt++) {
      await game.controls.press("a")
      await game.wait.frames(30)
    }
    await game.controls.press("up")
    await game.wait.forMap("vermilion-port-inside")
    await expect(game.state.read()).resolves.toMatchObject({ ready: true, player: { x: 8, y: 9 } })
  })
  it("offers Lilycove's Battle Frontier charter once Scott has invited the player, next to the ship in port", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "lilycove-city-harbor", x: 8, y: 11 } },
      story: { vars: { ssAquaState: 8 }, flags: { metScottOnSsTidal: true } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })
    // Rows: the ship in port (the Aqua by day), BATTLE FRONTIER, CANCEL. Pick the charter and confirm.
    await game.player.interact()
    await game.wait.frames(120)
    // "May I see your ticket?" waits for a press, then the menu opens on its first row. Give it time to open:
    // a press before it does nothing, and the next A would take the ship row.
    await game.controls.press("a")
    await game.wait.frames(240)
    await game.controls.press("down")
    await game.wait.frames(20)
    await game.controls.press("a")
    for (let attempt = 0; attempt < 120; attempt++) {
      if (await readyOn(game, "battle-frontier-outside-west")) break
      await game.wait.frames(30)
      await game.controls.press("a")
    }
    await expect(game.state.read()).resolves.toMatchObject({
      map: { name: "battle-frontier-outside-west" },
    })
  })

  it("boards the ship in port directly when no charter is available", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "lilycove-city-harbor", x: 8, y: 11 } },
      story: { vars: { ssAquaState: 8 } },
      bag: { items: { ssTicket: 1 } },
      determinism: { textSpeed: "instant" },
    })
    await boardFerryAtDock(game)
    await waitAboard(game, "ss-aqua-1f")
  })
})
