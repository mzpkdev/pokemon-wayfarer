import { beforeAll, describe, expect, it } from "webanvil/test"

import { type GameMap, GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  receiveBirchStarter,
  stepOpening,
} from "../playbooks/regional-opening"
import { rideAquaFromVermilionToSlateport } from "../playbooks/wayfarer-ports"

// Brendan's and May's rivalry is for a player who started in Hoenn. A visitor never battles them and
// never needs Birch: the rival appears only for a gift, introduces themself the first time, hands it
// over, and leaves. A Hoenn-origin player keeps every battle.

const NO_STARTER = 0xffff

const route103Rival = 2
const route110Rival = 28
const rustboroRival = 14
const lilycoveRival = 17
const route119Scott = 43
const littlerootRival = 7
const birchLabBirch = 2
const birchLabRival = 3

const sees = (state: GameState, localId: number): boolean =>
  state.objects.some((object) => object.localId === localId && object.visible)

const expectFreeAt = async (game: GameSession, map: GameMap, x: number, y: number) => {
  const state = await game.state.read()
  expect(state).toMatchObject({
    ready: true,
    controlsLocked: false,
    scriptActive: false,
    dialogueOpen: false,
    map: { name: map },
    player: { x, y },
  })
  expect(state.battle.active).toBe(false)
}

/** Walks `steps` tiles up from a warp tile: each step must move the player, so a scene that took control fails it. */
const walkUpFrom = async (
  game: GameSession,
  map: GameMap,
  x: number,
  y: number,
  steps: number,
): Promise<void> => {
  await game.player.warp(map, x, y, "up")
  await game.wait.forReady()
  for (let step = 0; step < steps; step++) await stepOpening(game, "up")
  await game.wait.frames(60)
  await expectFreeAt(game, map, x, y - steps)
}

/**
 * Reads a scene through with A until `done`, then until the field is free again. Returns what was
 * said, and fails on a battle.
 */
const playSceneUntil = async (
  game: GameSession,
  done: () => Promise<boolean>,
  description: string,
  allowBattle = false,
): Promise<string> => {
  let said = (await game.state.read()).dialogue.fullText
  const before = said
  for (let attempt = 0; attempt < 160; attempt++) {
    await game.wait.frames(30)
    const state = await game.state.read()
    if (!allowBattle) expect(state.battle.active).toBe(false)
    if (!said.endsWith(state.dialogue.fullText)) said += ` ${state.dialogue.fullText}`
    // The ready bit can report true between two pages, so the scene's own state decides.
    if (state.ready && !state.scriptActive && !state.controlsLocked && (await done()))
      return said.slice(before.length)
    await game.controls.press("a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

const arrangeVisitor = async (
  game: GameSession,
  bag: { items?: { dowsingMachine: number }; hms?: { fly: number } } = {},
) => {
  // A Johto start that sails to Slateport: the ordinary way a visitor's Hoenn begins.
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing: "down", position: { map: "vermilion-port-inside", x: 8, y: 9 } },
    story: {
      vars: { ssAquaState: 8 },
      flags: { disableEncounters: true, disableTrainers: true },
    },
    bag: { items: { ssTicket: 1, ...bag.items }, hms: bag.hms },
    party: [{ species: "salamence", level: 60 }],
    determinism: { textSpeed: "instant" },
  })
  await rideAquaFromVermilionToSlateport(game)
}

describe.sequential("Wayfarer Hoenn rival scenes for a visitor", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it(
    "gives a visitor who never met Birch the gifts in any order, with no battle",
    { timeout: 300_000 },
    async () => {
      await arrangeVisitor(game)
      expect((await game.state.read()).origin).toMatchObject({
        hoennInitialized: true,
        hoennChoice: NO_STARTER,
        hoennReceived: false,
      })
      // A visitor arrives after Birch's rescue, which belongs to a Hoenn-origin player.
      await expect(game.story.flag("hoennRescuedBirch")).resolves.toBe(true)
      await expect(game.story.var("hoennBirchLabState")).resolves.toBe(0)

      // Route 119 first: the rival rides in, introduces themself, and hands over Fly. Scott stays away.
      await game.player.warp("route-119", 25, 33, "up")
      await game.wait.forReady()
      await stepOpening(game, "up")
      await game.player.move("up")
      const route119 = await playSceneUntil(
        game,
        async () => (await game.story.var("hoennRoute119State")) === 1,
        "The Route 119 scene did not finish",
      )
      expect(route119).toContain("I’m MAY")
      expect(route119).toContain("is my dad")
      expect(route119).not.toContain("Way to go")
      await expect(game.inventory.contains("fly")).resolves.toBe(true)
      await expect(game.story.var("hoennScottState")).resolves.toBe(1)
      expect(sees(await game.state.read(), route119Scott)).toBe(false)

      // It plays once, and leaves the road free.
      await stepOpening(game, "down")
      await stepOpening(game, "up")
      await game.wait.frames(60)
      await expectFreeAt(game, "route-119", 25, 31)

      // Lavaridge, right after Flannery with no Birch: the scene waits at the door, and greets again.
      await game.story.setVar("hoennLavaridgeTownState", 4)
      await game.player.warp("lavaridge-gym-1f", 13, 18, "down")
      await game.wait.forReady()
      await stepOpening(game, "down")
      const lavaridge = await playSceneUntil(
        game,
        async () => (await game.story.var("hoennLavaridgeTownState")) === 2,
        "The Lavaridge scene did not finish",
      )
      expect(lavaridge).toContain("again")
      expect(lavaridge).not.toContain("I’m MAY")
      expect(lavaridge).not.toContain("NORMAN")
      await expect(game.inventory.count("goGoggles")).resolves.toBe(1)

      // Route 110 last: the rival stands on the road and gives the Itemfinder.
      await game.player.warp("route-110", 33, 58, "up")
      await game.wait.forReady()
      expect(sees(await game.state.read(), route110Rival)).toBe(true)
      await stepOpening(game, "up")
      await game.player.move("up")
      const route110 = await playSceneUntil(
        game,
        async () => (await game.story.var("hoennRoute110State")) === 1,
        "The Route 110 scene did not finish",
      )
      expect(route110).toContain("again")
      await expect(game.inventory.count("dowsingMachine")).resolves.toBe(1)
      expect(sees(await game.state.read(), route110Rival)).toBe(false)
      await stepOpening(game, "down")
      expect((await game.state.read()).origin.hoennChoice).toBe(NO_STARTER)

      // The scenes without a gift never show the rival.
      await game.player.warp("route-103", 10, 5, "up")
      await game.wait.forReady()
      expect(sees(await game.state.read(), route103Rival)).toBe(false)
      await game.story.setVar("hoennRustboroCityState", 7)
      await walkUpFrom(game, "rustboro-city", 16, 55, 3)
      expect(sees(await game.state.read(), rustboroRival)).toBe(false)
      await expect(game.story.var("hoennRustboroCityState")).resolves.toBe(7)
      await game.story.setVar("hoennRoute104State", 1)
      await walkUpFrom(game, "route-104", 17, 53, 2)
      await expect(game.story.var("hoennRoute104State")).resolves.toBe(1)
      await game.player.warp("lilycove-city", 27, 9, "up")
      await game.wait.forReady()
      expect(sees(await game.state.read(), lilycoveRival)).toBe(false)

      // The Game Corner girl's spare Doll is the player's Hoenn starter, and a visitor has none.
      await game.player.warp("mauville-game-corner", 7, 4, "up")
      await game.wait.forReady()
      await game.player.interact()
      await game.wait.until((state) => state.dialogueOpen, "the Game Corner girl's line")
      expect(await game.dialogue.text()).toContain("jackpot")
      await finishOpeningDialogue(game)
      await stepOpening(game, "down")
      await expectFreeAt(game, "mauville-game-corner", 7, 5)
    },
  )

  it("plays no scene for a gift a visitor already holds", { timeout: 300_000 }, async () => {
    await arrangeVisitor(game, { items: { dowsingMachine: 1 }, hms: { fly: 1 } })

    await game.player.warp("route-110", 33, 58, "up")
    await game.wait.forReady()
    expect(sees(await game.state.read(), route110Rival)).toBe(false)
    await walkUpFrom(game, "route-110", 33, 58, 4)
    await expect(game.story.var("hoennRoute110State")).resolves.toBe(0)

    // Scott still counts this meeting, so his reward stays reachable.
    await game.player.warp("route-119", 25, 33, "up")
    await game.wait.forReady()
    await stepOpening(game, "up")
    await stepOpening(game, "up")
    await game.wait.frames(60)
    await expectFreeAt(game, "route-119", 25, 31)
    await expect(game.story.var("hoennRoute119State")).resolves.toBe(1)
    await expect(game.story.var("hoennScottState")).resolves.toBe(1)
    await expect(game.inventory.count("dowsingMachine")).resolves.toBe(1)
    await expect(game.inventory.count("fly")).resolves.toBe(1)
    await expect(game.story.flag("hoennMetRivalAsVisitor")).resolves.toBe(false)
  })

  it("keeps the rival out of a visitor's National Dex scene", { timeout: 300_000 }, async () => {
    await arrangeVisitor(game)
    // The scene starts on arriving in town: here, stepping out of the rival's house.
    await game.player.warp("mays-house-1f", 2, 8, "down")
    await game.wait.forReady()
    await game.story.setVar("hoennDexUpgradeState", 1)
    await game.player.move("down")
    await game.wait.until(
      (state) => state.map.name === "littleroot-town" && state.dialogueOpen,
      "Birch's National Dex scene",
    )
    expect(sees(await game.state.read(), littlerootRival)).toBe(false)
    // His call in town: a Johto starter who never talked to him neither met him nor got his POKéDEX.
    const call = (await game.state.read()).dialogue.fullText
    expect(call).toContain("from another region")
    expect(call).not.toContain("I gave you")
    expect(call).not.toContain("first saw you")
    const said = await playSceneUntil(
      game,
      async () => (await game.story.var("hoennDexUpgradeState")) === 2,
      "The National Dex scene did not finish",
    )
    expect(said).not.toContain("MAY:")
    expect(said).not.toContain("BRENDAN:")
    // In the lab: no rival helped, and one with no POKéDEX is handed one before the upgrade.
    expect(said).toContain("take this one")
    expect(said).toContain("come all the way")
    expect(said).not.toContain("Thanks to your help")
    expect(said).toContain("upgraded")
    expect(await game.state.read()).toMatchObject({ map: { name: "birch-lab" } })
    expect(sees(await game.state.read(), birchLabRival)).toBe(false)

    // A League win keeps Birch in his lab for the scenes after this one, thanked or not.
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "up", position: { map: "birch-lab", x: 6, y: 8 } },
      story: {
        vars: { hoennDexUpgradeState: 3 },
        flags: { hoennGameClear: true, hoennHideBirchLabBirch: true, hoennHideBirchLabRival: true },
      },
      party: [{ species: "salamence", level: 60 }],
      determinism: { textSpeed: "instant" },
    })
    expect(sees(await game.state.read(), birchLabBirch)).toBe(true)
    expect(sees(await game.state.read(), birchLabRival)).toBe(false)
  })

  it("keeps every battle for a player who started in Hoenn", { timeout: 300_000 }, async () => {
    await game.arrange({
      checkpoint: "hoenn-before-rescue",
      player: { facing: "right", position: { map: "route-101", x: 9, y: 19 } },
      story: { flags: { disableEncounters: true, disableTrainers: true } },
      determinism: { textSpeed: "instant" },
    })
    await receiveBirchStarter(game, 1)

    // Route 110: the rival is on the road, battles, and gives the Itemfinder.
    await game.player.warp("route-110", 33, 58, "up")
    await game.wait.forReady()
    expect(sees(await game.state.read(), route110Rival)).toBe(true)
    await stepOpening(game, "up")
    await game.player.move("up")
    await advanceOpeningUntil(
      game,
      (state) => state.battle.active,
      "The Route 110 rival did not battle",
    )
    await game.battle.win()
    await playSceneUntil(
      game,
      async () => (await game.story.var("hoennRoute110State")) === 1,
      "The Route 110 scene did not finish",
      true,
    )
    await expect(game.inventory.contains("dowsingMachine")).resolves.toBe(true)
    expect(sees(await game.state.read(), route110Rival)).toBe(false)
    await stepOpening(game, "down")

    // Route 119: the rival rides in, battles, and gives Fly; Scott follows.
    await game.player.warp("route-119", 25, 33, "up")
    await game.wait.forReady()
    await stepOpening(game, "up")
    await game.player.move("up")
    await advanceOpeningUntil(
      game,
      (state) => state.battle.active,
      "The Route 119 rival did not battle",
    )
    await game.battle.win()
    const route119 = await playSceneUntil(
      game,
      async () => (await game.story.var("hoennScottState")) === 1,
      "The Route 119 scene did not finish",
      true,
    )
    expect(route119).toContain("Way to go")
    await expect(game.story.var("hoennRoute119State")).resolves.toBe(1)
    await expect(game.inventory.contains("fly")).resolves.toBe(true)
    await stepOpening(game, "down")

    // The rivals who stand in the open are there too.
    await game.story.setVar("hoennRustboroCityState", 7)
    await game.player.warp("rustboro-city", 16, 55, "up")
    await game.wait.forReady()
    expect(sees(await game.state.read(), rustboroRival)).toBe(true)
    await game.player.warp("lilycove-city", 27, 9, "up")
    await game.wait.forReady()
    expect(sees(await game.state.read(), lilycoveRival)).toBe(true)
  })
})
