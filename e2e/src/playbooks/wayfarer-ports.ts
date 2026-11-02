import { type GameMap, type GameSession } from "../harness/game-session"

/**
 * The S.S. ferries (.product/specs/ss-ferries.md). A ship in port is boarded from its gangway
 * sailor with the S.S. Ticket, casts off 30 in-game minutes later and spends 6 in-game hours at
 * sea per leg. These helpers move the in-game clock rather than playing the time out, and they
 * never cross midnight: `clock.setTimeOfDay` keeps the date, so a journey starts early in the day
 * (a new game starts at 8:00, which leaves room for two legs).
 */

/** Presses A with a pause until `done` is true, to ride out announcements and dialogue. */
const advanceUntil = async (
  game: GameSession,
  done: () => Promise<boolean>,
  description: string,
  attempts = 80,
): Promise<void> => {
  for (let attempt = 0; attempt < attempts; attempt++) {
    if (await done()) return
    await game.wait.frames(30)
    // Look again before pressing: arriving on a ship map faces the player at its exit sailor, and an A press
    // after arrival would talk to him and go straight back ashore.
    if (await done()) return
    await game.controls.press("a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

/**
 * Talks to the Vermilion sailor and picks the first row of his menu, which is the ship in port
 * (S.S. Aqua by day, S.S. Anne by night). The row is missing when the berth is empty: the first
 * row is then SEVII ISLANDS, so call this only once the ship is in service.
 */
export const beginWayfarerRegularAquaDeparture = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.wait.frames(30)
  await game.controls.press("a")
}

/** Boards at a dock whose sailor has no menu (Olivine, Slateport): talk, and the walk aboard follows. */
export const boardFerryAtDock = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.wait.frames(30)
}

/** Runs the boarding dialogue and the walk until the player stands on a ship map. */
export const waitAboard = async (game: GameSession, ship: GameMap): Promise<void> => {
  await advanceUntil(
    game,
    async () => {
      const state = await game.state.read()
      return state.map.name === ship && state.ready
    },
    `The ferry did not reach ${ship}`,
  )
}

const addMinutes = async (
  game: GameSession,
  minutes: number,
): Promise<{ hour: number; minute: number; second: number }> => {
  const now = await game.clock.read()
  const total = now.hour * 60 + now.minute + minutes
  if (total >= 24 * 60)
    throw new Error(
      `The ferry journey would cross midnight (${now.hour}:${now.minute} + ${minutes} min); start earlier in the day`,
    )
  return { hour: Math.floor(total / 60), minute: total % 60, second: 0 }
}

/**
 * Waits for the PA script a changed clock starts (it plays once the field is free) and reads it through.
 * Returning before the script has started would let the next step press its buttons into the message.
 */
const readAnnouncement = async (game: GameSession, description: string): Promise<void> => {
  let seen = false
  for (let attempt = 0; attempt < 60; attempt++) {
    await game.wait.frames(30)
    const state = await game.state.read()
    if (state.scriptActive || state.dialogueOpen || !state.ready) {
      seen = true
      await game.controls.press("a")
    } else if (seen) return
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

/**
 * Rides one leg: the ship casts off half an in-game hour after it was boarded or arrived, and
 * arrives six hours later. Each announcement plays once the field is free and is read through.
 */
export const sailFerryLeg = async (game: GameSession): Promise<void> => {
  await game.clock.setTimeOfDay(await addMinutes(game, 31))
  await readAnnouncement(game, "The cast-off was not announced")
  await game.clock.setTimeOfDay(await addMinutes(game, 361))
  await readAnnouncement(game, "The arrival was not announced")
}

/**
 * Goes ashore from the Aqua or the Tidal: the exit sailor stands next to the entry tile (the
 * Aqua's above it, the Tidal's below). Each attempt turns the player toward him while the field is
 * free (a turn into a blocked tile is only a turn), talks, and reads the dialogue through.
 */
export const goAshoreFromExitSailor = async (
  game: GameSession,
  facing: "up" | "down",
  destination: GameMap,
): Promise<void> => {
  for (let attempt = 0; attempt < 60; attempt++) {
    const state = await game.state.read()
    if (state.map.name === destination && state.ready) return
    if (state.ready && !state.scriptActive && !state.dialogueOpen) {
      await game.controls.press(facing)
      await game.wait.frames(12)
      await game.player.interact()
    } else await game.controls.press("a")
    await game.wait.frames(30)
  }
  throw new Error(
    `Going ashore did not reach ${destination}: ${JSON.stringify(await game.state.read())}`,
  )
}

/** Boards the S.S. Aqua at Vermilion and rides one leg to the next port (Slateport), then goes ashore. */
export const rideAquaFromVermilionToSlateport = async (game: GameSession): Promise<void> => {
  await beginWayfarerRegularAquaDeparture(game)
  await waitAboard(game, "ss-aqua-1f")
  await sailFerryLeg(game)
  await goAshoreFromExitSailor(game, "up", "slateport-city-harbor")
}
