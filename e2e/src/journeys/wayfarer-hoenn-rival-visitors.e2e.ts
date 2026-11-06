import { beforeAll, describe, expect, it } from "webanvil/test"

import { type GameMap, GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"
import {
  advanceOpeningUntil,
  finishOpeningDialogue,
  stepOpening,
} from "../playbooks/regional-opening"
import { rideAquaFromVermilionToSlateport } from "../playbooks/wayfarer-ports"

// Brendan's and May's teams answer the starter taken from Birch. A visitor from another region
// has none, so their scenes wait: the forced ones do not start, and the rivals who would stand in
// the open are not there. Each scene plays, and pays out, once the starter is chosen.

const NO_STARTER = 0xffff

const route110Rival = 28
const rustboroRival = 14
const lilycoveRival = 17

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

/** Reads a scene through with A until `done`, then until the field is free again. */
const playSceneUntil = async (
  game: GameSession,
  done: () => Promise<boolean>,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 120; attempt++) {
    await game.wait.frames(45)
    const state = await game.state.read()
    // The ready bit can report true between two pages, so the scene's own state decides.
    if (state.ready && !state.scriptActive && !state.controlsLocked && (await done())) return
    await game.controls.press("a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

describe.sequential("Wayfarer Hoenn rival scenes for a visitor", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it(
    "leaves a visitor without Birch's starter in control on every Brendan and May scene",
    { timeout: 300_000 },
    async () => {
      // A Johto start that sails to Slateport: the ordinary way a visitor's Hoenn begins.
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: "vermilion-port-inside", x: 8, y: 9 } },
        story: {
          vars: { ssAquaState: 8 },
          flags: { disableEncounters: true, disableTrainers: true },
        },
        bag: { items: { ssTicket: 1 } },
        party: [{ species: "salamence", level: 60 }],
        determinism: { textSpeed: "instant" },
      })
      await rideAquaFromVermilionToSlateport(game)
      expect((await game.state.read()).origin).toMatchObject({
        hoennInitialized: true,
        hoennChoice: NO_STARTER,
        hoennReceived: false,
      })

      // Route 110: the road's two trigger tiles are (33-34,56), and the rival would stand at (34,54).
      await walkUpFrom(game, "route-110", 33, 58, 4)
      expect(sees(await game.state.read(), route110Rival)).toBe(false)
      await expect(game.story.var("hoennRoute110State")).resolves.toBe(0)
      await expect(game.inventory.contains("dowsingMachine")).resolves.toBe(false)

      // Route 119: the trigger is (25,31), where the rival would ride in and hand over Fly.
      await walkUpFrom(game, "route-119", 25, 33, 2)
      await stepOpening(game, "down")
      await stepOpening(game, "up")
      await game.wait.frames(60)
      await expectFreeAt(game, "route-119", 25, 31)
      await expect(game.story.var("hoennRoute119State")).resolves.toBe(0)
      await expect(game.story.var("hoennScottState")).resolves.toBe(0)
      await expect(game.inventory.contains("fly")).resolves.toBe(false)

      // Rustboro after the Devon errand: the row of triggers is y=53, below the rival at (16,50).
      await game.story.setVar("hoennRustboroCityState", 7)
      await walkUpFrom(game, "rustboro-city", 16, 55, 3)
      expect(sees(await game.state.read(), rustboroRival)).toBe(false)
      await expect(game.story.var("hoennRustboroCityState")).resolves.toBe(7)

      // Route 104, outside Mr. Briney's cottage: the trigger is the tile below his door.
      await game.story.setVar("hoennRoute104State", 1)
      await walkUpFrom(game, "route-104", 17, 53, 2)
      await expect(game.story.var("hoennRoute104State")).resolves.toBe(1)
      await expect(game.story.var("hoennRustboroCityState")).resolves.toBe(7)

      // Lilycove: the rival shops at (27,7) from the start of an Emerald game.
      await game.player.warp("lilycove-city", 27, 9, "up")
      await game.wait.forReady()
      expect(sees(await game.state.read(), lilycoveRival)).toBe(false)

      // The Game Corner girl's spare Doll is the player's Hoenn starter: she keeps it for now.
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

  it(
    "plays the Route 110 and Route 119 scenes, with their gifts, once Birch's starter is taken",
    { timeout: 300_000 },
    async () => {
      // The visitor's rescue: Birch's bag is refused, the wild battle is fought with the own party,
      // and Birch offers a local starter in his lab.
      await game.player.warp("route-101", 9, 19, "right")
      await game.wait.forReady()
      await game.player.move("right")
      await finishOpeningDialogue(game)
      await game.player.warp("route-101", 7, 15, "up")
      await game.player.interact()
      await advanceOpeningUntil(
        game,
        (state) => state.battle.ui === "action-menu",
        "The visitor's rescue battle did not start",
      )
      await game.battle.win()
      await advanceOpeningUntil(
        game,
        (state) => state.dialogue.text.includes("You handled"),
        "Birch did not offer the visitor a starter",
      )
      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.frames(45)
      await game.controls.press("a")
      await game.wait.frames(45)
      await game.controls.press("down")
      await game.controls.press("a")
      await advanceOpeningUntil(
        game,
        (state) => state.dialogue.text.includes("will guide"),
        "Birch did not offer the chosen starter",
      )
      await game.wait.frames(45)
      await game.controls.press("a")
      await game.wait.frames(45)
      await game.controls.press("a")
      // B from here: it reads the messages on and answers the nickname question with no.
      await advanceOpeningUntil(
        game,
        (state) => state.ready && state.origin.hoennReceived,
        "Birch did not hand over the starter",
        "b",
      )
      expect(await game.state.read()).toMatchObject({
        origin: { hoennChoice: 1, hoennReceived: true },
        party: [{ species: "salamence" }, { species: "torchic" }],
      })

      // Route 110: the rival is back on the road, battles, and gives the Itemfinder.
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
      await playSceneUntil(
        game,
        async () => (await game.story.var("hoennScottState")) === 1,
        "The Route 119 scene did not finish",
      )
      await expect(game.story.var("hoennRoute119State")).resolves.toBe(1)
      await expect(game.inventory.contains("fly")).resolves.toBe(true)
      await stepOpening(game, "down")

      // The rivals who stand in the open are there again too.
      await game.story.setVar("hoennRustboroCityState", 7)
      await game.player.warp("rustboro-city", 16, 55, "up")
      await game.wait.forReady()
      expect(sees(await game.state.read(), rustboroRival)).toBe(true)
      await game.player.warp("lilycove-city", 27, 9, "up")
      await game.wait.forReady()
      expect(sees(await game.state.read(), lilycoveRival)).toBe(true)
    },
  )
})
