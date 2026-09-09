import { beforeEach, describe, expect, it } from "webanvil/test"
import { GameSession } from "../harness/game-session"
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

describe.sequential("Wayfarer Silph staff and rewards", () => {
  let game: GameSession
  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("gives Lapras to the party before liberation and without a Blue encounter", async () => {
    await arrangeSilph(game, {
      player: { position: { map: "silph-7f", x: 1, y: 7 }, facing: "left" },
      party: [{ species: "pikachu", moves: ["tackle"] }],
    })
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
      player: { position: { map: "silph-7f", x: 1, y: 7 }, facing: "left" },
      pc: { observedSlots: [{ box: 0, slot: 0, mon: null }] },
    })
    await game.storage.arrangeCapacity("full")
    await talkSilph(game, true)
    expect(await game.story.flag("gotSilphLapras")).toBe(false)
    expect((await game.storage.slot(0, 0)).mon?.species).toBe("pidgey")
    await game.saveAndReload()
    await game.storage.arrangeCapacity("one-pc-slot-free")
    await talkSilph(game, true)
    expect(await game.story.flag("gotSilphLapras")).toBe(true)
    expect((await game.state.read()).party).toHaveLength(6)
    expect((await game.storage.slot(0, 0)).mon).toMatchObject({ species: "lapras", level: 25 })
    await game.saveAndReload()
    await talkSilph(game, true)
    expect((await game.storage.slot(0, 0)).mon).toMatchObject({ species: "lapras", level: 25 })
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

  for (const liberated of [false, true]) {
    it(`defers Steven and President rewards with liberation=${liberated}`, async () => {
      await arrangeSilph(game, {
        player: { position: { map: "silph-lobby", x: 17, y: 13 }, facing: "down" },
        story: { flags: { silphLiberated: liberated, silphMasterBallPending: liberated } },
      })
      const before = await game.state.read()
      await talkSilph(game, true)
      expect(await game.story.flag("receivedHnsHoennStarter")).toBe(false)
      expect((await game.state.read()).party).toEqual(before.party)
      expect((await game.state.read()).origin).toEqual(before.origin)
      await game.player.warp("silph-11f", 9, 10, "up")
      await talkSilph(game)
      expect(await game.story.flag("silphMasterBallPending")).toBe(liberated)
      expect(await game.inventory.count("masterBall")).toBe(0)
      await game.player.warp("silph-11f", 11, 10, "up")
      await game.player.interact()
      await game.dialogue.waitForOpen()
      if (!liberated) expect((await game.state.read()).dialogue.text.toLowerCase()).not.toContain("thanks")
      await settleSilph(game)
      expect(await game.story.flag("silphLiberated")).toBe(liberated)
    })
  }

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
    await game.inventory.freeFixtureSlot("items")
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
