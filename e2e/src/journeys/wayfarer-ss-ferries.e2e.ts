import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import {
  boardFerryAtDock,
  goAshoreFromExitSailor,
  sailFerryLeg,
  waitAboard,
} from "../playbooks/wayfarer-ports"

// The S.S. ferries (.product/specs/ss-ferries.md): boarding, sailing, arriving and going ashore, and the cabins.
// A new game starts at 8:00 on day 1, and `clock.setTimeOfDay` never changes the date, so each test keeps
// to the morning and afternoon, or moves the date explicitly.

const readyOn = async (game: GameSession, map: string): Promise<boolean> => {
  const state = await game.state.read()
  return state.ready && state.map.name === map
}

/**
 * Talks to the host in front of the player and reads the dialogue through (A only while a script runs).
 * Returns everything the dialogue showed.
 */
const talkAndReadThrough = async (game: GameSession, description: string): Promise<string> => {
  await game.player.interact()
  let seen = false
  let shown = ""
  for (let attempt = 0; attempt < 60; attempt++) {
    await game.wait.frames(30)
    const state = await game.state.read()
    if (state.scriptActive || state.dialogueOpen) {
      seen = true
      shown += `${state.dialogue.fullText}\n`
      await game.controls.press("a")
    } else if (seen && state.ready) return shown
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
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
    // Rows: the ship in port (the Aqua by day), BATTLE FRONTIER, CANCEL. Read through "May I see your ticket?"
    // until the menu is really open (the harness reports it), then move to the Frontier row.
    await game.player.interact()
    let menu = await game.state.read()
    for (let attempt = 0; attempt < 40 && menu.choice.kind !== "multichoice"; attempt++) {
      await game.wait.frames(30)
      // Re-read after the wait: an A on the open menu would pick the ship row.
      menu = await game.state.read()
      if (menu.choice.kind === "multichoice") break
      if (menu.dialogueOpen || menu.scriptActive) await game.controls.press("a")
      menu = await game.state.read()
    }
    expect(menu.choice).toMatchObject({ kind: "multichoice", optionCount: 3 })
    await game.controls.press("down")
    await game.wait.frames(20)
    expect((await game.state.read()).choice.cursor).toBe(1)
    await game.controls.press("a")
    // "BATTLE FRONTIER, is it?": wait for the real yes/no, answer YES (its first row), then let the boarding
    // walk and the ferry's departure run, reading only the "please board" message.
    let ask = await game.state.read()
    for (let attempt = 0; attempt < 40 && ask.choice.kind !== "yes-no"; attempt++) {
      await game.wait.frames(30)
      ask = await game.state.read()
    }
    expect(ask.choice.kind).toBe("yes-no")
    expect(ask.map.name).toBe("lilycove-city-harbor")
    await game.controls.press("a")
    // Read the boarding message through, but stop pressing the moment the Frontier map is reached: an A press
    // there would talk to the Frontier's ferry attendant and sail straight back.
    for (let attempt = 0; attempt < 120; attempt++) {
      await game.wait.frames(30)
      if ((await game.state.read()).map.name === "battle-frontier-outside-west") break
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

  it("meets a cabin host's request, sleeps at sea and wakes at the next port", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: { map: "olivine-port-inside", x: 8, y: 16 } },
      story: { vars: { ssAquaState: 8 } },
      bag: { items: { ssTicket: 1, fullHeal: 1 } },
      determinism: { textSpeed: "instant" },
    })
    // The Aqua's merchant (host row 4, in the south-south-east cabin) wants a FULL HEAL (his parameter 1);
    // the gambler is the voyage's other host.
    await game.ferry.pinHosts({ host: 4, param: 1 }, { host: 2 })
    await boardFerryAtDock(game)
    await waitAboard(game, "ss-aqua-1f")

    // The merchant stands at (1,6) of his cabin. The request is checked at once, and the FULL HEAL is taken.
    await game.player.warp("ss-aqua-room-sse", 1, 7, "up")
    await game.wait.frames(60)
    await expect(game.state.read()).resolves.toMatchObject({ ready: true, map: { name: "ss-aqua-room-sse" } })
    const request = await talkAndReadThrough(game, "The merchant's request was not read through")
    expect(request).toContain("FULL HEAL")
    await expect(game.inventory.contains("fullHeal")).resolves.toBe(false)

    // The cabin is the player's now, but the ship is in port: the bed (his offer) asks nothing and moves no time.
    const before = await game.clock.read()
    await talkAndReadThrough(game, "The port refusal was not read through")
    await expect(game.state.read()).resolves.toMatchObject({ ready: true, map: { name: "ss-aqua-room-sse" } })
    const inPort = await game.clock.read()
    expect(inPort.hour * 60 + inPort.minute - (before.hour * 60 + before.minute)).toBeLessThan(5)

    // Half an hour on, the ship casts off; the cast-off announcement plays and is read through.
    await game.clock.setTimeOfDay({ hour: before.hour, minute: before.minute + 31, second: 0 })
    let announced = false
    for (let attempt = 0; attempt < 60; attempt++) {
      await game.wait.frames(30)
      const state = await game.state.read()
      if (state.scriptActive || state.dialogueOpen) {
        announced = true
        await game.controls.press("a")
      } else if (announced && state.ready) break
    }
    expect(announced).toBe(true)

    // At sea the offer is real: "Sleep until we reach VERMILION CITY?" Wait for the real yes/no, answer YES.
    await game.player.interact()
    let ask = await game.state.read()
    for (let attempt = 0; attempt < 40 && ask.choice.kind !== "yes-no"; attempt++) {
      await game.wait.frames(30)
      ask = await game.state.read()
    }
    expect(ask.choice.kind).toBe("yes-no")
    await game.controls.press("a")

    // The clock jumps to the arrival (six in-game hours after cast-off, 14:30), the map reloads with the player
    // where they stood, and the arrival announcement plays. Read it through only once its text was seen.
    let arrived = false
    for (let attempt = 0; attempt < 80; attempt++) {
      await game.wait.frames(30)
      const state = await game.state.read()
      if (state.scriptActive || state.dialogueOpen) {
        if (state.dialogue.fullText.includes("Now arriving at VERMILION CITY")) arrived = true
        await game.controls.press("a")
      } else if (arrived && state.ready) break
    }
    expect(arrived).toBe(true)
    const woke = await game.clock.read()
    expect(woke.hour).toBe(14)
    expect(woke.minute).toBeGreaterThanOrEqual(30)
    expect(woke.minute).toBeLessThan(40)
    await expect(game.state.read()).resolves.toMatchObject({
      ready: true,
      map: { name: "ss-aqua-room-sse" },
      player: { x: 1, y: 7 },
    })

    // In port at Vermilion: the exit sailor lets the player ashore at its berth.
    await game.player.warp("ss-aqua-1f", 29, 3, "up")
    await game.wait.frames(60)
    await goAshoreFromExitSailor(game, "up", "vermilion-port-inside")
    await expect(game.state.read()).resolves.toMatchObject({ player: { x: 8, y: 9 } })
  })
})
