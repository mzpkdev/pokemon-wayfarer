import { beforeEach, describe, expect, it } from "webanvil/test"
import { GameSession } from "../harness/game-session"
import { arrangeSilph, talkSilph } from "../playbooks/silph"

const president = { position: { map: "silph-11f", x: 9, y: 10 }, facing: "up" } as const
const readyReward = { silphLiberated: true, silphMasterBallPending: true } as const

describe.sequential("Wayfarer Silph President Master Ball", () => {
  let game: GameSession
  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("keeps the occupied President separate from the local reward", async () => {
    await arrangeSilph(game, { player: president })
    await talkSilph(game)
    expect(await game.inventory.count("masterBall")).toBe(0)
    expect(await game.story.flag("silphMasterBallPending")).toBe(false)
    expect(await game.story.flag("silphMasterBallReceived")).toBe(false)
  })

  it("adds one Silph Master Ball to a pre-owned stack and persists one local receipt", async () => {
    await arrangeSilph(game, {
      player: president,
      story: { flags: readyReward },
      bag: { items: { masterBall: 1 } },
    })
    await talkSilph(game)
    expect(await game.inventory.count("masterBall")).toBe(2)
    expect(await game.story.flag("silphMasterBallReceived")).toBe(true)
    expect(await game.story.flag("silphMasterBallPending")).toBe(false)
    await game.saveAndReload()
    await talkSilph(game)
    expect(await game.inventory.count("masterBall")).toBe(2)
  })

  it("retries the local handoff after a full Ball pocket is freed", async () => {
    await arrangeSilph(game, {
      player: president,
      story: { flags: readyReward },
      bag: { fullPockets: ["balls"] },
    })
    await talkSilph(game)
    expect(await game.inventory.count("masterBall")).toBe(0)
    expect(await game.story.flag("silphMasterBallReceived")).toBe(false)
    expect(await game.story.flag("silphMasterBallPending")).toBe(true)
    await game.saveAndReload()
    await game.inventory.freeFixtureSlot("balls")
    await talkSilph(game)
    expect(await game.inventory.count("masterBall")).toBe(1)
    expect(await game.story.flag("silphMasterBallReceived")).toBe(true)
    expect(await game.story.flag("silphMasterBallPending")).toBe(false)
  })
})
