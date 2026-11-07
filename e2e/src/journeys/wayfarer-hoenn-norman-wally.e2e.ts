import { beforeAll, describe, expect, it } from "webanvil/test"

import { type GameMap, GameSession, type StoryFlag } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"
import { advanceOpeningUntil, stepOpening } from "../playbooks/regional-opening"
import { rideAquaFromVermilionToSlateport } from "../playbooks/wayfarer-ports"

// Norman, Wally, and the Lavaridge rival in an order Emerald never allowed: a visitor with no Hoenn
// badge and no Birch's starter walks into the Gym at Petalburg first.

const norman = 1
const mauvilleWally = 6
const mauvilleScott = 11
const verdanturfWally = 1

const sees = (state: GameState, localId: number): boolean =>
  state.objects.some((object) => object.localId === localId && object.visible)

const expectFreeAt = async (game: GameSession, map: GameMap, x: number, y: number) => {
  await game.wait.frames(60)
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

/** Reads a scene through with A until `done`, then until the field is free again. */
const playSceneUntil = async (
  game: GameSession,
  done: () => Promise<boolean>,
  description: string,
  watch?: (state: GameState) => void,
): Promise<void> => {
  let won = false
  for (let attempt = 0; attempt < 240; attempt++) {
    await game.wait.frames(45)
    const state = await game.state.read()
    watch?.(state)
    // The ready bit can report true between two pages, so the scene's own state decides.
    if (state.ready && !state.scriptActive && !state.controlsLocked && (await done())) return
    if (!state.battle.active) won = false
    if (state.battle.ui === "action-menu" && !won) {
      await game.battle.win()
      won = true
    } else await game.controls.press("a")
  }
  throw new Error(`${description}: ${JSON.stringify(await game.state.read())}`)
}

/** Beats Wally from one side of him. Scott then walks up to wherever that left the player. */
const beatWallyFrom = async (
  game: GameSession,
  x: number,
  y: number,
  facing: "up" | "right",
): Promise<void> => {
  await game.player.warp("mauville-city", x, y, facing)
  await game.wait.forReady()
  await game.player.interact()
  let scottGap: number | undefined
  await playSceneUntil(
    game,
    () => game.story.flag("hoennDefeatedWallyMauville"),
    "Wally's Mauville scene did not finish",
    (state) => {
      const scott = state.objects.find((object) => object.localId === mauvilleScott)
      if (scott?.visible && state.dialogueOpen) {
        scottGap = Math.abs(scott.x - state.player.x) + Math.abs(scott.y - state.player.y)
      }
    },
  )
  expect(scottGap).toBe(1)
  await expectFreeAt(game, "mauville-city", x, y)
}

/** Talks to what stands ahead of a tile, reading on until the story flag is set. */
const talkFrom = async (
  game: GameSession,
  map: GameMap,
  x: number,
  y: number,
  facing: "up" | "down" | "left" | "right",
  flag: StoryFlag,
): Promise<void> => {
  await game.player.warp(map, x, y, facing)
  await game.wait.forReady()
  await game.player.interact()
  await playSceneUntil(game, () => game.story.flag(flag), `(${x},${y}) did not set ${flag}`)
}

/** Steps through a door and returns on the far side of its warp. */
const leaveThrough = async (
  game: GameSession,
  from: GameMap,
  x: number,
  y: number,
  to: GameMap,
): Promise<void> => {
  await game.player.warp(from, x, y, "down")
  await game.wait.forReady()
  await stepOpening(game, "down")
  await game.wait.forMap(to)
  await game.wait.frames(60)
}

/** Wally's catching tutorial, then NORMAN at the back of his Gym, through Wally's father's thanks. */
const tutorialThenNorman = async (game: GameSession): Promise<void> => {
    await game.player.warp("petalburg-gym", 4, 108, "up")
    await game.wait.forReady()
    await game.player.interact()
    await playSceneUntil(
      game,
      async () => (await game.story.var("hoennPetalburgGymState")) === 6,
      "The tutorial did not finish",
    )

    // The Gym reloaded around the player: its rooms stand open and the guide is in.
    await expectFreeAt(game, "petalburg-gym", 4, 108)
    await expect(game.story.flag("hidePetalburgGymGreeter")).resolves.toBe(false)
    await stepOpening(game, "up")
    await stepOpening(game, "up")

    // NORMAN waits at the back of the Gym, not at the entrance where he gave the tutorial.
    await game.player.warp("petalburg-gym", 4, 3, "up")
    await game.wait.forReady()
    const atTheBack = await game.state.read()
    expect(atTheBack.objects.find((object) => object.localId === norman)?.y).toBe(2)
    await game.player.interact()
    await advanceOpeningUntil(game, (s) => s.dialogueOpen, "Norman did not answer")
    expect(await game.dialogue.text()).not.toContain("BADGES")
    await playSceneUntil(
      game,
      async () => (await game.story.var("hoennPetalburgCityState")) >= 5,
      "Norman's win and Wally's father did not finish",
    )
    await expect(game.state.read()).resolves.toMatchObject({ circuit: { badges: { hoenn: 1 } } })

  // WALLY is still ahead in Mauville, so his mother has no reason to fear he ran away.
  await game.player.warp("petalburg-wallys-house", 7, 6, "up")
  await game.wait.forReady()
  await game.player.interact()
  await advanceOpeningUntil(game, (s) => s.dialogueOpen, "Wally's mother did not answer")
  expect(await game.dialogue.text()).toContain("really happy")
  await playSceneUntil(game, async () => true, "Wally's mother did not finish")
}

describe.sequential("Wayfarer Hoenn Norman, Wally and the Lavaridge rival", () => {
  let game: GameSession

  beforeAll(async () => {
    game = await GameSession.launch()
    return () => game.close()
  })

  it(
    "keeps Wally away and every road open until the catching tutorial",
    { timeout: 300_000 },
    async () => {
      await game.arrange({
        checkpoint: "new-bark-after-intro",
        player: { facing: "down", position: { map: "vermilion-port-inside", x: 8, y: 9 } },
        story: {
          vars: { ssAquaState: 8 },
          flags: { disableEncounters: true, disableTrainers: true },
        },
        bag: { items: { ssTicket: 1 } },
        party: [{ species: "salamence", level: 100, moves: ["tackle"] }],
        determinism: { textSpeed: "instant" },
      })
      await rideAquaFromVermilionToSlateport(game)

      // Mauville: WALLY and his uncle stay away, and the door above (8,6) is open.
      await game.player.warp("mauville-city", 8, 8, "up")
      await game.wait.forReady()
      expect(sees(await game.state.read(), mauvilleWally)).toBe(false)
      await stepOpening(game, "up")
      await stepOpening(game, "up")
      await stepOpening(game, "up")
      await game.wait.forMap("mauville-gym")

      // Victory Road: the triggers at (2-3,23) do nothing and stay armed.
      await game.player.warp("victory-road-1f", 2, 24, "up")
      await game.wait.forReady()
      await stepOpening(game, "up")
      await expectFreeAt(game, "victory-road-1f", 2, 23)
      await expect(game.story.var("hoennVictoryRoad1fState")).resolves.toBe(0)
    },
  )

  it(
    "readies the Gym from the tutorial, and Norman battles with no Hoenn badge",
    { timeout: 300_000 },
    async () => {
      await tutorialThenNorman(game)

      // Mr. Briney keeps sailing and Wattson stays in his Gym: Norman's win is only Norman's.
      await expect(game.story.flag("hoennHideRoute104BrineyBoat")).resolves.toBe(false)
      await expect(game.story.flag("hideMauvilleGymWattson")).resolves.toBe(false)
      await expect(game.story.flag("hideMauvilleCityWattson")).resolves.toBe(true)
    },
  )

  it("brings Wally back, and Victory Road recalls no win that never happened", async () => {
    await game.player.warp("mauville-city", 8, 8, "up")
    await game.wait.forReady()
    expect(sees(await game.state.read(), mauvilleWally)).toBe(true)
    await expect(game.story.flag("hoennDefeatedWallyMauville")).resolves.toBe(false)

    // He waits beside the door now, so the Gym stays open before his battle.
    await stepOpening(game, "up")
    await stepOpening(game, "up")
    await stepOpening(game, "up")
    await game.wait.forMap("mauville-gym")

    await game.player.warp("victory-road-1f", 2, 24, "up")
    await game.wait.forReady()
    await stepOpening(game, "up")
    await advanceOpeningUntil(game, (s) => s.dialogueOpen, "Wally did not come")
    const seen: string[] = []
    await advanceOpeningUntil(
      game,
      (s) => {
        seen.push(s.dialogue.fullText)
        return s.battle.active
      },
      "Wally did not battle",
    )
    expect(seen.join(" ")).toContain("Here I come")
    expect(seen.join(" ")).not.toContain("losing to you")
    await game.battle.win()
    await playSceneUntil(
      game,
      async () => (await game.story.var("hoennVictoryRoad1fState")) === 1,
      "Wally's scene did not finish",
    )
  })

  it("plays Wally's Mauville scene from below him", { timeout: 300_000 }, async () => {
    await beatWallyFrom(game, 9, 7, "up")
  })

  it(
    "lets the survey give the Go-Goggles first, and the rival's scene waits for a front door",
    { timeout: 300_000 },
    async () => {
      // Flannery's defeat parks the scene until a front door. The survey on Route 111 needs no badge.
      await game.story.setVar("hoennLavaridgeTownState", 4)
      await talkFrom(game, "route-111", 13, 63, "right", "hoennDesertSurveyStarted")
      await talkFrom(game, "route-111", 6, 71, "up", "hoennDesertSurveyStake1")
      await talkFrom(game, "route-111", 14, 89, "up", "hoennDesertSurveyStake2")
      await talkFrom(game, "route-111", 15, 116, "up", "hoennDesertSurveyStake3")
      await talkFrom(game, "route-111", 13, 63, "right", "hoennReceivedGoGoggles")
      await expect(game.inventory.count("goGoggles")).resolves.toBe(1)
      await expect(game.story.flag("hoennReceivedGoGoggles")).resolves.toBe(true)

      // The road and the back exit leave the scene parked: it has no layout there.
      await game.player.warp("lavaridge-town", 9, 12, "down")
      await game.wait.forReady()
      await expectFreeAt(game, "lavaridge-town", 9, 12)
      await game.player.warp("lavaridge-pokemon-center", 2, 2, "up")
      await game.wait.forReady()
      await stepOpening(game, "up")
      await game.wait.forMap("lavaridge-town")
      await expect(game.story.var("hoennLavaridgeTownState")).resolves.toBe(4)

      // The Pokemon Center's front door plays it, with no second pair of Go-Goggles.
      await game.player.warp("lavaridge-pokemon-center", 7, 8, "down")
      await game.wait.forReady()
      await stepOpening(game, "down")
      await playSceneUntil(
        game,
        async () => (await game.story.var("hoennLavaridgeTownState")) === 2,
        "The rival's scene did not finish at the Pokemon Center door",
      )
      await expect(game.inventory.count("goGoggles")).resolves.toBe(1)

      // It plays once.
      await leaveThrough(game, "lavaridge-gym-1f", 13, 18, "lavaridge-town")
      await expectFreeAt(game, "lavaridge-town", 5, 16)
      await expect(game.story.var("hoennLavaridgeTownState")).resolves.toBe(2)
    },
  )

  it("gives the Go-Goggles at the Gym door to a player who has none", async () => {
    await game.arrange({
      checkpoint: "new-bark-after-intro",
      player: { facing: "down", position: { map: "lavaridge-gym-1f", x: 13, y: 18 } },
      story: {
        vars: { hoennLavaridgeTownState: 4 },
        flags: { disableEncounters: true, disableTrainers: true },
      },
      party: [{ species: "salamence", level: 60 }],
      determinism: { textSpeed: "instant" },
    })
    await stepOpening(game, "down")
    await playSceneUntil(
      game,
      async () => (await game.story.var("hoennLavaridgeTownState")) === 2,
      "The rival's scene did not finish at the Gym door",
    )
    await expect(game.inventory.count("goGoggles")).resolves.toBe(1)
    await expect(game.story.flag("hoennReceivedGoGoggles")).resolves.toBe(true)
  })

  it("gives a Hoenn native the same road to Norman", { timeout: 300_000 }, async () => {
    await game.arrange({
      checkpoint: "hoenn-before-rescue",
      player: { facing: "up", position: { map: "petalburg-gym", x: 4, y: 108 } },
      story: { flags: { disableEncounters: true, disableTrainers: true } },
      party: [{ species: "salamence", level: 100 }],
      determinism: { textSpeed: "instant" },
    })
    await tutorialThenNorman(game)
  })

  // A new Hoenn game of its own: the Hoenn bank's Wally flag cannot be cleared to replay him.
  it("plays Wally's Mauville scene from his left", { timeout: 300_000 }, async () => {
    await game.arrange({
      checkpoint: "hoenn-before-rescue",
      player: { facing: "up", position: { map: "mauville-city", x: 8, y: 8 } },
      story: { flags: { disableEncounters: true, disableTrainers: true } },
      party: [{ species: "salamence", level: 100, moves: ["tackle"] }],
      determinism: { textSpeed: "instant" },
    })
    await game.story.setVar("hoennPetalburgGymState", 6)
    await beatWallyFrom(game, 8, 6, "right")
  })

  // FLANNERY first: Wally still comes to Verdanturf, and slips off only once he has said his piece.
  it("keeps Wally's Verdanturf talk when Flannery was beaten first", { timeout: 300_000 }, async () => {
    await game.arrange({
      checkpoint: "hoenn-before-rescue",
      player: { facing: "up", position: { map: "mauville-city", x: 9, y: 7 } },
      story: { flags: { disableEncounters: true, disableTrainers: true, defeatedLavaridgeGym: true } },
      party: [{ species: "salamence", level: 100, moves: ["tackle"] }],
      determinism: { textSpeed: "instant" },
    })
    await game.story.setVar("hoennPetalburgGymState", 6)
    await beatWallyFrom(game, 9, 7, "up")
    await expect(game.story.flag("hideVerdanturfWandasHouseWally")).resolves.toBe(false)

    // His uncle points next door while Wally is there, not to a boy who slipped off.
    await game.player.warp("verdanturf-wandas-house", 7, 3, "up")
    await game.wait.forReady()
    expect(sees(await game.state.read(), verdanturfWally)).toBe(true)
    await game.player.interact()
    await game.wait.until((state) => state.dialogueOpen, "Wally's uncle's line")
    expect(await game.dialogue.text()).toContain("next door")
    await playSceneUntil(game, async () => true, "Wally's uncle did not finish")

    // He wanders a step either way once the house loads, so try again if he stepped aside first.
    for (let attempt = 0; attempt < 5 && !(await game.story.flag("hoennWallySpeech")); attempt++) {
      await game.player.warp("verdanturf-wandas-house", 14, 6, "up")
      await game.wait.forReady()
      await game.player.interact()
      await game.wait.frames(30)
      if (!(await game.state.read()).dialogueOpen) continue
      await playSceneUntil(game, () => game.story.flag("hoennWallySpeech"), "Wally's speech did not finish")
    }
    await expect(game.story.flag("hoennWallySpeech")).resolves.toBe(true)
    await expect(game.story.flag("hideVerdanturfWandasHouseWally")).resolves.toBe(true)

    // Back in the house, he has gone, and his uncle says so.
    await game.player.warp("verdanturf-wandas-house", 7, 3, "up")
    await game.wait.forReady()
    expect(sees(await game.state.read(), verdanturfWally)).toBe(false)
    await game.player.interact()
    await game.wait.until((state) => state.dialogueOpen, "Wally's uncle's line")
    expect(await game.dialogue.text()).toContain("slipped off")
    await playSceneUntil(game, async () => true, "Wally's uncle did not finish")
  })
})
