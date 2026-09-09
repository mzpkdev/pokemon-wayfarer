import { beforeEach, describe, expect, it } from "webanvil/test"
import { GameSession, type PartyMonFixture, type StoryFlag } from "../harness/game-session"
import { storyFlags } from "../harness/game-session/catalog"
import { enterSilphMap, arrangeSilph, moveSilph, settleSilph, talkSilph, triggerSilphGiovanni, waitSilphBattle } from "../playbooks/silph"

const occupationFlags = Object.keys(storyFlags).filter((flag) =>
  flag.startsWith("silph") && flag.endsWith("Defeated") && flag !== "silphGiovanniDefeated",
) as StoryFlag[]

const assertUnrelated = async (game: GameSession): Promise<void> => {
  expect(await game.story.var("saffronCityState")).toBe(2)
  expect(await game.story.var("goldenrodCityState")).toBe(5)
  expect(await game.story.var("cinnabarIslandState")).toBe(7)
  expect(await game.story.var("dragonsDenQuiz")).toBe(4)
  expect(await game.story.flag("celadonHideoutGiovanniDefeated")).toBe(false)
  expect(await game.inventory.count("goldenrodCardKey")).toBe(1)
  expect(await game.inventory.count("masterBall")).toBe(0)
}

const arrangeGiovanni = async (game: GameSession, party: PartyMonFixture[]): Promise<void> => {
  await arrangeSilph(game, {
    player: { position: { map: "silph-11f", x: 5, y: 16 }, facing: "up" },
    party,
    bag: { items: { goldenrodCardKey: 1 } },
    story: {
      flags: { silph11FDoor: true },
      vars: { saffronCityState: 2, goldenrodCityState: 5, cinnabarIslandState: 7, dragonsDenQuiz: 4 },
    },
  })
}

const selectElevator = async (game: GameSession, direction: "up" | "down", steps: number): Promise<void> => {
  await moveSilph(game, "up", 2, 3)
  await moveSilph(game, "up", 2, 2)
  await moveSilph(game, "left", 1, 2)
  await game.controls.press("left")
  await game.player.interact()
  await game.dialogue.waitForOpen()
  await game.wait.frames(90)
  for (let i = 0; i < steps; i++) {
    await game.controls.press(direction)
    await game.wait.frames(15)
  }
  await game.controls.press("a")
  await settleSilph(game)
  await moveSilph(game, "right", 2, 2)
  await moveSilph(game, "down", 2, 3)
  await moveSilph(game, "down", 2, 4)
  await game.controls.press("down")
  await game.wait.frames(20)
  await game.controls.press("down")
}

describe.sequential("Wayfarer Silph liberation", () => {
  let game: GameSession
  beforeEach(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it("walks from the retained HNS entrance to both upper-floor access interactions", async () => {
    await arrangeSilph(game, {
      player: { position: { map: "saffron-city", x: 33, y: 31 }, facing: "up" },
      story: { vars: { farawayIslandStepCounter: 173 } },
    })
    await enterSilphMap(game, "up", "silph-lobby")
    for (let y = 18; y >= 5; y--) await moveSilph(game, "up", 8, y)
    for (const x of [9, 10]) await moveSilph(game, "right", x, 5)
    await moveSilph(game, "up", 10, 4)
    for (let x = 11; x <= 30; x++) await moveSilph(game, "right", x, 4)
    await moveSilph(game, "up", 30, 3)
    await moveSilph(game, "right", 31, 3)
    await game.controls.press("up")
    await talkSilph(game)
    await game.wait.forMap("silph-2f")
    expect((await game.state.read()).player).toMatchObject({ x: 30, y: 3 })
    await moveSilph(game, "up", 30, 2)
    await enterSilphMap(game, "right", "silph-lobby")
    expect((await game.state.read()).player).toMatchObject({ x: 31, y: 3 })
    await game.wait.frames(90)
    expect((await game.state.read()).map.name).toBe("silph-lobby")
    await moveSilph(game, "left", 30, 3)
    await moveSilph(game, "up", 30, 2)
    await game.controls.press("right")
    await talkSilph(game)
    await game.wait.forMap("silph-2f")
    await moveSilph(game, "up", 30, 2)
    await enterSilphMap(game, "right", "silph-lobby")
    await moveSilph(game, "down", 31, 4)
    for (let x = 30; x >= 22; x--) await moveSilph(game, "left", x, 4)
    await game.controls.press("up")
    await talkSilph(game)
    await game.wait.forMap("silph-elevator")
    expect((await game.state.read()).player).toMatchObject({ x: 2, y: 4 })
    await selectElevator(game, "up", 10)
    await game.wait.forMap("silph-11f")
    expect(await game.story.var("farawayIslandStepCounter")).toBe(173)
    await game.player.warp("silph-11f", 13, 4, "up")
    await enterSilphMap(game, "up", "silph-elevator")
    await selectElevator(game, "down", 10)
    await game.wait.forMap("silph-lobby")
    expect((await game.state.read()).player).toMatchObject({ x: 22, y: 4 })
    await game.wait.frames(90)
    expect((await game.state.read()).map.name).toBe("silph-lobby")
    expect(await game.story.var("farawayIslandStepCounter")).toBe(173)
  })

  for (const liberated of [false, true]) {
    it(`crosses both removed Blue triggers with liberation=${liberated}`, async () => {
      await arrangeSilph(game, {
        player: { position: { map: "silph-7f", x: 2, y: 3 }, facing: "down" },
        story: { flags: { silphLiberated: liberated }, vars: { cinnabarIslandState: 7, dragonsDenQuiz: 4 } },
      })
      const before = await game.state.read()
      for (const y of [4, 5, 6]) {
        await moveSilph(game, "down", 2, y)
        expect(await game.state.read()).toMatchObject({ ready: true, scriptActive: false, controlsLocked: false, dialogueOpen: false })
        expect((await game.state.read()).battle.active).toBe(false)
      }
      expect(await game.story.var("cinnabarIslandState")).toBe(7)
      expect(await game.story.var("dragonsDenQuiz")).toBe(4)
      expect((await game.state.read()).circuit).toEqual(before.circuit)
    })
  }

  for (const [shape, party] of [
    ["empty", []], ["fainted", [{ species: "lapras", fainted: true }]],
    ["egg", [{ species: "lapras", egg: true }]],
    ["fainted and egg", [{ species: "lapras", fainted: true }, { species: "lapras", egg: true }]],
  ] as [string, PartyMonFixture[]][]) {
    it(`keeps a ${shape} party outside Giovanni's scene`, async () => {
      await arrangeGiovanni(game, party)
      expect((await game.state.read()).party).toHaveLength(party.length)
      await triggerSilphGiovanni(game, false)
      await settleSilph(game)
      expect(await game.story.flag("silphLiberated")).toBe(false)
      expect(await game.story.flag("silphGiovanniDefeated")).toBe(false)
      expect(await game.story.var("silphGiovanniScene")).toBe(0)
      await assertUnrelated(game)
    })
  }

  it("retries Giovanni after a loss without committing liberation", async () => {
    await arrangeGiovanni(game, [{ species: "lapras", level: 1 }])
    await triggerSilphGiovanni(game)
    await waitSilphBattle(game)
    await game.battle.lose()
    await settleSilph(game)
    expect(await game.story.flag("silphLiberated")).toBe(false)
    expect(await game.story.flag("silphMasterBallPending")).toBe(false)
    expect(await game.story.flag("silphGiovanniDefeated")).toBe(false)
    expect(await game.story.var("silphGiovanniScene")).toBe(0)
    await assertUnrelated(game)
    await game.player.warp("silph-11f", 5, 16, "up")
    await triggerSilphGiovanni(game)
    await waitSilphBattle(game)
  })

  it("forfeits Giovanni after confirmation and leaves the encounter retryable", async () => {
    await arrangeGiovanni(game, [{ species: "lapras", level: 100 }])
    await triggerSilphGiovanni(game)
    await waitSilphBattle(game)
    await game.controls.press("down")
    await game.wait.frames(12)
    await game.controls.press("right")
    await game.wait.frames(12)
    expect((await game.state.read()).battle.cursor).toBe(3)
    await game.controls.press("a")
    await game.wait.until(
      (state) => state.battle.active && state.battle.ui === "forfeit-prompt" && state.battle.cursor === 1,
      "Giovanni forfeit confirmation",
    )
    // The native yes/no box starts on No; choose Yes to confirm the forfeit.
    await game.controls.press("up")
    await game.wait.until(
      (state) => state.battle.active && state.battle.ui === "forfeit-prompt" && state.battle.cursor === 0,
      "Giovanni forfeit confirmation selected",
    )
    await game.controls.press("a")
    await settleSilph(game)
    expect((await game.state.read()).map.name).not.toBe("silph-11f")
    expect(await game.story.flag("silphLiberated")).toBe(false)
    expect(await game.story.flag("silphMasterBallPending")).toBe(false)
    expect(await game.story.flag("silphGiovanniDefeated")).toBe(false)
    expect(await game.story.var("silphGiovanniScene")).toBe(0)
    await assertUnrelated(game)
    await game.player.warp("silph-11f", 5, 16, "up")
    await triggerSilphGiovanni(game)
    await waitSilphBattle(game)
  })

  it("liberates Silph from a mixed usable party without inventing skipped victories or rewards", async () => {
    await arrangeGiovanni(game, [
      { species: "lapras", fainted: true }, { species: "lapras", egg: true }, { species: "lapras", level: 100 },
    ])
    const before = await game.state.read()
    await triggerSilphGiovanni(game)
    await waitSilphBattle(game)
    await game.battle.win()
    await settleSilph(game)
    expect(await game.story.flag("silphLiberated")).toBe(true)
    expect(await game.story.flag("silphMasterBallPending")).toBe(true)
    expect(await game.story.flag("silphGiovanniDefeated")).toBe(true)
    for (const flag of occupationFlags) expect(await game.story.flag(flag)).toBe(false)
    expect(await game.story.flag("gotSilphLapras")).toBe(false)
    expect((await game.state.read()).circuit.badges).toEqual(before.circuit.badges)
    expect((await game.state.read()).circuit.clears).toEqual(before.circuit.clears)
    await assertUnrelated(game)
    await moveSilph(game, "right", 6, 15)
    await game.player.warp("silph-11f", 16, 11, "down")
    await moveSilph(game, "down", 16, 12)
    await game.saveAndReload()
    await game.player.warp("silph-2f", 29, 13, "up")
    await moveSilph(game, "up", 29, 12)
    for (const flag of occupationFlags) expect(await game.story.flag(flag)).toBe(false)
    await assertUnrelated(game)
  })

  it("keeps ordinary occupation battles retryable without a party and records only a won encounter", async () => {
    await arrangeSilph(game, { player: { position: { map: "silph-2f", x: 29, y: 13 }, facing: "up" }, party: [] })
    await talkSilph(game)
    expect(await game.story.flag("silphLiberated")).toBe(false)
    for (const flag of occupationFlags) expect(await game.story.flag(flag)).toBe(false)
    await arrangeSilph(game, { player: { position: { map: "silph-2f", x: 29, y: 13 }, facing: "up" } })
    await game.player.interact()
    await game.dialogue.waitForOpen()
    await waitSilphBattle(game)
    await game.battle.win()
    await settleSilph(game)
    let wins = 0
    for (const flag of occupationFlags) if (await game.story.flag(flag)) wins++
    expect(wins).toBe(1)
    expect(await game.story.flag("silphLiberated")).toBe(false)
    expect(await game.story.flag("silphMasterBallPending")).toBe(false)
  })
})
