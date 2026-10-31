import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

// The in-game clock (.product/specs/in-game-clock.md): a new game starts on day 1 at 10:00, the
// clock runs 16 in-game seconds per second of play, the daily update starts the next day while the
// player stands on a map, and the clock survives a save and Continue.
const route31 = "route-31" as const
const secondOfPlay = 60 // frames
const clockSecondsPerSecond = 16

const settleField = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 200; attempt++) {
    const state = await game.state.read()
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.controlsLocked) return
    await game.wait.frames(10)
  }
  throw new Error(`the field never settled: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer in-game clock", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: route31, x: 23, y: 18 } },
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })
    await settleField(game)
  })

  it("starts a new game on day 1 in the morning", async () => {
    const clock = await game.clock.read()
    expect({ year: clock.year, month: clock.month, day: clock.day }).toEqual({
      year: 0,
      month: 1,
      day: 1,
    })
    // Arranging the game took some play: at most a few in-game minutes past 10:00.
    expect(clock.hour).toBe(10)
    expect(clock.minute).toBeLessThan(30)
    expect(await game.story.var("days")).toBe(1)
  })

  it("moves 16 in-game seconds for every second of play", async () => {
    const before = await game.clock.read()
    await game.wait.frames(10 * secondOfPlay)
    const after = await game.clock.read()
    // Ten seconds of play are ten ticks; the frame the reading lands on can shift one tick.
    const moved = after.totalSeconds - before.totalSeconds
    expect(moved).toBeGreaterThanOrEqual(9 * clockSecondsPerSecond)
    expect(moved).toBeLessThanOrEqual(11 * clockSecondsPerSecond)
  })

  it("starts the next day on the field when the clock passes midnight, without a map load", async () => {
    const map = (await game.state.read()).map.name
    await game.clock.setTimeOfDay({ hour: 23, minute: 59, second: 0 })
    // A minute of in-game time is under four seconds of play; the daily update checks every 512 frames.
    let days = await game.story.var("days")
    for (let waited = 0; waited < 20 * secondOfPlay && days !== 2; waited += 30) {
      await game.wait.frames(30)
      days = await game.story.var("days")
    }
    expect(days).toBe(2)
    const clock = await game.clock.read()
    expect({ month: clock.month, day: clock.day, hour: clock.hour }).toEqual({
      month: 1,
      day: 2,
      hour: 0,
    })
    expect((await game.state.read()).map.name).toBe(map)
  })

  it("keeps the clock across a save and Continue, and keeps running", async () => {
    const saved = await game.clock.read()
    await game.saveAndReload()
    const loaded = await game.clock.read()
    expect(loaded.totalSeconds).toBeGreaterThanOrEqual(saved.totalSeconds)
    // The reload itself is a few seconds of play at most, never a jump.
    expect(loaded.totalSeconds - saved.totalSeconds).toBeLessThan(30 * clockSecondsPerSecond)
    expect(await game.story.var("days")).toBe(2)
    await game.wait.frames(2 * secondOfPlay)
    expect((await game.clock.read()).totalSeconds).toBeGreaterThan(loaded.totalSeconds)
  })
})
