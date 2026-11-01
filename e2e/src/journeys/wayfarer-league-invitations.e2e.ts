import { afterEach, beforeEach, describe, expect, it } from "webanvil/test"

import { GameSession, type GameMap } from "../harness/game-session"

// League invitations by phone (.product/specs/leagues.md#invitations). A player who reaches TR 80
// starts a seven-day countdown on the in-game clock; the league that knows them then calls, and the
// player accepts or declines on the call. Indigo's and Hoenn's lobbies admit only an accepted event,
// naming the reigning champion; each event's end owes the league's result call.

const countingDown = 1 // LEAGUE_INVITATION_COUNTING_DOWN
const accepted = 3 // LEAGUE_INVITATION_ACCEPTED

// Presses through what the script shows until the field is free, recording each message just before
// the press that dismisses it.
const settle = async (game: GameSession, description: string): Promise<string[]> => {
  const seen: string[] = []
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.battle.active)
      return seen
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked) {
      await game.wait.frames(20)
      const text = (await game.state.read()).dialogue.fullText
      if (text && seen.at(-1) !== text) seen.push(text)
      await game.controls.press("a")
    } else await game.wait.frames(10)
  }
  throw new Error(
    `${description} did not release the field: ${JSON.stringify(await game.state.read())}`,
  )
}

// Steps onto a lobby trigger and returns what its script says.
const stepAndSettle = async (game: GameSession, description: string): Promise<string> => {
  await game.player.move("up")
  await game.wait.until(
    (state) => state.dialogueOpen || state.scriptActive,
    `${description} starts`,
    240,
  )
  return (await settle(game, description)).join("\n")
}

const walkTo = async (game: GameSession, x: number, y: number): Promise<void> => {
  for (let attempt = 0; attempt < 120; attempt++) {
    const state = await game.state.read()
    if (state.player.x === x && state.player.y === y) {
      await game.wait.frames(16)
      return
    }
    await game.player.move(
      state.player.x < x
        ? "right"
        : state.player.x > x
          ? "left"
          : state.player.y < y
            ? "down"
            : "up",
    )
  }
  throw new Error(`Could not walk to ${x},${y}: ${JSON.stringify(await game.state.read())}`)
}

const walkNorthToMap = async (game: GameSession, map: GameMap): Promise<void> => {
  for (let attempt = 0; attempt < 60; attempt++) {
    const state = await game.state.read()
    if (state.map.name === map) {
      await game.wait.until(
        (current) => current.map.name === map && (current.ready || current.scriptActive),
        `ready map ${map}`,
      )
      await settle(game, `enter ${map}`)
      return
    }
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked) await settle(game, map)
    else await game.player.move("up")
  }
  throw new Error(`Did not reach ${map}: ${JSON.stringify(await game.state.read())}`)
}

// The field's daily update starts a new day within 512 frames of play outdoors.
const waitForDaysRemaining = (game: GameSession, days: number) =>
  game.wait.until(
    (state) => state.circuit.event.daysRemaining === days,
    `${days} days left on the invitation countdown`,
    2_400,
  )

describe.sequential("Wayfarer league invitations by phone", () => {
  let game: GameSession

  beforeEach(async () => {
    game = await GameSession.launch()
  })

  afterEach(async () => {
    await game.close()
  })

  it("calls seven days after TR 80; accepting saves the lineup, admits the player, and a loss owes the result call", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "indigo-plateau", x: 11, y: 7 } },
      party: [{ species: "pidgey", level: 1, moves: ["tackle"] }],
      story: { flags: { hasMatchCall: true, pokegearMatchCall: true, sharedPokegear: true } },
      circuit: { badges: { kanto: 8 } },
      determinism: { textSpeed: "instant" },
    })
    // Qualifying at TR 80 starts the countdown at seven days, from today.
    await game.wait.until(
      (state) =>
        state.circuit.event.invitationState === countingDown &&
        state.circuit.event.daysRemaining === 7,
      "the countdown starts",
    )
    expect((await game.state.read()).circuit).toMatchObject({
      trainerRating: 80,
      leagues: { indigo: "available", hoenn: "locked" },
      event: { id: 0, acceptedLeague: null, invitedLeague: null, resultCall: null },
    })

    // Without an invitation, the lobby names its (absent) champion and turns the player away.
    await game.player.warp("indigo-league-lobby", 32, 4, "up")
    await game.wait.forMap("indigo-league-lobby")
    const refusal = await stepAndSettle(game, "Indigo refusal")
    expect(refusal).toContain("no reigning CHAMPION yet")
    expect(refusal).toContain("Only invited TRAINERS may enter")
    expect(refusal).toContain("come in 7 days")
    expect((await game.state.read()).circuit.run.active).toBe(false)

    // Days pass outdoors (the lobby is a Pokémon Center, where the daily update waits).
    await game.player.warp("indigo-plateau", 11, 7, "up")
    await game.wait.forMap("indigo-plateau")
    await game.clock.advanceDays(6)
    await waitForDaysRemaining(game, 1)
    expect((await game.state.read()).circuit.event.invitedLeague).toBeNull()
    await game.clock.advanceDays(1)
    await game.wait.until(
      (state) => state.circuit.event.invitedLeague === "indigo",
      "the Indigo League calls",
      2_400,
    )
    const call = (await game.leagues.answerCall("accept")).join("\n")
    expect(call).toContain("desk at INDIGO PLATEAU")
    expect(call).toContain("The INDIGO LEAGUE is holding its")
    expect(call).toContain("Accept the INDIGO LEAGUE’s")
    expect(call).toContain("in\nbattle order")
    expect(call).toContain("and last, ")
    const event = (await game.state.read()).circuit.event
    expect(event).toMatchObject({
      acceptedLeague: "indigo",
      invitationState: accepted,
      invitedLeague: null,
      daysRemaining: 0,
      worldProgress: 80,
    })
    expect(new Set(event.lineup.map((member) => member.characterId)).size).toBe(5)

    // The answer saved the accepted event itself: reset and continue from that save, with no harness
    // save first. It then waits across days, and no call rings again.
    await game.reloadSavedGame()
    await game.clock.advanceDays(10)
    await game.wait.frames(600)
    expect((await game.leagues.answerCall()).length).toBe(0)
    expect((await game.state.read()).circuit.event).toMatchObject({
      id: event.id,
      acceptedLeague: "indigo",
      lineup: event.lineup,
    })

    await game.player.warp("indigo-league-lobby", 32, 4, "up")
    await game.wait.forMap("indigo-league-lobby")
    const welcome = await stepAndSettle(game, "Indigo admission")
    expect(welcome).toContain("Your event is waiting")
    await walkNorthToMap(game, "indigo-lorelei")
    expect((await game.state.read()).circuit.run).toMatchObject({ active: true, stage: "indigo" })

    await walkTo(game, 6, 6)
    await game.player.interact()
    for (let attempt = 0; attempt < 360; attempt++) {
      const state = await game.state.read()
      if (state.battle.ui === "action-menu") break
      if (state.battle.active) {
        if (state.battle.ui === "text") await game.controls.press("a")
        else await game.wait.frames(4)
      } else if (state.dialogueOpen || state.scriptActive || state.controlsLocked || !state.ready)
        await game.controls.press("a")
      else await game.wait.frames(10)
    }
    expect((await game.state.read()).battle.ui).toBe("action-menu")
    await game.battle.lose()
    // The blackout returns the player to the lobby; the owed result call rings once they stand there.
    const afterLoss: string[] = []
    for (let attempt = 0; attempt < 6; attempt++) {
      afterLoss.push(...(await game.leagues.answerCall()))
      const state = await game.state.read()
      if (state.map.name === "indigo-league-lobby" && state.circuit.event.resultCall === null) break
    }
    expect((await game.state.read()).circuit).toMatchObject({
      run: { active: false },
      clears: { indigo: false },
      event: {
        id: 0,
        acceptedLeague: null,
        invitationState: countingDown,
        daysRemaining: 7,
        resultCall: null,
      },
    })
    const result = afterLoss.join("\n")
    expect(result).toContain("thank you for taking")
    const champion = /\n?([A-Z][A-Z. &é'-]+) won it, and reigns/.exec(result)?.[1]
    expect(champion, result).toBeTruthy()
    expect((await game.state.read()).circuit.event.resultCall).toBeNull()

    // The lobby now names that champion.
    await game.player.warp("indigo-league-lobby", 32, 4, "up")
    await game.wait.forMap("indigo-league-lobby")
    const lobby = await stepAndSettle(game, "Indigo lobby after the loss")
    expect(lobby).toContain("reigning CHAMPION of")
    expect(lobby).toContain(`INDIGO LEAGUE is ${champion}.`)
    expect(lobby).toContain("come in 7 days")
  }, 600_000)

  it("declining runs the event without the player, crowns its strongest, and restarts the countdown", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "hoenn-league-lobby", x: 9, y: 3 } },
      party: [{ species: "lapras", level: 100 }],
      circuit: { badges: { hoenn: 8 } },
      determinism: { textSpeed: "instant" },
    })
    await game.wait.until(
      (state) => state.circuit.event.invitationState === countingDown,
      "the countdown starts",
    )
    // A call placed while the player has no phone waits, survives a reload, and rings once they do.
    await game.leagues.forceInvitation("hoenn")
    await game.story.setFlag("hasMatchCall", false)
    await game.wait.until(
      (state) => state.circuit.event.invitedLeague === "hoenn",
      "the Hoenn League's call is placed",
    )
    await game.saveAndReload()
    expect((await game.state.read()).circuit.event.invitedLeague).toBe("hoenn")
    await game.story.setFlag("hasMatchCall", true)
    const call = (await game.leagues.answerCall("decline")).join("\n")
    expect(call).toContain("desk at EVER GRANDE CITY")
    expect(call).toContain("Accept the HOENN LEAGUE’s")
    expect(call).toContain("Decline? The event will go ahead")
    expect(call).toContain("go\nahead without you")
    const champion = /it’s over! ([A-Z][A-Z. &é'-]+) won,/.exec(call)?.[1]
    expect(champion, call).toBeTruthy()
    expect((await game.state.read()).circuit.event).toMatchObject({
      id: 0,
      acceptedLeague: null,
      invitedLeague: null,
      invitationState: countingDown,
      daysRemaining: 7,
      resultCall: null,
    })

    // The guards name the new champion and turn the player away; the decline survived a reload.
    await game.saveAndReload()
    await game.player.interact()
    const refusal = (await settle(game, "Hoenn guards' refusal")).join("\n")
    expect(refusal).toContain(`HOENN LEAGUE is ${champion}.`)
    expect(refusal).toContain("Only invited TRAINERS may enter")
    expect((await game.state.read()).circuit.run.active).toBe(false)
  }, 300_000)
})
