import { describe, expect, it } from "webanvil/test"

import {
  GameSession,
  type GameMap,
  type GameState,
  type PartyMonFixture,
} from "../harness/game-session"
import { maps, moves, species, storyFlags, storyVars } from "../harness/game-session/catalog"

// These are deliberately journey-local fixture registrations. The public catalog stays
// limited to recurring test destinations; this coverage reaches the real authored maps.
const fixtureMaps = maps as Record<string, { mapGroup: number; mapNum: number }>
const fixtureVars = storyVars as Record<string, number>
const fixtureFlags = storyFlags as Record<string, number>
const fixtureSpecies = species as Record<string, number>
const fixtureMoves = moves as Record<string, number>

const registerMap = (name: string, mapGroup: number, mapNum: number): GameMap => {
  fixtureMaps[name] = { mapGroup, mapNum }
  return name as GameMap
}
const registerVar = (name: string, id: number) => {
  fixtureVars[name] = id
  return name
}
const registerFlag = (name: string, id: number) => {
  fixtureFlags[name] = id
  return name
}

const burnedTower = registerMap("test-burned-tower-1f", 24, 16)
const goldenrodUnderground = registerMap("test-goldenrod-underground-switches", 5, 3)
const victoryRoadKanto = registerMap("test-victory-road-kanto-1f", 24, 55)
const indigoCenter = registerMap("test-indigo-plateau-pokemon-center", 21, 0)
const rocketHideoutB2 = registerMap("test-rocket-hideout-b2f", 24, 84)
const rocketHideoutB3 = registerMap("test-rocket-hideout-b3f", 24, 85)
const radioTower4 = registerMap("test-goldenrod-radio-tower-4f", 5, 18)
const radioTower5 = registerMap("test-goldenrod-radio-tower-5f", 5, 19)
const ecruteakTheater = registerMap("test-ecruteak-theater", 6, 2)
const sproutTower3 = registerMap("test-sprout-tower-3f", 24, 4)
const tohjoGiovanni = registerMap("test-tohjo-giovanni-room", 24, 52)

const ecruteakCityTheater = registerVar("test-ecruteak-city-theater", 0x4062)
const rocketPassword = registerVar("test-rocket-password", 0x407f)
const route27State = registerVar("test-route27-state", 0x4082)
const tohjoGiovanniState = registerVar("test-tohjo-giovanni-state", 0x408d)

const hideBurnedTowerNpcs = registerFlag("test-hide-burned-tower-npcs", 0x07c)
const hideSlowpokeWellKurt = registerFlag("test-hide-slowpoke-well-kurt", 0x05e)
const hideGoldenrodUndergroundSilver = registerFlag("test-hide-goldenrod-underground-silver", 0x06f)
const hideGoldenrodRockets = registerFlag("test-hide-goldenrod-rockets", 0x06c)
const hideEcruteakTheaterKimonos = registerFlag("test-hide-ecruteak-theater-kimonos", 0x076)
const hideEcruteakRocket = registerFlag("test-hide-ecruteak-rocket", 0x079)
const hideSsaquaSailor = registerFlag("test-hide-ss-aqua-sailor", 0x088)
const hideMahoganyRockets = registerFlag("test-hide-mahogany-rockets", 0x08c)
const hideRocketHideout2ArianaAndGrunt = registerFlag(
  "test-hide-rockethideout2-ariana-and-grunt",
  0x090,
)
const hideRocketHideout3Giovanni = registerFlag("test-hide-rockethideout3-giovanni", 0x096)
const hideRocketHideout3Petrel = registerFlag("test-hide-rockethideout3-petrel", 0x09a)
const hideGoldenrodDirector = registerFlag("test-hide-goldenrod-director", 0x069)
const hideGoldenrodPetrel = registerFlag("test-hide-goldenrod-petrel", 0x06b)
const hideTohjoGiovanni = registerFlag("test-hide-tohjo-giovanni", 0x0da)
const gotPasswordFromEto = registerFlag("test-got-password-eto", 0x225)
const gotPasswordFromGruntF = registerFlag("test-got-password-grunt-f", 0x226)
const dailyBeatSilver = registerFlag("test-daily-beat-silver", 0x910)
const silverAzaleaComplete = registerFlag("test-silver-azalea-complete", 0x932)
const silverBurnedComplete = registerFlag("test-silver-burned-complete", 0x933)
const burnedTowerDiscovered = registerFlag("test-burned-tower-discovered", 0x934)
const silverGoldenrodComplete = registerFlag("test-silver-goldenrod-complete", 0x935)
const silverMahoganyComplete = registerFlag("test-silver-mahogany-complete", 0x936)
const receivedHmFlash = registerFlag("test-received-hm-flash", 0x1f6)
fixtureSpecies.testCelebi = 251
fixtureMoves.testDragonClaw = 337

const silentDelay = 120
const retreatText = "You can't keep battling"
const battleOutcomeWon = 1

type StoryPatch = {
  vars?: Record<string, number>
  flags?: Record<string, boolean>
}

const patchStory = ({ vars = {}, flags = {} }: StoryPatch) => ({
  vars: vars as never,
  flags: flags as never,
})

const noPartyVariants: readonly { name: string; party: PartyMonFixture[] }[] = [
  { name: "empty", party: [] },
  { name: "fainted", party: [{ species: "rattata", fainted: true }] },
  { name: "Egg-only", party: [{ species: "rattata", egg: true }] },
]

const reliableVictoryParty: PartyMonFixture[] = Array.from({ length: 6 }, () => ({
  species: "salamence",
  // Wayfarer applies trainer-rating obedience to owned Pokémon. A fresh circuit
  // caps at level 15, so this remains a strong legal fixture without bypassing
  // the normal battle or adding unrelated circuit progress.
  level: 15,
  moves: ["testDragonClaw" as never],
}))

const waitForDialogueText = async (
  game: GameSession,
  expected: string,
  description: string,
): Promise<GameState> => {
  const observed = new Set<string>()
  const normalize = (text: string) => text.toLowerCase().replaceAll("'", "").replaceAll("é", "")
  for (let attempt = 0; attempt < 180; attempt++) {
    const state = await game.state.read()
    if (state.dialogue.text) observed.add(state.dialogue.text)
    if (normalize(state.dialogue.text).includes(normalize(expected))) return state
    await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not show ${JSON.stringify(expected)}; observed ${JSON.stringify([...observed])}`,
  )
}

const finishFieldScript = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 480; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready && !state.dialogueOpen && !state.scriptActive) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive)
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not release: ${JSON.stringify(await game.state.read())}`)
}

const waitForTrainerBattle = async (game: GameSession, description: string): Promise<void> => {
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.battle.active) return
    if (state.battle.ui === "text" || state.dialogueOpen || state.scriptActive)
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`${description} did not enter battle: ${JSON.stringify(await game.state.read())}`)
}

const waitForBattleActionMenu = async (game: GameSession): Promise<void> => {
  for (let attempt = 0; attempt < 600; attempt++) {
    const state = await game.state.read()
    if (state.battle.ui === "action-menu") return
    if (state.battle.ui === "text") await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(`battle action menu unavailable: ${JSON.stringify(await game.state.read())}`)
}

const chooseFightMove = async (game: GameSession, description: string): Promise<void> => {
  await waitForBattleActionMenu(game)
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (cursor % 2 !== 0) await game.controls.press("left")
  if (Math.floor(cursor / 2) !== 0) await game.controls.press("up")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "move-menu", `${description} move selection`)
  await game.controls.press("a")
}

const loseActualTrainerBattle = async (game: GameSession, description: string): Promise<void> => {
  await chooseFightMove(game, description)
  let sawFaintedParty = false
  for (let attempt = 0; attempt < 1_200; attempt++) {
    const state = await game.state.read()
    sawFaintedParty ||= state.partyVitals.some((vital) => vital.hp === 0)
    if (!state.battle.active) {
      expect(sawFaintedParty).toBe(true)
      return
    }
    if (state.battle.ui === "action-menu") await chooseFightMove(game, description)
    else if (
      state.battle.ui === "move-menu" ||
      state.battle.ui === "text" ||
      state.battle.ui === "other"
    )
      await game.controls.press("a")
    else await game.wait.frames(12)
  }
  throw new Error(
    `${description} did not finish after an actual party faint: ${JSON.stringify(await game.state.read())}`,
  )
}

type ActualBattleResult = {
  outcome: number
  spentMovePp: boolean
  sawPartyFaint: boolean
}

const winActualTrainerBattle = async (
  game: GameSession,
  description: string,
): Promise<ActualBattleResult> => {
  let entered = false
  let initialMovePp: number | undefined
  let spentMovePp = false
  let sawPartyFaint = false
  let handledFaintedPartyCount = 0
  for (let attempt = 0; attempt < 1_200; attempt++) {
    const state = await game.state.read()
    entered ||= state.battle.active
    if (entered) {
      initialMovePp ??= state.partyPp[0]?.[0]
      spentMovePp ||=
        initialMovePp !== undefined && (state.partyPp[0]?.[0] ?? initialMovePp) < initialMovePp
      sawPartyFaint ||= state.partyVitals.some((vital) => vital.hp === 0)
    }
    if (entered && !state.battle.active)
      return {
        outcome: state.battle.trainerOnly.outcome,
        spentMovePp,
        sawPartyFaint,
      }
    const faintedPartyCount = state.party.filter((mon) => mon.fainted).length
    if (state.battle.ui === "other" && faintedPartyCount > handledFaintedPartyCount) {
      // This is the normal in-battle replacement flow. The six-mon fixture
      // keeps the win proof deterministic while the game owns every switch,
      // attack, and final battle outcome.
      await game.controls.press("a")
      await game.wait.frames(60)
      await game.controls.press("b")
      await game.wait.frames(30)
      await game.controls.press("right")
      await game.wait.frames(30)
      await game.controls.press("a")
      await game.wait.frames(60)
      await game.controls.press("a")
      await game.wait.frames(120)
      if ((await game.state.read()).battle.ui !== "other")
        handledFaintedPartyCount = faintedPartyCount
    } else if (state.battle.ui === "action-menu") {
      const cursor = state.battle.cursor ?? 0
      await game.wait.frames(30)
      if (cursor % 2) await game.controls.press("left")
      if (Math.floor(cursor / 2)) await game.controls.press("up")
      await game.controls.press("a")
      await game.wait.until(
        (next) => next.battle.ui === "move-menu",
        `${description} actual move selection`,
      )
      await game.wait.frames(60)
      const moveState = await game.state.read()
      const moveCursor = moveState.battle.cursor ?? 0
      if (moveCursor % 2) await game.controls.press("left")
      if (Math.floor(moveCursor / 2)) await game.controls.press("up")
      await game.controls.press("a")
      await game.wait.frames(12)
    } else if (state.ui.mode === "party-menu" && faintedPartyCount === handledFaintedPartyCount) {
      // Acknowledge only the real battle prompts. If a transitional A landed as
      // the action menu rebuilt, dismiss that non-faint party picker and retry
      // the normal Fight flow instead of changing the party by fixture magic.
      await game.controls.press("b")
      await game.wait.frames(30)
    } else if (state.battle.ui === "other") {
      // Battle speech and animations are reported as `other`; acknowledge and
      // then give the game a full turn before looking for the rebuilt menu.
      await game.controls.press("a")
      await game.wait.frames(30)
    } else if (state.battle.ui === "move-menu") {
      // A move was submitted in the action-menu branch. Let the battle task
      // consume it before polling again rather than double-confirming a stale
      // cursor frame.
      await game.wait.frames(30)
    } else if (state.dialogueOpen || state.scriptActive || state.battle.ui === "text") {
      await game.controls.press("a")
    } else {
      await game.wait.frames(12)
    }
  }
  throw new Error(
    `${description} actual Fight inputs did not finish: ${JSON.stringify(await game.state.read())}`,
  )
}

const revive = async (game: GameSession, description: string): Promise<void> => {
  await game.battle.startWild({ species: "pidgey", level: 5 })
  await waitForBattleActionMenu(game)
  const cursor = (await game.state.read()).battle.cursor ?? 0
  if (Math.floor(cursor / 2) !== 0) await game.controls.press("up")
  if (cursor % 2 === 0) await game.controls.press("right")
  await game.controls.press("a")
  await game.wait.until((state) => state.battle.ui === "bag", `${description} Revive Bag`)
  for (let pocket = 0; pocket < 5; pocket++) {
    if ((await game.state.read()).battle.bag.item === "revive") break
    await game.controls.press("right")
    await game.wait.frames(60)
  }
  expect((await game.state.read()).battle.bag.item).toBe("revive")
  await game.controls.press("a")
  await game.wait.until(
    (state) => state.battle.ui === "bag-context",
    `${description} Revive context`,
  )
  await game.controls.press("a")
  await game.wait.until(
    (state) => state.ui.mode === "party-menu",
    `${description} Revive target picker`,
  )
  await game.wait.frames(60)
  await game.controls.press("a")
  for (let attempt = 0; attempt < 30; attempt++) {
    const state = await game.state.read()
    if (!state.battle.active && state.ready) return
    await game.controls.press("a")
    await game.wait.frames(60)
  }
  await finishFieldScript(game, `${description} Revive recovery`)
}

const expectSilent = async (
  game: GameSession,
  trigger: () => Promise<void>,
  description: string,
): Promise<GameState> => {
  const before = await game.state.read()
  await trigger()
  await game.wait.frames(silentDelay)
  const after = await game.state.read()
  expect(after.battle.active, `${description} battle`).toBe(false)
  expect(after.dialogue.sequence, `${description} dialogue`).toBe(before.dialogue.sequence)
  expect(after.dialogueOpen, `${description} dialogue open`).toBe(false)
  return after
}

const moveTo = async (
  game: GameSession,
  direction: "up" | "down" | "left" | "right",
  x: number,
  y: number,
  description: string,
): Promise<void> => {
  // PlayerApi's compact press can spend the first input turning in place. Keep the
  // same direction active through a complete field movement before concluding a
  // tile is blocked.
  for (let attempt = 0; attempt < 10; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(24)
    const state = await game.state.read()
    if (state.ready && state.player.x === x && state.player.y === y) return
  }
  throw new Error(
    `${description} did not move ${direction} to ${x}:${y}; ${JSON.stringify(await game.state.read())}`,
  )
}

const moveIntoScene = async (
  game: GameSession,
  direction: "up" | "down" | "left" | "right",
  x: number,
  y: number,
  description: string,
): Promise<void> => {
  for (let attempt = 0; attempt < 10; attempt++) {
    await game.player.move(direction)
    await game.wait.frames(24)
    const state = await game.state.read()
    if (state.battle.active || state.dialogueOpen || state.scriptActive) return
    if (state.player.x === x && state.player.y === y) return
  }
  throw new Error(
    `${description} did not reach ${x}:${y} or start; ${JSON.stringify(await game.state.read())}`,
  )
}

const arrange = async (
  game: GameSession,
  map: GameMap,
  position: { x: number; y: number },
  story: StoryPatch,
  party: PartyMonFixture[],
  facing: "up" | "down" | "left" | "right" = "up",
  reviveCount = 0,
): Promise<void> => {
  await game.arrange({
    checkpoint: "new-bark-after-intro",
    player: { facing, position: { map, ...position } },
    story: patchStory(story),
    party,
    bag: reviveCount ? { items: { revive: reviveCount } } : undefined,
    determinism: { textSpeed: "instant", rngSeed: 1 },
  })
}

describe.sequential("Wayfarer trainer-only Johto story coverage", () => {
  it("silently defers each Azalea approach across the shared pre-branch no-party gate", async () => {
    const game = await GameSession.launch()
    try {
      // The gate runs before starter selection. One case per physical entrance
      // retains all approaches while also covering each starter and unusable
      // party form without multiplying the same pre-branch behavior.
      for (const [name, position, direction, starterMon, variant] of [
        ["west", { x: 10, y: 16 }, "right", 0, noPartyVariants[0]!],
        ["east", { x: 12, y: 16 }, "left", 1, noPartyVariants[1]!],
        ["south", { x: 11, y: 17 }, "up", 2, noPartyVariants[2]!],
      ] as const) {
        await arrange(
          game,
          "azalea-town",
          position,
          {
            vars: { azaleaTownState: 6, starterMon },
            flags: {
              johtoStarterChoiceCommitted: true,
              hideAzaleaSilver: false,
              [silverAzaleaComplete]: false,
            },
          },
          variant.party,
          direction,
        )
        await expectSilent(
          game,
          () => game.player.move(direction),
          `Azalea ${name} ${starterMon} ${variant.name}`,
        )
        expect(await game.story.var("azaleaTownState")).toBe(6)
        expect(await game.story.flag(silverAzaleaComplete as never)).toBe(false)
      }
    } finally {
      await game.close()
    }
  })

  it("re-arms Azalea only after recovery leaves its activation area, without rewinding Ilex progress", async () => {
    const game = await GameSession.launch()
    try {
      await arrange(
        game,
        "azalea-town",
        { x: 10, y: 16 },
        {
          vars: { azaleaTownState: 6, starterMon: 0 },
          flags: {
            johtoStarterChoiceCommitted: true,
            hideAzaleaSilver: false,
            [silverAzaleaComplete]: false,
          },
        },
        [{ species: "rattata", fainted: true }],
        "right",
        1,
      )
      await expectSilent(game, () => game.player.move("right"), "Azalea initial suppression")
      await revive(game, "Azalea deferred Silver")
      await moveTo(game, "left", 10, 16, "Azalea recovery leaves activation area")
      await moveIntoScene(game, "right", 11, 16, "Azalea recovery returns to activation area")
      await waitForTrainerBattle(game, "Azalea recovered Silver")
      expect(await game.story.var("azaleaTownState")).toBe(6)
      expect(await game.story.flag(silverAzaleaComplete as never)).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("completes Azalea only through an actual usable-party victory and keeps Ilex progress", async () => {
    const game = await GameSession.launch()
    try {
      await arrange(
        game,
        "azalea-town",
        { x: 10, y: 16 },
        {
          // Capture's native Ilex journey separately proves the host's final
          // state. This battle must still complete cleanly after that host work
          // without restoring or consuming the finished Ilex chapter.
          vars: { azaleaTownState: 7, starterMon: 0 },
          flags: {
            johtoStarterChoiceCommitted: true,
            hideAzaleaSilver: false,
            [silverAzaleaComplete]: false,
          },
        },
        // Silver's three-mon battle is deliberately won through the normal Fight
        // menu. The fixture retains a full legal party and uses the game’s
        // normal replacement flow if a member faints.
        reliableVictoryParty,
        "right",
      )
      await moveIntoScene(game, "right", 11, 16, "Azalea usable Silver")
      const result = await winActualTrainerBattle(game, "Azalea usable Silver")
      expect(result, "Azalea usable Silver real battle result").toMatchObject({
        outcome: battleOutcomeWon,
        spentMovePp: true,
      })
      await finishFieldScript(game, "Azalea usable Silver victory")
      expect(await game.story.flag(silverAzaleaComplete as never)).toBe(true)
      expect(await game.story.var("azaleaTownState")).toBe(7)
      await game.player.warp("azalea-town", 10, 16, "right")
      await expectSilent(
        game,
        () => game.player.move("right"),
        "Azalea completed Silver remains retired",
      )
      expect(await game.story.flag(silverAzaleaComplete as never)).toBe(true)
    } finally {
      await game.close()
    }
  })

  it("completes the Slowpoke Well rescue while Azalea Silver remains pending", async () => {
    const game = await GameSession.launch()
    try {
      await arrange(
        game,
        "slowpoke-well-b1f",
        { x: 14, y: 4 },
        {
          vars: { azaleaTownState: 2, starterMon: 0 },
          flags: {
            johtoStarterChoiceCommitted: true,
            hideAzaleaTownRockets: false,
            [hideSlowpokeWellKurt]: false,
            [silverAzaleaComplete]: false,
          },
        },
        reliableVictoryParty,
        "up",
      )
      await game.player.interact()
      await waitForTrainerBattle(game, "Slowpoke Well Proton")
      const result = await winActualTrainerBattle(game, "Slowpoke Well Proton")
      expect(result, "Slowpoke Well Proton real battle result").toMatchObject({
        outcome: battleOutcomeWon,
        spentMovePp: true,
      })
      await finishFieldScript(game, "Slowpoke Well Proton victory")
      expect(await game.story.var("azaleaTownState")).toBe(4)
      expect(await game.story.flag("hideAzaleaTownRockets")).toBe(true)
      expect(await game.story.flag(hideSlowpokeWellKurt as never)).toBe(true)
      expect(await game.story.flag(silverAzaleaComplete as never)).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("defers the persistent Burned Tower rival after discovery", async () => {
    const game = await GameSession.launch()
    try {
      await arrange(
        game,
        burnedTower,
        { x: 18, y: 13 },
        {
          vars: { starterMon: 0, ecruteakCityState: 4, [ecruteakCityTheater]: 0 },
          flags: {
            johtoStarterChoiceCommitted: true,
            [hideBurnedTowerNpcs]: false,
            [burnedTowerDiscovered]: true,
            [silverBurnedComplete]: false,
          },
        },
        noPartyVariants[1]!.party,
      )
      await expectSilent(game, () => game.player.interact(), "Burned Tower Silver fainted")
      expect(await game.story.flag(silverBurnedComplete as never)).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("silently defers Goldenrod Underground's two approaches and its post-occupation return actor", async () => {
    const game = await GameSession.launch()
    try {
      for (const [name, position, direction, starterMon, variant] of [
        ["top", { x: 26, y: 4 }, "down", 0, noPartyVariants[0]!],
        ["bottom", { x: 26, y: 7 }, "up", 1, noPartyVariants[1]!],
      ] as const) {
        await arrange(
          game,
          goldenrodUnderground,
          position,
          {
            vars: { goldenrodCityState: 8, starterMon },
            flags: {
              johtoStarterChoiceCommitted: true,
              [hideGoldenrodUndergroundSilver]: false,
              [silverGoldenrodComplete]: false,
            },
          },
          variant.party,
          direction,
        )
        await expectSilent(
          game,
          () => game.player.move(direction),
          `Goldenrod ${name} ${starterMon} ${variant.name}`,
        )
        expect(await game.story.var("goldenrodCityState")).toBe(8)
        expect(await game.story.flag(silverGoldenrodComplete as never)).toBe(false)
      }

      await arrange(
        game,
        goldenrodUnderground,
        { x: 35, y: 2 },
        {
          vars: { goldenrodCityState: 10, starterMon: 0 },
          flags: {
            johtoStarterChoiceCommitted: true,
            [hideGoldenrodUndergroundSilver]: false,
            [silverGoldenrodComplete]: false,
          },
        },
        [{ species: "rattata", fainted: true }],
        "up",
        1,
      )
      await revive(game, "Goldenrod restored Silver")
      // The actor was hidden while recovery occurred on its tile. Reload, step clear,
      // then return through its actual neighboring warp: reconciliation must restore it
      // exactly once without host rollback. The template's other three sides are walls.
      await game.saveAndReload()
      await moveTo(game, "left", 34, 2, "Goldenrod restored Silver leaves template tile")
      await game.player.move("right")
      await game.wait.until(
        (state) => state.map.mapGroup === 5 && state.map.mapNum === 2,
        "Goldenrod restored Silver exits actor tile through tunnel warp",
      )
      await game.wait.forReady()
      await game.player.warp(goldenrodUnderground, 34, 2, "right")
      await game.player.interact()
      await waitForTrainerBattle(game, "Goldenrod restored Silver after reload")
      expect(await game.story.var("goldenrodCityState")).toBe(10)
      expect(await game.story.flag(silverGoldenrodComplete as never)).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("keeps Silver #5-#7 pending across their remaining approach families", async () => {
    const game = await GameSession.launch()
    try {
      for (const [x, label, variant] of [
        [27, "left", noPartyVariants[0]!],
        [28, "middle", noPartyVariants[1]!],
        [29, "right", noPartyVariants[2]!],
      ] as const) {
        await arrange(
          game,
          victoryRoadKanto,
          { x, y: 8 },
          {
            vars: { [route27State]: 2, starterMon: 0 },
            flags: { johtoStarterChoiceCommitted: true },
          },
          variant.party,
          "up",
        )
        const description = `Victory Road ${label} ${variant.name}`
        await game.player.move("up")
        const refusal = await waitForDialogueText(game, "need a Pokémon", description)
        expect(refusal.battle.active).toBe(false)
        await finishFieldScript(game, description)
        expect(await game.story.var(route27State as never)).toBe(2)
      }

      await arrange(
        game,
        "mt-moon-cave",
        { x: 9, y: 12 },
        {
          vars: { starterMon: 1 },
          flags: { johtoStarterChoiceCommitted: true, hideMtMoonSilver: false },
        },
        noPartyVariants[1]!.party,
        "up",
      )
      await expectSilent(game, () => game.player.interact(), "Mt. Moon Silver fainted")
      expect(await game.story.flag("hideMtMoonSilver")).toBe(false)

      await arrange(
        game,
        indigoCenter,
        { x: 32, y: 9 },
        {
          vars: { starterMon: 2 },
          flags: {
            johtoStarterChoiceCommitted: true,
            hideIndigoPlateauSilver: false,
            [dailyBeatSilver]: false,
          },
        },
        noPartyVariants[2]!.party,
        "up",
      )
      await expectSilent(game, () => game.player.interact(), "Indigo Silver Egg-only")
      expect(await game.story.flag(dailyBeatSilver as never)).toBe(false)
    } finally {
      await game.close()
    }
  })

  it("keeps every listed local objective at its pre-battle state for no-party entry", async () => {
    const game = await GameSession.launch()
    try {
      const objectives: readonly {
        name: string
        map: GameMap
        position: { x: number; y: number }
        facing?: "up" | "down" | "left" | "right"
        vars?: Record<string, number>
        flags?: Record<string, boolean>
        party?: PartyMonFixture[]
        trigger: "talk" | "up" | "left" | "right"
        text: string
        assert?: () => Promise<void>
      }[] = [
        {
          name: "Mahogany password Grunt F",
          map: rocketHideoutB3,
          position: { x: 29, y: 10 },
          vars: { mahoganyTownState: 7 },
          flags: { [hideMahoganyRockets]: false, [gotPasswordFromGruntF]: false },
          trigger: "talk",
          text: "You've got no",
          assert: async () =>
            expect(await game.story.flag(gotPasswordFromGruntF as never)).toBe(false),
        },
        {
          name: "Mahogany password Eto",
          map: rocketHideoutB3,
          position: { x: 4, y: 17 },
          vars: { mahoganyTownState: 7 },
          flags: { [hideMahoganyRockets]: false, [gotPasswordFromEto]: false },
          trigger: "talk",
          text: "You've got no",
          assert: async () =>
            expect(await game.story.flag(gotPasswordFromEto as never)).toBe(false),
        },
        {
          name: "Mahogany disguised Petrel",
          map: rocketHideoutB3,
          position: { x: 10, y: 4 },
          vars: { mahoganyTownState: 7, [rocketPassword]: 2 },
          flags: { [hideRocketHideout3Giovanni]: true, [hideRocketHideout3Petrel]: true },
          trigger: "talk",
          text: "This is no place",
          assert: async () => expect(await game.story.var(rocketPassword as never)).toBe(2),
        },
        {
          name: "Radio fake Director",
          map: radioTower5,
          position: { x: 11, y: 10 },
          vars: { goldenrodCityState: 6 },
          flags: { [hideGoldenrodDirector]: false, [hideGoldenrodPetrel]: true },
          trigger: "talk",
          text: "This is no place",
          assert: async () => {
            expect(await game.story.flag(hideGoldenrodDirector as never)).toBe(false)
            expect(await game.story.flag(hideGoldenrodPetrel as never)).toBe(true)
          },
        },
        {
          name: "Radio Proton",
          map: radioTower4,
          position: { x: 28, y: 10 },
          vars: { goldenrodCityState: 8 },
          flags: { [hideGoldenrodRockets]: false },
          trigger: "talk",
          text: "You've got no",
        },
        {
          name: "Radio Ariana",
          map: radioTower5,
          position: { x: 28, y: 11 },
          vars: { goldenrodCityState: 8 },
          flags: { [hideGoldenrodRockets]: false },
          trigger: "talk",
          text: "You've got no",
        },
        {
          name: "Radio Archer",
          map: radioTower5,
          position: { x: 25, y: 13 },
          vars: { goldenrodCityState: 9 },
          flags: { [hideGoldenrodRockets]: false },
          trigger: "talk",
          text: "You've got no",
          assert: async () => expect(await game.story.var("goldenrodCityState")).toBe(9),
        },
        {
          name: "Theater Rocket",
          map: ecruteakTheater,
          position: { x: 10, y: 5 },
          flags: { [hideEcruteakRocket]: false },
          trigger: "talk",
          text: "You've got no",
          assert: async () =>
            expect(await game.story.flag(hideEcruteakRocket as never)).toBe(false),
        },
        {
          name: "Elder Li",
          map: sproutTower3,
          position: { x: 11, y: 4 },
          flags: { [receivedHmFlash]: false },
          trigger: "talk",
          text: "Return with",
          assert: async () => expect(await game.story.flag(receivedHmFlash as never)).toBe(false),
        },
        {
          name: "S.S. Aqua Stanly",
          map: "ss-aqua-room-nw",
          position: { x: 2, y: 7 },
          vars: { ssAquaState: 2 },
          flags: { [hideSsaquaSailor]: false },
          trigger: "talk",
          text: "Come back when",
          assert: async () => expect(await game.story.flag(hideSsaquaSailor as never)).toBe(false),
        },
        {
          name: "Kimono trial top approach",
          map: ecruteakTheater,
          position: { x: 10, y: 17 },
          vars: { [ecruteakCityTheater]: 5 },
          flags: { [hideEcruteakTheaterKimonos]: false },
          trigger: "up",
          text: "Return with",
          assert: async () => expect(await game.story.var(ecruteakCityTheater as never)).toBe(5),
        },
        {
          name: "Kimono trial right approach",
          map: ecruteakTheater,
          position: { x: 12, y: 17 },
          facing: "left",
          vars: { [ecruteakCityTheater]: 5 },
          flags: { [hideEcruteakTheaterKimonos]: false },
          trigger: "left",
          text: "Return with",
          assert: async () => expect(await game.story.var(ecruteakCityTheater as never)).toBe(5),
        },
        {
          name: "Kimono trial left approach",
          map: ecruteakTheater,
          position: { x: 8, y: 17 },
          facing: "right",
          vars: { [ecruteakCityTheater]: 5 },
          flags: { [hideEcruteakTheaterKimonos]: false },
          trigger: "right",
          text: "Return with",
          assert: async () => expect(await game.story.var(ecruteakCityTheater as never)).toBe(5),
        },
        {
          name: "Mahogany confrontation",
          map: rocketHideoutB2,
          position: { x: 15, y: 19 },
          vars: { mahoganyTownState: 12 },
          flags: { [hideRocketHideout2ArianaAndGrunt]: true },
          trigger: "up",
          text: "You've got no",
          assert: async () =>
            expect(await game.story.flag(hideRocketHideout2ArianaAndGrunt as never)).toBe(true),
        },
      ]
      for (const [index, objective] of objectives.entries()) {
        const variant = noPartyVariants[index % noPartyVariants.length]!
        const description = `${objective.name} ${variant.name} no-party`
        await arrange(
          game,
          objective.map,
          objective.position,
          { vars: objective.vars, flags: objective.flags },
          objective.party ?? variant.party,
          objective.facing,
        )
        if (objective.trigger === "talk") await game.player.interact()
        else await game.player.move(objective.trigger)
        const refusal = await waitForDialogueText(game, objective.text, description)
        expect(refusal.battle.active, `${description} battle`).toBe(false)
        await finishFieldScript(game, description)
        await objective.assert?.()
      }

      // Route 24 relocates its Rocket in the preceding nonbattle map script. Set up
      // that actual host scene first, then return to the resulting guarded actor.
      const route24Variant = noPartyVariants[2]!
      const route24Description = `Route 24 Rocket ${route24Variant.name} no-party`
      await arrange(
        game,
        "route-24",
        { x: 17, y: 12 },
        {
          vars: { kantoRocketStoryState: 3 },
          flags: { hideCeruleanCapeRocket: false, returnedMachinePart: false },
        },
        route24Variant.party,
      )
      await game.player.move("up")
      await game.wait.until(
        (state) => state.scriptActive || state.dialogueOpen,
        `${route24Description} host staging starts`,
      )
      await finishFieldScript(game, `${route24Description} host staging`)
      expect(await game.story.var("kantoRocketStoryState")).toBe(4)
      await game.player.warp("route-24", 17, 8, "left")
      await game.player.interact()
      const route24Refusal = await waitForDialogueText(game, "You've got no", route24Description)
      expect(route24Refusal.battle.active, `${route24Description} battle`).toBe(false)
      await finishFieldScript(game, route24Description)
      expect(await game.story.var("kantoRocketStoryState")).toBe(4)
    } finally {
      await game.close()
    }
  })

  it("keeps the Tohjo Giovanni/Celebi exception on its native no-Celebi cancellation path", async () => {
    const game = await GameSession.launch()
    try {
      // CheckCelebi requires a fully healthy Celebi in slot 0 and its visible follower,
      // which necessarily passes the shared usable-party predicate. These no-party cases
      // therefore exercise the episode's existing safe cancellation rather than inventing
      // an unreachable trainer-only refusal path.
      for (const [name, position, direction, variant] of [
        ["front", { x: 5, y: 6 }, "down", noPartyVariants[0]!],
        ["right", { x: 7, y: 8 }, "left", noPartyVariants[1]!],
        ["left", { x: 3, y: 8 }, "right", noPartyVariants[2]!],
      ] as const) {
        await arrange(
          game,
          tohjoGiovanni,
          position,
          {
            vars: { [tohjoGiovanniState]: 1 },
            flags: { [hideTohjoGiovanni]: true },
          },
          variant.party,
          direction,
        )
        await expectSilent(game, () => game.player.move(direction), `Tohjo ${name} ${variant.name}`)
        expect(await game.story.var(tohjoGiovanniState as never)).toBe(0)
        expect(await game.story.flag(hideTohjoGiovanni as never)).toBe(true)
      }
    } finally {
      await game.close()
    }
  })

  it("leaves excluded Electrode on its authored encounter route", async () => {
    const game = await GameSession.launch()
    try {
      await arrange(
        game,
        rocketHideoutB2,
        { x: 10, y: 15 },
        { vars: { mahoganyTownState: 13 }, flags: {} },
        [{ species: "rattata", level: 5, moves: ["tackle"] }],
        "right",
      )
      await game.player.interact()
      await waitForTrainerBattle(game, "excluded Rocket Hideout Electrode")
      expect((await game.state.read()).dialogue.text).not.toContain("You've got no")
    } finally {
      await game.close()
    }
  })

  it("returns supported Petrel losses without passwords or Mahogany completion", async () => {
    const game = await GameSession.launch()
    try {
      await arrange(
        game,
        rocketHideoutB3,
        { x: 10, y: 4 },
        {
          vars: { mahoganyTownState: 7, [rocketPassword]: 2 },
          flags: {
            [hideRocketHideout3Giovanni]: true,
            [hideRocketHideout3Petrel]: true,
            [hideMahoganyRockets]: false,
          },
        },
        [{ species: "rattata", level: 1, moves: ["tackle"] }],
        "up",
      )
      const before = await game.state.read()
      await game.player.interact()
      await waitForTrainerBattle(game, "Petrel supported battle")
      await loseActualTrainerBattle(game, "Petrel loss")
      await waitForDialogueText(game, retreatText, "Petrel retreat")
      await finishFieldScript(game, "Petrel retreat")
      expect(await game.story.var(rocketPassword as never)).toBe(2)
      expect(await game.story.var("mahoganyTownState")).toBe(7)
      expect(await game.story.flag(gotPasswordFromEto as never)).toBe(false)
      expect(await game.story.flag(gotPasswordFromGruntF as never)).toBe(false)
      expect(await game.story.flag(silverMahoganyComplete as never)).toBe(false)
      expect((await game.state.read()).money).toBe(before.money - Math.min(before.money, 8))
    } finally {
      await game.close()
    }
  })
})
