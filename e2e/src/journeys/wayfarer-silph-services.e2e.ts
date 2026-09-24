import { beforeEach, describe, expect, it } from "webanvil/test"
import { GameSession } from "../harness/game-session"
import { type RunningSkyEmu } from "../harness/skyemu/server"
import { readSkyEmuSymbols } from "../harness/skyemu/symbols"
import { requireSymbolsPath } from "../harness/skyemu/utils"
import { arrangeSilph, settleSilph, talkSilph } from "../playbooks/silph"

const enterTutorChooser = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  for (let attempt = 0; attempt < 180; attempt++) {
    if ((await game.state.read()).partyMenu.open) return
    await game.controls.press("a")
    await game.wait.frames(20)
  }
  throw new Error(`Thunder Wave tutor chooser did not open: ${JSON.stringify(await game.state.read())}`)
}

const receiveStevenStarter = async (game: GameSession): Promise<void> => {
  await game.player.interact()
  await game.dialogue.waitForOpen()
  // Accept Steven's first choice, then decline the ordinary nickname prompt.
  for (let attempt = 0; attempt < 360; attempt++) {
    if ((await game.state.read()).party.some((mon) => mon.species === "treecko")) {
      await settleSilph(game, true)
      return
    }
    await game.controls.press("a")
    await game.wait.frames(12)
  }
  throw new Error(`Steven did not give Treecko: ${JSON.stringify(await game.state.read())}`)
}

const visibleSilphObjects = async (game: GameSession) => {
  const symbols = await readSkyEmuSymbols(requireSymbolsPath())
  const client = (game as unknown as { running: RunningSkyEmu }).running.client
  const objects = []
  for (let index = 0; index < 16; index++) {
    const bytes = await client.readBytes(symbols.address("gObjectEvents") + index * 0x24, 0x24)
    if (!(bytes[0]! & 1)) continue
    const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength)
    objects.push({ localId: bytes[8], x: view.getInt16(0x10, true) - 7, y: view.getInt16(0x12, true) - 7 })
  }
  return objects
}

describe.sequential("Wayfarer Silph staff and rewards", () => {
  let game: GameSession
  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  for (const liberated of [false, true]) {
    it(`keeps Steven's Hoenn starter gift available with Silph liberation=${liberated}`, async () => {
      await arrangeSilph(game, {
        player: { position: { map: "silph-lobby", x: 17, y: 15 }, facing: "up" },
        party: [{ species: "pikachu" }],
        story: { flags: { silphLiberated: liberated } },
      })
      const origin = (await game.state.read()).origin
      expect(await game.story.flag("receivedHnsHoennStarter")).toBe(false)
      await receiveStevenStarter(game)
      expect(await game.story.flag("receivedHnsHoennStarter")).toBe(true)
      expect(await game.story.flag("silphLiberated")).toBe(liberated)
      expect((await game.state.read()).party.map((mon) => mon.species)).toEqual(["pikachu", "treecko"])
      expect((await game.state.read()).origin).toEqual(origin)
      await game.saveAndReload()
      expect(await game.story.flag("receivedHnsHoennStarter")).toBe(true)
      expect((await game.state.read()).party.map((mon) => mon.species)).toEqual(["pikachu", "treecko"])
    })
  }

  it("gives Lapras to the party before liberation and without a Blue encounter", async () => {
    await arrangeSilph(game, {
      player: { position: { map: "silph-7f", x: 1, y: 7 }, facing: "right" },
      party: [{ species: "pikachu", moves: ["tackle"] }],
    })
    expect(await visibleSilphObjects(game)).toContainEqual({ localId: 1, x: 2, y: 7 })
    await talkSilph(game, true)
    expect(await game.story.flag("gotSilphLapras")).toBe(true)
    expect((await game.state.read()).party.map((mon) => mon.species)).toEqual(["pikachu", "lapras"])
    expect(await game.story.flag("silphLiberated")).toBe(false)
    await game.saveAndReload()
    await talkSilph(game, true)
    expect((await game.state.read()).party).toHaveLength(2)
  })

  it("keeps Lapras pending with full party and PC, then delivers to the freed PC slot", async () => {
    await arrangeSilph(game, {
      player: { position: { map: "silph-7f", x: 1, y: 7 }, facing: "right" },
    })
    await game.storage.arrangeCapacity("full")
    await talkSilph(game, true)
    expect(await game.story.flag("gotSilphLapras")).toBe(false)
    await game.saveAndReload()
    await game.storage.arrangeCapacity("one-pc-slot-free")
    await talkSilph(game, true)
    expect(await game.story.flag("gotSilphLapras")).toBe(true)
    expect((await game.state.read()).party).toHaveLength(6)
    expect((await game.storage.slot(0, 0)).mon).toMatchObject({ species: "lapras", level: 25 })
    await game.saveAndReload()
    await talkSilph(game, true)
    expect(await game.story.flag("gotSilphLapras")).toBe(true)
  })

  it("keeps Thunder Wave retryable after declining and cancelling, then records learning", async () => {
    await arrangeSilph(game, {
      player: { position: { map: "silph-2f", x: 10, y: 5 }, facing: "up" },
      party: [{ species: "pikachu", moves: ["tackle"] }],
    })
    await talkSilph(game, true)
    expect(await game.story.flag("silphThunderWaveTutor")).toBe(false)
    await enterTutorChooser(game)
    await game.wait.frames(60)
    await game.controls.press("b")
    await settleSilph(game, true)
    expect(await game.story.flag("silphThunderWaveTutor")).toBe(false)
    await game.saveAndReload()
    await enterTutorChooser(game)
    await game.wait.frames(60)
    await game.controls.press("a")
    await settleSilph(game)
    expect((await game.state.read()).party[0]!.moves).toContain("thunderWave")
    expect(await game.story.flag("silphThunderWaveTutor")).toBe(true)
    await game.saveAndReload()
    await talkSilph(game, true)
    expect((await game.state.read()).party[0]!.moves.filter((move) => move === "thunderWave")).toHaveLength(1)
  })

  it("keeps occupied lobby service pending while retaining upstairs access", async () => {
    await arrangeSilph(game, { player: { position: { map: "silph-lobby", x: 30, y: 2 }, facing: "right" } })
    await talkSilph(game, true)
    expect(await game.story.flag("receivedUpGrade")).toBe(false)
    expect(await game.inventory.count("upGrade")).toBe(0)
    await talkSilph(game)
    await game.wait.forMap("silph-2f")
    expect(await game.story.flag("silphLiberated")).toBe(false)
  })

  it("retries Up-Grade after freeing an Items slot and retains access after receipt", async () => {
    await arrangeSilph(game, {
      player: { position: { map: "silph-lobby", x: 30, y: 2 }, facing: "right" },
      story: { flags: { silphLiberated: true } }, bag: { fullPockets: ["items"] },
    })
    await talkSilph(game, true)
    expect(await game.story.flag("receivedUpGrade")).toBe(false)
    expect(await game.inventory.count("upGrade")).toBe(0)
    await game.saveAndReload()
    await game.inventory.freeSlot("items")
    await talkSilph(game, true)
    expect(await game.story.flag("receivedUpGrade")).toBe(true)
    expect(await game.inventory.count("upGrade")).toBe(1)
    await talkSilph(game)
    await game.wait.forMap("silph-2f")
    expect(await game.inventory.count("upGrade")).toBe(1)
  })

  it("adds the local Up-Grade to a pre-owned stack only once", async () => {
    await arrangeSilph(game, {
      player: { position: { map: "silph-lobby", x: 30, y: 2 }, facing: "right" },
      story: { flags: { silphLiberated: true } }, bag: { items: { upGrade: 1 }, fullPockets: ["items"] },
    })
    await talkSilph(game, true)
    expect(await game.inventory.count("upGrade")).toBe(2)
    expect(await game.story.flag("receivedUpGrade")).toBe(true)
    await game.saveAndReload()
    await talkSilph(game, true)
    expect(await game.inventory.count("upGrade")).toBe(2)
  })
})
