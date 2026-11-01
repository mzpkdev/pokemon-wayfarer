import { mkdir, writeFile } from "node:fs/promises"
import * as path from "node:path"

import { beforeAll, describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { decodeFieldMessageText } from "../harness/game-session/features/state"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"

// Notable trainers as phone contacts (.product/specs/notable-trainers.md#phone-contacts). Nobody reaches Friend until
// haunts ship, so the journey sets Brock's friendship score directly. The field then owes the number handover's line
// and plays it; Brock is in the Pokégear's contact list, and calling him gives his phone greeting, where he is (home-
// locked in his Gym before the Boulder Badge), and his farewell.
const brock = 1 // NOTABLE_TRAINER_BROCK
const friend = 20 // NOTABLE_FRIENDSHIP_FRIEND
const menuActionPokenav = 3 // MENU_ACTION_POKENAV (src/start_menu.c)
const brockHello = "BROCK here"
const brockWhere = "GYM in\nPEWTER CITY"
const brockBye = "Talk soon!"

const evidenceDirectory = process.env.E2E_EVIDENCE_DIR

const gameOf = (game: GameSession): RunningSkyEmu =>
  (game as unknown as { running: RunningSkyEmu }).running

const capture = async (game: GameSession, name: string): Promise<void> => {
  if (!evidenceDirectory) return
  await mkdir(evidenceDirectory, { recursive: true })
  await writeFile(path.join(evidenceDirectory, `${name}.png`), await game.screenshot())
}

// Presses through whatever the script shows until the field is free again; returns each distinct text seen.
const settle = async (game: GameSession, description: string): Promise<string[]> => {
  const seen: string[] = []
  for (let attempt = 0; attempt < 600; attempt++) {
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
  throw new Error(
    `${description} did not release the field: ${JSON.stringify(await game.state.read())}`,
  )
}

describe.sequential("Wayfarer notable phone contacts", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("hands Brock's number over at Friend, lists him, and his call says hello, where he is, and goodbye", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: { map: "olivine-city", x: 13, y: 22 } },
      party: [{ species: "lapras", level: 100 }],
      story: {
        flags: {
          hasMatchCall: true,
          pokegearMatchCall: true,
          sharedPokegear: true,
          disableTrainers: true,
        },
      },
      determinism: { rngSeed: 1, textSpeed: "instant" },
    })
    const symbols = await readSkyEmuSymbols(requireSymbolsPath())
    const client = gameOf(game).client
    const saveBlock3 = new DataView(
      (await client.readBytes(symbols.address("gSaveBlock3Ptr"), 4)).buffer,
    ).getUint32(0, true)
    const offset = async (name: string): Promise<number> =>
      new DataView((await client.readBytes(symbols.address(name), 2)).buffer).getUint16(0, true)
    const scoreAddress = saveBlock3 + (await offset("gE2ENotableFriendshipScoreOffset")) + brock - 1
    const shownAddress = saveBlock3 + (await offset("gE2ENotableNumberShownOffset"))

    // Friend, set directly: the field owes the handover's line and plays it once it has control.
    await client.writeBytes(scoreAddress, new Uint8Array([friend]))
    await game.wait.frames(30)
    const handover = await settle(game, "the number handover")
    expect(handover.join("\n")).toMatch(/Registered BROCK.s number/)
    expect((await client.readBytes(shownAddress, 1))[0]! & 1).toBe(1)
    // Shown once: the field owes nothing more.
    await game.wait.frames(60)
    expect((await game.state.read()).dialogueOpen).toBe(false)

    // Start menu -> POKéGEAR.
    await game.controls.press("start")
    await game.wait.frames(60)
    const actions = Array.from(
      await client.readBytes(symbols.address("sCurrentStartMenuActions"), 9),
    )
    const cursor = (await client.readBytes(symbols.address("sStartMenuCursorPos"), 1))[0]!
    const pokegear = actions.indexOf(menuActionPokenav)
    expect(pokegear).toBeGreaterThanOrEqual(0)
    for (let step = cursor; step < pokegear; step++) await game.controls.press("down")
    for (let step = cursor; step > pokegear; step--) await game.controls.press("up")
    await game.controls.press("a")
    await game.wait.frames(240)
    await capture(game, "phone-1-pokegear")
    // MAP, then MATCH CALL.
    await game.controls.press("down")
    await game.wait.frames(20)
    await game.controls.press("a")
    await game.wait.frames(240)
    await capture(game, "phone-2-contacts")
    // The special headers come first (MOM at this checkpoint), then the notable contacts: Brock is second, and
    // the route contacts would follow. A opens CALL / CANCEL, and CALL is first.
    await game.controls.press("down")
    await game.wait.frames(30)
    await capture(game, "phone-2b-brock")
    await game.controls.press("a")
    await game.wait.frames(30)
    await game.controls.press("a")
    // The phone dials ("......", waiting for A), then buffers the call into gStringVar4 and prints it.
    let call = ""
    let box = 0
    for (let attempt = 0; attempt < 40 && box < 4; attempt++) {
      await game.wait.frames(30)
      call = decodeFieldMessageText(
        Array.from(await client.readBytes(symbols.address("gStringVar4"), 320)),
      )
      if (call.includes(brockBye)) await capture(game, `phone-3-call-box-${++box}`)
      await game.controls.press("a")
    }
    expect(call).toContain(brockHello)
    expect(call).toContain(brockWhere)
    expect(call).toContain(brockBye)
  })
})
