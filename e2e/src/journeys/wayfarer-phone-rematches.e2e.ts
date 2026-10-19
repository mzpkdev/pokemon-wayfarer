import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { decodeFieldMessageText } from "../harness/game-session/features/state"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

// Phone rematches (.product/specs/phone-rematches.md): the number offer after a win, a Battle call that names today's
// place, and a gift hand-off from a rotating slot. Rose (Route 118) and Dalton are Hoenn contacts appended past the
// Elite Four rows of the rematch table (contacts 90 and 109), so their registered flags are in 0x8BE-0x8D9.
const route118 = "route-118" as const
const route31 = "route-31" as const
const seed = 0x51ab1e5
const firstDay = 23_000
const roseContact = 90
const wadeSlotRow = 2 // Wade's row in sWorldTrainerSlots (src/data/trainer_slots/slots.h)

const gameOf = (game: GameSession): RunningSkyEmu =>
  (game as unknown as { running: RunningSkyEmu }).running

const arrangeAt = async (
  game: GameSession,
  map: typeof route118 | typeof route31,
  x: number,
  y: number,
  flags = {},
  vars = {},
  facing: "up" | "right" = "up",
) => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing, position: { map, x, y } },
    party: [{ species: "lapras", level: 100 }],
    story: { flags: { hasMatchCall: true, ...flags }, vars },
    determinism: { rngSeed: 1, textSpeed: "instant" },
  })
}

// Presses through a trainer's intro until the battle's action menu.
const toActionMenu = async (game: GameSession, interact: boolean): Promise<void> => {
  if (interact) await game.player.interact()
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (
      state.battle.ui === "text" ||
      (!state.battle.active && (state.dialogueOpen || state.scriptActive || state.controlsLocked))
    )
      await game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
    else await game.wait.frames(4)
  }
  throw new Error(`no action menu: ${JSON.stringify(await game.state.read())}`)
}

// Presses A through the dialogue until a yes/no prompt is on screen, and returns what was said.
const toPrompt = async (game: GameSession): Promise<string> => {
  const said: string[] = []
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.fullText && said.at(-1) !== state.dialogue.fullText)
      said.push(state.dialogue.fullText)
    if (state.dialogue.fullText.includes("POKéGEAR?")) {
      await game.wait.frames(20)
      return said.join("\n")
    }
    if (state.dialogueOpen || state.scriptActive || state.controlsLocked || state.battle.active)
      await game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
    else await game.wait.frames(8)
  }
  throw new Error(`no prompt: ${JSON.stringify(await game.state.read())}`)
}

const settle = async (game: GameSession): Promise<string[]> => {
  const seen: string[] = []
  for (let attempt = 0; attempt < 400; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.fullText && seen.at(-1) !== state.dialogue.fullText)
      seen.push(state.dialogue.fullText)
    if (state.ready && !state.dialogueOpen && !state.scriptActive && !state.battle.active)
      return seen
    if (state.battle.active || state.dialogueOpen || state.scriptActive || state.controlsLocked) {
      await game.wait.frames(20)
      await game.controls.press("a")
    } else await game.wait.frames(12)
  }
  throw new Error(`did not release the field: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer phone rematches", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("offers a number after a win (yes registers an appended Hoenn contact, no does not), then calls with a Battle text naming today's place", async () => {
    try {
      await game.dailySlots.pin({ day: firstDay, seed, noEmpty: true })
      await arrangeAt(game, route118, 7, 13)
      expect(await game.story.flag("phoneRoseRegistered")).toBe(false)

      // Rose: win, hear her out, and accept the offer.
      await toActionMenu(game, true)
      await game.battle.win()
      const said = await toPrompt(game)
      expect(said).toContain("Register")
      expect(await game.story.flag("phoneRoseRegistered")).toBe(false)
      await game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
      expect((await settle(game)).join("\n")).toContain("Registered")
      expect(await game.story.flag("phoneRoseRegistered")).toBe(true)
      // Registering her touched no other contact's flag.
      expect(await game.story.flag("phoneDaltonRegistered")).toBe(false)

      // Dalton: win and say no.
      await arrangeAt(game, route118, 16, 11, { phoneRoseRegistered: true }, {}, "right")
      await toActionMenu(game, true)
      await game.battle.win()
      await toPrompt(game)
      // The menu takes input a few frames after it draws, so press until the cursor is on No.
      for (let attempt = 0; (await game.state.read()).choice.cursor !== 1; attempt++) {
        if (attempt === 10) throw new Error("the cursor never reached No")
        await game.controls.press("down", { holdFrames: 2, releaseFrames: 6 })
      }
      await game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
      await settle(game)
      expect(await game.story.flag("phoneDaltonRegistered")).toBe(false)
      expect(await game.story.flag("phoneRoseRegistered")).toBe(true)

      // The next day Rose is a wanderer, ready as the day begins: force her next call (bit 8 also makes her ready).
      const symbols = await readSkyEmuSymbols(requireSymbolsPath())
      const client = gameOf(game).client
      await game.dailySlots.pin({ day: firstDay + 1, seed, noEmpty: true })
      await game.player.warp("littleroot-town", 10, 12, "up")
      await game.wait.forReady()
      await client.writeBytes(
        symbols.address("gMatchCallDebugContact"),
        new Uint8Array([roseContact + 1, 1]),
      )
      await game.player.move("up")
      let text = ""
      for (let attempt = 0; attempt < 200 && !text.includes("rematch"); attempt++) {
        await game.wait.frames(10)
        // The call window waits for a button before it prints.
        if (attempt % 5 === 4) await game.controls.press("a", { holdFrames: 1, releaseFrames: 1 })
        text = decodeFieldMessageText(
          Array.from(await client.readBytes(symbols.address("gStringVar4"), 320)),
        )
      }
      expect(text).toContain("please give me a rematch")
      // The Battle text ends "be around {STR_VAR_2}.": today's place is filled in, whichever slot she stands in.
      const place = /be around ([A-Z0-9 .'-]+)\./.exec(text.replace(/\s+/g, " "))
      expect(place?.[1]?.length).toBeGreaterThan(0)
      expect(place?.[1]?.length).toBeLessThanOrEqual(16)
      expect(text).not.toContain("{")
      await settle(game) // hang up
    } finally {
      // The pinned day lives in EWRAM and stamps the save; the next test starts from its own arrangement and pin.
      await game.dailySlots.unpin()
    }
  })

  // Wade is beaten and holds a gift as the day starts, so he is placed and his own script hands it over before any
  // battle; with a full Bag he keeps it (NoRoom). Route 31 has a single slot, so he stands in his own: the hand-off from a
  // slot on another map is covered by the daily trainers mechanics tests, not here.
  const talkToWade = async (): Promise<string> => {
    await game.player.interact()
    return (await settle(game)).join("\n")
  }
  const startGiftDay = async (
    day: number,
    bag: { fullPockets?: "items"[] } = {},
  ): Promise<void> => {
    await game.dailySlots.pin({ day, seed, noEmpty: true })
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: route31, x: 23, y: 18 } },
      party: [{ species: "lapras", level: 100 }],
      bag,
      story: {
        flags: { hasMatchCall: true, wadeDefeated: true, wadeHasItem: true },
        vars: { wadeItem: 120 },
      },
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })
    // The next day's first map load takes the gift snapshot.
    await game.dailySlots.pin({ day: day + 1, seed, noEmpty: true })
    await game.player.warp(route31, 23, 18, "up")
    await game.wait.forReady()
    const occupants = await game.dailySlots.trainerOccupants()
    expect(occupants.map((entry) => entry.occupant)).toContain(wadeSlotRow)
  }

  it("hands a gift over from his daily slot before any battle, and keeps it when the Bag is full", async () => {
    await startGiftDay(firstDay + 100, { fullPockets: ["items"] })
    const noRoom = await talkToWade()
    expect(noRoom).toContain("BAG")
    expect(await game.story.flag("wadeHasItem")).toBe(true)
    expect(await game.inventory.count("escapeRope")).toBe(0)

    await startGiftDay(firstDay + 200)
    expect(await game.inventory.count("escapeRope")).toBe(0)
    const given = await talkToWade()
    expect(given).toContain("found something")
    expect(await game.story.flag("wadeHasItem")).toBe(false)
    expect(await game.inventory.count("escapeRope")).toBe(1)
  })
})
