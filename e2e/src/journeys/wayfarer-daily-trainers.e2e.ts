import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

// Daily world slots, trainer half. Route 8's two Super Nerds, Sam and Tom, are the only members of
// the Kanto Super Nerd group on the map: Sam's slot is a south-facing lane, Tom's a corridor. Once
// both are beaten, each day shuffles them between their two slots.
const route8 = "route-8" as const
const samTrainerId = 247 // TRAINER_SAM_HNS (include/constants/opponents.h)
const tomTrainerId = 248 // TRAINER_TOM_HNS
const samIntro = "How does the MAGNET TRAIN work?"
const tomIntro = "got many GYM BADGES"
const tomPostBattle = "GYM BADGES give you advantages"
const seed = 0x51ab1e5
// One tile east of Sam's lane (x = 46, looking south from y = 13); a step west puts the player in his sight.
const lane = { x: 47, y: 16 } as const
const firstDay = 22_000

const arrangeAtLane = async (game: GameSession): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "left", position: { map: route8, ...lane } },
    party: [{ species: "lapras", level: 100 }],
    determinism: { rngSeed: 1, textSpeed: "instant" },
  })
}

const settle = async (game: GameSession, description: string): Promise<string[]> => {
  const seen: string[] = []
  for (let attempt = 0; attempt < 400; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text && seen.at(-1) !== state.dialogue.text) seen.push(state.dialogue.text)
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.battle.active) return seen
    if (state.battle.active || state.dialogueOpen || state.scriptActive || state.controlsLocked) {
      await game.wait.frames(20)
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(`${description} did not release the field: ${JSON.stringify(await game.state.read())}`)
}

// Presses through the trainer's approach and intro until the battle's action menu; returns the
// dialogue it showed on the way.
const toActionMenu = async (game: GameSession, description: string, interact: boolean): Promise<string> => {
  if (interact) await game.player.interact()
  const said: string[] = []
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text && said.at(-1) !== state.dialogue.text) said.push(state.dialogue.text)
    if (state.battle.ui === "action-menu") return said.join("\n")
    if (
      state.battle.ui === "text" ||
      (!state.battle.active && (state.dialogueOpen || state.scriptActive || state.controlsLocked))
    )
      await game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
    else await game.wait.frames(4)
  }
  throw new Error(`${description} did not reach its action menu: ${JSON.stringify(await game.state.read())}`)
}

const opponentId = async (game: GameSession): Promise<number> => {
  const symbols = await readSkyEmuSymbols(requireSymbolsPath())
  const client = (game as unknown as { running: RunningSkyEmu }).running.client
  const parameter = await client.readBytes(symbols.address("gTrainerBattleParameter") + 2, 2)
  return new DataView(parameter.buffer, parameter.byteOffset, 2).getUint16(0, true)
}

const stepIntoLane = (game: GameSession) => game.player.move("left")

// Pins `day`, loads Route 8 and reports whether Sam's slot holds Tom today (a swapped day).
const isSwapped = async (game: GameSession, day: number): Promise<boolean> => {
  await game.dailySlots.pin({ day, seed, noEmpty: true })
  await game.player.warp(route8, lane.x, lane.y, "left")
  await game.wait.forReady()
  const entries = (await game.dailySlots.trainerOccupants()).sort((a, b) => a.slot - b.slot)
  // Two defeated trainers, two rotating slots (Sam's first, Tom's second): swapped when each holds the other.
  return entries.length === 2 && entries[0]!.occupant === entries[1]!.slot && entries[1]!.occupant === entries[0]!.slot
}

const findSwappedDay = async (game: GameSession, from: number): Promise<number> => {
  for (let day = from; day < from + 40; day++) if (await isSwapped(game, day)) return day
  throw new Error(`Sam's and Tom's slots were not swapped on 40 consecutive days from ${from}`)
}

describe.sequential("Wayfarer daily trainer slots", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("lets a beaten trainer wander: a wanderer approaches by sight with its own team and speaks after the win", async () => {
    await game.dailySlots.pin({ day: firstDay, seed, noEmpty: true })
    await arrangeAtLane(game)

    // Day one: both home trainers fight as authored.
    await stepIntoLane(game)
    expect(await toActionMenu(game, "Sam", false)).toContain(samIntro)
    expect(await opponentId(game)).toBe(samTrainerId)
    await game.battle.win()
    await settle(game, "Sam's win")
    await game.player.warp(route8, 53, 24, "right")
    await game.wait.forReady()
    expect(await toActionMenu(game, "Tom", true)).toContain(tomIntro)
    expect(await opponentId(game)).toBe(tomTrainerId)
    await game.battle.win()
    await settle(game, "Tom's win")
    // Their home slots keep their own trainers for the rest of the day.
    expect(await game.dailySlots.trainerOccupants()).toEqual([])

    // The next days shuffle them: find one where Tom stands in Sam's slot.
    const swapped = await findSwappedDay(game, firstDay + 1)
    await stepIntoLane(game)
    const intro = await toActionMenu(game, "wandering Tom", false)
    // Tom's own intro and Tom's own party (his trainer id), not Sam's authored battle.
    expect(intro).toContain(tomIntro)
    expect(intro).not.toContain(samIntro)
    expect(await opponentId(game)).toBe(tomTrainerId)
    await game.battle.win()
    await settle(game, "wandering Tom's win")
    // He stays and speaks his post-battle line; there is no second battle today.
    await game.player.interact()
    expect((await settle(game, "Tom's post-battle line")).join("\n")).toContain(tomPostBattle)

    // Sight is done for the day: leave the lane and step back in, and nobody approaches.
    await game.player.move("right")
    await stepIntoLane(game)
    await game.wait.frames(120)
    await expect(game.state.read()).resolves.toMatchObject({ battle: { active: false }, dialogueOpen: false, scriptActive: false })

    // Tomorrow the slot fights again, with whoever the day brings.
    await game.dailySlots.pin({ day: swapped + 1, seed, noEmpty: true })
    await game.player.warp(route8, lane.x, lane.y, "left")
    await game.wait.forReady()
    await stepIntoLane(game)
    await toActionMenu(game, "the next day", false)
    expect([samTrainerId, tomTrainerId]).toContain(await opponentId(game))
    await game.battle.win()
    await settle(game, "the next day's win")
  })

  it("rebuilds a rotating occupant on Continue: the wanderer fights, not the slot's authored trainer", async () => {
    const day = await findSwappedDay(game, firstDay + 100)
    const before = await game.dailySlots.trainerOccupants()
    await game.saveAndReload()
    // Continue applies no day change: the same day, the same occupants.
    expect(await game.dailySlots.trainerOccupants()).toEqual(before)
    await stepIntoLane(game)
    expect(await toActionMenu(game, "Tom after Continue", false)).toContain(tomIntro)
    expect(await opponentId(game)).toBe(tomTrainerId)
    expect(day).toBeGreaterThan(firstDay)
    await game.battle.win()
    await settle(game, "Tom's win after Continue")
  })
})
