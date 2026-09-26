import { describe, expect, it } from "webanvil/test"

import { GameSession } from "../harness/game-session"
import { type GameState } from "../harness/game-session/features/state"
import {
  firstBattleLines,
  frlgPalletDialogue,
  frlgStarterMatrix,
  labExitLanes,
} from "../fixtures/kanto-frlg-contract-fixture"
import {
  approachObject,
  BattleTranscript,
  expectFidelityState,
  FieldTranscript,
  step,
  walkThrough,
  walkTo,
} from "../playbooks/kanto-frlg-contract"
import {
  type AppearanceStyle,
  appearanceStyles,
  playThroughNewGameIntro,
} from "../playbooks/new-game-intro"

// The intro helper confirms the naming screen's first preset every time.
const player = "AAAAAAA"
const text = frlgPalletDialogue(player)

const localIds = {
  signLady: 2,
  labOak: 1,
  labBlue: 5,
  route1Clerk: 9,
  route1Boy: 10,
  // Viridian's Blue-intro object is excluded from Wayfarer, so later IDs shift.
  viridianGramps: 2,
  viridianGymOldMan: 3,
  viridianYoungster: 4,
  viridianGranddaughter: 24,
  viridianBoy: 25,
  palletFatMan: 1,
  daisy: 1,
  labPokedexes: [9, 10],
} as const

const deskPokedexes = (state: GameState) =>
  state.objects.filter(
    (object) =>
      (localIds.labPokedexes as readonly number[]).includes(object.localId) && object.visible,
  )

// Oak's last POKéMON: the ball neither the player nor Blue took.
const remainingStarter = (variant: Variant) =>
  frlgStarterMatrix.find(
    (candidate) =>
      candidate.player !== variant.starter.player && candidate.player !== variant.starter.rival,
  )!

type Variant = {
  style: AppearanceStyle
  starter: (typeof frlgStarterMatrix)[number]
  lane: (typeof labExitLanes)[number]
  /** "natural" fights the battle turn by turn instead of forcing a result. */
  outcome: "win" | "lose" | "natural"
  /** The full sweep also visits every ambient FRLG interaction on the path. */
  sweep: boolean
}

// Every appearance plays the whole opening on foot; together they cover all
// three starters, all three exit lanes, and both battle outcomes.
const variants: Variant[] = [
  { style: 1, starter: frlgStarterMatrix[0], lane: 12, outcome: "win", sweep: true },
  { style: 2, starter: frlgStarterMatrix[1], lane: 13, outcome: "natural", sweep: false },
  { style: 3, starter: frlgStarterMatrix[2], lane: 14, outcome: "lose", sweep: false },
  { style: 4, starter: frlgStarterMatrix[0], lane: 13, outcome: "lose", sweep: false },
]

// Oak's coord triggers must be crossed only when the playthrough means to.
const palletNorthTriggers = [
  { x: 12, y: 1 },
  { x: 13, y: 1 },
]
const palletSignLadyTrigger = { x: 13, y: 2 }
const labExitTriggers = labExitLanes.map((x) => ({ x, y: 16 }))

const playBedroomAndHome = async (
  game: GameSession,
  script: FieldTranscript,
  style: AppearanceStyle,
): Promise<void> => {
  const girl = appearanceStyles[style].gender === 1
  await walkTo(game, { x: 4, y: 3, facing: "up" }, "bedroom NES")
  await script.interact([text.bedroom.nes], "bedroom NES")
  await walkTo(game, { x: 10, y: 2, facing: "up" }, "bedroom HELP notice")
  await script.interact([text.bedroom.help], "bedroom HELP notice")
  await walkTo(game, { x: 2, y: 2, facing: "up" }, "bedroom PC")
  await game.player.interact()
  await script.expect(text.bedroom.pcBoot, "bedroom PC boot")
  await script.advance("bedroom PC boot")
  // The bedroom PC's item storage menu is not a script choice; the script
  // hands control to it and resumes only after it closes.
  await game.wait.until(
    (state) => !state.scriptActive && !state.dialogueOpen && !state.ready,
    "bedroom PC menu",
    300,
  )
  await game.wait.frames(30)
  await game.controls.press("b")
  await script.idle("bedroom PC closed")

  await walkThrough(game, { x: 9, y: 2 }, "up", "reds-house-1f", "bedroom stairs")
  await walkTo(game, { x: 7, y: 5, facing: "up" }, "approach Mom")
  await script.interact([girl ? text.home.momGirl : text.home.momBoy], "Mom before Oak")
  await walkTo(game, { x: 4, y: 2, facing: "up" }, "approach TV")
  await script.interact([girl ? text.home.tvGirl : text.home.tvBoy], "home TV")
  await walkThrough(game, { x: 9, y: 8 }, "down", "pallet-town", "home door")
  await script.idle("Pallet doorstep")
}

const sweepPalletBeforeOak = async (game: GameSession, script: FieldTranscript) => {
  for (const sign of [
    { x: 4, y: 8, message: text.town.playerHouseSign, label: "player house sign" },
    { x: 13, y: 8, message: text.town.rivalHouseSign, label: "rival house sign" },
    { x: 9, y: 12, message: text.town.townSign, label: "town sign" },
    { x: 16, y: 17, message: text.town.oakLabSign, label: "Oak lab sign" },
  ]) {
    await walkTo(game, { x: sign.x, y: sign.y, facing: "up" }, sign.label, palletNorthTriggers)
    await script.interact([sign.message], sign.label)
  }
  await script.talkTo(
    localIds.palletFatMan,
    [text.town.technology],
    "Pallet technology NPC",
    "down",
    { x: 17, y: 17 },
  )

  // The Sign Lady guards the Trainer Tips fence until the player listens.
  await walkTo(game, { x: 4, y: 10, facing: "right" }, "Sign Lady", palletNorthTriggers)
  await script.interact([text.town.signLadyInitial, text.town.signLadyLook], "Sign Lady first talk")
  await walkTo(game, { x: 5, y: 10 }, "step beside the Sign Lady")
  await walkTo(game, { x: 5, y: 10, facing: "right" }, "face the Sign Lady")
  await script.interact([text.town.signLadyRead], "Sign Lady read it")

  // Blue's house before the escort.
  await walkThrough(
    game,
    { x: 15, y: 7 },
    "up",
    "blues-house",
    "Blue's house door",
    palletNorthTriggers,
  )
  await walkTo(game, { x: 5, y: 5, facing: "up" }, "approach Daisy")
  await script.interact([text.daisy.beforeBattle], "Daisy before the lab")
  await walkTo(game, { x: 6, y: 3, facing: "down" }, "Town Map on the table")
  await script.interact([text.daisy.displayedMap], "displayed Town Map")
  for (const x of [9, 10]) {
    await walkTo(game, { x, y: 2, facing: "up" }, "bookshelf")
    await script.interact([text.daisy.bookshelf], `bookshelf ${x}`)
  }
  for (const x of [2, 3]) {
    await walkTo(game, { x, y: 2, facing: "up" }, "picture")
    await script.interact([text.daisy.picture], `picture ${x}`)
  }
  await walkThrough(game, { x: 4, y: 8 }, "down", "pallet-town", "Blue's house exit")
  await script.idle("left Blue's house")
}

const meetOak = async (game: GameSession, script: FieldTranscript) => {
  await walkTo(game, { x: 12, y: 2 }, "approach Route 1", palletNorthTriggers)
  await step(game, "up", "step onto trigger")
  await script.say([text.town.oakWait, text.town.oakWarning], "Oak interception")
  // The arrival scene starts as the lab loads, so the map never reports ready.
  await game.wait.until((state) => state.map.name === "oak-lab", "Oak's escort into the lab", 3_000)
  await script.say(text.lab.arrival, "Oak lab arrival")
  expectFidelityState(
    await script.idle("lab staging"),
    { map: "oak-lab", x: 13, y: 12, phase: 2, partySize: 0 },
    "lab staging",
  )
}

const sweepLabBeforeStarter = async (game: GameSession, script: FieldTranscript) => {
  await walkTo(game, { x: 13, y: 12, facing: "left" }, "face Blue")
  await script.interact([text.lab.rivalWaiting], "Blue before the choice")
  await walkTo(game, { x: 13, y: 12, facing: "up" }, "face Oak")
  await script.interact([text.lab.oakWaiting], "Oak before the choice")

  // Leaving before choosing is refused at every lane by Oak.
  await walkTo(game, { x: 13, y: 15 }, "exit lane", labExitTriggers)
  await step(game, "down", "step onto trigger")
  await script.say([text.lab.leaveWithoutStarter], "leave without a starter")
  expectFidelityState(
    await script.idle("exit guard"),
    { map: "oak-lab", x: 13, y: 15, phase: 2 },
    "exit guard",
  )

  for (const wall of [
    { x: 9, message: text.lab.computer, label: "lab e-mail" },
    // The right Pokédex stands in front of the START poster until the handoff.
    { x: 12, message: text.lab.blankPokedex, label: "blank Pokédex" },
    { x: 13, message: text.lab.blankPokedex, label: "blank Pokédex by the poster" },
    { x: 14, message: text.lab.saveSign, label: "SAVE poster" },
  ]) {
    await walkTo(game, { x: wall.x, y: 10, facing: "up" }, wall.label)
    await script.interact([wall.message], wall.label)
  }
}

const chooseStarter = async (game: GameSession, script: FieldTranscript, variant: Variant) => {
  const { starter } = variant
  await walkTo(game, { x: starter.ballX, y: 13, facing: "up" }, `${starter.player} ball`)
  if (variant.sweep) {
    await game.player.interact()
    await script.ask(text.lab.choices[starter.player], "no", "decline first", starter.player)
    const declined = await script.idle("declined starter")
    expect(declined.party).toHaveLength(0)
    expect(declined.origin.pallet.phase).toBe(2)
  }
  await game.player.interact()
  await script.ask(text.lab.choices[starter.player], "yes", "confirm starter", starter.player)
  await script.say(
    [text.lab.energetic, text.lab.received[starter.player]],
    `${starter.player} receipt`,
  )
  await script.ask(text.lab.nickname[starter.player], "no", "nickname prompt")
  await script.say(
    [text.lab.rivalTake, text.lab.rivalReceived[starter.rival]],
    "Blue's counter pick",
  )
  const received = await script.idle("starter received")
  expect(received.party.map((mon) => mon.species)).toEqual([starter.player])
  expect(received.origin.pallet).toMatchObject({ phase: 3, starterSlot: starter.slot })
}

const sweepLabAfterStarter = async (
  game: GameSession,
  script: FieldTranscript,
  variant: Variant,
) => {
  await script.talkTo(
    localIds.labBlue,
    [text.lab.rivalAfterStarter],
    "Blue after the choice",
    "left",
  )
  await walkTo(game, { x: 13, y: 12, facing: "up" }, "face Oak")
  await script.interact([text.lab.oakAfterStarter], "Oak after the choice")
  const remaining = remainingStarter(variant)
  await walkTo(game, { x: remaining.ballX, y: 13, facing: "up" }, "remaining ball")
  await script.interact([text.lab.lastBall], "Oak's last POKéMON")
}

const battleBlue = async (game: GameSession, script: FieldTranscript, variant: Variant) => {
  const lines = firstBattleLines(variant.starter, player)
  const battle = new BattleTranscript(game)
  await walkTo(game, { x: variant.lane, y: 15 }, "exit lane", labExitTriggers)
  await battle.sync()
  await step(game, "down", "exit lane trigger")
  await script.say([text.lab.battleChallenge], "Blue's challenge")
  await battle.runUntil((state) => state.battle.active, "battle start", 3_000)
  const opening = await game.state.read()
  expect(opening.battle.enemy).toMatchObject({ species: variant.starter.rival, level: 5 })
  await battle.runUntil(() => battle.seen.length >= lines.intro.length, "Oak's battle introduction")
  battle.expectSeen(lines.intro, "battle introduction")

  let outcome: "win" | "lose"
  if (variant.outcome === "natural") {
    // Pressing A fights with the first move every turn, so Oak's damage
    // lesson must appear, and the battle ends in whichever branch it earns.
    await battle.runUntil((state) => !state.battle.active, "natural battle")
    const seen = battle.seen.splice(0)
    outcome = seen.includes(lines.win[0]!) ? "win" : "lose"
    expect(seen).toContain(text.lab.battleTutorial.damage)
    expect(seen.filter((line) => line.startsWith("OAK:"))).toEqual([
      text.lab.battleTutorial.damage,
      outcome === "win" ? text.lab.battleTutorial.win : text.lab.battleTutorial.loss,
    ])
    expect(seen.slice(-lines[outcome].length)).toEqual(lines[outcome])
  } else {
    outcome = variant.outcome
    await game.battle[outcome]()
    await battle.runUntil((state) => !state.battle.active, "battle end")
    battle.expectSeen(lines[outcome], `battle ${outcome}`)
  }

  await script.sync()
  await script.say([text.lab.battleAftermath], "Blue leaves after the battle")
  const after = await script.idle("battle aftermath")
  expect(after.map.name).toBe("oak-lab")
  expect(after.origin.pallet.phase).toBe(4)
  expect(after.partyVitals[0]?.hp).toBeGreaterThan(0)
}

const leaveLabAndHeal = async (game: GameSession, script: FieldTranscript, variant: Variant) => {
  if (variant.sweep) {
    await walkTo(game, { x: 13, y: 12, facing: "up" }, "face Oak")
    await script.interact([text.lab.oakAfterBattle], "Oak after the battle")
    // The aides work below the shelf gap that Oak guards until the battle.
    await script.talkTo(2, [text.lab.aide], "lab aide", "down", { x: 13, y: 18 })
    await script.talkTo(3, [text.lab.authority], "lab aide on Oak", "down", { x: 13, y: 18 })
    await script.talkTo(4, [text.lab.aide], "second lab aide", "down", { x: 13, y: 18 })
  }
  await walkThrough(game, { x: 13, y: 20 }, "down", "pallet-town", "lab door")
  await script.idle("left the lab")
  if (variant.sweep) {
    await walkThrough(
      game,
      { x: 15, y: 7 },
      "up",
      "blues-house",
      "Blue's house door",
      palletNorthTriggers,
    )
    await walkTo(game, { x: 5, y: 5, facing: "up" }, "approach Daisy")
    await script.interact([text.daisy.afterBattle], "Daisy after the lab")
    await walkThrough(game, { x: 4, y: 8 }, "down", "pallet-town", "Blue's house exit")
  }
  await walkThrough(game, { x: 6, y: 7 }, "up", "reds-house-1f", "home door", palletNorthTriggers)
  await walkTo(game, { x: 7, y: 5, facing: "up" }, "approach Mom")
  await script.interact(text.home.heal, "Mom's heal")
  await walkThrough(game, { x: 9, y: 8 }, "down", "pallet-town", "home door")
  await script.idle("left home")
}

const leavePallet = async (game: GameSession, script: FieldTranscript) => {
  // The Sign Lady waits at the Route 1 entrance with her copied tip.
  await walkTo(game, { x: 13, y: 3 }, "Route 1 entrance", [palletSignLadyTrigger])
  await step(game, "up", "step onto trigger")
  await script.say([text.town.signLadyCopied, text.town.signLadyCopiedTip], "Sign Lady copied tip")
  await script.idle("Sign Lady show")
  await walkTo(game, { x: 13, y: 2, facing: "left" }, "face the Sign Lady")
  await script.interact([text.town.signLadyDone], "Sign Lady after her show")
  await walkThrough(game, { x: 13, y: -1 }, "up", "route-1", "enter Route 1")
}

const walkRoute1North = async (game: GameSession, script: FieldTranscript, variant: Variant) => {
  if (variant.sweep) {
    await script.talkTo(
      localIds.route1Clerk,
      [text.route1.clerk, text.route1.obtainedPotion, text.route1.potionAway],
      "Route 1 Potion",
      "down",
      { x: 18, y: 30 },
    )
    expect(await game.inventory.count("potion")).toBe(1)
    await script.talkTo(localIds.route1Clerk, [text.route1.clerkRepeat], "Route 1 clerk repeat")
    await script.talkTo(localIds.route1Boy, [text.route1.ledges], "Route 1 ledge boy", "down", {
      x: 30,
      y: 18,
    })
    await walkTo(game, { x: 21, y: 32, facing: "up" }, "Route 1 sign")
    await script.interact([text.route1.sign], "Route 1 sign")
  }
  await walkThrough(game, { x: 23, y: -1 }, "up", "viridian-city", "enter Viridian")
}

const sweepViridian = async (game: GameSession, script: FieldTranscript) => {
  const viridian = text.viridian
  await script.talkTo(
    localIds.viridianGramps,
    [viridian.privateProperty],
    "coffee gramps",
    "down",
    { x: 25, y: 21 },
  )
  await script.talkTo(localIds.viridianGranddaughter, [viridian.coffee], "granddaughter", "down", {
    x: 24,
    y: 22,
  })
  await script.talkTo(localIds.viridianBoy, [viridian.carry], "Viridian boy", "down", {
    x: 19,
    y: 42,
  })
  await script.idle("before the youngster")
  await approachObject(game, localIds.viridianYoungster, "caterpillar youngster", "down", {
    x: 27,
    y: 31,
  })
  await game.player.interact()
  await script.ask(viridian.caterpillarAsk, "yes", "caterpillar youngster")
  await script.say([viridian.caterpillarYes], "caterpillar answer")
  await script.idle("caterpillar youngster")
  await walkTo(game, { x: 21, y: 22, facing: "up" }, "Viridian sign")
  await script.interact([viridian.citySign], "Viridian sign")
  await script.talkTo(localIds.viridianGymOldMan, [viridian.gymOldMan], "Gym old man", "down", {
    x: 34,
    y: 12,
  })
  await walkTo(game, { x: 45, y: 15, facing: "up" }, "Gym sign")
  await script.interact([viridian.gymSign], "Gym sign")
  // Walking past the door sideways is not an approach and stays silent.
  await walkTo(game, { x: 42, y: 15 }, "west of the Gym door", [{ x: 43, y: 15 }])
  await walkTo(game, { x: 44, y: 15 }, "past the Gym door")
  await script.idle("walked past the Gym door")
  await walkTo(game, { x: 43, y: 16 }, "Gym door approach")
  await step(game, "up", "step onto trigger")
  await script.say([viridian.gymLocked], "locked Gym door")
  expectFidelityState(await script.idle("Gym door"), { x: 43, y: 16 }, "Gym door refusal")
}

const collectParcel = async (game: GameSession, script: FieldTranscript, variant: Variant) => {
  await walkThrough(game, { x: 40, y: 27 }, "up", "viridian-mart", "Viridian Mart door")
  await script.say([...text.mart.parcel, text.mart.parcelPutAway], "Viridian Mart Parcel")
  await script.idle("Parcel received")
  expect(await game.inventory.count("oaksParcel")).toBe(1)
  expect((await game.state.read()).origin.pallet.phase).toBe(5)
  await game.saveAndReload()
  await script.sync()
  expect(await game.inventory.count("oaksParcel")).toBe(1)
  await walkTo(game, { x: 4, y: 3, facing: "left" }, "face the clerk")
  await script.interact([text.mart.repeat], "Mart clerk repeat")
  if (variant.sweep) {
    await walkTo(game, { x: 6, y: 3, facing: "up" }, "Mart youngster")
    await script.interact([text.mart.youngster], "Mart youngster")
    await walkTo(game, { x: 9, y: 6, facing: "up" }, "Mart woman")
    await script.interact([text.mart.woman], "Mart woman")
  }
  expect(await game.inventory.count("oaksParcel")).toBe(1)
  await walkThrough(game, { x: 4, y: 7 }, "down", "viridian-city", "Mart exit")
  await script.idle("left the Mart")
}

const returnToOak = async (game: GameSession, script: FieldTranscript, variant: Variant) => {
  await walkThrough(game, { x: 25, y: 50 }, "down", "route-1", "Route 1 southbound")
  await walkThrough(game, { x: 24, y: 40 }, "down", "pallet-town", "back in Pallet")
  await walkThrough(game, { x: 16, y: 13 }, "up", "oak-lab", "lab door")
  await walkTo(game, { x: 13, y: 12, facing: "up" }, "approach Oak")
  // FRLG's two Pokédex units wait on the desk until Oak hands them over.
  expect(deskPokedexes(await game.state.read()).map((unit) => [unit.x, unit.y])).toEqual([
    [12, 9],
    [13, 9],
  ])
  await game.player.interact()
  await script.say(text.parcelReturn, "Parcel, Pokédex, and Poké Balls")
  const complete = await script.idle("opening complete")
  expect(deskPokedexes(complete)).toEqual([])
  expect(complete.origin.pallet.phase).toBe(9)
  expect(complete.origin.pokedex).toBe(true)
  expect(await game.inventory.count("oaksParcel")).toBe(0)
  expect(await game.inventory.count("pokeBall")).toBe(5)
  expect(await game.inventory.count("townMap")).toBe(0)
  await game.saveAndReload()
  await script.sync()
  expect(await game.inventory.count("pokeBall")).toBe(5)
  const reloaded = await game.state.read()
  expect(reloaded.origin.pallet.phase).toBe(9)
  expect(deskPokedexes(reloaded)).toEqual([])
  // As in FRLG, Oak's last POKéMON stays on the table after the opening.
  const lastBall = reloaded.objects.find(
    (object) => object.x === remainingStarter(variant).ballX && object.y === 12,
  )
  expect(lastBall?.visible).toBe(true)
}

// Daisy's Town Map belongs to a later Kanto specification; the handoff only
// switches the lab's SAVE poster to FRLG's post-Pokédex line.
const afterHandoff = async (game: GameSession, script: FieldTranscript, variant: Variant) => {
  await walkTo(game, { x: 13, y: 10, facing: "up" }, "START poster")
  await script.interact([text.lab.menuSign], "START poster after the Pokédexes left")
  await walkTo(game, { x: 14, y: 10, facing: "up" }, "SAVE poster")
  await script.interact([text.lab.typeSign], "post-opening types poster")
  await walkTo(game, { x: remainingStarter(variant).ballX, y: 13, facing: "up" }, "last ball")
  await script.interact([text.lab.lastBall], "Oak's last POKéMON after the opening")
}

describe.sequential("FRLG Pallet opening, verbatim on HNS maps", () => {
  for (const variant of variants) {
    it(`Style ${variant.style}: ${variant.starter.player}, lane ${variant.lane}, ${variant.outcome} — new game to the Pokédex handoff`, async () => {
      const game = await GameSession.launch()
      const script = new FieldTranscript(game)
      try {
        await playThroughNewGameIntro(game, "kanto", variant.style)
        expectFidelityState(
          await game.state.read(),
          { map: "reds-house-2f", x: 5, y: 5, phase: 0, partySize: 0 },
          "new-game bedroom",
        )
        await script.sync()
        await game.story.setFlag("disableEncounters", true)

        await playBedroomAndHome(game, script, variant.style)
        if (variant.sweep) await sweepPalletBeforeOak(game, script)
        await meetOak(game, script)
        if (variant.sweep) await sweepLabBeforeStarter(game, script)
        await chooseStarter(game, script, variant)
        if (variant.sweep) await sweepLabAfterStarter(game, script, variant)
        await battleBlue(game, script, variant)
        await leaveLabAndHeal(game, script, variant)
        await leavePallet(game, script)
        await walkRoute1North(game, script, variant)
        if (variant.sweep) await sweepViridian(game, script)
        await collectParcel(game, script, variant)
        await returnToOak(game, script, variant)
        if (variant.sweep) await afterHandoff(game, script, variant)
      } finally {
        await game.close()
      }
    }, 1_500_000)
  }

  it("Style 2: unopened lab before Oak's escort", async () => {
    const game = await GameSession.launch()
    const script = new FieldTranscript(game)
    try {
      await playThroughNewGameIntro(game, "kanto", 2)
      await script.sync()
      await walkThrough(game, { x: 9, y: 2 }, "up", "reds-house-1f", "bedroom stairs")
      await walkThrough(game, { x: 9, y: 8 }, "down", "pallet-town", "home door")
      await walkThrough(game, { x: 16, y: 13 }, "up", "oak-lab", "lab door", palletNorthTriggers)
      await script.talkTo(
        localIds.labBlue,
        [text.lab.grampsIsntAround],
        "Blue before the escort",
        "down",
        { x: 13, y: 14 },
      )
      await walkTo(game, { x: 15, y: 13, facing: "up" }, "unopened ball")
      await script.interact([text.lab.pokeBalls], "unopened Poké Balls")
      await walkTo(game, { x: 13, y: 15 }, "exit lane")
      await step(game, "down", "step onto trigger")
      await game.wait.frames(60)
      await script.idle("no guard before the escort")
      await walkThrough(game, { x: 13, y: 20 }, "down", "pallet-town", "lab door")
      await walkTo(game, { x: 4, y: 10, facing: "right" }, "Sign Lady", palletNorthTriggers)
      await script.interact(
        [text.town.signLadyInitial, text.town.signLadyLook],
        "Sign Lady first talk",
      )
      await walkTo(game, { x: 5, y: 10, facing: "down" }, "Trainer Tips fence")
      await script.interact([text.town.trainerTips], "Trainer Tips")
    } finally {
      await game.close()
    }
  }, 600_000)
})
