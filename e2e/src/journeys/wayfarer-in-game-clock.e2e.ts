import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"

// The in-game clock (.product/specs/in-game-clock.md): a new game starts on day 1 at 8:00, the
// clock runs 12 in-game seconds per second of play, the daily update starts the next day while the
// player stands on a map, and the clock survives a save and Continue.
const route31 = "route-31" as const
const secondOfPlay = 60 // frames
const clockSecondsPerSecond = 12

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

  it("starts a new game on day 1 at 8:00", async () => {
    const clock = await game.clock.read()
    expect({ year: clock.year, month: clock.month, day: clock.day }).toEqual({
      year: 0,
      month: 1,
      day: 1,
    })
    // Arranging the game took some play: at most a few in-game minutes past 8:00.
    expect(clock.hour).toBe(8)
    expect(clock.minute).toBeLessThan(30)
    expect(clock.dayOfWeek).toBe(6) // WEEKDAY_SAT
    expect(clock.dayNumber).toBe(1)
    expect(clock.localDay).toBe(1)
    expect(await game.story.var("days")).toBe(1)
  })

  it("moves 12 in-game seconds for every second of play", async () => {
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
    // A minute of in-game time is five seconds of play; the daily update checks every 512 frames.
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

  it("starts a day the save crossed into as soon as Continue loads the map", async () => {
    // A second of play after 23:59:59 the clock is past midnight; the save may or may not have
    // run the daily update yet, but Continue runs it on load, before the field takes a step.
    await game.clock.setTimeOfDay({ hour: 23, minute: 59, second: 59 })
    await game.wait.frames(secondOfPlay + 2)
    const saved = await game.clock.read()
    expect({ day: saved.day, hour: saved.hour }).toEqual({ day: 3, hour: 0 })
    // The field check runs every 512 frames, so it may already have started the day. When it
    // hasn't (most runs), the day below can only come from Continue's own daily update.
    const daysBeforeSave = await game.story.var("days")
    expect([2, 3]).toContain(daysBeforeSave)
    await game.saveAndReload()
    expect(await game.story.var("days")).toBe(3)
    expect((await game.clock.read()).day).toBe(3)
  })
})
